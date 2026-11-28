"""Verify the V5.1 Save As file with one 22-point continuous curve and one two-segment curve."""
# 20261128-38 add
import argparse
import hashlib
import json
import math
from pathlib import Path
import struct
from tempfile import TemporaryDirectory

from verify_native_drawing_drafts import ROOT, objects
from verify_native_reopened_curve_pixels import run_compiler


FIXTURES = ROOT / "tests/fixtures/native-source/curve-reopen-amz"


def verify(exe, work):
    """Compare actual source vertices, retained bytes, edited geometry and malformed paths via public file operations."""
    authored = FIXTURES / "AMZ-LIVE-01.sm"
    saved = FIXTURES / "native-saved.sm"
    first_model = work / "before.ksp"
    final_model = work / "after.ksp"
    restored = work / "restored.sm"
    raw = saved.read_bytes()
    layout = work / "layout.json"
    run_compiler(exe, "--inspect-native-source", saved, layout)
    spans = {row["path"]: slice(row["start"], row["end"]) for row in json.loads(layout.read_bytes())["fields"]}
    prefix = "object[0].curve."
    run_compiler(exe, "--migrate-project", authored, first_model)
    run_compiler(exe, "--migrate-project", saved, final_model)
    before = objects(json.loads(first_model.read_bytes()))
    project = json.loads(final_model.read_bytes())
    after = objects(project)
    if [(o["type"], o["name"]) for o in after] != [(o["type"], o["name"]) for o in reversed(before)]:
        raise ValueError("Native equal-layer Save As no longer preserves these four objects in reverse order")
    if before[-1]["graphic"]["curveControls"] != [1] or before[-2]["graphic"] != after[1]["graphic"]:
        raise ValueError("The cubic or the independent two-line witness changed")
    smooth = after[0]
    points = smooth["graphic"]["points"]
    if len(points) != 22 or smooth["graphic"]["curveControls"]:
        raise ValueError("A flattened native curve must expose its 22 sampled vertices without an inferred cubic handle")
    for i, point in enumerate(points):
        path = prefix + f"element[{i}]."
        if int.from_bytes(raw[spans[path + "0"]], "big") != int(i != 0):
            raise ValueError("Expected one initial move and only line elements after it")
        for j, (origin, length) in enumerate(((smooth["x"], smooth["width"]), (smooth["y"], smooth["height"]))):
            native = struct.unpack(">d", raw[spans[path + str(j + 1)]])[0]
            if not math.isclose(origin + length * point[j], native, rel_tol=0, abs_tol=1e-10):
                raise ValueError(f"Canvas point {i} differs from the native path")
    run_compiler(exe, "--export-native-project", final_model, restored)
    if restored.read_bytes() != raw:
        raise ValueError("The reloaded original Save As no longer round-trips byte-for-byte")
    smooth["x"] += 7
    smooth["y"] -= 9
    final_model.write_text(json.dumps(project, ensure_ascii=False), encoding="utf-8")
    moved = work / "moved.sm"
    run_compiler(exe, "--export-native-project", final_model, moved)
    expected = bytearray(raw)
    for key, value in (("style.3", 7.0), ("style.4", -9.0)):
        expected[spans[prefix + key]] = struct.pack(">d", value)
    if moved.read_bytes() != expected:
        raise ValueError("Translation changed another source field or flattened an independent segment")
    # Isolated alterations exercise the fail-closed continuity, tag and coordinate checks.
    for i, (key, wire) in enumerate((("element[2].0", struct.pack(">i", 0)),
                                     ("element[1].0", struct.pack(">i", 3)),
                                     ("element[2].1", struct.pack(">d", float("nan"))))):
        invalid = work / f"invalid-{i}.sm"
        output = work / f"invalid-{i}.ksp"
        altered = bytearray(raw)
        altered[spans[prefix + key]] = wire
        invalid.write_bytes(altered)
        run_compiler(exe, "--migrate-project", invalid, output, expected_exit=21)
        if output.exists():
            raise ValueError("A rejected source published a partial project")
    for source in (final_model, moved):
        run_compiler(exe, "--compile-project", source, work / (source.stem + "-compiled"))
    return {"exeSha256": hashlib.sha256(exe.read_bytes()).hexdigest(),
            "originalSaveAsSha256": hashlib.sha256(raw).hexdigest(), "canvasObjects": len(after),
            "wireVertices": len(points), "untouchedSourceExact": True, "translatedSourceExact": True,
            "editedSmAndKspCompiled": True, "rejectedCases": 3}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--scratch", required=True, type=Path)
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    with TemporaryDirectory(dir=args.scratch.resolve()) as temporary:
        result = verify(args.exe.resolve(), Path(temporary))
    if args.receipt:
        args.receipt.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf8")
    print(json.dumps(result))
