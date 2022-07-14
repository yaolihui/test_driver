/******************************************************************************
 * @file   silead_dump_path.h
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
 * Calvin Wang 2020/03/10 0.1.0      Init version
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#ifndef __SILEAD_DUMP_PATH_H__
#define __SILEAD_DUMP_PATH_H__

void silfp_dump_path_set(const void *path, uint32_t len);
const char *silfp_dump_path_get(void);

int32_t silfp_dump_path_get_save_path(char *path, uint32_t len, uint32_t type, uint32_t subtype, uint32_t mode);

#endif /* __SILEAD_DUMP_PATH_H__ */