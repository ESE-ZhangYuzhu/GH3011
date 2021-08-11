/**
  ****************************************************************************************
  * @file    app_io.c
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
#include "app_io.h"
#include "gr55xx_hal_gpio.h"
#include "gr55xx_hal_aon_gpio.h"
#include "gr55xx_hal_msio.h"

/*
 * DEFINES
 *****************************************************************************************
 */
#define IO_MODE_NONE   0x00

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static const uint32_t s_io_pull[APP_IO_TYPE_MAX][APP_IO_PULL_MAX] = 
{
    { GPIO_NOPULL,     GPIO_PULLUP,     GPIO_PULLDOWN },
    { AON_GPIO_NOPULL, AON_GPIO_PULLUP, AON_GPIO_PULLDOWN },
    { MSIO_NOPULL,     MSIO_PULLUP,     MSIO_PULLDOWN },
};

static const uint32_t s_io_mode[APP_IO_TYPE_MAX][APP_IO_MODE_MAX] = 
{
    {
        GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, GPIO_MODE_MUX, GPIO_MODE_IT_RISING,
        GPIO_MODE_IT_FALLING, GPIO_MODE_IT_HIGH, GPIO_MODE_IT_LOW, IO_MODE_NONE
    },
    {
        AON_GPIO_MODE_INPUT, AON_GPIO_MODE_OUTPUT, AON_GPIO_MODE_MUX, AON_GPIO_MODE_IT_RISING,
        AON_GPIO_MODE_IT_FALLING, AON_GPIO_MODE_IT_HIGH, AON_GPIO_MODE_IT_LOW, IO_MODE_NONE
    },
    { 
        MSIO_DIRECTION_INPUT, MSIO_DIRECTION_OUTPUT, MSIO_DIRECTION_INPUT, IO_MODE_NONE,
        IO_MODE_NONE, IO_MODE_NONE, IO_MODE_NONE, MSIO_DIRECTION_NONE
    },
};

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
uint16_t app_io_init(app_io_type_t type, app_io_init_t *p_init)
{
    gpio_init_t     io_config;
    aon_gpio_init_t aon_io_config;
    msio_init_t     msio_config;

    if(NULL == p_init)
    {
        return APP_DRV_ERR_POINTER_NULL;
    }
    else
    {
        switch(type)
        {
            case APP_IO_TYPE_NORMAL:
                if(APP_IO_MODE_ANALOG == p_init->mode)
                {
                    return APP_DRV_ERR_INVALID_MODE;
                }

                io_config.mode = s_io_mode[type][p_init->mode];
                io_config.pull = s_io_pull[type][p_init->pull];
                io_config.mux = p_init->mux;

                if(APP_IO_PINS_0_15 & p_init->pin)
                {
                    io_config.pin = (APP_IO_PINS_0_15 & p_init->pin);
                    hal_gpio_init(GPIO0, &io_config);
                }
                if (APP_IO_PINS_16_31 & p_init->pin)
                {
                    io_config.pin = (APP_IO_PINS_16_31 & p_init->pin) >> 16;
                    hal_gpio_init(GPIO1, &io_config);
                }
                break;

            case APP_IO_TYPE_AON:
                if(APP_IO_MODE_ANALOG == p_init->mode)
                {
                    return APP_DRV_ERR_INVALID_MODE;
                }
                aon_io_config.mode = s_io_mode[type][p_init->mode];
                aon_io_config.pull = s_io_pull[type][p_init->pull];
                aon_io_config.mux = p_init->mux;
                aon_io_config.pin = (APP_AON_IO_PIN_ALL & p_init->pin);
                hal_aon_gpio_init(&aon_io_config);
                break;
            
            case APP_IO_TYPE_MSIO:
                if(p_init->mode >= APP_IO_MODE_IT_RISING && p_init->mode <= APP_IO_MODE_IT_LOW)
                {
                    return APP_DRV_ERR_INVALID_MODE;
                }   
                msio_config.direction = (APP_IO_MODE_ANALOG == p_init->mode) ? MSIO_DIRECTION_INPUT : s_io_mode[type][p_init->mode];
                msio_config.mode = (APP_IO_MODE_ANALOG == p_init->mode)? MSIO_MODE_ANALOG : MSIO_MODE_DIGITAL;
                msio_config.pull = s_io_pull[type][p_init->pull];
                msio_config.mux = p_init->mux;
                msio_config.pin = (APP_MSIO_PIN_ALL & p_init->pin);
                hal_msio_init(&msio_config);
                break;

            default:
                return APP_DRV_ERR_INVALID_TYPE;
        }
    }

    return APP_DRV_SUCCESS;
}

uint16_t app_io_deinit(app_io_type_t type, uint32_t pin)
{
    switch(type)
    {
        case APP_IO_TYPE_NORMAL:
            if(APP_IO_PINS_0_15 & pin)
            {
                hal_gpio_deinit(GPIO0, (APP_IO_PINS_0_15 & pin));
            }
            if(APP_IO_PINS_16_31 & pin)
            {
                hal_gpio_deinit(GPIO1, (APP_IO_PINS_16_31 & pin) >> 16);
            }
            break;

        case APP_IO_TYPE_AON:
            hal_aon_gpio_deinit(APP_AON_IO_PIN_ALL & pin);
            break;

        case APP_IO_TYPE_MSIO:
            hal_msio_deinit(APP_MSIO_PIN_ALL & pin);
            break;

        default:
            return APP_DRV_ERR_INVALID_TYPE;
    }

    return APP_DRV_SUCCESS;
}


app_io_pin_state_t app_io_read_pin(app_io_type_t type, uint32_t pin)
{
    app_io_pin_state_t   pin_state        = APP_IO_PIN_RESET;
    gpio_pin_state_t     io_pin_state     = GPIO_PIN_RESET;
    aon_gpio_pin_state_t aon_io_pin_state = AON_GPIO_PIN_RESET;
    msio_pin_state_t     msio_pin_state   = MSIO_PIN_RESET;

    switch(type)
    {
        case APP_IO_TYPE_NORMAL:
            if(APP_IO_PINS_0_15 & pin)
            {
                io_pin_state = hal_gpio_read_pin(GPIO0, pin);
            }
            if(APP_IO_PINS_16_31 & pin)
            {
                io_pin_state = hal_gpio_read_pin(GPIO1, pin >> 16);
            }
            pin_state = (app_io_pin_state_t)io_pin_state;
            break;

        case APP_IO_TYPE_AON:
            aon_io_pin_state = hal_aon_gpio_read_pin(pin);
            pin_state = (app_io_pin_state_t)aon_io_pin_state;
            break;

        case APP_IO_TYPE_MSIO:
            msio_pin_state = hal_msio_read_pin(pin);
            pin_state = (app_io_pin_state_t)msio_pin_state;
            break;

        default:
            break;
    }

    return pin_state;
}

uint16_t app_io_write_pin(app_io_type_t type, uint32_t pin, app_io_pin_state_t pin_state)
{
    if (pin_state != APP_IO_PIN_RESET && pin_state != APP_IO_PIN_SET)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

    switch(type)
    {
        case APP_IO_TYPE_NORMAL:
            if(APP_IO_PINS_0_15 & pin)
            {
                hal_gpio_write_pin(GPIO0, (uint16_t)(APP_IO_PINS_0_15 & pin), (gpio_pin_state_t)pin_state);
            }
            if(APP_IO_PINS_16_31 & pin)
            {
                hal_gpio_write_pin(GPIO1, (uint16_t)((APP_IO_PINS_16_31 & pin) >> 16), (gpio_pin_state_t)pin_state);
            }
            break;

        case APP_IO_TYPE_AON:
            hal_aon_gpio_write_pin(pin, (aon_gpio_pin_state_t)pin_state);
            break;

        case APP_IO_TYPE_MSIO:
            hal_msio_write_pin(pin, (msio_pin_state_t)pin_state);
            break;

        default:
            return APP_DRV_ERR_INVALID_TYPE;
    }

    return APP_DRV_SUCCESS;
}

uint16_t app_io_toggle_pin(app_io_type_t type, uint32_t pin)
{
    switch(type)
    {
        case APP_IO_TYPE_NORMAL:
            if(APP_IO_PINS_0_15 & pin)
            {
                hal_gpio_toggle_pin(GPIO0, (uint16_t)(APP_IO_PINS_0_15 & pin));
            }
            if(APP_IO_PINS_16_31 & pin)
            {
                hal_gpio_toggle_pin(GPIO1, (uint16_t)((APP_IO_PINS_16_31 & pin) >> 16));
            }
            break;

        case APP_IO_TYPE_AON:
            hal_aon_gpio_toggle_pin(pin);
            break;

        case APP_IO_TYPE_MSIO:
           hal_msio_toggle_pin(pin);
           break;

        default:
            return APP_DRV_ERR_INVALID_TYPE;
    }

    return APP_DRV_SUCCESS;
}


