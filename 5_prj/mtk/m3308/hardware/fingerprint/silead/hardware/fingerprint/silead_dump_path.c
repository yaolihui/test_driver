/******************************************************************************
 * @file   silead_dump_path.c
 * @brief  Contains dump image functions.
 *
 *
 * Copyright (c) 2016-2021 GigaDevice/Silead Inc.
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * GigaDevice/Silead Inc. You shall not disclose the present software and
 * shall use it only in accordance with the terms of the license agreement
 * you entered into with GigaDevice/Silead Inc. This software may be
 * subject to export or import laws in certain countries.
 *
 *
 * ------------------- Revision History ------------------------------
 * <author>    <date>   <version>     <desc>
 * Calvin Wang 2020/03/10 0.1.0      Init version
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#ifdef SIL_DUMP_IMAGE

#define FILE_TAG "dump_path"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>

#include "silead_const.h"
#include "silead_util.h"
#include "silead_dump_param.h"
#include "silead_dump_path.h"
#include "silead_dump.h"

#ifndef SIL_DUMP_DATA_PATH
#define SIL_DUMP_DATA_PATH "/data/vendor/silead"
#endif

#define DUMP_PATH_PREFIX_ADD(type, item) { type, NUM_ELEMS(item), item }

typedef struct _dump_subpath_prefix {
    uint32_t subtype;
    void *prefix;
} dump_subpath_prefix_t;

typedef struct _dump_path_prefix {
    uint32_t type;
    uint32_t count;
    dump_subpath_prefix_t *prefix_list;
} dump_path_prefix_t;

/* DUMP_IMG_SUBTYPE_OTHER & DUMP_IMG_TYPE_OTHER must be the last element */
static dump_subpath_prefix_t m_dump_auth_prefix[] = {
    { DUMP_IMG_SUBTYPE_ORIG,            "work/raw/verify" },
    { DUMP_IMG_SUBTYPE_SHOT_FAIL,       "work/bin/verify" },
    { DUMP_IMG_SUBTYPE_RAW_SUCC,        "work/bin/verify" },
    { DUMP_IMG_SUBTYPE_RAW_FAIL,        "work/bin/verify" },
    { DUMP_IMG_SUBTYPE_RAW_SUCC_DIFF,   "work/bin/verify" },
    { DUMP_IMG_SUBTYPE_RAW_FAIL_DIFF,   "work/bin/verify" },
    { DUMP_IMG_SUBTYPE_MRAW,            "work/raw/verify_mraw" },
    { DUMP_IMG_SUBTYPE_OTHER,           "work/bin/verify" },
};
static dump_subpath_prefix_t m_dump_enroll_prefix[] = {
    { DUMP_IMG_SUBTYPE_ORIG,            "work/raw/enroll" },
    { DUMP_IMG_SUBTYPE_SHOT_FAIL,       "work/bin/enroll" },
    { DUMP_IMG_SUBTYPE_RAW_SUCC,        "work/bin/enroll" },
    { DUMP_IMG_SUBTYPE_RAW_FAIL,        "work/bin/enroll" },
    { DUMP_IMG_SUBTYPE_RAW_SUCC_DIFF,   "work/bin/enroll" },
    { DUMP_IMG_SUBTYPE_RAW_FAIL_DIFF,   "work/bin/enroll" },
    { DUMP_IMG_SUBTYPE_NEW,             "work/bin/enroll_new" },
    { DUMP_IMG_SUBTYPE_NEW_DIFF,        "work/bin/enroll_new" },
    { DUMP_IMG_SUBTYPE_OTHER,           "work/bin/enroll" },
};
static dump_subpath_prefix_t m_dump_nav_prefix[] = {
    { DUMP_IMG_SUBTYPE_OTHER,       "work/nav" },
};
static dump_subpath_prefix_t m_dump_cal_prefix[] = {
    { DUMP_IMG_SUBTYPE_OTHER,         "test/cal" },
};
static dump_subpath_prefix_t m_dump_ft_prefix[] = {
    { DUMP_IMG_SUBTYPE_OTHER,         "test/ft" },
};
static dump_subpath_prefix_t m_dump_snr_prefix[] = {
    { DUMP_IMG_SUBTYPE_OTHER,         "test/snr" },
};
static dump_subpath_prefix_t m_dump_other_prefix[] = {
    { DUMP_IMG_SUBTYPE_OTHER,         "other_orig" },
};

static dump_path_prefix_t m_dump_perfix[] = {
    DUMP_PATH_PREFIX_ADD(DUMP_IMG_TYPE_AUTH,    m_dump_auth_prefix),
    DUMP_PATH_PREFIX_ADD(DUMP_IMG_TYPE_ENROLL,  m_dump_enroll_prefix),
    DUMP_PATH_PREFIX_ADD(DUMP_IMG_TYPE_NAV,     m_dump_nav_prefix),
    DUMP_PATH_PREFIX_ADD(DUMP_IMG_TYPE_CAL,     m_dump_cal_prefix),
    DUMP_PATH_PREFIX_ADD(DUMP_IMG_TYPE_FT_QA,   m_dump_ft_prefix),
    DUMP_PATH_PREFIX_ADD(DUMP_IMG_TYPE_SNR,     m_dump_snr_prefix),
    DUMP_PATH_PREFIX_ADD(DUMP_IMG_TYPE_OTHER,   m_dump_other_prefix),
};

static char m_str_dump_path[MAX_PATH_LEN] = {0};
const char *silfp_dump_path_get(void)
{
    if (m_str_dump_path[0] != '\0') {
        return m_str_dump_path;
    } else {
        return SIL_DUMP_DATA_PATH;
    }
}

void silfp_dump_path_set(const void *path, uint32_t len)
{
    int32_t ret = 0;

    ret = silfp_util_path_copy(m_str_dump_path, sizeof(m_str_dump_path), path, len);
    if (ret < 0) {
        memset(m_str_dump_path, 0, sizeof(m_str_dump_path));
    }
    LOG_MSG_VERBOSE("path = %s", silfp_dump_path_get());
}

const char * __attribute__((weak)) silfp_cust_dump_path_get_prefix(uint32_t type, uint32_t subtype);
static const char *_dump_path_get_prefix(uint32_t type, uint32_t subtype)
{
    const char *prefix = NULL;
    int32_t i = 0;
    int32_t count = 0;
    dump_subpath_prefix_t *prefix_list = NULL;

    if (silfp_cust_dump_path_get_prefix != NULL) {
        prefix = silfp_cust_dump_path_get_prefix(type, subtype);
        if (prefix != NULL) {
            return prefix;
        }
    }

    count = NUM_ELEMS(m_dump_perfix);
    for (i = 0; i < count; i++) {
        if ((m_dump_perfix[i].type == type) || (m_dump_perfix[i].type == DUMP_IMG_TYPE_OTHER)) {
            count = m_dump_perfix[i].count;
            prefix_list = m_dump_perfix[i].prefix_list;
            break;
        }
    }

    for (i = 0; i < count; i++) {
        if ((prefix_list != NULL) && ((prefix_list[i].subtype == subtype) || (prefix_list[i].subtype == DUMP_IMG_SUBTYPE_OTHER))) {
            prefix = prefix_list[i].prefix;
            break;
        }
    }

    if (prefix == NULL) {
        prefix = "unknow";
    }

    return prefix;
}

int32_t silfp_dump_path_get_save_path(char *path, uint32_t len, uint32_t type, uint32_t subtype, uint32_t mode)
{
    const char *rootdir = NULL;
    const char *subdir = NULL;

    if ((path == NULL) || (len == 0)) {
        return -1;
    }

    memset(path, 0, len);

    if (CHECK_FLAG_UNSET(mode, DUMP_NAME_MODE_PARENT_DIR_NULL)) {
        rootdir = silfp_dump_path_get();
    }

    if (CHECK_FLAG_UNSET(mode, DUMP_NAME_MODE_SUBDIR_NULL)) {
        subdir = _dump_path_get_prefix(type, subtype);
    }

    if (rootdir != NULL) {
        if (subdir != NULL) {
            snprintf(path, len, "%s/%s", rootdir, subdir);
        } else {
            snprintf(path, len, "%s", rootdir);
        }
    } else {
        if (subdir != NULL) {
            snprintf(path, len, "%s", subdir);
        }
    }

    return 0;
}

#endif /* SIL_DUMP_IMAGE */