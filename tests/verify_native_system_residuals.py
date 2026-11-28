"""Predict the V122 compile language and audit the remaining system differences across original runs."""
# 20261128-10 add
import argparse
from collections import Counter, defaultdict
from functools import lru_cache
import gzip
import hashlib
import io
import itertools
import json
from pathlib import Path
import struct
import subprocess
import zipfile

from verify_native_system_clock import REVISION, ROOT, language_fields


REGIONS = ((0, 24), (24, 100), (100, 324), (324, 352), (352, 752), (752, 1152),
           (1152, 1308), (1308, 1320), (1320, 2092), (2092, 2132), (2132, 2152),
           (2152, 2172), (2172, 2178), (2178, 4738), (4738, 4868), (4868, 4998))
# Fixed candidate offsets from the original source-paired discovery set. Test unseen archives as well.
ALARM_CANDIDATES = {191: 46743, 195: 46743, 370: 46154, 371: 46155,
                    399: 46155, 799: 46155, 850: 46154, 851: 46155}


def digest(raw):
    """Identify a source or compiled file independently of its name."""
    return hashlib.sha256(raw).hexdigest()


def logged_language(name, raw):
    """Read a visible, unambiguous language selection from the original main window and its build labels."""
    if name.endswith(".gz"):
        raw = gzip.decompress(raw)
    if b'"nativeCompilations"' not in raw or b'"currentIndex"' not in raw:
        return []
    log = json.loads(raw)
    readings = set()
    for snapshot in log.get("snapshots", []):
        for window in snapshot.get("windows", []):
            if "KinSealStudio" not in (window.get("title") or ""):
                continue
            for widget in window.get("widgets", []):
                qt = widget.get("qt") or {}
                items = qt.get("items") or []
                if (qt.get("kind") == "QComboBox" and qt.get("visible") and items
                        and items[0] == "中文" and (len(items) == 1 or items[1] == "English")):
                    readings.add((int(qt["currentIndex"]), len(items)))
    if len(readings) != 1:
        return []
    state = readings.pop()
    return [(build["sourceSha256"], build["label"], state, item["sha256"], name)
            for build in log.get("nativeCompilations", []) for item in build.get("files", [])
            if item.get("path") == "SysPara.dat"]


def predict_languages(sources, files, read):
    """Select UI or saved inputs before comparing with the four compiled bytes."""
    logs = defaultdict(list)
    examined = set()
    for name, oid in files.items():
        if (not name.startswith("docs/stage") or "/compiled/" in name or
                ("probe" not in name and "capture" not in name) or
                not name.endswith((".json.gz", ".json")) or oid in examined):
            continue
        examined.add(oid)
        for source, label, state, expected_digest, path in logged_language(name, read(oid)):
            logs[(source, label)].append((state, expected_digest, path))
    rows = []
    counts = Counter()
    for folder, source, system in sources:
        saved = language_fields(source)
        witnesses = logs[(digest(source), folder.rsplit("/", 1)[-1])]
        choices = {w[0] for w in witnesses}
        if len(choices) > 1:
            raise ValueError("conflicting original UI snapshots for " + folder)
        selected = next(iter(choices)) if choices else saved
        encoded = struct.pack("<HH", *selected)
        # A logged output digest validates the run association; it never chooses an index or method.
        if witnesses and not any(w[1] == digest(system) for w in witnesses):
            raise ValueError("compiled output identity does not match the bound native log: " + folder)
        if system[2172:2176] != encoded:
            raise ValueError("independent prediction disagrees with the original: " + folder)
        counts["fromUi" if witnesses else "fromInputSm"] += 1
        if witnesses:
            counts["uiOverrides"] += selected != saved
            rows.append(dict(folder=folder, source=digest(source), visible=list(selected), saved=list(saved),
                             originalSystem=digest(system), logs=sorted({w[2] for w in witnesses})))
    return dict(examinedProbeLogs=len(examined), matched=len(sources), **counts, loggedBuilds=rows)


def distributions(sources):
    """Count per-offset variation and min/max Hamming distance for every same-source pair."""
    groups = defaultdict(list)
    for _, source, system in sources:
        if len(system) != 4998:
            raise ValueError("unexpected original system segment size")
        groups[digest(source)].append(system)
    freq = Counter()
    records = []
    for same in groups.values():
        for left, right in itertools.combinations(same, 2):
            delta = [i for i, (a, b) in enumerate(zip(left, right)) if a != b]
            freq.update(delta)
            records.append([sum(a <= i < b for i in delta) for a, b in REGIONS])
    blocks = []
    for index, (start, stop) in enumerate(REGIONS):
        counts = [r[index] for r in records]
        blocks.append(dict(start=start, stopExclusive=stop, positions=sum(freq[p] > 0 for p in range(start, stop)),
                           pairsWithDifferences=sum(v > 0 for v in counts), minimum=min(counts), maximum=max(counts)))
    clock = REGIONS.index((324, 352))
    residuals = [sum(row) - row[clock] for row in records]
    return dict(pairs=len(records), repeatedSources=sum(len(g) > 1 for g in groups.values()), blocks=blocks,
                nonClockDistances=dict(sorted(Counter(residuals).items())),
                nonClockOffsetsAndPairCounts={k: v for k, v in sorted(freq.items()) if not 324 <= k < 352})


def cross_archives(archives, paired):
    """Test preselected cross-file candidates against every archive and count genuinely new system hashes."""
    known = {digest(raw) for _, _, raw in paired}
    seen = set()
    extra = set()
    totals = Counter()
    new_hits = Counter()
    failures = []
    for label, raw in archives:
        archive_hash = digest(raw)
        if archive_hash in seen:
            continue
        seen.add(archive_hash)
        with zipfile.ZipFile(io.BytesIO(raw)) as archive:
            names = {n.lower(): n for n in archive.namelist()}
            system = archive.read(names["syspara.dat"])
            alarm = archive.read(names["alarmpara.dat"])
        if len(system) != 4998 or len(alarm) != 47840:
            raise ValueError("unexpected original segment sizes at " + label)
        system_hash = digest(system)
        unseen = system_hash not in known and system_hash not in extra
        if unseen:
            extra.add(system_hash)
        missed = []
        for position, witness in ALARM_CANDIDATES.items():
            if system[position] == alarm[witness]:
                totals[position] += 1
                new_hits[position] += unseen
            else:
                missed.append(position)
        if missed:
            failures.append(dict(reference=label, archive=archive_hash, offsets=missed))
    return dict(archives=len(seen), previouslyUnseenSystemHashes=len(extra), alarmWitnessOffsets=ALARM_CANDIDATES,
                matches=dict(totals), unseenMatches=dict(new_hits), failures=failures)


def search_candidates(paired, files, read, offsets):
    """Find fixed-offset correlations using disjoint source projects for discovery and validation."""
    results = {}
    for name in ("AlarmPara.dat", "Usbload.bin", "SheetPara.dat", "load.bin"):
        rows = [(files[f + "/input-native.sm"], sysdata, read(files[f + "/" + name]))
                for f, _, sysdata in paired if f + "/" + name in files]
        discovery_oids = set(sorted({oid for oid, _, _ in rows})[::2])
        discovery = [(sysdata, part) for oid, sysdata, part in rows if oid in discovery_oids]
        validation = [(sysdata, part) for oid, sysdata, part in rows if oid not in discovery_oids]
        # Match complete columns across original runs. Constant bytes are not evidence of a shared field.
        columns = defaultdict(list)
        for index in range(min(len(part) for _, _, part in rows)):
            column = bytes(part[index] for _, part in discovery)
            if len(set(column)) >= 4:
                columns[column].append(index)
        candidates = {}
        for offset in sorted(offsets):
            original = bytes(sysdata[offset] for sysdata, _ in discovery)
            hits = [i for i in columns.get(original, ()) if all(s[offset] == part[i] for s, part in validation)]
            if hits:
                candidates[offset] = hits
        if name == "AlarmPara.dat":
            for offset, witness in ALARM_CANDIDATES.items():
                if witness not in candidates.get(offset, ()):
                    raise ValueError("archived alarm candidate no longer passes the split-sample check")
        results[name] = dict(discoveryRuns=len(discovery), validationRuns=len(validation), candidates=candidates)
    return results


def audit(additional):
    """Read immutable historical blobs using a verified streaming Git reader."""
    listing = subprocess.check_output(["git", "ls-tree", "-rz", REVISION], cwd=ROOT)
    files = {}
    for entry in filter(None, listing.split(b"\0")):
        meta, name = entry.split(b"\t", 1)
        files[name.decode()] = meta.split()[2]
    pipe = subprocess.Popen(["git", "cat-file", "--batch"], cwd=ROOT, stdin=subprocess.PIPE, stdout=subprocess.PIPE)

    def read(oid):
        """Verify the requested blob identity and length before returning original bytes."""
        pipe.stdin.write(oid + b"\n")
        pipe.stdin.flush()
        actual, kind, size = pipe.stdout.readline().split()
        raw = pipe.stdout.read(int(size))
        if actual != oid or kind != b"blob" or len(raw) != int(size) or pipe.stdout.read(1) != b"\n":
            raise ValueError("incomplete native evidence in Git")
        return raw

    try:
        saved = lru_cache(maxsize=512)(read)
        folders = sorted(name.rsplit("/", 1)[0] for name in files if name.endswith("/input-native.sm")
                         and name.rsplit("/", 1)[0] + "/SysPara.dat" in files)
        paired = [(d, saved(files[d + "/input-native.sm"]), read(files[d + "/SysPara.dat"])) for d in folders]
        archived = sorted({v for name, v in files.items() if name.lower().endswith("/load.kns")})
        packages = itertools.chain(((oid.decode(), read(oid)) for oid in archived),
                                   ((p.name, p.read_bytes()) for p in additional))
        repeated = distributions(paired)
        discovered = search_candidates(paired, files, read, repeated["nonClockOffsetsAndPairCounts"])
        return dict(sourceRevision=REVISION, pairedRuns=len(paired), language=predict_languages(paired, files, read),
                    repeated=repeated, splitSourceSearch=discovered, alarmCorrelation=cross_archives(packages, paired))
    finally:
        pipe.stdin.close()
        pipe.stdout.close()
        pipe.wait()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--receipt", type=Path)
    parser.add_argument("extra_archives", type=Path, nargs="*")
    options = parser.parse_args()
    report = audit(options.extra_archives)
    if options.receipt:
        options.receipt.write_text(json.dumps(report, indent=1, ensure_ascii=False) + "\n", encoding="utf8")
    print(json.dumps(dict(paired=report["pairedRuns"], languageMatched=report["language"]["matched"],
                          ui=report["language"].get("fromUi", 0), pairs=report["repeated"]["pairs"],
                          residualPositions=len(report["repeated"]["nonClockOffsetsAndPairCounts"]),
                          archives=report["alarmCorrelation"]["archives"])))
