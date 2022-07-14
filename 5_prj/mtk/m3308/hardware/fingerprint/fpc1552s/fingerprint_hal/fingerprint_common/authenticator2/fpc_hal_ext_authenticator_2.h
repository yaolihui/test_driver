/*
 * Copyright (c) 2021 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#ifndef FPC_HAL_EXT_AUTHENTICATOR_2_H
#define FPC_HAL_EXT_AUTHENTICATOR_2_H

#include <inttypes.h>
#include <stdbool.h>
#include "fpc_tee_hal.h"

typedef struct {
    int64_t user_id;
    int64_t entity_id;
    uint8_t *result_blob;
    uint32_t size_result_blob;
} fpc_verify_user_data_2_t;


typedef struct {
    void (*on_enroll_result)(void *context, uint32_t gid, uint32_t remaining);
    void (*on_authenticated)(void *context, uint32_t id);
    void (*on_enumerate)(void *context, uint32_t id, uint32_t remaining);
    void (*on_removed)(void *context, uint32_t id, uint32_t remaining);
    void (*on_acquired)(void *context, int32_t code);
    void (*on_error)(void *context, int32_t code);
} fpc_authenticator_compat_callback_t;

typedef struct fpc_authenticator_2 fpc_authenticator_2_t;

struct fpc_authenticator_2 {
    void (*init)(fpc_authenticator_2_t *self,
                 const fpc_authenticator_compat_callback_t *callback,
                 void *callback_context);

    void (*enroll)(fpc_authenticator_2_t *self);

    void (*authenticate)(fpc_authenticator_2_t *self);

    void (*enumerate)(fpc_authenticator_2_t *self);

    void (*remove)(fpc_authenticator_2_t *self, uint32_t id);

    void (*cancel)(fpc_authenticator_2_t *self);
};

fpc_authenticator_2_t *fpc_authenticator_2_new(fpc_hal_common_t *hal);

void fpc_authenticator_2_destroy(fpc_authenticator_2_t *self);

#endif // FPC_HAL_EXT_AUTHENTICATOR_H
