/**
 *****************************************************************************************
 *
 * @file dfu_master.c
 *
 * @brief  DFU master Implementation.
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
 ****************************************************************************************
 */
#include "dfu_master.h"
#include <string.h>

/*
 * DEFINES
 ****************************************************************************************
 */
#define ONCE_SEND_LEN           1024     /**< DFU master once send length. */
#define RECEIVE_MAX_LEN         2048     /**< DFU master receive max length. */
#define CMD_FRAME_HEADER_L      0x44     /**< CMD header low byte. */
#define CMD_FRAME_HEADER_H      0x47     /**< CMD header high byte. */
#define PROGRAM_START           0x23     /**< Program start cmd. */
#define PROGRAME_FLASH          0x24     /**< Program flash cmd. */
#define PROGRAME_END            0x25     /**< Program end cmd. */
#define ACK_SUCCESS             0x01     /**< CMD ack success. */
#define ACK_ERROR               0x02     /**< CMD ack error. */

#define FLASH_OP_PAGE_SIZE      0x1000   /**< Flash page size. */
#define PATTERN_VALUE           (0x4744) /**< Pattern value. */

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
/**@brief DFU master submachine state. */
typedef enum
{
    CHECK_FRAME_L_STATE = 0x00,
    CHECK_FRAME_H_STATE,
    RECEIVE_CMD_TYPE_L_STATE,
    RECEIVE_CMD_TYPE_H_STATE,
    RECEIVE_LEN_L_STATE,
    RECEIVE_LEN_H_STATE,
    RECEIVE_DATA_STATE,
    RECEIVE_CHECK_SUM_L_STATE,
    RECEIVE_CHECK_SUM_H_STATE,
} cmd_parse_state_t;

/*
 * STRUCTURES
 *****************************************************************************************
 */
/**@brief DFU master receive frame structure define. */
typedef struct
{
    uint16_t cmd_type;
    uint16_t data_len;
    uint8_t  data[RECEIVE_MAX_LEN];
    uint16_t check_sum;
} receive_frame_t;

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
static receive_frame_t   s_receive_frame;
static bool              s_cmd_receive_flag;
static uint16_t          s_receive_data_count;
static uint32_t          s_receive_check_sum;

static dfu_img_info_t    s_now_img_info;
static uint32_t          s_page_start_addr;
static uint32_t          s_program_size;
static uint32_t          s_all_check_sum;
static uint32_t          s_file_size;
static bool              s_run_fw_flag;
static dfu_m_func_cfg_t *s_p_func_cfg;

static uint16_t          s_sended_len;
static uint16_t          s_all_send_len;
static uint16_t          s_once_size = 244;
static uint8_t*          s_send_data_buffer;

static cmd_parse_state_t s_parse_state = CHECK_FRAME_L_STATE;


/**
 *****************************************************************************************
 * @brief Function for getting updated firmware information.
 *
 * @param[in]  img_info: Pointer of firmware information
 *****************************************************************************************
 */
static void dfu_m_get_img_info(dfu_img_info_t *img_info)
{
    if(s_p_func_cfg -> dfu_m_get_img_info != NULL)
    {
        s_p_func_cfg -> dfu_m_get_img_info(img_info);
    }
}

/**
 *****************************************************************************************
 * @brief Function for get updated firmware data..
 *
 * @param[in]  addr: Get data address.
 * @param[in]  data: Pointer of get data.
 * @param[in]  len: Get data length
 *****************************************************************************************
 */
static void dfu_m_get_img_data(uint32_t addr, uint8_t *data, uint16_t len)
{
    if(s_p_func_cfg -> dfu_m_get_img_data != NULL)
    {
        s_p_func_cfg -> dfu_m_get_img_data(addr, data, len);
    }
}

/**
 *****************************************************************************************
 * @brief Function for start update firmware.
 *
 * @param[in]  security: Upgrade firmware is encrypted?.
 * @param[in]  run_fw: Whether to run the firmware immediately after the upgrade.
 *****************************************************************************************
 */
static void dfu_m_send_data(uint8_t *data, uint16_t len)
{
    if(s_p_func_cfg -> dfu_m_send_data != NULL)
    {
        s_p_func_cfg -> dfu_m_send_data(data, len);
    }
}

/**
 *****************************************************************************************
 * @brief Function for start update firmware.
 *
 * @param[in]  security: Upgrade firmware is encrypted?.
 * @param[in]  run_fw: Whether to run the firmware immediately after the upgrade.
 *****************************************************************************************
 */
static void dfu_m_event_handler(dfu_m_event_t event, uint8_t pre)
{
    if(s_p_func_cfg -> dfu_m_event_handler != NULL)
    {
        s_p_func_cfg -> dfu_m_event_handler(event, pre);
    }
}

/**
 *****************************************************************************************
 * @brief Function for start update firmware.
 *
 * @param[in]  security: Upgrade firmware is encrypted?.
 * @param[in]  run_fw: Whether to run the firmware immediately after the upgrade.
 *****************************************************************************************
 */
static void dfu_m_cmd_check(void)
{
    uint16_t i = 0;
    for(i=0; i<s_receive_frame.data_len; i++)
    {
        s_receive_check_sum += s_receive_frame.data[i];
    }
    
    if((s_receive_check_sum & 0xffff) == s_receive_frame.check_sum)
    {
        s_cmd_receive_flag = true;
    }
    else
    {
        s_cmd_receive_flag = false;
        dfu_m_event_handler(FRAM_CHECK_ERROR, 0);
    }
}

/**
 *****************************************************************************************
 * @brief Function for start update firmware.
 *
 * @param[in]  security: Upgrade firmware is encrypted?.
 * @param[in]  run_fw: Whether to run the firmware immediately after the upgrade.
 *****************************************************************************************
 */
static void dfu_m_send(uint8_t *data, uint16_t len)
{
    s_send_data_buffer = s_receive_frame.data;
    memcpy(s_send_data_buffer,data,len);
    s_all_send_len = len;
    if(len >= s_once_size)
    {
        s_sended_len = s_once_size;
    }
    else
    {
        s_sended_len = len;
    }

    dfu_m_send_data(s_send_data_buffer,s_sended_len);
}

/**
 *****************************************************************************************
 * @brief Function for start update firmware.
 *
 * @param[in]  security: Upgrade firmware is encrypted?.
 * @param[in]  run_fw: Whether to run the firmware immediately after the upgrade.
 *****************************************************************************************
 */
static void dfu_m_send_frame(uint8_t *data,uint16_t len,uint16_t cmd_type)
{
    uint8_t send_data[RECEIVE_MAX_LEN + 8];
    uint16_t i = 0;
    uint32_t check_sum = 0;
    send_data[0] = CMD_FRAME_HEADER_L;
    send_data[1] = CMD_FRAME_HEADER_H;
    send_data[2] = cmd_type;
    send_data[3] = cmd_type >> 8;
    send_data[4] = len;
    send_data[5] = len >> 8;
    
    for(i=2; i<6; i++)
    {
        check_sum += send_data[i];
    }
    
    for(i=0; i<len; i++)
    {
        send_data[6+i] = *(data+i);
        check_sum += *(data+i);
    }
    send_data[6+len] = check_sum;
    send_data[7+len] = check_sum >> 8;
    dfu_m_send(send_data,len+8);
}

/**
 *****************************************************************************************
 * @brief Function for start update firmware.
 *
 * @param[in]  security: Upgrade firmware is encrypted?.
 * @param[in]  run_fw: Whether to run the firmware immediately after the upgrade.
 *****************************************************************************************
 */
static void dfu_m_program_flash(uint16_t len)
{
    uint16_t i=0;
    s_program_size += len;
    dfu_m_get_img_data(s_page_start_addr, &s_receive_frame.data[7], len);
    for(i=0; i<len; i++)
    {
        s_all_check_sum += s_receive_frame.data[i+7];
    }
    s_receive_frame.data[0] = 0x01;
    s_receive_frame.data[1] = s_page_start_addr;
    s_receive_frame.data[2] = s_page_start_addr>>8;
    s_receive_frame.data[3] = s_page_start_addr>>16;
    s_receive_frame.data[4] = s_page_start_addr>>24;
    
    s_receive_frame.data[5] = len;
    s_receive_frame.data[6] = len>>8;
    dfu_m_send_frame(s_receive_frame.data, len+7, PROGRAME_FLASH);   
    s_page_start_addr += len;    
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
void dfu_m_send_data_cmpl_process(void)
{
    int remain = s_all_send_len - s_sended_len;

    if(remain >= s_once_size)
    {
        dfu_m_send_data(&s_send_data_buffer[s_sended_len], s_once_size);
        s_sended_len += s_once_size;
    }
    else if(remain > 0)
    {
        dfu_m_send_data(&s_send_data_buffer[s_sended_len],remain);
        s_sended_len += remain;
    }
}


void dfu_m_cmd_prase(uint8_t* data,uint16_t len)
{
    uint16_t i = 0;
    
    if(s_cmd_receive_flag == 0)
    {
        for(i=0; i<len; i++)
        {
            switch(s_parse_state)
            {
                case CHECK_FRAME_L_STATE:
                {
                    s_receive_check_sum = 0;
                    if(data[i] == CMD_FRAME_HEADER_L)
                    {
                        s_parse_state = CHECK_FRAME_H_STATE;
                    }
                }
                break;
                
                case CHECK_FRAME_H_STATE:
                {
                    if(data[i] == CMD_FRAME_HEADER_H)
                    {
                        s_parse_state = RECEIVE_CMD_TYPE_L_STATE;
                    } else if(data[i] == CMD_FRAME_HEADER_L) {
                        s_parse_state = CHECK_FRAME_H_STATE;
                    } else {
                        s_parse_state = CHECK_FRAME_L_STATE;
                    }
                }
                break;
                
                case RECEIVE_CMD_TYPE_L_STATE:
                {
                    s_receive_frame.cmd_type = data[i];
                    s_receive_check_sum += data[i];
                    s_parse_state = RECEIVE_CMD_TYPE_H_STATE;
                }
                break;
                
                case RECEIVE_CMD_TYPE_H_STATE:
                {
                    s_receive_frame.cmd_type |= (data[i] << 8);
                    s_receive_check_sum += data[i];
                    s_parse_state = RECEIVE_LEN_L_STATE;
                }
                break;
                
                case RECEIVE_LEN_L_STATE:
                {
                    s_receive_frame.data_len = data[i];
                    s_receive_check_sum += data[i];
                    s_parse_state = RECEIVE_LEN_H_STATE;
                }
                break;
                
                case RECEIVE_LEN_H_STATE:
                {
                    s_receive_frame.data_len |= (data[i] << 8);
                    s_receive_check_sum += data[i];
                    if(s_receive_frame.data_len == 0)
                    {
                        s_parse_state = RECEIVE_CHECK_SUM_L_STATE;
                    }
                    else if(s_receive_frame.data_len >= RECEIVE_MAX_LEN)
                    {
                        s_parse_state = CHECK_FRAME_L_STATE;
                    }
                    else
                    {
                        s_receive_data_count = 0;
                        s_parse_state = RECEIVE_DATA_STATE;
                    }
                }
                break;
                
                case RECEIVE_DATA_STATE:
                {
                    s_receive_frame.data[s_receive_data_count] = data[i];    
                    if(++s_receive_data_count == s_receive_frame.data_len)
                    {
                        s_parse_state = RECEIVE_CHECK_SUM_L_STATE;
                    }
                }
                break;
                
                case RECEIVE_CHECK_SUM_L_STATE:
                {
                    s_receive_frame.check_sum = data[i];
                    s_parse_state = RECEIVE_CHECK_SUM_H_STATE;
                }
                break;
                
                case RECEIVE_CHECK_SUM_H_STATE:
                {
                    s_receive_frame.check_sum |= (data[i] << 8);
                    s_parse_state = CHECK_FRAME_L_STATE;
                    dfu_m_cmd_check();
                }
                break;
                
                default:{s_parse_state=CHECK_FRAME_L_STATE;}break;
            }
        }
    }
}

void dfu_m_init(dfu_m_func_cfg_t *dfu_m_func_cfg, uint16_t once_send_size)
{
    if(once_send_size != 0)
    {
      s_once_size = once_send_size; 
    }

    if(dfu_m_func_cfg != NULL)
    {
        s_p_func_cfg = dfu_m_func_cfg;
    }
}


void dfu_m_program_start(bool security, bool run_fw)
{
    uint16_t img_len = sizeof(dfu_img_info_t);

    s_run_fw_flag = run_fw;
    s_page_start_addr = 0;
    s_all_check_sum = 0;
    s_file_size = 0;
    s_program_size = 0;

    dfu_m_get_img_info(&s_now_img_info);

    if((s_now_img_info.pattern != PATTERN_VALUE) || \
       (s_now_img_info.boot_info.load_addr % FLASH_OP_PAGE_SIZE != 0))
    {
        dfu_m_event_handler(IMG_INFO_CHECK_FAIL, 0);
    }

    s_page_start_addr = (s_now_img_info.boot_info.load_addr & 0xfffff000);

    if(security)//security mode
    {
        s_file_size = (s_now_img_info.boot_info.bin_size + 48 + 856);
    }
    else
    {
        s_file_size = (s_now_img_info.boot_info.bin_size + 48);
    }

    s_receive_frame.data[0] = 0x00;

    memcpy(&s_receive_frame.data[1], &s_now_img_info, img_len);

    dfu_m_send_frame(s_receive_frame.data, img_len+1, PROGRAM_START);
}

void dfu_m_parse_state_reset(void)
{
    s_parse_state = CHECK_FRAME_L_STATE;
    s_cmd_receive_flag   = false;
    s_receive_data_count = 0;
    s_receive_check_sum  = 0;
}

void dfu_m_schedule(dfu_m_rev_cmd_cb_t rev_cmd_cb)
{
    uint8_t pre = 0;

    if(s_cmd_receive_flag)
    {
        if (rev_cmd_cb)
        {
            rev_cmd_cb();
        }

        switch (s_receive_frame.cmd_type)
        {
            case PROGRAM_START:
                if(s_receive_frame.data[0] == ACK_SUCCESS)
                {
                    dfu_m_program_flash(ONCE_SEND_LEN);
                    dfu_m_event_handler(PRO_START_SUCCESS, 0);
                }
                else
                {
                    dfu_m_event_handler(PRO_START_ERROR, 0);
                }
                break;

            case PROGRAME_FLASH:
                if(s_receive_frame.data[0] == ACK_SUCCESS)
                {
                    pre = (s_program_size * 100) / s_file_size;
                    dfu_m_event_handler(PRO_FLASH_SUCCESS, pre);
                    //pro success, precent
                    if(s_program_size == s_file_size)
                    {
                        s_receive_frame.data[0] = s_run_fw_flag;
                        s_receive_frame.data[1] = s_all_check_sum;
                        s_receive_frame.data[2] = s_all_check_sum>>8;
                        s_receive_frame.data[3] = s_all_check_sum>>16;
                        s_receive_frame.data[4] = s_all_check_sum>>24;
                        dfu_m_send_frame(s_receive_frame.data, 5, PROGRAME_END);//progem end
                    }
                    else if(s_program_size + ONCE_SEND_LEN > s_file_size)
                    {
                        dfu_m_program_flash(s_file_size - s_program_size);
                    }
                    else
                    {
                        dfu_m_program_flash(ONCE_SEND_LEN);
                    }
                    
                }
                else
                {
                    dfu_m_event_handler(PRO_FLASH_FAIL, pre);
                }
                break;

            case PROGRAME_END:
                if(s_receive_frame.data[0] == ACK_SUCCESS)
                {
                    dfu_m_event_handler(PRO_END_SUCCESS, 0);
                }
                else
                {
                    dfu_m_event_handler(PRO_END_FAIL, 0);
                }
                break;

            default:
                break;
        }

        s_cmd_receive_flag = 0;
    }
}




