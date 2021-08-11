/**
 ****************************************************************************************
 *
 * @file cts_c.c
 *
 * @brief Current Time Service Client implementation.
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
#include "cts_c.h"
#include "ble_prf_utils.h"
#include "utility.h"
#include <string.h>

/*
 * STRUCT DEFINE
 *****************************************************************************************
 */
/**@brief Current Time Service Client environment variable. */
struct cts_c_env_t
{
    cts_c_handles_t      handles;            /**< Handles of CTS characteristics which will be got for peer. */
    cts_c_evt_handler_t  evt_handler;        /**< Handler of CTS Client event  */
    uint8_t              prf_id;             /**< CTS Client profile id. */
};

/*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static void cts_c_att_read_cb(uint8_t conn_idx, uint8_t status, const ble_gattc_read_rsp_t *p_read_rsp);
static void cts_c_att_write_cb(uint8_t conn_idx, uint8_t status, uint16_t handle);
static void cts_c_att_ntf_ind_cb(uint8_t conn_idx, const ble_gattc_ntf_ind_t *p_ntf_ind);
static void cts_c_srvc_browse_cb(uint8_t conn_idx, uint8_t status, const ble_gattc_browse_srvc_t *p_browse_srvc);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static struct cts_c_env_t s_cts_c_env;      /**< Current Time Service Client environment variable. */

/**@brief Current Time Service Client interface required by profile manager. */
static ble_prf_manager_cbs_t cts_c_mgr_cbs =
{
    NULL,
    NULL,
    NULL
};

/**@brief Current Time Service GATT Client Callbacks. */
static gattc_prf_cbs_t cts_c_gattc_cbs =
{
    NULL,
    NULL,
    NULL,
    NULL,
    cts_c_att_read_cb,
    cts_c_att_write_cb,
    cts_c_att_ntf_ind_cb,
    cts_c_srvc_browse_cb,
    NULL,
};

/**@brief Current Time Service Client Information. */
static const prf_client_info_t cts_c_prf_info =
{
    .max_connection_nb = CTS_C_CONNECTION_MAX,
    .manager_cbs       = &cts_c_mgr_cbs,
    .gattc_prf_cbs     = &cts_c_gattc_cbs
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 * @brief Excute Current Time Service Client event handler.
 *
 * @param[in] p_evt: Pointer to Current Time Service Client event structure.
 *****************************************************************************************
 */
static void cts_c_evt_handler_excute(cts_c_evt_t *p_evt)
{
    if (NULL != s_cts_c_env.evt_handler && CTS_C_EVT_INVALID != p_evt->evt_type)
    {
        s_cts_c_env.evt_handler(p_evt);
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
static void cts_c_cur_time_encode(const cts_c_cur_time_t *p_cur_time, uint8_t *p_encoded_data)
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
 * @param[in]  p_data:     Pointer to data to be decoded.
 * @param[out] p_cur_time: Pointer to Current Time.
 *****************************************************************************************
 */
static void cts_c_cur_time_decode(const uint8_t *p_data, cts_c_cur_time_t *p_cur_time)
{
    prf_unpack_date_time(p_data, &p_cur_time->day_date_time.date_time);

    p_cur_time->day_date_time.day_of_week   = p_data[7];
    p_cur_time->day_date_time.fractions_256 = p_data[8];
    p_cur_time->adjust_reason               = p_data[9];
}

/**
 *****************************************************************************************
 * @brief Decode for a Reference Time Information.
 *
 * @param[in]  p_data:          Pointer to data to be decoded.
 * @param[out] p_ref_time_info: Pointer to Reference Time Information.
 *****************************************************************************************
 */
static void cts_c_ref_time_info_decode(const uint8_t *p_data, cts_c_ref_time_info_t *p_ref_time_info)
{
    p_ref_time_info->source             = (cts_c_ref_time_source_t)p_data[0];
    p_ref_time_info->accuracy           = p_data[1];
    p_ref_time_info->days_since_update  = p_data[2];
    p_ref_time_info->hours_since_update = p_data[3];
}

/**
 *****************************************************************************************
 * @brief Check Current Time value is valid or not
 *
 * @param[out] p_cur_time: Pointer to Current Time.
 *****************************************************************************************
 */
static bool cts_c_cur_time_valid_check(cts_c_cur_time_t *p_cur_time)
{
    if ((p_cur_time->day_date_time.date_time.year > CTS_C_TIME_YEAR_VALID_VAL_MAX) || \
        ((p_cur_time->day_date_time.date_time.year < CTS_C_TIME_YEAR_VALID_VAL_MIN) && \
         (CTS_C_TIME_Y_M_D_UNKNOWN != p_cur_time->day_date_time.date_time.year)))
    {
        return false;
    }

    if (p_cur_time->day_date_time.date_time.month > 12)
    {
        return false;
    }

    if (p_cur_time->day_date_time.date_time.day > 31)
    {
        return false;
    }

    if (p_cur_time->day_date_time.date_time.hour > 23)
    {
        return false;
    }

    if (p_cur_time->day_date_time.date_time.min > 59)
    {
        return false;
    }

    if (p_cur_time->day_date_time.date_time.sec > 59)
    {
        return false;
    }

    if (p_cur_time->day_date_time.day_of_week > CTS_C_WEEK_SUNDAY)
    {
        return false;
    }

    if (p_cur_time->adjust_reason > 0x0f)
    {
        return false;
    }
    return true;
}

/**
 *****************************************************************************************
 * @brief Check Local Time Information value is valid or not
 *
 * @param[out] p_loc_time_info: Pointer to Local Time Information.
 *****************************************************************************************
 */
static bool cts_c_loc_time_info_valid_check(cts_c_loc_time_info_t *p_loc_time_info)
{
    if ((p_loc_time_info->time_zone < CTS_C_TIME_ZONE_OFFSET_MIN) || \
         (p_loc_time_info->time_zone > CTS_C_TIME_ZONE_OFFSET_MAX))
    {
        return false;
    }

    if ((CTS_C_DST_OFFSET_STANDAR_TIME != p_loc_time_info->dst_offset) && \
        (CTS_C_DST_OFFSET_HALF_HOUR != p_loc_time_info->dst_offset) && \
        (CTS_C_DST_OFFSET_DAYLIGHT_TIME != p_loc_time_info->dst_offset) && \
        (CTS_C_DST_OFFSET_DOUB_DAYLIGHT_TIME != p_loc_time_info->dst_offset))
    {
        return false;
    }

    return true;
}

/**
 *****************************************************************************************
 * @brief Check Reference Time Information value is valid or not
 *
 * @param[out] p_ref_time_info: Pointer to Local Time Information.
 *****************************************************************************************
 */
static bool cts_c_ref_time_info_valid_check(cts_c_ref_time_info_t *p_ref_time_info)
{
    if (p_ref_time_info->source > CTS_C_REF_TIME_SRC_CELLUAR_NET)
    {
        return false;
    }

    return true;
}

/**
 *****************************************************************************************
 * @brief This callback function will be called when receiving read response.
 *
 * @param[in] conn_idx:   The connection index.
 * @param[in] status:     The status of GATTC operation.
 * @param[in] p_read_rsp: The information of read response.
 *****************************************************************************************
 */
static void cts_c_att_read_cb(uint8_t conn_idx, uint8_t status, const ble_gattc_read_rsp_t *p_read_rsp)
{
    cts_c_evt_t cts_c_evt;

    cts_c_evt.conn_idx = conn_idx;
    cts_c_evt.evt_type = CTS_C_EVT_INVALID;

    if (BLE_SUCCESS != status)
    {
        return;
    }

    if (p_read_rsp->vals[0].handle == s_cts_c_env.handles.cts_cur_time_handle)
    {
        cts_c_cur_time_decode(p_read_rsp->vals[0].p_value, &cts_c_evt.value.cur_time);

        if (cts_c_cur_time_valid_check(&cts_c_evt.value.cur_time))
        {
            cts_c_evt.evt_type = CTS_C_EVT_VALID_CUR_TIME_REC;
        }
        else
        {
            cts_c_evt.evt_type = CTS_C_EVT_INVALID_CUR_TIME_REC;
        }
    }
    else if (p_read_rsp->vals[0].handle == s_cts_c_env.handles.cts_loc_time_info_handle)
    {
        cts_c_evt.value.loc_time_info.time_zone  = p_read_rsp->vals[0].p_value[0];
        cts_c_evt.value.loc_time_info.dst_offset = (cts_c_dst_offset_t)p_read_rsp->vals[0].p_value[1];

        if (cts_c_loc_time_info_valid_check(&cts_c_evt.value.loc_time_info))
        {
            cts_c_evt.evt_type = CTS_C_EVT_VALID_LOC_TIME_INFO_REC;
        }
        else
        {
            cts_c_evt.evt_type = CTS_C_EVT_INVALID_LOC_TIME_INFO_REC;
        }
    }
    else if (p_read_rsp->vals[0].handle == s_cts_c_env.handles.cts_ref_time_info_handle)
    {
        cts_c_ref_time_info_decode(p_read_rsp->vals[0].p_value, &cts_c_evt.value.ref_time_info);

        if (cts_c_ref_time_info_valid_check(&cts_c_evt.value.ref_time_info))
        {
            cts_c_evt.evt_type = CTS_C_EVT_VALID_REF_TIME_INFO_REC;
        }
        else
        {
            cts_c_evt.evt_type = CTS_C_EVT_INVALID_REF_TIME_INFO_REC;
        }
    }

    cts_c_evt_handler_excute(&cts_c_evt);
}

/**
 *****************************************************************************************
 * @brief This callback function will be called when receiving read response.
 *
 * @param[in] conn_idx:   The connection index.
 * @param[in] status:     The status of GATTC operation.
 * @param[in] handle:     The handle of attribute.
 *****************************************************************************************
 */
static void cts_c_att_write_cb(uint8_t conn_idx, uint8_t status, uint16_t handle)
{
    cts_c_evt_t cts_c_evt;

    cts_c_evt.conn_idx = conn_idx;
    cts_c_evt.evt_type = CTS_C_EVT_INVALID;

    if (handle == s_cts_c_env.handles.cts_cur_time_cccd_handle)
    {
        cts_c_evt.evt_type = (BLE_SUCCESS == status) ? \
                              CTS_C_EVT_CUR_TIME_NTF_SET_SUCCESS :
                              CTS_C_EVT_WRITE_OP_ERR;
    }
    else if (handle == s_cts_c_env.handles.cts_cur_time_handle)
    {
        cts_c_evt.evt_type = (BLE_SUCCESS == status) ? \
                              CTS_C_EVT_CUR_TIME_SET_SUCCESS :
                              CTS_C_EVT_WRITE_OP_ERR;
    }
    else if (handle == s_cts_c_env.handles.cts_loc_time_info_handle)
    {
        cts_c_evt.evt_type = (BLE_SUCCESS == status) ? \
                              CTS_C_EVT_LOC_TIME_INFO_SET_SUCCESS :
                              CTS_C_EVT_WRITE_OP_ERR;
    }

    cts_c_evt_handler_excute(&cts_c_evt);
}

/**
 *****************************************************************************************
 * @brief This callback function will be called when receiving notification or indication.
 *
 * @param[in] conn_idx:  The connection index.
 * @param[in] status:    The status of GATTC operation.
 * @param[in] p_ntf_ind: The information of notification or indication.
 *****************************************************************************************
 */
static void cts_c_att_ntf_ind_cb(uint8_t conn_idx, const ble_gattc_ntf_ind_t *p_ntf_ind)
{
    cts_c_evt_t cts_c_evt;

    cts_c_evt.conn_idx = conn_idx;
    cts_c_evt.evt_type = CTS_C_EVT_INVALID;

    if (p_ntf_ind->handle == s_cts_c_env.handles.cts_cur_time_handle)
    {
        cts_c_cur_time_decode(p_ntf_ind->p_value, &cts_c_evt.value.cur_time);

        if (cts_c_cur_time_valid_check(&cts_c_evt.value.cur_time))
        {
            cts_c_evt.evt_type = CTS_C_EVT_VALID_CUR_TIME_REC;
        }
        else
        {
            cts_c_evt.evt_type = CTS_C_EVT_INVALID_CUR_TIME_REC;
        }
    }

    cts_c_evt_handler_excute(&cts_c_evt);
}

/**
 *****************************************************************************************
 * @brief This callback function will be called when receiving browse service indication.
 *
 * @param[in] conn_idx:      The connection index.
 * @param[in] status:        The status of GATTC operation.
 * @param[in] p_browse_srvc: The information of service browse.
 *****************************************************************************************
 */
static void cts_c_srvc_browse_cb(uint8_t conn_idx, uint8_t status, const ble_gattc_browse_srvc_t *p_browse_srvc)
{
    cts_c_evt_t  cts_c_evt;
    uint16_t     uuid_disc;
    uint16_t     handle_disc;

    cts_c_evt.conn_idx = conn_idx;
    cts_c_evt.evt_type = CTS_C_EVT_DISCOVERY_FAIL;

    if(BLE_GATT_ERR_BROWSE_NO_ANY_MORE == status)
    {
        return;
    }

    if (BLE_SUCCESS == status)
    {
        uuid_disc = p_browse_srvc->uuid[0] | p_browse_srvc->uuid[1] << 8;

        if (BLE_ATT_SVC_CURRENT_TIME == uuid_disc)
        {
            s_cts_c_env.handles.cts_srvc_start_handle = p_browse_srvc->start_hdl;
            s_cts_c_env.handles.cts_srvc_end_handle   = p_browse_srvc->end_hdl;

            for (uint32_t i = 0; i < (p_browse_srvc->end_hdl - p_browse_srvc->start_hdl); i++)
            {
                uuid_disc   = p_browse_srvc->info[i].attr.uuid[0] | p_browse_srvc->info[i].attr.uuid[1] << 8;
                handle_disc = p_browse_srvc->start_hdl + i + 1;

                if (BLE_GATTC_BROWSE_ATTR_VAL == p_browse_srvc->info[i].attr_type)
                {
                    if (BLE_ATT_CHAR_CT_TIME == uuid_disc)
                    {
                        s_cts_c_env.handles.cts_cur_time_handle      = handle_disc;
                        s_cts_c_env.handles.cts_cur_time_cccd_handle = handle_disc + 1;
                    }
                    else if (BLE_ATT_CHAR_LOCAL_TIME_INFO == uuid_disc)
                    {
                        s_cts_c_env.handles.cts_loc_time_info_handle = handle_disc;
                    }
                    else if (BLE_ATT_CHAR_REFERENCE_TIME_INFO == uuid_disc)
                    {
                        s_cts_c_env.handles.cts_ref_time_info_handle = handle_disc;
                    }
                }
                else if (p_browse_srvc->info[i].attr_type == BLE_GATTC_BROWSE_NONE)
                {
                    break;
                }
            }

            cts_c_evt.evt_type = CTS_C_EVT_DISCOVERY_COMPLETE;
        }
    }

    cts_c_evt_handler_excute(&cts_c_evt);
}


/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
sdk_err_t cts_client_init(cts_c_evt_handler_t evt_handler)
{
    if (NULL == evt_handler)
    {
        return SDK_ERR_POINTER_NULL;
    }

    memset(&s_cts_c_env, 0, sizeof(s_cts_c_env));
    s_cts_c_env.evt_handler = evt_handler;

    return ble_client_prf_add(&cts_c_prf_info, &s_cts_c_env.prf_id);
}

sdk_err_t cts_c_disc_srvc_start(uint8_t conn_idx)
{
    uint8_t target_uuid[2];

    target_uuid[0] = LO_U16(BLE_ATT_SVC_CURRENT_TIME);
    target_uuid[1] = HI_U16(BLE_ATT_SVC_CURRENT_TIME);

    const ble_uuid_t cts_service_uuid =
    {
        .uuid_len = 2,
        .uuid     = target_uuid,
    };

    return ble_gattc_prf_services_browse(s_cts_c_env.prf_id, conn_idx, &cts_service_uuid);
}

sdk_err_t cts_c_cur_time_notify_set(uint8_t conn_idx, bool is_enable)
{
    gattc_write_attr_value_t write_attr_value;
    uint16_t ntf_value = is_enable ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

    if (BLE_ATT_INVALID_HDL == s_cts_c_env.handles.cts_cur_time_cccd_handle)
    {
        return SDK_ERR_INVALID_HANDLE;
    }

    write_attr_value.handle  = s_cts_c_env.handles.cts_cur_time_cccd_handle;
    write_attr_value.offset  = 0;
    write_attr_value.length  = 2;
    write_attr_value.p_value = (uint8_t *)&ntf_value;

    return ble_gattc_prf_write(s_cts_c_env.prf_id, conn_idx, &write_attr_value);
}

sdk_err_t cts_c_cur_time_read(uint8_t conn_idx)
{
    if (BLE_ATT_INVALID_HDL == s_cts_c_env.handles.cts_cur_time_handle)
    {
        return SDK_ERR_INVALID_HANDLE;
    }

    return ble_gattc_prf_read(s_cts_c_env.prf_id, conn_idx, s_cts_c_env.handles.cts_cur_time_handle, 0);
}

sdk_err_t cts_c_loc_time_info_read(uint8_t conn_idx)
{
    if (BLE_ATT_INVALID_HDL == s_cts_c_env.handles.cts_loc_time_info_handle)
    {
        return SDK_ERR_INVALID_HANDLE;
    }

    return ble_gattc_prf_read(s_cts_c_env.prf_id, conn_idx, s_cts_c_env.handles.cts_loc_time_info_handle, 0);
}

sdk_err_t cts_c_ref_time_info_read(uint8_t conn_idx)
{
    if (BLE_ATT_INVALID_HDL == s_cts_c_env.handles.cts_ref_time_info_handle)
    {
        return SDK_ERR_INVALID_HANDLE;
    }

    return ble_gattc_prf_read(s_cts_c_env.prf_id, conn_idx, s_cts_c_env.handles.cts_ref_time_info_handle, 0);
}

sdk_err_t cts_c_cur_time_set(uint8_t conn_idx, cts_c_cur_time_t *p_cur_time)
{
    gattc_write_attr_value_t write_attr_value;
    uint8_t encoded_buffer[CTS_C_CUR_TIME_VAL_LEN];

    if (BLE_ATT_INVALID_HDL == s_cts_c_env.handles.cts_cur_time_handle)
    {
        return SDK_ERR_INVALID_HANDLE;
    }

    cts_c_cur_time_encode(p_cur_time, encoded_buffer);

    write_attr_value.handle  = s_cts_c_env.handles.cts_cur_time_handle;
    write_attr_value.offset  = 0;
    write_attr_value.length  = CTS_C_CUR_TIME_VAL_LEN;
    write_attr_value.p_value = encoded_buffer;

    return ble_gattc_prf_write(s_cts_c_env.prf_id, conn_idx, &write_attr_value);
}

sdk_err_t cts_c_loc_time_info_set(uint8_t conn_idx, cts_c_loc_time_info_t *p_loc_time_info)
{
    gattc_write_attr_value_t write_attr_value;
    uint8_t encoded_buffer[CTS_C_LOC_TIME_INFO_VAL_LEN];

    if (BLE_ATT_INVALID_HDL == s_cts_c_env.handles.cts_loc_time_info_handle)
    {
        return SDK_ERR_INVALID_HANDLE;
    }

    encoded_buffer[0] = p_loc_time_info->time_zone;
    encoded_buffer[1] = p_loc_time_info->dst_offset;

    write_attr_value.handle  = s_cts_c_env.handles.cts_loc_time_info_handle;
    write_attr_value.offset  = 0;
    write_attr_value.length  = CTS_C_LOC_TIME_INFO_VAL_LEN;
    write_attr_value.p_value = encoded_buffer;

    return ble_gattc_prf_write(s_cts_c_env.prf_id, conn_idx, &write_attr_value);
}


