"""Update one control's actual reference download and independent-encoder refusal without conflating them."""
from pathlib import Path
import argparse
import datetime
import json
import re
root = Path(__file__).resolve().parent
work = root / 'worktree'
base = work / 'docs/stage4-evidence/physical-backfill'


def write(path, value):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def append(path, value):
    with path.open('a', encoding='utf-8', newline='\n') as stream:
        stream.write('\n\n' + value.strip() + '\n')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('id')
    parser.add_argument('run')
    args = parser.parse_args()
    ledger_path = base / 'ledger.json'
    ledger = json.loads(ledger_path.read_text(encoding='utf-8'))
    item = next(r for r in ledger['records'] if r['id'] == args.id)
    assert item['status'] == 'queued_native_reference_download_independent_encoder_blocked', 'Do not finalize the same control twice'
    family = item['family']
    relative = f'docs/stage4-evidence/{family}/physical/{args.run}/run.json'
    path = work / relative
    hardware = json.loads(path.read_text(encoding='utf-8'))
    cleanup = json.loads((path.parent / 'cleanup.json').read_text(encoding='utf-8-sig'))
    assert hardware['completed'] and hardware['inputUnchanged'] and hardware['sourceAtTransferMatchesInput'] and hardware['ownedProcessTerminated']
    assert hardware['sourceProvenance'] == 'retained_native_reference_source' and hardware['liteIndependentEncoderVerified'] is False
    assert all(not option['afterChecked'] for option in hardware['nativeHardware']['transferOptions'])
    assert hardware['nativeHardware']['completionLogWidget'] == 'DownLoadDlg/m_table'
    assert hardware['sourceSha256'] == item['nativeReferenceInput']['sha256']
    assert item['independentProductGate']['calls'][1]['exitCode'] != 0 and item['independentProductGate']['calls'][2]['exitCode'] != 0
    item.update(status='native_reference_download_passed_independent_encoder_blocked', nativeDownload=dict(evidence=relative, sourceSha256=hardware['sourceSha256'], completedAtUtc=hardware['completedAtUtc'], cleanup=dict(deletedFiles=cleanup['deletedFiles'], logicalBytes=cleanup['logicalBytes'])), displayAndTouchNotInferred=True)
    queued = [r['id'] for r in ledger['records'] if r['status'] == 'queued_native_reference_download_independent_encoder_blocked']
    ledger['queued'] = len(queued)
    ledger['newNativeReferenceCompleted'] = sum(r['status'] == 'native_reference_download_passed_independent_encoder_blocked' for r in ledger['records'])
    ledger['next'] = queued[0] if queued else None
    ledger['referenceBackfillComplete'] = not queued
    ledger['complete'] = False  # Independent encoding and display/touch still have their own unresolved gates.
    write(ledger_path, ledger)
    write(base / (args.id + '-gate.json'), item)
    progress_path = work / 'docs/stage4-evidence/progress.json'
    progress = json.loads(progress_path.read_text(encoding='utf-8'))
    control = next(c for c in progress['controls'] if c['id'] == args.id)
    control['layers']['T'] = 'native_reference_transport_passed_independent_encoder_blocked'
    control['hardwareEvidence'] = relative
    progress['usbAccessed'] = True
    progress['physicalBackfill'].update(nativeReferenceCompleted=sum(r['id'] != 'C20' and r['status'] == 'native_reference_download_passed_independent_encoder_blocked' for r in ledger['records']), totalBackfill=14, next=ledger['next'], referenceBackfillComplete=not queued)
    write(progress_path, progress)
    protocol = work / 'docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md'
    number = max(map(int, re.findall(r'^## (\d+)\.', protocol.read_text(encoding='utf-8-sig'), re.M))) + 1
    link = relative.removeprefix('docs/')
    gate_link = f'stage4-evidence/physical-backfill/{args.id}-gate.json'
    result = f"{args.id} {item['name']}：本轮保留原厂源SHA256 `{hardware['sourceSha256']}`，恰好1个可选根对象后原厂重新编译并USB实物下载；即时识别7365:616C、libusb0设备，从DownLoadDlg/m_table读到“下载完成”。清配方/清历史数据/清历史报警/强制底层更新均关闭并核验false，源输入未变且传输SHA一致，原厂进程已终止。真实结果见[下载记录]({link})。"
    boundary = f"本次是原厂保留源/原厂编译路径的设备基线；[新版单类型KSP门槛]({gate_link})实际模型校验通过，但源导出与独立设备编译仍拒绝。两套输入设置没有声明等价，原厂下载成功不替代新版独立编码兼容。没有逐字节USB抓包或屏内读回，日志完成不升级为完整帧/记录布局恢复；显示、触控、PLC行为未新增通过。"
    append(protocol, f'## {number}. 历史控件实物下载补验：{item["name"]}（2026-11-12）\n\n{result}\n\n{boundary}\n\n完整下载编译产物、原始压缩记录和清理记录保留；删除该轮{cleanup["deletedFiles"]}个已结束运行/Frida宿主/冗余文件，逻辑{int(cleanup["logicalBytes"])}B，不将硬链接逻辑大小当物理释放。')
    report = next((work / 'docs').glob(item['name'] + '*手册实现与验收.md'), None)
    if report is None:
        candidates = {'消息显示': '消息显示手册实现与验收.md', '图片显示': '图片显示手册实现与验收.md', 'GIF显示': 'GIF显示手册实现与验收.md', '数值显示': '数值显示手册实现与验收.md', 'ASCII显示': 'ASCII显示手册实现与验收.md', '时间显示': '时间显示手册实现与验收.md'}
        report = work / 'docs' / candidates[item['name']]
    assert report.is_file()
    append(report, '## 2026-11-12 实物下载补验\n\n' + result + '\n\n' + boundary + f'\n\n该轮已清理{cleanup["deletedFiles"]}个临时/冗余文件；完整编译包保留。历史编辑切片测试及原SM/C层未决项不回填为全部通过。')
    matrix_path = work / 'design_plan/控件文档与实施覆盖矩阵.md'
    lines = matrix_path.read_text(encoding='utf-8-sig').splitlines()
    assert any(line.startswith('| ' + item['name'] + ' |') for line in lines)
    for index, line in enumerate(lines):
        if line.startswith('| ' + item['name'] + ' |'):
            fields = line.split('|')
            fields[10] = ' 原厂基线下载通过；独立编码阻塞 '
            lines[index] = '|'.join(fields)
    matrix_path.write_text('\n'.join(lines) + '\n', encoding='utf-8')
    append(work / 'docs/阶段4逐控件进度.md', f'### 历史实物补验{14-len(queued)}/14：{args.id} {item["name"]}\n\n原厂保留源实际下载完成；新版独立源/设备编码仍拒绝，显示/触控未确认。协议§{number}与专项报告已同步；本轮临时文件已清理。下一项{ledger["next"] or "实物补验汇总"}。')
    append(work / '程序修改记录.md', f'### 实物补验{args.id}：{item["name"]}原厂基线与新版门槛\n- 修改内容：\n  1. 单类型新版KSP实际模型校验、源导出/独立编译拒绝记录；保留原厂源单根守卫、即时设备核验及真实USB下载完成记录。\n  2. 分开记录原厂基线成功和新版编码阻塞，更新协议§{number}、专项报告、覆盖和进度；清理{cleanup["deletedFiles"]}个已结束临时文件，完整编译包保留。\n- 修改文件：docs/stage4-evidence/{family}/physical/、physical-backfill/、协议与专项报告、阶段4进度/覆盖矩阵、本记录。\n- 影响范围：{item["name"]}下载验证文档及证据；没有扩展未知SM或设备编码。\n- 原因：用户要求补验此前未实物下载控件，并在以后每项交付时实物下载。\n- 验证：原厂下载窗口实际日志“下载完成”；新版独立编码仍阻塞，显示/触控未验。\n- 修改标记：//20261112-0 change')
    print(json.dumps(dict(id=args.id, protocolSection=number, referenceBackfillDone=14-len(queued), next=ledger['next']), ensure_ascii=False))


if __name__ == '__main__':
    main()
