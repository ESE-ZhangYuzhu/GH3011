/*
 * @Author: Zhang Yuzhu
 * @Date: 2021-08-05 14:25:35
 * @LastEditors: Zhang Yuzhu
 * @LastEditTime: 2021-08-25 10:12:21
 * @FilePath: \STM32F10x_FWLIB_Template\App\SPI.h
 * @Description: 
 */
#ifndef __SPI_H
#define __SPI_H

#include "stm32f10x.h"
#include "GPIO_Bit_Band.h"

u8 SPI_CS_Control(u8 CS_Level);
u8 SPI_WriteData(const u8 WriteBytesArr[], u16 WriteLen);
u8 SPI_ReadData(u8 ReadBytesArr[], u16 MaxReadLen);

#endif // !__SPI_H
