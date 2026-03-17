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

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    if (!vdevs || !vdevs[0]) {
        g_set_error_literal (error, BD_ZFS_ERROR, BD_ZFS_ERROR_FAIL, "No vdevs given");
        return FALSE;
    }

    for (vdev_p = vdevs; *vdev_p != NULL; vdev_p++)
        num_vdevs++;

    /* zpool create [-f] <name> [raid_level] <vdev1> ... <vdevN> NULL */
    num_args = 3 + num_vdevs + (raid_level ? 1 : 0) + 1;
    argv = g_new0 (const gchar*, num_args);

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "create";
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
    const gchar *argv[5] = {NULL};
    guint next_arg = 0;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "destroy";
    if (force)
        argv[next_arg++] = "-f";
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
    const gchar *argv[5] = {NULL};
    guint next_arg = 0;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "export";
    if (force)
        argv[next_arg++] = "-f";
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

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    if (search_dirs) {
        for (dir_p = search_dirs; *dir_p != NULL; dir_p++)
            num_dirs++;
    }

    /* zpool import [-f] [-d dir1 -d dir2 ...] <name_or_guid> [new_name] NULL */
    num_args = 3 + (force ? 1 : 0) + (num_dirs * 2) + (new_name ? 1 : 0) + 1;
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
                           name, NULL};
    gchar *output = NULL;
    gboolean success;
    gchar **lines = NULL;
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
    const gchar *argv[] = {"zpool", "status", "-P", name, NULL};
    gchar *output = NULL;
    gboolean success;
    gchar **lines = NULL;
    gchar **line_p = NULL;
    gboolean in_config = FALSE;
    gboolean header_skipped = FALSE;

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

    for (line_p = lines; *line_p; line_p++) {
        gchar *line = *line_p;

        /* Look for the "config:" line to start parsing */
        if (!in_config) {
            gchar *stripped = g_strstrip (g_strdup (line));
            if (g_strcmp0 (stripped, "config:") == 0)
                in_config = TRUE;
            g_free (stripped);
            continue;
        }

        /* Skip empty lines */
        if (strlen (line) == 0)
            continue;

        /* Skip the header line (NAME STATE READ WRITE CKSUM) */
        if (!header_skipped) {
            gchar *stripped = g_strstrip (g_strdup (line));
            if (g_str_has_prefix (stripped, "NAME")) {
                g_free (stripped);
                header_skipped = TRUE;
                continue;
            }
            g_free (stripped);
            /* If no header found, skip blank lines */
            continue;
        }

        /* Stop parsing at "errors:" line */
        {
            gchar *stripped = g_strstrip (g_strdup (line));
            if (g_str_has_prefix (stripped, "errors:")) {
                g_free (stripped);
                break;
            }
            g_free (stripped);
        }

        /* Count leading tabs to determine indent level */
        gint indent = 0;
        const gchar *p = line;
        while (*p == '\t') {
            indent++;
            p++;
        }

        /* Skip fully blank lines after stripping */
        if (*p == '\0')
            continue;

        /* Parse the fields: name state read write cksum (whitespace-separated) */
        gchar **fields = g_regex_split_simple ("\\s+", p, 0, 0);
        guint num_fields = g_strv_length (fields);

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
        if (stack_top < 64) {
            stack[stack_top] = vdev;
            stack_indent[stack_top] = indent;
        }
    }

    g_strfreev (lines);

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

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    if (!vdevs || !vdevs[0]) {
        g_set_error_literal (error, BD_ZFS_ERROR, BD_ZFS_ERROR_FAIL, "No vdevs given");
        return FALSE;
    }

    for (vdev_p = vdevs; *vdev_p != NULL; vdev_p++)
        num_vdevs++;

    /* zpool add [-f] <name> [raid_level] <vdev1> ... <vdevN> NULL */
    num_args = 3 + num_vdevs + (raid_level ? 1 : 0) + 1;
    argv = g_new0 (const gchar*, num_args);

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "add";
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
    const gchar *argv[5] = {NULL};
    guint next_arg = 0;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "remove";
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
    const gchar *argv[6] = {NULL};
    guint next_arg = 0;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "attach";
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
    const gchar *argv[5] = {NULL};
    guint next_arg = 0;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "detach";
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
    const gchar *argv[7] = {NULL};
    guint next_arg = 0;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "replace";
    if (force)
        argv[next_arg++] = "-f";
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
    const gchar *argv[6] = {NULL};
    guint next_arg = 0;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "online";
    if (expand)
        argv[next_arg++] = "-e";
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
    const gchar *argv[6] = {NULL};
    guint next_arg = 0;

    if (!check_deps (&avail_deps, DEPS_ZPOOL_MASK | DEPS_ZFS_MASK, deps, DEPS_LAST, &deps_check_lock, error))
        return FALSE;

    argv[next_arg++] = "zpool";
    argv[next_arg++] = "offline";
    if (temporary)
        argv[next_arg++] = "-t";
    argv[next_arg++] = name;
    argv[next_arg++] = vdev;
    argv[next_arg] = NULL;

    return bd_utils_exec_and_report_error (argv, NULL, error);
}
