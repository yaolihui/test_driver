/*
 * Copyright (c) 2021 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_ALGO_EXT_H_
#define FPC_ALGO_EXT_H_

#include <stdint.h>

#define FPC_ALGO_IDEN_INFO_SPOOF (1 << 1)

typedef enum {
    FPC_ALGO_IDENTIFY_NO_MATCH = 0,
    FPC_ALGO_IDENTIFY_MATCH = 1,
    FPC_ALGO_IDENTIFY_NO_MATCH_SPOOF = (FPC_ALGO_IDENTIFY_NO_MATCH | FPC_ALGO_IDEN_INFO_SPOOF),
} fpc_algo_identify_result_t;

#endif /* FPC_ALGO_EXT_H_ */
