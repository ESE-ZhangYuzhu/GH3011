/**
 ****************************************************************************************
 *
 * @file qyc32_config.c
 *
 * @brief Header file - qyc32_config.h
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
#ifndef __QYC32_CONFIG_H__
#define __QYC32_CONFIG_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "gr55xx_hal.h"
#include "boards.h"
#include "app_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******qiyun GDEW029C32 DRIVER IO CONFIG*******/
#define QYC32_IC_DRIVER
#ifdef QY_IO_OLD
#define QYC32_GPIO_MUX                     APP_IO_MUX_0
#define QYC32_SPIM_CS0_PIN                 APP_IO_PIN_17   //GPIO17
#define QYC32_CMD_AND_DATA_PIN             APP_IO_PIN_27   //GPIO27
#define QYC32_SPIM_CLK_PIN                 APP_IO_PIN_24   //GPIO24
#define QYC32_SPIM_MOSI_PIN                APP_IO_PIN_25   //GPIO25
#define QYC32_GPOI_BUSY                    APP_IO_PIN_26   //GPIO26
#define QYC32_GPOI_RESET                   APP_IO_PIN_0
#else
#define QYC32_GPIO_MUX                     APP_IO_MUX_1
#define QYC32_SPIM_CS0_PIN                 APP_IO_PIN_15   //GPIO15
#define QYC32_CMD_AND_DATA_PIN             APP_IO_PIN_14   //GPIO14
#define QYC32_SPIM_CLK_PIN                 APP_IO_PIN_12   //GPIO12
#define QYC32_SPIM_MOSI_PIN                APP_IO_PIN_13   //GPIO13
#define QYC32_GPOI_BUSY                    APP_IO_PIN_8    //GPIO8
#define QYC32_GPOI_RESET                   APP_IO_PIN_7    //GPIO7
#endif


#define QYC32_REG_PSR                       0x00        // Panel Setting 
#define QYC32_REG_PWRS                      0x01        // Power Setting
#define QYC32_REG_PWRO                      0x02        // Power OFF
#define QYC32_REG_PFS                       0x03        // Power off sequence setting
#define QYC32_REG_PON                       0x04        // Power ON
#define QYC32_REG_PMES                      0x05        // Power ON Measure
#define QYC32_REG_BTST                      0x06        // Booster Soft Start
#define QYC32_REG_DSLP                      0x07        // Deep Sleep    
#define QYC32_REG_DTM1                      0x10        // Data Start Transmission 1
#define QYC32_REG_DSP                       0x11        // Data Stop
#define QYC32_REG_DRF                       0x12        // Display Refresh 
#define QYC32_REG_DTM2                      0x13        // Data Start Transmission 2
#define QYC32_REG_LUTC                      0x20        // VCOM LUT
#define QYC32_REG_LUTWW                     0x21        // W2W LUT
#define QYC32_REG_LUTBW_LUTR                0x22        // B2W LUT
#define QYC32_REG_LUTWB_LUTW                0x23        // W2B LUT
#define QYC32_REG_LUTBB_LUTB                0x24        // B2B LUT
#define QYC32_REG_PPL                       0x30        // PLL Control
#define QYC32_REG_TSC                       0x40        // Temperature Sensor Calibration
#define QYC32_REG_TSE                       0x41        // Temperature Sensor Enable 
#define QYC32_REG_TSW                       0x42        // Temperature Sensor Write
#define QYC32_REG_TSR                       0x43        // Temperature Sensor Read
#define QYC32_REG_CDI                       0x50        // VCOM And Data Interval Setting
#define QYC32_REG_LPD                       0x51        // Low Power Detection
#define QYC32_REG_TCON                      0x60        // TCON Setting 
#define QYC32_REG_TRES                      0x61        // Resolution Setting
#define QYC32_REG_FLG                       0x71        //  Get Status
#define QYC32_REG_AMV                       0x80        // Auto Measure Vcom
#define QYC32_REG_VV                        0x81        // Vcom Value
#define QYC32_REG_VDCS                      0x82        // VCM_DC Setting
#define QYC32_REG_PTL                       0x90        // Partial Window
#define QYC32_REG_PTIN                      0x91        // Partial In
#define QYC32_REG_PTOUT                     0x92        // Partial Out
#define QYC32_REG_PGM                       0xA0        // Program Mode
#define QYC32_REG_APG                       0xA1        // Active Program 
#define QYC32_REG_ROTP                      0xA2        // Read OTP Data
#define QYC32_REG_PWS                       0xE3        // Power Saving 

/**< set cs pin to high. */
#define QYC32_CS_HIGH()                                                                  \
do                                                                                       \
{                                                                                        \
    app_io_write_pin(APP_IO_TYPE_NORMAL, QYC32_SPIM_CS0_PIN, APP_IO_PIN_SET);            \
}                                                                                        \
while(0)

/**< set cs pin to low. */
#define QYC32_CS_LOW()                                                                   \
do                                                                                       \
{                                                                                        \
    app_io_write_pin(APP_IO_TYPE_NORMAL, QYC32_SPIM_CS0_PIN, APP_IO_PIN_RESET);          \
}                                                                                        \
while(0)

/**< set cmd pin to low. */
#define QYC32_SEND_CMD()                                                                 \
do                                                                                       \
{                                                                                        \
    app_io_write_pin(APP_IO_TYPE_NORMAL, QYC32_CMD_AND_DATA_PIN, APP_IO_PIN_RESET);      \
}                                                                                        \
while(0)

/**< set cmd pin to high. */
#define QYC32_SEND_DATA()                                                                \
do                                                                                       \
{                                                                                        \
    app_io_write_pin(APP_IO_TYPE_NORMAL, QYC32_CMD_AND_DATA_PIN, APP_IO_PIN_SET);        \
}                                                                                        \
while(0)

#ifdef QY_IO_OLD

#define QYC32_RESET_HIGH()                                                               \
do                                                                                       \
{                                                                                        \
    app_io_write_pin(APP_IO_TYPE_AON, QYC32_GPOI_RESET, APP_IO_PIN_SET);                 \
}                                                                                        \
while(0)

/**< set reset pin to low. */
#define QYC32_RESET_LOW()                                                                \
do                                                                                       \
{                                                                                        \
    app_io_write_pin(APP_IO_TYPE_AON, QYC32_GPOI_RESET, APP_IO_PIN_RESET);               \
}                                                                                        \
while(0)

#else

/**< set reset pin to high. */
#define QYC32_RESET_HIGH()                                                               \
do                                                                                       \
{                                                                                        \
    app_io_write_pin(APP_IO_TYPE_NORMAL, QYC32_GPOI_RESET, APP_IO_PIN_SET);              \
}                                                                                        \
while(0)

/**< set reset pin to low. */
#define QYC32_RESET_LOW()                                                                \
do                                                                                       \
{                                                                                        \
    app_io_write_pin(APP_IO_TYPE_NORMAL, QYC32_GPOI_RESET, APP_IO_PIN_RESET);            \
}                                                                                        \
while(0)

#endif

/**
 * @defgroup qiyun GDEW029C32 IC CONFIG_FUNCTION Functions
 * @{
 */
/**
 *****************************************************************************************
 * @brief Write cmd to GDEW029C32.
 *
 * @param[in] cmd:  Cmd to write.
 *****************************************************************************************
 */
void qyc32_write_cmd(uint8_t cmd);

/**
 *****************************************************************************************
 * @brief Write one data to GDEW029C32.
 *
 * @param[in] data:  Data to write.
 *****************************************************************************************
 */
void qyc32_write_data(uint8_t data);

/**
 *****************************************************************************************
 * @brief Write data buffer to GDEW029C32.
 *
 * @param[in] p_data: The pointer of the data.
 * @param[in] length: The length of write data.
 *****************************************************************************************
 */
void qyc32_write_buffer(uint8_t *p_data, uint16_t length);

#ifdef __cplusplus
}
#endif
    
#endif

