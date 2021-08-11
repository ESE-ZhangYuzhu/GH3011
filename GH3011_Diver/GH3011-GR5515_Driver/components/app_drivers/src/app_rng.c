/**
  ****************************************************************************************
  * @file    app_rng.c
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
#include "app_rng.h"
#include "app_pwr_mgmt.h"
#include "app_systick.h"
#include "string.h"
#include "platform_sdk.h"

#ifdef HAL_RNG_MODULE_ENABLED

/*
 * DEFINES
 *****************************************************************************************
 */

#ifdef ENV_RTOS_USE_MUTEX

#define APP_RNG_DRV_SYNC_MUTEX_LOCK     app_driver_mutex_pend(s_rng_env.mutex_sync, MUTEX_WAIT_FOREVER)
#define APP_RNG_DRV_SYNC_MUTEX_UNLOCK   app_driver_mutex_post(s_rng_env.mutex_sync)

#define APP_RNG_DRV_ASYNC_MUTEX_LOCK    app_driver_mutex_pend(s_rng_env.mutex_async, MUTEX_WAIT_FOREVER)
#define APP_RNG_DRV_ASYNC_MUTEX_UNLOCK  app_driver_mutex_post(s_rng_env.mutex_async)

#endif

/*
 * STRUCT DEFINE
 *****************************************************************************************
 */

/**@brief App rng state types. */
typedef enum
{
    APP_RNG_INVALID = 0,
    APP_RNG_ACTIVITY,
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    APP_RNG_SLEEP,
#endif
} app_rng_state_t;

struct rng_env_t
{
    app_rng_evt_handler_t   evt_handler;
    rng_handle_t            handle;
    app_rng_type_t          ues_type;
    app_rng_state_t         rng_state;
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
static bool rng_prepare_for_sleep(void);
static void rng_sleep_canceled(void);
static void rng_wake_up_ind(void);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
struct rng_env_t s_rng_env = {
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
static pwr_id_t s_rng_pwr_id;

const static app_sleep_callbacks_t rng_sleep_cb =
{
    .app_prepare_for_sleep = rng_prepare_for_sleep,
    .app_sleep_canceled = rng_sleep_canceled,
    .app_wake_up_ind = rng_wake_up_ind
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
static bool rng_prepare_for_sleep(void)
{
    hal_rng_state_t state;

    if (s_rng_env.rng_state == APP_RNG_ACTIVITY)
    {
        state = hal_rng_get_state(&s_rng_env.handle);
        if ((state != HAL_RNG_STATE_READY) && (state != HAL_RNG_STATE_RESET))
        {
            return false;
        }

        GLOBAL_EXCEPTION_DISABLE();
        hal_rng_suspend_reg(&s_rng_env.handle);
        GLOBAL_EXCEPTION_ENABLE();
        #ifdef APP_DRIVER_WAKEUP_CALL_FUN
        s_rng_env.rng_state = APP_RNG_SLEEP;
        #endif
    }

    return true;
}

static void rng_sleep_canceled(void)
{
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    if (s_rng_env.rng_state == APP_RNG_SLEEP)
    {
        s_rng_env.rng_state = APP_RNG_ACTIVITY;
    }
#endif
}

SECTION_RAM_CODE static void rng_wake_up_ind(void)
{
#ifndef APP_DRIVER_WAKEUP_CALL_FUN
    if (s_rng_env.rng_state == APP_RNG_ACTIVITY)
    {
        GLOBAL_EXCEPTION_DISABLE();
        hal_rng_resume_reg(&s_rng_env.handle);
        GLOBAL_EXCEPTION_ENABLE();
        if (s_rng_env.ues_type == APP_RNG_TYPE_INTERRUPT)
        {
            hal_nvic_clear_pending_irq(RNG_IRQn);
            hal_nvic_enable_irq(RNG_IRQn);
        }
    }
#endif
}

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
static void rng_wake_up(void)
{
    if (s_rng_env.rng_state == APP_RNG_SLEEP)
    {
        GLOBAL_EXCEPTION_DISABLE();
        hal_rng_resume_reg(&s_rng_env.handle);
        GLOBAL_EXCEPTION_ENABLE();
        if (s_rng_env.ues_type == APP_RNG_TYPE_INTERRUPT)
        {
            hal_nvic_clear_pending_irq(RNG_IRQn);
            hal_nvic_enable_irq(RNG_IRQn);
        }
        s_rng_env.rng_state = APP_RNG_ACTIVITY;
    }
}
#endif

static void app_rng_event_call(rng_handle_t *p_rng, app_rng_evt_type_t evt_type, uint32_t random32bit)
{
    app_rng_evt_t rng_evt = {APP_RNG_EVT_ERROR, 0x0};

    if (p_rng->p_instance == RNG)
    {
        rng_evt.type = evt_type;
        rng_evt.random_data = random32bit;
    }

#ifdef  ENV_RTOS_USE_SEMP
    app_driver_sem_post_from_isr(s_rng_env.sem_rx);
#endif

    if (s_rng_env.evt_handler != NULL)
    {
        s_rng_env.evt_handler(&rng_evt);
    }
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
uint16_t app_rng_init(app_rng_params_t *p_params, app_rng_evt_handler_t evt_handler)
{
    hal_status_t  hal_err_code = HAL_OK;
    app_drv_err_t app_err_code = APP_DRV_SUCCESS;

    if (p_params == NULL)
    {
        return APP_DRV_ERR_POINTER_NULL;
    }

#ifdef  ENV_RTOS_USE_SEMP
    if(s_rng_env.sem_rx == NULL)
    {
        app_err_code = app_driver_sem_init(&s_rng_env.sem_rx);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
#endif

#ifdef ENV_RTOS_USE_MUTEX
    if(s_rng_env.mutex_async == NULL)
    {
        app_err_code = app_driver_mutex_init(&s_rng_env.mutex_async);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
     if(s_rng_env.mutex_sync == NULL)
     {
         app_err_code = app_driver_mutex_init(&s_rng_env.mutex_sync);
         APP_DRV_ERR_CODE_CHECK(app_err_code);
     }
#endif

    app_systick_init();

    if (p_params->use_type == APP_RNG_TYPE_INTERRUPT)
    {
        hal_nvic_clear_pending_irq(RNG_IRQn);
        hal_nvic_enable_irq(RNG_IRQn);
    }

    s_rng_env.ues_type = p_params->use_type;
    s_rng_env.evt_handler = evt_handler;

    memcpy(&s_rng_env.handle.init, &p_params->init, sizeof(rng_init_t));
    s_rng_env.handle.p_instance = RNG;
    hal_err_code = hal_rng_deinit(&s_rng_env.handle);
    APP_DRV_ERR_CODE_CHECK(hal_err_code);

    hal_err_code = hal_rng_init(&s_rng_env.handle);
    APP_DRV_ERR_CODE_CHECK(hal_err_code);

    if(s_sleep_cb_registered_flag == false)    // register sleep callback
    {
        s_sleep_cb_registered_flag = true;
        s_rng_pwr_id = pwr_register_sleep_cb(&rng_sleep_cb, APP_DRIVER_RNG_WAPEUP_PRIORITY);
        if (s_rng_pwr_id < 0)
        {
            return APP_DRV_ERR_INVALID_PARAM;
        }
    }

    s_rng_env.rng_state = APP_RNG_ACTIVITY;

    return app_err_code;
}

uint16_t app_rng_deinit(void)
{
    hal_status_t  hal_err_code;

    if (s_rng_env.rng_state == APP_RNG_INVALID)
    {
        return APP_DRV_ERR_INVALID_ID;
    }

#ifdef  ENV_RTOS_USE_SEMP
    if(s_rng_env.sem_rx != NULL)
    {
        app_driver_sem_deinit(s_rng_env.sem_rx);
        s_rng_env.sem_rx = NULL;
    }
#endif

#ifdef ENV_RTOS_USE_MUTEX
    if(s_rng_env.mutex_sync != NULL)
    {
        app_driver_mutex_deinit(s_rng_env.mutex_sync);
        s_rng_env.mutex_sync = NULL;
    }
    if(s_rng_env.mutex_async != NULL){
        app_driver_mutex_deinit(s_rng_env.mutex_async);
        s_rng_env.mutex_async = NULL;
    }
#endif

    hal_nvic_disable_irq(RNG_IRQn);
    s_rng_env.rng_state = APP_RNG_INVALID;

    GLOBAL_EXCEPTION_DISABLE();
    pwr_unregister_sleep_cb(s_rng_pwr_id);
    s_sleep_cb_registered_flag = false;
    GLOBAL_EXCEPTION_ENABLE();

    app_systick_deinit();

    hal_err_code = hal_rng_deinit(&s_rng_env.handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_rng_gen_sync(uint16_t *p_seed, uint32_t *p_random32bit)
{
    hal_status_t err_code;

    if (s_rng_env.rng_state == APP_RNG_INVALID ||
        p_seed == NULL ||
        p_random32bit == NULL)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    rng_wake_up();
#endif

    err_code = hal_rng_generate_random_number(&s_rng_env.handle, p_seed, p_random32bit);
    if (err_code != HAL_OK)
    {
        return (uint16_t)err_code;
    }

    return APP_DRV_SUCCESS;
}

#ifdef  ENV_RTOS_USE_SEMP
uint16_t app_rng_gen_sem_sync(uint16_t *p_seed)
{
    hal_status_t err_code;

#ifdef ENV_RTOS_USE_MUTEX
    APP_RNG_DRV_ASYNC_MUTEX_LOCK;
#endif

    if (s_rng_env.rng_state == APP_RNG_INVALID ||
        s_rng_env.ues_type == APP_RNG_TYPE_POLLING ||
        p_seed == NULL)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_RNG_DRV_ASYNC_MUTEX_UNLOCK;
#endif
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    rng_wake_up();
#endif

    err_code = hal_rng_generate_random_number_it(&s_rng_env.handle, p_seed);
    if (err_code != HAL_OK)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_RNG_DRV_ASYNC_MUTEX_UNLOCK;
#endif
        return (uint16_t)err_code;
    }

    app_driver_sem_pend(s_rng_env.sem_rx, OS_WAIT_FOREVER);

#ifdef ENV_RTOS_USE_MUTEX
    APP_RNG_DRV_ASYNC_MUTEX_UNLOCK;
#endif

    return APP_DRV_SUCCESS;
}
#endif

uint16_t app_rng_gen_async(uint16_t *p_seed)
{
    hal_status_t err_code;

    if (s_rng_env.rng_state == APP_RNG_INVALID ||
        s_rng_env.ues_type == APP_RNG_TYPE_POLLING ||
        p_seed == NULL)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    rng_wake_up();
#endif

    err_code = hal_rng_generate_random_number_it(&s_rng_env.handle, p_seed);
    if (err_code != HAL_OK)
    {
        return (uint16_t)err_code;
    }

    return APP_DRV_SUCCESS;
}

rng_handle_t *app_rng_get_handle(void)
{
    if (s_rng_env.rng_state == APP_RNG_INVALID)
    {
        return NULL;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    rng_wake_up();
#endif

    return &s_rng_env.handle;
}

void hal_rng_ready_data_callback(rng_handle_t *p_rng, uint32_t random32bit)
{
    app_rng_event_call(p_rng, APP_RNG_EVT_DONE, random32bit);
}

SECTION_RAM_CODE void RNG_IRQHandler(void)
{
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_push();
#endif
    hal_rng_irq_handler(&s_rng_env.handle);
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_pop();
#endif
}

#endif
