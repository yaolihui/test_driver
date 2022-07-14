/******************************************************************************
 * @file   silead_config_algo_dump_util.h
 * @brief  Contains dump config header file.
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
 * <author>       <date>     <version>     <desc>
 * Calvin Wang  2021/3/1    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_CFG_ALGO_DUMP_UTIL_H__
#define __SILEAD_CFG_ALGO_DUMP_UTIL_H__

#define ALGO_DUMP_ITEM_MAGIC                0xACFD0511
#define ALGO_DUMP_ITEM_TYPE_STR             0xACFD0000
#define ALGO_DUMP_ITEM_TYPE_PARAM           0xACFD1000
#define ALGO_DUMP_ITEM_TYPE_PARAM_FLOAT     0xACFD1001
#define ALGO_DUMP_ITEM_TYPE_PARAM_DOUBLE    0xACFD1002
#define ALGO_DUMP_ITEM_TYPE_PARAM_INT       0xACFD1003

int32_t silfp_algo_cfg_dump_util_save(int32_t fd, void *buf, uint32_t size);

#endif /* __SILEAD_CFG_ALGO_DUMP_UTIL_H__ */