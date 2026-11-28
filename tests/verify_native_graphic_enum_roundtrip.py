"""Check the five V122 gated enum reads in retained drawings and newly authored shapes."""  # 20261128-22 add
import argparse
import hashlib
import json
from pathlib import Path
import re
import tempfile
import zlib

from verify_native_drawing_drafts import ROOT, git_blob, run
from verify_native_media_gallery import fields

MAXIMUM = {"touch.1": 3, "gate.0": 1, "gate.3": 1, "visibility.1": 3, "end.0": 1}
PATTERN = re.compile(r"object\[\d+\]\.(?:curve|box)\.(touch\.1|gate\.0|gate\.3|visibility\.1|end\.0)$")


def enum_values(exe, work, raw):
    """Read the actual big-endian integers using the verified SM field boundaries."""
    result = {}
    for key, span in fields(exe, work, raw).items():
        match = PATTERN.fullmatch(key)
        if match:
            result[key] = (int.from_bytes(raw[span], "big", signed=True), MAXIMUM[match[1]])
    return result


def roundtrips(exe, work):
    """Retain old source bytes and edit evidence while requiring valid enums for nine new graphic types."""
    seeds = sorted((ROOT / "resources").glob("native-*-default.sm"))
    seeds = [s for s in seeds if s.stem.split("-")[1] in ("rectangle", "ellipse", "polygon", "sector", "roundrect")]
    seeds += sorted((ROOT / "tests/fixtures/native-source/strokes").glob("*.sm"))
    seeds.append(ROOT / "tests/fixtures/native-source/linecurve/curve-threeclick-default.sm")
    results = []
    for seed in seeds:
        temp_model, restored, edited, fresh = (work / f for f in ("model.ksp", "restored.sm", "edited.sm", "fresh.sm"))
        raw = seed.read_bytes()
        old_values = enum_values(exe, work, raw)
        run(exe, work, "--export-native-project", seed, restored)
        if restored.read_bytes() != raw or len(old_values) != 5:
            raise ValueError("retained original or enum count differs: " + seed.name)
        run(exe, work, "--migrate-project", seed, temp_model)
        model = json.loads(temp_model.read_bytes())
        obj = model["pages"][0]["objects"][0]
        obj["x"] += 3
        obj["name"] = "Moved geometry"
        temp_model.write_text(json.dumps(model, ensure_ascii=False), encoding="utf8")
        run(exe, work, "--export-native-project", temp_model, edited)
        if edited.read_bytes() == raw or enum_values(exe, work, edited.read_bytes()) != old_values:
            raise ValueError("an unrelated edit replaced an existing enum: " + seed.name)
        template = json.loads((ROOT / "samples/stage5/mixed-fourteen.ksp").read_bytes())
        template.pop("nativeSourceSnapshot", None)
        template.pop("nativeSourceWarnings", None)
        if obj["type"] in ("line", "polyline", "freeline", "linecurve"):
            obj["graphic"]["endArrow"] = 0
        template["pages"][0]["objects"] = [obj]
        temp_model.write_text(json.dumps(template, ensure_ascii=False), encoding="utf8")
        run(exe, work, "--export-native-project", temp_model, fresh)
        new_values = enum_values(exe, work, fresh.read_bytes())
        if len(new_values) != 5:
            raise ValueError("new graphic omitted its underlying enum record")
        invalid = {k: v for k, (v, bound) in new_values.items() if not 0 <= v <= bound}
        run(exe, work, "--export-native-project", fresh, restored)
        if restored.read_bytes() != fresh.read_bytes():
            raise ValueError("new source did not roundtrip: " + seed.name)
        results.append(dict(seed=str(seed.relative_to(ROOT)).replace("\\", "/"), kind=obj["type"],
                            retainedExact=True, editedOldFieldsPreserved=True, freshSmRoundtripExact=True,
                            previousSeedValues={k: v for k, (v, _) in old_values.items()},
                            freshValues={k: v for k, (v, _) in new_values.items()}, freshValuesRejectedByNative=invalid))
    if len(results) != 9:
        raise ValueError("missing graphic seed")
    return results


def frozen(exe, work):
    """Test whether a changed enum in each pinned native graphic KNR had an out-of-range input SM value."""
    history = json.loads((ROOT / "docs/文件规则/样例/介质KNR原厂248源对照_20261128.json").read_bytes())
    records = []
    for row in history["cases"]:
        if not any(PATTERN.fullmatch(k) for k in row.get("remainingSourceFields", ())):
            continue
        ref = row["reference"]
        source = git_blob(ref, "input-native.sm")
        archive = zlib.decompress(git_blob(ref, "upLoad.knr")[4:])
        if hashlib.sha256(source).hexdigest() != row["source"]:
            raise ValueError("wrong original SM")
        before, after = enum_values(exe, work, source), enum_values(exe, work, archive)
        if set(before) != set(after):
            raise ValueError("different original object arrays")
        changed_valid = []
        diff = []
        valid = 0
        for key, (old, maximum) in before.items():
            accepted = 0 <= old <= maximum
            valid += accepted
            new, _ = after[key]
            if old != new:
                diff.append(dict(field=key, input=old, saved=new, nativeReadAcceptsInput=accepted))
                if accepted:
                    changed_valid.append(key)
        records.append(dict(source=row["source"], reference=ref, enumCount=len(before), validInput=valid,
                            changed=diff, changedInRange=changed_valid))
    return records


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    parser.add_argument("--scratch", type=Path, required=True)
    parser.add_argument("--receipt", type=Path, required=True)
    parser.add_argument("--corpus", action="store_true")
    args = parser.parse_args()
    exe = args.exe.resolve()
    with tempfile.TemporaryDirectory(dir=args.scratch) as td:
        work = Path(td).resolve()
        cases = roundtrips(exe, work)
        native = frozen(exe, work) if args.corpus else []
    result = dict(exeSha256=hashlib.sha256(exe.read_bytes()).hexdigest(), revision="e3b72657", authored=cases, native=native)
    args.receipt.write_text(json.dumps(result, ensure_ascii=False, indent=1) + "\n", encoding="utf8")
    bad = sum(len(r["freshValuesRejectedByNative"]) for r in cases)
    print("authored", len(cases), "invalid", bad, "native cases", len(native),
          "changed valid input", sum(len(r["changedInRange"]) for r in native))
    if bad or (args.corpus and (len(native) != 42 or any(r["changedInRange"] for r in native))):
        raise SystemExit(1)


if __name__ == "__main__":
    main()
