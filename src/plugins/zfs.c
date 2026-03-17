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
