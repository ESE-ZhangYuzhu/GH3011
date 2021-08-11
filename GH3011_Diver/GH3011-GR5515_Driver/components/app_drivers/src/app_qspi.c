/**
  ****************************************************************************************
  * @file    app_qspi.c
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
#include "app_qspi.h"
#include "app_io.h"
#include "app_dma.h"
#include "app_pwr_mgmt.h"
#include "app_systick.h"
#include <string.h>
#include "platform_sdk.h"

#ifdef HAL_QSPI_MODULE_ENABLED

/*
 * DEFINES
 *****************************************************************************************
 */
#ifdef ENV_RTOS_USE_MUTEX

#define APP_QSPI_DRV_SYNC_MUTEX_LOCK(id)    app_driver_mutex_pend(s_qspi_env[id].mutex_sync, MUTEX_WAIT_FOREVER)
#define APP_QSPI_DRV_SYNC_MUTEX_UNLOCK(id)  app_driver_mutex_post(s_qspi_env[id].mutex_sync)

#define APP_QSPI_DRV_ASYNC_MUTEX_LOCK(id)   app_driver_mutex_pend(s_qspi_env[id].mutex_async, MUTEX_WAIT_FOREVER)
#define APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id) app_driver_mutex_post(s_qspi_env[id].mutex_async)

#endif

#define DISABLE_BLE_INT()           \
do {                                \
    uint32_t __nvic_iser0;          \
    uint32_t __nvic_iser1;          \
    __nvic_iser0 = NVIC->ISER[0];   \
    __nvic_iser1 = NVIC->ISER[1];   \
    NVIC_DisableIRQ(BLE_IRQn);      \
    NVIC_DisableIRQ(BLESLP_IRQn);   \

#define RESTORE_BLE_INT()           \
    NVIC->ISER[0] = __nvic_iser0;   \
    NVIC->ISER[1] = __nvic_iser1;   \
} while(0)


/*
 * STRUCT DEFINE
 *****************************************************************************************
 */

/**@brief App qspi state types. */
typedef enum
{
    APP_QSPI_INVALID = 0,
    APP_QSPI_ACTIVITY,
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    APP_QSPI_SLEEP,
#endif
} app_qspi_state_t;

struct qspi_env_t
{
    app_qspi_evt_handler_t  evt_handler;
    qspi_handle_t           handle;
    app_qspi_mode_t         use_mode;
    app_qspi_pin_cfg_t      pin_cfg;
    dma_id_t                dma_id;
    app_qspi_state_t        qspi_state;
    bool                    start_flag;
    volatile uint8_t        rx_done;
    volatile uint8_t        tx_done;
    volatile uint8_t        user_mode;

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
static bool qspi_prepare_for_sleep(void);
static void qspi_sleep_canceled(void);
static void qspi_wake_up_ind(void);
static uint16_t qspi_gpio_config(app_qspi_pin_cfg_t pin_cfg);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static const IRQn_Type   s_qspi_irq[APP_QSPI_ID_MAX] = { QSPI0_IRQn, QSPI1_IRQn };
static const uint32_t    s_qspi_instance[APP_QSPI_ID_MAX] = { QSPI0_BASE, QSPI1_BASE };

struct qspi_env_t s_qspi_env[APP_QSPI_ID_MAX] = {
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
static pwr_id_t   s_qspi_pwr_id;

static const app_sleep_callbacks_t qspi_sleep_cb =
{
    .app_prepare_for_sleep = qspi_prepare_for_sleep,
    .app_sleep_canceled    = qspi_sleep_canceled,
    .app_wake_up_ind       = qspi_wake_up_ind
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
static bool qspi_prepare_for_sleep(void)
{
    hal_qspi_state_t state;
    uint8_t i;

    for (i = 0; i < APP_QSPI_ID_MAX; i++)
    {
        if (s_qspi_env[i].qspi_state == APP_QSPI_ACTIVITY)
        {
            state = hal_qspi_get_state(&s_qspi_env[i].handle);
            if ((state != HAL_QSPI_STATE_RESET) && (state != HAL_QSPI_STATE_READY))
            {
                return false;
            }
            
            GLOBAL_EXCEPTION_DISABLE();
            hal_qspi_suspend_reg(&s_qspi_env[i].handle);
            GLOBAL_EXCEPTION_ENABLE();
            #ifdef APP_DRIVER_WAKEUP_CALL_FUN
            s_qspi_env[i].qspi_state = APP_QSPI_SLEEP;
            #endif
        }
    }

    return true;
}

static void qspi_sleep_canceled(void)
{
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    for (uint8_t i = 0; i < APP_QSPI_ID_MAX; i++)
    {
        if (s_qspi_env[i].qspi_state == APP_QSPI_SLEEP)
        {
            s_qspi_env[i].qspi_state = APP_QSPI_ACTIVITY;
        }
    }
#endif
}

SECTION_RAM_CODE static void qspi_wake_up_ind(void)
{
#ifndef APP_DRIVER_WAKEUP_CALL_FUN
    uint8_t i;

    for (i = 0; i < APP_QSPI_ID_MAX; i++)
    {
        if (s_qspi_env[i].qspi_state == APP_QSPI_ACTIVITY)
        {
            GLOBAL_EXCEPTION_DISABLE();
            hal_qspi_resume_reg(&s_qspi_env[i].handle);
            GLOBAL_EXCEPTION_ENABLE();

            if(s_qspi_env[i].use_mode.type == APP_QSPI_TYPE_INTERRUPT ||
                s_qspi_env[i].use_mode.type == APP_QSPI_TYPE_DMA)
            {
                hal_nvic_clear_pending_irq(s_qspi_irq[i]);
                hal_nvic_enable_irq(s_qspi_irq[i]);
            }
        }
    }
#endif
}

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
static void qspi_wake_up(app_qspi_id_t id)
{
    if (s_qspi_env[id].qspi_state == APP_QSPI_SLEEP)
    {
        GLOBAL_EXCEPTION_DISABLE();
        hal_qspi_resume_reg(&s_qspi_env[id].handle);
        GLOBAL_EXCEPTION_ENABLE();

        if(s_qspi_env[id].use_mode.type == APP_QSPI_TYPE_INTERRUPT ||
            s_qspi_env[id].use_mode.type == APP_QSPI_TYPE_DMA)
        {
            hal_nvic_clear_pending_irq(s_qspi_irq[id]);
            hal_nvic_enable_irq(s_qspi_irq[id]);
        }
        s_qspi_env[id].qspi_state = APP_QSPI_ACTIVITY;
    }

}
#endif

static uint16_t qspi_gpio_config(app_qspi_pin_cfg_t pin_cfg)
{
    app_io_init_t io_init = APP_IO_DEFAULT_CONFIG;
    app_drv_err_t err_code = APP_DRV_SUCCESS;

    io_init.mode = APP_IO_MODE_MUX;

    if (pin_cfg.cs.enable == APP_QSPI_PIN_ENABLE)
    {
        io_init.pull = pin_cfg.cs.pull;
        io_init.pin  = pin_cfg.cs.pin;
        io_init.mux  = pin_cfg.cs.mux;
        err_code = app_io_init(pin_cfg.cs.type, &io_init);
        APP_DRV_ERR_CODE_CHECK(err_code);
    }
    if (pin_cfg.clk.enable == APP_QSPI_PIN_ENABLE)
    {
        io_init.pull = pin_cfg.clk.pull;
        io_init.pin  = pin_cfg.clk.pin;
        io_init.mux  = pin_cfg.clk.mux;
        err_code = app_io_init(pin_cfg.clk.type, &io_init);
        APP_DRV_ERR_CODE_CHECK(err_code);
    }
    if (pin_cfg.io_0.enable == APP_QSPI_PIN_ENABLE)
    {
        io_init.pull = pin_cfg.io_0.pull;
        io_init.pin  = pin_cfg.io_0.pin;
        io_init.mux  = pin_cfg.io_0.mux;
        err_code = app_io_init(pin_cfg.io_0.type, &io_init);
        APP_DRV_ERR_CODE_CHECK(err_code);
    }
    if (pin_cfg.io_1.enable == APP_QSPI_PIN_ENABLE)
    {
        io_init.pull = pin_cfg.io_1.pull;
        io_init.pin  = pin_cfg.io_1.pin;
        io_init.mux  = pin_cfg.io_1.mux;
        err_code = app_io_init(pin_cfg.io_1.type, &io_init);
        APP_DRV_ERR_CODE_CHECK(err_code);
    }
    if (pin_cfg.io_2.enable == APP_QSPI_PIN_ENABLE)
    {
        io_init.pull = pin_cfg.io_2.pull;
        io_init.pin  = pin_cfg.io_2.pin;
        io_init.mux  = pin_cfg.io_2.mux;
        err_code = app_io_init(pin_cfg.io_2.type, &io_init);
        APP_DRV_ERR_CODE_CHECK(err_code);
    }
    if (pin_cfg.io_3.enable == APP_QSPI_PIN_ENABLE)
    {
        io_init.pull = pin_cfg.io_3.pull;
        io_init.pin  = pin_cfg.io_3.pin;
        io_init.mux  = pin_cfg.io_3.mux;
        err_code = app_io_init(pin_cfg.io_3.type, &io_init);
        APP_DRV_ERR_CODE_CHECK(err_code);
    }

    return err_code;
}

static uint16_t app_qspi_config_dma(app_qspi_params_t *p_params)
{
    app_dma_params_t dma_params = {DMA_Channel0, {0}};

    dma_params.channel_number             = p_params->use_mode.dma_channel;
    dma_params.init.direction             = DMA_MEMORY_TO_PERIPH;
    dma_params.init.src_increment         = DMA_SRC_INCREMENT;
    dma_params.init.dst_increment         = DMA_DST_NO_CHANGE;
    dma_params.init.src_data_alignment    = DMA_SDATAALIGN_BYTE;
    dma_params.init.dst_data_alignment    = DMA_DDATAALIGN_BYTE;
    dma_params.init.mode                  = DMA_NORMAL;
    dma_params.init.priority              = DMA_PRIORITY_LOW;

    s_qspi_env[p_params->id].dma_id = app_dma_init(&dma_params, NULL);
    if (s_qspi_env[p_params->id].dma_id < 0)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }
    s_qspi_env[p_params->id].handle.p_dma = app_dma_get_handle(s_qspi_env[p_params->id].dma_id);
    s_qspi_env[p_params->id].handle.p_dma->p_parent = (void*)&s_qspi_env[p_params->id].handle;

    return APP_DRV_SUCCESS;
}

static void app_qspi_event_call(qspi_handle_t *p_qspi, app_qspi_evt_type_t evt_type)
{
    app_qspi_evt_t qspi_evt;
    app_qspi_id_t id = APP_QSPI_ID_MAX;

    if (p_qspi->p_instance == QSPI0)
    {
        id = APP_QSPI_ID_0;
    }
    else if (p_qspi->p_instance == QSPI1)
    {
        id = APP_QSPI_ID_1;
    }

    qspi_evt.type = evt_type;
    if (evt_type == APP_QSPI_EVT_ERROR)
    {
        qspi_evt.data.error_code = p_qspi->error_code;
#ifdef  ENV_RTOS_USE_SEMP
        if (!s_qspi_env[id].user_mode)
        {
            app_driver_sem_post_from_isr(s_qspi_env[id].sem_tx);
            app_driver_sem_post_from_isr(s_qspi_env[id].sem_rx);
        }
#endif
        s_qspi_env[id].rx_done = 1;
        s_qspi_env[id].tx_done = 1;
    }
    else if (evt_type == APP_QSPI_EVT_TX_CPLT)
    {
        qspi_evt.data.size = p_qspi->tx_xfer_size - p_qspi->tx_xfer_count;
#ifdef  ENV_RTOS_USE_SEMP
        if (!s_qspi_env[id].user_mode)
        {
            app_driver_sem_post_from_isr(s_qspi_env[id].sem_tx);
        }
#endif
        s_qspi_env[id].tx_done = 1;
    }
    else if (evt_type == APP_QSPI_EVT_RX_DATA)
    {
        qspi_evt.data.size = p_qspi->rx_xfer_size - p_qspi->rx_xfer_count;
#ifdef  ENV_RTOS_USE_SEMP        
        if (!s_qspi_env[id].user_mode)
        {
            app_driver_sem_post_from_isr(s_qspi_env[id].sem_rx);
        }
#endif
        s_qspi_env[id].rx_done = 1;
    }

    s_qspi_env[id].start_flag = false;
    if(s_qspi_env[id].evt_handler != NULL)
    {
        s_qspi_env[id].evt_handler(&qspi_evt);
    }
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
uint16_t app_qspi_init(app_qspi_params_t *p_params, app_qspi_evt_handler_t evt_handler)
{
    uint8_t id = p_params->id;
    app_drv_err_t app_err_code;
    hal_status_t  hal_err_code;

    if (NULL == p_params)
    {
        return APP_DRV_ERR_POINTER_NULL;
    }

    if (id >= APP_QSPI_ID_MAX)
    {
        return APP_DRV_ERR_INVALID_ID;
    }

#ifdef  ENV_RTOS_USE_SEMP
    if(s_qspi_env[id].sem_rx == NULL){
        app_err_code = app_driver_sem_init(&s_qspi_env[id].sem_rx);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
    if(s_qspi_env[id].sem_tx == NULL){
        app_err_code = app_driver_sem_init(&s_qspi_env[id].sem_tx);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
#endif

#ifdef ENV_RTOS_USE_MUTEX
    if(s_qspi_env[id].mutex_async == NULL){
        app_err_code = app_driver_mutex_init(&s_qspi_env[id].mutex_async);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
    if(s_qspi_env[id].mutex_sync == NULL){
        app_err_code = app_driver_mutex_init(&s_qspi_env[id].mutex_sync);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
#endif

    app_systick_init();

    app_err_code = qspi_gpio_config(p_params->pin_cfg);
    APP_DRV_ERR_CODE_CHECK(app_err_code);

    if(p_params->use_mode.type == APP_QSPI_TYPE_DMA)
    {
        app_err_code = app_qspi_config_dma(p_params);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }

    if(p_params->use_mode.type != APP_QSPI_TYPE_POLLING)
    {
        hal_nvic_clear_pending_irq(s_qspi_irq[id]);
        hal_nvic_enable_irq(s_qspi_irq[id]);
    }

    s_qspi_env[id].use_mode.type = p_params->use_mode.type;
    s_qspi_env[id].use_mode.dma_channel = p_params->use_mode.dma_channel;
    memcpy(&s_qspi_env[id].pin_cfg, &p_params->pin_cfg, sizeof(app_qspi_pin_cfg_t));
    s_qspi_env[id].evt_handler = evt_handler;

    memcpy(&s_qspi_env[id].handle.init, &p_params->init, sizeof(qspi_init_t));
    s_qspi_env[id].handle.p_instance = (ssi_regs_t *)s_qspi_instance[id];
    hal_err_code = hal_qspi_deinit(&s_qspi_env[id].handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    hal_err_code =hal_qspi_init(&s_qspi_env[id].handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    if(!s_sleep_cb_registered_flag)// register sleep callback
    {
        s_sleep_cb_registered_flag = true;
        s_qspi_pwr_id = pwr_register_sleep_cb(&qspi_sleep_cb, APP_DRIVER_QSPI_WAPEUP_PRIORITY);
        if (s_qspi_pwr_id < 0)
        {
            return APP_DRV_ERR_INVALID_PARAM;
        }
    }

    s_qspi_env[id].qspi_state = APP_QSPI_ACTIVITY;

    return APP_DRV_SUCCESS;
}

uint16_t app_qspi_deinit(app_qspi_id_t id)
{
    app_drv_err_t app_err_code;
    hal_status_t  hal_err_code;

    if ((id >= APP_QSPI_ID_MAX) || (s_qspi_env[id].qspi_state == APP_QSPI_INVALID))
    {
        return APP_DRV_ERR_INVALID_ID;
    }

#ifdef  ENV_RTOS_USE_SEMP
    if(s_qspi_env[id].sem_tx != NULL){
        app_driver_sem_deinit(s_qspi_env[id].sem_tx);
        s_qspi_env[id].sem_tx = NULL;
    }
    if(s_qspi_env[id].sem_rx != NULL){
        app_driver_sem_deinit(s_qspi_env[id].sem_rx);
        s_qspi_env[id].sem_rx = NULL;
    }
#endif

#ifdef ENV_RTOS_USE_MUTEX
    if(s_qspi_env[id].mutex_sync != NULL){
        app_driver_mutex_deinit(s_qspi_env[id].mutex_sync);
        s_qspi_env[id].mutex_sync = NULL;
    }
    if(s_qspi_env[id].mutex_async != NULL){
        app_driver_mutex_deinit(s_qspi_env[id].mutex_async);
        s_qspi_env[id].mutex_async = NULL;
    }
#endif

    if (s_qspi_env[id].pin_cfg.cs.enable == APP_QSPI_PIN_ENABLE)
    {
        app_err_code = app_io_deinit(s_qspi_env[id].pin_cfg.cs.type, s_qspi_env[id].pin_cfg.cs.pin);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
    if (s_qspi_env[id].pin_cfg.clk.enable == APP_QSPI_PIN_ENABLE)
    {
        app_err_code = app_io_deinit(s_qspi_env[id].pin_cfg.clk.type, s_qspi_env[id].pin_cfg.clk.pin);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
    if (s_qspi_env[id].pin_cfg.io_0.enable == APP_QSPI_PIN_ENABLE)
    {
        app_err_code = app_io_deinit(s_qspi_env[id].pin_cfg.io_0.type, s_qspi_env[id].pin_cfg.io_0.pin);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
    if (s_qspi_env[id].pin_cfg.io_1.enable == APP_QSPI_PIN_ENABLE)
    {
        app_err_code = app_io_deinit(s_qspi_env[id].pin_cfg.io_1.type, s_qspi_env[id].pin_cfg.io_1.pin);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
    if (s_qspi_env[id].pin_cfg.io_2.enable == APP_QSPI_PIN_ENABLE)
    {
        app_err_code = app_io_deinit(s_qspi_env[id].pin_cfg.io_2.type, s_qspi_env[id].pin_cfg.io_2.pin);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
    if (s_qspi_env[id].pin_cfg.io_3.enable == APP_QSPI_PIN_ENABLE)
    {
        app_err_code = app_io_deinit(s_qspi_env[id].pin_cfg.io_3.type, s_qspi_env[id].pin_cfg.io_3.pin);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
    hal_nvic_disable_irq(s_qspi_irq[id]);
    if(s_qspi_env[id].use_mode.type == APP_QSPI_TYPE_DMA)
    {
        app_dma_deinit(s_qspi_env[id].dma_id);
    }
    s_qspi_env[id].qspi_state = APP_QSPI_INVALID;

    GLOBAL_EXCEPTION_DISABLE();
    if(s_qspi_env[APP_QSPI_ID_0].qspi_state == APP_QSPI_INVALID && 
        s_qspi_env[APP_QSPI_ID_1].qspi_state == APP_QSPI_INVALID)
    {
         pwr_unregister_sleep_cb(s_qspi_pwr_id);
         s_sleep_cb_registered_flag = false;
    }
    GLOBAL_EXCEPTION_ENABLE();

    app_systick_deinit();

    hal_err_code = hal_qspi_deinit(&s_qspi_env[id].handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_qspi_command_receive_sync(app_qspi_id_t id, app_qspi_command_t *p_cmd, uint8_t *p_data, uint32_t timeout)
{
    hal_status_t err_code;

    if (id >= APP_QSPI_ID_MAX ||
        p_cmd == NULL ||
        p_data == NULL ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif

    err_code = hal_qspi_command_receive(&s_qspi_env[id].handle, p_cmd, p_data, timeout);
    if (err_code != HAL_OK)
    {
        return (uint16_t)err_code;
    }

    return APP_DRV_SUCCESS;
}

#ifdef  ENV_RTOS_USE_SEMP
uint16_t app_qspi_command_receive_sem_sync(app_qspi_id_t id, app_qspi_command_t *p_cmd, uint8_t *p_data)
{
    hal_status_t err_code = HAL_OK;

#ifdef ENV_RTOS_USE_MUTEX
    APP_QSPI_DRV_ASYNC_MUTEX_LOCK(id);
#endif

    if (id >= APP_QSPI_ID_MAX ||
        p_cmd == NULL ||
        p_data == NULL ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID ||
        s_qspi_env[id].use_mode.type == APP_QSPI_TYPE_POLLING)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif

    if (s_qspi_env[id].start_flag == false)
    {
        s_qspi_env[id].start_flag = true;
        switch (s_qspi_env[id].use_mode.type)
        {
            case APP_QSPI_TYPE_INTERRUPT:
                err_code = hal_qspi_command_receive_it(&s_qspi_env[id].handle, p_cmd, p_data);
                break;
            case APP_QSPI_TYPE_DMA:
                err_code = hal_qspi_command_receive_dma(&s_qspi_env[id].handle, p_cmd, p_data);
                break;
            default:
                break;
        }
        if (HAL_OK != err_code)
        {
#ifdef ENV_RTOS_USE_MUTEX
            APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
            return (uint16_t)err_code;
        }
    }
    else
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
        return APP_DRV_ERR_BUSY;
    }

    app_driver_sem_pend(s_qspi_env[id].sem_rx, OS_WAIT_FOREVER);

#ifdef ENV_RTOS_USE_MUTEX
    APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif

    return APP_DRV_SUCCESS;
}

#endif

uint16_t app_qspi_command_receive_high_speed_sync(app_qspi_id_t id, app_qspi_command_t *p_cmd, uint8_t *p_data)
{
    app_drv_err_t app_err_code = APP_DRV_SUCCESS;
    hal_status_t  hal_err_code = HAL_OK;

    if (id >= APP_QSPI_ID_MAX ||
        p_cmd == NULL ||
        p_data == NULL ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID ||
        s_qspi_env[id].use_mode.type == APP_QSPI_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif

    s_qspi_env[id].user_mode = 0x1;
    DISABLE_BLE_INT();
    s_qspi_env[id].rx_done = 0;
    if (s_qspi_env[id].start_flag == false)
    {
        s_qspi_env[id].start_flag = true;
        switch (s_qspi_env[id].use_mode.type)
        {
            case APP_QSPI_TYPE_INTERRUPT:
                hal_err_code = hal_qspi_command_receive_it(&s_qspi_env[id].handle, p_cmd, p_data);
                break;
            case APP_QSPI_TYPE_DMA:
                hal_err_code = hal_qspi_command_receive_dma(&s_qspi_env[id].handle, p_cmd, p_data);
                break;
            default:
                break;
        }
        if (HAL_OK != hal_err_code)
        {
            app_err_code = (uint16_t)hal_err_code;
            goto exit;
        }
    }
    else
    {
        app_err_code = APP_DRV_ERR_BUSY;
        goto exit;
    }

    while(s_qspi_env[id].rx_done == 0);

exit:
    s_qspi_env[id].user_mode = 0x0;
    RESTORE_BLE_INT();
    return app_err_code;
}


uint16_t app_qspi_command_receive_async(app_qspi_id_t id, app_qspi_command_t *p_cmd, uint8_t *p_data)
{
    hal_status_t err_code = HAL_OK;

    if (id >= APP_QSPI_ID_MAX ||
        p_cmd == NULL ||
        p_data == NULL ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID ||
        s_qspi_env[id].use_mode.type == APP_QSPI_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif

    if (s_qspi_env[id].start_flag == false)
    {
        s_qspi_env[id].start_flag = true;
        switch (s_qspi_env[id].use_mode.type)
        {
            case APP_QSPI_TYPE_INTERRUPT:
                err_code = hal_qspi_command_receive_it(&s_qspi_env[id].handle, p_cmd, p_data);
                break;
            case APP_QSPI_TYPE_DMA:
                err_code = hal_qspi_command_receive_dma(&s_qspi_env[id].handle, p_cmd, p_data);
                break;
            default:
                break;
        }
        if (HAL_OK != err_code)
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

uint16_t app_qspi_command_transmit_sync(app_qspi_id_t id, app_qspi_command_t *p_cmd, uint8_t *p_data, uint32_t timeout)
{
    hal_status_t err_code;

    if (id >= APP_QSPI_ID_MAX ||
        p_cmd == NULL ||
        p_data == NULL ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif

    err_code = hal_qspi_command_transmit(&s_qspi_env[id].handle, p_cmd, p_data, timeout);
    if (err_code != HAL_OK)
    {
        return (uint16_t)err_code;
    }

    return APP_DRV_SUCCESS;
}

#ifdef  ENV_RTOS_USE_SEMP
uint16_t app_qspi_command_transmit_sem_sync(app_qspi_id_t id, app_qspi_command_t *p_cmd, uint8_t *p_data)
{
    hal_status_t err_code = HAL_OK;

#ifdef ENV_RTOS_USE_MUTEX
    APP_QSPI_DRV_ASYNC_MUTEX_LOCK(id);
#endif

    if (id >= APP_QSPI_ID_MAX ||
        p_cmd == NULL ||
        p_data == NULL ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID ||
        s_qspi_env[id].use_mode.type == APP_QSPI_TYPE_POLLING)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif

    if (s_qspi_env[id].start_flag == false)
    {
        s_qspi_env[id].start_flag = true;
        switch (s_qspi_env[id].use_mode.type)
        {
            case APP_QSPI_TYPE_INTERRUPT:
                err_code = hal_qspi_command_transmit_it(&s_qspi_env[id].handle, p_cmd, p_data);
                break;

            case APP_QSPI_TYPE_DMA:
                err_code = hal_qspi_command_transmit_dma(&s_qspi_env[id].handle, p_cmd, p_data);
                break;

            default:
                break;
        }
        if (err_code != HAL_OK)
        {
#ifdef ENV_RTOS_USE_MUTEX
            APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
            return (uint16_t)err_code;
        }
    }
    else
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
        return APP_DRV_ERR_BUSY;
    }

    app_driver_sem_pend(s_qspi_env[id].sem_tx, OS_WAIT_FOREVER);

#ifdef ENV_RTOS_USE_MUTEX
    APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif

    return APP_DRV_SUCCESS;
}
#endif


uint16_t app_qspi_command_transmit_high_speed_sync(app_qspi_id_t id, app_qspi_command_t *p_cmd, uint8_t *p_data)
{
    app_drv_err_t app_err_code = APP_DRV_SUCCESS;
    hal_status_t  hal_err_code = HAL_OK;

    if (id >= APP_QSPI_ID_MAX ||
        p_cmd == NULL ||
        p_data == NULL ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID ||
        s_qspi_env[id].use_mode.type == APP_QSPI_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif

    s_qspi_env[id].user_mode = 0x1;
    DISABLE_BLE_INT();
    s_qspi_env[id].tx_done = 0;
    if (s_qspi_env[id].start_flag == false)
    {
        s_qspi_env[id].start_flag = true;
        switch (s_qspi_env[id].use_mode.type)
        {
            case APP_QSPI_TYPE_INTERRUPT:
                hal_err_code = hal_qspi_command_transmit_it(&s_qspi_env[id].handle, p_cmd, p_data);
                break;

            case APP_QSPI_TYPE_DMA:
                hal_err_code = hal_qspi_command_transmit_dma(&s_qspi_env[id].handle, p_cmd, p_data);
                break;

            default:
                break;
        }
        if (hal_err_code != HAL_OK)
        {
            app_err_code =  (uint16_t)hal_err_code;
            goto exit;
        }
    }
    else
    {
        app_err_code = APP_DRV_ERR_BUSY;
        goto exit;
    }

    while(s_qspi_env[id].tx_done == 0);

exit:
    s_qspi_env[id].user_mode = 0x0;
    RESTORE_BLE_INT();
    return app_err_code;
}

uint16_t app_qspi_command_transmit_async(app_qspi_id_t id, app_qspi_command_t *p_cmd, uint8_t *p_data)
{
    hal_status_t err_code = HAL_OK;

    if (id >= APP_QSPI_ID_MAX ||
        p_cmd == NULL ||
        p_data == NULL ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID ||
        s_qspi_env[id].use_mode.type == APP_QSPI_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif

    if (s_qspi_env[id].start_flag == false)
    {
        s_qspi_env[id].start_flag = true;
        switch (s_qspi_env[id].use_mode.type)
        {
            case APP_QSPI_TYPE_INTERRUPT:
                err_code = hal_qspi_command_transmit_it(&s_qspi_env[id].handle, p_cmd, p_data);
                break;

            case APP_QSPI_TYPE_DMA:
                err_code = hal_qspi_command_transmit_dma(&s_qspi_env[id].handle, p_cmd, p_data);
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

uint16_t app_qspi_command_sync(app_qspi_id_t id, app_qspi_command_t *p_cmd, uint32_t timeout)
{
    hal_status_t err_code;

    if (id >= APP_QSPI_ID_MAX ||
        p_cmd == NULL ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif

    err_code = hal_qspi_command(&s_qspi_env[id].handle, p_cmd, timeout);
    if (err_code != HAL_OK)
    {
        return (uint16_t)err_code;
    }

    return APP_DRV_SUCCESS;
}

#ifdef  ENV_RTOS_USE_SEMP
uint16_t app_qspi_command_sem_sync(app_qspi_id_t id, app_qspi_command_t *p_cmd)
{
    hal_status_t err_code;

#ifdef ENV_RTOS_USE_MUTEX
    APP_QSPI_DRV_ASYNC_MUTEX_LOCK(id);
#endif

    if (id >= APP_QSPI_ID_MAX ||
        p_cmd == NULL ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID ||
        s_qspi_env[id].use_mode.type == APP_QSPI_TYPE_POLLING)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif

    if (s_qspi_env[id].start_flag == false)
    {
        s_qspi_env[id].start_flag = true;
        switch (s_qspi_env[id].use_mode.type)
        {
            case APP_QSPI_TYPE_INTERRUPT:
                err_code = hal_qspi_command_it(&s_qspi_env[id].handle, p_cmd);
                break;

            case APP_QSPI_TYPE_DMA:
                err_code = hal_qspi_command_dma(&s_qspi_env[id].handle, p_cmd);
                break;

            default:
                break;
        }
        if (err_code != HAL_OK)
        {
#ifdef ENV_RTOS_USE_MUTEX
            APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
            return (uint16_t)err_code;
        }
    }
    else
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
        return APP_DRV_ERR_BUSY;
    }

    app_driver_sem_pend(s_qspi_env[id].sem_tx, OS_WAIT_FOREVER);

#ifdef ENV_RTOS_USE_MUTEX
    APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif

    return APP_DRV_SUCCESS;
}
#endif

uint16_t app_qspi_command_high_speed_sync(app_qspi_id_t id, app_qspi_command_t *p_cmd)
{
    app_drv_err_t app_err_code = APP_DRV_SUCCESS;
    hal_status_t  hal_err_code = HAL_OK;

    if (id >= APP_QSPI_ID_MAX ||
        p_cmd == NULL ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID ||
        s_qspi_env[id].use_mode.type == APP_QSPI_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif

    s_qspi_env[id].user_mode = 0x1;
    DISABLE_BLE_INT();
    s_qspi_env[id].tx_done = 0;
    if (s_qspi_env[id].start_flag == false)
    {
        s_qspi_env[id].start_flag = true;
        switch (s_qspi_env[id].use_mode.type)
        {
            case APP_QSPI_TYPE_INTERRUPT:
                hal_err_code = hal_qspi_command_it(&s_qspi_env[id].handle, p_cmd);
                break;

            case APP_QSPI_TYPE_DMA:
                hal_err_code = hal_qspi_command_dma(&s_qspi_env[id].handle, p_cmd);
                break;

            default:
                break;
        }
        if (hal_err_code != HAL_OK)
        {
            app_err_code = (uint16_t)hal_err_code;
            goto exit;
        }
    }
    else
    {
        app_err_code =  APP_DRV_ERR_BUSY;
        goto exit;
    }

    while(s_qspi_env[id].tx_done == 0);

exit:
    s_qspi_env[id].user_mode = 0x0;
    RESTORE_BLE_INT();
    return app_err_code;
}

uint16_t app_qspi_command_async(app_qspi_id_t id, app_qspi_command_t *p_cmd)
{
    hal_status_t err_code;

    if (id >= APP_QSPI_ID_MAX ||
        p_cmd == NULL ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID ||
        s_qspi_env[id].use_mode.type == APP_QSPI_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif

    if (s_qspi_env[id].start_flag == false)
    {
        s_qspi_env[id].start_flag = true;
        switch (s_qspi_env[id].use_mode.type)
        {
            case APP_QSPI_TYPE_INTERRUPT:
                err_code = hal_qspi_command_it(&s_qspi_env[id].handle, p_cmd);
                break;

            case APP_QSPI_TYPE_DMA:
                err_code = hal_qspi_command_dma(&s_qspi_env[id].handle, p_cmd);
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

uint16_t app_qspi_transmit_sync(app_qspi_id_t id, uint8_t *p_data, uint32_t length, uint32_t timeout)
{
    hal_status_t err_code;

    if (id >= APP_QSPI_ID_MAX ||
        p_data == NULL ||
        length == 0 ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif

    err_code = hal_qspi_transmit(&s_qspi_env[id].handle, p_data, length, timeout);
    if (err_code != HAL_OK)
    {
        return err_code;
    }

    return APP_DRV_SUCCESS;
}

#ifdef  ENV_RTOS_USE_SEMP
uint16_t app_qspi_transmit_sem_sync(app_qspi_id_t id, uint8_t *p_data, uint32_t length)
{
    hal_status_t err_code = HAL_OK;

#ifdef ENV_RTOS_USE_MUTEX
    APP_QSPI_DRV_ASYNC_MUTEX_LOCK(id);
#endif

    if (id >= APP_QSPI_ID_MAX ||
        p_data == NULL ||
        length == 0 ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID ||
        s_qspi_env[id].use_mode.type == APP_QSPI_TYPE_POLLING)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif

    if (s_qspi_env[id].start_flag == false)
    {
        s_qspi_env[id].start_flag = true;
        switch (s_qspi_env[id].use_mode.type)
        {
            case APP_QSPI_TYPE_INTERRUPT:
                err_code = hal_qspi_transmit_it(&s_qspi_env[id].handle, p_data, length);
                break;

            case APP_QSPI_TYPE_DMA:
                err_code = hal_qspi_transmit_dma(&s_qspi_env[id].handle, p_data, length);
                break;

            default:
                break;
        }

        if (err_code != HAL_OK)
        {
#ifdef ENV_RTOS_USE_MUTEX
            APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
            return (uint16_t)err_code;
        }
    }
    else
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
        return APP_DRV_ERR_BUSY;
    }

    app_driver_sem_pend(s_qspi_env[id].sem_tx, OS_WAIT_FOREVER);

#ifdef ENV_RTOS_USE_MUTEX
    APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif

    return APP_DRV_SUCCESS;
}
#endif

uint16_t app_qspi_transmit_high_speed_sync(app_qspi_id_t id, uint8_t *p_data, uint32_t length)
{
    app_drv_err_t app_err_code = APP_DRV_SUCCESS;
    hal_status_t  hal_err_code = HAL_OK;

    if (id >= APP_QSPI_ID_MAX ||
        p_data == NULL ||
        length == 0 ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID ||
        s_qspi_env[id].use_mode.type == APP_QSPI_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif

    s_qspi_env[id].user_mode = 0x1;
    DISABLE_BLE_INT();
    s_qspi_env[id].tx_done = 0;
    if (s_qspi_env[id].start_flag == false)
    {
        s_qspi_env[id].start_flag = true;
        switch (s_qspi_env[id].use_mode.type)
        {
            case APP_QSPI_TYPE_INTERRUPT:
                hal_err_code = hal_qspi_transmit_it(&s_qspi_env[id].handle, p_data, length);
                break;

            case APP_QSPI_TYPE_DMA:
                hal_err_code = hal_qspi_transmit_dma(&s_qspi_env[id].handle, p_data, length);
                break;

            default:
                break;
        }

        if (hal_err_code != HAL_OK)
        {
            app_err_code = (uint16_t)hal_err_code;
            goto exit;
        }
    }
    else
    {
        app_err_code =  APP_DRV_ERR_BUSY;
        goto exit;
    }

    while(s_qspi_env[id].tx_done == 0);

exit:
    s_qspi_env[id].user_mode = 0x0;
    RESTORE_BLE_INT();
    return app_err_code;
}

uint16_t app_qspi_transmit_async(app_qspi_id_t id, uint8_t *p_data, uint32_t length)
{
    hal_status_t err_code = HAL_OK;

    if (id >= APP_QSPI_ID_MAX ||
        p_data == NULL ||
        length == 0 ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID ||
        s_qspi_env[id].use_mode.type == APP_QSPI_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif

    if (s_qspi_env[id].start_flag == false)
    {
        s_qspi_env[id].start_flag = true;
        switch (s_qspi_env[id].use_mode.type)
        {
            case APP_QSPI_TYPE_INTERRUPT:
                err_code = hal_qspi_transmit_it(&s_qspi_env[id].handle, p_data, length);
                break;

            case APP_QSPI_TYPE_DMA:
                err_code = hal_qspi_transmit_dma(&s_qspi_env[id].handle, p_data, length);
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

uint16_t app_qspi_receive_sync(app_qspi_id_t id, uint8_t *p_data, uint32_t length, uint32_t timeout)
{
    hal_status_t err_code;

    if (id >= APP_QSPI_ID_MAX ||
        length == 0 ||
        p_data == NULL ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif

    err_code = hal_qspi_receive(&s_qspi_env[id].handle, p_data, length, timeout);
    if (err_code != HAL_OK)
    {
        return (uint16_t)err_code;
    }

    return APP_DRV_SUCCESS;
}

#ifdef  ENV_RTOS_USE_SEMP
uint16_t app_qspi_receive_sem_sync(app_qspi_id_t id, uint8_t *p_data, uint32_t length)
{
    hal_status_t err_code = HAL_OK;

#ifdef ENV_RTOS_USE_MUTEX
    APP_QSPI_DRV_ASYNC_MUTEX_LOCK(id);
#endif

    if (id >= APP_QSPI_ID_MAX ||
        length == 0 ||
        p_data == NULL ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID ||
        s_qspi_env[id].use_mode.type == APP_QSPI_TYPE_POLLING)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif

    if (s_qspi_env[id].start_flag == false)
    {
        s_qspi_env[id].start_flag = true;
        switch (s_qspi_env[id].use_mode.type)
        {
            case APP_QSPI_TYPE_INTERRUPT:
                err_code = hal_qspi_receive_it(&s_qspi_env[id].handle, p_data, length);
                break;

            case APP_QSPI_TYPE_DMA:
                err_code = hal_qspi_receive_dma(&s_qspi_env[id].handle, p_data, length);
                break;

            default:
                break;
        }
        if (err_code != HAL_OK)
        {
#ifdef ENV_RTOS_USE_MUTEX
            APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
            return (uint16_t)err_code;
        }
    }
    else
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif
        return APP_DRV_ERR_BUSY;
    }

    app_driver_sem_pend(s_qspi_env[id].sem_rx, OS_WAIT_FOREVER);

#ifdef ENV_RTOS_USE_MUTEX
    APP_QSPI_DRV_ASYNC_MUTEX_UNLOCK(id);
#endif

    return APP_DRV_SUCCESS;
}
#endif

uint16_t app_qspi_receive_high_speed_sync(app_qspi_id_t id, uint8_t *p_data, uint32_t length)
{
    app_drv_err_t app_err_code = APP_DRV_SUCCESS;
    hal_status_t  hal_err_code = HAL_OK;

    if (id >= APP_QSPI_ID_MAX ||
        length == 0 ||
        p_data == NULL ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID ||
        s_qspi_env[id].use_mode.type == APP_QSPI_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif
    
    s_qspi_env[id].user_mode = 0x1;
    DISABLE_BLE_INT();
    s_qspi_env[id].rx_done = 0;
    if (s_qspi_env[id].start_flag == false)
    {
        s_qspi_env[id].start_flag = true;
        switch (s_qspi_env[id].use_mode.type)
        {
            case APP_QSPI_TYPE_INTERRUPT:
                hal_err_code = hal_qspi_receive_it(&s_qspi_env[id].handle, p_data, length);
                break;

            case APP_QSPI_TYPE_DMA:
                hal_err_code = hal_qspi_receive_dma(&s_qspi_env[id].handle, p_data, length);
                break;

            default:
                break;
        }
        if (hal_err_code != HAL_OK)
        {
            app_err_code = (uint16_t)hal_err_code;
            goto exit;
        }
    }
    else
    {
        app_err_code = APP_DRV_ERR_BUSY;
        goto exit;
    }

    while(s_qspi_env[id].rx_done == 0);

exit:
    s_qspi_env[id].user_mode = 0x0;
    RESTORE_BLE_INT();
    return app_err_code;
}

uint16_t app_qspi_receive_async(app_qspi_id_t id, uint8_t *p_data, uint32_t length)
{
    hal_status_t err_code = HAL_OK;

    if (id >= APP_QSPI_ID_MAX ||
        length == 0 ||
        p_data == NULL ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID ||
        s_qspi_env[id].use_mode.type == APP_QSPI_TYPE_POLLING)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif

    if (s_qspi_env[id].start_flag == false)
    {
        s_qspi_env[id].start_flag = true;
        switch (s_qspi_env[id].use_mode.type)
        {
            case APP_QSPI_TYPE_INTERRUPT:
                err_code = hal_qspi_receive_it(&s_qspi_env[id].handle, p_data, length);
                break;

            case APP_QSPI_TYPE_DMA:
                err_code = hal_qspi_receive_dma(&s_qspi_env[id].handle, p_data, length);
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

qspi_handle_t *app_qspi_get_handle(app_qspi_id_t id)
{
    if (id >= APP_QSPI_ID_MAX ||
        s_qspi_env[id].qspi_state == APP_QSPI_INVALID)
    {
        return NULL;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    qspi_wake_up(id);
#endif

    return &s_qspi_env[id].handle;
}

void hal_qspi_error_callback(qspi_handle_t *p_qspi)
{
    app_qspi_event_call(p_qspi, APP_QSPI_EVT_ERROR); 
}

void hal_qspi_rx_cplt_callback(qspi_handle_t *p_qspi)
{
    app_qspi_event_call(p_qspi, APP_QSPI_EVT_RX_DATA); 
}

void hal_qspi_tx_cplt_callback(qspi_handle_t *p_qspi)
{
    app_qspi_event_call(p_qspi, APP_QSPI_EVT_TX_CPLT); 
}

SECTION_RAM_CODE void QSPI0_IRQHandler(void)
{
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_push();
#endif
    hal_qspi_irq_handler(&s_qspi_env[APP_QSPI_ID_0].handle);
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_pop();
#endif
}

SECTION_RAM_CODE void QSPI1_IRQHandler(void)
{
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_push();
#endif
    hal_qspi_irq_handler(&s_qspi_env[APP_QSPI_ID_1].handle);
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_pop();
#endif
}

#endif
