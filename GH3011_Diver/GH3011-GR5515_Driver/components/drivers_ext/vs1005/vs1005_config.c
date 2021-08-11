/**
 *****************************************************************************************
 *
 * @file vs1005_config.c
 *
 * @brief vs1005 config function Implementation.
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

/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "vs1005_config.h"
#include "gr55xx_sys.h"  
#include "boards.h"

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static spi_handle_t g_SPIMHandle;
/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void hal_spi_msp_init(spi_handle_t *p_spi)
{
    gpio_init_t GPIO_InitStructure;
    
    /* CONFIG FOR GPIO */
    GPIO_InitStructure.mode = GPIO_MODE_OUTPUT;
    GPIO_InitStructure.pin  = VS_XCS_PIN;
    GPIO_InitStructure.mux  = GPIO_PIN_MUX_GPIO;
    hal_gpio_init(VS_XCS_PIN_GRP, &GPIO_InitStructure);
    hal_gpio_write_pin(VS_XCS_PIN_GRP, VS_XCS_PIN, GPIO_PIN_RESET);
  
    GPIO_InitStructure.pin  = VS_XDCS_PIN;
    GPIO_InitStructure.mux  = GPIO_PIN_MUX_GPIO;
    hal_gpio_init(VS_XDCS_PIN_GRP, &GPIO_InitStructure);
    hal_gpio_write_pin(VS_XDCS_PIN_GRP, VS_XDCS_PIN, GPIO_PIN_RESET);
    
    GPIO_InitStructure.pin  = VS_RST_PIN;
    GPIO_InitStructure.mux  = GPIO_PIN_MUX_GPIO;
    hal_gpio_init(VS_RST_PIN_GRP, &GPIO_InitStructure);
   
    GPIO_InitStructure.pin = VS_DQ_PIN;
    GPIO_InitStructure.mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.pull = LL_GPIO_PULL_DOWN;
    GPIO_InitStructure.mux = GPIO_PIN_MUX_GPIO;
    hal_gpio_init(VS_DQ_PIN_GRP, &GPIO_InitStructure);
    
    /* CONFIG FOR SPI CONTROL IO */
    GPIO_InitStructure.mode = GPIO_MODE_MUX;
    GPIO_InitStructure.pin  = VS_CLK_PIN | VS_MOSI_PIN | VS_MISO_PIN;
    GPIO_InitStructure.mux  = VS_SPI_GPIO_MUX;
    hal_gpio_init(VS_SPI_GPIO_GRP, &GPIO_InitStructure);
}

void hal_spi_msp_deinit(spi_handle_t *p_spi)
{
    hal_gpio_deinit(VS1005_GROUP, VS_XCS_PIN | VS_XDCS_PIN | VS_CLK_PIN | VS_MOSI_PIN | VS_MISO_PIN);
}

void vs_config_init(void)
{
    g_SPIMHandle.p_instance              = SPIM;
    g_SPIMHandle.init.data_size          = SPI_DATASIZE_8BIT;
    g_SPIMHandle.init.clock_polarity     = SPI_POLARITY_HIGH;
    g_SPIMHandle.init.clock_phase        = SPI_PHASE_2EDGE;
    g_SPIMHandle.init.baudrate_prescaler = SystemCoreClock / 2000000;
    g_SPIMHandle.init.ti_mode            = SPI_TIMODE_DISABLE;
    g_SPIMHandle.init.slave_select       = SPI_SLAVE_SELECT_0;
    hal_spi_deinit(&g_SPIMHandle); 
    hal_spi_init(&g_SPIMHandle);
}

uint8_t vs_spi_read_byte(void)
{
    uint8_t rdata = 0;
    hal_spi_receive(&g_SPIMHandle, &rdata, 1, 5000);
    return rdata;
}

void vs_spi_write_byte(uint8_t data)
{
    uint8_t wdata = data;
    hal_spi_transmit(&g_SPIMHandle, &wdata, 1, 5000);
}

void vs_spi_write_buffer(uint8_t *data, uint16_t len)
{
    hal_spi_transmit(&g_SPIMHandle, data, len, 5000);
}

void vs_set_rst_pin(bit_action_t bit_val)
{
    if (bit_val == BIT_RESET)
    {
        hal_gpio_write_pin(VS_RST_PIN_GRP, VS_RST_PIN, GPIO_PIN_RESET);
    }
    else
    {
        hal_gpio_write_pin(VS_RST_PIN_GRP, VS_RST_PIN, GPIO_PIN_SET);
    }
}

void vs_set_xcs_pin(bit_action_t bit_val)
{
    if (bit_val == BIT_RESET)
    {
        hal_gpio_write_pin(VS_XCS_PIN_GRP, VS_XCS_PIN, GPIO_PIN_RESET);
    }
    else
    {
        hal_gpio_write_pin(VS_XCS_PIN_GRP, VS_XCS_PIN, GPIO_PIN_SET);
    }
}

void vs_set_xdcs_pin(bit_action_t bit_val)
{
    if (bit_val == BIT_RESET)
    {
        hal_gpio_write_pin(VS_XDCS_PIN_GRP, VS_XDCS_PIN, GPIO_PIN_RESET);
    }
    else
    {
        hal_gpio_write_pin(VS_XDCS_PIN_GRP, VS_XDCS_PIN, GPIO_PIN_SET);
    }
}

bit_action_t vs_read_dq_pin(void)
{
    gpio_pin_state_t state;
    state = hal_gpio_read_pin(VS_DQ_PIN_GRP, VS_DQ_PIN);
    if (state == GPIO_PIN_RESET)
    {
        return BIT_RESET;
    }
    else
    {
        return BIT_SET;
    }
}


void vs_spi_speed_set(vs_speed_t speed)
{
    if (speed == SPI_HIGH)
    {
        
    }
    else
    {
        
    }
}


void vs_delay_ms(uint16_t ms)
{
    sys_delay_ms(ms);
}

void vs_delay_us(uint16_t us)
{
    sys_delay_us(us);
}
