/*
 * @Author: Zhang Yuzhu
 * @Date: 2021-08-05 15:28:10
 * @LastEditors: Zhang Yuzhu
 * @LastEditTime: 2021-08-13 16:23:43
 * @FilePath: \STM32F10x_FWLIB_Template\Device\GH3011\GH3011.h
 * @Description: 
 */

#define _DEBUG_ 1
#if _DEBUG_
#define GH3011_DEBUG_LOG(...) printf(__VA_ARGS__)
#else
#define PR(...)
#endif

#ifndef __GH3011_H
#define __GH3011_H

#include "SPI.h"
#include "hbd_ctrl.h"
#include <stdio.h>

#define __GH3011_SAMPLE_RATE__ (50)
#define __GH3011_FIFO_TH__ (100)

typedef struct
{
    uint16_t SampleRate;
    EM_HBD_FUNCTIONAL_STATE EnableFifo;
    uint16_t FifoThreshold;
    const ST_REGISTER *reg_config_ptr;
    uint16_t reg_config_len;
} ST_GH30X_CONFIG;

struct GH3011_DeviceStruct_Type
{
    ST_GH30X_CONFIG ConfigStruct;
    ST_HBD_INIT_CONFIG_TYPE InitStruct;
};

extern struct GH3011_DeviceStruct_Type GH3011;

u8 GH3011_Init(void);
void GH3011_Load_New_Config(struct GH3011_DeviceStruct_Type *GH3011);
void GH3011_GetRawData_Start(struct GH3011_DeviceStruct_Type *GH3011);
void GH3011_ADT_WeraDetect_Start(struct GH3011_DeviceStruct_Type *GH3011);

#endif //   !__GH3011_H
