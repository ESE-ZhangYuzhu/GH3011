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

#if defined(HAL_UART_MODULE_ENABLED) && defined(GR5515_D)

/* extern function -----------------------------------------------------------*/

extern hal_status_t hal_uart_init_ext(uart_handle_t *p_uart);
extern hal_status_t hal_uart_deinit_ext (uart_handle_t *p_uart);
extern void hal_uart_register_callback(hal_uart_callback_t *hal_uart_callback);

/* Private variables ---------------------------------------------------------*/

static hal_uart_callback_t uart_callback =
{
    .uart_msp_init                  = hal_uart_msp_init,
    .uart_msp_deinit                = hal_uart_msp_deinit,
    .uart_tx_cplt_callback          = hal_uart_tx_cplt_callback,
    .uart_rx_cplt_callback          = hal_uart_rx_cplt_callback,
    .uart_error_callback            = hal_uart_error_callback,
    .uart_abort_cplt_callback       = hal_uart_abort_cplt_callback,
    .uart_abort_tx_cplt_callback    = hal_uart_abort_tx_cplt_callback,
    .uart_abort_rx_cplt_callback    = hal_uart_abort_rx_cplt_callback
};

/* Private function prototypes -----------------------------------------------*/

hal_status_t hal_uart_init(uart_handle_t *p_uart)
{
    hal_uart_register_callback(&uart_callback);

    return hal_uart_init_ext(p_uart);
}

hal_status_t hal_uart_deinit(uart_handle_t *p_uart)
{
    hal_uart_register_callback(&uart_callback);

    return hal_uart_deinit_ext(p_uart);
}

__WEAK void hal_uart_msp_init(uart_handle_t *p_uart)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(p_uart);
}

__WEAK void hal_uart_msp_deinit(uart_handle_t *p_uart)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(p_uart);
}

__WEAK void hal_uart_tx_cplt_callback(uart_handle_t *p_uart)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(p_uart);
}

__WEAK void hal_uart_rx_cplt_callback(uart_handle_t *p_uart)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(p_uart);
}

__WEAK void hal_uart_error_callback(uart_handle_t *p_uart)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(p_uart);
}

__WEAK void hal_uart_abort_cplt_callback (uart_handle_t *p_uart)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(p_uart);
}

__WEAK void hal_uart_abort_tx_cplt_callback (uart_handle_t *p_uart)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(p_uart);
}

__WEAK void hal_uart_abort_rx_cplt_callback (uart_handle_t *p_uart)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(p_uart);
}

#endif
