/**
 ****************************************************************************************
 *
 * @file    app_hmac.h
 * @author  BLE Driver Team
 * @brief   Header file containing functions prototypes of HMAC app library.
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

/** @defgroup APP_HMAC HMAC
  * @brief HMAC APP module driver.
  * @{
  */


#ifndef _APP_HMAC_H_
#define _APP_HMAC_H_

#include "gr55xx_hal.h"
#include "app_drv_error.h"
#ifdef ENV_USE_FREERTOS
#include "app_rtos_cfg.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAL_HMAC_MODULE_ENABLED

/** @addtogroup APP_HMAC_ENUM Enumerations
  * @{
  */

/**
  * @brief HMAC operating mode Enumerations definition
  */
typedef enum
{
    APP_HMAC_TYPE_INTERRUPT,          /**< Interrupt operation mode */
    APP_HMAC_TYPE_POLLING,            /**< Polling operation mode   */
    APP_HMAC_TYPE_DMA,                /**< DMA operation mode       */
    APP_HMAC_TYPE_MAX                 /**< Only for check parameter, not used as input parameters. */
} app_hmac_type_t;

/**
  * @brief HMAC event Enumerations definition
  */
typedef enum
{
    APP_HMAC_EVT_ERROR,              /**< Error reported by HMAC peripheral. */
    APP_HMAC_EVT_DONE                /**< HMAC operation completed.          */
} app_hmac_evt_type_t;
/** @} */

/** @addtogroup APP_HMAC_STRUCTURES Structures
  * @{
  */
/**
  * @brief HMAC parameters structure definition
  */
typedef struct 
{
    app_hmac_type_t      use_type;   /**< Specifies the operation mode of I2C. */
    hmac_init_t          init;       /**< HMAC operation parameters            */
} app_hmac_params_t;

/**
  * @brief HMAC event structure definition
  */
typedef struct
{
    app_hmac_evt_type_t type;       /**< Type of event.    */
    uint32_t error_code;            /**< HMAC Error code . */    
} app_hmac_evt_t;

/**
  * @brief HMAC event callback definition
  */
typedef void (*app_hmac_evt_handler_t)(app_hmac_evt_t *p_evt);

/** @} */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup HAL_APP_HMAC_DRIVER_FUNCTIONS Functions
  * @{
  */
/**
 ****************************************************************************************
 * @brief  Initialize the APP HMAC DRIVER according to the specified parameters
 *         in the app_hmac_params_t and app_hmac_evt_handler_t.
 * @note   If interrupt mode is set, you can use blocking mode. Conversely, if blocking mode
 *         is set, you can't use interrupt mode.
 *
 * @param[in]  p_params: Pointer to app_hmac_params_t parameter which contains the
 *                       configuration information for the specified HMAC module.
 * @param[in]  evt_handler: HMAC user callback function.
 *
 * @return Result of initialization.
 ****************************************************************************************
 */
uint16_t app_hmac_init(app_hmac_params_t *p_params, app_hmac_evt_handler_t evt_handler);

/**
 ****************************************************************************************
 * @brief  De-initialize the APP HMAC DRIVER peripheral.
 *
 * @return Result of De-initialization.
 ****************************************************************************************
 */
uint16_t app_hmac_deinit(void);

/**
 ****************************************************************************************
 * @brief  Update p_user_hash parameters.
 *
 * @param[in]  p_params: Pointer to p_user_hash.
 *
 * @return Result of initialization.
 ****************************************************************************************
 */
uint16_t app_hmac_user_hash(uint32_t *p_user_hash);

/**
 ****************************************************************************************
 * @brief  xxx in blocking mode in SHA256 mode.
 *
 * @param[in]  p_message: Pointer to message buffer
 * @param[in]  number: Amount of data
 * @param[out] p_digest: Pointer to digest buffer
 * @param[in]  timeout: Timeout duration
 *
 * @return Result of initialization.
 ****************************************************************************************
 */
uint16_t app_hmac_sha256_sync(uint32_t *p_message, uint32_t number, uint32_t *p_digest, uint32_t timeout);

/**
 ****************************************************************************************
 * @brief  xxx in non-blocking mode with interrupt in SHA256 mode.
 *
 * @param[in]  p_message: Pointer to message buffer
 * @param[in]  number: Amount of data
 * @param[out] p_digest: Pointer to digest buffer
 *
 * @return Result of initialization.
 ****************************************************************************************
 */
uint16_t app_hmac_sha256_async(uint32_t *p_message, uint32_t number, uint32_t *p_digest);

/**
 ****************************************************************************************
 * @brief  xxx in blocking mode in HMAC mode.
 *
 * @param[in]  p_message: Pointer to message buffer
 * @param[in]  number: Amount of data
 * @param[in]  timeout: Timeout duration
 *
 * @return Result of initialization.
 ****************************************************************************************
 */
uint16_t app_hmac_sha256_hmac_start_sync(uint32_t *p_message, uint32_t number, uint32_t timeout);

/**
 ****************************************************************************************
 * @brief  xxx in blocking mode in HMAC mode.
 *
 * @param[in]  p_message: Pointer to message buffer
 * @param[in]  number: Amount of data
 * @param[in]  timeout: Timeout duration
 *
 * @return Result of initialization.
 ****************************************************************************************
 */
uint16_t app_hmac_sha256_hmac_continue_sync(uint32_t *p_message, uint32_t number, uint32_t timeout);

/**
 ****************************************************************************************
 * @brief  xxx in blocking mode in HMAC mode.
 *
 * @param[in]  p_message: Pointer to message buffer
 * @param[in]  number: Amount of data
 * @param[out] digest: Pointer to digest buffer
 * @param[in]  timeout: Timeout duration
 *
 * @return Result of initialization.
 ****************************************************************************************
 */
uint16_t app_hmac_sha256_hmac_finish_sync(uint32_t *p_message, uint32_t number, uint32_t *digest, uint32_t timeout);

/**
 ****************************************************************************************
 * @brief  xxx in non-blocking mode with interrupt in SHA mode.
 *
 * @param[in]  p_message: Pointer to message buffer
 * @param[in]  number: Amount of data
 *
 * @return Result of initialization.
 ****************************************************************************************
 */
uint16_t app_hmac_sha256_hmac_start_async(uint32_t *p_message, uint32_t number);

/**
 ****************************************************************************************
 * @brief  xxx in non-blocking mode with interrupt in SHA mode.
 *
 * @param[in]  p_message: Pointer to message buffer
 * @param[in]  number: Amount of data
 *
 * @return Result of initialization.
 ****************************************************************************************
 */
uint16_t app_hmac_sha256_hmac_continue_async(uint32_t *p_message, uint32_t number);

/**
 ****************************************************************************************
 * @brief  xxx in non-blocking mode with interrupt in SHA mode.
 *
 * @param[in]  p_message: Pointer to message buffer
 * @param[in]  number: Amount of data
 * @param[out] digest: Pointer to digest buffer
 *
 * @return Result of initialization.
 ****************************************************************************************
 */
uint16_t app_hmac_sha256_hmac_finish_async(uint32_t *p_message, uint32_t number, uint32_t *digest);

/**
 ****************************************************************************************
 * @brief  Return the hmac handle.
 *
 * @return Pointer to the hmac handle.
 ****************************************************************************************
 */
hmac_handle_t *app_hmac_get_handle(void);

#ifdef  ENV_RTOS_USE_SEMP
/**
 ****************************************************************************************
 * @brief  [RTOS] xxx in blocking mode in SHA256 mode.
 *
 * @param[in]  p_message: Pointer to message buffer
 * @param[in]  number: Amount of data
 * @param[out] p_digest: Pointer to digest buffer
 *
 * @return Result of initialization.
 ****************************************************************************************
 */
uint16_t app_hmac_sha256_sem_sync(uint32_t *p_message, uint32_t number, uint32_t *p_digest);
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
