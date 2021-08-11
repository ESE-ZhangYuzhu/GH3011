/**
 *****************************************************************************************
 *
 * @file wechat.h
 *
 * @brief WeChat Service API.
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

/**
 * @addtogroup BLE_SRV BLE Services
 * @{
 * @brief Definitions and prototypes for the BLE Service interface.
 */
/**
 * @defgroup BLE_SDK_WECHAT Wechat (WECHAT)
 * @{
 * @brief Definitions and prototypes for the WeChat interface.
 *
 * @details The WeChat Service contains two modules: WeChat Airsync Protocol and WeChat
 *          Pedometer Protocol.
 *
 *          WeChat Airsync Protocol opens the data link between the device and the vendor server,
 *          which supports sending data from the device to the vendor's server, as well as the vendor's
 *          data to the device. The protocol also opens the data link between the device and the WeChat server.
 *          The data format between the device and the WeChat server is stipulated by WeChat, such as login,
 *          new message notification, etc.
 *
 *          WeChat Pedometer Protocol is based on GATT protocol, which requires less hardware
 *          capability of the device, and the manufacturer does not need a back-end server (that is,
 *          only need to develop the device) to dock with WeChat. It allows pedometers to connect to WeChat
 *          and transmit steps, kilometers, calories, moving targets, etc.
 */

#ifndef __WECHART_H__
#define __WECHART_H__

#include "gr55xx_sys.h"
#include "custom_config.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @defgroup WECHAT_MACRO Defines
 * @{
 */
#define WECHAT_CONNECTION_MAX               (10 < CFG_MAX_CONNECTIONS ?\
                                             10 : CFG_MAX_CONNECTIONS)   /**< Maximum number of WeChat connections. */
#define WECHAT_DATA_LEN                      20                          /**< Maximum length of WeChat Data. */
#define WECHAT_PEDO_TARGET_VAL_LEN           0x04                        /**< Maximum length of WeChat pedometer target value. */
#define WECHAT_PEDO_STEP_COUNT_MAX           0xFFFFFF                    /**< Maximum value of WeChat pedometer step count. */

/**
 * @defgroup WECHAT_UUID WECHAT Service and Characteristic UUID
 * @{
 * @brief WeChat Service, Airsync and Pedometer Characteristic UUID.
 */
#define WECHAT_SERVICE_UUID                 0XFEE7                       /**< WeChat Service UUID. */
#define WECHAT_WRITE_CHAR_UUID              0XFEC7                       /**< WeChat Airsync Write Characteristic UUID. */
#define WECHAT_INDICATE_CHAR_UUID           0XFEC8                       /**< WeChat Airsync Indication Characteristic UUID. */
#define WECHAT_READ_CHAR_UUID               0XFEC9                       /**< WeChat Airsync Read Characteristic UUID. */
#define WECHAT_PEDOMETER_MEASUREMENT        0XFEA1                       /**< WeChat Current Pedometer Measurement Characteristic UUID. */
#define WECHAT_TARGET                       0XFEA2                       /**< WeChat Pedometer Target Characteristic UUID. */
/** @} */

/**
 * @defgroup WECHAT_PEDO_FLAG WECHAT pedeometer measurement flag
 * @{
 * @brief WeChat pedeometer measurement flag bits.
 */
#define WECHAT_PEDO_FLAG_STEP_COUNT_BIT     0X01                       /**< WeChat pedometer measurement step count flag bit. */
#define WECHAT_PEDO_FLAG_STEP_DISTENCE_BIT  0X02                       /**< WeChat pedometer measurement step distance flag bit. */
#define WECHAT_PEDO_FLAG_STEP_CALORIE_BIT   0X04                       /**< WeChat pedometer measurement step calorie flag bit. */
#define WECHAT_PEDO_FLAG_ALL_SUP_BIT        0X07                       /**< WeChat pedometer measurement all flag bit. */
/** @} */
/** @} */

/**
 * @defgroup WECHAT_ENUM Enumerations
 * @{
 */
/**@brief WeChat Service event type.*/
typedef enum
{
    WECHAT_EVT_INVALID,                    /**< WeChat invalid event. */
    WECHAT_EVT_AIRSYNC_IND_ENABLE,         /**< WeChat Airsync indication has been enabled. */
    WECHAT_EVT_AIRSYNC_IND_DISABLE,        /**< WeChat Airsync indication has been disabled. */
    WECHAT_EVT_PEDO_MEAS_NTF_ENABLE,       /**< WeChat Pedometer measurement notification has been enabled. */
    WECHAT_EVT_PEDO_MEAS_NTF_DISABLE,      /**< WeChat Pedometer measurement notification has been disabled. */
    WECHAT_EVT_PEDO_TARGET_IND_ENABLE,     /**< WeChat Pedometer target indicaiton has been enabled. */
    WECHAT_EVT_PEDO_TARGET_IND_DISABLE,    /**< WeChat Pedometer target indicaiton has been disabled. */
    WECHAT_EVT_PEDO_TARGET_UPDATE,         /**< WeChat Pedometer target has been updated. */
    WECHAT_EVT_AIRSYNC_DATA_RECIEVE,       /**< Recieved Airsync data. */
} wechat_evt_type_t;
/** @} */

/**
 * @defgroup WECHAT_STRUCT Structures
 * @{
 */
/**@brief WeChat current pedometer measurement variable. */
typedef struct
{
    uint8_t flag;                 /**< Flag for WeChat current pedometer measurement. */
    uint8_t step_count[3];        /**< Step counts of pedometer measurement. */
    uint8_t step_dist[3];         /**< Step distance of pedometer measurement. */
    uint8_t step_calorie[3];      /**< Step Calorie of pedometer measurement. */
} wechat_pedo_meas_t;

/**@brief WeChat pedometer target variable. */
typedef struct
{
    uint8_t flag;                /**< Flag for WeChat pedometer target. */
    uint8_t step_count[3];       /**< Target of step pedometer counts. */
} wechat_pedo_target_t;

/**@brief WeChat service data. */
typedef struct 
{
    const uint8_t   *p_data;     /**< Pointer to data. */
    uint16_t         length;     /**< Length of data. */
    uint16_t         offset;     /**< Offset of data. */
} wechat_data_t;

/**@brief WeChat Service event.*/
typedef struct
{
    uint8_t            conn_idx;              /**< The index of connection. */
    wechat_evt_type_t  evt_type;              /**< Event type. */
    union
    {
        wechat_pedo_target_t  pedo_target;    /**< Pedometer target set value. */
        wechat_data_t         data;           /**< Data of Airsync. */
    } param;                                  /**< Parameter of WeChat airsync event. */
} wechat_evt_t;
/** @} */

/**
 * @defgroup WECHAT_TYPEDEF Typedefs
 * @{
 */
/**@brief WeChat Service event handler type.*/
typedef void (*wechat_evt_handler_t)(wechat_evt_t *p_evt);
/** @} */

/**
 * @defgroup WECHAT_STRUCT Structures
 * @{
 */
/**@brief WeChat Service Init variable. */
typedef struct
{
    wechat_evt_handler_t  evt_handler;            /**< WeChat Service event handler. */
    uint32_t              step_count_target;      /**< WeChat pedometer step count target value. */
    uint8_t              *p_dev_mac;              /**< Pointer to WeChat device MAC address. */
} wechat_init_t;
/** @} */

/**
 * @defgroup WECHAT_FUNCTION Functions
 * @{
 */
/**
 *****************************************************************************************
 * @brief Initialize a WeChat Service instance and add in the DB.
 *
 * @param[in] p_wechat_init: Pointer to WeChat init value.
 *
 * @return Result of service initialization.
 *****************************************************************************************
 */
sdk_err_t wechat_service_init(wechat_init_t *p_wechat_init);

/**
 *****************************************************************************************
 * @brief WeChat Service Airsync indicate data.
 *
 * @param[in] conn_idx: The index of connection.
 * @param[in] p_data:   Pointer to data.
 * @param[in] length:   Length of data.
 *
 * @return Result of indicaition.
 *****************************************************************************************
 */
sdk_err_t wechat_airsync_data_indicate(uint8_t conn_idx, uint8_t *p_data, uint16_t length);

/**
 *****************************************************************************************
 * @brief Send WeChat pedometer measurement information.
 *
 * @param[in] conn_idx: The index of connection.
 * @param[in] p_pedo_meas: Pointer to pedometer measurement.
 *
 * @return Result of send.
 *****************************************************************************************
 */
sdk_err_t wechat_pedo_measurement_send(uint8_t conn_idx, wechat_pedo_meas_t *p_pedo_meas);

/**
 *****************************************************************************************
 * @brief Send WeChat pedometer target value.
 *
 * @param[in] conn_idx: Connection index.
 *
 * @return Result of send.
 *****************************************************************************************
 */
sdk_err_t wechat_pedo_target_send(uint8_t conn_idx);
/** @} */
#endif
/** @} */
/** @} */
