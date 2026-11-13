"""Update the per-control evidence, protocol and recovery ledgers after an actual successful test run."""
import argparse
import copy
import json
from pathlib import Path
import re
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parent / 'worktree'


def append(relative, text):
    """Input: Repository Markdown path and text. Return: None while retaining historical evidence."""
    with (ROOT / relative).open('a', encoding='utf-8', newline='\n') as stream:
        stream.write('\n\n' + text.strip() + '\n')


def main():
    """Input: A reviewed control configuration. Return: Per-control documentation grounded in actual results."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('configuration', type=Path)
    args = parser.parse_args()
    spec = json.loads(args.configuration.read_text(encoding='utf-8'))
    usb_accessed = bool(spec.get('usbAccessed', False))
    hardware_sentence = '本轮未访问USB或新增E结论。'
    transport_layer = 'not_run'
    matrix_transport = ' 未执行 '
    if usb_accessed:
        hardware = json.loads((ROOT / spec['hardwareRun']).read_text(encoding='utf-8-sig'))
        assert hardware['completed'] and hardware['inputUnchanged'] and hardware['nativeHardware']['completed']
        assert hardware['nativeHardware']['usbCompletion']['ready'] and hardware['nativeHardware']['usbCompletion']['resetReadCompleted']
        assert hardware['sourceProvenance'] == 'retained_native_reference_source' and hardware['liteIndependentEncoderVerified'] is False
        hardware_sentence = f"本轮实际执行[原厂基线USB下载]({spec['hardwareRun'].removeprefix('docs/')})，原厂提交ACK、重启及后续读取已完整记录；复刻版独立编码仍阻塞，完整显示/触控E验收未完成。"
        transport_layer = 'native_reference_transport_passed_independent_encoder_blocked'
        matrix_transport = ' 原厂基线下载通过；独立编码阻塞 '
    record_path = ROOT / spec['testRun']
    run = json.loads(record_path.read_text(encoding='utf-8-sig'))
    assert run['passed']
    totals = {key: sum(item['totals'][key] for item in run['runs']) for key in ('passed', 'failed', 'skipped')}
    assert totals['failed'] == 0 and totals['skipped'] == 0 and totals['passed'] == spec['expectedPassed']
    progress_path = ROOT / 'docs/stage4-evidence/progress.json'
    progress = json.loads(progress_path.read_text(encoding='utf-8'))
    current = next(item for item in progress['controls'] if item['id'] == spec['id'])
    assert current['status'] in ('queued', 'in_progress')
    current.update(status='editor_slice_delivered_native_layers_blocked', scope=spec['scope'], evidence=spec['testRun'].removeprefix('docs/stage4-evidence/'),
                   layers=dict(D='bounded_spec', U='editor_slice', P_KSP='passed_slice', P_SM='blocked', R_PC='slice', C='blocked', T=transport_layer, E='unobserved'), unresolved=spec['unresolved'])
    if usb_accessed:
        current['hardwareEvidence'] = spec['hardwareRun']
    progress['usbAccessed'] = progress.get('usbAccessed', False) or usb_accessed
    if spec.get('next'):
        next(item for item in progress['controls'] if item['id'] == spec['next'])['status'] = 'in_progress'
    progress['active'] = spec.get('next')
    if progress.get('screenWidthDiagnosis'):
        progress['screenWidthDiagnosis']['activeControl'] = spec.get('next')
        progress['screenWidthDiagnosis']['processedIncludingExisting'] = progress['skippedExisting'] + sum(item['status'] in ('editor_slice_delivered_native_layers_blocked', 'investigated_geometry_blocked') for item in progress['controls'])
    if spec.get('holdNextForHardwareBackfill'):
        next(item for item in progress['controls'] if item['id'] == spec['next'])['status'] = 'queued'
        progress['active'] = 'physical-backfill'
        progress['physicalBackfill'] = dict(state='in_progress', beforeNextControl=spec['next'], authorization='2026-11-13 user physical download requirement', independentEncoderAndNativeReferenceMustRemainDistinct=True)
    progress['editorSlicesDelivered'] = sum(item['status'] == 'editor_slice_delivered_native_layers_blocked' for item in progress['controls'])
    progress['investigationSlicesRecorded'] = sum(item['status'] == 'investigated_geometry_blocked' for item in progress['controls'])
    progress['processedIncludingExisting'] = progress['skippedExisting'] + progress['editorSlicesDelivered'] + progress['investigationSlicesRecorded']
    progress['blockedLayers'] = sum(len(item.get('unresolved', [])) for item in progress['controls'])
    progress_path.write_text(json.dumps(progress, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    next_action = '先执行历史控件实物下载补验，再进入' + spec['next'] if spec.get('holdNextForHardwareBackfill') else spec.get('next', '统一回归')
    progress_doc = ROOT / 'docs/阶段4逐控件进度.md'
    progress_text = progress_doc.read_text(encoding='utf-8-sig')
    progress['investigationSlicesRecorded'] = sum(item['status'] == 'investigated_geometry_blocked' for item in progress['controls'])
    progress_count = progress['skippedExisting'] + progress['editorSlicesDelivered'] + progress['investigationSlicesRecorded']
    progress_text = re.sub(r'^\| 当前步骤 \|.*$', '| 当前步骤 | 已处理'+str(progress_count)+'/41项（含'+str(progress['investigationSlicesRecorded'])+'项几何阻塞调查）；下一项'+str(progress.get('active') or '统一回归')+'；新下载均核查提交/重启后的USB调用 |', progress_text, flags=re.M)
    progress_doc.write_text(progress_text, encoding='utf-8', newline='\n')
    append('docs/阶段4逐控件进度.md', f"## {spec['id']}：第{spec['ordinal']}/41项交付（编辑切片）\n\n{spec['scope']}。本次{totals['passed']}/0/0；原生源/设备/现场未决见{', '.join(spec['unresolved'])}。[专项报告]({spec['report'].removeprefix('docs/')})。下一步{next_action}；未将局部切片声明为整控件或全阶段完成。{hardware_sentence}")
    protocol_path = ROOT / 'docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md'
    protocol = protocol_path.read_text(encoding='utf-8-sig')
    number = max(map(int, re.findall(r'^## (\d+)\.', protocol, re.M))) + 1
    append(str(protocol_path.relative_to(ROOT)), f"## {number}. 阶段4{spec['name']}原厂观测及输出边界（2026-11-13）\n\n{spec['protocol']}\n\nLite当前交付范围：{spec['scope']}。实际Windows私有桌面{totals['passed']}/0/0，结果见[{spec['id']}测试]({spec['testRun'].removeprefix('docs/')})。完整源/设备编码未恢复时继续拒绝输出。{hardware_sentence}")
    append(spec['manual'], f"## 2026-11-13 原生补证与阶段4编辑切片\n\n{spec['nativeSummary']}\n\n本轮编辑实现与分层验收见[专项报告](../{spec['report']})。原手册历史未知项只按本轮已取得结果纠正；源/设备/现场范围保持独立，未宣称全控件通过。")
    matrix_path = ROOT / 'design_plan/控件文档与实施覆盖矩阵.md'
    lines = matrix_path.read_text(encoding='utf-8-sig').splitlines()
    for index, line in enumerate(lines):
        if line.startswith('| ' + spec['name'] + ' |'):
            fields = line.split('|')
            fields[4:13] = [f" [本轮报告](../{spec['report']}) ", ' 编目+本轮补证 ', ' 编辑切片 ', ' KSP切片；SM阻塞 ', ' PC切片 ', ' 阻塞 ', matrix_transport, ' 待现场 ', ' 接续专项报告未决项 ']
            lines[index] = '|'.join(fields)
    matrix_path.write_text('\n'.join(lines) + '\n', encoding='utf-8')
    index_path = ROOT / 'design_plan/A04_31控件实施索引.md'
    index_lines = index_path.read_text(encoding='utf-8-sig').splitlines()
    for index, line in enumerate(index_lines):
        fields = line.split('|')
        if len(fields) == 8 and fields[2].strip() == spec['name'] and spec['id'] in fields[1]:
            fields[5] = f" [本轮报告](../{spec['report']}) "
            index_lines[index] = '|'.join(fields)
        if len(fields) == 8 and fields[2].strip() == '消息显示' and 'C06' in fields[1]:
            fields[5] = ' [本轮报告](../docs/消息显示手册实现与验收.md) '
            index_lines[index] = '|'.join(fields)
    index_path.write_text('\n'.join(index_lines) + '\n', encoding='utf-8')
    ledger_path = ROOT / 'design_plan/分阶段文档证据/阶段交付与清理记录.json'
    ledger = json.loads(ledger_path.read_text(encoding='utf-8-sig'))
    task = next(item for item in ledger['taskStates'] if item['taskId'] == spec['task'])
    previous = {key: copy.deepcopy(task.get(key)) for key in ('documentation', 'investigation', 'implementation', 'blockingDependencies', 'resumePoint')}
    task.setdefault('executionHistory', []).append(dict(executionId=f"stage4-{spec['slug']}-20261113", previousState=previous, recordedAtUtc=datetime.now(timezone.utc).isoformat()))
    task.update(documentation='updated_per_control', investigation=dict(state='native_ui_and_minimal_compile_captured', evidence=spec['nativeManifest']),
                implementation=dict(state='partial_editor_and_KSP_slices', scope=f"本次{spec['id']}编辑切片；全批次和源/设备兼容未完成"),
                resumePoint=dict(document='05_阶段4_控件逐项复刻.md', nextAction=f"{next_action}；保留本控件未决层"))
    ledger.setdefault('stage4Executions', []).append(dict(id=f"stage4-{spec['slug']}-20261113", productPassed=totals['passed'], productFailed=0, productSkipped=0, usbAccessed=usb_accessed, completeControl=False, hardwareEvidence=spec.get('hardwareRun')))
    ledger_path.write_text(json.dumps(ledger, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    review_path = ROOT / f"docs/stage4-evidence/{spec['slug']}/review.json"
    review_path.write_text(json.dumps(dict(mode='read_only_review_before_delivery', findings=spec.get('reviewFindings', []), finalRun=spec['testRun'], tests=totals,
        deviceAndSourceGateVerified=True, nativePixelEqualityClaimed=False, userStage3EditsIncluded=False), ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    modification_number = max(map(int, re.findall(r'^### 修改(\d+)：', (ROOT / '程序修改记录.md').read_text(encoding='utf-8-sig'), re.M)), default=0)+1
    append('程序修改记录.md', f"### 修改{modification_number}：阶段4{spec['name']}独立编辑与事务切片\n- 修改内容：\n  1. {spec['scope']}。\n  2. {spec['nativeSummary']}\n  3. 更新专项报告、协议、覆盖和恢复台账；未知源/设备格式继续拒绝。\n- 修改文件：{spec['codeFiles']}；tests/stage4_controls_tests.cpp、samples/stage4/、docs和design_plan对应记录、本记录。\n- 影响范围：{spec['name']}编辑器/KSP与共享命令分流；已有源/设备格式不扩展猜测布局。\n- 原因：用户要求逐项复刻、完成项跳过、每项更新协议/清理/单独提交，并补充每控件实物下载。\n- 验证：本次真实Windows私有桌面{totals['passed']}/0/0，测试范围详见专项报告；{hardware_sentence}\n- 修改标记：{spec.get('codeMarker', '//20261113-1')} add/change")
    print(json.dumps(dict(id=spec['id'], tests=totals, protocolSection=number, next=progress['active']), ensure_ascii=False))


if __name__ == '__main__':
    main()
