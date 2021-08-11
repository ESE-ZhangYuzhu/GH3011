/**
  ****************************************************************************************
  * @file    gr55xx_hal_aon_gpio.c
  * @author  BLE Driver Team
  * @brief   AON GPIO HAL module driver.
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

#if defined(HAL_AON_GPIO_MODULE_ENABLED) && defined(GR5515_D)

/* extern function -----------------------------------------------------------*/

extern void hal_aon_gpio_init_ext(aon_gpio_init_t *p_aon_gpio_init);
extern void hal_aon_gpio_deinit_ext(uint32_t aon_gpio_pin);
extern void hal_aon_gpio_register_callback(aon_gpio_callback_t *aon_gpio_callback);

/* Private variables ---------------------------------------------------------*/

static aon_gpio_callback_t aon_gpio_callback = 
{
    .aon_gpio_callback = hal_aon_gpio_callback,
};

/* Private function prototypes -----------------------------------------------*/

void hal_aon_gpio_init(aon_gpio_init_t *p_aon_gpio_init)
{
    hal_aon_gpio_register_callback(&aon_gpio_callback);

    hal_aon_gpio_init_ext(p_aon_gpio_init);
}

void hal_aon_gpio_deinit(uint32_t aon_gpio_pin)
{
    hal_aon_gpio_deinit_ext(aon_gpio_pin);
}

__WEAK void hal_aon_gpio_callback(uint16_t aon_gpio_pin)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(aon_gpio_pin);
}

#endif /* HAL_AON_GPIO_MODULE_ENABLED */
