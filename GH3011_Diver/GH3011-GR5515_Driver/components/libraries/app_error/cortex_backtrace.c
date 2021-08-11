/**
 *****************************************************************************************
 *
 * @file cortex_backtrace.c
 *
 * @brief Cortex Backtrace Implementation.
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
#include "cortex_backtrace.h"
#include "app_error.h"
#include "fault_trace.h"
#include "app_assert.h"
#include "app_log.h"
#include <stdbool.h>
#include <string.h>

#if __STDC_VERSION__ < 199901L
    #error "must be C99 or higher. try to add '-std=c99' to compile parameters"
#endif

/*
 * DEFINE
 *****************************************************************************************
 */
#if defined(__CC_ARM)
    #define CSTACK_BLOCK_NAME               ARM_LIB_STACKHEAP     /**< C stack block name: ARM_LIB_STACKHEAP. */
    #define CODE_SECTION_NAME               FLASH_CODE            /**< Code section name: ER_FLASH. */

    #define SECTION_START(_name_)           _name_##$$Base
    #define SECTION_END(_name_)             _name_##$$Limit
    #define IMAGE_SECTION_START(_name_)     Image$$##_name_##$$Base
    #define IMAGE_SECTION_END(_name_)       Image$$##_name_##$$ZI$$Limit
    #define CSTACK_BLOCK_START(_name_)      IMAGE_SECTION_START(_name_)
    #define CSTACK_BLOCK_END(_name_)        IMAGE_SECTION_END(_name_)
    #define CODE_SECTION_START(_name_)      IMAGE_SECTION_START(_name_)
    #define CODE_SECTION_END(_name_)        IMAGE_SECTION_END(_name_)

    extern const int CSTACK_BLOCK_START(CSTACK_BLOCK_NAME);
    extern const int CSTACK_BLOCK_END(CSTACK_BLOCK_NAME);
    extern const int CODE_SECTION_START(CODE_SECTION_NAME);
    extern const int CODE_SECTION_END(CODE_SECTION_NAME);
#elif defined(__ICCARM__)
    #define CSTACK_BLOCK_NAME          "CSTACK"              /**< C stack block name, default is 'CSTACK'. */
    #define CODE_SECTION_NAME          ".text"               /**< Code section name, default is '.text'. */

    #pragma section = CMB_CSTACK_BLOCK_NAME
    #pragma section = CMB_CODE_SECTION_NAME
#elif defined(__GNUC__)
    #define CSTACK_BLOCK_START         _sstack               /**< C stack block start address, defined on linker script file, default is _sstack. */
    #define CSTACK_BLOCK_END           _estack               /**< C stack block end address, defined on linker script file, default is _estack. */
    #define CODE_SECTION_START         _stext                /**< code section start address, defined on linker script file, default is _stext. */
    #define CODE_SECTION_END           _etext                /**< code section end address, defined on linker script file, default is _etext. */

    extern const int CSTACK_BLOCK_START;
    extern const int CSTACK_BLOCK_END;
    extern const int CODE_SECTION_START;
    extern const int CODE_SECTION_END;
#else
    #error "not supported compiler"
#endif

/*
 * ENUMERATION
 *****************************************************************************************
 */
enum
{
    CB_PRINT_ASSERT_ON_THREAD,
    CB_PRINT_ASSERT_ON_HANDLER,
    CB_PRINT_THREAD_STACK_INFO,
    CB_PRINT_MAIN_STACK_INFO,
    CB_PRINT_THREAD_STACK_OVERFLOW,
    CB_PRINT_MAIN_STACK_OVERFLOW,
    CB_PRINT_CALL_STACK_INFO,
    CB_PRINT_CALL_STACK_ERR,
    CB_PRINT_FAULT_ON_THREAD,
    CB_PRINT_FAULT_ON_HANDLER,
    CB_PRINT_REGS_TITLE,
    CB_PRINT_HFSR_VECTBL,
    CB_PRINT_MFSR_IACCVIOL,
    CB_PRINT_MFSR_DACCVIOL,
    CB_PRINT_MFSR_MUNSTKERR,
    CB_PRINT_MFSR_MSTKERR,
    CB_PRINT_MFSR_MLSPERR,
    CB_PRINT_BFSR_IBUSERR,
    CB_PRINT_BFSR_PRECISERR,
    CB_PRINT_BFSR_IMPREISERR,
    CB_PRINT_BFSR_UNSTKERR,
    CB_PRINT_BFSR_STKERR,
    CB_PRINT_BFSR_LSPERR,
    CB_PRINT_UFSR_UNDEFINSTR,
    CB_PRINT_UFSR_INVSTATE,
    CB_PRINT_UFSR_INVPC,
    CB_PRINT_UFSR_NOCP,
    CB_PRINT_UFSR_UNALIGNED,
    CB_PRINT_UFSR_DIVBYZERO0,
    CB_PRINT_DFSR_HALTED,
    CB_PRINT_DFSR_BKPT,
    CB_PRINT_DFSR_DWTTRAP,
    CB_PRINT_DFSR_VCATCH,
    CB_PRINT_DFSR_EXTERNAL,
    CB_PRINT_MMAR,
    CB_PRINT_BFAR,
};

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static const char * const s_print_info[] =
{
        [CB_PRINT_ASSERT_ON_THREAD]      = "Assert on thread %s",
        [CB_PRINT_ASSERT_ON_HANDLER]     = "Assert on interrupt or bare metal(no OS) environment",
        [CB_PRINT_THREAD_STACK_INFO]     = "===== Thread stack information =====",
        [CB_PRINT_MAIN_STACK_INFO]       = "====== Main stack information ======",
        [CB_PRINT_THREAD_STACK_OVERFLOW] = "Error: Thread stack(%08x) was overflow",
        [CB_PRINT_MAIN_STACK_OVERFLOW]   = "Error: Main stack(%08x) was overflow",
        [CB_PRINT_CALL_STACK_INFO]       = "Call stack info : %.*s",
        [CB_PRINT_CALL_STACK_ERR]        = "Dump call stack has an error",
        [CB_PRINT_FAULT_ON_THREAD]       = "Fault on thread %s",
        [CB_PRINT_FAULT_ON_HANDLER]      = "Fault on interrupt or bare metal(no OS) environment",
        [CB_PRINT_REGS_TITLE]            = "====== Registers information =======",
        [CB_PRINT_HFSR_VECTBL]           = "Hard fault is caused by failed vector fetch",
        [CB_PRINT_MFSR_IACCVIOL]         = "Memory management fault is caused by instruction access violation",
        [CB_PRINT_MFSR_DACCVIOL]         = "Memory management fault is caused by data access violation",
        [CB_PRINT_MFSR_MUNSTKERR]        = "Memory management fault is caused by unstacking error",
        [CB_PRINT_MFSR_MSTKERR]          = "Memory management fault is caused by stacking error",
        [CB_PRINT_MFSR_MLSPERR]          = "Memory management fault is caused by floating-point lazy state preservation",
        [CB_PRINT_BFSR_IBUSERR]          = "Bus fault is caused by instruction access violation",
        [CB_PRINT_BFSR_PRECISERR]        = "Bus fault is caused by precise data access violation",
        [CB_PRINT_BFSR_IMPREISERR]       = "Bus fault is caused by imprecise data access violation",
        [CB_PRINT_BFSR_UNSTKERR]         = "Bus fault is caused by unstacking error",
        [CB_PRINT_BFSR_STKERR]           = "Bus fault is caused by stacking error",
        [CB_PRINT_BFSR_LSPERR]           = "Bus fault is caused by floating-point lazy state preservation",
        [CB_PRINT_UFSR_UNDEFINSTR]       = "Usage fault is caused by attempts to execute an undefined instruction",
        [CB_PRINT_UFSR_INVSTATE]         = "Usage fault is caused by attempts to switch to an invalid state (e.g., ARM)",
        [CB_PRINT_UFSR_INVPC]            = "Usage fault is caused by attempts to do an exception with a bad value in the EXC_RETURN number",
        [CB_PRINT_UFSR_NOCP]             = "Usage fault is caused by attempts to execute a coprocessor instruction",
        [CB_PRINT_UFSR_UNALIGNED]        = "Usage fault is caused by indicates that an unaligned access fault has taken place",
        [CB_PRINT_UFSR_DIVBYZERO0]       = "Usage fault is caused by Indicates a divide by zero has taken place (can be set only if DIV_0_TRP is set)",
        [CB_PRINT_DFSR_HALTED]           = "Debug fault is caused by halt requested in NVIC",
        [CB_PRINT_DFSR_BKPT]             = "Debug fault is caused by BKPT instruction executed",
        [CB_PRINT_DFSR_DWTTRAP]          = "Debug fault is caused by DWT match occurred",
        [CB_PRINT_DFSR_VCATCH]           = "Debug fault is caused by Vector fetch occurred",
        [CB_PRINT_DFSR_EXTERNAL]         = "Debug fault is caused by EDBGRQ signal asserted",
        [CB_PRINT_MMAR]                  = "The memory management fault occurred address is %08x",
        [CB_PRINT_BFAR]                  = "The bus fault occurred address is %08x",
};

static uint32_t s_main_stack_start_addr       = 0;
static uint32_t s_main_stack_size             = 0;
static uint32_t s_code_start_addr             = 0;
static uint32_t s_code_size                   = 0;
static bool     s_is_stack_overflow           = false;
static bool     s_is_on_thread_before_fault   = false;
static bool     s_is_on_fault                 = false;

static char     s_call_stack_info[APP_ERROR_CALL_STACK_DEPTH_MAX * (8 + 1)] = { 0 };

static cb_hard_fault_regs_t s_regs;
/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**@brief Include or export for supported cb_psp_get and cb_sp_get function */
#if APP_IS_USING_FREEROTS
#if defined(__CC_ARM)
    static __inline __asm uint32_t cb_psp_get(void)
    {
        mrs r0, psp
        bx lr
    }
    static __inline __asm uint32_t cb_sp_get(void)
    {
        mov r0, sp
        bx lr
    }
#elif defined(__ICCARM__)
// IAR iccarm specific functions Close Raw Asm Code Warning.
#pragma diag_suppress=Pe940
    static uint32_t cb_psp_get(void)
    {
      __asm("mrs r0, psp");
      __asm("bx lr");
    }
    static uint32_t cb_sp_get(void)
    {
      __asm("mov r0, sp");
      __asm("bx lr");
    }
#pragma diag_default=Pe940
#elif defined(__GNUC__)
    __attribute__( ( always_inline ) ) static inline uint32_t cb_psp_get(void)
    {
        register uint32_t result;
        __asm volatile ("MRS %0, psp\n" : "=r" (result) );
        return(result);
    }
    __attribute__( ( always_inline ) ) static inline uint32_t cb_sp_get(void)
    {
        register uint32_t result;
        __asm volatile ("MOV %0, sp\n" : "=r" (result) );
        return(result);
    }
#else
    #error "not supported compiler"
#endif
#endif

#if APP_IS_USING_FREEROTS
/**
 *****************************************************************************************
 * Get current thread stack information.
 *
 * @param[in] sp:           Stack current pointer.
 * @param[in] p_start_addr: Pointer to stack start address.
 * @param[in] p_stack_size: Pointer to size of stack.
 *****************************************************************************************
 */
static void cb_cur_thread_stack_info_get(uint32_t sp, uint32_t *p_start_addr, uint32_t *p_stack_size)
{
    APP_ASSERT_CHECK(p_start_addr);
    APP_ASSERT_CHECK(p_stack_size);

    *p_start_addr = (uint32_t)vTaskStackAddr();
    *p_stack_size = vTaskStackSize() * sizeof( StackType_t );
}

/**
 *****************************************************************************************
 * Get current thread name information.
 *****************************************************************************************
 */
static const char *cb_cur_thread_name_get(void)
{
    return vTaskName();
}
#endif

#if APP_ERROR_DUMP_STACK_INFO_ENABLE
/**
 *****************************************************************************************
 * Dump current stack information.
 *
 * @param[in] stack_start_addr: Stack start address.
 * @param[in] stack_size:       Size of stack.
 * @param[in] p_stack:          Pointer to stack.
 *****************************************************************************************
 */
static void cb_stack_info_dump(uint32_t stack_start_addr, uint32_t stack_size, uint32_t *p_stack)
{
    if (s_is_stack_overflow)
    {
        if (s_is_on_thread_before_fault)
        {
            APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_THREAD_STACK_OVERFLOW], p_stack);
        }
        else
        {
            APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_MAIN_STACK_OVERFLOW], p_stack);
        }

        if ((uint32_t)p_stack < stack_start_addr)
        {
            p_stack = (uint32_t *)stack_start_addr;
        } 
        else if ((uint32_t) p_stack > stack_start_addr + stack_size)
        {
            p_stack = (uint32_t *)(stack_start_addr + stack_size);
        }
    }
    else
    {
        if (s_is_on_thread_before_fault)
        {
            APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_THREAD_STACK_INFO]);
        }
        else
        {
            APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_MAIN_STACK_INFO]);
        }
    }

    for (; (uint32_t)p_stack < stack_start_addr + stack_size; p_stack++)
    {
        APP_ERROR_INFO_PRINT("  addr: %08x    data: %08x", p_stack, *p_stack);
    }
    APP_ERROR_INFO_PRINT("====================================");
}
#endif

#if (__CORTEX_M != CB_CPU_ARM_CORTEX_M0)
/**
 *****************************************************************************************
 * Fault diagnosis then print cause of fault
 *****************************************************************************************
 */
static void cb_fault_diagnosis(void)
{
    if (s_regs.hfsr.bits.VECTBL)
    {
        APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_HFSR_VECTBL]);
    }

    if (s_regs.hfsr.bits.FORCED)
    {
        // Memory Management Fault.
        if (s_regs.mfsr.value)
        {
            if (s_regs.mfsr.bits.IACCVIOL)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_MFSR_IACCVIOL]);
            }

            if (s_regs.mfsr.bits.DACCVIOL)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_MFSR_DACCVIOL]);
            }

            if (s_regs.mfsr.bits.MUNSTKERR)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_MFSR_MUNSTKERR]);
            }

            if (s_regs.mfsr.bits.MSTKERR)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_MFSR_MSTKERR]);
            }

#if (__CORTEX_M == CB_CPU_ARM_CORTEX_M4) || (__CORTEX_M == CB_CPU_ARM_CORTEX_M7)
            if (s_regs.mfsr.bits.MLSPERR)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_MFSR_MLSPERR]);
            }
#endif

            if (s_regs.mfsr.bits.MMARVALID)
            {
                if (s_regs.mfsr.bits.IACCVIOL || s_regs.mfsr.bits.DACCVIOL)
                {
                    APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_MMAR], s_regs.mmar);
                }
            }
        }

        //Bus Fault
        if (s_regs.bfsr.value)
        {
            if (s_regs.bfsr.bits.IBUSERR)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_BFSR_IBUSERR]);
            }

            if (s_regs.bfsr.bits.PRECISERR)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_BFSR_PRECISERR]);
            }

            if (s_regs.bfsr.bits.IMPREISERR)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_BFSR_IMPREISERR]);
            }

            if (s_regs.bfsr.bits.UNSTKERR)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_BFSR_UNSTKERR]);
            }

            if (s_regs.bfsr.bits.STKERR)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_BFSR_STKERR]);
            }

#if (__CORTEX_M == CB_CPU_ARM_CORTEX_M4) || (__CORTEX_M == CB_CPU_ARM_CORTEX_M7)
            if (s_regs.bfsr.bits.LSPERR)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_BFSR_LSPERR]);
            }
#endif

            if (s_regs.bfsr.bits.BFARVALID)
            {
                if (s_regs.bfsr.bits.PRECISERR)
                {
                    APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_BFAR], s_regs.bfar);
                }
            }
        }

        // Usage Fault
        if (s_regs.ufsr.value)
        {
            if (s_regs.ufsr.bits.UNDEFINSTR)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_UFSR_UNDEFINSTR]);
            }

            if (s_regs.ufsr.bits.INVSTATE)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_UFSR_INVSTATE]);
            }

            if (s_regs.ufsr.bits.INVPC)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_UFSR_INVPC]);
            }

            if (s_regs.ufsr.bits.NOCP)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_UFSR_NOCP]);
            }

            if (s_regs.ufsr.bits.UNALIGNED)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_UFSR_UNALIGNED]);
            }

            if (s_regs.ufsr.bits.DIVBYZERO0)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_UFSR_DIVBYZERO0]);
            }
        }
    }

    // Debug Fault
    if (s_regs.hfsr.bits.DEBUGEVT)
    {
        if (s_regs.dfsr.value)
        {
            if (s_regs.dfsr.bits.HALTED)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_DFSR_HALTED]);
            }

            if (s_regs.dfsr.bits.BKPT)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_DFSR_BKPT]);
            }

            if (s_regs.dfsr.bits.DWTTRAP)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_DFSR_DWTTRAP]);
            }

            if (s_regs.dfsr.bits.VCATCH)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_DFSR_VCATCH]);
            }

            if (s_regs.dfsr.bits.EXTERNAL)
            {
                APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_DFSR_EXTERNAL]);
            }
        }
    }
}
#endif

#if (__CORTEX_M == CB_CPU_ARM_CORTEX_M4) || (__CORTEX_M == CB_CPU_ARM_CORTEX_M7)
static uint32_t cb_statck_fpu_reg_del(uint32_t fault_handler_lr, uint32_t sp)
{
    bool statck_has_fpu_regs = (fault_handler_lr & (1UL << 4)) == 0 ? true : false;

    // The stack has S0~S15 and FPSCR registers when statck_has_fpu_regs is true, double word align.
    return statck_has_fpu_regs == true ? sp + sizeof(size_t) * 18 : sp;
}
#endif

/**
 *****************************************************************************************
 * Backtrace function call stack
 *
 * @param[in] p_buffer: Pointer to call stack buffer.
 * @param[in] size:     Size of call stack buffer.
 * @param[in] sp:      Stack pointer
 *
 * @return Depth
 *****************************************************************************************
 */
static uint32_t cb_backtrace_call_stack(uint32_t *p_buffer, uint32_t size, uint32_t sp)
{
    uint32_t stack_start_addr = s_main_stack_start_addr;
    uint32_t depth            = 0;
    uint32_t stack_size       = s_main_stack_size;
    uint32_t pc;

    bool is_regs_saved_lr_valid = false;

    if (s_is_on_fault)
    {
        if (!s_is_stack_overflow)
        {
            // First depth is PC
            p_buffer[depth++] = s_regs.saved.pc;

            // Second depth is from LR, so need decrease a word to PC
            pc = s_regs.saved.lr - sizeof(uint32_t);

            if ((pc >= s_code_start_addr) && \
                (pc <= s_code_start_addr + s_code_size) && \
                (depth < APP_ERROR_CALL_STACK_DEPTH_MAX) && \
                (depth < size))
            {
                p_buffer[depth++] = pc;
                is_regs_saved_lr_valid = true;
            }
        }

#if APP_IS_USING_FREEROTS
        // Program is running on thread before fault.
        if (s_is_on_thread_before_fault)
        {
            cb_cur_thread_stack_info_get(sp, &stack_start_addr, &stack_size);
        }
    } 
    else 
    {
        // OS environment.
        if (cb_sp_get() == cb_psp_get())
        {
            cb_cur_thread_stack_info_get(sp, &stack_start_addr, &stack_size);
        }
#endif

    }

    if (s_is_stack_overflow)
    {
        if (sp < stack_start_addr)
        {
            sp = stack_start_addr;
        }
        else if (sp > stack_start_addr + stack_size)
        {
            sp = stack_start_addr + stack_size;
        }
    }

    // Copy called function address.
    for (; sp < stack_start_addr + stack_size; sp += sizeof(uint32_t))
    {
        // The *sp value may be LR, so need decrease a word to PC.
        pc = *((uint32_t *)sp) - sizeof(uint32_t);

        // The Cortex-M using thumb instruction, so the pc must be an odd number.
        if (pc % 2 == 0)
        {
            continue;
        }

        if ((pc >= s_code_start_addr) && \
            (pc <= s_code_start_addr + s_code_size) && \
            (depth < APP_ERROR_CALL_STACK_DEPTH_MAX) && \
            (depth < size))
        {
            // The second depth function may be already saved, so need ignore repeat.
            if ((depth == 2) && is_regs_saved_lr_valid && (pc == p_buffer[1]))
            {
                continue;
            }
            p_buffer[depth++] = pc;
        }
    }

    return depth;
}

/**
 *****************************************************************************************
 * Dump function call stack
 *
 * @param[in] sp:Pointer to stack.
 *****************************************************************************************
 */
static void cb_call_stack_print(uint32_t sp)
{
    uint32_t i;
    uint32_t cur_depth = 0;
    uint32_t call_stack_buf[APP_ERROR_CALL_STACK_DEPTH_MAX] = {0};

    cur_depth = cb_backtrace_call_stack(call_stack_buf, APP_ERROR_CALL_STACK_DEPTH_MAX, sp);

    for (i = 0; i < cur_depth; i++)
    {
        sprintf(s_call_stack_info + i * (8 + 1), "%08lx", call_stack_buf[i]);
        s_call_stack_info[i * (8 + 1) + 8] = ' ';
    }

    if (cur_depth)
    {
        APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_CALL_STACK_INFO], cur_depth * (8 + 1), s_call_stack_info);
    }
    else
    {
        APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_CALL_STACK_ERR]);
    }
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 * @brief Initialize Cortex backtrace.
 *****************************************************************************************
 */
void cortex_backtrace_fault_handler(uint32_t fault_handler_lr, uint32_t fault_handler_sp)
{
#if SYS_FAULT_TRACE_ENABLE
    hardfault_trace_handler(fault_handler_sp);
#endif
#if defined(__CC_ARM)
    s_main_stack_start_addr = (uint32_t)&CSTACK_BLOCK_START(CSTACK_BLOCK_NAME);
    s_main_stack_size       = (uint32_t)&CSTACK_BLOCK_END(CSTACK_BLOCK_NAME) - s_main_stack_start_addr;
    s_code_start_addr       = (uint32_t)&CODE_SECTION_START(CODE_SECTION_NAME);
    s_code_size             = (uint32_t)&CODE_SECTION_END(CODE_SECTION_NAME) - s_code_start_addr;
#elif defined(__ICCARM__)
    uint32_t s_main_stack_start_addr = (uint32_t)__section_begin(CSTACK_BLOCK_NAME);
    uint32_t s_main_stack_size       = (uint32_t)__section_end(CSTACK_BLOCK_NAME) - s_main_stack_start_addr;
    uint32_t s_code_start_addr       = (uint32_t)__section_begin(CODE_SECTION_NAME);
    uint32_t s_code_size             = (uint32_t)__section_end(CODE_SECTION_NAME) - s_code_start_addr;
#elif defined(__GNUC__)
    uint32_t s_main_stack_start_addr = (uint32_t)(&CSTACK_BLOCK_START);
    uint32_t s_main_stack_size       = (uint32_t)(&CSTACK_BLOCK_END) - s_main_stack_start_addr;
    uint32_t s_code_start_addr       = (uint32_t)(&CODE_SECTION_START);
    uint32_t s_code_size             = (uint32_t)(&CODE_SECTION_END) - s_code_start_addr;
#else
    #error "not supported compiler"
#endif

    uint32_t    stack_pointer   = fault_handler_sp;
    uint32_t    saved_regs_addr = stack_pointer;
    const char *regs_name[]     = { "R0 ", "R1 ", "R2 ", "R3 ", "R12", "LR ", "PC ", "PSR" };

#if APP_ERROR_DUMP_STACK_INFO_ENABLE
    uint32_t    stack_start_addr = s_main_stack_start_addr;
    uint32_t    stack_size       = s_main_stack_size;
#endif

    // Only call once
    APP_ASSERT_CHECK(!s_is_on_fault);

    s_is_on_fault = true;

#if APP_IS_USING_FREEROTS
    s_is_on_thread_before_fault = fault_handler_lr & (1UL << 2);

    // Check which stack was used before (MSP or PSP).
    if (s_is_on_thread_before_fault)
    {
        APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_FAULT_ON_THREAD],
                             cb_cur_thread_name_get() ? cb_cur_thread_name_get() : "NO_NAME");
        stack_pointer   = cb_psp_get();
        saved_regs_addr = stack_pointer;
        
#if APP_ERROR_DUMP_STACK_INFO_ENABLE
        cb_cur_thread_stack_info_get(stack_pointer, &stack_start_addr, &stack_size);
#endif
    }
    else
    {
        APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_FAULT_ON_HANDLER]);
    }
#else
    APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_FAULT_ON_HANDLER]);
#endif

    // Delete saved R0~R3, R12, LR, PC, xPSR registers space
    stack_pointer += sizeof(uint32_t) * 8;

#if (__CORTEX_M == CB_CPU_ARM_CORTEX_M4) || (__CORTEX_M == CB_CPU_ARM_CORTEX_M7)
    stack_pointer = cb_statck_fpu_reg_del(fault_handler_lr, stack_pointer);
#endif

#if APP_ERROR_DUMP_STACK_INFO_ENABLE
    // Check stack overflow.
    if (stack_pointer < stack_start_addr || stack_pointer > stack_start_addr + stack_size)
    {
        s_is_stack_overflow = true;
    }

    // Dump stack information.
    cb_stack_info_dump(stack_start_addr, stack_size, (uint32_t *)stack_pointer);
#endif

    if (!s_is_stack_overflow)
    {
        // Dump register.
        APP_ERROR_INFO_PRINT(s_print_info[CB_PRINT_REGS_TITLE]);

        s_regs.saved.r0        = ((uint32_t *)saved_regs_addr)[0];  // Register R0
        s_regs.saved.r1        = ((uint32_t *)saved_regs_addr)[1];  // Register R1
        s_regs.saved.r2        = ((uint32_t *)saved_regs_addr)[2];  // Register R2
        s_regs.saved.r3        = ((uint32_t *)saved_regs_addr)[3];  // Register R3
        s_regs.saved.r12       = ((uint32_t *)saved_regs_addr)[4];  // Register R12
        s_regs.saved.lr        = ((uint32_t *)saved_regs_addr)[5];  // Link register LR
        s_regs.saved.pc        = ((uint32_t *)saved_regs_addr)[6];  // Program Counter PC
        s_regs.saved.psr.value = ((uint32_t *)saved_regs_addr)[7];  // Program status word PSR

        APP_ERROR_INFO_PRINT("  %s: %08x     %s: %08x", regs_name[0], s_regs.saved.r0,  regs_name[1], s_regs.saved.r1);
        APP_ERROR_INFO_PRINT("  %s: %08x     %s: %08x", regs_name[2], s_regs.saved.r1,  regs_name[3], s_regs.saved.r3);
        APP_ERROR_INFO_PRINT("  %s: %08x     %s: %08x", regs_name[4], s_regs.saved.r12, regs_name[5], s_regs.saved.lr);
        APP_ERROR_INFO_PRINT("  %s: %08x     %s: %08x", regs_name[6], s_regs.saved.pc,  regs_name[7], s_regs.saved.psr.value);
        APP_ERROR_INFO_PRINT("====================================");
    }

// The Cortex-M0 is not support fault diagnosis.
#if (__CORTEX_M != CB_CPU_ARM_CORTEX_M0)
    s_regs.syshndctrl.value = CB_SYSHND_CTRL;  // System Handler Control and State Register
    s_regs.mfsr.value       = CB_NVIC_MFSR;    // Memory Fault Status Register
    s_regs.mmar             = CB_NVIC_MMAR;    // Memory Management Fault Address Register
    s_regs.bfsr.value       = CB_NVIC_BFSR;    // Bus Fault Status Register
    s_regs.bfar             = CB_NVIC_BFAR;    // Bus Fault Manage Address Register
    s_regs.ufsr.value       = CB_NVIC_UFSR;    // Usage Fault Status Register
    s_regs.hfsr.value       = CB_NVIC_HFSR;    // Hard Fault Status Register
    s_regs.dfsr.value       = CB_NVIC_DFSR;    // Debug Fault Status Register
    s_regs.afsr             = CB_NVIC_AFSR;    // Auxiliary Fault Status Register

    cb_fault_diagnosis();
#endif

    cb_call_stack_print(stack_pointer);
    app_log_flush();
}

