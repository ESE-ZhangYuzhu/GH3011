/*
 * @Author: Zhang Yuzhu
 * @Date: 2021-08-04 16:10:41
 * @LastEditors: Zhang Yuzhu
 * @LastEditTime: 2021-08-04 23:10:43
 * @FilePath: \STM32F10x_FWLIB_Template\App\I2C.h
 * @Description: I2H header file
 */

#ifndef __I2C_H
#define __I2C_H

#include "stm32f10x.h"

u8 I2C_WriteData(u8 DeviceID, const u8 WriteBytesArr[], u16 WriteLen);
u8 I2C_ReadData(u8 DeviceID, const u8 CMDBytesArr[], u16 CMDLen, u8 ReadBytesArr[], u16 MaxReadLen);

#endif // !__I2C_H
