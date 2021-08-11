/**
 *****************************************************************************************
 *
 * @file rscs.c
 *
 * @brief Running Speed and Cadence Profile Sensor implementation.
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
#include "rscs.h"
#include "ble_prf_types.h"
#include "ble_prf_utils.h"
#include "utility.h"

/*
 * ENUMERATIONS
 *****************************************************************************************
 */
/**@brief Running Speed and Cadence Service Attributes Indexes. */
enum
{
    RSCS_IDX_SVC,

    RSCS_IDX_RSC_MEAS_CHAR,
    RSCS_IDX_RSC_MEAS_VAL,
    RSCS_IDX_RSC_MEAS_NTF_CFG,

    RSCS_IDX_RSC_FEAT_CHAR,
    RSCS_IDX_RSC_FEAT_VAL,

    RSCS_IDX_SENSOR_LOC_CHAR,
    RSCS_IDX_SENSOR_LOC_VAL,

    RSCS_IDX_CTRL_POINT_CHAR,
    RSCS_IDX_CTRL_POINT_VAL,
    RSCS_IDX_CTRL_POINT_IND_CFG,

    RSCS_IDX_NB
};

/*
 * STRUCTURES
 *****************************************************************************************
 */
/**@brief Running Speed and Cadence Service environment variable. */
struct rscs_env_t
{
    rscs_init_t  rscs_init;                               /**< Running Speed and Cadence Service initialization variables. */
    uint16_t     start_hdl;                               /**< Running Speed and Cadence Service start handle. */
    bool         ctrl_pt_op_in_progress;                  /**< A previously triggered SC Control Point operation is still in progress. */
    bool         ctrl_pt_op_rsp_cplt;                     /**< A previously triggered SC Control Point operation response cplt. */
    uint16_t     meas_ntf_cfg[RSCS_CONNECTION_MAX];       /**< The configuration of RCS Measurement Notification which is configured by the peer devices. */
    uint16_t     ctrl_point_ind_cfg[RSCS_CONNECTION_MAX]; /**< The configuration of SC Control Point Notification which is configured by the peer devices. */
};

/*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static sdk_err_t   rscs_init(void);
static void        rscs_read_att_cb(uint8_t  conn_idx, const gatts_read_req_cb_t  *p_param);
static void        rscs_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param);
static void        rscs_cccd_set_cb(uint8_t conn_idx, uint16_t handle, uint16_t cccd_value);
static void        rscs_disconnect_cb(uint8_t conn_idx, uint8_t reason);
static void        rscs_ntf_ind_cb(uint8_t conn_idx, uint8_t status, const ble_gatts_ntf_ind_t *p_ntf_ind);
static void        rscs_sc_ctrl_pt_handler(uint8_t conn_idx, const uint8_t *p_data, uint16_t length);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static struct rscs_env_t s_rscs_env;

/**@brief Full RSCS Database Description - Used to add attributes into the database. */
static const attm_desc_t rscs_attr_tab[RSCS_IDX_NB]  =
{
    // Running Speed and Cadence Service Declaration
    [RSCS_IDX_SVC]              = {BLE_ATT_DECL_PRIMARY_SERVICE, READ_PERM_UNSEC, 0, 0},

    // RSC Measurement Characteristic - Declaration
    [RSCS_IDX_RSC_MEAS_CHAR]    = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // RSC Measurement Characteristic - Value
    [RSCS_IDX_RSC_MEAS_VAL]     = {BLE_ATT_CHAR_RSC_MEAS,
                                   NOTIFY_PERM_UNSEC, 
                                   ATT_VAL_LOC_USER,
                                   RSCS_MEAS_VAL_LEN_MAX},
    // RSC Measurement Characteristic - Client Characteristic Configuration Descriptor
    [RSCS_IDX_RSC_MEAS_NTF_CFG] = {BLE_ATT_DESC_CLIENT_CHAR_CFG,
                                   READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC,
                                   0,
                                   0},

    // RSC Feature Characteristic - Declaration
    [RSCS_IDX_RSC_FEAT_CHAR]    = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // RSC Feature Characteristic - Value
    [RSCS_IDX_RSC_FEAT_VAL]     = {BLE_ATT_CHAR_RSC_FEAT,
                                   READ_PERM_UNSEC,
                                   ATT_VAL_LOC_USER,
                                   RSCS_FEAT_VAL_LEN_MAX},

    // Sensor Location Characteristic - Declaration
    [RSCS_IDX_SENSOR_LOC_CHAR]  = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // Sensor Location Characteristic - Value
    [RSCS_IDX_SENSOR_LOC_VAL]   = {BLE_ATT_CHAR_SENSOR_LOC,
                                   READ_PERM_UNSEC,
                                   ATT_VAL_LOC_USER,
                                   RSCS_SENSOR_LOC_VAL_LEN_MAX},

    // SC Control Point Characteristic - Declaration
    [RSCS_IDX_CTRL_POINT_CHAR]    = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // SC Control Point Characteristic - Value
    [RSCS_IDX_CTRL_POINT_VAL]     = {BLE_ATT_CHAR_SC_CNTL_PT,
                                     WRITE_REQ_PERM_UNSEC | INDICATE_PERM_UNSEC,
                                     ATT_VAL_LOC_USER,
                                     RSCS_CTRL_PT_VAL_LEN_MAX},
    // SC Control Point Characteristic - Client Characteristic Configuration Descriptor
    [RSCS_IDX_CTRL_POINT_IND_CFG] = {BLE_ATT_DESC_CLIENT_CHAR_CFG,
                                     READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC,
                                     0,
                                     0},
};

/**@brief RSCS Task interface required by profile manager. */
static ble_prf_manager_cbs_t rscs_task_cbs =
{
    (prf_init_func_t) rscs_init,
    NULL,
    rscs_disconnect_cb,
};

/**@brief RSCS Task Callbacks. */
static gatts_prf_cbs_t rscs_cb_func =
{
    rscs_read_att_cb,
    rscs_write_att_cb,
    NULL,
    rscs_ntf_ind_cb,
    rscs_cccd_set_cb
};

/**@brief RSCS Information. */
static const prf_server_info_t rscs_prf_info =
{
    .max_connection_nb = RSCS_CONNECTION_MAX,
    .manager_cbs       = &rscs_task_cbs,
    .gatts_prf_cbs     = &rscs_cb_func,
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 * @brief Initialize Running Speed and Cadence service  create db in att
 *
 * @return Error code to know if profile initialization succeed or not.
 *****************************************************************************************
 */
static sdk_err_t rscs_init(void)
{
    // The start hanlde must be set with PRF_INVALID_HANDLE to be allocated automatically by BLE Stack.
    uint16_t          start_hdl       = PRF_INVALID_HANDLE;
    const uint8_t     rscs_svc_uuid[] = BLE_ATT_16_TO_16_ARRAY(BLE_ATT_SVC_RUNNING_SPEED_CADENCE);
    sdk_err_t         error_code;
    gatts_create_db_t gatts_db; 

    memset(&gatts_db, 0, sizeof(gatts_db));

    gatts_db.shdl                 = &start_hdl;
    gatts_db.uuid                 = rscs_svc_uuid;
    gatts_db.attr_tab_cfg         = (uint8_t *)&(s_rscs_env.rscs_init.char_mask);
    gatts_db.max_nb_attr          = RSCS_IDX_NB;
    gatts_db.srvc_perm            = 0; 
    gatts_db.attr_tab_type        = SERVICE_TABLE_TYPE_16;
    gatts_db.attr_tab.attr_tab_16 = rscs_attr_tab;

    error_code = ble_gatts_srvc_db_create(&gatts_db);

    if (SDK_SUCCESS == error_code)
    {
        s_rscs_env.start_hdl = *gatts_db.shdl;
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
static void rscs_read_att_cb(uint8_t conn_idx, const gatts_read_req_cb_t *p_param)
{
    gatts_read_cfm_t  cfm;
    uint8_t           handle    = p_param->handle;
    uint8_t           tab_index = prf_find_idx_by_handle(handle, 
                                                         s_rscs_env.start_hdl,
                                                         RSCS_IDX_NB,
                                                        (uint8_t *)&s_rscs_env.rscs_init.char_mask);
    cfm.handle = handle;
    cfm.status = BLE_SUCCESS;

    switch (tab_index)
    {
        case RSCS_IDX_RSC_MEAS_NTF_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_rscs_env.meas_ntf_cfg[conn_idx];
            break;

        case RSCS_IDX_RSC_FEAT_VAL:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_rscs_env.rscs_init.feature;
            break;

        case RSCS_IDX_SENSOR_LOC_VAL:
            cfm.length = sizeof(uint8_t);
            cfm.value  = (uint8_t *)&s_rscs_env.rscs_init.sensor_location;
            break;
 
        case RSCS_IDX_CTRL_POINT_IND_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_rscs_env.ctrl_point_ind_cfg[conn_idx];
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
static void rscs_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param)
{
    uint16_t          handle      = p_param->handle;
    uint16_t          tab_index   = 0;
    uint16_t          cccd_value  = 0;
    bool              ctrl_pt_evt = false;
    rscs_evt_t        event;
    gatts_write_cfm_t cfm;

    tab_index  = prf_find_idx_by_handle(handle,
                                        s_rscs_env.start_hdl,
                                        RSCS_IDX_NB,
                                        (uint8_t *)&s_rscs_env.rscs_init.char_mask);
    cfm.handle     = handle;
    cfm.status     = BLE_SUCCESS;
    event.evt_type = RSCS_EVT_INVALID;
    event.conn_idx = conn_idx;
 
    switch (tab_index)
    {
        case RSCS_IDX_RSC_MEAS_NTF_CFG:
            cccd_value     = le16toh(&p_param->value[0]);
            event.evt_type = ((PRF_CLI_START_NTF == cccd_value) ?\
                               RSCS_EVT_RSC_MEAS_NOTIFICATION_ENABLE :\
                               RSCS_EVT_RSC_MEAS_NOTIFICATION_DISABLE);
            s_rscs_env.meas_ntf_cfg[conn_idx] = cccd_value;
            break;

        case RSCS_IDX_CTRL_POINT_VAL:
            if (PRF_CLI_START_IND != s_rscs_env.ctrl_point_ind_cfg[conn_idx])
            {
                cfm.status = RSCS_ERROR_CCCD_INVALID;
                break;
            }
            else if (s_rscs_env.ctrl_pt_op_in_progress)
            {
                cfm.status = RSCS_ERROR_PROC_IN_PROGRESS;
            }
            else if (PRF_CLI_START_IND == s_rscs_env.ctrl_point_ind_cfg[conn_idx])
            {
                s_rscs_env.ctrl_pt_op_in_progress = true;
                ctrl_pt_evt = true;
            }
            break;

        case RSCS_IDX_CTRL_POINT_IND_CFG:
            cccd_value     = le16toh(&p_param->value[0]);
            event.evt_type = ((PRF_CLI_START_IND == cccd_value) ?\
                               RSCS_EVT_CTRL_POINT_INDICATION_ENABLE :\
                               RSCS_EVT_CTRL_POINT_INDICATION_DISABLE);
            s_rscs_env.ctrl_point_ind_cfg[conn_idx] = cccd_value;
            break;

        default:
            cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
            break;
    }

    ble_gatts_write_cfm(conn_idx, &cfm);

    if (ctrl_pt_evt)
    {
        rscs_sc_ctrl_pt_handler(conn_idx, p_param->value, p_param->length);
    }
    else if (BLE_ATT_ERR_INVALID_HANDLE != cfm.status && RSCS_EVT_INVALID != event.evt_type && s_rscs_env.rscs_init.evt_handler)
    {
        s_rscs_env.rscs_init.evt_handler(&event);
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
static void rscs_cccd_set_cb(uint8_t conn_idx, uint16_t handle, uint16_t cccd_value)
{
    uint16_t          tab_index   = 0;
    rscs_evt_t        event;

    if (!prf_is_cccd_value_valid(cccd_value))
    {
        return;
    }

    tab_index  = prf_find_idx_by_handle(handle,
                                        s_rscs_env.start_hdl,
                                        RSCS_IDX_NB,
                                        (uint8_t *)&s_rscs_env.rscs_init.char_mask);

    event.evt_type = RSCS_EVT_INVALID;
    event.conn_idx = conn_idx;
 
    switch (tab_index)
    {
        case RSCS_IDX_RSC_MEAS_NTF_CFG:
            event.evt_type = ((PRF_CLI_START_NTF == cccd_value) ?\
                               RSCS_EVT_RSC_MEAS_NOTIFICATION_ENABLE :\
                               RSCS_EVT_RSC_MEAS_NOTIFICATION_DISABLE);
            s_rscs_env.meas_ntf_cfg[conn_idx] = cccd_value;
            break;

        case RSCS_IDX_CTRL_POINT_IND_CFG:
            event.evt_type = ((PRF_CLI_START_IND == cccd_value) ?\
                               RSCS_EVT_CTRL_POINT_INDICATION_ENABLE :\
                               RSCS_EVT_CTRL_POINT_INDICATION_DISABLE);
            s_rscs_env.ctrl_point_ind_cfg[conn_idx] = cccd_value;
            break;

        default:
            break;
    }

    if (RSCS_EVT_INVALID != event.evt_type && s_rscs_env.rscs_init.evt_handler)
    {
        s_rscs_env.rscs_init.evt_handler(&event);
    }
}

/**
 *****************************************************************************************
 * @brief Handles reception of the complete event.
 *
 * @param[in] conn_idx:   Connection index.
 * @param[in] status:     The status of GATTC operation.
 * @param[in] p_ntf_ind:  Pointer to the parameters of the complete event.
 *****************************************************************************************
 */
static void rscs_ntf_ind_cb(uint8_t conn_idx, uint8_t status, const ble_gatts_ntf_ind_t *p_ntf_ind)
{
    rscs_evt_t  event;

    event.evt_type = RSCS_EVT_INVALID;
    event.conn_idx = conn_idx;

    if (s_rscs_env.rscs_init.evt_handler && SDK_SUCCESS == status)
    {
        if (BLE_GATT_NOTIFICATION == p_ntf_ind->type)
        {
            event.evt_type = RSCS_EVT_RSC_MEAS_SEND_CPLT;
        }
        else if (BLE_GATT_INDICATION == p_ntf_ind->type)
        {
            event.evt_type = RSCS_EVT_CTRL_POINT_RSP_CPLT;
            s_rscs_env.ctrl_pt_op_in_progress = false;
        }
        s_rscs_env.rscs_init.evt_handler(&event);
    }
}

/**
 *****************************************************************************************
 * @brief SC Control Point Set Cumulative value handler.
 *
 * @param[in] conn_idx: Connection index.
 * @param[in] p_data:   Pointer to data.
 * @param[in] length:   Length of data.
 *****************************************************************************************
 */
static void rscs_op_set_cumulative_handler(uint8_t conn_idx, const uint8_t *p_data, uint16_t length)
{
    rscs_evt_t  event;
    uint8_t     rsp[RSCS_CTRL_PT_RSP_LEN_MIN];

    rsp[0] = RSCS_CTRL_PT_OP_RSP_CODE;
    rsp[1] = RSCS_CTRL_PT_OP_SET_CUMUL_VAL;
    rsp[2] = RSCS_CTRL_PT_RSP_FAILED;

    if ((sizeof(uint32_t) == length) && \
        (s_rscs_env.rscs_init.feature & RSCS_FEAT_TOTAL_DISTANCE_BIT) && \
        (s_rscs_env.rscs_init.evt_handler))
    {
        event.conn_idx = conn_idx;
        event.evt_type = RSCS_EVT_CUMUL_VAL_SET;
        event.p_data   = p_data;
        event.length   = length;
        s_rscs_env.rscs_init.evt_handler(&event);
    }
    else
    {
        rscs_ctrl_pt_rsp_send(conn_idx, rsp, RSCS_CTRL_PT_RSP_LEN_MIN);
    }
}

/**
 *****************************************************************************************
 * @brief SC Control Point Start Calibration handler.
 *
 * @param[in] conn_idx: Connection index.
 *****************************************************************************************
 */
static void rscs_op_start_calibration_handler(uint8_t conn_idx)
{
    rscs_evt_t  event;
    uint8_t     rsp[RSCS_CTRL_PT_RSP_LEN_MIN];

    rsp[0] = RSCS_CTRL_PT_OP_RSP_CODE;
    rsp[1] = RSCS_CTRL_PT_OP_START_CALIB;
    rsp[2] = RSCS_CTRL_PT_RSP_FAILED;

    if ((s_rscs_env.rscs_init.feature & RSCS_FEAT_CALIBRATION_PROCEDURE_BIT) && s_rscs_env.rscs_init.evt_handler)
    {
        event.conn_idx = conn_idx;
        event.evt_type = RSCS_EVT_SEBSOR_CALIBRATION;
        s_rscs_env.rscs_init.evt_handler(&event);
    }
    else
    {
        rscs_ctrl_pt_rsp_send(conn_idx, rsp, RSCS_CTRL_PT_RSP_LEN_MIN);
    }
}

/**
 *****************************************************************************************
 * @brief SC Control Point Sensor Location update handler.
 *
 * @param[in] conn_idx: Connection index.
 * @param[in] p_data:   Pointer to data.
 * @param[in] length:   Length of data.
 *****************************************************************************************
 */
static void rscs_op_sensor_loc_update_handler(uint8_t conn_idx, const uint8_t *p_data, uint16_t length)
{
    rscs_evt_t  event;
    uint8_t     rsp[RSCS_CTRL_PT_RSP_LEN_MIN];

    rsp[0] = RSCS_CTRL_PT_OP_RSP_CODE;
    rsp[1] = RSCS_CTRL_PT_OP_UPD_LOC;

    if (RSCS_SENSOR_LOC_SUP_NB <= p_data[0])
    {
        rsp[2] = RSCS_CTRL_PT_RSP_INVALID_PARAM;
        rscs_ctrl_pt_rsp_send(conn_idx, rsp, RSCS_CTRL_PT_RSP_LEN_MIN);
    }
    else if ((s_rscs_env.rscs_init.feature & RSCS_FEAT_MULTIPLE_SENSORS_BIT) && s_rscs_env.rscs_init.evt_handler)
    {
        event.conn_idx = conn_idx;
        event.evt_type = RSCS_EVT_SEBSOR_LOC_UPD;
        event.p_data   = p_data;
        event.length   = length;
        s_rscs_env.rscs_init.evt_handler(&event);
    }
    else
    {
        rsp[2] = RSCS_CTRL_PT_RSP_FAILED;
        rscs_ctrl_pt_rsp_send(conn_idx, rsp, RSCS_CTRL_PT_RSP_LEN_MIN);
    }
}

/**
 *****************************************************************************************
 * @brief SC Control Point Suppoted Sensor Location list request handler.
 *
 * @param[in] conn_idx: Connection index.
 *****************************************************************************************
 */
static void rscs_op_sup_sensor_loc_req_handler(uint8_t conn_idx)
{
    rscs_evt_t  event;
    uint8_t     rsp[RSCS_CTRL_PT_RSP_LEN_MIN + RSCS_SENSOR_LOC_SUP_NB];
    uint8_t     rsp_idx = RSCS_CTRL_PT_RSP_LEN_MIN;

    rsp[0] = RSCS_CTRL_PT_OP_RSP_CODE;
    rsp[1] = RSCS_CTRL_PT_OP_REQ_SUP_LOC;
    rsp[2] = RSCS_CTRL_PT_RSP_SUCCESS;

    if (s_rscs_env.rscs_init.feature & RSCS_FEAT_MULTIPLE_SENSORS_BIT)
    {
        event.conn_idx = conn_idx;
        event.evt_type = RSCS_EVT_SUP_SEBSOR_LOC_REQ;

        if (s_rscs_env.rscs_init.evt_handler)
        {
             s_rscs_env.rscs_init.evt_handler(&event);
        }

        for (uint8_t i = 0; i < RSCS_SENSOR_LOC_SUP_NB; i++)
        {
            rsp[rsp_idx++] = i;
        }
        rscs_ctrl_pt_rsp_send(conn_idx, rsp, RSCS_CTRL_PT_RSP_LEN_MIN + RSCS_SENSOR_LOC_SUP_NB);
    }
    else
    {
        rsp[2] = RSCS_CTRL_PT_RSP_FAILED;
        rscs_ctrl_pt_rsp_send(conn_idx, rsp, RSCS_CTRL_PT_RSP_LEN_MIN);
    }
}

/**
 *****************************************************************************************
 * @brief Handles reception of the disconnection event.
 *
 * @param[in] conn_idx: Connection index.
 * @param[in] reason:   Reason of disconnection.
 *****************************************************************************************
 */
static void rscs_disconnect_cb(uint8_t conn_idx, uint8_t reason)
{
    s_rscs_env.ctrl_pt_op_in_progress = false;
}

/**
 *****************************************************************************************
 * @brief SC Control Point handler.
 *
 * @param[in] conn_idx: Connection index.
 * @param[in] p_data:   Pointer to data.
 * @param[in] length:   Length of data.
 *****************************************************************************************
 */
static void rscs_sc_ctrl_pt_handler(uint8_t conn_idx, const uint8_t *p_data, uint16_t length)
{
    uint8_t rsp[RSCS_CTRL_PT_RSP_LEN_MIN];

    switch(p_data[0])
    {
        case RSCS_CTRL_PT_OP_SET_CUMUL_VAL:
            rscs_op_set_cumulative_handler(conn_idx, &p_data[1], length - 1);
            break;

        case RSCS_CTRL_PT_OP_START_CALIB:
            rscs_op_start_calibration_handler(conn_idx);
            break;

        case RSCS_CTRL_PT_OP_UPD_LOC:
            rscs_op_sensor_loc_update_handler(conn_idx, &p_data[1], length - 1);
            break;

        case RSCS_CTRL_PT_OP_REQ_SUP_LOC:
            rscs_op_sup_sensor_loc_req_handler(conn_idx);
            break;

        default:
            rsp[0] = RSCS_CTRL_PT_OP_RSP_CODE;
            rsp[1] = p_data[0];
            rsp[2] = RSCS_CTRL_PT_RSP_NOT_SUP;
            rscs_ctrl_pt_rsp_send(conn_idx, rsp, RSCS_CTRL_PT_RSP_LEN_MIN);
            break;
    }
}

/**
 *****************************************************************************************
 * @brief Encoding a RSC Measurement.
 *
 * @param[in]  p_meas:           Pointer to RSC measurement value to be encoded.
 * @param[out] p_encoded_buffer: Buffer where the encoded data will be written.
 *
 * @return Length of encoded data.
 *****************************************************************************************
 */
static uint16_t rsc_meas_value_encoded(rscs_meas_val_t *p_meas, uint8_t *p_encoded_buffer)
{
    uint8_t  flags  = 0;
    uint16_t length = 1;

    // Instantaneous speed field
    p_encoded_buffer[length++] = LO_U16(p_meas->inst_speed);
    p_encoded_buffer[length++] = HI_U16(p_meas->inst_speed);

    // Instantaneous cadence field
    p_encoded_buffer[length++] = p_meas->inst_cadence;

    // Instantaneous stride length field
    if (s_rscs_env.rscs_init.feature & RSCS_FEAT_INSTANT_STRIDE_LEN_BIT)
    {
        if (p_meas->inst_stride_length_present)
        {
            p_encoded_buffer[length++] = LO_U16(p_meas->inst_stride_length);
            p_encoded_buffer[length++] = HI_U16(p_meas->inst_stride_length);
            // Flags field
            flags |= RSCS_MEAS_FLAG_INST_STRIDE_LEN_BIT;
        }
    }

    // Total distance field
    if (s_rscs_env.rscs_init.feature & RSCS_FEAT_TOTAL_DISTANCE_BIT)
    {
        if (p_meas->total_distance_present)
        {
            p_encoded_buffer[length++] = LO_UINT32_T(p_meas->total_distance);
            p_encoded_buffer[length++] = L2_UINT32_T(p_meas->total_distance);
            p_encoded_buffer[length++] = L3_UINT32_T(p_meas->total_distance);
            p_encoded_buffer[length++] = HI_UINT32_T(p_meas->total_distance);
            // Flags field
            flags |= RSCS_MEAS_FLAG_TOTAL_DISTANCE_BIT;
        }
    }

    // Flags field
    if (s_rscs_env.rscs_init.feature & RSCS_FEAT_RUNNING_OR_WALKING_STATUS_BIT)
    {
        if (p_meas->is_run_or_walk)
        {
            flags |= RSCS_MEAS_FLAG_RUNNING_OR_WALKING_BIT;
        }
    }

    p_encoded_buffer[0] = flags;

    return length;
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
sdk_err_t rscs_measurement_send(uint8_t conn_idx, rscs_meas_val_t *p_meas)
{
    sdk_err_t        error_code = SDK_ERR_NTF_DISABLED;
    uint8_t          encoded_rsc_meas[RSCS_MEAS_VAL_LEN_MAX];
    uint16_t         length;
    gatts_noti_ind_t rsc_ntf;

    length = rsc_meas_value_encoded(p_meas, encoded_rsc_meas);

    if (PRF_CLI_START_NTF == s_rscs_env.meas_ntf_cfg[conn_idx])
    {
        rsc_ntf.type   = BLE_GATT_NOTIFICATION;
        rsc_ntf.handle = prf_find_handle_by_idx(RSCS_IDX_RSC_MEAS_VAL,
                                                s_rscs_env.start_hdl,
                                                (uint8_t *)&s_rscs_env.rscs_init.char_mask);
        rsc_ntf.length = length;
        rsc_ntf.value  = encoded_rsc_meas;
        error_code     = ble_gatts_noti_ind(conn_idx, &rsc_ntf);
    }

    return error_code;
}

sdk_err_t rscs_ctrl_pt_rsp_send(uint8_t conn_idx, uint8_t *p_data, uint16_t length)
{
    sdk_err_t        error_code = SDK_ERR_IND_DISABLED;
    gatts_noti_ind_t ctrl_pt_rsp;

    if (PRF_CLI_START_IND == s_rscs_env.ctrl_point_ind_cfg[conn_idx])
    {
        ctrl_pt_rsp.type   = BLE_GATT_INDICATION;
        ctrl_pt_rsp.handle = prf_find_handle_by_idx(RSCS_IDX_CTRL_POINT_VAL,
                                                    s_rscs_env.start_hdl,
                                                    (uint8_t *)&s_rscs_env.rscs_init.char_mask);
        ctrl_pt_rsp.length = length;
        ctrl_pt_rsp.value  = p_data;
        error_code         = ble_gatts_noti_ind(conn_idx, &ctrl_pt_rsp);
    }

    return error_code;
}

sdk_err_t rscs_sensor_loc_update(rscs_sensor_loc_t sensor_loc)
{
    sdk_err_t   error_code = BLE_SUCCESS;

    if (s_rscs_env.rscs_init.feature & RSCS_FEAT_MULTIPLE_SENSORS_BIT)
    {
        s_rscs_env.rscs_init.sensor_location = sensor_loc;
    }
    else
    {
        error_code = SDK_ERR_DISALLOWED;
    }

    return error_code;
}

sdk_err_t rscs_service_init(rscs_init_t *p_rscs_init)
{
    if (NULL == p_rscs_init)
    {
        return SDK_ERR_POINTER_NULL;
    }

    if (p_rscs_init->feature & RSCS_FEAT_MULTIPLE_SENSORS_BIT)
    {
        p_rscs_init->char_mask |= RSCS_CHAR_SENSOR_LOC_SUP;
    }
    else
    {
        p_rscs_init->char_mask &= ~RSCS_CHAR_SENSOR_LOC_SUP;
    }

    memcpy(&s_rscs_env.rscs_init, p_rscs_init, sizeof(rscs_init_t));

    return ble_server_prf_add(&rscs_prf_info);
}

