/**
 * @copyright (c) 2003 - 2020, Goodix Co., Ltd. All rights reserved.
 *
 * @file    gh3011_example_port.c
 *
 * @brief   example code for gh3011 (condensed  hbd_ctrl lib)
 *
 */


#include "gh3011_example_common.h"


/* gh30x i2c interface */

/// i2c for gh30x init
void hal_gh30x_i2c_init(void)
{
	// code implement by user
}

/// i2c for gh30x wrtie
uint8_t hal_gh30x_i2c_write(uint8_t device_id, const uint8_t write_buffer[], uint16_t length)
{
	uint8_t ret = GH30X_EXAMPLE_OK_VAL;
	// code implement by user
	return ret;
}

/// i2c for gh30x read
uint8_t hal_gh30x_i2c_read(uint8_t device_id, const uint8_t write_buffer[], uint16_t write_length, uint8_t read_buffer[], uint16_t read_length)
{
	uint8_t ret = GH30X_EXAMPLE_OK_VAL;
	// code implement by user
	return ret;
}

/* gh30x spi interface */

/// spi for gh30x init
void hal_gh30x_spi_init(void)
{
	// code implement by user
}

/// spi for gh30x wrtie
uint8_t hal_gh30x_spi_write(const uint8_t write_buffer[], uint16_t length)
{
	uint8_t ret = 1;
	// code implement by user
	return ret;
}

/// spi for gh30x read
uint8_t hal_gh30x_spi_read(uint8_t read_buffer[], uint16_t length)
{
	uint8_t ret = 1;
	// code implement by user
	return ret;
}

/// spi cs set low for gh30x
void hal_gh30x_spi_cs_set_low(void)
{
	// code implement by user
}

/// spi cs set high for gh30x
void hal_gh30x_spi_cs_set_high(void)
{
	// code implement by user
}


/* delay */

/// delay us
void hal_gh30x_delay_us(uint16_t us_cnt)
{
	// code implement by user
}

/* int */

/// gh30x int handler
void hal_gh30x_int_handler(void)
{
    gh30x_int_msg_handler();
}

/// gh30x int pin init, should config as rise edge trigger
void hal_gh30x_int_init(void)
{
	// code implement by user
    // must register func hal_gh30x_int_handler as callback
}


/* handle result */
/// handle getrawdata only mode result
void handle_getrawdata_mode_result(uint8_t wearing_state_val, GS32 rawdata_ptr[][2], uint16_t rawdata_len)
{
    // code implement by user
}

/// handle wear status result
void handle_wear_status_result(uint8_t wearing_state_val)
{
	// code implement by user
}

/// handle wear status result: {0-11}, led_num: {0-2};
///test_res: <0=> ok , <1=> order err , <2=> comm read err,<3=> comm write err,<4=> otp read err,<5=> ctr not pass
///<6=> rawdata not pass , <7=> noise not pass , <8=> leak not pass, <9=> leakratio not pass,<10=> resource error
void handle_system_test_result(uint8_t test_res,uint8_t led_num)
{
	// code implement by user
}

void handle_before_system_os_test()
{
    //code implement by user
    EXAMPLE_DEBUG_LOG_L1("begin new term test");
    DELAY_MS(4000);
}

/* ble */

/// send value via heartrate profile
void ble_module_send_heartrate(uint32_t heartrate)
{
	// code implement by user
}

/// add value to heartrate profile
void ble_module_add_rr_interval(uint16_t rr_interval_arr[], uint8_t cnt)
{
	// code implement by user
}

/// send string via through profile
uint8_t ble_module_send_data_via_gdcs(uint8_t string[], uint8_t length)
{
	uint8_t ret = GH30X_EXAMPLE_OK_VAL;
	// code implement by user
	return ret;
}

/// recv data via through profile
void ble_module_recv_data_via_gdcs(uint8_t *data, uint8_t length)
{
	gh30x_app_cmd_parse(data, length);
}


/* timer */

/// gh30x fifo int timeout timer handler
void hal_gh30x_fifo_int_timeout_timer_handler(void)
{
	gh30x_fifo_int_timeout_msg_handler();
}

/// gh30x fifo int timeout timer start
void hal_gh30x_fifo_int_timeout_timer_start(void)
{
    // code implement by user
}

/// gh30x fifo int timeout timer stop
void hal_gh30x_fifo_int_timeout_timer_stop(void)
{
    // code implement by user
}

/// gh30x fifo int timeout timer init 
void hal_gh30x_fifo_int_timeout_timer_init(void)
{
	// code implement by user
	// must register func gh30x_fifo_int_timeout_timer_handler as callback
	/* should setup timer interval with fifo int freq(e.g. 1s fifo int setup 1080ms timer)
	*/
}

#if ((__USE_GOODIX_APP__) && (__ALGO_CALC_WITH_DBG_DATA__))
/// ble repeat send data timer handler
void ble_module_repeat_send_timer_handler(void)
{
    send_mcu_rawdata_packet_repeat();
}

/// ble repeat send data timer start
void ble_module_repeat_send_timer_start(void)
{
    // code implement by user
}

/// ble repeat send data timer stop
void ble_module_repeat_send_timer_stop(void)
{
    // code implement by user
}

/// ble repeat send data timer init 
void ble_module_repeat_send_timer_init(void)
{
    // code implement by user
    // must register func ble_module_repeat_send_timer_handler as callback
	/* should setup 100ms timer and ble connect interval should < 100ms*/
}
#endif

/* uart */

/// recv data via uart
void uart_module_recv_data(uint8_t *data, uint8_t length)
{
	gh30x_uart_cmd_parse(data, length);
}

/// send value via uart
uint8_t uart_module_send_data(uint8_t string[], uint8_t length)
{
	uint8_t ret = GH30X_EXAMPLE_OK_VAL;
	// code implement by user
	return ret;
}


/* handle cmd with all ctrl cmd @ref EM_COMM_CMD_TYPE */
void handle_goodix_communicate_cmd(EM_COMM_CMD_TYPE cmd_type)
{
	// code implement by user
}


/* log */

/// print dbg log
void example_dbg_log(char *log_string)
{
	// code implement by user
}

#if (__HBD_USE_DYN_MEM__)
void *hal_gh30x_memory_malloc(GS32 size)
{
    // code implement by user    
}

void hal_gh30x_memory_free(void *pmem)
{
    // code implement by user
}
#endif

/********END OF FILE********* Copyright (c) 2003 - 2020, Goodix Co., Ltd. ********/
