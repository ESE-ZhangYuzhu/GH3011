/**
  ****************************************************************************************
  * @file    gr55xx_hal_rng.c
  * @author  BLE Driver Team
  * @brief   RNG HAL module driver.
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
#include "gr55xx_hal.h"

#if defined(HAL_RNG_MODULE_ENABLED) && defined(GR5515_D)

/* extern function -----------------------------------------------------------*/

extern hal_status_t hal_rng_init_ext(rng_handle_t *p_rng);
extern hal_status_t hal_rng_deinit_ext(rng_handle_t *p_rng);
extern void hal_rng_register_callback(hal_rng_callback_t *hal_rng_callback);

/* Private variables ---------------------------------------------------------*/

static hal_rng_callback_t rng_callback =
{
    .rng_msp_init               = hal_rng_msp_init,
    .rng_msp_deinit             = hal_rng_msp_deinit,
    .rng_ready_data_callback    = hal_rng_ready_data_callback
};

/* Private function prototypes -----------------------------------------------*/

hal_status_t hal_rng_init(rng_handle_t *p_rng)
{
    hal_rng_register_callback(&rng_callback);

    return hal_rng_init_ext(p_rng);
}

hal_status_t hal_rng_deinit(rng_handle_t *p_rng)
{
    hal_rng_register_callback(&rng_callback);

    return hal_rng_deinit_ext(p_rng);
}

__WEAK void hal_rng_msp_init(rng_handle_t *p_rng)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(p_rng);

    /* NOTE: This function should not be modified, when the callback is needed,
        the hal_rng_msp_init could be implemented in the user file
    */
}

__WEAK void hal_rng_msp_deinit(rng_handle_t *p_rng)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(p_rng);

    /* NOTE: This function should not be modified, when the callback is needed,
        the hal_rng_msp_init could be implemented in the user file
    */
}

__weak void hal_rng_ready_data_callback(rng_handle_t *p_rng, uint32_t random32bit)
{
    UNUSED(p_rng);
    UNUSED(random32bit);
    /* NOTE : This function should not be modified. When the callback is needed,
            function hal_rng_ready_data_callback must be implemented in the user file.
     */
}

#endif /* HAL_WDT_MODULE_ENABLED */
