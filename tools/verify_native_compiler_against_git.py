"""Recompile the frozen V5.1 inputs and compare the actual files with independent original Git blobs."""
# 20261128-2 add
import argparse
import gzip
import hashlib
import json
import ntpath
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import uuid

ROOT = Path(__file__).resolve().parents[1]
REFERENCE = ROOT / "docs/文件规则/样例/原厂编译字节复核_20261127.json"


def on_private_desktop(arguments):
    """Run the Qt Windows font backend on a separate desktop for reproducible native rendering."""
    import win32event
    import win32process
    import win32service

    name = "KsFormat_" + uuid.uuid4().hex
    desktop = win32service.CreateDesktop(name, 0, 0x1FF, None)
    startup = win32process.STARTUPINFO()
    startup.lpDesktop = "WinSta0\\" + name
    command = subprocess.list2cmdline([sys.executable, str(Path(__file__).resolve()), *arguments, "--child"])
    process, thread, _, _ = win32process.CreateProcess(
        sys.executable, command, None, None, False, subprocess.CREATE_NO_WINDOW,
        dict(os.environ, QT_QPA_PLATFORM="windows"), str(Path.cwd()), startup)
    try:
        win32event.WaitForSingleObject(process, win32event.INFINITE)
        return win32process.GetExitCodeProcess(process)
    finally:
        thread.Close()
        process.Close()
        del desktop


def original_work_directory(entry, read_blob):
    """Recover the original opened SM's parent from an independent native run log."""
    raw = read_blob(entry["log"])
    log = json.loads(gzip.decompress(raw) if raw.startswith(b"\x1f\x8b") else raw)
    if entry["key"] == ".workingCopyAfter.path":
        name = log["workingCopyAfter"]["path"]
    elif entry["key"] == ".snapshots[0].windows[0].title":
        title = log["snapshots"][0]["windows"][0]["title"]
        name = title.removeprefix("KinSealStudio--").rsplit("--", 1)[0]
    else:
        raise ValueError("unsupported original log field")
    if ntpath.basename(name).lower() != "nativepages.sm":
        raise ValueError("wrong original working copy")
    path = ntpath.dirname(ntpath.normpath(name)).rstrip("\\") + "\\"
    if hashlib.sha256(path.encode("utf-16-be")).hexdigest() != entry["directorySHA256"]:
        raise ValueError("original work-directory identity mismatch")
    return path


def compare(exe, scratch, require_all, media_driver, replay_directory):
    """Select one original run per source before compiling and compare the complete shared files."""
    baseline = json.loads(REFERENCE.read_bytes())
    revision = baseline["sourceRevision"]
    current = REFERENCE.with_name("原厂SM与介质散件增量复验_20261128.json")
    pinned = {}
    if current.is_file() and media_driver in (None, "reference"):
        receipt = json.loads(current.read_bytes())
        if receipt["revision"] != revision:
            raise ValueError("wrong source revision in regression receipt")
        outputs = receipt["compiledOutputs"]
        mode = "usb" if media_driver is None else "media"
        if replay_directory and "loggedMedia" in outputs:
            mode = "loggedMedia"
        pinned = outputs[mode]["bySource"]
    preferred = {x["sourceSHA256"]: x for x in baseline["preferredSameSourceOriginalBuild"]["preferredSourceRows"]}
    git_tree = subprocess.check_output(["git", "ls-tree", "-rz", revision], cwd=ROOT)
    blobs = {}
    for entry in git_tree.decode("utf-8").split("\0"):
        if entry:
            info, path = entry.split("\t", 1)
            blobs[path] = info.split()[2]
    members = {}
    for path in blobs:
        folder, _, name = path.rpartition("/")
        if Path(name).suffix.lower() in (".bin", ".dat", ".knr", ".kns"):
            members.setdefault(folder, []).append(name)
    reader = subprocess.Popen(["git", "cat-file", "--batch"], cwd=ROOT, stdin=subprocess.PIPE, stdout=subprocess.PIPE)

    def blob(path):
        reader.stdin.write((blobs[path] + "\n").encode("ascii"))
        reader.stdin.flush()
        size = int(reader.stdout.readline().split()[2])
        data = reader.stdout.read(size)
        if len(data) != size or reader.stdout.read(1) != b"\n":
            raise OSError("incomplete Git blob")
        return data

    history = {}
    if replay_directory:
        log_index = json.loads(REFERENCE.with_name("介质KNR原厂248源对照_20261128.json").read_bytes())
        history = {x["source"]: x for x in log_index["historicalWorkingDirectoryExperiment"]["records"]}
    compiler_env = dict(os.environ, TEMP=str(scratch), TMP=str(scratch), TMPDIR=str(scratch))
    rows = []
    try:
        for case in baseline["sourceToCompiledFiles"]["allCases"]:
            key = case["sourceSHA256"]
            oracle = preferred.get(key, {})
            folder = oracle.get("selectedReference", case["path"])
            filenames = members.get(folder, [])
            full_reference = "load.kns" in filenames and "Usbload.bin" in filenames
            source = blob(case["path"] + "/input-native.sm")
            if hashlib.sha256(source).hexdigest() != key:
                raise ValueError("reference source SHA mismatch: " + case["path"])
            row = dict(source=key, reference=folder, fullReference=full_reference, compiled=False, identical=[], differences={}, missing=[])
            with tempfile.TemporaryDirectory(dir=scratch) as temp:
                path = Path(temp)
                sm = path / "input-native.sm"
                sm.write_bytes(source)
                output = path / "out"
                command = [str(exe), "--compile-project", str(sm), str(output)]
                if media_driver is not None:
                    command[1] = "--compile-native-media-parts"
                    omit = media_driver == "omit" or (media_driver == "reference" and "Modbus_RTU_com1.bin" not in filenames)
                    row["mediaCom1Driver"] = "omitted" if omit else "included"
                    if omit:
                        command.append("--without-com1-driver")
                    if key in history:
                        evidence = history[key]
                        if evidence["reference"] != folder:
                            raise ValueError("work-directory log is for a different build")
                        command.extend(["--native-work-directory", original_work_directory(evidence, blob)])
                        row["workDirectoryLog"] = evidence["log"]
                result = subprocess.run(command, cwd=path, env=compiler_env,
                                        capture_output=True, timeout=120, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
                if result.returncode:
                    response = json.loads(result.stdout)
                    if response.get("ok") is not False or not response.get("message"):
                        raise ValueError("unexpected compiler failure: " + case["path"])
                    row["diagnostic"] = response["message"]
                else:
                    row["compiled"] = True
                    for filename in filenames:
                        file = output / filename
                        if not file.is_file():
                            row["missing"].append(filename)
                            continue
                        original, actual = blob(folder + "/" + filename), file.read_bytes()
                        if original == actual:
                            row["identical"].append(filename)
                        else:
                            row["differences"][filename] = next((i for i, (x, y) in enumerate(zip(original, actual))
                                                                 if x != y), min(len(original), len(actual)))
                    row["extra"] = sorted(p.name for p in output.iterdir() if p.name not in filenames)
            expected = {name for name, fields in oracle.get("files", {}).items() if fields["equal"]}
            newer = pinned.get(key, {})
            if newer:
                if newer["reference"] != folder:
                    raise ValueError("regression reference changed: " + folder)
                expected.update(newer["identical"])
            if media_driver == "omit":
                expected.discard("Modbus_RTU_com1.bin")
            row["regressed"] = bool((oracle or newer.get("compiled")) and not row["compiled"]) or not expected.issubset(row["identical"])
            row["allExact"] = full_reference and row["compiled"] and not (row["missing"] or row["differences"] or row.get("extra"))
            rows.append(row)
    finally:
        reader.stdin.close()
        reader.wait()
    totals = dict(sources=len(rows), compiled=sum(r["compiled"] for r in rows),
                  equal=sum(len(r["identical"]) for r in rows), common=sum(len(r["identical"]) + len(r["differences"]) for r in rows),
                  regressed=sum(r["regressed"] for r in rows), allFilesExact=sum(r["allExact"] for r in rows))
    ok = totals["regressed"] == 0 and (not require_all or totals["allFilesExact"] == len(rows))
    return dict(exeSha256=hashlib.sha256(exe.read_bytes()).hexdigest(), revision=revision, mediaDriver=media_driver, replayWorkDirectory=replay_directory, totals=totals, rows=rows), ok


def main():
    """Default to a regression check; --require-all additionally requires complete byte identity for every project."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--temp-root", type=Path, default=ROOT / "build")
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--media-driver", choices=("include", "omit", "reference"),
                        help="Compare offline media parts. 'reference' supplies only the original COM1 driver presence.")
    parser.add_argument("--replay-work-directory", action="store_true",
                        help="Use original logs where available with --media-driver reference.")
    parser.add_argument("--require-all", action="store_true")
    parser.add_argument("--child", action="store_true", help=argparse.SUPPRESS)
    args = parser.parse_args()
    if args.replay_work_directory and args.media_driver != "reference":
        parser.error("--replay-work-directory requires --media-driver reference")
    if sys.platform == "win32" and not args.child:
        return on_private_desktop(sys.argv[1:])
    args.temp_root.mkdir(parents=True, exist_ok=True)
    report, ok = compare(args.exe.resolve(), args.temp_root.resolve(), args.require_all, args.media_driver, args.replay_work_directory)
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(report, ensure_ascii=False, indent=1), encoding="utf-8")
    print(json.dumps(report["totals"]))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
