/*
 * @Author: Zhang Yuzhu
 * @Date: 2021-08-05 15:28:10
 * @LastEditors: Zhang Yuzhu
 * @LastEditTime: 2021-08-12 21:06:23
 * @FilePath: \STM32F10x_FWLIB_Template\Device\GH3011\GH3011.h
 * @Description: 
 */
#ifndef __GH3011_H
#define __GH3011_H

#include "SPI.h"
#include "hbd_ctrl.h"
#include <stdio.h>

#define _DEBUG_ 1

#if _DEBUG_
#define GH3011_DEBUG_LOG(...) printf(__VA_ARGS__)
#else
#define PR(...)
#endif

#define __GH3011_SAMPLE_RATE__ (25)
#define __GH3011_FIFO_TH__ (25)

typedef struct
{
    uint16_t SampleRate;
    EM_HBD_FUNCTIONAL_STATE EnableFifo;
    uint16_t FifoThreshold; //Fifo interrupt threshold
    const ST_REGISTER *reg_config_ptr;
    uint16_t reg_config_len;
} ST_GH30X_CONFIG;

extern ST_HBD_INIT_CONFIG_TYPE GH3011_InitStruct;

extern uint16_t gh30x_SampleRate;
extern uint8_t gh30x_EnableFifo;
extern uint16_t gh30x_FifoThreshold;
extern const ST_REGISTER *gh30x_reg_config_ptr;
extern uint16_t gh30x_reg_config_len;

u8 GH3011_Init(void);
void gh30x_module_config(ST_GH30X_CONFIG *stGh30xConfig);
void gh30x_Load_new_config(const ST_REGISTER *config_ptr, uint16_t config_len);
void gh30x_getrawdata_start(void);
void gh30x_adt_wear_detect_start(const ST_REGISTER *config_ptr, GU16 config_len);

#endif //   !__GH3011_H
