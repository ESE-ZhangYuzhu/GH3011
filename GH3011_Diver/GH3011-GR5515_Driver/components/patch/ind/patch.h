/**
 ******************************************************************************
 *
 * @file patch.h
 *
 * @brief offer the interface for the patch function based on the FPB of the cortex arm-m4;
 *
 ******************************************************************************
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
 
#ifndef __PATCH_H_
#define __PATCH_H_

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
enum
{
    BIT_CO_LIST_PUSH_BACK,
    BIT_CO_LIST_POP_FRONT,
    BIT_GAPM_SEND_COMPLETE_EVT,
    BIT_SCH_ARB_INSERT,  // it's used to realize the ble idle time notify function, not MANDATORY
    BIT_ATTMDB_SVC_INIT,
    BIT_LLC_PREF_PARAM_COMPUTE, // it's used to implement the anchor point movement, not MANDATORY
    BIT_LLC_HCI_CON_UPD_INFO_SEND, // it's used to implement the anchor point movement accuracy, not MANDATORY
};

/*
 * MACRO DECLARATIONS
 ****************************************************************************************
 */
#define PATCH_ENABLE_FLAG(BIT) (1<<BIT)
//please add the macro for the different application(Only Support 6 patches);
#define MANDATORY_PATCH         ( PATCH_ENABLE_FLAG(BIT_CO_LIST_PUSH_BACK)         \
                                | PATCH_ENABLE_FLAG(BIT_CO_LIST_POP_FRONT)         \
                                | PATCH_ENABLE_FLAG(BIT_GAPM_SEND_COMPLETE_EVT)    \
                                | PATCH_ENABLE_FLAG(BIT_ATTMDB_SVC_INIT)           \
                                | PATCH_ENABLE_FLAG(BIT_LLC_PREF_PARAM_COMPUTE)    \
                                | PATCH_ENABLE_FLAG(BIT_LLC_HCI_CON_UPD_INFO_SEND) \
                                )

#define OPTIMIZING_PATCH        0
                               
#define THROUGHPUT_PATCH        MANDATORY_PATCH

#define MULTI_LINK_PATCH        ( PATCH_ENABLE_FLAG(BIT_CO_LIST_PUSH_BACK)         \
                                | PATCH_ENABLE_FLAG(BIT_CO_LIST_POP_FRONT)         \
                                | PATCH_ENABLE_FLAG(BIT_GAPM_SEND_COMPLETE_EVT)    \
                                | PATCH_ENABLE_FLAG(BIT_GAPM_SMP_RESOLV_OP_CONT)   \
                                | PATCH_ENABLE_FLAG(BIT_ATTMDB_SVC_INIT)           \
                                | PATCH_ENABLE_FLAG(BIT_LLC_PREF_PARAM_COMPUTE)    \
                                )
                                
#define IDLE_NOTIFY_PATCH       ( PATCH_ENABLE_FLAG(BIT_CO_LIST_PUSH_BACK)         \
                                | PATCH_ENABLE_FLAG(BIT_CO_LIST_POP_FRONT)         \
                                | PATCH_ENABLE_FLAG(BIT_GAPM_SEND_COMPLETE_EVT)    \
                                | PATCH_ENABLE_FLAG(BIT_SCH_ARB_INSERT)            \
                                | PATCH_ENABLE_FLAG(BIT_ATTMDB_SVC_INIT)           \
                                | PATCH_ENABLE_FLAG(BIT_LLC_PREF_PARAM_COMPUTE)    \
                                )

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 * @brief  The enable of the patch featurn based on the FPB of the Cortex ARM-m4.
 *
 * @param[in]  patch_flag: The flag used to control the function to be selected as the patch function,one bit for one function.
 *
 * @note This parameter can be a combiantion of the following values:
 *       @arg @ref (1<<BIT_LLD_LLCP_OPCODE_IS_INVALID ,    )
 *       @arg @ref (1<<BIT_LLD_TEST_ISR,                )
 *       please used the MACRO PATCH_ENABLE_FLAG(BIT) just like the MANDATORY_PATCH;
 *       and the different MACRO maybe defined for the different application;
 *****************************************************************************************
 */
extern void set_patch_flag(uint32_t patch_flag);

/**
 *****************************************************************************************
 * @brief  Patch Enabling Function 
 *         This function can not be used directly. It needs to be registered with FPB unit 
 *         and automatically enabled by the system.
 *****************************************************************************************
 */
void fpb_patch_enable(void);

#endif  // __PATCH_H_

