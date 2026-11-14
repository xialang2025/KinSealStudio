"""Freeze read-only sample and historical recovery identities for Stage 10."""

import hashlib
import json
import re
from datetime import datetime, timezone
from pathlib import Path


def identity(path, root):  # //20261114-9 add
    """Return exact input bytes and SHA without executing or altering the input."""
    data = path.read_bytes()
    return {"path": path.relative_to(root).as_posix(), "bytes": len(data),
            "sha256": hashlib.sha256(data).hexdigest()}


def scalar_summary(value):  # //20261114-9 add
    """Preserve model scalars while omitting embedded pixels and large arrays."""
    if not isinstance(value, dict):
        return value
    summary = {}
    for key, item in value.items():
        if key in {"picturePng", "imagePng", "png", "data", "rawData"}:
            continue
        if isinstance(item, dict):
            summary[key] = scalar_summary(item)
        elif isinstance(item, list):
            summary[key + "Count"] = len(item)
        elif not isinstance(item, str) or len(item) < 512:
            summary[key] = item
    return summary


def save(path, value):  # //20261114-9 add
    """Write one owned JSON artifact, preserving all independent result fields."""
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def main():  # //20261114-9 add
    """Audit retained samples and wire identities; never access USB or a PLC."""
    root = Path(__file__).resolve().parents[3]
    output = Path(__file__).resolve().parent
    timestamp = datetime.now(timezone.utc).isoformat()
    files = [p for directory in ["static-text", "bit-button", "word-button", "lamp", "function-button"]
             for p in (root / "samples" / directory).glob("*.ksp")]
    files += [root / "samples/stage6" / (name + ".ksp") for name in ["static-10", "dynamic-12", "macro-14"]]
    files += [root / "samples/stage7/internal-five-family.ksp"]
    files += [root / "samples/stage5" / (name + ".ksp") for name in
              ["ellipse-default", "roundrect-default", "polygon-default", "table-default",
               "rectangle-default", "line-default", "freeline-default", "orthogonal", "sector-default"]]
    detailed = {"四操作实机验收.ksp", "七种操作实机验收.ksp", "七种数据类型.ksp",
                "指示灯实机联调.ksp", "多态U32三十二状态.ksp", "功能按钮实机联调.ksp",
                "外观与样式验证.ksp", "internal-five-family.ksp"}
    samples = []
    for path in sorted(set(files)):
        source = json.loads(path.read_text(encoding="utf-8-sig"))
        entry = identity(path, root)
        entry.update({key: source.get(key) for key in
                      ["format", "schemaVersion", "targetModel", "resolution", "languages"]})
        entry["pageCount"] = len(source.get("pages", []))
        entry["objectCount"] = sum(len(page.get("objects", [])) for page in source.get("pages", []))
        entry["macroCount"] = len(source.get("macros", []))
        if path.name in detailed:
            entry["objects"] = []
            for page_index, page in enumerate(source.get("pages", [])):
                for index, obj in enumerate(page.get("objects", [])):
                    selected = {key: scalar_summary(obj[key]) for key in
                                ["id", "type", "name", "text", "address", "x", "y", "width", "height",
                                 "bitButton", "wordButton", "lamp", "functionButton", "staticText"] if key in obj}
                    selected["sourcePointer"] = f"/pages/{page_index}/objects/{index}"
                    entry["objects"].append(selected)
        samples.append(entry)
    save(output / "sample-input-audit.json", {
        "schemaVersion": 1, "stage": 10, "observedAtUtc": timestamp,
        "purpose": "read-only input audit; no new C/T/E result",
        "baselineCommit": "daefcdb5287f97ec3190263d991881f3de9e1933",
        "auditRoot": str(root), "usbAccessed": False, "plcAccessed": False,
        "previousScratchArtifactsLost": True,
        "previousScratchResultsInherited": False,
        "note": "Inactive stored addresses are retained fields, not active access claims. Shared bitButton storage does not identify the actual control family.",
        "samples": samples})

    contract_path = root / "docs/stage6-evidence/physical-r2/macro-14-contract.json"
    contract = json.loads(contract_path.read_text(encoding="utf-8-sig"))
    trace_path = root / "docs/stage6-evidence/physical-r2/macro-14-download-stdout.txt"
    records = [json.loads(line) for line in trace_path.read_text(encoding="utf-8-sig").splitlines() if line.strip()]
    receipt = next(record for record in records if record.get("kind") == "transfer-result")
    payload_events = {event["segment"]: event for event in receipt["events"] if event.get("operation") == "payload-write"}
    payload_root = root / "docs/stage6-evidence/physical-r2/macro-14-compiled"
    payloads = []
    for segment in contract["segments"]:
        path = payload_root / segment["name"]
        entry = identity(path, root)
        data = path.read_bytes()
        if segment["snapshotOuterLengthBE32"]:
            data = len(data).to_bytes(4, "big") + data
        wire_sha = hashlib.sha256(data).hexdigest()
        event = payload_events[segment["name"]]
        entry.update({"name": segment["name"], "snapshotOuterLengthBE32": segment["snapshotOuterLengthBE32"],
                      "wireBytes": len(data), "wireSha256": wire_sha, "contractWireSha256": segment["wireSha256"],
                      "historicalPayloadEventSequence": event["sequence"], "historicalPayloadEventSha256": event["sha256"],
                      "matchesContract": wire_sha == segment["wireSha256"] and len(data) == segment["wireBytes"],
                      "matchesHistoricalPayloadWrite": wire_sha == event["sha256"] and len(data) == event["returned"] and event["accepted"]})
        payloads.append(entry)
    source = identity(root / "samples/stage6/macro-14.ksp", root)
    recovery = {"schemaVersion": 1, "stage": 10, "observedAtUtc": timestamp, "auditRoot": str(root),
                "purpose": "historical test-project recovery input audit; not HMI readback",
                "previousScratchArtifactsLost": True, "previousScratchResultsInherited": False,
                "currentHardwareAccessPerformed": False, "currentScreenIdentityObservedByThisAuditor": False,
                "source": source, "sourceMatchesHistoricalContract": source["sha256"] == contract["identity"]["sourceSha256"],
                "historicalContract": identity(contract_path, root), "historicalReceipt": identity(trace_path, root),
                "historicalProgramSha256": "7dd94b3ccb01b3fa695c773cf6a4e59bbb47ee384277ef242cded1233188c2ef",
                "targetModelDeclaredByContract": contract["identity"]["targetModel"], "firmwareIdentity": contract["identity"]["firmwareVersion"],
                "historicalStartOptionsHex": receipt["startOptionsHex"],
                "historicalReceiptFields": {key: receipt[key] for key in
                                           ["ok", "transferAccepted", "commitAccepted", "resetWritten", "postResetReadCompleted",
                                            "postResetReadResult", "releaseResult", "closeResult", "screenDisplayAndTouchConfirmed"]},
                "payloads": payloads,
                "allFourteenWirePayloadsMatch": len(payloads) == 14 and all(p["matchesContract"] and p["matchesHistoricalPayloadWrite"] for p in payloads),
                "limits": ["No original editable HMI project was uploaded or backed up.",
                           "A visible page marker does not identify all on-device bytes or runtime values.",
                           "Re-downloading the known test project does not restore LB/LW/RTC/history runtime state.",
                           "Current EXE recompilation and actual recovery downloads need separate receipts.",
                           "Start options 000000 do not prove data retention semantics."]}
    save(output / "recovery-input-audit.json", recovery)

    matrix = json.loads((output / "task-matrix.json").read_text(encoding="utf-8"))
    document = root / "docs/阶段10现场验收矩阵.md"
    document_text = document.read_text(encoding="utf-8")
    checks = [{"name": "eight_unique_tasks", "passed": len(matrix["tasks"]) == 8 and len({x["id"] for x in matrix["tasks"]}) == 8}]
    checks += [{"name": "sample_identity", "path": x["path"], "passed": hashlib.sha256((root / x["path"]).read_bytes()).hexdigest() == x["sha256"]} for x in samples]
    checks += [{"name": "case_in_human_matrix", "caseId": case_id, "passed": case_id in document_text}
               for task in matrix["tasks"] for case_id in task.get("caseIds", [])]
    checks += [{"name": "local_link_exists", "href": href, "passed": (root / "docs" / href.split("#")[0]).resolve().exists()}
               for href in re.findall(r"\]\(([^)]+)\)", document_text)]
    checks += [{"name": "fourteen_wire_and_trace_identities_match", "passed": recovery["allFourteenWirePayloadsMatch"]},
               {"name": "source_matches_historical_contract", "passed": recovery["sourceMatchesHistoricalContract"]},
               {"name": "runtime_results_match_recorded_slices", "passed": all(
                   t.get("eResult", t.get("hmiEResult")) ==
                   ({"S10-02": "partial_pass", "S10-03": "partial_pass", "S10-06": "partial_pass"}.get(t["id"], "not_run"))
                   for t in matrix["tasks"]) and not matrix["allStage10TasksComplete"]}]
    report = {"schemaVersion": 1, "observedAtUtc": timestamp, "checks": checks,
              "passed": all(x["passed"] for x in checks), "checkCount": len(checks),
              "productionOrDeviceMutations": False,
              "files": [identity(p, root) for p in [Path(__file__), output / "task-matrix.json", output / "sample-input-audit.json", output / "recovery-input-audit.json", document]]}
    save(output / "validation-reproduced.json", report)
    print(json.dumps({"sampleCount": len(samples), "allFourteenWirePayloadsMatch": recovery["allFourteenWirePayloadsMatch"],
                      "sourceMatchesHistoricalContract": recovery["sourceMatchesHistoricalContract"],
                      "checks": len(checks), "passed": report["passed"], "failed": [x for x in checks if not x["passed"]]}, ensure_ascii=False))


if __name__ == "__main__":
    main()
