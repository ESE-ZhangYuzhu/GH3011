/*
 * @Author: Zhang Yuzhu
 * @Date: 2021-08-05 14:25:35
 * @LastEditors: Zhang Yuzhu
 * @LastEditTime: 2021-08-09 16:41:47
 * @FilePath: \STM32F10x_FWLIB_Template\App\SPI.c
 * @Description: 
 */
#include "SPI.h"
#include <stdio.h>

#define WAIT_TIME 200
#define SPI_DUMMY 0xFF

#define SPI_ERRORFLAG_BUSY 0xFF
#define SPI_ERRORFLAG_TXE 0xFE
#define SPI_ERRORFLAG_RXNE 0xFD

__INLINE static u8 SPI_TimeOverFlow_UserCallBack(u8 ErrorCode)
{
    printf("\n\rSPI waiting for a long time,Error Code = %d", ErrorCode);
    return ErrorCode;
}

void SPI_CS_Control(u8 CS_Level)
{
    __IO u8 SPI_WaitTime = 0;
    if (CS_Level != 0)
    {
        // Before you stop SPI Bus , make sure Bus free!
        SPI_WaitTime = WAIT_TIME;
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET)
        {
            if ((SPI_WaitTime--) == 0)
                SPI_TimeOverFlow_UserCallBack(SPI_ERRORFLAG_BUSY);
        }
        PAout(4) = CS_Level & 0x01;
    }
    else
    {
        PAout(4) = 0;
    }
}

u8 SPI_WriteData(const u8 WriteBytesArr[], u16 WriteLen)
{
    u8 SPI_WriteFlag = SUCCESS;
    __IO u8 SPI_WaitTime = 0;
    /************ Function Body ****************/
    for (u8 i = 0; i < WriteLen; i++)
    {

        SPI_WaitTime = WAIT_TIME;
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
        {
            if ((SPI_WaitTime--) == 0)
                return SPI_TimeOverFlow_UserCallBack(SPI_ERRORFLAG_TXE);
        }

        SPI_I2S_SendData(SPI1, WriteBytesArr[i]);

        SPI_WaitTime = WAIT_TIME;
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
        {
            if ((SPI_WaitTime--) == 0)
                return SPI_TimeOverFlow_UserCallBack(SPI_ERRORFLAG_RXNE);
        }

        SPI_I2S_ReceiveData(SPI1);
    }
    /************ Function Body ****************/
    return SPI_WriteFlag;
}

u8 SPI_ReadData(u8 ReadBytesArr[], u16 MaxReadLen)
{
    u8 SPI_ReadFlag = SUCCESS;
    __IO u8 SPI_WaitTime = 0;
    /************ Function Body ****************/
    for (u8 i = 0; i < MaxReadLen; i++)
    {
        SPI_WaitTime = WAIT_TIME;
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
        {
            if ((SPI_WaitTime--) == 0)
                return SPI_TimeOverFlow_UserCallBack(SPI_ERRORFLAG_TXE);
        }

        SPI_I2S_SendData(SPI1, SPI_DUMMY);

        SPI_WaitTime = WAIT_TIME;
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
        {
            if ((SPI_WaitTime--) == 0)
                return SPI_TimeOverFlow_UserCallBack(SPI_ERRORFLAG_RXNE);
        }

        ReadBytesArr[i] = SPI_I2S_ReceiveData(SPI1);
    }
    /************ Function Body ****************/
    return SPI_ReadFlag;
}
