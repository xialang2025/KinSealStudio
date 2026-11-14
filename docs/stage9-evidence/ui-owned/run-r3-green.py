"""Run the frozen online-widget tests on a private desktop with injected in-memory I/O only."""
from __future__ import annotations
import hashlib
import importlib.util
import json
from pathlib import Path
import sys

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[3]
OUT = Path(__file__).resolve().parent
BUILD = json.loads((OUT / "build-r3-green.json").read_text(encoding="utf-8"))
EXE = Path(BUILD["executable"])
if hashlib.sha256(EXE.read_bytes()).hexdigest() != BUILD["executableSha256"]:
    raise ValueError("Frozen test executable changed.")
spec = importlib.util.spec_from_file_location("online_ui_private", ROOT / "docs/stage1-evidence/gui-portable/probe.py")
probe = importlib.util.module_from_spec(spec)
spec.loader.exec_module(probe)
environment = probe.clean_environment(Path(r"G:\01_Professional\QT6\Installed\6.12.0\mingw_64\bin"))
environment["QT_QPA_PLATFORM_PLUGIN_PATH"] = r"G:\01_Professional\QT6\Installed\6.12.0\mingw_64\plugins\platforms"
for name in ("TEMP", "TMP", "TMPDIR"):
    environment[name] = str(Path(BUILD["output"]) / "temp")
environment["KINSEAL_STAGE4_CAPTURE_DIR"] = str(OUT / "captures-r3-green")
desktop = probe.PrivateDesktop(OUT)
try:
    result = desktop.run("online-ui-r3-green", EXE,
        ["-o", str(OUT / "tests-r3-green.txt") + ",txt", "-o", str(OUT / "tests-r3-green.xml") + ",xml"],
        environment, expected_exit=0, timeout=55)
finally:
    desktop.close()
result["privateDesktopClosed"] = True
result["executableSha256"] = hashlib.sha256(EXE.read_bytes()).hexdigest()
result["physicalSerialOpened"] = False
result["vendorDllOrSdkInvoked"] = False
result["controlledIoOnly"] = True
result["captureDirectory"] = environment["KINSEAL_STAGE4_CAPTURE_DIR"]
result["temporaryRoot"] = environment["TEMP"]
(OUT / "run-r3-green.json").write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
print(json.dumps({key: result.get(key) for key in ("exitCode", "passed", "error", "executableSha256")}))
raise SystemExit(0 if result["passed"] else 1)


