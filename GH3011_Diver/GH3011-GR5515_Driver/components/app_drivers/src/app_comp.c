/**
  ****************************************************************************************
  * @file    app_comp.c
  * @author  BLE Driver Team
  * @brief   HAL APP module driver.
  ****************************************************************************************
  * @attention
  #####Copyright (c) 2019 GOODIX
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   * Neither the name of GOODIX nor the names of its contributors may be used
     to endorse or promote products derived from this software without
     specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
  ****************************************************************************************
  */
/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "app_comp.h"
#include "app_pwr_mgmt.h"
#include "app_systick.h"
#include "string.h"
#include "platform_sdk.h"

#ifdef HAL_COMP_MODULE_ENABLED

/*
 * DEFINES
 *****************************************************************************************
 */

#ifdef ENV_RTOS_USE_MUTEX

#define APP_COMP_DRV_MUTEX_LOCK     app_driver_mutex_pend(s_comp_env.mutex_comp, MUTEX_WAIT_FOREVER)
#define APP_COMP_DRV_MUTEX_UNLOCK   app_driver_mutex_post(s_comp_env.mutex_comp)

#endif

/*
 * STRUCT DEFINE
 *****************************************************************************************
 */

/**@brief App comp state types. */
typedef enum
{
    APP_COMP_INVALID = 0,
    APP_COMP_ACTIVITY,
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    APP_COMP_SLEEP,
#endif
} app_comp_state_t;

struct comp_env_t
{
    app_comp_evt_handler_t  evt_handler;
    comp_handle_t           handle;
    app_comp_pin_cfg_t      pin_cfg;
    app_comp_state_t        comp_state;
#ifdef ENV_RTOS_USE_SEMP   
    APP_DRV_SEM_DECL(sem_rx);
#endif

#ifdef ENV_RTOS_USE_MUTEX
    APP_DRV_MUTEX_DECL(mutex_comp);
#endif
};

/*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static bool comp_prepare_for_sleep(void);
static void comp_sleep_canceled(void);
static void comp_wake_up_ind(void);
static uint16_t comp_config_gpio(uint32_t ref_source, app_comp_pin_cfg_t pin_cfg);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
struct comp_env_t s_comp_env = {
    .evt_handler = NULL,
#ifdef ENV_RTOS_USE_SEMP   
    .sem_rx = NULL,
#endif
#ifdef ENV_RTOS_USE_MUTEX
    .mutex_comp = NULL,
#endif
};

static bool s_sleep_cb_registered_flag = false;
static pwr_id_t s_comp_pwr_id;

const static app_sleep_callbacks_t comp_sleep_cb =
{
    .app_prepare_for_sleep = comp_prepare_for_sleep,
    .app_sleep_canceled = comp_sleep_canceled,
    .app_wake_up_ind = comp_wake_up_ind
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */

static bool comp_prepare_for_sleep(void)
{
    if (s_comp_env.comp_state == APP_COMP_ACTIVITY)
    {
        GLOBAL_EXCEPTION_DISABLE();
        hal_comp_suspend_reg(&s_comp_env.handle);
        GLOBAL_EXCEPTION_ENABLE();

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
        s_comp_env.comp_state = APP_COMP_SLEEP;
#endif
    }

    return true;
}

static void comp_sleep_canceled(void)
{
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    if (s_comp_env.comp_state == APP_COMP_SLEEP)
    {
        s_comp_env.comp_state = APP_COMP_ACTIVITY;
    }
#endif
}

SECTION_RAM_CODE static void comp_wake_up_ind(void)
{
#ifndef APP_DRIVER_WAKEUP_CALL_FUN
    if (s_comp_env.comp_state == APP_COMP_ACTIVITY)
    {
        GLOBAL_EXCEPTION_DISABLE();
        hal_comp_resume_reg(&s_comp_env.handle);
        GLOBAL_EXCEPTION_ENABLE();
    }
#endif
}

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
static void comp_wake_up(void)
{
    if (s_comp_env.comp_state == APP_COMP_SLEEP)
    {
        GLOBAL_EXCEPTION_DISABLE();
        hal_comp_resume_reg(&s_comp_env.handle);
        GLOBAL_EXCEPTION_ENABLE();

        hal_nvic_clear_pending_irq(COMP_EXT_IRQn);
        hal_nvic_enable_irq(COMP_EXT_IRQn);
        s_comp_env.comp_state = APP_COMP_ACTIVITY;
    }
}
#endif

static uint16_t comp_config_gpio(uint32_t ref_source, app_comp_pin_cfg_t pin_cfg)
{
    app_io_init_t io_init = APP_IO_DEFAULT_CONFIG;
    app_drv_err_t err_code = APP_DRV_SUCCESS;

    io_init.mode = APP_IO_MODE_ANALOG;
    io_init.pin  = pin_cfg.input.pin;
    io_init.mux  = pin_cfg.input.mux;
    err_code = app_io_init(pin_cfg.input.type, &io_init);
    APP_DRV_ERR_CODE_CHECK(err_code);

    if (ref_source != COMP_REF_SRC_VBAT && ref_source != COMP_REF_SRC_VREF)
    {
        io_init.pin  = pin_cfg.vref.pin;
        io_init.mux  = pin_cfg.vref.mux;
        err_code = app_io_init(pin_cfg.vref.type, &io_init);
        APP_DRV_ERR_CODE_CHECK(err_code);
    }

    return err_code;
}

static void app_comp_event_call(comp_handle_t *p_comp, app_comp_evt_t evt_type)
{
    app_comp_evt_t comp_evt = APP_COMP_EVT_ERROR;

    if (evt_type == APP_COMP_EVT_DONE)
    {
#ifdef  ENV_RTOS_USE_SEMP         
        app_driver_sem_post_from_isr(s_comp_env.sem_rx);
#endif 
        comp_evt = APP_COMP_EVT_DONE;
    }

    if (s_comp_env.evt_handler != NULL)
    {
        s_comp_env.evt_handler(&comp_evt);
    }
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
uint16_t app_comp_init(app_comp_params_t *p_params, app_comp_evt_handler_t evt_handler)
{
    app_drv_err_t app_err_code;
    hal_status_t  hal_err_code;

    if (p_params == NULL)
    {
        return APP_DRV_ERR_POINTER_NULL;
    }
    
#ifdef  ENV_RTOS_USE_SEMP
    if(s_comp_env.sem_rx == NULL)
    {
        app_err_code = app_driver_sem_init(&s_comp_env.sem_rx);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
#endif

#ifdef ENV_RTOS_USE_MUTEX
    if(s_comp_env.mutex_comp == NULL)
    {
        app_err_code = app_driver_mutex_init(&s_comp_env.mutex_comp);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
#endif

    app_err_code = comp_config_gpio(p_params->init.ref_source, p_params->pin_cfg);
    APP_DRV_ERR_CODE_CHECK(app_err_code);

    hal_nvic_clear_pending_irq(COMP_EXT_IRQn);
    hal_nvic_enable_irq(COMP_EXT_IRQn);

    memcpy(&s_comp_env.pin_cfg, &p_params->pin_cfg, sizeof(app_comp_pin_cfg_t));
    s_comp_env.evt_handler = evt_handler;

    memcpy(&s_comp_env.handle.init, &p_params->init, sizeof(comp_init_t));
    hal_err_code = hal_comp_deinit(&s_comp_env.handle);
    APP_DRV_ERR_CODE_CHECK(hal_err_code);

    hal_err_code = hal_comp_init(&s_comp_env.handle);
    APP_DRV_ERR_CODE_CHECK(hal_err_code);

    if(s_sleep_cb_registered_flag == false)    // register sleep callback
    {
        s_sleep_cb_registered_flag = true;
        s_comp_pwr_id = pwr_register_sleep_cb(&comp_sleep_cb, APP_DRIVER_COMP_WAPEUP_PRIORITY);
        if (s_comp_pwr_id < 0)
        {
            return APP_DRV_ERR_INVALID_PARAM;
        }
    }

    s_comp_env.comp_state = APP_COMP_ACTIVITY;

    return 0;
}

uint16_t app_comp_deinit(void)
{
    hal_status_t  hal_err_code;

    if (s_comp_env.comp_state == APP_COMP_INVALID)
    {
        return APP_DRV_ERR_INVALID_ID;
    }

#ifdef  ENV_RTOS_USE_SEMP
    if(s_comp_env.sem_rx != NULL)
    {
        app_driver_sem_deinit(s_comp_env.sem_rx);
        s_comp_env.sem_rx = NULL;
    }
#endif

#ifdef ENV_RTOS_USE_MUTEX
    if(s_comp_env.mutex_comp != NULL)
    {
        app_driver_mutex_deinit(s_comp_env.mutex_comp);
        s_comp_env.mutex_comp = NULL;
    }
#endif

    hal_nvic_disable_irq(COMP_EXT_IRQn);
    s_comp_env.comp_state = APP_COMP_INVALID;

    GLOBAL_EXCEPTION_DISABLE();
    pwr_unregister_sleep_cb(s_comp_pwr_id);
    s_sleep_cb_registered_flag = false;
    GLOBAL_EXCEPTION_ENABLE();

    hal_err_code = hal_comp_deinit(&s_comp_env.handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_comp_start(void)
{
    hal_status_t  hal_err_code;

    if (s_comp_env.comp_state == APP_COMP_INVALID)
    {
        return APP_DRV_ERR_INVALID_ID;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    comp_wake_up();
#endif

    pwr_mgmt_wakeup_source_setup(PWR_WKUP_COND_MSIO_COMP);
    hal_err_code = hal_comp_start(&s_comp_env.handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    return APP_DRV_SUCCESS;
}

#ifdef  ENV_RTOS_USE_SEMP
uint16_t app_comp_sem_start(void)
{
    hal_status_t  hal_err_code;

#ifdef ENV_RTOS_USE_MUTEX
    APP_COMP_DRV_MUTEX_LOCK;
#endif

    if (s_comp_env.comp_state == APP_COMP_INVALID)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_COMP_DRV_MUTEX_UNLOCK;
#endif
        return APP_DRV_ERR_INVALID_ID;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    comp_wake_up();
#endif

    pwr_mgmt_wakeup_source_setup(PWR_WKUP_COND_MSIO_COMP);
    hal_err_code = hal_comp_start(&s_comp_env.handle);
    if (hal_err_code != HAL_OK)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_COMP_DRV_MUTEX_UNLOCK;
#endif
        return (uint16_t)hal_err_code;
    }

    app_driver_sem_pend(s_comp_env.sem_rx, OS_WAIT_FOREVER);

#ifdef ENV_RTOS_USE_MUTEX
    APP_COMP_DRV_MUTEX_UNLOCK;
#endif

    return APP_DRV_SUCCESS;
}
#endif

uint16_t app_comp_stop(void)
{
    hal_status_t  hal_err_code;

    if (s_comp_env.comp_state == APP_COMP_INVALID)
    {
        return APP_DRV_ERR_INVALID_ID;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    comp_wake_up();
#endif

    pwr_mgmt_wakeup_source_clear(PWR_WKUP_COND_MSIO_COMP);
    hal_err_code = hal_comp_stop(&s_comp_env.handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    return APP_DRV_SUCCESS;
}

comp_handle_t *app_comp_get_handle(void)
{
    if (s_comp_env.comp_state == APP_COMP_INVALID)
    {
        return NULL;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    comp_wake_up();
#endif

    return &s_comp_env.handle;
}

void hal_comp_trigger_callback(comp_handle_t *p_comp)
{
    app_comp_event_call(p_comp, APP_COMP_EVT_DONE);
}

SECTION_RAM_CODE void COMP_IRQHandler(void)
{
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_push();
#endif
    hal_comp_irq_handler(&s_comp_env.handle);
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_pop();
#endif
}

#endif
