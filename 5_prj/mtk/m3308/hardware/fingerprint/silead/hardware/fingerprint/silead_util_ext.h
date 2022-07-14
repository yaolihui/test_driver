/******************************************************************************
 * @file   silead_util_ext.h
 * @brief  Contains fingerprint utilities functions header file.
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
 * calvin wang 2018/1/2   0.1.0      Init version
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#ifndef __SILEAD_UTIL_EXT_H__
#define __SILEAD_UTIL_EXT_H__

#ifdef HOST_OS_WINDOWS
#define DT_DIR 4
#define DT_REG 8
#endif

#include <dirent.h>

int32_t silfp_util_make_dir(const char *path);
int32_t silfp_util_dir_get_type(char *path, struct dirent *pEntry);

#endif /* __SILEAD_UTIL_EXT_H__ */