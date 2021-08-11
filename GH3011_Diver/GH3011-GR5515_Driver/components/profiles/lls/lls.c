/**
 *****************************************************************************************
 *
 * @file bas.c
 *
 * @brief Link Loss Server Implementation.
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
#include "lls.h"
#include "ble_prf_types.h"
#include "ble_prf_utils.h"
#include "utility.h"

/*
 * ENUMERATIONS
 *****************************************************************************************
 */
/**@brief LLS Attributes database index list. */
enum lls_attr_idx_t
{
    LLS_IDX_SVC,

    LLS_IDX_ALERT_LVL_CHAR,
    LLS_IDX_ALERT_LVL_VAL,

    LLS_IDX_NB,
};

/*
 * STRUCTURES
 *****************************************************************************************
 */
/**@brief Link Loss Service environment variable. */
struct lls_env_t
{
    lls_init_t lls_init;         /**< Link Loss Service initialization variables. */
    uint16_t   start_hdl;        /**< Link Loss Service start handle. */
};

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
/** Pointer to Link Loss Service environment variable. */
static struct lls_env_t s_lls_env;            /**< Link Loss Service instance. */
static uint8_t          s_char_mask = 0x07;   /**< Features added into ATT database.
                                               * bit0 - Link Loss Service Declaration
                                               * bit1 - Alert Level Characteristic Declaration
                                               * bit2 - Alert Level Characteristic Value
                                               */

/**@brief Full LLS Database Description - Used to add attributes into the
 *        database.
 */
static const attm_desc_t lls_attr_tab[LLS_IDX_NB] =
{
    // Link Loss Service Declaration
    [LLS_IDX_SVC]            = {BLE_ATT_DECL_PRIMARY_SERVICE, READ_PERM_UNSEC, 0, 0},
    // Alert Level Characteristic Declaration
    [LLS_IDX_ALERT_LVL_CHAR] = {BLE_ATT_DECL_CHARACTERISTIC,  READ_PERM_UNSEC, 0, 0},
    // Alert Level Characteristic Value
    [LLS_IDX_ALERT_LVL_VAL]  = {BLE_ATT_CHAR_ALERT_LEVEL,     READ_PERM(AUTH) | WRITE_REQ_PERM(AUTH), 0, sizeof(uint8_t)},
};

/*
 * LOCAL FUNCTION DECLARATIONS
 *******************************************************************************
 */
static sdk_err_t   lls_init(void);
static void        lls_on_connect(uint8_t conn_idx);
static void        lls_on_disconnect(uint8_t conn_idx, uint8_t reason);
static void        lls_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param);

/**@brief LLS interface required by profile manager. */
static ble_prf_manager_cbs_t lls_mgr_cbs =
{
    (prf_init_func_t)lls_init,
    lls_on_connect,
    lls_on_disconnect
};

/**@brief LLS GATT server callbacks. */
static gatts_prf_cbs_t lls_gatts_cbs =
{
    NULL,
    lls_write_att_cb,
    NULL,
    NULL
};

/**@brief LLS Information. */
static const prf_server_info_t lls_prf_info =
{
    /* There shall be only one connection on a device. */
    .max_connection_nb = 1,
    .manager_cbs       = &lls_mgr_cbs,
    .gatts_prf_cbs     = &lls_gatts_cbs
};

/**
 *****************************************************************************************
 * @brief Initialize Link Loss service and create database in BLE Stack.
 *
 * @return BLE_ATT_ERR_NO_ERROR on success, otherwise error code.
 *****************************************************************************************
 */
static sdk_err_t lls_init(void)
{
    const uint8_t lls_svc_uuid[] = BLE_ATT_16_TO_16_ARRAY(BLE_ATT_SVC_LINK_LOSS);
    gatts_create_db_t gatts_db;
    uint16_t start_hdl = PRF_INVALID_HANDLE; /* The start hanlde is an in/out
                                              * parameter of ble_gatts_srvc_db_create().
                                              * It must be set with PRF_INVALID_HANDLE
                                              * to be allocated automatically by BLE Stack.*/

    memset(&gatts_db, 0, sizeof(gatts_db));

    gatts_db.shdl                 = &start_hdl;
    gatts_db.uuid                 = (uint8_t *)lls_svc_uuid;
    gatts_db.attr_tab_cfg         = &s_char_mask;
    gatts_db.max_nb_attr          = LLS_IDX_NB;
    gatts_db.srvc_perm            = 0;
    gatts_db.attr_tab_type        = SERVICE_TABLE_TYPE_16;
    gatts_db.attr_tab.attr_tab_16 = lls_attr_tab;

    sdk_err_t   error_code = ble_gatts_srvc_db_create(&gatts_db);
    if (SDK_SUCCESS == error_code)
    {
        s_lls_env.start_hdl = *gatts_db.shdl;

        uint16_t handle = prf_find_handle_by_idx(LLS_IDX_ALERT_LVL_VAL,  s_lls_env.start_hdl, &s_char_mask);
        ble_gatts_value_set(handle, sizeof(uint8_t), 0, (unsigned char *)(&s_lls_env.lls_init.initial_alert_level));
    }

    return error_code;
}

/**
 *****************************************************************************************
 * @brief Handle the connect event.
 *
 * @param[in] conn_idx: Connection index.
 *****************************************************************************************
 */
static void lls_on_connect(uint8_t conn_idx)
{
    lls_evt_t   evt;
    sdk_err_t   ret;

    if (s_lls_env.lls_init.evt_handler)
    {
        evt.evt_type = LLS_EVT_LINK_LOSS_ALERT;

        ret = lls_alert_level_get((unsigned char *)(&evt.alert_level));

        if (SDK_SUCCESS == ret && s_lls_env.lls_init.evt_handler)
        {
            /* Inform Application the link is (re)connected */
            s_lls_env.lls_init.evt_handler(&evt);
        }
    }
}

/**
 *****************************************************************************************
 * @brief Handle the disconnect event.
 *
 * @param[in] conn_idx:Connect index.
 * @param[in] reason:  The reason of disconnect.
 *****************************************************************************************
 */
static void lls_on_disconnect(uint8_t conn_idx, uint8_t reason)
{
    /* The reason is HCI Connection Timeout */
    if (BLE_LL_ERR_CON_TIMEOUT == reason || BLE_LL_ERR_INSTANT_PASSED == reason)
    {
        /* Link loss detected, inform application */
        lls_evt_t evt;
        sdk_err_t ret;

        evt.evt_type = LLS_EVT_LINK_LOSS_ALERT;

        ret = lls_alert_level_get((unsigned char *)(&evt.alert_level));

        if (SDK_SUCCESS == ret)
        {
            if (s_lls_env.lls_init.evt_handler)
            {
                s_lls_env.lls_init.evt_handler(&evt);
            }
        }
    }

    return;
}

/**
 *****************************************************************************************
 * @brief Handles reception of the write request.
 *
 * @param[in] conn_idx: Connection index
 * @param[in] p_param:  Pointer to the parameters of the write request.
 *****************************************************************************************
 */
static void   lls_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param)
{
    uint16_t handle = prf_find_handle_by_idx(LLS_IDX_ALERT_LVL_VAL, s_lls_env.start_hdl, &s_char_mask);
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
        }
    }

    ble_gatts_write_cfm(conn_idx, &cfm);
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
sdk_err_t lls_service_init(lls_init_t *p_lls_init)
{
    if (NULL == p_lls_init)
    {
        return SDK_ERR_POINTER_NULL;
    }

    s_lls_env.lls_init.evt_handler = p_lls_init->evt_handler;

    return ble_server_prf_add(&lls_prf_info);
}

sdk_err_t   lls_alert_level_get(uint8_t *p_alert_level)
{
    uint16_t handle = prf_find_handle_by_idx(LLS_IDX_ALERT_LVL_VAL, s_lls_env.start_hdl, (uint8_t *)&s_char_mask);
    uint16_t length = sizeof(uint8_t);

    return ble_gatts_value_get(handle, &length, p_alert_level);
}

