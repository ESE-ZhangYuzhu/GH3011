/**
 *****************************************************************************************
 *
 * @file gus_c.c
 *
 * @brief Goodix UART Client Implementation.
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
#include "gus_c.h"

/*
 * STRUCT DEFINE
 *****************************************************************************************
 */
/**@brief Goodix UART Service Client environment variable. */
struct gus_c_env_t
{
    gus_c_handles_t      handles;            /**< Handles of GUS characteristics which will be got for peer. */
    gus_c_evt_handler_t  evt_handler;        /**< Handler of GUS client event  */
    uint8_t              prf_id;             /**< GUS Client profile id. */
};

/*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static void gus_c_att_write_cb(uint8_t conn_idx, uint8_t status, uint16_t handle);
static void gus_c_att_ntf_ind_cb(uint8_t conn_idx, const ble_gattc_ntf_ind_t *p_ntf_ind);
static void gus_c_srvc_browse_cb(uint8_t conn_idx, uint8_t status, const ble_gattc_browse_srvc_t *p_browse_srvc);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static struct gus_c_env_t s_gus_c_env;       /**< GUS Client environment variable. */
static uint8_t            s_gus_uuid[16]                = GUS_SVC_UUID;
static uint8_t            s_gus_rx_char_uuid[16]        = GUS_RX_CHAR_UUID;
static uint8_t            s_gus_tx_char_uuid[16]        = GUS_TX_CHAR_UUID;
static uint8_t            s_gus_flow_ctrl_char_uuid[16] = GUS_FLOW_CTRL_UUID;

/**@brief GUS Client interface required by profile manager. */
static ble_prf_manager_cbs_t gus_c_mgr_cbs =
{
    NULL,
    NULL,
    NULL
};

/**@brief GUS GATT Client Callbacks. */
static gattc_prf_cbs_t gus_c_gattc_cbs =
{
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    gus_c_att_write_cb,
    gus_c_att_ntf_ind_cb,
    gus_c_srvc_browse_cb,
    NULL,
};

/**@brief GUS Client Information. */
static const prf_client_info_t gus_c_prf_info =
{
    .max_connection_nb = GUS_C_CONNECTION_MAX,
    .manager_cbs       = &gus_c_mgr_cbs,
    .gattc_prf_cbs     = &gus_c_gattc_cbs
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 * @brief Excute GUS Service Client event handler.
 *
 * @param[in] p_evt: Pointer to GUS Service Client event structure.
 *****************************************************************************************
 */
static void gus_c_evt_handler_excute(gus_c_evt_t *p_evt)
{
    if (NULL != s_gus_c_env.evt_handler && GUS_C_EVT_INVALID != p_evt->evt_type)
    {
        s_gus_c_env.evt_handler(p_evt);
    }
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
static void gus_c_att_write_cb(uint8_t conn_idx, uint8_t status, uint16_t handle)
{
    gus_c_evt_t gus_c_evt;

    gus_c_evt.conn_idx = conn_idx;
    gus_c_evt.evt_type = GUS_C_EVT_INVALID;

    if (handle == s_gus_c_env.handles.gus_tx_cccd_handle)
    {
        gus_c_evt.evt_type = (BLE_SUCCESS == status) ? \
                              GUS_C_EVT_TX_NTF_SET_SUCCESS : \
                              GUS_C_EVT_WRITE_OP_ERR;
    }
    else if (handle == s_gus_c_env.handles.gus_flow_ctrl_cccd_handle)
    {
        gus_c_evt.evt_type = (BLE_SUCCESS == status) ? \
                              GUS_C_EVT_FLOW_CTRL_NTF_SET_SUCCESS : \
                              GUS_C_EVT_WRITE_OP_ERR;
    }
    else if (handle == s_gus_c_env.handles.gus_rx_handle)
    {
        gus_c_evt.evt_type = (BLE_SUCCESS == status) ? \
                              GUS_C_EVT_TX_CPLT : \
                              GUS_C_EVT_WRITE_OP_ERR;
    }
    else if (handle == s_gus_c_env.handles.gus_flow_ctrl_handle)
    {
        gus_c_evt.evt_type = (BLE_SUCCESS == status) ? \
                              GUS_C_EVT_RX_FLOW_UPDATE_CPLT : \
                              GUS_C_EVT_WRITE_OP_ERR;
    }

    gus_c_evt_handler_excute(&gus_c_evt);
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
static void gus_c_att_ntf_ind_cb(uint8_t conn_idx, const ble_gattc_ntf_ind_t *p_ntf_ind)
{
    gus_c_evt_t gus_c_evt;

    gus_c_evt.conn_idx = conn_idx;
    gus_c_evt.evt_type = GUS_C_EVT_INVALID;

    if (p_ntf_ind->handle == s_gus_c_env.handles.gus_flow_ctrl_handle)
    {
        if (FLOW_ON == p_ntf_ind->p_value[0])
        {
            gus_c_evt.evt_type = GUS_C_EVT_TX_FLOW_ON;
        }
        else if (FLOW_OFF == p_ntf_ind->p_value[0])
        {
            gus_c_evt.evt_type = GUS_C_EVT_TX_FLOW_OFF;
        }
    }
    else if (p_ntf_ind->handle == s_gus_c_env.handles.gus_tx_handle)
    {
        gus_c_evt.evt_type = GUS_C_EVT_PEER_DATA_RECEIVE;
        gus_c_evt.p_data   = p_ntf_ind->p_value;
        gus_c_evt.length   = p_ntf_ind->length;
    }

    gus_c_evt_handler_excute(&gus_c_evt);
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
static void gus_c_srvc_browse_cb(uint8_t conn_idx, uint8_t status, const ble_gattc_browse_srvc_t *p_browse_srvc)
{
    gus_c_evt_t  gus_c_evt;
    uint16_t     handle_disc;

    gus_c_evt.conn_idx = conn_idx;
    gus_c_evt.evt_type = GUS_C_EVT_DISCOVERY_FAIL;

    if(BLE_GATT_ERR_BROWSE_NO_ANY_MORE == status)
    {
        return;
    }

    if (BLE_SUCCESS == status)
    {
        if (16 == p_browse_srvc->uuid_len && 0 == memcmp(p_browse_srvc->uuid, s_gus_uuid, 16))
        {
            s_gus_c_env.handles.gus_srvc_start_handle = p_browse_srvc->start_hdl;
            s_gus_c_env.handles.gus_srvc_end_handle   = p_browse_srvc->end_hdl;

            for (uint32_t i = 0; i < (p_browse_srvc->end_hdl - p_browse_srvc->start_hdl); i++)
            {
                handle_disc = p_browse_srvc->start_hdl + i + 1;

                if (BLE_GATTC_BROWSE_ATTR_VAL == p_browse_srvc->info[i].attr_type)
                {
                    if (0 == memcmp(p_browse_srvc->info[i].attr.uuid, s_gus_rx_char_uuid, 16))
                    {
                        s_gus_c_env.handles.gus_rx_handle = handle_disc;
                    }
                    else if (0 == memcmp(p_browse_srvc->info[i].attr.uuid, s_gus_tx_char_uuid, 16))
                    {
                        s_gus_c_env.handles.gus_tx_handle      = handle_disc;
                        s_gus_c_env.handles.gus_tx_cccd_handle = handle_disc + 1;
                    }
                    else if (0 == memcmp(p_browse_srvc->info[i].attr.uuid, s_gus_flow_ctrl_char_uuid, 16))
                    {
                        s_gus_c_env.handles.gus_flow_ctrl_handle      = handle_disc;
                        s_gus_c_env.handles.gus_flow_ctrl_cccd_handle = handle_disc + 1;
                    }
                }
            
                if (p_browse_srvc->info[i].attr_type == BLE_GATTC_BROWSE_NONE)
                {
                    break;
                }
            }

            gus_c_evt.evt_type = GUS_C_EVT_DISCOVERY_COMPLETE;
        }
    }

    gus_c_evt_handler_excute(&gus_c_evt);
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
sdk_err_t gus_client_init(gus_c_evt_handler_t evt_handler)
{
    if (NULL == evt_handler)
    {
        return SDK_ERR_POINTER_NULL;
    }

    memset(&s_gus_c_env, 0, sizeof(s_gus_c_env));
    s_gus_c_env.evt_handler = evt_handler;

    return ble_client_prf_add(&gus_c_prf_info, &s_gus_c_env.prf_id);
}

sdk_err_t gus_c_disc_srvc_start(uint8_t conn_idx)
{
    const ble_uuid_t gus_uuid =
    {
        .uuid_len = 16,
        .uuid     = s_gus_uuid,
    };

    return ble_gattc_prf_services_browse(s_gus_c_env.prf_id, conn_idx, &gus_uuid);
}

sdk_err_t gus_c_tx_notify_set(uint8_t conn_idx, bool is_enable)
{
    gattc_write_attr_value_t write_attr_value;
    uint16_t ntf_value = is_enable ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

    if (BLE_ATT_INVALID_HDL == s_gus_c_env.handles.gus_tx_cccd_handle)
    {
        return SDK_ERR_INVALID_HANDLE;
    }

    write_attr_value.handle  = s_gus_c_env.handles.gus_tx_cccd_handle;
    write_attr_value.offset  = 0;
    write_attr_value.length  = 2;
    write_attr_value.p_value = (uint8_t *)&ntf_value;

    return ble_gattc_prf_write(s_gus_c_env.prf_id, conn_idx, &write_attr_value);
}

sdk_err_t gus_c_flow_ctrl_notify_set(uint8_t conn_idx, bool is_enable)
{
    gattc_write_attr_value_t write_attr_value;
    uint16_t ntf_value = is_enable ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

    if (BLE_ATT_INVALID_HDL == s_gus_c_env.handles.gus_flow_ctrl_cccd_handle)
    {
        return SDK_ERR_INVALID_HANDLE;
    }

    write_attr_value.handle  = s_gus_c_env.handles.gus_flow_ctrl_cccd_handle;
    write_attr_value.offset  = 0;
    write_attr_value.length  = 2;
    write_attr_value.p_value = (uint8_t *)&ntf_value;

    return ble_gattc_prf_write(s_gus_c_env.prf_id, conn_idx, &write_attr_value);
}

sdk_err_t gus_c_tx_data_send(uint8_t conn_idx, uint8_t *p_data, uint16_t length)
{
    gattc_write_no_resp_t write_attr_value;

    if (BLE_ATT_INVALID_HDL == s_gus_c_env.handles.gus_rx_handle)
    {
        return SDK_ERR_INVALID_HANDLE;
    }

    if (NULL == p_data)
    {
        return SDK_ERR_POINTER_NULL;
    }

    write_attr_value.signed_write  = false;
    write_attr_value.handle        = s_gus_c_env.handles.gus_rx_handle;
    write_attr_value.length        = length;
    write_attr_value.p_value       = p_data;

    return ble_gattc_prf_write_no_resp(s_gus_c_env.prf_id, conn_idx, &write_attr_value);
}

sdk_err_t gus_c_rx_flow_ctrl_set(uint8_t conn_idx, uint8_t flow_ctrl)
{
    gattc_write_attr_value_t write_attr_value;

    if (BLE_ATT_INVALID_HDL == s_gus_c_env.handles.gus_flow_ctrl_handle)
    {
        return SDK_ERR_INVALID_HANDLE;
    }

    write_attr_value.handle  = s_gus_c_env.handles.gus_flow_ctrl_handle;
    write_attr_value.offset  = 0;
    write_attr_value.length  = 1;
    write_attr_value.p_value = &flow_ctrl;

    return ble_gattc_prf_write(s_gus_c_env.prf_id, conn_idx, &write_attr_value);
}


