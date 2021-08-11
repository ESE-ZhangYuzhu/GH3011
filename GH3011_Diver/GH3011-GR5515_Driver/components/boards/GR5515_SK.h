/**
 ****************************************************************************************
 *
 * @file GR5515_SK.h
 *
 * @brief GR5515 Start Kit Macro.
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
#ifndef __GR5515_SK_H__
#define __GR5515_SK_H__

/**@brief IO PINs for leakage protection. */
#define IO_PIN(x)          ((uint32_t)1 << (x))    /**< IO PIN. */

#define GPIO_USE_TABLE      IO_PIN(0) | IO_PIN(1) | IO_PIN(10) | IO_PIN(11) 

#define AON_IO_USE_TABLE    IO_PIN(1)
/** @} */

#if APP_DRIVER_USE_ENABLE

/*******HCI UART IO CONFIG***********************/
#define HCI_UART_ID                     APP_UART_ID_0
#define HCI_UART_FLOW_ON                0
#define HCI_UART_BAUDRATE               115200
#define HCI_UART_TRN_PORT               APP_IO_TYPE_NORMAL
#define HCI_UART_FLOW_PORT              APP_IO_TYPE_NORMAL
#define HCI_UART_TX_PIN                 APP_IO_PIN_10
#define HCI_UART_RX_PIN                 APP_IO_PIN_11
#define HCI_UART_CTS_PIN                APP_IO_PIN_2
#define HCI_UART_RTS_PIN                APP_IO_PIN_5
#define HCI_UART_TX_PINMUX              APP_IO_MUX_2
#define HCI_UART_RX_PINMUX              APP_IO_MUX_2
#define HCI_UART_CTS_PINMUX             APP_IO_MUX_0
#define HCI_UART_RTS_PINMUX             APP_IO_MUX_0

/*******UART DRIVER IO CONFIG*******************/
#define APP_UART_ID                     APP_UART_ID_1
#define APP_UART_BAUDRATE               115200
#define APP_UART_TX_IO_TYPE             APP_IO_TYPE_MSIO
#define APP_UART_RX_IO_TYPE             APP_IO_TYPE_MSIO
#define APP_UART_TX_PIN                 APP_IO_PIN_0
#define APP_UART_RX_PIN                 APP_IO_PIN_1
#define APP_UART_TX_PINMUX              APP_IO_MUX_2
#define APP_UART_RX_PINMUX              APP_IO_MUX_2
#define APP_UART_TX_PULL                APP_IO_PULLUP
#define APP_UART_RX_PULL                APP_IO_PULLUP

/*******KEY DRIVER IO CONFIG********************/
#define KEY_OK_IO_TYPE                  APP_IO_TYPE_AON
#define KEY_UP_IO_TYPE                  APP_IO_TYPE_NORMAL
#define KEY_DOWN_IO_TYPE                APP_IO_TYPE_NORMAL
#define KEY_LEFT_IO_TYPE                APP_IO_TYPE_NORMAL
#define KEY_RIGHT_IO_TYPE               APP_IO_TYPE_NORMAL
#define KEY_OK_PIN                      APP_IO_PIN_1
#define KEY_UP_PIN                      APP_IO_PIN_12
#define KEY_DOWN_PIN                    APP_IO_PIN_13
#define KEY_LEFT_PIN                    APP_IO_PIN_14
#define KEY_RIGHT_PIN                   APP_IO_PIN_15

/*******KEY TRIGGER & PULL MODE CONFIG*******************/
#define KEY_TRIGGER_MODE                APP_IO_MODE_IT_FALLING
#define KEY_PULL_MODE                   APP_IO_PULLUP

/*******LED IO CONFIG FOR SK*********************/
#define LED_NUM_0_IO                    APP_IO_PIN_4
#define LED_NUM_1_IO                    MSIO_PIN_4

/*******ADC IO CONFIG***************************/
#define ADC_P_INPUT_PIN                 MSIO_PIN_0
#define ADC_N_INPUT_PIN                 MSIO_PIN_1

/*******UC1701 DRIVER IO CONFIG*****************/
#define DISPLAY_DRIVER_TYPE_HW_SPI
//#define DISPLAY_DRIVER_TYPE_SW_IO
#define DISPLAY_SPIM_CS0_PIN            APP_IO_PIN_3
#define DISPLAY_CMD_AND_DATA_PIN        APP_IO_PIN_5
#define DISPLAY_SPIM_CLK_PIN            APP_IO_PIN_7
#define DISPLAY_SPIM_MOSI_PIN           APP_IO_PIN_6
#define DISPLAY_BACK_LIGHT_PIN          APP_IO_PIN_2
#define DISPLAY_SPIM_GPIO_TYPE          APP_IO_TYPE_NORMAL

/*******PWM IO CONFIG***************************/
#define PWM0_MODULE                     PWM0
#define PWM0_GPIO_MUX                   APP_IO_MUX_5
#define PWM0_CHANNEL_C                  APP_IO_PIN_4
#define PWM0_PORT                       APP_IO_TYPE_NORMAL

#define PWM1_MODULE                     PWM1
#define PWM1_GPIO_MUX                   APP_IO_MUX_0
#define PWM1_CHANNEL_B                  APP_IO_PIN_4
#define PWM1_PORT                       APP_IO_TYPE_MSIO

/*******VS1005 MP3 CODEC DRIVER IO CONFIG*******/
#else

/*******HCI UART IO CONFIG***********************/
#define HCI_UART_ID                     0
#define HCI_UART_FLOW_ON                0
#define HCI_UART_BAUDRATE               115200
#define HCI_UART_TRN_PORT               GPIO0
#define HCI_UART_FLOW_PORT              GPIO0
#define HCI_UART_TX_PIN                 GPIO_PIN_10
#define HCI_UART_RX_PIN                 GPIO_PIN_11
#define HCI_UART_CTS_PIN                GPIO_PIN_2
#define HCI_UART_RTS_PIN                GPIO_PIN_5
#define HCI_UART_TX_PINMUX              GPIO_MUX_2
#define HCI_UART_RX_PINMUX              GPIO_MUX_2
#define HCI_UART_CTS_PINMUX             GPIO_MUX_0
#define HCI_UART_RTS_PINMUX             GPIO_MUX_0

/*******UART DRIVER IO CONFIG*******************/
#define SERIAL_PORT_GRP                 UART0
#define SERIAL_PORT_PORT                GPIO0
#define SERIAL_PORT_BAUDRATE            115200
#define SERIAL_PORT_TX_PIN              GPIO_PIN_10
#define SERIAL_PORT_RX_PIN              GPIO_PIN_11
#define SERIAL_PORT_TX_PINMUX           GPIO_MUX_2
#define SERIAL_PORT_RX_PINMUX           GPIO_MUX_2

/*******KEY DRIVER IO CONFIG********************/
#define KEY_OK_PIN                      AON_GPIO_PIN_1
#define KEY_UP_PIN                      GPIO_PIN_12
#define KEY_DOWN_PIN                    GPIO_PIN_13
#define KEY_LEFT_PIN                    GPIO_PIN_14
#define KEY_RIGHT_PIN                   GPIO_PIN_15

/*******KEY TRIGGER MODE CONFIG*******************/
#define KEY_ANO_TRIGGER_MODE            AON_GPIO_MODE_IT_FALLING
#define KEY_TRIGGER_MODE                GPIO_MODE_IT_FALLING

/*******LED IO CONFIG FOR SK*********************/
#define LED_NUM_0_IO                    GPIO_PIN_4
#define LED_NUM_0_GRP                   GPIO0 
#define LED_NUM_1_IO                    MSIO_PIN_4

/*******ADC IO CONFIG***************************/
#define ADC_P_INPUT_PIN                 MSIO_PIN_0
#define ADC_N_INPUT_PIN                 MSIO_PIN_1

/*******GPIO KEY********************************/
#define GPIO_KEY0                       GPIO_PIN_12
#define GPIO_KEY1                       GPIO_PIN_13
#define GPIO_KEY_PORT                   GPIO0

/*******I2C IO CONFIG***************************/
#define I2C_MODULE                      I2C0
#define I2C_GPIO_MUX                    GPIO_MUX_2
#define I2C_GPIO_PORT                   GPIO1
#define I2C_SCL_PIN                     GPIO_PIN_14  //GPIO30
#define I2C_SDA_PIN                     GPIO_PIN_10  //GPIO26

#define I2C_MASTER_MODULE               I2C0
#define I2C_MASTER_GPIO_MUX             GPIO_MUX_3
#define I2C_MASTER_GPIO_PORT            MSIO
#define I2C_MASTER_SCL_PIN              MSIO_PIN_0   //MSIO0
#define I2C_MASTER_SDA_PIN              MSIO_PIN_1   //MSIO1

#define I2C_SLAVE_MODULE                I2C1
#define I2C_SLAVE_GPIO_MUX              GPIO_MUX_0
#define I2C_SLAVE_GPIO_PORT             GPIO1
#define I2C_SLAVE_SCL_PIN               GPIO_PIN_14  //GPIO30
#define I2C_SLAVE_SDA_PIN               GPIO_PIN_10  //GPIO26

/*******I2S IO CONFIG**************************/
#define I2S_MASTER_GPIO_MUX             AON_GPIO_MUX_2
#define I2S_MASTER_WS_PIN               AON_GPIO_PIN_2
#define I2S_MASTER_TX_SDO_PIN           AON_GPIO_PIN_3
#define I2S_MASTER_RX_SDI_PIN           AON_GPIO_PIN_4
#define I2S_MASTER_SCLK_PIN             AON_GPIO_PIN_5

#define I2S_SLAVE_GPIO_MUX              GPIO_MUX_4
#define I2S_SLAVE_GPIO_PORT             GPIO1
#define I2S_SLAVE_WS_PIN                GPIO_PIN_8
#define I2S_SLAVE_TX_SDO_PIN            GPIO_PIN_9
#define I2S_SLAVE_RX_SDI_PIN            GPIO_PIN_0
#define I2S_SLAVE_SCLK_PIN              GPIO_PIN_1

/*******PWM IO CONFIG***************************/
#define PWM0_MODULE                     PWM0
#define PWM0_GPIO_MUX                   GPIO_MUX_5
#define PWM0_CHANNEL_C                  GPIO_PIN_4
#define PWM0_PORT                       GPIO0

#define PWM1_MODULE                     PWM1
#define PWM1_GPIO_MUX                   MSIO_MUX_0
#define PWM1_CHANNEL_B                  MSIO_PIN_4
#define PWM1_PORT                       MSIO

/*******COMP IO CONFIG***************************/
#define COMP_INPUT_PIN                  MSIO_PIN_0
#define COMP_INPUT_PORT                 MSIO

#define COMP_VREF_PIN                   MSIO_PIN_1
#define COMP_VREF_PORT                  MSIO

/*******QSPI IO CONFIG**************************/
#define QSPI_MODULE                     QSPI1
#define QSPI_GPIO_MUX                   GPIO_MUX_2
#define QSPI_GPIO_PORT                  GPIO0
#define QSPI_CS_PIN                     GPIO_PIN_15  //GPIO15
#define QSPI_CLK_PIN                    GPIO_PIN_9   //GPIO9
#define QSPI_IO0_PIN                    GPIO_PIN_8   //GPIO8
#define QSPI_IO1_PIN                    GPIO_PIN_14  //GPIO14
#define QSPI_IO2_PIN                    GPIO_PIN_13  //GPIO13
#define QSPI_IO3_PIN                    GPIO_PIN_12  //GPIO12

/*******SPIM IO CONFIG**************************/
#define SPIM_GPIO_MUX                   GPIO_MUX_0
#define SPIM_GPIO_PORT                  GPIO1
#define SPIM_CS0_PIN                    GPIO_PIN_1  //GPIO17
#define SPIM_CS1_PIN                    GPIO_PIN_15 //GPIO31
#define SPIM_CLK_PIN                    GPIO_PIN_8  //GPIO24
#define SPIM_MOSI_PIN                   GPIO_PIN_9  //GPIO25
#define SPIM_MISO_PIN                   GPIO_PIN_0  //GPIO16

#define SPIS_GPIO_MUX                   GPIO_MUX_1
#define SPIS_GPIO_PORT                  GPIO1
#define SPIS_CS0_PIN                    GPIO_PIN_1  //GPIO17
#define SPIS_CLK_PIN                    GPIO_PIN_8  //GPIO24
#define SPIS_MOSI_PIN                   GPIO_PIN_0  //GPIO16
#define SPIS_MISO_PIN                   GPIO_PIN_9  //GPIO25
#endif
#endif
