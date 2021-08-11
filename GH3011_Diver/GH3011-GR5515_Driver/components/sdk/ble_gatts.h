/**
 ****************************************************************************************
 *
 * @file ble_gatts.h
 *
 * @brief BLE GATTS API
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
  @addtogroup BLE_SDK_GATTS Generic Attribute Profile (GATT) Server
  @{
  @brief  Definitions and prototypes for the GATT server interface.
 */

 
#ifndef __BLE_GATTS_H__
#define __BLE_GATTS_H__

#include "ble_error.h"
#include "ble_att.h"
#include "ble_gatt.h"

#include <stdint.h>

/** @addtogroup BLE_GATTS_DEFINES Defines
 * @{ */

/** @defgroup BLE_GATTS_MAX_INC_SRVC_NUM Max Number of Included Services
 * @{ */
#define MAX_INC_SRVC_NUM            (5)         /**< The max number of Included Services a Primary/Secondary service can have. Used by @ref gatts_create_db_t. */
/** @} */
 
/** @defgroup BLE_GATTS_ATTR_PERM_BIT Attribute Permission Bit
 * @{ */
#define BROADCAST                   (0x01)      /**< In one byte, bit0 means: Broadcast bit. Used by @ref BROADCAST_ENABLE. */
#define READ                        (0x02)      /**< In one byte, bit1 means: Read bit. Used by @ref READ_PERM_UNSEC, @ref READ_PERM */
#define WRITE_CMD                   (0x04)      /**< In one byte, bit2 means: Write_cmd bit. Used by @ref WRITE_CMD_PERM_UNSEC, @ref WRITE_CMD_PERM. */
#define WRITE_REQ                   (0x08)      /**< In one byte, bit3 means: Write_req bit. Used by @ref WRITE_REQ_PERM_UNSEC, @ref WRITE_REQ_PERM. */
#define NOTIFY                      (0x10)      /**< In one byte, bit4 means: Notify bit. Used by @ref NOTIFY_PERM_UNSEC, @ref NOTIFY_PERM. */
#define INDICATE                    (0x20)      /**< In one byte, bit5 means: Indicate bit. Used by @ref INDICATE_PERM_UNSEC, @ref INDICATE_PERM */
#define WRITE_SIGNED                (0x40)      /**< In one byte, bit6 means: Write_signed bit. Used by @ref WRITE_SIGNED_PERM_UNSEC, @ref WRITE_SIGNED_PERM. */
#define EXT_PROP                    (0x80)      /**< In one byte, bit7 means: Ext_property bit. Used by @ref EXT_PROP_ENABLE. */
/** @} */

/** @defgroup BLE_GATTS_ATTR_PERM_POS Attribute Permission Value Position
 * @{ */
#define READ_POS                    (0x00)      /**< Bit position of read permission. Used by @ref READ_PERM. */
#define WRITE_POS                   (0x02)      /**< Bit position of write permission. Used by @ref WRITE_CMD_PERM, @ref WRITE_REQ_PERM, @ref WRITE_SIGNED_PERM. */
#define INDICATE_POS                (0x04)      /**< Bit position of indicate bit. Used by @ref INDICATE_PERM. */
#define NOTIFY_POS                  (0x06)      /**< Bit position of notify bit. Used by @ref NOTIFY_PERM. */
/** @} */

/** @defgroup BLE_GATTS_SEC_LEVEL Attribute and Service Access Rights
 * @{ */
 
#define NOAUTH                      (0x00)      /**< LE security mode 1, level 1. Link does not need to be encrypted or authenticated.
                                                     Parameter of @ref SRVC_PERM, @ref READ_PERM, @ref WRITE_REQ_PERM, @ref WRITE_CMD_PERM, @ref WRITE_SIGNED_PERM, @ref INDICATE_PERM, @ref NOTIFY_PERM. */
#define UNAUTH                      (0x01)      /**< LE security mode 1, level 2. Link needs to be encrypted, but not to be authenticated.
                                                     Parameter of @ref SRVC_PERM, @ref READ_PERM, @ref WRITE_REQ_PERM, @ref WRITE_CMD_PERM, @ref WRITE_SIGNED_PERM, @ref INDICATE_PERM, @ref NOTIFY_PERM. */
#define AUTH                        (0x02)      /**< LE security mode 1, level 3. Link needs to be encrypted and authenticated (MITM).
                                                     Parameter of @ref SRVC_PERM, @ref READ_PERM, @ref WRITE_REQ_PERM, @ref WRITE_CMD_PERM, @ref WRITE_SIGNED_PERM, @ref INDICATE_PERM, @ref NOTIFY_PERM. */
#define SEC_CON                     (0x03)      /**< LE security mode 1, level 4. Link needs to be encrypted and authenticated (secure connections).
                                                     Parameter of @ref SRVC_PERM, @ref READ_PERM, @ref WRITE_REQ_PERM, @ref WRITE_CMD_PERM, @ref WRITE_SIGNED_PERM, @ref INDICATE_PERM, @ref NOTIFY_PERM. */
/** @} */

/** @defgroup BLE_GATTS_SEC_LEVEL_MASK Attribute and Service Security Level Mask
 * @{ */
#define SEC_LEVEL_MASK              (0x03)      /**< Security level mask.
                                                     Used by @ref SRVC_PERM, @ref READ_PERM, @ref WRITE_REQ_PERM, @ref WRITE_CMD_PERM, @ref WRITE_SIGNED_PERM, @ref INDICATE_PERM, @ref NOTIFY_PERM. */
/** @} */

/** @defgroup BLE_GATTS_UUID_TYPE Attribute and Service UUID Type
 * @{ */
#define UUID_TYPE_16                (0x00)      /**< 16-bit UUID length. Parameter of @ref SRVC_UUID_TYPE_SET, @ref ATT_UUID_TYPE_SET. */
#define UUID_TYPE_128               (0x02)      /**< 128-bit UUID length. Parameter of @ref SRVC_UUID_TYPE_SET, @ref ATT_UUID_TYPE_SET. */
/** @} */

/**
 * Service permissions
 *
 * |  7 |  6--5  |  4 |  3--2  |  1 |  0 |
 * |----|--------|----|--------|----|----|
 * |SEC |UUID_LEN|DIS |  AUTH  |EKS | MI |
 *
 * Bit [0]  : Service is multi-instantiated (0 = not support; 1 = support) \n
 * Bit [1]  : Encryption key size must be 16 bytes (0 = not need; 1 = need)  \n
 * Bit [2-3]: Service permission      (0 = NOAUTH; 1 = UNAUTH; 2 = AUTH; 3 = SEC_CON)  \n
 * Bit [4]  : Disable the service     (0 = no; 1 = yes) \n
 * Bit [5-6]: UUID Length Type        (0 = 16 bits; 2 = 128 bits)  \n
 * Bit [7]  : Secondary Service       (0 = Primary Service; 1 = Secondary Service)  \n
 */

/** @defgroup BLE_GATTS_SRVC_PERM Service Permission
 * @{ */
#define SRVC_SECONDARY_SET                       (0x80)             /**< Secondary service set. */
#define SRVC_UUID_TYPE_SET(uuid_len)             ((uuid_len) << 5)  /**< Service UUID length set. See @ref BLE_GATTS_UUID_TYPE. */
#define SRVC_DISABLE                             (0x10)             /**< Service disable. */
#define SRVC_PERM(sec_level)                     (((sec_level) & SEC_LEVEL_MASK) << 2)    /**< Service permission authentication. See @ref BLE_GATTS_SEC_LEVEL. */
#define SRVC_ENCRP_KEY_SIZE_16                   (0x02)             /**< 16 bytes service encryption key size . */
#define SRVC_MULTI_ENABLE                        (0x01)             /**< Service is multi-instantiated. */
/** @} */

/**
 * Attribute permission
 *
 * | 15 | 14 | 13 | 12 | 11 | 10 |  9 |  8 |  7--6  |  5--4  |  3--2  |  1--0  |
 * |----|----|----|----|----|----|----|----|--------|--------|--------|--------|
 * |EXT | WS | I  | N  | WR | WC | RD | B  |   NP   |   IP   |   WP   |   RP   |
 *
 * Bit [0-1]: Read permission         (0 = NOAUTH; 1 = UNAUTH; 2 = AUTH; 3 = SEC_CON)  \n
 * Bit [2-3]: Write permission        (0 = NOAUTH; 1 = UNAUTH; 2 = AUTH; 3 = SEC_CON)  \n
 * Bit [4-5]: Indicate permission     (0 = NOAUTH; 1 = UNAUTH; 2 = AUTH; 3 = SEC_CON)  \n
 * Bit [6-7]: Notify permission       (0 = NOAUTH; 1 = UNAUTH; 2 = AUTH; 3 = SEC_CON)  \n
 *
 * Bit [8]  : Broadcast permission  \n
 * Bit [9]  : Read accepted  \n
 * Bit [10] : Write Command accepted  \n
 * Bit [11] : Write Request accepted  \n
 * Bit [12] : Notify accepted  \n
 * Bit [13] : Indicate accepted  \n
 * Bit [14] : Write Signed accepted  \n
 * Bit [15] : Extended Properties present  \n
 */

/** @defgroup BLE_GATTS_ATTR_PERM Attribute Permission
 * @{ */
#define READ_PERM_UNSEC                 (READ << 8)                                                           /**< Default Read permission. */
#define READ_PERM(sec_level)            (READ << 8 | (((sec_level) & SEC_LEVEL_MASK) << READ_POS))            /**< Read permission set. 
                                                                                                                                                                                               See @ref BLE_GATTS_SEC_LEVEL. */
#define WRITE_REQ_PERM_UNSEC            (WRITE_REQ << 8)                                                      /**< Default Write Permission. */
#define WRITE_REQ_PERM(sec_level)       (WRITE_REQ << 8 | (((sec_level) & SEC_LEVEL_MASK) << WRITE_POS))      /**<  Write permission set. 
                                                                                                                                                                                                See @ref BLE_GATTS_SEC_LEVEL. */
#define WRITE_CMD_PERM_UNSEC            (WRITE_CMD << 8)                                                      /**< Default Write without Response Permission. */
#define WRITE_CMD_PERM(sec_level)       (WRITE_CMD << 8 | (((sec_level) & SEC_LEVEL_MASK) << WRITE_POS))      /**< Write without Response permission set. 
                                                                                                                                                                                                See @ref BLE_GATTS_SEC_LEVEL. */
#define WRITE_SIGNED_PERM_UNSEC         (WRITE_SIGNED << 8)                                                   /**< Default Authenticated Signed Write Permission. */
#define WRITE_SIGNED_PERM(sec_level)    (WRITE_SIGNED << 8 | (((sec_level) & SEC_LEVEL_MASK) << WRITE_POS))   /**< Authenticated Signed Write permission set. 
                                                                                                                                                                                               See @ref BLE_GATTS_SEC_LEVEL. */
#define INDICATE_PERM_UNSEC             (INDICATE << 8)                                                       /**< Default Indicate Permission. */
#define INDICATE_PERM(sec_level)        (INDICATE << 8 | (((sec_level) & SEC_LEVEL_MASK) << INDICATE_POS))    /**< Indicate permission set. 
                                                                                                                                                                                               See @ref BLE_GATTS_SEC_LEVEL. */
#define NOTIFY_PERM_UNSEC               (NOTIFY << 8)                                                         /**< Default Notify Permission. */
#define NOTIFY_PERM(sec_level)          (NOTIFY << 8 | (((sec_level) & SEC_LEVEL_MASK) << NOTIFY_POS))        /**< Notify permission set. 
                                                                                                                                                                                               See @ref BLE_GATTS_SEC_LEVEL. */
#define BROADCAST_ENABLE                (BROADCAST << 8)                                                      /**< Broadcast enable. */
#define EXT_PROP_ENABLE                 (EXT_PROP << 8)                                                       /**< Extended Properties enable. */
/** @} */

/**
 * Attribute extend permission
 *
 * | 15 |  14--13 | 12 |                       11--0                               |
 * |----|---------|----|-----------------------------------------------------------|
 * | VL |UUID_LEN |EKS |                      RESERVED                             |
 *
 * Bit [0-11] : Reserved \n
 * Bit [12]   : Encryption key size must be 16 bytes (0 = not need; 1 = need) \n
 * Bit [14-13]: UUID length type        (0 = 16 bits; 2 = 128 bits)  \n
 * Bit [15]   : Value location (0 = value saved in BLE Stack; 1 = value saved in user space)  \n
 */

/** @defgroup BLE_GATTS_ATTR_EXT_PERM Attribute Extend Permission
 * @{ */
#define ATT_VAL_LOC_USER                   (1 << 15)            /**< Value location which means value saved in user space, the profile's read/write callback will be called. */
#define ATT_VAL_LOC_STACK                  (0 << 15)            /**< Value location which means value saved in BLE Stack. */
#define ATT_UUID_TYPE_SET(uuid_len)        ((uuid_len) << 13)   /**< Attribute UUID length set. See @ref BLE_GATTS_UUID_TYPE */
#define ATT_ENC_KEY_SIZE_16                (0x1000)             /**< 16 bytes attribute encryption key size . */
/** @} */


/** @} */

/** @addtogroup BLE_GATTS_ENUMERATIONS Enumerations
 * @{ */

/**
 * @brief Service table type.
 */
typedef enum
{
    SERVICE_TABLE_TYPE_16 = 0x00,      /**< 16-bit service table type. */
    SERVICE_TABLE_TYPE_128,            /**< 128-bit service table type. */
} gatts_service_type_t;


/** @} */


/** @addtogroup BLE_GATTS_STRUCTURES Structures
 * @{ */

/**
 * @brief Service(16-bit UUID) description.
 */
typedef struct
{
    uint16_t uuid;           /**< 16-bit LSB-first UUID */
    uint16_t perm;           /**< Attribute permissions, see @ref BLE_GATTS_ATTR_PERM. \n
                                  - For Primary/Secondary/Included Services, must be @ref READ_PERM_UNSEC. \n
                                  - For Characteristic Declaration, must be @ref READ_PERM_UNSEC. \n
                                  - For Characteristic Extended Properties, must be @ref READ_PERM_UNSEC. \n
                                  - For Characteristic Presentation Format, must be @ref READ_PERM_UNSEC. \n
                                  - For Characteristic Aggregate Format, must be @ref READ_PERM_UNSEC. */   
                                  
    uint16_t ext_perm;       /**< Attribute extended permissions, see @ref BLE_GATTS_ATTR_EXT_PERM. \n
                                  - For Primary/Secondary/Included Services, this field is not used and should be set to 0. \n
                                  - For Characteristic Declaration, this field is not used and should be set to 0. \n
                                  - For Characteristic Extended Properties, this field is not used and should be set to 0. \n
                                  - For Client Characteristic Configuration and Server Characteristic Configuration, value must be saved in user space,
                                    user needn't to set this value location bit. The UUID length type must be set to 0.*/
                                  
    uint16_t max_size;       /**< Attribute max size. \n
                                  - For Primary/Secondary/Included Services, this field is not used, set to 0. \n
                                  - For Characteristic Declaration, this field is not used, set to 0. \n
                                  - For Characteristic Extended Properties, this field contains 2-byte value. \n
                                  - For Client Characteristic Configuration and Server Characteristic Configuration, this field is not used, set to 0. \n
                                  - For others, this field is attribute max size. */
} attm_desc_t;

/**
 * @brief Service(128 bits UUID) description.
 */
typedef struct
{
    uint8_t uuid[16];        /**< 128 bits UUID LSB First. */
    uint16_t perm;           /**< Attribute permissions, see @ref BLE_GATTS_ATTR_PERM. \n
                                  - For Primary/Secondary/Included Services, must be @ref READ_PERM_UNSEC. \n
                                  - For Characteristic Declaration, must be @ref READ_PERM_UNSEC. \n
                                  - For Characteristic Extended Properties, must be @ref READ_PERM_UNSEC. \n
                                  - For Characteristic Presentation Format, must be @ref READ_PERM_UNSEC. \n
                                  - For Characteristic Aggregate Format, must be @ref READ_PERM_UNSEC. */   
                                  
    uint16_t ext_perm;       /**< Attribute extended permissions, see @ref BLE_GATTS_ATTR_EXT_PERM. \n
                                  - For Primary/Secondary/Included Services, this field is not used, set to 0. \n
                                  - For Characteristic Declaration, this field is not used, set to 0. \n
                                  - For Characteristic Extended Properties, this field is not used, set to 0. \n
                                  - For Client Characteristic Configuration and Server Characteristic Configuration, value must be saved in user space,
                                    user needn't to set this value location bit. The UUID length type must be set to 0.*/
                                  
    uint16_t max_size;       /**< Attribute max size. \n
                                  - For Primary/Secondary/Included Services, this field is not used, set to 0. \n
                                  - For Characteristic Declaration, this field is not used, set to 0. \n
                                  - For Characteristic Extended Properties, this field contains 2-byte value. \n
                                  - For Client Characteristic Configuration and Server Characteristic Configuration, this field is not used, set to 0. \n
                                  - For others, this field is attribute max size. */
} attm_desc_128_t;

/**
 * @brief Parameter of Added service description.
 */
typedef struct
{

    uint16_t            *shdl;                              /**< Service start handle pointer.
                                                                 If *shdl = 0, it returns a handle using the first available handle (*shdl is modified); 
                                                                 otherwise it verifies if the given start handle can be used to allocate handle range.  */
    const uint8_t       *uuid;                              /**< Service UUID pointer. The pointer points to the Service UUID LSB. */
    uint8_t             *attr_tab_cfg;                      /**< Attribute table selector pointer. It can be set to NULL to select all items of attribute table. 
                                                                 Each bit matches with an attribute of attribute table. \n
                                                                 EXAMPLE:if attr_tab_cfg points to array {0x3F, 0x03}, it means that the 0.1.2.3.4.5.8.9 items of attribute table will be added to database. */
                                                    
    uint8_t             max_nb_attr;                        /**< Number of attributes in attribute table. */
    union attribute_table                                   /**< Attribute table. */
    {
        const attm_desc_t       *attr_tab_16;                /**< 16 bits service  description. The pointer point to attribute table of 16 bits service. See @ref attm_desc_t. */
        const attm_desc_128_t   *attr_tab_128;               /**< 128 bits service  description. The pointer point to attribute table of 128 bits service. See @ref attm_desc_128_t. */
    } attr_tab;                                              /**< Attribute table. */
    uint16_t            *inc_srvc_handle[MAX_INC_SRVC_NUM];  /**< Pointer array of Included Service start handle's address. */
    uint16_t             inc_srvc_num;                       /**< Included Service number for this service. */
    uint8_t              srvc_perm;                          /**< Service permission. See @ref BLE_GATTS_SRVC_PERM. */
    gatts_service_type_t attr_tab_type;                      /**< Service table type.  See @ref gatts_service_type_t. */
} gatts_create_db_t;

/**
 * @brief GATT read attribute result description.
 */
typedef struct
{
    uint16_t handle;         /**< Handle of the read attribute. */
    uint16_t length;         /**< Length of read data. */
    uint8_t  status;         /**< Status of read operation by upper layers. See @ref BLE_STACK_ERROR_CODES.*/
    uint8_t *value;          /**< Attribute value pointer. */
} gatts_read_cfm_t;

/**
 * @brief GATT write attribute result description.
 */
typedef struct
{
    uint16_t handle;          /**< Handle of the attribute written. */
    uint8_t  status;          /**< Status of write operation by upper layers. See @ref BLE_STACK_ERROR_CODES.*/
} gatts_write_cfm_t;

/**
 * @brief GATT prepare write result description.
 */
typedef struct
{
    uint16_t handle;         /**< Handle of the attribute in prepare write operation. */
    uint16_t length;         /**< Current length of the attribute. */
    uint8_t  status;         /**< Status of prepare write operation by upper layers. See @ref BLE_STACK_ERROR_CODES.*/
} gatts_prep_write_cfm_t;


/**
 * @brief GATT sending Notification or Indication event param description.
 */
typedef struct
{
    gatt_evt_type_t        type;       /**< Request type (Notification/Indication). see @ref gatt_evt_type_t. */
    uint16_t               handle;     /**< Characteristic Value handle to be notified or indicated. */
    uint16_t               length;     /**< Length of Characteristic Value to be sent. */
    uint8_t               *value;      /**< Characteristic Value pointer. */
} gatts_noti_ind_t;

/** @} */


/** @addtogroup BLE_GATTS_FUNCTIONS Functions
* @{ */
/**
 ****************************************************************************************
 * @brief Register a service's attribute list.
 *
 * @param[in, out] p_param: Pointer to the parameter used in creating databases. see @ref gatts_create_db_t.
 *
 * @retval ::SDK_SUCCESS: Database has been registered successfully.
 * @retval ::SDK_ERR_POINTER_NULL: Param is NULL or param's members are NULL.
 * @retval ::SDK_ERR_INVALID_PARAM: The member of param is invalid.
 * @retval ::SDK_ERR_INVALID_HANDLE: The service handles can not be allocated.
 * @retval ::SDK_ERR_NO_RESOURCES: There is not enough memory to allocate service buffer.
 * @retval ::SDK_ERR_INVALID_PERM: Permissions of Client Characteristic Configuration or Server Characteristic Configuration are not set correctly.
 ****************************************************************************************   
 */
uint16_t ble_gatts_srvc_db_create(gatts_create_db_t *p_param);

/**
 ****************************************************************************************
 * @brief Update attribute value only if the attribute value is saved in the BLE Stack space.
 *
 * @param[in] handle:     Attribute handle.
 * @param[in] length:     Size of the value to set.
 * @param[in] offset:     Data offset of the value in attribute value.
 * @param[in] p_value:    The value to set. If offset = 0, the value is the new attribute value; otherwise, the value is part of the new attribute value.
 *
 * @retval ::SDK_SUCCESS: Successfully update the attribute value.
 * @retval ::SDK_ERR_POINTER_NULL: Value is NULL.
 * @retval ::SDK_ERR_INVALID_HANDLE: Handle not exist in database.
 * @retval ::SDK_ERR_REQ_NOT_SUPPORTED: Attribute data is not present in database or cannot be modified.                       
 * @retval ::SDK_ERR_INVALID_ATT_VAL_LEN: New value length exceeds maximum attribute value length.            
 * @retval ::SDK_ERR_INVALID_OFFSET: Offset exceeds current attribute value length.
 ****************************************************************************************                              
 */
uint16_t ble_gatts_value_set(uint16_t handle, uint16_t length, uint16_t offset, const uint8_t* p_value);

/**
 ****************************************************************************************
 * @brief Retrieve attribute value only if the attribute value is saved in the BLE Stack space.
 *
 * @param[in]       handle:          Attribute handle.
 * @param[in,out]   p_length:        Input as buffer size and output as attribute value length.
 * @param[out]      p_value:         Buffer to store attribute value when buffer size is enough.
 *
 * @retval ::SDK_SUCCESS: Attribute value retrieved successfully.
 * @retval ::SDK_ERR_POINTER_NULL: The pointer to input buffer size or input buffer is NULL.
 * @retval ::SDK_ERR_INVALID_HANDLE: Handle not exist in the database.
 * @retval ::SDK_ERR_REQ_NOT_SUPPORTED: Attribute data is not present in database.
 * @retval ::SDK_ERR_INVALID_ATT_VAL_LEN: Attribute data value size is bigger than buffer size.
 * @retval ::SDK_ERR_APP_ERROR: Database is not correctly initialized by application.
 **************************************************************************************** 
 */
uint16_t ble_gatts_value_get(uint16_t handle, uint16_t* p_length, uint8_t* p_value);

/**
 ****************************************************************************************
 * @brief Update attribute permission.
 * @note  The modifications of attribute permission to service/character/include/character_extended_properties_descriptor declaration: not supported. \n
 *        The modifications of attribute permission to others: perm can be updated and EKS of ext_perm can be updated.See @ref BLE_GATTS_ATTR_PERM.
 *
 * @param[in] handle:       Attribute handle.
 * @param[in] perm:         New attribute permission.
 * @param[in] ext_perm:     New attribute extended permission.
 *
 * @retval ::SDK_SUCCESS: Update attribute permission successfully.
 * @retval ::SDK_ERR_INVALID_HANDLE: Handle not exist in the database.
 * @retval ::SDK_ERR_REQ_NOT_SUPPORTED: Attribute permission can't be modified.
 ****************************************************************************************
 */
uint16_t ble_gatts_attr_permission_set(uint16_t handle, uint16_t perm, uint16_t ext_perm);

/**
 ****************************************************************************************
 * @brief Retrieve attribute permission.
 *
 * @param[in]   handle:         Attribute handle.
 * @param[out]  p_perm:         Attribute permission value to be returned.
 * @param[out]  p_ext_perm:     Attribute extended permission value to be returned.
 *
 * @retval ::SDK_SUCCESS: Retrieve attribute permission successfully.
 * @retval ::SDK_ERR_POINTER_NULL: Perm or ext_perm is NULL.
 * @retval ::SDK_ERR_INVALID_HANDLE: Handle not exist in the database.
 ****************************************************************************************
 */
uint16_t ble_gatts_attr_permission_get(uint16_t handle, uint16_t *p_perm, uint16_t *p_ext_perm);

/**
 ****************************************************************************************
 * @brief Respond to an attribute read request. It is used in profile read callback function @ref gatts_prf_cbs_t::app_gatts_read_cb
 *        to send attribute value to stack which is saved in user space.
 *
 * @note The status member gatts_read_cfm_t::status should be set to @ref BLE_ATT_ERR_INSUFF_AUTHOR
 *       to control the authorization of particular read operations of a client.
 *
 * @param[in] conn_idx:        Current connection index.
 * @param[in] p_param:         Pointer to the parameters filled by profile. See @ref gatts_read_cfm_t.
 *
 * @retval ::SDK_SUCCESS: Send read confirm value to stack successfully.
 * @retval ::SDK_ERR_POINTER_NULL:  Param is NULL.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Conidx is invalid.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gatts_read_cfm(uint8_t conn_idx, const gatts_read_cfm_t *p_param);

/**
 ****************************************************************************************
 * @brief Respond to an attribute write request. It is used in profile write callback function
 * @ref gatts_prf_cbs_t::app_gatts_write_cb to send write operation status to stack. 
 *
 * @note The status member gatts_write_cfm_t::status should be set to @ref BLE_ATT_ERR_INSUFF_AUTHOR
 *           to control the authorization of particular client's write operation.
 *
 * @param[in] conn_idx:        Current connection index.
 * @param[in] p_param:         Pointer to the parameters filled by profile. see @ref gatts_write_cfm_t.
 *
 * @retval ::SDK_SUCCESS: Send write confirm status to stack successfully.
 * @retval ::SDK_ERR_POINTER_NULL:  Param is NULL.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Conidx is invalid.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gatts_write_cfm(uint8_t conn_idx, const gatts_write_cfm_t *p_param);

/**
 ****************************************************************************************
 * @brief Respond to an attribute prepare write request. It is used in profile prepare write callback
 *        function @ref gatts_prf_cbs_t::app_gatts_prep_write_cb to send prepare write operation status to stack. 
 *
 * @note The status member gatts_prep_write_cfm_t::status should be set to @ref BLE_ATT_ERR_INSUFF_AUTHOR
 *           to control the authorization of particular client's write operation.
 *
 * @param[in] conn_idx:        Current connection index.
 * @param[in] p_param:         Pointer to the parameters filled by profile. see @ref gatts_prep_write_cfm_t.
 *
 * @retval ::SDK_SUCCESS: Send prepare write confirm status to stack successfully.
 * @retval ::SDK_ERR_POINTER_NULL:  Param is NULL.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Conidx is invalid.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gatts_prepare_write_cfm(uint8_t conn_idx, const gatts_prep_write_cfm_t *p_param);

/**
 ****************************************************************************************
 * @brief Send out a notification or an indication. The execution status of sending notification or
 *        indication will be retrieved in the complete callback function @ref gatts_prf_cbs_t::app_gatts_ntf_ind_cb.
 *
 * @note Check whether the relevant Client Characteristic Configuration Descriptor is enabled before using this API.
 *
 * @param[in] conn_idx:       Current connection index.
 * @param[in] p_param:        Pointer to the parameters filled by profile. see @ref gatts_noti_ind_t.
 * 
 * @retval ::SDK_SUCCESS: Send Notification or Indication event to stack successfully.
 * @retval ::SDK_ERR_POINTER_NULL:  Param is NULL.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Conidx is invalid.
 * @retval ::SDK_ERR_INVALID_PARAM: Type is invalid.
 * @retval ::SDK_ERR_INVALID_HANDLE: Handle not exist in the database.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_gatts_noti_ind(uint8_t conn_idx, const gatts_noti_ind_t *p_param);

/**
 ****************************************************************************************
 * @brief When service on local device finishes upgrade, call this API to send service upgrade to stack.
 *        If the bonded device connects again, the stack will send service change to the bonded device until the bonded
 *        device has sent back indication confirmation.
 ****************************************************************************************
 */
void ble_gatts_service_changed(void);
/** @} */

#endif // BLE_SDK_GATTS_H_
/** @} */

/** @} */
/** @} */

