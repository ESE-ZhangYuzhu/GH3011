/**
  ****************************************************************************************
  * @file    app_systick.c
  * @author  BLE Driver Team
  * @brief   HAL APP module driver.
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

/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "app_systick.h"
#include "gr55xx_hal.h"
#include "app_pwr_mgmt.h"
#include <stdbool.h>

/*
 * DEFINES
 *****************************************************************************************
 */
#define SYSTICK_USE_PATTERN     0x47

/*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static bool systick_prepare_for_sleep(void);
static void systick_sleep_canceled(void);
static void systick_wake_up_ind(void);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static uint8_t s_systick_use_flag = 0;
static bool    s_sleep_cb_registered_flag = false;

static const app_sleep_callbacks_t systick_sleep_cb =
{
    .app_prepare_for_sleep = systick_prepare_for_sleep,
    .app_sleep_canceled    = systick_sleep_canceled,
    .app_wake_up_ind       = systick_wake_up_ind
};

static bool systick_prepare_for_sleep(void)
{
    return true;
}

static void systick_sleep_canceled(void)
{
}

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
SECTION_RAM_CODE static void systick_wake_up_ind(void)
{
    if (s_systick_use_flag != SYSTICK_USE_PATTERN)
    {
        return;
    }

    hal_init();
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void app_systick_init(void)
{
    s_systick_use_flag = SYSTICK_USE_PATTERN;

    if(s_sleep_cb_registered_flag == false)    // register sleep callback
    {
        if ( !(SysTick->CTRL & SysTick_CTRL_ENABLE_Msk) )
        {
             hal_init();
        }
        s_sleep_cb_registered_flag = true;
        pwr_register_sleep_cb(&systick_sleep_cb, APP_DRIVER_SYSTICK_WAPEUP_PRIORITY);
    }
}

void app_systick_deinit(void)
{

}

