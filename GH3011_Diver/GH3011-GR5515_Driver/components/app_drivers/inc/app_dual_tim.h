/**
 ****************************************************************************************
 *
 * @file    app_dual_tim.h
 * @author  BLE Driver Team
 * @brief   Header file containing functions prototypes of DAUL TIM app library.
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

/** @defgroup APP_DUAL_TIMER DUAL TIMER
  * @brief DUAL TIMER APP module driver.
  * @{
  */


#ifndef _APP_DUAL_TIM_H_
#define _APP_DUAL_TIM_H_

#include "gr55xx_hal.h"
#include "app_drv_error.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAL_DUAL_TIMER_MODULE_ENABLED

/** @addtogroup APP_TIM_ENUM Enumerations
  * @{
  */

/**
  * @brief APP_DUAL_TIM module Enumerations definition
  */
typedef enum
{
    APP_DUAL_TIM_ID_0,     /**< DUAL TIMER module 0 */
    APP_DUAL_TIM_ID_1,     /**< DUAL TIMER module 1 */
    APP_DUAL_TIM_ID_MAX    /**< Only for check parameter, not used as input parameters. */
} app_dual_tim_id_t;

/**
  * @brief DUAL_TIM event Enumerations definition
  */
typedef enum
{
    APP_DUAL_TIM_EVT_ERROR,    /**< Error reported by DUAL TIMER peripheral. */
    APP_DUAL_TIM_EVT_DONE      /**< Interrupt done by DUAL TIMER peripheral. */
} app_dual_tim_evt_t;
/** @} */

/** @addtogroup APP_DUAL_TIM_STRUCTURES Structures
  * @{
  */

/**
  * @brief DUAL_TIM parameters structure definition
  */
typedef struct 
{
    app_dual_tim_id_t   id;    /**< specified DUAL TIMER module ID.    */
    dual_timer_init_t   init;  /**< DUAL_TIM Base required parameters. */
} app_dual_tim_params_t;

/**
  * @brief APP_DUAL_TIM event callback definition
  */
typedef void (*app_dual_tim_evt_handler_t)(app_dual_tim_evt_t *p_evt);

/** @} */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup APP_DUAL_TIM_DRIVER_FUNCTIONS Functions
  * @{
  */
/**
 ****************************************************************************************
 * @brief  Initialize the APP DUAL TIM DRIVER according to the specified parameters
 *         in the app_dual_tim_params_t and app_dual_tim_evt_handler_t.
 *
 * @param[in]  p_params: Pointer to app_dual_tim_params_t parameter which contains the
 *                       configuration information for the specified DUAL TIM module.
 * @param[in]  evt_handler: DUAL TIM user callback function.
 *
 * @return Result of initialization.
 ****************************************************************************************
 */
uint16_t app_dual_tim_init(app_dual_tim_params_t *p_params, app_dual_tim_evt_handler_t evt_handler);

/**
 ****************************************************************************************
 * @brief  De-initialize the APP DUAL TIM DRIVER peripheral.
 *
 * @param[in]  id: De-initialize for a specific ID.
 *
 * @return Result of De-initialization.
 ****************************************************************************************
 */
uint16_t app_dual_tim_deinit(app_dual_tim_id_t id);

/**
 ****************************************************************************************
 * @brief  Starts the DUAL TIM counter in interrupt mode.
 * @param[in]  id: which DUAL TIM module want to start.
 *
 * @return Result of initialization.
 *
 ****************************************************************************************
 */
uint16_t app_dual_tim_start(app_dual_tim_id_t id);

/**
 ****************************************************************************************
 * @brief  Stops the DUAL TIM counter in interrupt mode.
 * @param[in]  id: which DUAL TIM module want to stop.
 *
 * @return Result of initialization.
 *
 ****************************************************************************************
 */
uint16_t app_dual_tim_stop(app_dual_tim_id_t id);

/**
 ****************************************************************************************
 * @brief  Return the DUAL TIM handle.
 *
 * @param[in]  id: DUAL TIM Channel ID.
 *
 * @return Pointer to the specified ID's DUAL TIM handle.
 ****************************************************************************************
 */

dual_timer_handle_t *app_dual_tim_get_handle(app_dual_tim_id_t id);

/** @} */

#endif

#ifdef __cplusplus
}
#endif

#endif

/** @} */
/** @} */
/** @} */
