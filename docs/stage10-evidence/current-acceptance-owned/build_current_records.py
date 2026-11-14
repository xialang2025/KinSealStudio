#!/usr/bin/env python3
"""Bind existing current-device receipts and user answers without performing I/O."""
#20261114-9 add
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import subprocess
import sys

REPOSITORY = Path(__file__).resolve().parents[3]
WORKSPACE = REPOSITORY.parents[1]
OWNED = Path(__file__).resolve().parent
sys.dont_write_bytecode = True
sys.path.insert(0, str(REPOSITORY / "tools"))
import stage10_acceptance as gate


def write(name, value):
    """Write only this task's owned, reviewable JSON artifacts."""
    path = OWNED / name
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return path


def build():
    """Recheck actual files and preserve unknown hardware/timing rather than filling them."""
    gate.require(not (OWNED / "current-cases-r1.json").exists(), "R1 already frozen; use its immutable records or a new round")
    recorded_at = datetime.now(timezone.utc).isoformat()
    artifacts, identities, requirements, cases, local = {}, {}, [], [], []
    artifact_paths = {}

    def artifact(name, path):
        path = Path(path).resolve()
        gate.require(path.is_relative_to(WORKSPACE), "input outside workspace")
        if path in artifact_paths:
            return artifact_paths[path]
        artifacts[name] = {"path": str(path.relative_to(WORKSPACE)).replace("\\", "/"),
                           "sha256": gate.digest(path.read_bytes())}
        artifact_paths[path] = name
        return name

    observation_path = REPOSITORY / "docs/stage10-evidence/user-observations.json"
    observations = gate.parse_json(observation_path.read_bytes())
    observation_map = {item["id"]: item for item in observations["observations"]}
    artifact("user-observations", observation_path)
    prepare_path = REPOSITORY / "docs/stage10-evidence/samples-r3/run.json"
    prepare = gate.parse_json(prepare_path.read_bytes())
    artifact("prepare-run", prepare_path)
    artifact("acceptance-design", REPOSITORY / "design_plan/A06_运行与发布验收用例.md")
    artifact("compile-design", REPOSITORY / "design_plan/A05_源格式与设备编译契约.md")
    artifact("bit-spec", REPOSITORY / "design_plan/位按钮功能与交互复刻手册.md")
    artifact("word-spec", REPOSITORY / "design_plan/字按钮功能与交互复刻手册.md")
    artifact("stage10-taskbook", REPOSITORY / "design_plan/11_阶段10_设备运行兼容与性能验收.md")

    def add_case(name, identity_id, layer, execution, status, reason, proof_id=None):
        definition = {"caseId": name, "identityId": identity_id, "layer": layer, "execution": execution}
        requirements.append(definition)
        case = {**definition, "status": status, "reason": reason}
        if proof_id:
            case["proof"] = proof_id
        cases.append(case)

    for prefix, sample, folder, observation_ids in (
            ("bit", "bit-lamp-local", "physical-bit-r1", ["bit-display", "bit-touch"]),
            ("word", "word-lamp-local", "physical-word-r1", ["word-display-touch"]),
            ("restore", "restore-macro-14", "physical-restore-r1", ["restore-macro-display"])):
        run_path = REPOSITORY / "docs/stage10-evidence" / folder / "run.json"
        run = gate.parse_json(run_path.read_bytes())
        frozen = prepare["cases"][sample]
        gate.require(run["preparedIdentity"] == frozen and run["passed"] is True and
                     run["usbAccessed"] is True, "prepared/download identity mismatch")
        source = Path(frozen["source"])
        executable = Path(frozen["executable"])
        contract_path = Path(frozen["contract"])
        contract = gate.parse_json(contract_path.read_bytes())
        gate.require(gate.digest(source.read_bytes()) == frozen["sourceSha256"] == contract["identity"]["sourceSha256"], "current KSP hash mismatch")
        gate.require(gate.digest(executable.read_bytes()) == frozen["executableSha256"], "current EXE hash mismatch")
        gate.require(gate.digest(contract_path.read_bytes()) == frozen["contractSha256"], "current contract hash mismatch")
        source_manifest = executable.parent / "manifest.json"
        package_manifest = gate.parse_json(source_manifest.read_bytes())
        gate.require(package_manifest["executableSha256"] == frozen["executableSha256"], "package EXE identity mismatch")
        identity = {"sourceSha256": gate.digest(source_manifest.read_bytes()),
                    "programSha256": frozen["executableSha256"], "projectSha256": frozen["sourceSha256"],
                    "payloadSha256": frozen["contractSha256"], "model": contract["identity"]["targetModel"],
                    "hardwareId": "unknown", "endpointId": observations["deviceEndpoint"], "protocol": "KinSealUSB",
                    "firmware": {"state": "unknown", "basis": "user-observations.json firmwareVersion=not-observed; no on-device version read"}}
        identities[prefix] = identity
        source_id = artifact(prefix + "-source-manifest", source_manifest)
        program_id = artifact(prefix + "-program", executable)
        project_id = artifact(prefix + "-project", source)
        contract_id = artifact(prefix + "-contract", contract_path)
        artifact(prefix + "-physical-run", run_path)
        run_entry = run["runs"][0]
        gate.require(run_entry["exitCode"] == 0 and run_entry["arguments"] == ["--download-project", str(source)], "download command receipt mismatch")
        raw_stdout_path = run_path.parent / run_entry["stdout"]
        artifact(prefix + "-raw-download", raw_stdout_path)
        receipt = gate._last_receipt(raw_stdout_path.read_bytes())
        gate.require(receipt == run["finalResult"], "raw/final receipt mismatch")
        transport = gate._native_transfer(receipt, contract)
        gate.require(transport["transportComplete"], "current transport incomplete")
        receipt_id = artifact(prefix + "-receipt", write(prefix + "-receipt.json", receipt))

        compiler_manifest_path = contract_path.parent / (sample + "-compiled/compiler-manifest.json")
        compiler = gate.parse_json(compiler_manifest_path.read_bytes())
        artifact(prefix + "-compiler-manifest", compiler_manifest_path)
        gate.require(compiler["complete"] is True and compiler["compilerExecutableSha256"] == frozen["executableSha256"] and
                     compiler["inputProjectSha256"] == contract["identity"]["modelSha256"], "compiler identity mismatch")
        for segment in contract["segments"]:
            path = compiler_manifest_path.parent / segment["name"]
            raw = path.read_bytes()
            metadata = compiler["files"][segment["name"]]
            wire = len(raw).to_bytes(4, "big") + raw if segment["name"] == "upLoad.knr" else raw
            gate.require(metadata["length"] == len(raw) and metadata["sha256"] == gate.digest(raw) and
                         segment["wireBytes"] == len(wire) and segment["wireSha256"] == gate.digest(wire), "compiled payload/wire mismatch")
            artifact(prefix + "-payload-" + segment["name"], path)
        for operation in ("compile", "verify", "contract"):
            row = [item for item in prepare["runs"] if item["name"] == sample + "-" + operation]
            gate.require(len(row) == 1 and row[0]["exitCode"] == 0, "current compile/verify run missing")

        selected_observations = [observation_map[item] for item in observation_ids]
        fact = {"format": "KinSealStage10-current-facts-v1", "sample": sample, "identityId": prefix,
                "identitySha256": gate.canonical_digest(identity), "artifactAuditAtUtc": recorded_at,
                "stimulusExactTime": "unknown; no per-touch timestamps were recorded",
                "observationClockMeaning": "User record timestamps are answer-recording times, not invented stimulus/device times.",
                "transportAudit": transport, "localDisplayAndSelectedBehaviorConfirmedByUser": True,
                "rawUserObservations": selected_observations,
                "recoveryUserObservation": observation_map["restore-macro-display"],
                "recoveryScope": "Known macro-14 test project re-downloaded and marker/button visible; no runtime values, RTC, history, native upload, original project backup, macro execution or power retention claim.",
                "hardwareUniqueSerialKnown": False, "firmwareKnown": False,
                "sourceManifestMeaning": "Hash of original executable package source/build manifest; not the KSP hash or current edited source tree.",
                "sourceCodeCommitAsRecorded": package_manifest.get("codeCommit", package_manifest.get("sourceCommit", "unknown")),
                "nativeRawSourceSha256": contract["identity"]["sourceSha256"],
                "normalizedProjectSha256": contract["identity"]["modelSha256"],
                "strictTResult": "partial", "strictEResult": "partial",
                "limits": ["Unique hardware serial not observed; USB instance path is an endpoint identity only.",
                           "Firmware version not read; no cross-firmware or cross-device claim.",
                           "Only the user's explicitly listed stimuli and independent lamp/display states are confirmed."]}
        fact_id = artifact(prefix + "-facts", write(prefix + "-facts.json", fact))
        local.append(fact)

        # This is a newly timestamped read-only audit of actual C receipts/bytes,
        # not a fabricated timestamp for the earlier compiler or human touches.
        for layer, status in (("C", "pass"), ("T", "partial"), ("E", "partial")):
            case_id = "S10-CURRENT-" + prefix.upper() + "-" + layer
            proof = {"format": gate.PROOF_FORMAT, "caseId": case_id, "layer": layer,
                     "execution": "host" if layer == "C" else "physical_hmi", "status": status,
                     "identitySha256": gate.canonical_digest(identity), "actor": "Codex read-only artifact auditor; user observations retained verbatim",
                     "initialState": "Frozen samples-r3 prepare outputs and exact physical download receipt already exist.",
                     "stimulusSteps": ["Read the retained prepare/download command receipts.", "Rehash the actual EXE, KSP, contract and every compiled block/wire payload.", "Link the user's actual answers without inventing touch or device timestamps."],
                     "expected": "Exact sample input/executable/compiled and wire identities match the successful retained compiler/verify/transfer receipts; physical behavior limited to the recorded answers.",
                     "expectedMatched": True, "expectedSource": "compile-design" if layer == "C" else "acceptance-design",
                     "actual": "C receipts and bytes match; native transportComplete=true; user answers as preserved in " + prefix + "-facts.json; unique hardware serial and firmware remain unknown.",
                     "observedAt": recorded_at, "observedAtMeaning": "time of this read-only artifact audit; not a human stimulus or device timing measurement",
                     "rawObservation": fact_id, "sourceArtifact": source_id, "programArtifact": program_id,
                     "projectArtifact": project_id, "payloadArtifact": contract_id,
                     "transferReceipt": receipt_id, "transferContract": contract_id,
                     "independentObservation": True, "recovery": {"status": "pass", "action": "Re-download only the known macro-14 test project.", "actual": observation_map["restore-macro-display"]["answer"]},
                     "originalUserRecord": "user-observations", "unknownHardwareSerial": True, "unknownFirmware": True}
            proof_id = artifact(prefix + "-proof-" + layer, write(prefix + "-proof-" + layer + ".json", proof))
            reason = ("Successful current compile/verify receipts and exact EXE/KSP/contract/compiled byte audit; limited C scope."
                      if layer == "C" else "Actual transfer completed and selected observations exist, but strict physical identity requires a unique hardware serial; current USB instance is not one. No full T/E pass or cross-firmware claim.")
            add_case(case_id, prefix, layer, "host" if layer == "C" else "physical_hmi", status, reason, proof_id)

    task_rows = (
        ("S10-01", "partial", "Endpoint/model and known test-project restore are recorded; unique hardware serial/firmware and original screen project backup remain unknown."),
        ("S10-02", "partial", "Bit toggle/set/reset/hold-release and selected LW20/LW30 constant-write lamp states confirmed; full five-control/type/boundary matrix remains open."),
        ("S10-03", "not_run", "Permissions, notification, pressure timing, language and overlap field axes not exercised in this physical session."),
        ("S10-04", "blocked_external", "Multipage/configuration/device compile prerequisites and broader macro scheduling/data subsystems are not restored."),
        ("S10-05", "blocked_external", "No dedicated real PLC endpoint/register test zone or HMI PLC execution evidence is supplied."),
        ("S10-06", "partial", "Known macro-14 test page re-downloaded and visually restored; upload/original backup/power/RTC/history/runtime-value retention remain not_run."),
        ("S10-07", "partial", "Host performance measurements exist separately; native/product thresholds, HMI latency and actual long-duration operation are not closed."),
        ("S10-08", "partial", "Evidence gate and limits exist; declaration does not close all required device/firmware/protocol/layer cases."))
    for task, status, reason in task_rows:
        add_case(task + "-OVERALL", "bit", "E" if task != "S10-08" else "D", "physical_hmi" if task != "S10-08" else "host", status, reason)
    document = {"format": gate.FORMAT, "identities": identities, "artifacts": artifacts,
                "requirements": requirements, "cases": cases,
                "scopeNote": "Three current sample identities; eight tasks remain incomplete. Unknown device serial/firmware do not erase the user's positive local observations, but cannot be silently upgraded to strict T/E pass."}
    manifest = write("current-cases.json", document)
    output = OWNED / "current-summary.json"
    command = [sys.executable, "-B", str(REPOSITORY / "tools/stage10_acceptance.py"), "validate",
               str(manifest), str(output), "--root", str(WORKSPACE), "--require-complete"]
    process = subprocess.run(command, capture_output=True)
    gate.require(process.returncode == 3, "expected incomplete release gate exit 3: " + process.stderr.decode("utf-8"))
    result = gate.parse_json(output.read_bytes())
    write("validation.json", {"recordedAtUtc": recorded_at, "command": command, "exitCode": process.returncode,
                              "expectedExitCode": 3, "stdout": process.stdout.decode("utf-8"), "stderr": process.stderr.decode("utf-8"),
                              "currentCasesSha256": gate.digest(manifest.read_bytes()), "currentSummarySha256": gate.digest(output.read_bytes()),
                              "frozenToolSha256": gate.digest((REPOSITORY / "tools/stage10_acceptance.py").read_bytes()),
                              "newUsbAccessedByThisAuditor": False, "newPlcAccessedByThisAuditor": False, "devicePowerInterruptedByThisAuditor": False,
                              "counts": result["counts"], "status": result["status"], "allEightTasksComplete": False,
                              "localNativeTransportCompleteCount": sum(item["transportAudit"]["transportComplete"] for item in local),
                              "localSelectedUserObservationCount": len(local), "hardwareSerialKnown": False, "firmwareKnown": False,
                              "humanStimulusExactTimesRecorded": False})
    print(json.dumps({"exitCode": process.returncode, "counts": result["counts"], "status": result["status"],
                      "transportComplete": len(local), "currentCasesSha256": gate.digest(manifest.read_bytes())}, ensure_ascii=False))


if __name__ == "__main__":
    build()
