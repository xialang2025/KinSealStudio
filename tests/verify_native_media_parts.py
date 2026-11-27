"""Check native offline compiler member generation, destination preservation and driver context."""
# 20261128-3 add
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
VENDOR = "e3b72657:docs/stage2-evidence/lamp-regression-restore/compiled/lamp-default-r1/"


def verify(exe, root):
    """Compare generated members with independent Git blobs and test publication failure on an occupied directory."""
    def original(name):
        return subprocess.check_output(["git", "show", VENDOR + name], cwd=ROOT)

    def run(*options, ok=True):
        result = subprocess.run([str(exe), "--compile-native-media-parts", *map(str, options)],
                                capture_output=True, timeout=120, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        status = json.loads(result.stdout)
        if (result.returncode == 0) != ok or status.get("ok") is not ok:
            raise RuntimeError(status)

    with tempfile.TemporaryDirectory(dir=root) as temporary:
        folder = Path(temporary)
        source = folder / "lamp.sm"
        source.write_bytes(original("input-native.sm"))
        media = folder / "media"
        run(source, media)
        for member in ("load.bin", "AlarmMuti.dat", "MarcoVar.dat", "Modbus_RTU_com1.bin"):
            if (media / member).read_bytes() != original(member):
                raise ValueError("original member mismatch: " + member)
        saved = {f.name: hashlib.sha256(f.read_bytes()).digest() for f in media.iterdir()}
        run(source, media, ok=False)
        assert saved == {f.name: hashlib.sha256(f.read_bytes()).digest() for f in media.iterdir()}
        other = folder / "without-driver"
        run(source, other, "--without-com1-driver")
        assert not (other / "Modbus_RTU_com1.bin").exists()
        assert set(saved) - {f.name for f in other.iterdir()} == {"Modbus_RTU_com1.bin"}
        assert (media / "load.bin").read_bytes() != (other / "load.bin").read_bytes()
        for name in saved.keys() - {"Modbus_RTU_com1.bin", "load.bin"}:
            assert (media / name).read_bytes() == (other / name).read_bytes()
        absent = folder / "invalid"
        run(source, absent, "--unknown-option", ok=False)
        assert not absent.exists()
        assert not list(folder.glob(".kinseal-media-*"))
        return {"originalMembersExact": 4, "sourceSha256": hashlib.sha256(source.read_bytes()).hexdigest(),
                "driverToggle": True, "occupiedOutputPreserved": True, "invalidOptionNoOutput": True}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    args = parser.parse_args()
    print(json.dumps(verify(args.exe.resolve(), ROOT / "build")))
