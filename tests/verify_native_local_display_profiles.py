"""Verify the V122 single-display serial profiles against ten vendor runs and both normal compiler routes."""
# 20261128-22 add
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile

from verify_amz_local_page_io import edit, layout, sha, ROUTES
from verify_native_drawing_drafts import ROOT, REVISION, git_blob


BASE = "docs/stage4-evidence"
TABLES = ("docs/stage5-evidence/native/table/table-flags-0-reopen-r1/compiled/fresh-process-3",
          BASE + "/table/native/flags-investigation/compiled/table-flags-1-r1")
CASES = (("ascii", "ascii-default-r1", 0), ("number", "number-default-r1", 0), ("time", "time-default-r1", 0),
         ("message", "message-default-r1", 0), ("message", "message-visibility-bit-level", 0),
         ("message", "message-state1", 1), ("message", "message-state3", 1),
         ("message", "message-state3-left-speed5", 1), ("message", "message-state3-left-speed10", 1),
         ("message", "message-u32", 1))
CHANGES = {"number-default-r1": {"display.23.1": 1, "display.56.1": 1, "display.18": 1},
           "ascii-default-r1": {"display.15.1": 1, "display.40.1": 1, "display.16": 1},
           "time-default-r1": {"common.e4.1": 1, "common.touch.0": 1, "common.visibility.0": 1},
           "message-default-r1": {"private.head.3.1": 1, "common.58.2": 1},
           "message-visibility-bit-level": {"common.visibility.1": 1, "common.90.1": 1}}
SYSTEM_ERROR = "与已验证设备默认配置不同"


def fixture(exe, work, reference):
    """Load a vendor source and its matched system file, and bind the typed spans to this source."""
    source = git_blob(reference, "input-native.sm")
    path = work / "original.sm"
    path.write_bytes(source)
    fields = layout(exe, work, path)
    settings = {key: source[f["start"]:f["end"]] for key, f in fields.items()
                if key.startswith(("system.", "config.", "history."))}
    return path, source, fields, settings, git_blob(reference, "SysPara.dat")


def compile_case(exe, work, mode, sm, out):
    """Return only observed public diagnostics and check for accidental publication on failure."""
    process = subprocess.run([str(exe), mode, str(sm), str(out)], cwd=work, capture_output=True, timeout=90,
                             env=dict(os.environ, QT_QPA_PLATFORM="offscreen", TEMP=str(work), TMP=str(work)),
                             creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
    result = json.loads(process.stdout)
    if (process.returncode == 0) != result["ok"] or (not result["ok"] and out.exists() and any(out.iterdir())):
        raise ValueError("compile result and published files disagree")
    return result


def differing(a, b):
    """Return all different offsets, checking native system segment length first."""
    if len(a) != 4998 or len(b) != 4998:
        raise ValueError("invalid system segment length")
    return {i for i, (x, y) in enumerate(zip(a, b)) if x != y}


def verify(exe, previous, baseline, scratch):
    """Compare raw config fields and system blocks, then verify the exact acceptance boundary."""
    residue = ROOT / "docs/文件规则/样例/SysPara同源残差与窗口语言交叉复验_20261128.json"
    variable = {int(i) for i in json.loads(residue.read_bytes())["repeated"]["nonClockOffsetsAndPairCounts"]}
    variable.update(range(324, 352))
    if len(variable) != 200:
        raise ValueError("the controlled repeated-run exclusion set has changed")
    baseline_by_source = {r["source"]: r for r in baseline["rows"]} if baseline else {}
    tables = []
    for reference in TABLES:
        with tempfile.TemporaryDirectory(dir=scratch) as name:
            _, _, _, settings, system = fixture(exe, Path(name), reference)
            tables.append((settings, system))
    rows = []
    for family, name, profile in CASES:
        reference = f"{BASE}/{family}/native/compiled/{name}"
        with tempfile.TemporaryDirectory(dir=scratch) as temp:
            work = Path(temp).resolve()
            sm, source, fields, settings, system = fixture(exe, work, reference)
            differences = differing(system, tables[profile][1])
            if settings != tables[profile][0] or differences - variable:
                raise ValueError("unexplained native source config or system byte in " + name)
            row = dict(reference=reference, sourceSha256=sha(source), nativeSystemSha256=sha(system),
                       matchingTable=TABLES[profile], equalRawSettings=len(settings),
                       nativeSystemDifferencesFromTable=len(differences), equalOutside200VolatileBytes=True,
                       compilation={}, rejectedChanges=[])
            if baseline:
                old = baseline_by_source[row["sourceSha256"]]
                if baseline["revision"] != REVISION or old["reference"] != reference or old["compiled"] or SYSTEM_ERROR not in old["diagnostic"]:
                    raise ValueError("historical source or system-gate refusal differs for " + name)
                row["previousReceiptDiagnostic"] = old["diagnostic"]
            if previous:
                old = compile_case(previous, work, ROUTES[0], sm, work / "old")
                if old["ok"] or SYSTEM_ERROR not in old["message"]:
                    raise ValueError("previous binary did not reproduce the original condition for " + name)
                row["previousSystemGateRefusal"] = True
            for mode in ROUTES:
                out = work / ("output-" + mode[2:])
                result = compile_case(exe, work, mode, sm, out)
                entry = dict(compiled=result["ok"])
                if result["ok"]:
                    generated = (out / "SysPara.dat").read_bytes()
                    delta = differing(system, generated)
                    entry.update(systemSha256=sha(generated), differingFileBytes=len(delta),
                                 differingOutside200=sorted(delta - variable))
                    if system[:100] != generated[:100] or system[1152:] != generated[1152:]:
                        raise ValueError("stable vendor system options differ in " + name)
                elif not any(code in result["message"] for code in ("该类型尚未恢复独立设备编码", "设备编译尚不支持控件类型")):
                    raise ValueError("unexpected refusal for " + name + ": " + result["message"])
                else:
                    entry["controlDiagnostic"] = result["message"]
                row["compilation"][mode] = entry
            changes = {"object[0]." + key: value for key, value in CHANGES.get(name, {}).items()}
            if name == "message-default-r1":
                changes["config.48.6"] = 12345
            for index, (key, replacement) in enumerate(changes.items()):
                update = {key: replacement}
                if key in ("object[0].display.16", "object[0].display.18"):
                    update["object[0].display." + ("17" if family == "ascii" else "19")] = "InputMacro"
                sm.write_bytes(edit(source, fields, update))
                result = compile_case(exe, work, ROUTES[0], sm, work / ("invalid-" + str(index)))
                if result["ok"] or not any(reason in result["message"] for reason in
                                           (SYSTEM_ERROR, "外部地址", "数据控件引用的宏不存在")):
                    raise ValueError("unexpected result for " + key + ": " + result["message"])
                row["rejectedChanges"].append(dict(field=key, diagnostic=result["message"]))
            rows.append(row)
    return dict(referenceRevision=REVISION, compilerSha256=sha(exe.read_bytes()),
                previousSha256=sha(previous.read_bytes()) if previous else None,
                baselineReceiptCompiler=baseline["exeSha256"] if baseline else None, rows=rows)


def main():
    """Retain the requested compact evidence file and release all per-case temporary files."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    parser.add_argument("--previous", type=Path)
    parser.add_argument("--baseline-receipt", type=Path, help="optional earlier 248-source comparison report")
    parser.add_argument("--scratch", type=Path, default=ROOT / ".local")
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    baseline = json.loads(args.baseline_receipt.read_bytes()) if args.baseline_receipt else None
    result = verify(args.exe.resolve(), args.previous.resolve() if args.previous else None, baseline, args.scratch)
    if args.receipt:
        args.receipt.write_text(json.dumps(result, ensure_ascii=False, indent=1) + "\n", encoding="utf8")
    rows = result["rows"]
    print(json.dumps(dict(profiles=len(rows), compiledRoutes=sum(x["compiled"] for r in rows
                     for x in r["compilation"].values()), rejectedChanges=sum(len(r["rejectedChanges"]) for r in rows))))


if __name__ == "__main__":
    main()
