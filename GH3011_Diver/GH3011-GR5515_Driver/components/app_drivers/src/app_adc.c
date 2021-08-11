/**
  ****************************************************************************************
  * @file    app_adc.c
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
#include <string.h>

#include "app_adc.h"
#include "app_dma.h"
#include "app_pwr_mgmt.h"
#include "app_systick.h"
#include "gr551x_adc_voltage_api.h"


#ifdef HAL_ADC_MODULE_ENABLED

/*
 * DEFINES
 *****************************************************************************************
 */

#ifdef ENV_RTOS_USE_MUTEX

#define APP_ADC_DRV_SYNC_MUTEX_LOCK     app_driver_mutex_pend(s_adc_env.mutex_sync, MUTEX_WAIT_FOREVER)
#define APP_ADC_DRV_SYNC_MUTEX_UNLOCK   app_driver_mutex_post(s_adc_env.mutex_sync)

#define APP_ADC_DRV_ASYNC_MUTEX_LOCK    app_driver_mutex_pend(s_adc_env.mutex_async, MUTEX_WAIT_FOREVER)
#define APP_ADC_DRV_ASYNC_MUTEX_UNLOCK  app_driver_mutex_post(s_adc_env.mutex_async)

#endif

/*
 * STRUCT DEFINE
 *****************************************************************************************
 */

/**@brief App adc state types. */
typedef enum
{
    APP_ADC_INVALID = 0,
    APP_ADC_ACTIVITY,
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    APP_ADC_SLEEP,
#endif
} app_adc_state_t;

struct adc_env_t
{
    app_adc_evt_handler_t   evt_handler;
    adc_handle_t            handle;
    app_adc_mode_t          use_mode;
    app_adc_pin_cfg_t       pin_cfg;
    dma_id_t                dma_id;
    app_adc_state_t         adc_state;

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
static bool adc_prepare_for_sleep(void);
static void adc_sleep_canceled(void);
static void adc_wake_up_ind(void);
static uint16_t adc_config_gpio(uint32_t input_mode, app_adc_pin_cfg_t pin_cfg, uint32_t ref_source);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
struct adc_env_t s_adc_env = {
    .evt_handler = NULL,
#ifdef ENV_RTOS_USE_SEMP   
    .sem_rx = NULL,
#endif
#ifdef ENV_RTOS_USE_MUTEX
    .mutex_sync = NULL,
    .mutex_async = NULL,
#endif
};
static bool      s_sleep_cb_registered_flag = false;
static pwr_id_t  s_adc_pwr_id;

const static app_sleep_callbacks_t adc_sleep_cb =
{
    .app_prepare_for_sleep = adc_prepare_for_sleep,
    .app_sleep_canceled    = adc_sleep_canceled,
    .app_wake_up_ind       = adc_wake_up_ind
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
static bool adc_prepare_for_sleep(void)
{
    hal_adc_state_t state;

    if (s_adc_env.adc_state == APP_ADC_ACTIVITY)
    {
        state = hal_adc_get_state(&s_adc_env.handle);
        if ((state != HAL_ADC_STATE_READY) && (state != HAL_ADC_STATE_RESET))
        {
            return false;
        }

        GLOBAL_EXCEPTION_DISABLE();
        hal_adc_suspend_reg(&s_adc_env.handle);
        GLOBAL_EXCEPTION_ENABLE();

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
        s_adc_env.adc_state = APP_ADC_SLEEP;
#endif
    }

    return true;
}


static void adc_sleep_canceled(void)
{
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    if (s_adc_env.adc_state == APP_ADC_SLEEP)
    {
        s_adc_env.adc_state = APP_ADC_ACTIVITY;
    }
#endif
}

SECTION_RAM_CODE static void adc_wake_up_ind(void)
{
#ifndef APP_DRIVER_WAKEUP_CALL_FUN
    if (s_adc_env.adc_state == APP_ADC_ACTIVITY)
    {
        GLOBAL_EXCEPTION_DISABLE();
        hal_adc_resume_reg(&s_adc_env.handle);
        GLOBAL_EXCEPTION_ENABLE();
    }
#endif
}

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
static void adc_wake_up(void)
{
    if (s_adc_env.adc_state == APP_ADC_SLEEP)
    {
        GLOBAL_EXCEPTION_DISABLE();
        hal_adc_resume_reg(&s_adc_env.handle);
        GLOBAL_EXCEPTION_ENABLE();

        s_adc_env.adc_state = APP_ADC_ACTIVITY;
    }
}
#endif

static uint16_t adc_config_gpio(uint32_t input_mode, app_adc_pin_cfg_t pin_cfg, uint32_t ref_source)
{
    app_io_init_t io_init = APP_IO_DEFAULT_CONFIG;
    app_drv_err_t err_code = APP_DRV_SUCCESS;

    io_init.mode = APP_IO_MODE_ANALOG;
    if (input_mode == LL_ADC_INPUT_DIFFERENTIAL)
    {
        io_init.pin  = pin_cfg.channel_p.pin;
        io_init.mux  = pin_cfg.channel_p.mux;
        err_code = app_io_init(pin_cfg.channel_p.type, &io_init);
        APP_DRV_ERR_CODE_CHECK(err_code);
    }

    io_init.pin  = pin_cfg.channel_n.pin;
    io_init.mux  = pin_cfg.channel_n.mux;
    err_code = app_io_init(pin_cfg.channel_n.type, &io_init);
    APP_DRV_ERR_CODE_CHECK(err_code);
    
    if (ref_source >= LL_ADC_REF_SRC_IO0)
    {
        io_init.pin  = pin_cfg.extern_ref.pin;
        io_init.mux  = pin_cfg.extern_ref.mux;
        err_code = app_io_init(pin_cfg.extern_ref.type, &io_init);
        APP_DRV_ERR_CODE_CHECK(err_code);
    }

    return err_code;
}

static uint16_t adc_config_dma(app_adc_params_t *p_params)
{
    app_dma_params_t dma_params;

    dma_params.channel_number            = p_params->use_mode.dma_channel;
    dma_params.init.src_request          = DMA_REQUEST_SNSADC;
    dma_params.init.direction            = DMA_PERIPH_TO_MEMORY;
    dma_params.init.src_increment        = DMA_SRC_NO_CHANGE;
    dma_params.init.dst_increment        = DMA_DST_INCREMENT;
    dma_params.init.src_data_alignment   = DMA_SDATAALIGN_WORD;
    dma_params.init.dst_data_alignment   = DMA_DDATAALIGN_WORD;
    dma_params.init.mode                 = DMA_NORMAL;
    dma_params.init.priority             = DMA_PRIORITY_LOW;

    s_adc_env.dma_id = app_dma_init(&dma_params, NULL);
    if (s_adc_env.dma_id < 0)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }
    s_adc_env.handle.p_dma = app_dma_get_handle(s_adc_env.dma_id);
    s_adc_env.handle.p_dma->p_parent = (void*)&s_adc_env.handle;

    return APP_DRV_SUCCESS;
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
uint16_t app_adc_init(app_adc_params_t *p_params, app_adc_evt_handler_t evt_handler)
{
    app_drv_err_t app_err_code;
    hal_status_t  hal_err_code;

    if (p_params == NULL)
    {
        return APP_DRV_ERR_POINTER_NULL;
    }

    app_systick_init();
    
#ifdef  ENV_RTOS_USE_SEMP
    if(s_adc_env.sem_rx == NULL){
        app_err_code = app_driver_sem_init(&s_adc_env.sem_rx);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
#endif

#ifdef ENV_RTOS_USE_MUTEX
    if(s_adc_env.mutex_async == NULL)
    {
        app_err_code = app_driver_mutex_init(&s_adc_env.mutex_async);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
    if(s_adc_env.mutex_sync == NULL){
        app_err_code = app_driver_mutex_init(&s_adc_env.mutex_sync);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
#endif

    app_err_code = adc_config_gpio(p_params->init.input_mode, p_params->pin_cfg, p_params->init.ref_source);
    APP_DRV_ERR_CODE_CHECK(app_err_code);

    if(p_params->use_mode.type == APP_ADC_TYPE_DMA)
    {
        app_err_code = adc_config_dma(p_params);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }

    s_adc_env.use_mode.type = p_params->use_mode.type;
    s_adc_env.use_mode.dma_channel = p_params->use_mode.dma_channel;
    memcpy(&s_adc_env.pin_cfg, &p_params->pin_cfg, sizeof(app_adc_pin_cfg_t));
    s_adc_env.evt_handler = evt_handler;

    memcpy(&s_adc_env.handle.init, &p_params->init, sizeof(adc_init_t));
    hal_err_code = hal_adc_deinit(&s_adc_env.handle);
    HAL_ERR_CODE_CHECK(hal_err_code);
    hal_err_code = hal_adc_init(&s_adc_env.handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    if(!s_sleep_cb_registered_flag)    // register sleep callback
    {
        s_sleep_cb_registered_flag = true;
        s_adc_pwr_id = pwr_register_sleep_cb(&adc_sleep_cb, APP_DRIVER_ADC_WAPEUP_PRIORITY);
        if (s_adc_pwr_id < 0)
        {
            return APP_DRV_ERR_INVALID_PARAM;
        }
    }

    s_adc_env.adc_state = APP_ADC_ACTIVITY;

    return APP_DRV_SUCCESS;
}

uint16_t app_adc_deinit(void)
{
    app_drv_err_t app_err_code;
    hal_status_t  hal_err_code;

#ifdef  ENV_RTOS_USE_SEMP
    if(s_adc_env.sem_rx != NULL)
    {
        app_driver_sem_deinit(s_adc_env.sem_rx);
        s_adc_env.sem_rx = NULL;
    }
#endif

#ifdef ENV_RTOS_USE_MUTEX
    if(s_adc_env.mutex_sync != NULL)
    {
        app_driver_mutex_deinit(s_adc_env.mutex_sync);
        s_adc_env.mutex_sync = NULL;
    }
    if(s_adc_env.mutex_async != NULL){
        app_driver_mutex_deinit(s_adc_env.mutex_async);
        s_adc_env.mutex_async = NULL;
    }
#endif

    if(s_adc_env.use_mode.type == APP_ADC_TYPE_DMA)
    {
        app_err_code = app_dma_deinit(s_adc_env.dma_id);
        APP_DRV_ERR_CODE_CHECK(app_err_code);
    }
    s_adc_env.adc_state = APP_ADC_INVALID;

    GLOBAL_EXCEPTION_DISABLE();
    pwr_unregister_sleep_cb(s_adc_pwr_id);
    s_sleep_cb_registered_flag = false;
    GLOBAL_EXCEPTION_ENABLE();

    app_systick_deinit();

    hal_err_code = hal_adc_deinit(&s_adc_env.handle);
    HAL_ERR_CODE_CHECK(hal_err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_adc_conversion_sync(uint16_t *p_data, uint32_t length, uint32_t timeout)
{
    hal_status_t err_code;

    if (p_data == NULL ||
        length == 0 ||
        s_adc_env.adc_state == APP_ADC_INVALID)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    adc_wake_up();
#endif

    err_code = hal_adc_poll_for_conversion(&s_adc_env.handle, p_data, length);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

#ifdef  ENV_RTOS_USE_SEMP  
uint16_t app_adc_conversion_sem_sync(uint16_t *p_data, uint32_t length)
{
    hal_status_t err_code;

#ifdef ENV_RTOS_USE_MUTEX
    APP_ADC_DRV_ASYNC_MUTEX_LOCK;
#endif

    if (p_data == NULL ||
        length == 0 ||
        s_adc_env.adc_state == APP_ADC_INVALID)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_ADC_DRV_ASYNC_MUTEX_UNLOCK;
#endif
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    adc_wake_up();
#endif

    err_code = hal_adc_start_dma(&s_adc_env.handle, p_data, length);
    if (err_code != HAL_OK)
    {
#ifdef ENV_RTOS_USE_MUTEX
        APP_ADC_DRV_ASYNC_MUTEX_UNLOCK;
#endif
        return (uint16_t)err_code;
    }
    app_driver_sem_pend(s_adc_env.sem_rx, OS_WAIT_FOREVER);

#ifdef ENV_RTOS_USE_MUTEX
    APP_ADC_DRV_ASYNC_MUTEX_UNLOCK;
#endif

    return APP_DRV_SUCCESS;
}
#endif

uint16_t app_adc_conversion_async(uint16_t *p_data, uint32_t length)
{
    hal_status_t err_code;

    if (p_data == NULL ||
        length == 0 ||
        s_adc_env.adc_state == APP_ADC_INVALID ||
        s_adc_env.use_mode.type != APP_ADC_TYPE_DMA)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    adc_wake_up();
#endif

    err_code = hal_adc_start_dma(&s_adc_env.handle, p_data, length);
    HAL_ERR_CODE_CHECK(err_code);

    return APP_DRV_SUCCESS;
}

uint16_t app_adc_voltage_intern(uint16_t *inbuf, double *outbuf, uint32_t buflen)
{
    if (inbuf == NULL || outbuf == NULL || buflen == 0 || s_adc_env.adc_state == APP_ADC_INVALID)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

    hal_gr551x_adc_voltage_intern(&s_adc_env.handle, inbuf, outbuf, buflen);

    return APP_DRV_SUCCESS;
}

uint16_t app_adc_voltage_extern(double ref, uint16_t *inbuf, double *outbuf, uint32_t buflen)
{
    if (inbuf == NULL || outbuf == NULL || buflen == 0 || s_adc_env.adc_state == APP_ADC_INVALID)
    {
        return APP_DRV_ERR_INVALID_PARAM;
    }

    hal_gr551x_adc_voltage_extern(&s_adc_env.handle, ref, inbuf, outbuf, buflen);

    return APP_DRV_SUCCESS;
}

adc_handle_t *app_adc_get_handle(void)
{
    if (s_adc_env.adc_state == APP_ADC_INVALID)
    {
        return NULL;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    adc_wake_up();
#endif

    return &s_adc_env.handle;
}

void hal_adc_conv_cplt_callback(adc_handle_t *p_adc)
{
    app_adc_evt_t evt;

    evt.type = APP_ADC_EVT_CONV_CPLT;
    
#ifdef  ENV_RTOS_USE_SEMP        
    app_driver_sem_post_from_isr(s_adc_env.sem_rx);
#endif  

    if (s_adc_env.evt_handler != NULL)
    {
        s_adc_env.evt_handler(&evt);
    }
}

#endif
