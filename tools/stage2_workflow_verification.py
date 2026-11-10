"""Verify the stage-2 portable editor's source and device boundary.

Offline checks run by default. --download writes only the supported one-page
five-control sample, after immutable input and unique online device checks.
"""
#20261110-7 add: Reuse the audited hidden CLI and USB identity checks without opening user windows.
import argparse
import copy
from pathlib import Path
import sys
from stage1_model_verification import ROOT, cli, devices, now, read, save, sha


def prepare_samples():
    """Input: None. Return: Explicit single-page, multi-page and window workflow samples."""
    base = read(ROOT / "samples/stage1/stage1-single-page.ksp")
    base.update(projectName="阶段2工程工作流与下载验证", projectId="stage2-project")
    base["pages"][0]["objects"][0]["name"] = "阶段2标题"
    base["pages"][0]["objects"][0]["text"] = "阶段2 · 工程工作流 / STAGE 2"
    base["pages"][0]["objects"][0]["staticText"]["texts"] = {
        "中文": "阶段2 · 工程工作流 / STAGE 2", "English": "STAGE 2 - PROJECT WORKFLOW"}
    multiple = copy.deepcopy(base)
    second = copy.deepcopy(base["pages"][0])
    second.update(id="stage2-second", name="第二页", objects=[copy.deepcopy(second["objects"][0])])
    second["objects"][0].update(id="stage2-second-text", name="第二页文字")
    second["objects"][0]["staticText"]["texts"] = {"中文": "第二页", "English": "SECOND PAGE"}
    popup = dict(id="stage2-window", kind="window", name="窗口", width=240, height=180,
                 objects=[], extensions={})
    multiple["pages"].extend([second, popup])
    multiple["pageOrder"].extend([second["id"], popup["id"]])
    samples = {"stage2-single-page.ksp": base, "stage2-two-pages-one-window.ksp": multiple}
    for name, project in samples.items():
        save(ROOT / "samples/stage2" / name, project)
    return samples


def verify(executable, output, download):
    """Input: Frozen EXE, a new evidence directory and transfer opt-in. Return: Observed results."""
    if output.exists():
        raise RuntimeError("Each run requires a new output directory.")
    output.mkdir(parents=True)
    record = dict(startedAtUtc=now(), status="running", steps=[], usbAccessed=False,
                  physicalTouchConfirmed=False, firmwareVersion=None,
                  scope="one-page-five-control-transfer; multi-page/window editor persistence only")
    try:
        if sha(executable) != sha(ROOT / "build/KinSealStudioLite.exe"):
            raise RuntimeError("Portable EXE differs from the frozen build.")
        frozen = {str(executable): sha(executable)}
        record["executable"] = dict(path=str(executable), sha256=sha(executable), bytes=executable.stat().st_size)
        for source in sorted((ROOT / "samples/stage2").glob("*.ksp")):
            packaged = executable.parent / "samples/stage2" / source.name
            if sha(source) != sha(packaged):
                raise RuntimeError("Packaged sample differs: " + source.name)
            frozen[str(packaged)] = sha(packaged)
            local = output / source.name
            local.write_bytes(packaged.read_bytes())
            record["steps"].append(cli(executable, ["--validate-project", local], output, source.stem + "-validate"))
            migrated = output / (source.stem + "-migrated.ksp")
            record["steps"].append(cli(executable, ["--migrate-project", local, migrated], output, source.stem + "-reopen"))
            if "window" in source.stem:
                loaded = read(migrated)
                if len(loaded["pages"]) != 3 or loaded["pages"][2]["kind"] != "window":
                    raise RuntimeError("Window topology was lost on save/reopen.")
                caps = cli(executable, ["--project-capabilities", local], output, "window-capabilities")["result"]
                if caps["deviceCompile"]["status"] != "blocked":
                    raise RuntimeError("Window topology incorrectly advertised as downloadable.")
                protected = output / "protected-window.sm"
                protected.write_bytes(b"previous-source")
                record["steps"].append(cli(executable, ["--export-native-project", local, protected], output, "window-source-refusal", 20))
                if protected.read_bytes() != b"previous-source":
                    raise RuntimeError("Rejected export changed the destination.")
                record["steps"].append(cli(executable, ["--compile-project", local, output / "window-rejected"], output, "window-compile-refusal", 19))
        single = output / "stage2-single-page.ksp"
        native = output / "stage2-single-page.sm"
        restored = output / "stage2-single-page-restored.sm"
        record["steps"].append(cli(executable, ["--export-native-project", single, native], output, "export-source"))
        record["steps"].append(cli(executable, ["--export-native-project", native, restored], output, "restore-source"))
        if sha(native) != sha(restored):
            raise RuntimeError("Unedited native source was not preserved.")
        for label, source in (("direct", single), ("roundtrip", native)):
            record["steps"].append(cli(executable, ["--compile-project", source, output / label], output, "compile-" + label))
        comparison = []
        for name in ("ControlPara.dat", "ControlPixel.dat", "PicLenPara.dat", "AddrPara.dat"):
            left, right = output / "direct" / name, output / "roundtrip" / name
            comparison.append(dict(name=name, direct=sha(left), roundtrip=sha(right), equal=sha(left) == sha(right)))
        if not all(item["equal"] for item in comparison):
            raise RuntimeError("Native source roundtrip changed device output.")
        save(output / "roundtrip-comparison.json", comparison)
        frozen[str(single)] = sha(single)
        frozen[str(native)] = sha(native)
        save(output / "input-identities.json", frozen)
        if any(sha(Path(path)) != expected for path, expected in frozen.items()):
            raise RuntimeError("A frozen input changed during validation.")
        if download:
            observed = devices()
            observed["observedAtUtc"] = now()
            save(output / "pre-transfer-devices.json", observed)
            online = [device for device in observed["devices"] if device["Status"] == "OK"]
            if len(online) != 1:
                raise RuntimeError("Expected exactly one online matching 7365:616C device.")
            record["usbDevice"] = online[0]
            record["usbAccessed"] = True
            result = cli(executable, ["--download-project", single], output, "usb-download")
            record["steps"].append(result)
            if not (result.get("result") or {}).get("ok"):
                raise RuntimeError("Transport did not confirm all segments and commit.")
            record["transportAccepted"] = True
        record["status"] = "passed_with_physical_observation_pending"
    except Exception as error:
        record.update(status="failed", error=str(error))
    record["completedAtUtc"] = now()
    save(output / "run.json", record)
    return record


def main():
    """Input: Explicit output path and optional transfer flag. Return: Failure exit status."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--prepare", action="store_true")
    parser.add_argument("--executable", type=Path, default=ROOT / "dist/KinSealStudioLite-Stage2-0.9.0/KinSealStudioLite.exe")
    parser.add_argument("--output", type=Path)
    parser.add_argument("--download", action="store_true")
    args = parser.parse_args()
    if args.prepare:
        print(list(prepare_samples()))
        return 0
    if args.output is None:
        parser.error("--output is required for verification")
    result = verify(args.executable.resolve(), args.output.resolve(), args.download)
    print({key: result.get(key) for key in ("status", "error", "transportAccepted")})
    return result["status"] == "failed"


if __name__ == "__main__":
    sys.exit(main())
