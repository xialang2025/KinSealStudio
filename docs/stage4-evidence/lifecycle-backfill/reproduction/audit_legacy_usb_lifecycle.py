"""Revalidate historical UI-only reference runs, cleaning and committing each actual device result."""
from pathlib import Path
import datetime
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
WORK = ROOT / 'worktree'
AUDIT = WORK / 'docs/stage4-evidence/lifecycle-backfill'


def read(path):
    """Input: JSON path. Return: Decoded retained record."""
    return json.loads(path.read_text(encoding='utf-8-sig'))


def write(path, value):
    """Input: JSON path and record. Return: None after writing UTF8 evidence."""
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def git(*arguments):
    """Input: Explicit Git arguments. Return: Captured result from the isolated checkout."""
    return subprocess.run(['git', '-C', str(WORK), *arguments], capture_output=True, check=True)


def step(name, arguments):
    """Input: Phase and argv. Return: Actual success; retain nonzero diagnostics and stop."""
    result = subprocess.run(arguments, cwd=ROOT, env=os.environ.copy(), capture_output=True, timeout=180,
                            creationflags=subprocess.CREATE_NO_WINDOW)
    log = ROOT / ('lifecycle-' + name + '.txt') if name.endswith(('-publish', '-push')) else AUDIT / (name + '.txt')
    log.write_bytes(result.stdout + result.stderr)
    if result.returncode:
        raise RuntimeError(name + ': ' + (result.stdout + result.stderr).decode('utf-8', errors='replace')[-2500:])


def main():
    """Input: None. Return: Per-control physical completion records with isolated commits and pushes."""
    #20261113-4 add: Earlier UI rows remain historical evidence until a fresh full lifecycle succeeds.
    AUDIT.mkdir(parents=True, exist_ok=True)
    reproduction = AUDIT / 'reproduction'
    reproduction.mkdir(exist_ok=True)
    for name in ('audit_legacy_usb_lifecycle.py', 'run_stage4_native_physical.py', 'usb_trace_agent.js',
                 'archive_physical_run.py', 'execute_native_cleanup.ps1'):
        shutil.copy2(ROOT / name, reproduction / name)
    progress_path = WORK / 'docs/stage4-evidence/progress.json'
    if (AUDIT / 'queue.json').exists():
        queue = read(AUDIT / 'queue.json')
    else:
        queue = []
        for item in read(progress_path)['controls']:
            if not item.get('hardwareEvidence'):
                continue
            record = read(WORK / item['hardwareEvidence'])
            if record.get('nativeHardware', {}).get('usbCompletion', {}).get('ready'):
                continue
            queue.append(dict(id=item['id'], name=item['name'], previous=item['hardwareEvidence'],
                              source=record['source'], sourceSha256=record['sourceSha256'],
                              family=Path(item['hardwareEvidence']).parts[2]))
        write(AUDIT / 'queue.json', queue)
    state = read(AUDIT / 'audit.json') if (AUDIT / 'audit.json').exists() else dict(total=len(queue), completed=[], complete=False)
    for item in queue:
        if any(entry['id'] == item['id'] for entry in state['completed']):
            continue
        slug = item['family']
        run = slug + '-lifecycle-r1'
        source = Path(item['source'])
        assert source.is_file() and hashlib.sha256(source.read_bytes()).hexdigest() == item['sourceSha256']
        previous_main = subprocess.check_output(['git', '-C', str(ROOT.parents[1] / 'KinSealStudio'), 'rev-parse', 'HEAD']).decode().strip()
        prefix = item['id'].lower()
        directory = WORK / 'docs/stage4-evidence' / slug / 'physical' / run
        if not (directory / 'run.json').exists():
            step(prefix + '-transfer', [sys.executable, '-B', str(ROOT / 'run_stage4_native_physical.py'),
                 '--family', slug, '--run', run, '--project', str(source), '--download'])
        record = read(directory / 'run.json')
        assert record['completed'] and record['nativeHardware']['usbCompletion']['resetReadCompleted']
        step(prefix + '-raw-inspection', [sys.executable, '-B', str(WORK / 'tools/inspect_native_usb_trace.py'), str(directory)])
        if not (directory / 'raw-manifest.json').exists():
            step(prefix + '-archive', [sys.executable, '-B', str(ROOT / 'archive_physical_run.py'), slug, run])
        plan = ROOT / 'native-physical' / ('physical-cleanup-' + run + '.json')
        step(prefix + '-cleanup', [shutil.which('pwsh.exe') or 'pwsh.exe', '-NoProfile', '-NonInteractive', '-File', str(ROOT / 'execute_native_cleanup.ps1'), '-Plan', str(plan)])
        shutil.copy2(Path(str(plan) + '.result.json'), directory / 'cleanup.json')
        relative = (directory / 'run.json').relative_to(WORK).as_posix()
        entry = dict(id=item['id'], name=item['name'], evidence=relative, sourceSha256=record['sourceSha256'],
                     resetReadCompleted=True, clearFlagsHex=record['nativeHardware']['usbCompletion']['clearFlagsHex'],
                     screenDisplayConfirmed=False, independentEncoderVerified=False)
        state['completed'].append(entry)
        state['complete'] = len(state['completed']) == len(queue)
        write(AUDIT / 'audit.json', state)
        progress = read(progress_path)
        current = next(control for control in progress['controls'] if control['id'] == item['id'])
        current.setdefault('legacyHardwareEvidence', []).append(current['hardwareEvidence'])
        current['hardwareEvidence'] = relative
        progress['lifecycleBackfill'] = dict(total=len(queue), completed=len(state['completed']), complete=state['complete'],
                                             evidence='docs/stage4-evidence/lifecycle-backfill/audit.json')
        write(progress_path, progress)
        summary_path = WORK / 'docs/历史控件实物下载补验汇总.md'
        summary = summary_path.read_text(encoding='utf-8-sig')
        heading = '## 2026-11-13完整下载生命周期复核'
        if heading not in summary:
            summary += '\n\n' + heading + '\n\n旧UI完成记录保留；以下逐项重新下载，确认开始/提交ACK、完整重启写入和重启后读取退出，四清除/更新关闭。仍仅证明原厂基线传输，不新增独立编码、屏显/触控/PLC结论。\n\n| 编号 | 控件 | 新实物证据 |\n|---|---|---|\n'
        summary += '| ' + item['id'] + ' | ' + item['name'] + ' | [完整提交/重启](' + relative.removeprefix('docs/') + ') |\n'
        summary_path.write_text(summary, encoding='utf-8', newline='\n')
        protocol_path = WORK / 'docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md'
        protocol = protocol_path.read_text(encoding='utf-8-sig')
        if '<!-- lifecycle-backfill-20261113 -->' not in protocol:
            number = max(map(int, re.findall(r'^## (\d+)\.', protocol, re.M))) + 1
            protocol += '\n\n## ' + str(number) + '. 历史原厂基线完整下载生命周期复核（2026-11-13）\n\n<!-- lifecycle-backfill-20261113 -->\n\n采用§73修复后的完成条件逐项重新实物下载；保留旧轮次，所有新轮次关闭清除/更新并保存原始USB。T不升级为独立编码或E通过。\n\n| 编号 | 控件 | 原厂源SHA256 | 新下载证据 |\n|---|---|---|---|\n'
        protocol += '| ' + item['id'] + ' | ' + item['name'] + ' | `' + record['sourceSha256'] + '` | [提交/重启完整](' + relative.removeprefix('docs/') + ') |\n'
        protocol_path.write_text(protocol, encoding='utf-8', newline='\n')
        journal_path = WORK / '程序修改记录.md'
        journal = journal_path.read_text(encoding='utf-8-sig')
        number = max(map(int, re.findall(r'^### 修改(\d+)：', journal, re.M)), default=0) + 1
        journal += '\n\n### 修改' + str(number) + '：' + item['id'] + item['name'] + '原厂实物下载生命周期补验\n- 修改内容：使用保留原厂源重新编译并下载，确认开始/提交ACK、完整重启写入及重启后读取退出；关闭四清除/更新，保留原始USB和旧轮次，清理本轮自有临时文件。\n- 修改文件：对应控件physical新轮次证据、协议、历史补验汇总、进度及本记录。\n- 影响范围：本控件原厂基线传输证据，不修改控件业务逻辑。\n- 原因：旧验证过早终止进程，UI日志完成不足以证明重启完成。\n- 验证：真实设备原始USB及源SHA验证通过；独立设备编码、屏显/触控/PLC未扩大通过。\n- 修改标记：//20261113-4 add/change\n'
        journal_path.write_text(journal, encoding='utf-8', newline='\n')
        paths = ['docs/stage4-evidence/lifecycle-backfill/', str(directory.relative_to(WORK)).replace('\\', '/') + '/',
                 'docs/stage4-evidence/progress.json', 'docs/历史控件实物下载补验汇总.md',
                 'docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md', '程序修改记录.md']
        assert not git('diff', '--cached', '--name-only').stdout.strip()
        git('status', '--porcelain=v1')
        git('diff', '--check')
        git('add', '--', *paths)
        staged = filter(None, git('diff', '--cached', '--name-only', '-z').stdout.decode('utf-8').split('\0'))
        assert all(any(p == allowed or (allowed.endswith('/') and p.startswith(allowed)) for allowed in paths) for p in staged)
        now = datetime.datetime.now(datetime.timezone(datetime.timedelta(hours=8), 'Asia/Shanghai'))
        day = now.strftime('%Y年%m月%d日')
        subjects = git('log', '--all', '--format=%s', '--since=' + now.strftime('%Y-%m-%dT00:00:00+08:00')).stdout.decode('utf-8')
        sequence = max([int(m.group(1)) for m in re.finditer(re.escape(day) + r' \d\d:\d\d:\d\d，修改(\d+)：', subjects)], default=0) + 1
        git('commit', '-m', now.strftime('%Y年%m月%d日 %H:%M:%S') + f'，修改{sequence}：补验{item["name"]}原厂下载提交和完整重启')
        step(prefix + '-publish', [sys.executable, '-B', str(ROOT / 'publish_stage4.py'), '--previous', previous_main])
        step(prefix + '-push', [sys.executable, '-B', str(ROOT / 'push_stage4_current.py')])
        print(json.dumps(dict(completed=len(state['completed']), total=len(queue), id=item['id'], name=item['name'],
                              commit=git('rev-parse', '--short', 'HEAD').stdout.decode().strip()), ensure_ascii=False), flush=True)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
