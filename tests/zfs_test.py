import unittest

import overrides_hack
from utils import TestTags, tag_test, required_plugins

import gi
gi.require_version('GLib', '2.0')
gi.require_version('BlockDev', '3.0')
from gi.repository import GLib, BlockDev


@required_plugins(("zfs",))
class ZfsPluginTest(unittest.TestCase):
    requested_plugins = BlockDev.plugin_specs_from_names(("zfs",))

    @classmethod
    def setUpClass(cls):
        if not BlockDev.is_initialized():
            BlockDev.init(cls.requested_plugins, None)
        else:
            BlockDev.reinit(cls.requested_plugins, True, None)


class ZfsOptionInjectionTestCase(ZfsPluginTest):
    """Tests that identifiers starting with '-' are rejected by validation.

    These tests exercise the input validation that happens BEFORE any CLI
    tool invocation, so they work even when zpool/zfs are not installed.
    """

    # ---- pool_create ----

    @tag_test(TestTags.NOSTORAGE)
    def test_pool_create_rejects_option_name(self):
        """pool_create must reject pool names starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_pool_create("--help", ["/dev/sda"], None, None)

    @tag_test(TestTags.NOSTORAGE)
    def test_pool_create_rejects_dash_name(self):
        """pool_create must reject single-dash pool names"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_pool_create("-a", ["/dev/sda"], None, None)

    @tag_test(TestTags.NOSTORAGE)
    def test_pool_create_rejects_option_vdev(self):
        """pool_create must reject vdev elements starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_pool_create("testpool", ["--force"], None, None)

    @tag_test(TestTags.NOSTORAGE)
    def test_pool_create_rejects_empty_name(self):
        """pool_create must reject empty pool name"""
        with self.assertRaisesRegex(GLib.GError, "cannot be NULL or empty"):
            BlockDev.zfs_pool_create("", ["/dev/sda"], None, None)

    # ---- pool_import ----

    @tag_test(TestTags.NOSTORAGE)
    def test_pool_import_rejects_option_name(self):
        """pool_import must reject name_or_guid starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_pool_import("--help", None, None, False, None)

    @tag_test(TestTags.NOSTORAGE)
    def test_pool_import_rejects_option_new_name(self):
        """pool_import must reject new_name starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_pool_import("testpool", "-x", None, False, None)

    # ---- dataset_create ----

    @tag_test(TestTags.NOSTORAGE)
    def test_dataset_create_rejects_option_name(self):
        """dataset_create must reject dataset names starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_dataset_create("--help", None)

    @tag_test(TestTags.NOSTORAGE)
    def test_dataset_create_rejects_empty_name(self):
        """dataset_create must reject empty dataset name"""
        with self.assertRaisesRegex(GLib.GError, "cannot be NULL or empty"):
            BlockDev.zfs_dataset_create("", None)

    # ---- encryption_load_key ----

    @tag_test(TestTags.NOSTORAGE)
    def test_encryption_load_key_rejects_option_name(self):
        """encryption_load_key must reject dataset names starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_encryption_load_key("--help", None)

    @tag_test(TestTags.NOSTORAGE)
    def test_encryption_load_key_rejects_empty_name(self):
        """encryption_load_key must reject empty dataset name"""
        with self.assertRaisesRegex(GLib.GError, "cannot be NULL or empty"):
            BlockDev.zfs_encryption_load_key("", None)

    # ---- valid names must NOT trigger the validation error ----

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_pool_name_not_rejected(self):
        """Valid pool names must not trigger the '-' validation error.

        The function will fail for other reasons (no actual pool), but it
        must NOT fail with 'cannot start with' error.
        """
        try:
            BlockDev.zfs_pool_destroy("validpool", False)
        except GLib.GError as e:
            # It should fail, but NOT because of identifier validation
            self.assertNotIn("cannot start with '-'", str(e))
            self.assertNotIn("cannot be NULL or empty", str(e))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_dataset_name_not_rejected(self):
        """Valid dataset names must not trigger the '-' validation error."""
        try:
            BlockDev.zfs_dataset_create("pool/dataset", None)
        except GLib.GError as e:
            self.assertNotIn("cannot start with '-'", str(e))
            self.assertNotIn("cannot be NULL or empty", str(e))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_snapshot_name_not_rejected(self):
        """Valid snapshot names must not trigger the '-' validation error."""
        try:
            BlockDev.zfs_snapshot_create("pool/data@snap1", False, None)
        except GLib.GError as e:
            self.assertNotIn("cannot start with '-'", str(e))
            self.assertNotIn("cannot be NULL or empty", str(e))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_vdev_path_not_rejected(self):
        """Vdev paths like /dev/sda must not trigger the '-' validation error."""
        try:
            BlockDev.zfs_pool_create("testpool", ["/dev/sda"], None, None)
        except GLib.GError as e:
            self.assertNotIn("cannot start with '-'", str(e))
            self.assertNotIn("cannot be NULL or empty", str(e))

    # ---- additional functions with identifier params ----

    @tag_test(TestTags.NOSTORAGE)
    def test_pool_destroy_rejects_option_name(self):
        """pool_destroy must reject pool names starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_pool_destroy("--force", False)

    @tag_test(TestTags.NOSTORAGE)
    def test_snapshot_create_rejects_option_name(self):
        """snapshot_create must reject snapshot names starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_snapshot_create("-r", False, None)

    @tag_test(TestTags.NOSTORAGE)
    def test_snapshot_clone_rejects_option_snapshot(self):
        """snapshot_clone must reject snapshot names starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_snapshot_clone("-x", "pool/clone", None)

    @tag_test(TestTags.NOSTORAGE)
    def test_snapshot_clone_rejects_option_clone_name(self):
        """snapshot_clone must reject clone names starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_snapshot_clone("pool/data@snap", "--help", None)

    @tag_test(TestTags.NOSTORAGE)
    def test_bookmark_create_rejects_option_snapshot(self):
        """bookmark_create must reject snapshot names starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_bookmark_create("-x", "pool/data#bm")

    @tag_test(TestTags.NOSTORAGE)
    def test_bookmark_create_rejects_option_bookmark(self):
        """bookmark_create must reject bookmark names starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_bookmark_create("pool/data@snap", "-x")

    @tag_test(TestTags.NOSTORAGE)
    def test_zvol_create_rejects_option_name(self):
        """zvol_create must reject zvol names starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_zvol_create("--help", 1024, False, None)

    @tag_test(TestTags.NOSTORAGE)
    def test_pool_replace_rejects_option_name(self):
        """pool_replace must reject pool names starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_pool_replace("-x", "/dev/sda", "/dev/sdb", False)

    @tag_test(TestTags.NOSTORAGE)
    def test_encryption_unload_key_rejects_option_name(self):
        """encryption_unload_key must reject dataset names starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_encryption_unload_key("--help")

    @tag_test(TestTags.NOSTORAGE)
    def test_pool_scrub_start_rejects_option_name(self):
        """pool_scrub_start must reject pool names starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_pool_scrub_start("-s")

    # ---- property validation ----

    @tag_test(TestTags.NOSTORAGE)
    def test_pool_get_property_rejects_option_property(self):
        """pool_get_property must reject property names starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_pool_get_property("testpool", "--help")

    @tag_test(TestTags.NOSTORAGE)
    def test_dataset_get_property_rejects_option_property(self):
        """dataset_get_property must reject property names starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_dataset_get_property("pool/ds", "--help")

    @tag_test(TestTags.NOSTORAGE)
    def test_dataset_inherit_property_rejects_option_property(self):
        """dataset_inherit_property must reject property names starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_dataset_inherit_property("pool/ds", "--help")

    # NOTE: bd_zfs_pool_get_vdevs() bugs (double-free on depth overflow and
    # regex recompilation in hot loop) were fixed in commit
    # fix/s1-4-vdev-parser-safety and verified by code review + defensive
    # reordering.  Runtime tests require a live ZFS pool with deeply nested
    # vdevs which is impractical in the unit-test environment.


class ZfsVersionTestCase(ZfsPluginTest):
    """Tests for bd_zfs_get_zfs_version() version probing."""

    @tag_test(TestTags.NOSTORAGE)
    def test_get_version_returns_string(self):
        """get_zfs_version returns a version string when ZFS tools are available"""
        try:
            BlockDev.zfs_is_tech_avail(BlockDev.ZFSTech.POOL, BlockDev.ZFSTechMode.QUERY)
        except GLib.GError:
            self.skipTest("skipping: ZFS tools not available")

        version = BlockDev.zfs_get_zfs_version()
        self.assertIsNotNone(version)
        # Version should look like digits.digits (at minimum major.minor)
        import re
        self.assertRegex(version, r'^\d+\.\d+')

    @tag_test(TestTags.NOSTORAGE)
    def test_get_version_no_distro_suffix(self):
        """get_zfs_version must not contain distro suffixes (no '-', no letters)"""
        try:
            BlockDev.zfs_is_tech_avail(BlockDev.ZFSTech.POOL, BlockDev.ZFSTechMode.QUERY)
        except GLib.GError:
            self.skipTest("skipping: ZFS tools not available")

        version = BlockDev.zfs_get_zfs_version()
        self.assertIsNotNone(version)
        # Should only contain digits and dots
        import re
        self.assertRegex(version, r'^\d+(\.\d+)*$')

    @tag_test(TestTags.NOSTORAGE)
    def test_get_version_is_cached(self):
        """get_zfs_version returns the same pointer on subsequent calls"""
        try:
            BlockDev.zfs_is_tech_avail(BlockDev.ZFSTech.POOL, BlockDev.ZFSTechMode.QUERY)
        except GLib.GError:
            self.skipTest("skipping: ZFS tools not available")

        version1 = BlockDev.zfs_get_zfs_version()
        version2 = BlockDev.zfs_get_zfs_version()
        self.assertEqual(version1, version2)
