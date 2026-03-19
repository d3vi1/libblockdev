/*
 * Copyright (C) 2026  Red Hat, Inc.
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
 */

#include <glib.h>
#include <string.h>
#include <blockdev/utils.h>

#include "zfs.h"
#include "check_deps.h"

/**
 * SECTION: zfs
 * @short_description: plugin for operations with ZFS pools and datasets
 * @title: ZFS
 * @include: zfs.h
 *
 * A plugin for operations with ZFS pools and datasets using CLI tools.
 */

/**
 * bd_zfs_error_quark: (skip)
 */
GQuark bd_zfs_error_quark (void)
{
    return g_quark_from_static_string ("g-bd-zfs-error-quark");
}

BDZFSPoolInfo* bd_zfs_pool_info_copy (BDZFSPoolInfo *info) {
    if (info == NULL)
        return NULL;

    BDZFSPoolInfo *new_info = g_new0 (BDZFSPoolInfo, 1);

    new_info->name = g_strdup (info->name);
    new_info->guid = g_strdup (info->guid);
    new_info->state = info->state;
    new_info->size = info->size;
    new_info->allocated = info->allocated;
    new_info->free = info->free;
    new_info->fragmentation = info->fragmentation;
    new_info->dedup_ratio = info->dedup_ratio;
    new_info->readonly = info->readonly;
    new_info->altroot = g_strdup (info->altroot);
    new_info->ashift = g_strdup (info->ashift);

    return new_info;
}

void bd_zfs_pool_info_free (BDZFSPoolInfo *info) {
    if (info == NULL)
        return;

    g_free (info->name);
    g_free (info->guid);
    g_free (info->altroot);
    g_free (info->ashift);
    g_free (info);
}

BDZFSVdevInfo* bd_zfs_vdev_info_copy (BDZFSVdevInfo *info) {
    if (info == NULL)
        return NULL;

    BDZFSVdevInfo *new_info = g_new0 (BDZFSVdevInfo, 1);

    new_info->path = g_strdup (info->path);
    new_info->type = info->type;
    new_info->state = info->state;
    new_info->alloc = info->alloc;
    new_info->space = info->space;
    new_info->read_errors = info->read_errors;
    new_info->write_errors = info->write_errors;
    new_info->checksum_errors = info->checksum_errors;

    if (info->children) {
        guint64 count = 0;
        for (BDZFSVdevInfo **child = info->children; *child != NULL; child++)
            count++;
        new_info->children = g_new0 (BDZFSVdevInfo *, count + 1);
        for (guint64 i = 0; i < count; i++)
            new_info->children[i] = bd_zfs_vdev_info_copy (info->children[i]);
        new_info->children[count] = NULL;
    } else {
        new_info->children = NULL;
    }

    return new_info;
}

void bd_zfs_vdev_info_free (BDZFSVdevInfo *info) {
    if (info == NULL)
        return;

    g_free (info->path);

    if (info->children) {
        for (BDZFSVdevInfo **child = info->children; *child != NULL; child++)
            bd_zfs_vdev_info_free (*child);
        g_free (info->children);
    }

    g_free (info);
}

BDZFSDatasetInfo* bd_zfs_dataset_info_copy (BDZFSDatasetInfo *info) {
    if (info == NULL)
        return NULL;

    BDZFSDatasetInfo *new_info = g_new0 (BDZFSDatasetInfo, 1);

    new_info->name = g_strdup (info->name);
    new_info->type = info->type;
    new_info->mountpoint = g_strdup (info->mountpoint);
    new_info->origin = g_strdup (info->origin);
    new_info->used = info->used;
    new_info->available = info->available;
    new_info->referenced = info->referenced;
    new_info->compression = g_strdup (info->compression);
    new_info->encryption = g_strdup (info->encryption);
    new_info->key_status = info->key_status;
    new_info->mounted = info->mounted;

    return new_info;
}

void bd_zfs_dataset_info_free (BDZFSDatasetInfo *info) {
    if (info == NULL)
        return;

    g_free (info->name);
    g_free (info->mountpoint);
    g_free (info->origin);
    g_free (info->compression);
    g_free (info->encryption);
    g_free (info);
}

BDZFSSnapshotInfo* bd_zfs_snapshot_info_copy (BDZFSSnapshotInfo *info) {
    if (info == NULL)
        return NULL;

    BDZFSSnapshotInfo *new_info = g_new0 (BDZFSSnapshotInfo, 1);

    new_info->name = g_strdup (info->name);
    new_info->dataset = g_strdup (info->dataset);
    new_info->used = info->used;
    new_info->referenced = info->referenced;
    new_info->creation = g_strdup (info->creation);

    return new_info;
}

void bd_zfs_snapshot_info_free (BDZFSSnapshotInfo *info) {
    if (info == NULL)
        return;

    g_free (info->name);
    g_free (info->dataset);
    g_free (info->creation);
    g_free (info);
}

BDZFSPropertyInfo* bd_zfs_property_info_copy (BDZFSPropertyInfo *info) {
    if (info == NULL)
        return NULL;

    BDZFSPropertyInfo *new_info = g_new0 (BDZFSPropertyInfo, 1);

    new_info->name = g_strdup (info->name);
    new_info->value = g_strdup (info->value);
    new_info->source = g_strdup (info->source);

    return new_info;
}

void bd_zfs_property_info_free (BDZFSPropertyInfo *info) {
    if (info == NULL)
        return;

    g_free (info->name);
    g_free (info->value);
    g_free (info->source);
    g_free (info);
}

BDZFSScrubInfo* bd_zfs_scrub_info_copy (BDZFSScrubInfo *info) {
    if (info == NULL)
        return NULL;

    BDZFSScrubInfo *new_info = g_new0 (BDZFSScrubInfo, 1);

    new_info->state = info->state;
    new_info->total_bytes = info->total_bytes;
    new_info->scanned_bytes = info->scanned_bytes;
    new_info->issued_bytes = info->issued_bytes;
    new_info->errors = info->errors;
    new_info->percent_done = info->percent_done;
    new_info->start_time = g_strdup (info->start_time);
    new_info->end_time = g_strdup (info->end_time);

    return new_info;
}

void bd_zfs_scrub_info_free (BDZFSScrubInfo *info) {
    if (info == NULL)
        return;

    g_free (info->start_time);
    g_free (info->end_time);
    g_free (info);
}

static volatile guint avail_deps = 0;
static GMutex deps_check_lock;

#define DEPS_ZPOOL 0
#define DEPS_ZPOOL_MASK (1 << DEPS_ZPOOL)
#define DEPS_ZFS 1
#define DEPS_ZFS_MASK (1 << DEPS_ZFS)
#define DEPS_LAST 2

static const UtilDep deps[DEPS_LAST] = {
    {"zpool", NULL, NULL, NULL},
    {"zfs", NULL, NULL, NULL},
};

/**
 * validate_name_not_option:
 * @name: identifier to validate
 * @param_name: human-readable parameter name for error messages
 * @error: (out) (optional): place to store error (if any)
 *
 * Validates that @name is not NULL, not empty, and does not start with '-'.
 * Identifiers starting with '-' could be interpreted as command-line options
 * when passed to zpool/zfs CLI tools.
 *
 * Returns: %TRUE if valid, %FALSE on error
 */
static gboolean
validate_name_not_option (const gchar *name, const gchar *param_name, GError **error) {
    if (!name || *name == '\0') {
        g_set_error (error, BD_ZFS_ERROR, BD_ZFS_ERROR_FAIL,
                     "%s cannot be NULL or empty", param_name);
        return FALSE;
    }
    if (*name == '-') {
        g_set_error (error, BD_ZFS_ERROR, BD_ZFS_ERROR_FAIL,
                     "%s cannot start with '-': %s", param_name, name);
        return FALSE;
    }
    return TRUE;
}

/**
 * bd_zfs_init:
 *
 * Initializes the plugin. **This function is called automatically by the
 * library's initialization functions.**
 *
 */
gboolean bd_zfs_init (void) {
    /* nothing to do here */
    return TRUE;
}

/**
 * bd_zfs_close:
 *
 * Cleans up after the plugin. **This function is called automatically by the
 * library's functions that unload it.**
 *
 */
void bd_zfs_close (void) {
    /* reset the cached deps so re-init re-checks tool availability */
    avail_deps = 0;
}

/**
 * bd_zfs_is_tech_avail:
 * @tech: the queried tech
 * @mode: a bit mask of queried modes of operation (#BDZFSTechMode) for @tech
 * @error: (out) (optional): place to store error (details about why the @tech-@mode combination is not available)
 *
 * Returns: whether the @tech-@mode combination is available -- both combos
 *          require the ``zpool`` and ``zfs`` utilities
 *
 * Tech category: always available
 */
gboolean bd_zfs_is_tech_avail (BDZFSTech tech G_GNUC_UNUSED, guint64 mode G_GNUC_UNUSED, GError **error) {
    /* all techs require both zpool and zfs tools */
    return check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error);
}

static BDZFSPoolState parse_pool_state (const gchar *state_str) {
    if (g_strcmp0 (state_str, "ONLINE") == 0)
        return BD_ZFS_POOL_STATE_ONLINE;
    else if (g_strcmp0 (state_str, "DEGRADED") == 0)
        return BD_ZFS_POOL_STATE_DEGRADED;
    else if (g_strcmp0 (state_str, "FAULTED") == 0)
        return BD_ZFS_POOL_STATE_FAULTED;
    else if (g_strcmp0 (state_str, "OFFLINE") == 0)
        return BD_ZFS_POOL_STATE_OFFLINE;
    else if (g_strcmp0 (state_str, "REMOVED") == 0)
        return BD_ZFS_POOL_STATE_REMOVED;
    else if (g_strcmp0 (state_str, "UNAVAIL") == 0)
        return BD_ZFS_POOL_STATE_UNAVAIL;
    else
        return BD_ZFS_POOL_STATE_UNKNOWN;
}

static BDZFSVdevState parse_vdev_state (const gchar *state_str) {
    if (g_strcmp0 (state_str, "ONLINE") == 0)
        return BD_ZFS_VDEV_STATE_ONLINE;
    else if (g_strcmp0 (state_str, "DEGRADED") == 0)
        return BD_ZFS_VDEV_STATE_DEGRADED;
    else if (g_strcmp0 (state_str, "FAULTED") == 0)
        return BD_ZFS_VDEV_STATE_FAULTED;
    else if (g_strcmp0 (state_str, "OFFLINE") == 0)
        return BD_ZFS_VDEV_STATE_OFFLINE;
    else if (g_strcmp0 (state_str, "REMOVED") == 0)
        return BD_ZFS_VDEV_STATE_REMOVED;
    else if (g_strcmp0 (state_str, "UNAVAIL") == 0)
        return BD_ZFS_VDEV_STATE_UNAVAIL;
    else
        return BD_ZFS_VDEV_STATE_UNKNOWN;
}

static BDZFSVdevType infer_vdev_type (const gchar *name) {
    if (g_str_has_prefix (name, "mirror-"))
        return BD_ZFS_VDEV_TYPE_MIRROR;
    else if (g_str_has_prefix (name, "raidz1-") || g_str_has_prefix (name, "raidz-"))
        return BD_ZFS_VDEV_TYPE_RAIDZ1;
    else if (g_str_has_prefix (name, "raidz2-"))
        return BD_ZFS_VDEV_TYPE_RAIDZ2;
    else if (g_str_has_prefix (name, "raidz3-"))
        return BD_ZFS_VDEV_TYPE_RAIDZ3;
    else if (g_str_has_prefix (name, "draid"))
        return BD_ZFS_VDEV_TYPE_DRAID;
    else if (g_strcmp0 (name, "spare") == 0 || g_strcmp0 (name, "spares") == 0)
        return BD_ZFS_VDEV_TYPE_SPARE;
    else if (g_strcmp0 (name, "log") == 0 || g_strcmp0 (name, "logs") == 0)
        return BD_ZFS_VDEV_TYPE_LOG;
    else if (g_strcmp0 (name, "cache") == 0)
        return BD_ZFS_VDEV_TYPE_CACHE;
    else if (g_strcmp0 (name, "special") == 0)
        return BD_ZFS_VDEV_TYPE_SPECIAL;
    else if (g_strcmp0 (name, "dedup") == 0)
        return BD_ZFS_VDEV_TYPE_DEDUP;
    else if (name[0] == '/' || g_str_has_prefix (name, "sd") || g_str_has_prefix (name, "nvme") ||
             g_str_has_prefix (name, "vd") || g_str_has_prefix (name, "hd") ||
             g_str_has_prefix (name, "xvd") || g_str_has_prefix (name, "da"))
        return BD_ZFS_VDEV_TYPE_DISK;
    else
        return BD_ZFS_VDEV_TYPE_UNKNOWN;
}

/**
 * parse_pool_info_line:
 *
 * Parses a single tab-separated line from `zpool list -H -p` output into
 * a BDZFSPoolInfo struct.
 *
 * Fields expected (in order):
 *   name, guid, health, size, alloc, free, frag, dedupratio, altroot, ashift, readonly
 *
 * Returns: (transfer full): a new BDZFSPoolInfo or %NULL on parse error
 */
static BDZFSPoolInfo* parse_pool_info_line (const gchar *line) {
    gchar **fields = NULL;
    BDZFSPoolInfo *info = NULL;
    guint num_fields;

    if (!line || strlen (line) == 0)
        return NULL;

    fields = g_strsplit (line, "\t", -1);
    num_fields = g_strv_length (fields);

    if (num_fields < 11) {
        g_strfreev (fields);
        return NULL;
    }

    info = g_new0 (BDZFSPoolInfo, 1);

    info->name = g_strdup (fields[0]);
    info->guid = g_strdup (fields[1]);
    info->state = parse_pool_state (fields[2]);
    info->size = g_ascii_strtoull (fields[3], NULL, 10);
    info->allocated = g_ascii_strtoull (fields[4], NULL, 10);
    info->free = g_ascii_strtoull (fields[5], NULL, 10);
    info->fragmentation = g_ascii_strtoull (fields[6], NULL, 10);
    info->dedup_ratio = g_ascii_strtod (fields[7], NULL);

    if (g_strcmp0 (fields[8], "-") == 0)
        info->altroot = NULL;
    else
        info->altroot = g_strdup (fields[8]);

    info->ashift = g_strdup (fields[9]);
    info->readonly = (g_strcmp0 (fields[10], "on") == 0);

    g_strfreev (fields);
    return info;
}

/**
 * bd_zfs_pool_create:
 * @name: name of the pool to create
 * @vdevs: (array zero-terminated=1): NULL-terminated array of device paths
 * @raid_level: (nullable): vdev type ("mirror", "raidz", "raidz2", "raidz3") or %NULL for stripe
 * @extra: (nullable) (array zero-terminated=1): extra options for pool creation
 * @error: (out) (optional): place to store error (if any)
 *
 * Creates a new ZFS pool with the given name and vdevs.
 *
 * Returns: whether the pool was successfully created or not
 *
 * Tech category: %BD_ZFS_TECH_POOL-%BD_ZFS_TECH_MODE_CREATE
 */
gboolean bd_zfs_pool_create (const gchar *name, const gchar **vdevs, const gchar *raid_level,
                             const BDExtraArg **extra, GError **error) {
    const gchar **argv = NULL;
    guint num_vdevs = 0;
    guint num_args = 0;
    guint next_arg = 0;
    gboolean success = FALSE;
    const gchar **vdev_p = NULL;

    if (!validate_name_not_option (name, "Pool name", error))
        return FALSE;

    if (raid_level && !validate_name_not_option (raid_level, "RAID level", error))
        return FALSE;

    if (!vdevs || !vdevs[0]) {
        g_set_error_literal (error, BD_ZFS_ERROR, BD_ZFS_ERROR_FAIL, "No vdevs given");
        return FALSE;
    }

    for (vdev_p = vdevs; *vdev_p != NULL; vdev_p++) {
        if (!validate_name_not_option (*vdev_p, "Vdev", error))
            return FALSE;
        num_vdevs++;
    }

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    /* zpool create -- <name> [raid_level] <vdev1> ... <vdevN> NULL */
    num_args = 4 + num_vdevs + (raid_level ? 1 : 0) + 1;
    argv = g_new0 (const gchar*, num_args);

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "create";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    if (raid_level)
        argv[next_arg++] = raid_level;
    for (vdev_p = vdevs; *vdev_p != NULL; vdev_p++)
        argv[next_arg++] = *vdev_p;
    argv[next_arg] = NULL;

    success = bd_utils_exec_and_report_error (argv, extra, error);
    g_free (argv);
    return success;
}

/**
 * bd_zfs_pool_destroy:
 * @name: name of the pool to destroy
 * @force: whether to force the destruction
 * @error: (out) (optional): place to store error (if any)
 *
 * Destroys the ZFS pool with the given name.
 *
 * Returns: whether the pool was successfully destroyed or not
 *
 * Tech category: %BD_ZFS_TECH_POOL-%BD_ZFS_TECH_MODE_DELETE
 */
gboolean bd_zfs_pool_destroy (const gchar *name, gboolean force, GError **error) {
    const gchar *argv[6] = {NULL};
    guint next_arg = 0;

    if (!validate_name_not_option (name, "Pool name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "destroy";
    if (force)
        argv[next_arg++] = "-f";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    argv[next_arg] = NULL;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_pool_export:
 * @name: name of the pool to export
 * @force: whether to force the export
 * @error: (out) (optional): place to store error (if any)
 *
 * Exports the ZFS pool with the given name.
 *
 * Returns: whether the pool was successfully exported or not
 *
 * Tech category: %BD_ZFS_TECH_POOL-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_pool_export (const gchar *name, gboolean force, GError **error) {
    const gchar *argv[6] = {NULL};
    guint next_arg = 0;

    if (!validate_name_not_option (name, "Pool name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "export";
    if (force)
        argv[next_arg++] = "-f";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    argv[next_arg] = NULL;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_pool_import:
 * @name_or_guid: name or GUID of the pool to import
 * @new_name: (nullable): new name for the pool or %NULL to keep the original
 * @search_dirs: (nullable) (array zero-terminated=1): directories to search for devices or %NULL
 * @force: whether to force the import
 * @extra: (nullable) (array zero-terminated=1): extra options for pool import
 * @error: (out) (optional): place to store error (if any)
 *
 * Imports a ZFS pool by name or GUID, optionally renaming it.
 *
 * Returns: whether the pool was successfully imported or not
 *
 * Tech category: %BD_ZFS_TECH_POOL-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_pool_import (const gchar *name_or_guid, const gchar *new_name,
                             const gchar **search_dirs, gboolean force,
                             const BDExtraArg **extra, GError **error) {
    const gchar **argv = NULL;
    guint num_args = 0;
    guint next_arg = 0;
    guint num_dirs = 0;
    gboolean success = FALSE;
    const gchar **dir_p = NULL;

    if (!validate_name_not_option (name_or_guid, "Pool name or GUID", error))
        return FALSE;

    if (new_name && !validate_name_not_option (new_name, "New pool name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    if (search_dirs) {
        for (dir_p = search_dirs; *dir_p != NULL; dir_p++)
            num_dirs++;
    }

    /* zpool import [-f] [-d dir1 -d dir2 ...] -- <name_or_guid> [new_name] NULL */
    num_args = 4 + (force ? 1 : 0) + (num_dirs * 2) + (new_name ? 1 : 0) + 1;
    argv = g_new0 (const gchar*, num_args);

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "import";
    if (force)
        argv[next_arg++] = "-f";
    if (search_dirs) {
        for (dir_p = search_dirs; *dir_p != NULL; dir_p++) {
            argv[next_arg++] = "-d";
            argv[next_arg++] = *dir_p;
        }
    }
    argv[next_arg++] = "--";
    argv[next_arg++] = name_or_guid;
    if (new_name)
        argv[next_arg++] = new_name;
    argv[next_arg] = NULL;

    success = bd_utils_exec_and_report_error (argv, extra, error);
    g_free (argv);
    return success;
}

/**
 * bd_zfs_pool_list:
 * @error: (out) (optional): place to store error (if any)
 *
 * Lists all imported ZFS pools with their properties.
 *
 * Returns: (array zero-terminated=1) (transfer full): a NULL-terminated array of
 *          #BDZFSPoolInfo or %NULL in case of error
 *
 * Tech category: %BD_ZFS_TECH_POOL-%BD_ZFS_TECH_MODE_QUERY
 */
BDZFSPoolInfo** bd_zfs_pool_list (GError **error) {
    const gchar *argv[] = {"zpool", "list", "-H", "-p", "-o",
                           "name,guid,health,size,alloc,free,frag,dedupratio,altroot,ashift,readonly",
                           NULL};
    gchar *output = NULL;
    gboolean success;
    gchar **lines = NULL;
    gchar **line_p = NULL;
    GPtrArray *pool_infos;
    BDZFSPoolInfo *info = NULL;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return NULL;

    success = bd_utils_exec_and_capture_output (argv, NULL, &output, error);
    if (!success) {
        g_free (output);
        return NULL;
    }

    lines = g_strsplit (output, "\n", -1);
    g_free (output);

    pool_infos = g_ptr_array_new ();
    for (line_p = lines; *line_p; line_p++) {
        if (strlen (*line_p) == 0)
            continue;
        info = parse_pool_info_line (*line_p);
        if (info)
            g_ptr_array_add (pool_infos, info);
    }
    g_strfreev (lines);

    g_ptr_array_add (pool_infos, NULL);
    return (BDZFSPoolInfo **) g_ptr_array_free (pool_infos, FALSE);
}

/**
 * bd_zfs_pool_list_importable:
 * @error: (out) (optional): place to store error (if any)
 *
 * Lists ZFS pools available for import by parsing ``zpool import`` output.
 * Each pool block in the output contains a name, numeric id (GUID), and state.
 *
 * Returns: (array zero-terminated=1) (transfer full): a NULL-terminated array of
 *          #BDZFSPoolInfo or %NULL in case of error.  Only the name, guid, and
 *          state fields are populated; size/alloc/free etc. are zero.
 *
 * Tech category: %BD_ZFS_TECH_POOL-%BD_ZFS_TECH_MODE_QUERY
 */
BDZFSPoolInfo** bd_zfs_pool_list_importable (GError **error) {
    const gchar *argv[] = {"zpool", "import", NULL};
    gchar *output = NULL;
    gboolean success;
    gchar **lines = NULL;
    gchar **line_p = NULL;
    GPtrArray *pool_infos;
    gchar *cur_name = NULL;
    gchar *cur_id = NULL;
    gchar *cur_state = NULL;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return NULL;

    success = bd_utils_exec_and_capture_output (argv, NULL, &output, error);
    if (!success) {
        /* zpool import returns exit code 1 when there are no pools to import
         * but also when there is a real error.  If stderr contains "no pools
         * available to import" we treat it as an empty list rather than an
         * error. */
        if (error && *error) {
            if (g_strrstr ((*error)->message, "no pools available to import") != NULL) {
                g_clear_error (error);
                g_free (output);
                pool_infos = g_ptr_array_new ();
                g_ptr_array_add (pool_infos, NULL);
                return (BDZFSPoolInfo **) g_ptr_array_free (pool_infos, FALSE);
            }
        }
        g_free (output);
        return NULL;
    }

    /* If the command succeeded but produced no output, return an empty list. */
    if (output == NULL || strlen (output) == 0) {
        g_free (output);
        pool_infos = g_ptr_array_new ();
        g_ptr_array_add (pool_infos, NULL);
        return (BDZFSPoolInfo **) g_ptr_array_free (pool_infos, FALSE);
    }

    /*
     * The output of `zpool import` (no arguments) looks like:
     *
     *    pool: testpool
     *      id: 4627607309443620138
     *   state: ONLINE
     *  action: The pool can be imported using its name or numeric identifier.
     *  config:
     *      testpool  ONLINE
     *        loop6   ONLINE
     *
     * Multiple pool blocks are separated by blank lines.
     * We extract pool, id, and state from each block.
     */
    lines = g_strsplit (output, "\n", -1);
    g_free (output);

    pool_infos = g_ptr_array_new ();

    for (line_p = lines; *line_p; line_p++) {
        gchar *trimmed = g_strstrip (g_strdup (*line_p));

        if (g_str_has_prefix (trimmed, "pool:")) {
            /* If we already have a complete pool entry, flush it */
            if (cur_name && cur_id) {
                BDZFSPoolInfo *info = g_new0 (BDZFSPoolInfo, 1);
                info->name = cur_name;
                info->guid = cur_id;
                info->state = cur_state ? parse_pool_state (cur_state) : BD_ZFS_POOL_STATE_UNKNOWN;
                g_free (cur_state);
                g_ptr_array_add (pool_infos, info);
                cur_name = NULL;
                cur_id = NULL;
                cur_state = NULL;
            } else {
                g_free (cur_name);
                g_free (cur_id);
                g_free (cur_state);
            }
            cur_name = g_strdup (g_strstrip (trimmed + 5));
        } else if (g_str_has_prefix (trimmed, "id:")) {
            g_free (cur_id);
            cur_id = g_strdup (g_strstrip (trimmed + 3));
        } else if (g_str_has_prefix (trimmed, "state:")) {
            g_free (cur_state);
            cur_state = g_strdup (g_strstrip (trimmed + 6));
        }

        g_free (trimmed);
    }

    /* Flush the last pool entry */
    if (cur_name && cur_id) {
        BDZFSPoolInfo *info = g_new0 (BDZFSPoolInfo, 1);
        info->name = cur_name;
        info->guid = cur_id;
        info->state = cur_state ? parse_pool_state (cur_state) : BD_ZFS_POOL_STATE_UNKNOWN;
        g_free (cur_state);
        g_ptr_array_add (pool_infos, info);
    } else {
        g_free (cur_name);
        g_free (cur_id);
        g_free (cur_state);
    }

    g_strfreev (lines);

    g_ptr_array_add (pool_infos, NULL);
    return (BDZFSPoolInfo **) g_ptr_array_free (pool_infos, FALSE);
}

/**
 * bd_zfs_pool_get_info:
 * @name: name of the pool to get info for
 * @error: (out) (optional): place to store error (if any)
 *
 * Gets information about a single ZFS pool.
 *
 * Returns: (transfer full): a #BDZFSPoolInfo for the given pool or %NULL in case of error
 *
 * Tech category: %BD_ZFS_TECH_POOL-%BD_ZFS_TECH_MODE_QUERY
 */
BDZFSPoolInfo* bd_zfs_pool_get_info (const gchar *name, GError **error) {
    const gchar *argv[] = {"zpool", "list", "-H", "-p", "-o",
                           "name,guid,health,size,alloc,free,frag,dedupratio,altroot,ashift,readonly",
                           "--", name, NULL};
    gchar *output = NULL;
    gboolean success;
    gchar **lines = NULL;
    BDZFSPoolInfo *info = NULL;

    if (!validate_name_not_option (name, "Pool name", error))
        return NULL;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return NULL;

    success = bd_utils_exec_and_capture_output (argv, NULL, &output, error);
    if (!success) {
        g_free (output);
        return NULL;
    }

    lines = g_strsplit (output, "\n", -1);
    g_free (output);

    /* Take the first non-empty line */
    for (gchar **line_p = lines; *line_p; line_p++) {
        if (strlen (*line_p) == 0)
            continue;
        info = parse_pool_info_line (*line_p);
        break;
    }
    g_strfreev (lines);

    if (!info) {
        g_set_error (error, BD_ZFS_ERROR, BD_ZFS_ERROR_PARSE,
                     "Failed to parse pool info for '%s'", name);
        return NULL;
    }

    return info;
}

/**
 * bd_zfs_pool_get_vdevs:
 * @name: name of the pool to get vdev tree for
 * @error: (out) (optional): place to store error (if any)
 *
 * Gets the vdev tree for a ZFS pool by parsing ``zpool status`` output.
 *
 * Returns: (array zero-terminated=1) (transfer full): a NULL-terminated array of
 *          top-level #BDZFSVdevInfo (each may have recursive children) or %NULL in case of error
 *
 * Tech category: %BD_ZFS_TECH_VDEV-%BD_ZFS_TECH_MODE_QUERY
 */
BDZFSVdevInfo** bd_zfs_pool_get_vdevs (const gchar *name, GError **error) {
    const gchar *argv[] = {"zpool", "status", "-P", "--", name, NULL};
    gchar *output = NULL;
    gboolean success;
    gchar **lines = NULL;
    gchar **line_p = NULL;
    gboolean in_config = FALSE;
    gboolean header_skipped = FALSE;

    if (!validate_name_not_option (name, "Pool name", error))
        return NULL;

    /* Stack for building the tree. We track indent level and corresponding vdev info. */
    /* Max depth of 64 should be more than enough for any ZFS pool. */
    BDZFSVdevInfo *stack[64];
    gint stack_indent[64];
    gint stack_top = -1;

    GPtrArray *top_vdevs;
    gint root_indent = -1;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return NULL;

    success = bd_utils_exec_and_capture_output (argv, NULL, &output, error);
    if (!success) {
        g_free (output);
        return NULL;
    }

    lines = g_strsplit (output, "\n", -1);
    g_free (output);

    top_vdevs = g_ptr_array_new ();

    GRegex *ws_regex = g_regex_new ("\\s+", 0, 0, NULL);

    for (line_p = lines; *line_p; line_p++) {
        gchar *line = *line_p;

        /* Skip leading whitespace for section detection */
        const gchar *s = line;
        while (*s == ' ' || *s == '\t')
            s++;

        /* Look for the "config:" line to start parsing */
        if (!in_config) {
            if (g_str_has_prefix (s, "config:"))
                in_config = TRUE;
            continue;
        }

        /* Skip empty lines */
        if (*s == '\0')
            continue;

        /* Skip the header line (NAME STATE READ WRITE CKSUM) */
        if (!header_skipped) {
            if (g_str_has_prefix (s, "NAME")) {
                header_skipped = TRUE;
                continue;
            }
            continue;
        }

        /* Stop parsing at "errors:" line */
        if (g_str_has_prefix (s, "errors:"))
            break;

        /* Count leading whitespace to determine indent level.
         * zpool status uses a leading tab then spaces for hierarchy. */
        gint indent = 0;
        const gchar *p = line;
        while (*p == '\t' || *p == ' ') {
            indent++;
            p++;
        }

        /* Skip fully blank lines after stripping */
        if (*p == '\0')
            continue;

        /* Split fields on whitespace. Use g_strsplit_set on the trimmed
         * portion (p already past leading whitespace). First strip any
         * trailing whitespace too. */
        gchar *trimmed = g_strstrip (g_strdup (p));
        gchar **fields = g_regex_split (ws_regex, trimmed, 0);
        g_free (trimmed);
        guint num_fields = g_strv_length (fields);

        /* g_regex_split may produce trailing empty string */
        if (num_fields > 0 && fields[num_fields - 1][0] == '\0')
            num_fields--;

        if (num_fields < 5) {
            g_strfreev (fields);
            continue;
        }

        /* The first line at the lowest indent is the root (pool name itself) - remember its indent */
        if (root_indent < 0)
            root_indent = indent;

        /* Skip the root vdev (pool name itself) */
        if (indent == root_indent) {
            g_strfreev (fields);
            continue;
        }

        BDZFSVdevInfo *vdev = g_new0 (BDZFSVdevInfo, 1);
        vdev->path = g_strdup (fields[0]);
        vdev->state = parse_vdev_state (fields[1]);
        vdev->read_errors = g_ascii_strtoull (fields[2], NULL, 10);
        vdev->write_errors = g_ascii_strtoull (fields[3], NULL, 10);
        vdev->checksum_errors = g_ascii_strtoull (fields[4], NULL, 10);
        vdev->type = infer_vdev_type (fields[0]);
        vdev->children = NULL;

        g_strfreev (fields);

        /* Pop from stack any entries with indent >= current (they are siblings or deeper) */
        while (stack_top >= 0 && stack_indent[stack_top] >= indent)
            stack_top--;

        /* Check depth BEFORE linking vdev into the tree so that on overflow
         * we can free the unlinked vdev and the tree independently without
         * a double-free. */
        if (stack_top + 1 >= 64) {
            g_set_error (error, BD_ZFS_ERROR, BD_ZFS_ERROR_PARSE,
                         "Vdev tree depth exceeds maximum (64) for pool '%s'", name);
            bd_zfs_vdev_info_free (vdev);
            for (guint i = 0; i < top_vdevs->len; i++)
                bd_zfs_vdev_info_free (g_ptr_array_index (top_vdevs, i));
            g_ptr_array_free (top_vdevs, TRUE);
            g_strfreev (lines);
            g_regex_unref (ws_regex);
            return NULL;
        }

        if (stack_top < 0) {
            /* This is a top-level vdev (direct child of root) */
            g_ptr_array_add (top_vdevs, vdev);
        } else {
            /* This is a child of the vdev at stack_top */
            BDZFSVdevInfo *parent = stack[stack_top];
            if (parent->children == NULL) {
                parent->children = g_new0 (BDZFSVdevInfo *, 2);
                parent->children[0] = vdev;
                parent->children[1] = NULL;
            } else {
                /* Count existing children */
                guint count = 0;
                for (BDZFSVdevInfo **ch = parent->children; *ch != NULL; ch++)
                    count++;
                parent->children = g_renew (BDZFSVdevInfo *, parent->children, count + 2);
                parent->children[count] = vdev;
                parent->children[count + 1] = NULL;
            }
        }

        /* Push this vdev onto the stack */
        stack_top++;
        stack[stack_top] = vdev;
        stack_indent[stack_top] = indent;
    }

    g_strfreev (lines);
    g_regex_unref (ws_regex);

    if (top_vdevs->len == 0) {
        g_set_error (error, BD_ZFS_ERROR, BD_ZFS_ERROR_PARSE,
                     "Failed to parse any vdevs from 'zpool status' output for pool '%s'", name);
        g_ptr_array_free (top_vdevs, TRUE);
        return NULL;
    }

    g_ptr_array_add (top_vdevs, NULL);
    return (BDZFSVdevInfo **) g_ptr_array_free (top_vdevs, FALSE);
}

/**
 * bd_zfs_pool_add_vdev:
 * @name: name of the pool to add vdevs to
 * @vdevs: (array zero-terminated=1): NULL-terminated array of device paths to add
 * @raid_level: (nullable): vdev type ("mirror", "raidz", "raidz2", "raidz3") or %NULL for stripe
 * @extra: (nullable) (array zero-terminated=1): extra options for the operation
 * @error: (out) (optional): place to store error (if any)
 *
 * Adds one or more vdevs to an existing ZFS pool.
 *
 * Returns: whether the vdevs were successfully added or not
 *
 * Tech category: %BD_ZFS_TECH_VDEV-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_pool_add_vdev (const gchar *name, const gchar **vdevs, const gchar *raid_level,
                                const BDExtraArg **extra, GError **error) {
    const gchar **argv = NULL;
    guint num_vdevs = 0;
    guint num_args = 0;
    guint next_arg = 0;
    gboolean success = FALSE;
    const gchar **vdev_p = NULL;

    if (!validate_name_not_option (name, "Pool name", error))
        return FALSE;

    if (raid_level && !validate_name_not_option (raid_level, "RAID level", error))
        return FALSE;

    if (!vdevs || !vdevs[0]) {
        g_set_error_literal (error, BD_ZFS_ERROR, BD_ZFS_ERROR_FAIL, "No vdevs given");
        return FALSE;
    }

    for (vdev_p = vdevs; *vdev_p != NULL; vdev_p++) {
        if (!validate_name_not_option (*vdev_p, "Vdev", error))
            return FALSE;
        num_vdevs++;
    }

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    /* zpool add -- <name> [raid_level] <vdev1> ... <vdevN> NULL */
    num_args = 4 + num_vdevs + (raid_level ? 1 : 0) + 1;
    argv = g_new0 (const gchar*, num_args);

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "add";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    if (raid_level)
        argv[next_arg++] = raid_level;
    for (vdev_p = vdevs; *vdev_p != NULL; vdev_p++)
        argv[next_arg++] = *vdev_p;
    argv[next_arg] = NULL;

    success = bd_utils_exec_and_report_error (argv, extra, error);
    g_free (argv);
    return success;
}

/**
 * bd_zfs_pool_remove_vdev:
 * @name: name of the pool to remove a vdev from
 * @vdev: the vdev to remove
 * @error: (out) (optional): place to store error (if any)
 *
 * Removes a vdev from a ZFS pool.
 *
 * Returns: whether the vdev was successfully removed or not
 *
 * Tech category: %BD_ZFS_TECH_VDEV-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_pool_remove_vdev (const gchar *name, const gchar *vdev, GError **error) {
    const gchar *argv[6] = {NULL};
    guint next_arg = 0;

    if (!validate_name_not_option (name, "Pool name", error))
        return FALSE;

    if (!validate_name_not_option (vdev, "Vdev", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "remove";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    argv[next_arg++] = vdev;
    argv[next_arg] = NULL;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_pool_attach:
 * @name: name of the pool
 * @existing_vdev: the existing vdev to attach to
 * @new_vdev: the new vdev to attach
 * @error: (out) (optional): place to store error (if any)
 *
 * Attaches a new vdev to an existing vdev in a ZFS pool (creating or extending a mirror).
 *
 * Returns: whether the vdev was successfully attached or not
 *
 * Tech category: %BD_ZFS_TECH_VDEV-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_pool_attach (const gchar *name, const gchar *existing_vdev, const gchar *new_vdev, GError **error) {
    const gchar *argv[7] = {NULL};
    guint next_arg = 0;

    if (!validate_name_not_option (name, "Pool name", error))
        return FALSE;

    if (!validate_name_not_option (existing_vdev, "Existing vdev", error))
        return FALSE;

    if (!validate_name_not_option (new_vdev, "New vdev", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "attach";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    argv[next_arg++] = existing_vdev;
    argv[next_arg++] = new_vdev;
    argv[next_arg] = NULL;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_pool_detach:
 * @name: name of the pool
 * @vdev: the vdev to detach
 * @error: (out) (optional): place to store error (if any)
 *
 * Detaches a vdev from a ZFS pool mirror.
 *
 * Returns: whether the vdev was successfully detached or not
 *
 * Tech category: %BD_ZFS_TECH_VDEV-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_pool_detach (const gchar *name, const gchar *vdev, GError **error) {
    const gchar *argv[6] = {NULL};
    guint next_arg = 0;

    if (!validate_name_not_option (name, "Pool name", error))
        return FALSE;

    if (!validate_name_not_option (vdev, "Vdev", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "detach";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    argv[next_arg++] = vdev;
    argv[next_arg] = NULL;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_pool_replace:
 * @name: name of the pool
 * @old_vdev: the vdev to replace
 * @new_vdev: the replacement vdev
 * @force: whether to force the replacement
 * @error: (out) (optional): place to store error (if any)
 *
 * Replaces a vdev in a ZFS pool with a new device.
 *
 * Returns: whether the vdev was successfully replaced or not
 *
 * Tech category: %BD_ZFS_TECH_VDEV-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_pool_replace (const gchar *name, const gchar *old_vdev, const gchar *new_vdev,
                               gboolean force, GError **error) {
    const gchar *argv[8] = {NULL};
    guint next_arg = 0;

    if (!validate_name_not_option (name, "Pool name", error))
        return FALSE;

    if (!validate_name_not_option (old_vdev, "Old vdev", error))
        return FALSE;

    if (!validate_name_not_option (new_vdev, "New vdev", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "replace";
    if (force)
        argv[next_arg++] = "-f";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    argv[next_arg++] = old_vdev;
    argv[next_arg++] = new_vdev;
    argv[next_arg] = NULL;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_pool_online:
 * @name: name of the pool
 * @vdev: the vdev to bring online
 * @expand: whether to expand the device to use all available space
 * @error: (out) (optional): place to store error (if any)
 *
 * Brings a vdev in a ZFS pool online.
 *
 * Returns: whether the vdev was successfully brought online or not
 *
 * Tech category: %BD_ZFS_TECH_VDEV-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_pool_online (const gchar *name, const gchar *vdev, gboolean expand, GError **error) {
    const gchar *argv[7] = {NULL};
    guint next_arg = 0;

    if (!validate_name_not_option (name, "Pool name", error))
        return FALSE;

    if (!validate_name_not_option (vdev, "Vdev", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "online";
    if (expand)
        argv[next_arg++] = "-e";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    argv[next_arg++] = vdev;
    argv[next_arg] = NULL;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_pool_offline:
 * @name: name of the pool
 * @vdev: the vdev to take offline
 * @temporary: whether the offline state is temporary (reverts on reboot)
 * @error: (out) (optional): place to store error (if any)
 *
 * Takes a vdev in a ZFS pool offline.
 *
 * Returns: whether the vdev was successfully taken offline or not
 *
 * Tech category: %BD_ZFS_TECH_VDEV-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_pool_offline (const gchar *name, const gchar *vdev, gboolean temporary, GError **error) {
    const gchar *argv[7] = {NULL};
    guint next_arg = 0;

    if (!validate_name_not_option (name, "Pool name", error))
        return FALSE;

    if (!validate_name_not_option (vdev, "Vdev", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "offline";
    if (temporary)
        argv[next_arg++] = "-t";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    argv[next_arg++] = vdev;
    argv[next_arg] = NULL;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_pool_scrub_start:
 * @name: name of the pool to scrub
 * @error: (out) (optional): place to store error (if any)
 *
 * Starts a scrub on the ZFS pool.
 *
 * Returns: whether the scrub was successfully started or not
 *
 * Tech category: %BD_ZFS_TECH_MAINTENANCE-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_pool_scrub_start (const gchar *name, GError **error) {
    const gchar *argv[] = {"zpool", "scrub", "--", name, NULL};

    if (!validate_name_not_option (name, "Pool name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_pool_scrub_stop:
 * @name: name of the pool to stop scrubbing
 * @error: (out) (optional): place to store error (if any)
 *
 * Stops a running scrub on the ZFS pool.
 *
 * Returns: whether the scrub was successfully stopped or not
 *
 * Tech category: %BD_ZFS_TECH_MAINTENANCE-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_pool_scrub_stop (const gchar *name, GError **error) {
    const gchar *argv[] = {"zpool", "scrub", "-s", "--", name, NULL};

    if (!validate_name_not_option (name, "Pool name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_pool_scrub_pause:
 * @name: name of the pool to pause scrubbing
 * @error: (out) (optional): place to store error (if any)
 *
 * Pauses a running scrub on the ZFS pool.
 *
 * Returns: whether the scrub was successfully paused or not
 *
 * Tech category: %BD_ZFS_TECH_MAINTENANCE-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_pool_scrub_pause (const gchar *name, GError **error) {
    const gchar *argv[] = {"zpool", "scrub", "-p", "--", name, NULL};

    if (!validate_name_not_option (name, "Pool name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * parse_size_suffix:
 *
 * Parses a size string with a suffix (e.g. "1.23G", "456M", "789K") and
 * returns the value in bytes. Returns 0 on parse failure.
 */
static guint64 parse_size_suffix (const gchar *str) {
    gdouble val;
    gchar *end = NULL;

    if (!str || *str == '\0')
        return 0;

    val = g_ascii_strtod (str, &end);
    if (end == str)
        return 0;

    if (end && *end != '\0') {
        switch (*end) {
            case 'K':
                val *= 1024.0;
                break;
            case 'M':
                val *= 1024.0 * 1024.0;
                break;
            case 'G':
                val *= 1024.0 * 1024.0 * 1024.0;
                break;
            case 'T':
                val *= 1024.0 * 1024.0 * 1024.0 * 1024.0;
                break;
            case 'P':
                val *= 1024.0 * 1024.0 * 1024.0 * 1024.0 * 1024.0;
                break;
            case 'B':
                /* just bytes */
                break;
            default:
                break;
        }
    }

    return (guint64) val;
}

/**
 * bd_zfs_pool_scrub_status:
 * @name: name of the pool to get scrub status for
 * @error: (out) (optional): place to store error (if any)
 *
 * Gets the scrub status for a ZFS pool by parsing ``zpool status`` output.
 *
 * Returns: (transfer full): a #BDZFSScrubInfo for the pool or %NULL in case of error
 *
 * Tech category: %BD_ZFS_TECH_MAINTENANCE-%BD_ZFS_TECH_MODE_QUERY
 */
BDZFSScrubInfo* bd_zfs_pool_scrub_status (const gchar *name, GError **error) {
    const gchar *argv[] = {"zpool", "status", "--", name, NULL};
    gchar *output = NULL;
    gboolean success;
    gchar **lines = NULL;
    gchar **line_p = NULL;
    BDZFSScrubInfo *info = NULL;
    gchar *scan_line = NULL;
    gchar *next_line = NULL;

    if (!validate_name_not_option (name, "Pool name", error))
        return NULL;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return NULL;

    success = bd_utils_exec_and_capture_output (argv, NULL, &output, error);
    if (!success) {
        g_free (output);
        return NULL;
    }

    lines = g_strsplit (output, "\n", -1);
    g_free (output);

    /* Find the "scan:" line */
    for (line_p = lines; *line_p; line_p++) {
        gchar *stripped = g_strstrip (g_strdup (*line_p));
        if (g_str_has_prefix (stripped, "scan:")) {
            scan_line = stripped;
            /* Grab the next line if available (for progress info) */
            if (*(line_p + 1))
                next_line = g_strstrip (g_strdup (*(line_p + 1)));
            break;
        }
        g_free (stripped);
    }

    if (!scan_line) {
        g_strfreev (lines);
        g_set_error (error, BD_ZFS_ERROR, BD_ZFS_ERROR_PARSE,
                     "Failed to find scan status for pool '%s'", name);
        return NULL;
    }

    info = g_new0 (BDZFSScrubInfo, 1);
    info->state = BD_ZFS_SCRUB_STATE_NONE;
    info->total_bytes = 0;
    info->scanned_bytes = 0;
    info->issued_bytes = 0;
    info->errors = 0;
    info->percent_done = 0.0;
    info->start_time = NULL;
    info->end_time = NULL;

    if (strstr (scan_line, "in progress")) {
        info->state = BD_ZFS_SCRUB_STATE_SCANNING;

        /* Try to parse "since <timestamp>" */
        {
            const gchar *since = strstr (scan_line, "since ");
            if (since) {
                since += 6; /* skip "since " */
                info->start_time = g_strdup (since);
            }
        }

        /* Parse the next line for progress details */
        /* Example: "1.23G scanned at 456M/s, 789M issued at 123M/s, 2.00G total" */
        if (next_line) {
            gchar **tokens = g_regex_split_simple ("\\s+", next_line, 0, 0);
            guint num_tokens = g_strv_length (tokens);

            /* Look for "scanned" token to get scanned bytes */
            for (guint i = 0; i + 1 < num_tokens; i++) {
                if (g_strcmp0 (tokens[i + 1], "scanned") == 0 || g_str_has_prefix (tokens[i + 1], "scanned")) {
                    info->scanned_bytes = parse_size_suffix (tokens[i]);
                    break;
                }
            }

            /* Look for "issued" token to get issued bytes */
            for (guint i = 0; i + 1 < num_tokens; i++) {
                if (g_strcmp0 (tokens[i + 1], "issued") == 0 || g_str_has_prefix (tokens[i + 1], "issued")) {
                    info->issued_bytes = parse_size_suffix (tokens[i]);
                    break;
                }
            }

            /* Look for "total" token to get total bytes */
            for (guint i = 0; i + 1 < num_tokens; i++) {
                if (g_strcmp0 (tokens[i + 1], "total") == 0 || g_str_has_prefix (tokens[i + 1], "total")) {
                    info->total_bytes = parse_size_suffix (tokens[i]);
                    break;
                }
            }

            g_strfreev (tokens);

            /* Look for "% done" in subsequent lines */
            for (gchar **lp = line_p + 1; *lp; lp++) {
                if (strstr (*lp, "done")) {
                    gchar *pct = strstr (*lp, "% done");
                    if (!pct)
                        pct = strstr (*lp, "done");
                    if (pct) {
                        /* Walk backward from "% done" to find the number */
                        gchar *stripped_l = g_strstrip (g_strdup (*lp));
                        gchar **parts = g_regex_split_simple ("\\s+", stripped_l, 0, 0);
                        guint nparts = g_strv_length (parts);
                        for (guint j = 0; j < nparts; j++) {
                            if (strstr (parts[j], "done") && j > 0) {
                                /* The previous token or this token might contain the percentage */
                                gchar *pct_str = parts[j];
                                if (strstr (pct_str, "%")) {
                                    info->percent_done = g_ascii_strtod (pct_str, NULL);
                                } else if (j > 0) {
                                    pct_str = parts[j - 1];
                                    /* Remove trailing % if present */
                                    gchar *pct_end = strchr (pct_str, '%');
                                    if (pct_end)
                                        *pct_end = '\0';
                                    info->percent_done = g_ascii_strtod (pct_str, NULL);
                                }
                                break;
                            }
                        }
                        g_strfreev (parts);
                        g_free (stripped_l);
                    }
                    break;
                }
            }

            /* Look for errors in subsequent lines */
            for (gchar **lp = line_p + 1; *lp; lp++) {
                if (strstr (*lp, "errors")) {
                    gchar *stripped_l = g_strstrip (g_strdup (*lp));
                    gchar **parts = g_regex_split_simple ("\\s+", stripped_l, 0, 0);
                    guint nparts = g_strv_length (parts);
                    for (guint j = 0; j + 1 < nparts; j++) {
                        if (g_strcmp0 (parts[j + 1], "errors") == 0) {
                            info->errors = g_ascii_strtoull (parts[j], NULL, 10);
                            break;
                        }
                    }
                    g_strfreev (parts);
                    g_free (stripped_l);
                    break;
                }
            }
        }
    } else if (strstr (scan_line, "repaired")) {
        info->state = BD_ZFS_SCRUB_STATE_FINISHED;
        info->percent_done = 100.0;

        /* Try to parse "on <timestamp>" for end_time */
        {
            const gchar *on = strstr (scan_line, " on ");
            if (on) {
                on += 4; /* skip " on " */
                info->end_time = g_strdup (on);
            }
        }

        /* Try to parse errors from the scan line itself: "with N errors" */
        {
            const gchar *with_err = strstr (scan_line, "with ");
            if (with_err) {
                with_err += 5; /* skip "with " */
                info->errors = g_ascii_strtoull (with_err, NULL, 10);
            }
        }
    } else if (strstr (scan_line, "canceled")) {
        info->state = BD_ZFS_SCRUB_STATE_CANCELED;

        /* Try to parse "on <timestamp>" */
        {
            const gchar *on = strstr (scan_line, " on ");
            if (on) {
                on += 4;
                info->end_time = g_strdup (on);
            }
        }
    } else if (strstr (scan_line, "paused")) {
        info->state = BD_ZFS_SCRUB_STATE_PAUSED;

        /* Try to parse "since <timestamp>" */
        {
            const gchar *since = strstr (scan_line, "since ");
            if (since) {
                since += 6;
                info->start_time = g_strdup (since);
            }
        }
    } else if (strstr (scan_line, "none requested")) {
        info->state = BD_ZFS_SCRUB_STATE_NONE;
    }

    g_free (scan_line);
    g_free (next_line);
    g_strfreev (lines);

    return info;
}

/**
 * bd_zfs_pool_trim_start:
 * @name: name of the pool to trim
 * @vdev: (nullable): specific vdev to trim or %NULL to trim all
 * @error: (out) (optional): place to store error (if any)
 *
 * Starts a TRIM operation on the ZFS pool or a specific vdev.
 *
 * Returns: whether the trim was successfully started or not
 *
 * Tech category: %BD_ZFS_TECH_MAINTENANCE-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_pool_trim_start (const gchar *name, const gchar *vdev, GError **error) {
    const gchar *argv[6] = {NULL};
    guint next_arg = 0;

    if (!validate_name_not_option (name, "Pool name", error))
        return FALSE;

    if (vdev && !validate_name_not_option (vdev, "Vdev", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "trim";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    if (vdev)
        argv[next_arg++] = vdev;
    argv[next_arg] = NULL;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_pool_trim_stop:
 * @name: name of the pool to stop trimming
 * @vdev: (nullable): specific vdev to stop trimming or %NULL for all
 * @error: (out) (optional): place to store error (if any)
 *
 * Stops a TRIM operation on the ZFS pool or a specific vdev.
 *
 * Returns: whether the trim was successfully stopped or not
 *
 * Tech category: %BD_ZFS_TECH_MAINTENANCE-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_pool_trim_stop (const gchar *name, const gchar *vdev, GError **error) {
    const gchar *argv[7] = {NULL};
    guint next_arg = 0;

    if (!validate_name_not_option (name, "Pool name", error))
        return FALSE;

    if (vdev && !validate_name_not_option (vdev, "Vdev", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "trim";
    argv[next_arg++] = "-s";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    if (vdev)
        argv[next_arg++] = vdev;
    argv[next_arg] = NULL;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_pool_clear:
 * @name: name of the pool to clear errors on
 * @vdev: (nullable): specific vdev to clear errors on or %NULL for all
 * @error: (out) (optional): place to store error (if any)
 *
 * Clears device errors in a ZFS pool.
 *
 * Returns: whether the errors were successfully cleared or not
 *
 * Tech category: %BD_ZFS_TECH_MAINTENANCE-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_pool_clear (const gchar *name, const gchar *vdev, GError **error) {
    const gchar *argv[6] = {NULL};
    guint next_arg = 0;

    if (!validate_name_not_option (name, "Pool name", error))
        return FALSE;

    if (vdev && !validate_name_not_option (vdev, "Vdev", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "clear";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    if (vdev)
        argv[next_arg++] = vdev;
    argv[next_arg] = NULL;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * parse_property_line:
 *
 * Parses a single tab-separated line from `zpool get -H -p` output into
 * a BDZFSPropertyInfo struct.
 *
 * Fields expected (in order): name, property, value, source
 *
 * Returns: (transfer full): a new BDZFSPropertyInfo or %NULL on parse error
 */
static BDZFSPropertyInfo* parse_property_line (const gchar *line) {
    gchar **fields = NULL;
    BDZFSPropertyInfo *info = NULL;
    guint num_fields;

    if (!line || strlen (line) == 0)
        return NULL;

    fields = g_strsplit (line, "\t", -1);
    num_fields = g_strv_length (fields);

    if (num_fields < 4) {
        g_strfreev (fields);
        return NULL;
    }

    info = g_new0 (BDZFSPropertyInfo, 1);

    info->name = g_strdup (fields[1]);
    info->value = g_strdup (fields[2]);
    info->source = g_strdup (fields[3]);

    g_strfreev (fields);
    return info;
}

/**
 * bd_zfs_pool_get_property:
 * @name: name of the pool
 * @property: name of the property to get
 * @error: (out) (optional): place to store error (if any)
 *
 * Gets a single property from a ZFS pool.
 *
 * Returns: (transfer full): a #BDZFSPropertyInfo or %NULL in case of error
 *
 * Tech category: %BD_ZFS_TECH_POOL-%BD_ZFS_TECH_MODE_QUERY
 */
BDZFSPropertyInfo* bd_zfs_pool_get_property (const gchar *name, const gchar *property, GError **error) {
    const gchar *argv[] = {"zpool", "get", "-H", "-p", property, "--", name, NULL};
    gchar *output = NULL;
    gboolean success;
    gchar **lines = NULL;
    BDZFSPropertyInfo *info = NULL;

    if (!validate_name_not_option (name, "Pool name", error))
        return NULL;

    if (!validate_name_not_option (property, "Property name", error))
        return NULL;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return NULL;

    success = bd_utils_exec_and_capture_output (argv, NULL, &output, error);
    if (!success) {
        g_free (output);
        return NULL;
    }

    lines = g_strsplit (output, "\n", -1);
    g_free (output);

    /* Take the first non-empty line */
    for (gchar **line_p = lines; *line_p; line_p++) {
        if (strlen (*line_p) == 0)
            continue;
        info = parse_property_line (*line_p);
        break;
    }
    g_strfreev (lines);

    if (!info) {
        g_set_error (error, BD_ZFS_ERROR, BD_ZFS_ERROR_PARSE,
                     "Failed to parse property '%s' for pool '%s'", property, name);
        return NULL;
    }

    return info;
}

/**
 * bd_zfs_pool_set_property:
 * @name: name of the pool
 * @property: name of the property to set
 * @value: value to set the property to
 * @error: (out) (optional): place to store error (if any)
 *
 * Sets a property on a ZFS pool.
 *
 * Returns: whether the property was successfully set or not
 *
 * Tech category: %BD_ZFS_TECH_POOL-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_pool_set_property (const gchar *name, const gchar *property, const gchar *value, GError **error) {
    gchar *prop_val = NULL;
    const gchar *argv[5] = {NULL};
    gboolean success;

    if (!validate_name_not_option (name, "Pool name", error))
        return FALSE;

    if (!validate_name_not_option (property, "Property name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    prop_val = g_strdup_printf ("%s=%s", property, value);

    argv[0] = "zpool";
    argv[1] = "set";
    argv[2] = prop_val;
    argv[3] = name;
    argv[4] = NULL;

    success = bd_utils_exec_and_report_error (argv, NULL, error);
    g_free (prop_val);
    return success;
}

/**
 * bd_zfs_pool_get_properties:
 * @name: name of the pool
 * @error: (out) (optional): place to store error (if any)
 *
 * Gets all properties from a ZFS pool.
 *
 * Returns: (array zero-terminated=1) (transfer full): a NULL-terminated array of
 *          #BDZFSPropertyInfo or %NULL in case of error
 *
 * Tech category: %BD_ZFS_TECH_POOL-%BD_ZFS_TECH_MODE_QUERY
 */
BDZFSPropertyInfo** bd_zfs_pool_get_properties (const gchar *name, GError **error) {
    const gchar *argv[] = {"zpool", "get", "-H", "-p", "all", "--", name, NULL};
    gchar *output = NULL;
    gboolean success;
    gchar **lines = NULL;
    gchar **line_p = NULL;
    GPtrArray *props;
    BDZFSPropertyInfo *info = NULL;

    if (!validate_name_not_option (name, "Pool name", error))
        return NULL;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return NULL;

    success = bd_utils_exec_and_capture_output (argv, NULL, &output, error);
    if (!success) {
        g_free (output);
        return NULL;
    }

    lines = g_strsplit (output, "\n", -1);
    g_free (output);

    props = g_ptr_array_new ();
    for (line_p = lines; *line_p; line_p++) {
        if (strlen (*line_p) == 0)
            continue;
        info = parse_property_line (*line_p);
        if (info)
            g_ptr_array_add (props, info);
    }
    g_strfreev (lines);

    g_ptr_array_add (props, NULL);
    return (BDZFSPropertyInfo **) g_ptr_array_free (props, FALSE);
}

/**
 * parse_dataset_info_line:
 *
 * Parses a single tab-separated line from `zfs list -H -p` output into
 * a BDZFSDatasetInfo struct.
 *
 * Fields expected (in order):
 *   name, type, mountpoint, origin, used, avail, refer, compress, encryption, keystatus, mounted
 *
 * Returns: (transfer full): a new BDZFSDatasetInfo or %NULL on parse error
 */
static BDZFSDatasetInfo* parse_dataset_info_line (const gchar *line) {
    gchar **fields = NULL;
    BDZFSDatasetInfo *info = NULL;
    guint num_fields;

    if (!line || strlen (line) == 0)
        return NULL;

    fields = g_strsplit (line, "\t", -1);
    num_fields = g_strv_length (fields);

    if (num_fields < 11) {
        g_strfreev (fields);
        return NULL;
    }

    info = g_new0 (BDZFSDatasetInfo, 1);

    /* [0] name */
    info->name = g_strdup (fields[0]);

    /* [1] type */
    if (g_strcmp0 (fields[1], "filesystem") == 0)
        info->type = BD_ZFS_DATASET_TYPE_FILESYSTEM;
    else if (g_strcmp0 (fields[1], "volume") == 0)
        info->type = BD_ZFS_DATASET_TYPE_VOLUME;
    else if (g_strcmp0 (fields[1], "snapshot") == 0)
        info->type = BD_ZFS_DATASET_TYPE_SNAPSHOT;
    else if (g_strcmp0 (fields[1], "bookmark") == 0)
        info->type = BD_ZFS_DATASET_TYPE_BOOKMARK;
    else
        info->type = BD_ZFS_DATASET_TYPE_FILESYSTEM;

    /* [2] mountpoint ("-" means none -> NULL) */
    if (g_strcmp0 (fields[2], "-") == 0 || g_strcmp0 (fields[2], "none") == 0)
        info->mountpoint = NULL;
    else
        info->mountpoint = g_strdup (fields[2]);

    /* [3] origin ("-" means none -> NULL) */
    if (g_strcmp0 (fields[3], "-") == 0)
        info->origin = NULL;
    else
        info->origin = g_strdup (fields[3]);

    /* [4] used (guint64) */
    info->used = g_ascii_strtoull (fields[4], NULL, 10);

    /* [5] available (guint64) */
    info->available = g_ascii_strtoull (fields[5], NULL, 10);

    /* [6] referenced (guint64) */
    info->referenced = g_ascii_strtoull (fields[6], NULL, 10);

    /* [7] compression */
    info->compression = g_strdup (fields[7]);

    /* [8] encryption */
    info->encryption = g_strdup (fields[8]);

    /* [9] keystatus */
    if (g_strcmp0 (fields[9], "available") == 0)
        info->key_status = BD_ZFS_KEY_STATUS_AVAILABLE;
    else if (g_strcmp0 (fields[9], "unavailable") == 0)
        info->key_status = BD_ZFS_KEY_STATUS_UNAVAILABLE;
    else
        info->key_status = BD_ZFS_KEY_STATUS_NONE;

    /* [10] mounted ("yes"/"no"/"-") */
    info->mounted = (g_strcmp0 (fields[10], "yes") == 0);

    g_strfreev (fields);
    return info;
}

/**
 * bd_zfs_dataset_create:
 * @name: name of the dataset to create (e.g. "pool/dataset")
 * @extra: (nullable) (array zero-terminated=1): extra options for dataset creation (e.g. -o property=value)
 * @error: (out) (optional): place to store error (if any)
 *
 * Creates a new ZFS dataset with the given name.
 *
 * Returns: whether the dataset was successfully created or not
 *
 * Tech category: %BD_ZFS_TECH_DATASET-%BD_ZFS_TECH_MODE_CREATE
 */
gboolean bd_zfs_dataset_create (const gchar *name, const BDExtraArg **extra, GError **error) {
    const gchar *argv[] = {"zfs", "create", "--", name, NULL};

    if (!validate_name_not_option (name, "Dataset name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    return bd_utils_exec_and_report_error (argv, extra, error);
}

/**
 * bd_zfs_dataset_destroy:
 * @name: name of the dataset to destroy
 * @recursive: whether to recursively destroy all children
 * @force: whether to force the destruction of dependents
 * @error: (out) (optional): place to store error (if any)
 *
 * Destroys the ZFS dataset with the given name.
 *
 * Returns: whether the dataset was successfully destroyed or not
 *
 * Tech category: %BD_ZFS_TECH_DATASET-%BD_ZFS_TECH_MODE_DELETE
 */
gboolean bd_zfs_dataset_destroy (const gchar *name, gboolean recursive, gboolean force, GError **error) {
    const gchar *argv[7] = {NULL};
    guint next_arg = 0;

    if (!validate_name_not_option (name, "Dataset name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zfs";
    argv[next_arg++] = "destroy";
    if (recursive)
        argv[next_arg++] = "-r";
    if (force)
        argv[next_arg++] = "-f";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    argv[next_arg] = NULL;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_dataset_list:
 * @pool_or_parent: (nullable): pool or parent dataset to list datasets for, or %NULL for all
 * @recursive: whether to list datasets recursively
 * @error: (out) (optional): place to store error (if any)
 *
 * Lists ZFS datasets under the given pool or parent dataset.
 *
 * Returns: (array zero-terminated=1) (transfer full): a NULL-terminated array of
 *          #BDZFSDatasetInfo or %NULL in case of error
 *
 * Tech category: %BD_ZFS_TECH_DATASET-%BD_ZFS_TECH_MODE_QUERY
 */
BDZFSDatasetInfo** bd_zfs_dataset_list (const gchar *pool_or_parent, gboolean recursive, GError **error) {
    const gchar **argv = NULL;
    guint next_arg = 0;
    guint num_args;
    gchar *output = NULL;
    gboolean success;
    gchar **lines = NULL;
    gchar **line_p = NULL;
    GPtrArray *dataset_infos;
    BDZFSDatasetInfo *info = NULL;

    if (pool_or_parent && !validate_name_not_option (pool_or_parent, "Pool or parent dataset", error))
        return NULL;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return NULL;

    /* zfs list -H -p -t all [-r] -o <fields> [-- pool_or_parent] NULL */
    num_args = 8 + (recursive ? 1 : 0) + (pool_or_parent ? 2 : 0) + 1;
    argv = g_new0 (const gchar*, num_args);

    argv[next_arg++] = "zfs";
    argv[next_arg++] = "list";
    argv[next_arg++] = "-H";
    argv[next_arg++] = "-p";
    argv[next_arg++] = "-t";
    argv[next_arg++] = "all";
    if (recursive)
        argv[next_arg++] = "-r";
    argv[next_arg++] = "-o";
    argv[next_arg++] = "name,type,mountpoint,origin,used,avail,refer,compress,encryption,keystatus,mounted";
    if (pool_or_parent) {
        argv[next_arg++] = "--";
        argv[next_arg++] = pool_or_parent;
    }
    argv[next_arg] = NULL;

    success = bd_utils_exec_and_capture_output (argv, NULL, &output, error);
    g_free (argv);
    if (!success) {
        g_free (output);
        return NULL;
    }

    lines = g_strsplit (output, "\n", -1);
    g_free (output);

    dataset_infos = g_ptr_array_new ();
    for (line_p = lines; *line_p; line_p++) {
        if (strlen (*line_p) == 0)
            continue;
        info = parse_dataset_info_line (*line_p);
        if (info)
            g_ptr_array_add (dataset_infos, info);
    }
    g_strfreev (lines);

    g_ptr_array_add (dataset_infos, NULL);
    return (BDZFSDatasetInfo **) g_ptr_array_free (dataset_infos, FALSE);
}

/**
 * bd_zfs_dataset_get_info:
 * @name: name of the dataset to get info for
 * @error: (out) (optional): place to store error (if any)
 *
 * Gets information about a single ZFS dataset.
 *
 * Returns: (transfer full): a #BDZFSDatasetInfo for the given dataset or %NULL in case of error
 *
 * Tech category: %BD_ZFS_TECH_DATASET-%BD_ZFS_TECH_MODE_QUERY
 */
BDZFSDatasetInfo* bd_zfs_dataset_get_info (const gchar *name, GError **error) {
    const gchar *argv[] = {"zfs", "list", "-H", "-p", "-t", "all", "-o",
                           "name,type,mountpoint,origin,used,avail,refer,compress,encryption,keystatus,mounted",
                           "--", name, NULL};
    gchar *output = NULL;
    gboolean success;
    gchar **lines = NULL;
    BDZFSDatasetInfo *info = NULL;

    if (!validate_name_not_option (name, "Dataset name", error))
        return NULL;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return NULL;

    success = bd_utils_exec_and_capture_output (argv, NULL, &output, error);
    if (!success) {
        g_free (output);
        return NULL;
    }

    lines = g_strsplit (output, "\n", -1);
    g_free (output);

    /* Take the first non-empty line */
    for (gchar **line_p = lines; *line_p; line_p++) {
        if (strlen (*line_p) == 0)
            continue;
        info = parse_dataset_info_line (*line_p);
        break;
    }
    g_strfreev (lines);

    if (!info) {
        g_set_error (error, BD_ZFS_ERROR, BD_ZFS_ERROR_PARSE,
                     "Failed to parse dataset info for '%s'", name);
        return NULL;
    }

    return info;
}

/**
 * bd_zfs_dataset_rename:
 * @name: current name of the dataset
 * @new_name: new name for the dataset
 * @create_parent: whether to create parent datasets if they do not exist
 * @force: whether to force unmount any filesystems that need to be unmounted
 * @error: (out) (optional): place to store error (if any)
 *
 * Renames a ZFS dataset.
 *
 * Returns: whether the dataset was successfully renamed or not
 *
 * Tech category: %BD_ZFS_TECH_DATASET-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_dataset_rename (const gchar *name, const gchar *new_name, gboolean create_parent,
                                 gboolean force, GError **error) {
    const gchar *argv[8] = {NULL};
    guint next_arg = 0;

    if (!validate_name_not_option (name, "Dataset name", error))
        return FALSE;

    if (!validate_name_not_option (new_name, "New dataset name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zfs";
    argv[next_arg++] = "rename";
    if (create_parent)
        argv[next_arg++] = "-p";
    if (force)
        argv[next_arg++] = "-f";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    argv[next_arg++] = new_name;
    argv[next_arg] = NULL;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_dataset_mount:
 * @name: name of the dataset to mount
 * @mountpoint: (nullable): mountpoint to use or %NULL to use the dataset's mountpoint property
 * @extra: (nullable) (array zero-terminated=1): extra mount options
 * @error: (out) (optional): place to store error (if any)
 *
 * Mounts a ZFS dataset. If @mountpoint is specified, it will be used as the
 * mountpoint override via ``-o mountpoint=``.
 *
 * Returns: whether the dataset was successfully mounted or not
 *
 * Tech category: %BD_ZFS_TECH_DATASET-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_dataset_mount (const gchar *name, const gchar *mountpoint, const BDExtraArg **extra, GError **error) {
    gchar *mp_opt = NULL;
    const gchar *argv[7] = {NULL};
    guint next_arg = 0;
    gboolean success;

    if (!validate_name_not_option (name, "Dataset name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zfs";
    argv[next_arg++] = "mount";
    if (mountpoint) {
        mp_opt = g_strdup_printf ("mountpoint=%s", mountpoint);
        argv[next_arg++] = "-o";
        argv[next_arg++] = mp_opt;
    }
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    argv[next_arg] = NULL;

    success = bd_utils_exec_and_report_error (argv, extra, error);
    g_free (mp_opt);
    return success;
}

/**
 * bd_zfs_dataset_unmount:
 * @name: name of the dataset to unmount
 * @force: whether to force the unmount
 * @error: (out) (optional): place to store error (if any)
 *
 * Unmounts a ZFS dataset.
 *
 * Returns: whether the dataset was successfully unmounted or not
 *
 * Tech category: %BD_ZFS_TECH_DATASET-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_dataset_unmount (const gchar *name, gboolean force, GError **error) {
    const gchar *argv[6] = {NULL};
    guint next_arg = 0;

    if (!validate_name_not_option (name, "Dataset name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zfs";
    argv[next_arg++] = "unmount";
    if (force)
        argv[next_arg++] = "-f";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    argv[next_arg] = NULL;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_dataset_get_property:
 * @name: name of the dataset
 * @property: name of the property to get
 * @error: (out) (optional): place to store error (if any)
 *
 * Gets a single property from a ZFS dataset.
 *
 * Returns: (transfer full): a #BDZFSPropertyInfo or %NULL in case of error
 *
 * Tech category: %BD_ZFS_TECH_DATASET-%BD_ZFS_TECH_MODE_QUERY
 */
BDZFSPropertyInfo* bd_zfs_dataset_get_property (const gchar *name, const gchar *property, GError **error) {
    const gchar *argv[] = {"zfs", "get", "-H", "-p", property, "--", name, NULL};
    gchar *output = NULL;
    gboolean success;
    gchar **lines = NULL;
    BDZFSPropertyInfo *info = NULL;

    if (!validate_name_not_option (name, "Dataset name", error))
        return NULL;

    if (!validate_name_not_option (property, "Property name", error))
        return NULL;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return NULL;

    success = bd_utils_exec_and_capture_output (argv, NULL, &output, error);
    if (!success) {
        g_free (output);
        return NULL;
    }

    lines = g_strsplit (output, "\n", -1);
    g_free (output);

    /* Take the first non-empty line */
    for (gchar **line_p = lines; *line_p; line_p++) {
        if (strlen (*line_p) == 0)
            continue;
        info = parse_property_line (*line_p);
        break;
    }
    g_strfreev (lines);

    if (!info) {
        g_set_error (error, BD_ZFS_ERROR, BD_ZFS_ERROR_PARSE,
                     "Failed to parse property '%s' for dataset '%s'", property, name);
        return NULL;
    }

    return info;
}

/**
 * bd_zfs_dataset_set_property:
 * @name: name of the dataset
 * @property: name of the property to set
 * @value: value to set the property to
 * @error: (out) (optional): place to store error (if any)
 *
 * Sets a property on a ZFS dataset.
 *
 * Returns: whether the property was successfully set or not
 *
 * Tech category: %BD_ZFS_TECH_DATASET-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_dataset_set_property (const gchar *name, const gchar *property, const gchar *value, GError **error) {
    gchar *prop_val = NULL;
    const gchar *argv[5] = {NULL};
    gboolean success;

    if (!validate_name_not_option (name, "Dataset name", error))
        return FALSE;

    if (!validate_name_not_option (property, "Property name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    prop_val = g_strdup_printf ("%s=%s", property, value);

    argv[0] = "zfs";
    argv[1] = "set";
    argv[2] = prop_val;
    argv[3] = name;
    argv[4] = NULL;

    success = bd_utils_exec_and_report_error (argv, NULL, error);
    g_free (prop_val);
    return success;
}

/**
 * bd_zfs_dataset_get_properties:
 * @name: name of the dataset
 * @error: (out) (optional): place to store error (if any)
 *
 * Gets all properties from a ZFS dataset.
 *
 * Returns: (array zero-terminated=1) (transfer full): a NULL-terminated array of
 *          #BDZFSPropertyInfo or %NULL in case of error
 *
 * Tech category: %BD_ZFS_TECH_DATASET-%BD_ZFS_TECH_MODE_QUERY
 */
BDZFSPropertyInfo** bd_zfs_dataset_get_properties (const gchar *name, GError **error) {
    const gchar *argv[] = {"zfs", "get", "-H", "-p", "all", "--", name, NULL};
    gchar *output = NULL;
    gboolean success;
    gchar **lines = NULL;
    gchar **line_p = NULL;
    GPtrArray *props;
    BDZFSPropertyInfo *info = NULL;

    if (!validate_name_not_option (name, "Dataset name", error))
        return NULL;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return NULL;

    success = bd_utils_exec_and_capture_output (argv, NULL, &output, error);
    if (!success) {
        g_free (output);
        return NULL;
    }

    lines = g_strsplit (output, "\n", -1);
    g_free (output);

    props = g_ptr_array_new ();
    for (line_p = lines; *line_p; line_p++) {
        if (strlen (*line_p) == 0)
            continue;
        info = parse_property_line (*line_p);
        if (info)
            g_ptr_array_add (props, info);
    }
    g_strfreev (lines);

    g_ptr_array_add (props, NULL);
    return (BDZFSPropertyInfo **) g_ptr_array_free (props, FALSE);
}

/**
 * parse_snapshot_info_line:
 *
 * Parses a single tab-separated line from `zfs list -H -p -t snapshot` output
 * into a BDZFSSnapshotInfo struct.
 *
 * Fields expected (in order):
 *   name, used, refer, creation
 *
 * Returns: (transfer full): a new BDZFSSnapshotInfo or %NULL on parse error
 */
static BDZFSSnapshotInfo* parse_snapshot_info_line (const gchar *line) {
    gchar **fields = NULL;
    BDZFSSnapshotInfo *info = NULL;
    guint num_fields;
    const gchar *at_sign = NULL;

    if (!line || strlen (line) == 0)
        return NULL;

    fields = g_strsplit (line, "\t", -1);
    num_fields = g_strv_length (fields);

    if (num_fields < 4) {
        g_strfreev (fields);
        return NULL;
    }

    info = g_new0 (BDZFSSnapshotInfo, 1);

    /* [0] full name (e.g. "pool/data@snap1") */
    info->name = g_strdup (fields[0]);

    /* Extract dataset from name: everything before '@' */
    at_sign = strchr (fields[0], '@');
    if (at_sign)
        info->dataset = g_strndup (fields[0], at_sign - fields[0]);
    else
        info->dataset = g_strdup (fields[0]);

    /* [1] used (guint64) */
    info->used = g_ascii_strtoull (fields[1], NULL, 10);

    /* [2] referenced (guint64) */
    info->referenced = g_ascii_strtoull (fields[2], NULL, 10);

    /* [3] creation (string) */
    info->creation = g_strdup (fields[3]);

    g_strfreev (fields);
    return info;
}

/**
 * bd_zfs_snapshot_create:
 * @name: full snapshot name (dataset@snapname)
 * @recursive: whether to recursively snapshot all descendant datasets
 * @extra: (nullable) (array zero-terminated=1): extra options for snapshot creation
 * @error: (out) (optional): place to store error (if any)
 *
 * Creates a new ZFS snapshot.
 *
 * Returns: whether the snapshot was successfully created or not
 *
 * Tech category: %BD_ZFS_TECH_SNAPSHOT-%BD_ZFS_TECH_MODE_CREATE
 */
gboolean bd_zfs_snapshot_create (const gchar *name, gboolean recursive,
                                  const BDExtraArg **extra, GError **error) {
    const gchar *argv[6] = {NULL};
    guint next_arg = 0;

    if (!validate_name_not_option (name, "Snapshot name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zfs";
    argv[next_arg++] = "snapshot";
    if (recursive)
        argv[next_arg++] = "-r";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    argv[next_arg] = NULL;

    return bd_utils_exec_and_report_error (argv, extra, error);
}

/**
 * bd_zfs_snapshot_destroy:
 * @name: full snapshot name (dataset@snapname)
 * @recursive: whether to recursively destroy all dependent snapshots
 * @error: (out) (optional): place to store error (if any)
 *
 * Destroys a ZFS snapshot.
 *
 * Returns: whether the snapshot was successfully destroyed or not
 *
 * Tech category: %BD_ZFS_TECH_SNAPSHOT-%BD_ZFS_TECH_MODE_DELETE
 */
gboolean bd_zfs_snapshot_destroy (const gchar *name, gboolean recursive, GError **error) {
    const gchar *argv[6] = {NULL};
    guint next_arg = 0;

    if (!validate_name_not_option (name, "Snapshot name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zfs";
    argv[next_arg++] = "destroy";
    if (recursive)
        argv[next_arg++] = "-r";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    argv[next_arg] = NULL;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_snapshot_list:
 * @dataset: (nullable): dataset to list snapshots for, or %NULL for all
 * @recursive: whether to list snapshots recursively
 * @error: (out) (optional): place to store error (if any)
 *
 * Lists ZFS snapshots for the given dataset or all snapshots.
 *
 * Returns: (array zero-terminated=1) (transfer full): a NULL-terminated array of
 *          #BDZFSSnapshotInfo or %NULL in case of error
 *
 * Tech category: %BD_ZFS_TECH_SNAPSHOT-%BD_ZFS_TECH_MODE_QUERY
 */
BDZFSSnapshotInfo** bd_zfs_snapshot_list (const gchar *dataset, gboolean recursive, GError **error) {
    const gchar **argv = NULL;
    guint next_arg = 0;
    guint num_args;
    gchar *output = NULL;
    gboolean success;
    gchar **lines = NULL;
    gchar **line_p = NULL;
    GPtrArray *snap_infos;
    BDZFSSnapshotInfo *info = NULL;

    if (dataset && !validate_name_not_option (dataset, "Dataset", error))
        return NULL;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return NULL;

    /* zfs list -H -p -t snapshot [-r] -o name,used,refer,creation [-- dataset] NULL */
    num_args = 8 + (recursive ? 1 : 0) + (dataset ? 2 : 0) + 1;
    argv = g_new0 (const gchar*, num_args);

    argv[next_arg++] = "zfs";
    argv[next_arg++] = "list";
    argv[next_arg++] = "-H";
    argv[next_arg++] = "-p";
    argv[next_arg++] = "-t";
    argv[next_arg++] = "snapshot";
    if (recursive)
        argv[next_arg++] = "-r";
    argv[next_arg++] = "-o";
    argv[next_arg++] = "name,used,refer,creation";
    if (dataset) {
        argv[next_arg++] = "--";
        argv[next_arg++] = dataset;
    }
    argv[next_arg] = NULL;

    success = bd_utils_exec_and_capture_output (argv, NULL, &output, error);
    g_free (argv);
    if (!success) {
        g_free (output);
        return NULL;
    }

    lines = g_strsplit (output, "\n", -1);
    g_free (output);

    snap_infos = g_ptr_array_new ();
    for (line_p = lines; *line_p; line_p++) {
        if (strlen (*line_p) == 0)
            continue;
        info = parse_snapshot_info_line (*line_p);
        if (info)
            g_ptr_array_add (snap_infos, info);
    }
    g_strfreev (lines);

    g_ptr_array_add (snap_infos, NULL);
    return (BDZFSSnapshotInfo **) g_ptr_array_free (snap_infos, FALSE);
}

/**
 * bd_zfs_snapshot_rollback:
 * @name: full snapshot name (dataset@snapname)
 * @force: whether to force unmount of any clones
 * @destroy_newer: whether to destroy any snapshots more recent than the given one
 * @error: (out) (optional): place to store error (if any)
 *
 * Rolls back a ZFS dataset to the given snapshot.
 *
 * Returns: whether the rollback was successful or not
 *
 * Tech category: %BD_ZFS_TECH_SNAPSHOT-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_snapshot_rollback (const gchar *name, gboolean force, gboolean destroy_newer, GError **error) {
    const gchar *argv[7] = {NULL};
    guint next_arg = 0;

    if (!validate_name_not_option (name, "Snapshot name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zfs";
    argv[next_arg++] = "rollback";
    if (force)
        argv[next_arg++] = "-f";
    if (destroy_newer)
        argv[next_arg++] = "-r";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    argv[next_arg] = NULL;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_snapshot_clone:
 * @snapshot: full snapshot name (dataset@snapname) to clone from
 * @clone_name: name for the new clone dataset
 * @extra: (nullable) (array zero-terminated=1): extra options for clone creation
 * @error: (out) (optional): place to store error (if any)
 *
 * Creates a clone dataset from a ZFS snapshot.
 *
 * Returns: whether the clone was successfully created or not
 *
 * Tech category: %BD_ZFS_TECH_SNAPSHOT-%BD_ZFS_TECH_MODE_CREATE
 */
gboolean bd_zfs_snapshot_clone (const gchar *snapshot, const gchar *clone_name,
                                 const BDExtraArg **extra, GError **error) {
    const gchar *argv[] = {"zfs", "clone", "--", snapshot, clone_name, NULL};

    if (!validate_name_not_option (snapshot, "Snapshot name", error))
        return FALSE;

    if (!validate_name_not_option (clone_name, "Clone name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    return bd_utils_exec_and_report_error (argv, extra, error);
}

/**
 * bd_zfs_bookmark_create:
 * @snapshot: full snapshot name (dataset@snapname) to bookmark
 * @bookmark: full bookmark name (dataset#bookmark)
 * @error: (out) (optional): place to store error (if any)
 *
 * Creates a ZFS bookmark from an existing snapshot.
 *
 * Returns: whether the bookmark was successfully created or not
 *
 * Tech category: %BD_ZFS_TECH_SNAPSHOT-%BD_ZFS_TECH_MODE_CREATE
 */
gboolean bd_zfs_bookmark_create (const gchar *snapshot, const gchar *bookmark, GError **error) {
    const gchar *argv[] = {"zfs", "bookmark", "--", snapshot, bookmark, NULL};

    if (!validate_name_not_option (snapshot, "Snapshot name", error))
        return FALSE;

    if (!validate_name_not_option (bookmark, "Bookmark name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_bookmark_destroy:
 * @name: full bookmark name (dataset#bookmark)
 * @error: (out) (optional): place to store error (if any)
 *
 * Destroys a ZFS bookmark.
 *
 * Returns: whether the bookmark was successfully destroyed or not
 *
 * Tech category: %BD_ZFS_TECH_SNAPSHOT-%BD_ZFS_TECH_MODE_DELETE
 */
gboolean bd_zfs_bookmark_destroy (const gchar *name, GError **error) {
    const gchar *argv[] = {"zfs", "destroy", "--", name, NULL};

    if (!validate_name_not_option (name, "Bookmark name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_bookmark_list:
 * @dataset: (nullable): dataset to list bookmarks for, or %NULL for all
 * @error: (out) (optional): place to store error (if any)
 *
 * Lists ZFS bookmarks for the given dataset or all bookmarks.
 *
 * Returns: (array zero-terminated=1) (transfer full): a NULL-terminated array of
 *          #BDZFSPropertyInfo or %NULL in case of error
 *
 * Tech category: %BD_ZFS_TECH_SNAPSHOT-%BD_ZFS_TECH_MODE_QUERY
 */
BDZFSPropertyInfo** bd_zfs_bookmark_list (const gchar *dataset, GError **error) {
    const gchar **argv = NULL;
    guint next_arg = 0;
    guint num_args;
    gchar *output = NULL;
    gboolean success;
    gchar **lines = NULL;
    gchar **line_p = NULL;
    GPtrArray *bookmarks;

    if (dataset && !validate_name_not_option (dataset, "Dataset", error))
        return NULL;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return NULL;

    /* zfs list -H -p -t bookmark -o name,creation [-- dataset] NULL */
    num_args = 8 + (dataset ? 2 : 0) + 1;
    argv = g_new0 (const gchar*, num_args);

    argv[next_arg++] = "zfs";
    argv[next_arg++] = "list";
    argv[next_arg++] = "-H";
    argv[next_arg++] = "-p";
    argv[next_arg++] = "-t";
    argv[next_arg++] = "bookmark";
    argv[next_arg++] = "-o";
    argv[next_arg++] = "name,creation";
    if (dataset) {
        argv[next_arg++] = "--";
        argv[next_arg++] = dataset;
    }
    argv[next_arg] = NULL;

    success = bd_utils_exec_and_capture_output (argv, NULL, &output, error);
    g_free (argv);
    if (!success) {
        g_free (output);
        return NULL;
    }

    lines = g_strsplit (output, "\n", -1);
    g_free (output);

    bookmarks = g_ptr_array_new ();
    for (line_p = lines; *line_p; line_p++) {
        gchar **fields = NULL;
        guint num_fields;
        BDZFSPropertyInfo *info = NULL;

        if (strlen (*line_p) == 0)
            continue;

        fields = g_strsplit (*line_p, "\t", -1);
        num_fields = g_strv_length (fields);

        if (num_fields < 2) {
            g_strfreev (fields);
            continue;
        }

        info = g_new0 (BDZFSPropertyInfo, 1);
        info->name = g_strdup (fields[0]);
        info->value = g_strdup (fields[1]);
        info->source = g_strdup ("");

        g_strfreev (fields);
        g_ptr_array_add (bookmarks, info);
    }
    g_strfreev (lines);

    g_ptr_array_add (bookmarks, NULL);
    return (BDZFSPropertyInfo **) g_ptr_array_free (bookmarks, FALSE);
}

/**
 * bd_zfs_encryption_load_key:
 * @dataset: name of the encrypted dataset to load the key for
 * @key_location: (nullable): key location URI (e.g. "file:///path/to/keyfile") or %NULL to use the dataset's keylocation property
 * @error: (out) (optional): place to store error (if any)
 *
 * Loads the encryption key for a ZFS dataset. If @key_location is %NULL, the
 * dataset's own keylocation property is used. If @key_location starts with
 * "file://", it is passed directly as the key location via -L. Otherwise the
 * value is treated as a passphrase and piped to ``zfs load-key`` via stdin
 * using ``-L prompt``.
 *
 * Returns: whether the key was successfully loaded or not
 *
 * Tech category: %BD_ZFS_TECH_ENCRYPTION-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_encryption_load_key (const gchar *dataset, const gchar *key_location, GError **error) {
    if (!validate_name_not_option (dataset, "Dataset name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    if (key_location == NULL) {
        /* Use the dataset's own keylocation property */
        const gchar *argv[] = {"zfs", "load-key", "--", dataset, NULL};
        return bd_utils_exec_and_report_error (argv, NULL, error);
    } else if (g_str_has_prefix (key_location, "file://")) {
        /* File-based key location — pass directly via -L */
        const gchar *argv[] = {"zfs", "load-key", "-L", key_location, "--", dataset, NULL};
        return bd_utils_exec_and_report_error (argv, NULL, error);
    } else {
        /* Passphrase string — pipe via stdin with -L prompt */
        const gchar *argv[] = {"zfs", "load-key", "-L", "prompt", "--", dataset, NULL};
        return bd_utils_exec_with_input (argv, key_location, NULL, error);
    }
}

/**
 * bd_zfs_encryption_unload_key:
 * @dataset: name of the encrypted dataset to unload the key for
 * @error: (out) (optional): place to store error (if any)
 *
 * Unloads the encryption key for a ZFS dataset, making it inaccessible
 * until the key is loaded again.
 *
 * Returns: whether the key was successfully unloaded or not
 *
 * Tech category: %BD_ZFS_TECH_ENCRYPTION-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_encryption_unload_key (const gchar *dataset, GError **error) {
    const gchar *argv[] = {"zfs", "unload-key", "--", dataset, NULL};

    if (!validate_name_not_option (dataset, "Dataset name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_encryption_change_key:
 * @dataset: name of the encrypted dataset to change the key for
 * @new_key_location: (nullable): new key location URI (e.g. "file:///path/to/keyfile") or %NULL to inherit from parent
 * @extra: (nullable) (array zero-terminated=1): extra options for the key change operation
 * @error: (out) (optional): place to store error (if any)
 *
 * Changes the encryption key or key location for a ZFS dataset. If
 * @new_key_location is %NULL, the ``-i`` flag is used to inherit the
 * key from the parent dataset. If @new_key_location starts with "file://",
 * it is passed as the new key location via ``-l``.
 *
 * Returns: whether the key was successfully changed or not
 *
 * Tech category: %BD_ZFS_TECH_ENCRYPTION-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_encryption_change_key (const gchar *dataset, const gchar *new_key_location,
                                        const BDExtraArg **extra, GError **error) {
    if (!validate_name_not_option (dataset, "Dataset name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    if (new_key_location == NULL) {
        /* Inherit key from parent */
        const gchar *argv[] = {"zfs", "change-key", "-i", "--", dataset, NULL};
        return bd_utils_exec_and_report_error (argv, extra, error);
    } else {
        /* Set new key location via -o keylocation=<value> */
        gchar *opt = g_strdup_printf ("keylocation=%s", new_key_location);
        const gchar *argv[] = {"zfs", "change-key", "-o", opt, "--", dataset, NULL};
        gboolean ret = bd_utils_exec_and_report_error (argv, extra, error);
        g_free (opt);
        return ret;
    }
}

/**
 * bd_zfs_encryption_key_status:
 * @dataset: name of the dataset to query key status for
 * @error: (out) (optional): place to store error (if any)
 *
 * Queries the encryption key status of a ZFS dataset by reading the
 * ``keystatus`` property.
 *
 * Returns: the #BDZFSKeyStatus of the dataset, or %BD_ZFS_KEY_STATUS_NONE on error
 *
 * Tech category: %BD_ZFS_TECH_ENCRYPTION-%BD_ZFS_TECH_MODE_QUERY
 */
BDZFSKeyStatus bd_zfs_encryption_key_status (const gchar *dataset, GError **error) {
    const gchar *argv[] = {"zfs", "get", "-H", "-p", "-o", "value", "keystatus", "--", dataset, NULL};
    gchar *output = NULL;
    gboolean success;
    gchar *value = NULL;
    BDZFSKeyStatus ret = BD_ZFS_KEY_STATUS_NONE;

    if (!validate_name_not_option (dataset, "Dataset name", error))
        return BD_ZFS_KEY_STATUS_NONE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return BD_ZFS_KEY_STATUS_NONE;

    success = bd_utils_exec_and_capture_output (argv, NULL, &output, error);
    if (!success) {
        g_free (output);
        return BD_ZFS_KEY_STATUS_NONE;
    }

    /* Strip trailing whitespace/newlines */
    value = g_strstrip (output);

    if (g_strcmp0 (value, "available") == 0)
        ret = BD_ZFS_KEY_STATUS_AVAILABLE;
    else if (g_strcmp0 (value, "unavailable") == 0)
        ret = BD_ZFS_KEY_STATUS_UNAVAILABLE;
    else
        ret = BD_ZFS_KEY_STATUS_NONE;

    g_free (output);
    return ret;
}

/**
 * bd_zfs_zvol_create:
 * @name: name of the zvol to create (e.g. "pool/zvol")
 * @size: size of the zvol in bytes
 * @sparse: whether to create a sparse (thin provisioned) zvol
 * @extra: (nullable) (array zero-terminated=1): extra options for zvol creation
 * @error: (out) (optional): place to store error (if any)
 *
 * Creates a new ZFS zvol (block volume) with the given name and size.
 *
 * Returns: whether the zvol was successfully created or not
 *
 * Tech category: %BD_ZFS_TECH_ZVOL-%BD_ZFS_TECH_MODE_CREATE
 */
gboolean bd_zfs_zvol_create (const gchar *name, guint64 size, gboolean sparse, const BDExtraArg **extra, GError **error) {
    const gchar **argv = NULL;
    guint num_args = 0;
    guint next_arg = 0;
    gchar *size_str = NULL;
    gboolean success = FALSE;

    if (!validate_name_not_option (name, "Zvol name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    /* zfs create -V <size> [-s] -- <name> NULL */
    num_args = 6 + (sparse ? 1 : 0) + 1;
    argv = g_new0 (const gchar*, num_args);

    size_str = g_strdup_printf ("%" G_GUINT64_FORMAT, size);

    argv[next_arg++] = "zfs";
    argv[next_arg++] = "create";
    argv[next_arg++] = "-V";
    argv[next_arg++] = size_str;
    if (sparse)
        argv[next_arg++] = "-s";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;

    success = bd_utils_exec_and_report_error (argv, extra, error);
    g_free (size_str);
    g_free (argv);
    return success;
}

/**
 * bd_zfs_zvol_destroy:
 * @name: name of the zvol to destroy
 * @recursive: whether to recursively destroy all dependents
 * @force: whether to force the destruction
 * @error: (out) (optional): place to store error (if any)
 *
 * Destroys the ZFS zvol with the given name.
 *
 * Returns: whether the zvol was successfully destroyed or not
 *
 * Tech category: %BD_ZFS_TECH_ZVOL-%BD_ZFS_TECH_MODE_DELETE
 */
gboolean bd_zfs_zvol_destroy (const gchar *name, gboolean recursive, gboolean force, GError **error) {
    const gchar *argv[7] = {NULL};
    guint next_arg = 0;

    if (!validate_name_not_option (name, "Zvol name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zfs";
    argv[next_arg++] = "destroy";
    if (recursive)
        argv[next_arg++] = "-r";
    if (force)
        argv[next_arg++] = "-f";
    argv[next_arg++] = "--";
    argv[next_arg++] = name;
    argv[next_arg] = NULL;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_zvol_resize:
 * @name: name of the zvol to resize
 * @new_size: new size for the zvol in bytes
 * @error: (out) (optional): place to store error (if any)
 *
 * Resizes a ZFS zvol to the given size.
 *
 * Returns: whether the zvol was successfully resized or not
 *
 * Tech category: %BD_ZFS_TECH_ZVOL-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_zvol_resize (const gchar *name, guint64 new_size, GError **error) {
    gchar *volsize_str = NULL;
    const gchar *argv[5] = {NULL};
    gboolean success;

    if (!validate_name_not_option (name, "Zvol name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    volsize_str = g_strdup_printf ("volsize=%" G_GUINT64_FORMAT, new_size);

    argv[0] = "zfs";
    argv[1] = "set";
    argv[2] = volsize_str;
    argv[3] = name;
    argv[4] = NULL;

    success = bd_utils_exec_and_report_error (argv, NULL, error);
    g_free (volsize_str);
    return success;
}

/**
 * bd_zfs_snapshot_promote:
 * @clone_name: name of the clone dataset to promote
 * @error: (out) (optional): place to store error (if any)
 *
 * Promotes a ZFS clone dataset to no longer depend on its origin snapshot.
 *
 * Returns: whether the clone was successfully promoted or not
 *
 * Tech category: %BD_ZFS_TECH_SNAPSHOT-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_snapshot_promote (const gchar *clone_name, GError **error) {
    const gchar *argv[] = {"zfs", "promote", "--", clone_name, NULL};

    if (!validate_name_not_option (clone_name, "Clone name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_snapshot_hold:
 * @snapshot: full snapshot name (dataset@snapname) to hold
 * @tag: user-defined tag for the hold
 * @error: (out) (optional): place to store error (if any)
 *
 * Places a hold with the given tag on a ZFS snapshot, preventing it from being destroyed.
 *
 * Returns: whether the hold was successfully placed or not
 *
 * Tech category: %BD_ZFS_TECH_SNAPSHOT-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_snapshot_hold (const gchar *snapshot, const gchar *tag, GError **error) {
    const gchar *argv[] = {"zfs", "hold", "--", tag, snapshot, NULL};

    if (!validate_name_not_option (snapshot, "Snapshot name", error))
        return FALSE;

    if (!validate_name_not_option (tag, "Hold tag", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_snapshot_release:
 * @snapshot: full snapshot name (dataset@snapname) to release
 * @tag: user-defined tag to release
 * @error: (out) (optional): place to store error (if any)
 *
 * Releases a hold with the given tag from a ZFS snapshot.
 *
 * Returns: whether the hold was successfully released or not
 *
 * Tech category: %BD_ZFS_TECH_SNAPSHOT-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_snapshot_release (const gchar *snapshot, const gchar *tag, GError **error) {
    const gchar *argv[] = {"zfs", "release", "--", tag, snapshot, NULL};

    if (!validate_name_not_option (snapshot, "Snapshot name", error))
        return FALSE;

    if (!validate_name_not_option (tag, "Hold tag", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_dataset_inherit_property:
 * @name: name of the dataset
 * @property: name of the property to inherit
 * @error: (out) (optional): place to store error (if any)
 *
 * Clears the specified property on a ZFS dataset, causing it to be inherited from its parent.
 *
 * Returns: whether the property was successfully inherited or not
 *
 * Tech category: %BD_ZFS_TECH_DATASET-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_dataset_inherit_property (const gchar *name, const gchar *property, GError **error) {
    const gchar *argv[] = {"zfs", "inherit", "--", property, name, NULL};

    if (!validate_name_not_option (name, "Dataset name", error))
        return FALSE;

    if (!validate_name_not_option (property, "Property name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_pool_upgrade:
 * @name: name of the pool to upgrade
 * @error: (out) (optional): place to store error (if any)
 *
 * Upgrades a ZFS pool to the latest on-disk version.
 *
 * Returns: whether the pool was successfully upgraded or not
 *
 * Tech category: %BD_ZFS_TECH_POOL-%BD_ZFS_TECH_MODE_MODIFY
 */
gboolean bd_zfs_pool_upgrade (const gchar *name, GError **error) {
    const gchar *argv[] = {"zpool", "upgrade", "--", name, NULL};

    if (!validate_name_not_option (name, "Pool name", error))
        return FALSE;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}

/**
 * bd_zfs_pool_history:
 * @name: name of the pool to query history for
 * @error: (out) (optional): place to store error (if any)
 *
 * Retrieves the command history of a ZFS pool.
 *
 * Returns: (transfer full): the raw history output as a string, or %NULL on error
 *
 * Tech category: %BD_ZFS_TECH_POOL-%BD_ZFS_TECH_MODE_QUERY
 */
gchar* bd_zfs_pool_history (const gchar *name, GError **error) {
    const gchar *argv[] = {"zpool", "history", "--", name, NULL};
    gchar *output = NULL;
    gboolean success;

    if (!validate_name_not_option (name, "Pool name", error))
        return NULL;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return NULL;

    success = bd_utils_exec_and_capture_output (argv, NULL, &output, error);
    if (!success) {
        g_free (output);
        return NULL;
    }

    return output;
}
