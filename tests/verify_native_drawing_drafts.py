"""Verify editable incomplete native drawings against the actual V5.1 compiled file corpus."""
# 20261128-4 add
import argparse
import collections
from contextlib import ExitStack
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
import uuid
import zlib


ROOT = Path(__file__).resolve().parents[1]
REFERENCES = ROOT / "docs/文件规则/样例/未完成图元原厂空帧对照_20261128.json"
REVISION = "e3b72657"


def git_blob(folder, name):
    """Obtain the unmodified bytes from the archived vendor run."""
    return subprocess.check_output(["git", "show", REVISION + ":" + folder + "/" + name], cwd=ROOT)


def run_on_private_desktop(command, directory, environment, timeout):  # 20261128-41 add
    """Capture both streams while keeping even Qt loader error dialogs on an unshown Windows desktop."""
    import msvcrt
    import win32con
    import win32event
    import win32process
    import win32service

    name = "KsDrawing_" + uuid.uuid4().hex
    startup = win32process.STARTUPINFO()
    startup.lpDesktop = "WinSta0\\" + name
    startup.dwFlags = win32con.STARTF_USESTDHANDLES
    flags = subprocess.CREATE_NO_WINDOW | win32con.CREATE_UNICODE_ENVIRONMENT
    # File-backed capture avoids pipe-buffer deadlocks for full JSON source inspections.
    with ExitStack() as resources:
        desktop = win32service.CreateDesktop(name, 0, 0x1ff, None)
        resources.callback(desktop.CloseDesktop)
        stdin = resources.enter_context(open(os.devnull, "rb"))
        stdout = resources.enter_context(tempfile.TemporaryFile(dir=directory))
        stderr = resources.enter_context(tempfile.TemporaryFile(dir=directory))
        handles = [msvcrt.get_osfhandle(stream.fileno()) for stream in (stdin, stdout, stderr)]
        startup.hStdInput, startup.hStdOutput, startup.hStdError = handles
        try:
            for handle in handles:
                os.set_handle_inheritable(handle, True)
            process, thread, _, _ = win32process.CreateProcess(
                command[0], subprocess.list2cmdline(command), None, None, True, flags,
                environment, str(directory), startup)
            resources.callback(process.Close)
            thread.Close()
        finally:
            for handle in handles:
                os.set_handle_inheritable(handle, False)
        try:
            waited = win32event.WaitForSingleObject(process, int(timeout * 1000))
        finally:
            if win32event.WaitForSingleObject(process, 0) != win32event.WAIT_OBJECT_0:
                win32process.TerminateProcess(process, 1)
                win32event.WaitForSingleObject(process, win32event.INFINITE)
        stdout.seek(0)
        stderr.seek(0)
        output, errors = stdout.read(), stderr.read()
        if waited == win32event.WAIT_TIMEOUT:
            raise subprocess.TimeoutExpired(command, timeout, output=output, stderr=errors)
        return subprocess.CompletedProcess(command, win32process.GetExitCodeProcess(process), output, errors)


def run(exe, directory, option, *args, success=True):
    """Run a public file action, respecting the selected Qt platform and isolating Windows UI."""
    child_env = dict(os.environ, TEMP=str(directory), TMP=str(directory))
    child_env.setdefault("QT_QPA_PLATFORM", "offscreen")  # 20261128-41 change: Respect a private desktop runner's chosen platform.
    if os.name == "nt" and child_env["QT_QPA_PLATFORM"] == "offscreen":
        child_env.setdefault("QT_QPA_FONTDIR", str(Path(os.environ["SystemRoot"]) / "Fonts"))
    command = [str(exe), option, *map(str, args)]
    if os.name == "nt":
        proc = run_on_private_desktop(command, directory, child_env, 90)
    else:
        proc = subprocess.run(command, cwd=directory, capture_output=True, timeout=90, env=child_env)
    if (proc.returncode == 0) != success:
        raise ValueError(f"{option} {proc.returncode} stdout={proc.stdout.decode('utf8', 'replace')[:400]} "
                         f"stderr={proc.stderr.decode('utf8', 'replace')[:400]}")
    return proc.stdout


def objects(project):
    """Return the canonical page object array for the saved project schema."""
    if "pages" in project:
        return project["pages"][0]["objects"]
    return project["objects"]


def verify_edits(exe, path, model):
    """Move and rename a draft, round-trip it, and check that invalid topology fails before publication."""
    source = json.loads(model.read_bytes())
    obj = next(o for o in objects(source) if o["graphic"].get("nativeDrawingDraft"))
    original_position = (obj["x"], obj["y"])
    obj["x"] += 7
    obj["y"] += 9
    obj["name"] = "Draft interop"
    edited = path / "changed.ksp"
    saved = path / "changed.sm"
    reread = path / "reread.ksp"
    edited.write_text(json.dumps(source, ensure_ascii=False), encoding="utf8")
    run(exe, path, "--export-native-project", edited, saved)
    run(exe, path, "--migrate-project", saved, reread)
    found = next(o for o in objects(json.loads(reread.read_bytes())) if o["name"] == "Draft interop")
    if (found["x"], found["y"]) != (original_position[0] + 7, original_position[1] + 9) or found["graphic"] != obj["graphic"]:
        raise ValueError("draft translation/name or graph properties were lost")
    del obj["graphic"]["nativeDrawingDraft"]
    edited.write_text(json.dumps(source, ensure_ascii=False), encoding="utf8")
    output = path / "invalid-package"
    run(exe, path, "--compile-project", edited, output, success=False)
    if output.exists():
        raise ValueError("a rejected topology published a package")


def examine(exe, path, source, media=False):
    """Compare the SM, editable KNR payload and all common original output files by their actual bytes."""
    folder = source["path"]
    data = git_blob(folder, "input-native.sm")
    digest = hashlib.sha256(data).hexdigest()
    if digest != source["sourceSHA256"]:
        raise ValueError("original source hash mismatch")
    sm = path / "input.sm"
    model = path / "saved.ksp"
    restored = path / "restored.sm"
    direct = path / "direct.sm"
    payload = path / "compiled"
    sm.write_bytes(data)
    run(exe, path, "--export-native-project", sm, direct)
    run(exe, path, "--migrate-project", sm, model)
    run(exe, path, "--export-native-project", model, restored)
    if restored.read_bytes() != data or direct.read_bytes() != data:
        raise ValueError("the exported SM differs from the original")
    verify_edits(exe, path, model)
    names = subprocess.check_output(["git", "ls-tree", "-z", "--name-only", REVISION + ":" + folder], cwd=ROOT)
    files = [s for s in names.decode("utf8").split("\0") if Path(s).suffix.lower() in (".bin", ".kns", ".dat", ".knr")]
    if media:
        switches = () if "Modbus_RTU_com1.bin" in files else ("--without-com1-driver",)
        run(exe, path, "--compile-native-media-parts", sm, payload, *switches)
    else:
        run(exe, path, "--compile-project", sm, payload)
    archive = (payload / "upLoad.knr").read_bytes()
    recovered = zlib.decompress(archive[4:])
    expected_archive_source = data
    if media:
        spans = {part["path"]: part for part in json.loads(run(exe, path, "--inspect-native-source", sm))["fields"]}
        current = spans["config.head.12"]
        page = spans["page.head.18"]
        if data[current["start"]:current["end"]] == bytes.fromhex("ffffffff"):
            expected_archive_source = data[:current["start"]] + data[page["start"]:page["end"]] + data[current["end"]:]
    if len(recovered) != int.from_bytes(archive[:4], "big") or recovered != expected_archive_source:
        raise ValueError("the KNR archive differs from its expected source")
    same = []
    differences = {}
    absent = []
    sheet_offsets = []
    for name in files:
        item = payload / name
        if not item.exists():
            absent.append(name)
            continue
        expected, actual = git_blob(folder, name), item.read_bytes()
        if expected == actual:
            same.append(name)
        else:
            if name == "SheetPara.dat":
                sheet_offsets = [i for i, (a, b) in enumerate(zip(expected, actual)) if a != b]
                sheet_offsets.extend(range(min(len(expected), len(actual)), max(len(expected), len(actual))))
            first = next((i for i, (a, b) in enumerate(zip(expected, actual)) if a != b), min(len(expected), len(actual)))
            differences[name] = first
    return dict(sourceSHA256=digest, reference=folder, smDirectAndViaKspExact=True, editedRoundTrip=True,
                invalidUnmarkedTopologyRefused=True, knrSourceExact=recovered == data, knrExpectedSourceExact=True, identical=same, differences=differences,
                absent=absent, sheetParameterDifferenceOffsets=sheet_offsets,
                extra=sorted(p.name for p in payload.iterdir() if p.suffix.lower() in (".bin", ".kns", ".dat", ".knr") and p.name not in files))


def verify_controlled_cases(exe, scratch, refs):
    """Recheck the exact edited wire fields, whole compressed frame and fresh writer on four witnesses."""
    evidence = json.loads((REFERENCES.parent / "未完成几何图元定点编辑对照_20261128.json").read_bytes())
    by_source = {r["source"]: r for r in evidence["tests"]}
    new_records = {(r["type"], r["points"]): r["source"] for r in evidence["freshSources"]}
    done = set()
    for entry in refs:
        source = entry["sourceSHA256"]
        witness = by_source.get(source)
        info = entry["objects"]
        wire_type = info["object[0].type"]
        kind = {0x10026: "polygon", 0x10027: "polyline", 0x10028: "freeline"}[wire_type]
        count = info.get("object[0].box.points.count", info.get("object[0].curve.elements.count"))
        if not witness and (kind, count) in done:
            continue
        with tempfile.TemporaryDirectory(dir=scratch) as space:
            path = Path(space).resolve()
            sm, model = path / "input.sm", path / "base.ksp"
            sm.write_bytes(git_blob(entry["path"], "input-native.sm"))
            run(exe, path, "--migrate-project", sm, model)
            project = json.loads(model.read_bytes())
            obj = next(o for o in objects(project) if o["graphic"].get("nativeDrawingDraft"))
            kind = obj["type"], len(obj["graphic"]["points"])
            if witness:
                verify_edits(exe, path, model)
                edited = path / "changed.sm"
                if hashlib.sha256(edited.read_bytes()).hexdigest() != witness["translatedSource"]:
                    raise ValueError("edited SM differs from the field-bounded reference")
                output = path / "edited-package"
                run(exe, path, "--compile-project", edited, output)
                if hashlib.sha256((output / "SheetPixel.dat").read_bytes()).hexdigest() != witness["compiledSheetSHA256"]:
                    raise ValueError("translated draft pixels differ from the reference")
            if kind not in done and kind in new_records:
                del project["nativeSourceSnapshot"]
                project.pop("nativeSourceWarnings", None)
                obj["name"] = "Independent draft"
                obj["x"] += 5
                obj["y"] += 3
                model.write_text(json.dumps(project, ensure_ascii=False), encoding="utf8")
                fresh = path / "fresh.sm"
                run(exe, path, "--export-native-project", model, fresh)
                if hashlib.sha256(fresh.read_bytes()).hexdigest() != new_records[kind]:
                    raise ValueError("standalone draft record differs from the reference")
                reread = path / "fresh.ksp"
                run(exe, path, "--migrate-project", fresh, reread)
                again = objects(json.loads(reread.read_bytes()))[0]
                if any(again[k] != obj[k] for k in ("x", "y", "name", "type", "graphic")):
                    raise ValueError("the new standalone record lost an editable property")
                run(exe, path, "--compile-project", fresh, path / "fresh-package")
                done.add(kind)
    if done != set(new_records):
        raise ValueError("a standalone native drawing family was not tested")


def main():
    """Run the corpus one temporary project at a time and retain only the requested receipt."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    parser.add_argument("--scratch", required=True, type=Path)
    parser.add_argument("--receipt", required=True, type=Path)
    parser.add_argument("--controls", action="store_true", help="Additionally verify the saved edit and clean-writer references")
    parser.add_argument("--media", action="store_true", help="Exercise the external media-part compilation path")
    parser.add_argument("--require-all", action="store_true", help="Additionally require an exact, complete native file set")
    args = parser.parse_args()
    refs = json.loads(REFERENCES.read_bytes())
    entries = refs["lineCases"] + refs["polygonCases"]
    rows = []
    for item in entries:
        with tempfile.TemporaryDirectory(dir=args.scratch) as space:
            try:
                row = examine(args.exe.resolve(), Path(space).resolve(), item, args.media)
            except (ValueError, OSError, subprocess.SubprocessError) as error:
                row = dict(reference=item["path"], sourceSHA256=item["sourceSHA256"], error=str(error))
            rows.append(row)
    if args.controls:
        verify_controlled_cases(args.exe.resolve(), args.scratch, entries)
    total = collections.Counter(name for row in rows for name in row.get("identical", []))
    details = dict(revision=REVISION, executableSHA256=hashlib.sha256(args.exe.read_bytes()).hexdigest(),
                   samples=len(rows), outputMode="nativeMediaParts" if args.media else "deviceProject", exactFileCounts=dict(total), rows=rows)
    args.receipt.write_text(json.dumps(details, ensure_ascii=False, indent=2) + "\n", encoding="utf8")
    print(json.dumps(dict(total), ensure_ascii=False))
    for row in rows:
        if "error" in row or not {"SheetPixel.dat", "PicLenPara.dat", "AddrPara.dat", "Collect.dat", "Curve.dat"}.issubset(row["identical"]):
            return 1
        if row["sheetParameterDifferenceOffsets"] not in ([], [59]) or "SheetPara.dat" in row["absent"]:
            return 1
        if args.media and "upLoad.knr" not in row["identical"]:
            return 1
        if args.require_all and (row["differences"] or row["absent"] or row["extra"]):
            return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
