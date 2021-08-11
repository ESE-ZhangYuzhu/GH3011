/**
  ****************************************************************************************
  * @file    app_dual_tim.c
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
#include "app_dual_tim.h"
#include "string.h"
#include "app_pwr_mgmt.h"
#include "platform_sdk.h"

#ifdef HAL_DUAL_TIMER_MODULE_ENABLED

/*
 * DEFINES
 *****************************************************************************************
 */

/*
 * STRUCT DEFINE
 *****************************************************************************************
 */

/**@brief App dual tim state types. */
typedef enum
{
    APP_DUAL_TIM_INVALID = 0,
    APP_DUAL_TIM_ACTIVITY,
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    APP_DUAL_TIM_SLEEP,
#endif
} app_dual_tim_state_t;

struct dual_tim_env_t
{
    app_dual_tim_evt_handler_t  evt_handler;
    dual_timer_handle_t         handle;
    app_dual_tim_state_t        dual_tim_state;
};

/*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static bool dual_tim_prepare_for_sleep(void);
static void dual_tim_sleep_canceled(void);
static void dual_tim_wake_up_ind(void);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static const IRQn_Type   s_dual_tim_irq[APP_DUAL_TIM_ID_MAX] = { DUAL_TIMER_IRQn, DUAL_TIMER_IRQn };
static const uint32_t    s_dual_tim_instance[APP_DUAL_TIM_ID_MAX] = { DUAL_TIMER0_BASE, DUAL_TIMER1_BASE };

static bool s_sleep_cb_registered_flag = false;
static struct dual_tim_env_t s_dual_tim_env[APP_DUAL_TIM_ID_MAX];
static pwr_id_t s_dual_tim_pwr_id;

const static app_sleep_callbacks_t dual_tim_sleep_cb =
{
    .app_prepare_for_sleep = dual_tim_prepare_for_sleep,
    .app_sleep_canceled = dual_tim_sleep_canceled,
    .app_wake_up_ind = dual_tim_wake_up_ind,
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
static bool dual_tim_prepare_for_sleep(void)
{
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    for (uint8_t i = 0; i < APP_DUAL_TIM_ID_MAX; i++)
    {
        if (s_dual_tim_env[i].dual_tim_state == APP_DUAL_TIM_ACTIVITY)
        {
            s_dual_tim_env[i].dual_tim_state = APP_DUAL_TIM_SLEEP;
        }
    }
#endif

    return true;
}

static void dual_tim_sleep_canceled(void)
{
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    for (uint8_t i = 0; i < APP_DUAL_TIM_ID_MAX; i++)
    {
        if (s_dual_tim_env[i].dual_tim_state == APP_DUAL_TIM_SLEEP)
        {
            s_dual_tim_env[i].dual_tim_state = APP_DUAL_TIM_ACTIVITY;
        }
    }
#endif
}

SECTION_RAM_CODE static void dual_tim_wake_up_ind(void)
{
#ifndef APP_DRIVER_WAKEUP_CALL_FUN
    for (uint8_t i = 0; i < APP_DUAL_TIM_ID_MAX; i++)
    {
        if (s_dual_tim_env[i].dual_tim_state == APP_DUAL_TIM_ACTIVITY)
        {
            hal_nvic_clear_pending_irq(s_dual_tim_irq[i]);
            hal_nvic_enable_irq(s_dual_tim_irq[i]);
 
            hal_dual_timer_base_deinit(&s_dual_tim_env[i].handle);
            hal_dual_timer_base_init(&s_dual_tim_env[i].handle);
        }
    }
#endif
}

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
static void dual_tim_wake_up(app_dual_tim_id_t id)
{
    if (s_dual_tim_env[id].dual_tim_state == APP_DUAL_TIM_SLEEP)
    {
        hal_nvic_clear_pending_irq(s_dual_tim_irq[id]);
        hal_nvic_enable_irq(s_dual_tim_irq[id]);
    
        hal_dual_timer_base_deinit(&s_dual_tim_env[id].handle);
        hal_dual_timer_base_init(&s_dual_tim_env[id].handle);
        s_dual_tim_env[id].dual_tim_state = APP_DUAL_TIM_ACTIVITY;
    }

}
#endif

static void app_dual_tim_event_call(dual_timer_handle_t *p_dual_tim, app_dual_tim_evt_t evt_type)
{
    app_dual_tim_evt_t dual_tim_evt = APP_DUAL_TIM_EVT_ERROR;
    app_dual_tim_id_t id = APP_DUAL_TIM_ID_0;

    if(p_dual_tim->p_instance == DUAL_TIMER0)
    {
        id = APP_DUAL_TIM_ID_0;
    }
    else if(p_dual_tim->p_instance == DUAL_TIMER1)
    {
        id = APP_DUAL_TIM_ID_1;
    }

    if (evt_type == APP_DUAL_TIM_EVT_DONE)
    {
        dual_tim_evt = APP_DUAL_TIM_EVT_DONE;
    }

    if (s_dual_tim_env[id].dual_tim_state == APP_DUAL_TIM_INVALID)
    {
        dual_tim_evt = APP_DUAL_TIM_EVT_ERROR;
    }

    if (s_dual_tim_env[id].evt_handler != NULL)
    {
        s_dual_tim_env[id].evt_handler(&dual_tim_evt);
    }
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
uint16_t app_dual_tim_init(app_dual_tim_params_t *p_params, app_dual_tim_evt_handler_t evt_handler)
{
    uint8_t id = p_params->id;
    hal_status_t  hal_err_code;

    if (NULL == p_params)
    {
        return APP_DRV_ERR_POINTER_NULL;
    }

    if (id >= APP_DUAL_TIM_ID_MAX)
    {
        return APP_DRV_ERR_INVALID_ID;
    }

    if ( (s_dual_tim_env[0].dual_tim_state == APP_DUAL_TIM_INVALID) &&
         (s_dual_tim_env[1].dual_tim_state == APP_DUAL_TIM_INVALID))
    {
        hal_nvic_clear_pending_irq(s_dual_tim_irq[id]);
        hal_nvic_enable_irq(s_dual_tim_irq[id]);
    }

    s_dual_tim_env[id].evt_handler = evt_handler;
    
    memcpy(&s_dual_tim_env[id].handle.init, &p_params->init, sizeof(dual_timer_init_t));
    s_dual_tim_env[id].handle.p_instance = (dual_timer_regs_t *)s_dual_tim_instance[id];
    hal_err_code = hal_dual_timer_base_deinit(&s_dual_tim_env[id].handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    hal_err_code = hal_dual_timer_base_init(&s_dual_tim_env[id].handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    if(s_sleep_cb_registered_flag == false)// register sleep callback
    {
        s_sleep_cb_registered_flag = true;
        s_dual_tim_pwr_id = pwr_register_sleep_cb(&dual_tim_sleep_cb, APP_DRIVER_DUAL_TIM_WAPEUP_PRIORITY);

        if (s_dual_tim_pwr_id < 0)
        {
            return APP_DRV_ERR_INVALID_PARAM;
        }
    }

    s_dual_tim_env[id].dual_tim_state = APP_DUAL_TIM_ACTIVITY;

    return APP_DRV_SUCCESS;
}

uint16_t app_dual_tim_deinit(app_dual_tim_id_t id)
{
    hal_status_t  hal_err_code;

    if ((id >= APP_DUAL_TIM_ID_MAX) || (s_dual_tim_env[id].dual_tim_state == APP_DUAL_TIM_INVALID))
    {
        return APP_DRV_ERR_INVALID_ID;
    }

    if ( (s_dual_tim_env[0].dual_tim_state == APP_DUAL_TIM_INVALID) &&
         (s_dual_tim_env[1].dual_tim_state == APP_DUAL_TIM_INVALID))
    {
        hal_nvic_disable_irq(s_dual_tim_irq[id]);
    }

    s_dual_tim_env[id].dual_tim_state = APP_DUAL_TIM_INVALID;

    GLOBAL_EXCEPTION_DISABLE();
    if(s_dual_tim_env[APP_DUAL_TIM_ID_0].dual_tim_state == APP_DUAL_TIM_INVALID && 
        s_dual_tim_env[APP_DUAL_TIM_ID_1].dual_tim_state == APP_DUAL_TIM_INVALID)
    {
         pwr_unregister_sleep_cb(s_dual_tim_pwr_id);
         s_sleep_cb_registered_flag = false;
    }
    GLOBAL_EXCEPTION_ENABLE();

    hal_err_code = hal_dual_timer_base_deinit(&s_dual_tim_env[id].handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_dual_tim_start(app_dual_tim_id_t id)
{
    hal_status_t err_code;

    if (id >= APP_DUAL_TIM_ID_MAX ||
        s_dual_tim_env[id].dual_tim_state == APP_DUAL_TIM_INVALID)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    dual_tim_wake_up(id);
#endif

    err_code = hal_dual_timer_base_start_it(&s_dual_tim_env[id].handle);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_dual_tim_stop(app_dual_tim_id_t id)
{
    hal_status_t err_code;

    if (id >= APP_DUAL_TIM_ID_MAX ||
        s_dual_tim_env[id].dual_tim_state == APP_DUAL_TIM_INVALID)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    dual_tim_wake_up(id);
#endif

    err_code = hal_dual_timer_base_stop_it(&s_dual_tim_env[id].handle);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

dual_timer_handle_t *app_dual_tim_get_handle(app_dual_tim_id_t id)
{
    if (id >= APP_DUAL_TIM_ID_MAX ||
        s_dual_tim_env[id].dual_tim_state == APP_DUAL_TIM_INVALID)
    {
        return NULL;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    dual_tim_wake_up(id);
#endif

    return &s_dual_tim_env[id].handle;
}

void hal_dual_timer_period_elapsed_callback(dual_timer_handle_t *p_dual_timer)
{
    app_dual_tim_event_call(p_dual_timer, APP_DUAL_TIM_EVT_DONE);
}

SECTION_RAM_CODE void DUAL_TIMER_IRQHandler(void)
{
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_push();
#endif
    if (s_dual_tim_env[0].dual_tim_state != APP_DUAL_TIM_INVALID)
    {
        hal_dual_timer_irq_handler(&s_dual_tim_env[0].handle);
    }

    if (s_dual_tim_env[1].dual_tim_state != APP_DUAL_TIM_INVALID)
    {
        hal_dual_timer_irq_handler(&s_dual_tim_env[1].handle);
    }
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_pop();
#endif
}

#endif
