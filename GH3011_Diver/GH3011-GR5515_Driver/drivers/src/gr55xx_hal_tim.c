/**
  ****************************************************************************************
  * @file    gr55xx_hal_tim.c
  * @author  BLE Driver Team
  * @brief   TIMER HAL module driver.
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

#include "gr55xx_hal.h"

#if defined(HAL_TIMER_MODULE_ENABLED) && defined(GR5515_D)

/* extern function -----------------------------------------------------------*/

extern hal_status_t hal_timer_init_ext(timer_handle_t *p_timer);
extern hal_status_t hal_timer_deinit_ext(timer_handle_t *p_timer);
extern void hal_timer_register_callback(hal_timer_callback_t *hal_timer_callback);

/* Private variables ---------------------------------------------------------*/

static hal_timer_callback_t timer_callback =
{
    .timer_msp_init                   = hal_timer_base_msp_init,
    .timer_msp_deinit                 = hal_timer_base_msp_deinit,
    .timer_period_elapsed_callback    = hal_timer_period_elapsed_callback
};

/* Private function prototypes -----------------------------------------------*/

hal_status_t hal_timer_base_init(timer_handle_t *p_timer)
{
    hal_timer_register_callback(&timer_callback);

    return hal_timer_init_ext(p_timer);
}

hal_status_t hal_timer_base_deinit(timer_handle_t *p_timer)
{
    hal_timer_register_callback(&timer_callback);

    return hal_timer_deinit_ext(p_timer);
}

__WEAK void hal_timer_base_msp_init(timer_handle_t *p_timer)
{
    /* Prevent unused argument(s) compilation warning */
    return;
}

__WEAK void hal_timer_base_msp_deinit(timer_handle_t *p_timer)
{
    /* Prevent unused argument(s) compilation warning */
    return;
}

__weak void hal_timer_period_elapsed_callback(timer_handle_t *p_timer)
{
    return;
}

#endif
