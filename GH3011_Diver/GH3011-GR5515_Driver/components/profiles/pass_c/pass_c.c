/**
 *****************************************************************************************
 *
 * @file pass_c.c
 *
 * @brief Phone Alert Status Service Client Implementation.
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
#include "pass_c.h"
#include "ble_prf_utils.h"
#include "utility.h"
#include <string.h>

/*
 * STRUCT DEFINE
 *****************************************************************************************
 */
/**@brief Phone Alert Status Service Client environment variable. */
struct pass_c_env_t
{
    pass_c_handles_t      handles;           /**< Handles of PASS characteristics which will be got for peer. */
    pass_c_evt_handler_t  evt_handler;       /**< Handler of PASS Client event  */
    uint8_t               prf_id;            /**< PASS Client profile id. */
};

/*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static void pass_c_att_read_cb(uint8_t conn_idx, uint8_t status, const ble_gattc_read_rsp_t *p_read_rsp);
static void pass_c_att_write_cb(uint8_t conn_idx, uint8_t status, uint16_t handle);
static void pass_c_att_ntf_ind_cb(uint8_t conn_idx, const ble_gattc_ntf_ind_t *p_ntf_ind);
static void pass_c_srvc_browse_cb(uint8_t conn_idx, uint8_t status, const ble_gattc_browse_srvc_t *p_browse_srvc);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static struct pass_c_env_t s_pass_c_env;    /**< Phone Alert Status Service Client environment variable. */

/**@brief Phone Alert Status Service Client interface required by profile manager. */
static ble_prf_manager_cbs_t pass_c_mgr_cbs =
{
    NULL,
    NULL,
    NULL
};

/**@brief Phone Alert Status Service GATT Client Callbacks. */
static gattc_prf_cbs_t pass_c_gattc_cbs =
{
    NULL,
    NULL,
    NULL,
    NULL,
    pass_c_att_read_cb,
    pass_c_att_write_cb,
    pass_c_att_ntf_ind_cb,
    pass_c_srvc_browse_cb,
    NULL,
};

/**@brief Phone Alert Status Service Client Information. */
static const prf_client_info_t pass_c_prf_info =
{
    .max_connection_nb = PASS_C_CONNECTION_MAX,
    .manager_cbs       = &pass_c_mgr_cbs,
    .gattc_prf_cbs     = &pass_c_gattc_cbs
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 * @brief Excute Phone Alert Status Service Client event handler.
 *
 * @param[in] p_evt: Pointer to Alert Service Client event structure.
 *****************************************************************************************
 */
static void pass_c_evt_handler_excute(pass_c_evt_t *p_evt)
{
    if (NULL != s_pass_c_env.evt_handler && PASS_C_EVT_INVALID != p_evt->evt_type)
    {
        s_pass_c_env.evt_handler(p_evt);
    }
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
static void pass_c_att_read_cb(uint8_t conn_idx, uint8_t status, const ble_gattc_read_rsp_t *p_read_rsp)
{
    pass_c_evt_t pass_c_evt;

    pass_c_evt.conn_idx = conn_idx;
    pass_c_evt.evt_type = PASS_C_EVT_INVALID;

    if (BLE_SUCCESS != status)
    {
        return;
    }

    if (p_read_rsp->vals[0].handle == s_pass_c_env.handles.pass_alert_status_handle)
    {
        pass_c_evt.evt_type           = PASS_C_EVT_ALERT_STATUS_RECEIVE;
        pass_c_evt.value.alert_status = p_read_rsp->vals[0].p_value[0];
    }
    else if (p_read_rsp->vals[0].handle == s_pass_c_env.handles.pass_ringer_set_handle)
    {
        pass_c_evt.evt_type         = PASS_C_EVT_RINGER_SET_RECEIVE;
        pass_c_evt.value.ringer_set = p_read_rsp->vals[0].p_value[0];
    }

    pass_c_evt_handler_excute(&pass_c_evt);
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
static void pass_c_att_write_cb(uint8_t conn_idx, uint8_t status, uint16_t handle)
{
    pass_c_evt_t pass_c_evt;

    pass_c_evt.conn_idx  = conn_idx;
    pass_c_evt.evt_type  = PASS_C_EVT_INVALID;

    if (handle == s_pass_c_env.handles.pass_alert_status_cccd_handle)
    {
        pass_c_evt.evt_type = (BLE_SUCCESS == status) ? \
                               PASS_C_EVT_ALERT_STATUS_NTF_SET_SUCCESS :
                               PASS_C_EVT_WRITE_OP_ERR;
    }
    else if (handle == s_pass_c_env.handles.pass_ringer_set_cccd_handle)
    {
        pass_c_evt.evt_type = (BLE_SUCCESS == status) ? \
                               PASS_C_EVT_RINGER_SET_NTF_SET_SUCCESS :
                               PASS_C_EVT_WRITE_OP_ERR;
    }
    else if (handle == s_pass_c_env.handles.pass_ringer_ctrl_pt_handle)
    {
        pass_c_evt.evt_type = (BLE_SUCCESS == status) ? \
                               PASS_C_EVT_CTRL_POINT_SET_SUCCESS :
                               PASS_C_EVT_WRITE_OP_ERR;
    }

    pass_c_evt_handler_excute(&pass_c_evt);;
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
static void pass_c_att_ntf_ind_cb(uint8_t conn_idx, const ble_gattc_ntf_ind_t *p_ntf_ind)
{
    pass_c_evt_t pass_c_evt;

    pass_c_evt.conn_idx = conn_idx;
    pass_c_evt.evt_type = PASS_C_EVT_INVALID;

    if (p_ntf_ind->handle == s_pass_c_env.handles.pass_alert_status_handle)
    {
        pass_c_evt.evt_type           = PASS_C_EVT_ALERT_STATUS_RECEIVE;
        pass_c_evt.value.alert_status = p_ntf_ind->p_value[0];
    }
    else if (p_ntf_ind->handle == s_pass_c_env.handles.pass_ringer_set_handle)
    {
        pass_c_evt.evt_type         = PASS_C_EVT_RINGER_SET_RECEIVE;
        pass_c_evt.value.ringer_set = p_ntf_ind->p_value[0];
    }

    pass_c_evt_handler_excute(&pass_c_evt);
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
static void pass_c_srvc_browse_cb(uint8_t conn_idx, uint8_t status, const ble_gattc_browse_srvc_t *p_browse_srvc)
{
    pass_c_evt_t  pass_c_evt;
    uint16_t      uuid_disc;
    uint16_t      handle_disc;

    pass_c_evt.conn_idx = conn_idx;
    pass_c_evt.evt_type = PASS_C_EVT_DISCOVERY_FAIL;
 
    if(BLE_GATT_ERR_BROWSE_NO_ANY_MORE == status)
    {
        return;
    }

    if (BLE_SUCCESS == status)
    {
        uuid_disc = p_browse_srvc->uuid[0] | p_browse_srvc->uuid[1] << 8;

        if (BLE_ATT_SVC_PHONE_ALERT_STATUS == uuid_disc)
        {
            s_pass_c_env.handles.pass_srvc_start_handle = p_browse_srvc->start_hdl;
            s_pass_c_env.handles.pass_srvc_end_handle   = p_browse_srvc->end_hdl;

            for (uint32_t i = 0; i < p_browse_srvc->end_hdl - p_browse_srvc->start_hdl; i++)
            {
                if (BLE_GATTC_BROWSE_ATTR_VAL == p_browse_srvc->info[i].attr_type)
                {
                    uuid_disc   = p_browse_srvc->info[i].attr.uuid[0] | p_browse_srvc->info[i].attr.uuid[1] << 8;
                    handle_disc = p_browse_srvc->start_hdl + i + 1;

                    if (BLE_ATT_CHAR_ALERT_STATUS == uuid_disc)
                    {
                        s_pass_c_env.handles.pass_alert_status_handle      = handle_disc;
                        s_pass_c_env.handles.pass_alert_status_cccd_handle = handle_disc + 1;
                    }
                    else if (BLE_ATT_CHAR_RINGER_CNTL_POINT == uuid_disc)
                    {
                        s_pass_c_env.handles.pass_ringer_ctrl_pt_handle = handle_disc;
                    }
                    else if (BLE_ATT_CHAR_RINGER_SETTING == uuid_disc)
                    {
                        s_pass_c_env.handles.pass_ringer_set_handle      = handle_disc;
                        s_pass_c_env.handles.pass_ringer_set_cccd_handle = handle_disc + 1;
                    }
                }
                else if (p_browse_srvc->info[i].attr_type == BLE_GATTC_BROWSE_NONE)
                {
                    break;
                }
            }

            pass_c_evt.evt_type = PASS_C_EVT_DISCOVERY_COMPLETE;
        }
    }

    pass_c_evt_handler_excute(&pass_c_evt);
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
sdk_err_t pass_client_init(pass_c_evt_handler_t evt_handler)
{
    if (NULL == evt_handler)
    {
        return SDK_ERR_POINTER_NULL;
    }

    memset(&s_pass_c_env, 0, sizeof(s_pass_c_env));
    s_pass_c_env.evt_handler = evt_handler;

    return ble_client_prf_add(&pass_c_prf_info, &s_pass_c_env.prf_id);
}

sdk_err_t pass_c_disc_srvc_start(uint8_t conn_idx)
{
    uint8_t target_uuid[2];
    target_uuid[0] = LO_U16(BLE_ATT_SVC_PHONE_ALERT_STATUS);
    target_uuid[1] = HI_U16(BLE_ATT_SVC_PHONE_ALERT_STATUS);
    const ble_uuid_t pass_service_uuid =
    {
        .uuid_len = 2,
        .uuid     = target_uuid,
    };
    return ble_gattc_prf_services_browse(s_pass_c_env.prf_id, conn_idx, &pass_service_uuid);
}

sdk_err_t pass_c_alert_status_notify_set(uint8_t conn_idx, bool is_enable)
{
    gattc_write_attr_value_t write_attr_value;
    uint16_t ntf_value = is_enable ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

    if (BLE_ATT_INVALID_HDL == s_pass_c_env.handles.pass_alert_status_cccd_handle)
    {
        return BLE_ATT_ERR_INVALID_HANDLE;
    }

    write_attr_value.handle  = s_pass_c_env.handles.pass_alert_status_cccd_handle;
    write_attr_value.offset  = 0;
    write_attr_value.length  = 2;
    write_attr_value.p_value = (uint8_t *)&ntf_value;

    return ble_gattc_prf_write(s_pass_c_env.prf_id, conn_idx, &write_attr_value);
}

sdk_err_t pass_c_ringer_set_notify_set(uint8_t conn_idx, bool is_enable)
{
    gattc_write_attr_value_t write_attr_value;
    uint16_t ntf_value = is_enable ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

    if (BLE_ATT_INVALID_HDL == s_pass_c_env.handles.pass_ringer_set_cccd_handle)
    {
        return BLE_ATT_ERR_INVALID_HANDLE;
    }

    write_attr_value.handle  = s_pass_c_env.handles.pass_ringer_set_cccd_handle;
    write_attr_value.offset  = 0;
    write_attr_value.length  = 2;
    write_attr_value.p_value = (uint8_t *)&ntf_value;

    return ble_gattc_prf_write(s_pass_c_env.prf_id, conn_idx, &write_attr_value);
}

sdk_err_t pass_c_ctrl_point_set(uint8_t conn_idx, uint8_t ctrl_value)
{
    gattc_write_no_resp_t write_attr_value;

    if (BLE_ATT_INVALID_HDL == s_pass_c_env.handles.pass_ringer_ctrl_pt_handle)
    {
        return BLE_ATT_ERR_INVALID_HANDLE;
    }

    write_attr_value.signed_write  = false;
    write_attr_value.handle        =  s_pass_c_env.handles.pass_ringer_ctrl_pt_handle;
    write_attr_value.length        = PASS_C_RINGER_CTRL_PT_VAL_LEN;
    write_attr_value.p_value       = &ctrl_value;

    return ble_gattc_prf_write_no_resp(s_pass_c_env.prf_id, conn_idx, &write_attr_value);
}

sdk_err_t pass_c_alert_status_read(uint8_t conn_idx)
{
    if (BLE_ATT_INVALID_HDL == s_pass_c_env.handles.pass_alert_status_handle)
    {
        return BLE_ATT_ERR_INVALID_HANDLE;
    }

    return ble_gattc_prf_read(s_pass_c_env.prf_id, conn_idx, s_pass_c_env.handles.pass_alert_status_handle, 0);
}

sdk_err_t pass_c_ringer_set_read(uint8_t conn_idx)
{
    if (BLE_ATT_INVALID_HDL == s_pass_c_env.handles.pass_ringer_set_handle)
    {
        return BLE_ATT_ERR_INVALID_HANDLE;
    }

    return ble_gattc_prf_read(s_pass_c_env.prf_id, conn_idx, s_pass_c_env.handles.pass_ringer_set_handle, 0);
}

