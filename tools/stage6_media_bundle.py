#!/usr/bin/env python3
"""Validate and copy an existing, narrowly scoped native KinSeal media bundle.

This tool does not compile a project or synthesize media metadata. The caller
supplies a native-generated compile directory; structural checks cannot certify
who generated its bytes. No USB, removable-media discovery, or device API is used.
"""

#20261114-5 add
import argparse
import hashlib
import io
import json
import os
from pathlib import Path
import shutil
import stat
import struct
import sys
import tempfile
from datetime import datetime, timezone
import zipfile
import zlib


ZIP_MEMBERS = (
    "AddrPara.dat", "AlarmMuti.dat", "AlarmPara.dat", "Collect.dat", "Curve.dat",
    "load.bin", "MarcoVar.dat", "PicLenPara.dat", "SheetPara.dat",
    "SheetPixel.dat", "SysPara.dat", "upLoad.knr", "Usbload.bin",
)
FILES = frozenset((*ZIP_MEMBERS, "load.kns"))
CENTRAL_DIRECTORY_BYTES = sum(46 + len(name.encode("ascii")) for name in ZIP_MEMBERS)
LOAD_BIN_BYTES = 233
LOAD_BIN_SHA256 = "2423b04f2e8371e9a22dd8b534186ff2f993cc43a45e31ae54adbd5897333303"
USBLOAD_BYTES = 1704
MAX_FILE_BYTES = 64 * 1024 * 1024
MAX_TOTAL_BYTES = 128 * 1024 * 1024
REPARSE_ATTRIBUTE = 0x400


class BundleError(ValueError):
    """The input or requested transaction is outside the supported boundary."""


# /******************************************************************************
# Function Name   : _absolute
# Description     : Normalize a path lexically without following links.
# Input Parameters: path: filesystem path.
# Return Value    : Absolute Path.
# Date            : 2026.11.14
# ******************************************************************************/
def _absolute(path):
    return Path(os.path.abspath(os.fspath(path)))


# /******************************************************************************
# Function Name   : _ordinary_path
# Description     : Reject symbolic links and Windows reparse points in a path.
# Input Parameters: path: existing absolute path.
# Return Value    : Final lstat result.
# Date            : 2026.11.14
# ******************************************************************************/
def _ordinary_path(path):
    result = None
    for component in (*reversed(path.parents), path):
        result = component.lstat()
        if stat.S_ISLNK(result.st_mode) or (
            getattr(result, "st_file_attributes", 0) & REPARSE_ATTRIBUTE
        ):
            raise BundleError(f"Links and reparse points are unsupported: {component}")
    return result


# /******************************************************************************
# Function Name   : _identity
# Description     : Capture metadata used to detect replacement during a read.
# Input Parameters: snapshot: stat result.
# Return Value    : Comparable file-identity tuple.
# Date            : 2026.11.14
# ******************************************************************************/
def _identity(snapshot):
    return (snapshot.st_dev, snapshot.st_ino, snapshot.st_size,
            snapshot.st_mtime_ns, snapshot.st_ctime_ns)


# /******************************************************************************
# Function Name   : _read_file
# Description     : Read a bounded ordinary file and reject concurrent replacement.
# Input Parameters: path: existing file; maximum: byte ceiling.
# Return Value    : Original file bytes.
# Date            : 2026.11.14
# ******************************************************************************/
def _read_file(path, maximum=MAX_FILE_BYTES):
    before = _ordinary_path(path)
    if not stat.S_ISREG(before.st_mode) or before.st_size > maximum:
        raise BundleError(f"Not an ordinary file within the size limit: {path}")
    flags = os.O_RDONLY | getattr(os, "O_BINARY", 0) | getattr(os, "O_NOFOLLOW", 0)
    descriptor = os.open(path, flags)
    with os.fdopen(descriptor, "rb") as stream:
        opened = os.fstat(stream.fileno())
        if _identity(before) != _identity(opened):
            raise BundleError(f"Source changed before reading: {path}")
        data = stream.read(maximum + 1)
        after = os.fstat(stream.fileno())
    final = _ordinary_path(path)
    if (len(data) != before.st_size or len(data) > maximum or
            _identity(before) != _identity(after) or
            _identity(before) != _identity(final)):
        raise BundleError(f"Source changed or exceeded the size limit: {path}")
    return data


# /******************************************************************************
# Function Name   : _sha256
# Description     : Hash complete content without normalizing unknown metadata.
# Input Parameters: data: original bytes.
# Return Value    : Lowercase SHA-256 hexadecimal string.
# Date            : 2026.11.14
# ******************************************************************************/
def _sha256(data):
    return hashlib.sha256(data).hexdigest()


# /******************************************************************************
# Function Name   : _inflate
# Description     : Decode exactly one bounded stream with no ignored trailing data.
# Input Parameters: data: compressed bytes; size: expected size; window: zlib mode.
# Return Value    : Validated uncompressed bytes.
# Date            : 2026.11.14
# ******************************************************************************/
def _inflate(data, size, window):
    if size < 0 or size > MAX_FILE_BYTES:
        raise BundleError("Uncompressed stream exceeds the size limit")
    decoder = zlib.decompressobj(window)
    decoded = decoder.decompress(data, size + 1)
    if (len(decoded) != size or not decoder.eof or decoder.unused_data or
            decoder.unconsumed_tail):
        raise BundleError("Invalid compressed stream, length, or trailing data")
    return decoded


# /******************************************************************************
# Function Name   : _source_entries
# Description     : Admit only the fourteen exact files observed in native exports.
# Input Parameters: root: existing compile directory.
# Return Value    : Filename to bounded file-size mapping.
# Date            : 2026.11.14
# ******************************************************************************/
def _source_entries(root):
    snapshot = _ordinary_path(root)
    if not stat.S_ISDIR(snapshot.st_mode):
        raise BundleError("Source must be an ordinary compile directory")
    entries = list(root.iterdir())
    names = {entry.name for entry in entries}
    if names != FILES or len(entries) != len(FILES):
        missing = sorted(FILES - names)
        unknown = sorted(names - FILES)
        raise BundleError(f"Expected exactly 14 files; missing={missing}, unknown={unknown}")
    sizes = {}
    for entry in entries:
        snapshot = _ordinary_path(entry)
        if not stat.S_ISREG(snapshot.st_mode) or snapshot.st_size > MAX_FILE_BYTES:
            raise BundleError(f"Not an ordinary file within the size limit: {entry}")
        sizes[entry.name] = snapshot.st_size
    if sum(sizes.values()) > MAX_TOTAL_BYTES:
        raise BundleError("Bundle exceeds the total size limit")
    return sizes


# /******************************************************************************
# Function Name   : _bounded_central_headers
# Description     : Admit only thirteen fixed-size headers before allocating ZipInfo objects.
# Input Parameters: data: ZIP bytes; offset: central start; end_offset: EOCD start.
# Return Value    : Thirteen validated central-header tuples.
# Date            : 2026.11.14
# ******************************************************************************/
def _bounded_central_headers(data, offset, end_offset):  #20261114-5 add
    if end_offset - offset != CENTRAL_DIRECTORY_BYTES:
        raise BundleError("ZIP central directory size is outside the fixed 13-member layout")
    cursor = offset
    result = []
    # The observed layout has no extra fields or comments. A fixed byte ceiling
    # and bounded loop prevent the general ZIP parser from allocating an attacker-
    # controlled number of entries, even when its end record falsely declares 13.
    for name in ZIP_MEMBERS:
        if cursor + 46 > end_offset:
            raise BundleError("Truncated ZIP central directory")
        central = struct.unpack_from("<4s6H3I5H2I", data, cursor)
        variable_end = cursor + 46 + central[10] + central[11] + central[12]
        if (central[0] != b"PK\x01\x02" or variable_end > end_offset or
                central[11:14] != (0, 0, 0)):
            raise BundleError("Unsupported ZIP central entry, extra data, or disk")
        raw_name = data[cursor + 46:cursor + 46 + central[10]]
        if raw_name != name.encode("ascii"):
            raise BundleError("ZIP members or order differ from the observed 13-member layout")
        if central[2] != 20:
            raise BundleError(f"Unsupported ZIP required version before general parsing: {central[2]}")
        result.append(central)
        cursor = variable_end
    if cursor != end_offset:
        raise BundleError("ZIP central directory contains unaccounted records or bytes")
    return result


# /******************************************************************************
# Function Name   : _validate_zip
# Description     : Check the observed ZIP envelope, exact members, CRCs and copies.
# Input Parameters: data: load.kns bytes; records: loose-file hash/size mapping.
# Return Value    : Manifest entries for the thirteen validated ZIP members.
# Date            : 2026.11.14
# ******************************************************************************/
def _validate_zip(data, records):  #20261114-5 change
    if len(data) < 22:
        raise BundleError("Truncated ZIP end record")
    end_offset = len(data) - 22
    end = struct.unpack_from("<4s4H2IH", data, end_offset)
    if (end[0] != b"PK\x05\x06" or end[1:3] != (0, 0) or
            end[3:5] != (13, 13) or end[7] != 0 or
            end[6] + end[5] != end_offset):
        raise BundleError("Unsupported ZIP end record, disks, comments, or bounds")
    directory_offset = end[6]
    central_headers = _bounded_central_headers(data, directory_offset, end_offset)
    next_local_offset = 0
    result = []
    with zipfile.ZipFile(io.BytesIO(data), "r") as archive:
        infos = archive.infolist()
        names = [info.filename for info in infos]
        if len(set(names)) != len(names):
            raise BundleError("Duplicate ZIP members are forbidden")
        if tuple(names) != ZIP_MEMBERS:
            raise BundleError("ZIP members or order differ from the observed 13-member layout")
        for info, central in zip(infos, central_headers):
            raw_name = info.filename.encode("ascii")
            mode = (central[15] >> 16) & 0o170000
            if mode not in (0, stat.S_IFREG) or info.is_dir():
                raise BundleError("ZIP links and directory entries are forbidden")
            if (central[2] != 20 or central[3] != 8 or central[4] != 8 or
                    info.flag_bits != 8 or info.compress_type != zipfile.ZIP_DEFLATED or
                    central[7:10] != (info.CRC, info.compress_size, info.file_size) or
                    central[16] != info.header_offset or info.header_offset != next_local_offset):
                raise BundleError("Unsupported ZIP flags, method, version, or member bounds")
            if info.file_size != records[info.filename]["bytes"]:
                raise BundleError(f"ZIP member size differs from loose file: {info.filename}")
            if info.header_offset + 30 > directory_offset:
                raise BundleError("Truncated ZIP local header")
            local = struct.unpack_from("<4s5H3I2H", data, info.header_offset)
            payload_offset = info.header_offset + 30 + local[9] + local[10]
            local_name = data[info.header_offset + 30:info.header_offset + 30 + local[9]]
            if (local[0] != b"PK\x03\x04" or local[1:6] != central[2:7] or
                    local_name != raw_name or local[10] != 0 or
                    local[6] not in (0, info.CRC) or
                    local[7] not in (0, info.compress_size) or
                    local[8] not in (0, info.file_size)):
                raise BundleError("ZIP local header disagrees with central directory")
            payload_end = payload_offset + info.compress_size
            next_local_offset = payload_end + 16
            if next_local_offset > directory_offset:
                raise BundleError("ZIP payload or descriptor overlaps central directory")
            descriptor = struct.pack("<4s3I", b"PK\x07\x08", info.CRC,
                                     info.compress_size, info.file_size)
            if data[payload_end:next_local_offset] != descriptor:
                raise BundleError("Missing or inconsistent ZIP data descriptor")
            payload = _inflate(data[payload_offset:payload_end], info.file_size, -15)
            digest = _sha256(payload)
            if zlib.crc32(payload) & 0xFFFFFFFF != info.CRC:
                raise BundleError(f"ZIP member has a bad CRC: {info.filename}")
            if digest != records[info.filename]["sha256"]:
                raise BundleError(f"ZIP member differs from loose file: {info.filename}")
            result.append({"name": info.filename, "bytes": info.file_size,
                           "compressedBytes": info.compress_size, "sha256": digest,
                           "crc32": f"{info.CRC:08x}", "method": 8})
    if next_local_offset != directory_offset:
        raise BundleError("ZIP contains unaccounted data or inconsistent directory bounds")
    return result


# /******************************************************************************
# Function Name   : validate_bundle
# Description     : Inspect existing native files without extracting or modifying them.
# Input Parameters: source_compile: native-generated fourteen-file directory.
# Return Value    : Complete content manifest; raises BundleError on refusal.
# Date            : 2026.11.14
# ******************************************************************************/
def validate_bundle(source_compile):
    try:
        root = _absolute(source_compile)
        sizes = _source_entries(root)
        records = {}
        for name in sorted(FILES):
            data = _read_file(root / name)
            if len(data) != sizes[name]:
                raise BundleError(f"Source size changed: {name}")
            records[name] = {"path": f"compile/{name}", "bytes": len(data),
                             "sha256": _sha256(data)}
        load = records["load.bin"]
        if load["bytes"] != LOAD_BIN_BYTES or load["sha256"] != LOAD_BIN_SHA256:
            raise BundleError("load.bin is outside the observed 233-byte AMT identity")
        if records["Usbload.bin"]["bytes"] != USBLOAD_BYTES:
            raise BundleError("Usbload.bin is outside the observed 1704-byte scope")
        knr = _read_file(root / "upLoad.knr")
        if _sha256(knr) != records["upLoad.knr"]["sha256"]:
            raise BundleError("KNR source changed during validation")
        if len(knr) < 6:
            raise BundleError("Truncated KNR envelope")
        knr_bytes = struct.unpack_from(">I", knr)[0]
        _inflate(knr[4:], knr_bytes, 15)
        zip_data = _read_file(root / "load.kns")
        if _sha256(zip_data) != records["load.kns"]["sha256"]:
            raise BundleError("ZIP source changed during validation")
        members = _validate_zip(zip_data, records)
        if _source_entries(root) != sizes:
            raise BundleError("Source directory changed during validation")
        return {
            "schemaVersion": 1, "mode": "copy-only", "sourceCompileDir": str(root),
            "originClaim": "Caller-supplied native-generated compile artifacts",
            "generatorAuthenticated": False,
            "identityScope": {"loadBinBytes": LOAD_BIN_BYTES,
                              "loadBinSha256": LOAD_BIN_SHA256,
                              "observedTarget": "AMT070W124-L800X480R",
                              "usbloadBytes": USBLOAD_BYTES},
            "files": [records[name] for name in sorted(FILES)],
            "zip": {"members": members, "crcValid": True, "looseCopiesEqual": True},
            "knrUncompressedBytes": knr_bytes,
            "limits": {"perFileBytes": MAX_FILE_BYTES, "totalBytes": MAX_TOTAL_BYTES,
                       "zipMembers": len(ZIP_MEMBERS),
                       "zipCentralDirectoryBytes": CENTRAL_DIRECTORY_BYTES},
            "limitations": [
                "No compilation, metadata synthesis, or normalization is performed.",
                "Structural checks and one known load.bin hash cannot authenticate the generator.",
                "No firmware consumer, removable-media acceptance, or real-screen upgrade is verified.",
            ],
        }
    except BundleError:
        raise
    except (OSError, ValueError, NotImplementedError, zipfile.BadZipFile, zlib.error) as error:
        raise BundleError(f"Bundle validation failed: {error}") from error


# /******************************************************************************
# Function Name   : _write_new_file
# Description     : Write and flush a new staged file while refusing replacement.
# Input Parameters: path: absent stage path; data: exact bytes to write.
# Return Value    : None.
# Date            : 2026.11.14
# ******************************************************************************/
def _write_new_file(path, data):
    with path.open("xb") as stream:
        stream.write(data)
        stream.flush()
        os.fsync(stream.fileno())


# /******************************************************************************
# Function Name   : _discard_owned_stage
# Description     : Remove only this call's unchanged, randomly named staging root.
# Input Parameters: stage: owned path; parent: checked parent; identity: inode pair.
# Return Value    : None; refuses a replaced or redirected staging path.
# Date            : 2026.11.14
# ******************************************************************************/
def _discard_owned_stage(stage, parent, identity):
    snapshot = _ordinary_path(stage)
    if (stage.parent != parent or (snapshot.st_dev, snapshot.st_ino) != identity or
            not stat.S_ISDIR(snapshot.st_mode)):
        raise BundleError(f"Staging identity changed; retained for inspection: {stage}")
    shutil.rmtree(stage)


# /******************************************************************************
# Function Name   : publish_bundle
# Description     : Verify exact copies in a sibling stage, then publish without overwrite.
# Input Parameters: source_compile: native artifacts; destination: new directory.
# Return Value    : Publication receipt including complete manifest SHA-256.
# Date            : 2026.11.14
# ******************************************************************************/
def publish_bundle(source_compile, destination):
    stage = None
    stage_identity = None
    try:
        if os.name != "nt":
            raise BundleError("Atomic no-replace directory publication currently requires Windows")
        source = _absolute(source_compile)
        target = _absolute(destination)
        parent = target.parent
        if target == source or source in target.parents:
            raise BundleError("Destination must be outside the source compile directory")
        if os.path.lexists(target):
            raise BundleError(f"Destination already exists: {target}")
        if not stat.S_ISDIR(_ordinary_path(parent).st_mode):
            raise BundleError("Destination parent must be an existing ordinary directory")
        original = validate_bundle(source)
        stage = Path(tempfile.mkdtemp(prefix=f".{target.name}.kinseal-media-", dir=parent))
        snapshot = stage.lstat()
        stage_identity = (snapshot.st_dev, snapshot.st_ino)
        (stage / "compile").mkdir()
        for record in original["files"]:
            name = Path(record["path"]).name
            data = _read_file(source / name)
            if len(data) != record["bytes"] or _sha256(data) != record["sha256"]:
                raise BundleError(f"Source changed before copying: {name}")
            _write_new_file(stage / "compile" / name, data)
        copied = validate_bundle(stage / "compile")
        if copied["files"] != original["files"] or copied["zip"] != original["zip"]:
            raise BundleError("Staged content differs from the validated source snapshot")
        manifest = dict(original)
        manifest.update({
            "publishedAtUtc": datetime.now(timezone.utc).isoformat(),
            "destination": str(target), "stagedCopiesVerified": True,
            "publication": "Windows no-replace rename after staged validation",
            "powerLossDurabilityVerified": False,
        })
        manifest_data = (json.dumps(manifest, ensure_ascii=True, sort_keys=True,
                                    indent=2) + "\n").encode("utf-8")
        _write_new_file(stage / "manifest.json", manifest_data)
        if os.path.lexists(target):
            raise BundleError(f"Destination appeared before publication: {target}")
        # Windows rename refuses an existing destination, including an empty directory.
        # POSIX rename has different semantics and is deliberately excluded above.
        os.rename(stage, target)
        stage = None
        return {"status": "published", "mode": "copy-only", "destination": str(target),
                "manifest": str(target / "manifest.json"),
                "manifestSha256": _sha256(manifest_data), "filesCopied": len(FILES)}
    except BundleError:
        raise
    except (OSError, ValueError) as error:
        raise BundleError(f"Bundle publication failed: {error}") from error
    finally:
        if stage is not None:
            original_failure = sys.exc_info()[1]
            try:
                _discard_owned_stage(stage, parent, stage_identity)
            except (OSError, BundleError) as cleanup_error:
                raise BundleError(
                    f"Staging cleanup failed; retained for inspection: {stage}; "
                    f"cleanup error: {cleanup_error}; original failure: {original_failure}"
                ) from cleanup_error


# /******************************************************************************
# Function Name   : main
# Description     : Expose inspection and explicit new-directory copy transactions.
# Input Parameters: argv: optional command-line arguments.
# Return Value    : Process exit status, zero for success and two for refusal.
# Date            : 2026.11.14
# ******************************************************************************/
def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    validate = commands.add_parser("validate", help="Inspect existing native compile artifacts")
    validate.add_argument("source_compile")
    publish = commands.add_parser("publish", help="Copy validated artifacts into a new directory")
    publish.add_argument("source_compile")
    publish.add_argument("destination")
    options = parser.parse_args(argv)
    try:
        if options.command == "validate":
            result = validate_bundle(options.source_compile)
        else:
            result = publish_bundle(options.source_compile, options.destination)
        print(json.dumps(result, ensure_ascii=True, indent=2))
        return 0
    except BundleError as error:
        print(json.dumps({"status": "refused", "error": str(error)}, ensure_ascii=True),
              file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
