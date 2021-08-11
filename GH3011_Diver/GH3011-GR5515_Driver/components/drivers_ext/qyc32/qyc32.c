/**
 ****************************************************************************************
 *
 * @file qyc32.c
 *
 * @brief TFT display controller driver of qiyun GDEW029C32 IC.
 *
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
 *****************************************************************************************
 */
#include "qyc32_config.h"

#ifdef QYC32_IC_DRIVER

#include "gr55xx_delay.h"
#include "qyc32.h"

#define QYC32_BUFFER_X_MAX      296
#define QYC32_BUFFER_Y_MAX      16

static uint8_t qyc32_buffer[QYC32_BUFFER_X_MAX][QYC32_BUFFER_Y_MAX];

static void qyc32_delay2(uint32_t xms)
{
    uint32_t i = 0 , j = 0;

    for(j = 0;j < xms; j++)
    {
        for(i = 0; i < 256; i++);
    }
}

static void qyc32_tft_reset(void)
{
    QYC32_RESET_LOW();
    delay_ms(100);
    QYC32_RESET_HIGH();
    delay_ms(100);
}

static void qyc32_tft_check(void)
{
    app_io_pin_state_t gpio_state;

    while(1)
    {
        gpio_state = app_io_read_pin(APP_IO_TYPE_NORMAL, QYC32_GPOI_BUSY);
        if (gpio_state == 1)
            break;
    }
    delay_ms(100);
}

static void qyc32_tft_init(void)
{
    qyc32_tft_reset();
    qyc32_tft_check();
}

bool qyc32_is_busy(void)
{
    delay_ms(5);

    return (bool)(app_io_read_pin(APP_IO_TYPE_NORMAL, QYC32_GPOI_BUSY) == 0);
}

static bool qyc32_display(uint8_t *data, uint32_t size)
{
    qyc32_tft_init();

    qyc32_write_cmd(QYC32_REG_PWRS);
    qyc32_write_data(0x03);
    qyc32_write_data(0x02);
    qyc32_write_data(0x0D);
    qyc32_write_data(0x0D);
    qyc32_write_data(0x0D);

    qyc32_write_cmd(QYC32_REG_PSR);
    qyc32_write_data(0x9F);
    qyc32_write_data(0x06);

    qyc32_write_cmd(QYC32_REG_PFS);
    qyc32_write_data(0x00);

    qyc32_write_cmd(QYC32_REG_BTST);
    qyc32_write_data(0x17);
    qyc32_write_data(0x17);
    qyc32_write_data(0x17);

    qyc32_write_cmd(QYC32_REG_CDI);
    qyc32_write_data(0x97);

    qyc32_write_cmd(QYC32_REG_TCON);
    qyc32_write_data(0x22);

    qyc32_write_cmd(QYC32_REG_DTM2);
    qyc32_write_buffer(data, size);

    qyc32_write_cmd(QYC32_REG_PON);
    qyc32_tft_check();

    qyc32_write_cmd(QYC32_REG_DRF);
    qyc32_tft_check();

    qyc32_write_cmd(QYC32_REG_PWRO);
    qyc32_tft_check();
    qyc32_delay2(10);

// dbg : deep sleep config
    qyc32_write_cmd(QYC32_REG_DSLP);
    qyc32_write_data(0xA5);

    return true;
}

void qyc32_gui_fill_mem(uint8_t data)
{
    uint16_t x;
    uint16_t y;

    for(x = 0; x < QYC32_BUFFER_X_MAX; x++)
    {
        for(y = 0; y < QYC32_BUFFER_Y_MAX; y++)
        {
            qyc32_buffer[x][y] = ~data;
        }
    }
}

void qyc32_gui_point(uint16_t x, uint16_t y, uint8_t color)
{
    if (x > QYC32_BUFFER_X_MAX - 1 || y > QYC32_BUFFER_Y_MAX * 8 - 1 || color > 1)
    {
        return;
    }

    uint16_t point_x = QYC32_BUFFER_X_MAX - x - 1;
    uint16_t point_y = (QYC32_BUFFER_Y_MAX << 3) - y - 1;
    uint16_t position_y = point_y >> 3;
    uint16_t bit_y = point_y & 7;

    qyc32_buffer[point_x][QYC32_BUFFER_Y_MAX - 1 - position_y] &= ~(1 << bit_y);
    qyc32_buffer[point_x][QYC32_BUFFER_Y_MAX - 1 - position_y] |= ((color == QYC32_COLOR_BLACK ? QYC32_COLOR_WHITE : QYC32_COLOR_BLACK) << bit_y);
}

void qyc32_gui_rectangle_fill_mem(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t color)
{
    uint16_t x;
    uint16_t y;

    for(x = x0; x <= x1; x++)
    {
        for (y = y0; y <= y1; y++)
        {
            qyc32_gui_point(x, y, color);
        }
    }
}

bool qyc32_gui_refresh(void)
{
    return qyc32_display((uint8_t *)qyc32_buffer, 4736);
}

void qyc32_gui_init(void)
{
    qyc32_hal_init();
    qyc32_tft_init();
    qyc32_gui_fill_mem(0);
}

#endif

