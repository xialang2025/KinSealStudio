"""Save new V122 scale controls from KSP, compare raw object bodies, and verify the surrounding project."""
# 20261128-35 add
import argparse
from copy import deepcopy
import json
from pathlib import Path
from tempfile import TemporaryDirectory

from verify_amz_local_page_io import layout, sha
from verify_native_drawing_drafts import REVISION, ROOT, git_blob, run
from verify_native_scale_source import BASE, CASES
from verify_amz_firmware_page_images import firmware_frame


PRODUCTS = ("SheetPixel.dat", "PicLenPara.dat", "AddrPara.dat", "SheetPara.dat")
ROUTES = ("--compile-project", "--compile-native-media-parts")


def compiled(exe, work, source_paths, firmware, reference=None):
    """Compare both public compilers and both fresh input formats against the complete vendor page products."""
    report = {}
    pages = set()
    reference_files = ({n: git_blob(reference, n) for n in PRODUCTS} if reference else {})
    reference_page = firmware_frame(firmware, reference_files["SheetPixel.dat"]) if reference else None
    for source_path in source_paths:
        for route in ROUTES:
            name = source_path.suffix[1:] + route
            out = work / ("parts-" + name)
            run(exe, work, route, source_path, out)
            members = {n: (out/n).read_bytes() for n in PRODUCTS}
            if (out/"ControlPara.dat").exists() or (out/"ControlPixel.dat").exists():
                raise ValueError("static scale generated a dynamic device record")
            data = firmware_frame(firmware, members["SheetPixel.dat"])
            pages.add(sha(data))
            if reference and data != reference_page:
                raise ValueError("different physical pixels with fresh " + name)
            files = {}
            for key, actual in members.items():
                files[key] = dict(generated=sha(actual))
                if reference:
                    raw = reference_files[key]
                    delta = [i for i, (a,b) in enumerate(zip(actual,raw)) if a!=b]
                    if len(actual) != len(raw) or (delta and (key != "SheetPara.dat" or delta != [59])):
                        raise ValueError(f"{reference} {name} {key}: lengths {len(actual)}/{len(raw)}, offsets {delta[:12]}")
                    files[key].update(vendor=sha(raw), differences=delta, exact=actual==raw)
            report[name] = files
    if len(pages) != 1 or any(len({route[n]["generated"] for route in report.values()}) != 1 for n in PRODUCTS):
        raise ValueError("the two fresh inputs or compilation routes produced different files")
    return dict(rgb565=pages.pop(), routes=report)


def inspect(exe, work, path, index=0):
    """Return the isolated type-to-private.17 body plus the complete host values outside all object trees."""
    spans = layout(exe, work, path)
    data = path.read_bytes()
    prefix = f"object[{index}]."
    body_start = spans[prefix + "type"]["start"]
    body_end = spans[prefix + "private.17"]["end"]
    body = data[body_start:body_end]
    fields = {k: data[v["start"]:v["end"]] for k, v in spans.items()
              if k.startswith(prefix) and body_start <= v["start"] and v["end"] <= body_end}
    host = {k: data[v["start"]:v["end"]] for k, v in spans.items()
            if not k.startswith("object[") and k != "page.objects.count"}
    return body, fields, host


def save(exe, work, stem, model):
    """Use the same native source writer used when saving the editable canvas."""
    ksp = work / (stem + ".ksp")
    sm = work / (stem + ".sm")
    ksp.write_text(json.dumps(model, ensure_ascii=False), encoding="utf8")
    run(exe, work, "--export-native-project", ksp, sm)
    return sm


def reopen(exe, work, path):
    """Read the saved SM through the ordinary native migration entry point."""
    ksp = work / (path.stem + "-opened.ksp")
    run(exe, work, "--migrate-project", path, ksp)
    return json.loads(ksp.read_bytes())


def fresh_witness(exe, work, variant, firmware):
    """Discard all source bytes and require editable properties alone to reproduce the vendor graph."""
    native = work/"original.sm"
    native.write_bytes(git_blob(BASE, "scale-" + variant + ".sm"))
    body, before, _ = inspect(exe, work, native)
    model = reopen(exe, work, native)
    for key in ("nativeSourceSnapshot", "nativeSourceWarnings"):
        model.pop(key, None)
    asset = ROOT/"resources/native-scale"/("scale-" + variant + ".body")
    if variant != "direction-down" and (asset.read_bytes() != body or body[:4] != bytes.fromhex("0000001f")):
        raise ValueError("resource does not contain exactly one vendor scale body: " + variant)
    exported = save(exe, work, "fresh", model)
    after_body, after, _ = inspect(exe, work, exported)
    differences = {k for k in before if before[k] != after[k]}
    expected = {"object[0].common.58.3"} if variant == "direction-down" else set()
    if before.keys() != after.keys() or differences != expected or (after_body == body) != (not differences):
        raise ValueError("unexpected native field differences for " + variant + ": " + str(differences))
    opened = reopen(exe, work, exported)
    original = model["pages"][0]["objects"][0]
    check = opened["pages"][0]["objects"][0]
    if any(check[k] != original[k] for k in ("x", "y", "width", "height", "scale", "name")):
        raise ValueError("fresh source lost editor settings")
    compiled_parts = compiled(exe, work, (work/"fresh.ksp", exported), firmware,
                              BASE + "/compiled/scale-" + variant + "-r1")
    return dict(variant=variant, reference=BASE + "/scale-" + variant + ".sm", inputSM=sha(native.read_bytes()),
                originalObjectBytes=len(body), originalBodySHA256=sha(body), liteBodySHA256=sha(after_body),
                bodyExact=body == after_body, differentFields=sorted(differences), compiled=compiled_parts)


def fresh_host(exe, work, firmware, legacy_lock):
    """Make a new host without native data, then exercise new roots, property edits and host isolation."""
    model = json.loads((ROOT/"samples/stage4/scale.ksp").read_bytes())
    if "nativeSourceSnapshot" in model:
        raise ValueError("fresh host must have no archived native bytes")
    page = model["pages"][0]
    obj = next(o for o in page["objects"] if o["type"] == "scale")
    page["objects"] = [obj]
    page["name"] = "自建刻度实验"
    obj.update(name="新建刻度A", id="fresh-A", x=73, y=61, width=200, height=100, layer=7)
    obj["scale"].update(minimum=4, maximum=104, decimals=0, position=3, direction=1, showValues=True,
                        majorCount=4, minorCount=1, fontSize=16)
    platform = deepcopy(model)
    platform["pages"][0]["objects"] = []
    blank = save(exe, work, "empty", platform)
    blank_raw = blank.read_bytes()
    host_spans = layout(exe, work, blank)
    expected = {k: blank_raw[v["start"]:v["end"]] for k, v in host_spans.items() if k != "page.objects.count"}
    first = save(exe, work, "one", model)
    first_body, _, host = inspect(exe, work, first)
    if host != expected:
        raise ValueError("the draft touched host data outside the page object table: " + str(host.keys() ^ expected.keys()))
    opened = reopen(exe, work, first)
    if opened["pages"][0]["name"] != page["name"]:
        raise ValueError("new scale changed the host page name")
    actual = opened["pages"][0]["objects"][0]
    if any(actual[k] != obj[k] for k in ("name", "layer", "scale", "x", "y", "width", "height")):
        raise ValueError("new non-witness scale did not survive reopening")
    if save(exe, work, "roundtrip", opened).read_bytes() != first.read_bytes():
        raise ValueError("freshly generated native file failed exact open/save stability")
    images = compiled(exe, work, (work/"one.ksp", first), firmware)
    # Add a new scale to a source-backed host while retaining the existing object bytes, layer and page metadata.
    second = deepcopy(obj)
    second.update(id="fresh-B", name="本次加入B", x=366, y=210, layer=13, locked=True)
    second["scale"].update(minimum=1, maximum=101, direction=0, majorCount=6)
    opened["pages"][0]["objects"].append(second)
    if legacy_lock:
        # The frozen clean GUI model predates the object.locked field. It must explicitly refuse it.
        lock_input = work/"old-root-lock.ksp"
        lock_output = work/"old-root-lock.sm"
        lock_input.write_text(json.dumps(opened, ensure_ascii=False), encoding="utf8")
        outcome = run(exe, work, "--export-native-project", lock_input, lock_output, success=False)
        if lock_output.exists() or "object.locked" not in json.loads(outcome)["message"]:
            raise ValueError("legacy root lock unexpectedly serialized or failed for a different reason")
        second.pop("locked")
    both = save(exe, work, "two", opened)
    existing, _, actual_host = inspect(exe, work, both)
    if existing != first_body or actual_host != expected:
        raise ValueError("appending a scale altered the original host or first scale")
    again = reopen(exe, work, both)["pages"][0]["objects"]
    if len(again) != 2 or any(again[1][k] != second[k] for k in ("name", "x", "y", "scale", "layer")) or \
            bool(again[1].get("locked")) != bool(second.get("locked")):
        raise ValueError("the appended scale lost private settings or root state")
    rejected = {}
    for key, value in (("width", 201), ("fontSize", 19), ("position", 1), ("majorCount", 9),
                       ("decimals", 2), ("lineColor", "#ffab3012"), ("maximum", 1300), ("minimum", -2)):
        modified = deepcopy(model)
        target = modified["pages"][0]["objects"][0]
        (target if key == "width" else target["scale"])[key] = value
        ksp = work/("reject-" + key + ".ksp")
        sm = ksp.with_suffix(".sm")
        ksp.write_text(json.dumps(modified, ensure_ascii=False), encoding="utf8")
        run(exe, work, "--export-native-project", ksp, sm, success=False)
        if sm.exists():
            raise ValueError("a rejected scale emitted a partial SM")
        rejected[key] = value
    return dict(firstSource=sha(first.read_bytes()), secondSource=sha(both.read_bytes()), hostFieldsPreserved=len(expected),
                firstValues=dict(x=obj["x"], y=obj["y"], min=4, max=104, direction=1), preservedFirstBody=sha(first_body),
                addedLayer=13, lockResult="oldSchemaRefused" if legacy_lock else "preserved",
                exactFullFileReopen=True, compiled=images, rejected=rejected)


def main():
    """Run all donor and independently created cases against the current production command."""
    parser = argparse.ArgumentParser()
    parser.add_argument("exe", type=Path)
    parser.add_argument("--receipt", type=Path)
    parser.add_argument("--legacy-no-root-lock", action="store_true",
                        help="Verify the frozen clean tree's pre-existing rejection of object.locked.")
    parser.add_argument("--compare-clean-receipt", type=Path,
                        help="Record an independently measured clean build after comparing both inputs and routes.")
    args = parser.parse_args()
    exe = args.exe.resolve()
    firmware = (ROOT/"bin/AMZ070W01.bin").read_bytes()
    if sha(firmware) != "7f625b7a967c1ecc0ff5db1fba4d6ed68cb1d9474216e87009395f1d53a88db5":
        raise ValueError("unexpected AMZ firmware")
    rows = []
    for variant in CASES:
        with TemporaryDirectory(dir=ROOT/".local") as temporary:
            rows.append(fresh_witness(exe, Path(temporary), variant, firmware))
    with TemporaryDirectory(dir=ROOT/".local") as temporary:
        host = fresh_host(exe, Path(temporary), firmware, args.legacy_no_root_lock)
    result = dict(exe=sha(exe.read_bytes()), originalRevision=REVISION, firmwareSHA256=sha(firmware), cases=rows, freshHost=host)
    if args.compare_clean_receipt:
        other = json.loads(args.compare_clean_receipt.read_bytes())
        if (other["cases"] != rows or other["originalRevision"] != REVISION or other["firmwareSHA256"] != sha(firmware)
                or other["freshHost"]["compiled"] != host["compiled"]
                or other["freshHost"]["hostFieldsPreserved"] != host["hostFieldsPreserved"]
                or other["freshHost"]["lockResult"] != "oldSchemaRefused" or host["lockResult"] != "preserved"):
            raise ValueError("independent clean and production scale results differ")
        result["cleanBuild"] = dict(exe=other["exe"], sameSourceRows=len(rows), sameFreshCompiledProducts=True,
                                    hostFieldsPreserved=other["freshHost"]["hostFieldsPreserved"],
                                    rootLock=other["freshHost"]["lockResult"])
    if args.receipt:
        args.receipt.write_text(json.dumps(result, ensure_ascii=False, indent=1) + "\n", encoding="utf8")
    print("exact vendor bodies", sum(r["bodyExact"] for r in rows), "of", len(rows),
          "preserved host fields", host["hostFieldsPreserved"], "refusals", len(host["rejected"]))


if __name__ == "__main__":
    main()
