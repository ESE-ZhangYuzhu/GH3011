/**
 ****************************************************************************************
 *
 * @file ble_gatt.h
 *
 * @brief BLE GATT
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
 * @addtogroup BLE
 * @{
 */
 
  /**
 * @addtogroup BLE_GATT Generic Attribute Profile (GATT)
 * @{
 * @brief Definitions and prototypes for the GATT interface.
 */

/**
  @addtogroup BLE_GATTS Generic Attribute Profile (GATT) Common
  @{
  @brief  Definitions and prototypes for the GATT Common interface.
 */
#ifndef __BLE_GATT_H__
#define __BLE_GATT_H__

#include "ble_error.h"
#include <stdint.h>

/** @addtogroup BLE_GATT_COMMON Enumerations
 * @{ */

/**
 * @brief GATT common events.
 */
typedef enum
{
    BLE_GATT_NOTIFICATION = 0x00,           /**< Handle Value Notification. */
    BLE_GATT_INDICATION,                    /**< Handle Value Indication. */
} gatt_evt_type_t;
/** @} */

/** @addtogroup BLE_GATT_COMMON_STRUCTURES Structures
 * @{ */

/**
 * @brief GATT UUID structure.
 */
typedef struct
{
    uint8_t  uuid_len;       /**< UUID length. */
    uint8_t *uuid;           /**< UUID value. */
} ble_uuid_t;

/**
 * @brief GATT common callback function description.
 */
typedef struct
{
    void (*app_gatt_mtu_exchange_cb)(uint8_t conn_idx, uint8_t status, uint16_t mtu);       /**< Exchange MTU callback function. */
    void (*app_gatt_prf_register_cb)(uint8_t status, uint8_t prf_index);                    /**< Profile register callback function.
                                                                                                 @note prf_index range is from 0 to profile count - 1. prf_index is current profile index. */
} gatt_common_cb_fun_t;

/** @} */

/** @addtogroup BLE_GATT_FUNCTIONS Functions
 * @{ */

/**
 ****************************************************************************************
 * @brief Set ATT_MTU size.
 * 
 * @param[in]  mtu:        ATT_MTU size.
 * 
 * @note This function should be called before exchange MTU operation. This MTU size is used to all connections.
 *       If not set these parameters, the stack will config the default value as (max_mtu = 512).
 *
 * @retval ::SDK_SUCCESS: Successfully get ATT_MTU.
 * @retval ::SDK_ERR_DISALLOWED: Operation is disallowed.
 ****************************************************************************************
 */
uint16_t ble_gatt_mtu_set(uint16_t mtu);

/**
 ****************************************************************************************
 * @brief Get the current ATT_MTU size for a given connection.
 *
 * @param[in]    conn_idx:     Current connection index.
 * @param[out]   p_mtu:        ATT_MTU size for the given connection.
 *
 * @retval ::SDK_SUCCESS: Successfully get ATT_MTU.
 * @retval ::SDK_ERR_INVALID_PARAM: Invalid parameter(s) supplied.
 ****************************************************************************************
 */
uint16_t ble_gatt_mtu_get(uint8_t conn_idx, uint16_t *p_mtu);

/** @} */

#endif
/** @} */
/** @} */
/** @} */

