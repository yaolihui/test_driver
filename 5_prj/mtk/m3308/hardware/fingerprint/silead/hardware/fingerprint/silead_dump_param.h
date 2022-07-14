/******************************************************************************
 * @file   silead_dump_param.h
 * @brief  Contains dump image header file.
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

#ifndef __SILEAD_DUMP_PARAM_H__
#define __SILEAD_DUMP_PARAM_H__

#include "silead_dump_param_v1.h"
#include "silead_dump_type.h"

#define DUMP_VER_V1 0
#define DUMP_VER_V2 1

#define DUMP_NAME_MODE_PARENT_DIR_NULL      0x0001  // parent dir is null
#define DUMP_NAME_MODE_SUBDIR_NULL          0x0002  // no separate dir by img type
#define DUMP_NAME_MODE_TIMESTAMP_NONE       0x0004  // no date & time in file name
#define DUMP_NAME_MODE_TIMESTAMP_DATE_ONLY  0x0008  // just date in file name
#define DUMP_NAME_MODE_TIMESTAMP_TIME_ONLY  0x0010  // just time in file name
#define DUMP_NAME_MODE_TIMESTAMP_NO_YEAR    0x0020  // date & time without year
#define DUMP_NAME_MODE_REBOOT_TIMES_NONE    0x0040  // no reboot times in file name
#define DUMP_NAME_MODE_CAL_TIMESTAMP_NONE   0x0080  // no data & time in cal dump file name

#endif /* __SILEAD_DUMP_PARAM_H__ */