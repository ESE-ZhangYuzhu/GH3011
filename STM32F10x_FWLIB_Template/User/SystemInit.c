/*
 * @Author: yz.zhang[zhang.yz2@yuyue.com.cn]
 * @Date: 2021-07-22 18:26:38
 * @LastEditors: Zhang Yuzhu
 * @LastEditTime: 2021-08-13 09:17:32
 * @FilePath: \STM32F10x_FWLIB_Template\User\SystemInit.c
 * @Description:
 */

#include "SystemInit.h"

__IO Word_Type System_Init_Flag;

static void System_Clock_Init(void)
{
  RCC_DeInit();
  RCC_HSEConfig(RCC_HSE_OFF);
  RCC_AdjustHSICalibrationValue(0xF0);
  RCC_HSICmd(ENABLE);
  RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_16); // 8M / 2 * 16 = 64M
  RCC_PLLCmd(ENABLE);
  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
  while (RCC_GetSYSCLKSource() != 0x08)
  {
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
  }

  RCC_HCLKConfig(RCC_SYSCLK_Div1);
  RCC_PCLK1Config(RCC_HCLK_Div2);
  RCC_PCLK2Config(RCC_HCLK_Div1);

  SystemCoreClockUpdate();
  RCC_ITConfig(RCC_IT_HSERDY | RCC_IT_HSIRDY | RCC_IT_LSERDY | RCC_IT_LSIRDY |
                   RCC_IT_PLLRDY,
               DISABLE);
  RCC_LSEConfig(RCC_LSE_OFF);
  RCC_LSICmd(DISABLE);
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq(&RCC_Clocks);
  RCC_ClockSecuritySystemCmd(DISABLE);

  RCC_MCOConfig(RCC_MCO_PLLCLK_Div2);

  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  System_Init_Flag.Bits.b0 = 1;
}

static void GPIO_Function_Init(void)
{
  GPIO_DeInit(GPIOA);
  GPIO_DeInit(GPIOB);
  GPIO_DeInit(GPIOC);
  GPIO_DeInit(GPIOD);
  GPIO_AFIODeInit();

  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_SetBits(GPIOC, GPIO_Pin_0 | GPIO_Pin_1);
  System_Init_Flag.Bits.b1 = 1;
}

static void USART_Function_Init(void)
{
  USART_DeInit(USART1);

  USART_InitTypeDef USART_InitStruct;
  USART_InitStruct.USART_BaudRate = 115200;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStruct.USART_Mode = USART_Mode_Tx;
  USART_InitStruct.USART_Parity = USART_Parity_No;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  USART_Init(USART1, &USART_InitStruct);

  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  USART_Cmd(USART1, ENABLE);
  System_Init_Flag.Bits.b2 = 1;
}

static void BasicTimer_Function_Init(void)
{
  TIM_DeInit(TIM6);

  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
  TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
  TIM_TimeBaseInitStruct.TIM_Prescaler = SystemCoreClock / 1000 - 1; // 1ms
  TIM_TimeBaseInitStruct.TIM_Period = 100 - 1;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStruct);

  TIM_ClearFlag(TIM6, TIM_FLAG_Update);
  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM6, DISABLE);
  System_Init_Flag.Bits.b3 = 1;
}

static void ADC_Function_Init(void)
{
  ADC_DeInit(ADC1);

  ADC_InitTypeDef ADC_InitStruct;
  ADC_StructInit(&ADC_InitStruct);
  ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;

  RCC_ADCCLKConfig(RCC_PCLK2_Div8);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  ADC_Init(ADC1, &ADC_InitStruct);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_13Cycles5);
  ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
  ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);

  /************ ADC1 Calibration ******************/
  ADC_Cmd(ADC1, ENABLE);
  ADC_ResetCalibration(ADC1);
  while (ADC_GetResetCalibrationStatus(ADC1) == SET)
    ;
  ADC_StartCalibration(ADC1);
  while (ADC_GetCalibrationStatus(ADC1) == SET)
    ;
  /************ ADC1 Calibration End **************/
  ADC_ExternalTrigConvCmd(ADC1, DISABLE);
  ADC_TempSensorVrefintCmd(ENABLE);
  System_Init_Flag.Bits.b4 = 1;
}

static void I2C_Function_Init(void)
{
  I2C_DeInit(I2C1);

  I2C_InitTypeDef I2C_InitStruct;
  I2C_StructInit(&I2C_InitStruct);
  I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStruct.I2C_OwnAddress1 = 0x76;
  I2C_InitStruct.I2C_ClockSpeed = 380000;
  I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
  I2C_Init(I2C1, &I2C_InitStruct);

  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_StructInit(&GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; // SCL--PB6 SDA--PB7

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  GPIO_Init(GPIOB, &GPIO_InitStruct);

  I2C_Cmd(I2C1, ENABLE);
  System_Init_Flag.Bits.b5 = 1;
}

static void SPI_Function_Init(void)
{
  SPI_I2S_DeInit(SPI1);

  SPI_InitTypeDef SPI_InitStruct;
  SPI_StructInit(&SPI_InitStruct);
  SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16; // PCLK2 -- 64M / 16 = 4M
  SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
  SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  SPI_Init(SPI1, &SPI_InitStruct);

  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_StructInit(&GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_5; // MOSI -- PA7 SCK -- PA5

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6; // MISO -- PA6
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4; // CS -- PA4
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_SetBits(GPIOA, GPIO_Pin_4);

  SPI_Cmd(SPI1, ENABLE);
  System_Init_Flag.Bits.b6 = 1;
}

static void EXTI_Function_Init(void)
{
  EXTI_DeInit();
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

  /*********************** User Key **************************/
  EXTI_InitTypeDef EXTI_InitStruct;
  EXTI_StructInit(&EXTI_InitStruct);
  EXTI_InitStruct.EXTI_Line = EXTI_Line2;
  EXTI_InitStruct.EXTI_LineCmd = ENABLE;
  EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;

  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_StructInit(&GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource2);

  EXTI_ClearFlag(EXTI_Line2);
  GPIO_Init(GPIOC, &GPIO_InitStruct);
  EXTI_Init(&EXTI_InitStruct);
  /*********************** User Key **************************/

  /*********************** GH3011 **************************/

  EXTI_StructInit(&EXTI_InitStruct);
  EXTI_InitStruct.EXTI_Line = EXTI_Line3;
  EXTI_InitStruct.EXTI_LineCmd = ENABLE;
  EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;

  GPIO_StructInit(&GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource3);

  EXTI_ClearFlag(EXTI_Line3);
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  EXTI_Init(&EXTI_InitStruct);
  /*********************** GH3011 **************************/

  System_Init_Flag.Bits.b7 = 1;
}

static unsigned char NVIC_Function_Init(void)
{
  __IO Byte_Type NVIC_Init_Flag;
  NVIC_Init_Flag.Byte = 0x00;
  // Set interrupt priority group, two Preemption two Sub
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  NVIC_InitTypeDef NVIC_InitStruct;

  /***************** TIM6 ******************/
  NVIC_InitStruct.NVIC_IRQChannel = TIM6_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
  NVIC_Init_Flag.Bits.b0 = 1;
  /***************** TIM6 ******************/

  /***************** ADC1_2 ******************/
  NVIC_InitStruct.NVIC_IRQChannel = ADC1_2_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
  NVIC_Init_Flag.Bits.b1 = 1;
  /***************** ADC1_2 ******************/

  /***************** EXTI ********************/

  /***************** Key ********************/
  NVIC_InitStruct.NVIC_IRQChannel = EXTI2_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
  NVIC_Init_Flag.Bits.b2 = 1;
  /***************** Key ********************/

  /***************** GH3011 *****************/
  NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
  NVIC_Init_Flag.Bits.b2 = 1;
  /***************** GH3011 *****************/

  /***************** EXTI *******************/

  return NVIC_Init_Flag.Byte;
}

uint16_t ALL_System_Driver_Init(void)
{
  System_Init_Flag.Word = 0x0000;
  System_Clock_Init();
  GPIO_Function_Init();
  USART_Function_Init();
  BasicTimer_Function_Init();
  ADC_Function_Init();
  I2C_Function_Init();
  SPI_Function_Init();
  EXTI_Function_Init();

  System_Init_Flag.Byte[1] = NVIC_Function_Init();
  return System_Init_Flag.Word;
}
