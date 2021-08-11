/**
 ****************************************************************************************
 *
 * @file    app_drv_config.h
 * @author  BLE Driver Team
 * @brief   Header file of app driver config code.
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

/** @defgroup APP_DRIVER_CONFIG DRIVER CONFIG
  * @brief APP DRIVER CONFIG
  * @{
  */


#ifndef _APP_DRV_CONFIG_H_
#define _APP_DRV_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup APP_DRV_PERIPHERAL_PRIORITY_DEFINE Defines
 * @{
 */
/**@brief APP driver peripheral priority define. */

#ifndef APP_DRIVER_ADC_WAPEUP_PRIORITY
#define APP_DRIVER_ADC_WAPEUP_PRIORITY              WAPEUP_PRIORITY_HIGH
#endif

#ifndef APP_DRIVER_AES_WAPEUP_PRIORITY
#define APP_DRIVER_AES_WAPEUP_PRIORITY              WAPEUP_PRIORITY_MID
#endif

#ifndef APP_DRIVER_COMP_WAPEUP_PRIORITY
#define APP_DRIVER_COMP_WAPEUP_PRIORITY             WAPEUP_PRIORITY_LOW
#endif

#ifndef APP_DRIVER_DUAL_TIM_WAPEUP_PRIORITY
#define APP_DRIVER_DUAL_TIM_WAPEUP_PRIORITY         WAPEUP_PRIORITY_MID
#endif

#ifndef APP_DRIVER_DMA_WAPEUP_PRIORITY
#define APP_DRIVER_DMA_WAPEUP_PRIORITY              WAPEUP_PRIORITY_HIGH
#endif

#ifndef APP_DRIVER_GPIOTE_WAPEUP_PRIORITY
#define APP_DRIVER_GPIOTE_WAPEUP_PRIORITY           WAPEUP_PRIORITY_LOW
#endif

#ifndef APP_DRIVER_SYSTICK_WAPEUP_PRIORITY
#define APP_DRIVER_SYSTICK_WAPEUP_PRIORITY          WAPEUP_PRIORITY_HIGH
#endif

#ifndef APP_DRIVER_UART_WAPEUP_PRIORITY
#define APP_DRIVER_UART_WAPEUP_PRIORITY             WAPEUP_PRIORITY_HIGH
#endif

#ifndef APP_DRIVER_HMAC_WAPEUP_PRIORITY
#define APP_DRIVER_HMAC_WAPEUP_PRIORITY             WAPEUP_PRIORITY_MID
#endif

#ifndef APP_DRIVER_I2C_WAPEUP_PRIORITY
#define APP_DRIVER_I2C_WAPEUP_PRIORITY              WAPEUP_PRIORITY_HIGH
#endif

#ifndef APP_DRIVER_I2S_WAPEUP_PRIORITY
#define APP_DRIVER_I2S_WAPEUP_PRIORITY              WAPEUP_PRIORITY_HIGH
#endif

#ifndef APP_DRIVER_QSPI_WAPEUP_PRIORITY
#define APP_DRIVER_QSPI_WAPEUP_PRIORITY             WAPEUP_PRIORITY_HIGH
#endif

#ifndef APP_DRIVER_RNG_WAPEUP_PRIORITY
#define APP_DRIVER_RNG_WAPEUP_PRIORITY              WAPEUP_PRIORITY_MID
#endif

#ifndef APP_DRIVER_SPI_WAPEUP_PRIORITY
#define APP_DRIVER_SPI_WAPEUP_PRIORITY              WAPEUP_PRIORITY_HIGH
#endif

#ifndef APP_DRIVER_TIM_WAPEUP_PRIORITY
#define APP_DRIVER_TIM_WAPEUP_PRIORITY              WAPEUP_PRIORITY_MID
#endif

#ifndef APP_DRIVER_PWM_WAPEUP_PRIORITY
#define APP_DRIVER_PWM_WAPEUP_PRIORITY              WAPEUP_PRIORITY_MID
#endif

/**@} */


/**@addtogroup APP_DRV_WAPEUP_PRIORITY_ENUM Enumerations 
 * @{ 
 */
/**@brief APP driver peripheral wakeup priority define. */
typedef enum
{
    WAPEUP_PRIORITY_LOW = 1,          /**< Wakeup priority low */
    WAPEUP_PRIORITY_MID,              /**< Wakeup priority mid */
    WAPEUP_PRIORITY_HIGH              /**< Wakeup priority high */
} wakeup_priority_t;
/** @} */

#ifndef APP_DRIVER_WAKEUP_CALL_FUN
//#define APP_DRIVER_WAKEUP_CALL_FUN
#endif


#ifdef __cplusplus
}
#endif

#endif

/** @} */
/** @} */
/** @} */

