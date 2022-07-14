/*
* Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef INCLUSION_GUARD_FPC_TA_SENSOR_INTERFACE
#define INCLUSION_GUARD_FPC_TA_SENSOR_INTERFACE

#include "fpc_ta_interface.h"
#include "fpc_hw_identification_types.h"

typedef struct {
    fpc_ta_cmd_header_t header;
    fpc_hw_module_info_t data;
} fpc_ta_sensor_otp_info_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    uint8_t finger_present_qualification;
    fpc_tee_capture_mode_t capture_mode;
    int32_t cac_result;
    uint32_t prev_fngr_present;
    /**
     *  Screen brightness (nits).
     *  This should be the nit value that the system intends to use
     *  for the hotzone during the capture.
     */
    uint32_t screen_brightness;
} fpc_ta_sensor_capture_info_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    uint8_t ctrl;
} fpc_ta_sensor_early_stop_ctrl_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    uint8_t value;
} fpc_ta_sensor_force_value_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    fpc_module_quality_test_limits_t data;
} fpc_ta_sensor_mqt_limits_t;

typedef union {
    fpc_ta_cmd_header_t header;
    fpc_ta_cmd_header_t is_otp_supported;
    fpc_ta_sensor_otp_info_t otp_info;
    fpc_ta_sensor_capture_info_t capture_info;
    fpc_ta_sensor_early_stop_ctrl_t early_stop_ctrl;
    fpc_ta_sensor_force_value_t get_force_value;
    fpc_ta_cmd_header_t is_sensor_force_supported;
    fpc_ta_sensor_mqt_limits_t mqt_limits;
} fpc_ta_sensor_command_t;

typedef enum {
    FPC_TA_SENSOR_DEVICE_INIT_CMD,
    FPC_TA_SENSOR_CHECK_FINGER_LOST_CMD,
    FPC_TA_SENSOR_FINGER_LOST_WAKEUP_SETUP_CMD,
    FPC_TA_SENSOR_WAKEUP_SETUP_CMD,
    FPC_TA_SENSOR_CAPTURE_IMAGE_CMD,
    FPC_TA_SENSOR_DEEP_SLEEP_CMD,
    FPC_TA_SENSOR_GET_OTP_INFO_CMD,
    FPC_TA_SENSOR_EARLY_STOP_CTRL_CMD,
    FPC_TA_SENSOR_GET_FORCE_VALUE,
    FPC_TA_SENSOR_IS_SENSOR_FORCE_SUPPORTED,
    FPC_TA_SENSOR_GET_MQT_LIMITS_CMD,
    FPC_TA_SENSOR_CHECK_FNGR_DWN_UP_INT_CMD,
    FPC_TA_SENSOR_GET_OTP_LIMITS_CMD,
    FPC_TA_SENSOR_GET_UNIFORMITY_LIMITS_CMD,
    FPC_TA_SENSOR_GET_SNR_LIMITS_CMD,
} fpc_ta_sensor_cmd_t;

#endif /* INCLUSION_GUARD_FPC_TA_SENSOR_INTERFACE */
