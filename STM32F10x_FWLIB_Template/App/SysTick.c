/*
 * @Author: yz.zhang[zhang.yz2@yuyue.com.cn]
 * @Date: 2021-07-23 10:26:26
 * @LastEditors: Zhang Yuzhu
 * @LastEditTime: 2021-08-25 10:13:19
 * @FilePath: \STM32F10x_FWLIB_Template\App\SysTick.c
 * @Description: Time base
 */

#include "SysTick.h"

void System_TickTimer_Init(uint32_t Time_base)
{
  while (SysTick_Config(Time_base) == 1)
    ;
  SysTick->CTRL &=
      ~SysTick_CTRL_TICKINT_Msk; // Disable SysTick interrupt request source
}

void SysTick_Delay_us(__IO uint32_t us)
{
  System_TickTimer_Init(SystemCoreClock / 1000000);
  for (u16 i = 0; i < us; i++)
  {
    while (!((SysTick->CTRL) & SysTick_CTRL_COUNTFLAG_Msk))
      ;
  }
  SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Delay_ms(__IO uint32_t ms)
{
  System_TickTimer_Init(SystemCoreClock / 1000);
  for (u32 i = 0; i < ms; i++)
  {
    while (!((SysTick->CTRL) & SysTick_CTRL_COUNTFLAG_Msk))
      ;
  }
  SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}
