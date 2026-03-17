#include <glib.h>
#include <blockdev/utils.h>

#ifndef BD_ZFS
#define BD_ZFS

GQuark bd_zfs_error_quark (void);
#define BD_ZFS_ERROR bd_zfs_error_quark ()
typedef enum {
    BD_ZFS_ERROR_TECH_UNAVAIL,
    BD_ZFS_ERROR_POOL,
    BD_ZFS_ERROR_DATASET,
    BD_ZFS_ERROR_SNAPSHOT,
    BD_ZFS_ERROR_PARSE,
    BD_ZFS_ERROR_FAIL,
    BD_ZFS_ERROR_NOEXIST,
    BD_ZFS_ERROR_BUSY,
    BD_ZFS_ERROR_ENCRYPT,
} BDZFSError;

typedef enum {
    BD_ZFS_TECH_POOL = 0,
    BD_ZFS_TECH_VDEV,
    BD_ZFS_TECH_DATASET,
    BD_ZFS_TECH_SNAPSHOT,
    BD_ZFS_TECH_ENCRYPTION,
    BD_ZFS_TECH_ZVOL,
    BD_ZFS_TECH_MAINTENANCE,
} BDZFSTech;

typedef enum {
    BD_ZFS_TECH_MODE_CREATE = 1 << 0,
    BD_ZFS_TECH_MODE_DELETE = 1 << 1,
    BD_ZFS_TECH_MODE_MODIFY = 1 << 2,
    BD_ZFS_TECH_MODE_QUERY  = 1 << 3,
} BDZFSTechMode;

/*
 * If using the plugin as a standalone library, the following functions should
 * be called to:
 *
 * init()       - initialize the plugin, returning TRUE on success
 * close()      - clean after the plugin at the end or if no longer used
 *
 */
gboolean bd_zfs_init (void);
void bd_zfs_close (void);

gboolean bd_zfs_is_tech_avail (BDZFSTech tech, guint64 mode, GError **error);

#endif  /* BD_ZFS */
