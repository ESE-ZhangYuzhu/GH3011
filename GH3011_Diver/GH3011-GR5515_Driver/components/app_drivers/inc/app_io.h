/**
 ****************************************************************************************
 *
 * @file    app_io.h
 * @author  BLE Driver Team
 * @brief   Header file containing functions prototypes of GPIO app library.
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

/** @defgroup APP_GPIO GPIO
  * @brief GPIO APP module driver.
  * @{
  */


#ifndef _APP_IO_H_
#define _APP_IO_H_

#include "app_drv_error.h"
#include <stdint.h>

/** @addtogroup APP_GPIO_DEFINE Defines
  * @{
  */

/**
* @brief GR551x_pins IO pins
*/
#define APP_IO_PIN_0                 ((uint32_t)0x00000001U)  /**< Pin 0 selected    */
#define APP_IO_PIN_1                 ((uint32_t)0x00000002U)  /**< Pin 1 selected    */
#define APP_IO_PIN_2                 ((uint32_t)0x00000004U)  /**< Pin 2 selected    */
#define APP_IO_PIN_3                 ((uint32_t)0x00000008U)  /**< Pin 3 selected    */
#define APP_IO_PIN_4                 ((uint32_t)0x00000010U)  /**< Pin 4 selected    */
#define APP_IO_PIN_5                 ((uint32_t)0x00000020U)  /**< Pin 5 selected    */
#define APP_IO_PIN_6                 ((uint32_t)0x00000040U)  /**< Pin 6 selected    */
#define APP_IO_PIN_7                 ((uint32_t)0x00000080U)  /**< Pin 7 selected    */
#define APP_IO_PIN_8                 ((uint32_t)0x00000100U)  /**< Pin 8 selected    */
#define APP_IO_PIN_9                 ((uint32_t)0x00000200U)  /**< Pin 9 selected    */
#define APP_IO_PIN_10                ((uint32_t)0x00000400U)  /**< Pin 10 selected   */
#define APP_IO_PIN_11                ((uint32_t)0x00000800U)  /**< Pin 11 selected   */
#define APP_IO_PIN_12                ((uint32_t)0x00001000U)  /**< Pin 12 selected   */
#define APP_IO_PIN_13                ((uint32_t)0x00002000U)  /**< Pin 13 selected   */
#define APP_IO_PIN_14                ((uint32_t)0x00004000U)  /**< Pin 14 selected   */
#define APP_IO_PIN_15                ((uint32_t)0x00008000U)  /**< Pin 15 selected   */
#define APP_IO_PIN_16                ((uint32_t)0x00010000U)  /**< Pin 16 selected   */
#define APP_IO_PIN_17                ((uint32_t)0x00020000U)  /**< Pin 17 selected   */
#define APP_IO_PIN_18                ((uint32_t)0x00040000U)  /**< Pin 18 selected   */
#define APP_IO_PIN_19                ((uint32_t)0x00080000U)  /**< Pin 19 selected   */
#define APP_IO_PIN_20                ((uint32_t)0x00100000U)  /**< Pin 20 selected   */
#define APP_IO_PIN_21                ((uint32_t)0x00200000U)  /**< Pin 21 selected   */
#define APP_IO_PIN_22                ((uint32_t)0x00400000U)  /**< Pin 22 selected   */
#define APP_IO_PIN_23                ((uint32_t)0x00800000U)  /**< Pin 23 selected   */
#define APP_IO_PIN_24                ((uint32_t)0x01000000U)  /**< Pin 24 selected   */
#define APP_IO_PIN_25                ((uint32_t)0x02000000U)  /**< Pin 25 selected   */
#define APP_IO_PIN_26                ((uint32_t)0x04000000U)  /**< Pin 26 selected   */
#define APP_IO_PIN_27                ((uint32_t)0x08000000U)  /**< Pin 27 selected   */
#define APP_IO_PIN_28                ((uint32_t)0x10000000U)  /**< Pin 28 selected   */
#define APP_IO_PIN_29                ((uint32_t)0x20000000U)  /**< Pin 29 selected   */
#define APP_IO_PIN_30                ((uint32_t)0x40000000U)  /**< Pin 30 selected   */
#define APP_IO_PIN_31                ((uint32_t)0x80000000U)  /**< Pin 31 selected   */

#define APP_IO_PINS_0_15             ((uint32_t)0x0000FFFFU)  /**< 0~15 pins selected  */
#define APP_IO_PINS_16_31            ((uint32_t)0xFFFF0000U)  /**< 16~31 pins selected */
#define APP_IO_PIN_ALL               ((uint32_t)0xFFFFFFFFU)  /**< All pins selected   */
#define APP_MSIO_PIN_MASK            ((uint32_t)0x0000001FU)  /**< PIN mask for assert test */
#define APP_MSIO_PIN_ALL             ((uint32_t)0x001FU)      /**< All pins selected */
#define APP_AON_IO_PIN_MASK          ((uint32_t)0x000000FFU)  /**< PIN mask for assert test */
#define APP_AON_IO_PIN_ALL           ((uint32_t)0x00FFU)      /**< All pins selected */

#define APP_IO_PIN_MASK              ((uint32_t)0xFFFFFFFFU)  /**< PIN mask for assert test */

/**
  * @brief GR551x_APP_GPIO_default_config initStruct default configuart APP_GPIOn
  */
#define APP_IO_DEFAULT_CONFIG                      \
{                                                  \
    .pin        = APP_IO_PIN_ALL,                  \
    .mode       = APP_IO_MODE_INPUT,               \
    .pull       = APP_IO_PULLDOWN,                 \
    .mux        = APP_IO_MUX_7,                    \
}

/** @} */


/** @addtogroup APP_GPIO_ENUMERATIONS Enumerations
  * @{
  */
/**
  * @brief   GPIO state Enumerations definition
  */
typedef enum
{
    APP_IO_PIN_RESET,        /**< IO pin low level. */
    APP_IO_PIN_SET,          /**< IO pin high level. */
} app_io_pin_state_t;

/**
  * @brief   GPIO type Enumerations definition
  */
typedef enum
{
    APP_IO_TYPE_NORMAL,      /**< General Purpose Input/Output. */
    APP_IO_TYPE_AON,         /**< Always-on Input/Output.       */
    APP_IO_TYPE_MSIO,        /**< Mixed Signal I/O.             */
    APP_IO_TYPE_MAX,         /**< Only for check parameter, not used as input parameters. */
}app_io_type_t;

/**
  * @brief   GPIO mode Enumerations definition
  */
typedef enum
{
    APP_IO_MODE_INPUT,          /**< Input Mode.                                         */
    APP_IO_MODE_OUT_PUT,        /**< Output Mode.                                        */
    APP_IO_MODE_MUX,            /**< Mux Mode.                                           */
    APP_IO_MODE_IT_RISING,      /**< Interrupt Mode with Rising edge trigger detection.  */
    APP_IO_MODE_IT_FALLING,     /**< Interrupt Mode with Falling edge trigger detection. */
    APP_IO_MODE_IT_HIGH,        /**< Interrupt Mode with High-level trigger detection.   */
    APP_IO_MODE_IT_LOW,         /**< Interrupt Mode with Low-level trigger detection.    */
    APP_IO_MODE_ANALOG,         /**< Analog IO Mode.                                     */
    APP_IO_MODE_MAX,            /**< Only for check parameter, not used as input parameters. */
}app_io_mode_t;

/**
  * @brief   GPIO wake-up mode Enumerations definition
  */
typedef enum
{
    APP_IO_NONE_WAKEUP,         /**< None Wakeup.         */
    APP_IO_DISABLE_WAKEUP,      /**< Disable AON GPIO Wakeup. */
    APP_IO_ENABLE_WAKEUP,       /**< Enable AON GPIO Wakeup.  */
}app_handle_mode_t;

/**
  * @brief   GPIO handler context type Enumerations definition
  */
typedef enum
{
    APP_IO_CTX_WAKEUP,          /**< the event of wakeup.    */
    APP_IO_CTX_INT,             /**< the event of interrupt. */
}app_ctx_type_t;

/**
  * @brief   GPIO pull Enumerations definition
  */
typedef enum
{
    APP_IO_NOPULL,              /**< No Pull-up or Pull-down activation.  */
    APP_IO_PULLUP,              /**< Pull-up activation.                  */
    APP_IO_PULLDOWN,            /**< Pull-down activation.                */
    APP_IO_PULL_MAX             /**< Only for check parameter, not used as input parameters. */
}app_io_pull_t;

/**
  * @brief   GPIO mux Enumerations definition
  */
typedef enum
{
    APP_IO_MUX_0,               /**< IO mux mode 0. */
    APP_IO_MUX_1,               /**< IO mux mode 1. */
    APP_IO_MUX_2,               /**< IO mux mode 2. */
    APP_IO_MUX_3,               /**< IO mux mode 3. */
    APP_IO_MUX_4,               /**< IO mux mode 4. */
    APP_IO_MUX_5,               /**< IO mux mode 5. */
    APP_IO_MUX_6,               /**< IO mux mode 6. */
    APP_IO_MUX_7,               /**< IO mux mode 7. */
    APP_IO_MUX_8,               /**< IO mux mode 8. */
    APP_IO_MUX_MAX,             /**< Only for check parameter, not used as input parameters. */
}app_io_mux_t;
/** @} */

/** @addtogroup APP_GPIO_STRUCT Structures
  * @{
  */
/**
  * @brief GPIO parameter structure definition
  */
typedef struct
{
    uint32_t  pin;              /**< Specifies the IO pins to be configured.
                                     This parameter can be any value of @ref GR551x_pins */
    app_io_mode_t mode;         /**< Specifies the operating mode for the selected pins. */
    app_io_pull_t pull;         /**< Specifies the Pull-up or Pull-Down activation for the selected pins. */
    app_io_mux_t  mux;          /**< Specifies the Peripheral to be connected to the selected pins. */
} app_io_init_t;
/** @} */



/* Exported functions --------------------------------------------------------*/
/** @addtogroup HAL_APP_GPIO_DRIVER_FUNCTIONS Functions
  * @{
  */
/**
 ****************************************************************************************
 * @brief  Initialize the APP GPIO DRIVER according to the specified parameters
 *         in the app_io_type_t and app_io_init_t.
 *
 * @param[in]  type:   GPIO type.
 * @param[in]  p_init: Pointer to app_io_init_t parameter which contains the
 *                     configuration information for the specified GPIO.
 *
 * @return Result of initialization.
 ****************************************************************************************
 */
uint16_t app_io_init(app_io_type_t type, app_io_init_t *p_init);

/**
 ****************************************************************************************
 * @brief  De-initialize the GPIOx peripheral.
 *
 * @param[in]  type: GPIO type, See app_io_type_t.
 * @param[in]  pin:  The pin want to De-initialization.
 *
 * @return Result of De-initialization.
 ****************************************************************************************
 */
uint16_t app_io_deinit(app_io_type_t type, uint32_t pin);

/**
 ****************************************************************************************
 * @brief  Read the specified input port pin..
 *
 * @param[in]  type: GPIO type, See app_io_type_t.
 * @param[in]  pin:  The pin want to read.
 *
 * @return The GPIO state.
 ****************************************************************************************
 */
app_io_pin_state_t app_io_read_pin(app_io_type_t type, uint32_t pin);

/**
 ****************************************************************************************
 * @brief  Set or clear the selected data port bit.
 *
 * @param[in]  type:      GPIO type, See app_io_type_t.
 * @param[in]  pin:       The pin want to set or clear.
 * @param[in]  pin_state: Specifies the value to be written to the selected bit.
 *
 * @return Result of write.
 ****************************************************************************************
 */
uint16_t app_io_write_pin(app_io_type_t type, uint32_t pin, app_io_pin_state_t pin_state);

/**
 ****************************************************************************************
 * @brief  Toggle the specified GPIO pin.
 *
 * @param[in]  type: GPIO type, See app_io_type_t.
 * @param[in]  pin:  The pin want to toggle.
 *
 * @return Result of toggle.
 ****************************************************************************************
 */
uint16_t app_io_toggle_pin(app_io_type_t type, uint32_t pin);
/** @} */

#endif

/** @} */
/** @} */
/** @} */


