/**
 *****************************************************************************************
 *
 * @file vs1005_config.h
 *
 * @brief Header file - User Function
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
#ifndef _VS1005_CONFIG_H__
#define _VS1005_CONFIG_H__

/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "gr55xx_hal.h"
#include "boards.h"
/*
 * DEFINES
 *****************************************************************************************
 */

/*******VS1005 MP3 CODEC DRIVER IO CONFIG*******/
#define VS1005_GROUP                    GPIO1

#define VS_XCS_PIN                      GPIO_PIN_1
#define VS_XCS_PIN_GRP                  VS1005_GROUP

#define VS_XDCS_PIN                     GPIO_PIN_15
#define VS_XDCS_PIN_GRP                 VS1005_GROUP

#define VS_RST_PIN                      GPIO_PIN_11
#define VS_RST_PIN_GRP                  VS1005_GROUP

#define VS_DQ_PIN                       GPIO_PIN_10
#define VS_DQ_PIN_GRP                   VS1005_GROUP

#define VS_SPI_GPIO_MUX                 GPIO_MUX_0
#define VS_SPI_GPIO_GRP                 VS1005_GROUP
#define VS_CLK_PIN                      GPIO_PIN_8
#define VS_MOSI_PIN                     GPIO_PIN_9
#define VS_MISO_PIN                     GPIO_PIN_0
typedef enum
{
    BIT_RESET = 0,
    BIT_SET,
}bit_action_t;

typedef enum
{
    SPI_LOW = 0,
    SPI_HIGH,
}vs_speed_t;
    
/*
 * GLOBAL FUNCTION DECLARATION
 *****************************************************************************************
 */
void         vs_config_init(void);
uint8_t      vs_spi_read_byte(void);
void         vs_spi_write_byte(uint8_t data);
void         vs_spi_write_buffer(uint8_t *data, uint16_t len);
void         vs_set_rst_pin(bit_action_t bit_val);
void         vs_set_xcs_pin(bit_action_t bit_val);
void         vs_set_xdcs_pin(bit_action_t bit_val);
bit_action_t vs_read_dq_pin(void);
void         vs_spi_speed_set(vs_speed_t speed);
void         vs_delay_ms(uint16_t ms);
void         vs_delay_us(uint16_t us);
#endif
