/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <linux/input.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "fpc_reset_device.h"
#include "fpc_types.h"
#include "fpc_sysfs.h"
#include "fpc_log.h"

extern const char *const fpc_ree_device_alias_file;
extern const char *const fpc_ree_device_name;
extern const char *const fpc_ree_device_path;

struct fpc_reset {
    int sysfs_reset_fd;
};

void fpc_reset_release(fpc_reset_t* device)
{
    if (!device) {
        return;
    }

    if (device->sysfs_reset_fd != -1) {
        close(device->sysfs_reset_fd);
    }

    free(device);
}

fpc_reset_t* fpc_reset_init()
{
    fpc_reset_t* device = malloc(sizeof(fpc_reset_t));

    if (!device) {
        goto err;
    }

    device->sysfs_reset_fd = -1;

#ifndef FPC_CONFIG_SIMPLY_BYPASS_FPC_SYSFS_APIS
//Simply bypass the calls to fpc sysfs api, for example for QEMU + Trusty
    char path[PATH_MAX];
    if (!fpc_sysfs_path_by_attr(fpc_ree_device_alias_file, fpc_ree_device_name, fpc_ree_device_path,
                                path, PATH_MAX)) {
        LOGE("%s Error didn't find phys path device", __func__);
        goto err;
    }

    device->sysfs_reset_fd = open(path, O_RDONLY);

    if (device->sysfs_reset_fd == -1) {
        LOGE("%s open %s failed %s", __func__, path, strerror(errno));
        goto err;
    }
#endif

    return device;

err:

    fpc_reset_release(device);
    return NULL;
}

#ifdef FPC_CONFIG_NORMAL_SPI_RESET
int32_t fpc_reset_spi(fpc_reset_t* device)
{
    if (!device) {
        return -FPC_ERROR_PARAMETER;
    }

    return -FPC_ERROR_NOT_SUPPORTED;
}
#endif

#ifdef FPC_CONFIG_NORMAL_SENSOR_RESET
int32_t fpc_reset_sensor(fpc_reset_t* device)
{
    if (!device) {
        return -FPC_ERROR_PARAMETER;
    }

#ifndef FPC_CONFIG_SIMPLY_BYPASS_FPC_SYSFS_APIS
//Simply bypass the calls to fpc sysfs api, for example for QEMU + Trusty
    return fpc_sysfs_node_write(device->sysfs_reset_fd, "hw_reset", "reset");
#else
    (void)(device);
    return 0;
#endif
}

#endif
