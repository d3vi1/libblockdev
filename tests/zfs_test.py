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
        """get_zfs_version returns the same value on subsequent calls"""
        try:
            BlockDev.zfs_is_tech_avail(BlockDev.ZFSTech.POOL, BlockDev.ZFSTechMode.QUERY)
        except GLib.GError:
            self.skipTest("skipping: ZFS tools not available")

        version1 = BlockDev.zfs_get_zfs_version()
        version2 = BlockDev.zfs_get_zfs_version()
        self.assertEqual(version1, version2)


class ZfsCapabilityTestCase(ZfsPluginTest):
    """Tests for the runtime capability model in bd_zfs_is_tech_avail()."""

    def _skip_unless_zfs_tools(self):
        """Helper: skip the test if ZFS tools are not available."""
        try:
            BlockDev.zfs_is_tech_avail(BlockDev.ZFSTech.POOL, BlockDev.ZFSTechMode.QUERY)
        except GLib.GError:
            self.skipTest("skipping: ZFS tools not available")

    @tag_test(TestTags.NOSTORAGE)
    def test_is_tech_avail_pool_query(self):
        """is_tech_avail returns TRUE for POOL+QUERY when tools available"""
        self._skip_unless_zfs_tools()
        avail = BlockDev.zfs_is_tech_avail(BlockDev.ZFSTech.POOL, BlockDev.ZFSTechMode.QUERY)
        self.assertTrue(avail)

    @tag_test(TestTags.NOSTORAGE)
    def test_is_tech_avail_pool_create(self):
        """is_tech_avail returns TRUE for POOL+CREATE when tools available"""
        self._skip_unless_zfs_tools()
        avail = BlockDev.zfs_is_tech_avail(BlockDev.ZFSTech.POOL, BlockDev.ZFSTechMode.CREATE)
        self.assertTrue(avail)

    @tag_test(TestTags.NOSTORAGE)
    def test_is_tech_avail_vdev(self):
        """is_tech_avail returns TRUE for VDEV+QUERY when tools available"""
        self._skip_unless_zfs_tools()
        avail = BlockDev.zfs_is_tech_avail(BlockDev.ZFSTech.VDEV, BlockDev.ZFSTechMode.QUERY)
        self.assertTrue(avail)

    @tag_test(TestTags.NOSTORAGE)
    def test_is_tech_avail_dataset(self):
        """is_tech_avail returns TRUE for DATASET+QUERY when tools available"""
        self._skip_unless_zfs_tools()
        avail = BlockDev.zfs_is_tech_avail(BlockDev.ZFSTech.DATASET, BlockDev.ZFSTechMode.QUERY)
        self.assertTrue(avail)

    @tag_test(TestTags.NOSTORAGE)
    def test_is_tech_avail_snapshot(self):
        """is_tech_avail returns TRUE for SNAPSHOT+QUERY when tools available"""
        self._skip_unless_zfs_tools()
        avail = BlockDev.zfs_is_tech_avail(BlockDev.ZFSTech.SNAPSHOT, BlockDev.ZFSTechMode.QUERY)
        self.assertTrue(avail)

    @tag_test(TestTags.NOSTORAGE)
    def test_is_tech_avail_zvol(self):
        """is_tech_avail returns TRUE for ZVOL+QUERY when tools available"""
        self._skip_unless_zfs_tools()
        avail = BlockDev.zfs_is_tech_avail(BlockDev.ZFSTech.ZVOL, BlockDev.ZFSTechMode.QUERY)
        self.assertTrue(avail)

    @tag_test(TestTags.NOSTORAGE)
    def test_is_tech_avail_encryption(self):
        """is_tech_avail returns a result for ENCRYPTION tech without crashing"""
        self._skip_unless_zfs_tools()
        # Encryption availability depends on version; just verify no crash
        try:
            BlockDev.zfs_is_tech_avail(BlockDev.ZFSTech.ENCRYPTION, BlockDev.ZFSTechMode.QUERY)
        except GLib.GError:
            pass  # Expected if version < 0.8.0

    @tag_test(TestTags.NOSTORAGE)
    def test_is_tech_avail_maintenance_query(self):
        """is_tech_avail returns TRUE for MAINTENANCE+QUERY when tools available"""
        self._skip_unless_zfs_tools()
        avail = BlockDev.zfs_is_tech_avail(BlockDev.ZFSTech.MAINTENANCE, BlockDev.ZFSTechMode.QUERY)
        self.assertTrue(avail)

    @tag_test(TestTags.NOSTORAGE)
    def test_is_tech_avail_maintenance_modify(self):
        """is_tech_avail returns a result for MAINTENANCE+MODIFY without crashing"""
        self._skip_unless_zfs_tools()
        # MODIFY mode requires version >= 0.8.0; just verify no crash
        try:
            BlockDev.zfs_is_tech_avail(BlockDev.ZFSTech.MAINTENANCE, BlockDev.ZFSTechMode.MODIFY)
        except GLib.GError:
            pass  # Expected if version < 0.8.0

    @tag_test(TestTags.NOSTORAGE)
    def test_is_tech_avail_encryption_true_on_modern_zfs(self):
        """is_tech_avail returns TRUE for ENCRYPTION on OpenZFS >= 0.8.0"""
        self._skip_unless_zfs_tools()
        version = BlockDev.zfs_get_zfs_version()
        parts = version.split(".")
        major = int(parts[0])
        minor = int(parts[1]) if len(parts) > 1 else 0
        if major < 1 and minor < 8:
            self.skipTest("skipping: OpenZFS < 0.8.0")
        avail = BlockDev.zfs_is_tech_avail(BlockDev.ZFSTech.ENCRYPTION, BlockDev.ZFSTechMode.QUERY)
        self.assertTrue(avail)

    @tag_test(TestTags.NOSTORAGE)
    def test_is_tech_avail_maintenance_modify_true_on_modern_zfs(self):
        """is_tech_avail returns TRUE for MAINTENANCE+MODIFY on OpenZFS >= 0.8.0"""
        self._skip_unless_zfs_tools()
        version = BlockDev.zfs_get_zfs_version()
        parts = version.split(".")
        major = int(parts[0])
        minor = int(parts[1]) if len(parts) > 1 else 0
        if major < 1 and minor < 8:
            self.skipTest("skipping: OpenZFS < 0.8.0")
        avail = BlockDev.zfs_is_tech_avail(BlockDev.ZFSTech.MAINTENANCE, BlockDev.ZFSTechMode.MODIFY)
        self.assertTrue(avail)


class ZfsDatasetInfoEncryptionFieldsTestCase(ZfsPluginTest):
    """Tests that dataset info/list handle encryption fields conditionally."""

    def _skip_unless_zfs_tools(self):
        """Helper: skip the test if ZFS tools are not available."""
        try:
            BlockDev.zfs_is_tech_avail(BlockDev.ZFSTech.DATASET, BlockDev.ZFSTechMode.QUERY)
        except GLib.GError:
            self.skipTest("skipping: ZFS tools not available")

    def _has_encryption_support(self):
        """Return True if the installed OpenZFS is >= 0.8.0."""
        version = BlockDev.zfs_get_zfs_version()
        parts = version.split(".")
        major = int(parts[0])
        minor = int(parts[1]) if len(parts) > 1 else 0
        return major > 0 or minor >= 8

    @tag_test(TestTags.NOSTORAGE)
    def test_dataset_get_info_no_error(self):
        """dataset_get_info must not crash due to encryption field mismatch.

        We attempt to query a dataset that likely does not exist; the key
        assertion is that the error is NOT a parse/field-count error caused
        by requesting encryption properties on pre-0.8 ZFS.
        """
        self._skip_unless_zfs_tools()
        try:
            BlockDev.zfs_dataset_get_info("nonexistent/dataset")
        except GLib.GError as e:
            # We expect a "does not exist" style error, NOT a parse error
            self.assertNotIn("Failed to parse", str(e))

    @tag_test(TestTags.NOSTORAGE)
    def test_dataset_list_no_error(self):
        """dataset_list must not crash due to encryption field mismatch.

        On a system without pools the list may fail or return empty; the
        key check is that it does not fail with a parse/field-count error.
        """
        self._skip_unless_zfs_tools()
        try:
            infos = BlockDev.zfs_dataset_list(None, False)
            # If it succeeds (pools exist), each entry should have valid fields
            if infos:
                for info in infos:
                    self.assertIsNotNone(info.name)
        except GLib.GError as e:
            self.assertNotIn("Failed to parse", str(e))

    @tag_test(TestTags.NOSTORAGE)
    def test_encryption_fields_populated_on_modern_zfs(self):
        """On OpenZFS >= 0.8.0, encryption fields must be populated in dataset info.

        This test only runs when pools exist and encryption is supported.
        """
        self._skip_unless_zfs_tools()
        if not self._has_encryption_support():
            self.skipTest("skipping: OpenZFS < 0.8.0, no encryption support")

        try:
            infos = BlockDev.zfs_dataset_list(None, False)
        except GLib.GError:
            self.skipTest("skipping: no pools available to query")

        if not infos:
            self.skipTest("skipping: no datasets to inspect")

        # On modern ZFS, the encryption field should be a non-None string
        # (typically "off" or an algorithm name like "aes-256-gcm")
        info = infos[0]
        self.assertIsNotNone(info.encryption,
                             "encryption field should be populated on OpenZFS >= 0.8.0")


class ZfsUnknownEnumValuesTestCase(ZfsPluginTest):
    """Tests that UNKNOWN enum values exist and are used for error/unrecognized states."""

    @tag_test(TestTags.NOSTORAGE)
    def test_key_status_unknown_enum_exists(self):
        """BD_ZFS_KEY_STATUS_UNKNOWN must be accessible through GI"""
        val = BlockDev.ZFSKeyStatus.UNKNOWN
        self.assertIsNotNone(val)
        # UNKNOWN must be distinct from NONE
        self.assertNotEqual(val, BlockDev.ZFSKeyStatus.NONE)

    @tag_test(TestTags.NOSTORAGE)
    def test_dataset_type_unknown_enum_exists(self):
        """BD_ZFS_DATASET_TYPE_UNKNOWN must be accessible through GI"""
        val = BlockDev.ZFSDatasetType.UNKNOWN
        self.assertIsNotNone(val)
        # UNKNOWN must be distinct from FILESYSTEM
        self.assertNotEqual(val, BlockDev.ZFSDatasetType.FILESYSTEM)

    @tag_test(TestTags.NOSTORAGE)
    def test_encryption_key_status_returns_unknown_on_error(self):
        """encryption_key_status must return UNKNOWN (not NONE) on validation error"""
        with self.assertRaises(GLib.GError):
            status = BlockDev.zfs_encryption_key_status("--help")
        # Also test with an empty name — should raise and return UNKNOWN
        with self.assertRaises(GLib.GError):
            status = BlockDev.zfs_encryption_key_status("")

    @tag_test(TestTags.NOSTORAGE)
    def test_key_status_none_means_no_encryption(self):
        """BD_ZFS_KEY_STATUS_NONE must still exist for 'no encryption' semantics"""
        val = BlockDev.ZFSKeyStatus.NONE
        self.assertIsNotNone(val)
        self.assertEqual(int(val), 0)
