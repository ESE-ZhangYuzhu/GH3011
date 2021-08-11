/**
 * @copyright (c) 2003 - 2020, Goodix Co., Ltd. All rights reserved.
 *
 * @file    gh3011_example_comm_pkg.c
 *
 * @brief   example code for gh3011 (condensed  hbd_ctrl lib)
 *
 */

#include "gh3011_example_common.h"


///  uart parse state
typedef enum 
{
    UART_PARSE_STATE_IDLE = 0,
    UART_PARSE_STATE_GOT_MAGIC_0,
    UART_PARSE_STATE_GOT_MAGIC_1,
    UART_PARSE_STATE_GOT_LEN,
    UART_PARSE_STATE_GOT_DATA,
} EM_UART_PARSE_STATE; 


/// communicate type
int8_t ble_comm_type = COMM_TYPE_INVALID_VAL;
int8_t uart_comm_type = COMM_TYPE_INVALID_VAL;

/// buffer & state for uart
uint8_t uart_module_recv_data_buffer[MUTLI_PKG_UART_MAX_LEN];
uint16_t uart_module_recv_index = 0;
uint16_t uart_module_recv_len = 0;
EM_UART_PARSE_STATE uart_module_parse_state = UART_PARSE_STATE_IDLE;



/// gh30x comm pkg init
void gh30x_comm_pkg_init(void)
{
    #if (__USE_GOODIX_APP__)
    ble_comm_type = HBD_SetSendDataFunc(ble_module_send_pkg); // setup ble send data function
    #endif

    #if (__USE_GOODIX_APP__)
    ble_module_repeat_send_timer_init(); // init repeat send data timer
    #endif

    #if (__UART_WITH_GOODIX_TOOLS__)
    uart_comm_type = HBD_SetSendDataFunc(uart_module_send_pkg);  // setup uart send data function
    #endif
}


/// app rx data handler
void gh30x_app_cmd_parse(uint8_t *buffer, uint8_t length) 
{
    gh30x_communicate_parse_handler(ble_comm_type, buffer, length);
}

/// uart rx data handler
void gh30x_uart_cmd_parse(uint8_t *buffer, uint8_t length) 
{
    uint8_t buffer_index = 0;
    for (buffer_index = 0; buffer_index < length; buffer_index++)
    {
        uart_module_handle_recv_byte(buffer[buffer_index]);
    }
}


/// send app pkg
void ble_module_send_pkg(uint8_t string[], uint8_t length) // send value via through profile
{
#if (__NEW_DATA_MULTI_PKG_NUM__)
    static uint8_t ble_module_send_pkg_index = 0; /// send mutli pkg index
    static uint8_t ble_module_send_pkg_buffer[__BLE_PKG_SIZE_MAX__ + 1]; /// send mutli pkg buffer
	if (MUTLI_PKG_CMD_VERIFY(string[0])) // if pkg cmd equal rawdata cmd
    {
        uint8_t new_pkg_index_of_buffer = ble_module_send_pkg_index * (length + MUTLI_PKG_HEADER_LEN);
        // fix ble_module_send_pkg_buffer
        ble_module_send_pkg_buffer[new_pkg_index_of_buffer++] = MUTLI_PKG_MAGIC_0_VAL;
        ble_module_send_pkg_buffer[new_pkg_index_of_buffer++] = MUTLI_PKG_MAGIC_1_VAL;
        ble_module_send_pkg_buffer[new_pkg_index_of_buffer++] = length;
        memcpy(&ble_module_send_pkg_buffer[new_pkg_index_of_buffer], string, length);
        ble_module_send_pkg_buffer[new_pkg_index_of_buffer + length] = MUTLI_PKG_MAGIC_2_VAL;
        ble_module_send_pkg_index ++;

        if ((((ble_module_send_pkg_index + 1) * (length + MUTLI_PKG_HEADER_LEN)) > __BLE_PKG_SIZE_MAX__)
            || (ble_module_send_pkg_index > __NEW_DATA_MULTI_PKG_NUM__))
        {
            ble_module_send_data_via_gdcs(ble_module_send_pkg_buffer, ble_module_send_pkg_index * (length + MUTLI_PKG_HEADER_LEN));
            ble_module_send_pkg_index = 0;
        }
    }
    else
    {
        ble_module_send_data_via_gdcs(string, length);
        ble_module_send_pkg_index = 0;
    }
#else
    ble_module_send_data_via_gdcs(string, length);
#endif
}


#if (__USE_GOODIX_APP__)

static uint8_t mcu_rawdata_buffer[__BLE_MCU_PKG_BUFFER_MAX_LEN__]; 
static uint16_t mcu_rawdata_buffer_len = 0;
static uint16_t mcu_rawdata_buffer_index = 0;
static uint8_t  mcu_rawdata_pkg_index = 0;
static uint8_t  mcu_algo_result_type = 0;
static uint8_t  mcu_algo_result_len = 0;

/// send mcu rawdata pkg repeat
void send_mcu_rawdata_packet_repeat(void)
{
    uint16_t nRes;
    uint16_t gdcs_send_data_len = 0;
    uint8_t m_gdcs_repeat_buffer[__BLE_PKG_SIZE_MAX__];
    gdcs_send_data_len = comm_packaging_rawdata_packet(m_gdcs_repeat_buffer, __BLE_PKG_SIZE_MAX__, 
                    mcu_algo_result_type, mcu_rawdata_buffer_len, mcu_rawdata_pkg_index, mcu_algo_result_len,
                    &mcu_rawdata_buffer[mcu_rawdata_buffer_index], mcu_rawdata_buffer_len - mcu_rawdata_buffer_index);
    mcu_rawdata_buffer_index += (gdcs_send_data_len - DBG_MCU_PKG_HEADER_LEN); /*fixed sub packet_header(8)*/
    nRes = ble_module_send_data_via_gdcs(m_gdcs_repeat_buffer, gdcs_send_data_len);
	if (nRes == GH30X_EXAMPLE_OK_VAL)
    {
        mcu_rawdata_pkg_index ++;   
    }
    else
    {
        EXAMPLE_DEBUG_LOG_L1("ble send fail code: %d\r\n", nRes);
        mcu_rawdata_buffer_len = 0;
    }
    if (mcu_rawdata_buffer_len > mcu_rawdata_buffer_index)
    {
        ble_module_repeat_send_timer_start();
    } 
    else
    {
        ble_module_repeat_send_timer_stop();
        mcu_rawdata_buffer_len = 0;
        mcu_rawdata_buffer_index = 0;
        mcu_rawdata_pkg_index = 0;
        mcu_algo_result_type = 0;
        mcu_algo_result_len = 0;
    }
}

/// send mcu rawdata pkg start
void send_mcu_rawdata_packet_start(uint8_t algo_result_type, uint8_t algo_result[], uint8_t algo_result_len, 
                                                    GS32 rawdata_out[][DBG_MCU_MODE_PKG_LEN], uint8_t rawdata_out_len)
{
    uint8_t rawdata_out_i = 0;
	ble_module_repeat_send_timer_stop();
    memcpy(mcu_rawdata_buffer, algo_result, algo_result_len);
    for (rawdata_out_i = 0; rawdata_out_i < rawdata_out_len; rawdata_out_i++)
    {
        mcu_rawdata_buffer[algo_result_len + rawdata_out_i * DBG_MCU_PKG_RAW_FRAME_LEN + 0]  = GET_BYTE2_FROM_DWORD(rawdata_out[rawdata_out_i][0]);
        mcu_rawdata_buffer[algo_result_len + rawdata_out_i * DBG_MCU_PKG_RAW_FRAME_LEN + 1]  = GET_BYTE1_FROM_DWORD(rawdata_out[rawdata_out_i][0]);
        mcu_rawdata_buffer[algo_result_len + rawdata_out_i * DBG_MCU_PKG_RAW_FRAME_LEN + 2]  = GET_BYTE0_FROM_DWORD(rawdata_out[rawdata_out_i][0]);
        mcu_rawdata_buffer[algo_result_len + rawdata_out_i * DBG_MCU_PKG_RAW_FRAME_LEN + 3]  = GET_BYTE2_FROM_DWORD(rawdata_out[rawdata_out_i][1]);
        mcu_rawdata_buffer[algo_result_len + rawdata_out_i * DBG_MCU_PKG_RAW_FRAME_LEN + 4]  = GET_BYTE1_FROM_DWORD(rawdata_out[rawdata_out_i][1]);
        mcu_rawdata_buffer[algo_result_len + rawdata_out_i * DBG_MCU_PKG_RAW_FRAME_LEN + 5]  = GET_BYTE0_FROM_DWORD(rawdata_out[rawdata_out_i][1]);
        mcu_rawdata_buffer[algo_result_len + rawdata_out_i * DBG_MCU_PKG_RAW_FRAME_LEN + 6]  = GET_BYTE1_FROM_DWORD(rawdata_out[rawdata_out_i][2]);
        mcu_rawdata_buffer[algo_result_len + rawdata_out_i * DBG_MCU_PKG_RAW_FRAME_LEN + 7]  = GET_BYTE0_FROM_DWORD(rawdata_out[rawdata_out_i][2]);
        mcu_rawdata_buffer[algo_result_len + rawdata_out_i * DBG_MCU_PKG_RAW_FRAME_LEN + 8]  = GET_BYTE1_FROM_DWORD(rawdata_out[rawdata_out_i][3]);
        mcu_rawdata_buffer[algo_result_len + rawdata_out_i * DBG_MCU_PKG_RAW_FRAME_LEN + 9]  = GET_BYTE0_FROM_DWORD(rawdata_out[rawdata_out_i][3]);
        mcu_rawdata_buffer[algo_result_len + rawdata_out_i * DBG_MCU_PKG_RAW_FRAME_LEN + 10] = GET_BYTE1_FROM_DWORD(rawdata_out[rawdata_out_i][4]);
        mcu_rawdata_buffer[algo_result_len + rawdata_out_i * DBG_MCU_PKG_RAW_FRAME_LEN + 11] = GET_BYTE0_FROM_DWORD(rawdata_out[rawdata_out_i][4]);
        mcu_rawdata_buffer[algo_result_len + rawdata_out_i * DBG_MCU_PKG_RAW_FRAME_LEN + 12] = GET_BYTE0_FROM_DWORD(rawdata_out[rawdata_out_i][5]);
    }
    mcu_rawdata_buffer_len = (uint16_t)algo_result_len + (uint16_t)rawdata_out_len * DBG_MCU_PKG_RAW_FRAME_LEN;
    mcu_rawdata_buffer_index = 0;
    mcu_rawdata_pkg_index = 0;
    mcu_algo_result_type = algo_result_type;
    mcu_algo_result_len = algo_result_len;
#if (__USE_GOODIX_APP__)    
    send_mcu_rawdata_packet_repeat();
#endif
}

/// send mcu hb mode wear status rawdata pkg
void send_mcu_hb_mode_wear_status_pkg(uint8_t wear_status, GS32 rawdata_dbg[][DBG_MCU_MODE_PKG_LEN], uint8_t rawdata_dbg_len)
{
    uint8_t algo_out_array[MCU_PKG_HB_ALGO_RESULT_LEN] = {0};
    algo_out_array[1] = wear_status;
    algo_out_array[5] = GH30X_EXAMPLE_OK_VAL;
    send_mcu_rawdata_packet_start(MCU_PKG_HB_TYPE_VAL, algo_out_array, MCU_PKG_HB_ALGO_RESULT_LEN, rawdata_dbg, rawdata_dbg_len);
}

/// send mcu hb mode rawdata pkg
void send_mcu_hb_mode_result_pkg(uint8_t wear_status,uint8_t current[3], GS32 rawdata_dbg[][DBG_MCU_MODE_PKG_LEN], uint16_t rawdata_dbg_len)
{
    uint8_t  algo_out_array[MCU_PKG_HB_ALGO_RESULT_LEN] = {0};
    algo_out_array[1] = wear_status;
    algo_out_array[8] = current[0];
    algo_out_array[9] = current[1];
    algo_out_array[10] = current[2];
    send_mcu_rawdata_packet_start(MCU_PKG_HB_TYPE_VAL, algo_out_array, MCU_PKG_HB_ALGO_RESULT_LEN, rawdata_dbg, rawdata_dbg_len);
}
#endif


/// send uart pkg
void uart_module_send_pkg(uint8_t string[], uint8_t length) // send value via through profile
{
    uint8_t uart_module_send_pkg_buffer[MUTLI_PKG_UART_MAX_LEN + MUTLI_PKG_HEADER_LEN]; /// send pkg buffer
    uint8_t uart_module_real_len = (length > MUTLI_PKG_UART_MAX_LEN)? (MUTLI_PKG_UART_MAX_LEN):(length);

    // fix uart_module_send_pkg_buffer
    uart_module_send_pkg_buffer[0] = MUTLI_PKG_MAGIC_0_VAL;
    uart_module_send_pkg_buffer[1] = MUTLI_PKG_MAGIC_1_VAL;
    uart_module_send_pkg_buffer[2] = uart_module_real_len;
    memcpy(&uart_module_send_pkg_buffer[3], string, uart_module_real_len);
    uart_module_send_pkg_buffer[3 + uart_module_real_len] = MUTLI_PKG_MAGIC_2_VAL;

    uart_module_send_data(uart_module_send_pkg_buffer, (uart_module_real_len + MUTLI_PKG_HEADER_LEN));
}

/// handle one byte from uart
void uart_module_handle_recv_byte(uint8_t recv_byte)
{
    switch (uart_module_parse_state)
    {
    case UART_PARSE_STATE_IDLE:
        if (recv_byte == MUTLI_PKG_MAGIC_0_VAL)
        {
            uart_module_parse_state = UART_PARSE_STATE_GOT_MAGIC_0;
        }
        break;
    case UART_PARSE_STATE_GOT_MAGIC_0:
        if (recv_byte == MUTLI_PKG_MAGIC_1_VAL)
        {
            uart_module_parse_state = UART_PARSE_STATE_GOT_MAGIC_1;
        }
        else
        {
            uart_module_parse_state = UART_PARSE_STATE_IDLE;
        }
        break;
    case UART_PARSE_STATE_GOT_MAGIC_1:
        if ((recv_byte > 0) && (recv_byte <= MUTLI_PKG_UART_MAX_LEN))
        {
            uart_module_parse_state = UART_PARSE_STATE_GOT_LEN;
            uart_module_recv_index = 0;
            uart_module_recv_len = recv_byte;
        }
        else
        {
            uart_module_parse_state = UART_PARSE_STATE_IDLE;
        }
        break;
    case UART_PARSE_STATE_GOT_LEN:
        uart_module_recv_data_buffer[uart_module_recv_index] = recv_byte;
        uart_module_recv_index ++;
        if (uart_module_recv_index >= uart_module_recv_len)
        {
            uart_module_parse_state = UART_PARSE_STATE_GOT_DATA;
        }
        break;

    case UART_PARSE_STATE_GOT_DATA:
        if (recv_byte == MUTLI_PKG_MAGIC_2_VAL)
        {
            gh30x_communicate_parse_handler(uart_comm_type, uart_module_recv_data_buffer, uart_module_recv_len);
        }
        uart_module_parse_state = UART_PARSE_STATE_IDLE;
        break;
    default:
        uart_module_parse_state = UART_PARSE_STATE_IDLE;
        break;
    }
}

/********END OF FILE********* Copyright (c) 2003 - 2020, Goodix Co., Ltd. ********/
