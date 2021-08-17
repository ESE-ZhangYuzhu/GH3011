/**
 ******************************************************************************
 * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c
 * @author  MCD Application Team
 * @version V3.5.0
 * @date    08-April-2011
 * @brief   Main Interrupt Service Routines.
 *          This file provides template for all exceptions handler and
 *          peripherals interrupt service routine.
 ******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "GH3011_ISR.h"
#include "GPIO_Bit_Band.h"
#include <stdio.h>

/** @addtogroup STM32F10x_StdPeriph_Template
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * @brief  This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void) {}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  This function handles SVCall exception.
 * @param  None
 * @retval None
 */
void SVC_Handler(void) {}

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler(void) {}

/**
 * @brief  This function handles PendSVC exception.
 * @param  None
 * @retval None
 */
void PendSV_Handler(void) {}

/**
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{
  while (1)
  {
    printf("\n\rSysTick Handler ERROR!!!!");
  }
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
 * @brief  This function handles PPP interrupt request.
 * @param  None
 * @retval None
 */
/*void PPP_IRQHandler(void)
{
}*/

__IO unsigned char TIM6_500ms_Flag = 0;
__IO unsigned char TIM6_1s_Flag = 0;

void TIM6_IRQHandler(void) // 500ms
{
  static u8 TIM6_500ms_Counter = 0;
  if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
  {
    /********** User Code **************/
    TIM6_500ms_Flag = 1;
    TIM6_500ms_Counter++;
    if (TIM6_500ms_Counter == 2)
    {
      TIM6_500ms_Counter = 0;
      TIM6_1s_Flag = 1;
    }
    /********** User Code End***********/
    TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
  }
}

void TIM7_IRQHandler(void) // 20ms
{
  if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
  {
    /********** User Code **************/
    TIM_Cmd(TIM7, DISABLE);
    if (0 == PCin(2))
      GH3011_ADT_WeraDetect_Start(&GH3011);
    /********** User Code End***********/
    TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
  }
}

__IO unsigned short int ADC1_Channel16_Value = 0;

void ADC1_2_IRQHandler(void)
{
  if (ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET)
  {
    /********** User Code **************/
    ADC1_Channel16_Value = ADC_GetConversionValue(ADC1);
    /********** User Code End***********/
    ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
  }
}

// Key
void EXTI2_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line2) != RESET)
  {
    /********** User Code **************/
    if (PCin(2) == 0)
    {
      TIM_Cmd(TIM7, ENABLE);
    }
    /********** User Code End***********/
    EXTI_ClearITPendingBit(EXTI_Line2);
  }
}

// GH3011
void EXTI3_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line3) != RESET)
  {
    /********** User Code **************/
    if (PAin(3) == 1)
    {
      gh30x_int_msg_handler();
    }
    /********** User Code End***********/
    EXTI_ClearITPendingBit(EXTI_Line3);
  }
}

/**
 * @}
 */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
