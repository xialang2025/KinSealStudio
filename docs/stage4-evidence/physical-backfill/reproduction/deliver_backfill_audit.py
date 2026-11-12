"""Publish the completed baseline audit and preserve the still-blocked independent download requirement."""
from pathlib import Path
import datetime
import json
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
WORK = ROOT / 'worktree'
BASE = WORK / 'docs/stage4-evidence/physical-backfill'


def git(*args):
    return subprocess.run(['git', '-C', str(WORK), *args], capture_output=True, check=True).stdout


def append(path, content):
    with path.open('a', encoding='utf-8', newline='\n') as stream:
        stream.write('\n\n' + content + '\n')


def main():
    audit = json.loads((BASE / 'completion-audit.json').read_text(encoding='utf-8'))
    assert audit['historyReferenceBackfill'] == 14 and audit['pendingReferenceBackfill'] == 0
    protocol = WORK / 'docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md'
    assert '历史原厂基线14项补验总核对' not in protocol.read_text(encoding='utf-8-sig')
    number = max(map(int, re.findall(r'^## (\d+)\.', protocol.read_text(encoding='utf-8-sig'), re.M))) + 1
    text = ('历史14项原厂基线逐项USB下载完成，加上C20原厂基线与已有5类独立传输，20个已处理控件都有分清来源的传输记录。'
            '本次总复核校验全部归档原始数据SHA、源SHA、14份新版单类型输入SHA与独立源/编译拒绝结果、四项下载选项关闭、进程结束以及所有清理目标不存在。'
            f'14轮合计清理{audit["deletedBackfillFiles"]}个文件、逻辑{audit["deletedLogicalBytes"]}B，不以硬链接逻辑大小声称物理释放量。'
            '15类新增控件独立编码仍阻塞，屏显/触控无新增通过，不能将20项传输覆盖解释成新版20控件全部可下载。'
            '[总报告](历史控件实物下载补验汇总.md)及[机器审计](stage4-evidence/physical-backfill/completion-audit.json)保留逐项路径。')
    append(protocol, f'## {number}. 历史原厂基线14项补验总核对（2026-11-12）\n\n' + text)
    append(WORK / 'docs/阶段4逐控件进度.md', '## 历史下载补验总核对及C21恢复\n\n' + text + '\n\n原厂基线补验14/14结束，开发顺序恢复C21移动轨迹。阶段4仍为20/41项已处理；新增独立编码和现场显示/触控继续保留未决。')
    append(WORK / '程序修改记录.md', '### 历史实物补验总核对\n- 修改内容：汇总C06～C19共14项真实原厂基线下载，复核C01～C05既有独立传输、C20有效第二轮、输入/归档哈希及清理完成；保持15类新增独立编码阻塞与现场E未验，恢复C21开发顺序。\n- 修改文件：docs/历史控件实物下载补验汇总.md、下载协议、阶段4进度、physical-backfill审计及复现配方、本记录。\n- 影响范围：传输证据与后续逐控件交付门槛；程序业务逻辑未改。\n- 原因：落实先补验历史控件、以后逐控件实物下载的新增要求，避免将原厂基线冒充新版独立生成通过。\n- 验证：14/14原厂基线下载，5份既有传输记录复核，15类新增独立编码保持阻塞，归档原文SHA一致且644个清理目标不存在。\n- 修改标记：//20261112-0 change')
    shutil.copy2(ROOT / 'deliver_backfill_audit.py', BASE / 'reproduction/deliver_backfill_audit.py')
    paths = ['docs/历史控件实物下载补验汇总.md', 'docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md', 'docs/阶段4逐控件进度.md', 'docs/stage4-evidence/progress.json',
             'docs/stage4-evidence/physical-backfill/completion-audit.json', 'docs/stage4-evidence/physical-backfill/reproduction/audit_physical_backfill.py',
             'docs/stage4-evidence/physical-backfill/reproduction/commit_physical_backfill.py', 'docs/stage4-evidence/physical-backfill/reproduction/complete_physical_backfill.ps1',
             'docs/stage4-evidence/physical-backfill/reproduction/deliver_backfill_audit.py', '程序修改记录.md']
    changed = [entry[3:] for entry in git('status', '--porcelain=v1', '-z', '--untracked-files=all').decode('utf-8').split('\0') if entry]
    assert set(changed) == set(paths), changed
    assert not git('diff', '--cached', '--name-only').strip()
    git('diff', '--check')
    previous = git('rev-parse', 'HEAD').decode().strip()
    now = datetime.datetime.now(datetime.timezone(datetime.timedelta(hours=8), 'Asia/Shanghai'))
    day = now.strftime('%Y年%m月%d日')
    subjects = git('log', '--all', '--format=%s', '--since=' + now.strftime('%Y-%m-%dT00:00:00+08:00')).decode('utf-8')
    number_commit = max([int(m.group(1)) for m in re.finditer(re.escape(day) + r' \d\d:\d\d:\d\d，修改(\d+)：', subjects)], default=0) + 1
    title = now.strftime('%Y年%m月%d日 %H:%M:%S') + f'，修改{number_commit}：汇总十四项原厂基线实物补验并保留新版独立下载阻塞'
    git('add', '--', *paths)
    git('commit', '-m', title)
    subprocess.run([sys.executable, str(ROOT / 'publish_stage4.py'), '--previous', previous], check=True)
    pushed = subprocess.run(['git', '-C', str(WORK), 'push', 'origin', 'HEAD:refs/heads/main'], capture_output=True, timeout=35)
    candidate = git('rev-parse', 'HEAD').decode().strip()
    (ROOT / 'pending-push.json').write_text(json.dumps(dict(status='pushed' if pushed.returncode == 0 else 'pending', head=candidate, detail=pushed.stderr.decode('utf-8', errors='replace')[-1500:]), ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(dict(commit=candidate, pushed=pushed.returncode == 0, protocolSection=number, next='C21'), ensure_ascii=False))


if __name__ == '__main__':
    main()
