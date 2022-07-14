/*
 * Copyright (c) 2018 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#include "fpc_tee_bio.h"
#include "fpc_tee.h"
#include "fpc_tee_bio_internal.h"

int fpc_tee_store_template_db(fpc_tee_bio_t *bio, const char *path)
{
    return fpc_tee_store_template_db_host(bio, path);
}

int fpc_tee_load_template_db(fpc_tee_bio_t *bio, const char *path)
{
    return fpc_tee_load_template_db_host(bio, path);
}
