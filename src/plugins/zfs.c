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
