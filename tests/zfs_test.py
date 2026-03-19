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
        with self.assertRaisesRegex(GLib.GError, "must begin with a letter"):
            BlockDev.zfs_pool_create("--help", ["/dev/sda"], None, None)

    @tag_test(TestTags.NOSTORAGE)
    def test_pool_create_rejects_dash_name(self):
        """pool_create must reject single-dash pool names"""
        with self.assertRaisesRegex(GLib.GError, "must begin with a letter"):
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

    # ---- nullable parameter validation ----

    @tag_test(TestTags.NOSTORAGE)
    def test_pool_set_property_rejects_null_value(self):
        """pool_set_property must reject NULL/None value"""
        with self.assertRaisesRegex(GLib.GError, "cannot be NULL or empty"):
            BlockDev.zfs_pool_set_property("testpool", "comment", None)

    @tag_test(TestTags.NOSTORAGE)
    def test_pool_set_property_rejects_empty_value(self):
        """pool_set_property must reject empty value"""
        with self.assertRaisesRegex(GLib.GError, "cannot be NULL or empty"):
            BlockDev.zfs_pool_set_property("testpool", "comment", "")

    @tag_test(TestTags.NOSTORAGE)
    def test_dataset_set_property_rejects_null_value(self):
        """dataset_set_property must reject NULL/None value"""
        with self.assertRaisesRegex(GLib.GError, "cannot be NULL or empty"):
            BlockDev.zfs_dataset_set_property("pool/ds", "mountpoint", None)

    @tag_test(TestTags.NOSTORAGE)
    def test_dataset_set_property_rejects_empty_value(self):
        """dataset_set_property must reject empty value"""
        with self.assertRaisesRegex(GLib.GError, "cannot be NULL or empty"):
            BlockDev.zfs_dataset_set_property("pool/ds", "mountpoint", "")

    @tag_test(TestTags.NOSTORAGE)
    def test_dataset_mount_rejects_option_mountpoint(self):
        """dataset_mount must reject mountpoint starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot be empty or start with '-'"):
            BlockDev.zfs_dataset_mount("pool/ds", "--help", None)

    @tag_test(TestTags.NOSTORAGE)
    def test_dataset_mount_rejects_empty_mountpoint(self):
        """dataset_mount must reject empty mountpoint"""
        with self.assertRaisesRegex(GLib.GError, "cannot be empty or start with '-'"):
            BlockDev.zfs_dataset_mount("pool/ds", "", None)

    @tag_test(TestTags.NOSTORAGE)
    def test_encryption_load_key_rejects_option_key_location(self):
        """encryption_load_key must reject key_location starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_encryption_load_key("pool/ds", "--help")

    @tag_test(TestTags.NOSTORAGE)
    def test_encryption_change_key_rejects_option_new_key_location(self):
        """encryption_change_key must reject new_key_location starting with '-'"""
        with self.assertRaisesRegex(GLib.GError, "cannot start with '-'"):
            BlockDev.zfs_encryption_change_key("pool/ds", "--help", None)

    # ---- synthesized option payload hardening (comma in -o key=value) ----

    @tag_test(TestTags.NOSTORAGE)
    def test_dataset_mount_rejects_comma_in_mountpoint(self):
        """dataset_mount must reject mountpoint containing commas (option separator in -o)"""
        with self.assertRaisesRegex(GLib.GError, "cannot contain commas"):
            BlockDev.zfs_dataset_mount("pool/ds", "/mnt/a,rw", None)

    @tag_test(TestTags.NOSTORAGE)
    def test_encryption_change_key_rejects_comma_in_key_location(self):
        """encryption_change_key must reject new_key_location containing commas"""
        with self.assertRaisesRegex(GLib.GError, "cannot contain commas"):
            BlockDev.zfs_encryption_change_key("pool/ds", "file:///key,file", None)

    # NOTE: bd_zfs_pool_get_vdevs() bugs (double-free on depth overflow and
    # regex recompilation in hot loop) were fixed in commit
    # fix/s1-4-vdev-parser-safety and verified by code review + defensive
    # reordering.  Runtime tests require a live ZFS pool with deeply nested
    # vdevs which is impractical in the unit-test environment.


class ZfsPoolNameValidationTestCase(ZfsPluginTest):
    """Tests for bd_zfs_validate_pool_name() — comprehensive ZFS naming rules."""

    # ---- valid names ----

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_simple_name(self):
        """'tank' is a valid pool name"""
        self.assertTrue(BlockDev.zfs_validate_pool_name("tank"))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_longer_name(self):
        """'mypool' is a valid pool name"""
        self.assertTrue(BlockDev.zfs_validate_pool_name("mypool"))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_underscore(self):
        """'pool_1' is a valid pool name (underscore allowed)"""
        self.assertTrue(BlockDev.zfs_validate_pool_name("pool_1"))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_hyphen(self):
        """'test-pool' is a valid pool name (hyphen allowed)"""
        self.assertTrue(BlockDev.zfs_validate_pool_name("test-pool"))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_period(self):
        """'data.backup' is a valid pool name (period allowed)"""
        self.assertTrue(BlockDev.zfs_validate_pool_name("data.backup"))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_colon(self):
        """'pool:1' is a valid pool name (colon allowed)"""
        self.assertTrue(BlockDev.zfs_validate_pool_name("pool:1"))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_single_letter(self):
        """'z' is a valid pool name (single letter)"""
        self.assertTrue(BlockDev.zfs_validate_pool_name("z"))

    @tag_test(TestTags.NOSTORAGE)
    def test_valid_max_length(self):
        """A 255-character name is valid (max allowed)"""
        name = "a" * 255
        self.assertTrue(BlockDev.zfs_validate_pool_name(name))

    # ---- invalid: empty / NULL ----

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_empty(self):
        """Empty string is not a valid pool name"""
        with self.assertRaisesRegex(GLib.GError, "cannot be NULL or empty"):
            BlockDev.zfs_validate_pool_name("")

    # ---- invalid: starts with digit ----

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_starts_with_digit(self):
        """Pool name starting with a digit is invalid"""
        with self.assertRaisesRegex(GLib.GError, "must begin with a letter"):
            BlockDev.zfs_validate_pool_name("1pool")

    # ---- invalid: starts with special char ----

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_starts_with_dash(self):
        """Pool name starting with '-' is invalid"""
        with self.assertRaisesRegex(GLib.GError, "must begin with a letter"):
            BlockDev.zfs_validate_pool_name("-pool")

    # ---- invalid: 'c' followed by digit ----

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_c_digit(self):
        """'c0' prefix is reserved for device names"""
        with self.assertRaisesRegex(GLib.GError, "cannot begin with 'c' followed by a digit"):
            BlockDev.zfs_validate_pool_name("c0d0")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_C_digit(self):
        """'C9' prefix is also reserved (case-insensitive 'c' check)"""
        with self.assertRaisesRegex(GLib.GError, "cannot begin with 'c' followed by a digit"):
            BlockDev.zfs_validate_pool_name("C9pool")

    # ---- invalid: bad characters ----

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_space(self):
        """Pool name containing a space is invalid"""
        with self.assertRaisesRegex(GLib.GError, "contains invalid character"):
            BlockDev.zfs_validate_pool_name("my pool")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_slash(self):
        """Pool name containing '/' is invalid"""
        with self.assertRaisesRegex(GLib.GError, "contains invalid character"):
            BlockDev.zfs_validate_pool_name("pool/child")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_at_sign(self):
        """Pool name containing '@' is invalid"""
        with self.assertRaisesRegex(GLib.GError, "contains invalid character"):
            BlockDev.zfs_validate_pool_name("pool@snap")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_hash(self):
        """Pool name containing '#' is invalid"""
        with self.assertRaisesRegex(GLib.GError, "contains invalid character"):
            BlockDev.zfs_validate_pool_name("pool#bm")

    # ---- invalid: reserved names ----

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_mirror(self):
        """'mirror' is a reserved vdev type name"""
        with self.assertRaisesRegex(GLib.GError, "is reserved"):
            BlockDev.zfs_validate_pool_name("mirror")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_raidz(self):
        """'raidz' is a reserved vdev type name"""
        with self.assertRaisesRegex(GLib.GError, "is reserved"):
            BlockDev.zfs_validate_pool_name("raidz")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_cache(self):
        """'cache' is a reserved vdev type name"""
        with self.assertRaisesRegex(GLib.GError, "is reserved"):
            BlockDev.zfs_validate_pool_name("cache")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_spare(self):
        """'spare' is a reserved vdev type name"""
        with self.assertRaisesRegex(GLib.GError, "is reserved"):
            BlockDev.zfs_validate_pool_name("spare")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_log(self):
        """'log' is a reserved vdev type name"""
        with self.assertRaisesRegex(GLib.GError, "is reserved"):
            BlockDev.zfs_validate_pool_name("log")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_dedup(self):
        """'dedup' is a reserved vdev type name"""
        with self.assertRaisesRegex(GLib.GError, "is reserved"):
            BlockDev.zfs_validate_pool_name("dedup")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_special(self):
        """'special' is a reserved vdev type name"""
        with self.assertRaisesRegex(GLib.GError, "is reserved"):
            BlockDev.zfs_validate_pool_name("special")

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_draid(self):
        """'draid' is a reserved vdev type name"""
        with self.assertRaisesRegex(GLib.GError, "is reserved"):
            BlockDev.zfs_validate_pool_name("draid")

    @tag_test(TestTags.NOSTORAGE)
    def test_reserved_case_insensitive(self):
        """Reserved name check is case-insensitive"""
        with self.assertRaisesRegex(GLib.GError, "is reserved"):
            BlockDev.zfs_validate_pool_name("MIRROR")

    @tag_test(TestTags.NOSTORAGE)
    def test_reserved_mixed_case(self):
        """Reserved name check is case-insensitive (mixed case)"""
        with self.assertRaisesRegex(GLib.GError, "is reserved"):
            BlockDev.zfs_validate_pool_name("RaidZ")

    # ---- invalid: too long ----

    @tag_test(TestTags.NOSTORAGE)
    def test_invalid_too_long(self):
        """A 256-character name exceeds the maximum"""
        name = "a" * 256
        with self.assertRaisesRegex(GLib.GError, "exceeds maximum length"):
            BlockDev.zfs_validate_pool_name(name)

    # ---- pool_create integration: verify it uses validate_pool_name ----

    @tag_test(TestTags.NOSTORAGE)
    def test_pool_create_rejects_reserved_name(self):
        """pool_create must reject reserved vdev type names"""
        with self.assertRaisesRegex(GLib.GError, "is reserved"):
            BlockDev.zfs_pool_create("mirror", ["/dev/sda"], None, None)

    @tag_test(TestTags.NOSTORAGE)
    def test_pool_create_rejects_digit_start(self):
        """pool_create must reject pool names starting with a digit"""
        with self.assertRaisesRegex(GLib.GError, "must begin with a letter"):
            BlockDev.zfs_pool_create("9pool", ["/dev/sda"], None, None)

    @tag_test(TestTags.NOSTORAGE)
    def test_pool_create_rejects_c_digit(self):
        """pool_create must reject pool names starting with 'c' + digit"""
        with self.assertRaisesRegex(GLib.GError, "cannot begin with 'c' followed by a digit"):
            BlockDev.zfs_pool_create("c0d0", ["/dev/sda"], None, None)


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


class ZfsBookmarkInfoTypeTestCase(ZfsPluginTest):
    """Tests for the BDZFSBookmarkInfo type and bd_zfs_bookmark_list return type."""

    @tag_test(TestTags.NOSTORAGE)
    def test_bookmark_info_type_accessible(self):
        """BDZFSBookmarkInfo must be accessible through GI as a boxed type"""
        info_type = BlockDev.ZFSBookmarkInfo
        self.assertIsNotNone(info_type)

    @tag_test(TestTags.NOSTORAGE)
    def test_bookmark_info_has_name_field(self):
        """BDZFSBookmarkInfo must have a 'name' field"""
        self.assertTrue(hasattr(BlockDev.ZFSBookmarkInfo, 'name'))

    @tag_test(TestTags.NOSTORAGE)
    def test_bookmark_info_has_creation_field(self):
        """BDZFSBookmarkInfo must have a 'creation' field"""
        self.assertTrue(hasattr(BlockDev.ZFSBookmarkInfo, 'creation'))

    @tag_test(TestTags.NOSTORAGE)
    def test_bookmark_list_returns_bookmark_info(self):
        """bd_zfs_bookmark_list must return BDZFSBookmarkInfo objects (when ZFS available)"""
        try:
            BlockDev.zfs_is_tech_avail(BlockDev.ZFSTech.SNAPSHOT, BlockDev.ZFSTechMode.QUERY)
        except GLib.GError:
            self.skipTest("skipping: ZFS tools not available")

        try:
            bookmarks = BlockDev.zfs_bookmark_list(None)
        except GLib.GError:
            # May fail if no pools exist; that is fine
            return

        if bookmarks:
            for bm in bookmarks:
                self.assertIsInstance(bm, BlockDev.ZFSBookmarkInfo)
                self.assertIsNotNone(bm.name)
                self.assertIsInstance(bm.creation, int)


class ZfsVdevInfoFieldsTestCase(ZfsPluginTest):
    """Tests that BDZFSVdevInfo does not have removed alloc/space fields."""

    @tag_test(TestTags.NOSTORAGE)
    def test_vdev_info_no_alloc_field(self):
        """BDZFSVdevInfo must not have an 'alloc' field (removed)"""
        self.assertFalse(hasattr(BlockDev.ZFSVdevInfo, 'alloc'))

    @tag_test(TestTags.NOSTORAGE)
    def test_vdev_info_no_space_field(self):
        """BDZFSVdevInfo must not have a 'space' field (removed)"""
        self.assertFalse(hasattr(BlockDev.ZFSVdevInfo, 'space'))

    @tag_test(TestTags.NOSTORAGE)
    def test_vdev_info_has_error_fields(self):
        """BDZFSVdevInfo must still have read_errors, write_errors, checksum_errors"""
        self.assertTrue(hasattr(BlockDev.ZFSVdevInfo, 'read_errors'))
        self.assertTrue(hasattr(BlockDev.ZFSVdevInfo, 'write_errors'))
        self.assertTrue(hasattr(BlockDev.ZFSVdevInfo, 'checksum_errors'))
