/**
  ****************************************************************************************
  * @file    gr55xx_hal_comp.c
  * @author  BLE Driver Team
  * @brief   COMP HAL module driver.
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

#if defined(HAL_COMP_MODULE_ENABLED) && defined(GR5515_D)

/* extern function -----------------------------------------------------------*/

extern hal_status_t hal_comp_init_ext(comp_handle_t *p_comp);
extern hal_status_t hal_comp_deinit_ext(comp_handle_t *p_comp);
extern void hal_comp_register_callback(comp_callback_t *comp_callback);

/* Private variables ---------------------------------------------------------*/

static comp_callback_t comp_callback = 
{
    .comp_msp_init             = hal_comp_msp_init,
    .comp_msp_deinit           = hal_comp_msp_deinit,
    .comp_trigger_callback     = hal_comp_trigger_callback
};

/* Private function prototypes -----------------------------------------------*/

hal_status_t hal_comp_init(comp_handle_t *p_comp)
{
    hal_comp_register_callback(&comp_callback);

    return hal_comp_init_ext(p_comp);
}

hal_status_t hal_comp_deinit(comp_handle_t *p_comp)
{
    hal_comp_register_callback(&comp_callback);

    return hal_comp_deinit_ext(p_comp);
}

__WEAK void hal_comp_trigger_callback(comp_handle_t *p_comp)
{
    return;
}

#endif /* HAL_COMP_MODULE_ENABLED */

