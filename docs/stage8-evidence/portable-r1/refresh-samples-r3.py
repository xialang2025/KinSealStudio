"""Incrementally copy only the three comment-only Stage8 sample updates."""
#20261114-7 add: Preserve production, runtime and all earlier acceptance evidence.
from __future__ import annotations
from datetime import datetime, timezone
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import shutil
import sys

sys.dont_write_bytecode = True
WORKTREE = Path(__file__).resolve().parents[3]
EVIDENCE = Path(__file__).resolve().parent
PACKAGE = Path(r"Z:\Project\vscode_prj_temp\REA\KinSealStudio\dist\KinSealStudioLite-Stage8-macro-r1")
SELECTED = ("samples/stage8/ClearLB1.c", "samples/stage8/SetLB1.c",
            "samples/stage8/ordinary-two-macros.ksp")


def sha(path):
    """Input: file. Return: exact SHA256."""
    return hashlib.sha256(path.read_bytes()).hexdigest()


def tokens(source):
    """Input: these known sample snippets. Return: tokens after stripping C comments and whitespace."""
    return re.findall(r"\w+|[^\s]", re.sub(r"/\*.*?\*/|//[^\r\n]*", "", source, flags=re.S))


def save(path, value):
    """Input: own JSON path/value. Return: None after saving the new content identity."""
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def main():
    """Input: None. Return: zero after three comment-only copies and manifest updates."""
    marker = EVIDENCE / "sample-refresh-r3.json"
    if marker.exists():
        raise ValueError("This exact sample amendment already has a receipt.")
    manifest_path = PACKAGE / "manifest.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    old_manifest_sha = sha(manifest_path)
    for row in manifest["files"]:
        path = PACKAGE / row["path"]
        if sha(path) != row["sha256"] or path.stat().st_nlink != 1:
            raise ValueError("Package changed outside this amendment: " + row["path"])
    old_project = json.loads((PACKAGE / SELECTED[2]).read_text(encoding="utf-8"))
    new_project = json.loads((WORKTREE / SELECTED[2]).read_text(encoding="utf-8"))
    if {k: v for k, v in old_project.items() if k != "macros"} != {k: v for k, v in new_project.items() if k != "macros"}:
        raise ValueError("Unexpected non-macro project changes.")
    for name, path in (("ClearLB1", SELECTED[0]), ("SetLB1", SELECTED[1])):
        if tokens((PACKAGE / path).read_text(encoding="utf-8")) != tokens((WORKTREE / path).read_text(encoding="utf-8")):
            raise ValueError("Unexpected executable C token change: " + name)
        if tokens(old_project["macros"][name]) != tokens(new_project["macros"][name]):
            raise ValueError("Unexpected executable embedded-source token change: " + name)
        if new_project["macros"][name] != (WORKTREE / path).read_text(encoding="utf-8"):
            raise ValueError("Embedded source does not equal the accompanying C sample: " + name)
    changes = []
    for relative in SELECTED:
        source = WORKTREE / relative
        destination = PACKAGE / relative
        old = next(row.copy() for row in manifest["files"] if row["path"] == relative)
        changes.append(dict(path=relative, before=old,
                            after=dict(bytes=source.stat().st_size, sha256=sha(source)),
                            inputPath=str(source)))
        shutil.copyfile(source, destination)
        if sha(destination) != sha(source) or destination.stat().st_nlink != 1:
            raise ValueError("Updated sample copy differs: " + relative)
        row = next(row for row in manifest["files"] if row["path"] == relative)
        row["bytes"] = destination.stat().st_size
        row["sha256"] = sha(destination)
    spec = importlib.util.spec_from_file_location("stage8_packaging", WORKTREE / "tools/package_stage8.py")
    helper = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(helper)
    manifest["fileContentSha256"] = helper.content_digest(manifest["files"])
    manifest["contentRefreshedAtUtc"] = datetime.now(timezone.utc).isoformat()
    save(manifest_path, manifest)
    receipt = dict(schemaVersion=1, changes=changes, executableTokensUnchanged=True,
                   embeddedSourcesMatchCFiles=True, productionExecutableUnchanged=True,
                   beforeManifestSha256=old_manifest_sha, afterManifestSha256=sha(manifest_path),
                   sampleCount=164, sourceContentSha256=manifest["sourceContentSha256"],
                   packageFileContentSha256=manifest["fileContentSha256"])
    save(marker, receipt)
    input_path = EVIDENCE / "inputs.json"
    inputs = json.loads(input_path.read_text(encoding="utf-8"))
    inputs.setdefault("contentAmendments", []).append(dict(
        reason="父代理按用户要求补全三个宏样例的英文函数头及日期注释；独立token检查无可执行变化",
        changedInputs=changes, receipt=str(marker), newManifestSha256=sha(manifest_path)))
    save(input_path, inputs)
    package_path = EVIDENCE / "package.json"
    record = json.loads(package_path.read_text(encoding="utf-8"))
    record["manifestSha256"] = sha(manifest_path)
    record["packageBytes"] = sum(row["bytes"] for row in manifest["files"]) + manifest_path.stat().st_size
    save(package_path, record)
    print(json.dumps(dict(updatedFiles=len(changes), manifestSha256=sha(manifest_path),
                          packageBytes=record["packageBytes"])))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

