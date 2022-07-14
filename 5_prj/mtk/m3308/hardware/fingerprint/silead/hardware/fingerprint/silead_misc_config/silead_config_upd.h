/******************************************************************************
 * @file   silead_config_upd.h
 * @brief  Contains Chip configurations header file.
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
 * calvin wang  2018/12/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_CONFIG_UPD_H__
#define __SILEAD_CONFIG_UPD_H__

#define CFG_UPD_MAGIC 0x511EADCF
#define ALGO_CFG_UPD_MAGIC 0x511EADAF
#define ALGO_CFG_APPEND_MAGIC 0x511EADAA

#define PARAM_UPD_ITEM_DEV_VER_INDEX    0
#define PARAM_UPD_ITEM_MASK_INDEX       1
#define PARAM_UPD_ITEM_INDEX_START      2

#define CFG_UPD_ID_ALGO_TYPE 0xFFFFFFFF

#define GEN_CFG_UPD_ID_1(f, a, b, c, e) cfg_upd_id_##f##_##a##_##e
#define GEN_CFG_UPD_ID_2(f, a, b, c, e) cfg_upd_id_##f##_##a##_##b##_##e
#define GEN_CFG_UPD_ID_3(f, a, b, c, e) cfg_upd_id_##f##_##a##_##b##_##c##_##e

#define PARAM_UPD_ID_FROM(f, n, a, b, e, v)     GEN_CFG_UPD_ID_##n(f, a, b, NULL, e) = (((v) << 23) & 0x7F800000),
#define PARAM_UPD_ITEM(f, n, a, b, c, e, p, t)  GEN_CFG_UPD_ID_##n(f, a, b, c, e),
#define PARAM_UPD_ITEM_EXT(f, n, a, b, c, e, p, t, ver)  GEN_CFG_UPD_ID_##n(f, a, b, c, e),

enum cfg_upd_type {
    CFG_UPD_TYPE_NORMAL = 0,
    CFG_UPD_TYPE_ALGO,
};

enum cfg_upd_id {
#include "silead_config_upd_param.h"
#include "silead_config_upd_config.h"
#include "silead_config_upd_algo.h"
};

#undef PARAM_UPD_ID_FROM
#undef PARAM_UPD_ITEM
#undef PARAM_UPD_ITEM_EXT

#define GEN_CFG_ALGO_PARAM_ITEM_UPD_ID(a) algo_cfg_param_item_##a
#define ALGO_PARAM_UPD_ITEM(a, item, t)  GEN_CFG_ALGO_PARAM_ITEM_UPD_ID(a),
enum {
#include "silead_config_upd_algo_param.h"
};
#undef ALGO_PARAM_UPD_ITEM

typedef struct __PACKED _dev_ver {
    uint32_t id;
    uint32_t sid;
    uint32_t vid;
} dev_ver_t;

typedef struct __PACKED _reg_cfg {
    uint32_t addr;
    uint32_t val;
} reg_cfg_t;

#endif /* __SILEAD_CONFIG_UPD_H__ */