/**
 * @copyright (c) 2003 - 2020, Goodix Co., Ltd. All rights reserved.
 *
 * @file    gh3011_example_common.h
 *
 * @brief   example code for gh3011 (condensed  hbd_ctrl lib)
 * 
 * @version  @ref GH30X_EXAMPLE_VER_STRING
 *
 */


#ifndef _GH3011_EXAMPLE_COMMON_H_
#define _GH3011_EXAMPLE_COMMON_H_

/* includes */
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "gh3011_example_config.h"
#include "hbd_ctrl.h"

#if (__USE_GOODIX_APP__)
#include "hbd_communicate.h"
#include "app_cmd_send.h"
#endif

#if (__SYSTEM_TEST_SUPPORT__)
#include "systemTest.h"
#endif

/// version string
#define GH30X_EXAMPLE_VER_STRING            "example code v0.2.0 (For hbd_ctrl lib v0.7.0.2 and later, without algorithm)\r\n"

/// define all dbg mcu mode api
#if (__USE_GOODIX_APP__)
#define BLE_MODULE_SEND_HB(val)           
#define BLE_MODULE_SEND_RRI(val, cnt)                                   
#define SEND_GH30X_RESET_EVT()                                          comm_send_app_cmd_reset(ble_module_send_pkg)
#define SEND_AUTOLED_FAIL_EVT()                                         comm_send_app_cmd_auto_led_fail(ble_module_send_pkg)
#define SEND_MCU_HB_MODE_WEAR_STATUS(sta, data, len)                    send_mcu_hb_mode_wear_status_pkg(sta, data, len)
#define SEND_MCU_HB_MODE_RESULT(sta, current, data, len)                send_mcu_hb_mode_result_pkg(sta, current, data, len)

#else
#define BLE_MODULE_SEND_HB(val)                                         ble_module_send_heartrate(val)
#define BLE_MODULE_SEND_RRI(val, cnt)                                   do { ble_module_add_rr_interval(val, cnt); ble_module_send_heartrate(0); } while (0)
#define SEND_GH30X_RESET_EVT()
#define SEND_AUTOLED_FAIL_EVT()
#define SEND_MCU_HB_MODE_WEAR_STATUS(sta, data, len)
#define SEND_MCU_HB_MODE_RESULT(sta, current, data, len)
#endif

/// define all fifo int timeout timer api
#if (__FIFO_INT_TIMEOUT_CHECK__)
#define HAL_GH30X_FIFO_INT_TIMEOUT_TIMER_INIT()                         hal_gh30x_fifo_int_timeout_timer_init()
#define HAL_GH30X_FIFO_INT_TIMEOUT_TIMER_START()                        hal_gh30x_fifo_int_timeout_timer_start()
#define HAL_GH30X_FIFO_INT_TIMEOUT_TIMER_STOP()                         hal_gh30x_fifo_int_timeout_timer_stop()
#else
#define HAL_GH30X_FIFO_INT_TIMEOUT_TIMER_INIT()                         
#define HAL_GH30X_FIFO_INT_TIMEOUT_TIMER_START()                      
#define HAL_GH30X_FIFO_INT_TIMEOUT_TIMER_STOP() 
#endif

/// ok and err val for some api ret
#define GH30X_EXAMPLE_OK_VAL                (1)
#define GH30X_EXAMPLE_ERR_VAL               (0)

/// autoled error val
#define GH30X_AUTOLED_ERR_VAL               (1)

/// interface type 
#define GH30X_COMMUNICATION_INTERFACE_I2C   (1)
#define GH30X_COMMUNICATION_INTERFACE_SPI   (2)

/// wear status
#define WEAR_STATUS_DETECTING               (0)
#define WEAR_STATUS_WEAR                    (1)
#define WEAR_STATUS_UNWEAR                  (2)
#define WEAR_STATUS_ALMOST_UNWEAR           (3)

/// mutli pkg magic byte
#define MUTLI_PKG_HEADER_LEN                (4)
#define MUTLI_PKG_MAGIC_0_VAL               (0x47)
#define MUTLI_PKG_MAGIC_1_VAL               (0x44)
#define MUTLI_PKG_MAGIC_2_VAL               (0x0A)
#define MUTLI_PKG_CMD_VERIFY(cmd)           ((cmd & 0xF0) == 0x80)
#define MUTLI_PKG_UART_MAX_LEN              (20)

/// mcu pkg 
#define MCU_PKG_HB_TYPE_VAL                 (0)
#define MCU_PKG_HB_ALGO_RESULT_LEN          (11)

/// mcu rawdata cnt
#define DBG_MCU_MODE_PKG_LEN                (6)

/// mcu rawdata frame len
#define DBG_MCU_PKG_RAW_FRAME_LEN           (13)

/// mcu pkg header len
#define DBG_MCU_PKG_HEADER_LEN              (8)

/// comm type val 
#define COMM_TYPE_INVALID_VAL               (0xFF)

/// hbd ret val base
#define DEBUG_HBD_RET_VAL_BASE              (7)

/* debug log control, if enable debug, example_dbg_log must define in platforms code */
#if (__EXAMPLE_DEBUG_LOG_LVL__) // debug level > 0
    /// dbg run mode string
    extern const char dbg_rum_mode_string[][24];
    /// dbg communicate cmd string
    extern const char dbg_comm_cmd_string[][35];
    /// dbg ret val string
    extern const char dbg_ret_val_string[][35];
    /// lv1 log string
    #define   EXAMPLE_DEBUG_LOG_L1(...)       do {\
                                                    char dbg_log_str[__EXAMPLE_LOG_DEBUG_SUP_LEN__] = {0};\
                                                    snprintf(dbg_log_str, __EXAMPLE_LOG_DEBUG_SUP_LEN__, \
                                                            "[gh30x]: "__VA_ARGS__);\
                                                    example_dbg_log(dbg_log_str);\
                                                } while (0)
    #if (__EXAMPLE_DEBUG_LOG_LVL__ > 1) // debug level > 1
        /// lv2 log string
        #define   EXAMPLE_DEBUG_LOG_L2(...)        { EXAMPLE_DEBUG_LOG_L1(__VA_ARGS__); }
        #define   EXAMPLE_LOG_RAWDARA(str, dat, len)    do { \
                                                            EXAMPLE_DEBUG_LOG_L2(str);\
                                                            uint8_t log_index = 0; \
                                                            for (log_index = 0; log_index < len; log_index++) \
                                                            {\
                                                                EXAMPLE_DEBUG_LOG_L2("raw,%ld,%ld,%ld,%ld,%ld,%ld\r\n", \
                                                                            dat[log_index * DBG_MCU_MODE_PKG_LEN + 0], \
                                                                            dat[log_index * DBG_MCU_MODE_PKG_LEN + 1], \
                                                                            dat[log_index * DBG_MCU_MODE_PKG_LEN + 2], \
                                                                            dat[log_index * DBG_MCU_MODE_PKG_LEN + 3], \
                                                                            dat[log_index * DBG_MCU_MODE_PKG_LEN + 4], \
                                                                            dat[log_index * DBG_MCU_MODE_PKG_LEN + 5]); \
                                                            }\
                                                        } while (0)
    #else
        #define   EXAMPLE_DEBUG_LOG_L2(...)
        #define   EXAMPLE_LOG_RAWDARA(str, dat, len)
    #endif
#else   // debug level <= 0
    #define   EXAMPLE_DEBUG_LOG_L1(...)  
    #define   EXAMPLE_DEBUG_LOG_L2(...) 
    #define   EXAMPLE_LOG_RAWDARA(str, dat, len)
#endif

//// get bytes macro
#define   GET_BYTE3_FROM_DWORD(val)     ((uint8_t)((val >> 24) & 0x000000FFU))
#define   GET_BYTE2_FROM_DWORD(val)     ((uint8_t)((val >> 16) & 0x000000FFU))
#define   GET_BYTE1_FROM_DWORD(val)     ((uint8_t)((val >> 8) & 0x000000FFU))
#define   GET_BYTE0_FROM_DWORD(val)     ((uint8_t)((val ) & 0x000000FFU))
#define   GET_HIGH_BYTE_FROM_WORD(val)  ((uint8_t)(((val) >> 8) & 0xFF))
#define   GET_LOW_BYTE_FROM_WORD(val)   ((uint8_t)((val) & 0xFF))

// var
extern bool goodix_app_start_app_mode;
extern uint16_t gh30x_SampleRate;
extern uint8_t gh30x_EnableFifo;
extern uint16_t gh30x_FifoThreshold;
extern const ST_REGISTER *gh30x_reg_config_ptr;
extern uint16_t gh30x_reg_config_len;

// init
extern ST_HBD_INIT_CONFIG_TYPE gh30x_init_config; 

// getrawdata
extern const ST_REGISTER getrawdata_reg_config_array[];
extern const uint8_t getrawdata_reg_config_array_len;

// system test
extern const ST_REGISTER systemtest_led0_reg_config_array[];
extern const uint8_t systemtest_led0_reg_config_array_len;			

extern const ST_REGISTER systemtest_led1_reg_config_array[];
extern const uint8_t systemtest_led1_reg_config_array_len;

extern const ST_REGISTER systemtest_led2_reg_config_array[];
extern const uint8_t systemtest_led2_reg_config_array_len;

#if (__SYSTEM_TEST_SUPPORT__)
extern HBDTEST_ROMAHBData systemtest_led0_os_result;
extern HBDTEST_ROMAHBData systemtest_led1_os_result;
extern HBDTEST_ROMAHBData systemtest_led2_os_result;
extern HBDTEST_ROMALEDCheckData led0std,led1std,led2std;
extern unsigned char ledmask[3];
extern uint8_t goodix_system_test_mode;
void gh30x_systemtest_init(uint8_t mode);
#endif


/// porting api

/// i2c
void hal_gh30x_i2c_init(void);
uint8_t hal_gh30x_i2c_write(uint8_t device_id, const uint8_t write_buffer[], uint16_t length);
uint8_t hal_gh30x_i2c_read(uint8_t device_id, const uint8_t write_buffer[], uint16_t write_length, uint8_t read_buffer[], uint16_t read_length);

/// spi
void hal_gh30x_spi_init(void);
uint8_t hal_gh30x_spi_write(const uint8_t write_buffer[], uint16_t length);
uint8_t hal_gh30x_spi_read(uint8_t read_buffer[], uint16_t length);
void hal_gh30x_spi_cs_set_low(void);
void hal_gh30x_spi_cs_set_high(void);

/// delay
void hal_gh30x_delay_us(uint16_t us_cnt);

/// int
void hal_gh30x_int_init(void);

/// handle result
void handle_getrawdata_mode_result(uint8_t wearing_state_val, GS32 rawdata[][2], uint16_t rawdata_len);
void handle_wear_status_result(uint8_t wearing_state_val);
void handle_system_test_result(uint8_t test_res,uint8_t led_num);
void handle_before_system_os_test(void);

/// ble 
void ble_module_send_heartrate(uint32_t heartrate); // send value via heartrate profile
void ble_module_add_rr_interval(uint16_t rr_interval_arr[], uint8_t cnt); // add value to heartrate profile
uint8_t ble_module_send_data_via_gdcs(uint8_t string[], uint8_t length); // send value via through profile

// ble repeat send timer
void ble_module_repeat_send_timer_start(void);
void ble_module_repeat_send_timer_stop(void);
void ble_module_repeat_send_timer_init(void);

// fifo int timeout
void hal_gh30x_fifo_int_timeout_timer_start(void);
void hal_gh30x_fifo_int_timeout_timer_stop(void);
void hal_gh30x_fifo_int_timeout_timer_init(void);

// uart
uint8_t uart_module_send_data(uint8_t string[], uint8_t length);

// handle cmd
#if (__USE_GOODIX_APP__)
void handle_goodix_communicate_cmd(EM_COMM_CMD_TYPE cmd_type);
#endif

// log
void example_dbg_log(char *log_string);

//For dynamic memory lib only
#if (__HBD_USE_DYN_MEM__)
void *hal_gh30x_memory_malloc(GS32 size);
void hal_gh30x_memory_free(void *pmem);
#endif


/// ble pkg api
void uart_module_handle_recv_byte(uint8_t recv_byte);
void uart_module_send_pkg(uint8_t string[], uint8_t length);
void gh30x_comm_pkg_init(void);
void gh30x_app_cmd_parse(uint8_t *buffer, uint8_t length); // ble recv data parse
void gh30x_uart_cmd_parse(uint8_t *buffer, uint8_t length); // uart recv data parse
void ble_module_send_pkg(uint8_t string[], uint8_t length);
void send_mcu_rawdata_packet_repeat(void);
void send_mcu_hb_mode_wear_status_pkg(uint8_t wear_status, GS32 rawdata_dbg[][DBG_MCU_MODE_PKG_LEN], uint8_t rawdata_dbg_len);
void send_mcu_hb_mode_result_pkg(uint8_t wear_status, uint8_t current[3], GS32 rawdata_dbg[][DBG_MCU_MODE_PKG_LEN], uint16_t rawdata_dbg_len);
/// ctrl api
#if (__HBD_USE_DYN_MEM__)
void gh30x_malloc_memory(void);
void gh30x_free_memory(void);
#endif
void gh30x_Load_new_config(const ST_REGISTER *config_ptr, uint16_t len);
void gh30x_adt_wear_detect_start(const ST_REGISTER *config_ptr, GU16 config_len);
void gh30x_getrawdata_start(void);
uint8_t gh30x_i2c_write_exchange_to_spi(uint8_t device_id, const uint8_t write_buffer[], uint16_t length);
uint8_t gh30x_i2c_read_exchange_to_spi(uint8_t device_id, const uint8_t write_buffer[], uint16_t write_length, uint8_t read_buffer[], uint16_t read_length);
#if (__SYSTEM_TEST_SUPPORT__)
uint8_t gh30x_systemtest_otp_check(void);
void gh30x_systemtest_os_start(uint8_t led_num);
uint8_t gh30x_systemtest_os_calc(uint8_t led_num);
uint8_t gh30x_systemtest_comm_check(void);
void gh30x_systemtest_param_set(uint8_t led_num, HBDTEST_ROMAConfigParam* param);
void gh30x_systemtest_part2_handle(GU8 ret);
#endif

/// process api
void gh30x_start_adt_with_mode(void);
void gh30x_start_func_with_mode(void);
void gh30x_start_func_whithout_adt_confirm(void);
void gh30x_stop_func(void);

void gh30x_communicate_parse_handler(GS8 communicate_type, GU8 *buffer, GU8 length);
void gh30x_int_msg_handler(void); // gh30x int msg handler
void gh30x_fifo_int_timeout_msg_handler(void);

/// hbd ctrl extern api & var
extern void HBD_DelayUs(GU16 usUsCnt);
extern void HBD_I2cSendCmd(GU8 uchCmd);
extern void HBD_I2cWriteReg(GU16 usAddr, GU16 usValue);
extern GU16 HBD_I2cReadReg(GU16 usAddr);
extern GU32 HBD_I2cReadRawdataReg(GU16 usAddr);
extern GU16 g_usReadRawdataRegList[2];

#endif /* _GH3011_EXAMPLE_COMMON_H_ */

/********END OF FILE********* Copyright (c) 2003 - 2020, Goodix Co., Ltd. ********/
