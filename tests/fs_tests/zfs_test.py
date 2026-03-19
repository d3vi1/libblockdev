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


class ZfsTestCheckLabel(ZfsNoDevTestCase):
    """Tests for bd_fs_zfs_check_label() — must match canonical OpenZFS rules
    from bd_zfs_validate_pool_name() in the ZFS top-level plugin."""

    # ---- valid names ----

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_simple_name(self):
        """'tank' is a valid pool name"""
        self.assertTrue(BlockDev.fs_zfs_check_label("tank"))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_longer_name(self):
        """'mypool' is a valid pool name"""
        self.assertTrue(BlockDev.fs_zfs_check_label("mypool"))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_underscore(self):
        """'pool_1' is a valid pool name (underscore allowed)"""
        self.assertTrue(BlockDev.fs_zfs_check_label("pool_1"))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_hyphen(self):
        """'test-pool' is a valid pool name (hyphen allowed)"""
        self.assertTrue(BlockDev.fs_zfs_check_label("test-pool"))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_period(self):
        """'data.backup' is a valid pool name (period allowed)"""
        self.assertTrue(BlockDev.fs_zfs_check_label("data.backup"))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_colon(self):
        """'pool:1' is a valid pool name (colon allowed)"""
        self.assertTrue(BlockDev.fs_zfs_check_label("pool:1"))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_single_letter(self):
        """'z' is a valid pool name (single letter)"""
        self.assertTrue(BlockDev.fs_zfs_check_label("z"))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_max_length(self):
        """A 239-character name is valid (max allowed)"""
        name = "a" * 239
        self.assertTrue(BlockDev.fs_zfs_check_label(name))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_cache(self):
        """'cache' is NOT a reserved pool name in OpenZFS"""
        self.assertTrue(BlockDev.fs_zfs_check_label("cache"))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_special(self):
        """'special' is NOT a reserved pool name in OpenZFS"""
        self.assertTrue(BlockDev.fs_zfs_check_label("special"))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_dedup(self):
        """'dedup' is NOT a reserved pool name in OpenZFS"""
        self.assertTrue(BlockDev.fs_zfs_check_label("dedup"))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_c_digit(self):
        """'c0d0' is valid — c+digit check was removed from OpenZFS in 2021"""
        self.assertTrue(BlockDev.fs_zfs_check_label("c0d0"))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_C_digit(self):
        """'C9pool' is valid — c+digit check was removed from OpenZFS in 2021"""
        self.assertTrue(BlockDev.fs_zfs_check_label("C9pool"))

    # ---- invalid: empty / NULL ----

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_empty(self):
        """Empty string is not a valid pool name"""
        with self.assertRaisesRegex(GLib.GError, "cannot be NULL or empty"):
            BlockDev.fs_zfs_check_label("")

    # ---- invalid: starts with digit ----

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_starts_with_digit(self):
        """Pool name starting with a digit is invalid"""
        with self.assertRaisesRegex(GLib.GError, "must begin with a letter"):
            BlockDev.fs_zfs_check_label("1pool")

    # ---- invalid: starts with special char ----

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_starts_with_dash(self):
        """Pool name starting with '-' is invalid"""
        with self.assertRaisesRegex(GLib.GError, "must begin with a letter"):
            BlockDev.fs_zfs_check_label("-pool")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_starts_with_underscore(self):
        """Pool name starting with '_' is invalid (must begin with letter)"""
        with self.assertRaisesRegex(GLib.GError, "must begin with a letter"):
            BlockDev.fs_zfs_check_label("_pool")

    # ---- invalid: bad characters ----

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_space(self):
        """Pool name containing a space is invalid"""
        with self.assertRaisesRegex(GLib.GError, "contains invalid character"):
            BlockDev.fs_zfs_check_label("my pool")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_slash(self):
        """Pool name containing '/' is invalid"""
        with self.assertRaisesRegex(GLib.GError, "contains invalid character"):
            BlockDev.fs_zfs_check_label("pool/child")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_at_sign(self):
        """Pool name containing '@' is invalid"""
        with self.assertRaisesRegex(GLib.GError, "contains invalid character"):
            BlockDev.fs_zfs_check_label("pool@snap")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_hash(self):
        """Pool name containing '#' is invalid"""
        with self.assertRaisesRegex(GLib.GError, "contains invalid character"):
            BlockDev.fs_zfs_check_label("pool#bm")

    # ---- invalid: reserved names (exact match for log, prefix match for others) ----

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_mirror(self):
        """'mirror' is a reserved vdev type prefix"""
        with self.assertRaisesRegex(GLib.GError, "conflicts with a reserved vdev type name"):
            BlockDev.fs_zfs_check_label("mirror")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_mirror_prefix(self):
        """'mirror0' is rejected (prefix match)"""
        with self.assertRaisesRegex(GLib.GError, "conflicts with a reserved vdev type name"):
            BlockDev.fs_zfs_check_label("mirror0")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_raidz(self):
        """'raidz' is a reserved vdev type prefix"""
        with self.assertRaisesRegex(GLib.GError, "conflicts with a reserved vdev type name"):
            BlockDev.fs_zfs_check_label("raidz")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_raidz1(self):
        """'raidz1' is rejected (prefix match)"""
        with self.assertRaisesRegex(GLib.GError, "conflicts with a reserved vdev type name"):
            BlockDev.fs_zfs_check_label("raidz1")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_raidz2(self):
        """'raidz2' is rejected (prefix match)"""
        with self.assertRaisesRegex(GLib.GError, "conflicts with a reserved vdev type name"):
            BlockDev.fs_zfs_check_label("raidz2")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_spare(self):
        """'spare' is a reserved vdev type prefix"""
        with self.assertRaisesRegex(GLib.GError, "conflicts with a reserved vdev type name"):
            BlockDev.fs_zfs_check_label("spare")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_spare1(self):
        """'spare1' is rejected (prefix match)"""
        with self.assertRaisesRegex(GLib.GError, "conflicts with a reserved vdev type name"):
            BlockDev.fs_zfs_check_label("spare1")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_draid(self):
        """'draid' is a reserved vdev type prefix"""
        with self.assertRaisesRegex(GLib.GError, "conflicts with a reserved vdev type name"):
            BlockDev.fs_zfs_check_label("draid")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_draid2(self):
        """'draid2' is rejected (prefix match)"""
        with self.assertRaisesRegex(GLib.GError, "conflicts with a reserved vdev type name"):
            BlockDev.fs_zfs_check_label("draid2")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_log(self):
        """'log' is a reserved vdev type name (exact match)"""
        with self.assertRaisesRegex(GLib.GError, "conflicts with a reserved vdev type name"):
            BlockDev.fs_zfs_check_label("log")

    @tag_test(TestTags.NOSTORAGE)
    def test_reserved_case_insensitive(self):
        """Reserved name check is case-insensitive"""
        with self.assertRaisesRegex(GLib.GError, "conflicts with a reserved vdev type name"):
            BlockDev.fs_zfs_check_label("MIRROR")

    @tag_test(TestTags.NOSTORAGE)
    def test_reserved_mixed_case(self):
        """Reserved name check is case-insensitive (mixed case)"""
        with self.assertRaisesRegex(GLib.GError, "conflicts with a reserved vdev type name"):
            BlockDev.fs_zfs_check_label("RaidZ")

    # ---- invalid: too long ----

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_too_long(self):
        """A 240-character name exceeds the maximum"""
        name = "a" * 240
        with self.assertRaisesRegex(GLib.GError, "exceeds maximum length"):
            BlockDev.fs_zfs_check_label(name)


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
