/**
  ****************************************************************************************
  * @file    app_pwr_mgmt.c
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
#include "app_pwr_mgmt.h"
#include "gr55xx_hal.h"
#include "gr55xx_pwr.h"

/*
 * STRUCT DEFINE
 *****************************************************************************************
 */
struct pwr_env_t
{
    app_sleep_callbacks_t *pwr_sleep_cb[APP_SLEEP_CB_MAX];
    wakeup_priority_t wakeup_priority[APP_SLEEP_CB_MAX];
    bool is_pwr_callback_reg;
};

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
struct pwr_env_t s_pwr_env;

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
pwr_id_t pwr_register_sleep_cb(const app_sleep_callbacks_t *p_cb, wakeup_priority_t wakeup_priority)
{
    pwr_id_t id = -1;
    uint8_t  i  = 0;

    if (!s_pwr_env.is_pwr_callback_reg)
    {
        pwr_mgmt_dev_init(pwr_wake_up_ind);
        pwr_mgmt_set_callback(pwr_enter_sleep_check, NULL);
        s_pwr_env.is_pwr_callback_reg = true;
    }

    if (p_cb == NULL || wakeup_priority > WAPEUP_PRIORITY_HIGH || wakeup_priority < WAPEUP_PRIORITY_LOW)
    {
        return id;
    }

    while ((i < APP_SLEEP_CB_MAX) && (s_pwr_env.pwr_sleep_cb[i] != NULL)) 
    {
        i++;
    }
    if (i < APP_SLEEP_CB_MAX) 
    {
        s_pwr_env.pwr_sleep_cb[i] = (app_sleep_callbacks_t *)p_cb;
        s_pwr_env.wakeup_priority[i] = wakeup_priority;
        id = i;
    }

    return id;
}

void pwr_unregister_sleep_cb(pwr_id_t id)
{
    if(id < APP_SLEEP_CB_MAX)// Is id valid?
    {
        s_pwr_env.pwr_sleep_cb[id] = NULL;
    }
}

SECTION_RAM_CODE void pwr_wake_up_ind(void)
{
    uint8_t i;
    app_sleep_callbacks_t *p_cb;
    wakeup_priority_t priority;

    for (priority = WAPEUP_PRIORITY_HIGH; priority != 0; priority--)
    {
        for (i = 0; i < APP_SLEEP_CB_MAX; i++) 
        {
            p_cb = s_pwr_env.pwr_sleep_cb[i];
            if ((p_cb != NULL) && (p_cb ->app_wake_up_ind != NULL) && (priority == s_pwr_env.wakeup_priority[i])) 
            {
                p_cb ->app_wake_up_ind();
            }
        }
    }
}

pwr_mgmt_dev_state_t pwr_enter_sleep_check(void)
{
    int16_t i;
    pwr_mgmt_dev_state_t allow_entering_sleep = DEVICE_IDLE;
    app_sleep_callbacks_t *p_cb;

    // 1. Inquiry Adapters
    for (i = APP_SLEEP_CB_MAX - 1; i >= 0; i--) 
    {
        p_cb = s_pwr_env.pwr_sleep_cb[i];
        if (( p_cb != NULL) && (p_cb->app_prepare_for_sleep != NULL) ) 
        {
            if (!p_cb->app_prepare_for_sleep()) 
            {
                allow_entering_sleep = DEVICE_BUSY;
                break;
            }
        }
    }

    // 2. If an Adapter rejected sleep, resume any Adapters that have already accepted it.
    if ( allow_entering_sleep == DEVICE_BUSY )
    {
        i++;
        while (i < APP_SLEEP_CB_MAX) 
        {
            p_cb = s_pwr_env.pwr_sleep_cb[i];
            if ( (p_cb != NULL) && (p_cb->app_sleep_canceled != NULL) ) 
            {
                p_cb->app_sleep_canceled();
            }
            i++;
        }
    }

    return allow_entering_sleep;
}

