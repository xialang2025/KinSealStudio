"""Verify the stage-1 model and native-source boundary using a frozen portable EXE.

The default run is offline. --download explicitly writes the validated one-page
sample to the single online matching USB screen. No editor window is opened.
"""
#20261110-5 add: Reproducible verification for the schema9 edition, independent of old release identities.
import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parents[1]


def now():
    """Input: None. Return: The actual UTC observation time."""
    return datetime.now(timezone.utc).isoformat()


def sha(path):
    """Input: A file path. Return: Its SHA-256 identity."""
    return hashlib.sha256(path.read_bytes()).hexdigest()


def read(path):
    """Input: UTF-8 or BOM JSON path. Return: The decoded value."""
    return json.loads(path.read_text(encoding="utf-8-sig"))


def save(path, value):
    """Input: Path and JSON value. Return: None after atomic publication."""
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".new")
    temporary.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    os.replace(temporary, path)


def prepare_samples():
    """Input: None. Return: New schema9 examples derived from retained five-family samples."""
    folder = ROOT / "samples/stage1"
    base = read(ROOT / "samples/stage0/stage0-baseline.ksp")
    base.update(schemaVersion=9, projectName="阶段1工程模型与启动语言验证", projectId="stage1-project",
                activePageId="stage1-main", startupPageId="stage1-main", startupLanguage="English",
                pageOrder=["stage1-main"], configurationNodes=[], references=[], macroIds={})
    base["pages"][0].update(id="stage1-main", kind="normal", width=800, height=480)
    caption = base["pages"][0]["objects"][0]
    caption.update(name="阶段1标题", text="阶段1 · 工程模型验证 / STAGE 1")
    caption["staticText"]["texts"] = {"中文": caption["text"], "English": "STAGE 1 - PROJECT MODEL"}
    base["macros"] = {}  # schema9 uses the canonical map; legacy empty arrays migrate only on old-schema load.
    variants = {"stage1-single-page.ksp": base}
    multiple = copy.deepcopy(base)
    extra = copy.deepcopy(base["pages"][0])
    extra.update(id="stage1-secondary", name="第二页", objects=[copy.deepcopy(caption)])
    extra["objects"][0].update(id="stage1-second-caption", text="第二页内容", name="第二页标题")
    extra["objects"][0]["staticText"]["texts"] = {"中文": "第二页内容", "English": "SECOND PAGE"}
    multiple["pages"].append(extra)
    multiple.update(activePageId=extra["id"], pageOrder=["stage1-main", extra["id"]])
    multiple["references"] = [{"id": "page-bookmark", "ownerKind": "project", "ownerId": base["projectId"],
                               "field": "editor.bookmark", "targetKind": "page", "targetId": "stage1-main"}]
    variants["stage1-two-pages.ksp"] = multiple
    placeholder = copy.deepcopy(base)
    placeholder["configurationNodes"] = [{"id": "stage1-alarm", "kind": "alarm", "name": "未实施报警配置占位",
                                          "ownerPageId": "", "implemented": False, "active": False, "properties": {}}]
    variants["stage1-inactive-config.ksp"] = placeholder
    reordered = copy.deepcopy(base)
    reordered["languages"] = ["English", "中文"]
    for obj in reordered["pages"][0]["objects"]:
        function = obj.get("functionButton", {})
        if function.get("function") == 13:
            function["type"] = 1 - function["type"]
    variants["stage1-reordered-languages.ksp"] = reordered
    for name, value in variants.items():
        path = folder / name
        if path.exists():
            raise RuntimeError(f"Refusing to replace existing sample: {path}")
        save(path, value)
    return list(variants)


def cli(executable, arguments, output, label, expected=0):
    """Input: EXE, exact CLI arguments, result folder, label and expected code. Return: Recorded invocation."""
    env = os.environ.copy()
    env["QT_QPA_PLATFORM"] = "windows"
    env["PATH"] = os.pathsep.join((str(Path(os.environ["SystemRoot"]) / "System32"), os.environ["SystemRoot"]))
    for key in ("QT_PLUGIN_PATH", "QT_QPA_PLATFORM_PLUGIN_PATH", "QML2_IMPORT_PATH", "QML_IMPORT_PATH", "QT_QPA_FONTDIR"):
        env.pop(key, None)
    start = now()
    process = subprocess.run([str(executable), *map(str, arguments)], cwd=executable.parent,
                             env=env, capture_output=True, timeout=150, creationflags=subprocess.CREATE_NO_WINDOW)
    (output / (label + "-stdout.txt")).write_bytes(process.stdout)
    (output / (label + "-stderr.txt")).write_bytes(process.stderr)
    stdout = process.stdout.decode("utf-8-sig", errors="replace").strip()
    try:
        result = json.loads(stdout) if stdout else None
    except ValueError:
        result = {"unparsedStdout": stdout}
    record = dict(label=label, arguments=list(map(str, arguments)), startedAtUtc=start,
                  completedAtUtc=now(), exitCode=process.returncode, expectedExitCode=expected, result=result)
    save(output / (label + ".json"), record)
    if process.returncode != expected:
        raise RuntimeError(f"{label}: expected {expected}, got {process.returncode}: {result}")
    return record


def devices():
    """Input: None. Return: Matching screen devices and their Windows driver identities without USB writes."""
    script = r"""
[Console]::OutputEncoding = [Text.UTF8Encoding]::new($false)
$ErrorActionPreference = 'Stop'
$screenDevices = @(Get-CimInstance Win32_PnPEntity -Filter "DeviceID LIKE '%VID_7365&PID_616C%'" | Select-Object Name,DeviceID,Status,Service)
$screenDrivers = @(Get-CimInstance Win32_PnPSignedDriver -Filter "DeviceID LIKE '%VID_7365&PID_616C%'" | Select-Object DeviceID,DriverVersion,DriverDate,InfName)
[ordered]@{devices=$screenDevices;drivers=$screenDrivers} | ConvertTo-Json -Depth 5
"""
    result = subprocess.run(["powershell.exe", "-NoProfile", "-NonInteractive", "-Command", script],
                            capture_output=True, timeout=60, creationflags=subprocess.CREATE_NO_WINDOW)
    if result.returncode:
        raise RuntimeError(result.stderr.decode("utf-8", errors="replace"))
    return json.loads(result.stdout.decode("utf-8-sig"))


def verify(executable, output, download):
    """Input: Frozen portable EXE, new output folder and hardware opt-in. Return: Complete observation record."""
    if output.exists():
        raise RuntimeError("Each verification must use a new result directory.")
    output.mkdir(parents=True)
    record = dict(startedAtUtc=now(), status="running", usbAccessed=False,
                  physicalTouchConfirmed=False, firmwareVersion=None, steps=[])
    try:
        if not executable.is_file() or sha(executable) != sha(ROOT / "build/KinSealStudioLite.exe"):
            raise RuntimeError("The portable EXE must be byte-identical to this build.")
        frozen = {str(executable): sha(executable)}
        record["executable"] = dict(path=str(executable), sha256=sha(executable), bytes=executable.stat().st_size)
        for path in sorted((ROOT / "samples/stage1").glob("*.ksp")):
            frozen[str(path)] = sha(path)
            local = output / path.name
            local.write_bytes(path.read_bytes())
            label = path.stem
            record["steps"].append(cli(executable, ["--validate-project", local], output, label + "-validate"))
            migrated = output / (label + "-migrated.ksp")
            second = output / (label + "-second.ksp")
            record["steps"].append(cli(executable, ["--migrate-project", local, migrated], output, label + "-migrate"))
            record["steps"].append(cli(executable, ["--migrate-project", migrated, second], output, label + "-repeat"))
            if read(migrated) != read(second):
                raise RuntimeError(f"Migration is not idempotent: {label}")
            caps = cli(executable, ["--project-capabilities", local], output, label + "-capabilities")["result"]
            if label in ("stage1-two-pages", "stage1-inactive-config"):
                if caps["deviceCompile"]["status"] != "blocked" or caps["nativeSource"]["status"] != "blocked":
                    raise RuntimeError("Unsupported topology/configuration was advertised as ready.")
                protected = output / (label + "-protected.sm")
                protected.write_bytes(b"previous-file")
                cli(executable, ["--export-native-project", local, protected], output, label + "-export-refusal", 20)
                if protected.read_bytes() != b"previous-file":
                    raise RuntimeError("Failed export changed the previous file.")
                cli(executable, ["--compile-project", local, output / (label + "-rejected")], output, label + "-compile-refusal", 19)
        single = output / "stage1-single-page.ksp"
        native = output / "stage1-single-page.sm"
        record["steps"].append(cli(executable, ["--compile-project", single, output / "direct"], output, "compile-direct"))
        record["steps"].append(cli(executable, ["--export-native-project", single, native], output, "export-source"))
        record["steps"].append(cli(executable, ["--compile-project", native, output / "roundtrip"], output, "compile-source"))
        comparison = []
        for name in ("ControlPara.dat", "ControlPixel.dat", "PicLenPara.dat", "AddrPara.dat"):
            left, right = output / "direct" / name, output / "roundtrip" / name
            comparison.append(dict(name=name, direct=sha(left), roundtrip=sha(right), equal=sha(left) == sha(right)))
        save(output / "roundtrip-comparison.json", comparison)
        if not all(item["equal"] for item in comparison):
            raise RuntimeError("Native source roundtrip changed compiled control/scan/image data.")
        for index, sample in enumerate((single, output / "stage1-reordered-languages.ksp")):
            destination = output / ("direct" if index == 0 else "reordered")
            if index:
                cli(executable, ["--compile-project", sample, destination], output, "compile-reordered")
            system = (destination / "SysPara.dat").read_bytes()
            actual = int.from_bytes(system[2172:2174], "little")
            if actual != (1 if index == 0 else 0):
                raise RuntimeError("Startup language followed the editor language or stale list position.")
        record["startupLanguageChecks"] = dict(editorLanguage="中文", startupLanguage="English", initialIndex=1, reorderedIndex=0)
        protected = output / "protected-native.sm"
        protected.write_bytes(native.read_bytes())
        original = sha(protected)
        cli(executable, ["--migrate-project", protected, protected], output, "reject-native-overwrite", 21)
        if sha(protected) != original:
            raise RuntimeError("Migration modified its original native source.")
        cli(executable, ["--inspect-native-source", protected, protected], output, "reject-inspect-source-overwrite", 22)
        if sha(protected) != original:
            raise RuntimeError("Structure inspection modified its original native source.")
        cli(executable, ["--inspect-native-source", protected, output / "source-fields.json"], output, "inspect-single-source")
        invalid = read(single)
        invalid["schemaVersion"] = 10
        save(output / "future.ksp", invalid)
        cli(executable, ["--validate-project", output / "future.ksp"], output, "reject-future", 7)
        invalid = read(single)
        invalid["unknownActiveFeature"] = True
        save(output / "unknown.ksp", invalid)
        cli(executable, ["--validate-project", output / "unknown.ksp"], output, "reject-unknown", 7)
        save(output / "input-identities.json", frozen)
        if any(sha(Path(path)) != value for path, value in frozen.items()):
            raise RuntimeError("Inputs changed after offline verification.")
        if download:
            current = devices()
            current["observedAtUtc"] = now()
            save(output / "pre-transfer-devices.json", current)
            online = [item for item in current["devices"] if item["Status"] == "OK"]
            if len(online) != 1:
                raise RuntimeError(f"Expected one online screen, observed {len(online)}.")
            record["usbDevice"] = online[0]
            record["usbAccessed"] = True
            result = cli(executable, ["--download-project", single], output, "usb-download")
            record["steps"].append(result)
            if not (result.get("result") or {}).get("ok"):
                raise RuntimeError("USB download did not report accepted segments and commit.")
            record["transportAccepted"] = True
        record["status"] = "passed_with_physical_observation_pending"
    except Exception as error:
        record.update(status="failed", error=str(error))
    record["completedAtUtc"] = now()
    save(output / "run.json", record)
    return record


def main():
    """Input: CLI options. Return: Nonzero on any identity, model, source or USB failure."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--prepare-samples", action="store_true")
    parser.add_argument("--executable", type=Path, default=ROOT / "dist/KinSealStudioLite-Stage1-0.8.1/KinSealStudioLite.exe") #20261110-6 change
    parser.add_argument("--output", type=Path)
    parser.add_argument("--download", action="store_true")
    args = parser.parse_args()
    if args.prepare_samples:
        print(json.dumps(prepare_samples()))
        return 0
    if args.output is None:
        parser.error("--output must select a new verification directory")
    result = verify(args.executable.resolve(), args.output.resolve(), args.download)
    print(json.dumps({key: result.get(key) for key in ("status", "error", "transportAccepted")}, ensure_ascii=False))
    return result["status"] == "failed"


if __name__ == "__main__":
    sys.exit(main())
