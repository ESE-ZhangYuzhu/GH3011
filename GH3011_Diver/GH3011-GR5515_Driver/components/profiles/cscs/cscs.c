/**
 *****************************************************************************************
 *
 * @file cscs.c
 *
 * @brief Cycling Speed and Cadence Profile Sensor implementation.
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
#include "cscs.h"
#include "ble_prf_types.h"
#include "ble_prf_utils.h"
#include "utility.h"

/*
 * ENUMERATIONS
 *****************************************************************************************
 */
/**@brief Cycling Speed and Cadence Service Attributes Indexes. */
enum
{
    CSCS_IDX_SVC,

    CSCS_IDX_CSC_MEAS_CHAR,
    CSCS_IDX_CSC_MEAS_VAL,
    CSCS_IDX_CSC_MEAS_NTF_CFG,

    CSCS_IDX_CSC_FEAT_CHAR,
    CSCS_IDX_CSC_FEAT_VAL,

    CSCS_IDX_SENSOR_LOC_CHAR,
    CSCS_IDX_SENSOR_LOC_VAL,

    CSCS_IDX_CTRL_POINT_CHAR,
    CSCS_IDX_CTRL_POINT_VAL,
    CSCS_IDX_CTRL_POINT_IND_CFG,

    CSCS_IDX_NB
};

/*
 * STRUCTURES
 *****************************************************************************************
 */
/**@brief Cycling Speed and Cadence Service environment variable. */
struct cscs_env_t
{
    cscs_init_t  cscs_init;                               /**< Cycling Speed and Cadence Service initialization variables. */
    uint16_t     start_hdl;                               /**< Cycling Speed and Cadence Service start handle. */
    bool         ctrl_pt_op_in_progress;                  /**< A previously triggered SC Control Point operation is still in progress. */
    bool         ctrl_pt_op_rsp_cplt;                     /**< A previously triggered SC Control Point operation response cplt. */
    uint16_t     meas_ntf_cfg[CSCS_CONNECTION_MAX];       /**< The configuration of CSC Measurement Notification which is configured by the peer devices. */
    uint16_t     ctrl_point_ind_cfg[CSCS_CONNECTION_MAX]; /**< The configuration of SC Control Point Notification which is configured by the peer devices. */
};

/*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static sdk_err_t cscs_init(void);
static void      cscs_read_att_cb(uint8_t  conn_idx, const gatts_read_req_cb_t  *p_param);
static void      cscs_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param);
static void      cscs_cccd_set_cb(uint8_t conn_idx, uint16_t handle, uint16_t cccd_value);
static void      cscs_disconnect_cb(uint8_t conn_idx, uint8_t reason);
static void      cscs_ntf_ind_cb(uint8_t conn_idx, uint8_t status, const ble_gatts_ntf_ind_t *p_ntf_ind);
static void      cscs_sc_ctrl_pt_handler(uint8_t conn_idx, const uint8_t *p_data, uint16_t length);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static struct cscs_env_t s_cscs_env;

/**@brief Full CSCS Database Description - Used to add attributes into the database. */
static const attm_desc_t rscs_attr_tab[CSCS_IDX_NB]  =
{
    // Cycling Speed and Cadence Service Declaration
    [CSCS_IDX_SVC]              = {BLE_ATT_DECL_PRIMARY_SERVICE, READ_PERM_UNSEC, 0, 0},

    // CSC Measurement Characteristic - Declaration
    [CSCS_IDX_CSC_MEAS_CHAR]    = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // CSC Measurement Characteristic - Value
    [CSCS_IDX_CSC_MEAS_VAL]     = {BLE_ATT_CHAR_CSC_MEAS,
                                   NOTIFY_PERM_UNSEC, 
                                   ATT_VAL_LOC_USER,
                                   CSCS_MEAS_VAL_LEN_MAX},
    // CSC Measurement Characteristic - Client Characteristic Configuration Descriptor
    [CSCS_IDX_CSC_MEAS_NTF_CFG] = {BLE_ATT_DESC_CLIENT_CHAR_CFG,
                                   READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC,
                                   0,
                                   0},

    // CS Feature Characteristic - Declaration
    [CSCS_IDX_CSC_FEAT_CHAR]    = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // CSC Feature Characteristic - Value
    [CSCS_IDX_CSC_FEAT_VAL]     = {BLE_ATT_CHAR_CSC_FEAT,
                                   READ_PERM_UNSEC,
                                   ATT_VAL_LOC_USER,
                                   CSCS_FEAT_VAL_LEN_MAX},

    // Sensor Location Characteristic - Declaration
    [CSCS_IDX_SENSOR_LOC_CHAR]  = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // Sensor Location Characteristic - Value
    [CSCS_IDX_SENSOR_LOC_VAL]   = {BLE_ATT_CHAR_SENSOR_LOC,
                                   READ_PERM_UNSEC,
                                   ATT_VAL_LOC_USER,
                                   CSCS_SENSOR_LOC_VAL_LEN_MAX},

    // SC Control Point Characteristic - Declaration
    [CSCS_IDX_CTRL_POINT_CHAR]    = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // SC Control Point Characteristic - Value
    [CSCS_IDX_CTRL_POINT_VAL]     = {BLE_ATT_CHAR_SC_CNTL_PT,
                                     WRITE_REQ_PERM_UNSEC | INDICATE_PERM_UNSEC,
                                     ATT_VAL_LOC_USER,
                                     CSCS_CTRL_PT_VAL_LEN_MAX},
    // SC Control Point Characteristic - Client Characteristic Configuration Descriptor
    [CSCS_IDX_CTRL_POINT_IND_CFG] = {BLE_ATT_DESC_CLIENT_CHAR_CFG,
                                     READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC,
                                     0,
                                     0},
};

/**@brief CSCS Task interface required by profile manager. */
static ble_prf_manager_cbs_t cscs_task_cbs =
{
    (prf_init_func_t) cscs_init,
    NULL,
    cscs_disconnect_cb
};

/**@brief CSCS Task Callbacks. */
static gatts_prf_cbs_t cscs_cb_func =
{
    cscs_read_att_cb,
    cscs_write_att_cb,
    NULL,
    cscs_ntf_ind_cb,
    cscs_cccd_set_cb
};

/**@brief CSCS Information. */
static const prf_server_info_t cscs_prf_info =
{
    .max_connection_nb = CSCS_CONNECTION_MAX,
    .manager_cbs       = &cscs_task_cbs,
    .gatts_prf_cbs     = &cscs_cb_func,
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 * @brief Initialize Cycling Speed and Cadence service  create db in att
 *
 * @return Error code to know if profile initialization succeed or not.
 *****************************************************************************************
 */
static sdk_err_t cscs_init(void)
{
    // The start hanlde must be set with PRF_INVALID_HANDLE to be allocated automatically by BLE Stack.
    uint16_t          start_hdl       = PRF_INVALID_HANDLE;
    const uint8_t     rscs_svc_uuid[] = BLE_ATT_16_TO_16_ARRAY(BLE_ATT_SVC_CYCLING_SPEED_CADENCE);
    sdk_err_t         error_code;
    gatts_create_db_t gatts_db; 

    memset(&gatts_db, 0, sizeof(gatts_db));

    gatts_db.shdl                 = &start_hdl;
    gatts_db.uuid                 = rscs_svc_uuid;
    gatts_db.attr_tab_cfg         = (uint8_t *)&(s_cscs_env.cscs_init.char_mask);
    gatts_db.max_nb_attr          = CSCS_IDX_NB;
    gatts_db.srvc_perm            = 0; 
    gatts_db.attr_tab_type        = SERVICE_TABLE_TYPE_16;
    gatts_db.attr_tab.attr_tab_16 = rscs_attr_tab;

    error_code = ble_gatts_srvc_db_create(&gatts_db);

    if (SDK_SUCCESS == error_code)
    {
        s_cscs_env.start_hdl = *gatts_db.shdl;
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
static void   cscs_read_att_cb(uint8_t conn_idx, const gatts_read_req_cb_t *p_param)
{
    gatts_read_cfm_t  cfm;
    uint8_t           handle    = p_param->handle;
    uint8_t           tab_index = prf_find_idx_by_handle(handle, 
                                                         s_cscs_env.start_hdl,
                                                         CSCS_IDX_NB,
                                                        (uint8_t *)&s_cscs_env.cscs_init.char_mask);
    cfm.handle = handle;
    cfm.status = BLE_SUCCESS;

    switch (tab_index)
    {
        case CSCS_IDX_CSC_MEAS_NTF_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_cscs_env.meas_ntf_cfg[conn_idx];
            break;

        case CSCS_IDX_CSC_FEAT_VAL:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_cscs_env.cscs_init.feature;
            break;

        case CSCS_IDX_SENSOR_LOC_VAL:
            cfm.length = sizeof(uint8_t);
            cfm.value  = (uint8_t *)&s_cscs_env.cscs_init.sensor_location;
            break;
 
        case CSCS_IDX_CTRL_POINT_IND_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_cscs_env.ctrl_point_ind_cfg[conn_idx];
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
static void   cscs_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param)
{
    uint16_t          handle      = p_param->handle;
    uint16_t          tab_index   = 0;
    uint16_t          cccd_value  = 0;
    bool              ctrl_pt_evt = false;
    cscs_evt_t        event;
    gatts_write_cfm_t cfm;

    tab_index  = prf_find_idx_by_handle(handle,
                                        s_cscs_env.start_hdl,
                                        CSCS_IDX_NB,
                                        (uint8_t *)&s_cscs_env.cscs_init.char_mask);
    cfm.handle     = handle;
    cfm.status     = BLE_SUCCESS;
    event.evt_type = CSCS_EVT_INVALID;
    event.conn_idx = conn_idx;
 
    switch (tab_index)
    {
        case CSCS_IDX_CSC_MEAS_NTF_CFG:
            cccd_value     = le16toh(&p_param->value[0]);
            event.evt_type = ((PRF_CLI_START_NTF == cccd_value) ?\
                               CSCS_EVT_CSC_MEAS_NOTIFICATION_ENABLE :\
                               CSCS_EVT_CSC_MEAS_NOTIFICATION_DISABLE);
            s_cscs_env.meas_ntf_cfg[conn_idx] = cccd_value;
            break;

        case CSCS_IDX_CTRL_POINT_VAL:
            if (PRF_CLI_START_IND != s_cscs_env.ctrl_point_ind_cfg[conn_idx])
            {
                cfm.status = CSCS_ERROR_CCCD_INVALID;
                break;
            }
            else if (s_cscs_env.ctrl_pt_op_in_progress)
            {
                cfm.status = CSCS_ERROR_PROC_IN_PROGRESS;
            }
            else if (PRF_CLI_START_IND == s_cscs_env.ctrl_point_ind_cfg[conn_idx])
            {
                s_cscs_env.ctrl_pt_op_in_progress = true;
                ctrl_pt_evt = true;
            }
            break;

        case CSCS_IDX_CTRL_POINT_IND_CFG:
            cccd_value     = le16toh(&p_param->value[0]);
            event.evt_type = ((PRF_CLI_START_IND == cccd_value) ?\
                               CSCS_EVT_CTRL_POINT_INDICATION_ENABLE :\
                               CSCS_EVT_CTRL_POINT_INDICATION_DISABLE);
            s_cscs_env.ctrl_point_ind_cfg[conn_idx] = cccd_value;
            break;

        default:
            cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
            break;
    }

    ble_gatts_write_cfm(conn_idx, &cfm);

    if (ctrl_pt_evt)
    {
        cscs_sc_ctrl_pt_handler(conn_idx, p_param->value, p_param->length);
    }
    else if (BLE_ATT_ERR_INVALID_HANDLE != cfm.status && CSCS_EVT_INVALID != event.evt_type && s_cscs_env.cscs_init.evt_handler)
    {
        s_cscs_env.cscs_init.evt_handler(&event);
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
static void cscs_cccd_set_cb(uint8_t conn_idx, uint16_t handle, uint16_t cccd_value)
{
    uint16_t          tab_index  = 0;
    cscs_evt_t        event;

    if (!prf_is_cccd_value_valid(cccd_value))
    {
        return;
    }

    tab_index  = prf_find_idx_by_handle(handle,
                                        s_cscs_env.start_hdl,
                                        CSCS_IDX_NB,
                                        (uint8_t *)&s_cscs_env.cscs_init.char_mask);

    event.evt_type = CSCS_EVT_INVALID;
    event.conn_idx = conn_idx;
 
    switch (tab_index)
    {
        case CSCS_IDX_CSC_MEAS_NTF_CFG:
            event.evt_type = ((PRF_CLI_START_NTF == cccd_value) ?\
                               CSCS_EVT_CSC_MEAS_NOTIFICATION_ENABLE :\
                               CSCS_EVT_CSC_MEAS_NOTIFICATION_DISABLE);
            s_cscs_env.meas_ntf_cfg[conn_idx] = cccd_value;
            break;

        case CSCS_IDX_CTRL_POINT_IND_CFG:
            event.evt_type = ((PRF_CLI_START_IND == cccd_value) ?\
                               CSCS_EVT_CTRL_POINT_INDICATION_ENABLE :\
                               CSCS_EVT_CTRL_POINT_INDICATION_DISABLE);
            s_cscs_env.ctrl_point_ind_cfg[conn_idx] = cccd_value;
            break;

        default:
            break;
    }


    if (CSCS_EVT_INVALID != event.evt_type && s_cscs_env.cscs_init.evt_handler)
    {
        s_cscs_env.cscs_init.evt_handler(&event);
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
static void cscs_ntf_ind_cb(uint8_t conn_idx, uint8_t status, const ble_gatts_ntf_ind_t *p_ntf_ind)
{
    cscs_evt_t  event;

    event.evt_type = CSCS_EVT_INVALID;
    event.conn_idx = conn_idx;

    if (s_cscs_env.cscs_init.evt_handler && SDK_SUCCESS == status)
    {
        if (BLE_GATT_NOTIFICATION == p_ntf_ind->type)
        {
            event.evt_type = CSCS_EVT_CSC_MEAS_SEND_CPLT;
        }
        else if (BLE_GATT_INDICATION == p_ntf_ind->type)
        {
            event.evt_type = CSCS_EVT_CTRL_POINT_RSP_CPLT;
            s_cscs_env.ctrl_pt_op_in_progress = false;
        }
        s_cscs_env.cscs_init.evt_handler(&event);
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
static void cscs_disconnect_cb(uint8_t conn_idx, uint8_t reason)
{
    s_cscs_env.ctrl_pt_op_in_progress = false;
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
static void cscs_op_set_cumulative_handler(uint8_t conn_idx, const uint8_t *p_data, uint16_t length)
{
    cscs_evt_t  event;
    uint8_t     rsp[CSCS_CTRL_PT_RSP_LEN_MIN];

    rsp[0] = CSCS_CTRL_PT_OP_RSP_CODE;
    rsp[1] = CSCS_CTRL_PT_OP_SET_CUMUL_VAL;
    rsp[2] = CSCS_CTRL_PT_RSP_FAILED;

    if ((sizeof(uint32_t) == length) && \
        (s_cscs_env.cscs_init.feature & CSCS_FEAT_WHEEL_REVOLUTION_SUP_BIT) && \
        (s_cscs_env.cscs_init.evt_handler))
    {
        event.conn_idx = conn_idx;
        event.evt_type = CSCS_EVT_CUMUL_VAL_SET;
        event.p_data   = p_data;
        event.length   = length;
        s_cscs_env.cscs_init.evt_handler(&event);
    }
    else
    {
        cscs_ctrl_pt_rsp_send(conn_idx, rsp, CSCS_CTRL_PT_RSP_LEN_MIN);
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
static void cscs_op_sensor_loc_update_handler(uint8_t conn_idx, const uint8_t *p_data, uint16_t length)
{
    cscs_evt_t  event;
    uint8_t     rsp[CSCS_CTRL_PT_RSP_LEN_MIN];

    rsp[0] = CSCS_CTRL_PT_OP_RSP_CODE;
    rsp[1] = CSCS_CTRL_PT_OP_UPD_LOC;

    if (CSCS_SENSOR_LOC_SUP_NB <= p_data[0])
    {
        rsp[2] = CSCS_CTRL_PT_RSP_INVALID_PARAM;
        cscs_ctrl_pt_rsp_send(conn_idx, rsp, CSCS_CTRL_PT_RSP_LEN_MIN);
    }
    else if ((s_cscs_env.cscs_init.feature & CSCS_FEAT_MULTIPLE_SENSORS_BIT) && s_cscs_env.cscs_init.evt_handler)
    {
        event.conn_idx = conn_idx;
        event.evt_type = CSCS_EVT_SEBSOR_LOC_UPD;
        event.p_data   = p_data;
        event.length   = length;
        s_cscs_env.cscs_init.evt_handler(&event);
    }
    else
    {
        rsp[2] = CSCS_CTRL_PT_RSP_FAILED;
        cscs_ctrl_pt_rsp_send(conn_idx, rsp, CSCS_CTRL_PT_RSP_LEN_MIN);
    }
}

/**
 *****************************************************************************************
 * @brief SC Control Point Suppoted Sensor Location list request handler.
 *
 * @param[in] conn_idx: Connection index.
 *****************************************************************************************
 */
static void cscs_op_sup_sensor_loc_req_handler(uint8_t conn_idx)
{
    cscs_evt_t  event;
    uint8_t     rsp[CSCS_CTRL_PT_RSP_LEN_MIN + CSCS_SENSOR_LOC_SUP_NB];
    uint8_t     rsp_idx = CSCS_CTRL_PT_RSP_LEN_MIN;

    rsp[0] = CSCS_CTRL_PT_OP_RSP_CODE;
    rsp[1] = CSCS_CTRL_PT_OP_REQ_SUP_LOC;
    rsp[2] = CSCS_CTRL_PT_RSP_SUCCESS;

    if (s_cscs_env.cscs_init.feature & CSCS_FEAT_MULTIPLE_SENSORS_BIT)
    {
        event.conn_idx = conn_idx;
        event.evt_type = CSCS_EVT_SUP_SEBSOR_LOC_REQ;

        if (s_cscs_env.cscs_init.evt_handler)
        {
             s_cscs_env.cscs_init.evt_handler(&event);
        }

        for (uint8_t i = 0; i < CSCS_SENSOR_LOC_SUP_NB; i++)
        {
            rsp[rsp_idx++] = i;
        }
        cscs_ctrl_pt_rsp_send(conn_idx, rsp, CSCS_CTRL_PT_RSP_LEN_MIN + CSCS_SENSOR_LOC_SUP_NB);
    }
    else
    {
        rsp[2] = CSCS_CTRL_PT_RSP_FAILED;
        cscs_ctrl_pt_rsp_send(conn_idx, rsp, CSCS_CTRL_PT_RSP_LEN_MIN);
    }
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
static void cscs_sc_ctrl_pt_handler(uint8_t conn_idx, const uint8_t *p_data, uint16_t length)
{
    uint8_t rsp[CSCS_CTRL_PT_RSP_LEN_MIN];

    switch(p_data[0])
    {
        case CSCS_CTRL_PT_OP_SET_CUMUL_VAL:
            cscs_op_set_cumulative_handler(conn_idx, &p_data[1], length - 1);
            break;

        case CSCS_CTRL_PT_OP_START_CALIB:
            rsp[0] = CSCS_CTRL_PT_OP_RSP_CODE;
            rsp[1] = p_data[0];
            rsp[2] = CSCS_CTRL_PT_RSP_NOT_SUP;
            cscs_ctrl_pt_rsp_send(conn_idx, rsp, CSCS_CTRL_PT_RSP_LEN_MIN);
            break;

        case CSCS_CTRL_PT_OP_UPD_LOC:
            cscs_op_sensor_loc_update_handler(conn_idx, &p_data[1], length - 1);
            break;

        case CSCS_CTRL_PT_OP_REQ_SUP_LOC:
            cscs_op_sup_sensor_loc_req_handler(conn_idx);
            break;

        default:
            rsp[0] = CSCS_CTRL_PT_OP_RSP_CODE;
            rsp[1] = p_data[0];
            rsp[2] = CSCS_CTRL_PT_RSP_NOT_SUP;
            cscs_ctrl_pt_rsp_send(conn_idx, rsp, CSCS_CTRL_PT_RSP_LEN_MIN);
            break;
    }
}

/**
 *****************************************************************************************
 * @brief Encoding a CSC Measurement.
 *
 * @param[in]  p_meas:           Pointer to CSC measurement value to be encoded.
 * @param[out] p_encoded_buffer: Buffer where the encoded data will be written.
 *
 * @return Length of encoded data.
 *****************************************************************************************
 */
static uint16_t csc_meas_value_encoded(cscs_meas_val_t *p_meas, uint8_t *p_encoded_buffer)
{
    uint8_t  flags  = 0;
    uint16_t length = 1;

    // Cumulative Wheel Revolutions and Last Wheel Event Time Fields
    if (s_cscs_env.cscs_init.feature & CSCS_FEAT_WHEEL_REVOLUTION_SUP_BIT)
    {
        if (p_meas->wheel_rev_data_present)
        {
            p_encoded_buffer[length++] = LO_UINT32_T(p_meas->cumulative_wheel_revs);
            p_encoded_buffer[length++] = L2_UINT32_T(p_meas->cumulative_wheel_revs);
            p_encoded_buffer[length++] = L3_UINT32_T(p_meas->cumulative_wheel_revs);
            p_encoded_buffer[length++] = HI_UINT32_T(p_meas->cumulative_wheel_revs);
            p_encoded_buffer[length++] = LO_U16(p_meas->last_wheel_event_time);
            p_encoded_buffer[length++] = HI_U16(p_meas->last_wheel_event_time);

            // Flags field
            flags |= CSCS_MEAS_FLAG_WHEEL_REVOLUTION_BIT;
        }
    }

    // Cumulative Crank Revolutions and Last Crank Event Time Fields
    if (s_cscs_env.cscs_init.feature & CSCS_FEAT_CRANK_REVOLUTION_SUP_BIT)
    {
        if (p_meas->crank_rev_data_present)
        {
            p_encoded_buffer[length++] = LO_U16(p_meas->cumulative_crank_revs);
            p_encoded_buffer[length++] = HI_U16(p_meas->cumulative_crank_revs);
            p_encoded_buffer[length++] = LO_U16(p_meas->last_crank_event_time);
            p_encoded_buffer[length++] = HI_U16(p_meas->last_crank_event_time);

            // Flags field
            flags |= CSCS_MEAS_FLAG_CRANK_REVOLUTION_BIT;
        }
    }

    p_encoded_buffer[0] = flags;

    return length;
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
sdk_err_t cscs_measurement_send(uint8_t conn_idx, cscs_meas_val_t *p_meas)
{
    sdk_err_t        error_code = SDK_ERR_NTF_DISABLED;
    uint8_t          encoded_csc_meas[CSCS_MEAS_VAL_LEN_MAX];
    uint16_t         length;
    gatts_noti_ind_t csc_ntf;

    length = csc_meas_value_encoded(p_meas, encoded_csc_meas);

    if (PRF_CLI_START_NTF == s_cscs_env.meas_ntf_cfg[conn_idx])
    {
        csc_ntf.type   = BLE_GATT_NOTIFICATION;
        csc_ntf.handle = prf_find_handle_by_idx(CSCS_IDX_CSC_MEAS_VAL,
                                                s_cscs_env.start_hdl,
                                                (uint8_t *)&s_cscs_env.cscs_init.char_mask);
        csc_ntf.length = length;
        csc_ntf.value  = encoded_csc_meas;
        error_code     = ble_gatts_noti_ind(conn_idx, &csc_ntf);
    }

    return error_code;
}

sdk_err_t cscs_ctrl_pt_rsp_send(uint8_t conn_idx, uint8_t *p_data, uint16_t length)
{
    sdk_err_t        error_code = SDK_ERR_IND_DISABLED;
    gatts_noti_ind_t ctrl_pt_rsp;

    if (PRF_CLI_START_IND == s_cscs_env.ctrl_point_ind_cfg[conn_idx])
    {
        ctrl_pt_rsp.type   = BLE_GATT_INDICATION;
        ctrl_pt_rsp.handle = prf_find_handle_by_idx(CSCS_IDX_CTRL_POINT_VAL,
                                                    s_cscs_env.start_hdl,
                                                    (uint8_t *)&s_cscs_env.cscs_init.char_mask);
        ctrl_pt_rsp.length = length;
        ctrl_pt_rsp.value  = p_data;
        error_code         = ble_gatts_noti_ind(conn_idx, &ctrl_pt_rsp);
    }

    return error_code;
}

sdk_err_t cscs_sensor_loc_update(cscs_sensor_loc_t sensor_loc)
{
    sdk_err_t   error_code = BLE_SUCCESS;

    if (s_cscs_env.cscs_init.feature & CSCS_FEAT_MULTIPLE_SENSORS_BIT)
    {
        s_cscs_env.cscs_init.sensor_location = sensor_loc;
    }
    else
    {
        error_code = SDK_ERR_DISALLOWED;
    }

    return error_code;
}

sdk_err_t cscs_service_init(cscs_init_t *p_cscs_init)
{
    if (NULL == p_cscs_init)
    {
        return SDK_ERR_POINTER_NULL;
    }

    if (p_cscs_init->feature & CSCS_FEAT_MULTIPLE_SENSORS_BIT)
    {
        p_cscs_init->char_mask |= CSCS_CHAR_SENSOR_LOC_SUP;
    }
    else
    {
        p_cscs_init->char_mask &= ~CSCS_CHAR_SENSOR_LOC_SUP;
    }

    memcpy(&s_cscs_env.cscs_init, p_cscs_init, sizeof(cscs_init_t));

    return ble_server_prf_add(&cscs_prf_info);
}
