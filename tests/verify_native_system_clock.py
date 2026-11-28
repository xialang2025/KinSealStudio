"""Check the V122 system clock against original member times and independent USB metadata."""
# 20261128-10 add
import argparse
from collections import Counter, defaultdict
from datetime import datetime, timedelta
import gzip
import hashlib
import io
import itertools
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import zipfile
import zlib

from verify_native_drawing_drafts import git_blob, REVISION, ROOT, run

sys.path.insert(0, str(ROOT / "tools"))
from verify_native_usb_metadata import archives


def clock_bytes(t):
    """Pack civil clock fields in vendor order with Monday as day one."""
    return struct.pack("<7I", t.second, t.minute, t.hour, t.isoweekday(), t.day, t.month, t.year)


def language_fields(sm):
    """Read the current language and count from the V122 Qt4 source identity prefix."""
    pos = 0
    for kind in "qiqaqqiqqqqqqqqqii":
        if kind == "q":
            width, = struct.unpack_from(">I", sm, pos)
            pos += 4 + (0 if width == 0xffffffff else width)
        else:
            pos += 4 if kind == "i" else 1
    count, current = struct.unpack_from(">HH", sm, pos)
    if not 0 <= current < count:
        raise ValueError("invalid source language registry")
    return current, count


def original_clocks(raw):
    """Compare a bounded DOS-time candidate set and a separately timed USB action with the system segment."""
    with zipfile.ZipFile(io.BytesIO(raw)) as archive:
        members = {entry.filename.lower(): entry for entry in archive.infolist()}
        system = archive.read(members["syspara.dat"])
        usb = archive.read(members["usbload.bin"])
        packed = archive.read(members["upload.knr"])
        sys_dos = datetime(*members["syspara.dat"].date_time)
        usb_dos = datetime(*members["usbload.bin"].date_time)
    if len(system) != 4998 or len(usb) != 1704:
        raise ValueError("unexpected system or USB segment length")
    source = zlib.decompress(packed[4:])
    if len(source) != int.from_bytes(packed[:4], "big"):
        raise ValueError("invalid source snapshot")
    # A write can straddle the next DOS tick. Preserve every candidate; never infer an exact input from the target.
    candidates = [sys_dos + timedelta(seconds=i) for i in (-1, 0, 1)]
    matches = [t for t in candidates if clock_bytes(t) == system[324:352]]
    usb_times = []
    for delta in (-1, 0, 1):
        t = usb_dos + timedelta(seconds=delta)
        seconds = t.hour * 3600 + t.minute * 60 + t.second
        rand = ((214013 * seconds + 2531011) >> 16) & 32767
        if struct.pack("<I", rand) == usb[4:8]:
            usb_times.append(t)
    if len(matches) != 1 or len(usb_times) != 1:
        raise ValueError("expected exactly one matching candidate per native clock")
    clock = matches[0]
    language = language_fields(source)
    sys_language = struct.unpack_from("<HH", system, 2172)
    return dict(systemSha256=hashlib.sha256(system).hexdigest(), dosTime=sys_dos.isoformat(),
                matchingSystemTime=clock.isoformat(), systemMinusDos=int((clock - sys_dos).total_seconds()),
                usbCallMinusSystem=int((usb_times[0] - clock).total_seconds()),
                systemLanguage=list(sys_language), snapshotLanguage=list(language))


def language_witnesses():
    """Check native UI state independently of saved KNR and compiled system words for three exceptional runs."""
    rows = []
    for name in ("base-only", "opacity128-top", "transparent-top"):
        parent = "docs/stage3-evidence/native"
        folder = parent + "/compiled/static-odd2-" + name
        log_name = "native-probe-pixel-" + name + "-r1.json.gz"
        log = json.loads(gzip.decompress(git_blob(parent, log_name)))
        sm = git_blob(folder, "input-native.sm")
        system = git_blob(folder, "SysPara.dat")
        selected = {int(widget["qt"]["currentIndex"]) for shot in log["snapshots"]
                    for window in shot["windows"] for widget in window["widgets"]
                    if widget.get("qt", {}).get("kind") == "QComboBox"
                    and widget["qt"].get("items") == ["中文", "English"]}
        saved = zlib.decompress(git_blob(folder, "upLoad.knr")[4:])
        result = struct.unpack_from("<HH", system, 2172)
        builds = [b for b in log["nativeCompilations"] if b["sourceSha256"] == hashlib.sha256(sm).hexdigest()
                  and any(f["path"] == "SysPara.dat" and f["sha256"] == hashlib.sha256(system).hexdigest()
                          for f in b["files"])]
        if (not builds or selected != {0} or language_fields(sm) != (1, 2)
                or language_fields(saved) != (0, 2) or result != (0, 2)):
            raise ValueError("native input, UI, archive or output language evidence disagrees: " + name)
        rows.append(dict(reference=folder, sourceSHA256=hashlib.sha256(sm).hexdigest(),
                         log=parent + "/" + log_name, sourceIndex=1, observedUiIndex=0, knrIndex=0, systemIndex=0))
    return rows


def same_source():
    """Count pairs of original runs with the same source and compare the rest of SysPara without the clock."""
    tree = subprocess.check_output(["git", "ls-tree", "-rz", REVISION], cwd=ROOT)
    files = {}
    groups = defaultdict(list)
    duplicates = Counter()
    trailing_language_word = Counter()
    source_matches = 0
    for entry in tree.split(b"\0"):
        if entry:
            ident, filename = entry.split(b"\t", 1)
            files[filename.decode("utf8")] = ident.split()[2].decode()
    # Batch all file reads. A failed/truncated Git read must not become an equal empty comparison.
    names = sorted(p for p in files if p.endswith("/input-native.sm")
                   and p.removesuffix("input-native.sm") + "SysPara.dat" in files)
    oids = list(dict.fromkeys([files[p] for p in names] +
                              [files[p.removesuffix("input-native.sm") + "SysPara.dat"] for p in names]))
    payload = subprocess.check_output(["git", "cat-file", "--batch"], cwd=ROOT,
                                      input=("\n".join(oids) + "\n").encode())
    reader = io.BytesIO(payload)
    blobs = {}
    for oid in oids:
        actual, kind, size = reader.readline().split()
        data = reader.read(int(size))
        if actual.decode() != oid or kind != b"blob" or len(data) != int(size) or reader.read(1) != b"\n":
            raise ValueError("invalid Git object while comparing independent runs")
        blobs[oid] = data
    for name in names:
        sysdata = blobs[files[name.removesuffix("input-native.sm") + "SysPara.dat"]]
        if len(sysdata) != 4998:
            raise ValueError("unexpected original system width")
        source_matches += language_fields(blobs[files[name]]) == struct.unpack_from("<HH", sysdata, 2172)
        trailing_language_word[struct.unpack_from("<H", sysdata, 2176)[0]] += 1
        groups[files[name]].append(sysdata)
        for left, right in ((352, 832), (392, 792), (432, 752)):
            if sysdata[left:left+40] == sysdata[right:right+40]:
                duplicates[f"{left}/{right}"] += 1
    counted = Counter()
    for outputs in groups.values():
        for a, b in itertools.combinations(outputs, 2):
            counted["pairs"] += 1
            counted["differentOutside28ClockBytes"] += a[:324] + a[352:] != b[:324] + b[352:]
    return dict(runs=len(names), distinctSources=len(groups), sourceLanguageMatches=source_matches,
                repeatedSourceGroups=sum(len(x) > 1 for x in groups.values()),
                sameSourceCounts=dict(counted), equal40ByteBlocks=dict(duplicates),
                sysWord2176Counts=dict(trailing_language_word))


def compiler_clock(exe, scratch):
    """Verify the optional public CLI setting, all 28 bytes, and failure without a partial package."""
    source_folder = "docs/stage2-evidence/lamp-regression-restore/compiled/lamp-default-r1"
    with tempfile.TemporaryDirectory(dir=scratch) as tmp:
        work = Path(tmp).resolve()
        sm = work / "input.sm"
        sm.write_bytes(git_blob(source_folder, "input-native.sm"))
        base = work / "default"
        run(exe, work, "--compile-native-media-parts", sm, base)
        default = (base / "SysPara.dat").read_bytes()
        for i, spelling in enumerate(("2000-02-29T23:59:59", "2027-01-03T00:00:00")):
            out = work / str(i)
            run(exe, work, "--compile-native-media-parts", sm, out, "--native-compile-time", spelling)
            expected = default[:324] + clock_bytes(datetime.fromisoformat(spelling)) + default[352:]
            if (out / "SysPara.dat").read_bytes() != expected:
                raise ValueError("invalid Gregorian clock, weekday, or changes outside the clock words")
        invalid = ("2025-02-29T12:00:00", "2026-11-28T24:00:00", "2026-11-28T10:00:00Z", "2026-1-2T3:04:05")
        for i, spelling in enumerate(invalid):
            out = work / ("invalid-" + str(i))
            run(exe, work, "--compile-native-media-parts", sm, out, "--native-compile-time", spelling, success=False)
            if out.exists():
                raise ValueError("invalid native time published output")
    return dict(exeSha256=hashlib.sha256(exe.read_bytes()).hexdigest(), validCivilTimes=2,
                invalidCivilTimesRejected=len(invalid))


def main():
    """Report all clock comparisons; a close ZIP time is not an exact historical time input."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path)
    parser.add_argument("--scratch", default=ROOT / "build", type=Path)
    parser.add_argument("--receipt", type=Path)
    parser.add_argument("extra_archives", type=Path, nargs="*")
    args = parser.parse_args()
    rows = {}
    for label, raw in archives(REVISION, args.extra_archives):
        key = hashlib.sha256(raw).hexdigest()
        if key not in rows:
            rows[key] = dict(reference=label, **original_clocks(raw))
    report = dict(sourceRevision=REVISION, archives=len(rows),
                  systemClockMinusZipSeconds=dict(Counter(r["systemMinusDos"] for r in rows.values())),
                  usbClockMinusSystemSeconds=dict(Counter(r["usbCallMinusSystem"] for r in rows.values())),
                  languageMatchesKnr=sum(r["snapshotLanguage"] == r["systemLanguage"] for r in rows.values()),
                  sourcePaired=same_source(), observedEditorOverrides=language_witnesses(), byArchive=rows)
    if args.exe:
        report["production"] = compiler_clock(args.exe.resolve(), args.scratch.resolve())
    if args.receipt:
        args.receipt.write_text(json.dumps(report, ensure_ascii=False, indent=1) + "\n", encoding="utf8")
    print(json.dumps({k: v for k, v in report.items() if k not in ("byArchive", "observedEditorOverrides")}))


if __name__ == "__main__":
    main()
