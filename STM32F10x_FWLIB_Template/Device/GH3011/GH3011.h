/*
 * @Author: Zhang Yuzhu
 * @Date: 2021-08-05 15:28:10
 * @LastEditors: Zhang Yuzhu
 * @LastEditTime: 2021-08-26 19:41:25
 * @FilePath: \STM32F10x_FWLIB_Template\Device\GH3011\GH3011.h
 * @Description: 
 */

#define _DEBUG_ 1
#if _DEBUG_
#define GH3011_DEBUG_LOG(...) printf(__VA_ARGS__)
#else
#define GH3011_DEBUG_LOG(...)
#endif

#ifndef __GH3011_H
#define __GH3011_H

#include "SPI.h"
#include "hbd_ctrl.h"
#include <stdio.h>

#define __GH3011_SAMPLE_RATE__ (80)
#define __GH3011_FIFO_TH__ (80)
#define __GH3011_IRQ_WIDTH__ (3)

typedef struct
{
    u16 SampleRate;
    EM_HBD_FUNCTIONAL_STATE EnableFifo;
    u16 FifoThreshold;
    const ST_REGISTER *Reg_Config_Ptr;
    u16 Reg_Config_Len;
} ST_GH30X_CONFIG;

typedef union
{
    u32 RawData[2];
    u8 RawData_Byte[8];
} GH3011_SingleRawData_Type;

typedef struct
{
    ST_GH30X_CONFIG ConfigStruct;
    ST_HBD_INIT_CONFIG_TYPE InitStruct;
    GH3011_SingleRawData_Type GH3011_RawData[__GH3011_FIFO_TH__];
} GH3011_DeviceStruct_Type;

extern GH3011_DeviceStruct_Type GH3011;

u8 GH3011_Init(void);
void GH3011_Load_New_Config(GH3011_DeviceStruct_Type *GH3011);
void GH3011_GetRawData_Start(GH3011_DeviceStruct_Type *GH3011);
void GH3011_ADT_WeraDetect_Start(GH3011_DeviceStruct_Type *GH3011);

#endif //   !__GH3011_H
