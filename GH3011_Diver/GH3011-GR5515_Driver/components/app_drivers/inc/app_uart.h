/**
 ****************************************************************************************
 *
 * @file    app_uart.h
 * @author  BLE Driver Team
 * @brief   Header file containing functions prototypes of UART app library.
 *
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

/** @addtogroup PERIPHERAL Peripheral Driver
  * @{
  */
  
/** @addtogroup APP_DRIVER APP DRIVER
 *  @{
 */

/** @defgroup APP_UART UART
  * @brief UART APP module driver.
  * @{
  */


#ifndef _APP_UART_H_
#define _APP_UART_H_

#include "gr55xx_hal.h"
#include "ring_buffer.h"
#include "app_io.h"

#ifdef ENV_USE_FREERTOS
#include "app_rtos_cfg.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAL_UART_MODULE_ENABLED

/** @addtogroup APP_UART_STRUCTURES Structures
  * @{
  */

/**
  * @brief UART module Enumerations definition
  */
typedef enum
{
    APP_UART_ID_0,                     /**< UART module 0 */
    APP_UART_ID_1,                     /**< UART module 1 */
    APP_UART_ID_MAX,                   /**< Only for check parameter, not used as input parameters. */
}app_uart_id_t;

/**
  * @brief UART operating mode Enumerations definition
  */
typedef enum
{
    APP_UART_TYPE_INTERRUPT,           /**< Interrupt operation mode */
    APP_UART_TYPE_POLLING,             /**< Polling operation mode   */
    APP_UART_TYPE_DMA,                 /**< DMA operation mode       */
    APP_UART_TYPE_MAX,                 /**< Only for check parameter, not used as input parameters. */
}app_uart_type_t;

/**
  * @brief UART event Enumerations definition
  */
typedef enum
{
    APP_UART_EVT_ERROR,                /**< Error reported by UART peripheral. */
    APP_UART_EVT_TX_CPLT,              /**< Requested TX transfer completed.   */
    APP_UART_EVT_RX_DATA,              /**< Requested RX transfer completed.   */
    APP_UART_EVT_ABORT_TX,             /**< Requested TX abort completed.      */
    APP_UART_EVT_ABORT_RX,             /**< Requested RX abort completed.      */
} app_uart_evt_type_t;

/** @} */

/** @addtogroup APP_UART_STRUCTURES Structures
  * @{
  */

/**
  * @brief UART IO configuration Structures
  */
typedef struct
{
    app_io_type_t      type;           /**< Specifies the type of UART IO. */
    app_io_mux_t       mux;            /**< Specifies the Peripheral to be connected to the selected pins. */
    uint32_t           pin;            /**< Specifies the IO pins to be configured.
                                            This parameter can be any value of @ref GR551x_pins. */
    app_io_pull_t      pull;           /**< Specifies the Pull-up or Pull-Down activation for the selected pins. */

}app_uart_pin_t;

typedef struct
{
    app_uart_pin_t    tx;              /**< Set the configuration of UART TX pin.  */
    app_uart_pin_t    rx;              /**< Set the configuration of UART RX pin.  */
    app_uart_pin_t    cts;             /**< Set the configuration of UART CTS pin. */
    app_uart_pin_t    rts;             /**< Set the configuration of UART RTS pin. */
}app_uart_pin_cfg_t;

/**
  * @brief UART operate mode Enumerations definition
  */
typedef struct
{
    app_uart_type_t   type;            /**< Specifies the operation mode of UART. */
    dma_channel_t     tx_dma_channel;  /**< Specifies the dma channel of UART TX. */
    dma_channel_t     rx_dma_channel;  /**< Specifies the dma channel of UART RX. */
}app_uart_mode_t;

/**
  * @brief UART parameters structure definition
  */
typedef struct
{
    app_uart_id_t      id;             /**< specified UART module ID.                                        */
    app_uart_pin_cfg_t pin_cfg;        /**< the pin configuration information for the specified UART module. */
    app_uart_mode_t    use_mode;       /**< UART operate mode.                                               */
    uart_init_t        init;           /**< UART communication parameters.                                   */
} app_uart_params_t;

/**
  * @brief UART event structure definition
  */
typedef struct
{
    app_uart_evt_type_t type; /**< Type of event. */
    union
    {
        uint32_t error_code;           /**< UART Error code . */     
        uint16_t size;                 /**< UART transmitted/received counter. */
    } data;
} app_uart_evt_t;

/**
  * @brief UART event callback definition
  */
typedef void (*app_uart_evt_handler_t)(app_uart_evt_t *p_evt);

/**
  * @brief UART buffer structure definition
  */
typedef struct
{
    uint8_t * tx_buf;      /**< Pointer to the TX buffer. */
    uint32_t  tx_buf_size; /**< Size of the TX buffer. */
} app_uart_tx_buf_t;
/** @} */


/* Exported functions --------------------------------------------------------*/
/** @addtogroup APP_UART_DRIVER_FUNCTIONS Functions
  * @{
  */
/**
 ****************************************************************************************
 * @brief  Initialize the APP UART DRIVER according to the specified parameters
 *         in the app_uart_params_t and app_uart_evt_handler_t.
 *
 * @param[in]  p_params: Pointer to app_uart_params_t parameter which contains the
 *                       configuration information for the specified UART module.
 * @param[in]  evt_handler: UART user callback function.
 * @param[in]  tx_buffer: Pointer to tx send buffer.
 *
 * @return Result of initialization.
 ****************************************************************************************
 */
uint16_t app_uart_init(app_uart_params_t *p_params, app_uart_evt_handler_t evt_handler, app_uart_tx_buf_t *tx_buffer);

/**
 ****************************************************************************************
 * @brief  De-initialize the APP UART DRIVER peripheral.
 *
 * @param[in]  id: De-initialize for a specific ID.
 *
 * @return Result of De-initialization.
 ****************************************************************************************
 */
uint16_t app_uart_deinit(app_uart_id_t id);

/**
 ****************************************************************************************
 * @brief  Send an amount of data in interrupt mode.
 *
 * @param[in]  id: which UART module want to receive.
 * @param[in]  p_data: Pointer to data buffer
 * @param[in]  size: Amount of data to be sent
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_uart_transmit_async(app_uart_id_t id, uint8_t *p_data, uint16_t size);

/**
 ****************************************************************************************
 * @brief  Send an amount of data in blocking mode.
 *
 * @param[in]  id: which UART module want to receive.
 * @param[in]  p_data: Pointer to data buffer
 * @param[in]  size: Amount of data to be sent
 * @param[in]  timeout: Timeout duration
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_uart_transmit_sync(app_uart_id_t id, uint8_t *p_data, uint16_t size, uint32_t timeout);

/**
 ****************************************************************************************
 * @brief  Receive an amount of data in interrupt mode.
 *
 * @param[in]  id: which UART module want to transmit.
 * @param[in]  p_data: Pointer to data buffer
 * @param[in]  size: Amount of data to be sent
 * @param[in]  timeout: Timeout duration
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_uart_receive_async(app_uart_id_t id, uint8_t *p_data, uint16_t size);

/**
 ****************************************************************************************
 * @brief  Receive an amount of data in blocking mode.
 *
 * @param[in]  id: which UART module want to transmit.
 * @param[in]  p_data: Pointer to data buffer
 * @param[in]  size: Amount of data to be sent
 * @param[in]  timeout: Timeout duration
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_uart_receive_sync(app_uart_id_t id, uint8_t *p_data, uint16_t size, uint32_t timeout);

/**
 ****************************************************************************************
 * @brief  Return the UART handle.
 *
 * @param[in]  id: UART Channel ID.
 *
 * @return Pointer to the specified ID's UART handle.
 ****************************************************************************************
 */
uart_handle_t *app_uart_get_handle(app_uart_id_t id);

/**
 *****************************************************************************************
 * @brief Flush all log entries from the buffer
 *
 * @param[in]  id: UART Channel ID.
 *
 *****************************************************************************************
 */
void app_uart_flush(app_uart_id_t id);


#ifdef  ENV_RTOS_USE_SEMP
/**
 ****************************************************************************************
 * @brief [RTOS] Receive an amount of data in blocking mode.
 *
 * @param[in]  id: which UART module want to transmit.
 * @param[in]  p_data: Pointer to data buffer
 * @param[in]  size: Amount of data to be sent
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_uart_receive_sem_sync(app_uart_id_t id, uint8_t *p_data, uint16_t size);

/**
 ****************************************************************************************
 * @brief [RTOS] Send an amount of data in blocking mode.
 *
 * @param[in]  id: which UART module want to receive.
 * @param[in]  p_data: Pointer to data buffer
 * @param[in]  size: Amount of data to be sent
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_uart_transmit_sem_sync(app_uart_id_t id, uint8_t *p_data, uint16_t size);

#endif

/** @} */

#endif

#ifdef __cplusplus
}
#endif

#endif

/** @} */
/** @} */
/** @} */


