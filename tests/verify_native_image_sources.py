"""Check native V122 image and QR archives through the same CLI used to import and save projects."""
import argparse
import hashlib
import json
import os
import subprocess
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
REVISION = "e3b72657"
SNAPSHOTS = "docs/stage4-evidence"
REFERENCE = ROOT / "docs/文件规则/样例/V122图片和二维码原厂样本_20261127.json"


def run_program(executable, *arguments):
    """Run without focus changes and fail if the native operation did not report success."""
    result = subprocess.run([str(executable), *map(str, arguments)], capture_output=True,
                            env={**os.environ, "QT_QPA_PLATFORM": "offscreen"}, timeout=90,
                            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
    try:
        reply = json.loads(result.stdout)
    except (ValueError, UnicodeDecodeError):
        reply = {"ok": False, "message": (result.stderr or result.stdout).decode("utf8", "replace")}
    if result.returncode or not reply.get("ok"):
        raise ValueError(f"{arguments[0]}: {reply.get('message', reply)}")


def original_gif_archive(name, sha, exe, work, fixtures):
    """Check the source-window route, including title updates without altering the frame table."""
    input_file = work / "gif-input.sm"
    output_file = work / "gif-output.sm"
    report = work / "gif-spans.json"
    raw = ((fixtures / name).read_bytes() if fixtures else subprocess.check_output(
        ["git", "show", f"{REVISION}:{SNAPSHOTS}/gif/native/{name}"], cwd=ROOT))
    require(hashlib.sha256(raw).hexdigest() == sha, f"{name} SHA-256")
    input_file.write_bytes(raw)
    run_program(exe, "--resave-native-source", input_file, output_file)
    require(output_file.read_bytes() == raw, f"{name} retained GIF differs")
    run_program(exe, "--resave-native-source", input_file, output_file, "GIF-REF")
    sources = []
    for path in (input_file, output_file):
        run_program(exe, "--inspect-native-source", path, report)
        fields = json.loads(report.read_bytes())["fields"]
        title = next(field for field in fields if field["path"] == "trailer.settings.1")
        data = path.read_bytes()
        if path == output_file:
            require(data[title["start"] + 4:title["end"]].decode("utf-16be") == "GIF-REF.sm",
                    f"{name} edited title differs")
        sources.append(data[:title["start"]] + data[title["end"]:])
    require(sources[0] == sources[1], f"{name} editing the title changed unrelated fields")


def original_bytes(name, fixtures):
    """Load a native reference from a supplied folder or the pinned project history."""
    if fixtures:
        return (fixtures / name).read_bytes()
    family = "qrcode" if name.startswith("qrcode-") else "picture"
    relative = f"{SNAPSHOTS}/{family}/native/{name}"
    return subprocess.check_output(["git", "show", f"{REVISION}:{relative}"], cwd=ROOT)


def require(predicate, message):
    if not predicate:
        raise ValueError(message)


def check_archive(exe, work, source_bytes, case):
    name = case["file"]
    original = work / "original.sm"
    model = work / "loaded.ksp"
    output = work / "exported.sm"
    reopened = work / "reopened.ksp"
    original.write_bytes(source_bytes)
    require(hashlib.sha256(source_bytes).hexdigest() == case["sourceSha256"], f"{name} SHA-256")
    run_program(exe, "--migrate-project", original, model)
    run_program(exe, "--export-native-project", model, output)
    require(output.read_bytes() == source_bytes, f"{name} unchanged archive differs")
    data = json.loads(model.read_bytes())
    objects = data["pages"][0]["objects"]
    require(len(objects) == 1, f"{name} reference object count")
    obj = objects[0]
    obj["x"] += 11
    obj["y"] -= 7
    prop = "qrCode" if name.startswith("qrcode-") else "imageDisplay"
    if prop == "qrCode":
        obj[prop]["description"] = "金玺 QR 16"
        obj[prop]["encoding"] = 1 - obj[prop]["encoding"]
        obj[prop]["swapBytes"] = not obj[prop]["swapBytes"]
        obj[prop]["characterLength"] = 1 if obj[prop]["characterLength"] == 255 else 255
        obj["address"] = obj[prop]["monitorAddress"] = "LW17"
    elif name == "picture-distinct-red-green.sm":
        obj[prop]["states"].reverse()
        obj[prop]["conditionEnabled"] = True
        obj[prop]["condition"] = 2
        obj["address"] = obj[prop]["monitorAddress"] = "LB17"
    model.write_text(json.dumps(data, ensure_ascii=False), encoding="utf8")
    run_program(exe, "--export-native-project", model, output)
    require(output.read_bytes() != source_bytes, f"{name} edit was skipped")
    run_program(exe, "--migrate-project", output, reopened)
    actual = json.loads(reopened.read_bytes())["pages"][0]["objects"][0]
    for key in ("type", "x", "y", "width", "height", "address", prop):
        require(actual[key] == obj[key], f"{name} edited field {key} differs")
    # A second save of the generated source must also preserve every byte.
    edited = output.read_bytes()
    run_program(exe, "--export-native-project", reopened, output)
    require(output.read_bytes() == edited, f"{name} re-save of edited source differs")


def compare_native_qr_variant(exe, work, case, fixtures):
    """Change only the documented QR properties and locate every differing byte in the factory variant."""
    reference = original_bytes(case["file"], fixtures)
    seed = work / "base.sm"
    native = work / "variant.sm"
    model = work / "baseline.ksp"
    variant = work / "variant.ksp"
    exported = work / "generated.sm"
    report = work / "fields.json"
    seed.write_bytes(original_bytes("qrcode-default.sm", fixtures))
    native.write_bytes(reference)
    run_program(exe, "--migrate-project", seed, model)
    run_program(exe, "--migrate-project", native, variant)
    data = json.loads(model.read_bytes())
    factory_model = json.loads(variant.read_bytes())
    target = factory_model["pages"][0]["objects"][0]["qrCode"]
    qr = data["pages"][0]["objects"][0]["qrCode"]
    properties = ("description", "characterLength", "swapBytes", "encoding")
    defaults = {name: qr[name] for name in properties}
    for property_name in properties:
        qr[property_name] = target[property_name]
    model.write_text(json.dumps(data, ensure_ascii=False), encoding="utf8")
    run_program(exe, "--export-native-project", model, exported)
    masked = []
    saved_fields = []
    for file in (native, exported):
        raw = file.read_bytes()
        run_program(exe, "--inspect-native-source", file, report)
        spans = {field["path"]: (field["start"], field["end"])
                 for field in json.loads(report.read_bytes())["fields"]}
        kept = bytearray()
        fields = []
        last = 0
        for field_name in sorted(case["referenceDifferences"], key=lambda key: spans[key][0]):
            start, end = spans[field_name]
            kept.extend(raw[last:start])
            fields.append(raw[start:end])
            last = end
        kept.extend(raw[last:])
        masked.append(kept)
        saved_fields.append(fields)
    require(masked[0] == masked[1], f"{case['file']} additional source bytes differ")
    require(all(old != new for old, new in zip(*saved_fields)),
            f"{case['file']} expected original differences changed")

    # From the actual factory archive, edit to the defaults, reopen, then restore the saved parameters.
    # The directory and retained original cache words must survive both independent export operations.
    original_values = {key: target[key] for key in properties}
    target.update(defaults)
    variant.write_text(json.dumps(factory_model, ensure_ascii=False), encoding="utf8")
    run_program(exe, "--export-native-project", variant, exported)
    require(exported.read_bytes() != reference, "Factory variant did not change")
    run_program(exe, "--migrate-project", exported, variant)
    restored = json.loads(variant.read_bytes())
    restored["pages"][0]["objects"][0]["qrCode"].update(original_values)
    variant.write_text(json.dumps(restored, ensure_ascii=False), encoding="utf8")
    run_program(exe, "--export-native-project", variant, exported)
    require(exported.read_bytes() == reference, f"{case['file']} restoring attributes altered other bytes")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path)
    parser.add_argument("--fixtures", type=Path, help="Folder containing all original .sm files")
    parser.add_argument("--temp-parent", type=Path, default=ROOT / "build")
    options = parser.parse_args()
    cases = json.loads(REFERENCE.read_bytes())
    with tempfile.TemporaryDirectory(prefix=".image-source-verify-", dir=options.temp_parent) as tmp:
        for case in cases:
            check_archive(options.executable.resolve(), Path(tmp),
                          original_bytes(case["file"], options.fixtures), case)
            if "referenceDifferences" in case:
                compare_native_qr_variant(options.executable.resolve(), Path(tmp), case, options.fixtures)
            print(f"PASS {case['file']}", flush=True)
        gif_archives = {
            "gif-after-gallery-cancel.sm": "5b45d5db6e9d134951f585592bce338b53c37ed9c004c5712cba89932b3127da",
            "gif-after-main-cancel.sm": "5b45d5db6e9d134951f585592bce338b53c37ed9c004c5712cba89932b3127da",
            "gif-bit-LB17-state0.sm": "8f05b8b62a04af3b9cb8970a0d4c8407ba4233f7f7a5bd773ef30f8c8dd304a5",
            "gif-default.sm": "5b45d5db6e9d134951f585592bce338b53c37ed9c004c5712cba89932b3127da",
        }
        for name, sha in gif_archives.items():
            original_gif_archive(name, sha, options.executable.resolve(), Path(tmp), options.fixtures)
            print(f"PASS {name} (retained source and title edit)", flush=True)
    print(f"{len(cases)} editable picture/QR archives checked, plus {len(gif_archives)} retained GIF archives.")


if __name__ == "__main__":
    main()
