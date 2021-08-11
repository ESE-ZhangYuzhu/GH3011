/**
 ****************************************************************************************
 *
 * @file    app_systick.h
 * @author  BLE Driver Team
 * @brief   Header file containing functions prototypes of systick app library.
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

/** @defgroup APP_SYSTICK SYSTICK
  * @brief SYSTICK APP module driver.
  * @{
  */


#ifndef _APP_SYSTICK_H_
#define _APP_SYSTICK_H_

#include <stdint.h>

/* Exported functions --------------------------------------------------------*/
/** @addtogroup APP_SYSTICK_DRIVER_FUNCTIONS Functions
  * @{
  */

/**
 ****************************************************************************************
 * @brief  This function configures time base source, NVIC and Low level hardware.
 *
 * @note   This function is called at the beginning of program after reset and before
 *         the clock configuration.
 *         The Systick configuration is based on AHB clock and the NVIC configuration
 *         is set to Priority group 4.
 *         The time base configuration is done, time base tick starts incrementing.
 *         In the default implementation, Systick is used as source of time base.
 *         The tick variable is incremented each 1ms in its ISR.
 *
 ****************************************************************************************
 */
void app_systick_init(void);

/**
 ****************************************************************************************
 * @brief  This function de-Initializes common part of the HAL and stops the source
 *         of time base.
 *
 * @note   This function is optional.
 *
 ****************************************************************************************
 */
void app_systick_deinit(void);

/** @} */

#endif

/** @} */
/** @} */
/** @} */
