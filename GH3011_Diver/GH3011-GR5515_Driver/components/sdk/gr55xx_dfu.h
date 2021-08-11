/**
 ******************************************************************************
 *
 * @file gr55xx_dfu.h
 *
 * @brief Device Firmware Update API
 *
 ******************************************************************************
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

/**
* @addtogroup SYSTEM
* @{
*/
/**
 @addtogroup DFU Device Firmware Update
 @{
 @brief Definitions and prototypes for the DFU interface.
*/

#ifndef __GR55XX_DFU_H__
#define __GR55XX_DFU_H__

#include <stdbool.h>
#include <stdint.h>

/** @addtogroup DFU_DEFINES Defines
 * @{ */
#define DFU_ADV_MAX_LENGTH  18      /**< The max length of advertisement is 18 bytes. */
#define RECEIVE_MAX_LEN     2048    /**< The max length of received data is 2048 bytes. */

/**@defgroup DFU_CMD_DISABLE DFU cmd disable option (bitmask)
 * @{ */
#define DFU_WRITE_RAM_DISABLE           0x0001     /**< Disable DFU write ram. */      
#define DFU_READ_RAM_DISABLE            0x0002     /**< Disable DFU read ram. */  
#define DFU_DUMP_FLASH_DISABLE          0x0004     /**< Disable DFU dump flash. */  
#define DFU_ERASE_FLASH_DISABLE         0x0008     /**< Disable DFU erase flash. */  
#define DFU_UPDAE_FLASH_DISABLE         0x0010     /**< Disable DFU update flash. */  
#define DFU_OPERATE_SYSTEM_INFO_DISABLE 0x0020     /**< Disable DFU operate system info area. */  
#define DFU_OPERATE_NVDS_DISABLE        0x0040     /**< Disable DFU operate NVDS. */  
#define DFU_OPERATE_EFUSE_DISABLE       0x0080     /**< Disable DFU operate EFUSE. */  
#define DFU_CONFIG_SPI_FLASH_DISABLE    0x0100     /**< Disable DFU config spi flash. */  
#define DFU_OPERATE_REG_DISABLE         0x0200     /**< Disable DFU operate register. */  
/**@} */

/** @} */

/**@addtogroup DFU_ENUMERATIONS Enumerations
 * @{ */

/**@brief UART baudrate definition. */
typedef enum
{
    DFU_UART_BAUDRATE_9600          = 9600,     /**< Baudrate 9600. */
    DFU_UART_BAUDRATE_19200         = 19200,    /**< Baudrate 19200. */
    DFU_UART_BAUDRATE_38400         = 38400,    /**< Baudrate 38400. */
    DFU_UART_BAUDRATE_57600         = 57600,    /**< Baudrate 57600. */
    DFU_UART_BAUDRATE_115200        = 115200,   /**< Baudrate 115200. */
    DFU_UART_BAUDRATE_230400        = 230400,   /**< Baudrate 230400. */
    DFU_UART_BAUDRATE_921600        = 921600,   /**< Baudrate 921600. */
} dfu_uart_baudrate_t;

/**@brief UART data bit definition. */
typedef enum
{
    DFU_UART_DATA_BIT_5             = 0,        /**< Data bit 5. */
    DFU_UART_DATA_BIT_6,                        /**< Data bit 6. */
    DFU_UART_DATA_BIT_7,                        /**< Data bit 7. */
    DFU_UART_DATA_BIT_8,                        /**< Data bit 8. */
} dfu_uart_data_bit_t;

/**@brief UART stop bit definition. */
typedef enum
{
    DFU_UART_STOP_BIT_1             = 0,        /**< Stop bit 1. Avaliable when data bit = 5,6,7,8. */
    DFU_UART_STOP_BIT_1_5,                      /**< Stop bit 1.5. Avaliable when data bit = 5. */
    DFU_UART_STOP_BIT_2,                        /**< Stop bit 2. Avaliable when data bit = 6,7,8. */
} dfu_uart_stop_bit_t;

/**@brief UART parity definition. */
typedef enum
{
    DFU_UART_PARITY_NONE            = 0,        /**< Parity none. */
    DFU_UART_PARITY_ODD,                        /**< Parity odd. */
    DFU_UART_PARITY_EVEN,                       /**< Parity even. */
    DFU_UART_PARITY_SP_0,                       /**< Parity stick 0. */
    DFU_UART_PARITY_SP_1,                       /**< Parity stick 1. */
} dfu_uart_parity_bit_t;

/**@brief UART pin group definition. */
typedef enum
{
    DFU_UART_PIN_GROUP_0 = 0,              /**< GPIO0_PIN3  -> UART0_TXD(MUX0),GPIO0_PIN4  -> UART0_RXD(MUX0)*/
    DFU_UART_PIN_GROUP_1,                  /**< GPIO0_PIN10 -> UART0_TXD(MUX2),GPIO0_PIN11 -> UART0_RXD(MUX2)*/
    DFU_UART_PIN_GROUP_2,                  /**< GPIO0_PIN0  -> UART0_TXD(MUX4),GPIO0_PIN1  -> UART0_RXD(MUX4)*/
    DFU_UART_PIN_GROUP_3,                  /**< GPIO1_PIN14 -> UART0_TXD(MUX5),GPIO1_PIN10 -> UART0_RXD(MUX5)*/
    DFU_UART_PIN_GROUP_4,                  /**< GPIO0_PIN7  -> UART1_TXD(MUX3),GPIO0_PIN6  -> UART1_RXD(MUX3)*/ 
    DFU_UART_PIN_GROUP_5,                  /**< GPIO0_PIN9  -> UART1_TXD(MUX3),GPIO0_PIN8  -> UART1_RXD(MUX3)*/ 
} dfu_uart_pin_group_t;

/**@brief DFU informatica status. */
typedef enum
{
    DFU_INFO_DISABLE = 0x5775,                 /**< DFU Config Info Disable. */
    DFU_INFO_ENABLE = 0x7ff7,                  /**< DFU Config Info Enable. */
    DFU_INFO_DEFAULT = 0x4774,                 /**< DFU Config Info DEFAULT */
} dfu_info_state;
/** @} */

/**@addtogroup DFU_STRUCTURES Structures
 * @{ */

/**@brief BootLoader information definition. */
typedef struct
{
    uint32_t bin_size;                      /**< Firmware Size. */
    uint32_t check_sum;                     /**< Firmware Check Sum Value. */
    uint32_t load_addr;                     /**< Firmware Load Address. */
    uint32_t run_addr ;                     /**< Firmware Run Address. */
    uint32_t xqspi_xip_cmd;                 /**< XIP Read Mode. 0x03: Read mode, 0x0B: Fast Read mode, 0x3B: DualOut Fast Read mode,
                                                 0xBB: DualIO Fast Read mode, 0x6B: QuadOut Fast Read mode, 0xEB: QuadIO Fast Read mode */
    uint32_t xqspi_speed: 4;                /**< Bit: 0-3  clock speed. 0 :64 MHz, 1:48 MHz, 2:32 MHz, 3:24 MHz, 4:16 MHz. */
    uint32_t code_copy_mode: 1;             /**< Bit: 4 code copy mode. 0:XIP,1:QSPI. */
    uint32_t system_clk: 3;                 /**< Bit: 5-7 system clock. 0:64 MHz, 1:48 MHz, 2:32 MHz(xo), 3:24 MHz, 4:16 MHz, 5:32 MHz(cpll). */
    uint32_t check_image:1;                 /**< Bit: 8 check image. */
    uint32_t boot_delay:1;                  /**< Bit: Boot delay flag. */
    uint32_t is_dap_boot:1;                 /**< Bit: 11 check if boot dap mode. */
    uint32_t reserved:21;                   /**< Bit: 24 reserved. */
} boot_info_t;


/**@brief DFU uart config definition. */
typedef struct
{
    dfu_info_state        state;              /**< DFU UART Enable or Disable. This parameter can be a value of @ref dfu_info_state.*/
    dfu_uart_baudrate_t   baud_rate;          /**< UART communication baudrate. This parameter can be a value of @ref dfu_uart_baudrate_t.*/
    dfu_uart_data_bit_t   data_bit;           /**< The number of data bits transmitted or received in a frame. This parameter can be a value of @ref dfu_uart_data_bit_t.*/
    dfu_uart_stop_bit_t   stop_bits;          /**< The number of stop bits transmitted. This parameter can be a value of @ref dfu_uart_stop_bit_t.*/
    dfu_uart_parity_bit_t parity;             /**< Specify the parity mode. This parameter can be a value of @ref dfu_uart_parity_bit_t.*/
    dfu_uart_pin_group_t  pin_group;          /**< Uart pin group. This parameter can be a value of @ref dfu_uart_pin_group_t.*/
} dfu_uart_info_t;

/**@brief DFU Advertisement name config definition. */
typedef struct
{
    dfu_info_state state;                          /**< If a set name is used, this parameter can be a value of @ref dfu_info_state.*/
    uint8_t        adv_name[DFU_ADV_MAX_LENGTH];   /**< Adv name data. */
    uint16_t       adv_name_length;                /**< The length of the name. */
} dfu_adv_name_info_t;

/**@brief DFU NVDS init info definition. */
typedef struct
{
    dfu_info_state  state;                         /**< If NVDS init is required, this parameter can be a value of @ref dfu_info_state.*/
    uint16_t        page_size;                     /**< NVDS page size. */
    uint32_t        start_address;                 /**< NVDS start address. */
}dfu_nvds_info_t;

/**@brief DFU cmd disable info definition. */
typedef struct
{
    dfu_info_state state;                         /**< If DFU cmd disable config is required, this parameter can be a value of @ref dfu_info_state.*/
    uint16_t cmd_bit_map;                         /**< NVDS start address. */
}dfu_cmd_disable_t;


/**@brief DFU used functions config definition. */
typedef struct
{
    void (*dfu_ble_send_data)(uint8_t *p_data, uint16_t length);                                  /**< The function is used to send data to master by BLE. */
    void (*dfu_uart_send_data)(uint8_t *p_data, uint16_t length);                                 /**< The function is used to send data to master by UART. */
    uint32_t (*dfu_flash_read)(const uint32_t addr, uint8_t *p_buf, const uint32_t size);         /**< The function is used to read data from flash. */
    uint32_t (*dfu_flash_write)(const uint32_t addr, const uint8_t *p_buf, const uint32_t size);  /**< The function is used to write data to flash. */
    bool (*dfu_flash_erase)(const uint32_t addr, const uint32_t size);                            /**< The function is used to erase flash by address. */
    bool (*dfu_flash_erase_chip)(void);                                                           /**< The function is used to erase flash chip. */
    void (*dfu_flash_set_security)(bool enable);                                                  /**< The function is used to set the flash security mode as Enable or Disable. */
    bool (*dfu_flash_get_security)(void);                                                         /**< The function is used to get the flash security mode (Enable or Disable). */
	void (*dfu_flash_get_info)(uint32_t *id, uint32_t *size);                                     /**< The function is used to get the flash id and size. */
} dfu_func_t;

/**@brief SPI used functions config definition. */
typedef struct
{
    void (*dfu_spi_flash_init)(uint8_t *p_data);                                                   /**< The function is used to config flash spi. */
    uint32_t (*dfu_spi_flash_read)(uint32_t addr, uint8_t *buf, uint32_t size);                    /**< The function is used to read external flash . */
    uint32_t (*dfu_spi_flash_write)(uint32_t addr, uint8_t *buf, uint32_t size);                   /**< The function is used to write external flash. */
    bool (*dfu_spi_flash_erase)(uint32_t addr, uint32_t size);                                     /**< The function is used to erase external flash by address. */
    bool (*dfu_spi_flash_erase_chip)(void);                                                        /**< The function is used to erase exteral flash chip. */
    void (*dfu_spi_flash_get_info)(uint32_t *id, uint32_t *size);                                  /**< The function is used to get external flash id and size. */
}dfu_spi_flash_func_t;


/**@brief DFU program state callback definition. */
typedef struct
{
    void (*dfu_program_start_callback)(void);          /**<DFU program start callback. */
    void (*dfu_programing_callback)(uint8_t pro);      /**<DFU programing callback. */
    void (*dfu_program_end_callback)(uint8_t status);  /**<DFU program end callback. */
} dfu_pro_callback_t;

/** @} */


/** @addtogroup DFU_FUNCTIONS Functions
 * @{ */
/**
 *****************************************************************************************
 * @brief Function for initializing the DFU Used and Program State Callback.
 *
 * @note When APP wants to add DFU features, the flash_read and flash_write functions should be registered.
 *       To creat own DFU bootloaders, all functions in dfu_func_t should be registered. In order to show
 *       DFU program states by own DFU bootloaders, all functions in @ref dfu_pro_callback_t should be registered.
 *
 * @param[in]  p_app_dfu_func: DFU used functions.
 * @param[in]  p_dfu_buffer: DFU data receiving buffer.
 * @param[in]  p_dfu_callback: DFU program state callback functions.
 *****************************************************************************************
 */
void dfu_init(dfu_func_t *p_app_dfu_func, uint8_t* p_dfu_buffer, dfu_pro_callback_t *p_dfu_callback);

/**
 *****************************************************************************************
 * @brief Function for reset the DFU cmd parse state.
 *
 * @note When APP wants to add DFU timeout feature, shoulde reset  DFU cmd parse state when timeout.
 *****************************************************************************************
 */
void dfu_cmd_parse_state_reset(void);

/**
 *****************************************************************************************
 * @brief Function for setting the BLE MTU size.
 *
 * @note If the user wants to create his or her own DFU bootloader, the DFU MTU size should be 
 *       set by using this function (when the MTU size is updated, this function should be called).
 *
 * @param[in]  mtu_size: The BLE MTU size.
 *****************************************************************************************
 */
void dfu_ble_set_mtu_size(uint16_t mtu_size);

/**
 *****************************************************************************************
 * @brief This function should be called when BLE stack sends data completely.
 *
 * @note This function should be used when the user wants to create his or her own DFU bootloader.
 *
 * @retval void
 *****************************************************************************************
 */
void dfu_ble_send_data_cmpl_process(void);

/**
 *****************************************************************************************
 * @brief This function should be called when BLE receives data.
 *
 * @note This function should be used when the user wants to create his or her own DFU bootloader.
 *
 * @param[in]  p_data: The received data from BLE.
 * @param[in]  length: The length of data.
 *****************************************************************************************
 */
void dfu_ble_receive_data_process(uint8_t *p_data, uint16_t length);

/**
 *****************************************************************************************
 * @brief This function should be called when UART receives data.
 *
 * @note This function should be used when the user wants to create his or her own DFU bootloader.
 *
 * @param[in]  p_data: The received data from UART
 * @param[in]  length: The length of data
 *****************************************************************************************
 */
void dfu_uart_receive_data_process(uint8_t *p_data, uint16_t length);

/**
 *****************************************************************************************
 * @brief Function for checking DFU cmd.
 *
 * @note This function should be called in main loop.
 *****************************************************************************************
 */
void dfu_schedule(void);

/**
 *****************************************************************************************
 * @brief Function for jumping to address to run.
 *
 * @note In Apps, if the user wants to jump to another APP or own a DFU bootloader and run the code,
 *       this function can be called. This function does not change the boot info, so the original APP will run after resetting.
 *
 * @param[in]  start_addr: The jumped address.
 *****************************************************************************************
 */
void dfu_start_jump(uint32_t start_addr);

/**
 *****************************************************************************************
 * @brief Function for changing the boot info and reseting device.
 *
 * @note In Apps, if the user wants to jump to another APP or own a DFU bootloader and run the code,
 *       this function can be called (the boot info will be changed).
 *
 * @param[in]  p_boot_info: The boot info of the APP you want to run.
 *****************************************************************************************
 */
void dfu_start_address(boot_info_t *p_boot_info);

/**
 *****************************************************************************************
 * @brief Function for starting the default DFU bootloader(Only available for GR5515_D Chip).
 *
 * @note If the user wants to enter the default DFU bootloader, this function will be called. 
 *       The dfu_uart_info can be used to start the UART DFU feature, and the DFU adv_name_info can be used to set the dfu adv name.
 *      (By default, the UART DFU feature ID is closed, and the default adv name is ble_dfu.).
 *
 * @param[in]  p_dfu_uart_info:     Default DFU bootloader UART info, NULL--the UART DFU feature is closed.
 * @param[in]  p_dfu_adv_name_info: Default DFU bootloader adv name, NULL--the adv name is ble_dfu.
 * @param[in]  p_dfu_nvds_info:     NVDS init info, NULL--Not init NVDS.
 *****************************************************************************************
 */
void dfu_start_default(dfu_uart_info_t *p_dfu_uart_info, dfu_adv_name_info_t *p_dfu_adv_name_info, dfu_nvds_info_t *p_dfu_nvds_info);

/**
 *****************************************************************************************
 * @brief Function for set the default DFU bootloader informatica(Only available for GR5515_D Chip).
 *
 * @note If the user wants to set the default DFU bootloader informatica, this function will be called. 
 *       The dfu_uart_info can be used to start the UART DFU feature, and the DFU adv_name_info can be used to set the dfu adv name.
 *      (By default, the UART DFU feature ID is closed, and the default adv name is ble_dfu.).
 *
 * @param[in]  p_dfu_uart_info:     Default DFU bootloader UART info, NULL--the UART DFU feature is closed.
 * @param[in]  p_dfu_adv_name_info: Default DFU bootloader adv name, NULL--the adv name is ble_dfu.
 * @param[in]  p_dfu_nvds_info:     NVDS init info, NULL--Not init NVDS.
 *****************************************************************************************
 */
void dfu_start_set(dfu_uart_info_t *p_dfu_uart_info, dfu_adv_name_info_t *p_dfu_adv_name_info, dfu_nvds_info_t *p_dfu_nvds_info);


/**
 *****************************************************************************************
 * @brief Function for set DFU disable cmd.
 *
 * @note In Apps, if the user wants to jump to ROM DFU run,but want to be able to control the operation permissions of DFU,
 *       this function can be called.
 *
 * @param[in]  disable_cmd_bit_map: The bitmask of DFU disable cmd  See @ref DFU_CMD_DISABLE.
 *****************************************************************************************
 */
void dfu_set_disable_cmd(uint16_t disable_cmd_bit_map);

/**
 *****************************************************************************************
 * @brief Function for initializing the DFU SPI Flash Callback.
 *
 * @note When APP wants to add DFU operationally external flash feature, the dfu_spi_flash_func_t should be registered.
 *
 * @param[in]  spi_flash_func: DFU operationally external flash used functions.
 *****************************************************************************************
 */
void dfu_spi_flash_func_config(dfu_spi_flash_func_t *spi_flash_func);

/** @} */
#endif
/** @} */
/** @} */
