"""Run an explicitly authorized native reference download without changing Lite's encoder guards."""
from pathlib import Path
import argparse
import ast
import datetime
import hashlib
import json
import os
import re
import subprocess
import sys

root = Path(__file__).resolve().parent
work = root / 'worktree'
sys.path.insert(0, str(work / 'tools'))
from native_usb_probe_lifecycle import build_hardware_block, instrument_probe_source


def write(path, value):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--family', required=True)
    parser.add_argument('--run', required=True)
    parser.add_argument('--project', type=Path, required=True)
    transfer = parser.add_mutually_exclusive_group(required=True)
    transfer.add_argument('--download', action='store_true')
    transfer.add_argument('--prepare-only', action='store_true')
    parser.add_argument('--source-provenance', choices=('retained_native_reference_source', 'lite_verified_static_text_source'), default='retained_native_reference_source')
    args = parser.parse_args()
    assert re.fullmatch('[a-z0-9-]+', args.family) and re.fullmatch('[a-z0-9-]+', args.run)
    project = args.project.resolve()
    assert project.is_relative_to(work / 'docs/stage4-evidence' / args.family / 'native') and project.suffix.lower() == '.sm'
    output = work / 'docs/stage4-evidence' / args.family / 'physical' / args.run
    output.mkdir(parents=True, exist_ok=False)
    owner = root / 'native-physical'
    temporary = owner / ('host-temp-' + args.run)
    temporary.mkdir(parents=True, exist_ok=False)
    (owner / 'temp').mkdir(exist_ok=True)
    source_sha = digest(project)
    record = dict(startedAtUtc=datetime.datetime.now(datetime.timezone.utc).isoformat(), source=str(project), sourceSha256=source_sha,
                  sourceProvenance=args.source_provenance, liteIndependentEncoderVerified=False,
                  downloadExplicitlyAuthorized=args.download, desktopSwitched=False, systemInput=False, completed=False)
    # Read the already retained Stage3 hardware sequence as data, without importing or modifying the user's tool.
    hardware_path = root.parents[1] / 'KinSealStudio/tools/stage3_native_screen_verification.py'
    tree = ast.parse(hardware_path.read_text(encoding='utf-8'))
    assignment = next(node for node in tree.body if isinstance(node, ast.Assign) and any(isinstance(t, ast.Name) and t.id == 'HARDWARE' for t in node.targets))
    hardware = ast.literal_eval(assignment.value)
    assert "if command.get('download')" in hardware and "m_downloadbtn" in hardware and "screenDisplayConfirmed" in hardware
    hardware = build_hardware_block(hardware)
    source = (root / 'native-wheel/wheel_probe.py').read_text(encoding='utf-8')
    source = source.replace('stage4-evidence/wheel/native', 'stage4-evidence/' + args.family + '/physical/' + args.run)
    source = source.replace('native-wheel', 'native-physical').replace('KinSealStage4Wheel20261112', 'KinSealStage4Physical20261112')
    assert source.count('    args = parser.parse_args()') == 1
    source = source.replace('    args = parser.parse_args()', "    parser.add_argument('--download', action='store_true')\n    args = parser.parse_args()")
    marker = "                compiled_source=destination/'input-native.sm';shutil.copy2(source,compiled_source)"
    assert source.count(marker) == 1
    source = source.replace(marker, marker + '\n' + hardware)
    source = instrument_probe_source(source, (root / 'usb_trace_agent.js').read_text(encoding='utf-8'), (work / 'tools/usb_transfer_completion.py').read_text(encoding='utf-8'))
    script = output / 'native_screen_probe.py'
    script.write_text(source, encoding='utf-8', newline='\n')
    scenario = output / 'scenario.json'
    write(scenario, [dict(op='scene-assert', label='native-reference-object-presence', minSelectableRoots=1), dict(op='native-compile', label='physical-native-reference', download=True)])
    if args.prepare_only:
        record.update(preparedOnly=True, usbAccessed=False, generatedProbe=str(script))
        write(output / 'run.json', record)
        print(json.dumps(dict(output=str(output), preparedOnly=True, usbAccessed=False), ensure_ascii=False))
        return 0
    preflight = subprocess.run(['powershell.exe', '-NoProfile', '-NonInteractive', '-Command',
        '[Console]::OutputEncoding=[Text.UTF8Encoding]::new($false); @(Get-PnpDevice -PresentOnly | Where-Object { $_.InstanceId -like "*VID_7365&PID_616C*" } | Select-Object Status,Class,FriendlyName,InstanceId) | ConvertTo-Json -Depth 4'],
        capture_output=True, timeout=30, creationflags=subprocess.CREATE_NO_WINDOW)
    assert preflight.returncode == 0, preflight.stderr.decode('utf-8', errors='replace')
    device = json.loads(preflight.stdout.decode('utf-8-sig'))
    devices = device if isinstance(device, list) else [device]
    record['devicePreflight'] = devices
    write(output / 'run.json', record)
    assert len(devices) == 1 and devices[0]['Status'] == 'OK' and devices[0]['Class'] == 'libusb-win32 devices', 'Exactly one present healthy matching screen is required.'
    environment = os.environ.copy()
    environment.update(TEMP=str(temporary), TMP=str(temporary), TMPDIR=str(temporary), PYTHONDONTWRITEBYTECODE='1', PYTHONIOENCODING='utf-8')
    process = subprocess.run([sys.executable, str(script), '--run', args.run, '--input', str(project), '--scenario', str(scenario), '--download'],
        env=environment, capture_output=True, timeout=150, creationflags=subprocess.CREATE_NO_WINDOW)
    (output / 'stdout.txt').write_bytes(process.stdout)
    (output / 'stderr.txt').write_bytes(process.stderr)
    probe_path = output / 'native-probe.json'
    probe = json.loads(probe_path.read_text(encoding='utf-8')) if probe_path.exists() else {}
    record.update(completedAtUtc=datetime.datetime.now(datetime.timezone.utc).isoformat(), processExitCode=process.returncode, probeStatus=probe.get('status'),
                  nativeHardware=probe.get('nativeHardware'), ownedProcessTerminated=probe.get('ownedProcessTerminated'),
                  error=probe.get('error'), inputUnchanged=digest(project) == source_sha)
    record['sourceAtTransferMatchesInput'] = probe.get('nativeHardware', {}).get('sourceSha256') == source_sha
    record['ownedProcessTerminateRequestedEpoch'] = probe.get('ownedProcessTerminateRequestedEpoch')
    record['ownedProcessTerminateReturnedEpoch'] = probe.get('ownedProcessTerminateReturnedEpoch')
    record['completed'] = probe.get('nativeHardware', {}).get('usbCompletion', {}).get('ready') is True and record['inputUnchanged'] and record['sourceAtTransferMatchesInput'] and probe.get('status') == 'native-stage3-scenario-completed' and probe.get('nativeHardware', {}).get('completed') is True
    write(output / 'run.json', record)
    summary = {k: v for k, v in (record.get('nativeHardware') or {}).items() if k != 'polls'}
    print(json.dumps(dict(output=str(output), completed=record['completed'], error=record.get('error'), nativeHardware=summary), ensure_ascii=False))
    return 0 if record['completed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
