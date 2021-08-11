/**
  ****************************************************************************************
  * @file    gr55xx_hal_calendar.c
  * @author  BLE Driver Team
  * @brief   CALENDAR HAL module driver.
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

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "gr55xx_hal.h"

#if defined(HAL_CALENDAR_MODULE_ENABLED) && defined(GR5515_D)

/* extern function ---------------------------------------------------------*/

extern hal_status_t hal_calendar_init_ext(calendar_handle_t *p_calendar);
extern hal_status_t hal_calendar_deinit_ext(calendar_handle_t *p_calendar);
extern void hal_calendar_register_callback(hal_calendar_callback_t *hal_calendar_callback);

/* Private variables -------------------------------------------------------*/

static hal_calendar_callback_t calendar_callback = 
{
    .calendar_alarm_callback = hal_calendar_alarm_callback,
    .calendar_tick_callback  = hal_calendar_tick_callback,
};

/* Private function prototypes ---------------------------------------------*/

hal_status_t hal_calendar_init(calendar_handle_t *p_calendar)
{
    hal_calendar_register_callback(&calendar_callback);

    return hal_calendar_init_ext(p_calendar);
}

hal_status_t hal_calendar_deinit(calendar_handle_t *p_calendar)
{
    hal_calendar_register_callback(&calendar_callback);
    
    return hal_calendar_deinit_ext(p_calendar);
}

__WEAK void hal_calendar_alarm_callback(calendar_handle_t *p_calendar)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(p_calendar);

    /* NOTE: This function should not be modified, when the callback is needed,
             the hal_calendar_alarm_callback could be implemented in the user file
    */
}

__WEAK void hal_calendar_tick_callback(calendar_handle_t *p_calendar)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(p_calendar);

    /* NOTE: This function should not be modified, when the callback is needed,
             the hal_calendar_interval_callback could be implemented in the user file
    */
}

#endif /* HAL_CALENDAR_MODULE_ENABLED */
