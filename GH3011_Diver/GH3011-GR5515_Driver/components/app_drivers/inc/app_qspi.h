/**
 ****************************************************************************************
 *
 * @file    app_qspi.h
 * @author  BLE Driver Team
 * @brief   Header file containing functions prototypes of QSPI app library.
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

/** @defgroup APP_QSPI QSPI
  * @brief QSPI APP module driver.
  * @{
  */


#ifndef _APP_QSPI_H_
#define _APP_QSPI_H_

#include "gr55xx_hal.h"
#include "app_io.h"
#include "app_drv_error.h"
#include "app_rtos_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAL_QSPI_MODULE_ENABLED

/** @addtogroup APP_QSPI_DEFINE Defines
  * @{
  */

#define APP_QSPI_PIN_ENABLE      1    /**< QSPI pin enable  */
#define APP_QSPI_PIN_DISABLE     0    /**< QSPI pin disable */

/** @} */

/** @addtogroup APP_QSPI_ENUM Enumerations
  * @{
  */

/**
  * @brief QSPI module Enumerations definition
  */
typedef enum
{
    APP_QSPI_ID_0,              /**< QSPI module 0 */
    APP_QSPI_ID_1,              /**< QSPI module 1 */
    APP_QSPI_ID_MAX             /**< Only for check parameter, not used as input parameters. */
} app_qspi_id_t;

/**
  * @brief QSPI operating mode Enumerations definition
  */
typedef enum
{
    APP_QSPI_TYPE_INTERRUPT,    /**< Interrupt operation mode */
    APP_QSPI_TYPE_POLLING,      /**< Polling operation mode   */
    APP_QSPI_TYPE_DMA,          /**< DMA operation mode       */
    APP_QSPI_TYPE_MAX,          /**< Only for check parameter, not used as input parameters. */
} app_qspi_type_t;

/** @addtogroup HAL_APP_QSPI_STRUCTURES Event Structures
  * @{
  */
typedef enum
{
    APP_QSPI_EVT_ERROR,          /**< Error reported by UART peripheral. */
    APP_QSPI_EVT_TX_CPLT,        /**< Requested TX transfer completed.   */
    APP_QSPI_EVT_RX_DATA,        /**< Requested RX transfer completed.   */
} app_qspi_evt_type_t;
/** @} */

/** @addtogroup APP_QSPI_STRUCTURES Structures
  * @{
  */
  
/**
  * @brief QSPI IO configuration Structures
  */
typedef struct
{
    app_io_type_t  type;         /**< Specifies the type of QSPI IO. */
    app_io_mux_t   mux;          /**< Specifies the Peripheral to be connected to the selected pins. */
    uint32_t       pin;          /**< Specifies the IO pins to be configured.
                                      This parameter can be any value of @ref GR551x_pins. */
    app_io_pull_t  pull;         /**< Specifies the Pull-up or Pull-Down activation for the selected pins. */
    uint8_t        enable;       /**< Enable or disable the pin. */
} app_qspi_pin_t;

/**
  * @brief QSPI configuration Structures
  */
typedef struct
{
    app_qspi_pin_t cs;           /**< Set the configuration of QSPI CS pin. */
    app_qspi_pin_t clk;          /**< Set the configuration of QSPI CLK pin. */
    app_qspi_pin_t io_0;         /**< Set the configuration of QSPI IO0 pin. */
    app_qspi_pin_t io_1;         /**< Set the configuration of QSPI IO1 pin. */
    app_qspi_pin_t io_2;         /**< Set the configuration of QSPI IO2 pin. */
    app_qspi_pin_t io_3;         /**< Set the configuration of QSPI IO3 pin. */
} app_qspi_pin_cfg_t;

/**
  * @brief QSPI operate mode Enumerations definition
  */
typedef struct
{
    app_qspi_type_t    type;        /**< Specifies the operation mode of QSPI. */
    dma_channel_t      dma_channel; /**< Specifies the dma channel of QSPI.    */
} app_qspi_mode_t;

/**
  * @brief QSPI parameters structure definition
  */
typedef struct
{
    app_qspi_id_t      id;       /**< specified QSPI module ID.                                        */
    app_qspi_pin_cfg_t pin_cfg;  /**< the pin configuration information for the specified QSPI module. */
    app_qspi_mode_t    use_mode; /**< QSPI operate mode.                                               */
    qspi_init_t        init;     /**< QSPI communication parameters.                                   */
} app_qspi_params_t;

/** @} */

/**
  * @brief QSPI command structure definition
  */
typedef qspi_command_t app_qspi_command_t;

/**
  * @brief QSPI event structure definition
  */
typedef struct
{
    app_qspi_evt_type_t type; /**< Type of event. */
    union
    {
        uint32_t error_code;     /**< QSPI Error code . */ 
        uint16_t size;           /**< QSPI transmitted/received counter. */
    } data;
} app_qspi_evt_t;

/**
  * @brief QSPI event callback definition
  */
typedef void (*app_qspi_evt_handler_t)(app_qspi_evt_t *p_evt);
/** @} */


/* Exported functions --------------------------------------------------------*/
/** @addtogroup HAL_APP_QSPI_DRIVER_FUNCTIONS Functions
  * @{
  */
/**
 ****************************************************************************************
 * @brief  Initialize the APP QSPI DRIVER according to the specified parameters
 *         in the app_qspi_params_t and app_qspi_evt_handler_t.
 * @note   If interrupt mode is set, you can use blocking mode. Conversely, if blocking mode
 *         is set, you can't use interrupt mode.
 *
 * @param[in]  p_params: Pointer to app_qspi_params_t parameter which contains the
 *                       configuration information for the specified QSPI module.
 * @param[in]  evt_handler: QSPI user callback function.
 *
 * @return Result of initialization.
 ****************************************************************************************
 */
uint16_t app_qspi_init(app_qspi_params_t *p_params, app_qspi_evt_handler_t evt_handler);

/**
 ****************************************************************************************
 * @brief  De-initialize the APP QSPI DRIVER peripheral.
 *
 * @param[in]  id: De-initialize for a specific ID.
 *
 * @return Result of De-initialization.
 ****************************************************************************************
 */
uint16_t app_qspi_deinit(app_qspi_id_t id);

/**
 ****************************************************************************************
 * @brief  Receive an amount of data with the specified instruction, address and dummy cycles in blocking mode.
 * @note   This function is used only in Indirect Read Mode.
 * @param[in]  id: which QSPI module want to receive.
 * @param[in]  p_cmd: Pointer to a app_qspi_command_t structure that contains the instruction and address for data transfer.
 * @param[out] p_data: Pointer to data buffer
 * @param[in]  timeout: Timeout duration
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_qspi_command_receive_sync(app_qspi_id_t id, app_qspi_command_t *p_cmd, uint8_t *p_data, uint32_t timeout);

/**
 ****************************************************************************************
 * @brief  Receive an amount of data with the specified instruction, address and dummy cycles in non-blocking mode with Interrupt.
 * @note   This function is used only in Indirect Read Mode.
 * @param[in]  id: which QSPI module want to receive.
 * @param[in]  p_cmd: Pointer to a app_qspi_command_t structure that contains the instruction and address for data transfer.
 * @param[out] p_data: Pointer to data buffer
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_qspi_command_receive_async(app_qspi_id_t id, app_qspi_command_t *p_cmd, uint8_t *p_data);

/**
 ****************************************************************************************
 * @brief  Receive an amount of data with the specified instruction, address and dummy cycles in blocking mode.
 * @note   This function is used only in Indirect Read Mode.
 * @param[in]  id: which QSPI module want to transmit.
 * @param[in]  p_cmd: Pointer to a app_qspi_command_t structure that contains the instruction and address for data transfer.
 * @param[out] p_data: Pointer to data buffer
 * @param[in]  timeout: Timeout duration
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_qspi_command_transmit_sync(app_qspi_id_t id, app_qspi_command_t *p_cmd, uint8_t *p_data, uint32_t timeout);

/**
 ****************************************************************************************
 * @brief  Receive an amount of data with the specified instruction, address and dummy cycles in non-blocking mode with Interrupt.
 * @note   This function is used only in Indirect Read Mode.
 * @param[in]  id: which QSPI module want to transmit.
 * @param[in]  p_cmd: Pointer to a app_qspi_command_t structure that contains the instruction and address for data transfer.
 * @param[out] p_data: Pointer to data buffer
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_qspi_command_transmit_async(app_qspi_id_t id, app_qspi_command_t *p_cmd, uint8_t *p_data);

/**
 ****************************************************************************************
 * @brief  Transmit only instruction in blocking mode.
 * @note   This function is used only in Indirect Write Mode.
 * @param[in]  id: which QSPI module want to transmit command.
 * @param[in]  p_cmd: Pointer to a app_qspi_command_t structure that contains the instruction and address for data transfer.
 * @param[in]  timeout: Timeout duration
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_qspi_command_sync(app_qspi_id_t id, app_qspi_command_t *p_cmd, uint32_t timeout);

/**
 ****************************************************************************************
 * @brief  Transmit instruction in non-blocking mode with Interrupt.
 * @note   This function is used only in Indirect Write Mode.
 * @param[in]  id: which QSPI module want to transmit command.
 * @param[in]  p_cmd: Pointer to a app_qspi_command_t structure that contains the instruction and address for data transfer.
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_qspi_command_async(app_qspi_id_t id, app_qspi_command_t *p_cmd);

/**
 ****************************************************************************************
 * @brief  Transmit an amount of data in blocking mode with standard SPI.
 * @note   This function is used only in Indirect Write Mode.
 * @param[in]  id: which QSPI module want to transmit.
 * @param[in]  p_data: Pointer to data buffer
 * @param[in]  length: Amount of data to be sent in bytes
 * @param[in]  timeout: Timeout duration
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_qspi_transmit_sync(app_qspi_id_t id, uint8_t *p_data, uint32_t length, uint32_t timeout);

/**
 ****************************************************************************************
 * @brief Transmit an amount of data in non-blocking mode at standard SPI with Interrupt.
 * @note   This function is used only in Indirect Write Mode.
 * @param[in]  id: which QSPI module want to transmit.
 * @param[in]  p_data: Pointer to data buffer
 * @param[in]  length: Amount of data to be sent in bytes
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_qspi_transmit_async(app_qspi_id_t id, uint8_t *p_data, uint32_t length);

/**
 ****************************************************************************************
 * @brief  Receive an amount of data in blocking mode with standard SPI.
 * @note   This function is used only in Indirect Read Mode.
 * @param[in]  id: which QSPI module want to receive.
 * @param[out] p_data: Pointer to data buffer
 * @param[in]  length: Amount of data to be received in bytes
 * @param[in]  timeout: Timeout duration
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_qspi_receive_sync(app_qspi_id_t id, uint8_t *p_data, uint32_t length, uint32_t timeout);

/**
 ****************************************************************************************
 * @brief Receive an amount of data in non-blocking mode at standard SPI with Interrupt.
 * @note   This function is used only in Indirect Read Mode.
 * @param[in]  id: which QSPI module want to receive.
 * @param[out] p_data: Pointer to data buffer
 * @param[in]  length: Amount of data to be received in bytes
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_qspi_receive_async(app_qspi_id_t id, uint8_t *p_data, uint32_t length);

/**
 ****************************************************************************************
 * @brief  Return the QSPI handle.
 *
 * @param[in]  id: QSPI Channel ID.
 *
 * @return Pointer to the specified ID's QSPI handle.
 ****************************************************************************************
 */
qspi_handle_t *app_qspi_get_handle(app_qspi_id_t id);

#ifdef  ENV_RTOS_USE_SEMP

/**
 ****************************************************************************************
 * @brief  [RTOS] Receive an amount of data with the specified instruction, address and dummy cycles in blocking mode.
 * @note   This function is used only in Indirect Read Mode.
 * @param[in]  id: which QSPI module want to receive.
 * @param[in]  p_cmd: Pointer to a app_qspi_command_t structure that contains the instruction and address for data transfer.
 * @param[out] p_data: Pointer to data buffer
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_qspi_command_receive_sem_sync(app_qspi_id_t id, app_qspi_command_t *p_cmd, uint8_t *p_data);

/**
 ****************************************************************************************
 * @brief  [RTOS] Receive an amount of data with the specified instruction, address and dummy cycles in blocking mode.
 * @note   This function is used only in Indirect Read Mode.
 * @param[in]  id: which QSPI module want to transmit.
 * @param[in]  p_cmd: Pointer to a app_qspi_command_t structure that contains the instruction and address for data transfer.
 * @param[out] p_data: Pointer to data buffer
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_qspi_command_transmit_sem_sync(app_qspi_id_t id, app_qspi_command_t *p_cmd, uint8_t *p_data);

/**
 ****************************************************************************************
 * @brief  [RTOS] Transmit only instruction in blocking mode.
 * @note   This function is used only in Indirect Write Mode.
 * @param[in]  id: which QSPI module want to transmit command.
 * @param[in]  p_cmd: Pointer to a app_qspi_command_t structure that contains the instruction and address for data transfer.
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_qspi_command_sem_sync(app_qspi_id_t id, app_qspi_command_t *p_cmd);

/**
 ****************************************************************************************
 * @brief  [RTOS] Transmit an amount of data in blocking mode with standard SPI.
 * @note   This function is used only in Indirect Write Mode.
 * @param[in]  id: which QSPI module want to transmit.
 * @param[in]  p_data: Pointer to data buffer
 * @param[in]  length: Amount of data to be sent in bytes
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_qspi_transmit_sem_sync(app_qspi_id_t id, uint8_t *p_data, uint32_t length);

/**
 ****************************************************************************************
 * @brief  [RTOS] Receive an amount of data in blocking mode with standard SPI.
 * @note   This function is used only in Indirect Read Mode.
 * @param[in]  id: which QSPI module want to receive.
 * @param[out] p_data: Pointer to data buffer
 * @param[in]  length: Amount of data to be received in bytes
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_qspi_receive_sem_sync(app_qspi_id_t id, uint8_t *p_data, uint32_t length);

#endif

/**
 ****************************************************************************************
 * @brief  [High speed] Receive an amount of data with the specified instruction, address and dummy cycles in blocking mode.
 * @note   This function is used only in Indirect Read Mode.
 * @param[in]  id: which QSPI module want to receive.
 * @param[in]  p_cmd: Pointer to a app_qspi_command_t structure that contains the instruction and address for data transfer.
 * @param[out] p_data: Pointer to data buffer
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_qspi_command_receive_high_speed_sync(app_qspi_id_t id, app_qspi_command_t *p_cmd, uint8_t *p_data);

/**
 ****************************************************************************************
 * @brief  [High speed] Receive an amount of data with the specified instruction, address and dummy cycles in blocking mode.
 * @note   This function is used only in Indirect Read Mode.
 * @param[in]  id: which QSPI module want to transmit.
 * @param[in]  p_cmd: Pointer to a app_qspi_command_t structure that contains the instruction and address for data transfer.
 * @param[out] p_data: Pointer to data buffer
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_qspi_command_transmit_high_speed_sync(app_qspi_id_t id, app_qspi_command_t *p_cmd, uint8_t *p_data);

/**
 ****************************************************************************************
 * @brief  [High speed] Transmit only instruction in blocking mode.
 * @note   This function is used only in Indirect Write Mode.
 * @param[in]  id: which QSPI module want to transmit command.
 * @param[in]  p_cmd: Pointer to a app_qspi_command_t structure that contains the instruction and address for data transfer.
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_qspi_command_high_speed_sync(app_qspi_id_t id, app_qspi_command_t *p_cmd);

/**
 ****************************************************************************************
 * @brief  [High speed] Transmit an amount of data in blocking mode with standard SPI.
 * @note   This function is used only in Indirect Write Mode.
 * @param[in]  id: which QSPI module want to transmit.
 * @param[in]  p_data: Pointer to data buffer
 * @param[in]  length: Amount of data to be sent in bytes
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_qspi_transmit_high_speed_sync(app_qspi_id_t id, uint8_t *p_data, uint32_t length);

/**
 ****************************************************************************************
 * @brief  [High speed] Receive an amount of data in blocking mode with standard SPI.
 * @note   This function is used only in Indirect Read Mode.
 * @param[in]  id: which QSPI module want to receive.
 * @param[out] p_data: Pointer to data buffer
 * @param[in]  length: Amount of data to be received in bytes
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_qspi_receive_high_speed_sync(app_qspi_id_t id, uint8_t *p_data, uint32_t length);

/** @} */

#endif

#ifdef __cplusplus
}
#endif

#endif

/** @} */
/** @} */
/** @} */
