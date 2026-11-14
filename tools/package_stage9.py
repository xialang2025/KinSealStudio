"""Freeze a Stage9 portable package with explicit serial runtime and real-commit binding."""
#20261114-8 add: Reuse reviewed identity/binding primitives; copy no SDK or vendor executable.
import argparse
from datetime import datetime, timezone
import json
from pathlib import Path
import shutil
import package_stage8 as previous

GUIDE = """KinSealStudioLite 阶段9 PLC通信验证包

启动 KinSealStudioLite.exe，从“在线模拟”入口打开“PLC通信与协议能力”。
PC串口默认未选择，不会自动连接。用户需选择端口、匹配波特率/校验/停止位、
填写站号及0起始PDU地址，再点击“执行一次”。8数据位，操作01/02/03/04/05/06/15/16。
写值用十进制逗号分隔；多写数量和值数须相同。停止/关闭将请求取消并等待清理。
不自动重试写操作；“可能已到达，未确认”须先核实从站状态再手工决定下一步。
工程画面联动、自动扫描调度和原厂地址族到PDU映射尚未开放。
串口工具参数是本工具配置，并非从原厂INI未知枚举推定的默认值。

“协议能力”页列139配置，区分已核地址族、受限设备编译、PC工程映射及HMI联动。
目录/模块文件存在不保证通信。当前设备编译仍仅原有默认AMT单连接COM1站1/node1；
LT779及其它协议组合未新增支持。原厂DLL完整缓冲区ABI仍未恢复，不动态加载DLL。

命令行 --plc-capabilities [型号] 将嵌入的能力快照输出为JSON，无端点I/O或文件写入。
普通宏编辑/外部SDK编译和前期离线模拟保持已有范围；SDK与许可证不附带。
本批仅受控RTU及PC界面验证，未进行真实PLC收发、HMI到PLC联动或USB下载。
设备下载成功、实际显示、触控和PLC读写分别验收；后续新增控件仍需实物下载。
manifest.json记录源/资源/EXE/运行库身份；不包含原厂EXE或到其目录的链接。
"""


def create(args):
    """Input: new Z-drive package/evidence paths. Return: None after byte-verified ordinary copies."""
    work = args.worktree.resolve()
    package = args.package.resolve()
    evidence = args.evidence.resolve()
    runtime = args.runtime.resolve()
    executable = work / "build/KinSealStudioLite.exe"
    if package.exists() or evidence.exists() or package.drive.lower() != "z:" or evidence.drive.lower() != "z:":
        raise ValueError("Require new package/evidence on Z")
    if previous.file_sha(executable) != args.expected_sha:
        raise ValueError("Build EXE changed")
    old = json.loads((runtime / "manifest.json").read_text(encoding="utf-8"))
    old_files = {row["path"]: row for row in old["files"]}
    inputs = []
    for path in previous.tree_files(runtime):
        relative = path.relative_to(runtime).as_posix()
        if path.suffix.lower() == ".dll" or relative in ("LZO-MIT-LICENSE.txt", "QR-MIT-LICENSE.txt"):
            row = previous.identity(path, relative)
            if row["sha256"] != old_files[relative]["sha256"]:
                raise ValueError("Runtime differs: " + relative)
            inputs.append({**row, "kind": "previous-runtime"})
    serial = args.serial_dll.resolve()
    if serial.name != "Qt6SerialPort.dll" or any(row["path"] == serial.name for row in inputs):
        raise ValueError("Unexpected or duplicate serial DLL")
    inputs.append({**previous.identity(serial), "kind": "new-qt-serial-runtime"})
    for path in previous.tree_files(work / "samples"):
        inputs.append({**previous.identity(path, path.relative_to(work).as_posix()), "kind": "current-sample"})
    inputs.append({**previous.identity(work / "tools/stage6_media_bundle.py", "tools/stage6_media_bundle.py"), "kind": "offline-tool"})
    inputs.append({**previous.identity(executable), "kind": "production-executable"})
    sources = previous.source_identity(work)
    evidence.mkdir(parents=True)
    previous.write_json(evidence / "inputs.json", dict(sourceFiles=sources, inputs=inputs,
        sourceContentSha256=previous.content_digest(sources), builder=previous.identity(Path(__file__)),
        identityAndBindingHelper=previous.identity(Path(previous.__file__).resolve()),
        previousManifestSha256=previous.file_sha(runtime / "manifest.json")))
    package.mkdir()
    for row in inputs:
        target = package / row["path"]
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(row["inputPath"], target)
        if previous.file_sha(target) != row["sha256"] or target.stat().st_nlink != 1:
            raise ValueError("Copy identity/link mismatch")
    (package / "使用说明.txt").write_text(GUIDE, encoding="utf-8-sig")
    files = [previous.identity(path, path.relative_to(package).as_posix()) for path in previous.tree_files(package)]
    for row in files:
        row.pop("inputPath")
    manifest = dict(schemaVersion=1, stage=9, createdAtUtc=datetime.now(timezone.utc).isoformat(),
        name=package.name, sourceFiles=sources, sourceContentSha256=previous.content_digest(sources),
        files=files, fileContentSha256=previous.content_digest(files), executableSha256=args.expected_sha,
        codeCommit=None, codeCommitState="awaiting-parent-verified-commit", completeStage9=False,
        hardwareIo=False, hmiExecutionConfirmed=False, vendorExecutableRedistributed=False, sdkRedistributed=False,
        capabilities=dict(rawPcRtuFunctions=[1,2,3,4,5,6,15,16], automaticProjectMapping=False,
                          fullNativeDllAbi=False, physicalPlcVerified=False, hmiPlcVerified=False))
    previous.write_json(package / "manifest.json", manifest)
    previous.write_json(evidence / "package.json", dict(package=str(package), codeCommit=None,
        fileCountIncludingManifest=len(files)+1, packageBytes=sum(row["bytes"] for row in files)+(package/"manifest.json").stat().st_size,
        executableSha256=args.expected_sha, sourceContentSha256=manifest["sourceContentSha256"],
        manifestSha256=previous.file_sha(package/"manifest.json")))
    print(json.dumps(dict(package=str(package), files=len(files)+1, productionFiles=len(sources))))


def main():
    """Input: explicit build/runtime/identity arguments. Return: zero after creation or existing-commit binding."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--worktree", type=Path, required=True)
    parser.add_argument("--package", type=Path, required=True)
    parser.add_argument("--evidence", type=Path, required=True)
    parser.add_argument("--runtime", type=Path)
    parser.add_argument("--serial-dll", type=Path)
    parser.add_argument("--expected-sha")
    parser.add_argument("--bind-code-commit")
    args = parser.parse_args()
    if args.bind_code_commit:
        previous.bind_commit(args)
    else:
        if not args.runtime or not args.serial_dll or not args.expected_sha:
            parser.error("Creation requires runtime, serial DLL and expected SHA")
        create(args)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
