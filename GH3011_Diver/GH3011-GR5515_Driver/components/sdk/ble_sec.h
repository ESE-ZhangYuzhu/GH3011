/**
 ****************************************************************************************
 *
 * @file ble_sec.h
 *
 * @brief BLE SEC API
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
  @addtogroup BLE_SEC Security Manager(SM)
  @{
  @brief Definitions and prototypes for the BLE_SEC interface.
 */

#ifndef __BLE_SEC_H__
#define __BLE_SEC_H__

#include "ble_error.h"
#include <stdbool.h>

/**@addtogroup BLE_SM_DEFINES Defines 
 * @{ 
 */
/**@defgroup SEC_AUTH_FLAG  SEC Auth Flag
* @{ 
*/
#define AUTH_NONE               0                 /**< No auth requirement. */
#define AUTH_BOND              (1 << 0)           /**< Bond flag. */
#define AUTH_MITM              (1 << 2)           /**< MITM flag. */
#define AUTH_SEC_CON           (1 << 3)           /**< Security connection flag. */
#define AUTH_KEY_PRESS_NOTIFY  (1 << 4)           /**< Key press notify flag. */
#define AUTH_ALL               (AUTH_BOND | AUTH_MITM | AUTH_SEC_CON | AUTH_KEY_PRESS_NOTIFY)  /**< All authentication flags are on. */
/**@} */

/**@defgroup SEC_KEY_DIST_FLAG  SEC Key Distribution Flag
* @{ 
*/
#define KDIST_NONE      0            /**< No key needs to be distributed. */
#define KDIST_ENCKEY   (1 << 0)      /**< Distribute encryption and master identification info. */
#define KDIST_IDKEY    (1 << 1)      /**< Distribute identity and address info. */
#define KDIST_SIGNKEY  (1 << 2)      /**< Distribute signing info. */
#define KDIST_ALL      (KDIST_ENCKEY | KDIST_IDKEY | KDIST_SIGNKEY)  /**< Distribute all info. */

/**@} */
/**@} */

/**@addtogroup BLE_SEC_ENUMERATIONS Enumerations
 * @{ */
/**@brief SEC IO Capability. */
typedef enum
{
    IO_DISPLAY_ONLY       = 0x00,       /**< Display only. */
    IO_DISPLAY_YES_NO     = 0x01,       /**< Display and input yes or no. */
    IO_KEYBOARD_ONLY      = 0x02,       /**< Keyboard only. */
    IO_NO_INPUT_NO_OUTPUT = 0x03,       /**< No input and no output. */
    IO_KEYBOARD_DISPLAY   = 0x04        /**< Keyboard and display. */
} sec_io_cap_t;

/**@brief SEC Encryption Request Type.
  *@note These types indicate some operations need to interact with app during pair process.
 */
typedef enum
{
    PAIR_REQ,       /**< Pair request. Apps need to decide whether to accept this request. */
    TK_REQ,         /**< TK request. Apps need to set the TK value. */
    OOB_REQ,        /**< OOB request. Apps need to set the OOB value. */
    NC_REQ          /**< Number comparison request. Apps need to check if it is the same number displayed in Master and Slave. */
} sec_enc_req_type_t;

/**@brief SEC Key Press Notify.  */
typedef enum
{
    KEY_PRESS_STARTED   = 0x00,        /**< Passkey entry started. */
    KEY_PRESS_ENTERED   = 0x01,        /**< Passkey digit entered. */
    KEY_PRESS_ERASED    = 0x02,        /**< Passkey digit erased. */
    KEY_PRESS_CLEARED   = 0x03,        /**< Passkey cleared. */
    KEY_PRESS_COMPLETED = 0x04         /**< Passkey entry completed. */
} sec_keypress_notify_t;

/**@brief SEC pair result.  */
typedef enum
{
    ENC_SUCCESS                     = 0x00, /**< Encrypt success. */
    ENC_FAIL_PASSKEY_ENTRY_FAIL     = 0x01, /**< The user input of passkey failed, for example, the user cancelled the operation. */
    ENC_FAIL_OOB_NOT_AVAILBL        = 0x02, /**< The OOB data is not available. */
    ENC_FAIL_AUTH_REQ               = 0x03, /**< The pairing procedure cannot be performed as authentication requirements cannot be met 
                                                 due to IO incapability of one or both devices. */                                                                                
    ENC_FAIL_CONFIRM_VAL_FAIL       = 0x04, /**< The confirm value does not match the calculated compare value. */
    ENC_FAIL_PAIRING_NOT_SUPPORT    = 0x05, /**< Pairing is not supported by the device. */
    ENC_FAIL_ENCRPT_KEY_SIZE        = 0x06, /**< The resultant encryption key size is insufficient for the security requirements of this device. */
    ENC_FAIL_COMMAND_NOT_SUPPORT    = 0x07, /**< The SMP command received is not supported on this device. */
    ENC_FAIL_UNSPECIFIED            = 0x08, /**< Pairing failed due to an unspecified reason. */
    ENC_FAIL_REPEAT_ATTEMPT         = 0x09, /**< Pairing or authentication procedure is disallowed because too little time has elapsed 
                                                 since last pairing request or security request. */
    ENC_FAIL_INVALID_PARAM          = 0x0A, /**< The Invalid Parameters error code indicates that the command length is invalid 
                                                 or that a parameter is outside of the specified range. */
    ENC_FAIL_DHKEY_CHECK_FAIL       = 0x0B, /**< Indicate to the remote device that the DHKey Check value received doesn't  match the one calculated 
                                                 by the local device. */
    ENC_FAIL_NUM_CMP_FAIL           = 0x0C, /**< Indicate that the confirm values in the numeric comparison protocol do not match. */
    ENC_FAIL_BR_EDR_IN_PROGRESS     = 0x0D, /**< Indicate that the pairing over the LE transport failed due to 
                                                 a Pairing Request sent over the BR/EDR transport in process. */
    ENC_FAIL_KEY_DRIV_GEN_NOT_ALLOW = 0x0E, /**< Indicate that the BR/EDR Link Key generated on the BR/EDR transport 
                                                 cannot be used to derive and distribute keys for the LE transport. */
    ENC_FAIL_LTK_MISSING = 0x0F,            /**< Indicate the LTK of peer devices missing. */
} sec_enc_ind_t;

/**@brief SEC mode and level.  */
typedef enum
{
    SEC_MODE1_LEVEL1 = 0x00,    /**< No security is needed. */
    SEC_MODE1_LEVEL2 = 0x01,    /**< Encrypted link is required. Unnecessary: MITM and SC. */
    SEC_MODE1_LEVEL3 = 0x02,    /**< Encrypted link is required. Necessary: MITM; unnecessary: SC. */
    SEC_MODE1_LEVEL4 = 0x03,    /**< Encrypted link is required. Necessary: MITM and SC. */
    SEC_MODE2_LEVEL1 = 0x04,    /**< Data signing is required. Unnecessary: MITM and SC. */
    SEC_MODE2_LEVEL2 = 0x05,    /**< Data signing is required. Necessary: MITM; unnecessary: SC. */
} sec_mode_level_t;
/** @} */

/**@addtogroup BLE_SEC_STRUCTURES Structures
 * @{ */
/**@brief SEC Parameter. */
typedef struct
{
    sec_mode_level_t level;         /**< Set the minimum security level of the device, see @ref sec_mode_level_t. */
    sec_io_cap_t     io_cap;        /**< Set the IO capability, see @ref sec_io_cap_t. */
    bool             oob;           /**< Indicate whether OOB is supported. */
    uint8_t          auth;          /**< Set the auth, see @ref SEC_AUTH_FLAG. */
    uint8_t          key_size;      /**< Indicate the supported maximum LTK size (range: 7-16). */
    uint8_t          ikey_dist;     /**< Set the initial key distribution, see @ref SEC_KEY_DIST_FLAG. */
    uint8_t          rkey_dist;     /**< Set the response key distribution, see @ref SEC_KEY_DIST_FLAG. */
} sec_param_t;

/**@brief TK value. */
typedef struct
{
    uint8_t key[16];          /**< TK value. */
} sec_tk_t;

/**@brief SEC OOB value. */
typedef struct
{
    uint8_t conf[16];        /**< Confirm value. */
    uint8_t rand[16];        /**< Random value. */
} sec_oob_t;

/**@brief SEC Confirm encryption data. */
typedef union
{
    sec_tk_t  tk;           /**< TK value, see @ref sec_tk_t. */
    sec_oob_t oob;          /**< OOB value, see @ref sec_oob_t. */
} sec_cfm_enc_data_t;

/**@brief SEC Confirm encryption. */
typedef struct
{
    sec_enc_req_type_t req_type;         /**< Request type, see @ref sec_enc_req_type_t. */
    bool               accept;           /**< Indicate whether to accept the request. */
    sec_cfm_enc_data_t data;             /**< SEC Confirm encryption data, see @ref sec_cfm_enc_data_t. */
} sec_cfm_enc_t;

/**@brief SEC number comparison value. */
typedef struct
{
    uint8_t value[4];       /**< Number comparison value (000000~999999). */
} sec_nc_t;

/**@brief SEC TK type. */
typedef enum
{
    SEC_TK_OOB = 0x00,        /**<TK got from OOB (out of band) method. */
    SEC_TK_DISPLAY,           /**<TK generated and shall be displayed by local device. */
    SEC_TK_KEY_ENTRY          /**<TK shall be entered by user using device keyboard. */
} sec_tk_type_t;

/**@brief SEC encryption request data. */
typedef union
{
    sec_tk_type_t tk_type;      /**<TK type, see @ref sec_tk_type_t. */
    sec_oob_t     oob_data;     /**<OOB data, see @ref sec_oob_t. */
    sec_nc_t      nc_data;      /**<Number comparison data, see @ref sec_nc_t. */
} sec_enc_req_data_t;

/**@brief SEC encryption request. */
typedef struct
{
    sec_enc_req_type_t req_type;        /**< Indicate the request type, @ref sec_enc_req_type_t. */
    sec_enc_req_data_t data;            /**< SEC encryption request data, @ref sec_enc_req_data_t. */
} sec_enc_req_t;

/**@brief SEC register call back. */
typedef struct
{
    void (*app_sec_enc_req_cb)(uint8_t conn_idx, sec_enc_req_t *p_enc_req);                   /**< Security manager module receives encryption request callback. */
    void (*app_sec_enc_ind_cb)(uint8_t conn_idx, sec_enc_ind_t enc_ind, uint8_t auth);        /**< Security manager module receives encryption indication callback, see @ref SEC_AUTH_FLAG. */
    void (*app_sec_keypress_notify_cb)(uint8_t conn_idx, sec_keypress_notify_t notify_type);  /**< Security manager module receives key press notify callback. */
    void (*app_sec_key_missing_cb)(uint8_t conn_idx);                                         /**< Security manager module receives key missing callback. */
} sec_cb_fun_t;
/** @} */

/** @addtogroup BLE_SEC_FUNCTIONS Functions
 * @{ */
/**
 ****************************************************************************************
 * @brief Set security parameter.
 *
 * @param[in] p_sec_param Pointer to the security parameter structure, @ref sec_param_t.
 *
 * @retval ::SDK_SUCCESS: The security parameter is successfully set to the BLE stack.
 * @retval ::SDK_ERR_POINTER_NULL: Invalid pointer supplied.
 * @retval ::SDK_ERR_INVALID_PARAM: Invalid parameter supplied.
 ****************************************************************************************
 */
uint16_t ble_sec_params_set(sec_param_t *p_sec_param);

/**
 ****************************************************************************************
 * @brief Start security encryption, this interface is used by both slave and master
 *
 * @note If the local device role is master, it will check that if the peer device is bonded firstly. If the peer device is bonded, 
 *  the stack will encrypt the link directly, otherwise the stack will send a pair request to the peer device.
 *  During the pairing, the callback @ref sec_cb_fun_t::app_sec_enc_req_cb may be called.
 *  After the link has been encrypted, the callback @ref sec_cb_fun_t::app_sec_enc_ind_cb will be called.
 * @note If the local device role is slave, the stack will send a security request to the peer device.
 *
 * @param[in] conn_idx ACL connection index, the first ACL connection index is 0, and increased one by one.
 *
 * @retval ::SDK_SUCCESS: The security encryption is successfully set to the BLE stack.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 ****************************************************************************************
 */
uint16_t ble_sec_enc_start(uint8_t conn_idx);

/**
 ****************************************************************************************
 * @brief Send the encrypt confirm information
 * @note This function should be called in the handle of callback @ref sec_cb_fun_t::app_sec_enc_req_cb.
 *
 * @param[in] conn_idx  ACL connection index, the first ACL connection index is 0, and increased one by one.
 * @param[in] p_cfm_enc Pointer to the confirm encryption structure, see @ref sec_cfm_enc_t.
 *
 * @retval ::SDK_SUCCESS: The confirm encryption is successfully set to the BLE stack.
 * @retval ::SDK_ERR_POINTER_NULL: Invalid pointer supplied.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_sec_enc_cfm(uint8_t conn_idx, const sec_cfm_enc_t *p_cfm_enc);

/**
 ****************************************************************************************
 * @brief Send key press notify
 *
 * @param[in] conn_idx    ACL connection index. The first ACL connection index is 0, and the index will be increased one by one.
 * @param[in] notify_type Key press notify type, see @ref sec_keypress_notify_t.
 *
 * @retval ::SDK_SUCCESS: The key press notify type is successfully set to the BLE stack.
 * @retval ::SDK_ERR_INVALID_CONN_IDX: Invalid connection index supplied.
 * @retval ::SDK_ERR_NO_RESOURCES: Not enough resources.
 ****************************************************************************************
 */
uint16_t ble_sec_keypress_notify_send(uint8_t conn_idx, uint8_t notify_type);
/** @} */

#endif

/** @} */
/** @} */
