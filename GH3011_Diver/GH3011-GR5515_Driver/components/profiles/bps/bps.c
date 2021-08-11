/**
 *****************************************************************************************
 *
 * @file bps.c
 *
 * @brief Blood Pressure Profile Sensor implementation.
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

/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "bps.h"
#include "ble_prf_types.h"
#include "ble_prf_utils.h"
#include "utility.h"

/*
 * ENUMERATIONS
 *****************************************************************************************
 */
/**@brief Blood Pressure Service Attributes Indexes. */
enum bps_attr_idx_t
{
    BPS_IDX_SVC,

    BPS_IDX_BP_MEAS_CHAR,
    BPS_IDX_BP_MEAS_VAL,
    BPS_IDX_BP_MEAS_IND_CFG,

    BPS_IDX_INTM_CUFF_PRESS_CHAR,
    BPS_IDX_INTM_CUFF_PRESS_VAL,
    BPS_IDX_INTM_CUFF_PRESS_NTF_CFG,

    BPS_IDX_BP_FEATURE_CHAR,
    BPS_IDX_BP_FEATURE_VAL,

    BPS_IDX_NB,
};

/**@brief Blood Pressure Measurement Flags bits. */
enum bps_meas_flag_t
{
    BPS_MEAS_BLOOD_PRESSURE_UNITS_FLAG_BIT = (0x01 << 0),    /**< Blood Pressure Units Flag bit. */
    BPS_MEAS_TIME_STAMP_FLAG_BIT           = (0x01 << 1),    /**< Time Stamp Flag bit. */
    BPS_MEAS_PULSE_RATE_FLAG_BIT           = (0x01 << 2),    /**< Pulse Rate Flag bit. */
    BPS_MEAS_USER_ID_FLAG_BIT              = (0x01 << 3),    /**< User ID Flag bit. */
    BPS_MEAS_MEASUREMENT_STATUS_FLAG_BIT   = (0x01 << 4),    /**< Measurement Status Flag bit. */
};

/*
 * STRUCTURES
 *****************************************************************************************
 */
/**@brief Blood Pressure Service environment variable. */
struct bps_env_t
{
    bps_init_t bps_init;                                /**< Blood Pressure Service initialization variables. */
    uint16_t   start_hdl;                               /**< Start handle of Blood Pressure Service. */
    uint16_t   ntf_ind_cfg[BPS_CONNECTION_MAX];         /**< Notification and indication configuration for each connections. */
};

 /*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static sdk_err_t bps_init(void);
static void      bps_read_att_cb(uint8_t conn_idx, const gatts_read_req_cb_t *p_param);
static void      bps_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param);
static void      bps_cccd_set_cb(uint8_t conn_idx, uint16_t handle, uint16_t cccd_value);
/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static struct bps_env_t s_bps_env;

/**@brief Full BPS Database Description which is used to add attributes into theATT database. */
static const attm_desc_t bps_attr_tab[BPS_IDX_NB] =
{
    // Blood Pressure Service Declaration
    [BPS_IDX_SVC] = {BLE_ATT_DECL_PRIMARY_SERVICE, READ_PERM_UNSEC, 0, 0},

    // Blood Pressure Measurement Characteristic Declaration
    [BPS_IDX_BP_MEAS_CHAR]    = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // Blood Pressure Measurement Characteristic Value
    [BPS_IDX_BP_MEAS_VAL]     = {BLE_ATT_CHAR_BLOOD_PRESSURE_MEAS,
#ifdef PTS_AUTO_TEST
                                 INDICATE_PERM_UNSEC,
#else
                                 INDICATE_PERM(NOAUTH),
#endif
                                 ATT_VAL_LOC_USER,
                                 BPS_BP_MEAS_MAX_LEN},
    // Blood Pressure Measurement Characteristic - Client Characteristic Configuration Descriptor
    [BPS_IDX_BP_MEAS_IND_CFG] = {BLE_ATT_DESC_CLIENT_CHAR_CFG,
#ifdef PTS_AUTO_TEST
                                 READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC,
#else
                                 READ_PERM(NOAUTH) | WRITE_REQ_PERM(NOAUTH),
#endif
                                 0,
                                 0},

    // Intermediate Cuff Pressure Characteristic Declaration
    [BPS_IDX_INTM_CUFF_PRESS_CHAR]    = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // Intermediate Cuff Pressure Characteristic Value
    [BPS_IDX_INTM_CUFF_PRESS_VAL]     = {BLE_ATT_CHAR_INTERMEDIATE_CUFF_PRESSURE,
#ifdef PTS_AUTO_TEST
                                         NOTIFY_PERM_UNSEC,
#else
                                         NOTIFY_PERM(NOAUTH),
#endif
                                         ATT_VAL_LOC_USER,
                                         BPS_BP_MEAS_MAX_LEN},
    // Intermediate Cuff Pressure Characteristic - Client Characteristic Configuration Descriptor
    [BPS_IDX_INTM_CUFF_PRESS_NTF_CFG] = {BLE_ATT_DESC_CLIENT_CHAR_CFG,
#ifdef PTS_AUTO_TEST
                                         READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC,
#else
                                         READ_PERM(NOAUTH) | WRITE_REQ_PERM(NOAUTH),
#endif
                                         0,
                                         0},

    // Blood Pressure Feature Characteristic Declaration
    [BPS_IDX_BP_FEATURE_CHAR] = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // Blood Pressure Feature Characteristic Value
    [BPS_IDX_BP_FEATURE_VAL]  = {BLE_ATT_CHAR_BLOOD_PRESSURE_FEATURE,
#ifdef PTS_AUTO_TEST
                                 READ_PERM_UNSEC,
#else
                                 READ_PERM(NOAUTH),
#endif
                                 ATT_VAL_LOC_USER,
                                 sizeof(uint16_t)},
};

/**@brief BPS interface required by profile manager. */
static ble_prf_manager_cbs_t bps_mgr_cbs =
{
    (prf_init_func_t)bps_init,
    NULL,
    NULL
};

/**@brief BPS GATT Server Callbacks. */
static gatts_prf_cbs_t bps_gatts_cbs =
{
    bps_read_att_cb,
    bps_write_att_cb,
    NULL,
    NULL,
    bps_cccd_set_cb,
};

/**@brief BPS Information. */
static const prf_server_info_t bps_prf_info =
{
    .max_connection_nb = BPS_CONNECTION_MAX,
    .manager_cbs       = &bps_mgr_cbs,
    .gatts_prf_cbs     = &bps_gatts_cbs
};

 /*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 * @brief Initialize Blood Pressure service and create DB in ATT.
 *
 * @retval ::Error code to know if profile initialization succeed or not.
 *****************************************************************************************
 */
static sdk_err_t bps_init(void)
{
    // The start hanlde must be set with PRF_INVALID_HANDLE to be allocated automatically by BLE Stack.
    uint16_t          start_hdl      = PRF_INVALID_HANDLE;
    const uint8_t     bps_svc_uuid[] = BLE_ATT_16_TO_16_ARRAY(BLE_ATT_SVC_BLOOD_PRESSURE);
    sdk_err_t         error_code;
    gatts_create_db_t gatts_db;

    memset(&gatts_db, 0, sizeof(gatts_db));

    gatts_db.shdl                 = &start_hdl;
    gatts_db.uuid                 = bps_svc_uuid;
    gatts_db.attr_tab_cfg         = (uint8_t*)&(s_bps_env.bps_init.char_mask);
    gatts_db.max_nb_attr          = BPS_IDX_NB;
    gatts_db.srvc_perm            = 0;
    gatts_db.attr_tab_type        = SERVICE_TABLE_TYPE_16;
    gatts_db.attr_tab.attr_tab_16 = bps_attr_tab;

    error_code = ble_gatts_srvc_db_create(&gatts_db);

    if (SDK_SUCCESS == error_code)
    {
        s_bps_env.start_hdl = *gatts_db.shdl;
    }

    return error_code;
}

/**
 *****************************************************************************************
 * @brief Handles reception of the attribute info request message.
 *
 * @param[in] conn_idx: Connection index
 * @param[in] p_param:  The parameters of the read request.
 *****************************************************************************************
 */
static void bps_read_att_cb(uint8_t conn_idx, const gatts_read_req_cb_t *p_param)
{
    gatts_read_cfm_t cfm;
    uint8_t          handle = p_param->handle;
    uint8_t          tab_index = prf_find_idx_by_handle(handle, s_bps_env.start_hdl,
                                                        BPS_IDX_NB,
                                                        (uint8_t *)&s_bps_env.bps_init.char_mask);

    cfm.handle = handle;
    cfm.status = BLE_SUCCESS;

    switch (tab_index)
    {
        case BPS_IDX_BP_MEAS_IND_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_bps_env.ntf_ind_cfg[conn_idx];
            break;

        case BPS_IDX_INTM_CUFF_PRESS_NTF_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_bps_env.ntf_ind_cfg[conn_idx];
            break;

        case BPS_IDX_BP_FEATURE_VAL:
            if (s_bps_env.bps_init.evt_handler)
            {
                s_bps_env.bps_init.evt_handler(conn_idx, BPS_EVT_READ_BL_PRESSURE_FEATURE);
            }
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_bps_env.bps_init.bp_feature;
            break;

        default:
            cfm.length = 0;
            cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
            break;
    }

    ble_gatts_read_cfm(conn_idx, &cfm);
}

/**
 *****************************************************************************************
 * @brief Handles reception of the write request.
 *
 * @param[in]: conn_idx: Connection index
 * @param[in]: p_param:  The parameters of the write request.
 *****************************************************************************************
 */
static void  bps_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param)
{
    uint16_t          handle     = p_param->handle;
    uint16_t          tab_index  = 0;
    uint16_t          cccd_value = 0;
    bps_evt_type_t    event;
    gatts_write_cfm_t cfm;

    tab_index  = prf_find_idx_by_handle(handle,
                                        s_bps_env.start_hdl,
                                        BPS_IDX_NB,
                                        (uint8_t *)&s_bps_env.bps_init.char_mask);
    cfm.handle = handle;
    cfm.status = BLE_SUCCESS;

    switch (tab_index)
    {
        case BPS_IDX_BP_MEAS_IND_CFG:
            cccd_value = le16toh(&p_param->value[0]);
            event      = ((PRF_CLI_START_IND == cccd_value) ?\
                           BPS_EVT_BP_MEAS_INDICATION_ENABLED :\
                           BPS_EVT_BP_MEAS_INDICATION_DISABLED);
            s_bps_env.ntf_ind_cfg[conn_idx] = cccd_value;
            break;

        case BPS_IDX_INTM_CUFF_PRESS_NTF_CFG:
            cccd_value = le16toh(&p_param->value[0]);
            event      = ((PRF_CLI_START_NTF == cccd_value) ?\
                           BPS_EVT_INTM_CUFF_PRESS_NTF_ENABLED :\
                           BPS_EVT_INTM_CUFF_PRESS_NTF_DISABLED);\
            s_bps_env.ntf_ind_cfg[conn_idx] = cccd_value;
            break;

        default:
            cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
            break;
    }

    if (BLE_ATT_ERR_INVALID_HANDLE != cfm.status && BPS_EVT_INVALID != event && s_bps_env.bps_init.evt_handler)
    {
        s_bps_env.bps_init.evt_handler(conn_idx, event);
    }
    ble_gatts_write_cfm(conn_idx, &cfm);
}

/**
 *****************************************************************************************
 * @brief Handles reception of the cccd recover request.
 *
 * @param[in]: conn_idx:   Connection index
 * @param[in]: handle:     The handle of cccd attribute.
 * @param[in]: cccd_value: The value of cccd attribute.
 *****************************************************************************************
 */
static void bps_cccd_set_cb(uint8_t conn_idx, uint16_t handle, uint16_t cccd_value)
{
    uint16_t          tab_index  = 0;
    bps_evt_type_t    event;

    if (!prf_is_cccd_value_valid(cccd_value))
    {
        return;
    }

    tab_index  = prf_find_idx_by_handle(handle,
                                        s_bps_env.start_hdl,
                                        BPS_IDX_NB,
                                        (uint8_t *)&s_bps_env.bps_init.char_mask);

    switch (tab_index)
    {
        case BPS_IDX_BP_MEAS_IND_CFG:
            event      = ((PRF_CLI_START_IND == cccd_value) ?\
                           BPS_EVT_BP_MEAS_INDICATION_ENABLED :\
                           BPS_EVT_BP_MEAS_INDICATION_DISABLED);
            s_bps_env.ntf_ind_cfg[conn_idx] = cccd_value;
            break;

        case BPS_IDX_INTM_CUFF_PRESS_NTF_CFG:
            event      = ((PRF_CLI_START_NTF == cccd_value) ?\
                           BPS_EVT_INTM_CUFF_PRESS_NTF_ENABLED :\
                           BPS_EVT_INTM_CUFF_PRESS_NTF_DISABLED);\
            s_bps_env.ntf_ind_cfg[conn_idx] = cccd_value;
            break;

        default:
            event = BPS_EVT_INVALID;
            break;
    }

    if (BPS_EVT_INVALID != event && s_bps_env.bps_init.evt_handler)
    {
        s_bps_env.bps_init.evt_handler(conn_idx, event);
    }
}

/**
 *****************************************************************************************
 * @brief Encode a Blood Pressure Measurement.
 *
 * @param[in]  p_bps_meas:       Measurement to be encoded.
 * @param[out] p_encoded_buffer: Buffer where the encoded data will be written.
 *
 * @retval ::Size of encoded data.
 *****************************************************************************************
 */
static uint16_t bps_measurement_encode(bps_meas_t *p_meas, uint8_t *p_encoded_buffer)
{
    uint8_t  flags          = 0;
    uint16_t length         = 1;
    uint16_t encoded_sfloat = 0;

    // Set measurement units flag
    if (p_meas->bl_unit_in_kpa)
    {
        flags |= BPS_MEAS_BLOOD_PRESSURE_UNITS_FLAG_BIT;
    }

    // Blood Pressure Measurement - Systolic
    encoded_sfloat = ((p_meas->systolic.exponent << 12) & 0xF000) |
                     ((p_meas->systolic.mantissa <<  0) & 0x0FFF);
    p_encoded_buffer[length++] = LO_U16(encoded_sfloat);
    p_encoded_buffer[length++] = HI_U16(encoded_sfloat);

    // Blood Pressure Measurement - Diastolic
    encoded_sfloat = ((p_meas->diastolic.exponent << 12) & 0xF000) |
                     ((p_meas->diastolic.mantissa <<  0) & 0x0FFF);
    p_encoded_buffer[length++] = LO_U16(encoded_sfloat);
    p_encoded_buffer[length++] = HI_U16(encoded_sfloat);

    // Blood Pressure Measurement - Mean Arterial Pressure
    encoded_sfloat = ((p_meas->mean_arterial_pr.exponent << 12) & 0xF000) |
                     ((p_meas->mean_arterial_pr.mantissa <<  0) & 0x0FFF);
    p_encoded_buffer[length++] = LO_U16(encoded_sfloat);
    p_encoded_buffer[length++] = HI_U16(encoded_sfloat);

    // Time Stamp field
    if (p_meas->time_stamp_present)
    {
        flags |= BPS_MEAS_TIME_STAMP_FLAG_BIT;
        p_encoded_buffer[length++] = LO_U16(p_meas->time_stamp.year);
        p_encoded_buffer[length++] = HI_U16(p_meas->time_stamp.year);
        p_encoded_buffer[length++] = p_meas->time_stamp.month;
        p_encoded_buffer[length++] = p_meas->time_stamp.day;
        p_encoded_buffer[length++] = p_meas->time_stamp.hour;
        p_encoded_buffer[length++] = p_meas->time_stamp.min;
        p_encoded_buffer[length++] = p_meas->time_stamp.sec;
    }

    // Pulse Rate
    if (p_meas->pulse_rate_present)
    {
        flags |= BPS_MEAS_PULSE_RATE_FLAG_BIT;
        encoded_sfloat = ((p_meas->pulse_rate.exponent << 12) & 0xF000) |
                         ((p_meas->pulse_rate.mantissa <<  0) & 0x0FFF);
        p_encoded_buffer[length++] = LO_U16(encoded_sfloat);
        p_encoded_buffer[length++] = HI_U16(encoded_sfloat);
    }

    // User ID
    if (p_meas->user_id_present)
    {
        flags |= BPS_MEAS_USER_ID_FLAG_BIT;
        p_encoded_buffer[length++] = p_meas->user_id;
    }

    // Measurement Status
    if (p_meas->meas_status_present)
    {
        flags |= BPS_MEAS_MEASUREMENT_STATUS_FLAG_BIT;
        p_encoded_buffer[length++] = LO_U16(p_meas->meas_status);
        p_encoded_buffer[length++] = HI_U16(p_meas->meas_status);
    }

    // Flags field
    p_encoded_buffer[0] = flags;

    return length;
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
sdk_err_t bps_measurement_send(uint8_t conidx, bps_meas_t *p_meas)
{
    sdk_err_t        error_code = SDK_ERR_IND_DISABLED;
    uint8_t          encoded_bps_meas[BPS_BP_MEAS_MAX_LEN] = {0};
    gatts_noti_ind_t bps_ind;

    uint16_t length = bps_measurement_encode(p_meas, encoded_bps_meas);

    if (PRF_CLI_START_IND == (s_bps_env.ntf_ind_cfg[conidx] & PRF_CLI_START_IND))
    {
        bps_ind.type   = BLE_GATT_INDICATION;
        bps_ind.handle = prf_find_handle_by_idx(BPS_IDX_BP_MEAS_VAL,
                                                s_bps_env.start_hdl,
                                                (uint8_t *)&s_bps_env.bps_init.char_mask);
        bps_ind.length = length;
        bps_ind.value  = encoded_bps_meas;

        error_code = ble_gatts_noti_ind(conidx, &bps_ind);
    }

    return error_code;
}

sdk_err_t bps_service_init(bps_init_t *p_bps_init)
{
    if (NULL == p_bps_init)
    {
        return SDK_ERR_POINTER_NULL;
    }

    memcpy(&s_bps_env.bps_init, p_bps_init, sizeof(bps_init_t));

    return ble_server_prf_add(&bps_prf_info);
}
