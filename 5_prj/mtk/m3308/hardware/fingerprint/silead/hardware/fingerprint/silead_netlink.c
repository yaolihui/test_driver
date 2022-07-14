/******************************************************************************
 * @file   silead_netlink.c
 * @brief  Contains netlink communication functions.
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
 * Luke Ma     2018/4/2   0.1.0      Init version
 * David Wang  2018/5/28  0.1.1      Support poll/read if netlink id invalid
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#define FILE_TAG "silead_nl"
#include "log/logmsg.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <poll.h>

#include "silead_netlink.h"
#include "silead_msg.h"

static pthread_t nl_rcv_thread;
static int32_t m_socket_id = 0;
static uint8_t m_nl_quit = 0;

static screen_cb m_screen_cb = NULL;
static void *m_screen_cb_param = NULL;

static int32_t m_receive_irq_by_kernel = 0;

#define MAX_NL_ID    31
#define MAXEVENTS    2
#define INFINITETIME -1

static void _relay_msg(unsigned char msg)
{
    switch(msg) {
    case SIFP_NETLINK_START: {
        LOG_MSG_VERBOSE("recv START msg");
        break;
    }
    case SIFP_NETLINK_IRQ: {
        LOG_MSG_DEBUG("recv IRQ msg");
        if (m_receive_irq_by_kernel) {
            silfp_msg_send(SIFP_MSG_IRQ);
        }
        break;
    }
    case SIFP_NETLINK_TOUCH_DOWN: {
        LOG_MSG_DEBUG("recv touch down msg");
        if (m_receive_irq_by_kernel) {
            silfp_msg_send(SIFP_MSG_DOWN);
        }
        break;
    }
    case SIFP_NETLINK_TOUCH_UP: {
        LOG_MSG_DEBUG("recv touch up msg");
        if (m_receive_irq_by_kernel) {
            silfp_msg_send(SIFP_MSG_UP);
        }
        break;
    }
    case SIFP_NETLINK_SCREEN_ON: {
        LOG_MSG_DEBUG("recv SCREEN ON msg");
        if (m_screen_cb != NULL) {
            m_screen_cb(1, m_screen_cb_param);
        }
        break;
    }
    case SIFP_NETLINK_SCREEN_OFF: {
        LOG_MSG_DEBUG("recv SCREEN OFF msg");
        if (m_screen_cb != NULL) {
            m_screen_cb(0, m_screen_cb_param);
        }
        break;
    }
    case SIFP_NETLINK_DISCONNECT: {
        LOG_MSG_VERBOSE("recv DISCONNECT msg");
        m_nl_quit = 1;
        break;
    }
    default:
        LOG_MSG_ERROR( "Unknown netlink msg %d", msg);
        break;
    }
}

static void *_thr_netlink_recv(void *handle)
{
    int32_t ret = 0;
    struct nlmsghdr *nlh = NULL;
    struct sockaddr_nl local, dest;
    struct msghdr msg;
    struct iovec iov;
    unsigned char value;

    do {
        m_socket_id = socket(AF_NETLINK, SOCK_RAW, (uint32_t)(unsigned long)handle);
        if (m_socket_id < 0) {
            LOG_MSG_ERROR("socket failed (%d:%s)", errno, strerror(errno));
            break;
        }

        memset(&local, 0, sizeof(struct sockaddr_nl));
        local.nl_family = AF_NETLINK;
        local.nl_pid = getpid(); /*local process id*/
        local.nl_groups = 0;

        ret = bind(m_socket_id, (struct sockaddr*)&local, sizeof(struct sockaddr_nl));
        if (ret != 0) {
            LOG_MSG_ERROR("bind failed (%d:%s)", errno, strerror(errno));
            break;
        }

        /* send init message */
        memset(&dest, 0, sizeof(struct sockaddr_nl));
        dest.nl_family = AF_NETLINK;
        dest.nl_pid = 0; /*destination is kernel so set to 0*/
        dest.nl_groups = 0;

        nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_NL_MSG_LEN));
        if (NULL == nlh) {
            LOG_MSG_ERROR("Alloc NLH fail");
            break;
        }
        memset(nlh, 0, NLMSG_SPACE(MAX_NL_MSG_LEN));
        nlh->nlmsg_len = NLMSG_SPACE(MAX_NL_MSG_LEN);
        nlh->nlmsg_pid = getpid();
        nlh->nlmsg_flags = 0;
        strcpy(NLMSG_DATA(nlh), "FP");

        iov.iov_base = (void*) nlh;
        iov.iov_len = nlh->nlmsg_len;

        memset(&msg, 0, sizeof(struct msghdr));
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_name = (void*) &dest;
        msg.msg_namelen = sizeof(struct sockaddr_nl);

        // send pid to kernel
        if (sendmsg(m_socket_id, &msg, 0) < 0) {
            break;
        }

        LOG_MSG_VERBOSE("Netlink recv thread running %d .....", getpid());

        memset(nlh, 0, NLMSG_SPACE(MAX_NL_MSG_LEN));
        while (!m_nl_quit) {
            ret = recvmsg(m_socket_id, &msg, 0);
            if (ret <= 0) {
                LOG_MSG_ERROR( "recvmsg failed, ret %d", ret);
                continue;
            }

            value = *((char *) NLMSG_DATA(nlh));
            _relay_msg(value);
        }
    } while(0);

    if (nlh) {
        free(nlh);
        nlh = NULL;
    }

    return NULL;
}

static void *_thr_read(void *handle)
{
    int32_t ret = 0;
    struct pollfd fds;
    unsigned char value = 0;

    do {
        if (NULL == handle) {
            LOG_MSG_ERROR("invalid drv handl!");
            break;
        }

        memset(&fds, 0, sizeof(struct pollfd));
        fds.fd = (int32_t)(long)handle;
        fds.events = POLLIN;

        LOG_MSG_VERBOSE("Read thread running %d .....", getpid());

        while (!m_nl_quit) {
            ret = poll(&fds, sizeof(fds)/sizeof(struct pollfd), INFINITETIME);
            if (ret <= 0) {
                LOG_MSG_ERROR( "poll failed, ret %d", ret);
                continue;
            }

            ret = read(fds.fd, &value, 1);
            if (ret <= 0) {
                LOG_MSG_ERROR( "read failed, ret %d", ret);
                continue;
            }
            _relay_msg(value);
        }
    } while(0);

    return NULL;
}

static void *_thr_eread(void *handle)
{
    int32_t ret = 0;
    int32_t epfd = -1;
    struct epoll_event ev, events[MAXEVENTS];
    unsigned char value = 0;

    do {
        if (NULL == handle) {
            LOG_MSG_ERROR("invalid drv handl!");
            break;
        }

        epfd = epoll_create(1);
        ev.data.fd = (int32_t)(long)handle;
        ev.events  = EPOLLIN;
        epoll_ctl(epfd, EPOLL_CTL_ADD, ev.data.fd, &ev);

        LOG_MSG_VERBOSE("eRead thread running %d .....", getpid());

        while (!m_nl_quit) {
            ret = epoll_wait(epfd, events, MAXEVENTS, INFINITETIME);
            if (ret <= 0) {
                LOG_MSG_ERROR( "epoll failed, ret %d", ret);
                continue;
            }

            ret = read(events[0].data.fd, &value, 1);
            if (ret <= 0) {
                LOG_MSG_ERROR( "read failed, ret %d", ret);
                continue;
            }
            _relay_msg(value);
        }
    } while(0);

    return NULL;
}

int32_t silfp_nl_init(uint8_t nl_id, int32_t fd)
{
    LOG_MSG_VERBOSE("init: nl_id = %d, fd = %d", nl_id, fd);

    m_nl_quit = 0;
    m_screen_cb = NULL;
    m_screen_cb_param = NULL;
    m_receive_irq_by_kernel = 0;

    if (nl_id > MAX_NL_ID) {
        if (pthread_create(&nl_rcv_thread, NULL, _thr_eread, (void *)(unsigned long)(fd)) != 0) {
            LOG_MSG_ERROR( "pthread_create failed");
        }
    } else if (nl_id > 0) {
        if (pthread_create(&nl_rcv_thread, NULL, _thr_netlink_recv, (void *)(unsigned long)(nl_id)) != 0) {
            LOG_MSG_ERROR( "pthread_create failed");
        }
    } else {
        if (pthread_create(&nl_rcv_thread, NULL, _thr_read, (void *)(unsigned long)(fd)) != 0) {
            LOG_MSG_ERROR( "pthread_create failed");
        }
    }
    return 0;
}

int32_t silfp_nl_deinit(void)
{
    if (pthread_join(nl_rcv_thread, NULL) != 0 ) {
        LOG_MSG_ERROR( "pthread_join failed");
    }

    if (m_socket_id > 0) {
        close(m_socket_id);
    }

    m_screen_cb = NULL;
    m_screen_cb_param = NULL;

    m_receive_irq_by_kernel = 0;

    LOG_MSG_VERBOSE("deinit");
    return 0;
}

int32_t silfp_nl_set_screen_cb(screen_cb listen, void *param)
{
    m_screen_cb = listen;
    m_screen_cb_param = param;

    return 0;
}

int32_t silfp_nl_set_finger_status_mode(int32_t mode)
{
    if ((mode == SIFP_FINGER_STATUS_IRQ) || (mode == SIFP_FINGER_STATUS_TP)) {
        m_receive_irq_by_kernel = 1;
    } else {
        m_receive_irq_by_kernel = 0;
    }
    return 0;
}
