/**
  ****************************************************************************************
  * @file    app_pwm.c
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
#include "app_pwm.h"
#include "app_io.h"
#include "app_systick.h"
#include "app_pwr_mgmt.h"
#include <string.h>

#ifdef HAL_CALENDAR_MODULE_ENABLED

/*
 * STRUCT DEFINE
 *****************************************************************************************
 */

/**@brief App pwm state types. */
typedef enum
{
    APP_PWM_INVALID = 0,
    APP_PWM_ACTIVITY,
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    APP_PWM_SLEEP,
#endif

} app_pwm_state_t;

/**@brief App pwm module state types. */
typedef enum
{
    APP_PWM_STOP = 0,
    APP_PWM_START,
} app_pwm_module_state_t;

struct pwm_env_t
{
    app_pwm_pin_cfg_t      pin_cfg;
    app_pwm_state_t        pwm_state;
    app_pwm_module_state_t pwm_module_state;
    pwm_handle_t           handle;
};


/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
static bool pwm_prepare_for_sleep(void);
static void pwm_sleep_canceled(void);
static void pwm_wake_up_ind(void);
static uint16_t pwm_gpio_config(app_pwm_pin_cfg_t pin_cfg);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
struct pwm_env_t s_pwm_env[APP_PWM_ID_MAX];
static const uint32_t s_pwm_instance[APP_PWM_ID_MAX] = {PWM0_BASE, PWM1_BASE};
static bool  s_sleep_cb_registered_flag = false;
static pwr_id_t   s_pwm_pwr_id;

static const app_sleep_callbacks_t pwm_sleep_cb =
{
    .app_prepare_for_sleep = pwm_prepare_for_sleep,
    .app_sleep_canceled    = pwm_sleep_canceled,
    .app_wake_up_ind       = pwm_wake_up_ind
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
static bool pwm_prepare_for_sleep(void)
{
    hal_pwm_state_t state;
    uint8_t i;

    for (i = 0; i < APP_PWM_ID_MAX; i++)
    {
        if (s_pwm_env[i].pwm_state == APP_PWM_ACTIVITY)
        {
            state = hal_pwm_get_state(&s_pwm_env[i].handle);
            if ((state != HAL_PWM_STATE_RESET) && (state != HAL_PWM_STATE_READY))
            {
                return false;
            }

            GLOBAL_EXCEPTION_DISABLE();
            hal_pwm_suspend_reg(&s_pwm_env[i].handle);
            GLOBAL_EXCEPTION_ENABLE();
            #ifdef APP_DRIVER_WAKEUP_CALL_FUN
            s_pwm_env[i].pwm_state = APP_PWM_SLEEP;
            #endif
        }
    }
    return true;
}

static void pwm_sleep_canceled(void)
{
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    for (uint8_t i = 0; i < APP_PWM_ID_MAX; i++)
    {
        if (s_pwm_env[i].pwm_state == APP_PWM_SLEEP)
        {
            s_pwm_env[i].pwm_state = APP_PWM_ACTIVITY;
        }
    }
#endif

}

SECTION_RAM_CODE static void pwm_wake_up_ind(void)
{
#ifndef APP_DRIVER_WAKEUP_CALL_FUN
    uint8_t i;

    for (i = 0; i < APP_PWM_ID_MAX; i++)
    {
        if (s_pwm_env[i].pwm_state == APP_PWM_ACTIVITY)
        {
            GLOBAL_EXCEPTION_DISABLE();
            hal_pwm_resume_reg(&s_pwm_env[i].handle);
            GLOBAL_EXCEPTION_ENABLE();

            if (s_pwm_env[i].pwm_module_state == APP_PWM_START)
            {
                hal_pwm_start(&s_pwm_env[i].handle);
            }
        }
    }
#endif
}


#ifdef APP_DRIVER_WAKEUP_CALL_FUN
static void pwm_wake_up(app_pwm_id_t id)
{
    if (s_pwm_env[id].pwm_state == APP_PWM_SLEEP)
    {
        GLOBAL_EXCEPTION_DISABLE();
        hal_pwm_resume_reg(&s_pwm_env[id].handle);
        GLOBAL_EXCEPTION_ENABLE();

        if (s_pwm_env[id].pwm_module_state == APP_PWM_START)
        {
            hal_pwm_start(&s_pwm_env[id].handle);
        }
    }
}
#endif


static uint16_t pwm_gpio_config(app_pwm_pin_cfg_t pin_cfg)
{
    app_io_init_t io_init = APP_IO_DEFAULT_CONFIG;
    app_drv_err_t err_code = APP_DRV_SUCCESS;

    io_init.pull = APP_IO_PULLUP;
    io_init.mode = APP_IO_MODE_MUX;

    if (pin_cfg.channel_a.enable == APP_PWM_PIN_ENABLE)
    {
        io_init.pin  = pin_cfg.channel_a.pin;
        io_init.mux  = pin_cfg.channel_a.mux;
        err_code = app_io_init(pin_cfg.channel_a.type, &io_init);
        APP_DRV_ERR_CODE_CHECK(err_code);
    }

    if (pin_cfg.channel_b.enable == APP_PWM_PIN_ENABLE)
    {
        io_init.pin  = pin_cfg.channel_b.pin;
        io_init.mux  = pin_cfg.channel_b.mux;
        err_code = app_io_init(pin_cfg.channel_b.type, &io_init);
        APP_DRV_ERR_CODE_CHECK(err_code);
    }

    if (pin_cfg.channel_c.enable == APP_PWM_PIN_ENABLE)
    {
        io_init.pin  = pin_cfg.channel_c.pin;
        io_init.mux  = pin_cfg.channel_c.mux;
        err_code = app_io_init(pin_cfg.channel_c.type, &io_init);
        APP_DRV_ERR_CODE_CHECK(err_code);
    }
    return err_code;
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

uint16_t app_pwm_init(app_pwm_params_t *p_params)
{
    uint8_t id = p_params->id;
    app_drv_err_t app_err_code;
    hal_status_t  hal_err_code;

    if (NULL == p_params)
    {
        return APP_DRV_ERR_POINTER_NULL;
    }

    if (id >= APP_PWM_ID_MAX)
    {
        return APP_DRV_ERR_INVALID_ID;
    }

    app_systick_init();

    app_err_code = pwm_gpio_config(p_params->pin_cfg);
    APP_DRV_ERR_CODE_CHECK(app_err_code);

    s_pwm_env[id].handle.active_channel = (hal_pwm_active_channel_t)p_params->active_channel;

    memcpy(&s_pwm_env[id].pin_cfg, &p_params->pin_cfg, sizeof(app_pwm_pin_cfg_t));
    memcpy(&s_pwm_env[id].handle.init, &p_params->init, sizeof(pwm_init_t));

    pwm_gpio_config(s_pwm_env[id].pin_cfg);
    
    s_pwm_env[id].handle.p_instance = (pwm_regs_t *)s_pwm_instance[id];

    hal_err_code = hal_pwm_deinit(&s_pwm_env[id].handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    hal_err_code = hal_pwm_init(&s_pwm_env[id].handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    if (!s_sleep_cb_registered_flag)
    {
        s_sleep_cb_registered_flag = true;

        s_pwm_pwr_id = pwr_register_sleep_cb(&pwm_sleep_cb, APP_DRIVER_PWM_WAPEUP_PRIORITY);
        if (s_pwm_pwr_id < 0)
        {
            return APP_DRV_ERR_INVALID_PARAM;
        }
    }

    s_pwm_env[id].pwm_state = APP_PWM_ACTIVITY;
    s_pwm_env[id].pwm_module_state = APP_PWM_STOP;

    return APP_DRV_SUCCESS;
}


uint16_t app_pwm_deinit(app_pwm_id_t id)
{
    hal_status_t err_code;

    if (s_pwm_env[id].pwm_state == APP_PWM_INVALID)
    {
        return APP_DRV_ERR_INVALID_ID;
    }
    
    GLOBAL_EXCEPTION_DISABLE();
    if(s_pwm_env[APP_PWM_ID_0].pwm_state == APP_PWM_INVALID && 
       s_pwm_env[APP_PWM_ID_1].pwm_state == APP_PWM_INVALID)
    {
         pwr_unregister_sleep_cb(s_pwm_pwr_id);
         s_sleep_cb_registered_flag = false;
    }
    GLOBAL_EXCEPTION_ENABLE();

    if (s_pwm_env[id].pin_cfg.channel_a.enable == APP_PWM_PIN_ENABLE)
    {
        app_io_deinit(s_pwm_env[id].pin_cfg.channel_a.type, s_pwm_env[id].pin_cfg.channel_a.pin);
    }
    if (s_pwm_env[id].pin_cfg.channel_b.enable == APP_PWM_PIN_ENABLE)
    {
        app_io_deinit(s_pwm_env[id].pin_cfg.channel_b.type, s_pwm_env[id].pin_cfg.channel_b.pin);
    }
    if (s_pwm_env[id].pin_cfg.channel_c.enable == APP_PWM_PIN_ENABLE)
    {
        app_io_deinit(s_pwm_env[id].pin_cfg.channel_c.type, s_pwm_env[id].pin_cfg.channel_c.pin);
    }
    
    err_code = hal_pwm_deinit(&s_pwm_env[id].handle);
    HAL_ERR_CODE_CHECK(err_code);
    
    s_pwm_env[id].pwm_state = APP_PWM_INVALID;
    s_pwm_env[id].pwm_module_state = APP_PWM_STOP;

    return APP_DRV_SUCCESS;
}


uint16_t app_pwm_start(app_pwm_id_t id)
{
    hal_status_t err_code;

    if (id >= APP_PWM_ID_MAX)
    {
        return APP_DRV_ERR_INVALID_ID;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pwm_wake_up(id);
#endif
    err_code = hal_pwm_start(&s_pwm_env[id].handle);
    HAL_ERR_CODE_CHECK(err_code);

    s_pwm_env[id].pwm_module_state = APP_PWM_START;

    return APP_DRV_SUCCESS;
}


uint16_t app_pwm_stop(app_pwm_id_t id)
{
    hal_status_t err_code;

    if (id >= APP_PWM_ID_MAX)
    {
        return APP_DRV_ERR_INVALID_ID;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pwm_wake_up(id);
#endif

    err_code = hal_pwm_stop(&s_pwm_env[id].handle);
    HAL_ERR_CODE_CHECK(err_code);

    s_pwm_env[id].pwm_module_state = APP_PWM_STOP;

    return APP_DRV_SUCCESS;
}


uint16_t app_pwm_update_freq(app_pwm_id_t id, uint32_t freq)
{
    hal_status_t err_code;

    if (id >= APP_PWM_ID_MAX)
    {
        return APP_DRV_ERR_INVALID_ID;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pwm_wake_up(id);
#endif

    err_code = hal_pwm_update_freq(&s_pwm_env[id].handle, freq);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}


uint16_t app_pwm_config_channel(app_pwm_id_t id, app_pwm_active_channel_t channel, app_pwm_channel_init_t *p_config)
{
    hal_status_t err_code;

    hal_pwm_active_channel_t active_channel;
    pwm_channel_init_t channel_cfg;

    if (id >= APP_PWM_ID_MAX)
    {
        return APP_DRV_ERR_INVALID_ID;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    pwm_wake_up(id);
#endif

    active_channel   = (hal_pwm_active_channel_t)channel;
    channel_cfg.duty = p_config->duty;
    channel_cfg.drive_polarity = p_config->drive_polarity;

    err_code = hal_pwm_config_channel(&s_pwm_env[id].handle, &channel_cfg, active_channel);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}
#endif
