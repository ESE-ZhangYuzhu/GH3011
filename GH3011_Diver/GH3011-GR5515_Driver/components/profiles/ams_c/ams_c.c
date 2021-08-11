/**
 ****************************************************************************************
 *
 * @file ams_c.c
 *
 * @brief Apple Media Service Client implementation.
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
#include "ams_c.h"

/*
 * STRUCT DEFINE
 *****************************************************************************************
 */
/**@brief Apple Media Service Client environment variable. */
static const uint8_t ams_service_uuid[]      = {AMS_SRVC_UUID};
static const uint8_t ams_cmd_uuid[]          = {AMS_CMD_UUID};
static const uint8_t ams_attr_update_uuid[]  = {AMS_ATTR_UPDATE_UUID};
static const uint8_t ams_attr_display_uuid[] = {AMS_ATTR_DISPLAY_UUID};

/**@brief Apple Media Service Client environment variable. */
struct ams_c_env_t
{
    ams_c_handles_t      handles;           /**< Handles of AMS characteristics which will be got for peer. */
    ams_c_evt_handler_t  evt_handler;       /**< Handler of AMS Client event handler. */
    uint8_t              prf_id;            /**< AMS Client profile id. */
    uint16_t             cmd_enable_flag;   /**< The flag bits of the available status for the commands. */
};

/*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static void ams_c_att_read_cb(uint8_t conn_idx, uint8_t status, 
                              const ble_gattc_read_rsp_t *p_read_rsp);
static void ams_c_att_write_cb(uint8_t conn_idx, uint8_t status, uint16_t handle);
static void ams_c_att_ntf_ind_cb(uint8_t conn_idx, const ble_gattc_ntf_ind_t *p_ntf_ind);
static void ams_c_srvc_browse_cb(uint8_t conn_idx, uint8_t status, 
                                 const ble_gattc_browse_srvc_t *p_browse_srvc);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static struct ams_c_env_t s_ams_c_env;    /**< Apple Media Service Client environment variable. */

/**@brief Apple Media Service Client interface required by profile manager. */
static ble_prf_manager_cbs_t ams_c_mgr_cbs =
{
    NULL,
    NULL,
    NULL
};

/**@brief Apple Media Service GATT Client Callbacks. */
static gattc_prf_cbs_t ams_c_gattc_cbs =
{
    NULL,
    NULL,
    NULL,
    NULL,
    ams_c_att_read_cb,
    ams_c_att_write_cb,
    ams_c_att_ntf_ind_cb,
    ams_c_srvc_browse_cb,
    NULL,
};

/**@brief Apple Media Service Client Information. */
static const prf_client_info_t ams_c_prf_info =
{
    .max_connection_nb = AMS_C_CONNECTION_MAX,
    .manager_cbs       = &ams_c_mgr_cbs,
    .gattc_prf_cbs     = &ams_c_gattc_cbs
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 * @brief Excute Apple Media Service Client event handler.
 *
 * @param[in] p_evt: Pointer to Apple Media Service Client event structure.
 *****************************************************************************************
 */
static void ams_c_evt_handler_excute(ams_c_evt_t *p_evt)
{
    if (NULL != s_ams_c_env.evt_handler && AMS_C_EVT_INVALID != p_evt->evt_type)
    {
        s_ams_c_env.evt_handler(p_evt);
    }
}

/**
 *****************************************************************************************
 * @brief Decode for attribue information.
 *
 * @param[in]  p_data:     Pointer to data to be decoded.
 * @param[in]  length:     Length of data to be decoded.
 * @param[out] p_cur_time: Pointer to the structure to store attribute information.
 *****************************************************************************************
 */
static void ams_c_attr_info_decode(uint8_t *p_data, uint16_t length,
                                   ams_c_attr_info_t *p_attr_info)
{
    p_attr_info->ett_id  = (ams_c_ett_id_t)(p_data[0]);
    p_attr_info->attr_id = p_data[1];
    p_attr_info->flag    = p_data[2];
    p_attr_info->p_data  = &(p_data[3]);
    p_attr_info->length  = length - 3;
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
static void ams_c_att_read_cb(uint8_t conn_idx, uint8_t status, 
                              const ble_gattc_read_rsp_t *p_read_rsp)
{
    ams_c_evt_t ams_c_evt;

    ams_c_evt.conn_idx = conn_idx;
    ams_c_evt.evt_type = AMS_C_EVT_INVALID;

    if (BLE_SUCCESS != status)
    {
        return;
    }

    if (p_read_rsp->vals[0].handle == s_ams_c_env.handles.ams_attr_display_handle)
    {
        ams_c_evt.conn_idx = conn_idx;
        ams_c_evt.evt_type = AMS_C_EVT_CPLT_ATTR_READ_RSP;
        ams_c_evt.param.cplt_attr_data.p_data = p_read_rsp->vals[0].p_value;
        ams_c_evt.param.cplt_attr_data.length = p_read_rsp->vals[0].length;
    }

    ams_c_evt_handler_excute(&ams_c_evt);
}

/**
 *****************************************************************************************
 * @brief This callback function will be called when receiving write response.
 *
 * @param[in] conn_idx:   The connection index.
 * @param[in] status:     The status of GATTC operation.
 * @param[in] handle:     The handle of attribute.
 *****************************************************************************************
 */
static void ams_c_att_write_cb(uint8_t conn_idx, uint8_t status, uint16_t handle)
{
    ams_c_evt_t ams_c_evt;

    ams_c_evt.conn_idx  = conn_idx;
    ams_c_evt.evt_type  = AMS_C_EVT_INVALID;

    if (handle == s_ams_c_env.handles.ams_cmd_handle)
    {
        ams_c_evt.evt_type = (BLE_SUCCESS == status) ? \
                              AMS_C_EVT_CMD_SEND_SUCCESS :
                              AMS_C_EVT_WRITE_OP_ERR;
    }
    else if (handle == s_ams_c_env.handles.ams_cmd_cccd_handle)
    {
        ams_c_evt.evt_type = (BLE_SUCCESS == status) ? \
                              AMS_C_EVT_CMD_UPDATE_NTF_SET_SUCCESS :
                              AMS_C_EVT_WRITE_OP_ERR;
    }
    else if (handle == s_ams_c_env.handles.ams_attr_update_handle)
    {
        ams_c_evt.evt_type = (BLE_SUCCESS == status) ? \
                              AMS_C_EVT_ATTR_FOCUS_SET_SUCCESS :
                              AMS_C_EVT_WRITE_OP_ERR;
    }
    else if (handle == s_ams_c_env.handles.ams_attr_update_cccd_handle)
    {
        ams_c_evt.evt_type = (BLE_SUCCESS == status) ? \
                              AMS_C_EVT_ATTR_UPDATE_NTF_SET_SUCCESS :
                              AMS_C_EVT_WRITE_OP_ERR;
    }
    else if (handle == s_ams_c_env.handles.ams_attr_display_handle)
    {
        ams_c_evt.evt_type = (BLE_SUCCESS == status) ? \
                              AMS_C_EVT_CPLT_ATTR_DISPLAY_SET_SUCCESS :
                              AMS_C_EVT_WRITE_OP_ERR;
    }

    ams_c_evt_handler_excute(&ams_c_evt);
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
static void ams_c_att_ntf_ind_cb(uint8_t conn_idx, const ble_gattc_ntf_ind_t *p_ntf_ind)
{
    ams_c_evt_t ams_c_evt;

    ams_c_evt.conn_idx = conn_idx;
    ams_c_evt.evt_type = AMS_C_EVT_INVALID;

    if (p_ntf_ind->handle == s_ams_c_env.handles.ams_cmd_handle)
    {
        ams_c_evt.evt_type = AMS_C_EVT_CMD_UPDATE_RECEIVE;
        
        ams_c_evt.param.cmd_list.p_cmd = (ams_c_cmd_id_t *)(p_ntf_ind->p_value);
        ams_c_evt.param.cmd_list.length = p_ntf_ind->length;
        
        s_ams_c_env.cmd_enable_flag = 0;
        for (uint16_t i = 0; i < p_ntf_ind->length; i++)
        {
            s_ams_c_env.cmd_enable_flag |= (0x01 << p_ntf_ind->p_value[i]);
        }
    }
    else if (p_ntf_ind->handle == s_ams_c_env.handles.ams_attr_update_handle)
    {
        ams_c_evt.evt_type = AMS_C_EVT_ATTR_UPDATE_RECEIVE;
        ams_c_attr_info_decode(p_ntf_ind->p_value, p_ntf_ind->length, 
        &ams_c_evt.param.attr_info);
    }

    ams_c_evt_handler_excute(&ams_c_evt);
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

static void ams_c_srvc_browse_cb(uint8_t conn_idx, uint8_t status, 
                                 const ble_gattc_browse_srvc_t *p_browse_srvc)
{
    ams_c_evt_t  ams_c_evt;
    uint16_t     handle_disc;

    ams_c_evt.conn_idx = conn_idx;
    ams_c_evt.evt_type = AMS_C_EVT_DISCOVERY_FAIL;

    if(BLE_GATT_ERR_BROWSE_NO_ANY_MORE == status)
    {
        return;
    }

    if (BLE_SUCCESS == status)
    {
        if (0 == memcmp(p_browse_srvc->uuid, ams_service_uuid, BLE_ATT_UUID_128_LEN))
        {
            s_ams_c_env.handles.ams_srvc_start_handle = p_browse_srvc->start_hdl;
            s_ams_c_env.handles.ams_srvc_end_handle   = p_browse_srvc->end_hdl;

            for (uint32_t i = 0; i < p_browse_srvc->end_hdl - p_browse_srvc->start_hdl; i++)
            {
                if (BLE_GATTC_BROWSE_ATTR_VAL == p_browse_srvc->info[i].attr_type)
                {
                    handle_disc = p_browse_srvc->start_hdl + i + 1;

                    if (0 == memcmp(p_browse_srvc->info[i].attr.uuid, ams_cmd_uuid, 
                        BLE_ATT_UUID_128_LEN))
                    {
                        s_ams_c_env.handles.ams_cmd_handle      = handle_disc;
                        s_ams_c_env.handles.ams_cmd_cccd_handle = handle_disc + 2;
                    }
                    else if (0 == memcmp(p_browse_srvc->info[i].attr.uuid, 
                             ams_attr_update_uuid, BLE_ATT_UUID_128_LEN))
                    {
                        s_ams_c_env.handles.ams_attr_update_handle      = handle_disc;
                        s_ams_c_env.handles.ams_attr_update_cccd_handle = handle_disc + 2;
                    }
                    else if (0 == memcmp(p_browse_srvc->info[i].attr.uuid, 
                             ams_attr_display_uuid, BLE_ATT_UUID_128_LEN))
                    {
                        s_ams_c_env.handles.ams_attr_display_handle = handle_disc;
                    }
                }

                else if (p_browse_srvc->info[i].attr_type == BLE_GATTC_BROWSE_NONE)
                {
                    break;
                }
            }
            ams_c_evt.evt_type = AMS_C_EVT_DISCOVERY_CPLT;
        }
    }

    ams_c_evt_handler_excute(&ams_c_evt);
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
sdk_err_t ams_c_client_init(ams_c_evt_handler_t evt_handler)
{
    if (NULL == evt_handler)
    {
        return SDK_ERR_POINTER_NULL;
    }

    memset(&s_ams_c_env, 0, sizeof(s_ams_c_env));
    s_ams_c_env.evt_handler = evt_handler;

    return ble_client_prf_add(&ams_c_prf_info, &s_ams_c_env.prf_id);
}

sdk_err_t ams_c_disc_srvc_start(uint8_t conn_idx)
{
    ble_uuid_t ble_ams_uuid =
    {
        .uuid_len = BLE_ATT_UUID_128_LEN,
        .uuid     = (uint8_t *)ams_service_uuid,
    };
    return ble_gattc_prf_services_browse(s_ams_c_env.prf_id, conn_idx, &ble_ams_uuid);
}

sdk_err_t ams_c_cmd_notify_set(uint8_t conn_idx, bool is_enable)
{
    gattc_write_attr_value_t write_attr_value;
    uint16_t ntf_value = is_enable ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

    if (BLE_ATT_INVALID_HDL == s_ams_c_env.handles.ams_cmd_cccd_handle)
    {
        return BLE_ATT_ERR_INVALID_HANDLE;
    }
    write_attr_value.handle  = s_ams_c_env.handles.ams_cmd_cccd_handle;
    write_attr_value.offset  = 0;
    write_attr_value.length  = 2;
    write_attr_value.p_value = (uint8_t *)&ntf_value;

    return ble_gattc_prf_write(s_ams_c_env.prf_id, conn_idx, &write_attr_value);
}

sdk_err_t ams_c_attr_update_notify_set(uint8_t conn_idx, bool is_enable)
{
    gattc_write_attr_value_t write_attr_value;
    uint16_t ntf_value = is_enable ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

    if (BLE_ATT_INVALID_HDL == s_ams_c_env.handles.ams_attr_update_cccd_handle)
    {
        return BLE_ATT_ERR_INVALID_HANDLE;
    }
    write_attr_value.handle  = s_ams_c_env.handles.ams_attr_update_cccd_handle;
    write_attr_value.offset  = 0;
    write_attr_value.length  = 2;
    write_attr_value.p_value = (uint8_t *)&ntf_value;

    return ble_gattc_prf_write(s_ams_c_env.prf_id, conn_idx, &write_attr_value);
}

sdk_err_t ams_c_cplt_attr_read(uint8_t conn_idx)
{
    if (BLE_ATT_INVALID_HDL == s_ams_c_env.handles.ams_attr_display_handle)
    {
        return BLE_ATT_ERR_INVALID_HANDLE;
    }
    return ble_gattc_prf_read(s_ams_c_env.prf_id, conn_idx, 
                              s_ams_c_env.handles.ams_attr_display_handle, 0);
}

sdk_err_t ams_c_cmd_send(uint8_t conn_idx, uint8_t cmd_id)
{
    if (BLE_ATT_INVALID_HDL == s_ams_c_env.handles.ams_cmd_handle)
    {
        return BLE_ATT_ERR_INVALID_HANDLE;
    }
    gattc_write_attr_value_t write_attr_value;
    write_attr_value.handle = s_ams_c_env.handles.ams_cmd_handle;
    write_attr_value.offset = 0;
    write_attr_value.length = 1;
    write_attr_value.p_value = (uint8_t *)& cmd_id;

    return ble_gattc_prf_write(s_ams_c_env.prf_id, conn_idx, &write_attr_value);
}

sdk_err_t ams_c_attr_focus_set(uint8_t conn_idx, const ams_c_ett_attr_id_t *p_ett_attr_id)
{
    if (BLE_ATT_INVALID_HDL == s_ams_c_env.handles.ams_attr_update_handle)
    {
        return BLE_ATT_ERR_INVALID_HANDLE;
    }
    gattc_write_attr_value_t write_attr_value;
    write_attr_value.handle = s_ams_c_env.handles.ams_attr_update_handle;
    write_attr_value.offset = 0;
    write_attr_value.length = p_ett_attr_id->attr_count + 1;
    write_attr_value.p_value = (uint8_t *)&(p_ett_attr_id->ett_id);

    return ble_gattc_prf_write(s_ams_c_env.prf_id, conn_idx, &write_attr_value);
}

sdk_err_t ams_c_attr_display_set(uint8_t conn_idx, const ams_c_attr_info_t *p_attr_info)
{
    if (BLE_ATT_INVALID_HDL == s_ams_c_env.handles.ams_attr_display_handle)
    {
        return BLE_ATT_ERR_INVALID_HANDLE;
    }

    gattc_write_attr_value_t write_attr_value;
    write_attr_value.handle = s_ams_c_env.handles.ams_attr_display_handle;
    write_attr_value.offset = 0;
    write_attr_value.length = 2;
    write_attr_value.p_value = (uint8_t *)&(p_attr_info->ett_id);

    return ble_gattc_prf_write(s_ams_c_env.prf_id, conn_idx, &write_attr_value);
}

bool ams_c_cmd_enable_check(ams_c_cmd_id_t cmd_id)
{
    if ((0x01 << cmd_id) & s_ams_c_env.cmd_enable_flag)
    {
        return true;
    }
    return false;
}
