"""Create an isolated Stage8 portable package from explicitly frozen inputs."""
#20261114-7 add: Copy only known runtime files and retain auditable content identities.
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess


README = """KinSealStudioLite 阶段8便携验证包

启动：双击本目录 KinSealStudioLite.exe。Qt运行依赖在本目录及子目录中，
无需从已安装的Qt或原厂程序目录加载。samples包含本工作树全部样例。
本包不含原厂EXE或ARM SDK，也不创建到原厂运行目录的链接。

阶段8使用入口
在主菜单中的“宏指令编辑器”，或工程树“宏指令编辑器”入口，
打开普通宏源码工作区。可新建、编辑、复制、改名、删除、导入/导出.c源码；
有引用或不透明原厂依赖的宏会受到删除保护。函数库/图形库共有36项，
用于插入已核对的源码示例。名称是显示键，编译入口仍按Macro_Entry约定。

当前编译范围
仅支持已核目标型号的普通宏、零地址变量编译，需自行配置外部原厂ARM SDK。
可以在应用的宏工具链设置中选择SDK目录，或从环境变量KINSEAL_ARM_SDK指定。
SDK应指向包含armcc.exe、armlink.exe、fromelf.exe、Include与配套库的script目录。
命令行格式：
  KinSealStudioLite.exe --compile-macros 工程.ksp 独立报告.json
报告输出不得覆盖输入工程。成功报告包含源码哈希、SDK/过程清单、
MarcoRun/MarcoVar及编译索引；成功编译不等于触摸屏已接受或执行。

临时目录
为避免C盘临时文件增长，建议将TEMP、TMP、TMPDIR及KINSEAL_MACRO_TEMP_ROOT
都设置到自己有写权限的Z盘临时目录；KINSEAL_MACRO_TEMP_ROOT必须已存在，
且不得位于SDK目录内部。宏临时物由编译事务管理，失败状态应查看过程报告。
本次包验收全部临时物均放在独立Z盘任务目录。

能力边界
地址变量表、初始化/全局/画面调度、实际触摸屏宏调用与变量交换仍待后续阶段。
普通编辑、导入和编译分别有自己的名称/编码/大小门限，保存草稿并不保证
原厂源导出或ANSI SDK编译一定接受。原厂源码改写仍按已恢复元数据保守拒绝。
前期局部“写宏”行为不代表完整宏系统：本地位/字按钮、滑动开关和下拉框的
源码绑定不能当作离线模拟器会执行C宏、地址变量或调度的证明。
本包保留前期已有功能，但本轮验收未进行USB/PLC下载、实屏显示或触控验证。

附带tools/stage6_media_bundle.py是已有离线介质工具，单独使用时需要Python。
manifest.json记录输入/输出SHA256与源码内容身份；codeCommit仅在真实提交后绑定。
"""


def file_sha(path: Path) -> str:  #20261114-7 add
    """Function Name: file_sha
    Description: Hash complete input bytes without encoding normalization.
    Input Parameters: path
    Return Value: Lowercase SHA256.
    Date: 2026.11.14
    """
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def identity(path: Path, relative: str | None = None) -> dict:  #20261114-7 add
    """Function Name: identity
    Description: Record the exact size and digest of a regular unlinked source.
    Input Parameters: path, relative
    Return Value: File identity with its original input path.
    Date: 2026.11.14
    """
    if path.is_symlink() or (path.stat().st_file_attributes & 0x400):
        raise ValueError("Reparse input is outside package scope: " + str(path))
    return {"path": relative or path.name, "inputPath": str(path),
            "bytes": path.stat().st_size, "sha256": file_sha(path)}


def tree_files(root: Path) -> list[Path]:  #20261114-7 add
    """Function Name: tree_files
    Description: Enumerate ordinary files and refuse directory or file reparse points.
    Input Parameters: root
    Return Value: Sorted regular files.
    Date: 2026.11.14
    """
    result = []
    for current, directories, files in os.walk(root, followlinks=False):
        for name in directories + files:
            entry = Path(current) / name
            if entry.is_symlink() or (entry.stat().st_file_attributes & 0x400):
                raise ValueError("Reparse tree entry is outside package scope: " + str(entry))
        result.extend(Path(current) / name for name in files)
    return sorted(result, key=lambda item: item.relative_to(root).as_posix())


def source_identity(worktree: Path) -> list[dict]:  #20261114-7 add
    """Function Name: source_identity
    Description: Freeze production source and embedded resources before packaging.
    Input Parameters: worktree
    Return Value: Ordered exact source identities; no commit is inferred.
    Date: 2026.11.14
    """
    paths = [worktree / "CMakeLists.txt"]
    paths.extend(worktree.glob("*.qrc"))
    for directory in ("src", "resources"):
        paths.extend(tree_files(worktree / directory))
    return [identity(path, path.relative_to(worktree).as_posix())
            for path in sorted(set(paths), key=lambda item: item.relative_to(worktree).as_posix())]


def content_digest(rows: list[dict]) -> str:  #20261114-7 add
    """Function Name: content_digest
    Description: Bind ordered path and exact file hashes without absolute-path dependence.
    Input Parameters: rows
    Return Value: SHA256 over UTF8(path), NUL, ASCII(sha256), LF for each row.
    Date: 2026.11.14
    """
    payload = b"".join(row["path"].encode("utf-8") + b"\0" +
                       row["sha256"].encode("ascii") + b"\n" for row in rows)
    return hashlib.sha256(payload).hexdigest()


def write_json(path: Path, value) -> None:  #20261114-7 add
    """Function Name: write_json
    Description: Write only a caller-owned manifest or evidence JSON.
    Input Parameters: path, value
    Return Value: None.
    Date: 2026.11.14
    """
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def create_package(args) -> None:  #20261114-7 add
    """Function Name: create_package
    Description: Copy frozen runtime, current executable, samples and one inherited tool into a new package.
    Input Parameters: args
    Return Value: None; refuses existing output paths and never deletes inputs.
    Date: 2026.11.14
    """
    worktree = args.worktree.resolve()
    stage7 = args.runtime.resolve()
    package = args.package.resolve()
    evidence = args.evidence.resolve()
    executable = worktree / "build/KinSealStudioLite.exe"
    if file_sha(executable) != args.expected_sha.lower():
        raise ValueError("Build EXE identity changed; no output created.")
    if package.exists() or evidence.exists():
        raise ValueError("Package and evidence must be new paths.")
    if package.drive.lower() != "z:" or evidence.drive.lower() != "z:":
        raise ValueError("Package and evidence must remain on the task Z drive.")
    sources = source_identity(worktree)
    selected = {}
    old_manifest = json.loads((stage7 / "manifest.json").read_text(encoding="utf-8-sig"))
    old_rows = {row["path"]: row for row in old_manifest["runtimeFiles"]}
    for path in tree_files(stage7):
        relative = path.relative_to(stage7).as_posix()
        if path.suffix.lower() == ".dll" or relative in ("LZO-MIT-LICENSE.txt", "QR-MIT-LICENSE.txt"):
            row = identity(path, relative)
            old = old_rows.get(relative)
            if old is None or old["sha256"] != row["sha256"]:
                raise ValueError("Stage7 runtime manifest mismatch: " + relative)
            row["kind"] = "stage7-runtime"
            selected[relative] = row
    for path in tree_files(worktree / "samples"):
        relative = path.relative_to(worktree).as_posix()
        selected[relative] = {**identity(path, relative), "kind": "current-sample"}
    tool = worktree / "tools/stage6_media_bundle.py"
    if not tool.is_file():
        tool = stage7 / "tools/stage6_media_bundle.py"
    selected["tools/stage6_media_bundle.py"] = {
        **identity(tool, "tools/stage6_media_bundle.py"), "kind": "existing-offline-tool"}
    selected["KinSealStudioLite.exe"] = {
        **identity(executable, "KinSealStudioLite.exe"), "kind": "current-production-executable"}
    inputs = [selected[key] for key in sorted(selected)]
    evidence.mkdir(parents=True, exist_ok=False)
    input_record = dict(schemaVersion=1, recordedAtUtc=datetime.now(timezone.utc).isoformat(),
                        worktree=str(worktree), runtimeSource=str(stage7), package=str(package),
                        buildExecutableSha256=file_sha(executable), sourceFiles=sources,
                        sourceContentSha256=content_digest(sources), inputs=inputs,
                        selectedInputContentSha256=content_digest(inputs),
                        packageBuilder=identity(Path(__file__).resolve()),
                        codeCommit=None, codeCommitState="awaiting-parent-verified-commit")
    write_json(evidence / "inputs.json", input_record)
    package.mkdir(parents=True, exist_ok=False)
    for row in inputs:
        destination = package / row["path"]
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(row["inputPath"], destination)
        if file_sha(destination) != row["sha256"]:
            raise ValueError("Copied file differs: " + str(destination))
        if destination.stat().st_nlink != 1:
            raise ValueError("Package output unexpectedly hardlinked: " + str(destination))
    (package / "使用说明.txt").write_text(README, encoding="utf-8-sig")
    output_rows = [identity(path, path.relative_to(package).as_posix()) for path in tree_files(package)]
    for row in output_rows:
        row.pop("inputPath")
    manifest = dict(schemaVersion=1, stage=8, name=package.name,
                    createdAtUtc=datetime.now(timezone.utc).isoformat(),
                    executableSha256=args.expected_sha.lower(), codeCommit=None,
                    codeCommitState="awaiting-parent-verified-commit",
                    sourceFiles=sources, sourceContentSha256=content_digest(sources),
                    files=output_rows, fileContentSha256=content_digest(output_rows),
                    fileContentHashExcludes=["manifest.json"], inputEvidence=str(evidence / "inputs.json"),
                    sdkRedistributed=False, vendorExecutableRedistributed=False,
                    hardlinksCreated=False, hardwareIo=False, completeStage8=False,
                    capabilities=dict(ordinaryMacroSourceWorkspace=True, libraryExamples=36,
                                      externalSdkZeroAddressVariableCompile=True,
                                      addressVariableEditing=False, globalInitPageScheduling=False,
                                      hmiMacroExecutionVerified=False))
    write_json(package / "manifest.json", manifest)
    write_json(evidence / "package.json",
               dict(schemaVersion=1, package=str(package), codeCommit=None,
                    manifestSha256=file_sha(package / "manifest.json"),
                    fileCountIncludingManifest=len(output_rows) + 1,
                    packageBytes=sum(row["bytes"] for row in output_rows) +
                                 (package / "manifest.json").stat().st_size,
                    sampleCount=sum(row["kind"] == "current-sample" for row in inputs),
                    runtimeCount=sum(row["kind"] == "stage7-runtime" for row in inputs),
                    executableSha256=file_sha(package / "KinSealStudioLite.exe"),
                    sourceContentSha256=content_digest(sources), noLinksVerified=True))
    print(json.dumps({"package": str(package), "evidence": str(evidence),
                      "files": len(output_rows) + 1,
                      "exeSha256": args.expected_sha.lower()}, ensure_ascii=False))


def refresh_content(args) -> None:  #20261114-7 add
    """Function Name: refresh_content
    Description: Add newly supplied samples and refresh this package's guide without replacing runtime or code.
    Input Parameters: args
    Return Value: None; records the previous manifest and exact added inputs.
    Date: 2026.11.14
    """
    worktree = args.worktree.resolve()
    package = args.package.resolve()
    evidence = args.evidence.resolve()
    manifest_path = package / "manifest.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    previous_manifest_sha = file_sha(manifest_path)
    for row in manifest["sourceFiles"]:
        if file_sha(Path(row["inputPath"])) != row["sha256"]:
            raise ValueError("Frozen production source changed: " + row["path"])
    for row in manifest["files"]:
        if file_sha(package / row["path"]) != row["sha256"]:
            raise ValueError("Package content changed outside this updater: " + row["path"])
    new_inputs = []
    for path in tree_files(worktree / "samples"):
        relative = path.relative_to(worktree).as_posix()
        destination = package / relative
        if destination.exists():
            if file_sha(destination) != file_sha(path):
                raise ValueError("Existing packaged sample differs: " + relative)
        else:
            row = {**identity(path, relative), "kind": "current-sample"}
            new_inputs.append(row)
    for row in new_inputs:
        destination = package / row["path"]
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(row["inputPath"], destination)
        if file_sha(destination) != row["sha256"] or destination.stat().st_nlink != 1:
            raise ValueError("New sample copy identity/link mismatch: " + row["path"])
    (package / "使用说明.txt").write_text(README, encoding="utf-8-sig")
    rows = [identity(path, path.relative_to(package).as_posix())
            for path in tree_files(package) if path != manifest_path]
    for row in rows:
        row.pop("inputPath")
    manifest["files"] = rows
    manifest["fileContentSha256"] = content_digest(rows)
    manifest["contentRefreshedAtUtc"] = datetime.now(timezone.utc).isoformat()
    write_json(manifest_path, manifest)
    input_path = evidence / "inputs.json"
    inputs = json.loads(input_path.read_text(encoding="utf-8"))
    inputs.setdefault("contentAmendments", []).append(dict(
        addedInputs=new_inputs, packageBuilder=identity(Path(__file__).resolve()),
        guideChange="SDK工具说明修正为实际ARMCC/ARMLINK/FROMELF，不影响生产EXE",
        previousManifestSha256=previous_manifest_sha,
        newManifestSha256=file_sha(manifest_path)))
    write_json(input_path, inputs)
    record_path = evidence / "package.json"
    record = json.loads(record_path.read_text(encoding="utf-8"))
    record["manifestSha256"] = file_sha(manifest_path)
    record["fileCountIncludingManifest"] = len(rows) + 1
    record["packageBytes"] = sum(row["bytes"] for row in rows) + manifest_path.stat().st_size
    record["sampleCount"] = sum(row["path"].startswith("samples/") for row in rows)
    write_json(record_path, record)
    print(json.dumps({"addedSamples": len(new_inputs), "files": len(rows) + 1,
                      "manifestSha256": record["manifestSha256"]}))


def bind_commit(args) -> None:  #20261114-7 add
    """Function Name: bind_commit
    Description: Bind only an existing real commit whose production source matches the frozen package inputs.
    Input Parameters: args
    Return Value: None; no Git staging, commit or push is performed.
    Date: 2026.11.14
    """
    worktree = args.worktree.resolve()
    package = args.package.resolve()
    evidence = args.evidence.resolve()
    manifest = json.loads((package / "manifest.json").read_text(encoding="utf-8"))
    commit = subprocess.check_output(["git", "-C", str(worktree), "rev-parse",
                                     args.bind_code_commit + "^{commit}"], text=True).strip()
    requests = b"".join((commit + ":" + row["path"] + "\n").encode("utf-8")
                        for row in manifest["sourceFiles"])
    batch = subprocess.run(["git", "-C", str(worktree), "cat-file", "--batch"], input=requests,
                           stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True).stdout
    offset = 0
    for row in manifest["sourceFiles"]:
        header_end = batch.find(b"\n", offset)
        header = batch[offset:header_end].split()
        if header_end < 0 or len(header) != 3 or header[1] != b"blob":
            raise ValueError("Commit lacks a frozen production file: " + row["path"])
        size = int(header[2])
        offset = header_end + 1
        blob = batch[offset:offset + size]
        offset += size + 1
        current = Path(row["inputPath"]).read_bytes()
        if hashlib.sha256(current).hexdigest() != row["sha256"]:
            raise ValueError("Frozen production source changed: " + row["path"])
        text_source = Path(row["path"]).suffix.lower() in (".cpp", ".c", ".h", ".qrc", ".qss", ".svg", ".json", ".txt", ".ui")
        if blob != current and (not text_source or
                                blob.replace(b"\r\n", b"\n") != current.replace(b"\r\n", b"\n")):
            raise ValueError("Commit does not match frozen production source: " + row["path"])
        row["gitBlobSha256"] = hashlib.sha256(blob).hexdigest()
    for row in manifest["files"]:
        path = package / row["path"]
        if file_sha(path) != row["sha256"]:
            raise ValueError("Package content changed before commit binding: " + row["path"])
    manifest["codeCommit"] = commit
    manifest["codeCommitState"] = "verified-real-commit-production-source-match"
    manifest["sourceFilesMatchCommitAfterOnlyGitEolNormalization"] = True
    write_json(package / "manifest.json", manifest)
    record_path = evidence / "package.json"
    record = json.loads(record_path.read_text(encoding="utf-8"))
    record["codeCommit"] = commit
    record["manifestSha256"] = file_sha(package / "manifest.json")
    record["packageBytes"] = sum(row["bytes"] for row in manifest["files"]) + (package / "manifest.json").stat().st_size
    write_json(record_path, record)
    write_json(evidence / "commit-binding.json",
               dict(commit=commit, productionFilesVerified=len(manifest["sourceFiles"]),
                    manifestSha256=record["manifestSha256"], gitMutated=False))
    print(json.dumps({"commit": commit, "manifestSha256": record["manifestSha256"]}))


def main() -> int:  #20261114-7 add
    """Function Name: main
    Description: Select package creation or explicitly requested real-commit binding.
    Input Parameters: Command line arguments.
    Return Value: Zero on success.
    Date: 2026.11.14
    """
    parser = argparse.ArgumentParser()
    parser.add_argument("--worktree", type=Path, required=True)
    parser.add_argument("--runtime", type=Path)
    parser.add_argument("--package", type=Path, required=True)
    parser.add_argument("--evidence", type=Path, required=True)
    parser.add_argument("--expected-sha")
    parser.add_argument("--bind-code-commit")
    parser.add_argument("--refresh-content", action="store_true")
    args = parser.parse_args()
    if args.bind_code_commit:
        bind_commit(args)
    elif args.refresh_content:
        refresh_content(args)
    else:
        if not args.runtime or not args.expected_sha:
            parser.error("Creation requires --runtime and --expected-sha.")
        create_package(args)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

