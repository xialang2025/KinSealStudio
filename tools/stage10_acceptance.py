#!/usr/bin/env python3
"""Check bounded, hash-bound acceptance records without device/network/GUI I/O.

Hashes establish retained bytes and declared bindings, not observer authenticity.
Historical imports retain their own version and never inherit the current EXE.
"""
#20261114-9 add
import argparse
from collections import Counter
from datetime import datetime
import hashlib
import json
import math
import os
from pathlib import Path
import re
import sys
import tempfile

FORMAT = "KinSealStage10-acceptance-v1"
PROOF_FORMAT = "KinSealStage10-proof-v1"
LAYERS = frozenset("DUPRCTE")
STATES = frozenset(("not_run", "blocked_external", "partial", "pass", "fail"))
EXECUTIONS = frozenset(("host", "mock", "physical_hmi", "pc_controlled", "pc_plc"))
MAX_JSON = 8 * 1024 * 1024
MAX_ARTIFACT = 64 * 1024 * 1024
MAX_TOTAL = 128 * 1024 * 1024
SHA = re.compile(r"[0-9a-f]{64}\Z")


class AcceptanceError(ValueError):
    """An input is malformed or claims more than retained evidence permits."""


def require(condition, message):
    """Reject invalid evidence without mutating any input."""
    if not condition:
        raise AcceptanceError(message)


def digest(data):
    """Return SHA256 for exact bytes."""
    return hashlib.sha256(data).hexdigest()


def canonical_digest(value):
    """Bind JSON semantic identities independently of indentation/key order."""
    return digest(json.dumps(value, ensure_ascii=False, sort_keys=True,
                             separators=(",", ":"), allow_nan=False).encode("utf-8"))


def _unique_pairs(pairs):
    """Reject duplicate JSON keys instead of silently accepting a hidden value."""
    result = {}
    for key, value in pairs:
        require(key not in result, "duplicate JSON key: " + key)
        result[key] = value
    return result


def parse_json(data):
    """Decode bounded UTF8 finite JSON, accepting an older tool's BOM."""
    require(len(data) <= MAX_JSON, "JSON exceeds bounded input size")
    try:
        return json.loads(data.decode("utf-8-sig"), object_pairs_hook=_unique_pairs,
                          parse_float=_finite_float,
                          parse_constant=lambda value: require(False, "non-finite JSON"))
    except (UnicodeError, json.JSONDecodeError, RecursionError) as error:
        raise AcceptanceError("invalid JSON: " + str(error)) from error


def _finite_float(value):
    """Reject exponent overflow as well as explicit non-finite JSON tokens."""
    converted = float(value)
    require(math.isfinite(converted), "non-finite JSON number")
    return converted


def _read(path, limit=MAX_ARTIFACT):
    """Read a bounded regular file without following its final symbolic link."""
    path = Path(path)
    require(path.is_file() and not path.is_symlink(), "missing/nonregular artifact: " + str(path))
    require(path.stat().st_size <= limit, "artifact too large: " + str(path))
    with path.open("rb") as stream:
        data = stream.read(limit + 1)
    require(len(data) <= limit, "artifact grew past limit")
    return data


def _text(value, field):
    """Require an explicit bounded observation/reason, not a truthy flag."""
    require(isinstance(value, str) and 0 < len(value.strip()) <= 8192,
            "missing/invalid " + field)


def _timestamp(value):
    """Require an observation timestamp with a UTC offset."""
    _text(value, "observedAt")
    try:
        require(datetime.fromisoformat(value.replace("Z", "+00:00")).tzinfo is not None,
                "observedAt needs timezone")
    except ValueError as error:
        raise AcceptanceError("invalid observedAt") from error


def _sha(value, field):
    """Require a lowercase full SHA256 identity."""
    require(isinstance(value, str) and SHA.fullmatch(value), "invalid " + field)


def _identity(value):
    """Keep source, program, project, payload, firmware and endpoint separate."""
    require(isinstance(value, dict), "identity must be an object")
    for key in ("sourceSha256", "programSha256", "projectSha256", "payloadSha256"):
        if value.get(key) != "unknown":
            _sha(value.get(key), key)
    for key in ("model", "hardwareId", "endpointId", "protocol"):
        _text(value.get(key), key)
    firmware = value.get("firmware")
    require(isinstance(firmware, dict) and firmware.get("state") in ("known", "unknown"),
            "invalid firmware identity")
    _text(firmware.get("basis"), "firmware.basis")
    if firmware["state"] == "known":
        _text(firmware.get("value"), "firmware.value")
    return canonical_digest(value)


def _contract_shape(contract):
    """Validate the current 10/12/14-block USB ABI independently of receipt flags.

    The plan order and reply lengths are frozen from device_transfer_plan.cpp and
    device_transfer_contract.cpp. Unknown formats need a new adapter, not a pass.
    """
    count = contract.get("payloadCount")
    require(type(count) is int and count in (10, 12, 14), "unsupported payload count")
    require(contract.get("identity", {}).get("targetModel") == "AMT070W124-L800X480R",
            "unsupported contract target model")
    require(contract.get("startOptionsHex") in ("000000", "010101"), "unsupported start options")
    segments = contract.get("segments")
    require(isinstance(segments, list) and len(segments) == count and
            all(isinstance(item, dict) for item in segments), "contract segments/payload count mismatch")
    specs = ((0x0a, 0, "SysPara.dat"), (0x0b, 10, "ControlPara.dat"),
             (0x0c, 1, "SheetPara.dat"), (0x36, 12, "MarcoVar.dat"),
             (0x1d, 13, "MarcoRun.dat"), (0x0e, 2, "AlarmPara.dat"),
             (0x16, -1, "0x16"), (0x15, 3, "Collect.dat"), (0x19, -1, "0x19"),
             (0x12, 4, "AddrPara.dat"), (0x14, 5, "PicLenPara.dat"),
             (0x0d, 6, "Curve.dat"), (0x1e, 7, "upLoad.knr"),
             (0x10, 11, "ControlPixel.dat"), (0x0f, 8, "SheetPixel.dat"),
             (0x11, 9, "Modbus_RTU_com1.bin"))
    steps = [item for item in specs if item[1] < count]
    expected = [("command", "identify", 0x31, "02"), ("command", "parameters", 0x53, "00"),
                ("command", "begin", 0x34, contract["startOptionsHex"]),
                ("command", "begin-status", 0x35, "")]
    segment_index = 0
    for code, payload_index, name in steps:
        options = ""
        if payload_index >= 0:
            segment = segments[segment_index]
            segment_index += 1
            require(segment.get("kind") == "payload" and segment.get("name") == name,
                    "contract segment order/name mismatch")
            size = segment.get("wireBytes")
            require(type(size) is int and 0 < size <= 0x7fffffff, "bad segment length")
            _sha(segment.get("wireSha256"), "segment wireSha256")
            require(segment.get("expectedReply") == "none-until-following-status" and
                    segment.get("snapshotOuterLengthBE32") is (code == 0x1e), "segment framing mismatch")
            if code == 0x1e:
                require(size > 4, "snapshot lacks outer length/body")
            if code == 0x11:
                options = size.to_bytes(4, "little").hex()
        expected.append(("command", name + "/command", code, options))
        if payload_index >= 0:
            expected.append(("payload", name, None, ""))
        expected.append(("command", name + "/status", 0x35, ""))
    expected.extend((("command", "commit", 0x1b, ""), ("command", "reset", 0x1a, ""),
                     ("post-reset-read", "reset-read-completion", None, "")))
    operations = contract.get("operations")
    require(isinstance(operations, list) and len(operations) == len(expected), "contract plan length mismatch")
    segment_map = {item["name"]: item for item in segments}
    for index, (actual, (kind, name, code, options)) in enumerate(zip(operations, expected), 1):
        require(isinstance(actual, dict) and actual.get("kind") == kind and actual.get("name") == name,
                "contract plan order mismatch")
        require(actual.get("sequence") == index and actual.get("nextSequence") == (index + 1 if index < len(expected) else 0),
                "contract sequence/nextSequence mismatch")
        if kind == "command":
            frame = b"KinSealStudioKiss" + bytes((code,)) + bytes.fromhex(options)
            frame = frame.ljust(32, b"\0")
            length, first = (24, "4c") if code == 0x31 else (24, "00") if code == 0x53 else (0, "") if code == 0x1a else (1, "4f")
            reply = {"length": length, "firstByte": first, "readRequestBytes": 32 if length == 24 else 4 if length else 0}
            require(actual.get("commandHex") == f"{code:02x}" and actual.get("optionsHex") == options and
                    actual.get("frameHex") == frame.hex() and actual.get("wireBytes") == 32 and
                    actual.get("wireSha256") == digest(frame) and actual.get("expectedReply") == reply,
                    "contract command frame/reply ABI mismatch")
        elif kind == "payload":
            require(all(actual.get(key) == segment_map[name].get(key) for key in
                        ("wireBytes", "wireSha256", "expectedReply")), "contract payload/segments mismatch")
        else:
            require(actual.get("readRequestBytes") == 4 and actual.get("observedReturn") == -5 and
                    actual.get("ackRequired") is False, "post-reset contract mismatch")
    return [item[2] for item in steps], [item["name"] for item in segments]


def _native_transfer(receipt, contract):
    """Compare retained engine events to a complete contract without USB access.

    Exact reply lengths follow the existing engine ABI; no new reassembly,
    rollback, power retention, physical identity or HMI execution is inferred.
    """
    require(isinstance(receipt, dict) and isinstance(contract, dict), "transfer objects required")
    require(contract.get("format") == "KinSealStage6-transfer-contract-v1",
            "unsupported transfer contract")
    step_names, segment_names = _contract_shape(contract)
    operations, events = contract.get("operations"), receipt.get("events")
    require(isinstance(operations, list) and 1 <= len(operations) <= 2048,
            "invalid contract operations")
    require(isinstance(events, list) and 1 <= len(events) <= 8192,
            "missing transfer events")
    require(all(isinstance(item, dict) for item in operations + events),
            "malformed transfer operation/event")
    require([item.get("sequence") for item in operations] == list(range(1, len(operations) + 1)),
            "contract sequence mismatch")
    require([item.get("sequence") for item in events] == list(range(1, len(events) + 1)),
            "transfer event sequence mismatch")
    require(len(operations) >= 7 and
            [(item.get("kind"), item.get("commandHex")) for item in operations[:4]] ==
            [("command", "31"), ("command", "53"), ("command", "34"), ("command", "35")] and
            [(item.get("kind"), item.get("commandHex")) for item in operations[-3:]] ==
            [("command", "1b"), ("command", "1a"), ("post-reset-read", None)],
            "contract lacks fixed identify/start/commit/reset lifecycle")
    wanted = []
    for operation in operations:
        kind = operation.get("kind")
        if kind == "command":
            frame = operation.get("frameHex", "")
            require(isinstance(frame, str) and re.fullmatch("[0-9a-f]{64}", frame),
                    "bad command frame")
            require(digest(bytes.fromhex(frame)) == operation.get("wireSha256"),
                    "contract command digest mismatch")
            require(bytes.fromhex(frame).startswith(b"KinSealStudioKiss") and
                    frame[34:36] == operation.get("commandHex"), "command code/frame mismatch")
            wanted.append(("command-write", operation))
            if isinstance(operation.get("expectedReply"), dict) and operation["expectedReply"].get("length", 0) > 0:
                wanted.append(("reply-read", operation))
        elif kind == "payload":
            _sha(operation.get("wireSha256"), "wireSha256")
            require(type(operation.get("wireBytes")) is int and operation["wireBytes"] > 0,
                    "bad wire length")
            wanted.append(("payload-write", operation))
        elif kind == "post-reset-read":
            wanted.append(("reset-read", operation))
        else:
            raise AcceptanceError("unknown contract operation")
    actual = [item for item in events if item.get("operation") in
              ("command-write", "reply-read", "payload-write", "reset-read")]
    mismatches = []
    for index, event in enumerate(actual):
        if index >= len(wanted):
            mismatches.append("extra wire operation")
            break
        operation_name, expected = wanted[index]
        if event.get("operation") != operation_name:
            mismatches.append("wire operation order mismatch at " + str(index + 1))
            continue
        if operation_name == "command-write":
            good = (event.get("bytesHex") == expected.get("frameHex") and
                    event.get("requested") == 32 and event.get("returned") == 32 and
                    event.get("command") == int(expected["commandHex"], 16) and
                    event.get("accepted") is True)
        elif operation_name == "payload-write":
            good = (event.get("segment") == expected.get("name") and
                    event.get("sha256") == expected.get("wireSha256") and
                    event.get("requested") == expected.get("wireBytes") and
                    event.get("returned") == expected.get("wireBytes") and
                    event.get("accepted") is True)
        elif operation_name == "reply-read":
            reply = expected["expectedReply"]
            raw = event.get("bytesHex", "")
            good = (isinstance(raw, str) and re.fullmatch("(?:[0-9a-f]{2})*", raw) is not None and
                    len(raw) == 2 * reply["length"] and raw.startswith(reply["firstByte"]) and
                    event.get("command") == int(expected["commandHex"], 16) and
                    event.get("requested") == reply["readRequestBytes"] and
                    event.get("returned") == reply["length"] and event.get("accepted") is True)
        else:
            good = (event.get("command") == 0x1a and
                    event.get("requested") == expected["readRequestBytes"] and
                    event.get("returned") == expected["observedReturn"] and event.get("accepted") is True)
        if not good:
            mismatches.append("wire result mismatch at " + str(index + 1))
    lifecycle = [item for item in events if item.get("operation") in ("open", "claim", "release", "close")]
    lifecycle_complete = (
        [item.get("operation") for item in lifecycle] == ["open", "claim", "release", "close"] and
        all(item.get("accepted") is True and item.get("returned") == 0 for item in lifecycle) and
        [item.get("operation") for item in events[:2]] == ["open", "claim"] and
        [item.get("operation") for item in events[-2:]] == ["release", "close"])
    milestones = ("planValidated", "opened", "interfaceClaimed", "interfaceReleased", "closed",
                  "startAccepted", "transferAccepted", "commitAccepted", "resetWritten", "postResetReadCompleted")
    complete = (len(actual) == len(wanted) and not mismatches and lifecycle_complete and
                len(events) == len(actual) + 4 and
                receipt.get("plannedSteps") == len(step_names) and receipt.get("completedSteps") == len(step_names) and
                receipt.get("payloadBlocks") == len(segment_names) and
                receipt.get("writtenSegments") == segment_names and receipt.get("acknowledgedSegments") == step_names and
                all(receipt.get(key) is True for key in milestones) and receipt.get("ok") is True and
                receipt.get("firstError", {}).get("present") is False and receipt.get("cancelled") is False)
    if receipt.get("ok") is True:
        require(complete, "success receipt contradicts wire history/milestones: " + "; ".join(mismatches))
    if receipt.get("commitAccepted") is True and not receipt.get("resetWritten"):
        recovery = "commit_accepted_reset_unconfirmed"
    elif receipt.get("commitWriteAttempted") is True and not receipt.get("commitAccepted"):
        recovery = "commit_state_unconfirmed"
    elif receipt.get("anyWriteAttempted") is True and not complete:
        recovery = "screen_state_unconfirmed"
    elif complete:
        recovery = "transport_complete_screen_unobserved"
    else:
        recovery = "no_successful_download_claim"
    return {"transportComplete": complete, "wireOperationsObserved": len(actual),
            "wireOperationsRequired": len(wanted), "mismatches": mismatches,
            "recoveryDisposition": recovery, "hmiExecutionConfirmed": False,
            "automaticRollbackProven": False, "powerRetentionProven": False}


class EvidenceStore:
    """Load exact-hash artifacts once and retain input paths for output safety."""

    def __init__(self, root, artifacts):
        self.root = Path(root).resolve()
        self.paths, self.values = [], {}
        require(isinstance(artifacts, dict) and len(artifacts) <= 4096, "invalid artifacts map")
        total = 0
        for name, artifact in artifacts.items():
            _text(name, "artifact id")
            require(isinstance(artifact, dict), "invalid artifact entry")
            _text(artifact.get("path"), "artifact.path")
            _sha(artifact.get("sha256"), "artifact.sha256")
            path = Path(artifact["path"])
            path = path if path.is_absolute() else self.root / path
            require(path.resolve().is_relative_to(self.root), "artifact escapes evidence root")
            raw = _read(path)
            total += len(raw)
            require(total <= MAX_TOTAL, "combined artifacts exceed size limit")
            require(digest(raw) == artifact["sha256"], "evidence hash mismatch: " + name)
            self.paths.append(path.resolve())
            self.values[name] = raw

    def json(self, name):
        """Decode referenced JSON only after validating its exact bytes."""
        self.present(name)
        return parse_json(self.values[name])

    def present(self, name):
        """Require a retained artifact reference."""
        require(isinstance(name, str) and name in self.values, "unknown evidence id: " + str(name))


def _proof(case, identity, store):
    """Check structured observations and disallow unsupported physical claims."""
    proof = store.json(case.get("proof"))
    require(isinstance(proof, dict) and proof.get("format") == PROOF_FORMAT,
            "structured proof required")
    for key in ("caseId", "layer", "execution", "status"):
        require(proof.get(key) == case.get(key), "proof/claim mismatch: " + key)
    require(proof.get("identitySha256") == canonical_digest(identity), "proof identity mismatch")
    for key in ("actor", "initialState", "actual", "expected"):
        _text(proof.get(key), key)
    _timestamp(proof.get("observedAt"))
    steps = proof.get("stimulusSteps")
    require(isinstance(steps, list) and 1 <= len(steps) <= 256, "missing stimulus steps")
    for step in steps:
        _text(step, "stimulus step")
    store.present(proof.get("expectedSource"))
    store.present(proof.get("rawObservation"))
    if case["status"] == "pass":
        require(proof.get("expectedMatched") is True, "pass needs recorded expectation match")
        require(all(identity.get(key) != "unknown" for key in
                    ("sourceSha256", "programSha256", "projectSha256", "payloadSha256")),
                "pass needs exact source/program/project/payload identity")
        for label in ("source", "program", "project", "payload"):
            artifact = proof.get(label + "Artifact")
            store.present(artifact)
            require(digest(store.values[artifact]) == identity[label + "Sha256"],
                    "identity/artifact mismatch: " + label)
    if case["layer"] == "E":
        require(case["execution"] == "physical_hmi", "E requires physical HMI observation")
        require(identity["hardwareId"] != "unknown" and identity["endpointId"] != "unknown",
                "E needs a specific hardware/endpoint identity")
        require(proof.get("independentObservation") is True, "E requires an independent observation")
        recovery = proof.get("recovery")
        require(isinstance(recovery, dict), "E requires recovery record")
        require(recovery.get("status") in ("pass", "not_required", "state_unconfirmed", "not_run"),
                "bad recovery status")
        _text(recovery.get("action"), "recovery.action")
        _text(recovery.get("actual"), "recovery.actual")
        if case["status"] == "pass":
            require(recovery["status"] in ("pass", "not_required"), "unconfirmed recovery cannot pass E")
            require(proof.get("expectedMatched") is True, "pass needs recorded expectation match")
    if case["layer"] == "T":
        require(identity["protocol"] == "KinSealUSB", "T adapter supports only KinSealUSB, not other protocols")
        contract = store.json(proof.get("transferContract"))
        require(contract.get("identity", {}).get("sourceSha256") == identity["projectSha256"],
                "transfer/project identity mismatch")
        require(contract.get("identity", {}).get("targetModel") == identity["model"],
                "transfer/model identity mismatch")
        require(digest(store.values[proof["transferContract"]]) == identity["payloadSha256"],
                "transfer/payload contract identity mismatch")
        transfer = _native_transfer(store.json(proof.get("transferReceipt")), contract)
        if case["status"] == "pass":
            require(transfer["transportComplete"], "incomplete transfer cannot pass T")
            if case["execution"] == "physical_hmi":
                require(identity["hardwareId"] != "unknown" and identity["endpointId"] != "unknown",
                        "physical T needs hardware/endpoint identity")
        return transfer
    return None


def evaluate(document, root):
    """Check declared coverage; missing required cases remain not_run."""
    require(isinstance(document, dict) and document.get("format") == FORMAT,
            "unsupported acceptance format")
    identities = document.get("identities")
    require(isinstance(identities, dict) and 1 <= len(identities) <= 256, "invalid identities")
    for name, identity in identities.items():
        _text(name, "identity id")
        _identity(identity)
    store = EvidenceStore(root, document.get("artifacts", {}))
    requirements, cases = document.get("requirements"), document.get("cases")
    require(isinstance(requirements, list) and 1 <= len(requirements) <= 4096,
            "nonempty required coverage needed")
    require(isinstance(cases, list) and len(cases) <= 4096, "invalid cases")
    required = {}
    for row in requirements:
        require(isinstance(row, dict), "bad requirement")
        _text(row.get("caseId"), "caseId")
        require(row["caseId"] not in required, "duplicate requirement id")
        require(row.get("layer") in LAYERS and row.get("execution") in EXECUTIONS and
                row.get("identityId") in identities, "invalid required scope")
        required[row["caseId"]] = row
    seen, results, restrictions = set(), [], []
    for case in cases:
        require(isinstance(case, dict) and case.get("caseId") in required, "undeclared case")
        name = case["caseId"]
        require(name not in seen, "duplicate case result")
        seen.add(name)
        for key in ("layer", "execution", "identityId"):
            require(case.get(key) == required[name][key], "case/requirement scope mismatch: " + key)
        require(case.get("status") in STATES, "invalid result state")
        _text(case.get("reason"), "case.reason")
        identity, transfer = identities[case["identityId"]], None
        if case["status"] in ("pass", "fail"):
            transfer = _proof(case, identity, store)
        elif case.get("proof"):
            store.present(case["proof"])
        limitation = []
        if case["status"] != "pass":
            limitation.append(case["status"] + ": " + case["reason"])
        if case["execution"] != "physical_hmi" and case["layer"] in ("T", "E"):
            limitation.append("controlled/PC evidence does not establish physical HMI T or E")
        if identity["firmware"]["state"] == "unknown":
            limitation.append("firmware unknown; claims apply only to this endpoint and recorded run")
        if identity["hardwareId"] == "unknown":
            limitation.append("hardware identity unavailable; no cross-device compatibility claim")
        result = {key: case[key] for key in ("caseId", "identityId", "layer", "execution", "status", "reason")}
        result.update({"identitySha256": canonical_digest(identity), "limits": limitation,
                       "physicalTransportPassed": case["status"] == "pass" and case["layer"] == "T" and case["execution"] == "physical_hmi",
                       "hmiExecutionPassed": case["status"] == "pass" and case["layer"] == "E" and case["execution"] == "physical_hmi"})
        if transfer:
            result["transfer"] = transfer
        results.append(result)
        restrictions.extend({"caseId": name, "reason": item} for item in limitation)
    for name, definition in required.items():
        if name not in seen:
            results.append({**definition, "status": "not_run", "reason": "no result submitted",
                            "physicalTransportPassed": False, "hmiExecutionPassed": False})
            restrictions.append({"caseId": name, "reason": "required case has not run"})
    for row in results:
        if row["layer"] == "E" and row["status"] == "pass":
            for needed in ("C", "T"):
                require(any(candidate["layer"] == needed and candidate["status"] == "pass" and
                            candidate["identityId"] == row["identityId"] and
                            (needed != "T" or candidate["execution"] == "physical_hmi")
                            for candidate in results),
                        "E pass requires same-identity C and physical T pass")
    counts = dict(Counter(row["status"] for row in results))
    all_pass = all(row["status"] == "pass" for row in results)
    status = ("pass" if all_pass else "fail" if counts.get("fail") else
              "not_run" if counts.get("not_run") == len(results) else
              "blocked_external" if counts.get("blocked_external") == len(results) else "partial")
    return {"format": "KinSealStage10-summary-v1", "status": status, "counts": counts,
            "declaredCoverageComplete": all_pass, "fullReplicaCertified": False,
            "results": results, "releaseRestrictions": restrictions,
            "usbAccessed": False, "plcAccessed": False, "devicePowerInterrupted": False,
            "integrityBoundary": "Hashes validate retained bytes and declared bindings, not observer authenticity or unrecorded behavior."}, store.paths


def _last_receipt(raw):
    """Read final JSONL receipt rather than promoting preceding progress rows."""
    lines = raw.splitlines()
    require(1 <= len(lines) <= 16384, "invalid JSONL receipt length")
    values = [parse_json(line) for line in lines if line.strip()]
    require(values and isinstance(values[-1], dict), "missing final receipt")
    require("events" in values[-1] and "outcome" in values[-1], "no final transfer receipt")
    return values[-1]


def import_stage6(root, run_name, sample):
    """Audit selected historical records without supplying missing identities.

    A model label and executable hash are not a unique hardware serial or source
    tree manifest. Therefore a complete old USB trace remains partial T here;
    screen execution and recovery do not pass without the required observations.
    """
    require(run_name in ("physical-r1", "physical-r2"), "unsupported stage6 run")
    require(sample in ("static-10", "dynamic-12", "macro-14"), "unsupported stage6 sample")
    root = Path(root).resolve()
    relative = Path("docs/stage6-evidence") / run_name
    sources = {"run": relative / "run.json", "contract": relative / (sample + "-contract.json"),
               "manifest": relative / (sample + "-compiled/compiler-manifest.json"),
               "receipt": relative / (sample + "-download-stdout.txt"),
               "observation": Path("docs/stage6-evidence/screen-observations.json")}
    paths = [root / item for item in sources.values()]
    require(all(path.resolve().is_relative_to(root) for path in paths), "historical artifact escapes evidence root")
    raw = {name: _read(root / item) for name, item in sources.items()}
    run, contract, manifest = (parse_json(raw[key]) for key in ("run", "contract", "manifest"))
    receipt = _last_receipt(raw["receipt"])
    audit = _native_transfer(receipt, contract)
    program = run.get("executableSha256", "unknown")
    _sha(program, "historical program hash")
    require(program == manifest.get("compilerExecutableSha256"), "runner/compiler executable mismatch")
    identity = contract.get("identity", {})
    require(identity.get("modelSha256") == manifest.get("inputProjectSha256"),
            "contract/compiler project mismatch")
    require(identity.get("targetModel") == manifest.get("targetModel") and
            run.get("model") == identity.get("targetModel"), "target model mismatch")
    contract_wires = [(item.get("name"), item.get("wireBytes"), item.get("wireSha256"))
                      for item in contract.get("segments", [])]
    manifest_wires = [(item.get("name"), item.get("wireBytes"), item.get("wireSha256"))
                      for item in manifest.get("transferPlan", []) if item.get("wireBytes", 0) > 0]
    require(contract_wires == manifest_wires, "contract/compiler payload mismatch")
    files = manifest.get("files")
    require(manifest.get("complete") is True and isinstance(files, dict) and
            set(files) == {item["name"] for item in contract["segments"]},
            "compiled file inventory mismatch")
    total_bytes = sum(len(data) for data in raw.values())
    for segment in contract["segments"]:
        name = segment["name"]
        # Names come from the fixed ABI checked above, never arbitrary manifest paths.
        file_relative = relative / (sample + "-compiled") / name
        file_path = root / file_relative
        require(file_path.resolve().is_relative_to(root), "compiled file escapes evidence root")
        data = _read(file_path)
        total_bytes += len(data)
        require(total_bytes <= MAX_TOTAL, "compiled files exceed total size limit")
        metadata = files[name]
        require(isinstance(metadata, dict) and metadata.get("length") == len(data) and
                metadata.get("sha256") == digest(data), "compiled file length/hash mismatch: " + name)
        wire = len(data).to_bytes(4, "big") + data if name == "upLoad.knr" else data
        require(segment["wireBytes"] == len(wire) and segment["wireSha256"] == digest(wire),
                "compiled file wire hash mismatch: " + name)
        artifact_id = "payload/" + name
        sources[artifact_id] = file_relative
        raw[artifact_id] = data
        paths.append(file_path)
    require(run.get("usbAccessed") is True, "historical run lacks physical USB access declaration")
    downloads = [item for item in run.get("runs", []) if item.get("name") == sample + "-download"]
    require(len(downloads) == 1, "historical sample download not uniquely identified")
    require((downloads[0].get("exitCode") == 0) == (receipt.get("ok") is True),
            "runner exit/receipt contradiction")
    observed = parse_json(raw["observation"])
    sample_observation = (observed.get("macro14" if sample == "macro-14" else "static10", {})
                          if sample != "dynamic-12" else {})
    return {"format": "KinSealStage10-historical-import-v1", "sample": sample,
            "sourceRun": run_name, "status": "partial" if audit["transportComplete"] else "fail",
            "identity": {"sourceSha256": "unknown", "programSha256": program,
                         "projectSha256": identity.get("sourceSha256", "unknown"),
                         "normalizedProjectSha256": identity.get("modelSha256", "unknown"),
                         "payloadSha256": digest(raw["contract"]),
                         "model": identity.get("targetModel"), "hardwareId": "unknown",
                         "endpointId": "historical-usb-" + run_name, "protocol": "KinSealUSB",
                         "firmware": {"state": "unknown", "basis": "historical runner: " + str(run.get("firmwareVersion"))}},
            "transferAudit": audit, "T": "partial" if audit["transportComplete"] else "fail", "E": "not_run", "recovery": "not_run",
            "originalReceipt": receipt, "originalScreenRecord": sample_observation,
            "retainedEvidence": {key: {"path": str(sources[key]).replace("\\", "/"), "sha256": digest(value)}
                                 for key, value in raw.items()},
            "releaseRestrictions": [
                "Historical T is bound to its old executable/project/payload, never the current program.",
                "Unique hardware identity and source-tree manifest were not recorded.",
                "USB reset completion does not prove screen execution, upload, rollback, or power retention.",
                "Screen record is preserved verbatim; incomplete stimuli/recovery cannot pass E."],
            "usbAccessed": False, "plcAccessed": False, "devicePowerInterrupted": False}, paths


def write_summary(path, value, protected_paths):
    """Atomically publish JSON and reject input/evidence aliases, including hard links."""
    destination = Path(path).absolute()
    require(destination.parent.is_dir(), "output parent does not exist")
    require(not destination.is_symlink(), "output cannot be symbolic link")
    for protected in protected_paths:
        candidate = Path(protected)
        same_name = os.path.normcase(str(destination.resolve())) == os.path.normcase(str(candidate.resolve()))
        same_inode = destination.exists() and candidate.exists() and os.path.samefile(destination, candidate)
        require(not same_name and not same_inode, "output aliases an input/evidence file")
    require(not destination.exists() or destination.is_file(), "output is not a regular file")
    data = (json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n").encode("utf-8")
    require(len(data) <= MAX_JSON, "summary exceeds size limit")
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(prefix=".stage10-", suffix=".tmp",
                                         dir=destination.parent, delete=False) as stream:
            temporary = Path(stream.name)
            stream.write(data)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, destination)
        temporary = None
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


def main(argv=None):
    """Validate a matrix or import an explicitly named historical run."""
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    validate = commands.add_parser("validate")
    validate.add_argument("input", type=Path)
    validate.add_argument("output", type=Path)
    validate.add_argument("--root", type=Path, required=True)
    validate.add_argument("--require-complete", action="store_true",
                          help="Return 3 when structurally valid declared coverage is not all pass.")
    historical = commands.add_parser("import-stage6")
    historical.add_argument("output", type=Path)
    historical.add_argument("--root", type=Path, required=True)
    historical.add_argument("--run", choices=("physical-r1", "physical-r2"), required=True)
    historical.add_argument("--sample", choices=("static-10", "dynamic-12", "macro-14"), required=True)
    args = parser.parse_args(argv)
    try:
        if args.command == "validate":
            value, protected = evaluate(parse_json(_read(args.input, MAX_JSON)), args.root)
            protected.append(args.input)
        else:
            value, protected = import_stage6(args.root, args.run, args.sample)
        write_summary(args.output, value, protected)
        print(json.dumps({"ok": True, "status": value["status"], "output": str(args.output)}, ensure_ascii=False))
        if args.command == "validate" and args.require_complete and value["status"] != "pass":
            return 3
        return 0
    except (AcceptanceError, OSError, TypeError, KeyError, AttributeError, RecursionError, UnicodeError) as error:
        print(json.dumps({"ok": False, "error": str(error)}, ensure_ascii=False), file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())

