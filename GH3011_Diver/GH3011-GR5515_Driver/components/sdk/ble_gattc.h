/**
 ****************************************************************************************
 *
 * @file ble_gattc.h
 *
 * @brief BLE GATTC API
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

 /**
 * @addtogroup BLE
 * @{
 */
 
  /**
 * @addtogroup BLE_GATT Generic Attribute Profile (GATT)
 * @{
 * @brief Definitions and prototypes for the GATT interface.
 */

/**
  @addtogroup BLE_GATTC Generic Attribute Profile (GATT) Client
  @{
  @brief Definitions and prototypes for the GATT client interfaces.
 */

#ifndef __BLE_GATTC_H__
#define __BLE_GATTC_H__

#include "ble_error.h"
#include "ble_gatt.h"
#include "ble_att.h"
#include "gr55xx_sys_cfg.h"

#include <stdint.h>
#include <stdbool.h>

/** @addtogroup BLE_GATTC_ENUMERATIONS Enumerations
 * @{ */

/**
 * @brief GATT Client Service Discover Attribute type IDs.
 */
typedef enum
{
    BLE_GATTC_BROWSE_NONE,          /**< No Attribute Information. */
    BLE_GATTC_BROWSE_INC_SRVC,      /**< Included Service information. */
    BLE_GATTC_BROWSE_ATTR_CHAR,     /**< Characteristic Declaration. */
    BLE_GATTC_BROWSE_ATTR_VAL,      /**< Attribute Value definition. */
    BLE_GATTC_BROWSE_ATTR_DESC,     /**< Attribute Descriptor. */
} gatt_attr_t;

/** @} */

/** @addtogroup BLE_GATTC_STRUCTURES Structures
 * @{ */

/**
 * @brief GATTC discovery characteristic structure.
 */
typedef struct
{
    uint16_t        start_hdl;              /**< Start handle. */
    uint16_t        end_hdl;                /**< End handle. */
    ble_uuid_t     *p_uuid;                 /**< Characteristic UUID. */
} gattc_disc_char_t;

/**
 * @brief GATTC read by characteristic UUID structure.
 */
typedef struct
{
    uint16_t        start_hdl;      /**< Start handle. */
    uint16_t        end_hdl;        /**< End handle. */
    ble_uuid_t     *p_uuid;         /**< Characteristic UUID. */
} gattc_read_by_uuid_t;

/**
 * @brief GATTC write attribute value structure.
 */
typedef struct
{
    uint16_t        handle;         /**< Attribute handle. */
    uint16_t        offset;         /**< value offset to start with. */
    uint16_t        length;         /**< Write length. */
    uint8_t        *p_value;        /**< Value to write. */
} gattc_write_attr_value_t;

/**
 * @brief GATTC write without response structure.
 */
typedef struct
{
    bool            signed_write;       /**< True if signed write should be used when possible/applicable. */
    uint16_t        handle;             /**< Attribute handle. */
    uint16_t        length;             /**< Write length. */
    uint8_t        *p_value;            /**< Value to write. */
} gattc_write_no_resp_t;

/**@brief Read Multiple Handles. */
typedef struct
{
    uint16_t    handle;    /**< Attribute handle. */
    uint16_t    len;       /**< Known value: length of the handle (len shall not be set to 0). */
} read_multiple_t;

/**@brief GATTC Read Multiple. */
typedef struct
{
    uint16_t            handle_count;      /**< Handle count of the multiple attributes to be read. */
    read_multiple_t    *p_read_multiple;   /**< Pointer to the multiple attributes to be read. */
} gattc_read_multiple_t;


/**@brief GATTC Browse information about Characteristic. */
typedef struct
{
    uint8_t     attr_type;                      /**< Attribute type. See @ref BLE_GATTC_BROWSE_ATTR_CHAR for Characteristic Declaration. */
    uint8_t     prop;                           /**< Value property. */
    uint16_t    handle;                         /**< Value handle. */
    uint8_t     uuid_len;                       /**< Characteristic UUID length. */
    uint8_t     uuid[BLE_ATT_UUID_128_LEN];     /**< Characteristic UUID. */
} gattc_browse_attr_char_t;

/**@brief GATTC Browse information about Included Service. */
typedef struct
{
    uint8_t  attr_type;                      /**< Attribute type. See @ref BLE_GATTC_BROWSE_INC_SRVC for Included Service Information. */
    uint8_t  uuid_len;                       /**< Included Service UUID length. */
    uint8_t  uuid[BLE_ATT_UUID_128_LEN];     /**< Included Service UUID. */
    uint16_t start_hdl;                      /**< Included Service start handle. */
    uint16_t end_hdl;                        /**< Included Service end handle. */
} gattc_browse_inc_srvc_t;

/**@brief GATTC Browse information about Attribute. */
typedef struct
{
    uint8_t  attr_type;                     /**< Attribute type. See @ref BLE_GATTC_BROWSE_ATTR_VAL for Attribute Value. See @ref BLE_GATTC_BROWSE_ATTR_DESC for Attribute Descriptor. */
    uint8_t  uuid_len;                      /**< Attribute UUID length. */
    uint8_t  uuid[BLE_ATT_UUID_128_LEN];    /**< Characteristic UUID or Characteristic Descriptor UUID. */
} gattc_browse_attr_t;

/**@brief GATTC Browse attribute information. */
union gattc_browse_attr_info
{
    uint8_t                  attr_type;        /**< Attribute type. See @ref gatt_attr_t. */
    gattc_browse_attr_char_t attr_char;        /**< Information about Characteristic. When union attr_type is @ref BLE_GATTC_BROWSE_ATTR_CHAR */
    gattc_browse_inc_srvc_t  inc_srvc;         /**< Information about Included Service. When union attr_type is @ref BLE_GATTC_BROWSE_INC_SRVC */
    gattc_browse_attr_t      attr;             /**< Information about Attribute. When union attr_type is @ref BLE_GATTC_BROWSE_ATTR_VAL or @ref BLE_GATTC_BROWSE_ATTR_DESC. */
};

/**@brief GATTC Browse service(s) indication. */
typedef struct
{
    uint8_t  uuid_len;                                  /**< Service UUID length. */
    uint8_t  uuid[BLE_ATT_UUID_128_LEN];                /**< Service UUID. */
    uint16_t start_hdl;                                 /**< Service start handle. */
    uint16_t end_hdl;                                   /**< Service end handle. */
    union gattc_browse_attr_info info[__ARRAY_EMPTY];   /**< Attribute information presented in the service(array length = end_hdl - start_hdl);
                                                             If attr_type is equal to BLE_GATTC_BROWSE_NONE, the last attribute information has been found in previous one, although not reach the service end handle. */
} ble_gattc_browse_srvc_t;



/**@brief GATT service. */
typedef struct
{
    uint16_t  start_hdl;            /**< Start handle. */
    uint16_t  end_hdl;              /**< End handle. */
    uint8_t   uuid_len;             /**< Service UUID length. */
    uint8_t  *p_uuid;               /**< Service UUID. */
} ble_gattc_service_t;

/**@brief GATT include. */
typedef struct
{
    uint16_t  attr_hdl;               /**< Attribute handle. */
    uint16_t  start_hdl;              /**< Start handle. */
    uint16_t  end_hdl;                /**< End handle. */
    uint8_t   uuid_len;               /**< Service UUID length. */
    uint8_t  *p_uuid;                 /**< Service UUID. */
} ble_gattc_include_t;

/**@brief GATT characteristic. */
typedef struct
{
    uint16_t  handle_decl;            /**< Handle of the Characteristic Declaration. */
    uint16_t  handle_value;           /**< Handle of the Characteristic Value. */
    uint8_t   prop;                   /**< Properties. */
    uint8_t   uuid_len;               /**< Characteristic UUID length. */    
    uint8_t  *p_uuid;                 /**< Characteristic UUID. */
} ble_gattc_char_t;

/**@brief GATT descriptor. */
typedef struct
{
    uint16_t  attr_hdl;               /**< Attribute handle. */
    uint8_t   uuid_len;               /**< Descriptor UUID length. */
    uint8_t  *p_uuid;                 /**< Descriptor UUID. */
} ble_gattc_desc_t;

/**@brief GATT service discovery. */
typedef struct
{
    uint16_t            count;                        /**< Service count. */
    ble_gattc_service_t services[__ARRAY_EMPTY];      /**< Service data. */
}ble_gattc_srvc_disc_t;

/**@brief GATT include discovery. */
typedef struct
{
    uint16_t            count;                       /**< Include count. */
    ble_gattc_include_t includes[__ARRAY_EMPTY];     /**< Include data. */
}ble_gattc_incl_disc_t;

/**@brief GATT characteristic discovery. */
typedef struct
{
    uint16_t            count;                  /**< Characteristic count. */
    ble_gattc_char_t    chars[__ARRAY_EMPTY];   /**< Characteristic data. */
}ble_gattc_char_disc_t;

/**@brief GATT characteristic descriptor discovery. */
typedef struct
{
    uint16_t            count;                      /**< Descriptor count. */
    ble_gattc_desc_t    char_descs[__ARRAY_EMPTY];  /**< Descriptor data. */
}ble_gattc_char_desc_disc_t;

/**@brief GATT Client Read value. */
typedef struct
{
    uint16_t  handle;                 /**< Attribute handle. */
    uint16_t  offset;                 /**< Offset of the attribute value. */
    uint16_t  length;                 /**< Attribute value length. */
    uint8_t  *p_value;                /**< Pointer to the attribute value data. */
} ble_gattc_read_value_t;

/**@brief GATT value Read response. */
typedef struct
{
    uint16_t                    count;                 /**< Value Count. */
    ble_gattc_read_value_t      vals[__ARRAY_EMPTY];   /**< Value(s) list. */
}ble_gattc_read_rsp_t;


/**@brief GATT Client Write. */
typedef struct
{
    uint16_t handle;            /**< Attribute handle. */
} ble_gattc_write_t;


/**@brief GATTC Notification and Indication value indication. */
typedef struct
{
    gatt_evt_type_t  type;               /**< Event type. */
    uint16_t         length;             /**< Attribute value length. */
    uint16_t         handle;             /**< Attribute handle. */
    uint8_t         *p_value;            /**< Pointer to the attribute value data. */
} ble_gattc_ntf_ind_t;

/**@brief GATTC Event callback Structures. */
typedef struct
{
    void (*app_gattc_srvc_disc_cb)(uint8_t conn_idx, uint8_t status, const ble_gattc_srvc_disc_t * p_prim_srvc_disc);                   /**< Primary Service Discovery Response callback. */
    void (*app_gattc_inc_srvc_disc_cb)(uint8_t conn_idx, uint8_t status, const ble_gattc_incl_disc_t * p_inc_srvc_disc);                /**< Relationship Discovery Response callback. */
    void (*app_gattc_char_disc_cb)(uint8_t conn_idx, uint8_t status, const ble_gattc_char_disc_t * p_char_disc);                        /**< Characteristic Discovery Response callback. */
    void (*app_gattc_char_desc_disc_cb)(uint8_t conn_idx, uint8_t status, const ble_gattc_char_desc_disc_t *p_char_desc_disc);          /**< Descriptor Discovery Response callback. */
    void (*app_gattc_read_cb)(uint8_t conn_idx, uint8_t status, const ble_gattc_read_rsp_t *p_read_rsp);                                /**< Read Response callback. */
    void (*app_gattc_write_cb)(uint8_t conn_idx, uint8_t status, uint16_t handle);                                                      /**< Write complete callback. */
    void (*app_gattc_ntf_ind_cb)(uint8_t conn_idx, const ble_gattc_ntf_ind_t *p_ntf_ind);                                               /**< Handle Value Notification/Indication Event callback. */
    void (*app_gattc_srvc_browse_cb)(uint8_t conn_idx, uint8_t status, const ble_gattc_browse_srvc_t *p_browse_srvc);                   /**< Service found callback during browsing procedure. */
} gattc_cb_fun_t;
/** @} */

/** @addtogroup BLE_GATTC_FUNCTIONS Functions
 * @{ */

/**
 ****************************************************************************************
 * @brief Perform MTU Exchange.
 *
 * @note MTU Exchange Callback @ref gatt_common_cb_fun_t::app_gatt_mtu_exchange_cb will be called to indicate to APP once receiving the peer response.
 *
 * @param[in] conn_idx:     Current connection index.
 *
 * @retval ::SDK_SUCCESS: Successfully send an MTU Exchange request.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_mtu_exchange(uint8_t conn_idx);

/**
 ****************************************************************************************
 * @brief Browse all Primary Services or specific Primary Service information on remote GATT server.
 *
 * @note This discovery automatically searches for Primary Services, Included Services, Characteristics and Descriptors of each service.
 *       To discover one or more services only, use ble_gattc_primary_services_discover() instead.
 *       This discovery is able to search all Primary Services or a specific one.
 *       If srvc_uuid is NULL, all services are returned.
 *
 * @note Function callback @ref gattc_cb_fun_t::app_gattc_srvc_browse_cb will be called for all attributes of each service found.
 *
 * @param[in] conn_idx:        Current connection index.
 * @param[in] p_srvc_uuid:     Pointer to Service UUID. If it is NULL, all services will be returned.
 *
 * @retval ::SDK_SUCCESS: Successfully start the Browse Service(s) procedure.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_services_browse(uint8_t conn_idx, const ble_uuid_t *p_srvc_uuid);

/**
 ****************************************************************************************
 * @brief Discover Primary Services on remote GATT server.
 *
 * @note Function callback @ref gattc_cb_fun_t::app_gattc_srvc_disc_cb will be called for service(s) found. 
 *
 * @param[in] conn_idx:       Current connection index.
 * @param[in] p_srvc_uuid:    Pointer to Service UUID. If it is NULL, all Primary Services will be returned.
 *
 * @retval ::SDK_SUCCESS: Successfully start the Primary Service Discovery procedure.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_primary_services_discover(uint8_t conn_idx, const ble_uuid_t *p_srvc_uuid);

/**
 ****************************************************************************************
 * @brief Discover Included Services on remote GATT server.
 *
 * @note Function callback @ref gattc_cb_fun_t::app_gattc_inc_srvc_disc_cb will be called for Included Service(s) found.
 * @param[in] conn_idx:     Current connection index.
 * @param[in] start_hdl:    Start handle.
 * @param[in] end_hdl:      End handle.
 *
 * @retval ::SDK_SUCCESS: Successfully start the Relationship Discovery procedure.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_included_services_discover(uint8_t conn_idx, uint16_t start_hdl, uint16_t end_hdl);

/**
 ****************************************************************************************
 * @brief Discover Characteristics on remote GATT server.
 * @note Function callback @ref gattc_cb_fun_t::app_gattc_char_disc_cb will be called for Characteristic Declaration(s) found.
 * @param[in] conn_idx:       Current connection index.
 * @param[in] start_hdl:      Start handle.
 * @param[in] end_hdl:        End handle.
 * @param[in] p_char_uuid:    Pointer to Characteristic UUID.If it is NULL, all characteristics are returned.
 *
 * @retval ::SDK_SUCCESS: Successfully start the Characteristic Discovery procedure.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_char_discover(uint8_t conn_idx, uint16_t start_hdl, uint16_t end_hdl, const ble_uuid_t *p_char_uuid);

/**
 ****************************************************************************************
 * @brief Discover Characteristics Descriptors on remote GATT server.
 *
 * @note Function callback @ref gattc_cb_fun_t::app_gattc_char_desc_disc_cb will be called for Characteristic Descriptor(s) found.      
 * If the last Descriptor has not been reached, this function must be called again with an updated handle range to continue the discovery.
 *
 * @param[in] conn_idx:     Current connection index.
 * @param[in] start_hdl:    Start handle.
 * @param[in] end_hdl:      End handle.
 *
 * @retval ::SDK_SUCCESS: Successfully start the Descriptor Discovery procedure.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_char_desc_discover(uint8_t conn_idx, uint16_t start_hdl, uint16_t end_hdl);

/**
 ****************************************************************************************
 * @brief Read Attribute from remote GATT server.
 *
 * @note This uses either the "Read Characteristic Value" procedure or the "Read Characteristic Descriptor"
 *       procedure, depending on the attribute pointed by handle. If offset is non-zero or the
 *       attribute length is larger than the MTU, the "Read Long Characteristic Value" procedure or the
 *       "Read Long Characteristic Descriptor" procedure will be used respectively.
 *
 * @note Function callback @ref gattc_cb_fun_t::app_gattc_read_cb will be called when Read is finished.
 * @param[in] conn_idx:   Current connection index.
 * @param[in] handle:     Attribute handle.
 * @param[in] offset:     Value offset to start with.
 *
 * @retval ::SDK_SUCCESS: Successfully start the Read (Long) procedure.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_read(uint8_t conn_idx, uint16_t handle, uint16_t offset);

/**
 ****************************************************************************************
 * @brief Read Attribute by UUID.
 *
 * @note Function callback @ref gattc_cb_fun_t::app_gattc_read_cb will be called when Read is finished.
 * @param[in] conn_idx:         Current connection index.
 * @param[in] start_hdl:        Start handle.
 * @param[in] end_hdl:          End handle.
 * @param[in] p_char_uuid:      Pointer to Characteristic UUID.
 *
 *
 * @retval ::SDK_SUCCESS: Successfully start the Read using Characteristic UUID procedure.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_POINTER_NULL:     Invalid pointer supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_read_by_uuid(uint8_t conn_idx, uint16_t start_hdl, uint16_t end_hdl, const ble_uuid_t *p_char_uuid);

/**
 ****************************************************************************************
 * @brief Initiate a Read Multiple Characteristic Values procedure
 *
 * @note Function callback @ref gattc_cb_fun_t::app_gattc_read_cb will be called for each handle value which is read.
 * @param[in] conn_idx:    Current connection index.
 * @param[in] p_param:     Pointer to the parameters of the value.
 *
 * @retval ::SDK_SUCCESS: Successfully start the Read Multiple Characteristic Values procedure.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_POINTER_NULL:     Invalid pointer supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_read_multiple(uint8_t conn_idx, const gattc_read_multiple_t *p_param);

/**
 ****************************************************************************************
 * @brief Write (long) Characteristic (Descriptor) Value.
 *
 * @note This uses either the "Write Characteristic Value" procedure or the "Write Characteristic
 *       Descriptor" procedure, depending on the attribute pointed by handle. If offset is non-zero
 *       or the attribute length is larger than the MTU, the "Write Long Characteristic Value" procedure
 *       or the "Write Long Characteristic Descriptor" procedure will be used respectively.
 *
 * @note Once completed @ref gattc_cb_fun_t::app_gattc_write_cb will be called.
 *
 * @param[in] conn_idx:     Current connection index.
 * @param[in] handle:       The handle of the attribute to be written.
 * @param[in] offset:       Offset into the attribute value to be written.
 * @param[in] length:       Length of the value data in bytes.
 * @param[in] p_value:      Pointer to the value data.
 *
 * @retval ::SDK_SUCCESS: Successfully start the Write procedure.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_POINTER_NULL:     Invalid pointer supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_write(uint8_t conn_idx, uint16_t handle, uint16_t offset, uint16_t length, const uint8_t *p_value);

/**
 ****************************************************************************************
 * @brief Prepare Long/Reliable Write to remote GATT server.
 *
 * @note When this function completes, @ref gattc_cb_fun_t::app_gattc_write_cb will be called.
 *
 * @param[in] conn_idx:     Current connection index.
 * @param[in] handle:       Attribute handle.
 * @param[in] offset:       Value offset to start with.
 * @param[in] length:       Value length.
 * @param[in] p_value:      Value data.
 *
 * @retval ::SDK_SUCCESS: Successfully send a Prepare Write request.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_POINTER_NULL:     Invalid pointer supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_write_prepare(uint8_t conn_idx, uint16_t handle, uint16_t offset, uint16_t length, const uint8_t *p_value);

/**
 ****************************************************************************************
 * @brief Execute Reliable/Long Write to remote GATT server.
 *
 * @note When this function completes, @ref gattc_cb_fun_t::app_gattc_write_cb will be called.
 *
 * @param[in] conn_idx:     Current connection index.
 * @param[in] execute:      True if data shall be written; false if cancel all prepared writes.
 *
 * @retval ::SDK_SUCCESS: Successfully send an Execute Write request.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_write_execute(uint8_t conn_idx, bool execute);

/**
 ****************************************************************************************
 * @brief Write Attribute to remote GATT server (without response).
 *
 * @note If signed_write is set to false, the "Write Without Response" procedure will be used.
 *       If signed_write is set to true, the "Signed Write Without Response" procedure will be used on
 *       a link which is not encrypted.
 *       If a link is already encrypted, "Write Without Response" procedure shall be used instead of "Signed Write Without Response".
 * @note Once completed @ref gattc_cb_fun_t::app_gattc_write_cb will be called.
 *
 * @param[in] conn_idx:         Current connection index.
 * @param[in] handle:           Attribute handle.
 * @param[in] signed_write:     True if signed write should be used, false write without response.
 * @param[in] length:           Value length.
 * @param[in] p_value:          Value data.
 *
 * @retval ::SDK_SUCCESS: Successfully start the (Signed) Write Without Response procedure.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_POINTER_NULL: Invalid pointer supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_write_no_resp(uint8_t conn_idx, bool signed_write, uint16_t handle, uint16_t length, const uint8_t *p_value);

/**
 ****************************************************************************************
 * @brief Confirm Reception of Indication.
 *
 * @note Confirm indication which has been correctly received from the peer.
 *
 * @param[in] conn_idx:     Current connection index.
 * @param[in] handle:       Value handle.
 *
 * @retval ::SDK_SUCCESS: Successfully send indication confirm.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_indicate_cfm(uint8_t conn_idx, uint16_t handle);

/** @} */

#endif

/** @} */

/** @} */
/** @} */

