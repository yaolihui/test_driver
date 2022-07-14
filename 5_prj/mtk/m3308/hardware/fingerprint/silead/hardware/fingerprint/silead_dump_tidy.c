/******************************************************************************
 * @file   silead_dump_tidy.c
 * @brief  Contains dump to file functions.
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
 * Calvin Wang 2020/2/2   0.1.0      Init version
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#define FILE_TAG "dump_tidy"
#include "log/logmsg.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/stat.h>

#include "silead_const.h"
#include "silead_util.h"
#include "silead_util_ext.h"
#include "silead_dump_param.h"
#include "silead_dump_path.h"
#include "silead_dump.h"
#include "silead_dump_tidy.h"
#include "silead_util.h"
#include "silead_util_ext.h"

#ifdef SIL_DUMP_IMAGE

#define SUBDIR_ENROLL_UNFINISH_DEFAULT "unfinish"
#define SUBDIR_AUTH_FAIL_DEFAULT "fail"

#define ITEM_SEPRATOR(p) ((p == NULL) ? "" : "/")
#define ITEM_VALUE(p) ((p == NULL) ? "" : p)

#define DUMP_TIDY_PREFIX_ADD(type, item, fail_dir) { type, NUM_ELEMS(item), fail_dir, item, }

typedef struct _dump_subpath_prefix {
    uint32_t subtype;
    void *subdir;
} dump_subpath_prefix_t;

typedef struct _dump_tidy_prefix {
    uint32_t type;
    uint32_t count;
    void *fail_dir;
    dump_subpath_prefix_t *prefix_list;
} dump_tidy_prefix_t;

static dump_subpath_prefix_t m_dump_auth_prefix[] = {
    { DUMP_IMG_SUBTYPE_ORIG,        "work/raw/verify/%s" },
    { DUMP_IMG_SUBTYPE_RAW_SUCC,    "work/bin/verify/%s" },
    { DUMP_IMG_SUBTYPE_MRAW,        "work/raw/verify/%s/verify_mraw" },
};
static dump_subpath_prefix_t m_dump_enroll_prefix[] = {
    { DUMP_IMG_SUBTYPE_ORIG,        "work/raw/enroll/%s" },
    { DUMP_IMG_SUBTYPE_RAW_SUCC,    "work/bin/enroll/%s" },
    { DUMP_IMG_SUBTYPE_NEW,         "work/bin/enroll/%s/enroll_new" },
};

static dump_tidy_prefix_t m_dump_perfix[] = {
    DUMP_TIDY_PREFIX_ADD(DUMP_IMG_TYPE_AUTH,    m_dump_auth_prefix,     SUBDIR_AUTH_FAIL_DEFAULT),
    DUMP_TIDY_PREFIX_ADD(DUMP_IMG_TYPE_ENROLL,  m_dump_enroll_prefix,   SUBDIR_ENROLL_UNFINISH_DEFAULT),
};
int32_t __attribute__((weak)) silfp_cust_dump_tidy_data(int32_t type, uint32_t fid);
dump_subpath_prefix_t* __attribute__((weak)) silfp_cust_dump_tidy_get_prefix(uint32_t type, uint32_t *count);
const char* __attribute__((weak)) silfp_cust_dump_tidy_get_fail_dir_name(uint32_t type);

static int32_t _dump_tidy_get_save_path(char *path, uint32_t len, const char *subdir, uint32_t fid, const char *fail_dir, uint32_t mode)
{
    const char *rootdir = NULL;
    char subpath[64] = {0};
    char fid_str[32] = {0};

    if ((path == NULL) || (len == 0) || (subdir == NULL)) {
        return -1;
    }

    memset(path, 0, len);

    if (CHECK_FLAG_UNSET(mode, DUMP_NAME_MODE_PARENT_DIR_NULL)) {
        rootdir = silfp_dump_path_get();
    }

    if (fid == 0) {
        snprintf(subpath, sizeof(subpath), subdir, ITEM_VALUE(fail_dir));
    } else {
        snprintf(fid_str, sizeof(fid_str), "%010u", fid);
        snprintf(subpath, sizeof(subpath), subdir, fid_str);
    }
    snprintf(path, len, "%s%s%s", ITEM_VALUE(rootdir), ITEM_SEPRATOR(rootdir), subpath);

    return 0;
}

static void _dump_tidy_file_move(char *file_name, char *src_path, char *dst_path)
{
    int32_t ret = 0;
    char old_path[MAX_PATH_LEN] = {0};
    char new_path[MAX_PATH_LEN] = {0};

    if ((file_name == NULL) || (src_path == NULL) || (dst_path == NULL)) {
        LOG_MSG_ERROR("path invalid");
        return;
    }

    snprintf(old_path, sizeof(old_path), "%s/%s", src_path, file_name);
    snprintf(new_path, sizeof(new_path), "%s/%s", dst_path, file_name);

    ret = rename(old_path, new_path);
    if (ret) {
        LOG_MSG_DEBUG("fail to move %s -> %s", old_path, new_path);
    } else {
        LOG_MSG_DEBUG("move %s -> %s", old_path, new_path);
    }

    rmdir(src_path); // if empty dir, remove
}

static void _dump_tidy_data_move(char *src_path, char *dst_path)
{
    DIR *pDir = NULL;
    struct dirent *pEntry = NULL;

    if ((src_path == NULL) || (dst_path == NULL)) {
        return;
    }

    if (strcmp(src_path, dst_path) == 0) { // same dir, ignor
        LOG_MSG_VERBOSE("same path: %s", src_path);
        return;
    }

    if (access(src_path, F_OK) != 0) { // src dir not exist, ignor
        LOG_MSG_VERBOSE("no exist: %s", src_path);
        return;
    }

    if (access(dst_path, F_OK) != 0) { // dst dir not exist, create
        silfp_util_make_dirs(dst_path);
    }

    pDir = opendir(src_path);
    if (pDir == NULL) {
        LOG_MSG_ERROR("opendir fail: %s", src_path);
        return;
    }

    while((pEntry = readdir(pDir)) != NULL) {
        if (strcmp(pEntry->d_name, ".") == 0 || strcmp(pEntry->d_name, "..") == 0) {
            continue;
        } else if (silfp_util_dir_get_type(src_path, pEntry) == 8) { // file
            _dump_tidy_file_move(pEntry->d_name, src_path, dst_path);
        }
    }

    closedir(pDir);
}

static const char* _dump_tidy_get_fail_dir_name(uint32_t type)
{
    int32_t i = 0;
    int32_t prefix_count = 0;
    const char *fail_dir_name = NULL;

    if (silfp_cust_dump_tidy_get_fail_dir_name != NULL) {
        fail_dir_name = silfp_cust_dump_tidy_get_fail_dir_name(type);
        if (fail_dir_name != NULL) {
            return fail_dir_name;
        }
    }

    prefix_count = NUM_ELEMS(m_dump_perfix);
    for (i = 0; i < prefix_count; i++) {
        if (m_dump_perfix[i].type == type) {
            fail_dir_name = m_dump_perfix[i].fail_dir;
            break;
        }
    }

    if (fail_dir_name == NULL) {
        fail_dir_name = "unknow";
    }

    return fail_dir_name;
}

static void _dump_tidy_data(uint32_t fid, uint32_t type, uint32_t mode)
{
    int32_t ret = 0;
    int32_t i = 0;
    char src_path[MAX_PATH_LEN] = {0};
    char dst_path[MAX_PATH_LEN] = {0};

    dump_subpath_prefix_t *prefix_list = NULL;
    int32_t prefix_count = 0;
    const char *fail_dir_name = NULL;

    if (silfp_cust_dump_tidy_get_prefix != NULL) {
        prefix_list = silfp_cust_dump_tidy_get_prefix(type, (uint32_t *)&prefix_count);
    }
    if ((prefix_list == NULL) || (prefix_count <= 0)) {
        prefix_count = NUM_ELEMS(m_dump_perfix);
        for (i = 0; i < prefix_count; i++) {
            if (m_dump_perfix[i].type == type) {
                prefix_count = m_dump_perfix[i].count;
                prefix_list = m_dump_perfix[i].prefix_list;
                break;
            }
        }
    }
    fail_dir_name = _dump_tidy_get_fail_dir_name(type);

    for (i = 0; i < prefix_count; i++) {
        if ((prefix_list != NULL) && (prefix_list[i].subdir != NULL)) {
            ret = silfp_dump_path_get_save_path(src_path, sizeof(src_path), type, prefix_list[i].subtype, mode);
            if (ret < 0) {
                continue;
            }
            ret = _dump_tidy_get_save_path(dst_path, sizeof(dst_path), prefix_list[i].subdir, fid, fail_dir_name, mode);
            if (ret < 0) {
                continue;
            }
            _dump_tidy_data_move(src_path, dst_path);
        }
    }
}

void silfp_dump_tidy_data(int32_t type, uint32_t fid)
{
    int32_t ret = 0;
    uint32_t mode = 0;

    if (silfp_cust_dump_tidy_data != NULL) {
        ret = silfp_cust_dump_tidy_data(type, fid);
        if (ret >= 0) {
            return;
        }
    }

    mode = silfp_dump_get_name_mode();
    if (CHECK_FLAG_SET(mode, DUMP_NAME_MODE_SUBDIR_NULL)) {
        LOG_MSG_DEBUG("unsupport tidy");
        return;
    }

    _dump_tidy_data(fid, type, mode);
}
#else
void silfp_dump_tidy_data(int32_t type, uint32_t fid)
{
    UNUSED(type);
    UNUSED(fid);
}
#endif /* SIL_DUMP_IMAGE */