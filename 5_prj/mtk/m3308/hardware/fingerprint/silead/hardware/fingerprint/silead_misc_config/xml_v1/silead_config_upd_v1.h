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

#ifndef __SILEAD_CONFIG_UPD_V1_H__
#define __SILEAD_CONFIG_UPD_V1_H__

#define GEN_CFG_UPD_V1_ID_1(f, a, b, c, e) cfg_upd_v1_id_##f##_##a##_##e
#define GEN_CFG_UPD_V1_ID_2(f, a, b, c, e) cfg_upd_v1_id_##f##_##a##_##b##_##e
#define GEN_CFG_UPD_V1_ID_3(f, a, b, c, e) cfg_upd_v1_id_##f##_##a##_##b##_##c##_##e

#define PARAM_UPD_ID_FROM(f, n, a, b, e, v)     GEN_CFG_UPD_V1_ID_##n(f, a, b, NULL, e) = ((((v) << 16) & 0xFF0000) | 0x7F000000),
#define PARAM_UPD_ITEM(f, n, a, b, c, e, p, t)  GEN_CFG_UPD_V1_ID_##n(f, a, b, c, e),

enum cfg_upd_v1_id {
#include "silead_config_upd_param_v1.h"
#include "silead_config_upd_config_v1.h"
};

#undef PARAM_UPD_ID_FROM
#undef PARAM_UPD_ITEM

#endif /* __SILEAD_CONFIG_UPD_H__ */