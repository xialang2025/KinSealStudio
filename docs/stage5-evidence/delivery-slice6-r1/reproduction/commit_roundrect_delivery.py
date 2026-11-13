"""Record and commit the actual sixth portable package and its rule update."""
from pathlib import Path
import datetime
import json
import re
import shutil
import subprocess
import sys

root=Path(__file__).resolve().parent;old=root.parent/'kinseal-stage4-20261112';work=old/'worktree';base=work/'docs/stage5-evidence';out=base/'delivery-slice6-r1'
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
write=lambda p,d:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
manifest=read(out/'package-manifest.json');assert manifest['checksPassed']==90 and read(out/'run.json')['passed']
assert manifest['sourceCommit']=='8c330efc1a83f6dab2b8f8d392edd8f933666931'
progress=read(base/'progress.json');progress['latestDelivery']=dict(package=manifest['package'],sourceCommit=manifest['sourceCommit'],evidence='docs/stage5-evidence/delivery-slice6-r1/package-manifest.json',checksPassed=90,usbAccessed=False);progress['active']='G06便携与规则补齐已交付，继续其余图形、动态记录与多页配置';write(base/'progress.json',progress)
text=f'第六批便携包通过90项实际检查，源提交`{manifest["sourceCommit"]}`；包含圆角矩形默认/线框/奇数尺寸及十一对象混合，10/12/14段均实际编译并复读。十一对象截图已实际检查，观察到的Qt DLL均来自包内。包路径：`{manifest["package"]}`。没有USB通信，阶段5整体仍未完成。'
def append(path,body):
    p=work/path;p.write_text(p.read_text(encoding='utf-8-sig').rstrip()+'\n\n'+body+'\n',encoding='utf-8')
append('docs/阶段5_圆角矩形定点背景编译与差分验收.md','## 第六批便携交付\n\n'+text+'\n\n[实际90项](stage5-evidence/delivery-slice6-r1/run.json) · [文件清单](stage5-evidence/delivery-slice6-r1/package-manifest.json)')
append('docs/阶段5逐项进度.md',text);append('docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md',text)
append('docs/UI编译规则与阶段交付规范.md','## 12. 第六批便携任务的规则与复验记录\n\n'+text+'\n\n本任务没有修改生产编码规则，复用本文第7～9节。新增样例键分别为roundrect-default/outline/odd，均radiusX=radiusY=10；outline仅fillStyle=0，odd为201×101。mixed-eleven中圆角位置350,80/200×100，仍只有5条动态控制记录。包内实际CLI验证源出口、旧单文本入口及radiusX=20拒绝，失败不生成目标；已有目录保护和损坏包退出23继续通过。Qt运行路径/EXE SHA绑定上述清单，历史包不覆盖。')
append('程序修改记录.md','### 修改82：阶段5圆角矩形便携包与十一对象样例\n- 修改内容：'+text+'\n- 修改文件：samples/stage5新增4个样例、docs/stage5-evidence/delivery-slice6-r1、阶段5报告/进度/协议、UI编译规则文档和本记录。\n- 影响范围：交付和证据；没有新增生产逻辑，不影响用户运行中的阶段4版本。\n- 原因：形成第六个受限静态C切片的可运行程序和实际离线复验样例，并按新增要求同步每阶段性任务的UI编译规则。\n- 修改标记：//20261113-16 add（交付）')
for name in ('commit_roundrect_delivery.py','prepare_package_slice6.py'):shutil.copy2(root/name,out/'reproduction'/name)
def git(*args):return subprocess.run(['git','-C',str(work),*args],check=True,capture_output=True).stdout
assert not git('diff','--cached','--name-only').strip();previous=git('rev-parse','HEAD').decode().strip();assert previous==manifest['sourceCommit']
paths=['samples/stage5/roundrect-default.ksp','samples/stage5/roundrect-outline.ksp','samples/stage5/roundrect-odd.ksp','samples/stage5/mixed-eleven.ksp','docs/stage5-evidence/delivery-slice6-r1','docs/stage5-evidence/progress.json','docs/UI编译规则与阶段交付规范.md','docs/阶段5_圆角矩形定点背景编译与差分验收.md','docs/阶段5逐项进度.md','docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md','程序修改记录.md']
git('status','--short');git('add','--',*paths);git('diff','--cached','--check')
staged=list(filter(None,git('diff','--cached','--name-only','-z').decode().split('\0')));assert all(any(n==p or n.startswith(p+'/') for p in paths) for n in staged)
now=datetime.datetime.now(datetime.timezone(datetime.timedelta(hours=8)));day=now.strftime('%Y年%m月%d日');subjects=git('log','--all','-400','--format=%s').decode();sequence=max([int(m[1]) for m in re.finditer(re.escape(day)+r' \d\d:\d\d:\d\d，修改(\d+)：',subjects)],default=0)+1
message=now.strftime('%Y年%m月%d日 %H:%M:%S')+f'，修改{sequence}：交付圆角矩形便携包并记录十一对象编译规则'
git('commit','-m',message)
result=dict(commit=git('rev-parse','HEAD').decode().strip(),message=message,files=len(staged),previous=previous)
(root/'package6-commit.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(json.dumps(result,ensure_ascii=False),flush=True)
subprocess.run([sys.executable,'-B',str(old/'publish_stage4.py'),'--previous',previous],check=True)
subprocess.run([sys.executable,'-B',str(old/'push_stage4_current.py')],check=True)
