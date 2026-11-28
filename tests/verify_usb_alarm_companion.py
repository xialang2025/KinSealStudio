"""Check explicitly requested same-run AlarmPara byte predictions against archived V5.1 files."""
# 20261128-30 add
import argparse
from collections import Counter, defaultdict
from hashlib import sha256
import io
from itertools import combinations
import json
from pathlib import Path
import struct
import subprocess
import sys
from tempfile import TemporaryDirectory
from zipfile import ZipFile
import zlib

from verify_amz_usb_media_descriptor import ROOT, clock_seconds
from rebuild_usb_metadata_fields import (
    LENGTH_FILES, Stream, earliest_native_seed_time, read_metadata, render_descriptor, same_build_alarm_ranges,
)


def digest(data):
    """Identify original bytes without relying on paths or timestamps."""
    return sha256(data).hexdigest()


def compare_member(raw, source_key):
    """Generate the candidate from other members before opening the target descriptor."""
    with ZipFile(io.BytesIO(raw)) as archive:
        names = {i.filename.lower(): i for i in archive.infolist()}
        alarm = archive.read(names["alarmpara.dat"])
        meta = archive.read(names["load.bin"])
        system = archive.read(names["syspara.dat"])
        t = earliest_native_seed_time(system, names["load.bin"].date_time)
        lengths = {n: names[n].file_size for n in LENGTH_FILES if n in names}
        baseline = render_descriptor(meta, lengths, clock_seconds(t))
        comparison = render_descriptor(meta, lengths, clock_seconds(t), native_alarm=alarm)
        fields = same_build_alarm_ranges(meta, alarm)
        if not source_key:
            source_key = digest(zlib.decompress(archive.read(names["upload.knr"])[4:]))
        truth = archive.read(names["usbload.bin"])
    positions = [p for off, value in fields.items() for p in range(off, off+len(value))]
    if len(set(positions)) != 35 or any(comparison[i] != truth[i] for i in positions):
        raise ValueError("alarm projections disagree with the original: " + digest(raw))
    if any(comparison[i] != baseline[i] for i in set(range(1704)) - set(positions)):
        raise ValueError("projection changed another field")
    return dict(archive=digest(raw), source=source_key, original=digest(truth), alarm=digest(alarm),
                selectedHex=bytes(truth[i] for i in positions).hex(), independentSample=digest(comparison),
                selectedPreviouslyExact=sum(baseline[i] == truth[i] for i in positions),
                remainingBefore=sum(a != b for a, b in zip(baseline, truth)),
                remainingAfter=sum(a != b for a, b in zip(comparison, truth)),
                model=read_metadata(meta)["model"], pages=len(read_metadata(meta)["sizes"]))


def verify_cli(raw):
    """Exercise the public opt-in without a target Usbload file, preserving default behavior and prior outputs."""
    with TemporaryDirectory(prefix="descriptor-comparison-", dir=ROOT / "build") as tmp, ZipFile(io.BytesIO(raw)) as archive:
        work = Path(tmp)
        for member in archive.namelist():
            if member.lower() in {"load.bin", "alarmpara.dat", *LENGTH_FILES}:
                (work / member.lower()).write_bytes(archive.read(member))
        cli = [sys.executable, "-B", str(ROOT / "tools/rebuild_usb_metadata_fields.py"), str(work), "--seconds", "0"]
        alarm = work / "alarmpara.dat"
        base = json.loads(subprocess.check_output(cli))
        flagged = [*cli, "--same-build-native-alarm", str(alarm)]
        info = json.loads(subprocess.check_output(flagged))
        ranges = info.pop("observedAlarmRanges")
        if info != base or sum(len(bytes.fromhex(f["hex"])) for f in ranges) != 35:
            raise ValueError("conditional observations affected the semantic field report")
        dest = work / "comparison.bin"
        status = json.loads(subprocess.check_output([*flagged, "--comparison-bin", str(dest)]))
        base_dest = work / "baseline.bin"
        subprocess.check_output([*cli, "--comparison-bin", str(base_dest)])
        before = dest.read_bytes()
        oracle = archive.read("Usbload.bin")
        if (status["observedAlarmBytes"] != 35 or len(before) != 1704 or
                any(before[f["offset"]:f["offset"]+len(bytes.fromhex(f["hex"]))] != bytes.fromhex(f["hex"]) for f in ranges)):
            raise ValueError("public CLI did not publish the requested comparison")
        if not all(oracle[f["offset"]:f["offset"] + len(bytes.fromhex(f["hex"]))] == bytes.fromhex(f["hex"]) for f in ranges):
            raise ValueError("CLI ranges differ from archive")
        if subprocess.run([*flagged, "--comparison-bin", str(dest)], capture_output=True).returncode == 0 or dest.read_bytes() != before:
            raise ValueError("existing comparison file changed")
        for size in (47839, 47841):
            alarm.write_bytes(bytes(size))
            if subprocess.run([*flagged, "--comparison-bin", str(work / "bad.bin")], capture_output=True).returncode == 0 or (work / "bad.bin").exists():
                raise ValueError("invalid companion was published")
        # Expand the independent page list to its supported maximum. All occupied page-size words must take precedence.
        metadata = (work / "load.bin").read_bytes()
        source = Stream(metadata)
        source.string()
        source.integer()
        source.string()
        count_at = source.pos
        count = source.integer()
        page_table = len(metadata) - 4 - (4 + 16 * count)
        count_bytes = struct.pack(">i", 200)
        large = (metadata[:count_at] + count_bytes + metadata[count_at+4:page_table] + count_bytes +
                 struct.pack(">dd", 320, 240) * 200 + metadata[-4:])
        fields = same_build_alarm_ranges(large, bytes(47840))
        if sum(map(len, fields.values())) != 13 or any(offset >= 904 for offset in fields):
            raise ValueError("observed bytes would overwrite an active ordinary page size")
        lengths = {n: (work/n).stat().st_size for n in LENGTH_FILES if (work/n).exists()}
        explicit = render_descriptor(large, lengths, 0, native_alarm=bytes(47840))
        if explicit[904:] != struct.pack("<I", 320*240*2) * 200:
            raise ValueError("an occupied page-size slot changed")


def verify(revision, extra):
    """Deduplicate archives by Git identity and group repeats using actual archived input SM bytes."""
    tree = subprocess.check_output(["git", "ls-tree", "-rz", revision], cwd=ROOT)
    dirs = defaultdict(dict)
    for row in filter(None, tree.split(b"\0")):
        header, path = row.split(b"\t", 1)
        folder, _, name = path.decode().rpartition("/")
        dirs[folder][name.lower()] = header.split()[2].decode()
    reader = subprocess.Popen(["git", "cat-file", "--batch"], cwd=ROOT, stdin=subprocess.PIPE, stdout=subprocess.PIPE)

    def read(oid):
        """Read one exact Git object from the preselected revision."""
        reader.stdin.write(oid.encode() + b"\n")
        reader.stdin.flush()
        n = int(reader.stdout.readline().split()[2])
        data = reader.stdout.read(n)
        if len(data) != n or reader.stdout.read(1) != b"\n":
            raise ValueError("incomplete Git object " + oid)
        return data

    rows, src_hashes, repeat_groups = {}, {}, defaultdict(list)
    try:
        for folder, files in dirs.items():
            oid = files.get("load.kns")
            if not oid:
                continue
            source = files.get("input-native.sm")
            if source and source not in src_hashes:
                src_hashes[source] = digest(read(source))
            if oid not in rows:
                rows[oid] = compare_member(read(oid), src_hashes.get(source))
                rows[oid]["reference"] = folder
            if source and folder.startswith("docs/") and {"usbload.bin", "load.bin"} <= files.keys():
                repeat_groups[src_hashes[source]].append(rows[oid])
        all_cases = list(rows.values())
        verify_cli(read(next(iter(rows))))
    finally:
        reader.stdin.close()
        reader.wait()
    historical_cases = len(all_cases)
    for path in extra:
        case = compare_member(path.read_bytes(), None)
        if any(r["archive"] == case["archive"] for r in all_cases):
            raise ValueError("external reference duplicates a case")
        case["reference"] = path.name + " (" + case["model"] + ")"
        all_cases.append(case)
    # Source grouping is fixed by a byte-identity hash independently of either compared output.
    # These retrospective source folds report coverage, not a prospective blind validation.
    split = Counter("source-fold-0" if int(r["source"][:2], 16) % 2 == 0 else "source-fold-1"
                    for r in all_cases[:historical_cases])
    pairs = Counter()
    example = None
    for source, group in repeat_groups.items():
        if len(group) < 2:
            continue
        pairs["sourceGroups"] += 1
        for a, b in combinations(group, 2):
            pairs["pairs"] += 1
            if a["alarm"] == b["alarm"]:
                pairs["sameAlarm"] += 1
            if a["selectedHex"] == b["selectedHex"]:
                pairs["sameSelectedBytes"] += 1
            else:
                pairs["differentSelectedBytes"] += 1
                if example is None:
                    example = [source, a["archive"], b["archive"], a["selectedHex"], b["selectedHex"]]
    return dict(revision=revision, historicalArchives=historical_cases, additionalArchives=len(extra),
                sourceFolds=dict(split), repeatGroups=dict(pairs), differentRepeatExample=example,
                checkedCompanionBytesPerArchive=35, casesWithExactDescriptor=sum(r["remainingAfter"] == 0 for r in all_cases),
                previouslyDifferentSelectedBytes=sum(35-r["selectedPreviouslyExact"] for r in all_cases),
                selectedValuesAcrossCases=len(set(r["selectedHex"] for r in all_cases)), cases=all_cases)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--revision", default="e3b72657")
    parser.add_argument("--report", type=Path)
    parser.add_argument("extra", nargs="*", type=Path)
    args = parser.parse_args()
    report = verify(args.revision, args.extra)
    if args.report:
        args.report.write_text(json.dumps(report, ensure_ascii=False, indent=1) + "\n", encoding="utf8")
    print(json.dumps({k: v for k, v in report.items() if k not in ("cases", "differentRepeatExample")}))
