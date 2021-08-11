/*
 * @Author: yz.zhang[zhang.yz2@yuyue.com.cn]
 * @Date: 2021-07-22 11:47:49
 * @LastEditors: Zhang Yuzhu
 * @LastEditTime: 2021-08-04 18:14:16
 * @FilePath: \STM32F10x_FWLIB_Template\App\GPIO_Bit_Band.h
 * @Description:
 */
#ifndef __GPIO_BIT_BAND_H
#define __GPIO_BIT_BAND_H

#include "stm32f10x.h"

#define GPIOA_ODR_ADDR (GPIOA_BASE + 0x0C)
#define GPIOB_ODR_ADDR (GPIOB_BASE + 0x0C)
#define GPIOC_ODR_ADDR (GPIOC_BASE + 0x0C)
#define GPIOD_ODR_ADDR (GPIOD_BASE + 0x0C)
#define GPIOE_ODR_ADDR (GPIOE_BASE + 0x0C)
#define GPIOF_ODR_ADDR (GPIOF_BASE + 0x0C)
#define GPIOG_ODR_ADDR (GPIOG_BASE + 0x0C)

#define GPIOA_IDR_ADDR (GPIOA_BASE + 0x08)
#define GPIOB_IDR_ADDR (GPIOB_BASE + 0x08)
#define GPIOC_IDR_ADDR (GPIOC_BASE + 0x08)
#define GPIOD_IDR_ADDR (GPIOD_BASE + 0x08)
#define GPIOE_IDR_ADDR (GPIOE_BASE + 0x08)
#define GPIOF_IDR_ADDR (GPIOF_BASE + 0x08)
#define GPIOG_IDR_ADDR (GPIOG_BASE + 0x08)

#define PAout(n)                                                          \
  *(volatile unsigned int *)((GPIOA_ODR_ADDR & 0xF0000000) + 0x02000000 + \
                             ((GPIOA_ODR_ADDR & 0x00FFFFFF) << 5) + (n << 2))
#define PBout(n)                                                          \
  *(volatile unsigned int *)((GPIOB_ODR_ADDR & 0xF0000000) + 0x02000000 + \
                             ((GPIOB_ODR_ADDR & 0x00FFFFFF) << 5) + (n << 2))
#define PCout(n)                                                          \
  *(volatile unsigned int *)((GPIOC_ODR_ADDR & 0xF0000000) + 0x02000000 + \
                             ((GPIOC_ODR_ADDR & 0x00FFFFFF) << 5) + (n << 2))
#define PDout(n)                                                          \
  *(volatile unsigned int *)((GPIOD_ODR_ADDR & 0xF0000000) + 0x02000000 + \
                             ((GPIOD_ODR_ADDR & 0x00FFFFFF) << 5) + (n << 2))
#define PEout(n)                                                          \
  *(volatile unsigned int *)((GPIOE_ODR_ADDR & 0xF0000000) + 0x02000000 + \
                             ((GPIOE_ODR_ADDR & 0x00FFFFFF) << 5) + (n << 2))
#define PFout(n)                                                          \
  *(volatile unsigned int *)((GPIOF_ODR_ADDR & 0xF0000000) + 0x02000000 + \
                             ((GPIOF_ODR_ADDR & 0x00FFFFFF) << 5) + (n << 2))
#define PGout(n)                                                          \
  *(volatile unsigned int *)((GPIOG_ODR_ADDR & 0xF0000000) + 0x02000000 + \
                             ((GPIOG_ODR_ADDR & 0x00FFFFFF) << 5) + (n << 2))

#define PAin(n)                                                           \
  *(volatile unsigned int *)((GPIOA_IDR_ADDR & 0xF0000000) + 0x02000000 + \
                             ((GPIOA_IDR_ADDR & 0x00FFFFFF) << 5) + (n << 2))
#define PBin(n)                                                           \
  *(volatile unsigned int *)((GPIOB_IDR_ADDR & 0xF0000000) + 0x02000000 + \
                             ((GPIOB_IDR_ADDR & 0x00FFFFFF) << 5) + (n << 2))
#define PCin(n)                                                           \
  *(volatile unsigned int *)((GPIOC_IDR_ADDR & 0xF0000000) + 0x02000000 + \
                             ((GPIOC_IDR_ADDR & 0x00FFFFFF) << 5) + (n << 2))
#define PDin(n)                                                           \
  *(volatile unsigned int *)((GPIOD_IDR_ADDR & 0xF0000000) + 0x02000000 + \
                             ((GPIOD_IDR_ADDR & 0x00FFFFFF) << 5) + (n << 2))
#define PEin(n)                                                           \
  *(volatile unsigned int *)((GPIOE_IDR_ADDR & 0xF0000000) + 0x02000000 + \
                             ((GPIOE_IDR_ADDR & 0x00FFFFFF) << 5) + (n << 2))
#define PFin(n)                                                           \
  *(volatile unsigned int *)((GPIOF_IDR_ADDR & 0xF0000000) + 0x02000000 + \
                             ((GPIOF_IDR_ADDR & 0x00FFFFFF) << 5) + (n << 2))
#define PGin(n)                                                           \
  *(volatile unsigned int *)((GPIOG_IDR_ADDR & 0xF0000000) + 0x02000000 + \
                             ((GPIOG_IDR_ADDR & 0x00FFFFFF) << 5) + (n << 2))

#endif // !__GPIO_BIT_BAND_H
