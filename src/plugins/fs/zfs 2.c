/*
 * Copyright (C) 2024  Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Razvan Vilt
 */

#include <blockdev/utils.h>
#include <check_deps.h>
#include <string.h>

#include "zfs.h"
#include "fs.h"
#include "common.h"

/* ZFS FS sub-plugin: SEVERELY RESTRICTED per security review.
 * Only get_info, check_label, and check_uuid are functional.
 * All other operations (mkfs, check, repair, resize, set_label, set_uuid)
 * return NOT_SUPPORTED to prevent pool-level operations from being
 * triggered through the generic FS dispatch path.
 */

static volatile guint avail_deps = 0;
static GMutex deps_check_lock;

#define DEPS_ZPOOL 0
#define DEPS_ZPOOL_MASK (1 << DEPS_ZPOOL)
#define DEPS_LAST 1

static const UtilDep deps[DEPS_LAST] = {
    {"zpool", NULL, NULL, NULL},
};


/**
 * bd_fs_zfs_is_tech_avail:
 * @tech: the queried tech
 * @mode: a bit mask of queried modes of operation (#BDFSTechMode) for @tech
 * @error: (out) (optional): place to store error (details about why the @tech-@mode combination is not available)
 *
 * Returns: whether the @tech-@mode combination is available -- supported by the
 *          plugin implementation and having all the runtime dependencies available
 */
G_GNUC_INTERNAL gboolean
bd_fs_zfs_is_tech_avail (BDFSTech tech G_GNUC_UNUSED, guint64 mode, GError **error) {
    /* only QUERY mode is supported */
    if (mode & ~BD_FS_TECH_MODE_QUERY) {
        g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_TECH_UNAVAIL,
                     "ZFS FS operations other than query are not supported through the generic FS interface. "
                     "Use the BD_PLUGIN_ZFS top-level plugin instead.");
        return FALSE;
    }

    return check_deps (&avail_deps, DEPS_ZPOOL_MASK, deps, DEPS_LAST, &deps_check_lock, error);
}

/**
 * bd_fs_zfs_info_copy: (skip)
 *
 * Creates a new copy of @data.
 */
BDFSZfsInfo* bd_fs_zfs_info_copy (BDFSZfsInfo *data) {
    if (data == NULL)
        return NULL;

    BDFSZfsInfo *ret = g_new0 (BDFSZfsInfo, 1);

    ret->label = g_strdup (data->label);
    ret->uuid = g_strdup (data->uuid);
    ret->size = data->size;
    ret->free_space = data->free_space;

    return ret;
}

/**
 * bd_fs_zfs_info_free: (skip)
 *
 * Frees @data.
 */
void bd_fs_zfs_info_free (BDFSZfsInfo *data) {
    if (data == NULL)
        return;

    g_free (data->label);
    g_free (data->uuid);
    g_free (data);
}

/**
 * bd_fs_zfs_check_label:
 * @label: label to check
 * @error: (out) (optional): place to store error
 *
 * Returns: whether @label is a valid label for a ZFS pool or not
 *          (reason is provided in @error)
 *
 * Tech category: always available
 */
gboolean bd_fs_zfs_check_label (const gchar *label, GError **error) {
    /* ZFS pool names: 1-255 chars, alphanumeric + _ - . : (no leading digit, no leading -) */
    if (label == NULL || *label == '\0') {
        g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_LABEL_INVALID,
                     "ZFS pool name cannot be empty");
        return FALSE;
    }

    if (strlen (label) > 255) {
        g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_LABEL_INVALID,
                     "ZFS pool name too long (max 255 characters)");
        return FALSE;
    }

    /* Must not start with digit or dash */
    if (g_ascii_isdigit (label[0]) || label[0] == '-') {
        g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_LABEL_INVALID,
                     "ZFS pool name must not start with a digit or dash");
        return FALSE;
    }

    /* Only alphanumeric, underscore, dash, period, colon */
    for (const gchar *p = label; *p; p++) {
        if (!g_ascii_isalnum (*p) && *p != '_' && *p != '-' && *p != '.' && *p != ':') {
            g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_LABEL_INVALID,
                         "ZFS pool name contains invalid character '%c'", *p);
            return FALSE;
        }
    }

    /* Reserved names */
    if (g_strcmp0 (label, "mirror") == 0 || g_strcmp0 (label, "raidz") == 0 ||
        g_strcmp0 (label, "draid") == 0 || g_strcmp0 (label, "spare") == 0 ||
        g_strcmp0 (label, "log") == 0 || g_strcmp0 (label, "cache") == 0 ||
        g_strcmp0 (label, "special") == 0 || g_strcmp0 (label, "dedup") == 0) {
        g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_LABEL_INVALID,
                     "ZFS pool name '%s' is reserved", label);
        return FALSE;
    }

    return TRUE;
}

/**
 * bd_fs_zfs_check_uuid:
 * @uuid: UUID to check
 * @error: (out) (optional): place to store error
 *
 * Returns: whether @uuid is a valid UUID (pool GUID) for ZFS or not
 *          (reason is provided in @error)
 *
 * Tech category: always available
 */
gboolean bd_fs_zfs_check_uuid (const gchar *uuid, GError **error) {
    /* ZFS pool GUIDs are 64-bit unsigned integers, not standard UUIDs */
    if (uuid == NULL || *uuid == '\0') {
        g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_UUID_INVALID,
                     "ZFS pool GUID cannot be empty");
        return FALSE;
    }

    /* Validate it's a valid uint64 */
    gchar *end = NULL;
    g_ascii_strtoull (uuid, &end, 10);
    if (end == uuid || *end != '\0') {
        g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_UUID_INVALID,
                     "ZFS pool GUID must be a decimal number");
        return FALSE;
    }

    return TRUE;
}

/**
 * bd_fs_zfs_get_info:
 * @device: the device containing the file system to get info for
 * @error: (out) (optional): place to store error (if any)
 *
 * Returns: (transfer full): information about the ZFS file system on @device or
 *                           %NULL in case of error
 *
 * Tech category: %BD_FS_TECH_ZFS-%BD_FS_TECH_MODE_QUERY
 */
BDFSZfsInfo* bd_fs_zfs_get_info (const gchar *device, GError **error) {
    const gchar *argv[] = {"zpool", "list", "-H", "-p", "-o", "name,guid,size,free", NULL};
    gchar *output = NULL;
    gboolean success;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return NULL;

    success = bd_utils_exec_and_capture_output (argv, NULL, &output, error);
    if (!success) {
        g_free (output);
        return NULL;
    }

    if (output == NULL || *output == '\0') {
        g_free (output);
        g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_FAIL,
                     "No ZFS pools found");
        return NULL;
    }

    /* Parse first line: name\tguid\tsize\tfree */
    gchar **lines = g_strsplit (output, "\n", 2);
    g_free (output);

    if (lines[0] == NULL || *lines[0] == '\0') {
        g_strfreev (lines);
        g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_FAIL,
                     "Failed to parse ZFS pool info");
        return NULL;
    }

    gchar **fields = g_strsplit (lines[0], "\t", -1);
    g_strfreev (lines);

    guint n_fields = g_strv_length (fields);
    if (n_fields < 4) {
        g_strfreev (fields);
        g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_FAIL,
                     "Unexpected zpool list output format");
        return NULL;
    }

    BDFSZfsInfo *info = g_new0 (BDFSZfsInfo, 1);
    info->label = g_strdup (fields[0]);        /* pool name */
    info->uuid = g_strdup (fields[1]);         /* pool GUID */
    info->size = g_ascii_strtoull (fields[2], NULL, 10);
    info->free_space = g_ascii_strtoull (fields[3], NULL, 10);

    g_strfreev (fields);
    return info;
}
