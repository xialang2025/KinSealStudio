#!/usr/bin/env python3
"""Freeze first-round small textual evidence before later user answers are appended."""
#20261114-9 add
from datetime import datetime, timezone
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
    """Create new R1 paths; do not overwrite the earlier provisional input/receipt."""
    source = HERE / "current-cases.json"
    target = HERE / "current-cases-r1.json"
    gate.require(not target.exists(), "round R1 is already frozen")
    document = gate.parse_json(source.read_bytes())
    snapshots = HERE / "round-r1"
    snapshots.mkdir(exist_ok=True)
    rows = []
    for name, item in document["artifacts"].items():
        old_path = WORKSPACE / item["path"]
        if (old_path.is_relative_to(HERE) or old_path.suffix.lower() not in (".json", ".md", ".txt", ".ksp") or
                old_path.stat().st_size > 1024 * 1024):
            gate.require(gate.digest(old_path.read_bytes()) == item["sha256"], "R1 input already changed: " + name)
            continue
        new_path = snapshots / (name + old_path.suffix)
        if new_path.exists():
            gate.require(gate.digest(new_path.read_bytes()) == item["sha256"], "snapshot hash mismatch")
        else:
            raw = old_path.read_bytes()
            gate.require(gate.digest(raw) == item["sha256"], "R1 input already changed: " + name)
            new_path.write_bytes(raw)
        rows.append({"artifactId": name, "originalPath": item["path"], "snapshotPath": str(new_path.relative_to(WORKSPACE)),
                     "sha256": item["sha256"], "bytes": new_path.stat().st_size})
        item["path"] = str(new_path.relative_to(WORKSPACE)).replace("\\", "/")
    document["round"] = "r1-frozen-before-function-page"
    target.write_text(json.dumps(document, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    summary = HERE / "current-summary-r1.json"
    command = [sys.executable, "-B", str(REPO / "tools/stage10_acceptance.py"), "validate", str(target), str(summary),
               "--root", str(WORKSPACE), "--require-complete"]
    result = subprocess.run(command, capture_output=True)
    gate.require(result.returncode == 3, "R1 gate should return 3: " + result.stderr.decode("utf-8"))
    frozen = {"round": "r1", "frozenAtUtc": datetime.now(timezone.utc).isoformat(), "command": command,
              "exitCode": result.returncode, "expectedExitCode": 3,
              "stdout": result.stdout.decode("utf-8"), "stderr": result.stderr.decode("utf-8"),
              "input": target.name, "inputSha256": gate.digest(target.read_bytes()),
              "output": summary.name, "outputSha256": gate.digest(summary.read_bytes()),
              "toolSha256": gate.digest((REPO / "tools/stage10_acceptance.py").read_bytes()),
              "snapshots": rows, "bigExecutablesCopied": False,
              "newDeviceIo": False, "stimulusTimesInvented": False,
              "supersedesForReproduction": ["current-cases.json", "current-summary.json", "validation.json"]}
    (HERE / "validation-r1.json").write_text(json.dumps(frozen, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"round": "r1", "exitCode": result.returncode, "snapshots": len(rows),
                      "snapshotBytes": sum(row["bytes"] for row in rows), "inputSha256": frozen["inputSha256"]}))


if __name__ == "__main__":
    main()
