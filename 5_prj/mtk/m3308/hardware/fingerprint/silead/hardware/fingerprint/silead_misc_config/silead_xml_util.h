/******************************************************************************
 * @file   silead_xml_util.h
 * @brief  Contains XML parse header file.
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
 * Calvin Wang  2019/12/29    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_XML_UTIL_H__
#define __SILEAD_XML_UTIL_H__

typedef struct {
    uint32_t id;
    uint32_t len;
    void *buf;
} xml_upd_item_t;

void silfp_ree_xml_util_item_clear(xml_upd_item_t *item);
void silfp_ree_xml_util_clear_all(void);
int32_t silfp_ree_xml_util_item_add(uint32_t id, void *buf, uint32_t len);
xml_upd_item_t* silfp_ree_xml_util_get_item(void);

#endif /* __SILEAD_XML_UTIL_H__ */