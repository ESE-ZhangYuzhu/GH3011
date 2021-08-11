/**
 *****************************************************************************************
 *
 * @file pcs.c
 *
 * @brief PCS Service Implementation.
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
******************************************************************************************
*/
#include "pcs.h"
#include "ble_prf_types.h"
#include "ble_prf_utils.h"
#include "utility.h"

/*
 * DEFINES
 *****************************************************************************************
 */
/**@brief The UUIDs of PCS characteristics. */
#define PCS_CHARACTERISTIC_TX_UUID      {0x1B, 0xD7, 0x90, 0xEC, 0xE8, 0xB9, 0x75, 0x80, 0x0A, 0x46, 0x44, 0xD3, 0x02, 0x05, 0xED, 0xA6}
#define PCS_CHARACTERISTIC_SETTING_UUID {0x1B, 0xD7, 0x90, 0xEC, 0xE8, 0xB9, 0x75, 0x80, 0x0A, 0x46, 0x44, 0xD3, 0x03, 0x05, 0xED, 0xA6}

/**@brief Macros for conversion of 128bit to 16bit UUID. */
#define ATT_128_PRIMARY_SERVICE BLE_ATT_16_TO_128_ARRAY(BLE_ATT_DECL_PRIMARY_SERVICE)
#define ATT_128_CHARACTERISTIC  BLE_ATT_16_TO_128_ARRAY(BLE_ATT_DECL_CHARACTERISTIC)
#define ATT_128_CLIENT_CHAR_CFG BLE_ATT_16_TO_128_ARRAY(BLE_ATT_DESC_CLIENT_CHAR_CFG)

/*
 * ENUMERATIONS
 *****************************************************************************************
 */
/**@brief PCS Service Attributes Indexes. */
enum pcs_attr_idx_t
{
    PCS_IDX_SVC,

    PCS_IDX_TX_CHAR,
    PCS_IDX_TX_VAL,
    PCS_IDX_TX_CFG,

    PCS_IDX_SETTING_CHAR,
    PCS_IDX_SETTING_VAL,
    PCS_IDX_SETTING_CFG,

    PCS_IDX_NB,
};

/*
 * STRUCTURES
 *****************************************************************************************
 */
/**@brief PCS Service environment variable. */
struct pcs_env_t
{
    pcs_init_t pcs_init;                               /**< PCS Service initialization variables. */
    uint16_t   start_hdl;                              /**< Start handle of services */
    uint16_t   tx_ntf_cfg[PCS_CONNECTION_MAX];         /**< TX Characteristic Notification configuration of the peers. */
    uint16_t   setting_ind_cfg[PCS_CONNECTION_MAX];    /**< Setting Characteristic Indication configuration of the peers. */
};

/*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static sdk_err_t   pcs_init(void);
static void        pcs_disconnected(uint8_t conn_idx, uint8_t disconn_reason);
static void        pcs_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param);
static void        pcs_read_att_cb(uint8_t conn_idx, const gatts_read_req_cb_t *p_param);
static void        pcs_cccd_set_cb(uint8_t conn_idx, uint16_t handle, uint16_t cccd_value);
static void        pcs_ntf_ind_cb(uint8_t conn_idx, uint8_t status, const ble_gatts_ntf_ind_t *p_ntf_ind);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static struct pcs_env_t s_pcs_env;
static const uint16_t   s_char_mask = 0xFFFF;

/**@brief Full PCS Database Description which is used to add attributes into the ATT database. */
static const attm_desc_128_t pcs_att_db[PCS_IDX_NB] =
{
    // PCS service
    [PCS_IDX_SVC]            = {ATT_128_PRIMARY_SERVICE, READ_PERM_UNSEC, 0, 0},

    // PCS TX Characteristic Declaration
    [PCS_IDX_TX_CHAR]        = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // PCS TX Characteristic Value
    [PCS_IDX_TX_VAL]         = {PCS_CHARACTERISTIC_TX_UUID, 
                                NOTIFY_PERM_UNSEC,
                                (ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET(UUID_TYPE_128)),
                                PCS_MAX_DATA_LEN},
    // PCS TX Characteristic - Client Characteristic Configuration Descriptor
    [PCS_IDX_TX_CFG]         = {ATT_128_CLIENT_CHAR_CFG,
                                READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC,
                                0,
                                0},
    
    // PCS settings
    [PCS_IDX_SETTING_CHAR] = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // PCS settings Value
    [PCS_IDX_SETTING_VAL]  = {PCS_CHARACTERISTIC_SETTING_UUID,
                              (WRITE_CMD_PERM_UNSEC | INDICATE_PERM_UNSEC),
                              (ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET(UUID_TYPE_128)),
                               PCS_MAX_DATA_LEN},
    // ths settings cfg
    [PCS_IDX_SETTING_CFG]  = {ATT_128_CLIENT_CHAR_CFG,
                              (READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC | WRITE_CMD_PERM_UNSEC),
                              0,
                              0},
};

/**@brief PCS Service interface required by profile manager. */
static ble_prf_manager_cbs_t pcs_mgr_cbs =
{
    (prf_init_func_t)pcs_init,
    NULL,
    pcs_disconnected,
};

/**@brief PCS GATT Server Callbacks. */
static gatts_prf_cbs_t pcs_gatts_cbs =
{
    pcs_read_att_cb,
    pcs_write_att_cb,
    NULL,
    pcs_ntf_ind_cb,
    pcs_cccd_set_cb
};

/**@brief PCS Server Information. */
static const prf_server_info_t pcs_prf_info =
{
    .max_connection_nb = PCS_CONNECTION_MAX,
    .manager_cbs       = &pcs_mgr_cbs,
    .gatts_prf_cbs     = &pcs_gatts_cbs
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 * @brief Initialize PCS and create DB in ATT.
 *
 * @return Error code to know if service initialization succeed or not.
 *****************************************************************************************
 */
static sdk_err_t pcs_init(void)
{
    const uint8_t     pcs_svc_uuid[] = {PCS_SERVICE_UUID};
    uint16_t          start_hdl      = PRF_INVALID_HANDLE;
    sdk_err_t         error_code;
    gatts_create_db_t gatts_db;
    
    memset(&gatts_db, 0, sizeof(gatts_db));

    gatts_db.shdl                  = &start_hdl;
    gatts_db.uuid                  = pcs_svc_uuid;
    gatts_db.attr_tab_cfg          = (uint8_t *)&s_char_mask;
    gatts_db.max_nb_attr           = PCS_IDX_NB;
    gatts_db.srvc_perm             = SRVC_UUID_TYPE_SET(UUID_TYPE_128);
    gatts_db.attr_tab_type         = SERVICE_TABLE_TYPE_128;
    gatts_db.attr_tab.attr_tab_128 = pcs_att_db;

    error_code = ble_gatts_srvc_db_create(&gatts_db);

    if (SDK_SUCCESS == error_code)
    {
        s_pcs_env.start_hdl = *gatts_db.shdl;
    }

    return error_code;
}

/**
 *****************************************************************************************
 * @brief Handles reception of the attribute info request message.
 *
 * @param[in] conn_idx: Index of the connection.
 * @param[in] p_param:  Pointer to the parameters of the read request.
 *****************************************************************************************
 */
static void   pcs_read_att_cb(uint8_t conn_idx, const gatts_read_req_cb_t *p_param)
{
    gatts_read_cfm_t cfm;
    uint16_t         handle    = p_param->handle;
    uint8_t          tab_index = 0;

    tab_index  = prf_find_idx_by_handle(handle, s_pcs_env.start_hdl, PCS_IDX_NB, (uint8_t *)&s_char_mask);
    cfm.handle = handle;
    cfm.status = BLE_SUCCESS;

    switch (tab_index)
    {
        case PCS_IDX_TX_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_pcs_env.tx_ntf_cfg[conn_idx];
            cfm.status = BLE_SUCCESS;
            break;

        case PCS_IDX_SETTING_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_pcs_env.setting_ind_cfg[conn_idx];
            cfm.status = BLE_SUCCESS;
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
 * @param[in] conn_idx: Index of the connection.
 * @param[in] p_param:  Point to the parameters of the write request.
 *****************************************************************************************
 */
static void   pcs_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param)
{
    uint8_t           handle    = p_param->handle;
    uint8_t           tab_index = 0;
    uint16_t          cccd_value;
    pcs_evt_t         event;
    gatts_write_cfm_t cfm;

    tab_index      = prf_find_idx_by_handle(handle, s_pcs_env.start_hdl, PCS_IDX_NB, (uint8_t *)&s_char_mask);
    event.conn_idx = conn_idx;
    cfm.handle     = handle;
    cfm.status     = BLE_SUCCESS;
    
    switch (tab_index)
    {

        case PCS_IDX_TX_CFG:
            cccd_value     = le16toh(&p_param->value[0]);
            event.evt_type = (PRF_CLI_START_NTF == cccd_value) ? PCS_EVT_TX_ENABLE : PCS_EVT_TX_DISABLE;
            s_pcs_env.tx_ntf_cfg[conn_idx] = cccd_value;
            break;

        case PCS_IDX_SETTING_VAL:
            event.evt_type = PCS_EVT_PARAM_SET;
            break;

        case PCS_IDX_SETTING_CFG:
            cccd_value     = le16toh(&p_param->value[0]);
            event.evt_type = (PRF_CLI_START_IND == cccd_value) ? PCS_EVT_SETTING_ENABLE : PCS_EVT_SETTING_DISABLE;
            s_pcs_env.setting_ind_cfg[conn_idx] = cccd_value;
            break;

        default:
            cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
            break;
    }

    if (BLE_ATT_ERR_INVALID_HANDLE != cfm.status && PCS_EVT_INVALID != event.evt_type && s_pcs_env.pcs_init.evt_handler)
    {
        event.conn_idx = conn_idx;
        event.p_data   = (uint8_t *)p_param->value;
        event.length   = p_param->length;

        s_pcs_env.pcs_init.evt_handler(&event);
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
static void pcs_cccd_set_cb(uint8_t conn_idx, uint16_t handle, uint16_t cccd_value)
{
    uint8_t           tab_index = 0;
    pcs_evt_t         event;

    if (!prf_is_cccd_value_valid(cccd_value))
    {
        return;
    }

    tab_index      = prf_find_idx_by_handle(handle, s_pcs_env.start_hdl, PCS_IDX_NB, (uint8_t *)&s_char_mask);
    event.conn_idx = conn_idx;
    event.evt_type = PCS_EVT_INVALID;

    switch (tab_index)
    {
        case PCS_IDX_SETTING_CFG:
            event.evt_type = (PRF_CLI_START_IND == cccd_value) ? PCS_EVT_SETTING_ENABLE : PCS_EVT_SETTING_DISABLE;
            s_pcs_env.setting_ind_cfg[conn_idx] = cccd_value;
            break;

        default:
            break;
    }

    if (PCS_EVT_INVALID != event.evt_type && s_pcs_env.pcs_init.evt_handler)
    {
        s_pcs_env.pcs_init.evt_handler(&event);
    }
}

/**
 *****************************************************************************************
 * @brief Handles reception of the disconnected event.
 *
 * @param[in] conn_idx: Index of the connection.
 *****************************************************************************************
 */
static void pcs_disconnected(uint8_t conn_idx, uint8_t disconn_reason)
{
    pcs_evt_t  event = 
    {
        .conn_idx = conn_idx,
        .evt_type = PCS_EVT_DISCONNECTED,
        .p_data   = &disconn_reason,
        .length   = sizeof(uint8_t)
    };

    if (s_pcs_env.pcs_init.evt_handler)
    {
        s_pcs_env.pcs_init.evt_handler(&event);
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
static void pcs_ntf_ind_cb(uint8_t conn_idx, uint8_t status, const ble_gatts_ntf_ind_t *p_ntf_ind)
{
    if (NULL != s_pcs_env.pcs_init.evt_handler)
    {
        pcs_evt_t event;
        event.conn_idx = conn_idx;

        if (BLE_SUCCESS == status && BLE_GATT_NOTIFICATION == p_ntf_ind->type)
        {
            event.evt_type = PCS_EVT_TX_DATA_SENT;
            s_pcs_env.pcs_init.evt_handler(&event);
        }
    }
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
sdk_err_t pcs_tx_data_send(uint8_t conn_idx, uint8_t *p_data, uint16_t length)
{
    sdk_err_t        error_code = SDK_ERR_NTF_DISABLED;
    gatts_noti_ind_t send_rsp;

    if (PRF_CLI_START_NTF == s_pcs_env.tx_ntf_cfg[conn_idx])
    {
        // Fill in the parameter structure
        send_rsp.type   = BLE_GATT_NOTIFICATION;
        send_rsp.handle = prf_find_handle_by_idx(PCS_IDX_TX_VAL, s_pcs_env.start_hdl, (uint8_t *)&s_char_mask);

        // Pack measured value in database
        send_rsp.length = length;
        send_rsp.value  = p_data;

        // Send notification to peer device
        error_code = ble_gatts_noti_ind(conn_idx, &send_rsp);
    }

    return error_code;
}

sdk_err_t pcs_setting_reply(uint8_t conn_idx, uint8_t *p_data, uint16_t length)
{
    sdk_err_t        error_code = SDK_ERR_IND_DISABLED;
    gatts_noti_ind_t send_cmd;

    if (PRF_CLI_START_IND == s_pcs_env.setting_ind_cfg[conn_idx])
    {
        // Fill in the parameter structure
        send_cmd.type   = BLE_GATT_INDICATION;
        send_cmd.handle = prf_find_handle_by_idx(PCS_IDX_SETTING_VAL, s_pcs_env.start_hdl, (uint8_t *)&s_char_mask);

        // Pack measured value in database
        send_cmd.length = length;
        send_cmd.value  = p_data;

        // Send notification to peer device
        error_code = ble_gatts_noti_ind(conn_idx, &send_cmd);
    }

    return error_code;
}

sdk_err_t pcs_service_init(pcs_init_t *p_pcs_init)
{
    if (NULL == p_pcs_init)
    {
        return SDK_ERR_POINTER_NULL;
    }

    memcpy(&s_pcs_env.pcs_init, p_pcs_init, sizeof(pcs_init_t));

    return ble_server_prf_add(&pcs_prf_info);
}
