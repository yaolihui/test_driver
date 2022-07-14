/*
* Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef FPC_TA_ENGINEERING_INTERFACE_H
#define FPC_TA_ENGINEERING_INTERFACE_H

#include "fpc_ta_interface.h"

typedef enum {
    ENG_NONE_DATA, //Initiated state
    ENG_RAW_IMAGE,
    ENG_ENHANCED_IMAGE,
} fpc_engineering_data_type_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    uint16_t width;
    uint16_t height;
} fpc_ta_engineering_get_sensor_info_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    uint16_t width;
    uint16_t height;
} fpc_ta_engineering_get_enhanced_image_size_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    uint32_t size;
    fpc_engineering_data_type_t retrieve_type;
    uint8_t array[];
} fpc_ta_engineering_retrieve_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    uint32_t size;
    fpc_engineering_data_type_t inject_type;
    uint8_t array[];
} fpc_ta_engineering_inject_byte_array_msg_t;

typedef union {
    fpc_ta_cmd_header_t                             header;
    fpc_ta_engineering_retrieve_t                   retrieve;
    fpc_ta_engineering_inject_byte_array_msg_t      inject;
    fpc_ta_engineering_get_sensor_info_t            sensor_info;
    fpc_ta_engineering_get_enhanced_image_size_t    enhanced_image_size;
    fpc_ta_size_msg_t                               raw_size;
} fpc_ta_engineering_command_t;

typedef enum {
    FPC_TA_ENGINEERING_RETRIEVE                 = 1,
    FPC_TA_ENGINEERING_GET_SENSOR_INFO          = 2,
    FPC_TA_ENGINEERING_INJECT_RAW               = 3,
    FPC_TA_ENGINEERING_SET_INJECT_SIZE          = 4,
    FPC_TA_ENGINEERING_GET_RETRIEVE_SIZE        = 5,
    FPC_TA_ENGINEERING_DISABLE_CHECK_PIXELS     = 6,
} fpc_ta_engineering_cmd_t;

#endif /* FPC_TA_ENGINEERING_INTERFACE_H */
