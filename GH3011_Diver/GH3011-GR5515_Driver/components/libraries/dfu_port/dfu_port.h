/**
 *****************************************************************************************
 *
 * @file dfu_port.h
 *
 * @brief  DFU port API.
 *
 *****************************************************************************************
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
 *****************************************************************************************
 */
#ifndef _DFU_PORT_H__
#define _DFU_PORT_H__

#include <stdint.h>
#include "gr55xx_dfu.h"

/**@brief DFU uart send data function definition. */
typedef void (*dfu_uart_send_data)(uint8_t *p_data, uint16_t length);

/**@brief DFU enter callback definition. */
typedef void (*dfu_enter_callback)(void);

/**
 *****************************************************************************************
 * @brief DFU BLE service init.
 * @details If dfu enter function is not used, dfu_enter can set NULL.
 *
 * @param[in] dfu_enter: DFU enter callback.
 *****************************************************************************************
 */
void dfu_service_init(dfu_enter_callback dfu_enter);

/**
 *****************************************************************************************
 * @brief DFU port init.
 * @details If not using serial port update function, uart_send_data can be set NULL.
            if the user doesn't care about the upgrade status,p_dfu_callback can set NULL.
 *
 * @param[in] uart_send_data: Function is used to send data to master by UART.
 * @param[in] p_dfu_callback: DFU program state callback functions.
 *****************************************************************************************
 */
void dfu_port_init(dfu_uart_send_data uart_send_data, dfu_pro_callback_t *p_dfu_callback);

#endif

