/******************************************************************************
 * @file   silead_dump_name.c
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

#define FILE_TAG "dump_name"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>

#include "silead_const.h"
#include "silead_util.h"
#include "silead_dump_param.h"
#include "silead_dump_util.h"
#include "silead_dump_name.h"
#include "silead_dump.h"
#include "silead_util.h"

#define DUMP_IMAGE_EXT "bmp"
#define DUMP_DATA_STEP_INVALID 0xFF
#define DUMP_DATA_TIMESTAMP_INVALID ((uint64_t)(-1))
#define DUMP_STEP_IS_INVALID(a) ((a) == DUMP_DATA_STEP_INVALID)
#define DUMP_STEP_VALUE(a) (a)
#define DUMP_TIMESTAMP_IS_INVALID(a) ((a) == DUMP_DATA_TIMESTAMP_INVALID)

#define ITEM_SEPRATOR(p) ((p == NULL) ? "" : "-")
#define ITEM_VALUE(p) (char *)((p == NULL) ? "" : p)
#define DUMP_NAME_PREFIX_ADD(type, item) { type, NUM_ELEMS(item), item }

typedef struct _dump_subname_prefix {
    uint32_t subtype;
    void *value;
} dump_subname_prefix_t, dump_name_ext_t;

typedef struct _dump_name_prefix {
    uint32_t type;
    uint32_t count;
    dump_subname_prefix_t *prefix_list;
} dump_name_prefix_t;

/* DUMP_IMG_SUBTYPE_OTHER & DUMP_IMG_TYPE_OTHER must be the last element */
static dump_subname_prefix_t m_dump_auth_prefix[] = {
    { DUMP_IMG_SUBTYPE_ORIG,            "auth_orig" },
    { DUMP_IMG_SUBTYPE_SHOT_FAIL,       "auth_shot_fail" },
    { DUMP_IMG_SUBTYPE_SHOT_FAIL_DIFF,  "auth_shot_fail" },
    { DUMP_IMG_SUBTYPE_RAW_SUCC,        "auth_succ" },
    { DUMP_IMG_SUBTYPE_RAW_FAIL,        "auth_fail" },
    { DUMP_IMG_SUBTYPE_RAW_SUCC_DIFF,   "auth_succ" },
    { DUMP_IMG_SUBTYPE_RAW_FAIL_DIFF,   "auth_fail" },
    { DUMP_IMG_SUBTYPE_MRAW,            "auth_mraw" },
    { DUMP_IMG_SUBTYPE_OTHER,           "auth_other" },
};
static dump_subname_prefix_t m_dump_enroll_prefix[] = {
    { DUMP_IMG_SUBTYPE_ORIG,            "enroll_orig" },
    { DUMP_IMG_SUBTYPE_SHOT_FAIL,       "enroll_shot_fail" },
    { DUMP_IMG_SUBTYPE_SHOT_FAIL_DIFF,  "enroll_shot_fail" },
    { DUMP_IMG_SUBTYPE_RAW_SUCC,        "enroll_succ" },
    { DUMP_IMG_SUBTYPE_RAW_FAIL,        "enroll_fail" },
    { DUMP_IMG_SUBTYPE_RAW_SUCC_DIFF,   "enroll_succ" },
    { DUMP_IMG_SUBTYPE_RAW_FAIL_DIFF,   "enroll_fail" },
    { DUMP_IMG_SUBTYPE_NEW,             "enroll_new" },
    { DUMP_IMG_SUBTYPE_NEW_DIFF,        "enroll_new" },
    { DUMP_IMG_SUBTYPE_OTHER,           "enroll_other" },
};
static dump_subname_prefix_t m_dump_nav_prefix[] = {
    { DUMP_IMG_SUBTYPE_RAW_SUCC,    "nav_succ" },
    { DUMP_IMG_SUBTYPE_RAW_FAIL,    "nav_fail" },
    { DUMP_IMG_SUBTYPE_OTHER,       "nav_other" },
};
static dump_subname_prefix_t m_dump_cal_prefix[] = {
    { DUMP_IMG_SUBTYPE_ORIG,        "cal_orig" },
    { DUMP_IMG_SUBTYPE_RAW_SUCC,    "cal_succ" },
    { DUMP_IMG_SUBTYPE_RAW_FAIL,    "cal_fail" },
    { DUMP_IMG_SUBTYPE_OTHER,       "cal_other" },
};
static dump_subname_prefix_t m_dump_ft_prefix[] = {
    { DUMP_IMG_SUBTYPE_ORIG,        "ft_orig" },
    { DUMP_IMG_SUBTYPE_OTHER,       "ft_other" },
};
static dump_subname_prefix_t m_dump_snr_prefix[] = {
    { DUMP_IMG_SUBTYPE_ORIG,        "snr_orig" },
    { DUMP_IMG_SUBTYPE_OTHER,       "snr_other" },
};
static dump_subname_prefix_t m_dump_other_prefix[] = {
    { DUMP_IMG_SUBTYPE_ORIG,        "other_orig" },
    { DUMP_IMG_SUBTYPE_RAW_SUCC,    "other_succ" },
    { DUMP_IMG_SUBTYPE_RAW_FAIL,    "other_fail" },
    { DUMP_IMG_SUBTYPE_OTHER,       "other" },
};

static dump_name_prefix_t m_dump_perfix[] = {
    DUMP_NAME_PREFIX_ADD(DUMP_IMG_TYPE_AUTH,    m_dump_auth_prefix),
    DUMP_NAME_PREFIX_ADD(DUMP_IMG_TYPE_ENROLL,  m_dump_enroll_prefix),
    DUMP_NAME_PREFIX_ADD(DUMP_IMG_TYPE_NAV,     m_dump_nav_prefix),
    DUMP_NAME_PREFIX_ADD(DUMP_IMG_TYPE_CAL,     m_dump_cal_prefix),
    DUMP_NAME_PREFIX_ADD(DUMP_IMG_TYPE_FT_QA,   m_dump_ft_prefix),
    DUMP_NAME_PREFIX_ADD(DUMP_IMG_TYPE_SNR,     m_dump_snr_prefix),
    DUMP_NAME_PREFIX_ADD(DUMP_IMG_TYPE_OTHER,   m_dump_other_prefix),
};

static dump_name_ext_t m_dump_name_ext[] = {
    { DUMP_IMG_SUBTYPE_SHOT_FAIL_DIFF,"dat" },
    { DUMP_IMG_SUBTYPE_RAW_SUCC_DIFF, "dat" },
    { DUMP_IMG_SUBTYPE_RAW_FAIL_DIFF, "dat" },
    { DUMP_IMG_SUBTYPE_NEW_DIFF, "dat" },
};

#define DUMP_REBOOT_TIMES_NO_GET ((uint32_t)-1)
static uint32_t m_reboot_times = DUMP_REBOOT_TIMES_NO_GET;
static uint32_t m_bmp_index = 0;
void silfp_dump_name_init(void)
{
    m_reboot_times = DUMP_REBOOT_TIMES_NO_GET;
    m_bmp_index = 0;
}

void silfp_dump_name_deinit(void)
{
    m_reboot_times = DUMP_REBOOT_TIMES_NO_GET;
    m_bmp_index = 0;
}

static int32_t m_dump_name_max_len = -1;
static char m_str_dump_ext[16] = {0};
static const char *_dump_name_get_ext_default(void)
{
    if (m_str_dump_ext[0] != '\0') {
        return m_str_dump_ext;
    } else {
        return DUMP_IMAGE_EXT;
    }
}

void silfp_dump_name_set_ext(int32_t maxlen, const void *ext, uint32_t len)
{
    int32_t ret = 0;

    m_dump_name_max_len = maxlen;
    ret = silfp_util_strcpy(m_str_dump_ext, sizeof(m_str_dump_ext), ext, len);
    if (ret < 0) {
        memset(m_str_dump_ext, 0, sizeof(m_str_dump_ext));
    }
    LOG_MSG_VERBOSE("ext=%s, maxlen=%d", _dump_name_get_ext_default(), m_dump_name_max_len);
}

const char * __attribute__((weak)) silfp_cust_dump_name_get_ext(uint32_t type, uint32_t subtype);
static const char *_dump_name_get_ext(uint32_t type, uint32_t subtype)
{
    const char *ext = NULL;
    int32_t i = 0;
    int32_t count = 0;

    if (silfp_cust_dump_name_get_ext != NULL) {
        ext = silfp_cust_dump_name_get_ext(type, subtype);
        if (ext != NULL) {
            return ext;
        }
    }

    count = NUM_ELEMS(m_dump_name_ext);
    for (i = 0; i < count; i++) {
        if (m_dump_name_ext[i].subtype == subtype) {
            ext = m_dump_name_ext[i].value;
            break;
        }
    }

    if (ext == NULL) {
        ext = _dump_name_get_ext_default();
    }

    return ext;
}
const char * __attribute__((weak)) silfp_cust_dump_name_get_prefix(uint32_t type, uint32_t subtype);
static const char *_dump_name_get_prefix(uint32_t type, uint32_t subtype)
{
    const char *prefix = NULL;
    int32_t i = 0;
    int32_t count = 0;
    dump_subname_prefix_t *prefix_list = NULL;

    if (silfp_cust_dump_name_get_prefix != NULL) {
        prefix = silfp_cust_dump_name_get_prefix(type, subtype);
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
            prefix = prefix_list[i].value;
            break;
        }
    }

    if (prefix == NULL) {
        prefix = "unknow";
    }

    return prefix;
}

/* dump reboot times, to distinguish the dump image when reboot the device or kill bio service */
#define DUMP_REBOOT_TIMES_FILE "bmp_times.dat"
static uint32_t _dump_name_get_reboot_times(const char *dir)
{
    int32_t ret = 0;
    char path[MAX_PATH_LEN] = {0};
    void *buf = NULL;
    int32_t len = 0;

    if (m_reboot_times == DUMP_REBOOT_TIMES_NO_GET) {
        if (dir != NULL && dir[0] != '\0') {
            snprintf(path, sizeof(path), "%s/%s", dir, DUMP_REBOOT_TIMES_FILE);
        } else {
            snprintf(path, sizeof(path), "%s", DUMP_REBOOT_TIMES_FILE);
        }

        len = silfp_util_file_get_size(path);
        if (len != sizeof(uint32_t)) {
            len = 0;
        }

        buf = malloc(len + sizeof(uint32_t));
        if (buf != NULL) {
            memset(buf, 0, len + sizeof(uint32_t));
            if (len > 0) {
                ret = silfp_util_file_load(dir, DUMP_REBOOT_TIMES_FILE, buf, len);
                if (ret > 0) {
                    m_reboot_times = *((uint32_t*)buf);
                }
            }
            m_reboot_times++;
            silfp_util_file_save(dir, DUMP_REBOOT_TIMES_FILE, (void *)&m_reboot_times, sizeof(uint32_t));
            free(buf);
        }
    }

    if (m_reboot_times == DUMP_REBOOT_TIMES_NO_GET) {
        return 0;
    }

    return m_reboot_times;
}

int32_t silfp_dump_name_get_save_index(void *buf, uint32_t len, const char *dir, uint32_t mode)
{
    uint32_t reboot_times = 0;

    if ((buf == NULL) || (len == 0)) {
        return -1;
    }

    memset(buf, 0, len);

    if (CHECK_FLAG_UNSET(mode, DUMP_NAME_MODE_REBOOT_TIMES_NONE)) {
        if (CHECK_FLAG_SET(mode, DUMP_NAME_MODE_PARENT_DIR_NULL)) {
            reboot_times = _dump_name_get_reboot_times(NULL);
        } else {
            reboot_times = _dump_name_get_reboot_times(dir);
        }
        snprintf(buf, len, "%04u-%04u", reboot_times, m_bmp_index++);
    } else {
        snprintf(buf, len, "%04u", m_bmp_index++);
    }

    return 0;
}

int32_t silfp_dump_name_get_save_name(char *name, uint32_t len, const char *index, dump_frame_t *frame, uint32_t mode)
{
    char datestr[64] = {0};
    uint64_t msec = 0;
    const char *perfix = NULL;
    uint8_t step = DUMP_DATA_STEP_INVALID;

    if ((name == NULL) || (len == 0) || (index == NULL)) {
        return -1;
    }

    if (frame != NULL) {
        msec = frame->timestamp;
        perfix = _dump_name_get_prefix(frame->type, frame->subtype);
        step = frame->step;
    } else {
        perfix = _dump_name_get_prefix(DUMP_IMG_OTHER, DUMP_IMG_SUBTYPE_OTHER);
    }

    if (!CHECK_FLAG_UNSET(mode, DUMP_NAME_MODE_TIMESTAMP_NONE)) {
        msec = 0;  // unneed timestamp
    } else if (DUMP_TIMESTAMP_IS_INVALID(msec)) {
        msec = 0; // unneed timestamp
    } else if (msec == 0) {
        msec = silfp_util_get_msec();
    }

    if (msec != 0) { // time format
        if (CHECK_FLAG_SET(mode, DUMP_NAME_MODE_TIMESTAMP_TIME_ONLY)) { // just time
            silfp_util_msec_to_date(msec, datestr, sizeof(datestr), MODE_GET_SEC_TYPE_TIME, MODE_GET_SEC_FORMAT_FILE_NAME);
        } else if (CHECK_FLAG_SET(mode, DUMP_NAME_MODE_TIMESTAMP_DATE_ONLY)) { // just date
            silfp_util_msec_to_date(msec, datestr, sizeof(datestr), MODE_GET_SEC_TYPE_DATE, MODE_GET_SEC_FORMAT_FILE_NAME);
        } else if (CHECK_FLAG_SET(mode, DUMP_NAME_MODE_TIMESTAMP_NO_YEAR)) { // date & time without year
            silfp_util_msec_to_date(msec, datestr, sizeof(datestr), MODE_GET_SEC_TYPE_NO_YEAR, MODE_GET_SEC_FORMAT_FILE_NAME);
        } else { // date & time
            silfp_util_msec_to_date(msec, datestr, sizeof(datestr), MODE_GET_SEC_TYPE_ALL, MODE_GET_SEC_FORMAT_FILE_NAME);
        }
    }

    if (msec == 0) {
        if (frame == NULL) {
            snprintf(name, len, "%s.%s", index, _dump_name_get_ext(DUMP_IMG_OTHER, DUMP_IMG_SUBTYPE_OTHER));
        } else if (DUMP_STEP_IS_INVALID(step)) {
            snprintf(name, len, "%s%s%s%s.%s", index, ITEM_SEPRATOR(perfix), ITEM_VALUE(perfix),
                     ITEM_VALUE(frame->suffix.data), _dump_name_get_ext(frame->type, frame->subtype));
        } else {
            snprintf(name, len, "%s%s%s-%02u%s.%s", index, ITEM_SEPRATOR(perfix), ITEM_VALUE(perfix), DUMP_STEP_VALUE(step),
                     ITEM_VALUE(frame->suffix.data), _dump_name_get_ext(frame->type, frame->subtype));
        }
    } else {
        if (frame == NULL) {
            snprintf(name, len, "%s-%s.%s", index, datestr, _dump_name_get_ext(DUMP_IMG_OTHER, DUMP_IMG_SUBTYPE_OTHER));
        } else if (DUMP_STEP_IS_INVALID(step)) {
            snprintf(name, len, "%s%s%s-%s%s.%s", index, ITEM_SEPRATOR(perfix), ITEM_VALUE(perfix), datestr,
                     ITEM_VALUE(frame->suffix.data), _dump_name_get_ext(frame->type, frame->subtype));
        } else {
            snprintf(name, len, "%s%s%s-%s-%02u%s.%s", index, ITEM_SEPRATOR(perfix), ITEM_VALUE(perfix), datestr, DUMP_STEP_VALUE(step),
                     ITEM_VALUE(frame->suffix.data), _dump_name_get_ext(frame->type, frame->subtype));
        }
    }

    return 0;
}

int32_t silfp_dump_name_get_save_name_simple(char *name, uint32_t len, const char *dir, uint32_t type, uint32_t subtype, uint32_t mode)
{
    char datestr[64] = {0};
    char index[64] = {0};
    uint64_t msec = 0;
    const char *perfix = NULL;

    if ((name == NULL) || (len == 0)) {
        return -1;
    }

    memset(name, 0, len);
    perfix = _dump_name_get_prefix(type, subtype);

    if (CHECK_FLAG_UNSET(mode, DUMP_NAME_MODE_TIMESTAMP_NONE)) { // if need timestamp
        msec = silfp_util_get_msec();
        if (CHECK_FLAG_SET(mode, DUMP_NAME_MODE_TIMESTAMP_TIME_ONLY)) { // just time
            silfp_util_msec_to_date(msec, datestr, sizeof(datestr), MODE_GET_SEC_TYPE_TIME, MODE_GET_SEC_FORMAT_FILE_NAME);
        } else if (CHECK_FLAG_SET(mode, DUMP_NAME_MODE_TIMESTAMP_DATE_ONLY)) { // just date
            silfp_util_msec_to_date(msec, datestr, sizeof(datestr), MODE_GET_SEC_TYPE_DATE, MODE_GET_SEC_FORMAT_FILE_NAME);
        } else if (CHECK_FLAG_SET(mode, DUMP_NAME_MODE_TIMESTAMP_NO_YEAR)) { // date & time without year
            silfp_util_msec_to_date(msec, datestr, sizeof(datestr), MODE_GET_SEC_TYPE_NO_YEAR, MODE_GET_SEC_FORMAT_FILE_NAME);
        } else { // date & time
            silfp_util_msec_to_date(msec, datestr, sizeof(datestr), MODE_GET_SEC_TYPE_ALL, MODE_GET_SEC_FORMAT_FILE_NAME);
        }
    }

    silfp_dump_name_get_save_index(index, sizeof(index), dir, mode);

    if (msec == 0) {
        snprintf(name, len, "%s%s%s.%s", index, ITEM_SEPRATOR(perfix), ITEM_VALUE(perfix), _dump_name_get_ext(type, subtype));
    } else {
        snprintf(name, len, "%s%s%s-%s.%s", index, ITEM_SEPRATOR(perfix), ITEM_VALUE(perfix), datestr, _dump_name_get_ext(type, subtype));
    }

    return 0;
}

#endif /* SIL_DUMP_IMAGE */