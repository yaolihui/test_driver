/*
 * Copyright (c) 2015-2021 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_TEE_H
#define FPC_TEE_H

#include <stdbool.h>
#include <stdint.h>
#include "fpc_ta_common_interface.h"

typedef struct fpc_tee fpc_tee_t;

fpc_tee_t* fpc_tee_init(void);
void fpc_tee_release(fpc_tee_t* tee);

/**
 * @brief Write TA log buffer to a file
 *
 * If the TA was built with FPC_LOG_TO_FILE this function will retrieve
 * the log buffer and write it to a file.
 *
 * @param scenario Description of the logged scenario, e.g. "authenticate" or "enroll".
 *                 This string will be appended to the filename.
 * @param tee      fpc_tee_t handle
 */
void fpc_tee_write_ta_log(const char *scenario, fpc_tee_t *tee);

/**
 * @brief Retrieve build information
 *
 * The build information originates from fpc_build_info.h under fpc_lib. Since
 * a rather large message will be allocated, sent to TA and then sent back,
 * the build info struct is returned via a callback so that the caller has to
 * deal less with memory management.
 *
 * @param tee                 fpc_tee_t handle
 * @param buildinfo_callback  Callback which will be called with the build info.
 *                            It's the full fpc_ta_common_build_info_msg_t
 *                            struct that's passed on, and the actual build info
 *                            string is available in build_info->array
 * @param usr                 Will be passed as the usr-parameter to
 *                            buildinfo_callback
 *
 * @return                    FPC_ERROR_NONE if successful (the callback will
 *                            only be called upon success)
 */
int fpc_tee_get_build_info(fpc_tee_t *tee,
                           void (*buildinfo_callback)(const fpc_ta_common_build_info_msg_t *build_info, void *usr), void *usr);

/**
 * @brief Write build information to the log
 *
 * Prints the build info string (from fpc_tee_get_build_info()) using LOGD()
 *
 * @param tee      fpc_tee_t handle
 *
 * @return         FPC_ERROR_NONE if successful
 */
int fpc_tee_log_build_info(fpc_tee_t *tee);

bool fpc_tee_engineering_enabled(const fpc_tee_t *tee);
bool fpc_tee_sensortest_enabled(const fpc_tee_t *tee);
bool fpc_tee_navigation_enabled(const fpc_tee_t *tee);
bool fpc_tee_authenticator_enabled(const fpc_tee_t *tee);
bool fpc_tee_authenticator_2_enabled(const fpc_tee_t *tee);
bool fpc_tee_navigation_debug_enabled(const fpc_tee_t *tee);
bool fpc_tee_log_to_file_enabled(const fpc_tee_t *tee);

/*
 * Check if ta was compiled with FPC_CONFIG_FORCE_SENSOR
 *
 * @param tee      fpc_tee_t handle
 *
 * @return true if the TA is compiled with FPC_CONFIG_FORCE_SENSOR
 */
bool fpc_tee_force_sensor_enabled(const fpc_tee_t *tee);

/*
 * Check if ta was compiled with FPC_TA_NAVIGATION_FORCE_SW
 *
 * @param tee      fpc_tee_t handle
 *
 * @return true if the TA is compiled with FPC_TA_NAVIGATION_FORCE_SW
 */
bool fpc_tee_navigation_force_sw_enabled(const fpc_tee_t *tee);

/*
 * Check if hardware or software force sensor is enabled (sensetouch)
 *
 * @param tee      fpc_tee_t handle

 * @return true if the TA is compiled with FPC_TA_NAVIGATION_FORCE_SW
 *         or FPC_CONFIG_FORCE_SENSOR.
 */
bool fpc_tee_sensetouch_enabled(const fpc_tee_t *tee);

/*
 * Get max number of templates that can be stored
 *
 * @param tee      fpc_tee_t handle
 *
 * @return max number of templates that can be stored at the same time
 */
uint32_t fpc_tee_get_max_number_of_templates(const fpc_tee_t *tee);

#endif // FPC_TEE_H
