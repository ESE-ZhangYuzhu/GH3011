/**
 ****************************************************************************************
 *
 * @file otas.h
 *
 * @brief OTA Service API
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
 *****************************************************************************************
 */

/**
 * @addtogroup BLE_SRV BLE Services
 * @{
 * @brief Definitions and prototypes for the BLE Service interface.
 */

/**
 * @defgroup BLE_SDK_OTA OTA Service (OTAS)
 * @{
 * @brief Definitions and prototypes for the OTAS interface.
 *
 * @details The OTA Service exposes the state of a battery within a device.
 *          This module implements the Battery Service with the Battery Level
 *          characteristic. After @ref otas_init_t variable is initialized, the
 *          developer shall call @ref otas_service_init() to add the OTA
 *          Service and RX, TX, Control characteristic to the BLE stack database.
 *
 *          This module also provides \ref otas_notify_tx_data() function to the application
 *          to send data to peer. 
 */

#ifndef _OTAS_H_
#define _OTAS_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "gr55xx_sys.h"
#include "custom_config.h"

/**
 * @defgroup OTAS_MACRO Defines
 * @{
 */
#define OTAS_CONNECTION_MAX         (10 < CFG_MAX_CONNECTIONS ?\
                                    10 : CFG_MAX_CONNECTIONS)                            /**< Maximum number of OTA Service connections. */
#define OTAS_MAX_DATA_LEN           244                                                  /**< Maximum length of OTAS characteristic. */
#define BLE_UUID_OTA_SERVICE        0x1B, 0xD7, 0x90, 0xEC, 0xE8, 0xB9, 0x75, 0x80,\
                                    0x0A, 0x46, 0x44, 0xD3, 0x01, 0x04, 0xED, 0xA6       /**< The UUID of OTA Service for setting advertising data. */
#define OTAS_CTRL_ENTER_DFU         0x474f4f44                                           /**< OTAS enter DFU control. */
/** @} */

/**
 * @defgroup OTA_ENUM Enumerations
 * @{
 */
/**@brief OTA Service event type. */
typedef enum
{
    OTAS_EVT_INVALID,
    OTAS_EVT_TX_NOTIFICATION_ENABLED,
    OTAS_EVT_TX_NOTIFICATION_DISABLED,
    OTAS_EVT_RX_RECEIVE_DATA,
    OTAS_EVT_NOTIFY_COMPLETE,
    OTAS_EVT_DFU_MODE_ENTER,
} otas_evt_type_t;
/** @} */

/**
 * @defgroup OTAS_STRUCT Structures
 * @{
 */
/**@brief OTA Service event. */
typedef struct
{
    otas_evt_type_t evt_type;         /**< The OTAS event. */
    uint8_t         conn_idx;         /**< Index of connection. */
    uint8_t        *p_data;           /**< Pointer to data. */
    uint16_t        length;           /**< Length of data. */
} otas_evt_t;
/** @} */

/**
 * @defgroup OTAS_TYPEDEF Typedefs
 * @{
 */
/**@brief OTA Service event handler type. */
typedef void (*otas_evt_handler_t)(otas_evt_t *p_evt);

/**@brief OTA Service function type. */
typedef  void (*function)(void);
/** @} */

/**
 * @defgroup OTAS_STRUCT Structures
 * @{
 */
/**@brief OTA Service initialization variable. */
typedef struct
{
    otas_evt_handler_t evt_handler;    /**< Handler to handle otas event. */
} otas_init_t;
/** @} */


/**
 * @defgroup OTAS_FUNCTION Functions
 * @{
 */
/**
 *****************************************************************************************
 * @brief Add an OTA Service instance in the DB
 *
 * @param[in] p_otas_init :Pointer to OTA Service environment variable
 *
 * @return Result of service initialization.
 *****************************************************************************************
 */
sdk_err_t otas_service_init(otas_init_t *p_otas_init);


/**
 *****************************************************************************************
 * @brief Send data to peer device
 *
 * @param[in] conn_idx: Connection index
 * @param[in] p_data:   The Pointer of send value
 * @param[in] length:   The Lenth of send value
 *
 * @return Result of notify and indicate value 
 *****************************************************************************************
 */
sdk_err_t otas_notify_tx_data(uint8_t conn_idx, uint8_t *p_data,uint16_t length);
/** @} */

#endif
/** @} */
/** @} */
