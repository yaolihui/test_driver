/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_SYSFS_H
#define FPC_SYSFS_H

#include <stdint.h>
#include <stdbool.h>

int fpc_sysfs_path_by_attr(const char *attr, const char *attr_val,
                                 const char *base, char *path, int path_max);

int fpc_sysfs_node_write(int base_fd, const char* name, const char* value);

/**
 * @brief Get file size.
 *
 * @param[in]   path        path to file.
 * @param[out]  size        size of file.
 * @return      int         0 on success, otherwise fail.
 */
int fpc_get_file_size(const char *const path, size_t *const size);

/**
 * @brief Read blob from file and put into buffer.
 *
 * @param[in]       path        path to file.
 * @param[in,out]   buffer      buffer to fill.
 * @param[in]       size        size of file.
 * @return          int         0 on success, otherwise fail.
 */
int fpc_read_blob_from_file(const char *path, uint8_t *buffer, int32_t size);

/**
 * @brief Write blob buffer to file.
 *
 * @param[in]   path        path to file.
 * @param[in]   buffer      buffer to write.
 * @param[in]   size        size of buffer.
 * @return      int         0 on success, otherwise fail.
 */
int fpc_write_blob_to_file(const char *path, const uint8_t *buffer, uint32_t size);

/**
 * @brief Check if file exists.
 *
 * @param[in]   path        path to file.
 * @return      true        File exists.
 * @return      false       File do not exist.
 */
bool fpc_sysfs_check_file(const char *path);

#endif // FPC_SYSFS_H
