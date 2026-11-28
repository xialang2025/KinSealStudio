"""Reproduce the two KNR path fields from independent V5.1 open/compile logs."""  # 20261128-22 add
import argparse
import hashlib
import json
from pathlib import Path, PureWindowsPath
import subprocess
import tempfile
import sys
import zlib

from verify_native_drawing_drafts import ROOT, REVISION, git_blob, run
from verify_native_media_gallery import fields

sys.path.insert(0, str(ROOT / "tools"))
from verify_native_compiler_against_git import original_opened_project

LIBRARY = ROOT / "docs/文件规则/样例/介质KNR原厂248源对照_20261128.json"


def collect_witnesses():
    """Select run records using the source hash, working-copy hash and compilation label before reading any KNR."""
    names = subprocess.check_output(["git", "ls-tree", "-rz", "--name-only", REVISION], cwd=ROOT).decode().split("\0")
    originals = set(names)
    def read_blob(name):
        folder, file = name.rsplit("/", 1)
        return git_blob(folder, file)

    records = {}
    for row in json.loads(LIBRARY.read_bytes())["cases"]:
        key = row["source"]
        selected = original_opened_project(key, row["reference"], originals, read_blob)
        if selected:
            records[key] = selected
    return records


def test_syntax(exe, work, source, expected):
    """Check offline UNC/drive inputs, UTF-16 filename casing, mutual exclusion and failure before publication."""
    sm, out = work / "input.sm", work / "syntax.knr"
    sm.write_bytes(source)
    good = ["//offline.test/share name/工程 α.SM", "Z:/root-file.sm", "Z:/sub.sm/sub.ksp/test.sm"]
    for path in good:
        out.unlink(missing_ok=True)
        run(exe, work, "--build-native-media-snapshot", sm, out, "--native-opened-project", path)
        decoded = zlib.decompress(out.read_bytes()[4:])
        fs = fields(exe, work, decoded)
        expected_dir = str(PureWindowsPath(path).parent).rstrip("\\") + "\\"
        for field, value in (("trailer.settings.0", expected_dir), ("trailer.settings.1", PureWindowsPath(path).name)):
            encoded = value.encode("utf-16-be")
            if decoded[fs[field]] != len(encoded).to_bytes(4, "big") + encoded:
                raise ValueError(field + " disagrees for " + path)
    bad = ["relative.sm", "C:relative.sm", "Z:/folder", "Z:/file.sm/", "Z:/name.ksp", "Z:/.sm", "Z:/CON.sm",
           "Z:/file?.sm", "//host/share.sm", "Z:/bad/../test.sm"]
    controls = [["--native-opened-project", path] for path in bad]
    controls += [["--native-opened-project", good[0], "--native-work-directory", "Z:/dir"],
                 ["--native-work-directory", "Z:/dir", "--native-opened-project", good[0]],
                 ["--native-opened-project", good[1], "--native-opened-project", good[0]]]
    for args in controls:
        out.unlink(missing_ok=True)
        run(exe, work, "--build-native-media-snapshot", sm, out, *args, success=False)
        if out.exists():
            raise ValueError("invalid context published a KNR")
    media = work / "disk"
    run(exe, work, "--compile-native-media-parts", sm, media, "--native-opened-project", expected, "--without-com1-driver")
    run(exe, work, "--build-native-media-snapshot", sm, out, "--native-opened-project", expected)
    if (media / "upLoad.knr").read_bytes() != out.read_bytes() or sm.read_bytes() != source:
        raise ValueError("the two public entry points or the source differ")
    return dict(validPaths=len(good), rejected=len(controls), bothCompilerEntrypointsMatched=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    parser.add_argument("--scratch", type=Path, required=True)
    parser.add_argument("--receipt", type=Path, required=True)
    args = parser.parse_args()
    exe = args.exe.resolve()
    witnesses = collect_witnesses()
    rows, syntax = [], None
    with tempfile.TemporaryDirectory(dir=args.scratch) as temporary:
        work = Path(temporary).resolve()
        sm, out = work / "input.sm", work / "generated.knr"
        for key, witness in witnesses.items():
            original = git_blob(witness["reference"], "input-native.sm")
            if hashlib.sha256(original).hexdigest() != key:
                raise ValueError("source hash mismatch")
            sm.write_bytes(original)
            out.unlink(missing_ok=True)
            run(exe, work, "--build-native-media-snapshot", sm, out, "--native-opened-project", witness["openedProject"])
            expected = git_blob(witness["reference"], "upLoad.knr")
            actual = out.read_bytes()
            if sm.read_bytes() != original:
                raise ValueError("input changed")
            comparison = []
            for data in (original, zlib.decompress(actual[4:]), zlib.decompress(expected[4:])):
                spans = fields(exe, work, data)
                comparison.append([data[spans["trailer.settings." + str(i)]] for i in (0, 1)])
            row = dict(**witness, directoryExact=comparison[1][0] == comparison[2][0],
                       filenameExact=comparison[1][1] == comparison[2][1],
                       originalDirectoryExact=comparison[0][0] == comparison[2][0],
                       originalFilenameExact=comparison[0][1] == comparison[2][1],
                       fullKnrExact=actual == expected)
            rows.append(row)
            if syntax is None and "lamp-default-r1" in witness["reference"]:
                syntax = test_syntax(exe, work, original, witness["openedProject"])
    result = dict(revision=REVISION, exeSHA256=hashlib.sha256(exe.read_bytes()).hexdigest(), accepted=len(rows),
                  summary={k:sum(r[k] for r in rows) for k in ("directoryExact", "filenameExact", "originalDirectoryExact",
                                                               "originalFilenameExact", "fullKnrExact")},
                  controls=syntax, rows=rows)
    print(result["accepted"], result["summary"], syntax)
    args.receipt.write_text(json.dumps(result, ensure_ascii=False, indent=1) + "\n", encoding="utf8")
    if len(rows) != 118 or syntax is None or any(not (row["directoryExact"] and row["filenameExact"]) for row in rows):
        raise ValueError("an independently bound original open/compile run or its path fields disagree")


if __name__ == "__main__":
    main()
