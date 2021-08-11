/**
 ****************************************************************************************
 *
 * @file gr55xx_delay.h
 *
 * @brief PERIPHERAL API DELAY DRIVER
 *
 ****************************************************************************************
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
 ****************************************************************************************
 */

#ifndef __GR55xx_DELAY_H__
#define __GR55xx_DELAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "gr55xx.h"


#if defined ( __CC_ARM )

#ifndef __STATIC_FORCEINLINE
#define __STATIC_FORCEINLINE                   static __forceinline     /**< Static inline define */
#endif

#elif defined ( __GNUC__ )

#ifndef __STATIC_FORCEINLINE
#define __STATIC_FORCEINLINE                   __attribute__((always_inline)) static inline /**< Static inline define */
#endif

#else

#ifndef __STATIC_FORCEINLINE
#define __STATIC_FORCEINLINE                   __STATIC_INLINE          /**< Static inline define */
#endif

#endif

/**
 * @brief Function for delaying execution for number of microseconds.
 *
 * @note GR55xxx is based on ARM Cortex-M4, and this fuction is based on Data Watchpoint and Trace (DWT) unit so delay is precise.
 *
 * @param number_of_us
 *
 */
__STATIC_FORCEINLINE void delay_us(uint32_t number_of_us)
{
#if defined(GR551xx)
    uint8_t clocks[] = {64, 48, 16, 24, 16, 32};
    uint32_t cycles = number_of_us * (clocks[AON->PWR_RET01 & AON_PWR_REG01_SYS_CLK_SEL]);
#else
    uint8_t clocks[] = {96, 64, 16, 48, 24, 16, 32};
    uint32_t cycles = number_of_us * clocks[AON_CTL->MCU_CLK_CTRL & AON_CTL_MCU_CLK_CTRL_SEL];
#endif
    
    if (number_of_us == 0)
    {
        return;
    }

    // Save the DEMCR register value which is used to restore it
    uint32_t core_debug_initial = CoreDebug->DEMCR;
    // Enable DWT
    CoreDebug->DEMCR = core_debug_initial | CoreDebug_DEMCR_TRCENA_Msk;

    // Save the CTRL register value which is used to restore it
    uint32_t dwt_ctrl_initial = DWT->CTRL;
    // Enable cycle counter
    DWT->CTRL = dwt_ctrl_initial | DWT_CTRL_CYCCNTENA_Msk;

    // Get start value of the cycle counter.
    uint32_t cyccnt_initial = DWT->CYCCNT;

    // Wait time end
    while ((DWT->CYCCNT - cyccnt_initial) < cycles)
    {}

    // Restore registers.
    DWT->CTRL = dwt_ctrl_initial;
    CoreDebug->DEMCR = core_debug_initial;
}

/**
 * @brief Function for delaying execution for number of milliseconds.
 *
 * @note GR55xx is based on ARM Cortex-M4, and this fuction is based on Data Watchpoint and Trace (DWT) unit so delay is precise.
 *
 * @note Function internally calls @ref delay_us so the maximum delay is the
 * same as in case of @ref delay_us.
 *
 * @param number_of_ms
 *
 */
__STATIC_FORCEINLINE void delay_ms(uint32_t number_of_ms)
{
    delay_us(1000 * number_of_ms);
    return;
}

#ifdef __cplusplus
}
#endif

#endif /* __GR55xx_DELAY_H__ */
