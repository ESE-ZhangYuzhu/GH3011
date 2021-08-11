/**
 ****************************************************************************************
 *
 * @file qyc32_config.c
 *
 * @brief qyc32 config Implementation.
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
#include "qyc32_config.h"
#include "app_spi.h"
#include "app_log.h"

#ifdef QYC32_IC_DRIVER

#include "gr55xx_delay.h"


/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
spi_handle_t s_SPIMHandle;

/*
 * GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 */

void hal_spi_msp_init(spi_handle_t *hspi)
{
    app_io_init_t io_config = APP_IO_DEFAULT_CONFIG;

    /* Configurate the clk pin of spi */
    io_config.mode = APP_IO_MODE_MUX;
    io_config.pin  = QYC32_SPIM_CLK_PIN;
    io_config.mux  = QYC32_GPIO_MUX;
    io_config.pull = APP_IO_NOPULL;
    app_io_init(APP_IO_TYPE_NORMAL, &io_config);

    /* Configurate the mosi pin of spi */
    io_config.mode = APP_IO_MODE_MUX;
    io_config.pin  = QYC32_SPIM_MOSI_PIN;
    io_config.mux  = QYC32_GPIO_MUX;
    io_config.pull = APP_IO_NOPULL;
    app_io_init(APP_IO_TYPE_NORMAL, &io_config);
}

static void qyc32_send_data(uint8_t cmd)
{
    //GLOBAL_EXCEPTION_DISABLE();
    app_spi_transmit_sync(APP_SPI_ID_MASTER, &cmd, 1, 2000);
    //GLOBAL_EXCEPTION_ENABLE();
}

void qyc32_write_cmd(uint8_t cmd)
{
    QYC32_CS_LOW();
    QYC32_SEND_CMD();
    qyc32_send_data(cmd);
    QYC32_CS_HIGH();
}

void qyc32_write_data(uint8_t data)
{
    QYC32_CS_LOW();
    QYC32_SEND_DATA();
    qyc32_send_data(data);
    QYC32_CS_HIGH();
}

void qyc32_write_buffer(uint8_t *p_data, uint16_t length)
{
    QYC32_CS_LOW();
    QYC32_SEND_DATA();
    //GLOBAL_EXCEPTION_DISABLE();
    app_spi_transmit_sync(APP_SPI_ID_MASTER, p_data, length, 5000);
    //GLOBAL_EXCEPTION_ENABLE();
    QYC32_CS_HIGH();
}


void qyc32_hal_init(void)
{
//    hal_status_t status;
    app_drv_err_t error = APP_DRV_SUCCESS;
    app_io_init_t io_config = APP_IO_DEFAULT_CONFIG;

    /* Configurate the data pin */
    io_config.mode = APP_IO_MODE_OUT_PUT;
    io_config.pin  = QYC32_CMD_AND_DATA_PIN;
    io_config.mux  = APP_IO_MUX_7;
    io_config.pull = APP_IO_NOPULL;
    app_io_init(APP_IO_TYPE_NORMAL, &io_config);

    /* Configurate the cs0 pin */
    io_config.mode = APP_IO_MODE_OUT_PUT;
    io_config.pin  = QYC32_SPIM_CS0_PIN;
    io_config.mux  = APP_IO_MUX_7;
    io_config.pull = APP_IO_NOPULL;
    app_io_init(APP_IO_TYPE_NORMAL, &io_config);
    QYC32_CS_HIGH();

    /* Configurate the reset pin */
    io_config.mode = APP_IO_MODE_OUT_PUT;
    io_config.pin  = QYC32_GPOI_RESET;
    io_config.mux  = APP_IO_MUX_7;
    io_config.pull = APP_IO_NOPULL;
#ifdef QY_IO_OLD
    app_io_init(APP_IO_TYPE_AON, &io_config);
#else
    app_io_init(APP_IO_TYPE_NORMAL, &io_config);
#endif


    /* Configurate the busy pin */
    io_config.mode = APP_IO_MODE_INPUT;
    io_config.pin  = QYC32_GPOI_BUSY;
    io_config.mux  = APP_IO_MUX_7;
    io_config.pull = APP_IO_NOPULL;
    app_io_init(APP_IO_TYPE_NORMAL, &io_config);
    
    app_spi_params_t spi_params;
    spi_params.id = APP_SPI_ID_MASTER;


    spi_params.pin_cfg.cs.type = DISPLAY_SPIM_GPIO_TYPE;
    spi_params.pin_cfg.cs.pin = QYC32_SPIM_CS0_PIN;
    spi_params.pin_cfg.cs.mux = APP_IO_MUX_7;
    spi_params.pin_cfg.cs.pull = APP_IO_NOPULL;
    
    spi_params.pin_cfg.clk.type = DISPLAY_SPIM_GPIO_TYPE;
    spi_params.pin_cfg.clk.pin = QYC32_SPIM_CLK_PIN;
    spi_params.pin_cfg.clk.mux = QYC32_GPIO_MUX;
    spi_params.pin_cfg.clk.pull = APP_IO_NOPULL;
    
    spi_params.pin_cfg.mosi.type = DISPLAY_SPIM_GPIO_TYPE;
    spi_params.pin_cfg.mosi.pin = QYC32_SPIM_MOSI_PIN;
    spi_params.pin_cfg.mosi.mux = QYC32_GPIO_MUX;
    spi_params.pin_cfg.mosi.pull = APP_IO_NOPULL;
    
    spi_params.pin_cfg.miso.type = DISPLAY_SPIM_GPIO_TYPE;
    spi_params.pin_cfg.miso.pin = QYC32_SPIM_MOSI_PIN;
    spi_params.pin_cfg.miso.mux = QYC32_GPIO_MUX;
    spi_params.pin_cfg.miso.pull = APP_IO_NOPULL;

    
    spi_params.init.data_size          = SPI_DATASIZE_8BIT;
    spi_params.init.clock_polarity     = SPI_POLARITY_LOW;
    spi_params.init.clock_phase        = SPI_PHASE_1EDGE;
    spi_params.init.baudrate_prescaler = SystemCoreClock / 4000000;
    spi_params.init.ti_mode            = SPI_TIMODE_DISABLE;
    spi_params.init.slave_select       = SPI_SLAVE_SELECT_0;
    error = app_spi_init(&spi_params, NULL);

    if(error != APP_DRV_SUCCESS)
        APP_LOG_ERROR("app_spi_init error:%d", error);
}

void qyc32_hal_deinit(void)
{
    hal_spi_deinit(&s_SPIMHandle);
    app_io_deinit(APP_IO_TYPE_NORMAL, QYC32_GPOI_BUSY);
#ifdef QY_IO_OLD
    app_io_deinit(APP_IO_TYPE_AON, QYC32_GPOI_RESET);
#else
    app_io_deinit(APP_IO_TYPE_NORMAL, QYC32_GPOI_RESET);
#endif
    app_io_deinit(APP_IO_TYPE_NORMAL, QYC32_CMD_AND_DATA_PIN);
    app_io_deinit(APP_IO_TYPE_NORMAL, QYC32_SPIM_CS0_PIN);
    app_io_deinit(APP_IO_TYPE_NORMAL, QYC32_SPIM_CLK_PIN);
    app_io_deinit(APP_IO_TYPE_NORMAL, QYC32_SPIM_MOSI_PIN);
}

#endif

