/*
 * @Author: yz.zhang[zhang.yz2@yuyue.com.cn]
 * @Date: 2021-07-26 14:37:28
 * @LastEditors: Zhang Yuzhu
 * @LastEditTime: 2021-08-17 11:05:43
 * @FilePath: \STM32F10x_FWLIB_Template\App\UART.h
 * @Description:
 */
#ifndef __UART_H
#define __UART_H

#include <stdio.h>
#include "stm32f10x.h"

void UART1_SendByte(uint8_t ch);
void UART1_SendString(uint8_t *str);
int fputc(int ch, FILE *stream);

#endif // !__UART_H
