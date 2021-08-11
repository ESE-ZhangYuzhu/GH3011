/**
  ****************************************************************************************
  * @file    gr551x_spi_flash.c
  * @author  BLE Driver Team
  * @brief   HAL APP module driver.
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

/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include <string.h>
#include "gr55xx_hal.h"
#include "gr551x_spi_flash.h"

/*
 * DEFINES
 *****************************************************************************************
 */
#define SPI_SPEED_1M                      (1000000)
#define SPI_SPEED_2M                      (2000000)
#define SPI_SPEED_4M                      (4000000)
#define SPI_SPEED_8M                      (8000000)
#define SPI_SPEED_16M                     (16000000)
#define SPI_SPEED_32M                     (32000000)

#define DEFAULT_QSPI_SPEED                (SPI_SPEED_1M)
#define DEFAULT_QSPI_IO_CONFIG            { { APP_IO_TYPE_NORMAL, APP_IO_MUX_2, APP_IO_PIN_15 }, { APP_IO_TYPE_NORMAL, APP_IO_MUX_2, APP_IO_PIN_9  },\
                                            { APP_IO_TYPE_NORMAL, APP_IO_MUX_2, APP_IO_PIN_8  }, { APP_IO_TYPE_NORMAL, APP_IO_MUX_2, APP_IO_PIN_14 },\
                                            { APP_IO_TYPE_NORMAL, APP_IO_MUX_2, APP_IO_PIN_13 }, { APP_IO_TYPE_NORMAL, APP_IO_MUX_2, APP_IO_PIN_12 } }
#define DEFAULT_QSPI_MODE_CONFIG          { APP_QSPI_TYPE_DMA, DMA_Channel7 }
#define DEFAULT_QSPI_CONFIG               { (SystemCoreClock / DEFAULT_QSPI_SPEED), QSPI_CLOCK_MODE_3, 0}
#define DEFAULT_QSPI_PARAM_CONFIG         { APP_QSPI_ID_1, DEFAULT_QSPI_IO_CONFIG, DEFAULT_QSPI_MODE_CONFIG, DEFAULT_QSPI_CONFIG}

#define DEFAULT_SPIM_SPEED                (SPI_SPEED_1M)
#define DEFAULT_SPIM_IO_CONFIG            {{APP_IO_TYPE_NORMAL, APP_IO_MUX_7, APP_IO_PIN_15}, {APP_IO_TYPE_NORMAL, APP_IO_MUX_1, APP_IO_PIN_12},\
                                           {APP_IO_TYPE_NORMAL, APP_IO_MUX_1, APP_IO_PIN_13}, {APP_IO_TYPE_NORMAL, APP_IO_MUX_1, APP_IO_PIN_14}}
#define DEFAULT_SPIM_MODE_CONFIG          {APP_SPI_TYPE_DMA, DMA_Channel5, DMA_Channel6}
#define DEFAULT_SPIM_CONFIG               {SPI_DATASIZE_8BIT, SPI_POLARITY_LOW, SPI_PHASE_1EDGE, (SystemCoreClock / DEFAULT_SPIM_SPEED), SPI_TIMODE_DISABLE, SPI_SLAVE_SELECT_0}
#define DEFAULT_SPIM_PARAM_CONFIG         {APP_SPI_ID_MASTER, DEFAULT_SPIM_IO_CONFIG, DEFAULT_SPIM_MODE_CONFIG, DEFAULT_SPIM_CONFIG}

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static volatile qspi_control_t g_qspi_ctl;
static dma_handle_t            g_dma_handle;
static spi_handle_t            g_spim_handle;
static qspi_handle_t           g_qspi_handle;
static flash_init_t            g_flash_init;

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
static void spi_app_qspi_callback(app_qspi_evt_t *p_evt)
{
    if (p_evt->type == APP_QSPI_EVT_TX_CPLT)
    {
        g_qspi_ctl.qspi_tmt_done = 1;
    }
    if (p_evt->type == APP_QSPI_EVT_RX_DATA)
    {
        g_qspi_ctl.qspi_rcv_done = 1;
    }
    if (p_evt->type == APP_QSPI_EVT_ERROR)
    {
        g_qspi_ctl.qspi_tmt_done = 1;
        g_qspi_ctl.qspi_rcv_done = 1;
    }
}

void spi_app_spim_callback(app_spi_evt_t *p_evt)
{
    if (p_evt->type == APP_SPI_EVT_TX_CPLT)
    {
        g_qspi_ctl.spi_tmt_done = 1;
    }
    if (p_evt->type == APP_SPI_EVT_RX_DATA)
    {
        g_qspi_ctl.spi_rcv_done = 1;
    }
    if (p_evt->type == APP_SPI_EVT_ERROR)
    {
        g_qspi_ctl.spi_tmt_done = 1;
        g_qspi_ctl.spi_rcv_done = 1;
    }
}

static void spi_flash_write_enable(void)
{
    uint8_t control_frame[1] = {SPI_FLASH_CMD_WREN};

    if (FLASH_SPIM_ID == g_flash_init.spi_type)
    {
        g_qspi_ctl.spi_tmt_done = 0;
        app_spi_transmit_async(g_qspi_ctl.spi_id, control_frame, sizeof(control_frame));
        while(g_qspi_ctl.spi_tmt_done == 0);
    }
    else
    {
        g_qspi_ctl.qspi_tmt_done = 0;
        app_qspi_transmit_async(g_qspi_ctl.qspi_id, control_frame, sizeof(control_frame));
        while(g_qspi_ctl.qspi_tmt_done == 0);
    }

    return;
}

static uint8_t spi_flash_read_status(void)
{
    uint8_t status = 0;

    if (FLASH_SPIM_ID == g_flash_init.spi_type)
    {
        uint8_t control_frame[1] = {SPI_FLASH_CMD_RDSR};
        
        g_qspi_ctl.spi_rcv_done = 0;
        app_spi_read_memory_async(g_qspi_ctl.spi_id, control_frame, (uint8_t*)&status, sizeof(control_frame), 1);
        while(g_qspi_ctl.spi_rcv_done == 0);
    }
    else
    {
        qspi_command_t command = {
            .instruction      = SPI_FLASH_CMD_RDSR,
            .address          = 0,
            .instruction_size = QSPI_INSTSIZE_08_BITS,
            .address_size     = QSPI_ADDRSIZE_00_BITS,
            .dummy_cycles     = 0,
            .instruction_address_mode = QSPI_INST_ADDR_ALL_IN_SPI,
            .data_mode        = QSPI_DATA_MODE_SPI,
            .length           = 1,
        };

        g_qspi_ctl.qspi_rcv_done = 0;
        app_qspi_command_receive_async(g_qspi_ctl.qspi_id, &command, (uint8_t*)&status);
        while(g_qspi_ctl.qspi_rcv_done == 0);
    }
    
    return status;
}

static uint32_t spi_flash_device_size(void)
{
    uint32_t flash_size = 0;

    if (FLASH_SPIM_ID == g_flash_init.spi_type)
    {
        uint8_t data[5] = {0};
        uint8_t control_frame[5] = {SPI_FLASH_CMD_SFUD, 0, 0, 0x34, DUMMY_BYTE};

        g_qspi_ctl.spi_rcv_done = 0;
        app_spi_read_memory_async(g_qspi_ctl.spi_id, control_frame, data, sizeof(control_frame), sizeof(data));
        while(g_qspi_ctl.spi_rcv_done == 0);
        
        if (data[0] != 0 && data[3] < 0xFF)
        {
            flash_size = ((data[3] << 24) + (data[2] << 16) + (data[1] << 8) + (data[0] << 0) + 1) / 8;
        }
    }
    else
    {
        uint8_t data[4] = {0};
        qspi_command_t command = {
            .instruction      = SPI_FLASH_CMD_SFUD,   // SPI_FLASH_CMD_SFUD  //SPI_FLASH_CMD_RDSR
            .address          = 0x000034,
            .instruction_size = QSPI_INSTSIZE_08_BITS,
            .address_size     = QSPI_ADDRSIZE_24_BITS,
            .dummy_cycles     = 8,
            .instruction_address_mode = QSPI_INST_ADDR_ALL_IN_SPI,
            .data_mode        = QSPI_DATA_MODE_SPI,
            .length           = sizeof(data),
        };
        
        g_qspi_ctl.qspi_rcv_done = 0;
        app_qspi_command_receive_async(g_qspi_ctl.qspi_id, &command, data);
        while(g_qspi_ctl.qspi_rcv_done == 0);

        if (data[0] != 0 && data[3] < 0xFF)
        {
            flash_size = ((data[3] << 24) + (data[2] << 16) + (data[1] << 8) + (data[0] << 0) + 1) / 8;
        }
    }

    return flash_size;
}


static uint32_t spim_flash_write(uint32_t address, uint8_t *buffer, uint32_t nbytes)
{
    uint8_t control_frame[4] = {0};

    control_frame[0] = SPI_FLASH_CMD_PP;
    control_frame[1] = (address >> 16) & 0xFF;
    control_frame[2] = (address >> 8) & 0xFF;
    control_frame[3] = address & 0xFF;

    g_qspi_ctl.spi_tmt_done = 0;
    app_spi_write_memory_async(g_qspi_ctl.spi_id, control_frame, buffer, sizeof(control_frame), nbytes);
    while(g_qspi_ctl.spi_tmt_done == 0);
    
    return nbytes;
}

static uint32_t qspi_flash_write(uint32_t address, uint8_t *buffer, uint32_t nbytes)
{
    qspi_command_t command = {
        .instruction      = SPI_FLASH_CMD_PP,
        .address          = address,
        .instruction_size = QSPI_INSTSIZE_08_BITS,
        .address_size     = QSPI_ADDRSIZE_24_BITS,
        .dummy_cycles     = 0,
        .instruction_address_mode = QSPI_INST_ADDR_ALL_IN_SPI,
        .data_mode        = QSPI_DATA_MODE_SPI,
        .length           = nbytes,
    };
    
    g_qspi_ctl.qspi_tmt_done = 0;
    app_qspi_command_transmit_async(g_qspi_ctl.qspi_id, &command, buffer);
    while(g_qspi_ctl.qspi_tmt_done == 0);

    return nbytes;
}

static uint32_t spim_flash_read(uint32_t address, uint8_t *buffer, uint32_t nbytes)
{
    uint8_t control_frame[4] = {0};

    control_frame[0] = SPI_FLASH_CMD_READ;
    control_frame[1] = (address >> 16) & 0xFF;
    control_frame[2] = (address >> 8) & 0xFF;
    control_frame[3] = address & 0xFF;

    g_qspi_ctl.spi_rcv_done = 0;
    app_spi_read_memory_async(g_qspi_ctl.spi_id, control_frame, buffer, sizeof(control_frame), nbytes);
    while(g_qspi_ctl.spi_rcv_done == 0);
    
    return nbytes;
}

static uint32_t qspi_flash_read(uint32_t address, uint8_t *buffer, uint32_t nbytes)
{
    qspi_command_t command = {
        .instruction      = SPI_FLASH_CMD_READ,
        .address          = address,
        .instruction_size = QSPI_INSTSIZE_08_BITS,
        .address_size     = QSPI_ADDRSIZE_24_BITS,
        .dummy_cycles     = 0,
        .instruction_address_mode = QSPI_INST_ADDR_ALL_IN_SPI,
        .data_mode        = QSPI_DATA_MODE_SPI,
        .length           = nbytes,
    };

    g_qspi_ctl.qspi_rcv_done = 0;
    app_qspi_command_receive_async(g_qspi_ctl.qspi_id, &command, buffer);
    while(g_qspi_ctl.qspi_rcv_done == 0);

    return nbytes;
}

bool spim_flash_sector_erase(uint32_t address)
{
    uint8_t control_frame[4] = {0};

    control_frame[0] = SPI_FLASH_CMD_SE;
    control_frame[1] = (address >> 16) & 0xFF;
    control_frame[2] = (address >> 8) & 0xFF;
    control_frame[3] = address & 0xFF;

    g_qspi_ctl.spi_tmt_done = 0;
    app_spi_transmit_async(g_qspi_ctl.spi_id, control_frame, sizeof(control_frame));
    while(g_qspi_ctl.spi_tmt_done == 0);

    return true;
}

bool qspi_flash_sector_erase(uint32_t address)
{
    uint8_t control_frame[4] = {0};

    control_frame[0] = SPI_FLASH_CMD_SE;
    control_frame[1] = (address >> 16) & 0xFF;
    control_frame[2] = (address >> 8) & 0xFF;
    control_frame[3] = address & 0xFF;

    g_qspi_ctl.qspi_tmt_done = 0;
    app_qspi_transmit_async(g_qspi_ctl.qspi_id, control_frame, sizeof(control_frame));
    while(g_qspi_ctl.qspi_tmt_done == 0);

    return true;
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void spi_flash_init(flash_init_t *p_flash_init)
{
    memcpy(&g_flash_init, p_flash_init, sizeof(flash_init_t));

    if (FLASH_SPIM_ID == p_flash_init->spi_type)
    {
       app_spi_params_t spim_params = DEFAULT_SPIM_PARAM_CONFIG;
        
       spim_params.pin_cfg.cs.type     = p_flash_init->flash_io.spi_cs.gpio;
       spim_params.pin_cfg.cs.pin      = p_flash_init->flash_io.spi_cs.pin;
       spim_params.pin_cfg.cs.mux      = p_flash_init->flash_io.spi_cs.mux;
       spim_params.pin_cfg.cs.pull     = APP_IO_NOPULL;
       spim_params.pin_cfg.cs.enable   = APP_SPI_PIN_ENABLE;
       spim_params.pin_cfg.clk.type    = p_flash_init->flash_io.spi_clk.gpio;
       spim_params.pin_cfg.clk.pin     = p_flash_init->flash_io.spi_clk.pin;
       spim_params.pin_cfg.clk.mux     = p_flash_init->flash_io.spi_clk.mux;
       spim_params.pin_cfg.clk.pull    = APP_IO_NOPULL;
       spim_params.pin_cfg.clk.enable  = APP_SPI_PIN_ENABLE;
       spim_params.pin_cfg.mosi.type   = p_flash_init->flash_io.spi_io0.qspi_io0.gpio;
       spim_params.pin_cfg.mosi.pin    = p_flash_init->flash_io.spi_io0.qspi_io0.pin;
       spim_params.pin_cfg.mosi.mux    = p_flash_init->flash_io.spi_io0.qspi_io0.mux;
       spim_params.pin_cfg.mosi.pull   = APP_IO_NOPULL;
       spim_params.pin_cfg.mosi.enable = APP_SPI_PIN_ENABLE;
       spim_params.pin_cfg.miso.type   = p_flash_init->flash_io.spi_io1.qspi_io1.gpio;
       spim_params.pin_cfg.miso.pin    = p_flash_init->flash_io.spi_io1.qspi_io1.pin;
       spim_params.pin_cfg.miso.mux    = p_flash_init->flash_io.spi_io1.qspi_io1.mux;
       spim_params.pin_cfg.miso.pull   = APP_IO_NOPULL;
       spim_params.pin_cfg.miso.enable = APP_SPI_PIN_ENABLE;

       g_qspi_ctl.spi_id = APP_SPI_ID_MASTER;

       app_spi_deinit(g_qspi_ctl.spi_id);
       app_spi_init(&spim_params, spi_app_spim_callback);
    }
    else if ((FLASH_QSPI_ID0 == p_flash_init->spi_type) || (FLASH_QSPI_ID1 == p_flash_init->spi_type))
    {
            app_qspi_params_t qspi_params = DEFAULT_QSPI_PARAM_CONFIG;

            if(FLASH_QSPI_ID0 == p_flash_init->spi_type){
                g_qspi_ctl.qspi_id = APP_QSPI_ID_0;
            }else{
                g_qspi_ctl.qspi_id = APP_QSPI_ID_1;
            }

            qspi_params.id                     = g_qspi_ctl.qspi_id;
            qspi_params.pin_cfg.cs.type        = p_flash_init->flash_io.spi_cs.gpio;
            qspi_params.pin_cfg.cs.pin         = p_flash_init->flash_io.spi_cs.pin;
            qspi_params.pin_cfg.cs.mux         = p_flash_init->flash_io.spi_cs.mux;
            qspi_params.pin_cfg.cs.pull        = APP_IO_NOPULL;
            qspi_params.pin_cfg.cs.enable      = APP_SPI_PIN_ENABLE;
            qspi_params.pin_cfg.clk.type       = p_flash_init->flash_io.spi_clk.gpio;
            qspi_params.pin_cfg.clk.pin        = p_flash_init->flash_io.spi_clk.pin;
            qspi_params.pin_cfg.clk.mux        = p_flash_init->flash_io.spi_clk.mux;
            qspi_params.pin_cfg.clk.pull       = APP_IO_NOPULL;
            qspi_params.pin_cfg.clk.enable     = APP_SPI_PIN_ENABLE;
            qspi_params.pin_cfg.io_0.type      = p_flash_init->flash_io.spi_io0.qspi_io0.gpio;
            qspi_params.pin_cfg.io_0.pin       = p_flash_init->flash_io.spi_io0.qspi_io0.pin;
            qspi_params.pin_cfg.io_0.mux       = p_flash_init->flash_io.spi_io0.qspi_io0.mux;
            qspi_params.pin_cfg.io_0.pull      = APP_IO_NOPULL;
            qspi_params.pin_cfg.io_0.enable    = APP_SPI_PIN_ENABLE;
            qspi_params.pin_cfg.io_1.type      = p_flash_init->flash_io.spi_io1.qspi_io1.gpio;
            qspi_params.pin_cfg.io_1.pin       = p_flash_init->flash_io.spi_io1.qspi_io1.pin;
            qspi_params.pin_cfg.io_1.mux       = p_flash_init->flash_io.spi_io1.qspi_io1.mux;
            qspi_params.pin_cfg.io_1.pull      = APP_IO_NOPULL;
            qspi_params.pin_cfg.io_1.enable    = APP_SPI_PIN_ENABLE;
            qspi_params.pin_cfg.io_2.type      = p_flash_init->flash_io.qspi_io2.gpio;
            qspi_params.pin_cfg.io_2.pin       = p_flash_init->flash_io.qspi_io2.pin;
            qspi_params.pin_cfg.io_2.mux       = p_flash_init->flash_io.qspi_io2.mux;
            qspi_params.pin_cfg.io_2.pull      = APP_IO_NOPULL;
            qspi_params.pin_cfg.io_2.enable    = APP_SPI_PIN_ENABLE;
            qspi_params.pin_cfg.io_3.type      = p_flash_init->flash_io.qspi_io3.gpio;
            qspi_params.pin_cfg.io_3.pin       = p_flash_init->flash_io.qspi_io3.pin;
            qspi_params.pin_cfg.io_3.mux       = p_flash_init->flash_io.qspi_io3.mux;
            qspi_params.pin_cfg.io_3.pull      = APP_IO_NOPULL;
            qspi_params.pin_cfg.io_3.enable    = APP_SPI_PIN_ENABLE;
            
            app_qspi_deinit(g_qspi_ctl.qspi_id);
            app_qspi_init(&qspi_params, spi_app_qspi_callback);
    }
    
    return;
}

uint32_t spi_flash_write(uint32_t address, uint8_t *buffer, uint32_t nbytes)
{
    uint32_t page_ofs, write_size, write_cont = nbytes;
    hal_status_t status      = HAL_OK;
    
    while (write_cont)
    {
        page_ofs = address & 0xFF;
        write_size = EXFLASH_SIZE_PAGE_BYTES - page_ofs;

        if (write_cont < write_size)
        {
            write_size = write_cont;
            write_cont = 0;
        }
        else
        {
            write_cont -= write_size;
        }

        spi_flash_write_enable();

        if (FLASH_SPIM_ID == g_flash_init.spi_type)
        {
            spim_flash_write(address, buffer, write_size);
        }
        else
        {
            qspi_flash_write(address, buffer, write_size);
        }

        while(spi_flash_read_status() & 0x1);
        
        address += write_size;
        buffer += write_size;
    }

    return ((status == HAL_OK) ? nbytes : 0);
}

uint32_t spi_flash_read(uint32_t address, uint8_t *buffer, uint32_t nbytes)
{
    uint32_t count = 0;

    if (FLASH_SPIM_ID == g_flash_init.spi_type)
    {
        count = spim_flash_read(address, buffer, nbytes);
    }
    else
    {
        count = qspi_flash_read(address, buffer, nbytes);
    }
    
    return count;
}

bool spi_flash_sector_erase(uint32_t address, uint32_t size)
{
    bool status = true;

    uint32_t erase_addr = address;
    uint32_t sector_ofs, erase_size, erase_cont = size;

    while (erase_cont)
    {
        sector_ofs = erase_addr & 0xFFF;
        erase_size = EXFLASH_SIZE_SECTOR_BYTES - sector_ofs;
    
        if (erase_cont < erase_size)
        {
            erase_size = erase_cont;
            erase_cont = 0;
        }
        else
        {
            erase_cont -= erase_size;
        }

        spi_flash_write_enable();
    
        if (FLASH_SPIM_ID == g_flash_init.spi_type)
        {
            status = spim_flash_sector_erase(erase_addr);
        }
        else
        {
            status = qspi_flash_sector_erase(erase_addr);
        }

        while(spi_flash_read_status() & 0x1);
        
        erase_addr += erase_size;
    }

    return status;
}

bool spi_flash_chip_erase(void)
{
    hal_status_t status = HAL_OK;
    uint8_t control_frame[1] = {SPI_FLASH_CMD_CE};

    spi_flash_write_enable();
    
    if (FLASH_SPIM_ID == g_flash_init.spi_type)
    {
        g_qspi_ctl.spi_tmt_done = 0;
        app_spi_transmit_async(g_qspi_ctl.spi_id, control_frame, sizeof(control_frame));
        while(g_qspi_ctl.spi_tmt_done == 0);
    }
    else
    {
        g_qspi_ctl.qspi_tmt_done = 0;
        app_qspi_transmit_async(g_qspi_ctl.qspi_id, control_frame, sizeof(control_frame));
        while(g_qspi_ctl.qspi_tmt_done == 0);
    }
    while(spi_flash_read_status() & 0x1);

    return ((status == HAL_OK) ? true : false);
}

void spi_flash_chip_reset(void)
{
    uint8_t control_frame[1] = {SPI_FLASH_CMD_RSTEN};

    if (FLASH_SPIM_ID == g_flash_init.spi_type)
    {
        g_qspi_ctl.spi_tmt_done = 0;
        app_spi_transmit_async(g_qspi_ctl.spi_id, control_frame, sizeof(control_frame));
        while(g_qspi_ctl.spi_tmt_done == 0);
    }
    else
    {
        hal_qspi_transmit(&g_qspi_handle, control_frame, sizeof(control_frame), 5000);
    }

    control_frame[0] = SPI_FLASH_CMD_RST;
    if (FLASH_SPIM_ID == g_flash_init.spi_type)
    {
        g_qspi_ctl.spi_tmt_done = 0;
        app_spi_transmit_async(g_qspi_ctl.spi_id, control_frame, sizeof(control_frame));
        while(g_qspi_ctl.spi_tmt_done == 0);
    }
    else
    {
        hal_qspi_transmit(&g_qspi_handle, control_frame, sizeof(control_frame), 5000);
    }
    
    return;
}

uint32_t spi_flash_device_id(void)
{
    uint8_t data[3] = {0};

    if (FLASH_SPIM_ID == g_flash_init.spi_type)
    {
        uint8_t control_frame[1] = {SPI_FLASH_CMD_RDID};
        
        g_qspi_ctl.spi_rcv_done = 0;
        app_spi_read_memory_async(g_qspi_ctl.spi_id, control_frame, data, sizeof(control_frame), sizeof(data));
        while(g_qspi_ctl.spi_rcv_done == 0);
    }
    else
    {
        qspi_command_t command = {
        .instruction      = SPI_FLASH_CMD_RDID,
        .address          = 0,
        .instruction_size = QSPI_INSTSIZE_08_BITS,
        .address_size     = QSPI_ADDRSIZE_00_BITS,
        .dummy_cycles     = 0,
        .instruction_address_mode = QSPI_INST_ADDR_ALL_IN_SPI,
        .data_mode        = QSPI_DATA_MODE_SPI,
        .length           = 3,
        };
        
        g_qspi_ctl.qspi_rcv_done = 0;
        app_qspi_command_receive_async(g_qspi_ctl.qspi_id, &command, data);
        while(g_qspi_ctl.qspi_rcv_done == 0);
    }

    return (((uint32_t)data[0] << 16) + ((uint32_t)data[1] << 8) + data[2]);
}

void spi_flash_device_info(uint32_t *id, uint32_t *size)
{
    if (NULL == id || NULL == size)
    {
        return;
    }

    *id   = spi_flash_device_id();
    *size = spi_flash_device_size();
    
    return;
}

