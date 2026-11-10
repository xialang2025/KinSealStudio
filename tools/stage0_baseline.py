"""Reproduce the stage-0 identities, five-family source roundtrip and USB check.

The default run is offline. Hardware transfer requires --download explicitly.
Only CLI branches that return before opening an editor are launched.
"""

#20261110-4 add: Keep current observations separate from historical acceptance.
import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
from datetime import datetime, timezone


ROOT = Path(__file__).resolve().parents[1]
WORKSPACE = ROOT.parent
NATIVE = Path("G:/01_Professional/触摸屏/金玺/KinSealStudio_Setup_V5.1/Installed")
PACKAGE = ROOT / "dist/KinSealStudioLite-FunctionButton"
EXPECTED_NATIVE = "305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c"
EXPECTED_HELP = "f87f031d82b199d8c66de7240ee07e1553f136358ea842e5c34e0f10e80171a1"
EXPECTED_CSV = "27f423c4b35d387b243d0e4ca69485ff2a892b41e8b3cd81e19cfc81b532b87c"


def utc_now():
    """Input: None. Return: An actual UTC observation timestamp."""
    return datetime.now(timezone.utc).isoformat()


def digest(path):
    """Input: file path. Return: Streaming SHA-256 without loading large CHM files."""
    result = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            result.update(block)
    return result.hexdigest()


def load_json(path):
    """Input: UTF-8/BOM JSON path. Return: Parsed source without rewriting it."""
    return json.loads(path.read_text(encoding="utf-8-sig"))


def save_json(path, value):
    """Input: output path and value. Return: None; publish complete JSON atomically."""
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + ".new")
    temporary.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    os.replace(temporary, path)


def identity(path, role, required=True):
    """Input: path, role, requirement. Return: Identity or explicit missing status."""
    absolute = path.absolute()
    record = {"path": str(absolute), "role": role, "required": required, "exists": path.is_file()}
    if record["exists"]:
        record.update(bytes=path.stat().st_size, sha256=digest(path))
    return record


def make_sample(path):
    """Input: new output path. Return: Five-family model built from retained samples."""
    if path.exists():
        raise ValueError(f"Refusing to replace an existing sample: {path}")
    project = load_json(ROOT / "samples/lamp/指示灯实机联调.ksp")
    functions = load_json(ROOT / "samples/function-button/功能按钮实机联调.ksp")
    captions = load_json(ROOT / "samples/static-text/外观与样式验证.ksp")
    project["schemaVersion"] = 8
    project["projectName"] = "阶段0五类基线联调"
    project["languages"] = ["中文", "English"]
    project["activeLanguage"] = "中文"
    project["macros"] = []
    objects = project["pages"][0]["objects"]
    prototype = functions["pages"][0]["objects"][0]
    for index, (function, subtype, text) in enumerate(((1, 0, "系统设置"), (13, 0, "中文"), (13, 1, "English"))):
        obj = copy.deepcopy(prototype)
        obj["id"] = f"stage0-function-{index}"
        obj["name"] = f"阶段0功能{index + 1}"
        obj["text"] = text
        obj["functionButton"].update(function=function, type=subtype, pressedEffect=True)
        for state in obj["bitButton"]["states"]:
            state["text"]["texts"] = {language: text for language in project["languages"]}
        objects.append(obj)
    for index, obj in enumerate(objects):
        obj.update(x=20 + index % 4 * 195, y=95 + index // 4 * 115, width=180, height=70, layer=index + 1)
        for state in obj.get("bitButton", {}).get("states", []):
            values = state["text"].setdefault("texts", {})
            values.setdefault("English", values.get("中文", obj.get("text", "")))
    caption = copy.deepcopy(captions["pages"][0]["objects"][0])
    caption.update(id="stage0-caption", name="阶段0基线标题", text="阶段0 · 五类控件基线 / STAGE 0", x=20, y=20, width=760, height=45, layer=0)
    caption["staticText"].update(texts={language: caption["text"] for language in project["languages"]}, fontSize=24, opacity=255, style=0, underline=False, bold=False)
    objects.insert(0, caption)
    save_json(path, project)
    return project


def collect_manifest(project):
    """Input: current sample path. Return: Used input identities and historical refs."""
    records = []
    used = [
        (WORKSPACE / "AGENTS.md", "workspace-rules"),
        (ROOT / "CMakeLists.txt", "build-definition"),
        (ROOT / "Build.ps1", "build-recipe"),
        (ROOT / "README.md", "current-capability-statement"),
        (Path(__file__), "baseline-recipe"),
        (NATIVE / "KinSealStudio5.1.exe", "native-original"),
        (NATIVE / "KinSealStudio.chm", "native-help"),
        (ROOT / "build/KinSealStudioLite.exe", "build-executable"),
        (PACKAGE / "KinSealStudioLite.exe", "tested-portable-executable"),
        (project, "current-run-input"),
        (ROOT / "samples/lamp/指示灯实机联调.ksp", "sample-parent"),
        (ROOT / "samples/function-button/功能按钮实机联调.ksp", "sample-parent"),
        (ROOT / "samples/static-text/外观与样式验证.ksp", "sample-parent"),
        (ROOT / "docs/function-button-evidence/delivery-result.json", "historical-acceptance-only"),
        (ROOT / "docs/function-button-evidence/usb-function-integration.json", "historical-transfer-only"),
        (ROOT / "docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md", "known-protocol"),
    ]
    for path in sorted((ROOT / "src").iterdir()):
        if path.suffix in (".h", ".cpp"):
            used.append((path, "captured-source-not-a-new-build-proof"))
    for path in sorted((ROOT / "resources").glob("native*.sm")):
        used.append((path, "native-source-seed"))
    plans = ROOT / "design_plan"
    for name in ("00_复刻执行总入口.md", "01_阶段0_基线范围与证据管理.md", "A01_公共属性与事务规范.md", "A02_全局入口与工程配置规范.md", "A03_基础图形与表格规范.md", "A04_31控件实施索引.md", "A05_源格式与设备编译契约.md", "A06_运行与发布验收用例.md", "复刻待验线索台账.csv", "复刻待验核销记录.md", "分阶段文档证据/V51后台入口采集_20261110.json", "分阶段文档证据/原厂帮助正文_20261110.json"):
        used.append((plans / name, "stage0-specification-or-evidence"))
    used.extend((path, "control-manual") for path in sorted(plans.glob("*功能与交互复刻手册.md")))
    for path, role in used:
        records.append(identity(path, role))
    historical = []
    protocol = (ROOT / "docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md").read_text(encoding="utf-8-sig")
    for relative in sorted(set(re.findall(r"\]\(([^)]+\.scratch[^)]*)\)", protocol))):
        path = (ROOT / "docs" / relative).resolve()
        historical.append({"documentReference": relative, "resolvedPath": str(path), "exists": path.exists(), "requiredForThisRun": False})
    return {"schemaVersion": 1, "capturedAtUtc": utc_now(), "targetModel": "AMT070W124-L800X480R", "resolution": [800, 480], "sourceBuildClaim": "Current source identities only; no rebuild is claimed by this capture.", "inputs": records, "historicalScratchReferences": historical}


def verify_manifest(manifest):
    """Input: manifest. Return: Missing/changed input diagnostics; no mutation."""
    mismatches = []
    for item in manifest["inputs"]:
        path = Path(item["path"])
        if not path.is_file():
            if item["required"]:
                mismatches.append({"path": str(path), "reason": "missing-required-input"})
        elif item.get("sha256") != digest(path):
            mismatches.append({"path": str(path), "reason": "input-identity-changed"})
    return mismatches


def observe_resources():
    """Input: None. Return: Read-only Windows OS/driver/port observations."""
    script = r"""
[Console]::OutputEncoding = [Text.UTF8Encoding]::new($false)
$ErrorActionPreference = 'Stop'
$devices = @(Get-CimInstance Win32_PnPEntity -Filter "DeviceID LIKE '%VID_7365&PID_616C%'" | Select-Object Name,DeviceID,Status,Service)
$drivers = @(Get-CimInstance Win32_PnPSignedDriver -Filter "DeviceID LIKE '%VID_7365&PID_616C%'" | Select-Object DeviceID,DriverVersion,DriverDate,InfName,Manufacturer)
$ports = @(Get-CimInstance Win32_SerialPort | Select-Object DeviceID,Name,Status)
$os = Get-CimInstance Win32_OperatingSystem | Select-Object Caption,Version,BuildNumber,OSArchitecture
[ordered]@{devices=$devices; drivers=$drivers; serialPorts=$ports; operatingSystem=$os; culture=[Globalization.CultureInfo]::CurrentCulture.Name} | ConvertTo-Json -Depth 6
"""
    completed = subprocess.run(["powershell.exe", "-NoProfile", "-NonInteractive", "-Command", script], capture_output=True, timeout=60, creationflags=subprocess.CREATE_NO_WINDOW)
    if completed.returncode:
        raise RuntimeError(completed.stderr.decode("utf-8", errors="replace"))
    observations = json.loads(completed.stdout.decode("utf-8-sig"))
    observations.update(observedAtUtc=utc_now(), firmwareIdentity=None, firmwareQueryPerformed=False, hardwareModel="expected AMT target; USB VID/PID alone does not identify model", physicalTouchObservation="not_performed", plcOrSimulatorConnection="not_established; serial-port enumeration alone is not communication proof", uiOperation="background CLI only; no input desktop or foreground activation")
    return observations


def run_cli(executable, arguments, output, label):
    """Input: exact CLI, arguments, folder and label. Return: This invocation record."""
    environment = os.environ.copy()
    environment["QT_QPA_PLATFORM"] = "windows"
    environment["PATH"] = os.pathsep.join((str(Path(os.environ["SystemRoot"]) / "System32"), os.environ["SystemRoot"]))
    started = utc_now()
    process = subprocess.run([str(executable), *map(str, arguments)], cwd=executable.parent, env=environment, capture_output=True, timeout=150, creationflags=subprocess.CREATE_NO_WINDOW)
    (output / f"{label}-stdout.txt").write_bytes(process.stdout)
    (output / f"{label}-stderr.txt").write_bytes(process.stderr)
    text = process.stdout.decode("utf-8-sig", errors="replace").strip()
    try:
        result = json.loads(text) if text else None
    except ValueError:
        result = {"unparsedStdout": text}
    record = {"label": label, "arguments": list(map(str, arguments)), "startedAtUtc": started, "completedAtUtc": utc_now(), "exitCode": process.returncode, "result": result}
    save_json(output / f"{label}.json", record)
    return record


def run_baseline(output, project, download):
    """Input: fresh output folder, sample and hardware opt-in. Return: Current run."""
    if output.exists():
        raise ValueError("Use a new output directory to preserve all earlier attempts.")
    output.mkdir(parents=True)
    record = {"schemaVersion": 1, "taskIds": ["S00-01", "S00-06"], "startedAtUtc": utc_now(), "status": "started", "shutdownRequested": False, "screenDisplayAndTouchConfirmed": False, "newBuildPerformed": False, "steps": []}
    try:
        if not project.exists():
            make_sample(project)
        manifest = collect_manifest(project)
        save_json(output / "input-manifest.json", manifest)
        missing = [item["path"] for item in manifest["inputs"] if item["required"] and not item["exists"]]
        if missing:
            raise ValueError(f"Missing required inputs: {missing}")
        for path, expected in ((NATIVE / "KinSealStudio5.1.exe", EXPECTED_NATIVE), (NATIVE / "KinSealStudio.chm", EXPECTED_HELP), (ROOT / "design_plan/复刻待验线索台账.csv", EXPECTED_CSV)):
            if digest(path) != expected:
                raise ValueError(f"Frozen native/candidate input differs: {path}")
        executable = PACKAGE / "KinSealStudioLite.exe"
        historical = load_json(ROOT / "docs/function-button-evidence/delivery-result.json")
        executable_hash = digest(executable)
        if executable_hash != historical["executableSha256"].lower() or executable_hash != digest(ROOT / "build/KinSealStudioLite.exe"):
            raise ValueError("Portable, build and historical release identities differ; revalidation is required.")
        record.update(executable=identity(executable, "actual-cli-executable"), project=identity(project, "actual-run-input"), historicalValidationIsNewRun=False)
        resources = observe_resources()
        save_json(output / "device-resources.json", resources)
        # Freeze an ASCII-path byte-identical CLI input to avoid MinGW argv encoding ambiguity.
        input_copy = output / "baseline.ksp"
        input_copy.write_bytes(project.read_bytes())
        for label, arguments in (("validate", ["--validate-project", input_copy]), ("compile-direct", ["--compile-project", input_copy, output / "direct"]), ("export-source", ["--export-native-project", input_copy, output / "baseline.sm"]), ("compile-source", ["--compile-project", output / "baseline.sm", output / "roundtrip"])):
            step = run_cli(executable, arguments, output, label)
            record["steps"].append(step)
            if step["exitCode"] != 0 or (step["result"] and step["result"].get("ok") is False):
                raise RuntimeError(f"Baseline CLI failed: {label}")
        comparison = []
        for name in ("ControlPara.dat", "ControlPixel.dat", "PicLenPara.dat", "AddrPara.dat"):
            left = identity(output / "direct" / name, "direct-compiled-data")
            right = identity(output / "roundtrip" / name, "source-roundtrip-compiled-data")
            match = left.get("sha256") is not None and left.get("sha256") == right.get("sha256")
            comparison.append({"name": name, "direct": left, "roundtrip": right, "equal": match})
        record["sourceRoundtripFiles"] = comparison
        save_json(output / "roundtrip-comparison.json", comparison)
        if not all(item["equal"] for item in comparison):
            raise RuntimeError("The mixed five-family source roundtrip changed compiled data.")
        changes = verify_manifest(manifest)
        record["preTransferIdentityMismatches"] = changes
        if changes:
            raise RuntimeError("Inputs changed during offline verification; transfer stopped.")
        if download:
            current_resources = observe_resources()
            save_json(output / "pre-transfer-device-resources.json", current_resources)
            devices = [device for device in current_resources["devices"] if device["Status"] == "OK"]
            if len(devices) != 1:
                raise RuntimeError(f"Expected exactly one online screen, observed {len(devices)}.")
            step = run_cli(executable, ["--download-project", input_copy], output, "usb-download")
            record["steps"].append(step)
            record["usbDevice"] = devices[0]
            if step["exitCode"] != 0 or not (step["result"] or {}).get("ok"):
                raise RuntimeError("The real USB download did not report success.")
            record["transportAccepted"] = True
        else:
            record["transportAccepted"] = None
        record["status"] = "passed_with_external_observation_gaps"
    except Exception as error:
        record["status"] = "failed"
        record["error"] = str(error)
    record["completedAtUtc"] = utc_now()
    save_json(output / "run.json", record)
    return record


def main():
    """Input: CLI options. Return: Nonzero on failed identities, roundtrip or USB."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--project", type=Path, default=ROOT / "samples/stage0/stage0-baseline.ksp")
    parser.add_argument("--download", action="store_true", help="Explicitly perform a real USB write after offline verification.")
    parser.add_argument("--verify-manifest", type=Path, help="Read-only identity comparison; never starts an application or USB.")
    args = parser.parse_args()
    if args.verify_manifest:
        mismatches = verify_manifest(load_json(args.verify_manifest))
        print(json.dumps({"mismatches": mismatches}, ensure_ascii=False))
        return bool(mismatches)
    if args.output is None:
        parser.error("--output must select a new run directory")
    result = run_baseline(args.output.resolve(), args.project.resolve(), args.download)
    print(json.dumps({"status": result["status"], "output": str(args.output.resolve()), "error": result.get("error"), "transportAccepted": result.get("transportAccepted")}, ensure_ascii=False))
    return result["status"] == "failed"


if __name__ == "__main__":
    sys.exit(main())
