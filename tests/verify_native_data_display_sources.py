"""Exercise V122 numeric, ASCII and time controls through the installed editor's public CLI."""

import argparse
import hashlib
import json
import os
import subprocess
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def require(condition, message):
    if not condition:
        raise ValueError(message)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def call(exe, work, *args, accepted=True):
    """Use the same offscreen CLI as normal project imports and atomic source saves."""
    result = subprocess.run([str(exe), *map(str, args)], capture_output=True, timeout=120,
                            env=dict(os.environ, QT_QPA_PLATFORM="offscreen", TEMP=str(work), TMP=str(work)),
                            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
    require((result.returncode == 0) == accepted, f"{args[0]}: {result.stdout!r}, {result.stderr!r}")
    return result


def roundtrip(exe, work, source):
    """Compare all original bytes after a real import, save and renewed source export."""
    ksp = work / "imported.ksp"
    output = work / "saved.sm"
    call(exe, work, "--migrate-project", source, ksp)
    call(exe, work, "--export-native-project", ksp, output)
    require(source.read_bytes() == output.read_bytes(), f"source differs: {source.name}")
    return json.loads(ksp.read_bytes())


def exercise(exe, work, source, kind):
    """Change real layout and control properties, compare restored values, and verify refused edits."""
    project = roundtrip(exe, work, source)
    control = next(obj for obj in project["pages"][0]["objects"] if obj["type"] == kind)
    # A project without an original byte snapshot exercises the writer's new-control seed selection.
    fresh = {key: value for key, value in project.items() if key not in ("nativeSourceSnapshot", "nativeSourceWarnings")}
    fresh_ksp = work / "fresh.ksp"
    fresh_source = work / "fresh.sm"
    fresh_ksp.write_text(json.dumps(fresh, ensure_ascii=False), encoding="utf-8")
    call(exe, work, "--export-native-project", fresh_ksp, fresh_source)
    fresh_control = roundtrip(exe, work, fresh_source)["pages"][0]["objects"][0]
    for key in ("type", "name", "x", "y", "width", "height", "address", kind + "Display"):
        require(fresh_control[key] == control[key], f"{source.name} new control {key} differs")
    properties = control[kind + "Display"]
    view = properties["common"] if kind == "ascii" else properties
    control["x"] += 15
    control["y"] -= 9
    control["width"] -= 3
    control["height"] += 7
    view.update(fontSize=24 if view["fontSize"] != 24 else 16, textColor="#1155bb", backgroundColor="#ccaa33")
    if kind == "number":
        view.update(totalDigits=4, decimalDigits=2)
    if kind == "ascii":
        properties["characterCount"] = 6
    if kind == "time":
        properties["dateFormat"] = 2
    else:
        # The dormant input address must survive, including when the active mode links input and monitor.
        view.update(monitorAddress="LW27", inputAddress="LW29")
        control["address"] = "LW27"
    ksp = work / "edited.ksp"
    output = work / "edited.sm"
    ksp.write_text(json.dumps(project, ensure_ascii=False), encoding="utf-8")
    call(exe, work, "--export-native-project", ksp, output)
    edited = output.read_bytes()
    require(edited != source.read_bytes(), "requested edits had no source representation")
    restored = roundtrip(exe, work, output)
    restored_control = next(obj for obj in restored["pages"][0]["objects"] if obj["type"] == kind)
    for key in ("x", "y", "width", "height", "address", kind + "Display"):
        require(restored_control[key] == control[key], f"{source.name} edited {key} differs")
    # An unsupported private field must leave the last usable file in place.
    view["opacity"] = 155 if view["opacity"] != 155 else 156
    ksp.write_text(json.dumps(project, ensure_ascii=False), encoding="utf-8")
    call(exe, work, "--export-native-project", ksp, output, accepted=False)
    require(output.read_bytes() == edited, f"refused {kind} edit changed existing file")
    return {"file": source.name, "family": kind, "bytes": source.stat().st_size,
            "sha256": digest(source.read_bytes()), "editedSha256": digest(edited),
            "originalByteEqual": True, "freshControlEqual": True,
            "editedFieldsEqual": True, "editedResaveByteEqual": True,
            "refusedEditPreservedDestination": True}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--work-root", type=Path, default=ROOT / "build")
    parser.add_argument("--real-project", type=Path, action="append", default=[])
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    exe = args.exe.resolve(strict=True)
    args.work_root.mkdir(parents=True, exist_ok=True)
    report = {"formatVersion": 122, "executableSha256": digest(exe.read_bytes()), "controls": [], "projects": []}
    with tempfile.TemporaryDirectory(prefix="data-source-", dir=args.work_root) as directory:
        work = Path(directory).resolve()
        for kind in ("number", "ascii", "time"):
            for source in (ROOT / "resources" / f"native-data-{kind}.sm",
                           ROOT / "tests/fixtures/native-source/data-display" / f"{kind}-bi.sm"):
                report["controls"].append(exercise(exe, work, source, kind))
        for source in args.real_project:
            project = roundtrip(exe, work, source.resolve(strict=True))
            report["projects"].append({"file": source.name, "bytes": source.stat().st_size,
                                       "sha256": digest(source.read_bytes()), "pages": len(project["pages"]),
                                       "objects": sum(len(page["objects"]) for page in project["pages"]),
                                       "originalByteEqual": True})
    if args.receipt:
        args.receipt.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f'{len(report["controls"])} edited controls and {len(report["projects"])} additional projects verified')


if __name__ == "__main__":
    main()
