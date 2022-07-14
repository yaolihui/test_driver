/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

#include "fpc_hal_ext_navigation.h"
#include "fpc_hal_navigation.h"
#include "fpc_log.h"
#include "fpc_worker.h"
#include "fpc_irq_device.h"
#include "fpc_tee_nav.h"
#include "fpc_types.h"
#include "fpc_nav_types.h"
#include "fpc_tee.h"
#include "fpc_hal_input_device.h"
#include "fpc_hal_sense_touch.h"
#include "fpc_hal_sense_touch_types.h"
#include "fpc_hal_private.h"

#ifndef FPC_NAV_DEBUG_SAVE_FILE_PATH
#define FPC_NAV_DEBUG_SAVE_FILE_PATH "data/fpc/"
#endif

typedef struct {
    fpc_navigation_t navigation;
    pthread_mutex_t mutex;
    pthread_mutex_t cancel_mutex;
    fpc_tee_t *tee;
    fpc_irq_t *irq;
    fpc_worker_t *worker_thread;
    bool enabled;
    bool paused;
    bool cancel;
    fpc_nav_config_t config;
    bool configured;
    const fpc_sense_touch_config_t *st_config;
    bool sensetouch_enabled;
    uint32_t debug_buffer_size;
    uint8_t *debug_buffer;
} nav_module_t;

static uint64_t current_u_time(void)
{
    struct timeval t1;
    gettimeofday(&t1, NULL);
    return (t1.tv_sec * 1000 * 1000) + (t1.tv_usec);
}

static int nav_debug_check_dir(void)
{
    struct stat st;
    memset(&st, 0, sizeof(st));
    int status = stat(FPC_NAV_DEBUG_SAVE_FILE_PATH, &st);
    if (status) {
        LOGE("%s %s not available %d", __func__, FPC_NAV_DEBUG_SAVE_FILE_PATH, errno);
    }
    return status;
}

static int nav_debug_save_data(uint8_t *nav_data, uint32_t nav_data_size)
{
    char curr_time[] = "00-00-00-00-00-00.000000";
    char fname[] = FPC_NAV_DEBUG_SAVE_FILE_PATH "fpc_" "00-00-00-00-00-00.000000" ".nav_dat";
    struct timeval t1;
    struct tm *tm = NULL;

    if (gettimeofday(&t1, NULL)) {
        LOGE("%s unable to get wall time, skip saving debug buffer %d", __func__, errno);
        return -1;
    }
    tm = localtime(&t1.tv_sec);
    if (NULL == tm) {
        LOGE("%s unable to get local time, skip saving debug buffer", __func__);
        return -1;
    }
    strftime(curr_time, sizeof(curr_time), "%y-%m-%d-%H-%M-%S", tm);
    snprintf(fname, sizeof(fname), FPC_NAV_DEBUG_SAVE_FILE_PATH "fpc_%s.%06ld.nav_dat", curr_time, t1.tv_usec);
    FILE *debug_fd = fopen(fname, "wb");
    if (!debug_fd) {
        LOGE("%s unable to open file for writing %d", __func__, errno);
        return -1;
    }

    size_t bytes_written = fwrite(nav_data, 1, nav_data_size, debug_fd);
    LOGD("%s created new file %s, OK(1=yes,0=no): %d", __func__, fname,
         (bytes_written == nav_data_size));
    fclose(debug_fd);

    return (bytes_written == nav_data_size) ? 0 : -1;
}

static void nav_debug_retrieve_and_save_data(nav_module_t *module)
{
    int status = 0;
    LOGD("%s retrieving and saving debug data for nav", __func__);
    if (nav_debug_check_dir() && !module->debug_buffer) {
        goto exit;
    }

    memset(module->debug_buffer, 0, module->debug_buffer_size);

    status = fpc_tee_nav_get_debug_buffer(module->tee,
                                          module->debug_buffer,
                                          &module->debug_buffer_size);
    if (status) {
        LOGE("%s failed to retrieve debug buffer: %d", __func__, status);
        goto exit;
    }

    status = nav_debug_save_data(module->debug_buffer, module->debug_buffer_size);
    if (status) {
        LOGE("%s failed to save debug buffer: %d", __func__, status);
    }

exit:
    return;
}

static void nav_loop(void *data)
{
    LOGD("%s", __func__);
    /* These are used to block colliding events from being reported,
       Such as force_press and nav_click */
    bool block_nav_events = false;
    bool block_st_events = false;
    int32_t prev_force = 0;
    bool force_button_down = false;
    const uint32_t frame_rate_limit = 150;
    const uint64_t frame_time = 1000 * 1000 / frame_rate_limit;
    uint64_t time = 0;
    uint64_t frame_delta;

    nav_module_t *module = (nav_module_t *) data;

    int status = 0;
    if (module->configured) {
        status = fpc_tee_nav_set_config(module->tee, &module->config);
        if (status) {
            LOGE("%s - fpc_tee_nav_set_config failed", __func__);
            return;
        }
    }

    status = fpc_tee_nav_init(module->tee);
    if (status) {
        LOGE("%s - fpc_tee_nav_init failed", __func__);
        return;
    }

    if (fpc_tee_nav_get_config(module->tee, &module->config)) {
        LOGE("%s fpc_tee_nav_get_config failed", __func__);
        goto out;
    }

    if (fpc_tee_navigation_debug_enabled(module->tee)) {
        if (module->debug_buffer_size == 0) {
            status = fpc_tee_nav_get_debug_buffer_size(module->tee, &module->debug_buffer_size);
            if (status) {
                LOGE("%s error on get debug buffer size, running without debug function", __func__);
                module->debug_buffer_size = 0;
            }
        }

        if (module->debug_buffer_size != 0) {
            if (module->debug_buffer_size > 1020*1024) {
                LOGD("%s debug buffer size: %" PRIu32 ", truncate to 1M", __func__, module->debug_buffer_size);
                module->debug_buffer_size = 1020*1024;
            }
            module->debug_buffer = (uint8_t *) calloc(sizeof(uint8_t), module->debug_buffer_size);
            if (!module->debug_buffer) {
                module->debug_buffer_size = 0;
                LOGE("%s failed to allocate memory, running without debug function", __func__);
            }
        }
    }

    for (;;) {
        pthread_mutex_lock(&module->cancel_mutex);
        bool cancel = module->cancel;
        pthread_mutex_unlock(&module->cancel_mutex);

        if (cancel) {
            status = -FPC_ERROR_CANCELLED;
            goto out;
        }

        fpc_nav_data_t nav;
        status = fpc_tee_nav_poll_data(module->tee, &nav);
        if (status) {
            goto out;
        }

        if (!block_nav_events && (nav.nav_event != FPC_NAV_EVENT_NONE)) {
            LOGD("%s Got navigation event: %d, dx=%d, dy=%d", __func__,
                 nav.nav_event, nav.vec.dx, nav.vec.dy);
            if (fpc_tee_navigation_debug_enabled(module->tee) && module->debug_buffer) {
                nav_debug_retrieve_and_save_data(module);
            } else if (!fpc_tee_navigation_debug_enabled(module->tee)) {
                LOGD("%s Navigation debug not enabled", __func__);
            } else if (! module->debug_buffer) {
                LOGD("%s debug buffer not allocated", __func__);
            }

            if (nav.nav_event == FPC_NAV_EVENT_ALGO_ERROR) {
                LOGD("%s ALGO_ERROR not reporting to android", __func__);
                goto request;
            }
            if (module->sensetouch_enabled) {
                block_st_events = true;
            }
            report_input_event(FPC_NAV_EVENT, nav.nav_event, FPC_HAL_INPUT_KEY_DOWN);
            report_input_event(FPC_NAV_EVENT, nav.nav_event, FPC_HAL_INPUT_KEY_UP);
        }

        if (module->sensetouch_enabled) {
            if (nav.force != FORCE_SENSOR_NOT_AVAILABLE && nav.force != prev_force) {
                LOGD("%s Reporting raw force: %d over input device.", __func__, nav.force);
                report_input_event(FPC_SENSE_TOUCH_EVENT, FPC_SENSE_TOUCH_RAW, nav.force);
                prev_force = nav.force;
            }

            if (module->st_config != NULL) {
                if (!block_st_events && module->st_config->trigger_threshold != 0 &&
                    nav.force >= module->st_config->trigger_threshold) {
                    if (!force_button_down) {
                        force_button_down = true;
                        block_nav_events = true;
                        report_input_event(FPC_SENSE_TOUCH_EVENT,
                                           FPC_SENSE_TOUCH_PRESS,
                                           FPC_HAL_INPUT_KEY_DOWN);
                    }
                } else if (nav.force <= module->st_config->untrigger_threshold) {
                    if (force_button_down) {
                        force_button_down = false;
                        report_input_event(FPC_SENSE_TOUCH_EVENT,
                                           FPC_SENSE_TOUCH_PRESS,
                                           FPC_HAL_INPUT_KEY_UP);
                    }
                }
            }

request:
            if (!nav.finger_down) {
                block_nav_events = false;
                block_st_events = false;
            }
        }

        switch (nav.request) {
        case FPC_NAV_REQUEST_WAIT_IRQ_HIGH:
            LOGD("%s FPC_NAV_REQUEST_WAIT_IRQ_HIGH", __func__);
            status = fpc_irq_wait(module->irq, 1);
            if (status) {
                goto out;
            }
            break;
        case FPC_NAV_REQUEST_WAIT_IRQ_LOW:
            LOGD("%s FPC_NAV_REQUEST_WAIT_IRQ_LOW", __func__);
            status = fpc_irq_wait(module->irq, 0);
            if (status) {
                goto out;
            }
            break;
        case FPC_NAV_REQUEST_POLL_DATA:
            frame_delta = current_u_time() - time;
            if (frame_delta < frame_time) {
                usleep((frame_time - frame_delta));
            }
            time = current_u_time();
            break;
        }
    }

out:
    if (module->debug_buffer) {
        free(module->debug_buffer);
        module->debug_buffer = NULL;
    }

    fpc_tee_nav_exit(module->tee);
}

static void set_config(fpc_navigation_t *self, const fpc_nav_config_t *config)
{
    LOGD("%s", __func__);
    nav_module_t *nav = (nav_module_t *) self;
    pthread_mutex_lock(&nav->mutex);
    nav->config = *config;
    nav->configured = true;
    pthread_mutex_unlock(&nav->mutex);
}

static void get_config(fpc_navigation_t *self, fpc_nav_config_t *config)
{
    LOGD("%s", __func__);
    nav_module_t *nav = (nav_module_t *) self;
    pthread_mutex_lock(&nav->mutex);
    *config = nav->config;
    pthread_mutex_unlock(&nav->mutex);
}

static void cancel(nav_module_t *nav)
{
    /* Set cancel states */
    fpc_irq_set_cancel(nav->irq);
    pthread_mutex_lock(&nav->cancel_mutex);
    nav->cancel = true;
    pthread_mutex_unlock(&nav->cancel_mutex);
    fpc_worker_join_task(nav->worker_thread);

    /* Reset cancel states */
    pthread_mutex_lock(&nav->cancel_mutex);
    nav->cancel = false;
    pthread_mutex_unlock(&nav->cancel_mutex);
    fpc_irq_clear_cancel(nav->irq);
}

static void set_enabled(fpc_navigation_t *self, bool enabled)
{
    LOGD("%s", __func__);
    nav_module_t *nav = (nav_module_t *) self;
    pthread_mutex_lock(&nav->mutex);

    bool is_running = nav->enabled && !nav->paused;
    bool should_run = enabled && !nav->paused;

    if (should_run && !is_running) {
        fpc_worker_run_task(nav->worker_thread, nav_loop, nav);
    } else if (!should_run && is_running) {
        cancel(nav);
    }

    nav->enabled = enabled;

    pthread_mutex_unlock(&nav->mutex);
}

static bool get_enabled(fpc_navigation_t *self)
{
    LOGD("%s", __func__);
    nav_module_t *nav = (nav_module_t *) self;
    pthread_mutex_lock(&nav->mutex);
    bool enabled = nav->enabled;
    pthread_mutex_unlock(&nav->mutex);
    return enabled;
}

void fpc_navigation_resume(fpc_navigation_t *self)
{
    LOGD("%s", __func__);
    nav_module_t *nav = (nav_module_t *) self;
    pthread_mutex_lock(&nav->mutex);

    if (nav->enabled && nav->paused) {
        fpc_worker_run_task(nav->worker_thread, nav_loop, nav);
    }
    nav->paused = false;
    pthread_mutex_unlock(&nav->mutex);
}

void fpc_navigation_pause(fpc_navigation_t *self)
{
    LOGD("%s", __func__);
    nav_module_t *nav = (nav_module_t *) self;
    pthread_mutex_lock(&nav->mutex);

    if (nav->enabled && !nav->paused) {
        cancel(nav);
    }

    nav->paused = true;
    pthread_mutex_unlock(&nav->mutex);
}

fpc_navigation_t *fpc_navigation_new(fpc_hal_common_t *hal)
{
    nav_module_t *module = (nav_module_t *) calloc(sizeof(nav_module_t), 1);
    if (!module) {
        return NULL;
    }

    pthread_mutex_init(&module->mutex, NULL);
    pthread_mutex_init(&module->cancel_mutex, NULL);

    module->irq = fpc_irq_init();
    if (!module->irq) {
        goto err;
    }

    module->tee = hal->tee_handle;

    module->sensetouch_enabled = fpc_tee_sensetouch_enabled(module->tee);

    if (module->sensetouch_enabled) {
        module->st_config = fpc_sense_touch_get_config();
    }

    module->worker_thread = fpc_worker_new();
    if (!module->worker_thread) {
        goto err;
    }

    module->navigation.set_config = set_config;
    module->navigation.get_config = get_config;
    module->navigation.set_enabled = set_enabled;
    module->navigation.get_enabled = get_enabled;
    module->enabled = true;
    module->paused = true;

    return (fpc_navigation_t *) module;

err:
    fpc_navigation_destroy((fpc_navigation_t *) module);

    return NULL;
}

void fpc_navigation_destroy(fpc_navigation_t *self)
{
    if (!self) {
        return;
    }

    nav_module_t *module = (nav_module_t *) self;

    fpc_irq_release(module->irq);
    fpc_worker_destroy(module->worker_thread);
    pthread_mutex_destroy(&module->mutex);
    pthread_mutex_destroy(&module->cancel_mutex);
    free(module);
}
