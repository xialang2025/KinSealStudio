"""Verify the stage-3 portable editor's source and device boundary.

Offline checks run by default. --download writes only the supported one-page
five-control sample, after immutable input and unique online device checks.
"""
#20261110-8 add: Reuse the audited hidden CLI and USB identity checks without opening user windows.
import argparse
import copy
from pathlib import Path
import sys
from stage1_model_verification import ROOT, cli, devices, now, read, save, sha


def prepare_samples():
    """Input: None. Return: Five-family odd-width image and editor-only multi-page samples."""
    import base64
    import struct
    import zlib
    base = read(ROOT / "samples/stage2/stage2-single-page.ksp")
    base.update(projectName="阶段3画布资源与下载验证", projectId="stage3-project")
    originals = base["pages"][0]["objects"]
    objects = [copy.deepcopy(originals[i]) for i in (0, 4, 5, 1, 9)]
    geometries = [(19, 19, 761, 45), (60, 120, 121, 65), (250, 120, 123, 65),
                  (450, 120, 125, 65), (640, 120, 127, 65)]
    for index, (obj, (x, y, width, height)) in enumerate(zip(objects, geometries)):
        obj.update(id="stage3-object-"+str(index), x=x, y=y, width=width, height=height, layer=index)
    caption = objects[0]
    caption.update(name="阶段3标题", text="阶段3 · 画布与资源 / STAGE 3")
    caption["staticText"]["texts"] = {"中文": caption["text"], "English": "STAGE 3 - CANVAS AND RESOURCES"}
    def chunk(tag, data):
        return struct.pack(">I",len(data))+tag+data+struct.pack(">I",zlib.crc32(tag+data)&0xffffffff)
    pixels = b"".join(b"\x00"+bytes((40,160,220,140))*17 for _ in range(13))
    png = b"\x89PNG\r\n\x1a\n"+chunk(b"IHDR",struct.pack(">IIBBBBB",17,13,8,6,0,0,0))+chunk(b"IDAT",zlib.compress(pixels))+chunk(b"IEND",b"")
    encoded = base64.b64encode(png).decode("ascii")
    state = objects[1]["bitButton"]["states"][0]
    state.update(picture="stage3-alpha", picturePng=encoded, useImage=True)
    base["pictures"] = {"stage3-alpha": encoded}
    base["pages"][0]["objects"] = objects
    multiple = copy.deepcopy(base)
    second = copy.deepcopy(base["pages"][0])
    second.update(id="stage3-second", name="第二页", objects=[copy.deepcopy(objects[0])])
    second["objects"][0].update(id="stage3-second-text", name="第二页文字")
    second["objects"][0]["staticText"]["texts"] = {"中文":"第二页", "English":"SECOND PAGE"}
    popup = dict(id="stage3-window",kind="window",name="窗口",width=240,height=180,objects=[],extensions={})
    multiple["pages"].extend([second,popup])
    multiple["pageOrder"].extend([second["id"],popup["id"]])
    compatible = copy.deepcopy(base)
    compatible["projectName"] = "阶段3原生图库源往返验证"
    compatible["pictures"] = {}
    compatible["pages"][0]["objects"][1]["bitButton"]["states"][0] = copy.deepcopy(originals[4]["bitButton"]["states"][0])
    samples = {"stage3-single-page.ksp":base,"stage3-two-pages-one-window.ksp":multiple,
        "stage3-native-source-compatible.ksp":compatible}
    for name, project in samples.items():
        save(ROOT / "samples/stage3" / name,project)
    return samples


def verify(executable, output, download):
    """Input: Frozen EXE, a new evidence directory and transfer opt-in. Return: Observed results."""
    if output.exists():
        raise RuntimeError("Each run requires a new output directory.")
    output.mkdir(parents=True)
    record = dict(startedAtUtc=now(), status="running", steps=[], usbAccessed=False,
                  physicalTouchConfirmed=False, firmwareVersion=None,
                  scope="one-page-five-control transfer including embedded PNG; native KVG-source roundtrip separate; multi-page/window editor persistence only")
    try:
        if sha(executable) != sha(ROOT / "build/KinSealStudioLite.exe"):
            raise RuntimeError("Portable EXE differs from the frozen build.")
        frozen = {str(executable): sha(executable)}
        record["executable"] = dict(path=str(executable), sha256=sha(executable), bytes=executable.stat().st_size)
        for source in sorted((ROOT / "samples/stage3").glob("*.ksp")):
            packaged = executable.parent / "samples/stage3" / source.name
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
        single = output / "stage3-single-page.ksp"
        native_input = output / "stage3-native-source-compatible.ksp"
        native = output / "stage3-native-source-compatible.sm"
        restored = output / "stage3-native-source-compatible-restored.sm"
        protected_asset = output / "asset-native-protected.sm"
        protected_asset.write_bytes(b"previous-source-sentinel")
        record["steps"].append(cli(executable,["--export-native-project",single,protected_asset],output,"embedded-asset-native-refusal",20))
        if protected_asset.read_bytes()!=b"previous-source-sentinel":
            raise RuntimeError("Rejected embedded asset export changed its destination.")
        record["embeddedAssetNativeSourceStatus"] = "blocked; direct device compile is tested independently"
        record["steps"].append(cli(executable,["--compile-project",single,output/"embedded-asset-direct"],output,"compile-embedded-asset-direct"))
        record["steps"].append(cli(executable, ["--export-native-project", native_input, native], output, "export-source"))
        record["steps"].append(cli(executable, ["--export-native-project", native, restored], output, "restore-source"))
        if sha(native) != sha(restored):
            raise RuntimeError("Unedited native source was not preserved.")
        for label, source in (("direct", native_input), ("roundtrip", native)):
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
    parser.add_argument("--executable", type=Path, default=ROOT / "dist/KinSealStudioLite-Stage3-0.10.0-final/KinSealStudioLite.exe")
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
