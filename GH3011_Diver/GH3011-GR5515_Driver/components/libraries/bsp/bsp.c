/**
 *****************************************************************************************
 *
 * @file bsp.c
 *
 * @brief Board Support Package Implementation.
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
#include "bsp.h"

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
#if APP_DRIVER_USE_ENABLE
static uint8_t s_uart_tx_buffer[UART_TX_BUFF_SIZE];
#else
static uart_handle_t s_uart_handle;
#endif

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
#if APP_DRIVER_USE_ENABLE
__WEAK void app_key_evt_handler(uint8_t key_id, app_key_click_type_t key_click_type)
{
    UNUSED(key_id);
    UNUSED(key_click_type);
}

__WEAK void app_uart_evt_handler(app_uart_evt_t *p_evt)
{
    UNUSED(p_evt);
}
#endif

#if APP_DRIVER_USE_ENABLE
void bsp_key_init(void)
{
    app_key_gpio_t app_key_inst[5];

    app_key_inst[0].gpio_type    = APP_IO_TYPE_NORMAL;
    app_key_inst[0].gpio_pin     = KEY_UP_PIN;
    app_key_inst[0].trigger_mode = KEY_TRIGGER_MODE;
    app_key_inst[0].pull         = KEY_PULL_MODE;
    app_key_inst[0].key_id       = BSP_KEY_UP_ID;

    app_key_inst[1].gpio_type    = APP_IO_TYPE_NORMAL;
    app_key_inst[1].gpio_pin     = KEY_DOWN_PIN;
    app_key_inst[1].trigger_mode = KEY_TRIGGER_MODE;
    app_key_inst[1].pull         = KEY_PULL_MODE;
    app_key_inst[1].key_id       = BSP_KEY_DOWN_ID;
 
    app_key_inst[2].gpio_type    = APP_IO_TYPE_NORMAL;
    app_key_inst[2].gpio_pin     = KEY_RIGHT_PIN;
    app_key_inst[2].trigger_mode = KEY_TRIGGER_MODE;
    app_key_inst[2].pull         = KEY_PULL_MODE;
    app_key_inst[2].key_id       = BSP_KEY_RIGHT_ID;
 
    app_key_inst[3].gpio_type    = APP_IO_TYPE_NORMAL;
    app_key_inst[3].gpio_pin     = KEY_LEFT_PIN;
    app_key_inst[3].trigger_mode = KEY_TRIGGER_MODE;
    app_key_inst[3].pull         = KEY_PULL_MODE;
    app_key_inst[3].key_id       = BSP_KEY_LEFT_ID;

    app_key_inst[4].gpio_type    = APP_IO_TYPE_AON;
    app_key_inst[4].gpio_pin     = KEY_OK_PIN;
    app_key_inst[4].trigger_mode = KEY_TRIGGER_MODE;
    app_key_inst[4].pull         = KEY_PULL_MODE;
    app_key_inst[4].key_id       = BSP_KEY_OK_ID;

    app_key_init(app_key_inst, 5, app_key_evt_handler);
}
#endif

void bsp_uart_send(uint8_t *p_data, uint16_t length)
{
#if APP_DRIVER_USE_ENABLE
    app_uart_transmit_async(APP_UART_ID, p_data, length);
#else
    hal_uart_transmit(&s_uart_handle, p_data, length, 5000);
#endif
}

void bsp_uart_flush(void)
{
#if APP_DRIVER_USE_ENABLE
    app_uart_flush(APP_UART_ID);
#endif
}

void bsp_uart_init(void)
{
#if APP_DRIVER_USE_ENABLE

    app_uart_tx_buf_t uart_buffer;
    app_uart_params_t uart_param;

    uart_buffer.tx_buf       = s_uart_tx_buffer;
    uart_buffer.tx_buf_size  = UART_TX_BUFF_SIZE;

    uart_param.id                   = APP_UART_ID;
    uart_param.init.baud_rate       = APP_UART_BAUDRATE;
    uart_param.init.data_bits       = UART_DATABITS_8;
    uart_param.init.stop_bits       = UART_STOPBITS_1;
    uart_param.init.parity          = UART_PARITY_NONE;
    uart_param.init.hw_flow_ctrl    = UART_HWCONTROL_NONE;
    uart_param.init.rx_timeout_mode = UART_RECEIVER_TIMEOUT_ENABLE;
    uart_param.pin_cfg.rx.type      = APP_UART_RX_IO_TYPE;
    uart_param.pin_cfg.rx.pin       = APP_UART_RX_PIN;
    uart_param.pin_cfg.rx.mux       = APP_UART_RX_PINMUX;
    uart_param.pin_cfg.rx.pull      = APP_UART_RX_PULL;
    uart_param.pin_cfg.tx.type      = APP_UART_TX_IO_TYPE;
    uart_param.pin_cfg.tx.pin       = APP_UART_TX_PIN;
    uart_param.pin_cfg.tx.mux       = APP_UART_TX_PINMUX;
    uart_param.pin_cfg.tx.pull      = APP_UART_TX_PULL;
    uart_param.use_mode.type        = APP_UART_TYPE_INTERRUPT;

    app_uart_init(&uart_param, app_uart_evt_handler, &uart_buffer);

#else

    gpio_init_t gpio_config = GPIO_DEFAULT_CONFIG;

    gpio_config.mode = GPIO_MODE_MUX;
    gpio_config.pin  = SERIAL_PORT_TX_PIN;
    gpio_config.mux  = SERIAL_PORT_TX_PINMUX;
    hal_gpio_init(SERIAL_PORT_PORT, &gpio_config);

    gpio_config.pin  = SERIAL_PORT_RX_PIN;
    gpio_config.mux  = SERIAL_PORT_RX_PINMUX;
    hal_gpio_init(SERIAL_PORT_PORT, &gpio_config);

    s_uart_handle.p_instance           = SERIAL_PORT_GRP;
    s_uart_handle.init.baud_rate       = SERIAL_PORT_BAUDRATE;
    s_uart_handle.init.data_bits       = UART_DATABITS_8;
    s_uart_handle.init.stop_bits       = UART_STOPBITS_1;
    s_uart_handle.init.parity          = UART_PARITY_NONE;
    s_uart_handle.init.hw_flow_ctrl    = UART_HWCONTROL_NONE;
    s_uart_handle.init.rx_timeout_mode = UART_RECEIVER_TIMEOUT_ENABLE;

    hal_uart_deinit(&s_uart_handle);
    hal_uart_init(&s_uart_handle);
#endif
}

void bsp_led_init(void)
{
#if APP_DRIVER_USE_ENABLE
    app_io_init_t io_init;

    io_init.pin  = LED_NUM_0_IO;
    io_init.mode = APP_IO_MODE_OUT_PUT;
    io_init.pull = APP_IO_PULLDOWN;
    io_init.mux  = APP_IO_MUX_7;
    app_io_init(APP_IO_TYPE_NORMAL, &io_init);

    io_init.pin  = LED_NUM_1_IO;
    io_init.mode = APP_IO_MODE_OUT_PUT;
    io_init.pull = APP_IO_PULLDOWN;
    io_init.mux  = APP_IO_MUX_7;
    app_io_init(APP_IO_TYPE_MSIO, &io_init);
#else
    msio_init_t msio_init = MSIO_DEFAULT_CONFIG;

    msio_init.pin         = LED_NUM_1_IO;
    msio_init.direction   = MSIO_DIRECTION_OUTPUT;
    msio_init.mode        = MSIO_MODE_DIGITAL;
    hal_msio_init(&msio_init);

    gpio_init_t gpio_init = GPIO_DEFAULT_CONFIG;

    gpio_init.mode        = GPIO_MODE_OUTPUT;
    gpio_init.pin         = LED_NUM_0_IO;
    hal_gpio_init(LED_NUM_0_GRP, &gpio_init); 
#endif
}

void bsp_led_open(bsp_led_num_t led_num)
{
    switch (led_num)
    {
        case BSP_LED_NUM_0:
#if APP_DRIVER_USE_ENABLE
            app_io_write_pin(APP_IO_TYPE_NORMAL, LED_NUM_0_IO, APP_IO_PIN_RESET);
#else
            ll_gpio_reset_output_pin(LED_NUM_0_GRP, LED_NUM_0_IO);
#endif
            break;

        case BSP_LED_NUM_1:
#if APP_DRIVER_USE_ENABLE
            app_io_write_pin(APP_IO_TYPE_MSIO, LED_NUM_1_IO, APP_IO_PIN_RESET);
#else
            hal_msio_write_pin(LED_NUM_1_IO, MSIO_PIN_RESET);
#endif
            break;

        default:
            break;
    }
}

void bsp_led_close(bsp_led_num_t led_num)
{
    switch (led_num)
    {
        case BSP_LED_NUM_0:
#if APP_DRIVER_USE_ENABLE
            app_io_write_pin(APP_IO_TYPE_NORMAL, LED_NUM_0_IO, APP_IO_PIN_SET);
#else
            ll_gpio_set_output_pin(LED_NUM_0_GRP, LED_NUM_0_IO);
#endif
            break;

        case BSP_LED_NUM_1:
#if APP_DRIVER_USE_ENABLE
            app_io_write_pin(APP_IO_TYPE_MSIO, LED_NUM_1_IO, APP_IO_PIN_SET);
#else
            hal_msio_write_pin(LED_NUM_1_IO, MSIO_PIN_SET);
#endif
            break;

        default:
            break;
    }
}

 
