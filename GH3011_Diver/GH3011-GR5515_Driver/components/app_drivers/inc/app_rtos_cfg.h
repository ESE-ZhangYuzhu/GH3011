/**
 ****************************************************************************************
 *
 * @file    app_rtos_cfg.h
 * @author  BLE Driver Team
 * @brief   Header file of app rtos config code.
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
 ****************************************************************************************
 */
 
/** @addtogroup PERIPHERAL Peripheral Driver
  * @{
  */
  
/** @addtogroup APP_DRIVER APP DRIVER
 *  @{
 */

/** @defgroup APP_RTOS_CONFIG RTOS CONFIG
  * @brief ADC RTOS CONFIG
  * @{
  */

 
#ifndef __APP_RTOS_ADAPTER_H
#define __APP_RTOS_ADAPTER_H
/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include <string.h>
#include <stdint.h>

#ifdef ENV_USE_FREERTOS

#include "FreeRTOS.h"
#include "semphr.h"


#define ENV_USE_RTOS

typedef SemaphoreHandle_t             sem_t;
typedef SemaphoreHandle_t             mutex_t;

#define OS_WAIT_FOREVER               portMAX_DELAY

#define SEM_WAIT_FOREVER              portMAX_DELAY
#define SEM_NO_WAIT                   (0)

#define MUTEX_WAIT_FOREVER            portMAX_DELAY
#define MUTEX_NO_WAIT                 (0)

#else

typedef void *                        sem_t;
typedef void *                        mutex_t;

#define SEM_WAIT_FOREVER              (0xFFFFUL)
#define SEM_NO_WAIT                   (0)

#define MUTEX_WAIT_FOREVER            (0xFFFFUL)
#define MUTEX_NO_WAIT                 (0)

#endif

#define APP_DRV_SEM_DECL(sem)          sem_t   sem
#define APP_DRV_MUTEX_DECL(mutex)      mutex_t mutex

#define APP_DRV_SEM_STATIC(sem)        static SEM_DECL(sem)
#define APP_DRV_MUTEX_STATIC(mutex)    static MUTEX_DECL(mutex)

#ifdef ENV_USE_RTOS

#define ENV_RTOS_USE_SEMP    1
//#define ENV_RTOS_USE_MUTEX 1

uint16_t app_driver_sem_init(sem_t *sem);
void app_driver_sem_deinit(sem_t sem);
uint16_t app_driver_sem_pend(sem_t sem, uint32_t time_out);
uint16_t app_driver_sem_post(sem_t sem);
uint16_t app_driver_sem_post_from_isr(sem_t sem);

uint16_t app_driver_mutex_init(mutex_t *mutex);
void app_driver_mutex_deinit(mutex_t mutex);
uint16_t app_driver_mutex_pend(mutex_t mutex, uint32_t time_out);
uint16_t app_driver_mutex_post(mutex_t mutex);

#else

#define app_driver_sem_init(x)        (0)
#define app_driver_sem_deinit(x)
#define app_driver_sem_pend(x, y)     (0)
#define app_driver_sem_post(x)
#define app_driver_sem_post_from_isr(x)

#define app_driver_mutex_init(x)      (0)
#define app_driver_mutex_deinit(x)
#define app_driver_mutex_pend(x, y) 
#define app_driver_mutex_post(x)

#endif

#endif
/** @} */
/** @} */
/** @} */

