/**
 *****************************************************************************************
 *
 * @file   interrupt_gr55xx.c
 *
 * @brief  Interrupt Service Routines.
 *
 *****************************************************************************************
 
 * @attention
  #####Copyright (c) 2019 GOODIX
  All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of GOODIX nor the names of its contributors may be used
    to endorse or promote products derived from this software without
    specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************************
 */

/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "gr55xx_sys.h"
#include "gr55xx_hal.h"
#include "custom_config.h"


/******************************************************************************/
/*           Cortex-M4F Processor Interruption and Exception Handlers         */ 
/*           Add here the Interrupt Handler for the BLE peripheral(s)         */ 
/******************************************************************************/

/**
 ****************************************************************************************
 * @brief  SVC Interrupt Handler
 * @retval void
 ****************************************************************************************
 */
#if defined ( __CC_ARM ) 
__asm void SVC_Handler (void)
{
    PRESERVE8
    IMPORT   SVC_handler_proc
                
    TST      LR,#4                   ; Called from Handler Mode?
    MRSNE    R12,PSP                 ; Yes, use PSP
    MOVEQ    R12,SP                  ; No, use MSP
    PUSH     {R0-R3,LR}

    MOV      R0, R12 
    BL       SVC_handler_proc
    MOV      R12, R0
    POP      {R0-R3}
    CMP      R12,#0                      //make sure current point isn't null
    BLXNE    R12
    POP      {LR}

    BX      LR                      ; RETI
SVC_Dead
    B       SVC_Dead                ; None Existing SVC
    ALIGN
}

#elif defined ( __GNUC__ )

void __attribute__((naked))SVC_Handler (void)
{
    __asm("TST R14,$4\n");
    __asm("IT NE\n");
    __asm("MRSNE   R12,PSP\n");
    __asm("IT EQ\n");
    __asm("MOVEQ   R12,SP\n");
    __asm("PUSH    {R0-R3,LR}\n");
    __asm("MOV  R0, R12\n");
    __asm("BL  SVC_handler_proc\n");
    __asm("MOV  R12, R0\n");
    __asm("POP {R0-R3}\n");
    __asm("CMP R12,$0\n");
    __asm("IT NE\n");
    __asm("BLXNE     R12\n");
    __asm("POP {LR}\n");
    __asm("BX      LR\n");
}

#elif defined (__ICCARM__)

extern uint32_t *SVC_Table;
extern uint32_t get_patch_rep_addr(uint32_t ori_func);
TINY_RAM_SECTION uint32_t SVC_handler_proc(uint32_t *svc_args)
{
    uint16_t svc_cmd;
    uint32_t svc_func_addr;
    uint32_t func_addr=0;
    svc_func_addr =svc_args[6];
    svc_cmd = *((uint16_t*)svc_func_addr-1);
    if((svc_cmd<=0xDFFF)&&(svc_cmd>=0xDF00))
    {
        func_addr =(uint32_t)SVC_Table[svc_cmd&(0xFF)];
        return func_addr ;
    }
    else
    {
        func_addr=get_patch_rep_addr(svc_func_addr);
        svc_args[6]=func_addr;
        return 0;
    }
}

void __attribute__((naked))SVC_Handler (void)
{
    asm volatile ( 
                  "TST R14,#4\n\t"
                  "IT NE\n\t"
                  "MRSNE   R12,PSP\n\t"
                  "IT EQ\n"
                  "MOVEQ   R12,SP \n\t"
                  "PUSH    {R0-R3,LR} \n\t"
                  "MOV  R0, R12 \n\t"
                  "BL  SVC_handler_proc \n\t"
                  "MOV  R12, R0 \n\t"
                  "POP {R0-R3} \n\t"
                  "CMP R12,#0\n\t"
                  "IT NE\n\t"
                  "BLXNE R12\n\t"
                  "POP {LR}\n\t"
                  "BX      LR\n\t");
  
}
#endif

/**
 ****************************************************************************************
 * @brief  hardfault Interrupt Handler
 * @retval void
 ****************************************************************************************
 */
#if defined ( __CC_ARM )

__WEAK void cortex_backtrace_fault_handler(void)
{
    while(1);
}

__asm void HardFault_Handler (void) 
{
#if SYS_FAULT_TRACE_ENABLE
    PRESERVE8
    IMPORT  cortex_backtrace_fault_handler  
    MOV     r0, lr        
    MOV     r1, sp
    BL      cortex_backtrace_fault_handler
#endif
Fault_Loop
    BL      Fault_Loop          
    ALIGN 
}

#elif defined ( __GNUC__ )

__WEAK void hardfault_trace_handler(unsigned int *args)
{
    while(1);
}

void __attribute__((naked))HardFault_Handler (void)
{
    __asm("TST     LR,#4\n");
    __asm("ITE     EQ\n");
    __asm("MRSEQ   R0,MSP\n");
    __asm("MRSNE   R0,PSP\n");
    __asm("BL      hardfault_trace_handler\n");
    while (1);
}

#endif


//void HardFault_Handler (void)
//{
//    while (1);
//}

/**
 ****************************************************************************************
 * @brief  MemManage fault Interrupt Handler
 * @retval  void
 ****************************************************************************************
 */
void MemManage_Handler(void)
{
    while (1);
}

/**
 ****************************************************************************************
 * @brief  Bus Fault Interrupt Handler
 * @retval  void
 ****************************************************************************************
 */
void BusFault_Handler(void)
{
    while (1);
}

/**
 ****************************************************************************************
 * @brief  UsageFault Interrupt Handler
 * @retval  void
 ****************************************************************************************
 */
void UsageFault_Handler(void)
{
    while (1);
}

/**
 ****************************************************************************************
 * @brief  sleep timer Interrupt Handler
 * @retval  void
 ****************************************************************************************
 */
void SLPTIMER_IRQHandler(void)
{
    hal_pwr_sleep_timer_irq_handler();
}

/**
 ****************************************************************************************
 * @brief  SysTick Interrupt Handler
 * @retval  void
 ****************************************************************************************
 */
__WEAK void SysTick_Handler(void)
{

}
