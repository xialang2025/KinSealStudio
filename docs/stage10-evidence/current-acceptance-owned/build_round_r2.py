#!/usr/bin/env python3
"""Extend immutable R1 with supplied function-page and final restore evidence."""
#20261114-9 add
from datetime import datetime, timezone
import argparse
import json
from pathlib import Path
import subprocess
import sys

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[2]
WORKSPACE = REPO.parents[1]
sys.dont_write_bytecode = True
sys.path.insert(0, str(REPO / "tools"))
import stage10_acceptance as gate


def main():
    """Append actual records; fail closed until final human observations exist."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--function-observation", action="append", required=True,
                        help="Actual recorded answer ID; repeat for separately observed function-page actions.")
    parser.add_argument("--restore-observation", required=True)
    args = parser.parse_args()
    target = HERE / "current-cases-r2.json"
    gate.require(not target.exists(), "round R2 already frozen")
    audit_at = datetime.now(timezone.utc).isoformat()
    owned = HERE / "round-r2"
    owned.mkdir(exist_ok=True)
    document = gate.parse_json((HERE / "current-cases-r1.json").read_bytes())
    artifacts = document["artifacts"]
    known_hashes = {item["sha256"]: name for name, item in artifacts.items()}

    def write(name, value):
        path = owned / name
        gate.require(not path.exists(), "round R2 file already exists: " + name)
        path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        return path

    def artifact(name, path):
        path = Path(path).resolve()
        gate.require(path.is_relative_to(WORKSPACE), "R2 artifact outside workspace")
        raw = path.read_bytes()
        sha = gate.digest(raw)
        if sha in known_hashes:
            return known_hashes[sha]
        retained = path
        if not path.is_relative_to(HERE) and path.suffix.lower() in (".json", ".md", ".txt", ".ksp") and len(raw) <= 1024 * 1024:
            retained = owned / (name + path.suffix)
            gate.require(not retained.exists(), "R2 snapshot already exists")
            retained.write_bytes(raw)
        artifacts[name] = {"path": str(retained.relative_to(WORKSPACE)).replace("\\", "/"), "sha256": sha}
        known_hashes[sha] = name
        return name

    observation_path = REPO / "docs/stage10-evidence/user-observations.json"
    observations = gate.parse_json(observation_path.read_bytes())
    answers = {item["id"]: item for item in observations["observations"]}
    # Exact IDs are supplied by the root recorder; do not infer answers from ACKs.
    function_answer_ids = args.function_observation
    restore_answer_id = args.restore_observation
    gate.require(all(item in answers for item in function_answer_ids + [restore_answer_id]), "final function and restore answers not yet recorded")
    raw_observation_id = artifact("r2-user-observations", observation_path)
    prepare_path = REPO / "docs/stage10-evidence/samples-r4/run.json"
    prepare = gate.parse_json(prepare_path.read_bytes())
    artifact("r2-prepare-run", prepare_path)
    artifact("r2-function-design", REPO / "docs/stage10-evidence/function-design-owned/derivation.json")
    artifact("r2-function-spec", REPO / "design_plan/功能按钮功能与交互复刻手册.md")
    local_results = []

    for prefix, sample, folder, answer_ids in (
            ("function", "function-local", "physical-function-r1", function_answer_ids),
            ("restore2", "restore-macro-14", "physical-restore-r2", [restore_answer_id])):
        run_path = REPO / "docs/stage10-evidence" / folder / "run.json"
        run = gate.parse_json(run_path.read_bytes())
        frozen = prepare["cases"][sample]
        gate.require(run.get("passed") is True and run.get("usbAccessed") is True and run["preparedIdentity"] == frozen,
                     "R2 prepare/download identity mismatch")
        source, executable, contract_path = (Path(frozen[key]) for key in ("source", "executable", "contract"))
        contract = gate.parse_json(contract_path.read_bytes())
        gate.require(gate.digest(source.read_bytes()) == frozen["sourceSha256"] == contract["identity"]["sourceSha256"], "R2 KSP identity mismatch")
        gate.require(gate.digest(executable.read_bytes()) == frozen["executableSha256"] and gate.digest(contract_path.read_bytes()) == frozen["contractSha256"], "R2 EXE/contract identity mismatch")
        source_manifest = executable.parent / "manifest.json"
        package = gate.parse_json(source_manifest.read_bytes())
        gate.require(package["executableSha256"] == frozen["executableSha256"], "R2 package identity mismatch")
        identity = {"sourceSha256": gate.digest(source_manifest.read_bytes()), "programSha256": frozen["executableSha256"],
                    "projectSha256": frozen["sourceSha256"], "payloadSha256": frozen["contractSha256"],
                    "model": contract["identity"]["targetModel"], "hardwareId": "unknown", "endpointId": observations["deviceEndpoint"],
                    "protocol": "KinSealUSB", "firmware": {"state": "unknown", "basis": "No actual screen firmware read or unique device serial supplied."}}
        document["identities"][prefix] = identity
        source_id = artifact(prefix + "-source-manifest", source_manifest)
        program_id = artifact(prefix + "-program", executable)
        project_id = artifact(prefix + "-project", source)
        contract_id = artifact(prefix + "-contract", contract_path)
        artifact(prefix + "-physical-run", run_path)
        row = run["runs"][0]
        gate.require(row["exitCode"] == 0 and row["arguments"] == ["--download-project", str(source)], "R2 actual download command mismatch")
        stdout = run_path.parent / row["stdout"]
        artifact(prefix + "-raw-download", stdout)
        receipt = gate._last_receipt(stdout.read_bytes())
        gate.require(receipt == run["finalResult"], "R2 raw/final result mismatch")
        audit = gate._native_transfer(receipt, contract)
        gate.require(audit["transportComplete"], "R2 native transfer incomplete")
        receipt_id = artifact(prefix + "-receipt", write(prefix + "-receipt.json", receipt))
        compiler_path = contract_path.parent / (sample + "-compiled/compiler-manifest.json")
        compiler = gate.parse_json(compiler_path.read_bytes())
        artifact(prefix + "-compiler-manifest", compiler_path)
        gate.require(compiler["complete"] is True and compiler["compilerExecutableSha256"] == frozen["executableSha256"] and
                     compiler["inputProjectSha256"] == contract["identity"]["modelSha256"], "R2 compile manifest mismatch")
        for segment in contract["segments"]:
            path = compiler_path.parent / segment["name"]
            raw = path.read_bytes()
            metadata = compiler["files"][segment["name"]]
            wire = len(raw).to_bytes(4, "big") + raw if segment["name"] == "upLoad.knr" else raw
            gate.require(metadata["length"] == len(raw) and metadata["sha256"] == gate.digest(raw) and
                         segment["wireBytes"] == len(wire) and segment["wireSha256"] == gate.digest(wire), "R2 compiled payload/wire mismatch")
            artifact(prefix + "-payload-" + segment["name"], path)
        for operation in ("compile", "verify", "contract"):
            matches = [item for item in prepare["runs"] if item["name"] == sample + "-" + operation]
            gate.require(len(matches) == 1 and matches[0]["exitCode"] == 0, "R2 actual compile/verify record missing")
        gate.require(all(answers[item]["transfer"] == folder + "/run.json" for item in answer_ids), "R2 user answer/download link mismatch")
        fact = {"format": "KinSealStage10-current-facts-v1", "round": "r2", "sample": sample, "identityId": prefix,
                "identitySha256": gate.canonical_digest(identity), "artifactAuditAtUtc": audit_at,
                "stimulusExactTime": "unknown; individual human stimulus timestamps were not recorded",
                "observationClockMeaning": "User recordedAt timestamps are answer receipt times, not invented stimulus/device timing.",
                "transportAudit": audit, "rawUserObservations": [answers[item] for item in answer_ids], "recoveryUserObservation": answers[restore_answer_id],
                "localBehaviorComparison": "Read the actual retained answer/confirmed/notCovered fields; this generator does not turn ACK or the presence of an answer into behavioral success.",
                "recoveryScope": "Known macro-14 test page re-download only; the retained user answer defines its visible result. No runtime values, RTC/history, power retention, macro execution, upload or original project backup claim.",
                "hardwareUniqueSerialKnown": False, "firmwareKnown": False, "strictTResult": "partial", "strictEResult": "partial"}
        fact_id = artifact(prefix + "-facts", write(prefix + "-facts.json", fact))
        local_results.append(fact)
        for layer, status in (("C", "pass"), ("T", "partial"), ("E", "partial")):
            case_id = "S10-CURRENT-" + prefix.upper() + "-" + layer
            execution = "host" if layer == "C" else "physical_hmi"
            definition = {"caseId": case_id, "identityId": prefix, "layer": layer, "execution": execution}
            document["requirements"].append(definition)
            proof = {"format": gate.PROOF_FORMAT, "caseId": case_id, "layer": layer, "execution": execution, "status": status,
                     "identitySha256": gate.canonical_digest(identity), "actor": "Codex read-only artifact auditor; actual user answer preserved",
                     "initialState": "Actual R2 prepare/download outputs and user answers already exist.",
                     "stimulusSteps": ["Read actual retained compile/verify/download command receipts.", "Rehash exact program, input, contract and all compiled wire bytes.", "Link actual human observation without inventing stimulation time."],
                     "expected": "Actual C outputs and wire identities match their successful command receipts; physical observation limited to recorded answer.",
                     "expectedMatched": True if layer in ("C", "T") else None,
                     "expectedSource": "compile-design" if layer == "C" else "acceptance-design",
                     "actual": "Current C receipts and bytes agree; transportComplete=true; raw user answer and unknown identities retained.",
                     "observedAt": audit_at, "observedAtMeaning": "Current read-only artifact audit time, not device/human timing",
                     "rawObservation": fact_id, "sourceArtifact": source_id, "programArtifact": program_id,
                     "projectArtifact": project_id, "payloadArtifact": contract_id, "transferReceipt": receipt_id, "transferContract": contract_id,
                     "independentObservation": True, "recovery": {"status": "state_unconfirmed", "action": "Re-download only known macro14 test project; strict physical identity remains unknown", "actual": answers[restore_answer_id]["answer"]},
                     "originalUserRecord": raw_observation_id}
            proof_id = artifact(prefix + "-proof-" + layer, write(prefix + "-proof-" + layer + ".json", proof))
            document["cases"].append({**definition, "status": status, "reason": "Exact current C artifact/command audit passed in narrow scope." if layer == "C" else
                                      "Actual full transport and selected user observation exist; unique hardware serial and on-device firmware remain unknown, so strict physical T/E pass is withheld.", "proof": proof_id})
    for case in document["cases"]:
        if case["caseId"] == "S10-02-OVERALL":
            case["reason"] += " R2 adds only the function page's recorded language/visibility/notification actions."
        elif case["caseId"] == "S10-03-OVERALL":
            case["status"] = "partial"
            case["reason"] = "R2 function page records selected language, LB41 display visibility and LB40 notification only; combined conditions/timing/touch permissions/overlap remain untested."
        elif case["caseId"] == "S10-04-OVERALL":
            case["status"] = "not_run"
            case["reason"] = "Internal multipage/configuration/device compile prerequisites are not implemented; this is not an external equipment failure."
        elif case["caseId"] == "S10-06-OVERALL":
            case["reason"] += " Final post-function restore is separately recorded; runtime data and power retention remain untested."
    document["round"] = "r2-function-and-final-restore"
    document["scopeNote"] = "R1 immutable results retained; R2 adds function-page and final known-test-project restore only; all eight stage tasks remain incomplete."
    target.write_text(json.dumps(document, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    summary = HERE / "current-summary-r2.json"
    command = [sys.executable, "-B", str(REPO / "tools/stage10_acceptance.py"), "validate", str(target), str(summary),
               "--root", str(WORKSPACE), "--require-complete"]
    process = subprocess.run(command, capture_output=True)
    gate.require(process.returncode == 3, "R2 gate expected exit 3: " + process.stderr.decode("utf-8"))
    value = gate.parse_json(summary.read_bytes())
    validation = {"round": "r2", "artifactAuditAtUtc": audit_at, "command": command, "exitCode": process.returncode,
                  "expectedExitCode": 3, "stdout": process.stdout.decode("utf-8"), "stderr": process.stderr.decode("utf-8"),
                  "input": target.name, "inputSha256": gate.digest(target.read_bytes()), "output": summary.name,
                  "outputSha256": gate.digest(summary.read_bytes()), "toolSha256": gate.digest((REPO / "tools/stage10_acceptance.py").read_bytes()),
                  "r1InputSha256": gate.digest((HERE / "current-cases-r1.json").read_bytes()), "counts": value["counts"], "status": value["status"],
                  "allEightTasksComplete": False, "newUsbAccessedByThisAuditor": False, "newPlcAccessedByThisAuditor": False,
                  "devicePowerInterruptedByThisAuditor": False, "stimulusTimesInvented": False,
                  "newRetainedNativeTransportCompleteCount": sum(fact["transportAudit"]["transportComplete"] for fact in local_results)}
    (HERE / "validation-r2.json").write_text(json.dumps(validation, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"round": "r2", "exitCode": 3, "counts": value["counts"], "inputSha256": validation["inputSha256"]}))


if __name__ == "__main__":
    main()
