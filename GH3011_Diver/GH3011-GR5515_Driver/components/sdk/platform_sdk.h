/**
 ******************************************************************************
 *
 * @file platform_sdk.h
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
 *****************************************************************************************
 */
 
 
/**
 * @addtogroup SYSTEM
 * @{
 */
 /**
  @addtogroup Plat_SDK Platform SDK
  @{
  @brief Definitions and prototypes for the Platform SDK
 */

#ifndef _PLATFORM_SDK_H
#define _PLATFORM_SDK_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "system_gr55xx.h"
#include "gr55xx_hal_def.h"

/**@addtogroup PlAT_SDK_ENUM Enumerations
 * @{ */

/**@brief system clock and run mode. */
typedef enum
{
   XIP_64M = 0,            /**< XIP 64M. */
   XIP_48M,                /**< XIP 48M. */
   XIP_XO16M,              /**< XIP XO 16M. */
   XIP_24M,                /**< XIP 24M. */
   XIP_16M,                /**< XIP 16M. */
   XIP_32M,                /**< XIP 32M. */
   MIRROR_64M,             /**< MIRROR 64M. */
   MIRROR_48M,             /**< MIRROR 48M. */
   MIRROR_XO16M,           /**< MIRROR X) 16M. */
   MIRROR_24M,             /**< MIRROR 24M. */
   MIRROR_16M,             /**< MIRROR 16M. */
   MIRROR_32M,             /**< MIRROR 32M. */
} table_idx_t;

/**@brief sdk clock type. */
typedef enum
{
    RNG_OSC_CLK = 0,       /**< RNG OSC CLOCK. */
    RTC_OSC_CLK,           /**< RTC OSC CLOCK. */
    RNG_OSC_CLK2,          /**< RNG OSC CLOCK2. */
} sdk_clock_type_t;


/**@brief memory power setting mode. */
typedef enum
{
   MEM_POWER_FULL_MODE = 0,   /**< Full mode. */
   MEM_POWER_AUTO_MODE,       /**< Auto mode. */
} mem_power_t;
 /** @} */
 
/**@addtogroup PlAT_SDK_STRUCT Structures
 * @{ */ 
/**
 ****************************************************************************************
 * @brief   IO leakage protection config table.
 * The three fields are defined in bitmap format, each bit is corresponding to each IO.
 * The configuration rules are as below.
 * 1. input to GR551x with on-board PU or PD resistors.
 * 2. output from GR551x.
 * IO setting must be set properly according to board configurations to avoid leackage
 * during sleep.
 ****************************************************************************************
 */
typedef struct
{
   uint32_t gpio;       /**< GPIO PIN. */
   uint8_t  aon_gpio;   /**< AON  GPIO  PIN .*/
} io_table_t;

 /** @} */

/** @addtogroup PLAT_SDK_FUNCTIONS Functions
 * @{ */

/**
 ****************************************************************************************
 * @brief   platform sdk init function.
 * @retval :  void
 ****************************************************************************************
 */
void platform_sdk_init(void);

/**
 ****************************************************************************************
 * @brief  set the memory power state to full. This function is mainly used in some 
 *         scenarios where all SRAMs need to be powered on
 * @retval : void
 ****************************************************************************************
 */
void mem_pwr_mgmt_full_power_set(void);

/**
 ****************************************************************************************
 * @brief  Set the memory power management mode, which can be automatic mode or full power on mode.
 * @param[in] mem_pwr_mode : MEM_POWER_FULL_MODE or MEM_POWER_AUTO_MODE.
 * @retval : void
 ****************************************************************************************
 */
void mem_pwr_mgmt_mode_set(mem_power_t mem_pwr_mode);

/**
 ****************************************************************************************
 * @brief  Control the memory power supply by specifying start address and length.
 * @param[in] start_addr : the start address of memory that user want to config
 * @param[in] size       : the size of memory that user want to config
 * @retval : void
 ****************************************************************************************
 */
void mem_pwr_mgmt_mode_set_from(uint32_t start_addr, uint32_t size);

/**
 ****************************************************************************************
 * @brief  memory check process
 * @retval : void
 ****************************************************************************************
 */
void mem_pwr_mgmt_init(void);

/**
 ****************************************************************************************
 * @brief  update the counter A and counter B.
 * @param[in] cnt_a :  Start Index Number.
 * @param[in] cnt_b :  End Index Number.
 * @retval :  void
 ****************************************************************************************
 */
void system_lp_counter_set(uint8_t cnt_a, uint8_t cnt_b);

/**
 ****************************************************************************************
 * @brief  Enable patch function.
 * @param[in] table_idx :  Start Index Number.
 * @param[in] dur_offset   :  duration setting.
 * @param[in] ext_offset   :  ext wakeup setting.
 * @param[in] osc_offset   :  pre-wakeup setting.
 * @retval :  void
 ****************************************************************************************
 */
void system_lp_table_update_twval(table_idx_t table_idx, int16_t dur_offset, int16_t ext_offset, int16_t osc_offset);

/**
 ****************************************************************************************
 * @brief  Leakage Protection for User's IO.
 *         The leakage protection only acts on the outside without pull-up and pull-down,
 *         and only for input mode IO. By default, flash-related IO has been internally processed.
 *         For instance : 
 *         io_table_t io_table =
 *         {
 *             .gpio     = GPIO_PIN(0) | GPIO_PIN(1),
 *             .aon_gpio = AON_GPIO_PIN(0) | AON_GPIO_PIN(1),
 *             .msio     = MSIO_PIN(0) | MSIO_PIN(1),
 *         };
 * @param[in] p_io_table : the potiner of io setting table.
 * @retval :  void
 ****************************************************************************************
 */
void system_io_leakage_protect(io_table_t *p_io_table);

/**
 ****************************************************************************************
 * @brief  Platform low power clock init function.
 * @param[in] sys_clock:  System clock.
 * @param[in] clock     :  External RTC setting or internal RNG/RNG2 setting.
 * @param[in] accuracy  :  Low speed clock accuracy.
 * @param[in] xo_offset :  Clock calibration parameter.
 * @retval :  void
 ****************************************************************************************
 */
void platform_clock_init(mcu_clock_type_t sys_clock, sdk_clock_type_t clock, uint16_t accuracy, uint16_t xo_offset);

/**
 ****************************************************************************************
 * @brief  Start RNG2 OSC calibration.  
 * @retval :  void
 ****************************************************************************************
 */
void platform_rng2_calibration_start(void);

/**
 ****************************************************************************************
 * @brief  Stop RNG2 OSC calibration.     
 * @param[in] wait : True will wait calibration register is cleared
 * @retval :  void
 ****************************************************************************************
 */
void platform_rng2_calibration_stop(bool wait);

/**
 ****************************************************************************************
 * @brief  Check whether RNG2 OSC calibration is on going.     
 * @param[in] void
 * @retval :  bool : true calibration is on going, false calibration is not started or done
 ****************************************************************************************
 */
bool platform_rng2_calibration_is_busy(void);

/**
 ****************************************************************************************
 * @brief  Get RNG2 OSC calibration result.     
 * @param[in] wait_result : true will wait calibration done to get RNG2 OSC frequency 
 * @param[in] allow_cached : true will using previous RNG2 OSC calibration frequency 
 * @retval :  uint32_t : RNG2 OSC frequency in HZ
 ****************************************************************************************
 */
uint32_t platform_rng2_calibration_get(bool wait_result, bool allow_cached);

/**
 ****************************************************************************************
 * @brief  Platform init function.
 * @retval :  void
 ****************************************************************************************
 */
void platform_init(void);

/**
 ****************************************************************************************
 * @brief  the first warm boot stage.
 * @retval :  void
 ****************************************************************************************
 */
void warm_boot_first(void);

 /**
 ****************************************************************************************
 * @brief  the second warm boot stage..
 * @retval :  void
 ****************************************************************************************
 */
void warm_boot_second(void);

/**
 ****************************************************************************************
 * @brief  PMU init function.
 * @param[in] clock_type :  clock type to be configured.
 * @retval : void
 ****************************************************************************************
 */
void system_pmu_init(mcu_clock_type_t clock_type);

/**
 ****************************************************************************************
 * @brief  PMU deinit function.
 * @retval : void
 ****************************************************************************************
 */
void system_pmu_deinit(void);

/**
 ****************************************************************************************
 * @brief  Warm boot process.
 * @retval :  void
 ****************************************************************************************
 */
void warm_boot(void);

/**
 ****************************************************************************************
 * @brief  PMU calibration handler.     
 * @param[in] p_arg : no args.
 * @retval :  void
 ****************************************************************************************
 */
void pmu_calibration_handler(void* p_arg);

/**
 ****************************************************************************************
 * @brief  RTC calibration function.
 * @retval : void
 ****************************************************************************************
 */
void rtc_calibration(void);

/**
 ****************************************************************************************
 * @brief  protection platform timeout through watchdog.
 * @retval : hal status
 ****************************************************************************************
 */
void platform_init_push(void);

/**
 ****************************************************************************************
 * @brief  protection platform timeout through watchdog.
 * @retval : hal status
 ****************************************************************************************
 */
void platform_init_pop(void);

/**
 ****************************************************************************************
 * @brief  write flash QE
 * @retval : hal status
 ****************************************************************************************
 */
hal_status_t platform_flash_enable_quad(void);

/**
 ****************************************************************************************
 * @brief  During Flash erase/write operation, Disable external interrupts with a priority less 
 *         than or equal to base_priority in the system.
 * @param[in] base_priority: Base Priority value to set.
 * @retval : hal status
 ****************************************************************************************
 */
hal_status_t platform_flash_protection(uint32_t base_priority);

/**
 ****************************************************************************************
 * @brief  protection interrupt handler during writing/erasing flash.
 * @retval : hal status
 ****************************************************************************************
 */
hal_status_t platform_interrupt_protection_push(void);

/**
 ****************************************************************************************
 * @brief  protection interrupt handler during writing/erasing flash.
 * @retval : hal status
 ****************************************************************************************
 */
hal_status_t platform_interrupt_protection_pop(void);

/** @} */

#endif

/** @} */
/** @} */

