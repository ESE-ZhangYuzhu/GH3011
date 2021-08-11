/**
 *****************************************************************************************
 *
 * @file wechat.c
 *
 * @briefWeChat Service API Implementation.
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
#include "wechat.h"
#include "ble_prf_types.h"
#include "ble_prf_utils.h"
#include "utility.h"

/*
 * ENUMERATIONS
 *****************************************************************************************
 */
/**@brief WeChat Service Attributes Indexes. */
enum wechat_attr_idx_t
{
    WECHAT_IDX_SVC,

    WECHAR_IDX_AIRSYNC_WRITE_CHAR,
    WECHAR_IDX_AIRSYNC_WRITE_VAL,

    WECHAR_IDX_AIRSYNC_INDICATE_CHAR,
    WECHAR_IDX_AIRSYNC_INDICATE_VAL,
    WECHAR_IDX_AIRSYNC_INDICATE_CFG,

    WECHAR_IDX_AIRSYNC_READ_CHAR,
    WECHAR_IDX_AIRSYNC_READ_VAL,

    WECHAT_IDX_PEDO_MEAS_CHAR,
    WECHAT_IDX_PEDO_MEAS_VAL,
    WECHAT_IDX_PEDO_MEAS_CFG,

    WECHAT_IDX_TARGET_CHAR,
    WECHAT_IDX_TARGET_VAL,
    WECHAT_IDX_TARGET_CFG,

    WECHAT_IDX_NB,
};

/*
 * STRUCTURES
 *****************************************************************************************
 */
/**@brief WeChat service environment variable. */
struct wechat_env_t
{
    wechat_evt_handler_t  evt_handler;                                  /**< WeChat Service event handler. */
    uint16_t              start_hdl;                                    /**< WeChat Service start handle. */
    wechat_pedo_meas_t    pedo_meas;                                    /**< WeChat pedometer measurement value. */
    wechat_pedo_target_t  pedo_target;                                  /**< WeChat pedometer target value. */
    uint8_t               dev_mac[GAP_ADDR_LEN];                       /**< WeChat device mac address. */
    uint16_t              airsync_ind_cfg[WECHAT_CONNECTION_MAX];       /**< Indication configuration for Airsync. */
    uint16_t              pedo_meas_ntf_cfg[WECHAT_CONNECTION_MAX];     /**< Notification configuration for pedometer measurement. */
    uint16_t              pedo_target_ind_cfg[WECHAT_CONNECTION_MAX];   /**< Indication configuration for pedometer target. */
};

/*
* LOCAL FUNCTION DECLARATION
*****************************************************************************************
*/
static sdk_err_t wechat_init(void);
static void      wechat_read_att_cb(uint8_t conn_idx, const gatts_read_req_cb_t *p_param);
static void      wechat_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param);
static void      wechat_gatts_cmpl_cb(uint8_t conn_idx, uint8_t status, const ble_gatts_ntf_ind_t *p_ntf_ind);
static void      wechat_cccd_set_cb(uint8_t conn_idx, uint16_t handle, uint16_t cccd_value);
static uint8_t   wechat_pedo_meas_encode(uint8_t *p_buff, wechat_pedo_meas_t *p_pedo_meas);
static uint8_t   wechat_pedo_target_encode(uint8_t *p_buff, wechat_pedo_target_t *p_pedo_target);
static void      wechat_pedo_target_set(const uint8_t *p_data, uint8_t length);
static sdk_err_t wechat_indicate_data_chunk(uint8_t conn_idx);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static struct wechat_env_t  s_wechat_env;
static wechat_data_t        s_ind_data;
static const uint16_t       s_char_mask = 0x3FFF;

static const attm_desc_t wechat_attr_tab[WECHAT_IDX_NB] =
{
    [WECHAT_IDX_SVC]          = {BLE_ATT_DECL_PRIMARY_SERVICE, READ_PERM_UNSEC, 0, 0},

    [WECHAR_IDX_AIRSYNC_WRITE_CHAR] = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    [WECHAR_IDX_AIRSYNC_WRITE_VAL]  = {WECHAT_WRITE_CHAR_UUID, WRITE_REQ_PERM_UNSEC, ATT_VAL_LOC_USER, WECHAT_DATA_LEN},

    [WECHAR_IDX_AIRSYNC_INDICATE_CHAR] = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    [WECHAR_IDX_AIRSYNC_INDICATE_VAL]  = {WECHAT_INDICATE_CHAR_UUID, INDICATE_PERM_UNSEC, ATT_VAL_LOC_USER, WECHAT_DATA_LEN},
    [WECHAR_IDX_AIRSYNC_INDICATE_CFG]  = {BLE_ATT_DESC_CLIENT_CHAR_CFG, READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC, 0, 0},

    [WECHAR_IDX_AIRSYNC_READ_CHAR] = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    [WECHAR_IDX_AIRSYNC_READ_VAL]  = {WECHAT_READ_CHAR_UUID, READ_PERM_UNSEC, ATT_VAL_LOC_USER, 6},

    [WECHAT_IDX_PEDO_MEAS_CHAR]    = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    [WECHAT_IDX_PEDO_MEAS_VAL]     = {WECHAT_PEDOMETER_MEASUREMENT, READ_PERM_UNSEC | NOTIFY_PERM_UNSEC, ATT_VAL_LOC_USER, WECHAT_DATA_LEN},
    [WECHAT_IDX_PEDO_MEAS_CFG]     = {BLE_ATT_DESC_CLIENT_CHAR_CFG, READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC, 0, 0},

    [WECHAT_IDX_TARGET_CHAR]  = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    [WECHAT_IDX_TARGET_VAL]   = {WECHAT_TARGET, READ_PERM_UNSEC | INDICATE_PERM_UNSEC | WRITE_REQ_PERM_UNSEC, ATT_VAL_LOC_USER, WECHAT_DATA_LEN},
    [WECHAT_IDX_TARGET_CFG]   = {BLE_ATT_DESC_CLIENT_CHAR_CFG, READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC, 0, 0},
};

/**@brief WeChat interface required by profile manager. */
static ble_prf_manager_cbs_t wechat_mgr_cbs =
{
    (prf_init_func_t)wechat_init,
    NULL,
    NULL
};

/**@brief WeChat GATT server Callbacks. */
static gatts_prf_cbs_t wechat_gatts_cbs =
{
    wechat_read_att_cb,
    wechat_write_att_cb,
    NULL,
    wechat_gatts_cmpl_cb,
    wechat_cccd_set_cb
};

/**@brief WeChat Information. */
static const prf_server_info_t wechat_prf_info =
{
    .max_connection_nb = WECHAT_CONNECTION_MAX,
    .manager_cbs       = &wechat_mgr_cbs,
    .gatts_prf_cbs     = &wechat_gatts_cbs
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 */
/**
 *****************************************************************************************
 *@brief Initialize WeChat service and create database in ATT.
 *
 * @return Error code to know if profile initialization succeed or not.
 *****************************************************************************************
 */
static sdk_err_t wechat_init(void)
{
    // The start hanlde must be set with PRF_INVALID_HANDLE to be allocated automatically by BLE Stack.
    uint16_t          start_hdl         = PRF_INVALID_HANDLE;
    const uint8_t     wechat_svc_uuid[] = BLE_ATT_16_TO_16_ARRAY(WECHAT_SERVICE_UUID);
    sdk_err_t         error_code;
    gatts_create_db_t gatts_db;

    memset(&gatts_db, 0, sizeof(gatts_db));

    gatts_db.shdl                 = &start_hdl;
    gatts_db.uuid                 = wechat_svc_uuid;
    gatts_db.attr_tab_cfg         = (uint8_t *)&s_char_mask;
    gatts_db.max_nb_attr          = WECHAT_IDX_NB;
    gatts_db.srvc_perm            = 0;
    gatts_db.attr_tab_type        = SERVICE_TABLE_TYPE_16;
    gatts_db.attr_tab.attr_tab_16 = wechat_attr_tab;

    error_code = ble_gatts_srvc_db_create(&gatts_db);

    if (SDK_SUCCESS == error_code)
    {
        s_wechat_env.start_hdl = *gatts_db.shdl;
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
static void wechat_read_att_cb(uint8_t conn_idx, const gatts_read_req_cb_t *p_param)
{
    gatts_read_cfm_t cfm;
    uint8_t          encode_data[WECHAT_DATA_LEN];
    uint8_t          handle = p_param->handle;
    uint8_t          tab_index = prf_find_idx_by_handle(handle,
                                                        s_wechat_env.start_hdl,
                                                        WECHAT_IDX_NB,
                                                        (uint8_t *)&s_char_mask);

    cfm.handle = handle;
    cfm.status = BLE_SUCCESS;

    switch (tab_index)
    {
        case WECHAR_IDX_AIRSYNC_INDICATE_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_wechat_env.airsync_ind_cfg[conn_idx];
            break;

        case WECHAR_IDX_AIRSYNC_READ_VAL:
            cfm.length = GAP_ADDR_LEN;
            cfm.value  = s_wechat_env.dev_mac;
            break;

        case WECHAT_IDX_PEDO_MEAS_VAL:
            cfm.length = wechat_pedo_meas_encode(encode_data, &s_wechat_env.pedo_meas);
            cfm.value  = encode_data;
            break;                   

        case WECHAT_IDX_PEDO_MEAS_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_wechat_env.pedo_meas_ntf_cfg[conn_idx];
            break;
       
        case WECHAT_IDX_TARGET_VAL:
            cfm.length = wechat_pedo_target_encode(encode_data, &s_wechat_env.pedo_target);
            cfm.value  = encode_data;
            break; 

        case WECHAT_IDX_TARGET_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_wechat_env.pedo_target_ind_cfg[conn_idx];
            break;

        default:
            cfm.length = 0;
            cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
            break;
    }

    ble_gatts_read_cfm(conn_idx, &cfm);
}

/**
 *******************************************************************************
 * @brief Handles reception of the write request.
 *
 * @param[in] conn_idx:  Connection index.
 * @param[in] p_param:   Pointer to the parameters of the write request.
 *******************************************************************************
 */
static void wechat_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param)
{
    uint16_t             handle     = p_param->handle;
    uint16_t             cccd_value = 0;
    uint8_t              tab_index  = 0;
    wechat_evt_t         event;
    gatts_write_cfm_t    cfm;

    tab_index  = prf_find_idx_by_handle(handle, 
                                        s_wechat_env.start_hdl,
                                        WECHAT_IDX_NB,
                                        (uint8_t *)&s_char_mask);
    cfm.handle = handle;
    cfm.status = BLE_SUCCESS;

    event.conn_idx = conn_idx;
    event.evt_type = WECHAT_EVT_INVALID;

    switch (tab_index)
    {
        case WECHAR_IDX_AIRSYNC_WRITE_VAL:
            event.evt_type          = WECHAT_EVT_AIRSYNC_DATA_RECIEVE;
            event.param.data.p_data = p_param->value;
            event.param.data.length = p_param->length;
            break;

        case WECHAR_IDX_AIRSYNC_INDICATE_CFG:
            cccd_value     = le16toh(&p_param->value[0]);
            event.evt_type = (PRF_CLI_START_IND == cccd_value ?\
                              WECHAT_EVT_AIRSYNC_IND_ENABLE :\
                              WECHAT_EVT_AIRSYNC_IND_DISABLE);
            s_wechat_env.airsync_ind_cfg[conn_idx] = cccd_value;
            break;

        case WECHAT_IDX_PEDO_MEAS_CFG:
            cccd_value     = le16toh(&p_param->value[0]);
            event.evt_type = (PRF_CLI_START_NTF == cccd_value ?\
                              WECHAT_EVT_PEDO_MEAS_NTF_ENABLE :\
                              WECHAT_EVT_PEDO_MEAS_NTF_DISABLE);
            s_wechat_env.pedo_meas_ntf_cfg[conn_idx] = cccd_value;
            break;  
     
        case WECHAT_IDX_TARGET_VAL:
            wechat_pedo_target_set(p_param->value, p_param->length);
            event.evt_type          = WECHAT_EVT_PEDO_TARGET_UPDATE;
            memcpy(&event.param.pedo_target, &s_wechat_env.pedo_target, sizeof(wechat_pedo_target_t));
            break;

        case WECHAT_IDX_TARGET_CFG:
            cccd_value     = le16toh(&p_param->value[0]);
            event.evt_type = (PRF_CLI_START_IND == cccd_value ?\
                              WECHAT_EVT_PEDO_TARGET_IND_ENABLE :\
                              WECHAT_EVT_PEDO_TARGET_IND_DISABLE);
            s_wechat_env.pedo_target_ind_cfg[conn_idx] = cccd_value;
            break;

        default:
            cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
            break;
    }

    ble_gatts_write_cfm(conn_idx, &cfm);

    if (WECHAT_EVT_INVALID != event.evt_type &&  s_wechat_env.evt_handler)
    {
        s_wechat_env.evt_handler(&event);
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
static void wechat_cccd_set_cb(uint8_t conn_idx, uint16_t handle, uint16_t cccd_value)
{
    uint8_t      tab_index = 0;
    wechat_evt_t event;

    event.conn_idx = conn_idx;
    event.evt_type = WECHAT_EVT_INVALID;

    if (!prf_is_cccd_value_valid(cccd_value))
    {
        return;
    }

    tab_index = prf_find_idx_by_handle(handle, s_wechat_env.start_hdl,
                                       WECHAT_IDX_NB,
                                       (uint8_t *)&s_char_mask);

    switch (tab_index)
    {
        case WECHAR_IDX_AIRSYNC_INDICATE_CFG:
            event.evt_type = (PRF_CLI_START_IND == cccd_value ?\
                              WECHAT_EVT_AIRSYNC_IND_ENABLE :\
                              WECHAT_EVT_AIRSYNC_IND_DISABLE);
            s_wechat_env.airsync_ind_cfg[conn_idx] = cccd_value;
            break;

        case WECHAT_IDX_PEDO_MEAS_CFG:
            event.evt_type = (PRF_CLI_START_NTF == cccd_value ?\
                              WECHAT_EVT_PEDO_MEAS_NTF_ENABLE :\
                              WECHAT_EVT_PEDO_MEAS_NTF_DISABLE);
            s_wechat_env.pedo_meas_ntf_cfg[conn_idx] = cccd_value;
            break;  

        case WECHAT_IDX_TARGET_CFG:
            event.evt_type = (PRF_CLI_START_IND == cccd_value ?\
                              WECHAT_EVT_PEDO_TARGET_IND_ENABLE :\
                              WECHAT_EVT_PEDO_TARGET_IND_DISABLE);
            s_wechat_env.pedo_target_ind_cfg[conn_idx] = cccd_value;
            break;

        default:
            break;
    }

    if (WECHAT_EVT_INVALID != event.evt_type &&  s_wechat_env.evt_handler)
    {
        s_wechat_env.evt_handler(&event);
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
static void wechat_gatts_cmpl_cb(uint8_t conn_idx, uint8_t status, const ble_gatts_ntf_ind_t *p_ntf_ind)
{
    uint8_t tab_index;

    tab_index = prf_find_idx_by_handle(p_ntf_ind->handle, s_wechat_env.start_hdl,
                                       WECHAT_IDX_NB,
                                       (uint8_t *)&s_char_mask);

    if (WECHAR_IDX_AIRSYNC_INDICATE_VAL == tab_index)
    {
        wechat_indicate_data_chunk(conn_idx);
    }
}

/**
 *****************************************************************************************
 * @brief Encode WeChat pedometer measurement value.
 *
 * @param[in] p_buff:       Pointer to encode buffer.
 * @param[in] p_pedo_meas:  Pointer to pedometer measurement value.
 *
 * @return Length of encoded.
 *****************************************************************************************
 */
static uint8_t wechat_pedo_meas_encode(uint8_t *p_buff, wechat_pedo_meas_t *p_pedo_meas)
{
    uint8_t encode_length = 0;

    p_buff[encode_length++] = p_pedo_meas->flag;

    // Encode step count
    if (p_pedo_meas->flag & WECHAT_PEDO_FLAG_STEP_COUNT_BIT)
    {
        p_buff[encode_length++] = p_pedo_meas->step_count[0];
        p_buff[encode_length++] = p_pedo_meas->step_count[1];
        p_buff[encode_length++] = p_pedo_meas->step_count[2];
    }

    // Encode step distance
    if (p_pedo_meas->flag & WECHAT_PEDO_FLAG_STEP_DISTENCE_BIT)
    {
        p_buff[encode_length++] = p_pedo_meas->step_dist[0];
        p_buff[encode_length++] = p_pedo_meas->step_dist[1];
        p_buff[encode_length++] = p_pedo_meas->step_dist[2];
    }

    // Encode step calorie
    if (p_pedo_meas->flag & WECHAT_PEDO_FLAG_STEP_CALORIE_BIT)
    {
        p_buff[encode_length++] = p_pedo_meas->step_calorie[0];
        p_buff[encode_length++] = p_pedo_meas->step_calorie[1];
        p_buff[encode_length++] = p_pedo_meas->step_calorie[2];
    }

    return encode_length;
}

/**
 *****************************************************************************************
 * @brief Encode WeChat pedometer target value.
 *
 * @param[in] p_buff:       Pointer to encode buffer.
 * @param[in] p_pedo_meas:  Pointer to pedometer target value.
 *
 * @return Length of encoded.
 *****************************************************************************************
 */
static uint8_t wechat_pedo_target_encode(uint8_t *p_buff, wechat_pedo_target_t *p_pedo_target)
{
    uint8_t encode_length = 0;

    if (p_pedo_target->flag & WECHAT_PEDO_FLAG_STEP_COUNT_BIT)
    {
        p_buff[encode_length++] = p_pedo_target->flag;
        p_buff[encode_length++] = p_pedo_target->step_count[0];
        p_buff[encode_length++] = p_pedo_target->step_count[1];
        p_buff[encode_length++] = p_pedo_target->step_count[2];
    }

    return encode_length;
}

/**
 *****************************************************************************************
 * @brief Set wechat pedometer target value.
 *
 * @param[in] p_data:  Pointer to data.
 * @param[in] length:  Length to data.
 *****************************************************************************************
 */
static void wechat_pedo_target_set(const uint8_t *p_data, uint8_t length)
{
    if (p_data[0] & WECHAT_PEDO_FLAG_STEP_COUNT_BIT && WECHAT_PEDO_TARGET_VAL_LEN == length)
    {
        s_wechat_env.pedo_target.flag          = WECHAT_PEDO_FLAG_STEP_COUNT_BIT;
        s_wechat_env.pedo_target.step_count[0] = p_data[1];
        s_wechat_env.pedo_target.step_count[1] = p_data[2];
        s_wechat_env.pedo_target.step_count[2] = p_data[3];
    }
    else
    {
        return;
    }
}

/**
 *****************************************************************************************
 * @brief Handle WeChat Airsync data indicate.
 *
 * @param[in] conn_idx: Connection index.
 *
 * @return Result of handle.
 *****************************************************************************************
 */
static sdk_err_t wechat_indicate_data_chunk(uint8_t conn_idx)
{
    uint16_t         chunk_len = 0;
    gatts_noti_ind_t wechat_ind;
    sdk_err_t        error_code;

    chunk_len = s_ind_data.length - s_ind_data.offset;
    chunk_len = chunk_len > WECHAT_DATA_LEN ? WECHAT_DATA_LEN : chunk_len;

    if (0 == chunk_len)
    {
        s_ind_data.p_data = NULL;
        s_ind_data.length = 0;
        s_ind_data.offset = 0;

        return SDK_SUCCESS;
    }

    wechat_ind.type   = BLE_GATT_INDICATION;
    wechat_ind.handle = prf_find_handle_by_idx(WECHAR_IDX_AIRSYNC_INDICATE_VAL,
                                               s_wechat_env.start_hdl,
                                               (uint8_t *)&s_char_mask);
    wechat_ind.length = chunk_len;
    wechat_ind.value  = (uint8_t *)s_ind_data.p_data + s_ind_data.offset;

    error_code = ble_gatts_noti_ind(conn_idx, &wechat_ind);

    if (SDK_SUCCESS == error_code)
    {
        s_ind_data.offset += chunk_len;
    }

    return error_code;
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
sdk_err_t wechat_service_init(wechat_init_t *p_wechat_init)
{
    if (NULL == p_wechat_init)
    {
        return SDK_ERR_POINTER_NULL;
    }

    s_wechat_env.evt_handler               = p_wechat_init->evt_handler;
    s_wechat_env.pedo_meas.flag            = WECHAT_PEDO_FLAG_ALL_SUP_BIT;
    s_wechat_env.pedo_target.flag          = WECHAT_PEDO_FLAG_STEP_COUNT_BIT;
    s_wechat_env.pedo_target.step_count[0] = LO_UINT32_T(p_wechat_init->step_count_target);
    s_wechat_env.pedo_target.step_count[1] = L2_UINT32_T(p_wechat_init->step_count_target);
    s_wechat_env.pedo_target.step_count[2] = L3_UINT32_T(p_wechat_init->step_count_target);
    memcpy(s_wechat_env.dev_mac, p_wechat_init->p_dev_mac, GAP_ADDR_LEN);

    return ble_server_prf_add(&wechat_prf_info);
}

sdk_err_t wechat_airsync_data_indicate(uint8_t conn_idx, uint8_t *p_data, uint16_t length)
{
    if (NULL == p_data || 0 == length)
    {
        return SDK_ERR_INVALID_PARAM;
    }

    s_ind_data.p_data = p_data;
    s_ind_data.length = length;
    s_ind_data.offset = 0;

    return wechat_indicate_data_chunk(conn_idx);
}

sdk_err_t wechat_pedo_measurement_send(uint8_t conn_idx, wechat_pedo_meas_t *p_pedo_meas)
{
    gatts_noti_ind_t pedo_meas_ntf;
    uint8_t          encode_data[WECHAT_DATA_LEN];
    uint8_t          encode_len = 0;
    sdk_err_t        error_code = SDK_ERR_NTF_DISABLED;

    if (NULL == p_pedo_meas)
    {
        return SDK_ERR_POINTER_NULL;
    }

    memcpy(&s_wechat_env.pedo_meas, p_pedo_meas, sizeof(wechat_pedo_meas_t));

    encode_len = wechat_pedo_meas_encode(encode_data, &s_wechat_env.pedo_meas);

    if (s_wechat_env.pedo_meas_ntf_cfg[conn_idx] & PRF_CLI_START_NTF)
    {
        pedo_meas_ntf.type   = BLE_GATT_NOTIFICATION;
        pedo_meas_ntf.handle = prf_find_handle_by_idx(WECHAT_IDX_PEDO_MEAS_VAL,
                                                      s_wechat_env.start_hdl,
                                                      (uint8_t *)&s_char_mask);
        pedo_meas_ntf.length = encode_len;
        pedo_meas_ntf.value  = encode_data;

        error_code  = ble_gatts_noti_ind(conn_idx, &pedo_meas_ntf);
    }

    return error_code;
}

sdk_err_t wechat_pedo_target_send(uint8_t conn_idx)
{
    gatts_noti_ind_t pedo_target_ind;
    uint8_t          encode_data[WECHAT_DATA_LEN];
    uint8_t          encode_len = 0;
    sdk_err_t        error_code = SDK_ERR_IND_DISABLED;

    encode_len = wechat_pedo_target_encode(encode_data, &s_wechat_env.pedo_target);

    if (s_wechat_env.pedo_target_ind_cfg[conn_idx] & PRF_CLI_START_IND)
    {
        pedo_target_ind.type   = BLE_GATT_INDICATION;
        pedo_target_ind.handle = prf_find_handle_by_idx(WECHAT_IDX_TARGET_VAL,
                                                        s_wechat_env.start_hdl,
                                                        (uint8_t *)&s_char_mask);
        pedo_target_ind.length = encode_len;
        pedo_target_ind.value  = encode_data;

        error_code  = ble_gatts_noti_ind(conn_idx, &pedo_target_ind);
    }

    return error_code;
}

