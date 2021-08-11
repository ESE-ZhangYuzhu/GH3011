/**
 *****************************************************************************************
 *
 * @file ancs_c.c
 *
 * @brief ANCS Profile Server implementation.
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

#include "ancs_c.h"
#include "ancs_protocol.h"
#include "string.h"

/*
 * STRUCT DEFINE
 *****************************************************************************************
 */
/**@brief Apple notification center client environment variable. */
struct ancs_c_env_t
{
    ancs_c_att_handles_t  handles;           /**< Handles of ANCS characteristics which will be got for peer. */
    ancs_c_evt_handler_t  evt_handler;       /**< Handler of ANCS Client event handler. */
    uint8_t               prf_id;            /**< ANCS Client profile id. */
};

/*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static void ancs_c_att_write_cb(uint8_t conn_idx, uint8_t status, uint16_t handle);
static void ancs_c_att_ntf_ind_cb(uint8_t conn_idx, const ble_gattc_ntf_ind_t *p_ntf_ind);
static void ancs_c_srvc_browse_cb(uint8_t conn_idx, uint8_t status, 
                                 const ble_gattc_browse_srvc_t *p_browse_srvc);


/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static const uint8_t ancs_service_uuid[] = {ANCS_SRVC_UUID};
static const uint8_t ancs_notification_source_uuid[] = {ANCS_NTF_SOURCE_UUID};
static const uint8_t ancs_control_point_uuid[] ={ANCS_CONTROL_POINT_UUID};
static const uint8_t ancs_data_source_uuid[] ={ANCS_DATA_SOURCE_UUID};

static struct ancs_c_env_t s_ancs_c_env;    /**< Apple notification center client environment variable. */

/**@brief ANCS Client interface required by profile manager. */
static ble_prf_manager_cbs_t ancs_c_mgr_cbs =
{
    NULL,
    NULL,
    NULL
};

/**@brief ANCS GATT Client Callbacks. */
static gattc_prf_cbs_t ancs_c_gattc_cbs =
{
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ancs_c_att_write_cb,
    ancs_c_att_ntf_ind_cb,
    ancs_c_srvc_browse_cb,
    NULL,
};

/**@brief ANCS Client Information. */
static const prf_client_info_t ancs_c_prf_info =
{
    .max_connection_nb = ANCS_C_CONNECTION_MAX,
    .manager_cbs       = &ancs_c_mgr_cbs,
    .gattc_prf_cbs     = &ancs_c_gattc_cbs
};

 /*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */

 /**
 *****************************************************************************************
 * @brief Excute ANCS client event handler.
 *
 * @param[in] p_evt: Pointer to ANCS client event structure.
 *****************************************************************************************
 */
static void ancs_c_evt_handler_excute(ancs_c_evt_t *p_evt)
{
    if (NULL != s_ancs_c_env.evt_handler && BLE_ANCS_C_EVT_INVALID != p_evt->evt_type)
    {
        s_ancs_c_env.evt_handler(p_evt);
    }
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
static void ancs_c_att_write_cb(uint8_t conn_idx, uint8_t status, uint16_t handle)
{
    ancs_c_evt_t ancs_c_evt;

    ancs_c_evt.conn_idx  = conn_idx;
    ancs_c_evt.evt_type  = BLE_ANCS_C_EVT_INVALID;

    if (handle == s_ancs_c_env.handles.ancs_ntf_source_cccd_handle)
    {
        ancs_c_evt.evt_type = (BLE_SUCCESS == status) ? \
                              BLE_ANCS_C_EVT_NTF_SOURCE_NTF_ENABLED :
                              BLE_ANCS_C_EVT_WRITE_OP_ERR;
    }
    else if (handle == s_ancs_c_env.handles.ancs_data_source_cccd_handle)
    {
        ancs_c_evt.evt_type = (BLE_SUCCESS == status) ? \
                              BLE_ANCS_C_EVT_DATA_SOURCE_NTF_ENABLED :
                              BLE_ANCS_C_EVT_WRITE_OP_ERR;
    }

    ancs_c_evt_handler_excute(&ancs_c_evt);
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
static void ancs_c_att_ntf_ind_cb(uint8_t conn_idx, const ble_gattc_ntf_ind_t *p_ntf_ind)
{
    ancs_c_evt_t ancs_c_evt;

    ancs_c_evt.conn_idx = conn_idx;
    ancs_c_evt.evt_type = BLE_ANCS_C_EVT_INVALID;

    if (p_ntf_ind->handle == s_ancs_c_env.handles.ancs_ntf_source_handle)
    {
        ancs_c_evt.evt_type = BLE_ANCS_C_EVT_NTF_SOURCE_RECEIVE;
        ancs_decode_notification_source(p_ntf_ind->p_value, p_ntf_ind->length);
    }
    else if (p_ntf_ind->handle == s_ancs_c_env.handles.ancs_data_source_handle)
    {
        ancs_c_evt.evt_type = BLE_ANCS_C_EVT_DATA_SOURCE_RECEIVE;
        ancs_decode_data_source(p_ntf_ind->p_value, p_ntf_ind->length);
    }

    ancs_c_evt_handler_excute(&ancs_c_evt);
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
sdk_err_t ancs_c_client_init(ancs_c_evt_handler_t evt_handler)
{
    if (NULL == evt_handler)
    {
        return SDK_ERR_POINTER_NULL;
    }

    memset(&s_ancs_c_env, 0, sizeof(s_ancs_c_env));
    s_ancs_c_env.evt_handler = evt_handler;

    return ble_client_prf_add(&ancs_c_prf_info, &s_ancs_c_env.prf_id);
}

/**
 *****************************************************************************************
 * @brief To access phone's all service about ANCS
 *
 * @param[in] conn_idx: Connection index
 *****************************************************************************************
*/
sdk_err_t ancs_c_discovery_service(uint8_t conn_idx)
{
    ble_uuid_t ble_ancs_uuid = 
    {
        .uuid_len = BLE_ATT_UUID_128_LEN,
        .uuid = (uint8_t*)ancs_service_uuid,
    };
    return ble_gattc_prf_services_browse(s_ancs_c_env.prf_id, conn_idx, &ble_ancs_uuid);
}

/**
 *****************************************************************************************
 * @brief Handles reception of the attribute info request message.
 *
 * @param[in] conn_idx: Connection index.
 * @param[in] p_browse_srvc: Pointer to the parameters of the browse services.
 *****************************************************************************************
 */
static void ancs_c_srvc_browse_cb(uint8_t conn_idx, uint8_t status, 
                                 const ble_gattc_browse_srvc_t *p_browse_srvc)
{
    uint16_t     handle_disc;
    
    ancs_c_evt_t  ancs_c_evt;
    ancs_c_evt.conn_idx = conn_idx;
    ancs_c_evt.evt_type = BLE_ANCS_C_EVT_DISCOVERY_FAILED;
    if(p_browse_srvc->uuid_len == BLE_ATT_UUID_128_LEN)
    {
        if(memcmp(p_browse_srvc->uuid, ancs_service_uuid, BLE_ATT_UUID_128_LEN) == 0)//find ancs service
        {
            s_ancs_c_env.handles.ancs_service_handle = p_browse_srvc->start_hdl;

            for(uint16_t i=0; i<(p_browse_srvc->end_hdl - p_browse_srvc->start_hdl); i++)//find characteristic
            {
                handle_disc = p_browse_srvc->start_hdl + i + 1;
                if(p_browse_srvc->info[i].attr_type == BLE_GATTC_BROWSE_ATTR_VAL)
                {
                   if(memcmp(p_browse_srvc->info[i].attr.uuid, ancs_notification_source_uuid, BLE_ATT_UUID_128_LEN) == 0)//find ancs notification source characteristic
                   {
                        s_ancs_c_env.handles.ancs_ntf_source_handle = handle_disc;
                   } 
                   else if(memcmp(p_browse_srvc->info[i].attr.uuid, ancs_control_point_uuid, BLE_ATT_UUID_128_LEN) == 0)//find ancs control point characteristic
                   {
                        s_ancs_c_env.handles.ancs_control_point_handle = handle_disc;
                   }
                   else if(memcmp(p_browse_srvc->info[i].attr.uuid, ancs_data_source_uuid, BLE_ATT_UUID_128_LEN) == 0)//find ancs data source characteristic
                   {
                        s_ancs_c_env.handles.ancs_data_source_handle = handle_disc;
                   }
                }
                else if((p_browse_srvc->info[i].attr_type == BLE_GATTC_BROWSE_ATTR_DESC) &&   //find cccd
                        ((*(uint16_t *)(p_browse_srvc->info[i].attr.uuid)) == BLE_ATT_DESC_CLIENT_CHAR_CFG))
                {
                    if(handle_disc == (s_ancs_c_env.handles.ancs_ntf_source_handle + 1))// find notification source cccd
                    {
                        s_ancs_c_env.handles.ancs_ntf_source_cccd_handle = handle_disc;
                    }
                    else if(handle_disc == (s_ancs_c_env.handles.ancs_data_source_handle + 1))// find data source cccd
                    {
                        s_ancs_c_env.handles.ancs_data_source_cccd_handle = handle_disc;
                    }
                }
                else if (p_browse_srvc->info[i].attr_type == BLE_GATTC_BROWSE_NONE)
                {
                    break;
                }
            } 
            ancs_c_evt.evt_type = BLE_ANCS_C_EVT_DISCOVERY_CPLT;
        }
    }
    ancs_c_evt_handler_excute(&ancs_c_evt);
}

sdk_err_t ancs_c_write_control_point(uint8_t conn_idx, uint8_t *p_data, uint16_t length)
{
    gattc_write_attr_value_t write_attr_value;


    write_attr_value.handle  = s_ancs_c_env.handles.ancs_control_point_handle;
    write_attr_value.offset  = 0;
    write_attr_value.length  = length;
    write_attr_value.p_value = (uint8_t *)p_data;

    return ble_gattc_prf_write(s_ancs_c_env.prf_id, conn_idx, &write_attr_value);
}

sdk_err_t ancs_c_ntf_source_notify_set(uint8_t conn_idx, bool is_enable)
{
    gattc_write_attr_value_t write_attr_value;
    uint16_t ntf_value = is_enable ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

    if (BLE_ATT_INVALID_HDL == s_ancs_c_env.handles.ancs_ntf_source_cccd_handle)
    {
        return BLE_ATT_ERR_INVALID_HANDLE;
    }
    write_attr_value.handle  = s_ancs_c_env.handles.ancs_ntf_source_cccd_handle;
    write_attr_value.offset  = 0;
    write_attr_value.length  = 2;
    write_attr_value.p_value = (uint8_t *)&ntf_value;

    return ble_gattc_prf_write(s_ancs_c_env.prf_id, conn_idx, &write_attr_value);
}

sdk_err_t ancs_c_data_source_notify_set(uint8_t conn_idx, bool is_enable)
{
    gattc_write_attr_value_t write_attr_value;
    uint16_t ntf_value = is_enable ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

    if (BLE_ATT_INVALID_HDL == s_ancs_c_env.handles.ancs_data_source_cccd_handle)
    {
        return BLE_ATT_ERR_INVALID_HANDLE;
    }
    write_attr_value.handle  = s_ancs_c_env.handles.ancs_data_source_cccd_handle;
    write_attr_value.offset  = 0;
    write_attr_value.length  = 2;
    write_attr_value.p_value = (uint8_t *)&ntf_value;

    return ble_gattc_prf_write(s_ancs_c_env.prf_id, conn_idx, &write_attr_value);
}
