#include <glib.h>
#include <glib-object.h>
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

typedef enum {
    BD_ZFS_POOL_STATE_ONLINE,
    BD_ZFS_POOL_STATE_DEGRADED,
    BD_ZFS_POOL_STATE_FAULTED,
    BD_ZFS_POOL_STATE_OFFLINE,
    BD_ZFS_POOL_STATE_REMOVED,
    BD_ZFS_POOL_STATE_UNAVAIL,
    BD_ZFS_POOL_STATE_UNKNOWN,
} BDZFSPoolState;

typedef enum {
    BD_ZFS_VDEV_TYPE_DISK,
    BD_ZFS_VDEV_TYPE_FILE,
    BD_ZFS_VDEV_TYPE_MIRROR,
    BD_ZFS_VDEV_TYPE_RAIDZ1,
    BD_ZFS_VDEV_TYPE_RAIDZ2,
    BD_ZFS_VDEV_TYPE_RAIDZ3,
    BD_ZFS_VDEV_TYPE_DRAID,
    BD_ZFS_VDEV_TYPE_SPARE,
    BD_ZFS_VDEV_TYPE_LOG,
    BD_ZFS_VDEV_TYPE_CACHE,
    BD_ZFS_VDEV_TYPE_SPECIAL,
    BD_ZFS_VDEV_TYPE_DEDUP,
    BD_ZFS_VDEV_TYPE_ROOT,
    BD_ZFS_VDEV_TYPE_UNKNOWN,
} BDZFSVdevType;

typedef enum {
    BD_ZFS_VDEV_STATE_ONLINE,
    BD_ZFS_VDEV_STATE_DEGRADED,
    BD_ZFS_VDEV_STATE_FAULTED,
    BD_ZFS_VDEV_STATE_OFFLINE,
    BD_ZFS_VDEV_STATE_REMOVED,
    BD_ZFS_VDEV_STATE_UNAVAIL,
    BD_ZFS_VDEV_STATE_UNKNOWN,
} BDZFSVdevState;

typedef enum {
    BD_ZFS_DATASET_TYPE_FILESYSTEM,
    BD_ZFS_DATASET_TYPE_VOLUME,
    BD_ZFS_DATASET_TYPE_SNAPSHOT,
    BD_ZFS_DATASET_TYPE_BOOKMARK,
    BD_ZFS_DATASET_TYPE_UNKNOWN,
} BDZFSDatasetType;

typedef enum {
    BD_ZFS_KEY_STATUS_NONE,
    BD_ZFS_KEY_STATUS_AVAILABLE,
    BD_ZFS_KEY_STATUS_UNAVAILABLE,
    BD_ZFS_KEY_STATUS_UNKNOWN,
} BDZFSKeyStatus;

typedef enum {
    BD_ZFS_SCRUB_STATE_NONE,
    BD_ZFS_SCRUB_STATE_SCANNING,
    BD_ZFS_SCRUB_STATE_FINISHED,
    BD_ZFS_SCRUB_STATE_CANCELED,
    BD_ZFS_SCRUB_STATE_PAUSED,
} BDZFSScrubState;

typedef struct BDZFSPoolInfo {
    gchar *name;
    gchar *guid;
    BDZFSPoolState state;
    guint64 size;
    guint64 allocated;
    guint64 free;
    guint64 fragmentation;
    gdouble dedup_ratio;
    gboolean readonly;
    gchar *altroot;
    gchar *ashift;
} BDZFSPoolInfo;

void bd_zfs_pool_info_free (BDZFSPoolInfo *info);
BDZFSPoolInfo* bd_zfs_pool_info_copy (BDZFSPoolInfo *info);

typedef struct BDZFSVdevInfo {
    gchar *path;
    BDZFSVdevType type;
    BDZFSVdevState state;
    guint64 alloc;
    guint64 space;
    guint64 read_errors;
    guint64 write_errors;
    guint64 checksum_errors;
    struct BDZFSVdevInfo **children;
} BDZFSVdevInfo;

void bd_zfs_vdev_info_free (BDZFSVdevInfo *info);
BDZFSVdevInfo* bd_zfs_vdev_info_copy (BDZFSVdevInfo *info);

typedef struct BDZFSDatasetInfo {
    gchar *name;
    BDZFSDatasetType type;
    gchar *mountpoint;
    gchar *origin;
    guint64 used;
    guint64 available;
    guint64 referenced;
    gchar *compression;
    gchar *encryption;
    BDZFSKeyStatus key_status;
    gboolean mounted;
} BDZFSDatasetInfo;

void bd_zfs_dataset_info_free (BDZFSDatasetInfo *info);
BDZFSDatasetInfo* bd_zfs_dataset_info_copy (BDZFSDatasetInfo *info);

typedef struct BDZFSSnapshotInfo {
    gchar *name;
    gchar *dataset;
    guint64 used;
    guint64 referenced;
    gchar *creation;
} BDZFSSnapshotInfo;

void bd_zfs_snapshot_info_free (BDZFSSnapshotInfo *info);
BDZFSSnapshotInfo* bd_zfs_snapshot_info_copy (BDZFSSnapshotInfo *info);

typedef struct BDZFSPropertyInfo {
    gchar *name;
    gchar *value;
    gchar *source;
} BDZFSPropertyInfo;

void bd_zfs_property_info_free (BDZFSPropertyInfo *info);
BDZFSPropertyInfo* bd_zfs_property_info_copy (BDZFSPropertyInfo *info);

typedef struct BDZFSScrubInfo {
    BDZFSScrubState state;
    guint64 total_bytes;
    guint64 scanned_bytes;
    guint64 issued_bytes;
    guint64 errors;
    gdouble percent_done;
    gchar *start_time;
    gchar *end_time;
} BDZFSScrubInfo;

void bd_zfs_scrub_info_free (BDZFSScrubInfo *info);
BDZFSScrubInfo* bd_zfs_scrub_info_copy (BDZFSScrubInfo *info);

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

gboolean bd_zfs_pool_create (const gchar *name, const gchar **vdevs, const gchar *raid_level,
                             const BDExtraArg **extra, GError **error);
gboolean bd_zfs_pool_destroy (const gchar *name, gboolean force, GError **error);
gboolean bd_zfs_pool_export (const gchar *name, gboolean force, GError **error);
gboolean bd_zfs_pool_import (const gchar *name_or_guid, const gchar *new_name,
                             const gchar **search_dirs, gboolean force,
                             const BDExtraArg **extra, GError **error);
BDZFSPoolInfo** bd_zfs_pool_list (GError **error);
BDZFSPoolInfo** bd_zfs_pool_list_importable (GError **error);
BDZFSPoolInfo* bd_zfs_pool_get_info (const gchar *name, GError **error);
BDZFSVdevInfo** bd_zfs_pool_get_vdevs (const gchar *name, GError **error);

gboolean bd_zfs_pool_add_vdev (const gchar *name, const gchar **vdevs, const gchar *raid_level,
                                const BDExtraArg **extra, GError **error);
gboolean bd_zfs_pool_remove_vdev (const gchar *name, const gchar *vdev, GError **error);
gboolean bd_zfs_pool_attach (const gchar *name, const gchar *existing_vdev,
                              const gchar *new_vdev, GError **error);
gboolean bd_zfs_pool_detach (const gchar *name, const gchar *vdev, GError **error);
gboolean bd_zfs_pool_replace (const gchar *name, const gchar *old_vdev, const gchar *new_vdev,
                               gboolean force, GError **error);
gboolean bd_zfs_pool_online (const gchar *name, const gchar *vdev, gboolean expand, GError **error);
gboolean bd_zfs_pool_offline (const gchar *name, const gchar *vdev, gboolean temporary, GError **error);

gboolean bd_zfs_pool_scrub_start (const gchar *name, GError **error);
gboolean bd_zfs_pool_scrub_stop (const gchar *name, GError **error);
gboolean bd_zfs_pool_scrub_pause (const gchar *name, GError **error);
BDZFSScrubInfo* bd_zfs_pool_scrub_status (const gchar *name, GError **error);

gboolean bd_zfs_pool_trim_start (const gchar *name, const gchar *vdev, GError **error);
gboolean bd_zfs_pool_trim_stop (const gchar *name, const gchar *vdev, GError **error);

gboolean bd_zfs_pool_clear (const gchar *name, const gchar *vdev, GError **error);

gboolean bd_zfs_pool_upgrade (const gchar *name, GError **error);
gchar* bd_zfs_pool_history (const gchar *name, GError **error);

BDZFSPropertyInfo* bd_zfs_pool_get_property (const gchar *name, const gchar *property, GError **error);
gboolean bd_zfs_pool_set_property (const gchar *name, const gchar *property, const gchar *value, GError **error);
BDZFSPropertyInfo** bd_zfs_pool_get_properties (const gchar *name, GError **error);

gboolean bd_zfs_dataset_create (const gchar *name, const BDExtraArg **extra, GError **error);
gboolean bd_zfs_dataset_destroy (const gchar *name, gboolean recursive, gboolean force, GError **error);
BDZFSDatasetInfo** bd_zfs_dataset_list (const gchar *pool_or_parent, gboolean recursive, GError **error);
BDZFSDatasetInfo* bd_zfs_dataset_get_info (const gchar *name, GError **error);
gboolean bd_zfs_dataset_rename (const gchar *name, const gchar *new_name, gboolean create_parent,
                                 gboolean force, GError **error);
gboolean bd_zfs_dataset_mount (const gchar *name, const gchar *mountpoint, const BDExtraArg **extra, GError **error);
gboolean bd_zfs_dataset_unmount (const gchar *name, gboolean force, GError **error);
BDZFSPropertyInfo* bd_zfs_dataset_get_property (const gchar *name, const gchar *property, GError **error);
gboolean bd_zfs_dataset_set_property (const gchar *name, const gchar *property, const gchar *value, GError **error);
gboolean bd_zfs_dataset_inherit_property (const gchar *name, const gchar *property, GError **error);
BDZFSPropertyInfo** bd_zfs_dataset_get_properties (const gchar *name, GError **error);

gboolean bd_zfs_snapshot_create (const gchar *name, gboolean recursive,
                                  const BDExtraArg **extra, GError **error);
gboolean bd_zfs_snapshot_destroy (const gchar *name, gboolean recursive, GError **error);
BDZFSSnapshotInfo** bd_zfs_snapshot_list (const gchar *dataset, gboolean recursive, GError **error);
gboolean bd_zfs_snapshot_rollback (const gchar *name, gboolean force, gboolean destroy_newer, GError **error);
gboolean bd_zfs_snapshot_clone (const gchar *snapshot, const gchar *clone_name,
                                 const BDExtraArg **extra, GError **error);
gboolean bd_zfs_snapshot_promote (const gchar *clone_name, GError **error);
gboolean bd_zfs_snapshot_hold (const gchar *snapshot, const gchar *tag, GError **error);
gboolean bd_zfs_snapshot_release (const gchar *snapshot, const gchar *tag, GError **error);

gboolean bd_zfs_bookmark_create (const gchar *snapshot, const gchar *bookmark, GError **error);
gboolean bd_zfs_bookmark_destroy (const gchar *name, GError **error);
BDZFSPropertyInfo** bd_zfs_bookmark_list (const gchar *dataset, GError **error);

gboolean bd_zfs_encryption_load_key (const gchar *dataset, const gchar *key_location, GError **error);
gboolean bd_zfs_encryption_unload_key (const gchar *dataset, GError **error);
gboolean bd_zfs_encryption_change_key (const gchar *dataset, const gchar *new_key_location,
                                        const BDExtraArg **extra, GError **error);
BDZFSKeyStatus bd_zfs_encryption_key_status (const gchar *dataset, GError **error);

gboolean bd_zfs_zvol_create (const gchar *name, guint64 size, gboolean sparse,
                              const BDExtraArg **extra, GError **error);
gboolean bd_zfs_zvol_destroy (const gchar *name, gboolean recursive, gboolean force, GError **error);
gboolean bd_zfs_zvol_resize (const gchar *name, guint64 new_size, GError **error);

const gchar* bd_zfs_get_zfs_version (GError **error);

#endif  /* BD_ZFS */
