from .fs_test import FSNoDevTestCase

import overrides_hack
import utils
from utils import TestTags, tag_test

from gi.repository import BlockDev, GLib


class ZfsNoDevTestCase(FSNoDevTestCase):
    pass


class ZfsTestAvailability(ZfsNoDevTestCase):

    def test_zfs_available(self):
        """Verify that it is possible to check ZFS tech availability"""
        try:
            available = BlockDev.fs_is_tech_avail(BlockDev.FSTech.ZFS,
                                                  BlockDev.FSTechMode.QUERY |
                                                  BlockDev.FSTechMode.SET_LABEL)
            self.assertTrue(available)
        except GLib.GError:
            self.skipTest("skipping ZFS: zpool or zdb not available")

    def test_zfs_unsupported_modes(self):
        """Verify that unsupported modes are rejected"""
        with self.assertRaisesRegex(GLib.GError, "not supported"):
            BlockDev.fs_is_tech_avail(BlockDev.FSTech.ZFS, BlockDev.FSTechMode.MKFS)

        with self.assertRaisesRegex(GLib.GError, "not supported"):
            BlockDev.fs_is_tech_avail(BlockDev.FSTech.ZFS, BlockDev.FSTechMode.CHECK)

        with self.assertRaisesRegex(GLib.GError, "not supported"):
            BlockDev.fs_is_tech_avail(BlockDev.FSTech.ZFS, BlockDev.FSTechMode.REPAIR)

        with self.assertRaisesRegex(GLib.GError, "not supported"):
            BlockDev.fs_is_tech_avail(BlockDev.FSTech.ZFS, BlockDev.FSTechMode.RESIZE)

    def test_zfs_zdb_dep(self):
        """Verify that zdb is required for ZFS QUERY and SET_LABEL modes"""
        BlockDev.reinit(self.requested_plugins, True, None)

        # without zdb, QUERY should fail
        with utils.fake_path(all_but="zdb"):
            with self.assertRaisesRegex(GLib.GError, "The 'zdb' utility is not available"):
                BlockDev.fs_is_tech_avail(BlockDev.FSTech.ZFS, BlockDev.FSTechMode.QUERY)

        # without zdb, SET_LABEL should fail
        with utils.fake_path(all_but="zdb"):
            with self.assertRaisesRegex(GLib.GError, "The 'zdb' utility is not available"):
                BlockDev.fs_is_tech_avail(BlockDev.FSTech.ZFS, BlockDev.FSTechMode.SET_LABEL)

        # without zpool, both modes should fail
        with utils.fake_path(all_but="zpool"):
            with self.assertRaisesRegex(GLib.GError, "The 'zpool' utility is not available"):
                BlockDev.fs_is_tech_avail(BlockDev.FSTech.ZFS, BlockDev.FSTechMode.SET_LABEL)

        with utils.fake_path(all_but="zpool"):
            with self.assertRaisesRegex(GLib.GError, "The 'zpool' utility is not available"):
                BlockDev.fs_is_tech_avail(BlockDev.FSTech.ZFS, BlockDev.FSTechMode.QUERY)


class ZfsTestSetLabelNoFallback(ZfsNoDevTestCase):

    def test_set_label_fails_on_nonexistent_device(self):
        """Verify that set_label fails when pool cannot be resolved from device"""
        try:
            BlockDev.fs_is_tech_avail(BlockDev.FSTech.ZFS,
                                      BlockDev.FSTechMode.SET_LABEL)
        except GLib.GError:
            self.skipTest("skipping ZFS: zpool or zdb not available")

        with self.assertRaisesRegex(GLib.GError, "Failed to determine ZFS pool name"):
            BlockDev.fs_zfs_set_label("/dev/nonexistent_device_xyz", "newpool")

    def test_set_label_no_zpool_list_fallback(self):
        """Verify that set_label does not fall back to 'zpool list' when zdb fails

        This is the critical security test: in a multi-pool system, falling
        back to 'the first pool from zpool list' could rename the wrong pool.
        The function must fail closed when zdb cannot resolve the device.
        """
        try:
            BlockDev.fs_is_tech_avail(BlockDev.FSTech.ZFS,
                                      BlockDev.FSTechMode.SET_LABEL)
        except GLib.GError:
            self.skipTest("skipping ZFS: zpool or zdb not available")

        # A bogus device that zdb -l will fail on should cause a hard failure,
        # NOT a fallback to zpool list
        with self.assertRaisesRegex(GLib.GError, "Failed to determine ZFS pool name"):
            BlockDev.fs_zfs_set_label("/dev/null", "newpool")


class ZfsTestOptionDeviceRejection(ZfsNoDevTestCase):

    def test_set_label_rejects_option_device(self):
        """set_label must reject device paths starting with '-'"""
        try:
            BlockDev.fs_is_tech_avail(BlockDev.FSTech.ZFS,
                                      BlockDev.FSTechMode.SET_LABEL)
        except GLib.GError:
            self.skipTest("skipping ZFS: zpool or zdb not available")

        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.fs_zfs_set_label("--help", "newname")

    def test_get_info_rejects_option_device(self):
        """get_info must reject device paths starting with '-'"""
        try:
            BlockDev.fs_is_tech_avail(BlockDev.FSTech.ZFS,
                                      BlockDev.FSTechMode.QUERY)
        except GLib.GError:
            self.skipTest("skipping ZFS: zpool or zdb not available")

        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.fs_zfs_get_info("--help")


class ZfsTestGetInfoNoFallback(ZfsNoDevTestCase):

    def test_get_info_fails_on_nonexistent_device(self):
        """Verify that get_info fails when pool cannot be resolved from device"""
        try:
            BlockDev.fs_is_tech_avail(BlockDev.FSTech.ZFS,
                                      BlockDev.FSTechMode.QUERY)
        except GLib.GError:
            self.skipTest("skipping ZFS: zpool or zdb not available")

        info = None
        with self.assertRaisesRegex(GLib.GError, "Failed to determine ZFS pool name"):
            info = BlockDev.fs_zfs_get_info("/dev/nonexistent_device_xyz")
        self.assertIsNone(info)

    def test_get_info_uses_device_not_first_pool(self):
        """Verify that get_info uses the device parameter to resolve pool

        This is the critical correctness test: get_info must NOT ignore the
        device parameter and return data for the first pool from 'zpool list'.
        It must use zdb -l to resolve which pool the device belongs to.
        """
        try:
            BlockDev.fs_is_tech_avail(BlockDev.FSTech.ZFS,
                                      BlockDev.FSTechMode.QUERY)
        except GLib.GError:
            self.skipTest("skipping ZFS: zpool or zdb not available")

        # A bogus device should produce an error, not return the first pool
        info = None
        with self.assertRaisesRegex(GLib.GError, "Failed to determine ZFS pool name"):
            info = BlockDev.fs_zfs_get_info("/dev/null")
        self.assertIsNone(info)
