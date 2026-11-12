"""Audit and publish exactly one completed physical backfill without staging unrelated files."""
from pathlib import Path
import datetime
import hashlib
import json
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
WORK = ROOT / 'worktree'


def git(*args, check=True):
    return subprocess.run(['git', '-C', str(WORK), '-c', 'core.quotePath=false', *args], capture_output=True, check=check)


def main():
    identifier = sys.argv[1]
    ledger = json.loads((WORK / 'docs/stage4-evidence/physical-backfill/ledger.json').read_text(encoding='utf-8'))
    item = next(row for row in ledger['records'] if row['id'] == identifier)
    assert item['status'] == 'native_reference_download_passed_independent_encoder_blocked'
    run_path = WORK / item['nativeDownload']['evidence']
    record = json.loads(run_path.read_text(encoding='utf-8'))
    assert all(record[key] for key in ('completed', 'inputUnchanged', 'sourceAtTransferMatchesInput', 'ownedProcessTerminated'))
    assert record['liteIndependentEncoderVerified'] is False
    assert record['sourceProvenance'] == 'retained_native_reference_source'
    assert hashlib.sha256(Path(record['source']).read_bytes()).hexdigest() == record['sourceSha256']
    hardware = record['nativeHardware']
    assert hardware['completionLogWidget'] == 'DownLoadDlg/m_table'
    assert '下载完成' in json.dumps(hardware, ensure_ascii=False)
    assert {row['name'] for row in hardware['transferOptions']} == {'clear_recipe_CB', 'clear_hisData_CB', 'clear_hisAlarm_CB', 'update_CB'}
    assert all(row['afterChecked'] is False for row in hardware['transferOptions'])
    assert (run_path.parent / 'cleanup.json').is_file() and (run_path.parent / 'raw-manifest.json').is_file()
    assert not git('diff', '--cached', '--name-only').stdout.strip(), 'Preserve any staged changes'
    git('diff', '--check')
    report = next((WORK / 'docs').glob(item['name'] + '*手册实现与验收.md'))
    paths = [
        'design_plan/控件文档与实施覆盖矩阵.md',
        'docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md',
        'docs/stage4-evidence/physical-backfill/' + identifier + '-gate.json',
        'docs/stage4-evidence/physical-backfill/ledger.json',
        'docs/stage4-evidence/progress.json', str(report.relative_to(WORK)).replace('\\', '/'),
        'docs/阶段4逐控件进度.md', '程序修改记录.md',
        str(run_path.parent.relative_to(WORK)).replace('\\', '/') + '/'
    ]
    entries = git('status', '--porcelain=v1', '-z', '--untracked-files=all').stdout.decode('utf-8').split('\0')
    changed = [entry[3:] for entry in entries if entry]
    assert changed and all(any(path == candidate or (candidate.endswith('/') and path.startswith(candidate)) for candidate in paths) for path in changed), changed
    previous = git('rev-parse', 'HEAD').stdout.decode().strip()
    now = datetime.datetime.now(datetime.timezone(datetime.timedelta(hours=8), 'Asia/Shanghai'))
    day = now.strftime('%Y年%m月%d日')
    subjects = git('log', '--all', '--format=%s', '--since=' + now.strftime('%Y-%m-%dT00:00:00+08:00')).stdout.decode('utf-8')
    numbers = [int(m.group(1)) for m in re.finditer(re.escape(day) + r' \d\d:\d\d:\d\d，修改(\d+)：', subjects)]
    number = max(numbers, default=0) + 1
    title = now.strftime('%Y年%m月%d日 %H:%M:%S') + f'，修改{number}：补验{item["name"]}原厂基线实物下载并同步独立编码限制'
    git('add', '--', *paths)
    git('commit', '-m', title)
    publication = subprocess.run([sys.executable, str(ROOT / 'publish_stage4.py'), '--previous', previous], capture_output=True, check=True)
    candidate = git('rev-parse', 'HEAD').stdout.decode().strip()
    try:
        pushed = subprocess.run(['git', '-C', str(WORK), 'push', 'origin', 'HEAD:refs/heads/main'], capture_output=True, timeout=35)
        push_ok = pushed.returncode == 0
        detail = pushed.stderr.decode('utf-8', errors='replace')[-1800:]
    except subprocess.TimeoutExpired:
        push_ok, detail = False, 'Git push exceeded 35 seconds; local commit preserved.'
    (ROOT / 'pending-push.json').write_text(json.dumps(dict(status='pushed' if push_ok else 'pending', head=candidate, detail=detail), ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(dict(id=identifier, name=item['name'], commit=candidate, title=title, published=json.loads(publication.stdout), pushed=push_ok, remaining=ledger['queued']), ensure_ascii=False), flush=True)


if __name__ == '__main__':
    main()
