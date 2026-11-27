"""Rebuild native KNRs from original SMs and separately recorded editor work directories."""
# 20261128-8 add
import argparse
import gzip
import hashlib
import json
from pathlib import Path, PureWindowsPath
import re
import struct
import tempfile
import zlib

from verify_native_drawing_drafts import git_blob, run, ROOT

WITNESSES = ROOT / "docs/文件规则/样例/介质KNR原厂248源对照_20261128.json"


def work_directory(row):
    """Read a run-bound working copy, window title or first MRU entry before loading the vendor archive."""
    folder, name = row["log"].rsplit("/", 1)
    encoded = git_blob(folder, name)
    node = json.loads(gzip.decompress(encoded) if name.endswith(".gz") else encoded)
    for attribute, index in re.findall(r"\.([^.\[]+)|\[(\d+)\]", row["key"]):
        node = node[attribute] if attribute else node[int(index)]
    match = re.search(r'([A-Z]:[\\/][^:*?"<>|]*?[\\/]NativePages\.sm)(?=--|$)', node, re.I)
    if not match:
        raise ValueError("the independent run log does not identify an opened NativePages.sm")
    directory = str(PureWindowsPath(match[1]).parent) + "\\"
    if hashlib.sha256(directory.encode("utf-16-be")).hexdigest() != row["directorySHA256"]:
        raise ValueError("the run log disagrees with its registered directory")
    return directory


def inspect_directory_value(exe, directory, knr, expected):
    """Read the generated trailer as a length-prefixed UTF-16BE QString."""
    decoded = zlib.decompress(knr.read_bytes()[4:])
    sm = directory / "recovered.sm"
    sm.write_bytes(decoded)
    fields = json.loads(run(exe, directory, "--inspect-native-source", sm))["fields"]
    field = next(f for f in fields if f["path"] == "trailer.settings.0")
    data = expected.encode("utf-16-be")
    if decoded[field["start"]:field["end"]] != struct.pack(">I", len(data)) + data:
        raise ValueError("the generated work-directory QString is not canonical")


def syntax_cases(exe, root, source):
    """Check offline drive/UNC paths and reject malformed, reserved or ambiguous inputs without publishing."""
    sm = root / "case.sm"
    sm.write_bytes(source)
    accepted = [("Z:/", "Z:\\"), ("z:/离线/Project///", "z:\\离线\\Project\\"),
                ("//unreachable-host/share", "\\\\unreachable-host\\share\\"),
                ("//unreachable-host/share/Offline", "\\\\unreachable-host\\share\\Offline\\")]
    refused = ("relative", "D:relative", "\\rootonly", "\\\\server", "\\\\.\\C:\\device",
               "C:\\A\\..\\B", "C:\\A\\aux.log", "C:\\A|B", "C:\\work\\file.sm", "C:\\work\\a.ksp",
               "C:\\work\\bad\x01", "C:\\work\\a.", "C:\\work\\a  ", "C:\\ok\\\\gap", "")
    for index, (path, canonical) in enumerate(accepted):
        output = root / ("valid" + str(index) + ".knr")
        run(exe, root, "--build-native-media-snapshot", sm, output, "--native-work-directory", path)
        inspect_directory_value(exe, root, output, canonical)
    for index, path in enumerate(refused):
        output = root / ("invalid" + str(index) + ".knr")
        run(exe, root, "--build-native-media-snapshot", sm, output, "--native-work-directory", path, success=False)
        if output.exists():
            raise ValueError("invalid path published a partial archive")
    return {"accepted": len(accepted), "rejected": len(refused)}


def run_corpus(exe, scratch, witnesses, directories):
    """Compile all comparable original SMs, using independently known directories only for the 47 documented runs."""
    preferred = json.loads(WITNESSES.read_bytes())["cases"]
    original = json.loads((WITNESSES.parent / "原厂编译字节复核_20261127.json").read_bytes())
    sources = {row["sourceSHA256"]: row["path"] for row in original["sourceToCompiledFiles"]["allCases"]}
    results = []
    for baseline in preferred:
        if "parseIssue" in baseline:
            continue
        key = baseline["source"]
        with tempfile.TemporaryDirectory(dir=scratch) as space:
            path = Path(space).resolve()
            source = git_blob(sources[key], "input-native.sm")
            if hashlib.sha256(source).hexdigest() != key:
                raise ValueError("incorrect input source during standalone compilation")
            sm = path / "source.sm"
            output = path / "compiled.knr"
            sm.write_bytes(source)
            row = dict(source=key, reference=baseline["reference"], workDirectoryFromIndependentLog=key in directories)
            args = ("--native-work-directory", directories[key]) if key in directories else ()
            try:
                run(exe, path, "--build-native-media-snapshot", sm, output, *args)
            except ValueError as error:
                row.update(compiled=False, fullFileExact=False, diagnostic=str(error))
            else:
                actual = output.read_bytes()
                expected = git_blob(baseline["reference"], "upLoad.knr")
                equal = actual == expected
                offset = next((i for i, (a, b) in enumerate(zip(actual, expected)) if a != b),
                              None if equal else min(len(actual), len(expected)))
                row.update(compiled=True, fullFileExact=equal, firstDifference=offset,
                           actualSHA256=hashlib.sha256(actual).hexdigest(), expectedSHA256=hashlib.sha256(expected).hexdigest())
            row["expectedReferenceMatch"] = bool(baseline.get("compressedArchiveExact") or key in witnesses)
            if sm.read_bytes() != source:
                raise ValueError("the standalone archive command modified its input")
            results.append(row)
    return {"comparable": len(results), "absentOriginal": len(preferred) - len(results),
            "compiled": sum(row["compiled"] for row in results), "exact": sum(row["fullFileExact"] for row in results),
            "regressions": sum(row["expectedReferenceMatch"] and not row["fullFileExact"] for row in results), "rows": results}


def main():
    """Require complete original file equality for every historical work-directory observation."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    parser.add_argument("--scratch", type=Path, required=True)
    parser.add_argument("--receipt", type=Path, required=True)
    parser.add_argument("--all", action="store_true", help="Also run the standalone compiler for all 242 original archives")
    args = parser.parse_args()
    exe = args.exe.resolve()
    rows = json.loads(WITNESSES.read_bytes())["historicalWorkingDirectoryExperiment"]["records"]
    # No original KNR participates in the derivation of this mapping.
    directories = {row["source"]: work_directory(row) for row in rows}
    checked = []
    syntax = None
    for row in rows:
        with tempfile.TemporaryDirectory(dir=args.scratch) as space:
            temp = Path(space).resolve()
            source = git_blob(row["reference"], "input-native.sm")
            if hashlib.sha256(source).hexdigest() != row["source"]:
                raise ValueError("wrong original SM")
            sm = temp / "original.sm"
            output = temp / "actual.knr"
            sm.write_bytes(source)
            run(exe, temp, "--build-native-media-snapshot", sm, output, "--native-work-directory", directories[row["source"]])
            actual = output.read_bytes()
            original = git_blob(row["reference"], "upLoad.knr")
            if original != actual or sm.read_bytes() != source:
                raise ValueError("full-file mismatch or source overwrite: " + row["reference"])
            if syntax is None:
                syntax = syntax_cases(exe, temp, source)
            checked.append({"source": row["source"], "reference": row["reference"], "directoryInputLog": row["log"],
                            "archiveSHA256": hashlib.sha256(actual).hexdigest(), "completeFileExact": True})
    report = {"exeSHA256": hashlib.sha256(exe.read_bytes()).hexdigest(), "originalRevision": "e3b72657",
              "samples": len(checked), "pathCases": syntax, "rows": checked}
    if args.all:
        report["standaloneCompiler"] = run_corpus(exe, args.scratch, set(directories), directories)
    args.receipt.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf8")
    print(len(checked), "complete KNRs match", syntax)
    if args.all:
        summary = report["standaloneCompiler"]
        print({k: v for k, v in summary.items() if k != "rows"})
        return int(summary["regressions"] != 0)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
