"""Commit the reviewed roundrect slice and the backfilled UI compilation rules only."""
from pathlib import Path
import datetime
import json
import re
import shutil
import subprocess

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree'
def git(*args):return subprocess.run(['git','-C',str(work),*args],check=True,capture_output=True).stdout
assert git('rev-parse','HEAD').decode().strip()=='f70adbe67d992fa63c18a4fd9ec468c5247f4cc3'
repro=work/'docs/stage5-evidence/native/roundrect/reproduction'
for name in ('commit_roundrect.py','finalize_roundrect_docs.py'):shutil.copy2(root/name,repro/name)
for n in ('r1','r2'):shutil.copy2(root/('roundrect-build-'+n+'.log'),work/'docs/stage5-evidence/roundrect-windows-r2'/('build-'+n+'.log'))
paths=['CMakeLists.txt','src/device_roundrect_raster.cpp','src/device_roundrect_raster.h','src/device_compile_plan.h','src/device_compile_plan.cpp','src/independent_downloader.cpp','tests/stage5_compiler_tests.cpp','docs/UI编译规则与阶段交付规范.md','docs/阶段5_圆角矩形定点背景编译与差分验收.md','docs/阶段5逐项进度.md','docs/阶段4逐控件进度.md','docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md','docs/stage5-evidence/progress.json','docs/stage5-evidence/native/roundrect','docs/stage5-evidence/native/audit','docs/stage5-evidence/roundrect-windows-r1','docs/stage5-evidence/roundrect-windows-r2','docs/stage5-evidence/roundrect-editor-windows-r1','docs/stage5-evidence/roundrect-editor-windows-r2','docs/stage5-evidence/roundrect-red-r1','design_plan/00_复刻执行总入口.md','design_plan/06_阶段5_设备编译与格式差分.md','design_plan/A05_源格式与设备编译契约.md','design_plan/控件文档与实施覆盖矩阵.md','design_plan/复刻待验线索台账.csv','程序修改记录.md']
status=git('status','--short','--untracked-files=no').decode('utf-8');assert status.strip()
existing=list(filter(None,git('diff','--cached','--name-only','-z').decode().split('\0')))
assert all(any(name==path or name.startswith(path+'/') for path in paths) for name in existing)
git('add','--',*paths)
staged=list(filter(None,git('diff','--cached','--name-only','-z').decode().split('\0')))
assert all(any(name==path or name.startswith(path+'/') for path in paths) for name in staged)
# Preserve exact third-party source and captured build-log bytes, including upstream trailing spaces.
git('diff','--cached','--check','--','.',':(exclude)docs/stage5-evidence/native/roundrect/reproduction/public-qt-sources/qrasterizer-qt463.cpp',':(exclude)docs/stage5-evidence/roundrect-windows-r2/build-r1.log')
now=datetime.datetime.now(datetime.timezone(datetime.timedelta(hours=8)));day=now.strftime('%Y年%m月%d日')
subjects=git('log','--all','-400','--format=%s').decode()
sequence=max([int(m[1]) for m in re.finditer(re.escape(day)+r' \d\d:\d\d:\d\d，修改(\d+)：',subjects)],default=0)+1
message=now.strftime('%Y年%m月%d日 %H:%M:%S')+f'，修改{sequence}：新增圆角矩形定点背景编译并补齐各阶段UI编译规则'
git('commit','-m',message)
result=dict(commit=git('rev-parse','HEAD').decode().strip(),message=message,files=len(staged),stage5Complete=False)
(root/'roundrect-commit.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps(result,ensure_ascii=False))
