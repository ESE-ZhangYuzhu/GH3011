/**
 *****************************************************************************************
 *
 * @file sample_profile.c
 *
 * @brief sample_profile Server Implementation.
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
#include "sample_service.h"
#include "ble_prf_types.h"
#include "ble_prf_utils.h"
#include "utility.h"

/*
 * DEFINES
 *****************************************************************************************
 */
/**@brief The UUIDs of GUS characteristics. */
#define SAMPLE_SERVER_TX_UUID      {0x1B, 0xD7, 0x90, 0xEC, 0xE8, 0xB9, 0x75, 0x80, 0x0A, 0x46, 0x44, 0xD3, 0x02, 0x01, 0xED, 0xA6}
#define SAMPLE_SERVER_RX_UUID      {0x1B, 0xD7, 0x90, 0xEC, 0xE8, 0xB9, 0x75, 0x80, 0x0A, 0x46, 0x44, 0xD3, 0x03, 0x01, 0xED, 0xA6}

/**@brief Macros for conversion of 128bit to 16bit UUID. */
#define ATT_128_PRIMARY_SERVICE     BLE_ATT_16_TO_128_ARRAY(BLE_ATT_DECL_PRIMARY_SERVICE)
#define ATT_128_CHARACTERISTIC      BLE_ATT_16_TO_128_ARRAY(BLE_ATT_DECL_CHARACTERISTIC)
#define ATT_128_CLIENT_CHAR_CFG     BLE_ATT_16_TO_128_ARRAY(BLE_ATT_DESC_CLIENT_CHAR_CFG)

/*
 * ENUMERATIONS
 *****************************************************************************************
 */
/**@brief Sample Service Attributes Indexes. */
enum samples_attr_idx_t
{
    SAMPLES_IDX_SVC,

    SAMPLES_IDX_TX_CHAR,
    SAMPLES_IDX_TX_VAL,
    SAMPLES_IDX_TX_CFG,
    SAMPLES_IDX_RX_CHAR,
    SAMPLES_IDX_RX_VAL,

    SAMPLES_IDX_NB,
};

/*
 * STRUCTURES
 *****************************************************************************************
 */
/**@brief Samples Service environment variable. */
typedef struct
{
    samples_init_t  samples_init;                            /**< Sample Service initialization variables. */
    uint16_t        start_hdl;                               /**< Service start handle. */ 
    uint16_t        tx_ntf_cfg[SAMPLES_CONNECTION_MAX];      /**< TX Character Notification configuration of peer devices. */
} samples_env_t;

/*
* LOCAL FUNCTION DECLARATION
*****************************************************************************************
*/
static sdk_err_t   samples_init(void);
static void        samples_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param);
static void        samples_read_att_cb(uint8_t conn_idx, const gatts_read_req_cb_t *p_param);
static void        samples_cccd_set_cb(uint8_t conn_idx, uint16_t handle, uint16_t cccd_value);
static void        samples_ntf_ind_cb(uint8_t conn_idx, uint8_t status, const ble_gatts_ntf_ind_t *p_ntf_ind);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static samples_env_t      s_samples_env[SAMPLES_INSTANCE_MAX];   /**< Samples service instance. */
static uint8_t            s_samples_ins_cnt = 0;                 /**< Number of Samples Server task instances. */
static uint8_t            s_now_ins_cnt = 0;
static samples_evt_type_t s_now_notify_cmp_type = SAMPLES_EVT_TX_NOTIFY_COMPLETE;
static const uint16_t     s_samples_features = 0xFFFF;

/**@brief Full SAMPLES Database Description - Used to add attributes into the database. */
static const attm_desc_128_t samples_att_db[SAMPLES_IDX_NB] =
{
    // SAMPLE service
    [SAMPLES_IDX_SVC]     = {ATT_128_PRIMARY_SERVICE, READ_PERM_UNSEC, 0, 0},

    // SAMPLE TX Characteristic Declaration
    [SAMPLES_IDX_TX_CHAR] = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // SAMPLE TX Characteristic Value
    [SAMPLES_IDX_TX_VAL]  = {SAMPLE_SERVER_TX_UUID,
                             NOTIFY_PERM_UNSEC,
                             (ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET(UUID_TYPE_128)),
                             SAMPLES_MAX_DATA_LEN },
    // SAMPLE TX Characteristic - Client Characteristic Configuration Descriptor
    [SAMPLES_IDX_TX_CFG]  = {ATT_128_CLIENT_CHAR_CFG,
                             READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC,
                             0,
                             0},

    // SAMPLE RX Characteristic Declaration
    [SAMPLES_IDX_RX_CHAR]  = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0 },
    // SAMPLE RX Characteristic Value
    [SAMPLES_IDX_RX_VAL]   = {SAMPLE_SERVER_RX_UUID,
                              WRITE_CMD_PERM_UNSEC,
                              (ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET(UUID_TYPE_128)),
                              SAMPLES_MAX_DATA_LEN},
};

/**@brief SAMPLE Server Task interface required by profile manager. */
static ble_prf_manager_cbs_t samples_tack_cbs =
{
    (prf_init_func_t) samples_init,
    NULL,
    NULL
};

/**@brief SAMPLE Server Task Callbacks. */
static gatts_prf_cbs_t samples_cb_func =
{
    samples_read_att_cb,
    samples_write_att_cb,
    NULL,
    samples_ntf_ind_cb,
    samples_cccd_set_cb
};

/**@brief SAMPLE Server Information. */
static const prf_server_info_t samples_prf_info =
{
    .max_connection_nb = SAMPLES_CONNECTION_MAX,
    .manager_cbs       = &samples_tack_cbs,
    .gatts_prf_cbs     = &samples_cb_func
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 * @brief Initialize Sample Service and create db in att
 *
 * @return Error code to know if profile initialization succeed or not.
 *****************************************************************************************
 */
static sdk_err_t samples_init(void)
{
    const uint8_t     samples_svc_uuid[] = {SAMPLES_SERVICE_UUID};
    sdk_err_t         error_code         = SDK_SUCCESS;
    uint16_t          start_hdl          = 0;
    gatts_create_db_t gatts_db;
    
    memset(&gatts_db, 0, sizeof(gatts_db));

    for (uint8_t i = 0; i < s_samples_ins_cnt; i++)
    {
        gatts_db.shdl                  = &start_hdl;
        gatts_db.uuid                  = samples_svc_uuid;
        gatts_db.attr_tab_cfg          = (uint8_t *)&s_samples_features;
        gatts_db.max_nb_attr           = SAMPLES_IDX_NB;
        gatts_db.srvc_perm             = SRVC_UUID_TYPE_SET(UUID_TYPE_128);
        gatts_db.attr_tab_type         = SERVICE_TABLE_TYPE_128;
        gatts_db.attr_tab.attr_tab_128 = samples_att_db;
        
        error_code = ble_gatts_srvc_db_create(&gatts_db);

        if (SDK_SUCCESS == error_code)
        {
            s_samples_env[i].start_hdl = *(gatts_db.shdl);
        }
        else
        {
            return error_code;
        }
    }

    return error_code;
}

/**
 *****************************************************************************************
 * @brief Handles reception of the attribute info request message.
 *
 * @param[in] conn_idx: Connection index
 * @param[in] p_param:  Pointer to the parameters of the read request.
 *****************************************************************************************
 */
static void samples_read_att_cb(uint8_t conn_idx, const gatts_read_req_cb_t *p_param)
{
    uint8_t          handle    = p_param->handle;
    uint8_t          tab_index = 0;
    uint8_t          i         = 0;
    gatts_read_cfm_t cfm;
    
    for (i = 0; i < s_samples_ins_cnt; i++)
    {
        tab_index = prf_find_idx_by_handle(handle,
                                           s_samples_env[i].start_hdl,
                                           SAMPLES_IDX_NB, 
                                           (uint8_t *)&s_samples_features);

        if (tab_index > 0)
        {
            break;
        }
    }

    cfm.handle = handle;
    cfm.status = BLE_SUCCESS;

    switch (tab_index)
    {
        case SAMPLES_IDX_TX_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value = (uint8_t *)&s_samples_env[i].tx_ntf_cfg[conn_idx];
            break;
        
        default:
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
 *****************************************************************************************
 */
static void samples_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param)
{
    uint8_t           handle     = p_param->handle;
    uint8_t           tab_index  = 0;
    uint8_t           i          = 0;
    uint16_t          cccd_value = 0;
    samples_evt_t     event;
    gatts_write_cfm_t cfm;

    cfm.handle = handle;
    cfm.status = BLE_SUCCESS;

    for (i = 0; i < s_samples_ins_cnt; i++)
    {
        tab_index = prf_find_idx_by_handle(handle,
                                           s_samples_env[i].start_hdl,
                                           SAMPLES_IDX_NB,
                                           (uint8_t *)&s_samples_features);
        if (tab_index > 0)
        {
            break;
        }
    }

    switch (tab_index)
    {
        case SAMPLES_IDX_RX_VAL:
                event.conn_idx = conn_idx;
                event.evt_type = SAMPLES_EVT_RX_RECEIVE_DATA;
            break;

        case SAMPLES_IDX_TX_CFG:
            cccd_value = le16toh(&p_param->value[0]);
            event.conn_idx = conn_idx;
            event.evt_type = (PRF_CLI_START_NTF == cccd_value) ? \
                              SAMPLES_EVT_TX_NOTIFICATION_ENABLED :\
                              SAMPLES_EVT_TX_NOTIFICATION_DISABLED;
            s_samples_env[i].tx_ntf_cfg[conn_idx] = cccd_value;
            break;

        default:
            cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
            break;
    }
    if ((BLE_ATT_ERR_INVALID_HANDLE != cfm.status) &&\
        (SAMPLES_EVT_INVALID != event.evt_type) &&\
        (s_samples_env[i].samples_init.evt_handler))
    {
        s_samples_env[i].samples_init.evt_handler(&event);
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
static void samples_cccd_set_cb(uint8_t conn_idx, uint16_t handle, uint16_t cccd_value)
{
    uint8_t           tab_index  = 0;
    uint8_t           i          = 0;
    samples_evt_t     event;

    if (!prf_is_cccd_value_valid(cccd_value))
    {
        return;
    }

    event.evt_type = SAMPLES_EVT_INVALID;
    event.conn_idx = conn_idx;

    for (i = 0; i < s_samples_ins_cnt; i++)
    {
        tab_index = prf_find_idx_by_handle(handle,
                                           s_samples_env[i].start_hdl,
                                           SAMPLES_IDX_NB,
                                           (uint8_t *)&s_samples_features);
        if (tab_index > 0)
        {
            break;
        }
    }

    switch (tab_index)
    {
        case SAMPLES_IDX_TX_CFG:
            event.conn_idx = conn_idx;
            event.evt_type = (PRF_CLI_START_NTF == cccd_value) ? \
                              SAMPLES_EVT_TX_NOTIFICATION_ENABLED :\
                              SAMPLES_EVT_TX_NOTIFICATION_DISABLED;
            s_samples_env[i].tx_ntf_cfg[conn_idx] = cccd_value;
            break;

        default:
            break;
    }
    if ((SAMPLES_EVT_INVALID != event.evt_type) &&\
        (s_samples_env[i].samples_init.evt_handler))
    {
        s_samples_env[i].samples_init.evt_handler(&event);
    }
}

/**
 *****************************************************************************************
 * @brief Handles reception of the complete event.
 *
 * @param[in] conn_idx: Connection index
 * @param[in] p_param:  Pointer to the parameters of the complete event.
 *
 * @return If the event was consumed or not.
 *****************************************************************************************
 */
static void samples_ntf_ind_cb(uint8_t conn_idx, uint8_t status, const ble_gatts_ntf_ind_t *p_ntf_ind)
{
    if (s_samples_env[s_now_ins_cnt].samples_init.evt_handler != NULL)
    {
        samples_evt_t event;
        event.conn_idx = conn_idx;

        if (status == BLE_SUCCESS)
        {
            if (BLE_GATT_NOTIFICATION == p_ntf_ind->type)
            {
                event.evt_type = s_now_notify_cmp_type;
                s_samples_env[s_now_ins_cnt].samples_init.evt_handler(&event);
            }
        }
    }
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
sdk_err_t samples_notify_tx_data(uint8_t conn_idx, uint8_t ins_idx, uint8_t *p_data, uint16_t length)
{
    sdk_err_t        error_code = SDK_ERR_NTF_DISABLED;
    gatts_noti_ind_t send_cmd;

    if (PRF_CLI_START_NTF == s_samples_env[ins_idx].tx_ntf_cfg[conn_idx])
    {
        if (ins_idx <= s_samples_ins_cnt)
        {
            // Fill in the parameter structure
            send_cmd.type   = BLE_GATT_NOTIFICATION;
            send_cmd.handle = prf_find_handle_by_idx(SAMPLES_IDX_TX_VAL,
                                                     s_samples_env[ins_idx].start_hdl,
                                                     (uint8_t *)&s_samples_features);
            // pack measured value in database
            send_cmd.length       = length;
            send_cmd.value        = p_data;
            s_now_ins_cnt         = ins_idx;
            s_now_notify_cmp_type = SAMPLES_EVT_TX_NOTIFY_COMPLETE;
            // send notification to peer device
            error_code = ble_gatts_noti_ind(conn_idx, &send_cmd);
        }
    }

    return error_code;
}


sdk_err_t samples_service_init(samples_init_t samples_init[], uint8_t ins_num)
{
    if (ins_num > SAMPLES_INSTANCE_MAX)
    {
        return SDK_ERR_INVALID_PARAM;
    }

    for (uint8_t i = 0; i < ins_num; i++)
    {
        memcpy(&s_samples_env[i].samples_init, &samples_init[i], sizeof(samples_init_t));
    }

    s_samples_ins_cnt = ins_num;
    return ble_server_prf_add(&samples_prf_info);
}
