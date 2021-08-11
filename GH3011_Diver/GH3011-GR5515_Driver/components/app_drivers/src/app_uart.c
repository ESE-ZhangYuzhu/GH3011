/**
  ****************************************************************************************
  * @file    app_uart.c
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
#include "app_uart.h"
#include "app_dma.h"
#include "app_pwr_mgmt.h"
#include "app_systick.h"
#include <string.h>
#include "platform_sdk.h"


/*
 * DEFINES
 *****************************************************************************************
 */
#define TX_ONCE_MAX_SIZE     128

#ifdef ENV_RTOS_USE_MUTEX

#define APP_UART_DRV_SYNC_MUTEX_LOCK(id)    app_driver_mutex_pend(s_uart_env[id].mutex_sync, MUTEX_WAIT_FOREVER)
#define APP_UART_DRV_SYNC_MUTEX_UNLOCK(id)  app_driver_mutex_post(s_uart_env[id].mutex_sync)

#define APP_UART_DRV_ASYNC_MUTEX_LOCK(id)   app_driver_mutex_pend(s_uart_env[id].mutex_async, MUTEX_WAIT_FOREVER)
#define APP_UART_DRV_ASYNC_MUTEX_UNLOCK(id) app_driver_mutex_post(s_uart_env[id].mutex_async)

#endif

/*
 * STRUCT DEFINE
 *****************************************************************************************
 */

/**@brief App uart state types. */
typedef enum
{
    APP_UART_INVALID = 0,
    APP_UART_ACTIVITY,
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    APP_UART_SLEEP,
#endif
} app_uart_state_t;

struct uart_env_t
{
    app_uart_evt_handler_t evt_handler;
    uart_handle_t          handle;
    app_uart_mode_t        use_mode;
    app_uart_pin_cfg_t     pin_cfg;
    dma_id_t               dma_id[2];
    app_uart_state_t       uart_state;
    ring_buffer_t          tx_ring_buffer;
    bool                   start_tx_flag;
    bool                   start_flush_flag;
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
static bool uart_prepare_for_sleep(void);
static void uart_sleep_canceled(void);
static void uart_wake_up_ind(void);
static uint16_t uart_gpio_config(uint32_t hw_flow_ctrl, app_uart_pin_cfg_t pin_cfg);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static const IRQn_Type s_uart_irq[APP_UART_ID_MAX]      = {UART0_IRQn, UART1_IRQn};
static const uint32_t  s_uart_instance[APP_UART_ID_MAX] = {UART0_BASE, UART1_BASE};

struct uart_env_t s_uart_env[APP_UART_ID_MAX] = {
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
    }
};
static bool       s_sleep_cb_registered_flag = false;
static pwr_id_t   s_uart_pwr_id;
static uint8_t    s_tx_send_buf[TX_ONCE_MAX_SIZE];

static const app_sleep_callbacks_t uart_sleep_cb =
{
    .app_prepare_for_sleep = uart_prepare_for_sleep,
    .app_sleep_canceled    = uart_sleep_canceled,
    .app_wake_up_ind       = uart_wake_up_ind,
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */

static bool uart_prepare_for_sleep(void)
{
    hal_uart_state_t state;

    for (uint8_t i = 0; i < APP_UART_ID_MAX; i++)
    {
        if (s_uart_env[i].uart_state == APP_UART_ACTIVITY)
        {
            state = hal_uart_get_state(&s_uart_env[i].handle);
            if ((state != HAL_UART_STATE_RESET) && (state != HAL_UART_STATE_READY))
            {
                return false;
            }

            GLOBAL_EXCEPTION_DISABLE();
            hal_uart_suspend_reg(&s_uart_env[i].handle);
            GLOBAL_EXCEPTION_ENABLE();
            #ifdef APP_DRIVER_WAKEUP_CALL_FUN
            s_uart_env[i].uart_state = APP_UART_SLEEP;
            #endif
        }
    }
    return true;
}


static void uart_sleep_canceled(void)
{
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    for (uint8_t i = 0; i < APP_UART_ID_MAX; i++)
    {
        if (s_uart_env[i].uart_state == APP_UART_SLEEP)
        {
            s_uart_env[i].uart_state = APP_UART_ACTIVITY;
        }
    }
#endif
}

SECTION_RAM_CODE static void uart_wake_up_ind(void)
{
#ifndef APP_DRIVER_WAKEUP_CALL_FUN
    uint8_t i;

    for (i = 0; i < APP_UART_ID_MAX; i++)
    {
        if (s_uart_env[i].uart_state == APP_UART_ACTIVITY)
        {
            GLOBAL_EXCEPTION_DISABLE();
            hal_uart_resume_reg(&s_uart_env[i].handle);
            GLOBAL_EXCEPTION_ENABLE();

            if (s_uart_env[i].use_mode.type == APP_UART_TYPE_INTERRUPT ||
                s_uart_env[i].use_mode.type == APP_UART_TYPE_DMA)
            {
                hal_nvic_clear_pending_irq(s_uart_irq[i]);
                hal_nvic_enable_irq(s_uart_irq[i]);
            }
        }
    }
#endif
}

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
static void uart_wake_up(app_uart_id_t id)
{
    if (s_uart_env[id].uart_state == APP_UART_SLEEP)
    {
        GLOBAL_EXCEPTION_DISABLE();
        hal_uart_resume_reg(&s_uart_env[id].handle);
        GLOBAL_EXCEPTION_ENABLE();
    
        if (s_uart_env[id].use_mode.type == APP_UART_TYPE_INTERRUPT ||
            s_uart_env[id].use_mode.type == APP_UART_TYPE_DMA)
        {
            hal_nvic_clear_pending_irq(s_uart_irq[id]);
            hal_nvic_enable_irq(s_uart_irq[id]);
        }
        s_uart_env[id].uart_state = APP_UART_ACTIVITY;
    }
}
#endif

static uint16_t uart_gpio_config(uint32_t hw_flow_ctrl, app_uart_pin_cfg_t pin_cfg)
{
    app_io_init_t io_init  = APP_IO_DEFAULT_CONFIG;
    app_drv_err_t err_code = APP_DRV_SUCCESS;

    io_init.pull = pin_cfg.tx.pull;
    io_init.mode = APP_IO_MODE_MUX;
    io_init.pin  = pin_cfg.tx.pin;
    io_init.mux  = pin_cfg.tx.mux;
    err_code = app_io_init(pin_cfg.tx.type, &io_init);
    APP_DRV_ERR_CODE_CHECK(err_code);

    io_init.pull = pin_cfg.rx.pull;
    io_init.pin  = pin_cfg.rx.pin;
    io_init.mux  = pin_cfg.rx.mux;
    err_code = app_io_init(pin_cfg.rx.type, &io_init);
    APP_DRV_ERR_CODE_CHECK(err_code);

    if (UART_HWCONTROL_RTS_CTS == hw_flow_ctrl)
    {
        io_init.pull = pin_cfg.cts.pull;
        io_init.pin  = pin_cfg.cts.pin;
        io_init.mux  = pin_cfg.cts.mux;
        err_code = app_io_init(pin_cfg.cts.type, &io_init);
        APP_DRV_ERR_CODE_CHECK(err_code);

        io_init.pull = pin_cfg.rts.pull;
        io_init.pin  = pin_cfg.rts.pin;
        io_init.mux  = pin_cfg.rts.mux;
        err_code = app_io_init(pin_cfg.rts.type, &io_init);
        APP_DRV_ERR_CODE_CHECK(err_code);
    }

    return err_code;
}

static uint16_t app_uart_config_dma(app_uart_params_t *p_params)
{
    app_dma_params_t tx_dma_params;
    app_dma_params_t rx_dma_params;

    tx_dma_params.channel_number           = p_params->use_mode.tx_dma_channel;
    tx_dma_params.init.src_request         = DMA_REQUEST_MEM;
    tx_dma_params.init.dst_request         = DMA_REQUEST_UART0_TX;
    tx_dma_params.init.direction           = DMA_MEMORY_TO_PERIPH;
    tx_dma_params.init.src_increment       = DMA_SRC_INCREMENT;
    tx_dma_params.init.dst_increment       = DMA_DST_NO_CHANGE;
    tx_dma_params.init.src_data_alignment  = DMA_SDATAALIGN_BYTE;
    tx_dma_params.init.dst_data_alignment  = DMA_DDATAALIGN_BYTE;
    tx_dma_params.init.mode                = DMA_NORMAL;
    tx_dma_params.init.priority            = DMA_PRIORITY_LOW;
    s_uart_env[p_params->id].dma_id[0]     = app_dma_init(&tx_dma_params, NULL);

    if (s_uart_env[p_params->id].dma_id[0] < 0)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

    /* Associate the initialized DMA handle to the UART handle */
    s_uart_env[p_params->id].handle.p_dmatx = app_dma_get_handle(s_uart_env[p_params->id].dma_id[0]);
    s_uart_env[p_params->id].handle.p_dmatx->p_parent = (void*)&s_uart_env[p_params->id].handle;

    rx_dma_params.channel_number           = p_params->use_mode.rx_dma_channel;
    rx_dma_params.init.src_request         = DMA_REQUEST_UART0_RX;
    rx_dma_params.init.dst_request         = DMA_REQUEST_MEM;
    rx_dma_params.init.direction           = DMA_PERIPH_TO_MEMORY;
    rx_dma_params.init.src_increment       = DMA_SRC_NO_CHANGE;
    rx_dma_params.init.dst_increment       = DMA_DST_INCREMENT;
    rx_dma_params.init.src_data_alignment  = DMA_SDATAALIGN_BYTE;
    rx_dma_params.init.dst_data_alignment  = DMA_DDATAALIGN_BYTE;
    rx_dma_params.init.mode                = DMA_NORMAL;
    rx_dma_params.init.priority            = DMA_PRIORITY_HIGH;
    s_uart_env[p_params->id].dma_id[1] = app_dma_init(&rx_dma_params, NULL);

    if (s_uart_env[p_params->id].dma_id[1] < 0)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

    /* Associate the initialized DMA handle to the the UART handle */
    s_uart_env[p_params->id].handle.p_dmarx = app_dma_get_handle(s_uart_env[p_params->id].dma_id[1]);
    s_uart_env[p_params->id].handle.p_dmarx->p_parent = (void*)&s_uart_env[p_params->id].handle;

    return APP_DRV_SUCCESS;
}

static uint16_t app_uart_start_transmit_async(app_uart_id_t id)
{
    uint16_t items_count = ring_buffer_items_count_get(&s_uart_env[id].tx_ring_buffer);
    uint16_t send_size   = items_count;
    hal_status_t err_code;

    if (items_count == 0)
    {
        s_uart_env[id].start_tx_flag = false;
        return APP_DRV_SUCCESS;
    }

    if (items_count >= TX_ONCE_MAX_SIZE)
    {
        ring_buffer_read(&s_uart_env[id].tx_ring_buffer, s_tx_send_buf, TX_ONCE_MAX_SIZE);
        send_size = TX_ONCE_MAX_SIZE;
    }
    else 
    {
        ring_buffer_read(&s_uart_env[id].tx_ring_buffer, s_tx_send_buf, items_count);
    }

    switch (s_uart_env[id].use_mode.type)
    {
        case APP_UART_TYPE_INTERRUPT:
            err_code = hal_uart_transmit_it(&s_uart_env[id].handle, s_tx_send_buf, send_size);
            HAL_ERR_CODE_CHECK(err_code);
            break;

        case APP_UART_TYPE_DMA:
            err_code = hal_uart_transmit_dma(&s_uart_env[id].handle, s_tx_send_buf, send_size);
            HAL_ERR_CODE_CHECK(err_code);
            break;

        default:
            break;
    }

    return APP_DRV_SUCCESS;
}


static void app_uart_event_call(uart_handle_t *p_uart, app_uart_evt_type_t evt_type)
{
    app_uart_evt_t uart_evt;
    app_uart_id_t id = APP_UART_ID_MAX;

    if (p_uart->p_instance == UART0)
    {
        id = APP_UART_ID_0;
    }
    else if (p_uart->p_instance == UART1)
    {
        id = APP_UART_ID_1;
    }

    uart_evt.type = evt_type;
    if (evt_type == APP_UART_EVT_ERROR)
    {
        uart_evt.data.error_code = p_uart->error_code;
#ifdef  ENV_RTOS_USE_SEMP         
        app_driver_sem_post_from_isr(s_uart_env[id].sem_tx);
        app_driver_sem_post_from_isr(s_uart_env[id].sem_rx);
#endif        
        if (s_uart_env[id].evt_handler != NULL)
        {
            s_uart_env[id].evt_handler(&uart_evt);
        }
    }
    else if (evt_type == APP_UART_EVT_TX_CPLT)
    {
#ifdef  ENV_RTOS_USE_SEMP        
        app_driver_sem_post_from_isr(s_uart_env[id].sem_tx);
#endif        
        uart_evt.data.size = p_uart->tx_xfer_size - p_uart->tx_xfer_count;
        app_uart_start_transmit_async(id);
        if(s_uart_env[id].start_tx_flag == false && s_uart_env[id].evt_handler != NULL)
        {
            s_uart_env[id].evt_handler(&uart_evt);
        }
    }
    else if (evt_type == APP_UART_EVT_RX_DATA)
    {
#ifdef  ENV_RTOS_USE_SEMP        
        app_driver_sem_post_from_isr(s_uart_env[id].sem_rx);
#endif 
        uart_evt.data.size = p_uart->rx_xfer_size - p_uart->rx_xfer_count;
        if (s_uart_env[id].evt_handler != NULL)
        {
            s_uart_env[id].evt_handler(&uart_evt);
        }
    }
    else if (evt_type == APP_UART_EVT_ABORT_TX)
    {
        s_uart_env[id].start_tx_flag = false;
#ifdef  ENV_RTOS_USE_SEMP        
        app_driver_sem_post_from_isr(s_uart_env[id].sem_tx);
#endif  
    }
    else if (evt_type == APP_UART_EVT_ABORT_RX)
    {
#ifdef  ENV_RTOS_USE_SEMP        
        app_driver_sem_post_from_isr(s_uart_env[id].sem_rx);
#endif 
    }
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
uint16_t app_uart_init(app_uart_params_t *p_params, app_uart_evt_handler_t evt_handler, app_uart_tx_buf_t *tx_buffer)
{
    uint8_t       id       = p_params->id;
    app_drv_err_t err_code = APP_DRV_SUCCESS;

    if (NULL == p_params)
    {
        return APP_DRV_ERR_POINTER_NULL;
    }

    if (id >= APP_UART_ID_MAX)
    {
        return APP_DRV_ERR_INVALID_ID;
    }

    if (APP_UART_TYPE_POLLING != p_params->use_mode.type && NULL == tx_buffer)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }
    
#ifdef  ENV_RTOS_USE_SEMP
    if(s_uart_env[id].sem_tx == NULL)
    {
        err_code = app_driver_sem_init(&s_uart_env[id].sem_tx);
        APP_DRV_ERR_CODE_CHECK(err_code);
    }
    if(s_uart_env[id].sem_rx == NULL)
    {
        err_code = app_driver_sem_init(&s_uart_env[id].sem_rx);
        APP_DRV_ERR_CODE_CHECK(err_code);
    }
#endif

#ifdef ENV_RTOS_USE_MUTEX
    if(s_uart_env[id].mutex_async == NULL)
    {
        err_code = app_driver_mutex_init(&s_uart_env[id].mutex_async);
        APP_DRV_ERR_CODE_CHECK(err_code);
    }
    if(s_uart_env[id].mutex_sync == NULL)
    {
        err_code = app_driver_mutex_init(&s_uart_env[id].mutex_sync);
        APP_DRV_ERR_CODE_CHECK(err_code);
    }
#endif

    app_systick_init();

    err_code = uart_gpio_config(p_params->init.hw_flow_ctrl, p_params->pin_cfg);
    APP_DRV_ERR_CODE_CHECK(err_code);

    if (APP_UART_TYPE_DMA == p_params->use_mode.type)
    {
        if (id != APP_UART_ID_0)
        {
            return APP_DRV_ERR_INVALID_ID;
        }
        else
        {
            err_code = app_uart_config_dma(p_params);
            APP_DRV_ERR_CODE_CHECK(err_code);
        }
    }

    if (p_params->use_mode.type != APP_UART_TYPE_POLLING)
    {
        ring_buffer_init(&s_uart_env[id].tx_ring_buffer, tx_buffer->tx_buf, tx_buffer->tx_buf_size);
        hal_nvic_clear_pending_irq(s_uart_irq[id]);
        hal_nvic_enable_irq(s_uart_irq[id]);
    }

    s_uart_env[id].use_mode.type = p_params->use_mode.type;
    s_uart_env[id].use_mode.rx_dma_channel = p_params->use_mode.rx_dma_channel;
    s_uart_env[id].use_mode.tx_dma_channel = p_params->use_mode.tx_dma_channel;
    memcpy(&s_uart_env[id].pin_cfg, &p_params->pin_cfg, sizeof(app_uart_pin_cfg_t));
    s_uart_env[id].evt_handler = evt_handler;

    memcpy(&s_uart_env[id].handle.init, &p_params->init, sizeof(uart_init_t));
    s_uart_env[id].handle.p_instance = (uart_regs_t *)s_uart_instance[id];

    hal_uart_deinit(&s_uart_env[id].handle);
    hal_uart_init(&s_uart_env[id].handle);

    if (s_sleep_cb_registered_flag == false)// register sleep callback
    {
        s_sleep_cb_registered_flag = true;
        s_uart_pwr_id = pwr_register_sleep_cb(&uart_sleep_cb, APP_DRIVER_UART_WAPEUP_PRIORITY);

        if (s_uart_pwr_id < 0)
        {
            return APP_DRV_ERR_INVALID_PARAM;
        }
    }

    s_uart_env[id].uart_state = APP_UART_ACTIVITY;

    return APP_DRV_SUCCESS;
}

uint16_t app_uart_deinit(app_uart_id_t id)
{
    app_drv_err_t err_code = APP_DRV_SUCCESS;

    if ((id >= APP_UART_ID_MAX) || (s_uart_env[id].uart_state == APP_UART_INVALID))
    {
        return APP_DRV_ERR_INVALID_ID;
    }

#ifdef  ENV_RTOS_USE_SEMP
    if(s_uart_env[id].sem_tx != NULL)
    {
        app_driver_sem_deinit(s_uart_env[id].sem_tx);
        s_uart_env[id].sem_tx = NULL;
    }
    if(s_uart_env[id].sem_rx != NULL)
    {
        app_driver_sem_deinit(s_uart_env[id].sem_rx);
        s_uart_env[id].sem_rx = NULL;
    }
#endif

#ifdef ENV_RTOS_USE_MUTEX
    if(s_uart_env[id].mutex_sync != NULL)
    {
        app_driver_mutex_deinit(s_uart_env[id].mutex_sync);
        s_uart_env[id].mutex_sync = NULL;
    }
    if(s_uart_env[id].mutex_async != NULL)
    {
        app_driver_mutex_deinit(s_uart_env[id].mutex_async);
        s_uart_env[id].mutex_async = NULL;
    }
#endif

    err_code = app_io_deinit(s_uart_env[id].pin_cfg.tx.type, s_uart_env[id].pin_cfg.tx.pin);
    APP_DRV_ERR_CODE_CHECK(err_code);

    err_code = app_io_deinit(s_uart_env[id].pin_cfg.rx.type, s_uart_env[id].pin_cfg.rx.pin);
    APP_DRV_ERR_CODE_CHECK(err_code);

    if (UART_HWCONTROL_RTS_CTS == s_uart_env[id].handle.init.hw_flow_ctrl)
    {
        err_code = app_io_deinit(s_uart_env[id].pin_cfg.rts.type, s_uart_env[id].pin_cfg.rts.pin);
        APP_DRV_ERR_CODE_CHECK(err_code);

        err_code = app_io_deinit(s_uart_env[id].pin_cfg.cts.type, s_uart_env[id].pin_cfg.cts.pin);
        APP_DRV_ERR_CODE_CHECK(err_code);
    }

    hal_nvic_disable_irq(s_uart_irq[id]);

    if (s_uart_env[id].use_mode.type == APP_UART_TYPE_DMA)
    {
        err_code = app_dma_deinit(s_uart_env[id].dma_id[0]);
        APP_DRV_ERR_CODE_CHECK(err_code);

        err_code = app_dma_deinit(s_uart_env[id].dma_id[1]);
        APP_DRV_ERR_CODE_CHECK(err_code);
    }

    s_uart_env[id].uart_state = APP_UART_INVALID;

    GLOBAL_EXCEPTION_DISABLE();
    if (s_uart_env[APP_UART_ID_0].uart_state == APP_UART_INVALID && 
        s_uart_env[APP_UART_ID_1].uart_state == APP_UART_INVALID)
    {
         pwr_unregister_sleep_cb(s_uart_pwr_id);
         s_sleep_cb_registered_flag = false;
    }
    GLOBAL_EXCEPTION_ENABLE();

    app_systick_deinit();


    hal_uart_deinit(&s_uart_env[id].handle);

    return APP_DRV_SUCCESS;
}

uint16_t app_uart_receive_async(app_uart_id_t id, uint8_t *p_data, uint16_t size)
{
    hal_status_t err_code = HAL_OK;

    if (id >= APP_UART_ID_MAX ||
        p_data == NULL ||
        size == 0 ||
        s_uart_env[id].uart_state == APP_UART_INVALID)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    uart_wake_up(id);
#endif

    switch (s_uart_env[id].use_mode.type)
    {
        case APP_UART_TYPE_INTERRUPT:
            err_code = hal_uart_receive_it(&s_uart_env[id].handle, p_data, size);
            break;

        case APP_UART_TYPE_DMA:
            err_code = hal_uart_receive_dma(&s_uart_env[id].handle, p_data, size);
            break;

        case APP_UART_TYPE_POLLING:
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

uint16_t app_uart_receive_sync(app_uart_id_t id, uint8_t *p_data, uint16_t size, uint32_t timeout)
{
    hal_status_t err_code;

    if (id >= APP_UART_ID_MAX ||
        p_data == NULL ||
        size == 0 ||
        s_uart_env[id].uart_state == APP_UART_INVALID)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    uart_wake_up(id);
#endif

    err_code = hal_uart_receive(&s_uart_env[id].handle, p_data, size, timeout);
    if (err_code != HAL_OK)
    {
        return (uint16_t)err_code;
    }

    return APP_DRV_SUCCESS;
}

#ifdef  ENV_RTOS_USE_SEMP
uint16_t app_uart_receive_sem_sync(app_uart_id_t id, uint8_t *p_data, uint16_t size)
{
    hal_status_t err_code = HAL_OK;

#ifdef ENV_RTOS_USE_MUTEX
    APP_UART_DRV_ASYNC_MUTEX_LOCK(id);
#endif

    if (id >= APP_UART_ID_MAX ||
        p_data == NULL ||
        size == 0 ||
        s_uart_env[id].uart_state == APP_UART_INVALID)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_UART_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    uart_wake_up(id);
#endif

    switch (s_uart_env[id].use_mode.type)
    {
        case APP_UART_TYPE_INTERRUPT:
            err_code = hal_uart_receive_it(&s_uart_env[id].handle, p_data, size);
            break;

        case APP_UART_TYPE_DMA:
            err_code = hal_uart_receive_dma(&s_uart_env[id].handle, p_data, size);
            break;

        case APP_UART_TYPE_POLLING:
            break;

        default:
            break;
    }
    if (err_code != HAL_OK)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_UART_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
        return (uint16_t)err_code;
    }
     
    app_driver_sem_pend(s_uart_env[id].sem_rx, OS_WAIT_FOREVER);

#ifdef ENV_RTOS_USE_MUTEX
    APP_UART_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif

    return APP_DRV_SUCCESS;
}
#endif

uint16_t app_uart_transmit_async(app_uart_id_t id, uint8_t *p_data, uint16_t size)
{
    uint16_t err_code;

    if (id >= APP_UART_ID_MAX ||
        p_data == NULL ||
        size == 0 ||
        s_uart_env[id].uart_state == APP_UART_INVALID)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    uart_wake_up(id);
#endif

    ring_buffer_write(&s_uart_env[id].tx_ring_buffer, p_data, size);

    if ((s_uart_env[id].start_tx_flag == false) && (s_uart_env[id].start_flush_flag == false) &&
        (s_uart_env[id].uart_state == APP_UART_ACTIVITY) && ll_uart_is_enabled_fifo(s_uart_env[id].handle.p_instance))
    {
        s_uart_env[id].start_tx_flag = true;

        err_code = app_uart_start_transmit_async(id);
        if (err_code != APP_DRV_SUCCESS)
        {
            return err_code;
        }
    }

    return APP_DRV_SUCCESS;
}

#ifdef  ENV_RTOS_USE_SEMP 
uint16_t app_uart_transmit_sem_sync(app_uart_id_t id, uint8_t *p_data, uint16_t size)
{
    uint16_t err_code;

#ifdef ENV_RTOS_USE_MUTEX
    APP_UART_DRV_ASYNC_MUTEX_LOCK(id);
#endif

    if (id >= APP_UART_ID_MAX ||
        p_data == NULL ||
        size == 0 ||
        s_uart_env[id].uart_state == APP_UART_INVALID)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_UART_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    uart_wake_up(id);
#endif
    
    ring_buffer_write(&s_uart_env[id].tx_ring_buffer, p_data, size);

    if ((s_uart_env[id].start_tx_flag == false) && (s_uart_env[id].start_flush_flag == false) &&
        (s_uart_env[id].uart_state == APP_UART_ACTIVITY) && ll_uart_is_enabled_fifo(s_uart_env[id].handle.p_instance))
    {
        s_uart_env[id].start_tx_flag = true;
        
        err_code = app_uart_start_transmit_async(id);
        if (err_code != APP_DRV_SUCCESS)
        {
#ifdef ENV_RTOS_USE_MUTEX
            APP_UART_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
            return err_code;
        }

        app_driver_sem_pend(s_uart_env[id].sem_tx, OS_WAIT_FOREVER);
    }

#ifdef ENV_RTOS_USE_MUTEX
    APP_UART_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif

    return APP_DRV_SUCCESS;
}
#endif  

uint16_t app_uart_transmit_sync(app_uart_id_t id, uint8_t *p_data, uint16_t size, uint32_t timeout)
{
    hal_status_t err_code;

    if (id >= APP_UART_ID_MAX ||
        p_data == NULL ||
        size == 0 ||
        s_uart_env[id].uart_state == APP_UART_INVALID)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    uart_wake_up(id);
#endif

    err_code = hal_uart_transmit(&s_uart_env[id].handle, p_data, size, timeout);
    if (err_code != HAL_OK)
    {
        return err_code;
    }

    return APP_DRV_SUCCESS;
}

uart_handle_t *app_uart_get_handle(app_uart_id_t id)
{
    if (id >= APP_UART_ID_MAX || s_uart_env[id].uart_state == APP_UART_INVALID)
    {
        return NULL;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    uart_wake_up(id);
#endif

    return &s_uart_env[id].handle;
}

void app_uart_flush(app_uart_id_t id)
{
    uint16_t items_count;
    uart_handle_t *p_uart = &s_uart_env[id].handle;

    if (APP_UART_ID_MAX <= id || s_uart_env[id].uart_state == APP_UART_INVALID)
    {
        return;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    uart_wake_up(id);
#endif

    app_systick_init();

    if (s_uart_env[id].uart_state == APP_UART_ACTIVITY)
    {
        s_uart_env[id].start_flush_flag = true;

        if (APP_UART_TYPE_POLLING != s_uart_env[id].use_mode.type)
        {
            uint16_t tx_xfer_size = 0;
            uint16_t tx_xfer_count = 0;
            uint32_t tx_wait_count = 0;

            /* Disable THRE interrupt */
            __HAL_UART_DISABLE_IT(p_uart, UART_IT_THRE);

            while(!ll_uart_is_active_flag_tfe(s_uart_env[id].handle.p_instance));

            if (APP_UART_TYPE_INTERRUPT == s_uart_env[id].use_mode.type)
            {
                tx_xfer_size  = s_uart_env[id].handle.tx_xfer_size;
                tx_xfer_count = s_uart_env[id].handle.tx_xfer_count;
                hal_uart_abort_transmit_it(&s_uart_env[id].handle);
                hal_uart_transmit(&s_uart_env[id].handle,
                                  s_tx_send_buf + tx_xfer_size - tx_xfer_count,
                                  tx_xfer_count,
                                  5000);
            }
            else
            {
                do
                {
                    tx_wait_count++;
                }
                while (HAL_UART_STATE_READY != hal_uart_get_state(&s_uart_env[id].handle) &&
                       (tx_wait_count <= TX_ONCE_MAX_SIZE * (SystemCoreClock/s_uart_env[id].handle.init.baud_rate)));
            }

            do{
                items_count = ring_buffer_items_count_get(&s_uart_env[id].tx_ring_buffer);
                while(items_count)
                {
                    uint8_t send_char;

                    ring_buffer_read(&s_uart_env[id].tx_ring_buffer, &send_char, 1);

                    while(!ll_uart_is_active_flag_tfnf(s_uart_env[id].handle.p_instance));

                    ll_uart_transmit_data8(s_uart_env[id].handle.p_instance, send_char);

                    items_count--;
                }
            } while(ring_buffer_items_count_get(&s_uart_env[id].tx_ring_buffer));
        }

        while(!ll_uart_is_active_flag_tfe(s_uart_env[id].handle.p_instance));

        if (APP_UART_TYPE_POLLING != s_uart_env[id].use_mode.type)
        {
            /* Enable the UART Transmit Data Register Empty Interrupt */
            __HAL_UART_ENABLE_IT(p_uart, UART_IT_THRE);
        }

        s_uart_env[id].start_flush_flag = false;
    }
}

void hal_uart_tx_cplt_callback(uart_handle_t *p_uart)
{
    app_uart_event_call(p_uart, APP_UART_EVT_TX_CPLT); 
}

void hal_uart_rx_cplt_callback(uart_handle_t *p_uart)
{
     app_uart_event_call(p_uart, APP_UART_EVT_RX_DATA);
}

void hal_uart_error_callback(uart_handle_t *p_uart)
{
    app_uart_event_call(p_uart, APP_UART_EVT_ERROR);
}

void hal_uart_abort_tx_cplt_callback(uart_handle_t *p_uart)
{
    app_uart_event_call(p_uart, APP_UART_EVT_ABORT_TX);
}

void hal_uart_abort_rx_cplt_callback(uart_handle_t *p_uart)
{
    app_uart_event_call(p_uart, APP_UART_EVT_ABORT_RX);
}

SECTION_RAM_CODE void UART0_IRQHandler(void)
{
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_push();
#endif
    hal_uart_irq_handler(&s_uart_env[APP_UART_ID_0].handle);
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_pop();
#endif
}

SECTION_RAM_CODE void UART1_IRQHandler(void)
{
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_push();
#endif
    hal_uart_irq_handler(&s_uart_env[APP_UART_ID_1].handle);
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_pop();
#endif
}

