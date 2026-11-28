"""Compare local page I/O in both production compilers with paired V5.1 runs and AMZ firmware reads."""
# 20261128-17 add
import argparse
from array import array
import hashlib
import json
from pathlib import Path
import struct
import sys
import tempfile

from verify_native_drawing_drafts import ROOT, REVISION, git_blob, run


IO = "docs/stage2-system-io-evidence"
CREATED = "docs/stage4-evidence/table/native/flags-investigation/compiled"
REOPENED = "docs/stage5-evidence/native/table"
ROUTES = ("--compile-project", "--compile-native-media-parts")
FIELDS = tuple(base + offset for base in (352, 752) for offset in (0, 4, 5, 6, 12, 13, 14, 15))
CASES = (("io-baseline", "io-baseline"), ("hmi-flag", "hmi-lw0"), ("hmi-addr", "hmi-lw321"),
         ("hmi-disabled-address-retention", "plc-baseline"), ("plc-flag", "plc-lw0"), ("plc-addr", "plc-lw789"))


def sha(raw):
    """Identify the complete observed artifact."""
    return hashlib.sha256(raw).hexdigest()


def firmware_evidence():
    """Cross-check the selected instruction bytes and their offsets against the shipped firmware."""
    raw = (ROOT / "bin/AMZ070W01.bin").read_bytes()
    if sha(raw) != "7f625b7a967c1ecc0ff5db1fba4d6ed68cb1d9474216e87009395f1d53a88db5":
        raise ValueError("unexpected AMZ firmware revision")
    addresses = {0x80072098, 0x8007209c, 0x800720a0, 0x800720a4, 0x800720a8, 0x800720ac, 0x800720b0,
                 0x800720b4, 0x8004fd50, 0x8004fd54, 0x8004fd64, 0x8004fd68, 0x8004fd8c, 0x800a801c,
                 0x800a8020, 0x800a8030, 0x800a8080, 0x80083070, 0x80083074, 0x8008307c}
    found = {}
    with (ROOT / "firmware_analysis/amz070w01-disasm-20261119/full_disassembly.asm").open(encoding="utf8") as asm:
        for line in asm:
            cols = line.split(None, 4)
            if len(cols) != 5 or line.startswith(";") or cols[2] != "ARM" or int(cols[1], 16) not in addresses:
                continue
            off = int(cols[0], 16)
            if raw[off:off+4] != bytes.fromhex(cols[3]):
                raise ValueError("firmware and instruction listing disagree at " + cols[1])
            found[int(cols[1], 16)] = dict(offset=off, bytes=cols[3], instruction=cols[4].strip())
    if found.keys() != addresses:
        raise ValueError("incomplete firmware evidence")
    return dict(sha256=sha(raw), instructions={f"{k:08X}": v for k,v in sorted(found.items())})


def layout(exe, work, path):
    """Bind primitive spans to this exact source and read their original values."""
    target = work / "fields.json"
    run(exe, work, "--inspect-native-source", path, target)
    raw = path.read_bytes()
    doc = json.loads(target.read_bytes())
    if doc["sha256"] != sha(raw):
        raise ValueError("layout belongs to another SM")
    return {field["path"]: field for field in doc["fields"]}


def value(raw, fields, key):
    """Read the Qt4 big-endian primitive from the SM, independently of the DAT."""
    f = fields[key]
    blob = raw[f["start"]:f["end"]]
    if f["kind"] == "q":
        return blob[4:].decode("utf-16-be")
    return int.from_bytes(blob, "big")


def predicted(raw, fields):
    """Predict the 16 enabled/space/type/station/address bytes from both original editor rows."""
    out = bytearray()
    for flag,prefix in (("config.13c.0","config.140."), ("config.188.0","config.18c.")):
        out.extend((value(raw, fields, flag), value(raw, fields, prefix+"1"),
                    ord("W") if value(raw, fields, prefix+"2") else ord("_"), value(raw, fields, prefix+"3")))
        out.extend(struct.pack("<I", int(value(raw, fields, prefix+"4"))))
    return bytes(out)


def edit(raw, fields, changes):
    """Apply typed one-variable or combined changes without reusing production field writers."""
    for key, new in sorted(changes.items(), key=lambda x: fields[x[0]]["start"], reverse=True):
        f = fields[key]
        if f["kind"] == "q":
            scalar = new.encode("utf-16-be")
            blob = struct.pack(">I", len(scalar)) + scalar
        else:
            blob = new.to_bytes(f["end"]-f["start"], "big")
        raw = raw[:f["start"]] + blob + raw[f["end"]:]
    return raw


def check_io(exe, work, name, native):
    """Compare every reference against both generated results and retain their full-file difference counts."""
    raw = git_blob(IO, name + ".sm")
    sm = work / (name + ".sm")
    sm.write_bytes(raw)
    expected = predicted(raw, layout(exe, work, sm))
    samples = [git_blob(IO, f"steady-{native}-r{repeat}-SysPara.dat") for repeat in (1, 2)]
    runs = []
    for route in ROUTES:
        out = work / (name + route)
        run(exe, work, route, sm, out)
        product = (out / "SysPara.dat").read_bytes()
        if len(product) != 4998 or bytes(product[i] for i in FIELDS) != expected:
            raise ValueError("production I/O bytes differ for " + name)
        for original in samples:
            if len(original) != len(product) or bytes(original[i] for i in FIELDS) != expected:
                raise ValueError("original I/O bytes differ for " + name)
            runs.append(dict(route=route, original=sha(original), generated=sha(product),
                             differingFileBytes=sum(a != b for a,b in zip(original, product))))
    return dict(source=name, sha256=sha(raw), reference=native, predictedHex=expected.hex(), comparisons=runs)


def render_table(page, created):
    """Build the observed integer table page directly in RGB565 from imported geometry and live/reopen context."""
    sw, sh = page["width"], page["height"]
    obj, = page["objects"]
    x, y, w, h = (obj[k] for k in ("x", "y", "width", "height"))
    g = obj["graphic"]
    if (sw,sh) != (800,480) or g["alpha"] != 255 or w % g["columns"] or h % g["rows"] or g["lineStyle"] != 1:
        raise ValueError("table lies outside the original integer, solid grid fixtures")

    def rgb(color):
        """Quantize a Qt #AARRGGBB color to one native RGB565 word."""
        c = int(color[-6:], 16)
        return ((c >> 8) & 0xf800) | ((c >> 5) & 0x7e0) | ((c >> 3) & 0x1f)

    pixels = array("H", [0]) * (sw*sh)

    def strip(first, last, top, color):
        """Fill an inclusive horizontal run in the image."""
        pixels[top*sw+first:top*sw+last+1] = array("H", [color]) * (last-first+1)

    for top in range(1, sh):
        strip(1, sw-1, top, rgb("#ffc5dfff"))
    for top in range(y, y+h):
        strip(x, x+w-1, top, rgb(g["background"]))
    if g["horizontal"]:
        for idx in range(1, g["rows"]):
            strip(x, x+w-1+int(created), y+h*idx//g["rows"], rgb(g["lineColor"]))
    if g["vertical"]:
        for idx in range(1, g["columns"]):
            for top in range(y, y+h+int(created)):
                pixels[top*sw+x+w*idx//g["columns"]] = rgb(g["lineColor"])
    if g["outline"]:
        for top in (y,y+h):
            strip(x, x+w, top, rgb(g["frameColor"]))
        for top in range(y+1, y+h):
            for left in (x,x+w):
                pixels[top*sw+left] = rgb(g["frameColor"])
    if sys.byteorder != "little":
        pixels.byteswap()
    return pixels.tobytes()


def check_tables(exe, work):
    """Use each original source twice and preserve separate native live and reopened reference hashes."""
    rows = []
    original_base = git_blob(CREATED + "/table-flags-0-r1", "SysPara.dat")
    residuals = ROOT / "docs/文件规则/样例/SysPara同源残差与窗口语言交叉复验_20261128.json"
    variation = json.loads(residuals.read_bytes())["repeated"]["nonClockOffsetsAndPairCounts"]
    known_variable = set(map(int, variation)) | set(range(324, 352))
    for mask in range(8):
        live = CREATED + f"/table-flags-{mask}-r1"
        fresh = REOPENED + f"/table-flags-{mask}-reopen-r1/compiled/fresh-process-3"
        raw = git_blob(live, "input-native.sm")
        native_system = git_blob(live, "SysPara.dat")
        system_changes = [i for i,(a,b) in enumerate(zip(original_base,native_system)) if a!=b]
        if len(native_system) != 4998 or len(original_base) != 4998 or set(system_changes) - known_variable:
            raise ValueError("additional native system changes remain for table mask " + str(mask))
        if raw != git_blob(fresh, "input-native.sm"):
            raise ValueError("different native sources for table mask " + str(mask))
        sm = work / f"table-{mask}.sm"
        sm.write_bytes(raw)
        ksp = work / f"table-{mask}.ksp"
        run(exe, work, "--migrate-project", sm, ksp)
        page = json.loads(ksp.read_bytes())["pages"][0]
        independent = {}
        for label, reference in (("created",live), ("reopened",fresh)):
            pixels = render_table(page, label=="created")
            frame, packed = work / "table.rgb565", work / "table.lzo"
            frame.write_bytes(pixels)
            run(exe, work, "--compress-rgb565-frame", frame, packed)
            actual = packed.read_bytes()
            original = git_blob(reference,"SheetPixel.dat")
            lengths = struct.pack("<I",len(actual)) + bytes(2396)
            if actual != original or lengths != git_blob(reference,"PicLenPara.dat"):
                raise ValueError(f"independent {label} raster or lengths differ for mask {mask}")
            independent[label] = dict(rgb565=sha(pixels), sheet=sha(actual), index=sha(lengths))
        observed = {}
        for route in ROUTES:
            out = work / (sm.stem + route)
            run(exe, work, route, sm, out)
            live_out = work / (sm.stem + route + "-created")
            run(exe, work, route, sm, live_out, "--native-created-drawing")
            cells = {}
            for name in ("SheetPixel.dat", "PicLenPara.dat", "AddrPara.dat"):
                before, after, current = git_blob(live,name), git_blob(fresh,name), (out/name).read_bytes()
                live_actual = (live_out/name).read_bytes()
                if after != current:
                    raise ValueError(f"{route} {mask} {name} differs from reopened original")
                if before != live_actual:
                    raise ValueError(f"{route} {mask} {name} differs from created original")
                cells[name] = dict(live=sha(before), reopened=sha(after), generated=sha(current),
                                   generatedCreated=sha(live_actual), defaultMatchesLive=before==current)
            if any((p/name).exists() for p in (out,live_out) for name in ("ControlPara.dat", "ControlPixel.dat")):
                raise ValueError("static table incorrectly emitted a device control")
            observed[route] = cells
        rows.append(dict(mask=mask, source=sha(raw), live=live, reopened=fresh, comparisons=observed,
                         independent=independent, liveSystem=sha(native_system), nativeSystemDelta=system_changes))
    return rows


def controlled(exe, work):
    """Check boundary numbers, row coexistence, model selection and failure before publishing unsupported profiles."""
    sm = work / "probe.sm"
    raw = git_blob(IO, "plc-addr.sm")
    sm.write_bytes(raw)
    fields = layout(exe, work, sm)
    changes = {"config.13c.0":1, "config.140.0":"LW65535", "config.140.4":"65535",
               "config.18c.0":"LW0", "config.18c.4":"0"}
    accepted = []
    for name, extra in (("AMT", {}), ("AMZ", {"system.identity.1":"AMZ070W01R"})):
        sm.write_bytes(edit(raw, fields, {**changes, **extra}))
        expected = predicted(sm.read_bytes(), layout(exe, work, sm))
        output = work / ("upper-"+name)
        run(exe, work, "--compile-project", sm, output)
        actual = (output/"SysPara.dat").read_bytes()
        if bytes(actual[i] for i in FIELDS) != expected:
            raise ValueError("combined active HMI and PLC case failed on " + name)
        accepted.append(dict(model=name, output=expected.hex()))
    invalid = [{"config.13c.1":1}, {"config.140.3":101}, {"config.140.6":0}, {"config.140.1":1},
               {"config.140.0":"LW0321"}, {"config.188.0":0}, {"config.18c.0":"LW65536","config.18c.4":"65536"},
               {"config.18c.8":"PLC"}, {"config.200.7":2}, {"object[0].common.visibility.0":1}]
    for change in invalid:
        sm.write_bytes(edit(raw, fields, change))
        out = work / "invalid"
        run(exe, work, "--compile-project", sm, out, success=False)
        if out.exists() and any(out.iterdir()):
            raise ValueError("unsupported I/O profile left a partial output")
    table = git_blob(CREATED+"/table-flags-1-r1", "input-native.sm")
    sm.write_bytes(table)
    table_fields = layout(exe, work, sm)
    table_invalid = [{"config.240.7":8}, {"object[0].table.rowAddress.1":1}, {"object[0].table.10":1},
                     {"object[0].table.columnControlled":1}, {"object[0].common.visibility.0":1}]
    for change in table_invalid:
        sm.write_bytes(edit(table, table_fields, change))
        out = work / "invalid-table"
        run(exe, work, "--compile-project", sm, out, success=False)
        if out.exists() and any(out.iterdir()):
            raise ValueError("unsupported table profile left a partial output")
    return dict(accepted=accepted, rejected=invalid, rejectedTables=table_invalid)


def main():
    """Verify complete original outputs without normalizing differences in a whole-file claim."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    parser.add_argument("--receipt", type=Path)
    parser.add_argument("--scratch", type=Path, default=ROOT / "build")
    args = parser.parse_args()
    exe = args.exe.resolve()
    report = dict(revision=REVISION, exe=sha(exe.read_bytes()), firmware=firmware_evidence(), fileOffsets=FIELDS)
    with tempfile.TemporaryDirectory(dir=args.scratch) as tmp:
        work = Path(tmp).resolve()
        report["io"] = [check_io(exe, work, *case) for case in CASES]
        report["controlled"] = controlled(exe, work)
        report["table"] = check_tables(exe, work)
    if args.receipt:
        args.receipt.write_bytes((json.dumps(report, ensure_ascii=False, indent=1)+"\n").encode())
    print("Six I/O sources, twelve original system segments, both compiler routes and eight table masks passed.")


if __name__ == "__main__":
    main()
