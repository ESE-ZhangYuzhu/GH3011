/**
 *****************************************************************************************
 *
 * @file dfu_master.h
 *
 * @brief  DFU master API.
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
#ifndef __DFU_MASTER_H__
#define __DFU_MASTER_H__

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup DFU_MASTER_ENUM Enumerations
 * @{
 */
/**@brief DFU master event type definition. */
typedef enum
{
    FRAM_CHECK_ERROR = 0,               /**< Frame check error event. */
    IMG_INFO_CHECK_FAIL,                /**< FW info check event. */
    PRO_START_ERROR,                    /**< FW program start error event. */
    PRO_START_SUCCESS,                  /**< FW program start success event. */
    PRO_FLASH_SUCCESS,                  /**< FW program success event. */
    PRO_FLASH_FAIL,                     /**< FW program fail event. */
    PRO_END_SUCCESS,                    /**< FW program end success event. */
    PRO_END_FAIL,                       /**< FW program end fail event. */
}dfu_m_event_t;
/** @} */

/**
 * @defgroup DFU_MASTER_STRUCT Structures
 * @{
 */
/**@brief BootLoader information definition. */
typedef struct
{   
    uint32_t bin_size;                 /**< Firmware Size. */
    uint32_t check_sum;                /**< Firmware Check Sum Value. */
    uint32_t load_addr;                /**< Firmware Load Address. */
    uint32_t run_addr ;                /**< Firmware Run Address. */
    uint32_t xqspi_xip_cmd;            /**< XIP Read Mode. */
    uint32_t xqspi_speed:4;            /**< bit: 0..3  clock speed */
    uint32_t code_copy_mode:1;         /**< bit: 4 code copy mode */
    uint32_t system_clk:3;             /**< bit: 5..7 system clock */
    uint32_t check_image:1;            /**< bit: 8 check image */
    uint32_t boot_delay:1;             /**< bit: 9 boot delay time */
    uint32_t reserved:22;              /**< bit: 22 reserved */
}dfu_boot_info_t;

/**@brief IMG information definition. */
typedef struct
{
    uint16_t        pattern;           /**< IMG info pattern. */
    uint16_t        version;           /**< IMG version. */
    dfu_boot_info_t boot_info;         /**< IMG boot info. */
    uint8_t         comments[12];      /**< IMG comments. */
}dfu_img_info_t;

/**@brief DFU master used function config definition. */
typedef struct
{
    void(*dfu_m_get_img_info)(dfu_img_info_t *img_info);                      /**< This function is used to get updated firmware information. */
    void (*dfu_m_get_img_data)(uint32_t addr, uint8_t *data, uint16_t len);   /**< This function is used to get updated firmware data. */
    void (*dfu_m_send_data)(uint8_t *data, uint16_t len);                     /**< This function is used to send data to peer device. */
    void (*dfu_m_event_handler)(dfu_m_event_t event, uint8_t pre);            /**< This function is used to send event to app. */
}dfu_m_func_cfg_t;
/** @} */

/**
 * @defgroup DFU_MASTER_TYPEDEF Typedefs
 * @{
 */
/**@brief DFU Master Receive CMD Callback type. */
typedef void (*dfu_m_rev_cmd_cb_t)(void);
/** @} */

/**
 * @defgroup DFU_MASTER_FUNCTION Functions
 * @{
 */
/**
 *****************************************************************************************
 * @brief Function for reset the DFU cmd parse state.
 *****************************************************************************************
 */
void dfu_m_parse_state_reset(void);

/**
 *****************************************************************************************
 * @brief Function for checking DFU master cmd.
 *
 * @note This function should be called in loop.
 *****************************************************************************************
 */
void dfu_m_schedule(dfu_m_rev_cmd_cb_t rev_cmd_cb);

/**
 *****************************************************************************************
 * @brief Function for start update firmware.
 *
 * @param[in]  security: Upgrade firmware is encrypted?.
 * @param[in]  run_fw: Whether to run the firmware immediately after the upgrade.
 *****************************************************************************************
 */
void dfu_m_program_start(bool security, bool run_fw);

/**
 *****************************************************************************************
 * @brief Function for initializing the DFU master.
 *
 * @note When APP wants to add DFU master feature,all functions in @ref dfu_m_func_cfg_t should be registered.
 *
 * @param[in]  dfu_m_func_cfg: DFU master used functions.
 * @param[in]  once_send_size: DFU master once send size.
 *****************************************************************************************
 */
void dfu_m_init(dfu_m_func_cfg_t *dfu_m_func_cfg, uint16_t once_send_size);

/**
 *****************************************************************************************
 * @brief Function for prase received data.
 *
 * @param[in]  data: Received data.
 * @param[in]  len: Data length.
 *****************************************************************************************
 */
void dfu_m_cmd_prase(uint8_t* data,uint16_t len);

/**
 *****************************************************************************************
 * @brief This function should be called when data sended completely.
 *
 * @retval void
 *****************************************************************************************
 */
void dfu_m_send_data_cmpl_process(void);
/** @} */
#endif
