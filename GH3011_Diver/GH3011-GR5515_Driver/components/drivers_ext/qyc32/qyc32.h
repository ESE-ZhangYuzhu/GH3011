/**
 ****************************************************************************************
 *
 * @file qyc32.h
 *
 * @brief TFT display controller driver of qiyun GDEW029C32 IC API.
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

#ifndef __QYC32_H__
#define __QYC32_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define    QYC32_COLOR_BLACK        1
#define    QYC32_COLOR_WHITE        0

/**
 *****************************************************************************************
 * @brief qyc32 hal init(config gpio spi).
 *****************************************************************************************
 */
void qyc32_hal_init(void);

/**
 *****************************************************************************************
 * @brief qyc32 hal deinit(config gpio spi).
 *****************************************************************************************
 */
void qyc32_hal_deinit(void);

/**
 *****************************************************************************************
 * @brief Draw a point to display memory.
 *
 * @param[in] x: X coordinate.
 * @param[in] y: Y coordinate.
 * @param[in] color: The color of the point.
 *****************************************************************************************
 */
void qyc32_gui_point(uint16_t x, uint16_t y, uint8_t color);

/**
 *****************************************************************************************
 * @brief Fill Data to gui display memory. Default: white.
 *****************************************************************************************
 */
void qyc32_gui_fill_mem(uint8_t data);

/**
 *****************************************************************************************
 * @brief Fill Data to gui display rectangle memory.
 *
 * @param[in] x0: X0 coordinate.
 * @param[in] y0: Y0 coordinate.
 * @param[in] x1: X1 coordinate.
 * @param[in] y1: Y1 coordinate.
 * @param[in] color:  Fill color.
 *****************************************************************************************
 */
void qyc32_gui_rectangle_fill_mem(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t color);

/**
 *****************************************************************************************
 * @brief Refresh the display memory data to qyc32 chip.
 *
 * @retval ::true: Operation is OK; false:Operation is Error(IC is busy).
 *****************************************************************************************
 */
bool qyc32_gui_refresh(void);

/**
 *****************************************************************************************
 * @brief qyc32 gui display init.
 *****************************************************************************************
 */
void qyc32_gui_init(void);

/**
 *****************************************************************************************
 * @brief Return the QYC32 status.
 * @retval ::true: QYC32 is busy; false:QYC32 is idle.
 *****************************************************************************************
 */
bool qyc32_is_busy(void);

/**
 *****************************************************************************************
 * @brief qyc32 spi init.
 *****************************************************************************************
 */
void qyc32_spi_init(void);


#ifdef __cplusplus
}
#endif

#endif

