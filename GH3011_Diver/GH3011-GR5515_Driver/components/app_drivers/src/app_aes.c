/**
  ****************************************************************************************
  * @file    app_aes.c
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
#include "app_aes.h"
#include "app_pwr_mgmt.h"
#include "app_systick.h"
#include "string.h"
#include "platform_sdk.h"

#ifdef HAL_AES_MODULE_ENABLED

/*
 * DEFINES
 *****************************************************************************************
 */

#ifdef ENV_RTOS_USE_MUTEX

#define APP_AES_DRV_SYNC_MUTEX_LOCK     app_driver_mutex_pend(s_aes_env.mutex_sync, MUTEX_WAIT_FOREVER)
#define APP_AES_DRV_SYNC_MUTEX_UNLOCK   app_driver_mutex_post(s_aes_env.mutex_sync)

#define APP_AES_DRV_ASYNC_MUTEX_LOCK    app_driver_mutex_pend(s_aes_env.mutex_async, MUTEX_WAIT_FOREVER)
#define APP_AES_DRV_ASYNC_MUTEX_UNLOCK  app_driver_mutex_post(s_aes_env.mutex_async)

#endif

/*
 * STRUCT DEFINE
 *****************************************************************************************
 */

/**@brief App aes state types. */
typedef enum
{
    APP_AES_INVALID = 0,
    APP_AES_ACTIVITY,
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    APP_AES_SLEEP,
#endif
} app_aes_state_t;

struct aes_env_t
{
    app_aes_evt_handler_t   evt_handler;
    aes_handle_t            handle;
    app_aes_mode_t          use_mode;
    app_aes_type_t          ues_type;
    app_aes_state_t         aes_state;
    bool                    start_flag;
#ifdef ENV_RTOS_USE_SEMP   
    APP_DRV_SEM_DECL(sem_rx);
#endif

#ifdef ENV_RTOS_USE_MUTEX
    APP_DRV_MUTEX_DECL(mutex_sync);
    APP_DRV_MUTEX_DECL(mutex_async);
#endif
};

/*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static bool aes_prepare_for_sleep(void);
static void aes_sleep_canceled(void);
static void aes_wake_up_ind(void);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
struct aes_env_t s_aes_env = {
    .evt_handler = NULL,
#ifdef ENV_RTOS_USE_SEMP   
    .sem_rx = NULL,
#endif
#ifdef ENV_RTOS_USE_MUTEX
    .mutex_sync = NULL,
    .mutex_async = NULL,
#endif
};
static bool s_sleep_cb_registered_flag = false;
static pwr_id_t s_aes_pwr_id;

const static app_sleep_callbacks_t aes_sleep_cb =
{
    .app_prepare_for_sleep = aes_prepare_for_sleep,
    .app_sleep_canceled = aes_sleep_canceled,
    .app_wake_up_ind = aes_wake_up_ind
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */

static bool aes_prepare_for_sleep(void)
{
    hal_aes_state_t state;

    if (s_aes_env.aes_state == APP_AES_ACTIVITY)
    {
        state = hal_aes_get_state(&s_aes_env.handle);
        if ((state != HAL_AES_STATE_READY) && (state != HAL_AES_STATE_RESET))
        {
            return false;
        }

        GLOBAL_EXCEPTION_DISABLE();
        hal_aes_suspend_reg(&s_aes_env.handle);
        GLOBAL_EXCEPTION_ENABLE();

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
        s_aes_env.aes_state = APP_AES_SLEEP;
#endif
    }

    return true;
}

static void aes_sleep_canceled(void)
{
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    if (s_aes_env.aes_state == APP_AES_SLEEP)
    {
        s_aes_env.aes_state = APP_AES_ACTIVITY;
    }
#endif
}

SECTION_RAM_CODE static void aes_wake_up_ind(void)
{
#ifndef APP_DRIVER_WAKEUP_CALL_FUN
    if (s_aes_env.aes_state == APP_AES_ACTIVITY)
    {
        GLOBAL_EXCEPTION_DISABLE();
        hal_aes_resume_reg(&s_aes_env.handle);
        GLOBAL_EXCEPTION_ENABLE();

        if (s_aes_env.ues_type != APP_AES_TYPE_POLLING)
        {
            hal_nvic_clear_pending_irq(AES_IRQn);
            hal_nvic_enable_irq(AES_IRQn);
        }
    }
#endif
}

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
static void aes_wake_up(void)
{
    if (s_aes_env.aes_state == APP_AES_SLEEP)
    {
        GLOBAL_EXCEPTION_DISABLE();
        hal_aes_resume_reg(&s_aes_env.handle);
        GLOBAL_EXCEPTION_ENABLE();

        if (s_aes_env.ues_type != APP_AES_TYPE_POLLING)
        {
            hal_nvic_clear_pending_irq(AES_IRQn);
            hal_nvic_enable_irq(AES_IRQn);
        }
        s_aes_env.aes_state = APP_AES_ACTIVITY;
    }
}
#endif

static void app_aes_event_call(aes_handle_t *p_aes, app_aes_evt_type_t evt_type)
{
    app_aes_evt_t aes_evt = { APP_AES_EVT_ERROR, 0};

    aes_evt.type = evt_type;
    s_aes_env.start_flag = false;
    if(evt_type == APP_AES_EVT_ERROR)
    {
        aes_evt.error_code = p_aes->error_code;
    }

#ifdef  ENV_RTOS_USE_SEMP
    app_driver_sem_post_from_isr(s_aes_env.sem_rx);
#endif

    if (s_aes_env.evt_handler != NULL)
    {
        s_aes_env.evt_handler(&aes_evt);
    }
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
uint16_t app_aes_init(app_aes_params_t *p_params, app_aes_evt_handler_t evt_handler)
{
    app_drv_err_t app_err_code = APP_DRV_SUCCESS;
    hal_status_t  hal_err_code = HAL_OK;

    if (p_params == NULL)
    {
        return APP_DRV_ERR_POINTER_NULL;
    }

    app_systick_init();
    
#ifdef  ENV_RTOS_USE_SEMP
    if(s_aes_env.sem_rx == NULL)
    {
        app_err_code = app_driver_sem_init(&s_aes_env.sem_rx);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
#endif

#ifdef ENV_RTOS_USE_MUTEX
    if(s_aes_env.mutex_sync == NULL)
    {
        app_err_code = app_driver_mutex_init(&s_aes_env.mutex_sync);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
    if(s_aes_env.mutex_async == NULL)
    {
        app_err_code = app_driver_mutex_init(&s_aes_env.mutex_async);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
#endif

    if (p_params->use_type != APP_AES_TYPE_POLLING)
    {
        hal_nvic_clear_pending_irq(AES_IRQn);
        hal_nvic_enable_irq(AES_IRQn);
    }

    s_aes_env.ues_type = p_params->use_type;
    s_aes_env.use_mode = p_params->use_mode;
    s_aes_env.evt_handler = evt_handler;

    memcpy(&s_aes_env.handle.init, &p_params->init, sizeof(aes_init_t));
    s_aes_env.handle.p_instance = AES;
    hal_err_code = hal_aes_deinit(&s_aes_env.handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    hal_err_code = hal_aes_init(&s_aes_env.handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    if(s_sleep_cb_registered_flag == false)    // register sleep callback
    {
        s_sleep_cb_registered_flag = true;
        s_aes_pwr_id = pwr_register_sleep_cb(&aes_sleep_cb, APP_DRIVER_AES_WAPEUP_PRIORITY);
        if (s_aes_pwr_id < 0)
        {
            return APP_DRV_ERR_INVALID_PARAM;
        }
    }

    s_aes_env.aes_state = APP_AES_ACTIVITY;

    return app_err_code;
}

uint16_t app_aes_deinit(void)
{
    hal_status_t  hal_err_code;

    if (s_aes_env.aes_state == APP_AES_INVALID)
    {
        return APP_DRV_ERR_INVALID_ID;
    }

#ifdef  ENV_RTOS_USE_SEMP
    if(s_aes_env.sem_rx != NULL)
    {
        app_driver_sem_deinit(s_aes_env.sem_rx);
        s_aes_env.sem_rx = NULL;
    }
#endif

#ifdef ENV_RTOS_USE_MUTEX
    if(s_aes_env.mutex_sync != NULL)
    {
        app_driver_mutex_deinit(s_aes_env.mutex_sync);
        s_aes_env.mutex_sync = NULL;
    }
    if(s_aes_env.mutex_async != NULL)
    {
        app_driver_mutex_deinit(s_aes_env.mutex_async);
        s_aes_env.mutex_async = NULL;
    }
#endif

    hal_nvic_disable_irq(AES_IRQn);
    s_aes_env.aes_state = APP_AES_INVALID;

    GLOBAL_EXCEPTION_DISABLE();
    pwr_unregister_sleep_cb(s_aes_pwr_id);
    s_sleep_cb_registered_flag = false;
    GLOBAL_EXCEPTION_ENABLE();

    app_systick_deinit();

    hal_err_code =  hal_aes_deinit(&s_aes_env.handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_aes_encrypt_sync(uint32_t *p_plain_data, uint32_t number, uint32_t *p_cypher_data, uint32_t timeout)
{
    hal_status_t err_code = HAL_OK;

    if (s_aes_env.aes_state == APP_AES_INVALID ||
        p_plain_data == NULL ||
        number == 0 ||
        p_cypher_data == NULL)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    aes_wake_up();
#endif

    switch(s_aes_env.use_mode)
    {
        case APP_AES_MODE_ECB:
            err_code = hal_aes_ecb_encrypt(&s_aes_env.handle, p_plain_data, number, p_cypher_data, timeout);
            HAL_ERR_CODE_CHECK(err_code);
            break;

        case APP_AES_MODE_CBC:
            err_code = hal_aes_cbc_encrypt(&s_aes_env.handle, p_plain_data, number, p_cypher_data, timeout);
            HAL_ERR_CODE_CHECK(err_code);
            break;
        
        default:
            break;
    }

    return APP_DRV_SUCCESS;
}

uint16_t app_aes_decrypt_sync(uint32_t *p_cypher_data, uint32_t number, uint32_t *p_plain_data, uint32_t timeout)
{
    hal_status_t err_code = HAL_OK;

    if (s_aes_env.aes_state == APP_AES_INVALID ||
        p_plain_data == NULL ||
        number == 0 ||
        p_cypher_data == NULL)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    aes_wake_up();
#endif

    switch(s_aes_env.use_mode)
    {
        case APP_AES_MODE_ECB:
            err_code = hal_aes_ecb_decrypt(&s_aes_env.handle, p_cypher_data, number, p_plain_data, timeout);
            HAL_ERR_CODE_CHECK(err_code);
            break;

        case APP_AES_MODE_CBC:
            err_code = hal_aes_cbc_decrypt(&s_aes_env.handle, p_cypher_data, number, p_plain_data, timeout);
            HAL_ERR_CODE_CHECK(err_code);
            break;
        
        default:
            break;
    }

    return APP_DRV_SUCCESS;
}

#ifdef  ENV_RTOS_USE_SEMP
uint16_t app_aes_encrypt_sem_sync(uint32_t *p_plain_data, uint32_t number, uint32_t *p_cypher_data)
{
    hal_status_t err_code = HAL_OK;

#ifdef ENV_RTOS_USE_MUTEX
    APP_AES_DRV_ASYNC_MUTEX_LOCK;
#endif

    if (s_aes_env.aes_state == APP_AES_INVALID ||
        p_plain_data == NULL ||
        number == 0 ||
        p_cypher_data == NULL ||
        s_aes_env.ues_type == APP_AES_TYPE_POLLING)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_AES_DRV_ASYNC_MUTEX_UNLOCK;
#endif
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    aes_wake_up();
#endif

    if(s_aes_env.start_flag == true)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_AES_DRV_ASYNC_MUTEX_UNLOCK;
#endif
        return APP_DRV_ERR_BUSY;
    }

    s_aes_env.start_flag = true;
    switch(s_aes_env.use_mode)
    {
        case APP_AES_MODE_ECB:
            switch(s_aes_env.ues_type)
            {
                case APP_AES_TYPE_INTERRUPT:
                    err_code = hal_aes_ecb_encrypt_it(&s_aes_env.handle, p_plain_data, number, p_cypher_data);
                    break;
                case APP_AES_TYPE_DMA:
                    err_code = hal_aes_ecb_encrypt_dma(&s_aes_env.handle, p_plain_data, number, p_cypher_data);
                    break;
                default:
                    break;
            }
            break;

        case APP_AES_MODE_CBC:
            switch(s_aes_env.ues_type)
            {
                case APP_AES_TYPE_INTERRUPT:
                    err_code = hal_aes_cbc_encrypt_it(&s_aes_env.handle, p_plain_data, number, p_cypher_data);
                    break;
                case APP_AES_TYPE_DMA:
                    err_code = hal_aes_cbc_encrypt_dma(&s_aes_env.handle, p_plain_data, number, p_cypher_data);
                    break;
                default:
                    break;
            }
            break;
        
        default:
            break;
    }
    if (err_code != HAL_OK)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_AES_DRV_ASYNC_MUTEX_UNLOCK;
#endif
        return (uint16_t)err_code;
    }

    app_driver_sem_pend(s_aes_env.sem_rx, OS_WAIT_FOREVER);

#ifdef ENV_RTOS_USE_MUTEX
    APP_AES_DRV_ASYNC_MUTEX_UNLOCK;
#endif

    return APP_DRV_SUCCESS;
}

uint16_t app_aes_decrypt_sem_sync(uint32_t *p_cypher_data, uint32_t number, uint32_t *p_plain_data)
{
    hal_status_t err_code = HAL_OK;

#ifdef ENV_RTOS_USE_MUTEX
    APP_AES_DRV_ASYNC_MUTEX_LOCK;
#endif

    if (s_aes_env.aes_state == APP_AES_INVALID ||
        p_plain_data == NULL ||
        number == 0 ||
        p_cypher_data == NULL ||
        s_aes_env.ues_type == APP_AES_TYPE_POLLING)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_AES_DRV_ASYNC_MUTEX_UNLOCK;
#endif
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    aes_wake_up();
#endif

    if(s_aes_env.start_flag == true)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_AES_DRV_ASYNC_MUTEX_UNLOCK;
#endif
        return APP_DRV_ERR_BUSY;
    }

    s_aes_env.start_flag = true;
    switch(s_aes_env.use_mode)
    {
        case APP_AES_MODE_ECB:
            switch(s_aes_env.ues_type)
            {
                case APP_AES_TYPE_INTERRUPT:
                    err_code = hal_aes_ecb_decrypt_it(&s_aes_env.handle, p_cypher_data, number, p_plain_data);
                    break;
                case APP_AES_TYPE_DMA:
                    err_code = hal_aes_ecb_decrypt_dma(&s_aes_env.handle, p_cypher_data, number, p_plain_data);
                    break;
                default:
                    break;
            }
            break;

        case APP_AES_MODE_CBC:
            switch(s_aes_env.ues_type)
            {
                case APP_AES_TYPE_INTERRUPT:
                    err_code = hal_aes_cbc_decrypt_it(&s_aes_env.handle, p_cypher_data, number, p_plain_data);
                    break;
                case APP_AES_TYPE_DMA:
                    err_code = hal_aes_cbc_decrypt_dma(&s_aes_env.handle, p_cypher_data, number, p_plain_data);
                    break;
                default:
                    break;
            }
            break;
        
        default:
            break;
    }

    if (err_code != HAL_OK)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_AES_DRV_ASYNC_MUTEX_UNLOCK;
#endif
        return (uint16_t)err_code;
    }

    app_driver_sem_pend(s_aes_env.sem_rx, OS_WAIT_FOREVER);

#ifdef ENV_RTOS_USE_MUTEX
    APP_AES_DRV_ASYNC_MUTEX_UNLOCK;
#endif

    return APP_DRV_SUCCESS;
}
#endif

uint16_t app_aes_encrypt_async(uint32_t *p_plain_data, uint32_t number, uint32_t *p_cypher_data)
{
    hal_status_t err_code = HAL_OK;

    if (s_aes_env.aes_state == APP_AES_INVALID ||
        p_plain_data == NULL ||
        number == 0 ||
        p_cypher_data == NULL ||
        s_aes_env.ues_type == APP_AES_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    aes_wake_up();
#endif

    if(s_aes_env.start_flag == true)
    {
        return APP_DRV_ERR_BUSY;
    }

    s_aes_env.start_flag = true;
    switch(s_aes_env.use_mode)
    {
        case APP_AES_MODE_ECB:
            switch(s_aes_env.ues_type)
            {
                case APP_AES_TYPE_INTERRUPT:
                    err_code = hal_aes_ecb_encrypt_it(&s_aes_env.handle, p_plain_data, number, p_cypher_data);
                    HAL_ERR_CODE_CHECK(err_code);
                    break;
                case APP_AES_TYPE_DMA:
                    err_code = hal_aes_ecb_encrypt_dma(&s_aes_env.handle, p_plain_data, number, p_cypher_data);
                    HAL_ERR_CODE_CHECK(err_code);
                    break;
                default:
                    break;
            }
            break;

        case APP_AES_MODE_CBC:
            switch(s_aes_env.ues_type)
            {
                case APP_AES_TYPE_INTERRUPT:
                    err_code = hal_aes_cbc_encrypt_it(&s_aes_env.handle, p_plain_data, number, p_cypher_data);
                    HAL_ERR_CODE_CHECK(err_code);
                    break;
                case APP_AES_TYPE_DMA:
                    err_code = hal_aes_cbc_encrypt_dma(&s_aes_env.handle, p_plain_data, number, p_cypher_data);
                    HAL_ERR_CODE_CHECK(err_code);
                    break;
                default:
                    break;
            }
            break;
        
        default:
            break;
    }

    return APP_DRV_SUCCESS;
}

uint16_t app_aes_decrypt_async(uint32_t *p_cypher_data, uint32_t number, uint32_t *p_plain_data)
{
    hal_status_t err_code = HAL_OK;

    if (s_aes_env.aes_state == APP_AES_INVALID ||
        p_plain_data == NULL ||
        number == 0 ||
        p_cypher_data == NULL ||
        s_aes_env.ues_type == APP_AES_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    aes_wake_up();
#endif

    if(s_aes_env.start_flag == true)
    {
        return APP_DRV_ERR_BUSY;
    }

    s_aes_env.start_flag = true;
    switch(s_aes_env.use_mode)
    {
        case APP_AES_MODE_ECB:
            switch(s_aes_env.ues_type)
            {
                case APP_AES_TYPE_INTERRUPT:
                    err_code = hal_aes_ecb_decrypt_it(&s_aes_env.handle, p_cypher_data, number, p_plain_data);
                    HAL_ERR_CODE_CHECK(err_code);
                    break;
                case APP_AES_TYPE_DMA:
                    err_code = hal_aes_ecb_decrypt_dma(&s_aes_env.handle, p_cypher_data, number, p_plain_data);
                    HAL_ERR_CODE_CHECK(err_code);
                    break;
                default:
                    break;
            }
            break;

        case APP_AES_MODE_CBC:
            switch(s_aes_env.ues_type)
            {
                case APP_AES_TYPE_INTERRUPT:
                    err_code = hal_aes_cbc_decrypt_it(&s_aes_env.handle, p_cypher_data, number, p_plain_data);
                    HAL_ERR_CODE_CHECK(err_code);
                    break;
                case APP_AES_TYPE_DMA:
                    err_code = hal_aes_cbc_decrypt_dma(&s_aes_env.handle, p_cypher_data, number, p_plain_data);
                    HAL_ERR_CODE_CHECK(err_code);
                    break;
                default:
                    break;
            }
            break;
        
        default:
            break;
    }

    return APP_DRV_SUCCESS;
}

aes_handle_t *app_aes_get_handle(void)
{
    if (s_aes_env.aes_state == APP_AES_INVALID)
    {
        return NULL;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    aes_wake_up();
#endif

    return &s_aes_env.handle;
}

void hal_aes_done_callback(aes_handle_t *p_aes)
{
    app_aes_event_call(p_aes, APP_AES_EVT_DONE);
}

void hal_aes_error_callback(aes_handle_t *p_aes)
{
    app_aes_event_call(p_aes, APP_AES_EVT_ERROR);
}

SECTION_RAM_CODE void AES_IRQHandler(void)
{
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_push();
#endif
    hal_aes_irq_handler(&s_aes_env.handle);
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_pop();
#endif
}

#endif
