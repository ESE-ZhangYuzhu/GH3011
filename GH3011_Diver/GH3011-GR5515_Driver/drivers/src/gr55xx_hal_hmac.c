/**
  ****************************************************************************************
  * @file    gr55xx_hal_hmac.c
  * @author  BLE Driver Team
  * @brief   HMAC HAL module driver.
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

#if defined(HAL_HMAC_MODULE_ENABLED) && defined(GR5515_D)

/* extern function -----------------------------------------------------------*/

extern hal_status_t hal_hmac_init_ext(hmac_handle_t *p_hmac);
extern hal_status_t hal_hmac_deinit_ext(hmac_handle_t *p_hmac);
extern void hal_hmac_register_callback(hal_hmac_callback_t *hal_hmac_callback);

/* Private variables ---------------------------------------------------------*/

static hal_hmac_callback_t hmac_callback =
{
    .hmac_msp_init              = hal_hmac_msp_init,
    .hmac_msp_deinit            = hal_hmac_msp_deinit,
    .hmac_done_callback         = hal_hmac_done_callback,
    .hmac_error_callback        = hal_hmac_error_callback,
    .hmac_abort_cplt_callback   = hal_hmac_abort_cplt_callback
};

/* Private function prototypes -----------------------------------------------*/

hal_status_t hal_hmac_init(hmac_handle_t *p_hmac)
{
    hal_hmac_register_callback(&hmac_callback);

    return hal_hmac_init_ext(p_hmac);
}

hal_status_t hal_hmac_deinit(hmac_handle_t *p_hmac)
{
    hal_hmac_register_callback(&hmac_callback);

    return hal_hmac_deinit_ext(p_hmac);
}

__WEAK void hal_hmac_msp_init(hmac_handle_t *p_hmac)
{
    /* Prevent unused argument(s) compilation warning */
    return;

    /* NOTE : This function should not be modified, when the callback is needed,
            the hal_hmac_msp_init can be implemented in the user file
     */
}

__WEAK void hal_hmac_msp_deinit(hmac_handle_t *p_hmac)
{
    /* Prevent unused argument(s) compilation warning */
    return;

    /* NOTE : This function should not be modified, when the callback is needed,
            the hal_hmac_msp_deinit can be implemented in the user file
     */
}

__WEAK void hal_hmac_done_callback(hmac_handle_t *p_hmac)
{

}

__WEAK void hal_hmac_error_callback(hmac_handle_t *p_hmac)
{

}

__WEAK void hal_hmac_abort_cplt_callback(hmac_handle_t *p_hmac)
{

}

#endif /* HAL_HMAC_MODULE_ENABLED */
