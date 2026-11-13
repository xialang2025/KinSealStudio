"""Record the verified second portable delivery and publish only its own documentation and samples."""
from pathlib import Path
import datetime
import hashlib
import json
import re
import shutil
import subprocess
import sys

root=Path(__file__).resolve().parent;old=root.parent/'kinseal-stage4-20261112';work=old/'worktree';base=work/'docs/stage5-evidence';out=base/'delivery-slice2-r2'
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
write=lambda p,d:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
manifest=read(out/'package-manifest.json');assert manifest['checksPassed']==47 and read(out/'run.json')['passed'] and read(out/'cleanup.json')['deletedFiles']==28
assert manifest['sourceCommit']=='bbfceacee645255670da9f0ba4eb59c555bee4c5'
progress=read(base/'progress.json');progress['latestDelivery']=dict(package=manifest['package'],sourceCommit=manifest['sourceCommit'],evidence='docs/stage5-evidence/delivery-slice2-r2/package-manifest.json',checksPassed=47,usbAccessed=False);write(base/'progress.json',progress)
text=f'第二批便携包已通过47项真实CLI/包核验，源提交`{manifest["sourceCommit"]}`。包含默认/无外框/30×30表格、七对象混合及折线/宏样例，实际10/12/14段均重新编译验证。所有观察到的Qt DLL来自包内；已检查七对象窗口截图。包路径：`{manifest["package"]}`。首轮样例生成器误用C++成员名，KSP严格校验正确拒绝；修正生成器后通过，没有放宽模型。失败输入/日志保留，28个重复文件共113972851逻辑字节已核保留peer后清理。本批未访问USB。'
def append(path,body):
    p=work/path;p.write_text(p.read_text(encoding='utf-8-sig').rstrip()+'\n\n'+body+'\n',encoding='utf-8',newline='\n')
append('docs/阶段5_静态表格设备背景编译与差分验收.md','## 便携交付\n\n'+text+'\n\n[实际47项结果](stage5-evidence/delivery-slice2-r2/run.json) · [文件清单](stage5-evidence/delivery-slice2-r2/package-manifest.json) · [清理回执](stage5-evidence/delivery-slice2-r2/cleanup.json)')
append('docs/阶段5逐项进度.md',text)
append('docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md',text)
append('程序修改记录.md','### 修改74：阶段5静态表格便携包与七类混合样例\n- 修改内容：'+text+'\n- 修改文件：samples/stage5新增4个样例、docs/stage5-evidence/delivery-slice2-r1/r2及阶段5报告/进度/协议、本记录。\n- 影响范围：交付样例及证据，没有新增生产逻辑，保留用户正在运行的阶段4程序。\n- 原因：将已提交的第二个C切片交付为可运行便携包，完成独立样例和定向清理。\n- 修改标记：//20261113-12 add（交付）')
for name in ('commit_table_delivery.py','cleanup_table_failed_delivery.ps1'):shutil.copy2(root/name,out/'reproduction'/name)
def git(*args):return subprocess.run(['git','-C',str(work),'-c','core.quotePath=false',*args],capture_output=True,check=True)
assert not git('diff','--cached','--name-only').stdout.strip();git('diff','--check');previous=git('rev-parse','HEAD').stdout.decode().strip()
paths=['samples/stage5/table-default.ksp','samples/stage5/table-no-outline.ksp','samples/stage5/table-thirty.ksp','samples/stage5/mixed-seven.ksp','docs/stage5-evidence/delivery-slice2-r1/','docs/stage5-evidence/delivery-slice2-r2/','docs/stage5-evidence/progress.json','docs/阶段5_静态表格设备背景编译与差分验收.md','docs/阶段5逐项进度.md','docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md','程序修改记录.md']
now=datetime.datetime.now(datetime.timezone(datetime.timedelta(hours=8)));day=now.strftime('%Y年%m月%d日');subjects=git('log','--all','--format=%s','--since='+now.strftime('%Y-%m-%dT00:00:00+08:00')).stdout.decode('utf-8')
sequence=max([int(m.group(1)) for m in re.finditer(re.escape(day)+r' \d\d:\d\d:\d\d，修改(\d+)：',subjects)],default=0)+1
title=now.strftime('%Y年%m月%d日 %H:%M:%S')+f'，修改{sequence}：交付静态表格便携包与七类混合离线样例'
git('add','--',*paths);git('commit','-m',title)
result=subprocess.run([sys.executable,'-B',str(old/'publish_stage4.py'),'--previous',previous],capture_output=True,check=True);print(result.stdout.decode('utf-8'));print(json.dumps(dict(head=git('rev-parse','HEAD').stdout.decode().strip(),title=title),ensure_ascii=False))
