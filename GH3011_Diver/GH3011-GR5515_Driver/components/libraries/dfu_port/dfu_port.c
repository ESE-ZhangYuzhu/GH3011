/**
 *****************************************************************************************
 *
 * @file dfu_port.c
 *
 * @brief  DFU port Implementation.
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
#include "dfu_port.h"
#include "hal_flash.h"
#include "otas.h"
#ifdef ENABLE_DFU_SPI_FLASH
    #include "gr551x_spi_flash.h"
#endif

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static void ble_send_data(uint8_t *p_data, uint16_t length); /**< BLE send data to peer device. */

static dfu_func_t dfu_func =                                 /**< DFU used functions config definition . */
{
    .dfu_ble_send_data      = ble_send_data,
    .dfu_uart_send_data     = NULL,
    .dfu_flash_read         = hal_flash_read,
    .dfu_flash_write        = hal_flash_write,
    .dfu_flash_erase        = hal_flash_erase,
    .dfu_flash_erase_chip   = hal_flash_erase_chip,
    .dfu_flash_set_security = hal_flash_set_security,
    .dfu_flash_get_security = hal_flash_get_security,
    .dfu_flash_get_info     = hal_flash_get_info,
};

static dfu_enter_callback dfu_enter_func = NULL;             /**< DFU enter callback. */
static uint8_t dfu_buffer[RECEIVE_MAX_LEN];

#ifdef ENABLE_DFU_SPI_FLASH
static void dfu_spi_flash_init(uint8_t* p_data);                                            /**< flash init. */
static uint32_t dfu_spi_flash_read(uint32_t address, uint8_t *buffer, uint32_t nbytes);     /**< read flash data. */
static uint32_t dfu_spi_flash_write(uint32_t address, uint8_t *buffer, uint32_t nbytes);    /**< write flash data. */
static bool dfu_spi_flash_sector_erase(uint32_t address, uint32_t size);                    /**< erase flash sector. */
static bool dfu_spi_flash_chip_erase(void);                                                 /**< erase flash chip. */
static void dfu_spi_flash_device_info(uint32_t *id, uint32_t *size);                        /**< get flash device information. */

static dfu_spi_flash_func_t dfu_spi_flash_func=                                             /**< SPI used functions config definition. */
{
    .dfu_spi_flash_init       = dfu_spi_flash_init,
    .dfu_spi_flash_read       = dfu_spi_flash_read,
    .dfu_spi_flash_write      = dfu_spi_flash_write,
    .dfu_spi_flash_erase      = dfu_spi_flash_sector_erase,
    .dfu_spi_flash_erase_chip = dfu_spi_flash_chip_erase,
    .dfu_spi_flash_get_info   = dfu_spi_flash_device_info,
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 * @brief The function is used to config flash spi.
 *
 * @param[in] p_data: Pointer to flash gpio config value.
 *****************************************************************************************
 */
static void dfu_spi_flash_init(uint8_t *p_data)
{
    flash_init_t flash_init;
    flash_io_t *flash_io = (flash_io_t*)&flash_init.flash_io;
    
    uint8_t flash_type = p_data[0];
    const app_io_type_t gpio_type[]   = {APP_IO_TYPE_NORMAL,APP_IO_TYPE_AON,APP_IO_TYPE_MSIO};
    const uint32_t      gpio_pin[]    = {APP_IO_PIN_0,APP_IO_PIN_1,APP_IO_PIN_2,APP_IO_PIN_3,APP_IO_PIN_4,APP_IO_PIN_5,APP_IO_PIN_6,APP_IO_PIN_7,\
                                         APP_IO_PIN_8,APP_IO_PIN_9,APP_IO_PIN_10,APP_IO_PIN_11,APP_IO_PIN_12,APP_IO_PIN_13,APP_IO_PIN_14,APP_IO_PIN_15,\
                                         APP_IO_PIN_16,APP_IO_PIN_17,APP_IO_PIN_18,APP_IO_PIN_19,APP_IO_PIN_20,APP_IO_PIN_21,APP_IO_PIN_22,APP_IO_PIN_23,\
                                         APP_IO_PIN_24,APP_IO_PIN_25,APP_IO_PIN_26,APP_IO_PIN_27,APP_IO_PIN_28,APP_IO_PIN_29,APP_IO_PIN_30,APP_IO_PIN_31,};
    const app_io_mux_t gpio_pin_mux[] = {APP_IO_MUX_0,APP_IO_MUX_1,APP_IO_MUX_2,APP_IO_MUX_3,APP_IO_MUX_4,APP_IO_MUX_5,APP_IO_MUX_6,APP_IO_MUX_7,APP_IO_MUX_8};

    if(flash_type ==  0x01){
        //SPI flash
        flash_io->spi_cs.gpio            = gpio_type[p_data[1]];
        flash_io->spi_cs.pin             = gpio_pin[p_data[2]];
        flash_io->spi_cs.mux             = gpio_pin_mux[p_data[3]];
        flash_io->spi_clk.gpio           = gpio_type[p_data[4]];
        flash_io->spi_clk.pin            = gpio_pin[p_data[5]];
        flash_io->spi_clk.mux            = gpio_pin_mux[p_data[6]];
        flash_io->spi_io0.spim_mosi.gpio = gpio_type[p_data[7]];
        flash_io->spi_io0.spim_mosi.pin  = gpio_pin[p_data[8]];
        flash_io->spi_io0.spim_mosi.mux  = gpio_pin_mux[p_data[9]];
        flash_io->spi_io1.spim_miso.gpio = gpio_type[p_data[10]];
        flash_io->spi_io1.spim_miso.pin  = gpio_pin[p_data[11]];
        flash_io->spi_io1.spim_miso.mux  = gpio_pin_mux[p_data[12]];

        flash_init.spi_type = FLASH_SPIM_ID;
        spi_flash_init(&flash_init);
    }else if(flash_type ==  0x02){
        //QSPI flash
        flash_io->spi_cs.gpio           = gpio_type[p_data[1]];
        flash_io->spi_cs.pin            = gpio_pin[p_data[2]];
        flash_io->spi_cs.mux            = gpio_pin_mux[p_data[3]];
        flash_io->spi_clk.gpio          = gpio_type[p_data[4]];
        flash_io->spi_clk.pin           = gpio_pin[p_data[5]];
        flash_io->spi_clk.mux           = gpio_pin_mux[p_data[6]];
        flash_io->spi_io0.qspi_io0.gpio = gpio_type[p_data[7]];
        flash_io->spi_io0.qspi_io0.pin  = gpio_pin[p_data[8]];
        flash_io->spi_io0.qspi_io0.mux  = gpio_pin_mux[p_data[9]];
        flash_io->spi_io1.qspi_io1.gpio = gpio_type[p_data[10]];
        flash_io->spi_io1.qspi_io1.pin  = gpio_pin[p_data[11]];
        flash_io->spi_io1.qspi_io1.mux  = gpio_pin_mux[p_data[12]];
        flash_io->qspi_io2.gpio         = gpio_type[p_data[13]];
        flash_io->qspi_io2.pin          = gpio_pin[p_data[14]];
        flash_io->qspi_io2.mux          = gpio_pin_mux[p_data[15]];
        flash_io->qspi_io3.gpio         = gpio_type[p_data[16]];
        flash_io->qspi_io3.pin          = gpio_pin[p_data[17]];
        flash_io->qspi_io3.mux          = gpio_pin_mux[p_data[18]];

        flash_init.spi_type = p_data[19] ? FLASH_QSPI_ID1 : FLASH_QSPI_ID0;
        spi_flash_init(&flash_init);
    }else{
        //Unkown flash type
    }
}

/**
 *****************************************************************************************
 * @brief The function is used to read flash data.
 *
 * @param[in] address: flash address of read data .
 * @param[in] buffer: Pointer to storage buffer.
 * @param[in] nbytes: read data size .
 *****************************************************************************************
 */
static uint32_t dfu_spi_flash_read(uint32_t address, uint8_t *buffer, uint32_t nbytes)
{
    return spi_flash_read(address, buffer, nbytes);
}

/**
 *****************************************************************************************
 * @brief The function is used to write flash data.
 *
 * @param[in] address: flash address of write data.
 * @param[in] buffer: Pointer to storage buffer.
 * @param[in] nbytes: write data size.
 *****************************************************************************************
 */
static uint32_t dfu_spi_flash_write(uint32_t address, uint8_t *buffer, uint32_t nbytes)
{
    return spi_flash_write(address, buffer, nbytes);
}

/**
 *****************************************************************************************
 * @brief The function is used to erase flash sector.
 *
 * @param[in] address: flash address of erase sector.
 * @param[in] size: erase size.
 *****************************************************************************************
 */
static bool dfu_spi_flash_sector_erase(uint32_t address, uint32_t size)
{ 
    return spi_flash_sector_erase(address, size);
}

/**
 *****************************************************************************************
 * @brief The function is used to erase flash chip.
 *
 *****************************************************************************************
 */
static bool dfu_spi_flash_chip_erase(void)
{
    return spi_flash_chip_erase();
}

/**
 *****************************************************************************************
 * @brief The function is used to get flash device information.
 *
 * @param[out] id: flash id.
 * @param[out] size: flash size.
 *****************************************************************************************
 */
static void dfu_spi_flash_device_info(uint32_t *id, uint32_t *size)
{
    spi_flash_device_info(id, size);
}

#endif

/**
 *****************************************************************************************
 * @brief Process ota service event.
 *
 * @param[in] p_evt: Pointer to otas event.
 *****************************************************************************************
 */
static void otas_evt_process(otas_evt_t *p_evt)
{
    switch (p_evt->evt_type)
    {
        case OTAS_EVT_TX_NOTIFICATION_ENABLED:
            dfu_cmd_parse_state_reset();
            break;
        
        case OTAS_EVT_RX_RECEIVE_DATA:
            dfu_ble_receive_data_process(p_evt->p_data, p_evt->length);
            break;

        case OTAS_EVT_NOTIFY_COMPLETE:
            dfu_ble_send_data_cmpl_process();
            break;
        
        case OTAS_EVT_DFU_MODE_ENTER:
            if(dfu_enter_func != NULL)
            {
                dfu_enter_func();
            }
            break;

        default:
            break;
    }
}

/**
 *****************************************************************************************
 * @brief Send data to peer device by BLE.
 *
 * @param[in] p_data: Pointer to send data.
 * @param[in] length: Length of send data.
 *****************************************************************************************
 */
static void ble_send_data(uint8_t *p_data, uint16_t length)
{
    otas_notify_tx_data(0, p_data, length);
}


void dfu_port_init(dfu_uart_send_data uart_send_data, dfu_pro_callback_t *p_dfu_callback)
{
    if(uart_send_data != NULL)
    {
        dfu_func.dfu_uart_send_data = uart_send_data;
    }
    dfu_init(&dfu_func, dfu_buffer, p_dfu_callback);
#ifdef ENABLE_DFU_SPI_FLASH
    dfu_spi_flash_func_config(&dfu_spi_flash_func);
#endif
}

void dfu_service_init(dfu_enter_callback dfu_enter)
{
    if(dfu_enter != NULL)
    {
        dfu_enter_func = dfu_enter;
    }
    otas_init_t otas_init;
    otas_init.evt_handler = otas_evt_process;
    otas_service_init(&otas_init);
}
