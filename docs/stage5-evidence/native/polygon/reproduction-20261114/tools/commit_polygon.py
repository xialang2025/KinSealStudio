"""Commit only the verified seventh background family and its measured evidence."""
from pathlib import Path
import datetime,json,re,shutil,subprocess
root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree'
def git(*args):return subprocess.run(['git','-C',str(work),*args],check=True,capture_output=True).stdout
assert git('rev-parse','HEAD').decode().strip()=='d311c1ac11ce75e7ad941d3ff8077be28cb74fbd'
paths=['CMakeLists.txt','src/device_polygon_raster.cpp','src/device_polygon_raster.h','src/device_compile_plan.cpp','src/device_compile_plan.h','src/independent_downloader.cpp','tests/stage5_compiler_tests.cpp','docs/UI编译规则与阶段交付规范.md','docs/阶段5_多边形单元格背景编译与差分验收.md','docs/阶段5逐项进度.md','docs/阶段4逐控件进度.md','docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md','docs/stage5-evidence/progress.json','docs/stage5-evidence/native/polygon','docs/stage5-evidence/native/sector-investigation-20261114','docs/stage5-evidence/polygon-windows-r1','docs/stage5-evidence/polygon-editor-windows-r1','docs/stage5-evidence/polygon-red-r1','design_plan/00_复刻执行总入口.md','design_plan/06_阶段5_设备编译与格式差分.md','design_plan/A03_基础图形与表格规范.md','design_plan/A05_源格式与设备编译契约.md','design_plan/控件文档与实施覆盖矩阵.md','design_plan/复刻待验线索台账.csv','程序修改记录.md']
assert not git('diff','--cached','--name-only').strip()
status=git('status','--short','--untracked-files=no').decode();assert status.strip()
repro=work/'docs/stage5-evidence/native/polygon/reproduction-20261114/tools'
for name in ('analyze_polygon_corpus.py','run_polygon_red.py','finalize_polygon_docs.py','commit_polygon.py','prepare_polygon_delivery.py','package_slice7.py','cleanup_polygon_decoded.ps1'):
 source=root/name;target=repro/name;assert source.is_file()
 if target.exists():assert target.read_bytes()==source.read_bytes()
 else:shutil.copy2(source,target)
git('add','--',*paths)
staged=list(filter(None,git('diff','--cached','--name-only','-z').decode().split('\0')))
assert all(any(n==p or n.startswith(p+'/') for p in paths) for n in staged)
# Preserve captured diagnostics and third-party bytes exactly, including their original whitespace.
git('diff','--cached','--check','--','.',':(exclude)docs/stage5-evidence/native/',':(exclude)docs/stage5-evidence/polygon-windows-r1/build-r1.log',':(exclude)docs/stage5-evidence/polygon-red-r1/polygon-red.txt')
now=datetime.datetime.now(datetime.timezone(datetime.timedelta(hours=8)));day=now.strftime('%Y年%m月%d日');subjects=git('log','--all','-400','--format=%s').decode()
sequence=max([int(m[1]) for m in re.finditer(re.escape(day)+r' \d\d:\d\d:\d\d，修改(\d+)：',subjects)],default=0)+1
message=now.strftime('%Y年%m月%d日 %H:%M:%S')+f'，修改{sequence}：新增多边形单元格面积背景编译并记录完整差分规则'
git('commit','-m',message)
result=dict(commit=git('rev-parse','HEAD').decode().strip(),message=message,files=len(staged),stage5Complete=False)
(root/'polygon-commit.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(json.dumps(result,ensure_ascii=False))
