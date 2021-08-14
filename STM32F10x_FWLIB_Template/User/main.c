/*
 * @Author: yz.zhang[zhang.yz2@yuyue.com.cn]
 * @Date: 2021-07-22 10:21:16
 * @LastEditors: Zhang Yuzhu
 * @LastEditTime: 2021-08-13 10:43:42
 * @FilePath: \STM32F10x_FWLIB_Template\User\main.c
 * @Description: Ues STM32F103RCT6 have a nice show!
 */

/*
 *  Total Read Only Size (Code + RO Data)
 *  Total Read Write Size(RW Data + ZI Data)
 *  Total ROM Size (Code + RO Data + RW Data)
 */

#include "main.h"
#include <stdio.h>

int main(void)
{
  ALL_System_Driver_Init();
  GH3011_Init();
  TIM_Cmd(TIM6, ENABLE);
  while (1)
  {
    if (TIM6_500ms_Flag)
    {
      TIM6_500ms_Flag = 0;
      ADC_SoftwareStartConvCmd(ADC1, ENABLE);
      PCout(0) = PCout(1) ^= 1;
    }
    if (TIM6_1s_Flag)
    {
      __IO float MCU_Temperature;
      TIM6_1s_Flag = 0;
      MCU_Temperature = (1.43 - ((float)ADC1_Channel16_Value / 0xFFF * 3.3)) / 0.0043 + 25;
      // printf("\n\rMCU_Temperature =%.2f", (float)MCU_Temperature);
    }
  }
}
