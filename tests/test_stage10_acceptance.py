#!/usr/bin/env python3
"""Adversarial acceptance evidence tests; no device, network or GUI access."""
#20261114-9 add
import copy
import contextlib
import io
import json
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock

sys.dont_write_bytecode = True
REPO = Path(os.environ.get("KINSEAL_STAGE10_REPOSITORY", Path(__file__).resolve().parents[1]))
sys.path.insert(0, str(REPO / "tools"))
import stage10_acceptance as acceptance


class AcceptanceTests(unittest.TestCase):
    """Prove claim boundaries with independent artifacts and actual historical records."""

    def setUp(self):
        directory = os.environ.get("KINSEAL_STAGE10_TEST_ROOT")
        if not directory:
            raise RuntimeError("set KINSEAL_STAGE10_TEST_ROOT to an owned Z: directory")
        self.temp = tempfile.TemporaryDirectory(prefix="acceptance-", dir=directory)
        self.root = Path(self.temp.name)
        self.addCleanup(self.temp.cleanup)
        self.artifacts = {}
        self.identity = {"sourceSha256": "", "programSha256": "", "projectSha256": "", "payloadSha256": "",
                         "model": "AMT070W124-L800X480R", "hardwareId": "fixture-hmi-1", "endpointId": "fixture-usb-1",
                         "protocol": "KinSealUSB", "firmware": {"state": "unknown", "basis": "not readable in fixture"}}
        for key in ("source", "program", "project", "payload"):
            self.artifact(key, (key + " known bytes").encode())
            self.identity[key + "Sha256"] = self.artifacts[key]["sha256"]
        self.artifact("spec", b"An independently retained expected contract")
        self.artifact("raw", b"Observed source/output changes")
        self.case = {"caseId": "C-01", "layer": "C", "execution": "host", "identityId": "one", "status": "pass", "reason": "verified output"}
        self.proof = {"format": acceptance.PROOF_FORMAT, "caseId": "C-01", "layer": "C", "execution": "host", "status": "pass",
                      "identitySha256": acceptance.canonical_digest(self.identity), "actor": "fixture executor", "initialState": "empty output",
                      "stimulusSteps": ["Compile exact project bytes"], "expected": "output matches", "actual": "output matched",
                      "observedAt": "2026-11-14T12:00:00+00:00", "expectedSource": "spec", "rawObservation": "raw",
                      "expectedMatched": True,
                      "sourceArtifact": "source", "programArtifact": "program", "projectArtifact": "project", "payloadArtifact": "payload"}
        self.save_proof()

    def artifact(self, name, data):
        """Write fixture bytes and retain the independently calculated file digest."""
        if not isinstance(data, bytes):
            data = (json.dumps(data, ensure_ascii=False) + "\n").encode()
        path = self.root / (name + ".json")
        path.write_bytes(data)
        self.artifacts[name] = {"path": path.name, "sha256": acceptance.digest(data)}

    def save_proof(self):
        self.artifact("proof", self.proof)
        self.case["proof"] = "proof"

    def document(self):
        return {"format": acceptance.FORMAT, "identities": {"one": self.identity}, "artifacts": self.artifacts,
                "requirements": [{key: self.case[key] for key in ("caseId", "layer", "execution", "identityId")}], "cases": [self.case]}

    def run_document(self):
        return acceptance.evaluate(self.document(), self.root)[0]

    def native(self):
        base = REPO / "docs/stage6-evidence/physical-r1"
        receipt = acceptance._last_receipt((base / "static-10-download-stdout.txt").read_bytes())
        contract = json.loads((base / "static-10-contract.json").read_text(encoding="utf-8"))
        return receipt, contract

    def configure_transport(self, execution="mock"):
        """Bind a synthetic declaration to copied historical wire bytes for schema tests only."""
        receipt, contract = self.native()
        contract["identity"]["sourceSha256"] = self.identity["projectSha256"]
        self.artifact("contract", contract)
        self.artifact("receipt", receipt)
        self.identity["payloadSha256"] = self.artifacts["contract"]["sha256"]
        self.case["caseId"] = self.proof["caseId"] = "T-01"
        self.case["layer"] = self.proof["layer"] = "T"
        self.case["execution"] = self.proof["execution"] = execution
        self.proof["identitySha256"] = acceptance.canonical_digest(self.identity)
        self.proof["payloadArtifact"] = self.proof["transferContract"] = "contract"
        self.proof["transferReceipt"] = "receipt"
        self.save_proof()

    def test_host_compile_pass_does_not_establish_physical_transport_or_E(self):
        result = self.run_document()
        self.assertEqual(result["status"], "pass")
        self.assertFalse(result["fullReplicaCertified"])
        self.assertFalse(result["results"][0]["physicalTransportPassed"])
        self.assertFalse(result["results"][0]["hmiExecutionPassed"])
        self.assertTrue(result["releaseRestrictions"])

    def test_all_seven_layers_are_distinct_and_unknown_layer_rejected(self):
        self.assertEqual(acceptance.LAYERS, set("DUPRCTE"))
        self.case["layer"] = "UPRC"
        with self.assertRaises(acceptance.AcceptanceError):
            self.run_document()

    def test_absent_required_result_is_not_run(self):
        document = self.document()
        document["cases"] = []
        self.assertEqual(acceptance.evaluate(document, self.root)[0]["status"], "not_run")

    def test_empty_coverage_cannot_pass(self):
        document = self.document()
        document["requirements"] = []
        with self.assertRaises(acceptance.AcceptanceError):
            acceptance.evaluate(document, self.root)

    def test_all_nonpass_states_retained(self):
        for state in ("not_run", "blocked_external", "partial", "fail"):
            with self.subTest(state=state):
                self.case["status"] = self.proof["status"] = state
                self.save_proof()
                self.assertEqual(self.run_document()["results"][0]["status"], state)

    def test_mixed_pass_and_blocked_is_partial(self):
        document = self.document()
        definition = {"caseId": "E-02", "layer": "E", "execution": "physical_hmi", "identityId": "one"}
        document["requirements"].append(definition)
        document["cases"].append({**definition, "status": "blocked_external", "reason": "no observer"})
        self.assertEqual(acceptance.evaluate(document, self.root)[0]["status"], "partial")

    def test_evidence_tamper_is_rejected(self):
        (self.root / "raw.json").write_bytes(b"replaced")
        with self.assertRaisesRegex(acceptance.AcceptanceError, "hash mismatch"):
            self.run_document()

    def test_claim_identity_cannot_be_swapped(self):
        self.identity["programSha256"] = "f" * 64
        with self.assertRaisesRegex(acceptance.AcceptanceError, "proof identity"):
            self.run_document()

    def test_known_identity_must_match_actual_artifact(self):
        self.identity["programSha256"] = "f" * 64
        self.proof["identitySha256"] = acceptance.canonical_digest(self.identity)
        self.save_proof()
        with self.assertRaisesRegex(acceptance.AcceptanceError, "identity/artifact"):
            self.run_document()

    def test_unknown_source_cannot_pass(self):
        self.identity["sourceSha256"] = "unknown"
        self.proof["identitySha256"] = acceptance.canonical_digest(self.identity)
        self.save_proof()
        with self.assertRaisesRegex(acceptance.AcceptanceError, "exact source"):
            self.run_document()

    def test_missing_actual_stimulus_spec_or_observation_cannot_pass(self):
        for key in ("actual", "initialState", "actor", "expected", "stimulusSteps", "expectedSource", "rawObservation"):
            with self.subTest(key=key):
                old = self.proof.pop(key)
                self.save_proof()
                with self.assertRaises(acceptance.AcceptanceError):
                    self.run_document()
                self.proof[key] = old

    def test_naive_timestamp_rejected(self):
        self.proof["observedAt"] = "2026-11-14T12:00:00"
        self.save_proof()
        with self.assertRaises(acceptance.AcceptanceError):
            self.run_document()

    def test_recorded_mismatch_cannot_be_declared_pass(self):
        self.proof["expectedMatched"] = False
        self.save_proof()
        with self.assertRaisesRegex(acceptance.AcceptanceError, "expectation match"):
            self.run_document()

    def test_duplicate_and_undeclared_cases_rejected(self):
        document = self.document()
        document["cases"].append(copy.deepcopy(self.case))
        with self.assertRaises(acceptance.AcceptanceError):
            acceptance.evaluate(document, self.root)
        document["cases"][-1]["caseId"] = "fake"
        with self.assertRaises(acceptance.AcceptanceError):
            acceptance.evaluate(document, self.root)

    def test_requirement_identity_cannot_be_changed(self):
        document = self.document()
        document["requirements"][0]["execution"] = "physical_hmi"
        with self.assertRaises(acceptance.AcceptanceError):
            acceptance.evaluate(document, self.root)

    def test_mock_cannot_be_E(self):
        self.case["layer"] = self.proof["layer"] = "E"
        self.case["execution"] = self.proof["execution"] = "mock"
        self.save_proof()
        with self.assertRaisesRegex(acceptance.AcceptanceError, "physical HMI"):
            self.run_document()

    def test_mock_transport_pass_is_explicitly_controlled_only(self):
        self.configure_transport()
        result = self.run_document()
        self.assertEqual(result["status"], "pass")
        self.assertTrue(result["results"][0]["transfer"]["transportComplete"])
        self.assertFalse(result["results"][0]["physicalTransportPassed"])
        self.assertFalse(result["results"][0]["hmiExecutionPassed"])
        self.assertTrue(any("controlled" in row["reason"] for row in result["releaseRestrictions"]))

    def test_transfer_contract_cannot_rebind_project_or_model(self):
        self.configure_transport()
        for key in ("sourceSha256", "targetModel"):
            with self.subTest(key=key):
                contract = json.loads((self.root / "contract.json").read_text())
                contract["identity"][key] = "wrong"
                self.artifact("contract", contract)
                self.identity["payloadSha256"] = self.artifacts["contract"]["sha256"]
                self.proof["identitySha256"] = acceptance.canonical_digest(self.identity)
                self.save_proof()
                with self.assertRaisesRegex(acceptance.AcceptanceError, "transfer/"):
                    self.run_document()

    def test_complete_synthetic_C_T_E_can_pass_only_for_one_unknown_firmware_endpoint(self):
        self.configure_transport("physical_hmi")
        document = self.document()
        for case_id, layer in (("C-01", "C"), ("E-01", "E")):
            proof = copy.deepcopy(self.proof)
            proof.update({"caseId": case_id, "layer": layer, "independentObservation": True,
                          "expectedMatched": True, "recovery": {"status": "not_required", "action": "no stateful recovery for this display-only case", "actual": "no recovery action required"}})
            proof_id = "proof-" + layer
            self.artifact(proof_id, proof)
            case = {**self.case, "caseId": case_id, "layer": layer, "proof": proof_id}
            document["requirements"].append({key: case[key] for key in ("caseId", "layer", "execution", "identityId")})
            document["cases"].append(case)
        result = acceptance.evaluate(document, self.root)[0]
        self.assertEqual(result["status"], "pass")
        self.assertTrue(next(row for row in result["results"] if row["layer"] == "E")["hmiExecutionPassed"])
        self.assertFalse(result["fullReplicaCertified"])
        self.assertTrue(all("firmware unknown" in row["limits"][0] for row in result["results"]))

    def test_unknown_hardware_cannot_be_physical_T(self):
        self.configure_transport("physical_hmi")
        self.identity["hardwareId"] = "unknown"
        self.proof["identitySha256"] = acceptance.canonical_digest(self.identity)
        self.save_proof()
        with self.assertRaisesRegex(acceptance.AcceptanceError, "physical T"):
            self.run_document()

    def test_hmi_E_needs_independent_observation_recovery_and_same_C_T(self):
        self.case["layer"] = self.proof["layer"] = "E"
        self.case["execution"] = self.proof["execution"] = "physical_hmi"
        self.save_proof()
        with self.assertRaisesRegex(acceptance.AcceptanceError, "independent"):
            self.run_document()
        self.proof["independentObservation"] = True
        self.save_proof()
        with self.assertRaisesRegex(acceptance.AcceptanceError, "recovery"):
            self.run_document()
        self.proof["recovery"] = {"status": "state_unconfirmed", "action": "no power action", "actual": "unconfirmed"}
        self.save_proof()
        with self.assertRaisesRegex(acceptance.AcceptanceError, "recovery"):
            self.run_document()
        self.proof["recovery"]["status"] = "not_required"
        self.proof["expectedMatched"] = True
        self.save_proof()
        with self.assertRaisesRegex(acceptance.AcceptanceError, "same-identity C"):
            self.run_document()

    def test_unknown_hardware_cannot_be_E(self):
        self.identity["hardwareId"] = "unknown"
        self.proof["identitySha256"] = acceptance.canonical_digest(self.identity)
        self.case["layer"] = self.proof["layer"] = "E"
        self.case["execution"] = self.proof["execution"] = "physical_hmi"
        self.save_proof()
        with self.assertRaisesRegex(acceptance.AcceptanceError, "hardware"):
            self.run_document()

    def test_actual_stage6_transport_not_E(self):
        receipt, contract = self.native()
        result = acceptance._native_transfer(receipt, contract)
        self.assertTrue(result["transportComplete"])
        self.assertFalse(result["hmiExecutionConfirmed"])
        self.assertFalse(result["automaticRollbackProven"])
        self.assertFalse(result["powerRetentionProven"])

    def test_native_success_flag_cannot_hide_wrong_ack_payload_or_order(self):
        receipt, contract = self.native()
        for mutation in ("ack", "payload", "missing", "order", "close", "firstError"):
            with self.subTest(mutation=mutation):
                value = copy.deepcopy(receipt)
                if mutation == "ack":
                    next(event for event in value["events"] if event["operation"] == "reply-read")["bytesHex"] = "00" * 24
                elif mutation == "payload":
                    next(event for event in value["events"] if event["operation"] == "payload-write")["sha256"] = "f" * 64
                elif mutation == "missing":
                    value["events"].pop(-3)
                elif mutation == "order":
                    value["events"][2], value["events"][3] = value["events"][3], value["events"][2]
                elif mutation == "close":
                    value["closed"] = False
                else:
                    value["firstError"]["present"] = True
                with self.assertRaises(acceptance.AcceptanceError):
                    acceptance._native_transfer(value, contract)

    def test_success_flags_cannot_replace_missing_commit_or_reset_wire_evidence(self):
        receipt, contract = self.native()
        for phase in ("commit", "reset"):
            with self.subTest(phase=phase):
                changed_receipt = copy.deepcopy(receipt)
                changed_contract = copy.deepcopy(contract)
                changed_receipt["events"] = [item for item in changed_receipt["events"] if item["phase"] != phase]
                changed_contract["operations"] = [item for item in changed_contract["operations"] if item["name"] != phase]
                for index, item in enumerate(changed_receipt["events"], 1):
                    item["sequence"] = index
                for index, item in enumerate(changed_contract["operations"], 1):
                    item["sequence"] = index
                with self.assertRaises(acceptance.AcceptanceError):
                    acceptance._native_transfer(changed_receipt, changed_contract)

    def test_contract_cannot_omit_payloads_segment_ack_or_segment_binding(self):
        receipt, contract = self.native()
        for mutation in ("all_payloads", "one_status", "segment_hash", "payload_count", "ack_length"):
            with self.subTest(mutation=mutation):
                changed_receipt = copy.deepcopy(receipt)
                changed_contract = copy.deepcopy(contract)
                if mutation == "all_payloads":
                    changed_contract["operations"] = changed_contract["operations"][:4] + changed_contract["operations"][-3:]
                    changed_receipt["events"] = [item for item in changed_receipt["events"] if item["phase"] not in ("segment-command", "segment-data", "segment-status")]
                elif mutation == "one_status":
                    changed_contract["operations"] = [item for item in changed_contract["operations"] if item["name"] != "SysPara.dat/status"]
                    changed_receipt["events"] = [item for item in changed_receipt["events"] if not (item["phase"] == "segment-status" and item["segment"] == "SysPara.dat")]
                elif mutation == "segment_hash":
                    changed_contract["segments"][0]["wireSha256"] = "f" * 64
                elif mutation == "payload_count":
                    changed_contract["payloadCount"] = 0
                else:
                    changed_contract["operations"][0]["expectedReply"] = {"firstByte": "", "length": 0, "readRequestBytes": 0}
                    changed_receipt["events"] = [item for item in changed_receipt["events"] if not (item["phase"] == "identify" and item["operation"] == "reply-read")]
                for index, item in enumerate(changed_receipt["events"], 1):
                    item["sequence"] = index
                for index, item in enumerate(changed_contract["operations"], 1):
                    item["sequence"] = index
                with self.assertRaises(acceptance.AcceptanceError):
                    acceptance._native_transfer(changed_receipt, changed_contract)

    def test_commit_ack_without_reset_remains_unconfirmed(self):
        receipt, contract = self.native()
        receipt["ok"] = False
        receipt["resetWritten"] = False
        receipt["postResetReadCompleted"] = False
        result = acceptance._native_transfer(receipt, contract)
        self.assertFalse(result["transportComplete"])
        self.assertEqual(result["recoveryDisposition"], "commit_accepted_reset_unconfirmed")

    def test_commit_write_without_ack_is_unknown(self):
        receipt, contract = self.native()
        receipt["ok"] = False
        receipt["commitAccepted"] = False
        self.assertEqual(acceptance._native_transfer(receipt, contract)["recoveryDisposition"], "commit_state_unconfirmed")

    def test_stage6_import_preserves_old_identity_and_observation(self):
        result, _ = acceptance.import_stage6(REPO, "physical-r2", "macro-14")
        self.assertEqual(result["T"], "partial")
        self.assertEqual(result["E"], "not_run")
        self.assertEqual(result["identity"]["sourceSha256"], "unknown")
        self.assertEqual(result["originalScreenRecord"]["answer"], "显示标识和按钮，没有宽度提示")
        self.assertFalse(result["usbAccessed"])

    def test_stage6_static_import_does_not_override_missing_table_report(self):
        result, _ = acceptance.import_stage6(REPO, "physical-r1", "static-10")
        self.assertEqual(result["originalScreenRecord"]["answer"], "显示字符，没有表格")
        self.assertEqual(result["E"], "not_run")
        self.assertTrue(result["originalScreenRecord"]["userDeferred"])

    def test_stage6_import_rejects_changed_compiled_payload_bytes(self):
        """Retain small real capture fixtures but corrupt one actual compiled block."""
        base = REPO / "docs/stage6-evidence/physical-r1"
        target = self.root / "docs/stage6-evidence/physical-r1"
        target.mkdir(parents=True)
        for name in ("run.json", "static-10-contract.json", "static-10-download-stdout.txt"):
            (target / name).write_bytes((base / name).read_bytes())
        (self.root / "docs/stage6-evidence/screen-observations.json").write_bytes((REPO / "docs/stage6-evidence/screen-observations.json").read_bytes())
        package = target / "static-10-compiled"
        package.mkdir()
        manifest = json.loads((base / "static-10-compiled/compiler-manifest.json").read_text(encoding="utf-8"))
        (package / "compiler-manifest.json").write_bytes((base / "static-10-compiled/compiler-manifest.json").read_bytes())
        for name in manifest["files"]:
            (package / name).write_bytes((base / "static-10-compiled" / name).read_bytes())
        with (package / "SysPara.dat").open("r+b") as stream:
            stream.write(b"changed")
        with self.assertRaisesRegex(acceptance.AcceptanceError, "compiled file"):
            acceptance.import_stage6(self.root, "physical-r1", "static-10")

    def test_duplicate_keys_nonfinite_and_oversized_json_rejected(self):
        for raw in (b'{"ok":false,"ok":true}', b'{"x":NaN}', b'{"x":1e999}', b'{"x":-1e999}', b" " * (acceptance.MAX_JSON + 1)):
            with self.subTest(raw=raw[:40]):
                with self.assertRaises(acceptance.AcceptanceError):
                    acceptance.parse_json(raw)

    def test_artifact_outside_root_is_rejected(self):
        self.artifacts["raw"]["path"] = "../outside.json"
        with self.assertRaisesRegex(acceptance.AcceptanceError, "escapes"):
            self.run_document()

    def test_input_and_hardlink_output_aliases_rejected(self):
        original = self.root / "raw.json"
        for target in (original, self.root / "hardlink.json"):
            if target != original:
                os.link(original, target)
            before = original.read_bytes()
            with self.assertRaisesRegex(acceptance.AcceptanceError, "aliases"):
                acceptance.write_summary(target, {}, [original])
            self.assertEqual(original.read_bytes(), before)

    def test_atomic_replace_failure_preserves_old_output_and_cleans_only_owned_temp(self):
        output = self.root / "output.json"
        output.write_bytes(b"old good output")
        with mock.patch.object(acceptance.os, "replace", side_effect=OSError("injected replace failure")):
            with self.assertRaises(OSError):
                acceptance.write_summary(output, {"new": 1}, [])
        self.assertEqual(output.read_bytes(), b"old good output")
        self.assertEqual(list(self.root.glob(".stage10-*.tmp")), [])

    def test_cli_invalid_evidence_preserves_output(self):
        manifest = self.root / "input.json"
        output = self.root / "output.json"
        document = self.document()
        document["artifacts"]["raw"]["sha256"] = "f" * 64
        manifest.write_text(json.dumps(document), encoding="utf-8")
        output.write_bytes(b"previous")
        with contextlib.redirect_stderr(io.StringIO()):
            status = acceptance.main(["validate", str(manifest), str(output), "--root", str(self.root)])
        self.assertEqual(status, 2)
        self.assertEqual(output.read_bytes(), b"previous")

    def test_cli_require_complete_returns_three_for_blocked_coverage(self):
        self.case.update({"status": "blocked_external", "reason": "device observer unavailable"})
        manifest = self.root / "input.json"
        output = self.root / "output.json"
        manifest.write_text(json.dumps(self.document()), encoding="utf-8")
        with contextlib.redirect_stdout(io.StringIO()):
            status = acceptance.main(["validate", str(manifest), str(output), "--root", str(self.root), "--require-complete"])
        self.assertEqual(status, 3)
        self.assertEqual(json.loads(output.read_text(encoding="utf-8"))["status"], "blocked_external")


if __name__ == "__main__":
    unittest.main()
