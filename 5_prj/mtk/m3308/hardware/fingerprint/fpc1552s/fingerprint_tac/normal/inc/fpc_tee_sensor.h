/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_TEE_SENSOR_H
#define FPC_TEE_SENSOR_H

#include "fpc_tee.h"
#include "fpc_hw_identification_types.h"

typedef struct fpc_tee_sensor fpc_tee_sensor_t;

fpc_tee_sensor_t* fpc_tee_sensor_init(fpc_tee_t* tee);
void fpc_tee_sensor_release(fpc_tee_sensor_t* sensor);

/*
 * If parameter wait_for_finger is set, the irq driver
 * is set to take wakelock when irq is fetched.
 * Then we wait for the finger to be removed from sensor.
 * In a qualification loop, wait for finger down if wait_for_finger
 * is set and request TA side to capture and image from the sensor.
 *
 * @param sensor The sensor struct as returned by fpc_tee_sensor_init
 * @param wait_for_finger This is a parameter that symbolises if we want to
 * perform a capture without a verification if there is a finger on the sensor.
 *
 * @return FPC_CAPTURE_*
 *         FPC_ERROR_*
 */
int fpc_tee_capture_image(fpc_tee_sensor_t *sensor, uint8_t wait_for_finger);

/*
 * The image capture function which will be used by swipe-to-enrol feature.
 *
 * @param sensor The sensor struct as returned by fpc_tee_sensor_init
 * @param wait_for_finger This is a parameter that symbolises if we want to
 * perform a capture without a verification if there is a finger on the sensor.
 *
 * @return FPC_CAPTURE_*
 *         FPC_ERROR_*
 */
int fpc_tee_capture_image_swipe(fpc_tee_sensor_t *sensor, uint8_t wait_for_finger);

/**
 * Get last CAC result code.
 *
 * @param sensor The sensor struct as returned by fpc_tee_sensor_init
 *
 * @return CAC_SUCCESS
 *         CAC_ERROR_*
 */
int32_t fpc_tee_get_last_cac_result(fpc_tee_sensor_t *sensor);

/**
 * Wait for finger to be removed from the sensor
 *
 * This function will wait forever or until cancelled until finger is removed from the sensor.
 * It will do so by setting up an interrupt and wait for it.
 *
 * @return 0                        if finger was removed
 *         -FPC_ERROR_CANCELLED     if the check was cancelled
 *
 */
int fpc_tee_wait_finger_lost(fpc_tee_sensor_t *sensor);
int fpc_tee_wait_finger_down(fpc_tee_sensor_t *sensor);
int fpc_tee_wait_finger_down_wo_fngr_int_qual(fpc_tee_sensor_t *sensor);
int fpc_tee_check_finger_lost(fpc_tee_sensor_t *sensor);
int fpc_tee_set_cancel(fpc_tee_sensor_t *sensor);
int fpc_tee_clear_cancel(fpc_tee_sensor_t *sensor);
int fpc_tee_wait_irq(fpc_tee_sensor_t *sensor, int irq_value);
int fpc_tee_sensor_cancelled(fpc_tee_sensor_t *sensor);
int fpc_tee_status_irq(fpc_tee_sensor_t *sensor);
int fpc_tee_get_sensor_otp_info(fpc_tee_sensor_t *sensor, fpc_hw_module_info_t *otp_info);
int fpc_tee_early_stop_ctrl(fpc_tee_sensor_t *sensor, uint8_t *ctrl);
int fpc_tee_wait_for_button_down_force(fpc_tee_sensor_t *sensor,
                                       uint32_t force_button_down_timeout_ms,
                                       uint8_t force_button_down_threshold);
int fpc_tee_wait_for_button_up_force(fpc_tee_sensor_t *sensor,
                                     uint32_t force_button_up_timeout_ms,
                                     uint8_t force_button_up_threshold);
int fpc_tee_get_mqt_limits(fpc_tee_sensor_t *sensor,
                           fpc_module_quality_test_limits_t *mqt_limits);

/**
 * Reads the force sensor value from ADC
 *
 * @param sensor
 * @param[out] value - value between 0-255 representing the pressure force
 *
 * @return 0 if successful, if not the error code
 */
int fpc_tee_get_sensor_force_value(fpc_tee_sensor_t* sensor, uint8_t* value);

/**
 * Checks if force sensor is supported
 *
 * @param sensor
 * @param[out] is_supported - 1 if force sensor is supported, 0 otherwise
 *
 * @return 0 if successful, if not the error code
 */
int fpc_tee_is_sensor_force_supported(fpc_tee_sensor_t* sensor, uint8_t* is_supported);

#endif /* FPC_TEE_SENSOR_H */
