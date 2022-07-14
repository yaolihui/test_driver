/******************************************************************************
 * @file   silead_xml_util.c
 * @brief  Contains XML parse functions.
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
 * <author>      <date>      <version>     <desc>
 * Calvin Wang  2021/2/3    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "xml_util"
#include "silead_logmsg.h"

#include "silead_xml_util.h"

typedef struct _xml_upd_list {
    struct _xml_upd_list *prev;
    struct _xml_upd_list *next;
    xml_upd_item_t *item;
} xml_upd_list_t;

static xml_upd_list_t *m_xml_upd_list_header = NULL;
static uint8_t m_xml_upd_count = 0;

static xml_upd_item_t* _xml_util_item_get(void)
{
    xml_upd_list_t *list_item = NULL;
    xml_upd_item_t *item = NULL;

    if (m_xml_upd_list_header != NULL) {
        list_item = m_xml_upd_list_header->prev;
    }

    if (list_item != NULL) {
        item = list_item->item;
        if (list_item == m_xml_upd_list_header) { // just one item
            m_xml_upd_list_header = NULL;
        } else {
            list_item->prev->next = list_item->next;
            list_item->next->prev = list_item->prev;
        }
        free(list_item);
        m_xml_upd_count--;
    }
    LOG_MSG_VERBOSE("item remain: %d", m_xml_upd_count);

    return item;
}

void silfp_ree_xml_util_item_clear(xml_upd_item_t *item)
{
    if (item != NULL) {
        if (item->buf != NULL) {
            free(item->buf);
        }
        free(item);
    }
}

void silfp_ree_xml_util_clear_all(void)
{
    xml_upd_item_t *item = NULL;

    item = _xml_util_item_get();
    while (item != NULL) {
        silfp_ree_xml_util_item_clear(item);
        item = _xml_util_item_get();
    }

    LOG_MSG_VERBOSE("frame count remain: %d", m_xml_upd_count);
}

static int32_t _xml_util_item_add(xml_upd_item_t *item)
{
    xml_upd_list_t *list_item = NULL;

    if (item == NULL) {
        LOG_MSG_ERROR("item data invalid");
        return -1;
    }

    list_item = (xml_upd_list_t *)malloc(sizeof(xml_upd_list_t));
    if (list_item == NULL) {
        LOG_MSG_ERROR("malloc (%d) fail", (int32_t)sizeof(xml_upd_list_t));
        return -1;
    }

    memset(list_item, 0, sizeof(xml_upd_list_t));
    list_item->item = item;
    if (m_xml_upd_list_header == NULL) {
        m_xml_upd_list_header = list_item;
        m_xml_upd_list_header->prev = m_xml_upd_list_header;
        m_xml_upd_list_header->next = m_xml_upd_list_header;
    } else {
        list_item->next = m_xml_upd_list_header;
        list_item->prev = m_xml_upd_list_header->prev;
        m_xml_upd_list_header->prev->next = list_item;
        m_xml_upd_list_header->prev = list_item;
        m_xml_upd_list_header = list_item;
    }

    m_xml_upd_count++;

    LOG_MSG_VERBOSE("add end frame count: %d", m_xml_upd_count);

    return 0;
}

int32_t silfp_ree_xml_util_item_add(uint32_t id, void *buf, uint32_t len)
{
    int32_t ret = 0;
    xml_upd_item_t *item = NULL;

    if ((buf == NULL) || (len == 0)) {
        return -1;
    }

    LOG_MSG_VERBOSE("add new frame len: %d", len);
    item = malloc(sizeof(xml_upd_item_t));
    if (item == NULL) {
        LOG_MSG_ERROR("malloc (%d) fail", (int32_t)sizeof(xml_upd_item_t));
        return -1;
    }

    memset(item, 0, sizeof(xml_upd_item_t));
    item->id = id;
    item->buf = buf;
    item->len = len;

    ret = _xml_util_item_add(item);
    if (ret < 0) {
        free(item);
    }

    return ret;
}

xml_upd_item_t* silfp_ree_xml_util_get_item(void)
{
    return _xml_util_item_get();
}