"""Close one graphic slice using actual software tests, original-source identity and hardware transport evidence."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
root=Path(__file__).resolve().parent;work=root/'worktree'
parser=argparse.ArgumentParser();parser.add_argument('spec',type=Path);parser.add_argument('--run',required=True);args=parser.parse_args()
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
write=lambda p,d:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
spec=read(args.spec);base=work/'docs/stage4-evidence'/spec['slug'];run=read(base/args.run/'run.json')
assert run['passed'] and all(r['totals']['failed']==r['totals']['skipped']==0 for r in run['runs'])
assert sum(r['totals']['passed'] for r in run['runs'])==spec['expectedPassed']
hardware=read(work/spec['hardwareRun']);assert all(hardware.get(k) for k in ['completed','inputUnchanged','sourceAtTransferMatchesInput','ownedProcessTerminated'])
assert hardware['sourceProvenance']=='retained_native_reference_source' and hardware['liteIndependentEncoderVerified'] is False
assert all(x['afterChecked'] is False for x in hardware['nativeHardware']['transferOptions'])
assert hashlib.sha256((base/'native'/spec['source']).read_bytes()).hexdigest()==hardware['sourceSha256']
log=(base/args.run/'KinSealStage4ControlTests.txt').read_text(encoding='utf-8-sig')
assert all('PASS   : Stage4ControlTests::'+method+'()' in log for method in spec['methods'])
sample=base/args.run/(spec['slug']+'-only.ksp');assert sample.is_file()
shutil.copy2(sample,work/'samples/stage4'/(spec['slug']+'.ksp'))
environment=os.environ.copy();temporary=root/'task-temp'/(spec['slug']+'-gates');temporary.mkdir(parents=True,exist_ok=True)
environment.update(TEMP=str(temporary),TMP=str(temporary),TMPDIR=str(temporary))
environment['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;'+environment.get('PATH','')
executable=work/'build/KinSealStudioLite.exe';calls=[]
for label,argv in [('capabilities',['--project-capabilities',str(sample)]),('compile',['--compile-project',str(sample),str(base/'unpublished-package')]),('export-source',['--export-native-project',str(sample),str(base/'unpublished.sm')])]:
    r=subprocess.run([str(executable),*argv],capture_output=True,env=environment,timeout=30,creationflags=subprocess.CREATE_NO_WINDOW)
    calls.append(dict(label=label,exitCode=r.returncode,result=json.loads(r.stdout.decode('utf-8-sig')),stderr=r.stderr.decode('utf-8-sig',errors='replace')))
assert calls[0]['exitCode']==0 and calls[0]['result']['modelPersistence']['allowed']
assert calls[1]['exitCode']!=0 and calls[2]['exitCode']!=0 and not (base/'unpublished.sm').exists() and not (base/'unpublished-package').exists()
write(base/'backend-gates.json',dict(executableSha256=hashlib.sha256(executable.read_bytes()).hexdigest(),inputSha256=hashlib.sha256(sample.read_bytes()).hexdigest(),calls=calls,usbAccessed=False))
spec['testRun']=str((base/args.run/'run.json').relative_to(work)).replace('\\','/')
report=f"""# {spec['name']}：阶段4实现与验收

本轮第{spec['ordinal']}/41项，编号{spec['id']}。这是编辑/KSP切片；完整源、独立设备编码和现场显示不在已通过范围。

## 原厂依据

{spec['nativeSummary']}

[原始证据清单](stage4-evidence/{spec['slug']}/native/retained-manifest.json)从主目录已有调查逐文件核验SHA复制，原始调查目录未修改。源输入SHA为`{hardware['sourceSha256']}`；原厂编译清单见[编译观测](stage4-evidence/{spec['slug']}/native/{spec['slug']}-compiled-observations.json)。不存在ControlPara/ControlPixel的情形保留为“未生成”，没有虚构零字节段。

## 实现范围

{spec['scope']}。

{spec.get('fieldMappingMarkdown', '')}

{spec['renderLimit']}

## 实际验证

[私有桌面测试](stage4-evidence/{spec['slug']}/{args.run}/run.json)合计{spec['expectedPassed']}/0/0；新增方法：{', '.join(spec['methods'])}。取消、未支持输入和源/设备失败路径均验证原工程或输出不变。测试样例见[样例](../samples/stage4/{spec['slug']}.ksp)。

[实际CLI门槛](stage4-evidence/{spec['slug']}/backend-gates.json)允许KSP保存，并拒绝未知的SM和设备输出，未创建伪包。

[实物下载](stage4-evidence/{spec['slug']}/physical/{spec['slug']}-baseline-r1/run.json)使用保留原厂SM，原厂编译和下载日志均完成，四清除/更新项false，下载输入SHA一致，结束后已终止自有进程。T只证明原厂基线传输成功；Lite独立输出、屏端显示、触控和PLC运行均未升级为通过。

## 只读复核与清理

{chr(10).join('- '+x for x in spec['reviewFindings'])}

原厂下载运行目录按文件身份清理，保留完整观测gzip、源和编译段；[物理清理](stage4-evidence/{spec['slug']}/physical/{spec['slug']}-baseline-r1/cleanup.json)。软件重复输出另见本目录cleanup.json。宿主/构建/测试的TEMP、TMP、TMPDIR均定向独占Z目录。

## 未决

{chr(10).join('- '+x for x in spec['unresolvedText'])}

按A03建议实施顺序进入{spec.get('next', '统一回归')}，原G编号保持不变。
"""
(work/spec['report']).write_text(report,encoding='utf-8',newline='\n')
matrix=work/'design_plan/控件文档与实施覆盖矩阵.md'
text=matrix.read_text(encoding='utf-8-sig')
if not any(line.startswith('| '+spec['name']+' |') for line in text.splitlines()):
    if '## 基础图形与表格逐项实施' not in text:
        text+='\n\n## 基础图形与表格逐项实施\n\n已有原厂调查随逐项交付核验，主规范为A03，原G编号保持。\n\n| 控件 | 行为手册 | 原始候选数 | 实施与验收依据 | D | U | P | R | C | T | E | 下一步 |\n|---|---|---:|---|---|---|---|---|---|---|---|---|\n'
    text+='| '+spec['name']+' | [A03](A03_基础图形与表格规范.md) | 未计 | 待更新 | 编目 | 待验 | 待验 | 待验 | 待验 | 待验 | 待验 | 待验 |\n'
    matrix.write_text(text,encoding='utf-8',newline='\n')
write(args.spec,spec)
subprocess.run([sys.executable,'-B',str(root/'finalize_xy.py'),str(args.spec)],check=True)
repro=base/'reproduction';repro.mkdir(exist_ok=True)
for name in spec.get('recipes',[])+['archive_graphic_evidence.py','preflight_graphics_contract.py','deliver_graphic.py','commit_graphic.py','finalize_xy.py','run_stage4_native_physical.py','archive_physical_run.py','execute_native_cleanup.ps1','prepare_control_cleanup.py','execute_control_cleanup.ps1']:
    shutil.copy2(root/name,repro/name)
shutil.copy2(args.spec,repro/args.spec.name)
for p in root.glob(spec['slug']+'-build-*.log'):shutil.copy2(p,base/p.name)
print('Delivered '+spec['id']+' from actual evidence.')
