/**
  ****************************************************************************************
  * @file    gr55xx_hal_pwr_br.c
  * @author  BLE Driver Team
  * @brief   PWR HAL module driver.
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

#if defined(HAL_PWR_MODULE_ENABLED) && defined(GR5515_D)

/* extern function -----------------------------------------------------------*/

extern void hal_pwr_config_timer_wakeup_ext(uint8_t timer_mode, uint32_t load_count);
extern void hal_pwr_register_timer_elaspsed_handler(pwr_slp_elapsed_handler_t pwr_slp_elapsed_hander);

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

void hal_pwr_config_timer_wakeup(uint8_t timer_mode, uint32_t load_count)
{
    hal_pwr_register_timer_elaspsed_handler(hal_pwr_sleep_timer_elapsed_callback);
    
    hal_pwr_config_timer_wakeup_ext(timer_mode, load_count);
}

__WEAK void hal_pwr_sleep_timer_elapsed_callback(void)
{
    return;
}


#endif /* HAL_PWR_MODULE_ENABLED */
