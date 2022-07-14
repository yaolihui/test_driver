/******************************************************************************
 * @file   silead_xml_item.h
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

#ifndef __SILEAD_CONFIG_ITEM_H__
#define __SILEAD_CONFIG_ITEM_H__

#include "silead_config_upd.h"

#define PARAM_UPD_ITEM_1(f, a, b, c, e, p, t)   {#a, NULL, NULL, #e, GEN_CFG_UPD_ID_1(f, a, b, c, e)},
#define PARAM_UPD_ITEM_2(f, a, b, c, e, p, t)   {#a, #b, NULL, #e, GEN_CFG_UPD_ID_2(f, a, b, c, e)},
#define PARAM_UPD_ITEM_3(f, a, b, c, e, p, t)   {#a, #b, #c, #e, GEN_CFG_UPD_ID_3(f, a, b, c, e)},

#define PARAM_UPD_ID_FROM(f, n, a, b, e, v)
#define PARAM_UPD_ITEM(f, n, a, b, c, e, p, t) PARAM_UPD_ITEM_##n(f, a, b, c, e, p, t)
#define PARAM_UPD_ITEM_EXT(f, n, a, b, c, e, p, t, ver) PARAM_UPD_ITEM_##n(f, a, b, c, e, p, t)

xml_parse_item_t m_param_parse_item_default[] = {
#include "silead_config_upd_param.h"
};
xml_parse_item_t m_config_parse_item_default[] = {
#include "silead_config_upd_config.h"
};
xml_parse_item_t m_algo_parse_item_default[] = {
#include "silead_config_upd_algo.h"
};

#undef PARAM_UPD_ID_FROM
#undef PARAM_UPD_ITEM
#undef PARAM_UPD_ITEM_EXT

#define ALGO_PARAM_UPD_ITEM(a, item, t)  {NULL, NULL, NULL, #a, GEN_CFG_ALGO_PARAM_ITEM_UPD_ID(a)},
xml_parse_item_t m_algo_param_item[] = {
#include "silead_config_upd_algo_param.h"
};
#undef ALGO_PARAM_UPD_ITEM

#endif /* __SILEAD_CONFIG_ITEM_H__ */