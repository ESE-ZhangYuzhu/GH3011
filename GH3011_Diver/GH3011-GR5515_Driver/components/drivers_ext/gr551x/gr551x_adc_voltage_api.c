/**
 ****************************************************************************************
 *
 * @file gr551x_adc_voltage_api.c
 *
 * @brief GR551x ADC voltage module.
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
 *****************************************************************************************
 */

/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include <math.h>
#include "gr55xx_hal.h"
#include "gr55xx_sys.h"
#include "gr551x_adc_voltage_api.h"

/*
 * DEFINE
 *****************************************************************************************
 */
#define ADC_UNUSED_CONV_LENGTH         (16UL)    /**< Invaild calibrated buffer. */
#define ADC_USED_CONV_LENGTH           (16UL)    /**< Used calibrated buffer.    */

/*
 * STRUCTURES
 *****************************************************************************************
 */
typedef struct {
    uint32_t offset_int_0p8;
    uint32_t slope_int_0p8;
    uint32_t offset_int_1p2;
    uint32_t slope_int_1p2;
    uint32_t offset_int_1p6;
    uint32_t slope_int_1p6;
    uint32_t offset_int_2p0;
    uint32_t slope_int_2p0;
    uint32_t offset_ext_1p0;
    uint32_t slope_ext_1p0;
} adc_trim_params_t;

/*
 * GLOBAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */

/*
 * STATIC VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static uint32_t diff_int_vref   = 0;
static uint16_t diff_int_offset = 0;
static double   diff_ext_vref   = 0;
static uint16_t diff_ext_offset = 0; 

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
static void load_trim_for_sdk(adc_trim_params_t *param)
{
    adc_trim_info_t adc_trim = {0};

    if(SDK_SUCCESS == sys_adc_trim_get(&adc_trim))
    {
        param->offset_int_0p8 = adc_trim.offset_int_0p8;
        param->slope_int_0p8  = adc_trim.slope_int_0p8;
        param->offset_int_1p2 = adc_trim.offset_int_1p2;
        param->slope_int_1p2  = adc_trim.slope_int_1p2;
        param->offset_int_1p6 = adc_trim.offset_int_1p6;
        param->slope_int_1p6  = adc_trim.slope_int_1p6;
        param->offset_int_2p0 = adc_trim.offset_int_2p0;
        param->slope_int_2p0  = adc_trim.slope_int_2p0;    
        param->offset_ext_1p0 = adc_trim.offset_ext_1p0;
        param->slope_ext_1p0  = adc_trim.slope_ext_1p0;
    }

    return;
}

static uint16_t diff_calculate_offset(adc_handle_t *hadc)
{
    uint32_t sum = 0;
    uint32_t adc_channel_n = 0;
    uint32_t adc_channel_p = 0;
    uint16_t conversion_avg = 0;
    uint16_t conversion[ADC_UNUSED_CONV_LENGTH + ADC_USED_CONV_LENGTH] = {0};

    adc_channel_n = hadc->init.channel_n;
    adc_channel_p = hadc->init.channel_p;
    hadc->init.channel_n = ADC_INPUT_SRC_REF;
    hadc->init.channel_p = ADC_INPUT_SRC_REF;

    hal_adc_deinit(hadc);
    hal_adc_init(hadc);
    hal_adc_poll_for_conversion(hadc, conversion, ADC_UNUSED_CONV_LENGTH + ADC_USED_CONV_LENGTH);
    for(uint16_t i = ADC_UNUSED_CONV_LENGTH; i < ADC_UNUSED_CONV_LENGTH + ADC_USED_CONV_LENGTH; i++)
    {
        sum += conversion[i];
    }
    conversion_avg = sum / ADC_USED_CONV_LENGTH;
    hadc->init.channel_n = adc_channel_n;
    hadc->init.channel_p = adc_channel_p;

    hal_adc_deinit(hadc);
    hal_adc_init(hadc);

    return (conversion_avg << 1);
}

void hal_gr551x_adc_voltage_intern(adc_handle_t *hadc, uint16_t *inbuf, double *outbuf, uint32_t buflen)
{
    double cslope = 0, coffset = 0;
    adc_trim_params_t trim = {0};

    load_trim_for_sdk(&trim);

    if (ADC_REF_VALUE_0P8 == hadc->init.ref_value)
    {
        coffset = (double)trim.offset_int_0p8;
        cslope  = (-1) * (double)trim.slope_int_0p8;
    }
    else if (ADC_REF_VALUE_1P2 == hadc->init.ref_value)
    {
        coffset = (double)trim.offset_int_1p2;
        cslope  = (-1) * (double)trim.slope_int_1p2;
    }
    else if (ADC_REF_VALUE_1P6 == hadc->init.ref_value)
    {
        coffset = (double)trim.offset_int_1p6;
        cslope  = (-1) * (double)trim.slope_int_1p6;
    }
    else if (ADC_REF_VALUE_2P0 == hadc->init.ref_value)
    {
        coffset = (double)trim.offset_int_2p0;
        cslope  = (-1) * (double)trim.slope_int_2p0;
    }

    if (ADC_INPUT_SINGLE == hadc->init.input_mode)
    {
        for (uint32_t i = 0; i < buflen; i++)
        {
            outbuf[i] = ((double)inbuf[i] - coffset) / cslope;
        }
    }
    else
    {
        if (diff_int_vref != hadc->init.ref_value)
        {
            diff_int_vref = hadc->init.ref_value;
            diff_int_offset = diff_calculate_offset(hadc);
        }
        coffset = (double)diff_int_offset;
        for (uint32_t i = 0; i < buflen; i++)
        {
            outbuf[i] = (coffset - (double)inbuf[i] * 2) / cslope;
        }
    }

    return;
}

void hal_gr551x_adc_voltage_extern(adc_handle_t *hadc, double vref, uint16_t *inbuf, double *outbuf, uint32_t buflen)
{
    double cslope = 0, coffset = 0;
    adc_trim_params_t trim = {0};

    load_trim_for_sdk(&trim);

    if (ADC_INPUT_SINGLE == hadc->init.input_mode)
    {
        coffset = (double)trim.offset_ext_1p0;
        cslope  = ((-1) * (double)trim.slope_ext_1p0) * 1.0f / vref;
        for (uint32_t i = 0; i < buflen; i++)
        {
            outbuf[i] = ((double)inbuf[i] - coffset) / cslope;
        }
    }
    else
    {
        if (fabs(diff_ext_vref - vref) > 0.00001)
        {
            diff_ext_vref = vref;
            diff_ext_offset = diff_calculate_offset(hadc);
        }
        coffset = (double)diff_ext_offset;
        cslope  = ((-1) * (double)trim.slope_ext_1p0) * 1.0f / vref;
        for (uint32_t i = 0; i < buflen; i++)
        {
            outbuf[i] = (coffset - (double)inbuf[i] * 2) / cslope;
        }
    }

    return;
}

