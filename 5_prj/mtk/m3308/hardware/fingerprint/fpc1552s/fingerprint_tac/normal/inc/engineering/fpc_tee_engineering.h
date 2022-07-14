/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#ifndef FPC_TEE_ENGINEERING_H
#define FPC_TEE_ENGINEERING_H

#include <stdint.h>
#include "fpc_tee.h"
#include "fpc_ta_engineering_interface.h"

int fpc_tee_debug_get_retrieve_size(
    fpc_tee_t *tee,
    const uint32_t type,
    uint32_t  *raw_size);

int fpc_tee_debug_retrieve(
    fpc_tee_t *const tee,
    const uint32_t type,
    uint8_t *const buffer,
    const uint32_t buffer_size);

int fpc_tee_get_sensor_info(
    fpc_tee_t *tee,
    uint16_t *width,
    uint16_t *height);

int fpc_tee_debug_inject(
    fpc_tee_t *tee,
    const uint32_t type,
    const uint8_t *buffer,
    size_t buffer_size);

int fpc_tee_debug_disable_check_pixels(fpc_tee_t *tee);

/**
 * @brief Get enhanced image size.
 *
 * @param[in, out]  tee         tee handle.
 * @param[in, out]  width       width of image.
 * @param[in, out]  height      height of image.
 * @return          int         0 on success, otherwise fail.
 */
int fpc_tee_get_enhanced_image_size(fpc_tee_t *tee, uint16_t *const width, uint16_t *const height);

#endif /* FPC_TEE_ENGINEERING_H */
