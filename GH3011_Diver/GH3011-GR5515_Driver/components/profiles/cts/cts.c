/**
 ****************************************************************************************
 *
 * @file cts.c
 *
 * @brief Current Time Service implementation.
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
 ****************************************************************************************
 */
#include "cts.h"
#include "ble_prf_types.h"
#include "ble_prf_utils.h"
#include "utility.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
/**@brief Current Time Service Attributes Indexes. */
enum
{
    // Current Time Service
    CTS_IDX_SVC,

    // Current Time
    CTS_IDX_CUR_TIME_CHAR,
    CTS_IDX_CUR_TIME_VAL,
    CTS_IDX_CUR_TIMR_NTF_CFG,

    // Local Time Information
    CTS_IDX_LOC_TIME_INFO_CHAR,
    CTS_IDX_LOC_TIME_INFO_VAL,

    // Reference Time Information
    CTS_IDX_REF_TIME_INFO_CHAR,
    CTS_IDX_REF_TIME_INFO_VAL,

    CTS_IDX_NB
};

/*
 * STRUCTURES
 *****************************************************************************************
 */
/**@brief Current Time Service environment variable. */
struct cts_env_t
{
    cts_init_t     cts_init;                              /**< Current Time Service initialization variables. */
    uint16_t       start_hdl;                             /**< Current Time Service start handle. */
    uint16_t       cur_time_ntf_cfg[CTS_CONNECTION_MAX];  /**< The configuration of Current Time Notification which is configured by the peer devices. */
};

/*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static sdk_err_t   cts_init(void);
static void        cts_read_att_cb(uint8_t conidx, const gatts_read_req_cb_t *p_param);
static void        cts_write_att_cb(uint8_t conidx, const gatts_write_req_cb_t *p_param);
static void        cts_cccd_set_cb(uint8_t conn_idx, uint16_t handle, uint16_t cccd_value);
static void        cts_gatts_ntf_ind_cb(uint8_t conn_idx, uint8_t status, const ble_gatts_ntf_ind_t *p_ntf_ind);
static void        cts_cur_time_read_handler(gatts_read_cfm_t *p_cfm, uint8_t *p_encode_buffer);
static void        cts_loc_time_info_read_handler(gatts_read_cfm_t *p_cfm, uint8_t *p_encode_buffer);
static void        cts_ref_time_info_read_handler(gatts_read_cfm_t *p_cfm, uint8_t *p_encode_buffer);
static void        cts_cur_time_write_handler(gatts_write_cfm_t *p_cfm, cts_evt_t *p_evt);
static void        cts_loc_time_info_write_handler(gatts_write_cfm_t *p_cfm, cts_evt_t *p_evt);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static struct cts_env_t s_cts_env;

/**@brief Full CTS Database Description - Used to add attributes into the database. */
static const attm_desc_t cts_attr_tab[CTS_IDX_NB] =
{
    // CTS Service Declaration
    [CTS_IDX_SVC] = {BLE_ATT_DECL_PRIMARY_SERVICE, READ_PERM_UNSEC, 0, 0},

    // Current Time Characteristic Declaration
    [CTS_IDX_CUR_TIME_CHAR]    = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // Current Time Characteristic Declaration value
    [CTS_IDX_CUR_TIME_VAL]     = {BLE_ATT_CHAR_CT_TIME,
                                  READ_PERM_UNSEC | NOTIFY_PERM_UNSEC | WRITE_REQ_PERM_UNSEC,
                                  ATT_VAL_LOC_USER,
                                  CTS_CUR_TIME_VAL_LEN},
    // Current Time Characteristic Declaration  - Client Characteristic Configuration Descriptor
    [CTS_IDX_CUR_TIMR_NTF_CFG] = {BLE_ATT_DESC_CLIENT_CHAR_CFG, READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC, 0, 0},

    // Local Time Information Characteristic Declaration
    [CTS_IDX_LOC_TIME_INFO_CHAR] = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // Local Time Information Characteristic Value
    [CTS_IDX_LOC_TIME_INFO_VAL]  = {BLE_ATT_CHAR_LOCAL_TIME_INFO,
                                    READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC,
                                    ATT_VAL_LOC_USER,
                                    CTS_LOC_TIME_INFO_VAL_LEN},

    // Reference Time Information Characteristic Declaration
    [CTS_IDX_REF_TIME_INFO_CHAR] = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // Reference Time Information Characteristic Value
    [CTS_IDX_REF_TIME_INFO_VAL]  = {BLE_ATT_CHAR_REFERENCE_TIME_INFO,
                                    READ_PERM_UNSEC,
                                    ATT_VAL_LOC_USER,
                                    CTS_REF_TIME_INFO_VAL_LEN},
};

/**@brief CTS Task interface required by profile manager. */
static ble_prf_manager_cbs_t cts_tack_cbs =
{
    (prf_init_func_t) cts_init,
    NULL,
    NULL
};

/**@brief CTS Task Callbacks. */
static gatts_prf_cbs_t cts_cb_func =
{
    cts_read_att_cb,
    cts_write_att_cb,
    NULL,
    cts_gatts_ntf_ind_cb,
    cts_cccd_set_cb
};

/**@brief CTS Information. */
static const prf_server_info_t cts_prf_info =
{
    .max_connection_nb = CTS_CONNECTION_MAX,
    .manager_cbs       = &cts_tack_cbs,
    .gatts_prf_cbs     = &cts_cb_func
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 * @brief Initialize Current Time service and create db in att
 *
 * @return Error code to know if profile initialization succeed or not.
 *****************************************************************************************
 */
static sdk_err_t cts_init(void)
{
    // The start hanlde must be set with PRF_INVALID_HANDLE to be allocated automatically by BLE Stack.
    uint16_t          start_hdl      = PRF_INVALID_HANDLE;
    const uint8_t     cts_svc_uuid[] = BLE_ATT_16_TO_16_ARRAY(BLE_ATT_SVC_CURRENT_TIME);
    sdk_err_t         error_code;
    gatts_create_db_t gatts_db;

    memset(&gatts_db, 0, sizeof(gatts_db));

    gatts_db.shdl                 = &start_hdl;
    gatts_db.uuid                 = cts_svc_uuid;
    gatts_db.attr_tab_cfg         = (uint8_t *)&(s_cts_env.cts_init.char_mask);
    gatts_db.max_nb_attr          = CTS_IDX_NB;
    gatts_db.srvc_perm            = 0;
    gatts_db.attr_tab_type        = SERVICE_TABLE_TYPE_16;
    gatts_db.attr_tab.attr_tab_16 = cts_attr_tab;

    error_code = ble_gatts_srvc_db_create(&gatts_db);

    if (SDK_SUCCESS == error_code)
    {
        s_cts_env.start_hdl = *gatts_db.shdl;
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
static void   cts_read_att_cb(uint8_t conn_idx, const gatts_read_req_cb_t *p_param)
{
    gatts_read_cfm_t  cfm;
    uint8_t           handle    = p_param->handle;
    uint8_t           tab_index = prf_find_idx_by_handle(handle,
                                  s_cts_env.start_hdl,
                                  CTS_IDX_NB,
                                  (uint8_t *)&s_cts_env.cts_init.char_mask);
    cfm.handle = handle;
    cfm.status = BLE_SUCCESS;

    switch (tab_index)
    {
        case CTS_IDX_CUR_TIME_VAL:
        {
            uint8_t encoded_buffer[CTS_CUR_TIME_VAL_LEN];
            cts_cur_time_read_handler(&cfm, encoded_buffer);
            break;
        }

        case CTS_IDX_CUR_TIMR_NTF_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_cts_env.cur_time_ntf_cfg[conn_idx];
            break;

        case CTS_IDX_LOC_TIME_INFO_VAL:
        {
            uint8_t encoded_buffer[CTS_LOC_TIME_INFO_VAL_LEN];
            cts_loc_time_info_read_handler(&cfm, encoded_buffer);
            break;
        }

        case CTS_IDX_REF_TIME_INFO_VAL:
        {
            uint8_t encoded_buffer[CTS_REF_TIME_INFO_VAL_LEN];
            cts_ref_time_info_read_handler(&cfm, encoded_buffer);
            break;
        }

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
static void   cts_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param)
{
    uint16_t          handle      = p_param->handle;
    uint16_t          tab_index   = 0;
    uint16_t          cccd_value  = 0;
    cts_evt_t         event;
    gatts_write_cfm_t cfm;

    tab_index  = prf_find_idx_by_handle(handle,
                                        s_cts_env.start_hdl,
                                        CTS_IDX_NB,
                                        (uint8_t *)&s_cts_env.cts_init.char_mask);
    cfm.handle     = handle;
    cfm.status     = BLE_SUCCESS;
    event.evt_type = CTS_EVT_INVALID;
    event.conn_idx = conn_idx;

    switch (tab_index)
    {
        case CTS_IDX_CUR_TIME_VAL:
            event.evt_type = CTS_EVT_CUR_TIME_SET_BY_PEER;
            event.p_data   = p_param->value;
            event.length   = p_param->length;
            cts_cur_time_write_handler(&cfm, &event);
            memcpy(&event.cur_time, &s_cts_env.cts_init.cur_time, sizeof(cts_cur_time_t));
            break;

        case CTS_IDX_CUR_TIMR_NTF_CFG:
            cccd_value     = le16toh(&p_param->value[0]);
            event.evt_type = ((PRF_CLI_START_NTF == cccd_value) ? \
                              CTS_EVT_CUR_TIME_NOTIFICATION_ENABLED : \
                              CTS_EVT_CUR_TIME_NOTIFICATION_DISABLED);
            s_cts_env.cur_time_ntf_cfg[conn_idx] = cccd_value;
            break;

        case CTS_IDX_LOC_TIME_INFO_VAL:
            event.evt_type = CTS_EVT_LOC_TIME_INFO_SET_BY_PEER;
            event.p_data   = p_param->value;
            event.length   = p_param->length;
            cts_loc_time_info_write_handler(&cfm, &event);
            memcpy(&event.loc_time_info, &s_cts_env.cts_init.loc_time_info, sizeof(cts_loc_time_info_t));
            break;

        default:
            cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
            break;
    }

    ble_gatts_write_cfm(conn_idx, &cfm);

    if (BLE_ATT_ERR_INVALID_HANDLE != cfm.status && CTS_EVT_INVALID != event.evt_type && s_cts_env.cts_init.evt_handler)
    {
        s_cts_env.cts_init.evt_handler(&event);
    }
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
static void cts_cccd_set_cb(uint8_t conn_idx, uint16_t handle, uint16_t cccd_value)
{
    uint16_t          tab_index   = 0;
    cts_evt_t         event;

    if (!prf_is_cccd_value_valid(cccd_value))
    {
        return;
    }

    tab_index  = prf_find_idx_by_handle(handle,
                                        s_cts_env.start_hdl,
                                        CTS_IDX_NB,
                                        (uint8_t *)&s_cts_env.cts_init.char_mask);

    event.evt_type = CTS_EVT_INVALID;
    event.conn_idx = conn_idx;

    switch (tab_index)
    {
        case CTS_IDX_CUR_TIMR_NTF_CFG:
            event.evt_type = ((PRF_CLI_START_NTF == cccd_value) ? \
                              CTS_EVT_CUR_TIME_NOTIFICATION_ENABLED : \
                              CTS_EVT_CUR_TIME_NOTIFICATION_DISABLED);
            s_cts_env.cur_time_ntf_cfg[conn_idx] = cccd_value;
            break;

        default:
            break;
    }

    if (CTS_EVT_INVALID != event.evt_type && s_cts_env.cts_init.evt_handler)
    {
        s_cts_env.cts_init.evt_handler(&event);
    }
}

/**
 *****************************************************************************************
 * @brief Handles reception of the complete event.
 *
 * @param[in] conn_idx: Connection index.
 * @param[in] p_param:  Pointer to the parameters of the complete event.
 *****************************************************************************************
 */
static void cts_gatts_ntf_ind_cb(uint8_t conn_idx, uint8_t status, const ble_gatts_ntf_ind_t *p_ntf_ind)
{
    if (BLE_GATT_NOTIFICATION == p_ntf_ind->type)
    {
        s_cts_env.cts_init.cur_time.adjust_reason = CTS_AR_NO_CHANGE;
    }
}

/**
 *****************************************************************************************
 * @brief Encode a Current Time.
 *
 * @param[in]  p_cur_time:     Pointer to Current Time value to be encoded.
 * @param[out] p_encoded_data: Pointer to encoded data will be written.
 *****************************************************************************************
 */
static void cts_cur_time_encode(const cts_cur_time_t *p_cur_time, uint8_t *p_encoded_data)
{
    prf_pack_date_time(p_encoded_data, &p_cur_time->day_date_time.date_time);

    p_encoded_data[7] = p_cur_time->day_date_time.day_of_week;
    p_encoded_data[8] = p_cur_time->day_date_time.fractions_256;
    p_encoded_data[9] = p_cur_time->adjust_reason;
}

/**
 *****************************************************************************************
 * @brief Decode for a Current Time.
 *
 * @param[in]  p_data: Pointer to data to be decoded.
 * @param[out] p_cur_time:     Pointer to Current Time.
 *****************************************************************************************
 */
static void cts_cur_time_decode(const uint8_t *p_data, cts_cur_time_t *p_cur_time)
{
    prf_unpack_date_time(p_data, &p_cur_time->day_date_time.date_time);

    p_cur_time->day_date_time.day_of_week   = p_data[7];
    p_cur_time->day_date_time.fractions_256 = p_data[8];
    p_cur_time->adjust_reason               = p_data[9];
}

/**
 *****************************************************************************************
 * @brief Handle Current Time read event.
 *
 * @param[out] p_cfm:           Pointer to GATT read attribute result description.
 * @param[out] p_encode_buffer: Pointer to encoded data will be written.
 *****************************************************************************************
 */
static void cts_cur_time_read_handler(gatts_read_cfm_t *p_cfm, uint8_t *p_encode_buffer)
{
    s_cts_env.cts_init.cur_time.adjust_reason = CTS_AR_NO_CHANGE;

    cts_cur_time_encode(&s_cts_env.cts_init.cur_time, p_encode_buffer);

    p_cfm->length = CTS_CUR_TIME_VAL_LEN;
    p_cfm->value  = p_encode_buffer;
}

/**
 *****************************************************************************************
 * @brief Handle Local Time Information read event.
 *
 * @param[out] p_cfm:           Pointer to GATT read attribute result description.
 * @param[out] p_encode_buffer: Pointer to encoded data will be written.
 *****************************************************************************************
 */
static void cts_loc_time_info_read_handler(gatts_read_cfm_t *p_cfm, uint8_t *p_encode_buffer)
{
    p_encode_buffer[0] = s_cts_env.cts_init.loc_time_info.time_zone;
    p_encode_buffer[1] = s_cts_env.cts_init.loc_time_info.dst_offset;

    p_cfm->value  = p_encode_buffer;
    p_cfm->length = CTS_LOC_TIME_INFO_VAL_LEN;
}

/**
 *****************************************************************************************
 * @brief Handle Reference Time Information read event.
 *
 * @param[out] p_cfm:           Pointer to GATT read attribute result description.
 * @param[out] p_encode_buffer: Pointer to encoded data will be written.
 *****************************************************************************************
 */
static void cts_ref_time_info_read_handler(gatts_read_cfm_t *p_cfm, uint8_t *p_encode_buffer)
{
    p_encode_buffer[0] = s_cts_env.cts_init.ref_time_info.source;
    p_encode_buffer[1] = s_cts_env.cts_init.ref_time_info.accuracy;
    p_encode_buffer[2] = s_cts_env.cts_init.ref_time_info.days_since_update;
    p_encode_buffer[3] = s_cts_env.cts_init.ref_time_info.hours_since_update;

    p_cfm->value  = p_encode_buffer;
    p_cfm->length = CTS_REF_TIME_INFO_VAL_LEN;
}

/**
 *****************************************************************************************
 * @brief Handle Current Time write event.
 *
 * @param[out] p_cfm: Pointer to GATT write attribute result description.
 * @param[in]  p_evt: Pointer to CTS event.
 *****************************************************************************************
 */
static void cts_cur_time_write_handler(gatts_write_cfm_t *p_cfm, cts_evt_t *p_evt)
{
    cts_cur_time_t  cur_time_set = {0};

    cts_cur_time_decode(p_evt->p_data, &cur_time_set);

    if ((CTS_TIME_YEAR_VALID_VAL_MIN <= cur_time_set.day_date_time.date_time.year) && \
            (CTS_TIME_YEAR_VALID_VAL_MIN >= cur_time_set.day_date_time.date_time.year))
    {
        s_cts_env.cts_init.cur_time.day_date_time.date_time.year = cur_time_set.day_date_time.date_time.year;
    }
    else
    {
        if (0 == cur_time_set.day_date_time.date_time.year)
        {
            s_cts_env.cts_init.cur_time.day_date_time.date_time.year = cur_time_set.day_date_time.date_time.year;
        }
        else
        {
            p_cfm->status = CTS_ERROR_FIELDS_IGNORED;
        }
    }

    if (12 >= cur_time_set.day_date_time.date_time.month)
    {
        s_cts_env.cts_init.cur_time.day_date_time.date_time.month = cur_time_set.day_date_time.date_time.month;
    }
    else
    {
        p_cfm->status = CTS_ERROR_FIELDS_IGNORED;
    }

    if (31 >= cur_time_set.day_date_time.date_time.day)
    {
        s_cts_env.cts_init.cur_time.day_date_time.date_time.day = cur_time_set.day_date_time.date_time.day;
    }
    else
    {
        p_cfm->status = CTS_ERROR_FIELDS_IGNORED;
    }

    if (23 >= cur_time_set.day_date_time.date_time.hour)
    {
        s_cts_env.cts_init.cur_time.day_date_time.date_time.hour = cur_time_set.day_date_time.date_time.hour;
    }
    else
    {
        p_cfm->status = CTS_ERROR_FIELDS_IGNORED;
    }

    if (59 >= cur_time_set.day_date_time.date_time.min)
    {
        s_cts_env.cts_init.cur_time.day_date_time.date_time.min = cur_time_set.day_date_time.date_time.min;
    }
    else
    {
        p_cfm->status = CTS_ERROR_FIELDS_IGNORED;
    }

    if (59 >= cur_time_set.day_date_time.date_time.sec)
    {
        s_cts_env.cts_init.cur_time.day_date_time.date_time.sec = cur_time_set.day_date_time.date_time.sec;
    }
    else
    {
        p_cfm->status = CTS_ERROR_FIELDS_IGNORED;
    }

    if (7 >= cur_time_set.day_date_time.day_of_week)
    {
        s_cts_env.cts_init.cur_time.day_date_time.day_of_week = cur_time_set.day_date_time.day_of_week;
    }
    else
    {
        p_cfm->status = CTS_ERROR_FIELDS_IGNORED;
    }

    if (0x0f >= cur_time_set.adjust_reason)
    {
        s_cts_env.cts_init.cur_time.adjust_reason = cur_time_set.adjust_reason;
    }
    else
    {
        p_cfm->status = CTS_ERROR_FIELDS_IGNORED;
    }
}

/**
 *****************************************************************************************
 * @brief Handle Local Time Information write event.
 *
 * @param[out] p_cfm: Pointer to GATT write attribute result description.
 * @param[in]  p_evt: Pointer to CTS event.
 *****************************************************************************************
 */
static void cts_loc_time_info_write_handler(gatts_write_cfm_t *p_cfm, cts_evt_t *p_evt)
{
    cts_dst_offset_t dst_offset = (cts_dst_offset_t)p_evt->p_data[1];

    if (CTS_TIME_ZONE_OFFSET_MIN <= (int8_t)p_evt->p_data[0] && CTS_TIME_ZONE_OFFSET_MAX >= (int8_t)p_evt->p_data[0])
    {
        s_cts_env.cts_init.loc_time_info.time_zone  = p_evt->p_data[0];
    }
    else
    {
        p_cfm->status = CTS_ERROR_FIELDS_IGNORED;
    }

    if (CTS_DST_OFFSET_DOUB_DAYLIGHT_TIME >= dst_offset)
    {
        s_cts_env.cts_init.loc_time_info.dst_offset = dst_offset;
    }
    else
    {
        p_cfm->status = CTS_ERROR_FIELDS_IGNORED;
    }

    s_cts_env.cts_init.loc_time_info.dst_offset = (cts_dst_offset_t)p_evt->p_data[1];
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
sdk_err_t cts_cur_time_send(uint8_t conn_idx, cts_cur_time_t *p_cur_time)
{
    sdk_err_t        error_code = SDK_ERR_NTF_DISABLED;
    uint8_t          encoded_cur_time[CTS_CUR_TIME_VAL_LEN];
    gatts_noti_ind_t ct_ntf;

    cts_cur_time_encode(p_cur_time, encoded_cur_time);

    if (PRF_CLI_START_NTF == s_cts_env.cur_time_ntf_cfg[conn_idx])
    {
        ct_ntf.type   = BLE_GATT_NOTIFICATION;
        ct_ntf.handle = prf_find_handle_by_idx(CTS_IDX_CUR_TIME_VAL,
                                               s_cts_env.start_hdl,
                                               (uint8_t *)&s_cts_env.cts_init.char_mask);
        ct_ntf.length = CTS_CUR_TIME_VAL_LEN;
        ct_ntf.value  = encoded_cur_time;
        error_code    = ble_gatts_noti_ind(conn_idx, &ct_ntf);
    }

    return error_code;
}

void cts_exact_time_get(cts_exact_time_256_t *p_cts_exact_time)
{
    memcpy(p_cts_exact_time, &s_cts_env.cts_init.cur_time.day_date_time, sizeof(cts_exact_time_256_t));
}

void cts_exact_time_update(cts_exact_time_256_t *p_cts_exact_time)
{
    memcpy(&s_cts_env.cts_init.cur_time.day_date_time, p_cts_exact_time, sizeof(cts_exact_time_256_t));
}

void cts_cur_time_adjust(cts_adj_info_t *p_adj_info)
{
    if (CTS_AR_MAUAL_TIME_UPDATE & p_adj_info->adjust_reason)
    {
        memcpy(&s_cts_env.cts_init.cur_time.day_date_time, &p_adj_info->day_date_time, sizeof(cts_exact_time_256_t));
    }

    if (CTS_AR_EXT_REF_TIME_UPDATE & p_adj_info->adjust_reason)
    {
        memcpy(&s_cts_env.cts_init.ref_time_info, &p_adj_info->ref_time_info, sizeof(cts_ref_time_info_t));
    }

    if (CTS_AR_TIME_ZONE_CHANGE & p_adj_info->adjust_reason)
    {
        s_cts_env.cts_init.loc_time_info.time_zone = p_adj_info->loc_time_info.time_zone;
    }

    if (CTS_AR_DST_CHANGE & p_adj_info->adjust_reason)
    {
        s_cts_env.cts_init.loc_time_info.dst_offset = p_adj_info->loc_time_info.dst_offset;
    }

    s_cts_env.cts_init.cur_time.adjust_reason = p_adj_info->adjust_reason;

    cts_cur_time_send(0, &s_cts_env.cts_init.cur_time);
}

sdk_err_t cts_service_init(cts_init_t *p_cts_init)
{
    if (NULL == p_cts_init)
    {
        return SDK_ERR_POINTER_NULL;
    }

    memset(&s_cts_env, 0, sizeof(s_cts_env));
    memcpy(&s_cts_env.cts_init, p_cts_init, sizeof(cts_init_t));

    return ble_server_prf_add(&cts_prf_info);
}


