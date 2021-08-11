/**
 *****************************************************************************************
 *
 * @file bcs.h
 *
 * @brief Body Composition Service API.
 *
 *****************************************************************************************
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

#ifndef _BCS_H_
#define _BCS_H_

#include "gr55xx_sys.h"
#include "custom_config.h"
#include <stdint.h>
#include <stdbool.h>

/*
 * DEFINES
 *****************************************************************************************
 */
/** Maximum number of BCS connections. */
#define BCS_CONNECTION_MAX  (10 < CFG_MAX_CONNECTIONS ?\
                             10 : CFG_MAX_CONNECTIONS)

/*
 * ENUMERATIONS
 *****************************************************************************************
 */
/**@brief Body Composition Feature characteristic bit values. The size of this
 * characteristic is 32bits. */
typedef enum
{
    /* Supported Flags */
    BCS_FEAT_TIME_STAMP        = 0x0001,  /**< Time Stamp supported */
    BCS_FEAT_MULTI_USER        = 0x0002,  /**< Multiple Users supported */
    BCS_FEAT_BASAL_METABOLISM  = 0x0004,  /**< Basal metabolism supported */
    BCS_FEAT_MUSCLE_PERCENTAGE = 0x0008,  /**< Muscle percentage supported */
    BCS_FEAT_MUSCLE_MASS       = 0x0010,  /**< Muscle mas supported */
    BCS_FEAT_FAT_FREE_MASS     = 0x0020,  /**< Fat free mass supported */
    BCS_FEAT_SOFT_LEAN_MASS    = 0x0040,  /**< Soft lean mass supported */
    BCS_FEAT_BODY_WATER_MASS   = 0x0080,  /**< Body water mass supported */
    BCS_FEAT_IMPEDANCE         = 0x0100,  /**< Impedance supported */
    BCS_FEAT_WEIGHT            = 0x0200,  /**< Weight supported */
    BCS_FEAT_HEIGHT            = 0x0400,  /**< Height supported */

    /* Mass Resolution */
    BCS_FEAT_MASS_RES_500G     = 0x0800, /**< Resolution of 0.5kg or 1lb */
    BCS_FEAT_MASS_RES_200G     = 0x1000, /**< Resolution of 0.2kg or 0.5lb */
    BCS_FEAT_MASS_RES_100G     = 0x1800, /**< Resolution of 0.1kg or 0.2lb */
    BCS_FEAT_MASS_RES_50G      = 0x2000, /**< Resolution of 0.05kg or 0.1lb */
    BCS_FEAT_MASS_RES_20G      = 0x2800, /**< Resolution of 0.02kg or 0.05lb */
    BCS_FEAT_MASS_RES_10G      = 0x3000, /**< Resolution of 0.01kg or 0.02lb */
    BCS_FEAT_MASS_RES_5G       = 0x3800, /**< Resolution of 0.005kg or 0.01lb */

    /* Height Resolution */
    BCS_FEAT_HEIGHT_RES_10MM   = 0x008000, /**< Resolution of 0.01m or 1in */
    BCS_FEAT_HEIGHT_RES_5MM    = 0x010000, /**< Resolution of 0.005m or 0.5in */
    BCS_FEAT_HEIGHT_RES_1MM    = 0x018000, /**< Resolution of 0.001m or 0.1in */
    
    WSS_FEAT_FORCE_SIZE = 0x7FFFFFFF,
} bcs_feature_t;

/**@brief BCS unit types */
typedef enum
{
    BCS_UNIT_SI,        /**< Weight in kilograms and height in meters */
    BCS_UNIT_IMPERIAL,  /**< Weight in pounds and height in inches */
} bcs_unit_t;

/**@brief Body Composition Service event type. */
typedef enum
{
    BCS_EVT_INDICATION_SENT,     /**< Response for sent Indicate */
    BCS_EVT_INDICATION_ENABLED,  /**< Indication status of BC Measurement */
} bcs_evt_type_t;

/*
 * TYPE DEFINITIONS
 *****************************************************************************************
 */
/**@brief Body composition Measurement data. All fixed point values have
 *        precision as defined in BCS spec. The unit of mass and height values
 *        are determined by the bit 0 of the Flags field, 0 for SI and 1 for
 *        imperial. */
typedef struct
{
    /** User id of 0xFF for guest user. */
    uint8_t         user_id;
    /** Measurement unit */
    bcs_unit_t      unit;
    /** Flag which indicates if time-stamp is present. */
    bool            timestamp_present;
    /** Time of measurement, NULL if timestamp_present is false. */
    prf_date_time_t timestamp;
    /** The precision of Body Fat Percentage is 0.1. */
    uint16_t        body_fat_percentage;
    /** The unit of Basal Metabolism is kilo Joules with resolution of 1. */
    uint16_t        basal_metabolism;
    /** The precision of Muscle Percentage is 0.1. */
    uint16_t        muscle_percentage;
    /** The unit of muscle mass is kilograms with resolution of 0.005, or is
     * pounds with resolution of 0.01. */
    uint16_t        muscle_mass;
    /** The unit of Fat Free Mass is kilograms with resolution of 0.005, or is
     *  pounds with resolution of 0.01. */
    uint16_t        fat_free_mass;
    /** The unit of Soft Lean Mass is kilograms with resolution of 0.005, or is
     *  pounds with resolution of 0.01. */
    uint16_t        soft_lean_mass;
    /** The unit of Body Water Mass is kilograms with resolution of 0.005, or is
     *  pounds with resolution of 0.01. */
    uint16_t        body_water_mass;
    /** The unit of Impedance is Ohms with resolution 0.1. */
    uint16_t        impedance;
    /** The unit of Weight is kilograms with resolution of 0.005, or is pounds
     *  with resolution 0.01. */
    uint16_t        weight;
    /** The unit of Height is meters with resolution of 0.001, or is inches with
     *  resolution 0.1. */
    uint16_t        height;
} bcs_meas_t;

/**@brief Body Composition Service event. */
typedef struct
{
    bcs_evt_type_t evt_type;    /**< Type of event. */
    union
    {
        /** status for sent indication, true for success, false for failure. */
        bool status;
        /** true for indication enabled, false for indication disabled */
        bool enabled;
    } params;
} bcs_evt_t;

/**@brief Body Composition Service event handler type. */
typedef void (*bcs_evt_handler_t)(bcs_evt_t *p_evt);

/**@brief Body Composition Service environment variable. */
typedef struct 
{
    /** Body Composition Service event handler. */
    bcs_evt_handler_t evt_handler;
    /** Body Composition Service start handle. */
    uint16_t          start_hdl;
    /** Bit mask of supported features. */
    bcs_feature_t     bcs_features;
    /** Client characteristic configuration descriptor for each connection. */
    uint16_t          meas_cccd[BCS_CONNECTION_MAX];
} bcs_env_t;

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */
/**@brief Add a Body Composition Service instance in ATT DB
 *
 * @param[in] Pointer to a Body Composition Service environment variable
 *
 * @return Result of service initialization.
 *****************************************************************************************
 */
sdk_err_t bcs_add_service(bcs_env_t *p_bcs_env);

/**@brief Send Body Composition Measurement indication.
 *
 * @param[in] conidx Connection index to sent indication to.
 * @param[in] p_meas Pointer to body composition measurement data.
 *
 * @return     BLE_SDK_SUCCESS on success, otherwise an error code
 */
sdk_err_t bcs_measurement_send(uint8_t conidx, const bcs_meas_t *p_meas);

/**@brief Check if indication is enabled.
 *
 * @param[in] conidx connection index to sent indication to.
 *
 * @return true if indication is enabled, false otherwise.
 */
bool bcs_is_indication_enabled(uint8_t conidx);

#endif

