/**
 *****************************************************************************************
 *
 * @file ths.c
 *
 * @brief Throughput Server Implementation.
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
#include "ths.h"
#include "ble_prf_types.h"
#include "ble_prf_utils.h"
#include "utility.h"

/*
* DEFINES
*****************************************************************************************
*/
#define SETTINGS_CHAR_VALUE_LEN           10   /**< Maximum length of the value of Settings characteristic. */
#define THS_TX_CHARACTERISTIC_UUID        {0x1B, 0xD7, 0x90, 0xEC, 0xE8, 0xB9, 0x75, 0x80, 0x0A, 0x46, 0x44, 0xD3, 0x02, 0x03, 0xED, 0xA6}
#define THS_RX_CHARACTERISTIC_UUID        {0x1B, 0xD7, 0x90, 0xEC, 0xE8, 0xB9, 0x75, 0x80, 0x0A, 0x46, 0x44, 0xD3, 0x03, 0x03, 0xED, 0xA6}
#define THS_SETTINGS_CHARACTERISTIC_UUID  {0x1B, 0xD7, 0x90, 0xEC, 0xE8, 0xB9, 0x75, 0x80, 0x0A, 0x46, 0x44, 0xD3, 0x04, 0x03, 0xED, 0xA6}
#define THS_TOGGLE_CHARACTERISTIC_UUID    {0x1B, 0xD7, 0x90, 0xEC, 0xE8, 0xB9, 0x75, 0x80, 0x0A, 0x46, 0x44, 0xD3, 0x05, 0x03, 0xED, 0xA6}

/**@brief Macros for conversion of 128bit to 16bit UUID. */
#define ATT_128_PRIMARY_SERVICE     BLE_ATT_16_TO_128_ARRAY(BLE_ATT_DECL_PRIMARY_SERVICE)
#define ATT_128_CHARACTERISTIC      BLE_ATT_16_TO_128_ARRAY(BLE_ATT_DECL_CHARACTERISTIC)
#define ATT_128_CLIENT_CHAR_CFG     BLE_ATT_16_TO_128_ARRAY(BLE_ATT_DESC_CLIENT_CHAR_CFG)

/*
 * ENUMERATIONS
 *****************************************************************************************
 */
/**@brief Throughput Service Attributes Indexes. */
enum ths_attr_idx_t
{
    THS_IDX_SVC,

    THS_IDX_TX_CHAR,
    THS_IDX_TX_VAL,
    THS_IDX_TX_CFG,

    THS_IDX_RX_CHAR,
    THS_IDX_RX_VAL,

    THS_IDX_SETTINGS_CHAR,
    THS_IDX_SETTINGS_VAL,
    THS_IDX_SETTINGS_CFG,

    THS_IDX_TOGGLE_CHAR,
    THS_IDX_TOGGLE_VAL,

    THS_IDX_NB,
};

/*
 * STRUCTURES
 *****************************************************************************************
 */
/**@brief Throughput service environment variable. */
struct ths_env_t
{
    ths_init_t    ths_init;                             /**< Throughput Service initialization variables. */
    uint16_t      start_hdl;                            /**< Service start handle. */
    uint16_t      data_ntf_cfg[THS_CONNECTION_MAX];     /**< Notification configuration for sending the data. */
    uint16_t      settings_ntf_cfg[THS_CONNECTION_MAX]; /**< Notification configuration for notifying the settings change. */
};

/*
* LOCAL FUNCTION DECLARATION
*****************************************************************************************
*/
static sdk_err_t   ths_init(void);
static void        ths_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param);
static void        ths_read_att_cb(uint8_t conn_idx, const gatts_read_req_cb_t *p_param);
static void        ths_gatts_cmpl_cb(uint8_t conn_idx, uint8_t status, const ble_gatts_ntf_ind_t *p_ntf_ind);
static void        ths_cccd_set_cb(uint8_t conn_idx, uint16_t handle, uint16_t cccd_value);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static struct ths_env_t s_ths_env;
static uint8_t          s_noti_cfg_idx;
static const uint16_t   s_char_mask = 0x07FF;

/**@brief Full THS Database Description - Used to add attributes into the database. */
static const attm_desc_128_t ths_attr_tab[THS_IDX_NB] =
{
    // ths service
    [THS_IDX_SVC]           = {ATT_128_PRIMARY_SERVICE, READ_PERM_UNSEC, 0, 0},

    // ths tx Declaration
    [THS_IDX_TX_CHAR]       = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // ths tx Value
    [THS_IDX_TX_VAL]        = {THS_TX_CHARACTERISTIC_UUID,
                               (NOTIFY_PERM_UNSEC),
                               (ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET(UUID_TYPE_128)),
                               THS_MAX_DATA_LEN},
    // ths tx Characteristic Configuration
    [THS_IDX_TX_CFG]        = {ATT_128_CLIENT_CHAR_CFG,
                               (READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC | WRITE_CMD_PERM_UNSEC),
                               0,
                               0},

    // ths rx
    [THS_IDX_RX_CHAR]       = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // ths rx Value
    [THS_IDX_RX_VAL]        = {THS_RX_CHARACTERISTIC_UUID,
                               WRITE_CMD_PERM_UNSEC,
                               (ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET(UUID_TYPE_128)),
                               THS_MAX_DATA_LEN},

    // ths settings
    [THS_IDX_SETTINGS_CHAR] = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // ths settings Value
    [THS_IDX_SETTINGS_VAL]  = {THS_SETTINGS_CHARACTERISTIC_UUID,
                               (WRITE_CMD_PERM_UNSEC | NOTIFY_PERM_UNSEC),
                               (ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET(UUID_TYPE_128)),
                               SETTINGS_CHAR_VALUE_LEN},
    // ths settings cfg
    [THS_IDX_SETTINGS_CFG]  = {ATT_128_CLIENT_CHAR_CFG,
                               (READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC | WRITE_CMD_PERM_UNSEC),
                               0,
                               0},

    // ths toggle
    [THS_IDX_TOGGLE_CHAR]   = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // ths toggle Value
    [THS_IDX_TOGGLE_VAL]    = {THS_TOGGLE_CHARACTERISTIC_UUID,
                               WRITE_CMD_PERM_UNSEC,
                               (ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET(UUID_TYPE_128)),
                               sizeof(uint8_t)},
};

/**@brief Throughput Service interface required by profile manager. */
static ble_prf_manager_cbs_t ths_mgr_cbs =
{
    (prf_init_func_t)ths_init,
    NULL,
    NULL
};

/**@brief Throughput GATT Server Callbacks. */
static gatts_prf_cbs_t ths_gatts_cbs =
{
    ths_read_att_cb,
    ths_write_att_cb,
    NULL,
    ths_gatts_cmpl_cb,
    ths_cccd_set_cb
};

/**@brief Throughput Service Information. */
static const prf_server_info_t ths_prf_info =
{
    .max_connection_nb = THS_CONNECTION_MAX,
    .manager_cbs       = &ths_mgr_cbs,
    .gatts_prf_cbs     = &ths_gatts_cbs
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 *@brief Initialize throughput service, and create database in ATT.
 *
 * @return Error code to know if profile initialization succeed or not.
 *****************************************************************************************
 */
static sdk_err_t ths_init(void)
{
    const uint8_t     ths_svc_uuid[] = {THS_SERVICE_UUID};
    uint16_t          start_hdl = PRF_INVALID_HANDLE;
    sdk_err_t         error_code;
    gatts_create_db_t gatts_db;

    memset(&gatts_db, 0, sizeof(gatts_db));

    gatts_db.shdl                  = &start_hdl;
    gatts_db.uuid                  = ths_svc_uuid;
    gatts_db.attr_tab_cfg          = (uint8_t *)&s_char_mask;
    gatts_db.max_nb_attr           = THS_IDX_NB;
    gatts_db.srvc_perm             = SRVC_UUID_TYPE_SET(UUID_TYPE_128);
    gatts_db.attr_tab_type         = SERVICE_TABLE_TYPE_128;
    gatts_db.attr_tab.attr_tab_128 = ths_attr_tab;
 
    error_code = ble_gatts_srvc_db_create(&gatts_db);

    if (SDK_SUCCESS == error_code)
    {
        s_ths_env.start_hdl = *gatts_db.shdl;
    }

    return error_code;
}

/**
 *****************************************************************************************
 * @brief Handles reception of the attribute info request message.
 *
 * @param[in] conn_idx: Connection index.
 * @param[in] p_param:  Pointer to the parameters of the read request.
 *****************************************************************************************
 */
static void   ths_read_att_cb(uint8_t conn_idx, const gatts_read_req_cb_t *p_param)
{
    gatts_read_cfm_t cfm;
    uint8_t          handle    = p_param->handle;
    uint8_t          tab_index = 0;

    tab_index  = prf_find_idx_by_handle(handle, s_ths_env.start_hdl, THS_IDX_NB, (uint8_t *)&s_char_mask);
    cfm.handle = handle;
    cfm.status = BLE_SUCCESS;

    switch (tab_index)
    {
        case THS_IDX_TX_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_ths_env.data_ntf_cfg[conn_idx];
            break;

        case THS_IDX_SETTINGS_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_ths_env.settings_ntf_cfg[conn_idx];
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
 * @param[in] conn_idx: Connection index
 * @param[in] p_param:  Pointer to the parameters of the write request.
 *
 * @return If the request was consumed or not.
 *****************************************************************************************
 */
static void   ths_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param)
{
    uint8_t           handle    = p_param->handle;
    uint8_t           tab_index = 0;
    ths_evt_t         event;
    gatts_write_cfm_t cfm;

    tab_index  = prf_find_idx_by_handle(handle, s_ths_env.start_hdl, THS_IDX_NB, (uint8_t *)&s_char_mask);
    cfm.handle = handle;
    cfm.status = BLE_SUCCESS;

    event.conn_idx = conn_idx;
    event.evt_type = THS_EVT_INVALID;

    switch (tab_index)
    {
        case THS_IDX_TX_CFG:
            s_ths_env.data_ntf_cfg[conn_idx] = le16toh(&p_param->value[0]);
            break;

        case THS_IDX_SETTINGS_CFG:
            s_ths_env.settings_ntf_cfg[conn_idx] = le16toh(&p_param->value[0]);
            break;

        case THS_IDX_RX_VAL:
            event.evt_type = THS_EVT_DATA_RECEIVED;
            break;

        case THS_IDX_SETTINGS_VAL:
            event.evt_type = THS_EVT_SETTINGS_CHANGED;
            if (THS_SETTINGS_TYPE_TRANS_MODE == p_param->value[0])
            {
                s_ths_env.ths_init.transport_mode = (ths_transport_mode_t)p_param->value[1];
            }
            break;

        case THS_IDX_TOGGLE_VAL:
            event.evt_type = THS_EVT_TOGGLE_SET;
            break;

        default:
            cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
            break;
    }

    if (BLE_ATT_ERR_INVALID_HANDLE != cfm.status && THS_EVT_INVALID != event.evt_type && s_ths_env.ths_init.evt_handler)
    {
        event.length   = p_param->length;
        event.p_data   = (uint8_t *)p_param->value;
        s_ths_env.ths_init.evt_handler(&event);
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
static void ths_cccd_set_cb(uint8_t conn_idx, uint16_t handle, uint16_t cccd_value)
{
    uint8_t  tab_index = 0;

    if (!prf_is_cccd_value_valid(cccd_value))
    {
        return;
    }

    tab_index  = prf_find_idx_by_handle(handle, s_ths_env.start_hdl, THS_IDX_NB, (uint8_t *)&s_char_mask);

    switch (tab_index)
    {
        case THS_IDX_TX_CFG:
            s_ths_env.data_ntf_cfg[conn_idx] = cccd_value;
            break;

        case THS_IDX_SETTINGS_CFG:
            s_ths_env.settings_ntf_cfg[conn_idx] = cccd_value;
            break;

        default:
            break;
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
static void ths_gatts_cmpl_cb(uint8_t conn_idx, uint8_t status, const ble_gatts_ntf_ind_t *p_ntf_ind)
{
    if (s_ths_env.ths_init.evt_handler && SDK_SUCCESS == status)
    {
        if (BLE_GATT_NOTIFICATION == p_ntf_ind->type && THS_IDX_TX_VAL == s_noti_cfg_idx)
        {
            ths_evt_t event;
            event.conn_idx  = conn_idx;
            event.evt_type  = THS_EVT_DATA_SENT;
            s_ths_env.ths_init.evt_handler(&event);
        }
    }
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
sdk_err_t ths_data_send(uint8_t conn_idx, uint8_t *p_data, uint16_t length)
{
    sdk_err_t   error_code = SDK_ERR_NTF_DISABLED;

    if (PRF_CLI_START_NTF == s_ths_env.data_ntf_cfg[conn_idx])
    {
            gatts_noti_ind_t send_cmd;

            // Fill in the parameter structure
            send_cmd.type   = BLE_GATT_NOTIFICATION;
            send_cmd.handle = prf_find_handle_by_idx(THS_IDX_TX_VAL, s_ths_env.start_hdl, (uint8_t *)&s_char_mask);
            // Pack measured value in database
            send_cmd.length = length;
            send_cmd.value  = p_data;
            // Send notification to peer device
            error_code      = ble_gatts_noti_ind(conn_idx, &send_cmd);
            s_noti_cfg_idx  = THS_IDX_TX_VAL;
    }

    return error_code;
}

sdk_err_t ths_settings_notify(uint8_t conn_idx, uint8_t *p_settings, uint16_t length)
{
    sdk_err_t   error_code = SDK_ERR_NTF_DISABLED;

    if (PRF_CLI_START_NTF == s_ths_env.settings_ntf_cfg[conn_idx])
    {
        gatts_noti_ind_t send_cmd;

        // Fill in the parameter structure
        send_cmd.type   = BLE_GATT_NOTIFICATION;
        send_cmd.handle = prf_find_handle_by_idx(THS_IDX_SETTINGS_VAL, s_ths_env.start_hdl, (uint8_t *)&s_char_mask);

        // Pack measured value in database
        send_cmd.length = length;
        send_cmd.value  = p_settings;

        // Send notification to peer device
        error_code       = ble_gatts_noti_ind(conn_idx, &send_cmd);
        s_noti_cfg_idx  = THS_IDX_SETTINGS_VAL;
    }

    return error_code;
}

ths_transport_mode_t ths_transport_mode_get(void)
{
    return s_ths_env.ths_init.transport_mode;
}

sdk_err_t ths_service_init(ths_init_t *p_ths_init)
{
    if (NULL == p_ths_init)
    {
        return SDK_ERR_POINTER_NULL;
    }

    memcpy(&s_ths_env.ths_init, p_ths_init, sizeof(ths_init_t));

    return ble_server_prf_add(&ths_prf_info);
}

