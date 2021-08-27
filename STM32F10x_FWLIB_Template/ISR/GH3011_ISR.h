/*
 * @Author: Zhang Yuzhu
 * @Date: 2021-08-10 14:26:56
 * @LastEditors: Zhang Yuzhu
 * @LastEditTime: 2021-08-26 11:18:45
 * @FilePath: \STM32F10x_FWLIB_Template\ISR\GH3011_ISR.h
 * @Description: 
 */
#ifndef __GH3011_ISR_H
#define __GH3011_ISR_H

#include "GH3011.h"

/// reinit max cnt
#define __RESET_REINIT_CNT_CONFIG__ (5)

/// get rawdata buffer length, max 256 (use heap space)
#define __GET_RAWDATA_BUF_LEN__ (128)

void GH3011_Handler(void);

#endif // !___GH3011_ISR_H
