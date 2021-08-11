/**
 ****************************************************************************************
 *
 * @file ble_prf.h
 *
 * @brief BLE PRF API
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
  @addtogroup BLE_PRF Profile
  @{
  @brief  Definitions and prototypes for the profile interface.
 */

#ifndef __BLE_PRF_H__
#define __BLE_PRF_H__

#include "ble_error.h"
#include "ble_att.h"
#include "ble_gatts.h"
#include "ble_gattc.h"
#include "ble_gatt.h"


/**
  @addtogroup BLE_PRF_COMMON Profile Common
  @{
  @brief  Definitions and prototypes for Profile Common interface.
 */

/** @addtogroup BLE_PRF_MANAGER_TYPEDEFS Typedefs
 * @{ */
/**
****************************************************************************************
* @brief  Initialization of the Profile module.
* @note   This function performs all the initializations of the Profile module, and it will be automatically called after the ble_server_prf_add() or ble_client_prf_add() function.
*         - Creation of database (if it's a service) and ble_gatts_srvc_db_create should be called.
*         - Allocation of profile-required memory.
*
* @retval status code to know if profile initialization succeeds or not.
 ****************************************************************************************
 */
typedef uint8_t (*prf_init_func_t)(void);

/**
****************************************************************************************
* @brief Handles Connection creation. There is no need to recovery CCCD because stack will do that.
*
* @param[in]  conn_idx:     Connection index.
 ****************************************************************************************
 */
typedef void (*prf_on_connect_func_t)(uint8_t conn_idx);

/**
****************************************************************************************
* @brief Handles Disconnection. There is no need to recovery CCCD because stack will do that.
*
* @param[in]  conn_idx:     Connection index.
* @param[in]  reason:       Disconnection reason.
 ****************************************************************************************
 */
typedef void (*prf_on_disconnect_func_t)(uint8_t conn_idx, uint8_t reason);
 
 /** @addtogroup BLE_PRF_MANAGER_STRUCTURES Structures
 * @{ */
 /**
 * @brief Profile manager callbacks.
 */
/** @} */ 
typedef struct
{
    prf_init_func_t          init;              /**< Initialization callback. See @ref prf_init_func_t. */
    prf_on_connect_func_t    on_connect;        /**< Connection callback. See @ref prf_on_connect_func_t. */
    prf_on_disconnect_func_t on_disconnect;     /**< Disconnection callback. See @ref prf_on_disconnect_func_t. */
} ble_prf_manager_cbs_t;

/** @} */

/** @} */


/**
  @addtogroup BLE_PRF_SERVER Profile Server
  @{
  @brief  Definitions and prototypes for Profile Server interface.
 */

/** @addtogroup BLE_PRF_SERVER_STRUCTURES Structures
 * @{ */
/**
 * @brief GATT read request struct.
 */
typedef struct
{
    uint16_t handle;                       /**< Handle of the attribute to be read. */
} gatts_read_req_cb_t;

/**
 * @brief GATT write request struct.
 */
typedef struct
{
    uint16_t handle;                       /**< Handle of the attribute to be written. */
    uint16_t offset;                       /**< Offset at which the data has to be written. */
    uint16_t length;                       /**< Data length to be written. */
    uint8_t  value[__ARRAY_EMPTY];         /**< Data to be written to characteristic value. */
} gatts_write_req_cb_t;

/**
 * @brief GATT prepare write request struct.
 */
typedef struct
{
    uint16_t handle;                       /**< Handle of the attribute for whose value is requested. */
} gatts_prep_write_req_cb_t;

/**
 * @brief GATTS Operation Complete event structure.
 */
typedef struct
{   
    gatt_evt_type_t type;               /**< Notification or indication event type. */
    uint16_t        handle;             /**< Handle of the write operation, or notification/indication operation. */
} ble_gatts_ntf_ind_t;

/**
 * @brief GATT server callback function in relation to a profile.
 */
typedef struct
{
    void (*app_gatts_read_cb)(uint8_t conidx, const gatts_read_req_cb_t *p_read_req);                   /**< Read attribute value callback which is used when value is present in user space. 
                                                                                                             Function @ref ble_gatts_read_cfm should be called to send attribute value to stack.*/
    void (*app_gatts_write_cb)(uint8_t conidx, const gatts_write_req_cb_t *p_write_req);                /**< Write attribute value callback. 
                                                                                                             Function @ref ble_gatts_write_cfm should be called to send write attribute value status to stack no matter the value is in user's zone or BLE stack.*/
    void (*app_gatts_prep_write_cb)(uint8_t conidx, const gatts_prep_write_req_cb_t *p_prep_req);       /**< Prepare write value callback function. 
                                                                                                             Function @ref ble_gatts_prepare_write_cfm should be called to send prepare write attribute value status to stack no matter the value is in user's zone or BLE stack.*/
    void (*app_gatts_ntf_ind_cb)(uint8_t conidx, uint8_t status, const ble_gatts_ntf_ind_t *p_ntf_ind); /**< Notification or indication callback function. */

    void (*app_gatts_cccd_set_cb)(uint8_t conidx, uint16_t handle, uint16_t cccd_val);                   /**< Set CCCD value callback is called when connected with peer device. If bonded, recovery CCCD; otherwise, set default value(0x0000) for CCCD. */ 
                                                                                                                                                                               
} gatts_prf_cbs_t;

/**
 * @brief Profile server register information structure.
 */
typedef struct
{
    uint16_t max_connection_nb;              /**< Maximum connections the profile supports. */
    ble_prf_manager_cbs_t* manager_cbs;      /**< Profile manager callbacks. */
    gatts_prf_cbs_t *gatts_prf_cbs;          /**< GATT server callback function in relation to the specific profile. */
} prf_server_info_t;

/** @} */

/** @addtogroup BLE_PRF_FUNCTIONS Functions
* @{ */

/**
 ****************************************************************************************
 * @brief Add a server profile by providing its detailed information, including manager callback functions and GATT server callback functions.
 *        This API should be called in application initialization function.
 *
 * @param[in] p_server_prf_info: Pointer to the prf_info. See @ref prf_server_info_t.
 *
 * @note If there are several profiles which need to be added, this function should be called corresponding times. 
 *
 * @retval ::SDK_SUCCESS: The profile info is recorded successfully, and the database will be created in profile initialization callback function.
 * @retval ::SDK_ERR_POINTER_NULL: The parameter prf_info is NULL, or input parameters that prf_info points to are invalid.
 * @retval ::SDK_ERR_NO_RESOURCES: The profile number is up to the maximum number the system can support.
 ****************************************************************************************
 */
uint16_t ble_server_prf_add(const prf_server_info_t *p_server_prf_info);
/** @} */

/** @} */



/**
  @addtogroup BLE_PRF_CLIENT Profile Client
  @{
  @brief  Definitions and prototypes for Profile Client interface.
 */

/** @addtogroup BLE_PRF_CLIENT_STRUCTURES Structures
 * @{ */

/**
 * @brief GATTC profile register to peer event info structure.
 */
typedef struct
{
    uint16_t start_hdl;     /**< Attribute start handle. */
    uint16_t end_hdl;       /**< Attribute end handle. */
}gattc_prf_reg_peer_evt_t;

/**
 * @brief GATTC profile register enumeration.
 */
typedef enum
{
    GATTC_EVT_REGISTER,   /**< GATT client event register. */ 
    GATTC_EVT_UNREGISTER, /**< GATT client event unregister. */
}gattc_prf_reg_evt_t;


/**@brief GATTC Profile callback Structures. */
typedef struct
{
    void (*app_gattc_srvc_disc_cb)(uint8_t conn_idx, uint8_t status, const ble_gattc_srvc_disc_t * p_prim_srvc_disc);                /**< Primary Service Discovery Response callback. */
    void (*app_gattc_inc_srvc_disc_cb)(uint8_t conn_idx, uint8_t status, const ble_gattc_incl_disc_t * p_inc_srvc_disc);             /**< Relationship Discovery Response callback. */
    void (*app_gattc_char_disc_cb)(uint8_t conn_idx, uint8_t status, const ble_gattc_char_disc_t * p_char_disc);                     /**< Characteristic Discovery Response callback. */
    void (*app_gattc_char_desc_disc_cb)(uint8_t conn_idx, uint8_t status, const ble_gattc_char_desc_disc_t *p_char_desc_disc);       /**< Descriptor Discovery Response callback. */
    void (*app_gattc_read_cb)(uint8_t conn_idx, uint8_t status, const ble_gattc_read_rsp_t *p_read_rsp);                             /**< Read Response callback. */
    void (*app_gattc_write_cb)(uint8_t conn_idx, uint8_t status, uint16_t handle);                                                   /**< Write complete callback. */
    void (*app_gattc_ntf_ind_cb)(uint8_t conn_idx, const ble_gattc_ntf_ind_t *p_ntf_ind);                                            /**< Handle Value Notification/Indication Event callback. */
    void (*app_gattc_srvc_browse_cb)(uint8_t conn_idx, uint8_t status, const ble_gattc_browse_srvc_t *p_browse_srvc);                /**< Service found callback during browsing procedure. */
    void (*app_gattc_prf_reg_cb)(uint8_t conn_idx, uint8_t status, gattc_prf_reg_evt_t reg_evt);                                     /**< GATT client event register complete callback. */
} gattc_prf_cbs_t;

/**
 * @brief Profile client register information structure.
 */
typedef struct
{
    uint16_t              max_connection_nb;    /**< Maximum connections the profile supports. */
    ble_prf_manager_cbs_t *manager_cbs;         /**< Profile manager callbacks. */
    gattc_prf_cbs_t       *gattc_prf_cbs;       /**< GATT client callback function in relation to the specific profile. */
} prf_client_info_t;

/** @} */


/**
  @addtogroup BLE_PRF_CLIENT_FUNCTIONS Functions 
  @{
  @brief  Definitions and prototypes for Profile Client interface.
 */
/**
 ****************************************************************************************
 * @brief Add a client profile by providing its detail information, including manager callback functions and GATT client callback functions.
 *        This API should be called in application initialization function.
 *
 * @param[in]  p_client_prf_info: Pointer to the p_client_prf_info. See @ref prf_client_info_t.
 * @param[out] p_client_prf_id:  Pointer to the client profile id.
 *
 * @note If there are several profiles which need to be added, this function should be called corresponding times. 
 *
 * @retval ::SDK_SUCCESS: The profile info is recorded successfully, and the profile ENV will be initialized in profile initialization callback function.
 * @retval ::SDK_ERR_POINTER_NULL: The parameter p_client_prf_info or p_client_prf_id is NULL, or input parameters that prf_info points to are invalid.
 * @retval ::SDK_ERR_NO_RESOURCES: The profile number is up to the maximum number the system can support.
 ****************************************************************************************
 */
uint16_t ble_client_prf_add(const prf_client_info_t *p_client_prf_info, uint8_t *p_client_prf_id);


/**
 ****************************************************************************************
 * @brief Profile client Browse Specific Primary Service information on remote GATT server.
 *
 * @note This discovery automatically searches for Primary Services, Included Services, Characteristics and Descriptors of each service.
 *       To discover one or more services only, use ble_gattc_primary_services_discover() instead.
 *       This discovery is able to search a specific Primary Service.
 *       If p_srvc_uuid is NULL, the invalid pointer error code will be returned immediately.
 *
 * @note Function callback @ref gattc_prf_cbs_t::app_gattc_srvc_browse_cb will be called for all attributes of each service found.
 *       After completed service handle range registeration for receiving peer device indication/notification will be executed internally.
 *       Because secondary service can't be browsed, so handle range registeration for receiving peer device indication/notification to this client
 *       profile may be necessary. App can call function ble_gattc_prf_evt_handle_register for registeration, it depends on user app.
 *       If user don't call this function, user shall call ble_gattc_prf_evt_handle_register to register handle range for receiving 
 *       peer device indication/notification in specific client profile callback.
 *       
 *
 * @param[in] prf_id:          Profile id.
 * @param[in] conn_idx:        Current connection index.
 * @param[in] p_srvc_uuid:     Pointer to Service UUID.
 *
 * @retval ::SDK_SUCCESS: Successfully start the Browse Service(s) procedure.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_POINTER_NULL:     Invalid pointer supplied.
 * @retval ::SDK_ERR_INVALID_PARAM:    Invalid parameter(s) supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_prf_services_browse(uint8_t prf_id, uint8_t conn_idx, const ble_uuid_t *p_srvc_uuid);

/**
 ****************************************************************************************
 * @brief Profile client Discover Primary Services on remote GATT server.
 *
 * @note Function callback @ref gattc_prf_cbs_t::app_gattc_srvc_disc_cb will be called for service(s) found. 
 *       If p_srvc_uuid is NULL, the invalid pointer error code will be returned immediately.
 *
 * @param[in] prf_id:          Profile id.
 * @param[in] conn_idx:        Current connection index.
 * @param[in] p_srvc_uuid:     Pointer to Service UUID.
 *
 * @retval ::SDK_SUCCESS:              Successfully start the Primary Service Discovery procedure.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_POINTER_NULL:     Invalid pointer supplied.
 * @retval ::SDK_ERR_INVALID_PARAM:    Invalid parameter(s) supplied.
 * @retval ::SDK_ERR_NO_RESOURCES:     Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_prf_primary_services_discover(uint8_t prf_id, uint8_t conn_idx, const ble_uuid_t *p_srvc_uuid);

/**
 ****************************************************************************************
 * @brief Profile client Discover Included Services on remote GATT server.
 *
 * @note Function callback @ref gattc_prf_cbs_t::app_gattc_inc_srvc_disc_cb will be called for Included Service(s) found.
 *
 * @param[in] prf_id:       Profile id.
 * @param[in] conn_idx:     Current connection index.
 * @param[in] start_hdl:    Start handle.
 * @param[in] end_hdl:      End handle.
 *
 * @retval ::SDK_SUCCESS: Successfully start the Relationship Discovery procedure.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_INVALID_PARAM:    Invalid parameter(s) supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_prf_included_services_discover(uint8_t prf_id, uint8_t conn_idx, uint16_t start_hdl, uint16_t end_hdl);

/**
 ****************************************************************************************
 * @brief Profile client Discover Characteristics on remote GATT server.
 * @note Function callback @ref gattc_prf_cbs_t::app_gattc_char_disc_cb will be called for Characteristic Declaration(s) found.
 *       If p_disc_char is NULL, the invalid pointer error code will be returned immediately.
 *
 * @param[in] prf_id:         Profile id.
 * @param[in] conn_idx:       Current connection index.
 * @param[in] p_disc_char:    Pointer to discover by characteristic UUID info.
 *
 * @retval ::SDK_SUCCESS: Successfully start the Characteristic Discovery procedure.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_POINTER_NULL:     Invalid pointer supplied.
 * @retval ::SDK_ERR_INVALID_PARAM:    Invalid parameter(s) supplied.
 * @retval ::SDK_ERR_NO_RESOURCES:     Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_prf_char_discover(uint8_t prf_id, uint8_t conn_idx, gattc_disc_char_t *p_disc_char);

/**
 ****************************************************************************************
 * @brief Profile client Discover Characteristics Descriptors on remote GATT server.
 *
 * @note Function callback @ref gattc_prf_cbs_t::app_gattc_char_desc_disc_cb will be called for Characteristic Descriptor(s) found.      
 * If the last Descriptor has not been reached, this function must be called again with an updated handle range to continue the discovery.
 *
 * @param[in] prf_id:       Profile id.
 * @param[in] conn_idx:     Current connection index.
 * @param[in] start_hdl:    Start handle.
 * @param[in] end_hdl:      End handle.
 *
 * @retval ::SDK_SUCCESS: Successfully start the Descriptor Discovery procedure.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_INVALID_PARAM: Invalid parameter(s) supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_prf_char_desc_discover(uint8_t prf_id, uint8_t conn_idx, uint16_t start_hdl, uint16_t end_hdl);

/**
 ****************************************************************************************
 * @brief Profile client Read Attribute from remote GATT server.
 *
 * @note This uses either the "Read Characteristic Value" procedure or the "Read Characteristic Descriptor"
 *       procedure, depending on the attribute pointed by handle. If offset is non-zero or the
 *       attribute length is larger than the MTU, the "Read Long Characteristic Value" procedure or the
 *       "Read Long Characteristic Descriptor" procedure will be used respectively.
 *
 * @note Function callback @ref gattc_prf_cbs_t::app_gattc_read_cb will be called when Read is finished.
 *
 * @param[in] prf_id:     Profile id.
 * @param[in] conn_idx:   Current connection index.
 * @param[in] handle:     Attribute handle.
 * @param[in] offset:     Value offset to start with.
 *
 * @retval ::SDK_SUCCESS: Successfully start the Read (Long) procedure.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_INVALID_PARAM: Invalid parameter(s) supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_prf_read(uint8_t prf_id, uint8_t conn_idx, uint16_t handle, uint16_t offset);

/**
 ****************************************************************************************
 * @brief Profile client Read Attribute by UUID.
 *
 * @note Function callback @ref gattc_prf_cbs_t::app_gattc_read_cb will be called when Read is finished.
 *
 * @param[in] prf_id:            Profile id.
 * @param[in] conn_idx:          Current connection index.
 * @param[in] p_read_by_uuid:    Pointer to Read by Characteristic UUID info.
 *
 * @retval ::SDK_SUCCESS: Successfully start the Read Using Characteristic UUID procedure.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_POINTER_NULL:     Invalid pointer supplied.
 * @retval ::SDK_ERR_INVALID_PARAM:    Invalid parameter(s) supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_prf_read_by_uuid(uint8_t prf_id, uint8_t conn_idx, gattc_read_by_uuid_t *p_read_by_uuid);

/**
 ****************************************************************************************
 * @brief Profile client Initiate a Read Multiple Characteristic Values procedure
 *
 * @note Function callback @ref gattc_prf_cbs_t::app_gattc_read_cb will be called for each handle value which is read.
 * @param[in] prf_id:      Profile id.
 * @param[in] conn_idx:    Current connection index.
 * @param[in] p_param:     Pointer to the parameters of the value.
 *
 * @retval ::SDK_SUCCESS: Successfully start the Read Multiple Characteristic Values procedure.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_POINTER_NULL:     Invalid pointer supplied.
 * @retval ::SDK_ERR_INVALID_PARAM:    Invalid parameter(s) supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_prf_read_multiple(uint8_t prf_id, uint8_t conn_idx, const gattc_read_multiple_t *p_param);

/**
 ****************************************************************************************
 * @brief Profile client Write (Long) Characteristic (Descriptor) Value.
 *
 * @note This uses either the "Write Characteristic Value" procedure or the "Write Characteristic
 *       Descriptor" procedure, depending on the attribute pointed by handle. If offset is non-zero
 *       or the attribute length is larger than the MTU, the "Write Long Characteristic Value" procedure
 *       or the "Write Long Characteristic Descriptor" procedure will be used respectively.
 *
 * @note Once completed @ref gattc_prf_cbs_t::app_gattc_write_cb will be called.
 *
 * @param[in] prf_id:               Profile id.
 * @param[in] conn_idx:             Current connection index.
 * @param[in] p_write_attr_value:   Pointer to the write attribue value info.
 *
 * @retval ::SDK_SUCCESS: Successfully start the Write procedure.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_POINTER_NULL:     Invalid pointer supplied.
 * @retval ::SDK_ERR_INVALID_PARAM:    Invalid parameter(s) supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_prf_write(uint8_t prf_id, uint8_t conn_idx, gattc_write_attr_value_t *p_write_attr_value);

/**
 ****************************************************************************************
 * @brief Profile client Prepare Long/Reliable Write to remote GATT server.
 * @note Once completed @ref gattc_prf_cbs_t::app_gattc_write_cb will be called.
 *
 * @param[in] prf_id:               Profile id.
 * @param[in] conn_idx:             Current connection index.
 * @param[in] p_write_attr_value:   Pointer to the write attribue value info.
 *
 * @retval ::SDK_SUCCESS: Successfully send prepare write request.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_POINTER_NULL:     Invalid pointer supplied.
 * @retval ::SDK_ERR_INVALID_PARAM:    Invalid parameter(s) supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
 uint16_t ble_gattc_prf_write_prepare(uint8_t prf_id, uint8_t conn_idx, gattc_write_attr_value_t *p_write_attr_value);

/**
 ****************************************************************************************
 * @brief Profile client Execute Reliable/Long Write to remote GATT server.
 *
 * @note Once completed @ref gattc_prf_cbs_t::app_gattc_write_cb will be called.
 *
 * @param[in] prf_id:       Profile id.
 * @param[in] conn_idx:     Current connection index.
 * @param[in] execute:     True if data shall be written; False if cancel all prepared writes.
 *
 * @retval ::SDK_SUCCESS: Successfully send an Execute Write request.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_INVALID_PARAM:    Invalid parameter(s) supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_prf_write_execute(uint8_t prf_id, uint8_t conn_idx, bool execute);

/**
 ****************************************************************************************
 * @brief Profile client Write Attribute to remote GATT server (without response).
 *
 * @note If signed_write is set to false, the "Write Without Response" procedure will be used.
 *       If signed_write is set to true, the "Signed Write Without Response" procedure will be used on
 *       a link which is not encrypted.
 *       If a link is already encrypted, "Write Without Response" procedure shall be used instead of "Signed Write Without Response".
 * @note Once completed @ref gattc_prf_cbs_t::app_gattc_write_cb will be called.
 *
 * @param[in] prf_id:            Profile id.
 * @param[in] conn_idx:          Current connection index.
 * @param[in] p_write_no_resp:   Pointer to the write without response info.
 *
 * @retval ::SDK_SUCCESS Successfully: start the (Signed) Write Without Response procedure.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_POINTER_NULL:     Invalid pointer supplied.
 * @retval ::SDK_ERR_INVALID_PARAM:    Invalid parameter(s) supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_prf_write_no_resp(uint8_t prf_id, uint8_t conn_idx, gattc_write_no_resp_t *p_write_no_resp);

/**
 ****************************************************************************************
 * @brief Profile client Confirm Reception of Indication.
 *
 * @note Confirm indication which has been correctly received from the peer.
 *
 * @param[in] prf_id       Profile id.
 * @param[in] conn_idx:     Current connection index.
 * @param[in] handle:       Value handle.
 *
 * @retval ::SDK_SUCCESS: Successfully send a Confirm Reception of Indication request.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_INVALID_PARAM:    Invalid parameter(s) supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_prf_indicate_cfm(uint8_t prf_id, uint8_t conn_idx, uint16_t handle);


/**
 ****************************************************************************************
 * @brief Profile client Register Indication/Notification event.
 *
 * @note Registration to peer device events (Indication/Notification) for specific client profile.
 *       Once completed @ref gattc_prf_cbs_t::app_gattc_prf_reg_cb with type: @ref GATTC_EVT_UNREGISTER will be called.
 *       If user don't call ble_gattc_prf_services_browse, user shall call this function to register handle range for receiving 
 *       peer device indication/notification in specific client profile callback.
 *
 * @param[in] prf_id:       Profile id.
 * @param[in] conn_idx:     Current connection index.
 * @param[in] env:          Pointer to the profile registeration event info.
 *
 * @retval ::SDK_SUCCESS: Successfully register Indication/Notification event.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_POINTER_NULL:     Invalid pointer supplied.
 * @retval ::SDK_ERR_INVALID_PARAM:    Invalid parameter(s) supplied.
 * @retval ::SDK_ERR_NO_RESOURCES:     Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_prf_evt_handle_register(uint8_t prf_id, uint8_t conn_idx, gattc_prf_reg_peer_evt_t *env);

/**
 ****************************************************************************************
 * @brief Profile client Unregister Indication/Notification event.
 *
 * @note Unregistration to peer device events (Indication/Notification) for specific client profile.
 *       Once completed @ref gattc_prf_cbs_t::app_gattc_prf_reg_cb with type: @ref GATTC_EVT_UNREGISTER will be called.
 *
 * @param[in] prf_id:       Profile id.
 * @param[in] conn_idx:     Current connection index.
 * @param[in] env:          Pointer to the profile registeration event info.
 *
 * @retval ::SDK_SUCCESS: Successfully unregister Indication/Notification event.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_POINTER_NULL:     Invalid pointer supplied.
 * @retval ::SDK_ERR_INVALID_PARAM:    Invalid parameter(s) supplied.
 * @retval ::SDK_ERR_NO_RESOURCES:     Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gattc_prf_evt_handle_unregister(uint8_t prf_id, uint8_t conn_idx, gattc_prf_reg_peer_evt_t *env);

/** @} */
/** @} */

#endif

/** @} */
/** @} */
