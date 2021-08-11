/**
  ****************************************************************************************
  * @file    gr55xx_hal.c
  * @author  BLE Driver Team
  * @brief   HAL module driver.
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

#if defined(HAL_MODULE_ENABLED) && defined(GR5515_D)

/* extern function -----------------------------------------------------------*/

extern hal_status_t hal_init_ext(void);
extern hal_status_t hal_deinit_ext(void);
extern void hal_register_callback(hal_callback_t *hal_callback);

/* Private variables ---------------------------------------------------------*/

static hal_callback_t hal_callback = 
{
    .msp_init       = hal_msp_init,
    .msp_deinit     = hal_msp_deinit
};

/* Private function prototypes -----------------------------------------------*/

hal_status_t hal_init(void)
{
    hal_register_callback(&hal_callback);
    
    return hal_init_ext();
}

hal_status_t hal_deinit(void)
{
    hal_register_callback(&hal_callback);

    return hal_deinit_ext();
}

__WEAK void hal_msp_init(void)
{
    /* NOTE : This function Should not be modified, when the callback is needed,
            the hal_msp_deinit could be implemented in the user file
    */
}

__WEAK void hal_msp_deinit(void)
{
    /* NOTE : This function Should not be modified, when the callback is needed,
            the hal_msp_deinit could be implemented in the user file
    */
}

#endif /* HAL_MODULE_ENABLED */
