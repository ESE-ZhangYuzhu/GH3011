/**
 *****************************************************************************************
 *
 * @file ias.c
 *
 * @brief Immediate Alert Server Implementation.
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
#include "ias.h"
#include "ble_prf_types.h"
#include "ble_prf_utils.h"
#include "utility.h"

/*
 * DEFINITIONS
 *****************************************************************************************
 */
#define INITIAL_ALERT_LEVEL IAS_ALERT_NONE

/*
 * ENUMERATIONS
 *****************************************************************************************
 */
/**@brief IAS Attributes database index list. */
enum ias_attr_idx_t
{
    IAS_IDX_SVC,

    IAS_IDX_ALERT_LVL_CHAR,
    IAS_IDX_ALERT_LVL_VAL,

    IAS_IDX_NB,
};

/**@brief Immediate Alert Service environment variable. */
struct ias_env_t
{
    ias_init_t ias_init;              /**< Immediate Alert Service initialization variables. */
    uint16_t   start_hdl;             /**< Immediate Alert Service start handle. */
};
/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static struct ias_env_t s_ias_env;   /**< Immediate Alert Service instance. */
static uint8_t s_char_mask = 0x07;   /**< Features added into ATT database.
                                      *   bit0 - Immediate Alert Service Declaration
                                      *   bit1 - Alert Level Characteristic Declaration
                                      *   bit2 - Alert Level Characteristic Value
                                      */

/**@brief IAS Database Description - Used to add attributes into the database. */ 
static const attm_desc_t ias_attr_tab[IAS_IDX_NB] =
{
    // Immediate Alert Service Declaration
    [IAS_IDX_SVC]            = {BLE_ATT_DECL_PRIMARY_SERVICE, READ_PERM_UNSEC, 0, 0},
    // Alert Level Characteristic Declaration
    [IAS_IDX_ALERT_LVL_CHAR] = {BLE_ATT_DECL_CHARACTERISTIC,  READ_PERM_UNSEC, 0, 0},
    // Alert Level Characteristic Value
    [IAS_IDX_ALERT_LVL_VAL]  = {BLE_ATT_CHAR_ALERT_LEVEL,     WRITE_CMD_PERM_UNSEC, 0, sizeof(uint8_t)},
};

/*
 * LOCAL FUNCTION DECLARATIONS
 *****************************************************************************************
 */
static sdk_err_t   ias_init(void);
static void        ias_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param);

/**@brief IAS interface required by profile manager. */
static ble_prf_manager_cbs_t ias_mgr_cbs =
{
    (prf_init_func_t)ias_init,
    NULL,
    NULL
};

/**@brief IAS GATT server Callbacks. */
static gatts_prf_cbs_t ias_gatts_cbs =
{
    NULL,
    ias_write_att_cb,
    NULL,
    NULL
};

/**@brief IAS Information. */
static const prf_server_info_t ias_prf_info =
{
    /* There shall be only one connection with a device. */
    .max_connection_nb = 1,
    .manager_cbs       = &ias_mgr_cbs,
    .gatts_prf_cbs     = &ias_gatts_cbs
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 * @brief Initialize Immediate Alert Service and create DB in ATT.
 *
 * @return Error code to know if service initialization succeed or not.
 *****************************************************************************************
 */
static sdk_err_t ias_init(void)
{
    const uint8_t ias_svc_uuid[] = BLE_ATT_16_TO_16_ARRAY(BLE_ATT_SVC_IMMEDIATE_ALERT);
    gatts_create_db_t gatts_db;
    uint16_t start_hdl = PRF_INVALID_HANDLE; /* The start hanlde is an in/out
                                              * parameter of ble_gatts_srvc_db_create().
                                              * It must be set with PRF_INVALID_HANDLE
                                              * to be allocated automatically by BLE Stack.*/

    memset(&gatts_db, 0, sizeof(gatts_db));

    gatts_db.shdl                 = &start_hdl;
    gatts_db.uuid                 = (uint8_t *)ias_svc_uuid;
    gatts_db.attr_tab_cfg         = &s_char_mask;
    gatts_db.max_nb_attr          = IAS_IDX_NB;
    gatts_db.srvc_perm            = 0;
    gatts_db.attr_tab_type        = SERVICE_TABLE_TYPE_16;
    gatts_db.attr_tab.attr_tab_16 = ias_attr_tab;
    
    sdk_err_t   error_code = ble_gatts_srvc_db_create(&gatts_db);
    
    if (SDK_SUCCESS == error_code)
    {
        s_ias_env.start_hdl = *gatts_db.shdl;

        uint16_t handle = prf_find_handle_by_idx(IAS_IDX_ALERT_LVL_VAL, s_ias_env.start_hdl, &s_char_mask);
        uint8_t initial_alert_level = INITIAL_ALERT_LEVEL;
        
        ble_gatts_value_set(handle, sizeof(uint8_t), 0, &initial_alert_level);
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
static void   ias_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param)
{
    uint16_t handle = prf_find_handle_by_idx(IAS_IDX_ALERT_LVL_VAL, s_ias_env.start_hdl, &s_char_mask);
    gatts_write_cfm_t cfm;

    cfm.handle = p_param->handle;

    if (handle != p_param->handle)
    {
        cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
    }
    else
    {
        if (p_param->length != sizeof(uint8_t))
        {
            cfm.status = SDK_ERR_INVALID_ATT_VAL_LEN;
        } 
        else 
        {
            //Send write response
            uint8_t value = p_param->value[0];

            cfm.status = (uint8_t)ble_gatts_value_set(cfm.handle, sizeof(uint8_t), 0, &value);
            if (BLE_SUCCESS == cfm.status)
            {
                /* Alert level updated by the peer, notify app the event. */
                if (s_ias_env.ias_init.evt_handler)
                {
                    ias_evt_t evt;

                    evt.evt_type    = IAS_EVT_ALERT_LEVEL_UPDATED;
                    evt.alert_level = value;
                    s_ias_env.ias_init.evt_handler(&evt);
                }
            }
        }
    }

    ble_gatts_write_cfm(conn_idx, &cfm);
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
sdk_err_t ias_alert_level_get(uint8_t *p_alert_level)
{
    uint16_t handle = prf_find_handle_by_idx(IAS_IDX_ALERT_LVL_VAL, s_ias_env.start_hdl, (uint8_t *)&s_char_mask);
    uint16_t length = sizeof(uint8_t);

    return ble_gatts_value_get(handle, &length, p_alert_level);
}

sdk_err_t ias_service_init(ias_init_t *p_ias_init)
{
    if (NULL == p_ias_init)
    {
        return SDK_ERR_POINTER_NULL;
    }

    s_ias_env.ias_init.evt_handler = p_ias_init->evt_handler;

    return ble_server_prf_add(&ias_prf_info);
}
