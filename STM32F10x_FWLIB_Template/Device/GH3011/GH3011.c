/*
 * @Author: Zhang Yuzhu
 * @Date: 2021-08-05 15:28:10
 * @LastEditors: Zhang Yuzhu
 * @LastEditTime: 2021-08-26 20:50:43
 * @FilePath: \STM32F10x_FWLIB_Template\Device\GH3011\GH3011.c
 * @Description: 
 */
#include "GH3011.h"
#include "ConfigArr.h"
#include "SysTick.h"

#define GET_BYTE3_FROM_DWORD(val) ((uint8_t)((val >> 24) & 0x000000FFU))
#define GET_BYTE2_FROM_DWORD(val) ((uint8_t)((val >> 16) & 0x000000FFU))
#define GET_BYTE1_FROM_DWORD(val) ((uint8_t)((val >> 8) & 0x000000FFU))
#define GET_BYTE0_FROM_DWORD(val) ((uint8_t)((val)&0x000000FFU))
#define GET_HIGH_BYTE_FROM_WORD(val) ((uint8_t)(((val) >> 8) & 0xFF))
#define GET_LOW_BYTE_FROM_WORD(val) ((uint8_t)((val)&0xFF))

#define __RETRY_MAX_CNT_CONFIG__ (3)

GH3011_DeviceStruct_Type GH3011;

static u8 GH3011_I2C_Write_Exchange_To_SPI(u8 Device_ID, const u8 Write_Buffer[], u16 Write_Length)
{
    uint8_t ret = SUCCESS;
    if ((Write_Length == 3) && (Write_Buffer[0] == 0xDD) && (Write_Buffer[1] == 0xDD))
    {
        ret = SPI_CS_Control(0);
        ret = SPI_WriteData(&Write_Buffer[2], 1);
        ret = SPI_CS_Control(1);
        SysTick_Delay_us(10);
    }
    else
    {
        uint8_t SPI_Write_Buffer[5] = {0};
        uint16_t spi_real_len = Write_Length - 2;

        ret = SPI_CS_Control(0);
        SPI_Write_Buffer[0] = 0xF0;
        SPI_Write_Buffer[1] = Write_Buffer[0];
        SPI_Write_Buffer[2] = Write_Buffer[1];
        SPI_Write_Buffer[3] = GET_HIGH_BYTE_FROM_WORD(spi_real_len);
        SPI_Write_Buffer[4] = GET_LOW_BYTE_FROM_WORD(spi_real_len);
        ret = SPI_WriteData(SPI_Write_Buffer, 5);
        ret = SPI_WriteData(&Write_Buffer[2], spi_real_len);
        SysTick_Delay_us(20);
        ret = SPI_CS_Control(1);
        SysTick_Delay_us(10);
    }
    return ret;
}

static u8 GH3011_I2C_Read_Exchange_To_SPI(uint8_t Device_ID, const uint8_t Write_Buffer[], uint16_t Write_Length, uint8_t Read_Buffer[], uint16_t Read_Length)
{
    uint8_t ret = SUCCESS;

    if (Write_Length == 2)
    {
        uint8_t SPI_Write_Buffer[3] = {0};

        ret = SPI_CS_Control(0);
        SPI_Write_Buffer[0] = 0xF0;
        SPI_Write_Buffer[1] = Write_Buffer[0];
        SPI_Write_Buffer[2] = Write_Buffer[1];
        ret = SPI_WriteData(SPI_Write_Buffer, 3);
        SysTick_Delay_us(20);
        ret = SPI_CS_Control(1);
        SysTick_Delay_us(10);

        ret = SPI_CS_Control(0);
        SPI_Write_Buffer[0] = 0xF1;
        ret = SPI_WriteData(SPI_Write_Buffer, 1);
        ret = SPI_ReadData(Read_Buffer, Read_Length);
        SysTick_Delay_us(20);
        ret = SPI_CS_Control(1);
        SysTick_Delay_us(10);
    }
    else
    {
        ret = ERROR;
    }

    return ret;
}

void GH3011_Load_New_Config(GH3011_DeviceStruct_Type *GH3011)
{
    GU8 ReLoad_Cnt = 0;

    GH3011_DEBUG_LOG("\n\r GH3011 Load new config start");

    if (HBD_LoadNewRegConfigArr(GH3011->ConfigStruct.Reg_Config_Ptr, GH3011->ConfigStruct.Reg_Config_Len) != HBD_RET_OK)
    {
        GH3011_DEBUG_LOG("\n\r GH3011 Load new config success!");
        for (ReLoad_Cnt = 0; ReLoad_Cnt < __RETRY_MAX_CNT_CONFIG__ && GH3011->ConfigStruct.Reg_Config_Ptr != NULL; ReLoad_Cnt++) // retry
        {
            if (HBD_LoadNewRegConfigArr(GH3011->ConfigStruct.Reg_Config_Ptr, GH3011->ConfigStruct.Reg_Config_Len) == HBD_RET_OK)
            {
                GH3011_DEBUG_LOG("\n\r GH3011 Load new config success!");
                break;
            }
        }
        if (ReLoad_Cnt >= __RETRY_MAX_CNT_CONFIG__)
            GH3011_DEBUG_LOG("\n\r GH3011 Load new config failed!");
    }
    else
    {
        GH3011_DEBUG_LOG("\n\r GH3011 Load new config success!");
    }
}

void GH3011_GetRawData_Start(GH3011_DeviceStruct_Type *GH3011)
{
    GU8 ReLoad_Cnt = 0;

    GH3011_DEBUG_LOG("\n\r GH3011 moudle get raw data start");

    if (HBD_StartHBDOnly(GH3011->ConfigStruct.SampleRate, GH3011->ConfigStruct.EnableFifo, GH3011->ConfigStruct.FifoThreshold) != HBD_RET_OK) // start
    {
        for (ReLoad_Cnt = 0; ReLoad_Cnt < __RETRY_MAX_CNT_CONFIG__; ReLoad_Cnt++) // retry
        {
            if (HBD_SimpleInit(&(GH3011->InitStruct)) == HBD_RET_OK)
            {
                if (HBD_LoadNewRegConfigArr(GH3011->ConfigStruct.Reg_Config_Ptr, GH3011->ConfigStruct.Reg_Config_Len) == HBD_RET_OK)
                {
                    if (HBD_StartHBDOnly(GH3011->ConfigStruct.SampleRate, GH3011->ConfigStruct.EnableFifo, GH3011->ConfigStruct.FifoThreshold) == HBD_RET_OK)
                    {
                        GH3011_DEBUG_LOG("\n\r GH3011 moudle get raw data start success!");
                        break;
                    }
                }
            }
        }
        if (ReLoad_Cnt >= __RETRY_MAX_CNT_CONFIG__)
            GH3011_DEBUG_LOG("\n\r GH3011 moudle get raw data start failed!");
    }
    else
    {
        GH3011_DEBUG_LOG("\n\r GH3011 moudle get raw data start success!");
    }
}

void GH3011_ADT_WeraDetect_Start(GH3011_DeviceStruct_Type *GH3011)
{
    GU8 ReLoad_Cnt = 0;

    GH3011_DEBUG_LOG("\n\r GH3011 moudle start with ADT");
  
    if ((GH3011->ConfigStruct.Reg_Config_Ptr != NULL) & (GH3011->ConfigStruct.Reg_Config_Len != 0))
    {
        GH3011_Load_New_Config(GH3011);
        if (HBD_AdtWearDetectStart() != HBD_RET_OK) // start
        {
            for (ReLoad_Cnt = 0; ReLoad_Cnt < __RETRY_MAX_CNT_CONFIG__; ReLoad_Cnt++) // retry
            {
                if (HBD_SimpleInit(&(GH3011->InitStruct)) == HBD_RET_OK)
                {
                    if (HBD_LoadNewRegConfigArr(GH3011->ConfigStruct.Reg_Config_Ptr, GH3011->ConfigStruct.Reg_Config_Len) == HBD_RET_OK)
                    {
                        if (HBD_AdtWearDetectStart() == HBD_RET_OK)
                        {
                            GH3011_DEBUG_LOG("\n\r GH3011 wear detect start success!");
                            break;
                        }
                    }
                }
            }
            if (__RETRY_MAX_CNT_CONFIG__ <= ReLoad_Cnt)
                GH3011_DEBUG_LOG("\n\r GH3011 wear detect start failed!");
        }
        else
        {
            GH3011_DEBUG_LOG("\n\r GH3011 wear detect start success!");
        }
    }
    else
    {
        GH3011_DEBUG_LOG("\n\r GH3011 wear detect function para error!");
    }
}

u8 GH3011_Init(void)
{
    u8 GH3011_Init_Flag = HBD_RET_OK;
    GH3011_Init_Flag = HBD_SetI2cRW(HBD_I2C_ID_SEL_1L0L, GH3011_I2C_Write_Exchange_To_SPI, GH3011_I2C_Read_Exchange_To_SPI);
    if (GH3011_Init_Flag == HBD_RET_OK)
    {
        GH3011_DEBUG_LOG("\n\r GH3011 SPI Function Login Success!");
        GH3011_Init_Flag = HBD_CommunicationInterfaceConfirm();
        if (GH3011_Init_Flag == HBD_RET_OK)
        {
            GH3011_DEBUG_LOG("\n\r GH3011 Communicate Success!");
            GH3011.InitStruct.stAdtInitConfig.emGINTEnable = HBD_FUNCTIONAL_STATE_DISABLE;
            GH3011.InitStruct.stAdtInitConfig.emLedLogicChannelMap = HBD_LED_LOGIC_CHANNEL_MAP_PHY012;
            GH3011.InitStruct.stAdtInitConfig.emLogicChannel0PDSelect = HBD_LED_PD_SEL_EXTERNAL;
            GH3011.InitStruct.stAdtInitConfig.emLogicChannel1PDSelect = HBD_LED_PD_SEL_EXTERNAL;
            GH3011.InitStruct.stAdtInitConfig.emLogicChannel0TiaGain = HBD_TIA_GAIN_4;
            GH3011.InitStruct.stAdtInitConfig.emLogicChannel1TiaGain = HBD_TIA_GAIN_4;
            GH3011.InitStruct.stAdtInitConfig.uchLogicChannel0Current = 0x14;
            GH3011.InitStruct.stAdtInitConfig.uchLogicChannel1Current = 0x14;
            GH3011.InitStruct.stAdtInitConfig.emSampleTime = HBD_SAMPLE_TIME_1024CYCLES;

            GH3011.ConfigStruct.EnableFifo = HBD_FUNCTIONAL_STATE_ENABLE;
            GH3011.ConfigStruct.SampleRate = __GH3011_SAMPLE_RATE__;
            GH3011.ConfigStruct.FifoThreshold = __GH3011_FIFO_TH__;
            GH3011.ConfigStruct.Reg_Config_Ptr = hb_reg_config_array;
            GH3011.ConfigStruct.Reg_Config_Len = hb_reg_config_array_len;

            GH3011_Init_Flag = HBD_SetIrqPluseWidth(__GH3011_IRQ_WIDTH__);
            GH3011_Init_Flag = HBD_SimpleInit(&GH3011.InitStruct);
        }
        else
        {
            GH3011_DEBUG_LOG("\n\r GH3011 Communicate Failed!,Please Check!");
        }
    }
    else
    {
        GH3011_DEBUG_LOG("\n\r GH3011 SPI Function Login Failed!");
    }
    return GH3011_Init_Flag;
}
