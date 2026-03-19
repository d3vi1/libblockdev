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

/* ZFS FS sub-plugin: RESTRICTED per security review.
 * Supported: get_info, check_label, check_uuid.
 * Not supported: mkfs, check, repair, resize, set_uuid, set_label.
 * mkfs/check/repair/resize are semantically incompatible with ZFS pools
 * and must go through the BD_PLUGIN_ZFS top-level plugin instead.
 * set_label (pool rename) requires export/reimport and cannot safely
 * preserve the original import context for rollback; it must go through
 * a dedicated pool rename operation instead.
 */

static volatile guint avail_deps = 0;
static GMutex deps_check_lock;

#define DEPS_ZPOOL 0
#define DEPS_ZPOOL_MASK (1 << DEPS_ZPOOL)
#define DEPS_ZDB 1
#define DEPS_ZDB_MASK (1 << DEPS_ZDB)
#define DEPS_LAST 2

static const UtilDep deps[DEPS_LAST] = {
    {"zpool", NULL, NULL, NULL},
    {"zdb", NULL, NULL, NULL},
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
    guint64 supported = BD_FS_TECH_MODE_QUERY;
    if (mode & ~supported) {
        g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_TECH_UNAVAIL,
                     "ZFS FS operations other than query are not supported through the generic FS interface. "
                     "Use the BD_PLUGIN_ZFS top-level plugin instead.");
        return FALSE;
    }

    return check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZDB_MASK, deps, DEPS_LAST, &deps_check_lock, error);
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
 * resolve_pool_name_from_device:
 * @device: a block device that is a member of a ZFS pool
 * @error: (out) (optional): place to store error (if any)
 *
 * Uses `zdb -l` to read the ZFS label directly from @device and extract
 * the pool name. This is the only safe way to determine which pool a
 * device belongs to -- listing all pools and picking the first one is a
 * fail-open vulnerability in multi-pool systems.
 *
 * Returns: (transfer full): the pool name, or %NULL on error
 */
static gchar *
resolve_pool_name_from_device (const gchar *device, GError **error) {
    if (device == NULL || *device == '\0') {
        g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_FAIL,
                     "Device path cannot be NULL or empty");
        return NULL;
    }
    if (*device == '-') {
        g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_FAIL,
                     "Device path cannot start with '-': %s", device);
        return NULL;
    }

    const gchar *argv_zdb[] = {"zdb", "-l", "--", device, NULL};
    gchar *zdb_output = NULL;
    gchar *pool_name = NULL;
    gboolean success;

    success = bd_utils_exec_and_capture_output (argv_zdb, NULL, &zdb_output, error);
    if (!success) {
        g_free (zdb_output);
        g_prefix_error (error,
                        "Failed to determine ZFS pool name for device '%s': ",
                        device);
        return NULL;
    }

    /* Parse zdb -l output for "name: 'poolname'" */
    gchar **zdb_lines = g_strsplit (zdb_output, "\n", -1);
    g_free (zdb_output);

    /* zdb -l may output multiple label copies (0-3), each with its own
     * "name: 'poolname'" line.  They all carry the same pool name, so
     * we take the first match. */
    for (gchar **lp = zdb_lines; *lp; lp++) {
        gchar *stripped = g_strstrip (g_strdup (*lp));
        if (g_str_has_prefix (stripped, "name: '")) {
            /* Extract pool name between quotes */
            gchar *start = stripped + 7;  /* skip "name: '" */
            gchar *end = strchr (start, '\'');
            if (end) {
                pool_name = g_strndup (start, end - start);
            }
        }
        g_free (stripped);
        if (pool_name)
            break;
    }
    g_strfreev (zdb_lines);

    if (!pool_name || *pool_name == '\0') {
        g_free (pool_name);
        g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_FAIL,
                     "Failed to determine ZFS pool name for device '%s'", device);
        return NULL;
    }

    return pool_name;
}

/**
 * bd_fs_zfs_set_label:
 * @device: a device that is a member of the ZFS pool to rename
 * @label: the new pool name (label)
 * @error: (out) (optional): place to store error (if any)
 *
 * ZFS pool rename requires export/reimport and cannot safely preserve the
 * original import context (search dirs, altroot, properties, etc.) for
 * rollback. This function always returns an error directing the caller to
 * use a dedicated pool rename operation instead.
 *
 * Returns: always %FALSE
 *
 * Tech category: %BD_FS_TECH_ZFS-%BD_FS_TECH_MODE_SET_LABEL
 */
gboolean bd_fs_zfs_set_label (const gchar *device G_GNUC_UNUSED, const gchar *label G_GNUC_UNUSED, GError **error) {
    g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_NOT_SUPPORTED,
                 "ZFS pool rename requires export/reimport and cannot be safely performed "
                 "through the filesystem label interface. Use a dedicated pool rename operation instead.");
    return FALSE;
}

/**
 * bd_fs_zfs_check_label:
 * @label: label to check
 * @error: (out) (optional): place to store error
 *
 * Validates that @label is a valid ZFS pool name per canonical OpenZFS naming
 * rules (see zpool_name_valid() in OpenZFS).  This function intentionally
 * mirrors the logic in bd_zfs_validate_pool_name() from the ZFS top-level
 * plugin; the two cannot call each other at runtime because libbd_fs and
 * libbd_zfs are separate shared libraries.  Any changes to naming rules must
 * be applied to both functions.
 *
 * Deliberate deviation from upstream OpenZFS: spaces are rejected even though
 * OpenZFS technically allows them in pool names.  Spaces break shell quoting,
 * D-Bus object paths, and systemd mount units; no sane deployment uses them.
 *
 * Returns: whether @label is a valid label for a ZFS pool or not
 *          (reason is provided in @error)
 *
 * Tech category: always available
 */
gboolean bd_fs_zfs_check_label (const gchar *label, GError **error) {
    if (label == NULL || *label == '\0') {
        g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_LABEL_INVALID,
                     "ZFS pool name cannot be NULL or empty");
        return FALSE;
    }

    /* Max pool name length: ZFS_MAX_DATASET_NAME_LEN(256) - 2 - strlen("$ORIGIN")*2 = 240 */
    if (strlen (label) >= 240) {
        g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_LABEL_INVALID,
                     "ZFS pool name exceeds maximum length of 239 characters");
        return FALSE;
    }

    /* Must begin with a letter */
    if (!g_ascii_isalpha (label[0])) {
        g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_LABEL_INVALID,
                     "ZFS pool name must begin with a letter: '%s'", label);
        return FALSE;
    }

    /* Only allowed characters: alphanumerics, underscore, hyphen, period, colon */
    for (const gchar *p = label; *p != '\0'; p++) {
        if (!g_ascii_isalnum (*p) && *p != '_' && *p != '-' && *p != '.' && *p != ':') {
            g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_LABEL_INVALID,
                         "ZFS pool name contains invalid character '%c': '%s'", *p, label);
            return FALSE;
        }
    }

    /* Reserved vdev type prefixes per OpenZFS zpool_name_valid() */
    if (g_ascii_strncasecmp (label, "mirror", 6) == 0 ||
        g_ascii_strncasecmp (label, "raidz", 5) == 0 ||
        g_ascii_strncasecmp (label, "draid", 5) == 0 ||
        g_ascii_strncasecmp (label, "spare", 5) == 0 ||
        g_ascii_strcasecmp (label, "log") == 0) {
        g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_LABEL_INVALID,
                     "ZFS pool name '%s' conflicts with a reserved vdev type name", label);
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
 * The pool is identified by reading the ZFS on-disk label from @device using
 * zdb(8). Requires both zpool and zdb to be available on the system.
 *
 * Returns: (transfer full): information about the ZFS file system on @device or
 *                           %NULL in case of error
 *
 * Tech category: %BD_FS_TECH_ZFS-%BD_FS_TECH_MODE_QUERY
 */
BDFSZfsInfo* bd_fs_zfs_get_info (const gchar *device, GError **error) {
    gchar *output = NULL;
    gchar *pool_name = NULL;
    gboolean success;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZDB_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return NULL;

    /* Resolve the pool name from the device using zdb -l */
    pool_name = resolve_pool_name_from_device (device, error);
    if (!pool_name)
        return NULL;

    /* Query the specific pool */
    const gchar *argv[] = {"zpool", "list", "-H", "-p", "-o", "name,guid,size,free", pool_name, NULL};
    success = bd_utils_exec_and_capture_output (argv, NULL, &output, error);
    if (!success) {
        g_free (output);
        g_free (pool_name);
        return NULL;
    }

    g_free (pool_name);

    if (output == NULL || *output == '\0') {
        g_free (output);
        g_set_error (error, BD_FS_ERROR, BD_FS_ERROR_FAIL,
                     "No ZFS pool info returned");
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
