/**
 *****************************************************************************************
 *
 * @file bcs.c
 *
 * @brief The implementation of Body Composition Service.
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
#include "bcs.h"
#include "ble_prf_types.h"
#include "ble_prf_utils.h"
#include "utility.h"

/*
 * DEFINES
 *****************************************************************************************
 */
/** Reference Value Fields of Body Composition Measurement in SIG Assigned
 *  Numbers */
#define BCS_MEAS_VAL_MAX_LEN        30

/* Reference section 3.4.7.2 Handle Value Indication, ATT, Core Spec. */
#define INDI_PAYLOAD_HEADER_LEN     3

/* The least value of MTU is 23 octets. The size of Attribute Value is (23-3)
 * octets which could be less than the size (30 octets) of all fields of Body
 * Composition Measurement. If the required data exceeds the current size
 * (MTU - 3) octets, the remaining optional fields shall be sent in the
 * subsequent indication. So we need no more than 2 packets. */
#define NUM_PACKETS                 2
#define MEAS_PACKET_FIRST           0
#define MEAS_PACKET_SUB             1

/*
 * TYPE DEFINITIONS
 *****************************************************************************************
 */
typedef struct
{
    uint16_t size;
    uint8_t  value[BCS_MEAS_VAL_MAX_LEN];
} meas_packet_t;

/*
 * ENUMERATIONS
 *****************************************************************************************
 */
/** Attributes index in ATT DB */
enum bcs_attr_idx_tag
{
    BCS_IDX_SVC,

    BCS_IDX_BODY_COMPS_FEAT_CHAR,
    BCS_IDX_BODY_COMPS_FEAT_VAL,

    BCS_IDX_BODY_COMPS_MEAS_CHAR,
    BCS_IDX_BODY_COMPS_MEAS_VAL,
    BCS_IDX_BODY_COMPS_MEAS_CCCD,

    BCS_IDX_NB,
};

enum bcs_body_comps_meas_flags
{
    BCM_FLAG_UNIT_SI            = 0x0000,
    BCM_FLAG_UNIT_IMPERIAL      = 0x0001,
    BCM_FLAG_DATE_TIME_PRESENT  = 0x0002,
    BCM_FLAG_USER_ID_PRESENT    = 0x0004,
    BCM_FLAG_BASAL_METABOLISM   = 0x0008,
    BCM_FLAG_MUSCLE_PERCENTAGE  = 0x0010,
    BCM_FLAG_MUSCLE_MASS        = 0x0020,
    BCM_FLAG_FAT_FREE_MASS      = 0x0040,
    BCM_FLAG_SOFT_LEAN_MASS     = 0x0080,
    BCM_FLAG_BODY_WATER_MASS    = 0x0100,
    BCM_FLAG_IMPEDANCE          = 0x0200,
    BCM_FLAG_WEIGHT             = 0x0400,
    BCM_FLAG_HEIGHT             = 0x0800,
    BCM_FLAG_MUTI_PACKET        = 0x1000,
};

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
/** Pointer to Body Composition Service environment variable. */
static bcs_env_t *s_p_env;

static uint8_t attrs_mask = 0x37;
                                /**< Attributes to be added into ATT database.
                                 * bit0 - BC Service declaration.
                                 * bit1 - BC Feature declaration.
                                 * bit2 - BC Feature value.
                                 * bit3 - BC measurement declaration.
                                 * bit4 - BC Measurement value.
                                 * bit5 - BC Measurement CCCD.
                                 */

/**@brief Full BCS attributes descriptor which is used to add attributes into
 *        the ATT database.
 */ 
static const attm_desc_t bcs_attr_tab[BCS_IDX_NB] =
{
    // BC Service Declaration
    [BCS_IDX_SVC] = {BLE_ATT_DECL_PRIMARY_SERVICE, READ_PERM_UNSEC, 0, 0},

    // BC Feature Characteristic Declaration
    [BCS_IDX_BODY_COMPS_FEAT_CHAR] = {BLE_ATT_DECL_CHARACTERISTIC,
                                      READ_PERM_UNSEC, 0, 0},
    // BC Feature Characteristic Value
    [BCS_IDX_BODY_COMPS_FEAT_VAL]  = {BLE_ATT_CHAR_BODY_COMPOSITION_FEATURE,
                                      READ_PERM(AUTH),
                                      0, sizeof(uint32_t)},

    [BCS_IDX_BODY_COMPS_MEAS_CHAR] = {BLE_ATT_DECL_CHARACTERISTIC,
                                      READ_PERM_UNSEC, 0, 0},
    [BCS_IDX_BODY_COMPS_MEAS_VAL]  = {BLE_ATT_CHAR_BODY_COMPOSITION_MEASUREMENT,
                                      INDICATE_PERM(AUTH),
                                      0, BCS_MEAS_VAL_MAX_LEN},
    [BCS_IDX_BODY_COMPS_MEAS_CCCD] = {BLE_ATT_DESC_CLIENT_CHAR_CFG,
                                      READ_PERM(AUTH) | WRITE_REQ_PERM(AUTH),
                                      0, 0},
};

/*
 * LOCAL FUNCTION DECLARATIONS
 *****************************************************************************************
 */
static sdk_err_t  init_bcs(void);
static void on_connect_bcs(uint8_t conidx);
static sdk_err_t  write_att_cb_bcs(uint8_t conidx,
                                    const gatts_write_req_cb_t *p_param);
static sdk_err_t  read_att_cb_bcs(uint8_t conidx,
                                   const gatts_read_req_cb_t *p_param);
static void measurement_encode_bcs(const bcs_meas_t *p_meas,
                                       uint16_t max_payload,
                                       meas_packet_t *p_packets);

/**@brief Callbacks required by profile manager. */
static ble_prf_manager_cbs_t bcs_mgr_cbs = {
    init_bcs,
    on_connect_bcs,
    NULL
};

/**@brief Callbacks for GATT server. */
static gatts_prf_cbs_t bcs_cb_func = {
    read_att_cb_bcs,
    write_att_cb_bcs,
    NULL,
    NULL
};

/**@brief Information for registering BC service. */
static const prf_server_info_t bcs_prf_info = {
    /* There shall be only one instance on a device */
    .max_connection_nb = 1,
    .manager_cbs       = &bcs_mgr_cbs,
    .gatts_prf_cbs          = &bcs_cb_func
};

/**@brief init BC service and create db in att
 *
 * @return BLE_ATT_ERR_NO_ERROR on success, otherwise error code
 */
static sdk_err_t init_bcs(void)
{
    sdk_err_t   err = SDK_SUCCESS;
    uint16_t start_hdl = PRF_INVALID_HANDLE;
    const uint8_t bcs_svc_uuid[] =
                           BLE_ATT_16_TO_16_ARRAY(BLE_ATT_SVC_BODY_COMPOSITION);
    gatts_create_db_t gatts_db;
    memset(&gatts_db, 0, sizeof(gatts_db));

    gatts_db.shdl                 = &start_hdl;
    gatts_db.uuid                 = (uint8_t*)bcs_svc_uuid;
    gatts_db.attr_tab_cfg         = &attrs_mask;
    gatts_db.max_nb_attr          = BCS_IDX_NB;
    gatts_db.srvc_perm            = 0;
    gatts_db.attr_tab_type        = SERVICE_TABLE_TYPE_16;
    gatts_db.attr_tab.attr_tab_16 = bcs_attr_tab;
    
    err = ble_gatts_srvc_db_create(&gatts_db);
    if (SDK_SUCCESS == err) {
        s_p_env->start_hdl = *gatts_db.shdl;

        uint16_t handle = prf_find_handle_by_idx(BCS_IDX_BODY_COMPS_FEAT_VAL,
                                                 s_p_env->start_hdl,
                                                 &attrs_mask);
        APP_PRINTF("Handle of BCS Feature Value: %d\n", handle);
        /* make sure the size of features is 32bits. */
        ble_gatts_value_set(handle, sizeof(uint32_t), 0,
                            (const uint8_t *)&s_p_env->bcs_features);
    }

    return err;
}

/**@brief Handle the connect event.
 *
 * @param[in] conidx Connection index
 */
static void on_connect_bcs(uint8_t conidx)
{
    if (s_p_env->evt_handler) {
        uint16_t cccd = s_p_env->meas_cccd[conidx];
        bcs_evt_t evt;

        evt.evt_type       = BCS_EVT_INDICATION_ENABLED;
        evt.params.enabled = prf_is_indication_enabled(cccd);
        /* Inform Application the status of indication once connected */
        s_p_env->evt_handler(&evt);
    }
}

/**@brief Handles reception of the write request.
 *
 * @param[in] conidx Connection index.
 * @param[in] param  Pointer to the parameters of the write request.
 *
 * @return If the request was consumed or not.
 */
static sdk_err_t   write_att_cb_bcs(uint8_t conidx,
                                    const gatts_write_req_cb_t *p_param)
{
    uint16_t handle = prf_find_handle_by_idx(BCS_IDX_BODY_COMPS_MEAS_CCCD,
                                             s_p_env->start_hdl, &attrs_mask);
    gatts_write_cfm_t cfm;

    cfm.handle = p_param->handle;

    if (handle != p_param->handle) {
        cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
    } else {
        if (p_param->length != sizeof(uint16_t)) {
            cfm.status = BLE_ATT_INVALID_ATTRIBUTE_VAL_LEN;
        } else {
            uint16_t cccd = le16toh(&p_param->value[0]);

            s_p_env->meas_cccd[conidx] = cccd;

            if (s_p_env->evt_handler != NULL) {
                bcs_evt_t evt;

                evt.evt_type       = BCS_EVT_INDICATION_ENABLED;
                evt.params.enabled = prf_is_indication_enabled(cccd);
                /* Inform Application the status of indication */
                s_p_env->evt_handler(&evt);
            }

            cfm.status = BLE_SUCCESS;
        }
    }

    return ble_gatts_write_cfm(conidx, &cfm);
}

/**@brief Handles reception of the read request.
 *
 * @param[in] conidx of connection index
 * @param[in] param Pointer to the parameters of the read request.
 *
 * @return If the request was consumed or not.
 */
static sdk_err_t   read_att_cb_bcs(uint8_t conidx,
                                   const gatts_read_req_cb_t *p_param)
{
    uint16_t cccd_hdl = prf_find_handle_by_idx(BCS_IDX_BODY_COMPS_MEAS_CCCD,
                                               s_p_env->start_hdl, &attrs_mask);
    gatts_read_cfm_t cfm;

    cfm.handle = p_param->handle;

    if (cccd_hdl != p_param->handle) {
        cfm.length = 0;
        cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
    } else {
        cfm.length = sizeof(uint16_t);
        cfm.value  = (uint8_t *)(&s_p_env->meas_cccd[conidx]);
        cfm.status = BLE_SUCCESS;
    }

    return ble_gatts_read_cfm(conidx, &cfm);
}

/**@brief Initialize the packet with mandatory data.
 *
 * @param[in]  p_meas   Pointer to the measurement data.
 * @param[in]  p_pkt    Pointer to the encoded packet.
 * @param[out] pp_flags Piont to pointer of flags in encoded packet.
 * @param[out] pp_field Point to pointer of field in encoded packet.
 *
 * @return The size of mandatory data in encoded packet.
 */
static uint16_t meas_packet_init(const bcs_meas_t *p_meas, meas_packet_t *p_pkt,
                                 uint16_t **pp_flags, uint8_t **pp_field)
{

    *pp_flags = (uint16_t *)p_pkt->value;
    /** the first 2 octets are reserved for flags. */
    *pp_field = &p_pkt->value[2];

    uint16_t *p_flags = *pp_flags;
    *p_flags |= (p_meas->unit == BCS_UNIT_SI) ?
                BCM_FLAG_UNIT_SI : BCM_FLAG_UNIT_IMPERIAL;

    uint16_t *p_field = (uint16_t *)*pp_field;
    *p_field = p_meas->body_fat_percentage;

    *pp_field += sizeof(p_meas->body_fat_percentage);

    return (sizeof(*p_flags) + sizeof(p_meas->body_fat_percentage));
}

/**@brief Add a field into a packet.
 *
 * @param[in]  flag     The flag of value to be added.
 * @param[in]  value    16bits value to be added.
 * @param[out] p_flags  Pointer to the flags in encoded packet.
 * @param[out] pp_field Point to pointer of the filed in encoded packet.
 *
 * @return The size of value being added.
 */
static uint16_t packet_field_add(uint16_t flag, uint16_t value,
                                 uint16_t *p_flags, uint8_t **pp_field)
{
    *p_flags |= flag;

    uint16_t *p_field = (uint16_t *)*pp_field;
    *p_field = value;

    *pp_field += sizeof(value);

    return sizeof(value);
}

/**@brief Check if switch to subsequent packet.
 *
 * @param[in] max_payload The max size of payload which is carried on ATT,
 *                        equals (MTU - MEASUREMENT_VAL_MAX_LEN).
 * @param[in] field_size  The size of field to be added.
 * @param[in] p_packets   Pointer to array of the packets where the encoded
 *                        data will be written.
 *
 * @return true for switching to subsequent packet.
 */
static bool subsequent_packet_switched(uint16_t max_payload, uint8_t field_size,
                                       meas_packet_t *p_packets)
{
    if (p_packets[MEAS_PACKET_SUB].size) {
        return false;
    }

    meas_packet_t *p_first_pkt = &p_packets[MEAS_PACKET_FIRST];

    if (p_first_pkt->size + field_size > max_payload) {
        /** The Multiple packet bit is setting for first packet. */
        uint16_t *p_flags = (uint16_t *)p_first_pkt->value;
        *p_flags |= BCM_FLAG_MUTI_PACKET;

        meas_packet_t *p_sub_pkt = &p_packets[MEAS_PACKET_SUB];

        p_flags = (uint16_t *)p_sub_pkt->value;
        *p_flags |= BCM_FLAG_MUTI_PACKET;

        return true;
    } else {
        return false;
    }
}

/**@brief Encode a Body Composition Measurement.
 *
 * @param[in]  p_meas      Pointer to the measurement to be encoded.
 * @param[in]  max_payload The max size of payload which is carried on ATT,
 *                         equals (MTU - MEASUREMENT_VAL_MAX_LEN).
 * @param[out] p_packets   Pointer to array of the packets where the encoded
 *                         data will be written.
 *
 */
static void measurement_encode_bcs(const bcs_meas_t *p_meas,
                                   uint16_t max_payload,
                                   meas_packet_t *p_packets)
{
    meas_packet_t *p_pkt = &p_packets[MEAS_PACKET_FIRST];
    uint16_t *p_flags;
    uint8_t *p_field;

    /** Put mandatory fields into packet. */
    meas_packet_init(p_meas, p_pkt, &p_flags, &p_field);

    /** The following are the optional fields. */
    if ((s_p_env->bcs_features & BCS_FEAT_TIME_STAMP)
        && p_meas->timestamp_present) {
        *p_flags |= BCM_FLAG_DATE_TIME_PRESENT;
        p_field += prf_pack_date_time(p_field, &p_meas->timestamp);
    }

    if (s_p_env->bcs_features & BCS_FEAT_MULTI_USER) {
        *p_flags |= BCM_FLAG_USER_ID_PRESENT;
        *p_field++ = p_meas->user_id;
    }

    if ((s_p_env->bcs_features & BCS_FEAT_BASAL_METABOLISM) &&
        p_meas->basal_metabolism > 0) {
        packet_field_add(BCM_FLAG_BASAL_METABOLISM, p_meas->basal_metabolism,
                         p_flags, &p_field);
    }

    if ((s_p_env->bcs_features & BCS_FEAT_MUSCLE_PERCENTAGE) &&
        p_meas->muscle_percentage > 0) {
        packet_field_add(BCM_FLAG_MUSCLE_PERCENTAGE, p_meas->muscle_percentage,
                         p_flags, &p_field);
    }

    if ((s_p_env->bcs_features & BCS_FEAT_MUSCLE_MASS) &&
        p_meas->muscle_mass > 0) {
        packet_field_add(BCM_FLAG_MUSCLE_MASS, p_meas->muscle_mass,
                         p_flags, &p_field);
    }


    if ((s_p_env->bcs_features & BCS_FEAT_FAT_FREE_MASS) &&
        p_meas->fat_free_mass > 0) {
        packet_field_add(BCM_FLAG_FAT_FREE_MASS, p_meas->fat_free_mass,
                         p_flags, &p_field);
    }

    p_pkt->size = p_field - p_pkt->value;

    /** If ATT_MTU=23 (max payload = 20), all above fields can be sent in one
     *  packet, but for the remaining optional fields below the current packet
     *  size shall be checked and the subsequent packet shall be used if needed.
     */
    if ((s_p_env->bcs_features & BCS_FEAT_SOFT_LEAN_MASS) &&
        p_meas->soft_lean_mass > 0) {
        if  (subsequent_packet_switched(max_payload, 2, p_packets) == true) {
            p_pkt = &p_packets[MEAS_PACKET_SUB];
            p_pkt->size = meas_packet_init(p_meas, p_pkt, &p_flags, &p_field);
        }

        p_pkt->size += packet_field_add(BCM_FLAG_SOFT_LEAN_MASS,
                                        p_meas->soft_lean_mass,
                                        p_flags, &p_field);
    }

    if ((s_p_env->bcs_features & BCS_FEAT_BODY_WATER_MASS) &&
        p_meas->body_water_mass > 0) {
        if  (subsequent_packet_switched(max_payload, 2, p_packets) == true) {
            p_pkt = &p_packets[MEAS_PACKET_SUB];
            p_pkt->size = meas_packet_init(p_meas, p_pkt, &p_flags, &p_field);
        }

        p_pkt->size += packet_field_add(BCM_FLAG_BODY_WATER_MASS,
                                        p_meas->body_water_mass,
                                        p_flags, &p_field);
    }

    if ((s_p_env->bcs_features & BCS_FEAT_IMPEDANCE) &&
        p_meas->impedance > 0) {
        if  (subsequent_packet_switched(max_payload, 2, p_packets) == true) {
            p_pkt = &p_packets[MEAS_PACKET_SUB];
            p_pkt->size = meas_packet_init(p_meas, p_pkt, &p_flags, &p_field);
        }

        p_pkt->size += packet_field_add(BCM_FLAG_IMPEDANCE,
                                        p_meas->impedance,
                                        p_flags, &p_field);
    }

    if ((s_p_env->bcs_features & BCS_FEAT_WEIGHT) &&
        p_meas->weight > 0) {
        if  (subsequent_packet_switched(max_payload, 2, p_packets) == true) {
            p_pkt = &p_packets[MEAS_PACKET_SUB];
            p_pkt->size = meas_packet_init(p_meas, p_pkt, &p_flags, &p_field);
        }

        p_pkt->size += packet_field_add(BCM_FLAG_WEIGHT,
                                        p_meas->weight,
                                        p_flags, &p_field);
    }

    if ((s_p_env->bcs_features & BCS_FEAT_HEIGHT) &&
        p_meas->weight > 0) {
        if  (subsequent_packet_switched(max_payload, 2, p_packets) == true) {
            p_pkt = &p_packets[MEAS_PACKET_SUB];
            p_pkt->size = meas_packet_init(p_meas, p_pkt, &p_flags, &p_field);
        }

        p_pkt->size += packet_field_add(BCM_FLAG_HEIGHT,
                                        p_meas->height,
                                        p_flags, &p_field);
    }
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
void bcs_add_service(bcs_env_t *p_bcs_env)
{
    s_p_env = p_bcs_env;
    ble_server_prf_add(&bcs_prf_info);
}

sdk_err_t   bcs_measurement_send(uint8_t conidx, const bcs_meas_t *p_meas)
{
    if (NULL == s_p_env) {
        return SDK_ERR_INVALID_PARAM;
    }

    if (!prf_is_indication_enabled(s_p_env->meas_cccd[conidx])) {
        return BLE_PRF_ERR_IND_DISABLED;
    }

    meas_packet_t packets[NUM_PACKETS];

    memset(packets, 0x00, sizeof(packets));

    uint16_t mtu = ble_gattc_mtu_get(conidx);
    measurement_encode_bcs(p_meas, mtu - INDI_PAYLOAD_HEADER_LEN, packets);

    gatts_noti_ind_t indi;

    indi.type   = BLE_GATT_INDICATION;
    indi.handle = prf_find_handle_by_idx(BCS_IDX_BODY_COMPS_MEAS_VAL,
                                         s_p_env->start_hdl,
                                         (uint8_t*)&attrs_mask);
    indi.length = packets[MEAS_PACKET_FIRST].size;
    indi.value  = packets[MEAS_PACKET_FIRST].value;

    sdk_err_t   err = ble_gatts_noti_ind(conidx, &indi);

    /* If subsequent packet is detected, remaining BC measurement data will be
     * send in the second packet transmission.*/
    if (BLE_SUCCESS == err && packets[MEAS_PACKET_SUB].size) {
        indi.length = packets[MEAS_PACKET_SUB].size;
        indi.value  = packets[MEAS_PACKET_SUB].value;

        err = ble_gatts_noti_ind(conidx, &indi);
    }

    return err;
}

bool bcs_is_indication_enabled(uint8_t conidx)
{
    if (NULL == s_p_env) {
        return false;
    }

    return prf_is_indication_enabled(s_p_env->meas_cccd[conidx]);
}

