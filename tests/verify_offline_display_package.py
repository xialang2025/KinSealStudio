"""Check published offline display layouts and reject self-consistent manifest hashes for inconsistent members."""
# 20261128-27 add
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
REVISION = "e3b72657"
MESSAGE = "docs/stage4-evidence/message/native/compiled/message-state3/input-native.sm"
QR = "docs/stage4-evidence/qrcode/physical/qrcode-lifecycle-r1/compiled/physical-native-reference/input-native.sm"


def verify(exe, directory):
    """Exercise the disk compiler, package rereader and offline transfer-contract preflight."""
    def run(*args, ok=True):
        process = subprocess.run([exe, *map(str, args)], capture_output=True, timeout=90,
                                 creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        if (process.returncode == 0) != ok:
            raise AssertionError((args, process.stdout.decode("utf8", "replace"), process.stderr))
        return json.loads(process.stdout)

    report = {}
    for kind, fixture, field in (("message", MESSAGE, 0x102), ("qr", QR, 0x22d)):
        sm = directory / (kind + ".sm")
        sm.write_bytes(subprocess.check_output(["git", "show", REVISION + ":" + fixture], cwd=ROOT))
        out = directory / kind
        run("--compile-project", sm, out)
        manifest = run("--verify-compiled-package", out)
        assert manifest["offlineOnly"] and not manifest["transferPlan"] and not manifest["usbAccessed"]
        contract = directory / (kind + "-contract.json")
        result = run("--transfer-contract", sm, contract, ok=False)
        assert not contract.exists() and "直连下载" in result["message"]
        manifest_path = out / "compiler-manifest.json"
        original_manifest = manifest_path.read_bytes()
        def attempt(overrides, name=None, data=None):
            metadata = json.loads(original_manifest)
            metadata.update(overrides)
            if name is not None:
                file = out / name
                saved = file.read_bytes()
                file.write_bytes(data)
                metadata["files"][name] = {"length": len(data), "sha256": hashlib.sha256(data).hexdigest()}
            manifest_path.write_text(json.dumps(metadata), encoding="utf8")
            try:
                run("--verify-compiled-package", out, ok=False)
            finally:
                if name is not None:
                    file.write_bytes(saved)
                manifest_path.write_bytes(original_manifest)
        attempt({"offlineOnly": False})
        attempt({"transferPlan": None})
        control = bytearray((out / "ControlPara.dat").read_bytes())
        control[field] = 0 if kind == "message" else 2
        attempt({}, "ControlPara.dat", control)
        control = bytearray((out / "ControlPara.dat").read_bytes())
        if kind == "message":
            control[0x101] = 1
        else:
            control[0x10c:0x110] = (4096).to_bytes(4, "little")
            control[0x228:0x22c] = (4).to_bytes(4, "little")
        attempt({}, "ControlPara.dat", control)
        if kind == "message":
            data = bytearray((out / "Message.dat").read_bytes())
            attempt({}, "Message.dat", data[:-104])
            data[3] = 1
            attempt({}, "Message.dat", data)
            data[3] = 0
            data[102] = 1
            attempt({}, "Message.dat", data)
        assert run("--verify-compiled-package", out) == manifest
        report[kind] = {"sourceSha256": hashlib.sha256(sm.read_bytes()).hexdigest(),
                        "validatedFiles": len(manifest["files"]), "offlineOnly": True,
                        "rejectedMutations": 7 if kind == "message" else 4}
    return report


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--scratch", required=True, type=Path)
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(dir=args.scratch) as work:
        result = verify(args.exe.resolve(), Path(work))
    if args.report:
        args.report.write_text(json.dumps(result, indent=2) + "\n", encoding="utf8")
    print(json.dumps(result))
