#include <glib.h>
#include <blockdev/utils.h>

#ifndef BD_FS_ZFS
#define BD_FS_ZFS

typedef struct BDFSZfsInfo {
    gchar *label;
    gchar *uuid;
    guint64 size;
    guint64 free_space;
} BDFSZfsInfo;

BDFSZfsInfo* bd_fs_zfs_info_copy (BDFSZfsInfo *data);
void bd_fs_zfs_info_free (BDFSZfsInfo *data);

gboolean bd_fs_zfs_check_label (const gchar *label, GError **error);
gboolean bd_fs_zfs_check_uuid (const gchar *uuid, GError **error);
BDFSZfsInfo* bd_fs_zfs_get_info (const gchar *device, GError **error);

#endif  /* BD_FS_ZFS */
