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


def write(path, value):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--family', required=True)
    parser.add_argument('--run', required=True)
    parser.add_argument('--project', type=Path, required=True)
    parser.add_argument('--download', action='store_true', required=True)
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
                  sourceProvenance='retained_native_reference_source', liteIndependentEncoderVerified=False,
                  downloadExplicitlyAuthorized=True, desktopSwitched=False, systemInput=False, completed=False)
    preflight = subprocess.run(['powershell.exe', '-NoProfile', '-NonInteractive', '-Command',
        '[Console]::OutputEncoding=[Text.UTF8Encoding]::new($false); @(Get-PnpDevice -PresentOnly | Where-Object { $_.InstanceId -like "*VID_7365&PID_616C*" } | Select-Object Status,Class,FriendlyName,InstanceId) | ConvertTo-Json -Depth 4'],
        capture_output=True, timeout=30, creationflags=subprocess.CREATE_NO_WINDOW)
    assert preflight.returncode == 0, preflight.stderr.decode('utf-8', errors='replace')
    device = json.loads(preflight.stdout.decode('utf-8-sig'))
    devices = device if isinstance(device, list) else [device]
    record['devicePreflight'] = devices
    write(output / 'run.json', record)
    assert len(devices) == 1 and devices[0]['Status'] == 'OK' and devices[0]['Class'] == 'libusb-win32 devices', 'Exactly one present healthy matching screen is required.'
    # Read the already retained Stage3 hardware sequence as data, without importing or modifying the user's tool.
    hardware_path = root.parents[1] / 'KinSealStudio/tools/stage3_native_screen_verification.py'
    tree = ast.parse(hardware_path.read_text(encoding='utf-8'))
    assignment = next(node for node in tree.body if isinstance(node, ast.Assign) and any(isinstance(t, ast.Name) and t.id == 'HARDWARE' for t in node.targets))
    hardware = ast.literal_eval(assignment.value)
    assert "if command.get('download')" in hardware and "m_downloadbtn" in hardware and "screenDisplayConfirmed" in hardware
    before = "                    button = widget({'name':'m_downloadbtn','kind':'QPushButton'})"
    after = """                    download_table = widget({'name':'m_table','kind':'QTableWidget'})
                    transfer_options = []
                    for option_name in ('clear_recipe_CB','clear_hisData_CB','clear_hisAlarm_CB','update_CB'):
                        option = widget({'name':option_name,'kind':'QCheckBox'})
                        initially_checked = option['qt'].get('checked') == 'true'
                        if initially_checked:
                            op({'op':'invoke','hwnd':option['hwnd'],'slot':'click'})
                            time.sleep(.15)
                        state = op({'op':'batch','hwnds':[option['hwnd']]})[0]['qt']
                        assert state.get('checked') == 'false', 'Clear-history/firmware-update option must be disabled'
                        transfer_options.append({'name':option_name,'beforeChecked':initially_checked,'afterChecked':False})
                    button = widget({'name':'m_downloadbtn','kind':'QPushButton'})"""
    assert hardware.count(before) == 1
    hardware = hardware.replace(before, after)
    before = "                    result['nativeHardware'] = hardware"
    after = before + "\n                    hardware['transferOptions'] = transfer_options\n                    hardware['completionLogWidget'] = 'DownLoadDlg/m_table'"
    assert hardware.count(before) == 1
    hardware = hardware.replace(before, after)
    before = "                        current_tables = [op({'op':'table-read','hwnd':w['hwnd']}) for w in tables]"
    after = "                        current_tables = [op({'op':'table-read','hwnd':download_table['hwnd']})]"
    assert hardware.count(before) == 1
    hardware = hardware.replace(before, after)
    source = (root / 'native-wheel/wheel_probe.py').read_text(encoding='utf-8')
    source = source.replace('stage4-evidence/wheel/native', 'stage4-evidence/' + args.family + '/physical/' + args.run)
    source = source.replace('native-wheel', 'native-physical').replace('KinSealStage4Wheel20261112', 'KinSealStage4Physical20261112')
    assert source.count('    args = parser.parse_args()') == 1
    source = source.replace('    args = parser.parse_args()', "    parser.add_argument('--download', action='store_true')\n    args = parser.parse_args()")
    marker = "                compiled_source=destination/'input-native.sm';shutil.copy2(source,compiled_source)"
    assert source.count(marker) == 1
    source = source.replace(marker, marker + '\n' + hardware)
    script = output / 'native_screen_probe.py'
    script.write_text(source, encoding='utf-8', newline='\n')
    scenario = output / 'scenario.json'
    write(scenario, [dict(op='scene-assert', label='native-reference-object-presence', minSelectableRoots=1), dict(op='native-compile', label='physical-native-reference', download=True)])
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
    record['completed'] = record['inputUnchanged'] and record['sourceAtTransferMatchesInput'] and probe.get('status') == 'native-stage3-scenario-completed' and probe.get('nativeHardware', {}).get('completed') is True
    write(output / 'run.json', record)
    summary = {k: v for k, v in (record.get('nativeHardware') or {}).items() if k != 'polls'}
    print(json.dumps(dict(output=str(output), completed=record['completed'], error=record.get('error'), nativeHardware=summary), ensure_ascii=False))
    return 0 if record['completed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
