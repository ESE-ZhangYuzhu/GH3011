/**
 ****************************************************************************************
 *
 * @file    app_adc.h
 * @author  BLE Driver Team
 * @brief   Header file containing functions prototypes of ADC app library.
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

/** @defgroup APP_ADC ADC
  * @brief ADC APP module driver.
  * @{
  */


#ifndef _APP_ADC_H_
#define _APP_ADC_H_

#include "gr55xx_hal.h"
#include "app_io.h"
#include "app_drv_error.h"
#ifdef ENV_USE_FREERTOS
#include "app_rtos_cfg.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAL_ADC_MODULE_ENABLED

/** @addtogroup APP_ADC_ENUM Enumerations
  * @{
  */

/**
  * @brief ADC operating mode Enumerations definition
  */
typedef enum
{
    APP_ADC_TYPE_POLLING,             /**< Polling operation mode   */
    APP_ADC_TYPE_DMA,                 /**< DMA operation mode       */
    APP_ADC_TYPE_MAX,                 /**< Only for check parameter, not used as input parameters. */
} app_adc_type_t;

/**
  * @brief ADC event Enumerations definition
  */
typedef enum
{
    APP_ADC_EVT_CONV_CPLT,           /**< Conversion completed by ADC peripheral. */
} app_adc_evt_type_t;

/** @} */


/** @addtogroup APP_ADC_STRUCT Structures
  * @{
  */
  
/**
  * @brief ADC pins config Structures
  */
typedef struct
{
   app_io_type_t        type;        /**< Specifies the type of ADC IO. */
   app_io_mux_t         mux;         /**< Specifies the Peripheral to be connected to the selected pins. */
   uint32_t             pin;         /**< Specifies the IO pins to be configured.
                                          This parameter can be any value of @ref GR551x_pins. */
} app_adc_pin_t;

typedef struct
{
    app_adc_pin_t       channel_p;   /**< Set the configuration of ADC pin. */
    app_adc_pin_t       channel_n;   /**< Set the configuration of ADC pin. */
    app_adc_pin_t       extern_ref;  /**< Set the configuration of extern reference pin. */
} app_adc_pin_cfg_t;

/**
  * @brief ADC operate mode Structures
  */
typedef struct
{
    app_adc_type_t      type;        /**< Specifies the operation mode of ADC. */
    dma_channel_t       dma_channel; /**< Specifies the dma channel of ADC. */
} app_adc_mode_t;

/**
  * @brief ADC parameters structure definition
  */
typedef struct
{
    app_adc_pin_cfg_t   pin_cfg;     /**< the pin configuration information for the specified ADC module. */
    app_adc_mode_t      use_mode;    /**< ADC operate mode. */
    adc_init_t          init;        /**< ADC configuration parameters. */
} app_adc_params_t;

/**
  * @brief ADC event structure definition
  */
typedef struct
{
    app_adc_evt_type_t  type; /**< Type of event. */
} app_adc_evt_t;

/**
  * @brief ADC event callback definition
  */
typedef void (*app_adc_evt_handler_t)(app_adc_evt_t *p_evt);

/** @} */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup APP_ADC_DRIVER_FUNCTIONS Functions
  * @{
  */

/**
 ****************************************************************************************
 * @brief  Initialize the APP ADC DRIVER according to the specified parameters
 *         in the app_adc_params_t and app_adc_evt_handler_t.
 * @note   If DMA mode is set, you can use blocking mode. Conversely, if blocking mode
 *         is set, you can't use DMA mode.
 *
 * @param[in]  p_params: Pointer to app_adc_params_t parameter which contains the
 *                       configuration information for the specified ADC module.
 * @param[in]  evt_handler: ADC user callback function.
 *
 * @return Result of initialization.
 ****************************************************************************************
 */
uint16_t app_adc_init(app_adc_params_t *p_params, app_adc_evt_handler_t evt_handler);

/**
 ****************************************************************************************
 * @brief  De-initialize the APP ADC DRIVER peripheral.
 *
 * @return Result of De-initialization.
 ****************************************************************************************
 */
uint16_t app_adc_deinit(void);

/**
 ****************************************************************************************
 * @brief  Polling for conversion.
 *
 * @param[in]  p_data: Pointer to data buffer which to storage ADC conversion results.
 * @param[in]  length: Length of data buffer.
 * @param[in]  timeout : Timeout duration
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_adc_conversion_sync(uint16_t *p_data, uint32_t length, uint32_t timeout);

/**
 ****************************************************************************************
 * @brief  DMA for conversion.
 *
 * @param[in]  p_data: Pointer to data buffer which to storage ADC conversion results.
 * @param[in]  length: Length of data buffer,  ranging between 0 and 4095.
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_adc_conversion_async(uint16_t *p_data, uint32_t length);

/**
 ****************************************************************************************
 * @brief  Convert the ADC conversion results to a voltage value(internal reference).
 *
 * @param[in]  inbuf: Pointer to data buffer which storage ADC conversion results.
 * @param[out] outbuf: Pointer to data buffer which to storage voltage results.
 * @param[in]  length: Length of data buffer,  ranging between 0 and 4095.
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_adc_voltage_intern(uint16_t *inbuf, double *outbuf, uint32_t buflen);

/**
 ****************************************************************************************
 * @brief  Convert the ADC conversion results to a voltage value(external reference).
 *
 * @param[in]  ref: slope of ADC.
 * @param[in]  inbuf: Pointer to data buffer which storage ADC conversion results.
 * @param[out] outbuf: Pointer to data buffer which to storage voltage results.
 * @param[in]  length: Length of data buffer,  ranging between 0 and 4095.
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_adc_voltage_extern(double ref, uint16_t *inbuf, double *outbuf, uint32_t buflen);

/**
 ****************************************************************************************
 * @brief  Return the ADC handle.
 *
 * @return Pointer to the ADC handle.
 ****************************************************************************************
 */
adc_handle_t *app_adc_get_handle(void);


#ifdef ENV_RTOS_USE_SEMP
/**
 ****************************************************************************************
 * @brief [RTOS] Polling for conversion.
 *
 * @param[in]  p_data: Pointer to data buffer which to storage ADC conversion results.
 * @param[in]  length: Length of data buffer.
 *
 * @return Result of operation.
 ****************************************************************************************
 */
uint16_t app_adc_conversion_sem_sync(uint16_t *p_data, uint32_t length);
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


