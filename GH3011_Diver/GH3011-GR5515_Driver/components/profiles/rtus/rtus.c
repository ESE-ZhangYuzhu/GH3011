/**
 ****************************************************************************************
 *
 * @file rtus.c
 *
 * @brief Reference Time Update Service implementation.
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
#include "rtus.h"
#include "ble_prf_types.h"
#include "ble_prf_utils.h"
#include "utility.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
/**@brief Reference Time Update Service Attributes Indexes. */
enum
{
    // Reference Time Update Service
    RTUS_IDX_SVC,

    // Time Update Control Point
    RTUS_IDX_CTRL_PT_CHAR,
    RTUS_IDX_CTRL_PT_VAL,

    // Time Update State
    RTUS_IDX_UPDATE_STATE_CHAR,
    RTUS_IDX_UPDATE_STATE_VAL,

    RTUS_IDX_NB
};

/*
 * STRUCTURES
 *****************************************************************************************
 */
/**@brief Reference Time Update Service environment variable. */
struct rtus_env_t
{
    rtus_init_t             rtus_init;      /**< Reference Time Update Service initialization variables. */
    uint16_t                start_hdl;      /**< Reference Time Update Service start handle. */
    rtus_update_state_t     update_state;   /**< State of time update. */
};
/*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static sdk_err_t   rtus_init(void);
static void        rtus_read_att_cb(uint8_t conidx, const gatts_read_req_cb_t *p_param);
static void        rtus_write_att_cb(uint8_t conidx, const gatts_write_req_cb_t *p_param);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static struct rtus_env_t s_rtus_env;

/**@brief Full RTUS Database Description - Used to add attributes into the database. */
static const attm_desc_t rtus_attr_tab[RTUS_IDX_NB] =
{
    // RTUS Service Declaration
    [RTUS_IDX_SVC] = {BLE_ATT_DECL_PRIMARY_SERVICE, READ_PERM_UNSEC, 0, 0},

    // Time Update Control Point Characteristic Declaration
    [RTUS_IDX_CTRL_PT_CHAR]    = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // Time Update Control Point Characteristic Declaration value
    [RTUS_IDX_CTRL_PT_VAL]     = {BLE_ATT_CHAR_TIME_UPDATE_CNTL_POINT,
                                  WRITE_CMD_PERM(AUTH),
                                  ATT_VAL_LOC_USER,
                                  RTUS_CTRL_PT_VAL_LEN},

    // Time Update State Characteristic Declaration
    [RTUS_IDX_UPDATE_STATE_CHAR]    = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // Time Update State Characteristic Declaration value
    [RTUS_IDX_UPDATE_STATE_VAL]     = {BLE_ATT_CHAR_TIME_UPDATE_STATE,
                                       READ_PERM(AUTH),
                                       ATT_VAL_LOC_USER,
                                       RTUS_UPDATE_STATE_VAL_LEN},
};

/**@brief RTUS Task interface required by profile manager. */
static ble_prf_manager_cbs_t rtus_tack_cbs =
{
    (prf_init_func_t) rtus_init,
    NULL,
    NULL
};

/**@brief RTUS Task Callbacks. */
static gatts_prf_cbs_t rtus_cb_func =
{
    rtus_read_att_cb,
    rtus_write_att_cb,
    NULL,
    NULL
};

/**@brief RTUS Information. */
static const prf_server_info_t rtus_prf_info =
{
    .max_connection_nb = RTUS_CONNECTION_MAX,
    .manager_cbs       = &rtus_tack_cbs,
    .gatts_prf_cbs     = &rtus_cb_func
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 * @brief Initialize Reference Time Update service and create db in att
 *
 * @return Error code to know if profile initialization succeed or not.
 *****************************************************************************************
 */
static sdk_err_t rtus_init(void)
{
    // The start hanlde must be set with PRF_INVALID_HANDLE to be allocated automatically by BLE Stack.
    uint16_t          start_hdl      = PRF_INVALID_HANDLE;
    const uint8_t     rtus_svc_uuid[] = BLE_ATT_16_TO_16_ARRAY(BLE_ATT_SVC_REF_TIME_UPDATE);
    sdk_err_t         error_code;
    gatts_create_db_t gatts_db;

    memset(&gatts_db, 0, sizeof(gatts_db));

    gatts_db.shdl                 = &start_hdl;
    gatts_db.uuid                 = rtus_svc_uuid;
    gatts_db.attr_tab_cfg         = (uint8_t *)&(s_rtus_env.rtus_init.char_mask);
    gatts_db.max_nb_attr          = RTUS_IDX_NB;
    gatts_db.srvc_perm            = 0;
    gatts_db.attr_tab_type        = SERVICE_TABLE_TYPE_16;
    gatts_db.attr_tab.attr_tab_16 = rtus_attr_tab;

    error_code = ble_gatts_srvc_db_create(&gatts_db);

    if (SDK_SUCCESS == error_code)
    {
        s_rtus_env.start_hdl = *gatts_db.shdl;
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
static void rtus_read_att_cb(uint8_t conn_idx, const gatts_read_req_cb_t *p_param)
{
    gatts_read_cfm_t  cfm;
    uint8_t           handle    = p_param->handle;
    uint8_t           tab_index = prf_find_idx_by_handle(handle,
                                  s_rtus_env.start_hdl,
                                  RTUS_IDX_NB,
                                  (uint8_t *)&s_rtus_env.rtus_init.char_mask);
    cfm.handle = handle;
    cfm.status = BLE_SUCCESS;

    switch (tab_index)
    {
        case RTUS_IDX_UPDATE_STATE_VAL:
            cfm.length = RTUS_UPDATE_STATE_VAL_LEN;
            cfm.value  = (uint8_t *)&s_rtus_env.update_state;
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
static void rtus_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param)
{
    uint16_t          handle      = p_param->handle;
    uint16_t          tab_index   = 0;
    rtus_ctrl_pt_t    ctrl_pt;
    rtus_evt_t        event;
    gatts_write_cfm_t cfm;

    tab_index  = prf_find_idx_by_handle(handle,
                                        s_rtus_env.start_hdl,
                                        RTUS_IDX_NB,
                                        (uint8_t *)&s_rtus_env.rtus_init.char_mask);
    cfm.handle     = handle;
    cfm.status     = BLE_SUCCESS;
    event.evt_type = RTUS_EVT_INVALID;
    event.conn_idx = conn_idx;

    switch (tab_index)
    {
        case RTUS_IDX_CTRL_PT_VAL:
            ctrl_pt = (rtus_ctrl_pt_t)p_param->value[0];

            if (RTUS_CTRL_PT_GET_UPDATE == ctrl_pt)
            {
                event.evt_type = RTUS_EVT_GET_UPDATE;
            }
            else if (RTUS_CTRL_PT_CANCEL_UPDATE == ctrl_pt)
            {
                event.evt_type = RTUS_EVT_CANCEL_UPDATE;
            }
            else
            {
                break;
            }

        default:
            cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
            break;
    }

    ble_gatts_write_cfm(conn_idx, &cfm);

    if (BLE_ATT_ERR_INVALID_HANDLE != cfm.status && RTUS_EVT_INVALID != event.evt_type && s_rtus_env.rtus_init.evt_handler)
    {
        s_rtus_env.rtus_init.evt_handler(&event);
    }
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
void rtus_current_state_set(rtus_cur_state_t cur_state)
{
    s_rtus_env.update_state.cur_state = cur_state;
}

void rtus_update_result_set(rtus_update_result_t update_result)
{
    s_rtus_env.update_state.update_result = update_result;
}

sdk_err_t rtus_service_init(rtus_init_t *p_rtus_init)
{
    if (NULL == p_rtus_init)
    {
        return SDK_ERR_POINTER_NULL;
    }

    memcpy(&s_rtus_env.rtus_init, p_rtus_init, sizeof(rtus_init_t));

    return ble_server_prf_add(&rtus_prf_info);
}

