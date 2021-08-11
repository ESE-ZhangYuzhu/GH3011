/**
  ****************************************************************************************
  * @file    app_gpiote.c
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
#include "app_gpiote.h"
#include "app_pwr_mgmt.h"
#include <string.h>
#include "platform_sdk.h"

/*
 * DEFINES
 *****************************************************************************************
 */
#define GPIOTE_USE_PATTERN     0x47
#define GPIOTE_USE_MAX         32

/*
 * STRUCT DEFINE
 *****************************************************************************************
 */
struct gpiote_env_t
{
    uint8_t            total_used;
    app_gpiote_param_t params[GPIOTE_USE_MAX];
};

/*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static bool gpiote_prepare_for_sleep(void);
static void gpiote_sleep_canceled(void);
static void gpiote_wake_up_ind(void);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static   struct gpiote_env_t s_gpiote_env;

static bool     s_sleep_cb_registered_flag = false;
static pwr_id_t s_gpiote_pwr_id;

static const app_sleep_callbacks_t gpiote_sleep_cb =
{
    .app_prepare_for_sleep = gpiote_prepare_for_sleep,
    .app_sleep_canceled    = gpiote_sleep_canceled,
    .app_wake_up_ind       = gpiote_wake_up_ind
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
static bool gpiote_prepare_for_sleep(void)
{
    return true;
}

static void gpiote_sleep_canceled(void)
{
}

SECTION_RAM_CODE static void gpiote_wake_up_ind(void)
{
    bool is_ext0_need_enable = false;
    bool is_ext1_need_enable = false;

    for (int idx = 0; idx < s_gpiote_env.total_used; idx++ )
    {
        if (s_gpiote_env.params[idx].type == APP_IO_TYPE_NORMAL)
        {
            if(APP_IO_PINS_0_15 & s_gpiote_env.params[idx].pin)
            {
                is_ext0_need_enable = true;
                continue;
            }
            if(APP_IO_PINS_16_31 & s_gpiote_env.params[idx].pin)
            {
                is_ext1_need_enable = true;
                continue;
            }
        }
    }

    if (is_ext0_need_enable)
    {
        hal_nvic_enable_irq(EXT0_IRQn);
    }

    if (is_ext1_need_enable)
    {
        hal_nvic_enable_irq(EXT1_IRQn);
    }

    return;
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
uint16_t app_gpiote_init(const app_gpiote_param_t *p_params, uint8_t table_cnt)
{
    static uint8_t exit_flag = 0x0;
    app_io_init_t io_init;
    app_drv_err_t err_code;

    if (NULL == p_params)
    {
        return APP_DRV_ERR_POINTER_NULL;
    }
    
    if ( ((s_gpiote_env.total_used + table_cnt) > GPIOTE_USE_MAX) && table_cnt)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }
    
    for (uint8_t idx = 0; idx < table_cnt; idx++)
    { 
        exit_flag = 0x0;
        
        for (uint8_t index = 0; index < s_gpiote_env.total_used; index ++)
        {
            if (s_gpiote_env.params[index].pin == p_params[idx].pin && 
                s_gpiote_env.params[index].type == p_params[idx].type )
            {
                exit_flag = 0x1;
                break;
            }
        }
        
        if (exit_flag)
        {
            continue;
        }
         
        memcpy(&s_gpiote_env.params[s_gpiote_env.total_used], &p_params[idx], sizeof(app_gpiote_param_t));
    
        io_init.pin  = p_params[idx].pin;
        io_init.mode = p_params[idx].mode;
        io_init.pull = p_params[idx].pull;
        io_init.mux  = APP_IO_MUX_7;

        err_code = app_io_init(p_params[idx].type, &io_init);
        APP_DRV_ERR_CODE_CHECK(err_code);

        if ((p_params[idx].handle_mode == APP_IO_ENABLE_WAKEUP) && (p_params[idx].type == APP_IO_TYPE_AON))
        {
           switch (p_params[idx].mode)
           {
             case APP_IO_MODE_IT_RISING:
                  hal_pwr_config_ext_wakeup(p_params[idx].pin, PWR_EXTWKUP_TYPE_RISING);
                  break;

             case APP_IO_MODE_IT_FALLING:
                  hal_pwr_config_ext_wakeup(p_params[idx].pin, PWR_EXTWKUP_TYPE_FALLING);
                  break;

             case APP_IO_MODE_IT_HIGH:
                  hal_pwr_config_ext_wakeup(p_params[idx].pin, PWR_EXTWKUP_TYPE_HIGH);
                  break;

             case APP_IO_MODE_IT_LOW:
                  hal_pwr_config_ext_wakeup(p_params[idx].pin, PWR_EXTWKUP_TYPE_LOW);
                  break;

             default:
                 break;
           }
           pwr_mgmt_wakeup_source_setup(PWR_WKUP_COND_EXT);
        }
        
        if (p_params[idx].type == APP_IO_TYPE_NORMAL)
        {
            hal_nvic_clear_pending_irq(EXT0_IRQn);
            hal_nvic_enable_irq(EXT0_IRQn);
            hal_nvic_clear_pending_irq(EXT1_IRQn);
            hal_nvic_enable_irq(EXT1_IRQn);
        }
        else if (p_params[idx].type == APP_IO_TYPE_AON)
        {
            hal_nvic_clear_pending_irq(EXT2_IRQn);
            hal_nvic_enable_irq(EXT2_IRQn);
        }
        
        s_gpiote_env.total_used += 1;
    }
    if (!s_sleep_cb_registered_flag ) // register sleep callback
    {
        s_gpiote_pwr_id = pwr_register_sleep_cb(&gpiote_sleep_cb, APP_DRIVER_GPIOTE_WAPEUP_PRIORITY);
        if (s_gpiote_pwr_id < 0)
        {
            return APP_DRV_ERR_INVALID_PARAM;
        }
        s_sleep_cb_registered_flag = true;
    }
 
    return APP_DRV_SUCCESS;
}

uint16_t app_gpiote_config(const app_gpiote_param_t *p_config)
{
    uint8_t exit_flag = 0x0;
    uint8_t index;
    app_io_init_t io_init;
    app_drv_err_t err_code;

    if (NULL == p_config)
    {
        return APP_DRV_ERR_POINTER_NULL;
    }

    for (index = 0; index < s_gpiote_env.total_used; index ++)
    {
        if (s_gpiote_env.params[index].pin == p_config->pin && 
            s_gpiote_env.params[index].type == p_config->type )
        {
            exit_flag = 0x1;
            break;
        }
    }

    if (!exit_flag || index >= GPIOTE_USE_MAX)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

    memcpy(&s_gpiote_env.params[index], p_config, sizeof(app_gpiote_param_t));

    io_init.pin  = p_config->pin;
    io_init.mode = p_config->mode;
    io_init.pull = p_config->pull;
    io_init.mux  = APP_IO_MUX_7;

    app_io_deinit(p_config->type, p_config->pin);
    err_code = app_io_init(p_config->type, &io_init);
    APP_DRV_ERR_CODE_CHECK(err_code);

    if ((p_config->handle_mode == APP_IO_ENABLE_WAKEUP) && (p_config->type == APP_IO_TYPE_AON))
    {
        switch (p_config->mode)
        {
            case APP_IO_MODE_IT_RISING:
                hal_pwr_config_ext_wakeup(p_config->pin, PWR_EXTWKUP_TYPE_RISING);
                break;

            case APP_IO_MODE_IT_FALLING:
                hal_pwr_config_ext_wakeup(p_config->pin, PWR_EXTWKUP_TYPE_FALLING);
                break;

            case APP_IO_MODE_IT_HIGH:
                hal_pwr_config_ext_wakeup(p_config->pin, PWR_EXTWKUP_TYPE_HIGH);
                break;

            case APP_IO_MODE_IT_LOW:
                hal_pwr_config_ext_wakeup(p_config->pin, PWR_EXTWKUP_TYPE_LOW);
                break;

            default:
            break;
        }
        pwr_mgmt_wakeup_source_setup(PWR_WKUP_COND_EXT);
    }

    if ((p_config->handle_mode == APP_IO_DISABLE_WAKEUP) && (p_config->type == APP_IO_TYPE_AON))
    {
        hal_pwr_disable_ext_wakeup(p_config->pin);
    }

    if (p_config->type == APP_IO_TYPE_NORMAL)
    {
        hal_nvic_clear_pending_irq(EXT0_IRQn);
        hal_nvic_enable_irq(EXT0_IRQn);
        hal_nvic_clear_pending_irq(EXT1_IRQn);
        hal_nvic_enable_irq(EXT1_IRQn);
    }
    else if (p_config->type == APP_IO_TYPE_AON)
    {
        hal_nvic_clear_pending_irq(EXT2_IRQn);
        hal_nvic_enable_irq(EXT2_IRQn);
    }

    return APP_DRV_SUCCESS;
}


void app_gpiote_deinit(void)
{
    for (int idx=0; idx<s_gpiote_env.total_used; idx ++ )
    { 
        app_io_deinit(s_gpiote_env.params[idx].type, s_gpiote_env.params[idx].pin);
    }
    hal_nvic_disable_irq(EXT0_IRQn);
    hal_nvic_disable_irq(EXT1_IRQn);
    hal_nvic_disable_irq(EXT2_IRQn);
    pwr_unregister_sleep_cb(s_gpiote_pwr_id);
    s_gpiote_env.total_used = 0;
}

void hal_gpio_exti_callback(gpio_regs_t *GPIOx, uint16_t gpio_pin)
{
    uint32_t io_pin = gpio_pin;
    app_gpiote_evt_t gpiote_evt;

    if (GPIO1 == GPIOx)
    {
        io_pin = (uint32_t)(gpio_pin << 16);
    }

    gpiote_evt.type = APP_IO_TYPE_NORMAL;
    gpiote_evt.pin = io_pin;
    gpiote_evt.ctx_type = APP_IO_CTX_INT;

    for (uint8_t idx=0; idx<s_gpiote_env.total_used; idx ++)
    {
        if ((s_gpiote_env.params[idx].type == APP_IO_TYPE_NORMAL) && (io_pin == s_gpiote_env.params[idx].pin))
        {
            if (s_gpiote_env.params[idx].io_evt_cb)
                s_gpiote_env.params[idx].io_evt_cb(&gpiote_evt);
        }
    }
}

void hal_aon_gpio_callback(uint16_t aon_gpio_pin)
{
    app_gpiote_evt_t gpiote_evt;

    gpiote_evt.type = APP_IO_TYPE_AON;
    gpiote_evt.pin = aon_gpio_pin;
    
    if (pwr_mgmt_get_wakeup_flag() == WARM_BOOT)
    {
        gpiote_evt.ctx_type = APP_IO_CTX_WAKEUP;
    }
    else
    {
        gpiote_evt.ctx_type = APP_IO_CTX_INT;
    }
   
    for (uint8_t idx=0; idx<s_gpiote_env.total_used; idx ++)
    {
        if ((s_gpiote_env.params[idx].type == APP_IO_TYPE_AON) && \
            (aon_gpio_pin & s_gpiote_env.params[idx].pin) && \
            (s_gpiote_env.params[idx].io_evt_cb))
        {
            if (gpiote_evt.ctx_type == APP_IO_CTX_WAKEUP)
            {
                if (s_gpiote_env.params[idx].handle_mode == APP_IO_ENABLE_WAKEUP)  
                {
                    s_gpiote_env.params[idx].io_evt_cb(&gpiote_evt);
                }    
            }
            else
            {
                s_gpiote_env.params[idx].io_evt_cb(&gpiote_evt);
            }
        }
    }
}


SECTION_RAM_CODE void EXT0_IRQHandler(void)
{
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_push();
#endif
    hal_gpio_exti_irq_handler(GPIO0);
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_pop();
#endif
}

SECTION_RAM_CODE void EXT1_IRQHandler(void)
{
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_push();
#endif
    hal_gpio_exti_irq_handler(GPIO1);
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_pop();
#endif
}

SECTION_RAM_CODE void EXT2_IRQHandler(void)
{
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_push();
#endif
    hal_aon_gpio_irq_handler();
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_pop();
#endif
}

