/**
  ****************************************************************************************
  * @file    gr55xx_hal_gpio.c
  * @author  BLE Driver Team
  * @brief   GPIO HAL module driver.
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

/* Includes ------------------------------------------------------------------*/
#include "gr55xx_hal.h"

#if defined(HAL_GPIO_MODULE_ENABLED) && defined(GR5515_D)

/* extern function -----------------------------------------------------------*/

extern void hal_gpio_init_ext(gpio_regs_t *GPIOx, gpio_init_t *p_gpio_init);
extern void hal_gpio_deinit_ext(gpio_regs_t *GPIOx, uint32_t gpio_pin);
extern void hal_gpio_register_callback(hal_gpio_callback_t *callback);

/* Private variables ---------------------------------------------------------*/

static hal_gpio_callback_t gpio_callback =
{
    .gpio_callback = hal_gpio_exti_callback,
};

/* Private function prototypes -----------------------------------------------*/

void hal_gpio_init(gpio_regs_t *GPIOx, gpio_init_t *p_gpio_init)
{
    hal_gpio_register_callback(&gpio_callback);
    
    hal_gpio_init_ext(GPIOx, p_gpio_init);
}

void hal_gpio_deinit(gpio_regs_t *GPIOx, uint32_t gpio_pin)
{
    hal_gpio_register_callback(&gpio_callback);
    
    hal_gpio_deinit_ext(GPIOx, gpio_pin);
}

__WEAK void hal_gpio_exti_callback(gpio_regs_t *GPIOx, uint16_t gpio_pin)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(GPIOx);
    UNUSED(gpio_pin);
}

#endif /* HAL_GPIO_MODULE_ENABLED */
