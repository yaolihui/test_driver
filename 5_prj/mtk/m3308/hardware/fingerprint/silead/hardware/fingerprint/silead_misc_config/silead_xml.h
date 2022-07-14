/******************************************************************************
 * @file   silead_xml.h
 * @brief  Contains XML parse functions header file.
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
 * Martin Wu  2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_CONFIG_XML_H__
#define __SILEAD_CONFIG_XML_H__

#include "silead_xml_util.h"

void silfp_ree_xml_set_path(const void *path, uint32_t len);
int32_t silfp_ree_xml_get_sysparams(uint32_t cid, uint32_t sid, uint32_t vid);
int32_t silfp_ree_xml_dump(const char *dir, const char *module);

void silfp_ree_xml_set_version(uint32_t ver);

#endif /* __SILEAD_CONFIG_XML_H__ */