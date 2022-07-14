/*
 * Copyright (c) 2015-2021 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#ifndef FPC_TEE_HAL_H
#define FPC_TEE_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <pthread.h>
#include <limits.h>

#define TEMPLATE_POSTFIX                "/user.db"

// This should be same as MAX_FAILED_ATTEMPTS in FingerprintService.java
#define FPC_MAX_IDENTIFY_ATTEMPTS 5

#ifndef FPC_CONFIG_RETRY_MATCH_TIMEOUT
#define FPC_CONFIG_RETRY_MATCH_TIMEOUT 600
#endif

#define DIFF_TIME(start,current) (current.tv_sec - start.tv_sec) * 1000 + \
                     (current.tv_nsec - start.tv_nsec) / 1000000

/* If the db file size (in bytes) is smaller than it, it is assumed that there is no templates in
 * the db. It is according to the observation with db file with/without templates. It is one or
 * several hundred kilo bytes for only 1 template in db. Thus the value we set here is a safe value
 * with enough margin. And it is only used by KPI features for KPI skipping.
 */
#define FPC_DB_FILE_SIZE_OF_NO_TEMPLATES (10000)

/* These enums are used by the retry on no match feature as state variables to know
 * to know which state identificaiton is in when giving feedback to Android.
 *
 * FPC_IDENTIFICATION_START:     No image have yet been taken and we are wating for finger on
                                 sensor.
 * FPC_IDENTIFICATION_RETRY:     An image have been taken but no identification attempt has been
                                 made.
 * FPC_IDENTIFICATION_ATTEMPTED: An image have successfully been taken and an identification attempt
                                 have been made. */

enum {
    FPC_IDENTIFICATION_START     = 0,
    FPC_IDENTIFICATION_RETRY     = 1,
    FPC_IDENTIFICATION_ATTEMPTED = 2,
};

#define HAL_COMPAT_VENDOR_BASE 1000
/* Compat vendor acquired message */
#define HAL_COMPAT_ACQUIRED_TOO_SIMILAR (HAL_COMPAT_VENDOR_BASE + 0)
#define HAL_COMPAT_ACQUIRED_LOW_MOBILITY (HAL_COMPAT_VENDOR_BASE + 1)
/* Compat vendor error code */
#define HAL_COMPAT_ERROR_ALREADY_ENROLLED (HAL_COMPAT_VENDOR_BASE + 0)

typedef enum {
    HAL_COMPAT_ERROR_NO_ERROR = 0,
    HAL_COMPAT_ERROR_HW_UNAVAILABLE = 1,
    HAL_COMPAT_ERROR_UNABLE_TO_PROCESS = 2,
    HAL_COMPAT_ERROR_TIMEOUT = 3,
    HAL_COMPAT_ERROR_NO_SPACE = 4,
    HAL_COMPAT_ERROR_CANCELED = 5,
    HAL_COMPAT_ERROR_UNABLE_TO_REMOVE = 6,
    HAL_COMPAT_ERROR_LOCKOUT = 7,
} fpc_hal_compat_error_t;

typedef enum {
    HAL_COMPAT_ACQUIRED_GOOD = 0,
    HAL_COMPAT_ACQUIRED_PARTIAL = 1,
    HAL_COMPAT_ACQUIRED_INSUFFICIENT = 2,
    HAL_COMPAT_ACQUIRED_IMAGER_DIRTY = 3,
    HAL_COMPAT_ACQUIRED_TOO_SLOW = 4,
    HAL_COMPAT_ACQUIRED_TOO_FAST = 5,
} fpc_hal_compat_acquired_t;

typedef struct fpc_hal_common fpc_hal_common_t;

typedef struct {
    void (*on_enroll_result)(void *context, uint32_t fid, uint32_t gid, uint32_t remaining);
    void (*on_acquired)(void *context, int code);
    void (*on_authenticated)(void *context, uint32_t fid, uint32_t gid, const uint8_t *token,
                             uint32_t size_token);
    void (*on_error)(void *context, int code);
    void (*on_removed)(void *context, uint32_t fid, uint32_t gid, uint32_t remaining);
    void (*on_enumerate)(void *context, uint32_t fid, uint32_t gid, uint32_t remaining);
} fpc_hal_compat_callback_t;

int fpc_hal_open(fpc_hal_common_t **device, const fpc_hal_compat_callback_t *callback,
                 void *callback_context);
void fpc_hal_close(fpc_hal_common_t *device);
uint64_t fpc_pre_enroll(fpc_hal_common_t *device);
int fpc_post_enroll(fpc_hal_common_t *device);
uint64_t fpc_get_authenticator_id(fpc_hal_common_t *device);
int fpc_set_active_group(fpc_hal_common_t *device, uint32_t gid, const char *store_path);

int fpc_authenticate(fpc_hal_common_t *device, uint64_t operation_id, uint32_t gid);

int fpc_enroll(fpc_hal_common_t *device, const uint8_t *hat, uint32_t size_hat, uint32_t gid,
               uint32_t timeout_sec);

int fpc_cancel(fpc_hal_common_t *device);
int fpc_remove(fpc_hal_common_t *device, uint32_t gid, uint32_t fid);
int fpc_enumerate(fpc_hal_common_t *device);

#ifdef __cplusplus
}
#endif

#endif // FPC_TEE_HAL_H
