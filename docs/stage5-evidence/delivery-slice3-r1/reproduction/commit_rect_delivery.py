"""Commit the actual third portable package evidence and new rectangle/eight-object samples."""
from pathlib import Path
import datetime
import json
import re
import shutil
import subprocess
import sys

root=Path(__file__).resolve().parent;old=root.parent/'kinseal-stage4-20261112';work=old/'worktree';base=work/'docs/stage5-evidence';out=base/'delivery-slice3-r1'
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
write=lambda p,d:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
manifest=read(out/'package-manifest.json');assert manifest['checksPassed']==53 and read(out/'run.json')['passed'];assert manifest['sourceCommit']=='c0e95d0f1de3768add569010bd001f13a7e40e26'
progress=read(base/'progress.json');progress['latestDelivery']=dict(package=manifest['package'],sourceCommit=manifest['sourceCommit'],evidence='docs/stage5-evidence/delivery-slice3-r1/package-manifest.json',checksPassed=53,usbAccessed=False);write(base/'progress.json',progress)
text=f'第三批便携包通过53项实际检查，源提交`{manifest["sourceCommit"]}`；包含矩形默认/线框、八对象混合、表格及折线/真实SDK宏样例，实际10/12/14段均编译后复读验证。已检查八对象窗口截图，观察到的Qt DLL均来自包内。包路径：`{manifest["package"]}`。没有访问USB，阶段5整体仍未完成。'
def append(path,body):
    p=work/path;p.write_text(p.read_text(encoding='utf-8-sig').rstrip()+'\n\n'+body+'\n',encoding='utf-8',newline='\n')
append('docs/阶段5_矩形设备背景编译与差分验收.md','## 便携交付\n\n'+text+'\n\n[实际53项](stage5-evidence/delivery-slice3-r1/run.json) · [文件清单](stage5-evidence/delivery-slice3-r1/package-manifest.json)')
append('docs/阶段5逐项进度.md',text);append('docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md',text)
append('程序修改记录.md','### 修改76：阶段5矩形便携包和八类混合样例\n- 修改内容：'+text+'\n- 修改文件：samples/stage5新增3个样例、docs/stage5-evidence/delivery-slice3-r1、阶段5报告/进度/协议和本记录。\n- 影响范围：交付与证据；没有新增生产逻辑，不影响用户运行中的阶段4版本。\n- 原因：将第三个独立C切片交付为可运行、可回验的便携包，继续后续格式调查。\n- 修改标记：//20261113-13 add（交付）')
for name in ('commit_rect_delivery.py','prepare_package_slice3.py'):shutil.copy2(root/name,out/'reproduction'/name)
def git(*args):return subprocess.run(['git','-C',str(work),'-c','core.quotePath=false',*args],capture_output=True,check=True)
assert not git('diff','--cached','--name-only').stdout.strip();git('diff','--check');previous=git('rev-parse','HEAD').stdout.decode().strip()
paths=['samples/stage5/rectangle-default.ksp','samples/stage5/rectangle-outline.ksp','samples/stage5/mixed-eight.ksp','docs/stage5-evidence/delivery-slice3-r1/','docs/stage5-evidence/progress.json','docs/阶段5_矩形设备背景编译与差分验收.md','docs/阶段5逐项进度.md','docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md','程序修改记录.md']
now=datetime.datetime.now(datetime.timezone(datetime.timedelta(hours=8)));day=now.strftime('%Y年%m月%d日');subjects=git('log','--all','--format=%s','--since='+now.strftime('%Y-%m-%dT00:00:00+08:00')).stdout.decode('utf-8');sequence=max([int(m.group(1)) for m in re.finditer(re.escape(day)+r' \d\d:\d\d:\d\d，修改(\d+)：',subjects)],default=0)+1
title=now.strftime('%Y年%m月%d日 %H:%M:%S')+f'，修改{sequence}：交付矩形编译器便携包与八类混合离线样例'
git('add','--',*paths);git('commit','-m',title);result=subprocess.run([sys.executable,'-B',str(old/'publish_stage4.py'),'--previous',previous],capture_output=True,check=True);print(result.stdout.decode('utf-8'));print(json.dumps(dict(head=git('rev-parse','HEAD').stdout.decode().strip(),title=title),ensure_ascii=False))
