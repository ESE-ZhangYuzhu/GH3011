/**
  ****************************************************************************************
  * @file    app_rtc.c
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
#include "app_rtc.h"
#include <string.h>
#include "platform_sdk.h"

#ifdef HAL_CALENDAR_MODULE_ENABLED

/*
 * STRUCT DEFINE
 *****************************************************************************************
 */

/**@brief App rtc state types. */
typedef enum
{
    APP_RTC_INVALID = 0,
    APP_RTC_ACTIVITY,
} app_rtc_state_t;

struct rtc_env_t
{
    app_rtc_evt_handler_t   evt_handler;
    calendar_handle_t       handle;
    app_rtc_state_t         rtc_state;
};

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */

struct rtc_env_t  s_rtc_env;

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
static void app_rtc_event_call(calendar_handle_t *p_calendar, app_rtc_evt_type_t evt_type)
{
    app_rtc_evt_t rtc_evt;

    rtc_evt.type = evt_type;
    if(s_rtc_env.evt_handler != NULL)
    {
        s_rtc_env.evt_handler(&rtc_evt);
    }
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
uint16_t app_rtc_init(app_rtc_evt_handler_t evt_handler)
{
    hal_status_t err_code;

    s_rtc_env.evt_handler = evt_handler;

    err_code = hal_calendar_deinit(&s_rtc_env.handle);
    HAL_ERR_CODE_CHECK(err_code);
    err_code = hal_calendar_init(&s_rtc_env.handle);
    HAL_ERR_CODE_CHECK(err_code);

    s_rtc_env.rtc_state = APP_RTC_ACTIVITY;

    return APP_DRV_SUCCESS;
}

uint16_t app_rtc_deinit(void)
{
    hal_status_t err_code;

    if (s_rtc_env.rtc_state == APP_RTC_INVALID)
    {
        return APP_DRV_ERR_INVALID_ID;
    }

    err_code = hal_calendar_disable_event(&s_rtc_env.handle, CALENDAR_ALARM_DISABLE_ALL);
    HAL_ERR_CODE_CHECK(err_code);

    err_code = hal_calendar_deinit(&s_rtc_env.handle);
    HAL_ERR_CODE_CHECK(err_code);

    s_rtc_env.rtc_state = APP_RTC_INVALID;

    return APP_DRV_SUCCESS;
}

uint16_t app_rtc_init_time(app_rtc_time_t *p_time)
{
    hal_status_t err_code;

    if ((s_rtc_env.rtc_state == APP_RTC_INVALID) || (p_time == NULL))
    {
        return APP_DRV_ERR_INVALID_ID;
    }

    err_code = hal_calendar_init_time(&s_rtc_env.handle, p_time);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_rtc_get_time(app_rtc_time_t *p_time)
{
    hal_status_t err_code;

    if ((s_rtc_env.rtc_state == APP_RTC_INVALID) || (p_time == NULL))
    {
        return APP_DRV_ERR_INVALID_ID;
    }

    err_code = hal_calendar_get_time(&s_rtc_env.handle, p_time);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_rtc_setup_alarm(app_rtc_alarm_t *p_alarm)
{
    hal_status_t err_code;

    if ((s_rtc_env.rtc_state == APP_RTC_INVALID) || (p_alarm == NULL))
    {
        return APP_DRV_ERR_INVALID_ID;
    }

    err_code = hal_calendar_set_alarm(&s_rtc_env.handle, p_alarm);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_rtc_setup_tick(uint32_t interval)
{
    hal_status_t err_code;

    if ((s_rtc_env.rtc_state == APP_RTC_INVALID) || (interval < 5))
    {
        return APP_DRV_ERR_INVALID_ID;
    }

    err_code = hal_calendar_set_tick(&s_rtc_env.handle, interval);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_rtc_disable_event(uint32_t disable_mode)
{
    hal_status_t err_code;

    if (s_rtc_env.rtc_state == APP_RTC_INVALID)
    {
        return APP_DRV_ERR_INVALID_ID;
    }

    err_code = hal_calendar_disable_event(&s_rtc_env.handle, disable_mode);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

void hal_calendar_alarm_callback(calendar_handle_t *p_calendar)
{
    app_rtc_event_call(p_calendar, APP_RTC_EVT_DATE_ALARM);
}

void hal_calendar_tick_callback(calendar_handle_t *p_calendar)
{
    app_rtc_event_call(p_calendar, APP_RTC_EVT_TICK_ALARM);
}

SECTION_RAM_CODE void CALENDAR_IRQHandler(void)
{
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_push();
#endif
    calendar_irq_handler(&s_rtc_env.handle);
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_pop();
#endif
}

#endif

