/**
  ****************************************************************************************
  * @file    gr55xx_hal_i2s_br.c
  * @author  BLE Driver Team
  * @brief   I2S HAL module driver.
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

#if defined(HAL_I2S_MODULE_ENABLED) && defined(GR5515_D)

/* extern function -----------------------------------------------------------*/

extern hal_status_t hal_i2s_init_ext(i2s_handle_t *p_i2s);
extern hal_status_t hal_i2s_deinit_ext(i2s_handle_t *p_i2s);
extern void hal_i2s_register_callback(hal_i2s_callback_t *hal_i2s_callback);

/* Private variables ---------------------------------------------------------*/

static hal_i2s_callback_t i2s_callback = 
{
    .i2s_msp_init            = hal_i2s_msp_init,
    .i2s_msp_deinit          = hal_i2s_msp_deinit,
    .i2s_error_callback      = hal_i2s_error_callback,
    .i2s_rx_cplt_callback    = hal_i2s_rx_cplt_callback,
    .i2s_tx_cplt_callback    = hal_i2s_tx_cplt_callback,
    .i2s_tx_rx_cplt_callback = hal_i2s_tx_rx_cplt_callback,
};

/* Private function prototypes -----------------------------------------------*/

hal_status_t hal_i2s_init(i2s_handle_t *p_i2s)
{
    hal_i2s_register_callback(&i2s_callback);

    return hal_i2s_init_ext(p_i2s);
}

hal_status_t hal_i2s_deinit(i2s_handle_t *p_i2s)
{
    hal_i2s_register_callback(&i2s_callback);

    return hal_i2s_deinit_ext(p_i2s);
}

__WEAK void hal_i2s_msp_init(i2s_handle_t *p_i2s)
{
    /* Prevent unused argument(s) compilation warning */
    return;

    /* NOTE : This function should not be modified, when the callback is needed,
              the hal_i2s_msp_init can be implemented in the user file
     */
}

__WEAK void hal_i2s_msp_deinit(i2s_handle_t *p_i2s)
{
    /* Prevent unused argument(s) compilation warning */
    return;

    /* NOTE : This function should not be modified, when the callback is needed,
              the hal_i2s_msp_deinit can be implemented in the user file
     */
}

__WEAK void hal_i2s_error_callback(i2s_handle_t *p_i2s)
{

}

__WEAK void hal_i2s_rx_cplt_callback(i2s_handle_t *p_i2s)
{

}

__WEAK void hal_i2s_tx_cplt_callback(i2s_handle_t *p_i2s)
{

}

__WEAK void hal_i2s_tx_rx_cplt_callback(i2s_handle_t *p_i2s)
{

}

#endif /* HAL_I2S_MODULE_ENABLED */
