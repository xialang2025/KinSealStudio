"""Audit existing transport evidence and isolate each new type before any hardware backfill."""
from pathlib import Path
import copy
import hashlib
import json
import os
import subprocess
root = Path(__file__).resolve().parent
work = root / 'worktree'
base = work / 'docs/stage4-evidence/physical-backfill'
inputs = base / 'inputs'
inputs.mkdir(exist_ok=True)
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()


def write(path, value):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


old = [
    ('C01', '静态文本', 'docs/stage0-evidence/baseline-run-usb-r2/usb-download.json'),
    ('C02', '位按钮', 'docs/stage1-evidence/portable-usb-final/usb-download.json'),
    ('C03', '字按钮', 'docs/word-button-evidence/usb-seven-operations.json'),
    ('C04', '指示灯', 'docs/lamp-evidence/usb-lamp-integration.json'),
    ('C05', '功能按钮', 'docs/function-button-evidence/usb-function-integration.json')]
records = []
for identifier, name, evidence in old:
    path = work / evidence
    data = json.loads(path.read_text(encoding='utf-8-sig'))
    result = data.get('transportResult', data.get('result', {}))
    assert data['exitCode'] == 0 and result['ok'] is True
    records.append(dict(id=identifier, name=name, status='existing_independent_transport_evidence_retained', evidence=evidence, sha256=sha(path), displayAndTouchNotInferred=True))
bit_sample = work / 'docs/stage1-evidence/portable-usb-final/stage1-single-page.ksp'
bit_data = json.loads(bit_sample.read_text(encoding='utf-8-sig'))
bit_objects = [o for p in bit_data['pages'] for o in p['objects']]
assert any(o['type'] == 'switch' and 'wordButton' not in o and 'lamp' not in o and 'functionButton' not in o for o in bit_objects)
records[1]['sampleAudit'] = dict(path=str(bit_sample.relative_to(work)), sha256=sha(bit_sample), plainBitButtonPresent=True)
specs = [
    ('C06', '消息显示', 'message', 'message-display.ksp', 'message', 'message-default.sm'),
    ('C07', '刻度', 'scale', 'scale.ksp', 'scale', 'scale-default.sm'),
    ('C08', '二维码', 'qrcode', 'qrcode.ksp', 'qrcode', 'qrcode-default.sm'),
    ('C09', '图片显示', 'picture', 'image-display.ksp', 'image', 'picture-static-red.sm'),
    ('C10', 'GIF显示', 'gif', 'gif-display.ksp', 'gif', 'gif-default.sm'),
    ('C11', '数值显示', 'number', 'number-display.ksp', 'number', 'number-default.sm'),
    ('C12', 'ASCII显示', 'ascii', 'ascii-display.ksp', 'ascii', 'ascii-default.sm'),
    ('C13', '时间显示', 'time', 'time-display.ksp', 'time', 'time-default.sm'),
    ('C14', '画面按钮', 'screen-button', 'screen-button.ksp', 'screen', 'screen-button-default.sm'),
    ('C15', '多态按钮', 'multi-state-button', 'multi-state-button.ksp', 'multistate', 'multi-state-button-default.sm'),
    ('C16', '多功能按钮', 'multi-function-button', 'multi-function-button.ksp', 'multifunction', 'multi-function-button-default.sm'),
    ('C17', '下拉框', 'dropdown', 'dropdown.ksp', 'dropdown', 'dropdown-default.sm'),
    ('C18', '流动块', 'flow', 'flow-block.ksp', 'flow', 'flow-single-default.sm'),
    ('C19', '滑动开关', 'slider', 'slider.ksp', 'slider', 'slider-default.sm')]
environment = os.environ.copy()
temporary = root / 'product-gate-temp'
temporary.mkdir(exist_ok=True)
environment.update(TEMP=str(temporary), TMP=str(temporary), TMPDIR=str(temporary))
environment['PATH'] = 'G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;' + environment.get('PATH', '')
executable = work / 'build/KinSealStudioLite.exe'
for identifier, name, family, sample, kind, native_name in specs:
    destination = inputs / (identifier + '-product.ksp')
    assert not destination.exists(), 'Do not regenerate an existing audited input'
    source = work / 'samples/stage4' / sample
    project = copy.deepcopy(json.loads(source.read_text(encoding='utf-8-sig')))
    found = []
    for page in project['pages']:
        page['objects'] = [o for o in page['objects'] if o['type'] == kind]
        found += page['objects']
    assert found, (identifier, len(found))
    original_variant_count = len(found)
    selected_id = found[0]['id']
    for page in project['pages']:
        page['objects'] = [o for o in page['objects'] if o['id'] == selected_id]
    project['projectName'] = '实物补验-' + identifier + '-' + name
    write(destination, project)
    calls = []
    for label, arguments in (
        ('capabilities', ['--project-capabilities', str(destination)]),
        ('compile', ['--compile-project', str(destination), str(inputs / (identifier + '-compiled'))]),
        ('export-source', ['--export-native-project', str(destination), str(inputs / (identifier + '-export.sm'))])):
        result = subprocess.run([str(executable), *arguments], env=environment, capture_output=True, timeout=30, creationflags=subprocess.CREATE_NO_WINDOW)
        stdout = result.stdout.decode('utf-8-sig', errors='replace')
        stderr = result.stderr.decode('utf-8-sig', errors='replace')
        parsed = json.loads(stdout) if stdout.strip().startswith('{') else None
        calls.append(dict(label=label, exitCode=result.returncode, result=parsed, stdout=stdout, stderr=stderr))
    assert calls[0]['exitCode'] == 0 and calls[0]['result']['modelPersistence']['allowed'] is True, (identifier, calls[0])
    assert calls[1]['exitCode'] != 0 and calls[2]['exitCode'] != 0
    native = work / 'docs/stage4-evidence' / family / 'native' / native_name
    assert native.is_file()
    record = dict(id=identifier, name=name, family=family, status='queued_native_reference_download_independent_encoder_blocked',
                  productInput=dict(path=str(destination.relative_to(work)), sha256=sha(destination), originalMixedSample=str(source.relative_to(work)), isolatedType=kind, selectedObjectId=selected_id, originalSameTypeVariantCount=original_variant_count),
                  independentProductGate=dict(executableSha256=sha(executable), calls=calls, usbAccessed=False),
                  nativeReferenceInput=dict(path=str(native.relative_to(work)), sha256=sha(native), bytes=native.stat().st_size),
                  referenceAndProductSettingsNotDeclaredEquivalent=True, nativeDownload=None)
    records.append(record)
    write(base / (identifier + '-gate.json'), record)
wheel = work / 'docs/stage4-evidence/wheel/physical/wheel-native-baseline-r2/run.json'
assert json.loads(wheel.read_text(encoding='utf-8'))['completed']
records.append(dict(id='C20', name='滚轮', status='native_reference_download_passed_independent_encoder_blocked', evidence=str(wheel.relative_to(work)), sourceProvenance='retained_native_reference_source', displayAndTouchNotInferred=True))
write(base / 'ledger.json', dict(totalReviewed=20, existingTransportEvidence=5, newNativeReferenceCompleted=1, queued=14, independentNewEncodersBlocked=15, records=records, next='C06', complete=False))
write(root / 'pending-push.json', dict(status='pending_network', localHead='245e4e8554424578b32bb28d1b8764cf9164afd5', lastKnownRemote='0d62c8aab6854bf49b5ec29e8c77c715315f1d1c', pendingCommits=['245e4e8'], error='GitHub TCP443 timeout on C20 push'))
print(json.dumps(dict(existingVerified=5, queued=14, productGatesBlocked=14, newNativeReferencePassed=1), ensure_ascii=False))
