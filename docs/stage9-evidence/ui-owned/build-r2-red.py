"""Build the small online-widget test target independently of the shared CMake build."""
from __future__ import annotations
import hashlib
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[3]
EVIDENCE = Path(__file__).resolve().parent
OUTPUT = Path(r"Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage9-20261114\temp\online-ui-owned-r2-red")
QT = Path(r"G:\01_Professional\QT6\Installed\6.12.0\mingw_64")
CC = Path(r"G:\01_Professional\QT6\Installed\Tools\mingw1310_64\bin\g++.exe")
OUTPUT.mkdir(parents=True, exist_ok=False)
TEMP = OUTPUT / "temp"
TEMP.mkdir()
ENV = os.environ.copy()
ENV["PATH"] = os.pathsep.join((str(CC.parent), str(QT / "bin"), ENV.get("SYSTEMROOT", r"C:\Windows") + r"\System32"))
for name in ("TEMP", "TMP", "TMPDIR"):
    ENV[name] = str(TEMP)
paths = ["src/plc_online_dialog.cpp", "src/plc_online_dialog.h", "src/plc_serial_session.cpp",
         "src/plc_serial_session.h", "src/plc_protocol_capabilities.cpp", "src/plc_protocol_capabilities.h",
         "src/plc_address.cpp", "src/plc_native_catalog.cpp", "src/simulation_modbus_transport.cpp",
         "plc_catalog.qrc", "resources/plc-catalog.json", "resources/plc-stage9-file-audit.json",
         "tests/stage9_online_ui_tests.cpp"]
hashes = {name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in paths}
record = dict(inputs=hashes, output=str(OUTPUT), steps=[], sharedBuildModified=False,
              physicalSerialOpened=False, sdkInvoked=False)
flags = ["-std=c++17", "-Wall", "-Wextra", "-Wconversion", "-DQT_WIDGETS_LIB", "-DQT_GUI_LIB",
         "-DQT_CORE_LIB", "-DQT_SERIALPORT_LIB", "-DQT_NO_CAST_FROM_ASCII", "-DQT_NO_CAST_TO_ASCII",
         "-Isrc", "-I" + str(OUTPUT)]
for module in ("", "QtCore", "QtGui", "QtWidgets", "QtTest", "QtSerialPort"):
    flags += ["-isystem", str(QT / "include" / module)]


def step(name, command):
    """Input: label and argv. Return: None after recording bounded compiler output and exit status."""
    completed = subprocess.run([str(value) for value in command], cwd=ROOT, env=ENV,
                               stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=60)
    log = EVIDENCE / (name + ".log")
    log.write_bytes(completed.stdout)
    record["steps"].append(dict(label=name, argv=[str(value) for value in command],
                                exitCode=completed.returncode, log=log.name, logBytes=log.stat().st_size))
    (EVIDENCE / "build-r2-red.json").write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")
    if completed.returncode:
        print(completed.stdout.decode("utf-8", errors="replace"))
        raise SystemExit(completed.returncode)


step("moc-tests-r2-red", [QT / "bin/moc.exe", "-Isrc", ROOT / "tests/stage9_online_ui_tests.cpp",
                       "-o", OUTPUT / "stage9_online_ui_tests.moc"])
step("moc-dialog-r2-red", [QT / "bin/moc.exe", "-Isrc", ROOT / "src/plc_online_dialog.h",
                        "-o", OUTPUT / "moc_plc_online_dialog.cpp"])
step("rcc-r2-red", [QT / "bin/rcc.exe", "-name", "plc_catalog", ROOT / "plc_catalog.qrc",
                "-o", OUTPUT / "qrc_plc_catalog.cpp"])
sources = [(Path(name).stem, ROOT / name) for name in
           ["src/plc_online_dialog.cpp", "src/plc_serial_session.cpp", "src/plc_protocol_capabilities.cpp",
            "src/plc_address.cpp", "src/plc_native_catalog.cpp", "src/simulation_modbus_transport.cpp",
            "tests/stage9_online_ui_tests.cpp"]]
sources += [("moc_dialog", OUTPUT / "moc_plc_online_dialog.cpp"), ("catalog", OUTPUT / "qrc_plc_catalog.cpp")]
objects = []
for name, source in sources:
    target = OUTPUT / (name + ".o")
    step("compile-" + name + "-r2-red", [CC] + flags + ["-c", source, "-o", target])
    objects.append(target)
executable = OUTPUT / "Stage9OnlineUiTests-r2-red.exe"
step("link-r2-red", [CC] + objects + ["-L" + str(QT / "lib"), "-lQt6Widgets", "-lQt6Gui", "-lQt6Test",
                                 "-lQt6SerialPort", "-lQt6Core", "-o", executable])
record["inputsStillUnchanged"] = hashes == {name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in paths}
record["executable"] = str(executable)
record["executableSha256"] = hashlib.sha256(executable.read_bytes()).hexdigest()
(EVIDENCE / "build-r2-red.json").write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")
print(json.dumps(dict(executable=str(executable), sha256=record["executableSha256"],
                      inputsStillUnchanged=record["inputsStillUnchanged"])))
raise SystemExit(0 if record["inputsStillUnchanged"] else 1)


