"""Check the completed reference backfill and retain distinct independent-encoder and screen-runtime gates."""
from pathlib import Path
import datetime
import gzip
import hashlib
import json
import shutil

ROOT = Path(__file__).resolve().parent
WORK = ROOT / 'worktree'
BASE = WORK / 'docs/stage4-evidence/physical-backfill'


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def write(path, data):
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    assert not (BASE / 'completion-audit.json').exists(), 'Do not overwrite a completed audit'
    ledger = read(BASE / 'ledger.json')
    assert ledger['queued'] == 0 and ledger['referenceBackfillComplete'] is True
    assert ledger['complete'] is False and len(ledger['records']) == 20
    rows = []
    removed = logical_bytes = 0
    for item in ledger['records']:
        if item['status'] == 'existing_independent_transport_evidence_retained':
            path = WORK / item['evidence']
            assert sha(path) == item['sha256']
            old = read(path)
            assert old['exitCode'] == 0 and old.get('transportResult', old.get('result', {}))['ok'] is True
            rows.append(dict(id=item['id'], name=item['name'], provenance='existing_independent_transport', evidence=item['evidence'], evidenceSha256=sha(path)))
            continue
        assert item['status'] == 'native_reference_download_passed_independent_encoder_blocked'
        relative = item.get('nativeDownload', {}).get('evidence', item.get('evidence'))
        path = WORK / relative
        record = read(path)
        assert all(record[k] is True for k in ('completed', 'inputUnchanged', 'sourceAtTransferMatchesInput', 'ownedProcessTerminated'))
        assert record['sourceProvenance'] == 'retained_native_reference_source'
        assert record['liteIndependentEncoderVerified'] is False and record['desktopSwitched'] is False and record['systemInput'] is False
        assert sha(Path(record['source'])) == record['sourceSha256']
        hardware = record['nativeHardware']
        assert hardware['completionLogWidget'] == 'DownLoadDlg/m_table' and '下载完成' in json.dumps(hardware, ensure_ascii=False)
        assert len(hardware['transferOptions']) == 4 and all(row['afterChecked'] is False for row in hardware['transferOptions'])
        assert hardware['screenDisplayConfirmed'] is False and hardware['screenTouchConfirmed'] is False
        raw = read(path.parent / 'raw-manifest.json')
        assert raw['allNativeCompilationOutputsRetained'] is True
        for archive in raw['records']:
            content = gzip.decompress((path.parent / archive['archive']).read_bytes())
            assert hashlib.sha256(content).hexdigest() == archive['originalSha256']
        assert any(path.parent.rglob('*.dat')), 'Full compilation payload must remain available'
        cleanup = read(path.parent / 'cleanup.json')
        assert cleanup['deletedFiles'] == len(cleanup['files'])
        assert all(not Path(entry['path']).exists() for entry in cleanup['files'])
        if item['id'] != 'C20':
            removed += cleanup['deletedFiles']
            logical_bytes += int(cleanup['logicalBytes'])
            assert sha(WORK / item['productInput']['path']) == item['productInput']['sha256']
            gate = item['independentProductGate']['calls']
            assert gate[0]['result']['modelPersistence']['allowed'] is True
            assert gate[1]['exitCode'] != 0 and gate[2]['exitCode'] != 0
        rows.append(dict(id=item['id'], name=item['name'], provenance=record['sourceProvenance'], evidence=relative.replace('\\', '/'), evidenceSha256=sha(path), sourceSha256=record['sourceSha256'], independentEncoder='blocked', displayAndTouch='unobserved', cleanupFiles=cleanup['deletedFiles']))
    assert sum(row['provenance'] == 'existing_independent_transport' for row in rows) == 5
    assert len(rows) - 5 == 15
    result = dict(completedAtUtc=datetime.datetime.now(datetime.timezone.utc).isoformat(), retainedExistingIndependentTransport=5, newReferenceTransport=15, historyReferenceBackfill=14, pendingReferenceBackfill=0,
                  independentNewEncodersBlocked=15, newDisplayOrTouchPasses=0, deletedBackfillFiles=removed, deletedLogicalBytes=logical_bytes,
                  logicalBytesAreNotPhysicalSpace=True, allArchivedRawHashesVerified=True, allCleanupTargetsAbsent=True,
                  allIndependentProductInputsUnchanged=True, completeIndependentDownloadRequirement=False, records=rows)
    write(BASE / 'completion-audit.json', result)
    report = ['# 历史控件实物下载补验汇总', '', '日期：2026-11-12。历史原厂基线补验14/14已完成；加上C20本项，15类新增控件均取得原厂基线下载完成记录。C01～C05沿用并复核了既有独立传输证据。', '',
              '15类新增控件的新版独立源/设备编码仍阻塞，因此“新版生成控件全部可下载”的目标尚未完成。下载完成不证明屏幕显示、触控或PLC行为；本轮没有新增这些通过项。', '',
              '| 编号 | 控件 | 输入来源 | 传输证据 |', '|---|---|---|---|']
    for row in rows:
        label = '既有独立传输' if row['provenance'] == 'existing_independent_transport' else '保留原厂源→原厂编译'
        relative = row['evidence'].replace('\\', '/').removeprefix('docs/')
        report.append(f'| {row["id"]} | {row["name"]} | {label} | [实际记录]({relative}) |')
    report.extend(['', '每轮重查唯一健康7365:616C设备，关闭并核验三项清除和强制底层更新，核对输入及传输源SHA，读取DownLoadDlg/m_table的实际完成日志，再终止自有进程。原始压缩记录和完整编译产物已保留，所有压缩记录哈希已复核。', '',
                   f'14轮补验合计清理{removed}个文件，逻辑大小{logical_bytes}B；硬链接的逻辑大小不代表实际磁盘释放量。已逐项核对清理目标不存在，后续临时目录继续使用Z盘。', '',
                   'C20首轮沿用原厂清除项且完成日志读取位置错误的历史问题仍保留，不能被第二轮成功覆盖；第二轮及历史14项都已强制关闭四选项。见[滚轮报告](滚轮手册实现与验收.md)。', '',
                   '以后每项都记录新版单控件源导出/独立编译的真实结果，并执行已支持路径的实物下载。独立编码未恢复的控件保持阻塞，原厂基线仅作参照，不替代新版通过。下一开发项C21移动轨迹；阶段4仍为20/41项已处理，完整功能层尚未全部通过。', '',
                   '[机器审计](stage4-evidence/physical-backfill/completion-audit.json) · [逐项台账](stage4-evidence/physical-backfill/ledger.json) · [协议](AMT070W124-L800X480R_USB下载协议_已验证部分.md)', ''])
    (WORK / 'docs/历史控件实物下载补验汇总.md').write_text('\n'.join(report), encoding='utf-8')
    progress_path = WORK / 'docs/stage4-evidence/progress.json'
    progress = read(progress_path)
    progress['physicalBackfill'].update(state='reference_backfill_complete_independent_encoders_blocked', audit='docs/stage4-evidence/physical-backfill/completion-audit.json')
    progress['active'] = 'C21'
    next(row for row in progress['controls'] if row['id'] == 'C21')['status'] = 'in_progress'
    write(progress_path, progress)
    (BASE / 'reproduction').mkdir(exist_ok=True)
    for name in ('commit_physical_backfill.py', 'complete_physical_backfill.ps1', 'audit_physical_backfill.py'):
        shutil.copy2(ROOT / name, BASE / 'reproduction' / name)
    print(json.dumps({key: value for key, value in result.items() if key != 'records'}, ensure_ascii=False))


if __name__ == '__main__':
    main()
