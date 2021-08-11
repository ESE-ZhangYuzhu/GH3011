/*
 * @Author: Zhang Yuzhu
 * @Date: 2021-08-04 16:10:29
 * @LastEditors: Zhang Yuzhu
 * @LastEditTime: 2021-08-05 15:20:42
 * @FilePath: \STM32F10x_FWLIB_Template\App\I2C.c
 * @Description: Use STM32 FWLIB Hardware I2C
 */

#include "I2C.h"
#include <stdio.h>

#define WAIT_TIME 200

__INLINE static u8 I2C_TimeOverFlow_UserCallBack(u8 ErrorCode)
{
    printf("I2C waiting for a long time,Error Code = %d", ErrorCode);
    return ErrorCode;
}

u8 I2C_WriteData(u8 DeviceID, const u8 WriteBytesArr[], u16 WriteLen)
{
    u8 I2C_WriteFlag = SUCCESS;
    u8 I2C_WaitTime;
    /******** Function Body *********/
    I2C_WaitTime = WAIT_TIME;
    while (!I2C_CheckEvent(I2C1, I2C_FLAG_BUSY)) // Before start check IIC bus whether busy
    {
        if ((I2C_WaitTime--) == 0)
            return I2C_TimeOverFlow_UserCallBack(0xFF);
    }

    I2C_GenerateSTART(I2C1, ENABLE);

    I2C_WaitTime = WAIT_TIME;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) // send start signal which decide master mode
    {
        if ((I2C_WaitTime--) == 0)
            return I2C_TimeOverFlow_UserCallBack(0xFE);
    }

    I2C_Send7bitAddress(I2C1, DeviceID, I2C_Direction_Transmitter);

    I2C_WaitTime = WAIT_TIME;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if ((I2C_WaitTime--) == 0)
            return I2C_TimeOverFlow_UserCallBack(0xFD);
    }

    for (u16 i = 0; i < WriteLen; i++)
    {
        I2C_SendData(I2C1, WriteBytesArr[i]);

        I2C_WaitTime = WAIT_TIME;
        while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        {
            if ((I2C_WaitTime--) == 0)
                return I2C_TimeOverFlow_UserCallBack(0xFC);
        }
    }

    I2C_GenerateSTOP(I2C1, ENABLE);

    /******** Function Body *********/
    return I2C_WriteFlag;
}

u8 I2C_ReadData(u8 DeviceID, const u8 CMDBytesArr[], u16 CMDLen, u8 ReadBytesArr[], u16 MaxReadLen)
{
    u8 I2C_ReadFlag = SUCCESS;
    u8 I2C_WaitTime;
    /******** Function Body *********/
    I2C_WaitTime = WAIT_TIME;
    while (!I2C_CheckEvent(I2C1, I2C_FLAG_BUSY))
    {
        if ((I2C_WaitTime--) == 0)
            return I2C_TimeOverFlow_UserCallBack(0xFB);
    }

    I2C_GenerateSTART(I2C1, ENABLE);

    I2C_WaitTime = WAIT_TIME;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((I2C_WaitTime--) == 0)
            return I2C_TimeOverFlow_UserCallBack(0xFA);
    }

    I2C_Send7bitAddress(I2C1, DeviceID, I2C_Direction_Transmitter);

    I2C_WaitTime = WAIT_TIME;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if ((I2C_WaitTime--) == 0)
            return I2C_TimeOverFlow_UserCallBack(0xF9);
    }

    for (u16 i = 0; i < CMDLen - 1; i++)
    {
        I2C_SendData(I2C1, CMDBytesArr[i]);

        I2C_WaitTime = WAIT_TIME;
        while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        {
            if ((I2C_WaitTime--) == 0)
                return I2C_TimeOverFlow_UserCallBack(0xF8);
        }
    }

    I2C_SendData(I2C1, CMDBytesArr[CMDLen - 1]);

    I2C_WaitTime = WAIT_TIME;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if ((I2C_WaitTime--) == 0)
            return I2C_TimeOverFlow_UserCallBack(0xF7);
    }

    I2C_GenerateSTART(I2C1, ENABLE);

    I2C_WaitTime = WAIT_TIME;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((I2C_WaitTime--) == 0)
            return I2C_TimeOverFlow_UserCallBack(0xF6);
    }

    I2C_Send7bitAddress(I2C1, DeviceID, I2C_Direction_Receiver);

    I2C_WaitTime = WAIT_TIME;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
        if ((I2C_WaitTime--) == 0)
            return I2C_TimeOverFlow_UserCallBack(0xF5);
    }

    if (MaxReadLen == 1)
    {
        I2C_AcknowledgeConfig(I2C1, DISABLE);

        I2C_WaitTime = WAIT_TIME;
        while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))
        {
            if ((I2C_WaitTime--) == 0)
                return I2C_TimeOverFlow_UserCallBack(0xF4);
        }

        ReadBytesArr[0] = I2C_ReceiveData(I2C1);
    }
    else
    {
        for (u16 i = 0; i < MaxReadLen - 1; i++)
        {
            I2C_WaitTime = WAIT_TIME;
            while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))
            {
                if ((I2C_WaitTime--) == 0)
                    return I2C_TimeOverFlow_UserCallBack(0xF4);
            }

            ReadBytesArr[i] = I2C_ReceiveData(I2C1);
        }
        I2C_AcknowledgeConfig(I2C1, DISABLE);

        I2C_WaitTime = WAIT_TIME;
        while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))
        {
            if ((I2C_WaitTime--) == 0)
                return I2C_TimeOverFlow_UserCallBack(0xF4);
        }

        ReadBytesArr[MaxReadLen - 1] = I2C_ReceiveData(I2C1);
    }

    I2C_GenerateSTOP(I2C1, ENABLE);
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    /******** Function Body *********/
    return I2C_ReadFlag;
}
