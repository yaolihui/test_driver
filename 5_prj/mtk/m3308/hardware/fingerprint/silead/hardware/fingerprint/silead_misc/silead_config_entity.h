/******************************************************************************
 * @file   silead_config_dbg.h
 * @brief  Contains dump config functions header file.
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
 * calvin wang  2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_CONFIG_XML_ENTITY_H__
#define __SILEAD_CONFIG_XML_ENTITY_H__

__WEAK void silfp_ree_cfg_dbg_set_path(const void *path, uint32_t len);
__WEAK void silfp_ree_cfg_dbg(uint32_t chipid, uint32_t subid, uint32_t vid);
__WEAK int32_t silfp_ree_cfg_update(uint32_t chipid, uint32_t subid, uint32_t vid);

__WEAK void silfp_ree_xml_set_version(uint32_t ver);

#define silfp_ree_cfg_dbg_set_path_entity(...) FUNC_INVOKE(silfp_ree_cfg_dbg_set_path, ##__VA_ARGS__)
#define silfp_ree_cfg_dbg_entity(...) FUNC_INVOKE(silfp_ree_cfg_dbg, ##__VA_ARGS__)
#define silfp_ree_cfg_update_entity(...) FUNC_INVOKE(silfp_ree_cfg_update, ##__VA_ARGS__)

#define silfp_ree_xml_set_version_entity(...) FUNC_INVOKE(silfp_ree_xml_set_version, ##__VA_ARGS__)

#endif /* __SILEAD_CONFIG_XML_ENTITY_H__ */