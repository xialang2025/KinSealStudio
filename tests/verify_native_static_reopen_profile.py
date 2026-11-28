"""Exercise the V122 remembered static-page profile against the two AMZ070 field captures."""
# 20261128-40 add
import argparse
from datetime import datetime
from itertools import product
import json
import os
import subprocess
import sys
import uuid
from pathlib import Path
import struct
from tempfile import TemporaryDirectory

from verify_amz_local_page_io import ROUTES, sha
from verify_native_drawing_drafts import ROOT


SAMPLES = ROOT / "tests/fixtures/native-source/curve-reopen-amz"
PAIRS = (
    ("native-saved.sm", "native-01-SysPara.dat",
     "676ec3a2fe967141fae229d0c1fadd059ca2d788d92302d332df89599ee6485f",
     "b79cbe3a223a7dfcf22418d9b5dbbabd5cd0b02165d3435be508790b1bc6de89"),
    ("AMZ-LIVE-02.sm", "native-02-SysPara.dat",
     "dd95f5a0a00724a2d53b43ae25b8d7b984251dc37da9893502ac0a0fbb4c95bf",
     "07522900427e998916383f4a249f62b940626d23860cbc9621326f1165ac0584"),
)
KEYS = ("config.200.7", "config.240.7", "config.head.12")


def private_process(arguments):
    """Launch the test driver on a separate desktop so Qt loader dialogs can never appear on the user's desktop."""
    import win32event
    import win32process
    import win32service

    name = "KinSealProfileReopen_" + uuid.uuid4().hex
    desktop = win32service.CreateDesktop(name, 0, 0x1ff, None)
    startup = win32process.STARTUPINFO()
    startup.lpDesktop = "WinSta0\\" + name
    args = [sys.executable, "-B", str(Path(__file__).resolve()), *arguments, "--private-child"]
    process, thread, _, _ = win32process.CreateProcess(
        sys.executable, subprocess.list2cmdline(args), None, None, False, subprocess.CREATE_NO_WINDOW,
        dict(os.environ, QT_QPA_PLATFORM="windows"), str(Path.cwd()), startup)
    try:
        win32event.WaitForSingleObject(process, win32event.INFINITE)
        return win32process.GetExitCodeProcess(process)
    finally:
        thread.Close()
        process.Close()
        desktop.Close()


def run(exe, work, option, *args, success=True):
    """Run file actions on the test driver desktop and check the process exit status."""
    env = dict(os.environ, TEMP=str(work), TMP=str(work), TMPDIR=str(work),
               QT_QPA_PLATFORM="windows" if os.name == "nt" else "offscreen")
    result = subprocess.run([str(exe), option, *map(str, args)], cwd=work, env=env, capture_output=True,
                            timeout=90, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
    if (result.returncode == 0) != success:
        raise ValueError(f"{option} {result.returncode} {result.stdout.decode('utf8', 'replace')[:400]}")
    return result.stdout


def layout(exe, work, path):
    """Resolve primitive spans against the exact SM being edited."""
    doc = json.loads(run(exe, work, "--inspect-native-source", path))
    if doc["sha256"] != sha(path.read_bytes()) or not doc["bodyValidated"]:
        raise ValueError("Mismatched or incomplete native source layout")
    return {field["path"]: field for field in doc["fields"]}


def replace_spans(raw, fields, changes):
    """Change primitive wire values without rewriting unaffected fields or adjusting object order."""
    for key in sorted(changes, key=lambda name: fields[name]["start"], reverse=True):
        span = fields[key]
        raw = raw[:span["start"]] + changes[key] + raw[span["end"]:]
    return raw


def byte_at(raw, span):
    """Return a captured primitive, including the null QString length when present."""
    return raw[span["start"]:span["end"]]


def check_sources(exe, work):
    """Compile the disk sources and retain actual whole-file differences, including volatile fields."""
    residuals = ROOT / "docs/文件规则/样例/SysPara同源残差与窗口语言交叉复验_20261128.json"
    known = set(map(int, json.loads(residuals.read_bytes())["repeated"]["nonClockOffsetsAndPairCounts"]))
    clock = set(range(324, 352))
    fixed = set(range(4998)) - known - clock
    refs = []
    results = []
    for source_name, system_name, source_sha, ref_sha in PAIRS:
        source = SAMPLES / source_name
        original = (SAMPLES / system_name).read_bytes()
        if (sha(source.read_bytes()), sha(original), len(original)) != (source_sha, ref_sha, 4998):
            raise ValueError("The source or original system capture changed: " + source_name)
        refs.append(original)
        ss, mm, hh, weekday, day, month, year = struct.unpack_from("<7I", original, 324)
        stamp = datetime(year, month, day, hh, mm, ss)
        if stamp.isoweekday() != weekday:
            raise ValueError("Invalid original calendar")
        for route in ROUTES:
            flags = ["--native-compile-time", stamp.isoformat()] if route.endswith("media-parts") else []
            dest = work / (source.stem + route)
            run(exe, work, route, source, dest, *flags)
            actual = (dest / "SysPara.dat").read_bytes()
            if len(actual) != len(original):
                raise ValueError("System output length differs")
            differences = {i for i, values in enumerate(zip(original, actual)) if values[0] != values[1]}
            fixed_diff = sorted(fixed & differences)
            # These eight bytes also differ on the older supported 01 page. Do not accept new sites.
            if not set(fixed_diff) <= {171, 207, 215, 219, 231, 383, 863, 903} or (flags and differences & clock):
                raise ValueError("Unexplained system bytes or failure to encode the native clock")
            results.append(dict(source=source_name, sourceSHA=source_sha, nativeSHA=ref_sha, route=route,
                                nativeClockRequested=bool(flags), nativeClock=stamp.isoformat(), generatedSHA=sha(actual),
                                wholeFileExact=actual == original, differentBytes=len(differences),
                                firstDifference=min(differences, default=None), stableDifferentOffsets=fixed_diff))
    delta = {i for i, (x, y) in enumerate(zip(*refs)) if x != y}
    if len(known) != 172 or len(fixed) != 4798 or delta & fixed:
        raise ValueError("The two native system segments diverge at a previously stable position")
    return dict(nativeStableBytes=len(fixed), nativeStableBytesExact=len(fixed), nativeWholeFileDifferences=len(delta),
                observedNativeClockChanges=sorted(delta & clock), observedNativeVariableChanges=len(delta & known),
                outputs=results)


def exercise_variant(exe, work, label, raw, expect_success):
    """Exercise both compilers and ensure no failed call can publish device members."""
    source = work / (label + ".sm")
    source.write_bytes(raw)
    output = []
    for route in ROUTES:
        dest = work / (label + route)
        result = json.loads(run(exe, work, route, source, dest, success=expect_success))
        if result.get("ok") is not expect_success or (not expect_success and dest.exists()):
            raise ValueError("Unexpected publication status for " + label)
        output.append(result.get("message"))
    return dict(label=label, sourceSHA=sha(raw), expectedSuccess=expect_success, messages=output)


def check_guards(exe, work):
    """Cross the three captured fields and vary independent address, visibility, touch and object inputs."""
    source = SAMPLES / "AMZ-LIVE-02.sm"
    before = SAMPLES / "native-saved.sm"
    fields, previous = layout(exe, work, source), layout(exe, work, before)
    raw, old = source.read_bytes(), before.read_bytes()
    rows = []
    for mask in product((False, True), repeat=3):
        changes = {key: byte_at(old, previous[key]) for key, use_old in zip(KEYS, mask) if use_old}
        changed = replace_spans(raw, fields, changes)
        label = "defaults-" + "".join("1" if value else "0" for value in mask)
        rows.append(exercise_variant(exe, work, label, changed, len(changes) in (0, 3)))
    for key in ("object[0].common.40.1", "object[0].common.visibility.0", "object[0].common.touch.0",
                "object[1].curve.visibility.0", "object[1].curve.touch.0", "object[3].table.columnAddress.1"):
        if byte_at(raw, fields[key]) != b"\x00":
            raise ValueError("Changed the wrong initial setting: " + key)
        rows.append(exercise_variant(exe, work, key, replace_spans(raw, fields, {key: b"\x01"}), False))
    wrong = "different-page".encode("utf-16be")
    changed = replace_spans(raw, fields, {"config.head.12": struct.pack(">I", len(wrong)) + wrong})
    rows.append(exercise_variant(exe, work, "unrelated-page-name", changed, False))
    # Graft one parseable original lamp into the mixed host. The host retains its native serial profile.
    lamp = ROOT / "resources/native-lamp-default.sm"
    donor = layout(exe, work, lamp)
    body = lamp.read_bytes()[donor["object[0].type"]["start"]:donor["object[0].z"]["end"]]
    changed = raw[:fields["object[1].type"]["start"]] + body + raw[fields["object[1].z"]["end"]:]
    path = work / "lamp-in-host.sm"
    path.write_bytes(changed)
    new_fields = layout(exe, work, path)
    if byte_at(changed, new_fields["object[1].type"]) != byte_at(lamp.read_bytes(), donor["object[0].type"]):
        raise ValueError("Unexpected object type after inserting original lamp")
    rows.append(exercise_variant(exe, work, "dynamic-lamp", changed, False))
    # Copy the original encoder's nonempty schedule table into the static host.
    # Its records remain parseable, and must not be admitted under the address-free static profile.
    schedule = ROOT / "docs/文件规则/样例/AMZ070W01R现场复验_20261125/08-schedule.ksp"
    schedule_sm = work / "macro-schedule-source.sm"
    run(exe, work, "--export-native-project", schedule, schedule_sm)
    sch = layout(exe, work, schedule_sm)
    source_data = schedule_sm.read_bytes()
    start, end = "system.174.count", "system.16c.count"
    chunk = source_data[sch[start]["start"]:sch[end]["start"]]
    if chunk == bytes(8):
        raise ValueError("The macro schedule test requires at least one record")
    changed = raw[:fields[start]["start"]] + chunk + raw[fields[end]["start"]:]
    path = work / "macro-host.sm"
    path.write_bytes(changed)
    layout(exe, work, path)
    rows.append(exercise_variant(exe, work, "scheduled-static-host", changed, False))
    return rows


def main():
    """Run the bounded system profile test without accessing hardware or the native editor."""
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("exe", type=Path)
    p.add_argument("--scratch", type=Path, required=True)
    p.add_argument("--receipt", type=Path)
    p.add_argument("--private-child", action="store_true", help=argparse.SUPPRESS)
    args = p.parse_args()
    if os.name == "nt" and not args.private_child:
        raise SystemExit(private_process(sys.argv[1:]))
    exe = args.exe.resolve()
    initial = sha(exe.read_bytes())
    with TemporaryDirectory(dir=args.scratch.resolve()) as tmp:
        work = Path(tmp)
        report = dict(exeSha256=initial, systems=check_sources(exe, work), variants=check_guards(exe, work))
    if initial != sha(exe.read_bytes()):
        raise ValueError("The compiler changed during this comparison")
    if args.receipt:
        args.receipt.write_text(json.dumps(report, ensure_ascii=False, indent=1) + "\n", encoding="utf8")
    print(f"Two original system segments and {len(report['variants'])} source variants on both compilers")


if __name__ == "__main__":
    main()
