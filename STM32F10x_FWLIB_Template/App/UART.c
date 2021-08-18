/*
 * @Author: yz.zhang[zhang.yz2@yuyue.com.cn]
 * @Date: 2021-07-26 14:37:28
 * @LastEditors: Zhang Yuzhu
 * @LastEditTime: 2021-08-17 11:01:22
 * @FilePath: \STM32F10x_FWLIB_Template\App\UART.c
 * @Description: Redirect printf of USART1, print char and string
 */

#include "UART.h"

void UART1_SendByte(uint8_t ch)
{
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
    ;
  USART_SendData(USART1, (u16)ch);
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
    ;
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
    ;
}

void UART1_SendString(uint8_t *str)
{
  while (*str)
  {
    UART1_SendByte(*str++);
  }
}

int fputc(int ch, FILE *stream)
{
  UART1_SendByte((u8)ch);
  return ch;
}
