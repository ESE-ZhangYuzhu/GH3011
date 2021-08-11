/**
  ****************************************************************************************
  * @file    app_pkc.c
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
#include "app_pkc.h"
#include "app_pwr_mgmt.h"
#include "app_systick.h"
#include "string.h"
#include "platform_sdk.h"

#ifdef HAL_PKC_MODULE_ENABLED
/*
 * DEFINES
 *****************************************************************************************
 */
 #ifdef ENV_RTOS_USE_MUTEX

#define APP_PKC_DRV_SYNC_MUTEX_LOCK     app_driver_mutex_pend(s_pkc_env.mutex_sync, MUTEX_WAIT_FOREVER)
#define APP_PKC_DRV_SYNC_MUTEX_UNLOCK   app_driver_mutex_post(s_pkc_env.mutex_sync)

#define APP_PKC_DRV_ASYNC_MUTEX_LOCK    app_driver_mutex_pend(s_pkc_env.mutex_async, MUTEX_WAIT_FOREVER)
#define APP_PKC_DRV_ASYNC_MUTEX_UNLOCK  app_driver_mutex_post(s_pkc_env.mutex_async)

#endif
/*
 * STRUCT DEFINE
 *****************************************************************************************
 */

/**@brief App pkc state types. */

typedef enum
{
    APP_PKC_INVALID = 0,
    APP_PKC_ACTIVITY,
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    APP_PKC_SLEEP,
#endif
} app_pkc_state_t;

struct pkc_env_t
{
    app_pkc_evt_handler_t   evt_handler;
    pkc_handle_t            handle;
    app_pkc_type_t          ues_type;
    app_pkc_state_t         pkc_state;
    bool                    start_flag;
#ifdef ENV_RTOS_USE_SEMP   
    APP_DRV_SEM_DECL(sem_rx);
#endif

#ifdef ENV_RTOS_USE_MUTEX
    APP_DRV_MUTEX_DECL(mutex_sync);
    APP_DRV_MUTEX_DECL(mutex_async);
#endif
};

/*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static bool pkc_prepare_for_sleep(void);
static void pkc_sleep_canceled(void);
static void pkc_wake_up_ind(void);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
struct pkc_env_t s_pkc_env = {
    .evt_handler = NULL,
#ifdef ENV_RTOS_USE_SEMP   
    .sem_rx = NULL,
#endif
#ifdef ENV_RTOS_USE_MUTEX
    .mutex_sync = NULL,
    .mutex_async = NULL,
#endif
};
static bool s_sleep_cb_registered_flag = false;
static pwr_id_t s_pkc_pwr_id;

const static app_sleep_callbacks_t pkc_sleep_cb =
{
    .app_prepare_for_sleep = pkc_prepare_for_sleep,
    .app_sleep_canceled = pkc_sleep_canceled,
    .app_wake_up_ind = pkc_wake_up_ind
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
static bool pkc_prepare_for_sleep(void)
{
    hal_pkc_state_t state;

    if (s_pkc_env.pkc_state == APP_PKC_ACTIVITY)
    {
        state = hal_pkc_get_state(&s_pkc_env.handle);
        if ((state != HAL_PKC_STATE_READY) && (state != HAL_PKC_STATE_RESET))
        {
            return false;
        }

        GLOBAL_EXCEPTION_DISABLE();
        hal_pkc_suspend_reg(&s_pkc_env.handle);
        GLOBAL_EXCEPTION_ENABLE();

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
        s_pkc_env.pkc_state = APP_PKC_SLEEP;
#endif
    }

    return true;
}

static void pkc_sleep_canceled(void)
{
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    if (s_pkc_env.pkc_state == APP_PKC_SLEEP)
    {
        s_pkc_env.pkc_state = APP_PKC_ACTIVITY;
    }
#endif
}

SECTION_RAM_CODE static void pkc_wake_up_ind(void)
{
#ifndef APP_DRIVER_WAKEUP_CALL_FUN
    if (s_pkc_env.pkc_state == APP_PKC_ACTIVITY)
    {
        GLOBAL_EXCEPTION_DISABLE();
        hal_pkc_resume_reg(&s_pkc_env.handle);
        GLOBAL_EXCEPTION_ENABLE();

        if (s_pkc_env.ues_type != APP_PKC_TYPE_POLLING)
        {
            hal_nvic_clear_pending_irq(PKC_IRQn);
            hal_nvic_enable_irq(PKC_IRQn);
        }
    }
#endif
}

#ifdef APP_DRIVER_WAKEUP_CALL_FUN

static void pkc_wake_up(void)
{
    if (s_pkc_env.pkc_state == APP_PKC_SLEEP)
    {
        GLOBAL_EXCEPTION_DISABLE();
        hal_pkc_resume_reg(&s_pkc_env.handle);
        GLOBAL_EXCEPTION_ENABLE();

        if (s_pkc_env.ues_type != APP_PKC_TYPE_POLLING)
        {
            hal_nvic_clear_pending_irq(PKC_IRQn);
            hal_nvic_enable_irq(PKC_IRQn);
        }
        s_pkc_env.pkc_state = APP_PKC_ACTIVITY;
    }
}
#endif

static void app_pkc_event_call(pkc_handle_t *p_pkc, app_pkc_evt_type_t evt_type)
{
    app_pkc_evt_t pkc_evt = { APP_PKC_EVT_ERROR, 0};

    pkc_evt.type = evt_type;
    s_pkc_env.start_flag = false;
    if(evt_type == APP_PKC_EVT_ERROR)
    {
        pkc_evt.error_code = p_pkc->error_code;
    }

#ifdef  ENV_RTOS_USE_SEMP
    app_driver_sem_post_from_isr(s_pkc_env.sem_rx);
#endif

    if (s_pkc_env.evt_handler != NULL)
    {
        s_pkc_env.evt_handler(&pkc_evt);
    }
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
 
uint16_t app_pkc_init(app_pkc_params_t *p_params, app_pkc_evt_handler_t evt_handler)
{
    app_drv_err_t app_err_code = APP_DRV_SUCCESS;
    hal_status_t  hal_err_code = HAL_OK;

    if (p_params == NULL)
    {
        return APP_DRV_ERR_POINTER_NULL;
    }

    app_systick_init();
    
#ifdef  ENV_RTOS_USE_SEMP
    if(s_pkc_env.sem_rx == NULL)
    {
        app_err_code = app_driver_sem_init(&s_pkc_env.sem_rx);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
#endif

#ifdef ENV_RTOS_USE_MUTEX
    if(s_pkc_env.mutex_sync == NULL)
    {
        app_err_code = app_driver_mutex_init(&s_pkc_env.mutex_sync);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
    if(s_pkc_env.mutex_async == NULL)
    {
        app_err_code = app_driver_mutex_init(&s_pkc_env.mutex_async);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
#endif

    if (p_params->use_type != APP_PKC_TYPE_POLLING)
    {
        hal_nvic_clear_pending_irq(PKC_IRQn);
        hal_nvic_enable_irq(PKC_IRQn);
    }

    s_pkc_env.ues_type = p_params->use_type;
    s_pkc_env.evt_handler = evt_handler;
    s_pkc_env.handle.p_result = p_params->p_result;
    s_pkc_env.handle.p_kout = p_params->p_kout;

    memcpy(&s_pkc_env.handle.init, &p_params->init, sizeof(pkc_init_t));
    s_pkc_env.handle.p_instance = PKC;
    hal_err_code = hal_pkc_deinit(&s_pkc_env.handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    hal_err_code = hal_pkc_init(&s_pkc_env.handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    if(s_sleep_cb_registered_flag == false)    // register sleep callback
    {
        s_sleep_cb_registered_flag = true;
        s_pkc_pwr_id = pwr_register_sleep_cb(&pkc_sleep_cb, APP_DRIVER_ADC_WAPEUP_PRIORITY);
        if (s_pkc_pwr_id < 0)
        {
            return APP_DRV_ERR_INVALID_PARAM;
        }
    }

    s_pkc_env.pkc_state = APP_PKC_ACTIVITY;

    return app_err_code;
}
 
uint16_t app_pkc_deinit(void)
{
    hal_status_t  hal_err_code;

    if (s_pkc_env.pkc_state == APP_PKC_INVALID)
    {
        return APP_DRV_ERR_INVALID_ID;
    }

#ifdef  ENV_RTOS_USE_SEMP
    if(s_pkc_env.sem_rx != NULL)
    {
        app_driver_sem_deinit(s_pkc_env.sem_rx);
        s_pkc_env.sem_rx = NULL;
    }
#endif

#ifdef ENV_RTOS_USE_MUTEX
    if(s_pkc_env.mutex_sync != NULL)
    {
        app_driver_mutex_deinit(s_pkc_env.mutex_sync);
        s_pkc_env.mutex_sync = NULL;
    }
    if(s_pkc_env.mutex_async != NULL)
    {
        app_driver_mutex_deinit(s_pkc_env.mutex_async);
        s_pkc_env.mutex_async = NULL;
    }
#endif

    hal_nvic_disable_irq(PKC_IRQn);
    s_pkc_env.pkc_state = APP_PKC_INVALID;

    GLOBAL_EXCEPTION_DISABLE();
    pwr_unregister_sleep_cb(s_pkc_pwr_id);
    s_sleep_cb_registered_flag = false;
    GLOBAL_EXCEPTION_ENABLE();

    app_systick_deinit();

    hal_err_code =  hal_pkc_deinit(&s_pkc_env.handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    return APP_DRV_SUCCESS;
}
 
uint16_t app_pkc_ecc_point_multi_sync(app_pkc_ecc_point_multi_t *p_input, uint32_t timeout)
{
    hal_status_t err_code = HAL_OK;

    if (s_pkc_env.pkc_state == APP_PKC_INVALID ||
        p_input == NULL)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pkc_wake_up();
#endif

	err_code = hal_pkc_ecc_point_multi(&s_pkc_env.handle, p_input, timeout);
	HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_pkc_ecc_point_multi_async(app_pkc_ecc_point_multi_t *p_input)
{
    hal_status_t err_code = HAL_OK;

    if (s_pkc_env.pkc_state == APP_PKC_INVALID ||
        p_input == NULL ||
        s_pkc_env.ues_type == APP_PKC_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pkc_wake_up();
#endif

    err_code = hal_pkc_ecc_point_multi_it(&s_pkc_env.handle, p_input);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_pkc_modular_add_sync(app_pkc_modular_add_t *p_input, uint32_t timeout)
{
    hal_status_t err_code = HAL_OK;

    if (s_pkc_env.pkc_state == APP_PKC_INVALID ||
        p_input == NULL)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pkc_wake_up();
#endif

    err_code = hal_pkc_modular_add(&s_pkc_env.handle, p_input, timeout);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_pkc_modular_add_async(app_pkc_modular_add_t *p_input)
{
    hal_status_t err_code = HAL_OK;

    if (s_pkc_env.pkc_state == APP_PKC_INVALID ||
        p_input == NULL ||
        s_pkc_env.ues_type == APP_PKC_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pkc_wake_up();
#endif

    err_code = hal_pkc_modular_add_it(&s_pkc_env.handle, p_input);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_pkc_modular_sub_sync(app_pkc_modular_sub_t *p_input, uint32_t timeout)
{
    hal_status_t err_code = HAL_OK;

    if (s_pkc_env.pkc_state == APP_PKC_INVALID ||
        p_input == NULL)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pkc_wake_up();
#endif

    err_code = hal_pkc_modular_sub(&s_pkc_env.handle, p_input, timeout);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_pkc_modular_sub_async(app_pkc_modular_sub_t *p_input)
{
    hal_status_t err_code = HAL_OK;

    if (s_pkc_env.pkc_state == APP_PKC_INVALID ||
        p_input == NULL ||
        s_pkc_env.ues_type == APP_PKC_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pkc_wake_up();
#endif

    err_code = hal_pkc_modular_sub_it(&s_pkc_env.handle, p_input);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_pkc_modular_left_shift_sync(app_pkc_modular_shift_t *p_input, uint32_t timeout)
{
    hal_status_t err_code = HAL_OK;

    if (s_pkc_env.pkc_state == APP_PKC_INVALID ||
        p_input == NULL)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pkc_wake_up();
#endif

    err_code = hal_pkc_modular_left_shift(&s_pkc_env.handle, p_input, timeout);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_pkc_modular_left_shift_async(app_pkc_modular_shift_t *p_input)
{
    hal_status_t err_code = HAL_OK;

    if (s_pkc_env.pkc_state == APP_PKC_INVALID ||
        p_input == NULL ||
        s_pkc_env.ues_type == APP_PKC_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pkc_wake_up();
#endif

    err_code = hal_pkc_modular_left_shift_it(&s_pkc_env.handle, p_input);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_pkc_modular_compare_sync(app_pkc_modular_compare_t *p_input, uint32_t timeout)
{
    hal_status_t err_code = HAL_OK;

    if (s_pkc_env.pkc_state == APP_PKC_INVALID ||
        p_input == NULL)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pkc_wake_up();
#endif

    err_code = hal_pkc_modular_compare(&s_pkc_env.handle, p_input, timeout);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_pkc_modular_compare_async(app_pkc_modular_compare_t *p_input)
{
    hal_status_t err_code = HAL_OK;

    if (s_pkc_env.pkc_state == APP_PKC_INVALID ||
        p_input == NULL ||
        s_pkc_env.ues_type == APP_PKC_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pkc_wake_up();
#endif

    err_code =  hal_pkc_modular_compare_it(&s_pkc_env.handle, p_input);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_pkc_montgomery_multi_sync(app_pkc_montgomery_multi_t *p_input, uint32_t timeout)
{
    hal_status_t err_code = HAL_OK;

    if (s_pkc_env.pkc_state == APP_PKC_INVALID ||
        p_input == NULL)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pkc_wake_up();
#endif

    err_code = hal_pkc_montgomery_multi(&s_pkc_env.handle, p_input, timeout);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_pkc_montgomery_multi_async(app_pkc_montgomery_multi_t *p_input)
{
    hal_status_t err_code = HAL_OK;

    if (s_pkc_env.pkc_state == APP_PKC_INVALID ||
        p_input == NULL ||
        s_pkc_env.ues_type == APP_PKC_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pkc_wake_up();
#endif

    err_code =  hal_pkc_montgomery_multi_it(&s_pkc_env.handle, p_input);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_pkc_montgomery_inversion_sync(app_pkc_montgomery_inversion_t *p_input, uint32_t *p_K, uint32_t timeout)
{
    hal_status_t err_code = HAL_OK;

    if (s_pkc_env.pkc_state == APP_PKC_INVALID ||
        p_input == NULL)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pkc_wake_up();
#endif

    err_code = hal_pkc_montgomery_inversion(&s_pkc_env.handle, p_input, p_K, timeout);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_pkc_montgomery_inversion_async(app_pkc_montgomery_inversion_t *p_input, uint32_t *p_K)
{
    hal_status_t err_code = HAL_OK;

    if (s_pkc_env.pkc_state == APP_PKC_INVALID ||
        p_input == NULL ||
        s_pkc_env.ues_type == APP_PKC_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pkc_wake_up();
#endif

    err_code =  hal_pkc_montgomery_inversion_it(&s_pkc_env.handle, p_input, p_K);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_pkc_big_number_multi_sync(app_pkc_big_number_multi_t *p_input, uint32_t timeout)
{
    hal_status_t err_code = HAL_OK;

    if (s_pkc_env.pkc_state == APP_PKC_INVALID ||
        p_input == NULL)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pkc_wake_up();
#endif

    err_code = hal_pkc_big_number_multi(&s_pkc_env.handle, p_input, timeout);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_pkc_big_number_multi_async(app_pkc_big_number_multi_t *p_input)
{
    hal_status_t err_code = HAL_OK;

    if (s_pkc_env.pkc_state == APP_PKC_INVALID ||
        p_input == NULL ||
        s_pkc_env.ues_type == APP_PKC_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pkc_wake_up();
#endif

    err_code =  hal_pkc_big_number_multi_it(&s_pkc_env.handle, p_input);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_pkc_big_number_add_sync(app_pkc_big_number_add_t *p_input, uint32_t timeout)
{
    hal_status_t err_code = HAL_OK;

    if (s_pkc_env.pkc_state == APP_PKC_INVALID ||
        p_input == NULL)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pkc_wake_up();
#endif

    err_code = hal_pkc_big_number_add(&s_pkc_env.handle, p_input, timeout);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_pkc_big_number_add_async(app_pkc_big_number_add_t *p_input)
{
    hal_status_t err_code = HAL_OK;

    if (s_pkc_env.pkc_state == APP_PKC_INVALID ||
        p_input == NULL ||
        s_pkc_env.ues_type == APP_PKC_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pkc_wake_up();
#endif

    err_code =  hal_pkc_big_number_add_it(&s_pkc_env.handle, p_input);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

pkc_handle_t *app_pkc_get_handle(void)
{
    if (s_pkc_env.pkc_state == APP_PKC_INVALID)
    {
        return NULL;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pkc_wake_up();
#endif

    return &s_pkc_env.handle;
}

void hal_pkc_done_callback(pkc_handle_t *p_pkc)
{
    app_pkc_event_call(p_pkc, APP_PKC_EVT_DONE);
}

void hal_pkc_error_callback(pkc_handle_t *p_pkc)
{
    app_pkc_event_call(p_pkc, APP_PKC_EVT_ERROR);
}

SECTION_RAM_CODE void PKC_IRQHandler(void)
{
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_push();
#endif
    hal_pkc_irq_handler(&s_pkc_env.handle);
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_pop();
#endif
}

#endif

