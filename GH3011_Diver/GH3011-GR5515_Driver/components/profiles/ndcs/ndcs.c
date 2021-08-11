/**
 ****************************************************************************************
 *
 * @file ndcs.c
 *
 * @brief Next DST Change Service implementation.
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
#include "ndcs.h"
#include "ble_prf_types.h"
#include "ble_prf_utils.h"
#include "utility.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
/**@brief Next DST Change Service Attributes Indexes. */
enum
{
    // Next DST Change Service
    NDCS_IDX_SVC,

    // Time with DST
    NDCS_IDX_TIME_DST_CHAR,
    NDCS_IDX_TIME_DST_VAL,

    NDCS_IDX_NB
};

/*
 * STRUCTURES
 *****************************************************************************************
 */
/**@brief Next DST Change Service environment variable. */
struct ndcs_env_t
{
    uint16_t        char_mask;      /**< Mask of supported characteristics. */
    uint16_t        start_hdl;      /**< Next DST Change Service start handle. */
    ndcs_time_dst_t time_with_dst;  /**< Time with DST value. */
};
/*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static sdk_err_t   ndcs_init(void);
static void        ndcs_read_att_cb(uint8_t conidx, const gatts_read_req_cb_t *p_param);
static void        ndcs_time_with_dst_read_handler(gatts_read_cfm_t *p_cfm, uint8_t *p_encode_buffer);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static struct ndcs_env_t s_ndcs_env;

/**@brief Full NDCS Database Description - Used to add attributes into the database. */
static const attm_desc_t ndcs_attr_tab[NDCS_IDX_NB] =
{
    // NDCS Service Declaration
    [NDCS_IDX_SVC] = {BLE_ATT_DECL_PRIMARY_SERVICE, READ_PERM_UNSEC, 0, 0},

    // Time with DST Characteristic Declaration
    [NDCS_IDX_TIME_DST_CHAR]    = {BLE_ATT_DECL_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // Time with DST Characteristic Declaration value
    [NDCS_IDX_TIME_DST_VAL]     = {BLE_ATT_CHAR_TIME_WITH_DST,
                                   READ_PERM(AUTH),
                                   ATT_VAL_LOC_USER,
                                   NDCS_TIME_WITH_DST_VAL_LEN},
};

/**@brief NDCS Task interface required by profile manager. */
static ble_prf_manager_cbs_t ndcs_tack_cbs =
{
    (prf_init_func_t) ndcs_init,
    NULL,
    NULL
};

/**@brief NDCS Task Callbacks. */
static gatts_prf_cbs_t ndcs_cb_func =
{
    ndcs_read_att_cb,
    NULL,
    NULL,
    NULL
};

/**@brief NDCS Information. */
static const prf_server_info_t ndcs_prf_info =
{
    .max_connection_nb = NDCS_CONNECTION_MAX,
    .manager_cbs       = &ndcs_tack_cbs,
    .gatts_prf_cbs     = &ndcs_cb_func
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 * @brief Initialize Next DST Change service and create db in att
 *
 * @return Error code to know if profile initialization succeed or not.
 *****************************************************************************************
 */
static sdk_err_t ndcs_init(void)
{
    // The start hanlde must be set with PRF_INVALID_HANDLE to be allocated automatically by BLE Stack.
    uint16_t          start_hdl      = PRF_INVALID_HANDLE;
    const uint8_t     ndcs_svc_uuid[] = BLE_ATT_16_TO_16_ARRAY(BLE_ATT_SVC_NEXT_DST_CHANGE);
    sdk_err_t         error_code;
    gatts_create_db_t gatts_db;

    memset(&gatts_db, 0, sizeof(gatts_db));

    gatts_db.shdl                 = &start_hdl;
    gatts_db.uuid                 = ndcs_svc_uuid;
    gatts_db.attr_tab_cfg         = (uint8_t *)&(s_ndcs_env.char_mask);
    gatts_db.max_nb_attr          = NDCS_IDX_NB;
    gatts_db.srvc_perm            = 0;
    gatts_db.attr_tab_type        = SERVICE_TABLE_TYPE_16;
    gatts_db.attr_tab.attr_tab_16 = ndcs_attr_tab;

    error_code = ble_gatts_srvc_db_create(&gatts_db);

    if (SDK_SUCCESS == error_code)
    {
        s_ndcs_env.start_hdl = *gatts_db.shdl;
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
static void ndcs_read_att_cb(uint8_t conn_idx, const gatts_read_req_cb_t *p_param)
{
    gatts_read_cfm_t  cfm;
    uint8_t           handle    = p_param->handle;
    uint8_t           tab_index = prf_find_idx_by_handle(handle,
                                  s_ndcs_env.start_hdl,
                                  NDCS_IDX_NB,
                                  (uint8_t *)&s_ndcs_env.char_mask);
    cfm.handle = handle;
    cfm.status = BLE_SUCCESS;

    switch (tab_index)
    {
        case NDCS_IDX_TIME_DST_VAL:
        {
            uint8_t encoded_buffer[NDCS_TIME_WITH_DST_VAL_LEN];
            ndcs_time_with_dst_read_handler(&cfm, encoded_buffer);
            break;
        }

        default:
            cfm.length = 0;
            cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
            break;
    }

    ble_gatts_read_cfm(conn_idx, &cfm);
}

/**
 *****************************************************************************************
 * @brief Handle Time with DSP Time read event.
 *
 * @param[out] p_cfm:           Pointer to GATT read attribute result description.
 * @param[out] p_encode_buffer: Pointer to encoded data will be written.
 *****************************************************************************************
 */
static void ndcs_time_with_dst_read_handler(gatts_read_cfm_t *p_cfm, uint8_t *p_encode_buffer)
{
    prf_pack_date_time(p_encode_buffer, &s_ndcs_env.time_with_dst.date_time);

    p_encode_buffer[7] = s_ndcs_env.time_with_dst.dst_offset;

    p_cfm->length = NDCS_TIME_WITH_DST_VAL_LEN;
    p_cfm->value  = p_encode_buffer;
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
void ndcs_day_time_update(prf_date_time_t *p_day_time)
{
    memcpy(&s_ndcs_env.time_with_dst.date_time, p_day_time, sizeof(prf_date_time_t));
}

void ndcs_dst_offset_update(ndcs_dst_offset_t dst_offset)
{
    s_ndcs_env.time_with_dst.dst_offset = dst_offset;
}

sdk_err_t ndcs_service_init(uint8_t char_mask)
{
    s_ndcs_env.char_mask = char_mask;

    return ble_server_prf_add(&ndcs_prf_info);
}
