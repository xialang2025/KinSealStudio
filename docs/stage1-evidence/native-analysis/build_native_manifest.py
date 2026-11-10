"""#20261110-5 add: Index retained native samples without modifying them."""
from datetime import datetime, timezone
from pathlib import Path
import hashlib
import json

ROOT = Path(r"Z:\Project\vscode_prj_temp\REA")
OUT = ROOT / "KinSealStudio/docs/stage1-evidence/native-analysis"
WORK = ROOT / ".scratch/kinseal-stage1-20261110/native"


def digest(path):
    """Return a file identity for review; never edit the referenced file."""
    return {
        "path": path.relative_to(ROOT).as_posix(),
        "bytes": path.stat().st_size,
        "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
    }


def log(run):
    """Load one immutable per-run result rather than the rolling checkpoint."""
    return json.loads((OUT / ("native-probe-" + run + ".json")).read_text(encoding="utf-8"))


def main():
    """Build provenance and record-only cleanup reports for this ownership scope."""
    rows = [
        ("A-empty-one-page.sm", "r4", "empty-a-reopen-r1", "native-original-project-delete-all-save"),
        ("B-two-empty-pages.sm", "r4", "r6", "native-new-screen-defaults-save"),
        ("C-third-page-number9.sm", "r6", "r8", "native-new-screen-number9-save"),
        ("D-third-page-name.sm", "r7", "named-d-reopen-r1", "native-new-screen-name-Stage1_Third-save"),
        ("bit-operation-set-native-ui.sm", "bit-operations-r2", None, "native-bit-operation-index0-save"),
        ("bit-operation-reset-native-ui.sm", "bit-operations-r2", None, "native-bit-operation-index1-save"),
        ("bit-operation-jog-native-ui.sm", "bit-operations-r2", None, "native-bit-operation-index2-save"),
        ("bit-operation-toggle-native-ui.sm", "bit-operations-r2", None, "native-bit-operation-index3-save"),
        ("bit-notice-word7-native-ui.sm", "bit-notice-r2", None, "native-notice-word-LW0-value7-save"),
        ("bit-external-U32-native-ui.sm", "bit-external-r1", None, "native-external-touch-U32-save"),
        ("bit-external-I32-native-ui.sm", "bit-external-r1", None, "native-external-touch-I32-save"),
        ("bit-external-F32-native-ui.sm", "bit-external-r1", None, "native-external-touch-F32-save"),
        ("static-visible-level3-native-ui.sm", "static-level-r1", None, "native-static-visible-userlevel3-save"),
        ("two-language-distinct-native-resaved.sm", "bilingual-resave-r1", "bilingual-reopen-r1", "product-export-native-bit-edit-restore-save"),
        ("mixed-two-language-distinct-native-resaved.sm", "mixed-list-resave-r3", "mixed-bilingual-reopen-r1", "product-export-native-control-list-select-single-bit-edit-restore-save"),
        ("two-language-startup-english-native-resaved.sm", "english-startup-resave-r1", "english-startup-reopen-r1", "product-export-native-bit-edit-restore-save-config-conflict-negative"),
        ("two-language-startup-english-ui-native-resaved.sm", "english-startup-resave-r2", "english-startup-reopen-r2", "product-export-native-editor-language-English-then-bit-edit-restore-save"),
        ("macro-bound-native-resaved.sm", "macro-bound-resave-r1", "macro-bound-reopen-r1", "product-export-native-bit-edit-restore-save-macro-binding-guard-negative"),
    ]
    samples = []
    for name, writer_run, reopen_run, origin in rows:
        run = log(writer_run)
        sample = {
            **digest(OUT / name), "origin": origin, "writerRun": writer_run,
            "writerPid": run["pid"], "writerStatus": run["status"],
            "writerLog": digest(OUT / ("native-probe-" + writer_run + ".json")),
            "writerInput": run["input"], "independentReopen": None,
        }
        if reopen_run:
            reopened = log(reopen_run)
            sample["independentReopen"] = {
                "run": reopen_run, "pid": reopened["pid"],
                "status": reopened["status"], "input": reopened["input"],
                "log": digest(OUT / ("native-probe-" + reopen_run + ".json")),
                "scope": "B is independently opened as input of r6; r6 then creates C. Other reopen runs are read-only.",
            }
        typed = OUT / (Path(name).stem + "-inspect-result.json")
        semantic = OUT / (Path(name).stem + "-decode-result.json")
        for key, path in (("typedInspection", typed), ("semanticDecode", semantic)):
            if path.is_file():
                sample[key] = {"result": json.loads(path.read_text(encoding="utf-8")), "file": digest(path)}
        samples.append(sample)
    runs = []
    for path in sorted(OUT.glob("native-probe-*.json")):
        if path.name == "native-probe-checkpoint.json":
            continue
        data = json.loads(path.read_text(encoding="utf-8"))
        if "status" not in data or "pid" not in data:
            continue
        runs.append({
            "run": data.get("run", path.stem.removeprefix("native-probe-")),
            "runNameInferredFromFilename": "run" not in data,
            "pid": data.get("pid"), "status": data.get("status"),
            "error": data.get("error"), "ownedProcessTerminated": data.get("ownedProcessTerminated", False),
            "exitCodeBeforeCleanup": data.get("exitCodeBeforeCleanup"), "log": digest(path),
        })
    manifest = {
        "marker": "//20261110-5", "capturedAtUtc": datetime.now(timezone.utc).isoformat(),
        "evidenceProvider": "ordinary Python, Frida and task-owned private desktop; not newly generated REA Evidence",
        "originalExecutableSha256": "305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c",
        "privateRuntimeSha256": "38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89",
        "privatePatch": {"offset": "0x7c4", "length": 3, "before": "5589e5", "after": "b001c3"},
        "desktop": "KinSealStage1Native20261110", "globalInput": False, "usbOrPlc": False,
        "qtCallsGuiThreadOnly": True, "nativeCompilationPerformed": False,
        "currentRecipe": digest(OUT / "native_probe.py"),
        "recipeScope": "Hash of final reviewable recipe, not a claim that every earlier run used identical script bytes.",
        "samples": samples, "runs": runs,
        "modelMetadata": [digest(OUT / name) for name in ("model-metadata.json", "model-metadata.md", "model.dat.target.xml", "modelInfo.dat.target.xml")],
        "limits": [
            "Native open and save do not prove semantic editability or compilation support.",
            "Product-export/native-writer samples are mixed provenance, not native creation from blank and not historical missing oracles.",
            "English r1 preserves configuration mismatch as a negative; actual editor English r2 is a separate sample.",
            "Macro UI is disabled with an empty macro list; binding is not confirmed and semantic guard remains active.",
            "No successful Save As output exists. All successful resaves use task-owned source working copies and normal native save.",
        ],
    }
    (OUT / "manifest.json").write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    files = sorted(p for p in WORK.rglob("*") if p.is_file())
    cleanup = {
        "marker": "//20261110-5", "scope": WORK.relative_to(ROOT).as_posix(),
        "capturedAtUtc": manifest["capturedAtUtc"], "action": "record-only-no-deletion",
        "reason": "Earlier deletion denied by automatic approval policy; no retry and no alternate deletion channel.",
        "allRecordedOwnedProcessesTerminated": all(r["ownedProcessTerminated"] for r in runs),
        "runCount": len(runs), "retainedFileCount": len(files),
        "retainedBytes": sum(p.stat().st_size for p in files),
        "retainedFiles": [digest(p) for p in files],
        "historicalStage0AndS02FilesTouched": False,
    }
    (OUT / "cleanup.json").write_text(json.dumps(cleanup, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"samples": len(samples), "runs": len(runs), "retainedFiles": len(files), "retainedBytes": cleanup["retainedBytes"], "allRecordedOwnedProcessesTerminated": cleanup["allRecordedOwnedProcessesTerminated"]}, ensure_ascii=False))


if __name__ == "__main__":
    main()
