/*
* Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef FPC_TA_HW_AUTH_INTERFACE_H
#define FPC_TA_HW_AUTH_INTERFACE_H

#include "fpc_ta_interface.h"

typedef struct {
    fpc_ta_cmd_header_t header;
    uint64_t challenge;
} fpc_ta_hw_auth_challenge_cmd_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    uint8_t enabled;
} fpc_ta_hw_auth_set_token_validation_t;

#ifdef FPC_CONFIG_ENROL_TIMEOUT
typedef struct {
    fpc_ta_cmd_header_t header;
    uint32_t timeout_sec;
    uint8_t is_set;
    int32_t response;
} fpc_ta_hw_auth_enrol_timeout_cmd_t;
#endif

typedef union {
    fpc_ta_cmd_header_t header;
    fpc_ta_hw_auth_challenge_cmd_t set_auth_challenge;
    fpc_ta_hw_auth_challenge_cmd_t get_enrol_challenge;
    fpc_ta_byte_array_msg_t authorize_enrol;
    fpc_ta_byte_array_msg_t get_auth_result;
    fpc_ta_byte_array_msg_t set_shared_key;
    fpc_ta_hw_auth_set_token_validation_t token_val;
#ifdef FPC_CONFIG_ENROL_TIMEOUT
    fpc_ta_hw_auth_enrol_timeout_cmd_t enrol_timeout;
#endif
} fpc_ta_hw_auth_command_t;

enum {
    FPC_TA_HW_AUTH_SET_AUTH_CHALLENGE = 1,
    FPC_TA_HW_AUTH_GET_ENROL_CHALLENGE = 2,
    FPC_TA_HW_AUTH_AUTHORIZE_ENROL = 3,
    FPC_TA_HW_AUTH_GET_AUTH_RESULT = 4,
    FPC_TA_HW_AUTH_SET_SHARED_KEY = 5,
    FPC_TA_HW_AUTH_SET_TOKEN_VALIDATION = 6,
#ifdef FPC_CONFIG_ENROL_TIMEOUT
    FPC_TA_HW_AUTH_CHECK_ENROL_TIMEOUT = 7,
#endif
};
#endif // FPC_TA_HW_AUTH_INTERFACE_H

