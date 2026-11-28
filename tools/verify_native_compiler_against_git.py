"""Recompile the frozen V5.1 inputs and compare the actual files with independent original Git blobs."""
# 20261128-2 add
import argparse
import gzip
import hashlib
import io
import json
import ntpath
import os
from pathlib import Path, PureWindowsPath
import subprocess
import sys
import tempfile
import uuid
import zipfile

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


def original_drawing_sessions(read_blob):
    """Bind the original table and boundary drawing sessions to their observed source and run label."""
    logs = ("docs/stage4-evidence/table/native/flags-investigation/native-probe-table-flags-r1.json.gz",
            "docs/stage5-evidence/native/freeline/freeline-screen-edge-create-r2/native-probe.json.gz",
            "docs/stage5-evidence/native/polyline/boundary-r1/native-probe.json.gz")
    sessions = {}
    for path in logs:
        log = json.loads(gzip.decompress(read_blob(path)))
        directory = path.rsplit("/", 1)[0]
        for build in log["nativeCompilations"]:
            sessions[(build["sourceSha256"], directory + "/compiled/" + build["label"])] = path
    return sessions


def original_opened_project(source, reference, originals, read_blob):  # 20261128-22 add
    """Bind one historical working copy to this exact source hash and original compile directory."""
    if "/compiled/" not in reference:
        return None
    root, label = reference.rsplit("/compiled/", 1)
    run_name = root.rsplit("/", 1)[-1]
    if root == "docs/stage2-evidence/lamp-regression-restore":
        candidates = [f"{root}/native-probe-{label}.json"]
    elif root == "docs/stage3-evidence/native" and label.startswith("static-odd2-"):
        tag = label.removeprefix("static-odd2-")
        candidates = [f"{root}/native-probe-pixel-{tag}-r1.json.gz"]
    elif root.startswith("docs/stage4-evidence/") and "/physical/" in root and label == "physical-native-reference":
        candidates = [f"{root}/native-probe-{run_name}.json", f"{root}/native-probe.json.gz"]
    elif root.startswith("docs/stage5-evidence/native/") and label == "fresh-process-3":
        candidates = [f"{root}/native-probe.json.gz"]
    else:
        return None
    for name in candidates:
        if name not in originals:
            continue
        data = read_blob(name)
        log = json.loads(gzip.decompress(data) if name.endswith(".gz") else data)
        working = log.get("workingCopyAfter") or {}
        matches = [c for c in log.get("nativeCompilations", []) if c["label"] == label and c.get("sourceSha256") == source]
        if log.get("input", {}).get("sha256") != source or working.get("sha256") != source or len(matches) != 1:
            continue
        opened = PureWindowsPath(working["path"])
        if opened.is_absolute() and opened.suffix.lower() == ".sm":
            return dict(source=source, reference=reference, log=name, openedProject=working["path"])
    return None


def original_message_sessions(originals, read_blob):  # 20261128-27 add
    """Bind each message-table profile to its own open process, source SHA and native compilation directory."""
    prefix = "docs/stage4-evidence/message/native/"
    sessions = {}
    for path in originals:
        if not path.startswith(prefix) or not path.endswith(".json") or "/native-probe-" not in path:
            continue
        log = json.loads(read_blob(path))
        for ordinal, build in enumerate(log.get("nativeCompilations", [])):
            key = (build["sourceSha256"], prefix + "compiled/" + build["label"])
            if key in sessions:
                raise ValueError("two different native sessions claim a message compilation: " + key[1])
            sessions[key] = ("download-defaults" if ordinal else "fresh", path)
    if not sessions:
        raise ValueError("missing original message compiler logs")
    return sessions


def compare(exe, scratch, require_all, media_driver, replay_directory, replay_drawing=False, replay_opened=False, replay_message=False):
    """Select one original run per source before compiling and compare the complete shared files."""
    initial_exe_hash = hashlib.sha256(exe.read_bytes()).hexdigest()  # 20261128-34 add
    baseline = json.loads(REFERENCE.read_bytes())
    revision = baseline["sourceRevision"]
    # Protect the latest whole files, including native text, curve pages and exact sprite padding.
    current = REFERENCE.with_name("AMZ固件与原厂编译逐文件复验_20261128.json")  # 20261128-19 change
    pinned = {}
    if media_driver in (None, "reference"):  # 20261128-9 change
        receipt = json.loads(current.read_bytes())
        if receipt["revision"] != revision:
            raise ValueError("wrong source revision in regression receipt")
        outputs = receipt["compiledOutputs"]
        mode = "usb" if media_driver is None else "media"
        if replay_directory:
            mode = "drawing" if replay_drawing else "loggedMedia"
        # The newer run froze default USB and the two fully specified historical replays.
        # Preserve the earlier baselines for other media options with a different set of inputs.
        replay_mode = None
        if media_driver is None and not replay_drawing:
            replay_mode = "usb"
        elif media_driver == "reference" and replay_directory and replay_drawing:
            replay_mode = "opened" if replay_opened else "drawing"
        if replay_mode is not None:
            latest = json.loads(REFERENCE.with_name("V122源档与原厂编译第七次逐件复验_20261128.json").read_bytes())
            if latest["revision"] != revision:
                raise ValueError("wrong source revision in the latest regression receipt")
            if replay_mode in ("usb", "opened"):
                latest = json.loads(REFERENCE.with_name("V122刻度图表与原厂编译第九次逐件复验_20261128.json").read_bytes())
                if latest["revision"] != revision:
                    raise ValueError("wrong source revision in the ninth whole-file comparison")
                if replay_mode == "opened" and replay_message:
                    replay_mode = "messageSession"
            pinned = latest["compiledOutputs"][replay_mode]["bySource"]
        else:
            pinned = outputs[mode]["bySource"]
        if not pinned:
            raise ValueError("empty regression receipt: " + mode)
    cases = baseline["sourceToCompiledFiles"]["allCases"]
    sources = {case["sourceSHA256"] for case in cases}
    if not cases or len(sources) != len(cases) or (pinned and sources != pinned.keys()):
        raise ValueError("empty, duplicate or mismatched source set in the frozen comparison corpus")
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
    drawing_sessions = original_drawing_sessions(blob) if replay_drawing else {}  # 20261128-19 add
    message_sessions = original_message_sessions(blobs, blob) if replay_message else {}
    compiler_env = dict(os.environ, TEMP=str(scratch), TMP=str(scratch), TMPDIR=str(scratch))
    rows = []
    try:
        for case in cases:
            key = case["sourceSHA256"]
            oracle = preferred.get(key, {})
            folder = oracle.get("selectedReference", case["path"])
            filenames = members.get(folder, [])
            full_reference = "load.kns" in filenames and "Usbload.bin" in filenames
            if blobs.get(folder + "/input-native.sm") != blobs[case["path"] + "/input-native.sm"]:
                raise ValueError("selected original output is for a different source: " + folder)
            if full_reference:  # 20261128-9 change
                # Every member of this original package must also have a loose comparison file.
                with zipfile.ZipFile(io.BytesIO(blob(folder + "/load.kns"))) as archive:
                    archived = archive.namelist()
                full_reference = (len(archived) == len(set(archived))
                                  and set(archived) == set(filenames) - {"load.kns"})
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
                    opened = original_opened_project(key, folder, blobs, blob) if replay_opened else None
                    if opened is not None:
                        if (opened["source"], opened["reference"]) != (key, folder):
                            raise ValueError("the opened-project log refers to a different source or compile directory")
                        command.extend(["--native-opened-project", opened["openedProject"]])
                        row["openedProjectLog"] = opened["log"]
                    elif key in history:
                        evidence = history[key]
                        if evidence["reference"] != folder:
                            raise ValueError("work-directory log is for a different build")
                        command.extend(["--native-work-directory", original_work_directory(evidence, blob)])
                        row["workDirectoryLog"] = evidence["log"]
                if (key, folder) in message_sessions:
                    profile, row["messageSessionLog"] = message_sessions[(key, folder)]
                    command.extend(["--simulator-session", profile])
                    row["simulatorSession"] = profile
                if (key, folder) in drawing_sessions:
                    row["createdDrawingLog"] = drawing_sessions[(key, folder)]
                    command.append("--native-created-drawing")
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
            # Preserve previously emitted files while their contents are still being matched.
            previously_emitted = set(oracle.get("files", {}))  # 20261128-9 add
            newer = pinned.get(key, {})
            if newer:
                if newer["reference"] != folder:
                    raise ValueError("regression reference changed: " + folder)
                expected.update(newer["identical"])
                previously_emitted.update(newer["identical"])
                previously_emitted.update(newer.get("differences", {}))
            if media_driver == "omit":
                expected.discard("Modbus_RTU_com1.bin")
                previously_emitted.discard("Modbus_RTU_com1.bin")
            row["lostOutputs"] = sorted(previously_emitted.difference(row["identical"], row["differences"]))
            row["regressed"] = (bool((oracle or newer.get("compiled")) and not row["compiled"])
                                or bool(row["lostOutputs"]) or not expected.issubset(row["identical"]))
            row["allExact"] = full_reference and row["compiled"] and not (row["missing"] or row["differences"] or row.get("extra"))
            rows.append(row)
    finally:
        reader.stdin.close()
        reader.wait()
    totals = dict(sources=len(rows), compiled=sum(r["compiled"] for r in rows),
                  equal=sum(len(r["identical"]) for r in rows), common=sum(len(r["identical"]) + len(r["differences"]) for r in rows),
                  regressed=sum(r["regressed"] for r in rows), allFilesExact=sum(r["allExact"] for r in rows))
    # These counts use compiled cases only. A rejected source never supplies an output directory.
    totals["different"] = totals["common"] - totals["equal"]
    totals["missing"] = sum(len(r["missing"]) for r in rows)
    totals["referenceFilesForCompiled"] = totals["common"] + totals["missing"]
    totals["extra"] = sum(len(r.get("extra", [])) for r in rows)
    ok = totals["regressed"] == 0 and (not require_all or totals["allFilesExact"] == len(rows))
    if hashlib.sha256(exe.read_bytes()).hexdigest() != initial_exe_hash:
        raise RuntimeError("the compiler was replaced during the original-file comparison")
    return dict(exeSha256=initial_exe_hash, revision=revision, mediaDriver=media_driver, replayWorkDirectory=replay_directory, replayDrawingState=replay_drawing, **({"replayOpenedProject": True} if replay_opened else {}), **({"replayMessageSession": True} if replay_message else {}), totals=totals, rows=rows), ok


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
    parser.add_argument("--replay-opened-project", action="store_true",
                        help="Replay both path fields when the open and compile logs match the selected source and run.")
    parser.add_argument("--replay-drawing-state", action="store_true",
                        help="Replay the freshly drawn pen only for source/run pairs in the three independent editor logs.")
    parser.add_argument("--replay-message-session", action="store_true",
                        help="Use V5.1 process logs to replay the default download-window flags of message projects.")
    parser.add_argument("--require-all", action="store_true")
    parser.add_argument("--child", action="store_true", help=argparse.SUPPRESS)
    args = parser.parse_args()
    if (args.replay_work_directory or args.replay_opened_project or args.replay_message_session) and args.media_driver != "reference":
        parser.error("the historical path options require --media-driver reference")
    if sys.platform == "win32" and not args.child:
        return on_private_desktop(sys.argv[1:])
    args.temp_root.mkdir(parents=True, exist_ok=True)
    report, ok = compare(args.exe.resolve(), args.temp_root.resolve(), args.require_all, args.media_driver, args.replay_work_directory, args.replay_drawing_state, args.replay_opened_project, args.replay_message_session)
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(report, ensure_ascii=False, indent=1), encoding="utf-8")
    print(json.dumps(report["totals"]))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
