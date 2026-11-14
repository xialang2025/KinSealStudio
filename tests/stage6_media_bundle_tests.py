#!/usr/bin/env python3
"""Adversarial offline copy-transaction tests; fixtures are not hardware evidence.

Set KINSEAL_STAGE6_TEST_ROOT to an owned task directory before running. The suite
never uses the system temporary directory or discovers/opens removable devices.
"""

#20261114-5 add
import contextlib
import hashlib
import io
import json
import os
from pathlib import Path
import stat
import struct
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest import mock
import warnings
import zipfile
import zlib

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import stage6_media_bundle as bundle


# This is one preserved, observed load.bin identity, not a metadata encoder.
OBSERVED_LOAD_HEX = (
    "00000028004b0069006e005300650061006c00530074007500640069006f002d00560031002e"
    "0030002e00320000007a000000280041004d00540030003700300057003100320034002d004c"
    "00380030003000580034003800300052000000010000000000010000000000000000000000"
    "0000000100000040004c0069006e006b002f00730065007200690061006c005f0061006d007a"
    "002f004d004f0044004200550053002f005f0063006f006d0031002e00620069006e00000000"
    "00000001ffffffff000000000000000200000000000000014089000000000000407e000000"
    "00000000000000"
)


class DescriptorBuffer(io.BytesIO):
    """Force a standard ZIP writer to emit signed data descriptors."""

    # /**************************************************************************
    # Function Name   : seek
    # Description     : Model a nonseekable ZIP sink, as required by this fixture.
    # Input Parameters: args, kwargs: ignored seek request.
    # Return Value    : None; always raises UnsupportedOperation.
    # Date            : 2026.11.14
    # **************************************************************************/
    def seek(self, *args, **kwargs):
        raise io.UnsupportedOperation("Nonseekable fixture sink")


class MediaBundleTests(unittest.TestCase):
    """Check corrupt inputs, source preservation, and no-overwrite publication."""

    # /**************************************************************************
    # Function Name   : setUp
    # Description     : Create only a new per-test directory under the explicit task root.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def setUp(self):
        configured = os.environ.get("KINSEAL_STAGE6_TEST_ROOT")
        if not configured:
            raise RuntimeError("Set KINSEAL_STAGE6_TEST_ROOT; system TEMP is not used")
        task_root = Path(configured).absolute()
        task_root.mkdir(parents=True, exist_ok=True)
        self.temporary = tempfile.TemporaryDirectory(prefix="case-", dir=task_root)
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.source = self.root / "native" / "compile"
        self.source.mkdir(parents=True)
        for ordinal, name in enumerate(bundle.ZIP_MEMBERS):
            (self.source / name).write_bytes(bytes([ordinal]) * (ordinal + 3))
        (self.source / "load.bin").write_bytes(bytes.fromhex(OBSERVED_LOAD_HEX))
        (self.source / "Usbload.bin").write_bytes(b"\x00" * 1704)
        payload = b"Synthetic KNR fixture; this is not a native project"
        (self.source / "upLoad.knr").write_bytes(struct.pack(">I", len(payload)) +
                                                zlib.compress(payload))
        self.write_zip()

    # /**************************************************************************
    # Function Name   : write_zip
    # Description     : Build synthetic containers for structural corruption tests.
    # Input Parameters: names, method, descriptor, comment: fixture variants.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def write_zip(self, names=None, method=zipfile.ZIP_DEFLATED, descriptor=True,
                  comment=b""):
        target = DescriptorBuffer() if descriptor else io.BytesIO()
        names = bundle.ZIP_MEMBERS if names is None else names
        with warnings.catch_warnings():
            warnings.simplefilter("ignore", UserWarning)
            with zipfile.ZipFile(target, "w", compression=method) as archive:
                for name in names:
                    info = zipfile.ZipInfo(name, (2026, 11, 14, 12, 0, 0))
                    info.create_system = 3
                    info.external_attr = (stat.S_IFREG | 0o600) << 16
                    source = self.source / Path(name).name
                    data = source.read_bytes() if source.is_file() else b"unknown fixture"
                    archive.writestr(info, data, compress_type=method)
                archive.comment = comment
        (self.source / "load.kns").write_bytes(target.getvalue())

    # /**************************************************************************
    # Function Name   : snapshot
    # Description     : Hash all existing source files for preservation assertions.
    # Input Parameters: None.
    # Return Value    : Filename to digest mapping.
    # Date            : 2026.11.14
    # **************************************************************************/
    def snapshot(self):
        return {p.name: hashlib.sha256(p.read_bytes()).hexdigest()
                for p in self.source.iterdir() if p.is_file()}

    # /**************************************************************************
    # Function Name   : assert_refused
    # Description     : Require an explicit format refusal without source modification.
    # Input Parameters: message: expected refusal fragment.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def assert_refused(self, message=None):
        before = self.snapshot()
        with self.assertRaises(bundle.BundleError) as caught:
            bundle.validate_bundle(self.source)
        if message:
            self.assertIn(message, str(caught.exception))
        self.assertEqual(before, self.snapshot())

    # /**************************************************************************
    # Function Name   : test_valid_manifest_and_source_preservation
    # Description     : Confirm full hashes and honest provenance for valid structure.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_valid_manifest_and_source_preservation(self):
        before = self.snapshot()
        result = bundle.validate_bundle(self.source)
        self.assertEqual(result["mode"], "copy-only")
        self.assertFalse(result["generatorAuthenticated"])
        self.assertEqual(len(result["files"]), 14)
        self.assertEqual(len(result["zip"]["members"]), 13)
        for record in result["files"]:
            self.assertEqual(record["sha256"], before[Path(record["path"]).name])
        self.assertEqual(result["identityScope"]["loadBinSha256"], bundle.LOAD_BIN_SHA256)
        self.assertEqual(before, self.snapshot())

    # /**************************************************************************
    # Function Name   : test_missing_or_unknown_loose_files
    # Description     : Reject incomplete exports and unrecognized extra files.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_missing_or_unknown_loose_files(self):
        (self.source / "AddrPara.dat").rename(self.source / "unknown.dat")
        self.assert_refused("exactly 14")

    # /**************************************************************************
    # Function Name   : test_directory_in_place_of_file
    # Description     : Reject a directory even when its name is a required member.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_directory_in_place_of_file(self):
        path = self.source / "AddrPara.dat"
        path.rename(self.root / "saved.dat")
        path.mkdir()
        self.assert_refused("ordinary file")

    # /**************************************************************************
    # Function Name   : test_source_symlink
    # Description     : Refuse redirected sources when OS permissions allow link creation.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_source_symlink(self):
        link = self.root / "linked-source"
        try:
            link.symlink_to(self.source, target_is_directory=True)
        except OSError as error:
            self.skipTest(f"Symlink privilege unavailable: {error}")
        with self.assertRaisesRegex(bundle.BundleError, "Links and reparse"):
            bundle.validate_bundle(link)

    # /**************************************************************************
    # Function Name   : test_windows_reparse_attribute_is_refused
    # Description     : Cover reparse rejection without requiring link-creation privilege.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_windows_reparse_attribute_is_refused(self):
        original = Path.lstat

        # /**********************************************************************
        # Function Name   : marked_reparse
        # Description     : Inject Windows junction metadata only for the fixture root.
        # Input Parameters: path: lstat target; args, kwargs: forwarded arguments.
        # Return Value    : Real metadata or the reparse-marked metadata stub.
        # Date            : 2026.11.14
        # **********************************************************************/
        def marked_reparse(path, *args, **kwargs):
            result = original(path, *args, **kwargs)
            if path == self.source:
                return SimpleNamespace(st_mode=result.st_mode,
                                       st_file_attributes=bundle.REPARSE_ATTRIBUTE)
            return result

        with mock.patch.object(Path, "lstat", marked_reparse):
            with self.assertRaisesRegex(bundle.BundleError, "Links and reparse"):
                bundle.validate_bundle(self.source)

    # /**************************************************************************
    # Function Name   : test_unknown_load_identity
    # Description     : Reject an unproved metadata identity even with a matching ZIP.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_unknown_load_identity(self):
        data = bytearray((self.source / "load.bin").read_bytes())
        data[-1] ^= 1
        (self.source / "load.bin").write_bytes(data)
        self.write_zip()
        self.assert_refused("load.bin")

    # /**************************************************************************
    # Function Name   : test_unknown_usbload_size
    # Description     : Reject a media metadata length outside the observed target scope.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_unknown_usbload_size(self):
        (self.source / "Usbload.bin").write_bytes(b"unproved")
        self.write_zip()
        self.assert_refused("Usbload.bin")

    # /**************************************************************************
    # Function Name   : test_zip_and_loose_content_mismatch
    # Description     : Detect same-length loose-file corruption outside the ZIP.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_zip_and_loose_content_mismatch(self):
        (self.source / "AddrPara.dat").write_bytes(b"bad")
        self.assert_refused("differs from loose file")

    # /**************************************************************************
    # Function Name   : test_zip_bad_crc_with_consistent_descriptor
    # Description     : Reject a false CRC even when central and descriptor values agree.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_zip_bad_crc_with_consistent_descriptor(self):
        path = self.source / "load.kns"
        data = bytearray(path.read_bytes())
        central = data.index(b"PK\x01\x02")
        descriptor = data.index(b"PK\x07\x08")
        wrong = struct.unpack_from("<I", data, central + 16)[0] ^ 1
        struct.pack_into("<I", data, central + 16, wrong)
        struct.pack_into("<I", data, descriptor + 4, wrong)
        path.write_bytes(data)
        self.assert_refused("bad CRC")

    # /**************************************************************************
    # Function Name   : test_zip_bad_descriptor
    # Description     : Reject descriptor corruption that a generic ZIP reader may ignore.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_zip_bad_descriptor(self):
        path = self.source / "load.kns"
        data = bytearray(path.read_bytes())
        data[data.index(b"PK\x07\x08") + 4] ^= 1
        path.write_bytes(data)
        self.assert_refused("data descriptor")

    # /**************************************************************************
    # Function Name   : test_zip_symlink_member
    # Description     : Reject a ZIP symlink even when its required name is unchanged.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_zip_symlink_member(self):
        path = self.source / "load.kns"
        data = bytearray(path.read_bytes())
        central = data.index(b"PK\x01\x02")
        struct.pack_into("<I", data, central + 38, (stat.S_IFLNK | 0o777) << 16)
        path.write_bytes(data)
        self.assert_refused("ZIP links")

    # /**************************************************************************
    # Function Name   : test_zip_duplicate_members
    # Description     : Reject duplicate names without extracting any file.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_zip_duplicate_members(self):
        self.write_zip((*bundle.ZIP_MEMBERS[:-1], bundle.ZIP_MEMBERS[0]))
        self.assert_refused("ZIP")

    # /**************************************************************************
    # Function Name   : test_zip_unknown_traversal_and_absolute_names
    # Description     : Reject unrecognized names and paths before any extraction.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_zip_unknown_traversal_and_absolute_names(self):
        for name in ("../escaped.dat", "/escaped.dat", "C:/escaped.dat", "unknown.dat"):
            with self.subTest(name=name):
                self.write_zip((name, *bundle.ZIP_MEMBERS[1:]))
                self.assert_refused("ZIP")
        self.assertFalse((self.root / "escaped.dat").exists())

    # /**************************************************************************
    # Function Name   : test_zip_reordered_members
    # Description     : Enforce the bounded member sequence observed in native exports.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_zip_reordered_members(self):
        self.write_zip(tuple(reversed(bundle.ZIP_MEMBERS)))
        self.assert_refused("members or order")

    # /**************************************************************************
    # Function Name   : test_zip_unsupported_method_or_flags
    # Description     : Refuse ZIP variants outside the observed deflate/descriptor layout.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_zip_unsupported_method_or_flags(self):
        self.write_zip(method=zipfile.ZIP_STORED)
        self.assert_refused("Unsupported ZIP flags")
        self.write_zip(descriptor=False)
        self.assert_refused("Unsupported ZIP flags")

    # /**************************************************************************
    # Function Name   : test_zip_extra_trailing_or_prefixed_data
    # Description     : Reject comments, arbitrary tails and self-extracting prefixes.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_zip_extra_trailing_or_prefixed_data(self):
        self.write_zip(comment=b"unobserved")
        self.assert_refused("ZIP end record")
        self.write_zip()
        path = self.source / "load.kns"
        original = path.read_bytes()
        for changed in (original + b"tail", b"prefix" + original, original[:100]):
            with self.subTest(length=len(changed)):
                path.write_bytes(changed)
                self.assert_refused("ZIP")

    # /**************************************************************************
    # Function Name   : test_knr_length_and_trailing_data
    # Description     : Reject corrupt or oversized zlib envelopes independently of ZIP.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_knr_length_and_trailing_data(self):
        path = self.source / "upLoad.knr"
        original = path.read_bytes()
        variants = (b"\x00", struct.pack(">I", 1) + original[4:], original + b"tail",
                    struct.pack(">I", bundle.MAX_FILE_BYTES + 1) + original[4:])
        for data in variants:
            with self.subTest(length=len(data)):
                path.write_bytes(data)
                self.assert_refused()

    # /**************************************************************************
    # Function Name   : test_per_file_and_total_size_limits
    # Description     : Check refusal boundaries without allocating large temporary files.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_per_file_and_total_size_limits(self):
        with mock.patch.object(bundle, "MAX_FILE_BYTES", 100):
            self.assert_refused("size limit")
        with mock.patch.object(bundle, "MAX_TOTAL_BYTES", 100):
            self.assert_refused("total size")

    # /**************************************************************************
    # Function Name   : test_zip_declared_size_limit
    # Description     : Refuse an inconsistent huge expansion before decompression.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_zip_declared_size_limit(self):
        path = self.source / "load.kns"
        data = bytearray(path.read_bytes())
        central = data.index(b"PK\x01\x02")
        struct.pack_into("<I", data, central + 24, 0x7FFFFFFF)
        path.write_bytes(data)
        self.assert_refused("size differs")

    # /**************************************************************************
    # Function Name   : test_zip_large_central_rejected_before_general_parser
    # Description     : Bound allocations before a forged thirteen-entry end record is parsed.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_zip_large_central_rejected_before_general_parser(self):  #20261114-5 add
        header = struct.pack("<4s6H3I5H2I", b"PK\x01\x02", 788, 20, 8, 8,
                             0, 0, 0, 2, 0, 1, 0, 0, 0, 0, stat.S_IFREG << 16, 0) + b"a"
        central = header * 2000
        data = central + struct.pack("<4s4H2IH", b"PK\x05\x06", 0, 0, 13, 13,
                                     len(central), 0, 0)
        with mock.patch.object(bundle.zipfile, "ZipFile",
                               side_effect=AssertionError("General parser was invoked")) as parser:
            with self.assertRaisesRegex(bundle.BundleError, "central directory size"):
                bundle._validate_zip(data, {})
        parser.assert_not_called()

    # /**************************************************************************
    # Function Name   : test_zip_unsupported_version_is_json_before_general_parser
    # Description     : Reject a version-6.4 central header without constructing ZipFile.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_zip_unsupported_version_is_json_before_general_parser(self):  #20261114-5 add
        path = self.source / "load.kns"
        data = bytearray(path.read_bytes())
        central = data.index(b"PK\x01\x02")
        struct.pack_into("<H", data, central + 6, 64)
        path.write_bytes(data)
        errors = io.StringIO()
        with mock.patch.object(bundle.zipfile, "ZipFile",
                               side_effect=AssertionError("General parser was invoked")) as parser:
            with contextlib.redirect_stderr(errors):
                status = bundle.main(["validate", str(self.source)])
        parser.assert_not_called()
        self.assertEqual(status, 2)
        result = json.loads(errors.getvalue())
        self.assertEqual(result["status"], "refused")
        self.assertIn("required version", result["error"])
        self.assertNotIn("Traceback", errors.getvalue())

    # /**************************************************************************
    # Function Name   : test_zip_library_not_implemented_is_json
    # Description     : Convert a library unsupported-feature exception to a bounded refusal.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_zip_library_not_implemented_is_json(self):  #20261114-5 add
        errors = io.StringIO()
        with mock.patch.object(bundle.zipfile, "ZipFile",
                               side_effect=NotImplementedError("unsupported ZIP feature")):
            with contextlib.redirect_stderr(errors):
                status = bundle.main(["validate", str(self.source)])
        self.assertEqual(status, 2)
        self.assertEqual(json.loads(errors.getvalue())["status"], "refused")
        self.assertIn("unsupported ZIP feature", errors.getvalue())
        self.assertNotIn("Traceback", errors.getvalue())

    # /**************************************************************************
    # Function Name   : test_cli_refusal_is_json_without_traceback
    # Description     : Return a bounded machine-readable refusal for invalid input.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    def test_cli_refusal_is_json_without_traceback(self):
        errors = io.StringIO()
        with contextlib.redirect_stderr(errors):
            status = bundle.main(["validate", str(self.root / "missing")])
        self.assertEqual(status, 2)
        self.assertEqual(json.loads(errors.getvalue())["status"], "refused")
        self.assertNotIn("Traceback", errors.getvalue())

    # /**************************************************************************
    # Function Name   : test_publication_has_exact_copies_and_manifest_hash
    # Description     : Confirm atomic publication preserves source and complete content.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    @unittest.skipUnless(os.name == "nt", "Windows no-replace publication contract")
    def test_publication_has_exact_copies_and_manifest_hash(self):
        before = self.snapshot()
        target = self.root / "published"
        receipt = bundle.publish_bundle(self.source, target)
        manifest_data = (target / "manifest.json").read_bytes()
        manifest = json.loads(manifest_data)
        self.assertEqual(receipt["manifestSha256"], hashlib.sha256(manifest_data).hexdigest())
        self.assertTrue(manifest["stagedCopiesVerified"])
        self.assertFalse(manifest["generatorAuthenticated"])
        self.assertEqual(manifest["mode"], "copy-only")
        self.assertEqual({p.name for p in target.iterdir()}, {"compile", "manifest.json"})
        self.assertEqual(before, self.snapshot())
        for name, digest in before.items():
            self.assertEqual(hashlib.sha256((target / "compile" / name).read_bytes()).hexdigest(),
                             digest)

    # /**************************************************************************
    # Function Name   : test_existing_destination_remains_untouched
    # Description     : Refuse replacement of either a file or an existing directory.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    @unittest.skipUnless(os.name == "nt", "Windows no-replace publication contract")
    def test_existing_destination_remains_untouched(self):
        for directory in (False, True):
            with self.subTest(directory=directory):
                target = self.root / ("existing-dir" if directory else "existing-file")
                if directory:
                    target.mkdir()
                    sentinel = target / "keep.txt"
                else:
                    sentinel = target
                sentinel.write_bytes(b"user content")
                with self.assertRaisesRegex(bundle.BundleError, "already exists"):
                    bundle.publish_bundle(self.source, target)
                self.assertEqual(sentinel.read_bytes(), b"user content")

    # /**************************************************************************
    # Function Name   : test_destination_inside_source_is_refused
    # Description     : Avoid introducing staged or final output into the native input.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    @unittest.skipUnless(os.name == "nt", "Windows no-replace publication contract")
    def test_destination_inside_source_is_refused(self):
        before = self.snapshot()
        with self.assertRaisesRegex(bundle.BundleError, "outside the source"):
            bundle.publish_bundle(self.source, self.source / "published")
        self.assertEqual(self.snapshot(), before)

    # /**************************************************************************
    # Function Name   : test_copy_failure_publishes_nothing
    # Description     : Roll back only the newly owned stage after a failed file write.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    @unittest.skipUnless(os.name == "nt", "Windows no-replace publication contract")
    def test_copy_failure_publishes_nothing(self):
        target = self.root / "failed"
        before = self.snapshot()
        with mock.patch.object(bundle, "_write_new_file", side_effect=OSError("disk full")):
            with self.assertRaisesRegex(bundle.BundleError, "disk full"):
                bundle.publish_bundle(self.source, target)
        self.assertFalse(target.exists())
        self.assertEqual(list(self.root.glob(".failed.kinseal-media-*")), [])
        self.assertEqual(self.snapshot(), before)

    # /**************************************************************************
    # Function Name   : test_cleanup_failure_reports_retained_stage_as_json
    # Description     : Report the owned retained path without an uncontrolled traceback.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    @unittest.skipUnless(os.name == "nt", "Windows no-replace publication contract")
    def test_cleanup_failure_reports_retained_stage_as_json(self):
        target = self.root / "cleanup-failed"
        errors = io.StringIO()
        with mock.patch.object(bundle, "_write_new_file", side_effect=OSError("disk full")):
            with mock.patch.object(bundle, "_discard_owned_stage",
                                   side_effect=PermissionError("mock cleanup denied")):
                with contextlib.redirect_stderr(errors):
                    status = bundle.main(["publish", str(self.source), str(target)])
        self.assertEqual(status, 2)
        error = json.loads(errors.getvalue())["error"]
        self.assertIn("retained for inspection", error)
        self.assertIn("disk full", error)
        self.assertFalse(target.exists())
        retained = list(self.root.glob(".cleanup-failed.kinseal-media-*"))
        self.assertEqual(len(retained), 1)
        self.assertIn(str(retained[0]), error)

    # /**************************************************************************
    # Function Name   : test_late_empty_destination_cannot_be_overwritten
    # Description     : Exercise real Windows rename when a target races final publication.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    @unittest.skipUnless(os.name == "nt", "Windows no-replace publication contract")
    def test_late_empty_destination_cannot_be_overwritten(self):
        target = self.root / "raced"
        real_rename = os.rename

        # /**********************************************************************
        # Function Name   : create_target_then_rename
        # Description     : Create the competing target at the final OS operation.
        # Input Parameters: source, destination: rename arguments.
        # Return Value    : OS rename result; Windows must raise FileExistsError.
        # Date            : 2026.11.14
        # **********************************************************************/
        def create_target_then_rename(source, destination):
            target.mkdir()
            return real_rename(source, destination)

        with mock.patch.object(bundle.os, "rename", side_effect=create_target_then_rename):
            with self.assertRaises(bundle.BundleError):
                bundle.publish_bundle(self.source, target)
        self.assertTrue(target.is_dir())
        self.assertEqual(list(target.iterdir()), [])
        self.assertEqual(list(self.root.glob(".raced.kinseal-media-*")), [])

    # /**************************************************************************
    # Function Name   : test_source_change_between_validation_and_copy
    # Description     : Abort if the native source snapshot changes before staging.
    # Input Parameters: None.
    # Return Value    : None.
    # Date            : 2026.11.14
    # **************************************************************************/
    @unittest.skipUnless(os.name == "nt", "Windows no-replace publication contract")
    def test_source_change_between_validation_and_copy(self):
        target = self.root / "changed"
        real_validate = bundle.validate_bundle

        # /**********************************************************************
        # Function Name   : change_source_after_validation
        # Description     : Simulate an external edit after the first validation.
        # Input Parameters: source: directory under validation.
        # Return Value    : Original pre-edit manifest.
        # Date            : 2026.11.14
        # **********************************************************************/
        def change_source_after_validation(source):
            manifest = real_validate(source)
            if Path(source) == self.source:
                (self.source / "AddrPara.dat").write_bytes(b"changed externally")
            return manifest

        with mock.patch.object(bundle, "validate_bundle", side_effect=change_source_after_validation):
            with self.assertRaisesRegex(bundle.BundleError, "changed before copying"):
                bundle.publish_bundle(self.source, target)
        self.assertFalse(target.exists())
        self.assertEqual(list(self.root.glob(".changed.kinseal-media-*")), [])


if __name__ == "__main__":
    unittest.main(verbosity=2)
