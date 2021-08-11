/*
 * @Author: Zhang Yuzhu
 * @Date: 2021-08-05 15:28:10
 * @LastEditors: Zhang Yuzhu
 * @LastEditTime: 2021-08-11 11:21:52
 * @FilePath: \STM32F10x_FWLIB_Template\Device\GH3011\GH3011.c
 * @Description: 
 */
#include "GH3011.h"
#include <stdio.h>
#include "ConfigArr.h"

#define GET_BYTE3_FROM_DWORD(val) ((uint8_t)((val >> 24) & 0x000000FFU))
#define GET_BYTE2_FROM_DWORD(val) ((uint8_t)((val >> 16) & 0x000000FFU))
#define GET_BYTE1_FROM_DWORD(val) ((uint8_t)((val >> 8) & 0x000000FFU))
#define GET_BYTE0_FROM_DWORD(val) ((uint8_t)((val)&0x000000FFU))
#define GET_HIGH_BYTE_FROM_WORD(val) ((uint8_t)(((val) >> 8) & 0xFF))
#define GET_LOW_BYTE_FROM_WORD(val) ((uint8_t)((val)&0xFF))

/// retry max cnt
#define __RETRY_MAX_CNT_CONFIG__ (3)

/// gh30x sample rate
uint16_t gh30x_SampleRate = 25;

/// gh30x fifo enable
uint8_t gh30x_EnableFifo = 1;

/// gh30x fifo threshold
uint16_t gh30x_FifoThreshold = 25;

const ST_REGISTER *gh30x_reg_config_ptr = NULL;
uint16_t gh30x_reg_config_len = 0;

HBD_INIT_CONFIG_DEFAULT_DEF(GH3011_InitStruct);

static u8 GH3011_I2C_Write_Exchange_To_SPI(u8 Device_ID, const u8 Write_Buffer[], u16 Length)
{
    uint8_t ret = SUCCESS;
    if ((Length == 3) && (Write_Buffer[0] == 0xDD) && (Write_Buffer[1] == 0xDD))
    {
        SPI_CS_Control(0);
        SPI_WriteData(&Write_Buffer[2], 1);
        SPI_CS_Control(1);
        SysTick_Delay_us(10);
    }
    else
    {
        uint8_t spi_write_buffer[5] = {0};
        uint16_t spi_real_len = Length - 2;

        SPI_CS_Control(0);
        spi_write_buffer[0] = 0xF0;
        spi_write_buffer[1] = Write_Buffer[0];
        spi_write_buffer[2] = Write_Buffer[1];
        spi_write_buffer[3] = GET_HIGH_BYTE_FROM_WORD(spi_real_len);
        spi_write_buffer[4] = GET_LOW_BYTE_FROM_WORD(spi_real_len);
        SPI_WriteData(spi_write_buffer, 5);
        SPI_WriteData(&Write_Buffer[2], spi_real_len);
        SysTick_Delay_us(20);
        SPI_CS_Control(1);
        SysTick_Delay_us(10);
    }
    return ret;
}

static u8 GH3011_I2C_Read_Exchange_To_SPI(uint8_t device_id, const uint8_t write_buffer[], uint16_t write_length, uint8_t read_buffer[], uint16_t read_length)
{
    uint8_t ret = SUCCESS;
    if (write_length == 2)
    {
        uint8_t spi_write_buffer[3] = {0};

        SPI_CS_Control(0);
        spi_write_buffer[0] = 0xF0;
        spi_write_buffer[1] = write_buffer[0];
        spi_write_buffer[2] = write_buffer[1];
        SPI_WriteData(spi_write_buffer, 3);
        SysTick_Delay_us(20);
        SPI_CS_Control(1);
        SysTick_Delay_us(10);

        SPI_CS_Control(0);
        spi_write_buffer[0] = 0xF1;
        SPI_WriteData(spi_write_buffer, 1);
        SPI_ReadData(read_buffer, read_length);
        SysTick_Delay_us(20);
        SPI_CS_Control(1);
        SysTick_Delay_us(10);
    }

    return ret;
}

void gh30x_module_config(ST_GH30X_CONFIG stGh30xConfig)
{
    gh30x_SampleRate = stGh30xConfig.SampleRate;
    gh30x_EnableFifo = stGh30xConfig.EnableFifo;
    gh30x_FifoThreshold = stGh30xConfig.FifoThreshold;
    gh30x_reg_config_ptr = stGh30xConfig.reg_config_ptr;
    gh30x_reg_config_len = stGh30xConfig.reg_config_len;
}

/// gh30x load new config
void gh30x_Load_new_config(const ST_REGISTER *config_ptr, uint16_t len)
{
    GU8 index = 0;

    for (index = 0; index < __RETRY_MAX_CNT_CONFIG__ && config_ptr != NULL; index++) // retry
    {
        if (HBD_LoadNewRegConfigArr(config_ptr, len) == HBD_RET_OK)
        {
            break;
        }
    }
}

void gh30x_getrawdata_start(void)
{
    GU8 index = 0;

    if (HBD_StartHBDOnly(gh30x_SampleRate, gh30x_EnableFifo, gh30x_FifoThreshold) != HBD_RET_OK) // start
    {
        for (index = 0; index < __RETRY_MAX_CNT_CONFIG__; index++) // retry
        {
            if (HBD_SimpleInit(&GH3011_InitStruct) == HBD_RET_OK)
            {
                if (HBD_LoadNewRegConfigArr(hb_reg_config_array, hb_reg_config_array_len) == HBD_RET_OK)
                {
                    if (HBD_StartHBDOnly(gh30x_SampleRate, gh30x_EnableFifo, gh30x_FifoThreshold) == HBD_RET_OK)
                    {
                        break;
                    }
                }
            }
        }
    }
}

/// gh30x adt func start
void gh30x_adt_wear_detect_start(const ST_REGISTER *config_ptr, GU16 config_len)
{
    GS16 nRes = 0;
    GU8 index = 0;

    if ((config_ptr != NULL) & (config_len != 0))
    {
        gh30x_Load_new_config(config_ptr, config_len);
        nRes = HBD_AdtWearDetectStart();
        if (nRes != HBD_RET_OK) // start
        {
            for (index = 0; index < __RETRY_MAX_CNT_CONFIG__; index++) // retry
            {
                if (HBD_SimpleInit(&GH3011_InitStruct) == HBD_RET_OK)
                {
                    if (HBD_LoadNewRegConfigArr(config_ptr, config_len) == HBD_RET_OK)
                    {
                        if (HBD_AdtWearDetectStart() == HBD_RET_OK)
                        {
                            break;
                        }
                    }
                }
            }
        }
    }
}

u8 GH3011_Init(void)
{
    u8 GH3011_Init_Flag;
    HBD_SetDelayUsCallback(SysTick_Delay_us);
    GH3011_Init_Flag = HBD_SetI2cRW(HBD_I2C_ID_SEL_1L0L, GH3011_I2C_Write_Exchange_To_SPI, GH3011_I2C_Read_Exchange_To_SPI);
    GH3011_Init_Flag = HBD_CommunicationInterfaceConfirm();

    GH3011_InitStruct.stAdtInitConfig.emGINTEnable = HBD_FUNCTIONAL_STATE_DISABLE;
    GH3011_InitStruct.stAdtInitConfig.emLedLogicChannelMap = HBD_LED_LOGIC_CHANNEL_MAP_PHY012;
    GH3011_InitStruct.stAdtInitConfig.emLogicChannel0PDSelect = HBD_LED_PD_SEL_EXTERNAL;
    GH3011_InitStruct.stAdtInitConfig.emLogicChannel1PDSelect = HBD_LED_PD_SEL_EXTERNAL;
    GH3011_InitStruct.stAdtInitConfig.emLogicChannel0TiaGain = HBD_TIA_GAIN_2;
    GH3011_InitStruct.stAdtInitConfig.emLogicChannel1TiaGain = HBD_TIA_GAIN_2;
    GH3011_InitStruct.stAdtInitConfig.uchLogicChannel0Current = 0x14;
    GH3011_InitStruct.stAdtInitConfig.uchLogicChannel1Current = 0x14;
    GH3011_InitStruct.stAdtInitConfig.emSampleTime = HBD_SAMPLE_TIME_128CYCLES;

    GH3011_Init_Flag = HBD_SimpleInit(&GH3011_InitStruct);
    GH3011_Init_Flag = HBD_SetIrqPluseWidth(255);

    gh30x_getrawdata_start();

    return GH3011_Init_Flag;
}
