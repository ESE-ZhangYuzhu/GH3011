/**
  ****************************************************************************************
  * @file    gr55xx_hal_aon_wdt.c
  * @author  BLE Driver Team
  * @brief   AON WDT HAL module driver.
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

#if defined(HAL_AON_WDT_MODULE_ENABLED) && defined(GR5515_D)

/* extern function -----------------------------------------------------------*/

extern hal_status_t hal_aon_wdt_init_ext(aon_wdt_handle_t *p_aon_wdt);
extern hal_status_t hal_aon_wdt_deinit_ext(aon_wdt_handle_t *p_aon_wdt);
extern void hal_aon_wdt_register_callback(hal_aon_wdt_callback_t *aon_wdt_callback);

/* Private variables ---------------------------------------------------------*/

static hal_aon_wdt_callback_t aon_wdt_callback = 
{
    .aon_wdt_alarm_callback = hal_aon_wdt_alarm_callback,
};

/* Private function prototypes -----------------------------------------------*/

hal_status_t hal_aon_wdt_init(aon_wdt_handle_t *p_aon_wdt)
{
    hal_aon_wdt_register_callback(&aon_wdt_callback);

    return hal_aon_wdt_init_ext(p_aon_wdt);
}

hal_status_t hal_aon_wdt_deinit(aon_wdt_handle_t *p_aon_wdt)
{
    hal_aon_wdt_register_callback(&aon_wdt_callback);

    return hal_aon_wdt_deinit_ext(p_aon_wdt);
}

__WEAK void hal_aon_wdt_alarm_callback(aon_wdt_handle_t *p_aon_wdt)
{

}

#endif /* HAL_AON_WDT_MODULE_ENABLED */
