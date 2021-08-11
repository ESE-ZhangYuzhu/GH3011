/**
  ****************************************************************************************
  * @file    app_i2c.c
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
#include "app_i2c.h"
#include "app_io.h"
#include "app_dma.h"
#include "app_pwr_mgmt.h"
#include "app_systick.h"
#include <string.h>
#include "platform_sdk.h"

#ifdef HAL_I2C_MODULE_ENABLED

/*
 * DEFINES
 *****************************************************************************************
 */

#ifdef ENV_RTOS_USE_MUTEX

#define APP_I2C_DRV_SYNC_MUTEX_LOCK(id)     app_driver_mutex_pend(s_i2c_env[id].mutex_sync, MUTEX_WAIT_FOREVER)
#define APP_I2C_DRV_SYNC_MUTEX_UNLOCK(id)   app_driver_mutex_post(s_i2c_env[id].mutex_sync)

#define APP_I2C_DRV_ASYNC_MUTEX_LOCK(id)    app_driver_mutex_pend(s_i2c_env[id].mutex_async, MUTEX_WAIT_FOREVER)
#define APP_I2C_DRV_ASYNC_MUTEX_UNLOCK(id)  app_driver_mutex_post(s_i2c_env[id].mutex_async)

#endif

/*
 * STRUCT DEFINE
 *****************************************************************************************
 */

/**@brief App i2c state types. */
typedef enum
{
    APP_I2C_INVALID = 0,
    APP_I2C_ACTIVITY,
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    APP_I2C_SLEEP,
#endif
} app_i2c_state_t;

struct i2c_env_t
{
    app_i2c_evt_handler_t   evt_handler;
    i2c_handle_t            handle;
    app_i2c_mode_t          use_mode;
    app_i2c_role_t          role;
    app_i2c_pin_cfg_t       pin_cfg;
    dma_id_t                dma_id[2];
    app_i2c_state_t         i2c_state;
    bool                    start_flag;
#ifdef ENV_RTOS_USE_SEMP   
    APP_DRV_SEM_DECL(sem_tx);
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
static bool i2c_prepare_for_sleep(void);
static void i2c_sleep_canceled(void);
static void i2c_wake_up_ind(void);
static uint16_t i2c_gpio_config(app_i2c_pin_cfg_t pin_cfg);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static const IRQn_Type   s_i2c_irq[APP_I2C_ID_MAX]      = { I2C0_IRQn, I2C1_IRQn };
static const uint32_t    s_i2c_instance[APP_I2C_ID_MAX] = { I2C0_BASE, I2C1_BASE };

struct i2c_env_t s_i2c_env[APP_I2C_ID_MAX] = {
    {
        .evt_handler = NULL,
#ifdef ENV_RTOS_USE_SEMP   
        .sem_tx = NULL,
        .sem_rx = NULL,
#endif
#ifdef ENV_RTOS_USE_MUTEX
        .mutex_sync = NULL,
        .mutex_async = NULL,
#endif
    },
    {
        .evt_handler = NULL,
#ifdef ENV_RTOS_USE_SEMP   
        .sem_tx = NULL,
        .sem_rx = NULL,
#endif
#ifdef ENV_RTOS_USE_MUTEX
        .mutex_sync = NULL,
        .mutex_async = NULL,
#endif
    },
};
static bool      s_sleep_cb_registered_flag = false;
static pwr_id_t  s_i2c_pwr_id;

static const app_sleep_callbacks_t i2c_sleep_cb =
{
    .app_prepare_for_sleep = i2c_prepare_for_sleep,
    .app_sleep_canceled    = i2c_sleep_canceled,
    .app_wake_up_ind       = i2c_wake_up_ind
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
static bool i2c_prepare_for_sleep(void)
{
    hal_i2c_state_t state;

    for (uint8_t i = 0; i < APP_I2C_ID_MAX; i++)
    {
        if (s_i2c_env[i].i2c_state == APP_I2C_ACTIVITY)
        {
            state = hal_i2c_get_state(&s_i2c_env[i].handle);
            if ((state != HAL_I2C_STATE_READY) && (state != HAL_I2C_STATE_RESET))
            {
                return false;
            }

            GLOBAL_EXCEPTION_DISABLE();
            hal_i2c_suspend_reg(&s_i2c_env[i].handle);
            GLOBAL_EXCEPTION_ENABLE();

            #ifdef APP_DRIVER_WAKEUP_CALL_FUN
            s_i2c_env[i].i2c_state = APP_I2C_SLEEP;
            #endif
        }
    }

    return true;
}

static void i2c_sleep_canceled(void)
{
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    for (uint8_t i = 0; i < APP_I2C_ID_MAX; i++)
    {
        if (s_i2c_env[i].i2c_state == APP_I2C_SLEEP)
        {
            s_i2c_env[i].i2c_state = APP_I2C_ACTIVITY;
        }
    }
#endif
}

SECTION_RAM_CODE static void i2c_wake_up_ind(void)
{
#ifndef APP_DRIVER_WAKEUP_CALL_FUN
    for (uint8_t i = 0; i < APP_I2C_ID_MAX; i++)
    {
        if (s_i2c_env[i].i2c_state == APP_I2C_ACTIVITY)
        {
            GLOBAL_EXCEPTION_DISABLE();
            hal_i2c_resume_reg(&s_i2c_env[i].handle);
            GLOBAL_EXCEPTION_ENABLE();

            if(s_i2c_env[i].use_mode.type != APP_I2C_TYPE_POLLING)
            {
                hal_nvic_clear_pending_irq(s_i2c_irq[i]);
                hal_nvic_enable_irq(s_i2c_irq[i]);
            }
        }
    }
#endif
}

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
static void i2c_wake_up(app_i2c_id_t id)
{
    if (s_i2c_env[id].i2c_state == APP_I2C_SLEEP)
    {
        GLOBAL_EXCEPTION_DISABLE();
        hal_i2c_resume_reg(&s_i2c_env[id].handle);
        GLOBAL_EXCEPTION_ENABLE();

        if(s_i2c_env[id].use_mode.type != APP_I2C_TYPE_POLLING)
        {
            hal_nvic_clear_pending_irq(s_i2c_irq[id]);
            hal_nvic_enable_irq(s_i2c_irq[id]);
        }
        s_i2c_env[id].i2c_state = APP_I2C_ACTIVITY;
    }
}
#endif

static uint16_t i2c_gpio_config(app_i2c_pin_cfg_t pin_cfg)
{
    app_io_init_t io_init = APP_IO_DEFAULT_CONFIG;
    app_drv_err_t err_code = APP_DRV_SUCCESS;

    io_init.pull = pin_cfg.scl.pull;
    io_init.mode = APP_IO_MODE_MUX;
    io_init.pin  = pin_cfg.scl.pin;
    io_init.mux  = pin_cfg.scl.mux;
    err_code = app_io_init(pin_cfg.scl.type, &io_init);
    APP_DRV_ERR_CODE_CHECK(err_code);

    io_init.pull = pin_cfg.sda.pull;
    io_init.pin  = pin_cfg.sda.pin;
    io_init.mux  = pin_cfg.sda.mux;
    err_code = app_io_init(pin_cfg.sda.type, &io_init);
    APP_DRV_ERR_CODE_CHECK(err_code);

    return err_code;
}

static uint16_t app_i2c_config_dma(app_i2c_params_t *p_params)
{
    app_dma_params_t tx_dma_params;
    app_dma_params_t rx_dma_params;

    tx_dma_params.channel_number             = p_params->use_mode.tx_dma_channel;
    tx_dma_params.init.src_request           = DMA_REQUEST_MEM;
    tx_dma_params.init.dst_request           = (p_params->id == APP_I2C_ID_0) ? DMA_REQUEST_I2C0_TX : DMA_REQUEST_I2C1_TX;
    tx_dma_params.init.direction             = DMA_MEMORY_TO_PERIPH;
    tx_dma_params.init.src_increment         = DMA_SRC_INCREMENT;
    tx_dma_params.init.dst_increment         = DMA_DST_NO_CHANGE;
    tx_dma_params.init.src_data_alignment    = DMA_SDATAALIGN_BYTE;
    tx_dma_params.init.dst_data_alignment    = DMA_DDATAALIGN_BYTE;
    tx_dma_params.init.mode                  = DMA_NORMAL;
    tx_dma_params.init.priority              = DMA_PRIORITY_LOW;

    s_i2c_env[p_params->id].dma_id[0] = app_dma_init(&tx_dma_params, NULL);
    if (s_i2c_env[p_params->id].dma_id[0] < 0)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }
    s_i2c_env[p_params->id].handle.p_dmatx = app_dma_get_handle(s_i2c_env[p_params->id].dma_id[0]);
    s_i2c_env[p_params->id].handle.p_dmatx->p_parent = (void*)&s_i2c_env[p_params->id].handle;

    rx_dma_params.channel_number             = p_params->use_mode.rx_dma_channel;
    rx_dma_params.init.src_request           = (p_params->id == APP_I2C_ID_0) ? DMA_REQUEST_I2C0_RX : DMA_REQUEST_I2C1_RX;
    rx_dma_params.init.dst_request           = DMA_REQUEST_MEM;
    rx_dma_params.init.direction             = DMA_PERIPH_TO_MEMORY;
    rx_dma_params.init.src_increment         = DMA_SRC_NO_CHANGE;
    rx_dma_params.init.dst_increment         = DMA_DST_INCREMENT;
    rx_dma_params.init.src_data_alignment    = DMA_SDATAALIGN_BYTE;
    rx_dma_params.init.dst_data_alignment    = DMA_DDATAALIGN_BYTE;
    rx_dma_params.init.mode                  = DMA_NORMAL;
    rx_dma_params.init.priority              = DMA_PRIORITY_LOW;

    s_i2c_env[p_params->id].dma_id[1] = app_dma_init(&rx_dma_params, NULL);
    if (s_i2c_env[p_params->id].dma_id[1] < 0)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }
    s_i2c_env[p_params->id].handle.p_dmarx = app_dma_get_handle(s_i2c_env[p_params->id].dma_id[1]);
    s_i2c_env[p_params->id].handle.p_dmarx->p_parent = (void*)&s_i2c_env[p_params->id].handle;

    return APP_DRV_SUCCESS;
}

static void app_i2c_event_call(i2c_handle_t *p_i2c, app_i2c_evt_type_t evt_type)
{
    app_i2c_evt_t i2c_evt;
    app_i2c_id_t id;
    if(p_i2c->p_instance == I2C0)
    {
        id = APP_I2C_ID_0;
    }
    else if(p_i2c->p_instance == I2C1)
    {
        id = APP_I2C_ID_1;
    }
    i2c_evt.type = evt_type;
    if(evt_type == APP_I2C_EVT_ERROR)
    {
#ifdef  ENV_RTOS_USE_SEMP        
        app_driver_sem_post_from_isr(s_i2c_env[id].sem_tx);
        app_driver_sem_post_from_isr(s_i2c_env[id].sem_rx);
#endif  
        i2c_evt.data.error_code = p_i2c->error_code;
    }
    else if(evt_type == APP_I2C_EVT_TX_CPLT)
    {
#ifdef  ENV_RTOS_USE_SEMP        
        app_driver_sem_post_from_isr(s_i2c_env[id].sem_tx);
#endif  
        i2c_evt.data.size = p_i2c->xfer_size - p_i2c->xfer_count;
    }
    else if(evt_type == APP_I2C_EVT_RX_DATA)
    {
#ifdef  ENV_RTOS_USE_SEMP        
        app_driver_sem_post_from_isr(s_i2c_env[id].sem_rx);
#endif  
        i2c_evt.data.size = p_i2c->xfer_size - p_i2c->xfer_count;
    }

    s_i2c_env[id].start_flag = false;
    if (s_i2c_env[id].evt_handler != NULL)
    {
        s_i2c_env[id].evt_handler(&i2c_evt);
    }
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
uint16_t app_i2c_init(app_i2c_params_t *p_params, app_i2c_evt_handler_t evt_handler)
{
    uint8_t       id       = p_params->id;
    app_drv_err_t app_err_code;
    hal_status_t  hal_err_code;

    if (NULL == p_params)
    {
        return APP_DRV_ERR_POINTER_NULL;
    }

    if (id >= APP_I2C_ID_MAX)
    {
        return APP_DRV_ERR_INVALID_ID;
    }

#ifdef  ENV_RTOS_USE_SEMP
    if(s_i2c_env[id].sem_tx == NULL)
    {
        app_err_code = app_driver_sem_init(&s_i2c_env[id].sem_tx);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
    if(s_i2c_env[id].sem_rx == NULL)
    {
        app_err_code = app_driver_sem_init(&s_i2c_env[id].sem_rx);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
#endif

#ifdef ENV_RTOS_USE_MUTEX
    if(s_i2c_env[id].mutex_async == NULL)
    {
        app_err_code = app_driver_mutex_init(&s_i2c_env[id].mutex_async);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
    if(s_i2c_env[id].mutex_sync == NULL)
    {
        app_err_code = app_driver_mutex_init(&s_i2c_env[id].mutex_sync);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
#endif

    app_systick_init();

    app_err_code = i2c_gpio_config(p_params->pin_cfg);
    APP_DRV_ERR_CODE_CHECK(app_err_code);

    if(p_params->use_mode.type == APP_I2C_TYPE_DMA)
    {
        app_err_code = app_i2c_config_dma(p_params);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }

    if(p_params->use_mode.type != APP_I2C_TYPE_POLLING)
    {
        hal_nvic_clear_pending_irq(s_i2c_irq[id]);
        hal_nvic_enable_irq(s_i2c_irq[id]);
    }

    s_i2c_env[id].use_mode.type = p_params->use_mode.type;
    s_i2c_env[id].use_mode.rx_dma_channel = p_params->use_mode.rx_dma_channel;
    s_i2c_env[id].use_mode.tx_dma_channel = p_params->use_mode.tx_dma_channel;
    s_i2c_env[id].role = p_params->role;
    memcpy(&s_i2c_env[id].pin_cfg, &p_params->pin_cfg, sizeof(app_i2c_pin_cfg_t));
    s_i2c_env[id].evt_handler = evt_handler;

    memcpy(&s_i2c_env[id].handle.init, &p_params->init, sizeof(i2c_init_t));
    s_i2c_env[id].handle.p_instance = (i2c_regs_t *)s_i2c_instance[id];
    hal_err_code = hal_i2c_deinit(&s_i2c_env[id].handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    hal_err_code = hal_i2c_init(&s_i2c_env[id].handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    if(s_sleep_cb_registered_flag == false)// register sleep callback
    {
        s_sleep_cb_registered_flag = true;
        s_i2c_pwr_id = pwr_register_sleep_cb(&i2c_sleep_cb, APP_DRIVER_I2C_WAPEUP_PRIORITY);

        if (s_i2c_pwr_id < 0)
        {
            return APP_DRV_ERR_INVALID_PARAM;
        }
    }
    s_i2c_env[id].i2c_state = APP_I2C_ACTIVITY;

    return APP_DRV_SUCCESS;
}

uint16_t app_i2c_deinit(app_i2c_id_t id)
{
    app_drv_err_t app_err_code;
    hal_status_t  hal_err_code;

    if ((id >= APP_I2C_ID_MAX) || (s_i2c_env[id].i2c_state == APP_I2C_INVALID))
    {
        return APP_DRV_ERR_INVALID_ID;
    }

#ifdef  ENV_RTOS_USE_SEMP
    if(s_i2c_env[id].sem_tx != NULL)
    {
        app_driver_sem_deinit(s_i2c_env[id].sem_tx);
        s_i2c_env[id].sem_tx = NULL;
    }
    if(s_i2c_env[id].sem_rx != NULL)
    {
        app_driver_sem_deinit(s_i2c_env[id].sem_rx);
        s_i2c_env[id].sem_rx = NULL;
    }
#endif

#ifdef ENV_RTOS_USE_MUTEX
    if(s_i2c_env[id].mutex_sync != NULL)
    {
        app_driver_mutex_deinit(s_i2c_env[id].mutex_sync);
        s_i2c_env[id].mutex_sync = NULL;
    }
    if(s_i2c_env[id].mutex_async != NULL)
    {
        app_driver_mutex_deinit(s_i2c_env[id].mutex_async);
        s_i2c_env[id].mutex_async = NULL;
    }
#endif

    app_err_code = app_io_deinit(s_i2c_env[id].pin_cfg.scl.type, s_i2c_env[id].pin_cfg.scl.pin);
    APP_DRV_ERR_CODE_CHECK(app_err_code);

    app_err_code = app_io_deinit(s_i2c_env[id].pin_cfg.sda.type, s_i2c_env[id].pin_cfg.sda.pin);
    APP_DRV_ERR_CODE_CHECK(app_err_code);

    hal_nvic_disable_irq(s_i2c_irq[id]);
    if(s_i2c_env[id].use_mode.type == APP_I2C_TYPE_DMA)
    {
        app_dma_deinit(s_i2c_env[id].dma_id[0]);
        app_dma_deinit(s_i2c_env[id].dma_id[1]);
    }
    s_i2c_env[id].i2c_state = APP_I2C_INVALID;

    GLOBAL_EXCEPTION_DISABLE();
    if(s_i2c_env[APP_I2C_ID_0].i2c_state == APP_I2C_INVALID && 
        s_i2c_env[APP_I2C_ID_1].i2c_state == APP_I2C_INVALID)
    {
         pwr_unregister_sleep_cb(s_i2c_pwr_id);
         s_sleep_cb_registered_flag = false;
    }
    GLOBAL_EXCEPTION_ENABLE();

    app_systick_deinit();

    hal_err_code = hal_i2c_deinit(&s_i2c_env[id].handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_i2c_receive_sync(app_i2c_id_t id, uint16_t target_address, uint8_t *p_data, uint16_t size, uint32_t timeout)
{
    hal_status_t err_code = HAL_OK;

    if (id >= APP_I2C_ID_MAX ||
        p_data == NULL ||
        size == 0 ||
        s_i2c_env[id].i2c_state == APP_I2C_INVALID)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    i2c_wake_up(id);
#endif

    switch(s_i2c_env[id].role)
    {
        case APP_I2C_ROLE_MASTER:
            err_code = hal_i2c_master_receive(&s_i2c_env[id].handle, target_address, p_data, size, timeout);
            break;

        case APP_I2C_ROLE_SLAVE:
            err_code = hal_i2c_slave_receive(&s_i2c_env[id].handle, p_data, size, timeout);
            break;

        default:
            break;
    }
    if (err_code != HAL_OK)
    {
        return (uint16_t)err_code;
    }
    
    return APP_DRV_SUCCESS;
}

#ifdef  ENV_RTOS_USE_SEMP
uint16_t app_i2c_receive_sem_sync(app_i2c_id_t id, uint16_t target_address, uint8_t *p_data, uint16_t size)
{
    hal_status_t err_code = HAL_OK;

#ifdef ENV_RTOS_USE_MUTEX
    APP_I2C_DRV_ASYNC_MUTEX_LOCK(id);
#endif

    if (id >= APP_I2C_ID_MAX ||
        p_data == NULL ||
        size == 0 ||
        s_i2c_env[id].i2c_state == APP_I2C_INVALID ||
        s_i2c_env[id].use_mode.type == APP_I2C_TYPE_POLLING)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_I2C_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    i2c_wake_up(id);
#endif

    if(s_i2c_env[id].start_flag == false)
    {
        s_i2c_env[id].start_flag = true;
        switch(s_i2c_env[id].role)
        {
            case APP_I2C_ROLE_MASTER:
                switch(s_i2c_env[id].use_mode.type)
                {
                    case APP_I2C_TYPE_INTERRUPT:
                        err_code = hal_i2c_master_receive_it(&s_i2c_env[id].handle, target_address, p_data, size);
                        break;

                    case APP_I2C_TYPE_DMA:
                        err_code = hal_i2c_master_receive_dma(&s_i2c_env[id].handle, target_address, p_data, size);
                        break;

                    default:
                        break;
                }
                break;

            case APP_I2C_ROLE_SLAVE:
                switch(s_i2c_env[id].use_mode.type)
                {
                    case APP_I2C_TYPE_INTERRUPT:
                        err_code = hal_i2c_slave_receive_it(&s_i2c_env[id].handle, p_data, size);
                        break;

                    case APP_I2C_TYPE_DMA:
                        err_code = hal_i2c_slave_receive_dma(&s_i2c_env[id].handle, p_data, size);
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
            APP_I2C_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
            return (uint16_t)err_code;
        }
    }
    else
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_I2C_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
        return APP_DRV_ERR_BUSY;
    }
    
    app_driver_sem_pend(s_i2c_env[id].sem_rx, OS_WAIT_FOREVER);

#ifdef ENV_RTOS_USE_MUTEX
    APP_I2C_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif

    return APP_DRV_SUCCESS;
}
#endif

uint16_t app_i2c_receive_async(app_i2c_id_t id, uint16_t target_address, uint8_t *p_data, uint16_t size)
{
    hal_status_t err_code = HAL_OK;

    if (id >= APP_I2C_ID_MAX ||
        p_data == NULL ||
        size == 0 ||
        s_i2c_env[id].i2c_state == APP_I2C_INVALID ||
        s_i2c_env[id].use_mode.type == APP_I2C_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    i2c_wake_up(id);
#endif

    if(s_i2c_env[id].start_flag == false)
    {
        s_i2c_env[id].start_flag = true;
        switch(s_i2c_env[id].role)
        {
            case APP_I2C_ROLE_MASTER:
                switch(s_i2c_env[id].use_mode.type)
                {
                    case APP_I2C_TYPE_INTERRUPT:
                        err_code = hal_i2c_master_receive_it(&s_i2c_env[id].handle, target_address, p_data, size);
                        break;

                    case APP_I2C_TYPE_DMA:
                        err_code = hal_i2c_master_receive_dma(&s_i2c_env[id].handle, target_address, p_data, size);
                        break;

                    default:
                        break;
                }
                break;

            case APP_I2C_ROLE_SLAVE:
                switch(s_i2c_env[id].use_mode.type)
                {
                    case APP_I2C_TYPE_INTERRUPT:
                        err_code = hal_i2c_slave_receive_it(&s_i2c_env[id].handle, p_data, size);
                        break;

                    case APP_I2C_TYPE_DMA:
                        err_code = hal_i2c_slave_receive_dma(&s_i2c_env[id].handle, p_data, size);
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
            return (uint16_t)err_code;
        }
    }
    else
    {
        return APP_DRV_ERR_BUSY;
    }
    
    return APP_DRV_SUCCESS;
}

uint16_t app_i2c_transmit_sync(app_i2c_id_t id, uint16_t target_address, uint8_t *p_data, uint16_t size, uint32_t timeout)
{
    hal_status_t err_code = HAL_OK;

    if (id >= APP_I2C_ID_MAX ||
        p_data == NULL ||
        size == 0 ||
        s_i2c_env[id].i2c_state == APP_I2C_INVALID)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    i2c_wake_up(id);
#endif

    switch(s_i2c_env[id].role)
    {
        case APP_I2C_ROLE_MASTER:
            err_code = hal_i2c_master_transmit(&s_i2c_env[id].handle, target_address, p_data, size, timeout);
            break;

        case APP_I2C_ROLE_SLAVE:
            err_code = hal_i2c_slave_transmit(&s_i2c_env[id].handle, p_data, size, timeout);
            break;

        default:
            break;
    }

    if (err_code != HAL_OK)
    {
        return (uint16_t)err_code;
    }

    return APP_DRV_SUCCESS;
}

#ifdef  ENV_RTOS_USE_SEMP
uint16_t app_i2c_transmit_sem_sync(app_i2c_id_t id, uint16_t target_address, uint8_t *p_data, uint16_t size)
{
    hal_status_t err_code;

#ifdef ENV_RTOS_USE_MUTEX
    APP_I2C_DRV_ASYNC_MUTEX_LOCK(id);
#endif

    if (id >= APP_I2C_ID_MAX ||
        p_data == NULL ||
        size == 0 ||
        s_i2c_env[id].i2c_state == APP_I2C_INVALID ||
        s_i2c_env[id].use_mode.type == APP_I2C_TYPE_POLLING)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_I2C_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    i2c_wake_up(id);
#endif

    if(s_i2c_env[id].start_flag == false)
    {
        s_i2c_env[id].start_flag = true;
        switch(s_i2c_env[id].role)
        {
            case APP_I2C_ROLE_MASTER:
                switch(s_i2c_env[id].use_mode.type)
                {
                    case APP_I2C_TYPE_INTERRUPT:
                        err_code = hal_i2c_master_transmit_it(&s_i2c_env[id].handle, target_address, p_data, size);
                        break;

                    case APP_I2C_TYPE_DMA:
                        err_code = hal_i2c_master_transmit_dma(&s_i2c_env[id].handle, target_address, p_data, size);
                        break;

                    default:
                        break;
                }
                break;

            case APP_I2C_ROLE_SLAVE:
                switch(s_i2c_env[id].use_mode.type)
                {
                    case APP_I2C_TYPE_INTERRUPT:
                        err_code = hal_i2c_slave_transmit_it(&s_i2c_env[id].handle, p_data, size);
                        break;

                    case APP_I2C_TYPE_DMA:
                        err_code = hal_i2c_slave_transmit_dma(&s_i2c_env[id].handle, p_data, size);
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
            APP_I2C_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
            return (uint16_t)err_code;
        }
    }
    else
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_I2C_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
        return APP_DRV_ERR_BUSY;
    }

    app_driver_sem_pend(s_i2c_env[id].sem_tx, OS_WAIT_FOREVER);

#ifdef ENV_RTOS_USE_MUTEX
    APP_I2C_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
    return APP_DRV_SUCCESS;
}
#endif

uint16_t app_i2c_transmit_async(app_i2c_id_t id, uint16_t target_address, uint8_t *p_data, uint16_t size)
{
    hal_status_t err_code = HAL_OK;

    if (id >= APP_I2C_ID_MAX ||
        p_data == NULL ||
        size == 0 ||
        s_i2c_env[id].i2c_state == APP_I2C_INVALID ||
        s_i2c_env[id].use_mode.type == APP_I2C_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    i2c_wake_up(id);
#endif

    if(s_i2c_env[id].start_flag == false)
    {
        s_i2c_env[id].start_flag = true;
        switch(s_i2c_env[id].role)
        {
            case APP_I2C_ROLE_MASTER:
                switch(s_i2c_env[id].use_mode.type)
                {
                    case APP_I2C_TYPE_INTERRUPT:
                        err_code = hal_i2c_master_transmit_it(&s_i2c_env[id].handle, target_address, p_data, size);
                        break;

                    case APP_I2C_TYPE_DMA:
                        err_code = hal_i2c_master_transmit_dma(&s_i2c_env[id].handle, target_address, p_data, size);
                        break;

                    default:
                        break;
                }
                break;

            case APP_I2C_ROLE_SLAVE:
                switch(s_i2c_env[id].use_mode.type)
                {
                    case APP_I2C_TYPE_INTERRUPT:
                        err_code = hal_i2c_slave_transmit_it(&s_i2c_env[id].handle, p_data, size);
                        break;

                    case APP_I2C_TYPE_DMA:
                        err_code = hal_i2c_slave_transmit_dma(&s_i2c_env[id].handle, p_data, size);
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
            return (uint16_t)err_code;
        }
    }
    else
    {
        return APP_DRV_ERR_BUSY;
    }

    return APP_DRV_SUCCESS;
}


uint16_t app_i2c_mem_read_sync(app_i2c_id_t id, uint16_t dev_address, uint16_t mem_address, uint16_t mem_addr_size, uint8_t *p_data, uint16_t size, uint32_t timeout)
{
    hal_status_t err_code = HAL_OK;

    if (id >= APP_I2C_ID_MAX ||
        p_data == NULL ||
        size == 0 ||
        s_i2c_env[id].i2c_state == APP_I2C_INVALID)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    i2c_wake_up(id);
#endif

    err_code = hal_i2c_mem_read(&s_i2c_env[id].handle, dev_address, mem_address, mem_addr_size, p_data, size, timeout);
    if (err_code != HAL_OK)
    {
        return (uint16_t)err_code;
    }

    return APP_DRV_SUCCESS;
}

uint16_t app_i2c_mem_read_async(app_i2c_id_t id, uint16_t dev_address, uint16_t mem_address, uint16_t mem_addr_size, uint8_t *p_data, uint16_t size)
{
    hal_status_t err_code = HAL_OK;

    if (id >= APP_I2C_ID_MAX ||
        p_data == NULL ||
        size == 0 ||
        s_i2c_env[id].i2c_state == APP_I2C_INVALID ||
        s_i2c_env[id].use_mode.type == APP_I2C_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    i2c_wake_up(id);
#endif

    if(s_i2c_env[id].start_flag == false)
    {
        s_i2c_env[id].start_flag = true;

        switch(s_i2c_env[id].use_mode.type)
        {
            case APP_I2C_TYPE_INTERRUPT:
                err_code = hal_i2c_mem_read_it(&s_i2c_env[id].handle, dev_address, mem_address, mem_addr_size, p_data, size);
                break;

            case APP_I2C_TYPE_DMA:
                err_code = hal_i2c_mem_read_dma(&s_i2c_env[id].handle, dev_address, mem_address, mem_addr_size, p_data, size);
                break;

            default:
                break;
        }
        if (err_code != HAL_OK)
        {
            return (uint16_t)err_code;
        }
    }
    else
    {
        return APP_DRV_ERR_BUSY;
    }

    return APP_DRV_SUCCESS;
}

uint16_t app_i2c_mem_write_sync(app_i2c_id_t id, uint16_t dev_address, uint16_t mem_address, uint16_t mem_addr_size, uint8_t *p_data, uint16_t size, uint32_t timeout)
{
    hal_status_t err_code = HAL_OK;

    if (id >= APP_I2C_ID_MAX ||
        p_data == NULL ||
        size == 0 ||
        s_i2c_env[id].i2c_state == APP_I2C_INVALID)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    i2c_wake_up(id);
#endif

    err_code = hal_i2c_mem_write(&s_i2c_env[id].handle, dev_address, mem_address, mem_addr_size, p_data, size, timeout);
    if (err_code != HAL_OK)
    {
        return (uint16_t)err_code;
    }

    return APP_DRV_SUCCESS;
}

uint16_t app_i2c_mem_write_async(app_i2c_id_t id, uint16_t dev_address, uint16_t mem_address, uint16_t mem_addr_size, uint8_t *p_data, uint16_t size)
{
    hal_status_t err_code = HAL_OK;

    if (id >= APP_I2C_ID_MAX ||
        p_data == NULL ||
        size == 0 ||
        s_i2c_env[id].i2c_state == APP_I2C_INVALID ||
        s_i2c_env[id].use_mode.type == APP_I2C_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    i2c_wake_up(id);
#endif

    if(s_i2c_env[id].start_flag == false)
    {
        s_i2c_env[id].start_flag = true;

        switch(s_i2c_env[id].use_mode.type)
        {
            case APP_I2C_TYPE_INTERRUPT:
                err_code = hal_i2c_mem_write_it(&s_i2c_env[id].handle, dev_address, mem_address, mem_addr_size, p_data, size);
                break;

            case APP_I2C_TYPE_DMA:
                err_code = hal_i2c_mem_write_dma(&s_i2c_env[id].handle, dev_address, mem_address, mem_addr_size, p_data, size);
                break;

            default:
                break;
        }
        if (err_code != HAL_OK)
        {
            return (uint16_t)err_code;
        }
    }
    else
    {
        return APP_DRV_ERR_BUSY;
    }
    
    return APP_DRV_SUCCESS;
}

i2c_handle_t *app_i2c_get_handle(app_i2c_id_t id)
{
    if (id >= APP_I2C_ID_MAX ||
        s_i2c_env[id].i2c_state == APP_I2C_INVALID)
    {
        return NULL;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    i2c_wake_up(id);
#endif

    return &s_i2c_env[id].handle;
}

void hal_i2c_master_tx_cplt_callback(i2c_handle_t *p_i2c)
{
    app_i2c_event_call(p_i2c, APP_I2C_EVT_TX_CPLT);
}

void hal_i2c_master_rx_cplt_callback(i2c_handle_t *p_i2c)
{
    app_i2c_event_call(p_i2c, APP_I2C_EVT_RX_DATA);
}

void hal_i2c_slave_tx_cplt_callback(i2c_handle_t *p_i2c)
{
    app_i2c_event_call(p_i2c, APP_I2C_EVT_TX_CPLT);
}

void hal_i2c_slave_rx_cplt_callback(i2c_handle_t *p_i2c)
{
    app_i2c_event_call(p_i2c, APP_I2C_EVT_RX_DATA);
}

void hal_i2c_mem_tx_cplt_callback(i2c_handle_t *p_i2c)
{
    app_i2c_event_call(p_i2c, APP_I2C_EVT_TX_CPLT);
}

void hal_i2c_mem_rx_cplt_callback(i2c_handle_t *p_i2c)
{
    app_i2c_event_call(p_i2c, APP_I2C_EVT_RX_DATA);
}

void hal_i2c_error_callback(i2c_handle_t *p_i2c)
{
    app_i2c_event_call(p_i2c, APP_I2C_EVT_ERROR);
}

SECTION_RAM_CODE void I2C0_IRQHandler(void)
{
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_push();
#endif
    hal_i2c_irq_handler(&s_i2c_env[APP_I2C_ID_0].handle);
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_pop();
#endif
}

SECTION_RAM_CODE void I2C1_IRQHandler(void)
{
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_push();
#endif
    hal_i2c_irq_handler(&s_i2c_env[APP_I2C_ID_1].handle);
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_pop();
#endif
}

#endif
