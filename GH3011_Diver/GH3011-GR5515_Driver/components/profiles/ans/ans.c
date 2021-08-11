/**
 ****************************************************************************************
 *
 * @file ans.c
 *
 * @brief Alert Notification Service implementation.
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
#include "ans.h"
#include "ble_prf_types.h"
#include "ble_prf_utils.h"
#include "gr55xx_sys.h"
#include "utility.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
/**@brief Alert Notification Service Attributes Indexes. */
enum
{
    // Alert Notification Service
    ANS_IDX_SVC,

    // Supported New Alert Category
    ANS_IDX_SUP_NEW_ALET_CAT_CHAR,
    ANS_IDX_SUP_NEW_ALET_CAT_VAL,

    // New Alert
    ANS_IDX_NEWS_ALERT_CHAR,
    ANS_IDX_NEWS_ALERT_VAL,
    ANS_IDX_NEWS_ALERT_NTF_CFG,

    // Supported Unread Alert Category
    ANS_IDX_SUP_UNREAD_ALERT_CAT_CHAR,
    ANS_IDX_SUP_UNREAD_ALERT_CAT_VAL,

    // Unread Alert Status
    ANS_IDX_UNREAD_ALERT_STA_CHAR,
    ANS_IDX_UNREAD_ALERT_STA_VAL,
    ANS_IDX_UNREAD_ALERT_STA_NTF_CFG,

    // Alert Notification Control Point
    ANS_IDX_ALERT_NTF_CTRL_PT_CHAR,
    ANS_IDX_ALERT_NTF_CTRL_PT_VAL,

    ANS_IDX_NB
};

/*
 * STRUCTURES
 *****************************************************************************************
 */
/**@brief Alert Notification Service environment variable. */
struct ans_env_t
{
    ans_init_t   ans_init;                                      /**< Alert Notification Service initialization variables. */
    uint16_t     start_hdl;                                     /**< Alert Notification Service start handle. */
    uint16_t     new_alert_ntf_cfg[ANS_CONNECTION_MAX];         /**< The configuration of New Alert Notification which is configured by the peer devices. */
    uint16_t     unread_alert_sta_ntf_cfg[ANS_CONNECTION_MAX];  /**< The configuration of Unread Alert Status Notification which is configured by the peer devices. */
    uint16_t     ntf_new_alert_cfg;                             /**< New Alert category notification configuration. */
    uint16_t     ntf_unread_alert_cfg;                          /**< Unread Alert Status category notification configuration. */
};

/*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static sdk_err_t ans_init(void);
static void      ans_read_att_cb(uint8_t  conn_idx, const gatts_read_req_cb_t  *p_param);
static void      ans_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param);
static void      ans_cccd_set_cb(uint8_t conn_idx, uint16_t handle, uint16_t cccd_value);
static void      ans_connected_cb(uint8_t conn_idx);
static bool      ans_ctrl_pt_sup_check(ans_ctrl_pt_t *p_ctrl_pt);
static void      ans_ctrl_pt_handler(uint8_t conn_idx, ans_ctrl_pt_t *p_ctrl_pt);



/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static struct ans_env_t s_ans_env;
static uint16_t         s_ans_char_mask = 0xffff;

/**@brief Full ANS Database Description - Used to add attributes into the database. */
static const attm_desc_t pass_attr_tab[ANS_IDX_NB] =
{
    // Alert Notification Service
    [ANS_IDX_SVC] = {BLE_ATT_DECL_PRIMARY_SERVICE, READ_PERM_UNSEC, 0, 0},

    // Alert Status Characteristic - Declaration
    [ANS_IDX_SUP_NEW_ALET_CAT_CHAR] = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // Alert Status Characteristic - Value
    [ANS_IDX_SUP_NEW_ALET_CAT_VAL]  = {BLE_ATT_CHAR_SUP_NEW_ALERT_CAT,
                                       READ_PERM_UNSEC,
                                       ATT_VAL_LOC_USER,
                                       ANS_SUP_NEW_ALERT_CAT_VAL_LEN},

    // New Alert Characteristic - Declaration
    [ANS_IDX_NEWS_ALERT_CHAR]    = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // New Alert Characteristic - Value
    [ANS_IDX_NEWS_ALERT_VAL]     = {BLE_ATT_CHAR_NEW_ALERT,
                                    NOTIFY_PERM_UNSEC,
                                    ATT_VAL_LOC_USER,
                                    ANS_NEWS_ALERT_VAL_LEN},
    // New Alert Characteristic - Client Characteristic Configuration Descriptor
    [ANS_IDX_NEWS_ALERT_NTF_CFG] = {BLE_ATT_DESC_CLIENT_CHAR_CFG, READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC, 0, 0},

    // Supported Unread AlertCategory Characteristic - Declaration
    [ANS_IDX_SUP_UNREAD_ALERT_CAT_CHAR] = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // Supported Unread Alert Category Characteristic - Value
    [ANS_IDX_SUP_UNREAD_ALERT_CAT_VAL]  = {BLE_ATT_CHAR_SUP_UNREAD_ALERT_CAT,
                                           READ_PERM_UNSEC,
                                           ATT_VAL_LOC_USER,
                                           ANS_SUP_UNREAD_ALERT_CAT_VAL_LEN},

    // Unread Alert Status Characteristic - Declaration
    [ANS_IDX_UNREAD_ALERT_STA_CHAR]    = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // Unread Alert Status Characteristic - Value
    [ANS_IDX_UNREAD_ALERT_STA_VAL]     = {BLE_ATT_CHAR_UNREAD_ALERT_STATUS,
                                          NOTIFY_PERM_UNSEC,
                                          ATT_VAL_LOC_USER,
                                          ANS_UNREAD_ALERT_STA_VAL_LEN},
    // Unread Alert Status Characteristic - Client Characteristic Configuration Descriptor
    [ANS_IDX_UNREAD_ALERT_STA_NTF_CFG] = {BLE_ATT_DESC_CLIENT_CHAR_CFG, READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC, 0, 0},

    // Alert Notification Control Point Characteristic - Declaration
    [ANS_IDX_ALERT_NTF_CTRL_PT_CHAR]   = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // Alert Notification Control Point Characteristic - Value
    [ANS_IDX_ALERT_NTF_CTRL_PT_VAL]    = {BLE_ATT_CHAR_ALERT_NTF_CTNL_PT,
                                          WRITE_CMD_PERM_UNSEC,
                                          ATT_VAL_LOC_USER,
                                          ANS_ALERT_NTF_CTRL_PT_VAL_LEN},
};

/**@brief ANS Task interface required by profile manager. */
static ble_prf_manager_cbs_t ans_task_cbs =
{
    (prf_init_func_t) ans_init,
    ans_connected_cb,
    NULL,
};

/**@brief ANS Task Callbacks. */
static gatts_prf_cbs_t ans_cb_func =
{
    ans_read_att_cb,
    ans_write_att_cb,
    NULL,
    NULL,
    ans_cccd_set_cb
};

/**@brief ANS Information. */
static const prf_server_info_t ans_prf_info =
{
    .max_connection_nb = ANS_CONNECTION_MAX,
    .manager_cbs       = &ans_task_cbs,
    .gatts_prf_cbs     = &ans_cb_func,
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 * @brief Initialize Alert Notify Service and create db in att
 *
 * @return Error code to know if profile initialization succeed or not.
 *****************************************************************************************
 */
static sdk_err_t ans_init(void)
{
    // The start hanlde must be set with PRF_INVALID_HANDLE to be allocated automatically by BLE Stack.
    uint16_t          start_hdl       = PRF_INVALID_HANDLE;
    const uint8_t     pass_svc_uuid[] = BLE_ATT_16_TO_16_ARRAY(BLE_ATT_SVC_ALERT_NTF);
    sdk_err_t         error_code;
    gatts_create_db_t gatts_db;

    memset(&gatts_db, 0, sizeof(gatts_db));

    gatts_db.shdl                 = &start_hdl;
    gatts_db.uuid                 = pass_svc_uuid;
    gatts_db.attr_tab_cfg         = (uint8_t *)&s_ans_char_mask;
    gatts_db.max_nb_attr          = ANS_IDX_NB;
    gatts_db.srvc_perm            = 0;
    gatts_db.attr_tab_type        = SERVICE_TABLE_TYPE_16;
    gatts_db.attr_tab.attr_tab_16 = pass_attr_tab;

    error_code = ble_gatts_srvc_db_create(&gatts_db);

    if (SDK_SUCCESS == error_code)
    {
        s_ans_env.start_hdl = *gatts_db.shdl;
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
static void   ans_read_att_cb(uint8_t conn_idx, const gatts_read_req_cb_t *p_param)
{
    gatts_read_cfm_t  cfm;
    uint8_t           handle    = p_param->handle;
    uint8_t           tab_index = prf_find_idx_by_handle(handle,
                                  s_ans_env.start_hdl,
                                  ANS_IDX_NB,
                                  (uint8_t *)&s_ans_char_mask);
    cfm.handle = handle;
    cfm.status = BLE_SUCCESS;

    switch (tab_index)
    {
        case ANS_IDX_SUP_NEW_ALET_CAT_VAL:
            cfm.length = ANS_SUP_NEW_ALERT_CAT_VAL_LEN;
            cfm.value  = (uint8_t *)&s_ans_env.ans_init.sup_new_alert_cat;
            break;

        case ANS_IDX_NEWS_ALERT_NTF_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_ans_env.new_alert_ntf_cfg[conn_idx];
            break;

        case ANS_IDX_SUP_UNREAD_ALERT_CAT_VAL:
            cfm.length = ANS_SUP_UNREAD_ALERT_CAT_VAL_LEN;
            cfm.value  = (uint8_t *)&s_ans_env.ans_init.sup_unread_alert_sta;
            break;

        case ANS_IDX_UNREAD_ALERT_STA_NTF_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_ans_env.unread_alert_sta_ntf_cfg[conn_idx];
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
 * @param[in]: conn_idx: Connection index
 * @param[in]: p_param:  The parameters of the write request.
 *****************************************************************************************
 */
static void   ans_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param)
{
    uint16_t          handle         = p_param->handle;
    uint16_t          tab_index      = 0;
    uint16_t          cccd_value     = 0;
    bool              is_ctrl_pt_rec = false;
    ans_ctrl_pt_t     ctrl_pt;
    ans_evt_t         event;
    gatts_write_cfm_t cfm;

    tab_index  = prf_find_idx_by_handle(handle,
                                        s_ans_env.start_hdl,
                                        ANS_IDX_NB,
                                        (uint8_t *)&s_ans_char_mask);
    cfm.handle     = handle;
    cfm.status     = BLE_SUCCESS;
    event.evt_type = ANS_EVT_INVALID;
    event.conn_idx = conn_idx;

    switch (tab_index)
    {
        case ANS_IDX_NEWS_ALERT_NTF_CFG:
            cccd_value     = le16toh(&p_param->value[0]);
            event.evt_type = ((PRF_CLI_START_NTF == cccd_value) ? \
                              ANS_EVT_NEW_ALERT_NTF_ENABLE : \
                              ANS_EVT_NEW_ALERT_NTF_DISABLE);
            s_ans_env.new_alert_ntf_cfg[conn_idx] = cccd_value;
            break;

        case ANS_IDX_UNREAD_ALERT_STA_NTF_CFG:
            cccd_value     = le16toh(&p_param->value[0]);
            event.evt_type = ((PRF_CLI_START_NTF == cccd_value) ? \
                              ANS_EVT_UNREAD_ALERT_STA_NTF_ENABLE : \
                              ANS_EVT_UNREAD_ALERT_STA_NTF_DISABLE);
            s_ans_env.unread_alert_sta_ntf_cfg[conn_idx] = cccd_value;
            break;

        case ANS_IDX_ALERT_NTF_CTRL_PT_VAL:
            ctrl_pt.cmd_id = (ans_ctrl_pt_id_t)p_param->value[0];
            ctrl_pt.cat_id = (ans_alert_cat_id_t)p_param->value[1];

            if (ans_ctrl_pt_sup_check(&ctrl_pt))
            {
                is_ctrl_pt_rec = true;
            }
            else
            {
                cfm.status = ANS_ERROR_CMD_NOT_SUP;
            }

            break;

        default:
            cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
            break;
    }

    ble_gatts_write_cfm(conn_idx, &cfm);

    if (is_ctrl_pt_rec)
    {
        is_ctrl_pt_rec = false;
        ans_ctrl_pt_handler(conn_idx, &ctrl_pt);
    }

    if (BLE_ATT_ERR_INVALID_HANDLE != cfm.status && \
        ANS_EVT_INVALID != event.evt_type && \
        s_ans_env.ans_init.evt_handler)
    {
        s_ans_env.ans_init.evt_handler(&event);
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
static void ans_cccd_set_cb(uint8_t conn_idx, uint16_t handle, uint16_t cccd_value)
{
    uint16_t  tab_index = 0;
    ans_evt_t event;

    if (!prf_is_cccd_value_valid(cccd_value))
    {
        return;
    }

    tab_index  = prf_find_idx_by_handle(handle,
                                        s_ans_env.start_hdl,
                                        ANS_IDX_NB,
                                        (uint8_t *)&s_ans_char_mask);

    event.evt_type = ANS_EVT_INVALID;
    event.conn_idx = conn_idx;

    switch (tab_index)
    {
        case ANS_IDX_NEWS_ALERT_NTF_CFG:
            event.evt_type = ((PRF_CLI_START_NTF == cccd_value) ? \
                              ANS_EVT_NEW_ALERT_NTF_ENABLE : \
                              ANS_EVT_NEW_ALERT_NTF_DISABLE);
            s_ans_env.new_alert_ntf_cfg[conn_idx] = cccd_value;
            break;

        case ANS_IDX_UNREAD_ALERT_STA_NTF_CFG:
            event.evt_type = ((PRF_CLI_START_NTF == cccd_value) ? \
                              ANS_EVT_UNREAD_ALERT_STA_NTF_ENABLE : \
                              ANS_EVT_UNREAD_ALERT_STA_NTF_DISABLE);
            s_ans_env.unread_alert_sta_ntf_cfg[conn_idx] = cccd_value;
            break;

        default:
            break;
    }

    if (ANS_EVT_INVALID != event.evt_type && \
        s_ans_env.ans_init.evt_handler)
    {
        s_ans_env.ans_init.evt_handler(&event);
    }
}

/**
 *****************************************************************************************
 * @brief Connected callback.
 *
 * @param[in]: conn_idx: Connection index
 *****************************************************************************************
 */
static void ans_connected_cb(uint8_t conn_idx)
{
    s_ans_env.ntf_new_alert_cfg    = 0;
    s_ans_env.ntf_unread_alert_cfg = 0;
}

/**
 *****************************************************************************************
 * @brief Check ANS Control Point is supported or not.
 *
 * @param[in] p_ctrl_pt: Pointer to control pointer.
 *
 * @return True or False
 *****************************************************************************************
 */
static bool ans_ctrl_pt_sup_check(ans_ctrl_pt_t *p_ctrl_pt)
{
    if (ANS_CTRL_PT_NTF_UNREAD_CAT_STA_IMME < p_ctrl_pt->cmd_id)
    {
        return false;
    }

    if (ANS_CAT_ID_INSTANT_MES < p_ctrl_pt->cat_id && ANS_CAT_ID_ALL != p_ctrl_pt->cat_id)
    {
        return false;
    }

    if ((ANS_CTRL_PT_EN_NEW_INC_ALERT_NTF == p_ctrl_pt->cmd_id) && \
        (ANS_CTRL_PT_DIS_NEW_INC_ALERT_NTF == p_ctrl_pt->cmd_id) && \
        (ANS_CTRL_PT_NTF_NEW_INC_ALERT_IMME == p_ctrl_pt->cmd_id))
    {
        if (ANS_CAT_ID_ALL == p_ctrl_pt->cat_id)
        {
            if ( 0 == s_ans_env.ans_init.sup_new_alert_cat)
            {
                return false;
            }
        }
        else if (!(s_ans_env.ans_init.sup_new_alert_cat & (1 << p_ctrl_pt->cat_id)))
        {
            return false;
        }
    }
    else
    {
        if (ANS_CAT_ID_ALL == p_ctrl_pt->cat_id)
        {
            if ( 0 == s_ans_env.ans_init.sup_new_alert_cat)
            {
                return false;
            }
        }
        else if (!(s_ans_env.ans_init.sup_unread_alert_sta & ( 1 << p_ctrl_pt->cat_id)))
        {
            return false;
        }
    }

    return true;
}

/**
 *****************************************************************************************
 * @brief ANS Control Point handler.
 *
 * @param[in] conn_idx:  Connection index.
 * @param[in] p_ctrl_pt: Pointer to control pointer.
 *****************************************************************************************
 */
static void ans_ctrl_pt_handler(uint8_t conn_idx, ans_ctrl_pt_t *p_ctrl_pt)
{
    ans_evt_t         event;
    event.evt_type = ANS_EVT_INVALID;
    event.conn_idx = conn_idx;

    switch (p_ctrl_pt->cmd_id)
    {
        case ANS_CTRL_PT_EN_NEW_INC_ALERT_NTF:
            if (ANS_CAT_ID_ALL == p_ctrl_pt->cat_id)
            {
                s_ans_env.ntf_new_alert_cfg |= s_ans_env.ans_init.sup_new_alert_cat;
            }
            else
            {
                s_ans_env.ntf_new_alert_cfg |= 1 << p_ctrl_pt->cat_id;
            }

            break;

        case ANS_CTRL_PT_EN_UNREAD_CAT_STA_NTF:
            if (ANS_CAT_ID_ALL == p_ctrl_pt->cat_id)
            {
                s_ans_env.ntf_unread_alert_cfg |= s_ans_env.ans_init.sup_new_alert_cat;
            }
            else
            {
                s_ans_env.ntf_unread_alert_cfg |= 1 << p_ctrl_pt->cat_id;
            }

            break;

        case ANS_CTRL_PT_DIS_NEW_INC_ALERT_NTF:
            if (ANS_CAT_ID_ALL == p_ctrl_pt->cat_id)
            {
                s_ans_env.ntf_new_alert_cfg &= ~s_ans_env.ans_init.sup_new_alert_cat;
            }
            else
            {
                s_ans_env.ntf_new_alert_cfg &= ~(1 << p_ctrl_pt->cat_id);
            }

            break;

        case ANS_CTRL_PT_DIS_UNREAD_CAT_STA_NTF:
            if (ANS_CAT_ID_ALL == p_ctrl_pt->cat_id)
            {
                s_ans_env.ntf_unread_alert_cfg &= ~s_ans_env.ans_init.sup_unread_alert_sta;
            }
            else
            {
                s_ans_env.ntf_unread_alert_cfg &= ~(1 << p_ctrl_pt->cat_id);
            }

            break;

        case ANS_CTRL_PT_NTF_NEW_INC_ALERT_IMME:
            if (ANS_CAT_ID_ALL == p_ctrl_pt->cat_id)
            {
                event.cat_ids = s_ans_env.ntf_new_alert_cfg;
            }
            else
            {
                event.cat_ids = 1 << p_ctrl_pt->cat_id;
            }

            event.evt_type = ANS_EVT_NEW_ALERT_IMME_NTF_REQ;
            break;

        case ANS_CTRL_PT_NTF_UNREAD_CAT_STA_IMME:
            if (ANS_CAT_ID_ALL == p_ctrl_pt->cat_id)
            {
                event.cat_ids = s_ans_env.ntf_unread_alert_cfg;
            }
            else
            {
                event.cat_ids = 1 << p_ctrl_pt->cat_id;
            }

            event.evt_type = ANS_EVT_Unread_ALERT_IMME_NTF_REQ;
            break;

        default:
            break;
    }

    if (ANS_EVT_INVALID != event.evt_type && s_ans_env.ans_init.evt_handler)
    {
        s_ans_env.ans_init.evt_handler(&event);
    }
}

/**
 *****************************************************************************************
 * @brief Encode a New Alert.
 *
 * @param[in]  p_new_alert:    Pointer to New Alert value to be encoded.
 * @param[out] p_encoded_data: Pointer to encoded buffer will be written.
 *
 * @return Length of encoded.
 *****************************************************************************************
 */
static uint16_t ans_new_alert_encode(ans_new_alert_t *p_new_alert, uint8_t *p_buff)
{
    uint16_t length = 0;

    p_buff[length++] = p_new_alert->cat_id;
    p_buff[length++] = p_new_alert->alert_num;

    if (0 < p_new_alert->length)
    {
        memcpy(&p_buff[length], p_new_alert->str_info, p_new_alert->length);
    }

    return (length + p_new_alert->length);
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
sdk_err_t ans_new_alert_send(uint8_t conn_idx, ans_new_alert_t *p_new_alert)
{
    uint8_t          encoded_new_alert[ANS_NEWS_ALERT_VAL_LEN];
    gatts_noti_ind_t new_alert_ntf;
    uint16_t         length;

    length = ans_new_alert_encode(p_new_alert, encoded_new_alert);

    if (ANS_UTF_8_STR_LEN_MAX < p_new_alert->length)
    {
        return SDK_ERR_INVALID_PARAM;
    }

    if (ANS_CAT_ID_INSTANT_MES < p_new_alert->cat_id)
    {
        return SDK_ERR_INVALID_PARAM;
    }

    if (!(s_ans_env.ans_init.sup_new_alert_cat & (1 << p_new_alert->cat_id)))
    {
        return SDK_ERR_INVALID_PARAM;
    }

    if (PRF_CLI_START_NTF != s_ans_env.new_alert_ntf_cfg[conn_idx])
    {
        return SDK_ERR_NTF_DISABLED;
    }

    if (!(s_ans_env.ntf_new_alert_cfg & (1 << p_new_alert->cat_id)))
    {
        return SDK_ERR_NTF_DISABLED;
    }

    new_alert_ntf.type   = BLE_GATT_NOTIFICATION;
    new_alert_ntf.handle = prf_find_handle_by_idx(ANS_IDX_NEWS_ALERT_VAL,
                           s_ans_env.start_hdl,
                           (uint8_t *)&s_ans_char_mask);
    new_alert_ntf.length = length;
    new_alert_ntf.value  = encoded_new_alert;

    return ble_gatts_noti_ind(conn_idx, &new_alert_ntf);
}

sdk_err_t ans_unread_alert_send(uint8_t conn_idx, ans_unread_alert_t *p_unread_alert)
{
    uint8_t          encoded_unread_alert[ANS_UNREAD_ALERT_STA_VAL_LEN];
    gatts_noti_ind_t unread_alert_ntf;

    encoded_unread_alert[0] = p_unread_alert->cat_id;
    encoded_unread_alert[1] = p_unread_alert->unread_num;


    if (ANS_CAT_ID_INSTANT_MES < p_unread_alert->cat_id)
    {
        return SDK_ERR_INVALID_PARAM;
    }

    if (!(s_ans_env.ans_init.sup_unread_alert_sta & (1 << p_unread_alert->cat_id)))
    {
        return SDK_ERR_INVALID_PARAM;
    }

    if (PRF_CLI_START_NTF != s_ans_env.unread_alert_sta_ntf_cfg[conn_idx])
    {
        return SDK_ERR_NTF_DISABLED;
    }

    if (!(s_ans_env.ntf_unread_alert_cfg & (1 << p_unread_alert->cat_id)))
    {
        return SDK_ERR_NTF_DISABLED;
    }

    unread_alert_ntf.type   = BLE_GATT_NOTIFICATION;
    unread_alert_ntf.handle = prf_find_handle_by_idx(ANS_IDX_UNREAD_ALERT_STA_VAL,
                              s_ans_env.start_hdl,
                              (uint8_t *)&s_ans_char_mask);
    unread_alert_ntf.length = ANS_UNREAD_ALERT_STA_VAL_LEN;
    unread_alert_ntf.value  = encoded_unread_alert;

    return ble_gatts_noti_ind(conn_idx, &unread_alert_ntf);
}

sdk_err_t ans_service_init(ans_init_t *p_ans_init)
{
    if (NULL == p_ans_init)
    {
        return SDK_ERR_POINTER_NULL;
    }

    memcpy(&s_ans_env.ans_init, p_ans_init, sizeof(ans_init_t));

    return ble_server_prf_add(&ans_prf_info);
}

