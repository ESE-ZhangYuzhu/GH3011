/**
 * @copyright (c) 2003 - 2020, Goodix Co., Ltd. All rights reserved.
 *
 * @file    gh3011_example_port.c
 *
 * @brief   example code for gh3011 (condensed  hbd_ctrl lib)
 *
 */


#include "gh3011_example_common.h"
#include <stdio.h>
#include <string.h>
#include "gr55xx_hal.h"
#include "boards.h"
#include "bsp.h"
#include "app_log.h"
#include "gr55xx_sys.h"
#include "transport_scheduler.h"
#include "app_tim.h"
#include "app_i2c.h"
#include "app_gpiote.h"
#include "gus.h"
#include "app_spi.h"






/****************************************************************************************************************
==================================================I2C接口=======================================================
****************************************************************************************************************/

#if ( __GH30X_COMMUNICATION_INTERFACE__ ==   GH30X_COMMUNICATION_INTERFACE_I2C)


/* gh30x i2c interface */
#define  MASTER_DEV_ADDR    0x28


#define  GR551X_IIC_ID          APP_I2C_ID_1
#define  IIC_IO_TYPE            APP_IO_TYPE_NORMAL
#define  IIC_IO_MUX             APP_IO_MUX_1

#define  GH30X_INT_TYPE         APP_IO_TYPE_NORMAL
#define  GH30X_INT_PIN          APP_IO_PIN_7



#define  I2C_SDA_PIN          APP_IO_PIN_8
#define  I2C_SCL_PIN          APP_IO_PIN_9
#define  RESET_GH3011_PIN     APP_IO_PIN_10


#define DEFAULT_IO_CONFIG                                                \
	{                                                                    \
		{IIC_IO_TYPE, IIC_IO_MUX, I2C_SCL_PIN, APP_IO_NOPULL},  \
		{                                                                \
			IIC_IO_TYPE, IIC_IO_MUX, I2C_SDA_PIN, APP_IO_NOPULL \
		}                                                                \
	}
	
	
#define DEFAULT_MODE_CONFIG                          \
	{                                                \
		APP_I2C_TYPE_DMA, DMA_Channel0, DMA_Channel1 \
	}
	
	
#define DEFAULT_I2C_CONFIG                                                                \
	{                                                                                     \
		I2C_SPEED_400K, MASTER_DEV_ADDR, I2C_ADDRESSINGMODE_7BIT, I2C_GENERALCALL_DISABLE \
	}
	
	
#define DEFAULT_PARAM_CONFIG                                                                          \
	{                                                                                                 \
		GR551X_IIC_ID, APP_I2C_ROLE_MASTER, DEFAULT_IO_CONFIG, DEFAULT_MODE_CONFIG, DEFAULT_I2C_CONFIG \
	}

#else
/****************************************************************************************************************
==================================================SPI接口=======================================================
****************************************************************************************************************/


/* int  APP_IO_TYPE_AON  APP_IO_TYPE_NORMAL  */
#define GH30X_INT_PIN    APP_IO_PIN_30
#define GH30X_INT_TYPE   APP_IO_TYPE_NORMAL



#define SPI_IO_TYPE   APP_IO_TYPE_NORMAL
#define SPI_IO_MUX    APP_IO_MUX_0


#define SPI_CS      APP_IO_PIN_17
#define SPI_CLK     APP_IO_PIN_24
#define SPI_MOSI    APP_IO_PIN_25
#define SPI_MISO    APP_IO_PIN_14
#define RESET_GH3011_PIN     APP_IO_PIN_31





#define SPI_DEFAULT_IO_CONFIG                                                                \
	{                                                                                \
		  {APP_IO_TYPE_NORMAL, SPI_IO_MUX, SPI_CS, APP_IO_NOPULL, APP_SPI_PIN_DISABLE}, \
			{APP_IO_TYPE_NORMAL, SPI_IO_MUX, SPI_CLK, APP_IO_NOPULL, APP_SPI_PIN_ENABLE},  \
			{APP_IO_TYPE_NORMAL, SPI_IO_MUX, SPI_MOSI, APP_IO_NOPULL, APP_SPI_PIN_ENABLE}, \
      {APP_IO_TYPE_NORMAL, SPI_IO_MUX, SPI_MISO, APP_IO_NOPULL, APP_SPI_PIN_ENABLE} }                                                         
                                                                            		

 
#define SPI_DEFAULT_MODE_CONFIG                      \
	{                                                \
		APP_SPI_TYPE_DMA, DMA_Channel0, DMA_Channel1 \
	}
#define SPI_DEFAULT_SPI_CONFIG                                                                                                    \
	{                                                                                                                             \
		SPI_DATASIZE_8BIT, SPI_POLARITY_LOW, SPI_PHASE_1EDGE, (SystemCoreClock / 4000000), SPI_TIMODE_DISABLE, SPI_SLAVE_SELECT_0 \
	}
#define SPI_DEFAULT_PARAM_CONFIG                                                                  \
	{                                                                                             \
		APP_SPI_ID_MASTER, SPI_DEFAULT_IO_CONFIG, SPI_DEFAULT_MODE_CONFIG, SPI_DEFAULT_SPI_CONFIG \
	}

//SPI_CS
const app_gpiote_param_t CS_IO_param[] =
	{
		{SPI_IO_TYPE, SPI_CS, APP_IO_MODE_OUT_PUT, APP_IO_PULLUP, APP_IO_NONE_WAKEUP, NULL},
};

#endif




/****************************************************************************************************************
**函数名：HW_ResetGH3011
**输入输出：
**
*****************************************************************************************************************/
void ResetGH3011ByRestPin(void)
{

	app_io_init_t GPIO_Pin; // = GPIO_DEFAULT_CONFIG;
	
	app_io_write_pin(APP_IO_TYPE_NORMAL, RESET_GH3011_PIN, APP_IO_PIN_RESET);
	// key init
	GPIO_Pin.pin = RESET_GH3011_PIN;
	GPIO_Pin.pull = APP_IO_NOPULL;
	GPIO_Pin.mode = APP_IO_MODE_OUT_PUT;
	GPIO_Pin.mux = APP_IO_MUX_7;
	app_io_init(APP_IO_TYPE_NORMAL, &GPIO_Pin);
	
	
	app_io_write_pin(APP_IO_TYPE_NORMAL, RESET_GH3011_PIN, APP_IO_PIN_RESET);
  hal_gh30x_delay_us(4000);
  app_io_write_pin(APP_IO_TYPE_NORMAL, RESET_GH3011_PIN, APP_IO_PIN_SET);
  hal_gh30x_delay_us(2000);
}

/****************************************************************************************************************
================================================I2C接口==========================================================
*****************************************************************************************************************/
#if (GH30X_COMMUNICATION_INTERFACE_I2C == __GH30X_COMMUNICATION_INTERFACE__)


volatile uint8_t g_tx_done = 0;
volatile uint8_t g_rx_done = 0;
volatile uint8_t g_tx_rx_error = 0;




//中断类型响应函数
void app_i2c_evt_handler(app_i2c_evt_t * p_evt)
{
    switch (p_evt->type)
    {
        case APP_I2C_EVT_ERROR:         //I2C数据传输错误，传输结束，并置位错误标志
            g_tx_done = 1;
            g_rx_done = 1;
            g_tx_rx_error = 1;
            break;

        case APP_I2C_EVT_TX_CPLT:       //I2C数据发送完成
            g_tx_done = 1;
            g_tx_rx_error = 0;
            break;

        case APP_I2C_EVT_RX_DATA:       //I2C数据接收完成
            g_rx_done = 1;
            g_tx_rx_error = 0;
            break;
    }
}


/// i2c for gh30x init
void hal_gh30x_i2c_init(void)
{
	// code implement by user
	app_i2c_params_t params_t = DEFAULT_PARAM_CONFIG;

	params_t.use_mode.type = APP_I2C_TYPE_INTERRUPT;
	if (app_i2c_init(&params_t, app_i2c_evt_handler) != 0)
	{
		EXAMPLE_DEBUG_LOG_L1("i2c initial failed! Please check the input paraments.\r\n");
		return ;
	}
}

/// i2c for gh30x wrtie
uint8_t hal_gh30x_i2c_write(uint8_t device_id, const uint8_t write_buffer[], uint16_t length)
{
	uint8_t ret = GH30X_EXAMPLE_OK_VAL;
	// code implement by user
	if (app_i2c_transmit_sync(GR551X_IIC_ID, device_id >> 1, (uint8_t *) write_buffer, length, 100)) //判断传输是否成功，如果失败则返回，或进行其他处理
	{
		EXAMPLE_DEBUG_LOG_L1("i2c transmit failed! \r\n");
		return GH30X_EXAMPLE_ERR_VAL;
	}
	return ret;
}

/// i2c for gh30x read
uint8_t hal_gh30x_i2c_read(uint8_t device_id, const uint8_t write_buffer[], uint16_t write_length, uint8_t read_buffer[], uint16_t read_length)
{
	uint8_t ret = GH30X_EXAMPLE_OK_VAL;
	// code implement by user
	uint8_t i2c_addr = device_id >> 1;
	if (app_i2c_transmit_sync(GR551X_IIC_ID, i2c_addr, (uint8_t *) write_buffer, write_length, 100)) //判断传输是否成功，如果失败则返回，或进行其他处理
	{
		EXAMPLE_DEBUG_LOG_L1("i2c transmit failed! \r\n");
		return GH30X_EXAMPLE_ERR_VAL;
	}

	if (app_i2c_receive_sync(GR551X_IIC_ID, i2c_addr, read_buffer, read_length, 100))//判断传输是否成功，如果失败则返回，或进行其他处理
	{
		EXAMPLE_DEBUG_LOG_L1("i2c receive failed! \r\n");
		return GH30X_EXAMPLE_ERR_VAL;
	}
	return ret;
}

#else


/****************************************************************************************************************
================================================SPI接口==========================================================
*****************************************************************************************************************/





/****************************************************************************************************************
**函数名：app_spi_callback
**输入输出：
**
*****************************************************************************************************************/
volatile uint8_t g_master_tdone = 0;
volatile uint8_t g_master_rdone = 0;
void app_spi_callback(app_spi_evt_t *p_evt)
{
	if (p_evt->type == APP_SPI_EVT_TX_CPLT)
	{
		g_master_tdone = 1;
	}
	if (p_evt->type == APP_SPI_EVT_RX_DATA)
	{
		g_master_rdone = 1;
	}
	if (p_evt->type == APP_SPI_EVT_ERROR)
	{
		g_master_tdone = 1;
		g_master_rdone = 1;
	}
}


/****************************************************************************************************************
**函数名：hal_gh30x_spi_init
**输入输出：
**
*****************************************************************************************************************/

void hal_gh30x_spi_init(void)
{
	uint16_t ret;

	app_spi_params_t spi_params = SPI_DEFAULT_PARAM_CONFIG;

	spi_params.use_mode.type = APP_SPI_TYPE_INTERRUPT;
	ret = app_spi_init(&spi_params, app_spi_callback);

	if (ret != 0)
	{

		EXAMPLE_DEBUG_LOG_L1("\r\n SPIM initial failed! \r\n");
	}
	else
	{
		EXAMPLE_DEBUG_LOG_L1("\r\n SPIM initial OK!\r\n");
	}
	
   hal_gh30x_spi_cs_set_high();
	 app_gpiote_init(CS_IO_param, sizeof(CS_IO_param) / sizeof(app_gpiote_param_t));
	 hal_gh30x_spi_cs_set_high();
	
	hal_gh30x_delay_us(10000);
}

/****************************************************************************************************************
**函数名：hal_gh30x_spi_write
**输入输出：
**
*****************************************************************************************************************/
uint8_t hal_gh30x_spi_write(const uint8_t write_buffer[], uint16_t length)
{
	uint8_t ret = GH30X_EXAMPLE_OK_VAL;
	// code implement by user
	if (app_spi_transmit_sync(APP_SPI_ID_MASTER, (uint8_t *)write_buffer, length, 100)) //判断传输是否成功，如果失败则返回，或进行其他处理
	{
		EXAMPLE_DEBUG_LOG_L1("SPI transmit failed! \r\n");
		return GH30X_EXAMPLE_ERR_VAL;
	}
	return ret;
}

/****************************************************************************************************************
**函数名：hal_gh30x_spi_read
**输入输出：
**
*****************************************************************************************************************/
uint8_t hal_gh30x_spi_read(uint8_t read_buffer[], uint16_t length)
{
	uint8_t ret = GH30X_EXAMPLE_OK_VAL;
	// code implement by user
	if (app_spi_receive_sync(APP_SPI_ID_MASTER, (uint8_t *)read_buffer, length, 100)) //判断传输是否成功，如果失败则返回，或进行其他处理
	{
		EXAMPLE_DEBUG_LOG_L1("SPI receive failed! \r\n");
		return GH30X_EXAMPLE_ERR_VAL;
	}
	return ret;
}

/****************************************************************************************************************
**函数名：hal_gh30x_spi_cs_set_low
**输入输出：
**
*****************************************************************************************************************/
void hal_gh30x_spi_cs_set_low(void)
{
	// code implement by user
	app_io_write_pin(SPI_IO_TYPE, SPI_CS, APP_IO_PIN_RESET);
}

/****************************************************************************************************************
**函数名：hal_gh30x_spi_cs_set_high
**输入输出：
**
*****************************************************************************************************************/
void hal_gh30x_spi_cs_set_high(void)
{
	// code implement by user
	app_io_write_pin(SPI_IO_TYPE, SPI_CS, APP_IO_PIN_SET);
}



#endif



















/* delay */

/// delay us
void hal_gh30x_delay_us(uint16_t us_cnt)
{
	// code implement by user
	delay_us(us_cnt);
}

/* gsensor driver */

/// gsensor motion detect mode flag
bool gsensor_drv_motion_det_mode = false;

/// gsensor init
int8_t gsensor_drv_init(void)
{
	int8_t ret = GH30X_EXAMPLE_OK_VAL;
	gsensor_drv_motion_det_mode = false;
	// code implement by user
	/* if enable all func equal 25Hz, should config > 25Hz;
	but if enable have 100hz, should config to > 100hz. if not, feeback to GOODIX!!!
	*/
	return ret;
}

/// gsensor enter normal mode
void gsensor_drv_enter_normal_mode(void)
{
	// code implement by user
	gsensor_drv_motion_det_mode = false;
}

/// gsensor enter fifo mode
void gsensor_drv_enter_fifo_mode(void)
{
	// code implement by user
	gsensor_drv_motion_det_mode = false;
}

/// gsensor enter motion det mode
void gsensor_drv_enter_motion_det_mode(void)
{
	// code implement by user
	/* if enable motion det mode that call @ref hal_gsensor_drv_int1_handler when motion generate irq
		e.g. 1. (hardware) use gsensor motion detect module with reg config
			 2. (software) gsensor enter normal mode, then define 30ms timer get gsensor rawdata,
			 	if now total acceleration(sqrt(x*x+y*y+z*z)) - last total acceleration >= 30 (0.05g @512Lsb/g) as motion
				generate that call @ref hal_gsensor_drv_int1_handler
	*/
	gsensor_drv_motion_det_mode = true;
}




/***********************************************************************************************************************
**
**
*************************************************************************************************************************/
void hal_gh30x_int_handler(void)
{
    gh30x_int_msg_handler();
}

void gh30x_int_pin_handler(app_gpiote_evt_t *p_evt)
{
	app_io_pin_state_t pin_level = APP_IO_PIN_RESET;

    switch(p_evt->type)
    {
        case GH30X_INT_TYPE:
            pin_level = app_io_read_pin(GH30X_INT_TYPE, p_evt->pin);
            if (pin_level == APP_IO_PIN_SET)
            {
                hal_gh30x_int_handler();
            }
            break;

        default:
            break;
    }
}

/// gh30x int pin init, should config as rise edge trigger
void hal_gh30x_int_init(void)
{

	const app_gpiote_param_t param[] =
    {
        {GH30X_INT_TYPE, GH30X_INT_PIN, APP_IO_MODE_IT_RISING, APP_IO_NOPULL, APP_IO_ENABLE_WAKEUP, gh30x_int_pin_handler},
    };
    app_gpiote_init(param, sizeof (param) / sizeof (app_gpiote_param_t));
}


/***********************************************************************************************************************
**
**
*************************************************************************************************************************/


/// gsensor int1 init, should config as rise edge trigger
void hal_gsensor_int1_init(void)
{
	// code implement by user
	// must register func hal_gsensor_drv_int1_handler as callback
    
	/* if using gsensor fifo mode,
	and gsensor fifo depth is not enough to store 1 second data,
	please connect gsensor fifo interrupt to the host,
	or if using gsensor motion detect mode(e.g  motion interrupt response by 0.5G * 5counts),
	and implement this function to receive gsensor interrupt.
	*/ 
}


/// handle wear status result
void handle_wear_status_result(uint8_t wearing_state_val)
{
	// code implement by user
}

/* handle result */
/// handle getrawdata only mode result
void handle_getrawdata_mode_result(uint8_t wearing_state_val, GS32 rawdata_ptr[][2], uint16_t rawdata_len)
{
    // code implement by user
}




/// handle wear status result, otp_res: <0=> ok , <1=> err , <2=> para err
void handle_system_test_otp_check_result(uint8_t otp_res)
{
	// code implement by user
	EXAMPLE_DEBUG_LOG_L1("otp res: %d\r\n", otp_res);
}

/// handle wear status result, led_num: {0-2};os_res: <0=> ok , <1=> rawdata err , <2=> noise err , <3=> para err
void handle_system_test_os_result(uint8_t led_num, uint8_t os_res)
{
	// code implement by user
	EXAMPLE_DEBUG_LOG_L1("os result%d, %d\r\n", led_num, os_res);
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
	gus_tx_data_send(0, string, length);
	return ret;
}

/// recv data via through profile
void ble_module_recv_data_via_gdcs(uint8_t *data, uint8_t length)
{
	gh30x_app_cmd_parse(data, length);
	
}


/* timer */
#define TIM1_PARAM           { APP_TIM_ID_1, { (SystemCoreClock - 1) + (SystemCoreClock / 12 - 1) }}

/// gh30x fifo int timeout timer handler
void hal_gh30x_fifo_int_timeout_timer_handler(void)
{
	gh30x_fifo_int_timeout_msg_handler();
}
void hal_gh30x_fifo_int_timeout_timer_handler_s(app_tim_evt_t *p_evt)
{
	hal_gh30x_fifo_int_timeout_timer_handler();
}

/// gh30x fifo int timeout timer start
void hal_gh30x_fifo_int_timeout_timer_start(void)
{
    // code implement by user
	app_tim_start(APP_TIM_ID_1);
}

/// gh30x fifo int timeout timer stop
void hal_gh30x_fifo_int_timeout_timer_stop(void)
{
    // code implement by user
	app_tim_stop(APP_TIM_ID_1);
}

/// gh30x fifo int timeout timer init 
void hal_gh30x_fifo_int_timeout_timer_init(void)
{
	// code implement by user
	// must register func gh30x_fifo_int_timeout_timer_handler as callback
	/* should setup timer interval with fifo int freq(e.g. 1s fifo int setup 1080ms timer)
	*/
	app_tim_params_t p_params_tim1 = TIM1_PARAM;
	app_tim_init(&p_params_tim1, (app_tim_evt_handler_t)hal_gh30x_fifo_int_timeout_timer_handler_s);
}

#if ((__USE_GOODIX_APP__) && (__ALGO_CALC_WITH_DBG_DATA__))
/// ble repeat send data timer handler
void ble_module_repeat_send_timer_handler(void)
{
#if (__USE_GOODIX_APP__)
    send_mcu_rawdata_packet_repeat();
#endif
}

void ble_module_repeat_send_timer_handler_s(app_tim_evt_t *p_evt)
{
	ble_module_repeat_send_timer_handler();
}

/// ble repeat send data timer start
void ble_module_repeat_send_timer_start(void)
{
    // code implement by user
	app_tim_start(APP_TIM_ID_0);
}

/// ble repeat send data timer stop
void ble_module_repeat_send_timer_stop(void)
{
    // code implement by user
	app_tim_stop(APP_TIM_ID_0);
}

#define TIM0_PARAM           { APP_TIM_ID_0, { (SystemCoreClock / 10 - 1) }}

/// ble repeat send data timer init 
void ble_module_repeat_send_timer_init(void)
{
    // code implement by user
    // must register func ble_module_repeat_send_timer_handler as callback
	/* should setup 100ms timer and ble connect interval should < 100ms
	*/
	app_tim_params_t p_params_tim0 = TIM0_PARAM;
	app_tim_init(&p_params_tim0, (app_tim_evt_handler_t)ble_module_repeat_send_timer_handler_s);
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
	ble_to_uart_push(string, length);
	return ret;
}


/* handle cmd with all ctrl cmd @ref EM_COMM_CMD_TYPE */
//void handle_goodix_communicate_cmd(EM_COMM_CMD_TYPE cmd_type)
//{
//	// code implement by user
//}


/* log */

/// print dbg log
void example_dbg_log(char *log_string)
{
	// code implement by user
	APP_LOG_RAW_INFO(log_string);
}

#if (__HBD_USE_DYN_MEM__)
void *hal_gh30x_memory_malloc(int size)
{
    // code implement by user    
}

void hal_gh30x_memory_free(void *pmem)
{
    // code implement by user
}
#endif
/********END OF FILE********* Copyright (c) 2003 - 2020, Goodix Co., Ltd. ********/
