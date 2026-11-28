"""Compare Lite's whole output files with independent original builds of the same input SM."""
# 20261128-27 add

import argparse
import collections
import concurrent.futures
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[1]
BASELINE = ROOT / "docs/文件规则/样例/V122显示控件第八次逐工程与逐文件复验_20261128.json"
MEMBERS = ("SheetPara.dat", "SysPara.dat", "AlarmPara.dat", "ControlPara.dat")
CATEGORIES = ("selectedExact", "alternateExact", "alternateDifferent", "alternateMemberMissing",
              "duplicateArchiveOnly", "noSecondArchive")


def sha(data):
    """Identify the exact bytes of an input or a complete output."""
    return hashlib.sha256(data).hexdigest()


def originals(revision):
    """Index directories by identical input SM and load the source and loose reference files."""
    tree = subprocess.check_output(["git", "ls-tree", "-rz", revision], cwd=ROOT)
    entries = {}
    for line in tree.split(b"\0"):
        if line:
            info, path = line.split(b"\t", 1)
            entries[path.decode()] = info.split()[2].decode()
    groups = collections.defaultdict(list)
    references = {}
    needed = set()
    for path, oid in entries.items():
        if path.endswith("/input-native.sm"):
            folder = path.rsplit("/", 1)[0]
            members = {name: entries[folder + "/" + name] for name in MEMBERS if folder + "/" + name in entries}
            if members:
                groups[oid].append(folder)
                references[folder] = (oid, members)
                needed.add(oid)
                needed.update(members.values())
    objects = {}
    reader = subprocess.Popen(["git", "cat-file", "--batch"], cwd=ROOT, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    try:
        for oid in sorted(needed):
            reader.stdin.write((oid + "\n").encode())
            reader.stdin.flush()
            header = reader.stdout.readline().split()
            if len(header) != 3 or header[:2] != [oid.encode(), b"blob"]:
                raise OSError("unexpected Git object " + oid)
            size = int(header[2])
            raw = reader.stdout.read(size)
            if len(raw) != size or reader.stdout.read(1) != b"\n":
                raise OSError("incomplete Git object " + oid)
            objects[oid] = raw
    finally:
        reader.stdin.close()
        reader.stdout.close()
        if reader.wait():
            raise OSError("Git failed to read the original files")
    return groups, references, objects


def differences(left, right):
    """Return the byte distance, including trailing bytes, and the first difference or None."""
    first = None
    count = abs(len(left) - len(right))
    for index, (a, b) in enumerate(zip(left, right)):
        if a != b:
            count += 1
            if first is None:
                first = index
    if first is None and count:
        first = min(len(left), len(right))
    return count, first


def compare(name, actual, preferred, independent, alternatives, references, objects):
    """Keep the selected comparison and independently search every available same-source output."""
    compared = [folder for folder in independent if name in references[folder][1]]
    distances = {}
    exact = []
    for folder in compared:
        original = objects[references[folder][1][name]]
        if original == actual:
            exact.append(folder)
            distances[folder] = 0
        else:
            distances[folder] = differences(original, actual)[0]
    distance, first = differences(preferred, actual)
    if distance == 0:
        category = "selectedExact"
    elif exact:
        category = "alternateExact"
    elif compared:
        category = "alternateDifferent"
    elif independent:
        category = "alternateMemberMissing"
    elif alternatives:
        category = "duplicateArchiveOnly"
    else:
        category = "noSecondArchive"
    return dict(category=category, selectedSha256=sha(preferred), actualSha256=sha(actual),
                selectedLength=len(preferred), actualLength=len(actual),
                selectedDifferentBytes=distance, selectedFirstDifference=first,
                alternateMembersCompared=len(compared), exactAlternateFolders=exact,
                nearestAlternateDifferentBytes=min(distances.values(), default=None))


def check(case, exe, workspace, catalog, only_sheet):
    """Compile one frozen source once and compare files from the compiler's normal output directory."""
    source, pinned = case
    groups, references, objects = catalog
    folder = pinned["reference"]
    oid, selected = references[folder]
    sm = objects[oid]
    if sha(sm) != source:
        raise ValueError("selected archive source mismatch in " + folder)
    alternatives = sorted(other for other in groups[oid] if other != folder)
    # Distinct bytes in a member present in both original outputs distinguish a recompile from a copy.
    independent = {}
    for other in alternatives:
        old = references[other][1]
        different = sorted(name for name in old.keys() & selected.keys() if old[name] != selected[name])
        if different:
            independent[other] = different
    names = [name for name in MEMBERS if name in pinned["differences"] and (not only_sheet or name == MEMBERS[0])]
    with tempfile.TemporaryDirectory(prefix="same-source-", dir=workspace) as tmp:
        directory = Path(tmp)
        path = directory / "input-native.sm"
        path.write_bytes(sm)
        output = directory / "out"
        env = dict(os.environ, TEMP=tmp, TMP=tmp, TMPDIR=tmp)
        env.setdefault("QT_QPA_PLATFORM", "offscreen")
        compilation = subprocess.run([str(exe), "--compile-project", str(path), str(output)], cwd=directory,
                                     env=env, capture_output=True, timeout=120,
                                     creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        if compilation.returncode:
            raise RuntimeError(folder + ": " + repr(compilation.stdout[-1000:]) + repr(compilation.stderr[-1000:]))
        members = {name: compare(name, (output / name).read_bytes(), objects[selected[name]], independent,
                                 alternatives, references, objects) for name in names}
    return dict(source=source, selectedFolder=folder, alternativeFolders=alternatives,
                independentOriginalMemberDifferences=independent, members=members)


def main():
    """Run the frozen selection in normal device-project mode without connecting to a device."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--second-exe", type=Path,
                        help="Repeat the same comparison on another build and require identical rows.")
    parser.add_argument("--receipt", type=Path)
    parser.add_argument("--only-sheet", action="store_true")
    parser.add_argument("--jobs", type=int, default=2)
    args = parser.parse_args()
    pinned = json.loads(BASELINE.read_bytes())
    previous = pinned["compiledOutputs"]["usb"]["bySource"]
    names = MEMBERS[:1] if args.only_sheet else MEMBERS
    cases = [(key, row) for key, row in previous.items() if row["compiled"] and set(names) & row["differences"].keys()]
    catalog = originals(pinned["revision"])
    exe = args.exe.resolve()
    exe_hash = sha(exe.read_bytes())
    workspace = ROOT / ".local"
    workspace.mkdir(exist_ok=True)
    with concurrent.futures.ThreadPoolExecutor(args.jobs) as workers:
        rows = list(workers.map(lambda case: check(case, exe, workspace, catalog, args.only_sheet), cases))
    counts = {name: dict.fromkeys(CATEGORIES, 0) for name in names}
    for row in rows:
        for name, member in row["members"].items():
            counts[name][member["category"]] += 1
    for name, counter in counts.items():
        if sum(counter.values()) != pinned["compiledOutputs"]["usb"]["byFile"]["differences"][name]:
            raise ValueError("missing pinned differences for " + name)
    if sha(exe.read_bytes()) != exe_hash:
        raise RuntimeError("the tested compiler changed while running")
    result = dict(revision=pinned["revision"], selectedBaseline=BASELINE.name, baselineExe=pinned["exeSha256"],
                  testedExeSha256=exe_hash, mode="--compile-project", originalFolders=len(catalog[1]),
                  originalSourceGroups=len(catalog[0]), pinnedSources=len(previous), rerunSources=len(rows),
                  counts=counts, rows=rows)
    if args.second_exe:
        second = args.second_exe.resolve()
        second_hash = sha(second.read_bytes())
        with concurrent.futures.ThreadPoolExecutor(args.jobs) as workers:
            repeated = list(workers.map(lambda case: check(case, second, workspace, catalog, args.only_sheet), cases))
        if repeated != rows or second_hash != sha(second.read_bytes()):
            raise ValueError("the second executable or its complete comparison rows differ")
        result["secondBuild"] = dict(testedExeSha256=second_hash, identicalRows=len(repeated))
    if args.receipt:
        args.receipt.write_text(json.dumps(result, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
    print(json.dumps({k: v for k, v in result.items() if k != "rows"}, indent=1))


if __name__ == "__main__":
    main()
