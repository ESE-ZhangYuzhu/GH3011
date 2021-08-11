/**
 ****************************************************************************************
 *
 * @file    app_pkc.h
 * @author  BLE Driver Team
 * @brief   Header file containing functions prototypes of PKC app library.
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

/** @defgroup APP_PKC PKC
  * @brief PKC APP module driver.
  * @{
  */


#ifndef _APP_PKC_H_
#define _APP_PKC_H_

#include "gr55xx_hal.h"
#include "app_drv_error.h"
#ifdef ENV_USE_FREERTOS
#include "app_rtos_cfg.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAL_PKC_MODULE_ENABLED

/** @addtogroup APP_PKC_ENUM Enumerations
  * @{
  */

/**
  * @brief PKC operating mode Enumerations definition
  */
typedef enum
{
    APP_PKC_TYPE_INTERRUPT,          /**< Interrupt operation mode. */
    APP_PKC_TYPE_POLLING,            /**< Polling operation mode.   */
    APP_PKC_TYPE_MAX                 /**< Only for check parameter, not used as input parameters. */
} app_pkc_type_t;


/**
  * @brief PKC event Enumerations definition
  */
typedef enum
{
    APP_PKC_EVT_ERROR,               /**< Error reported by PKC peripheral. */
    APP_PKC_EVT_DONE                 /**< Encryption and decryption completed. */
} app_pkc_evt_type_t;
/** @} */


/** @addtogroup APP_PKC_STRUCT Structures
  * @{
  */

/**
  * @brief PKC parameters structure definition
  */
typedef struct 
{
    app_pkc_type_t      use_type;    /**< Specifies the operation mode of PKC. */
    pkc_init_t          init;        /**< PKC operation parameters         */
    void                *p_result;   /**< Pointer to PKC result Buffer     */
	uint32_t            *p_kout;	 /**< Pointer to Kout result in montgomery inversion */
} app_pkc_params_t;

/**
  * @brief PKC event structure definition
  */
typedef struct
{
    app_pkc_evt_type_t type;         /**< Type of event. */
    uint32_t error_code;
} app_pkc_evt_t;

/**
  * @brief PKC
  */
typedef pkc_ecc_point_multi_t 	app_pkc_ecc_point_multi_t;
typedef pkc_modular_add_t 		app_pkc_modular_add_t;
typedef pkc_modular_sub_t       app_pkc_modular_sub_t;
typedef pkc_modular_shift_t     app_pkc_modular_shift_t;
typedef pkc_modular_compare_t   app_pkc_modular_compare_t;
typedef pkc_montgomery_multi_t  app_pkc_montgomery_multi_t;
typedef pkc_montgomery_inversion_t  app_pkc_montgomery_inversion_t;
typedef pkc_big_number_multi_t  app_pkc_big_number_multi_t;
typedef pkc_big_number_add_t    app_pkc_big_number_add_t;

/**
  * @brief PKC event callback definition
  */
typedef void (*app_pkc_evt_handler_t)(app_pkc_evt_t *p_evt);

/** @} */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup APP_PKC_DRIVER_FUNCTIONS Functions
  * @{
  */

/**
 ****************************************************************************************
 * @brief  Initialize the APP PKC DRIVER according to the specified parameters
 *         in the app_pkc_params_t and app_pkc_evt_handler_t.
 * @note   If interrupt mode is set, you can use blocking mode. Conversely, if blocking mode
 *         is set, you can't use interrupt mode.
 *
 * @param[in]  p_params: Pointer to app_pkc_params_t parameter which contains the
 *                       configuration information for the specified PKC module.
 * @param[in]  evt_handler: PKC user callback function.
 *
 * @return Result of initialization.
 ****************************************************************************************
 */
uint16_t app_pkc_init(app_pkc_params_t *p_params, app_pkc_evt_handler_t evt_handler);

/**
 ****************************************************************************************
 * @brief  De-initialize the APP PKC DRIVER peripheral.
 *
 * @return Result of De-initialization.
 ****************************************************************************************
 */
uint16_t app_pkc_deinit(void);
/**
 ****************************************************************************************
 * @brief  Realize the point multiplication operation of ECC ellipse algorithm，: result = k * point，Polling mode
 *
 * @param[in] p_input: pointer to the input parameter pkc_ecc_point_multi_t structure variable.
 * @param[in] timeout: Timeout duration
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_pkc_ecc_point_multi_sync(app_pkc_ecc_point_multi_t *p_input, uint32_t timeout);
/**
 ****************************************************************************************
 * @brief  Realize the point multiplication operation of ECC ellipse algorithm，: result = k * point，Interrupt mode
 *
 * @param[in] p_input: pointer to the input parameter pkc_ecc_point_multi_t structure variable.
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_pkc_ecc_point_multi_async(app_pkc_ecc_point_multi_t *p_input);
/**
 ****************************************************************************************
 * @brief  Realization of modular addition: result = (A + B) mod P，Polling mode
 *
 * @param[in] p_input: pointer to the input parameter pkc_modular_add_t structure variable.
 * @param[in] timeout: Timeout duration
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_pkc_modular_add_sync(app_pkc_modular_add_t *p_input, uint32_t timeout);
/**
 ****************************************************************************************
 * @brief  Realization of modular addition: result = (A + B) mod P，Interrupt mode
 *
 * @param[in] p_input: pointer to the input parameter pkc_modular_add_t structure variable.
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_pkc_modular_add_async(app_pkc_modular_add_t *p_input);
/**
 ****************************************************************************************
 * @brief  Implementation of modular subtraction: result = (A - B) mod P，Polling mode
 *
 * @param[in] p_input: pointer to the input parameter pkc_modular_sub_t structure variable.
 * @param[in] timeout: Timeout duration
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_pkc_modular_sub_sync(app_pkc_modular_sub_t *p_input, uint32_t timeout);
/**
 ****************************************************************************************
 * @brief  Implementation of modular subtraction: result = (A - B) mod P，Interrupt mode
 *
 * @param[in] p_input: pointer to the input parameter pkc_modular_sub_t structure variable.
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_pkc_modular_sub_async(app_pkc_modular_sub_t *p_input);
/**
 ****************************************************************************************
 * @brief  Realization of module shift left operation: result = (A<<ShiftBits) mod P，Polling mode
 *
 * @param[in] p_input: pointer to the input parameter pkc_modular_shift_t structure variable.
 * @param[in] timeout: Timeout duration
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_pkc_modular_left_shift_sync(app_pkc_modular_shift_t *p_input, uint32_t timeout);
/**
 ****************************************************************************************
 * @brief  Realization of module shift left operation: result = (A<<ShiftBits) mod P，Interrupt mode 
 *
 * @param[in] p_input: pointer to the input parameter pkc_modular_shift_t structure variable.
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_pkc_modular_left_shift_async(app_pkc_modular_shift_t *p_input);
/**
 ****************************************************************************************
 * @brief  Realization of modular comparison operation: result = A mod P，Polling mode
 *
 * @param[in] p_input: pointer to the input parameter pkc_modular_compare_t structure variable.
 * @param[in] timeout: Timeout duration
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_pkc_modular_compare_sync(app_pkc_modular_compare_t *p_input, uint32_t timeout);
/**
 ****************************************************************************************
 * @brief  Realization of modular comparison operation: result = A mod P，Interrupt mode 
 *
 * @param[in] p_input: pointer to the input parameter pkc_modular_compare_t structure variable.
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_pkc_modular_compare_async(app_pkc_modular_compare_t *p_input);
/**
 ****************************************************************************************
 * @brief  Realization of modular multiplication: result = A*B mod P，Polling mode
 *
 * @param[in] p_input: pointer to the input parameter pkc_montgomery_multi_t structure variable.
 * @param[in] timeout: Timeout duration
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_pkc_montgomery_multi_sync(app_pkc_montgomery_multi_t *p_input, uint32_t timeout);
/**
 ****************************************************************************************
 * @brief  Realization of modular multiplication: result = A*B mod P，Interrupt mode
 *
 * @param[in] p_input: pointer to the input parameter pkc_montgomery_multi_t structure variable.
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_pkc_montgomery_multi_async(app_pkc_montgomery_multi_t *p_input);
/**
 ****************************************************************************************
 * @brief  Realization of modular inversion: result = A^(-1)mod P，Polling mode
 *
 * @param[in] p_input: pointer to the input parameter pkc_montgomery_inversion_t structure variable.
 * @param[in] p_K:Pointer to output parameter K
 * @param[in]  timeout: Timeout duration
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_pkc_montgomery_inversion_sync(app_pkc_montgomery_inversion_t *p_input, uint32_t *p_K, uint32_t timeout);
/**
 ****************************************************************************************
 * @brief  Realization of modular inversion: result = A^(-1)mod P，Interrupt mode
 *
 * @param[in] p_input: pointer to the input parameter pkc_montgomery_inversion_t structure variable.
 * @param[in] p_K:Pointer to output parameter K
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_pkc_montgomery_inversion_async(app_pkc_montgomery_inversion_t *p_input, uint32_t *p_K);
/**
 ****************************************************************************************
 * @brief  Realize multiplication of large numbers: result = A*B，Polling mode
 *
 * @param[in] p_input: pointer to the input parameter pkc_big_number_multi_t structure variable.
 * @param[in] timeout: Timeout duration
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_pkc_big_number_multi_sync(app_pkc_big_number_multi_t *p_input, uint32_t timeout);
/**
 ****************************************************************************************
 * @brief  Realize multiplication of large numbers: result = A*B，Interrupt mode
 *
 * @param[in] p_input: pointer to the input parameter pkc_big_number_multi_t structure variable.
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_pkc_big_number_multi_async(app_pkc_big_number_multi_t *p_input);
/**
 ****************************************************************************************
 * @brief  Realize addition of large numbers: result = A+B，Polling mode
 *
 * @param[in] p_input: pointer to the input parameter pkc_big_number_add_t structure variable.
 * @param[in] timeout: Timeout duration
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_pkc_big_number_add_sync(app_pkc_big_number_add_t *p_input, uint32_t timeout);
/**
 ****************************************************************************************
 * @brief  Realize addition of large numbers: result = A+B，Interrupt mode
 *
 * @param[in] p_input: pointer to the input parameter pkc_big_number_add_t structure variable.
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_pkc_big_number_add_async(app_pkc_big_number_add_t *p_input);

/**
 ****************************************************************************************
 * @brief  Return the PKC handle.
 *
 * @return Pointer to the PKC handle.
 ****************************************************************************************
 */
pkc_handle_t *app_pkc_get_handle(void);

#ifdef  ENV_RTOS_USE_SEMP
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
