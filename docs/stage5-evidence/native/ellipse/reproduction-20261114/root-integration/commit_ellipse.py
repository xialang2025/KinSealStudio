"""Commit the verified G06 slice, current identity correction and bounded evidence only."""
from pathlib import Path
import datetime,json,re,shutil,subprocess
root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree'
def git(*a):return subprocess.run(['git','-C',str(work),*a],check=True,capture_output=True).stdout
assert git('rev-parse','HEAD').decode().strip()=='5c32629d87e868cbffb8642b29e68a0d3053fb5c'
assert not git('diff','--cached','--name-only').strip()
paths=['CMakeLists.txt','src/device_ellipse_raster.cpp','src/device_ellipse_raster.h','src/device_polygon_raster.cpp','src/device_polygon_raster.h','src/device_compile_plan.cpp','src/device_compile_plan.h','src/independent_downloader.cpp','tests/stage5_compiler_tests.cpp','docs/UI编译规则与阶段交付规范.md','docs/阶段5_椭圆定点细分背景编译与差分验收.md','docs/阶段5_圆角矩形定点背景编译与差分验收.md','docs/阶段5_多边形单元格背景编译与差分验收.md','docs/阶段5逐项进度.md','docs/阶段4逐控件进度.md','docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md','docs/stage5-evidence/progress.json','docs/stage5-evidence/native/ellipse','docs/stage5-evidence/ellipse-red-r1','docs/stage5-evidence/ellipse-windows-r2','docs/stage5-evidence/ellipse-editor-windows-r2','design_plan/00_复刻执行总入口.md','design_plan/06_阶段5_设备编译与格式差分.md','design_plan/A03_基础图形与表格规范.md','design_plan/A05_源格式与设备编译契约.md','design_plan/控件文档与实施覆盖矩阵.md','design_plan/复刻待验线索台账.csv','程序修改记录.md']
repro=work/'docs/stage5-evidence/native/ellipse/reproduction-20261114/root-integration';repro.mkdir(exist_ok=True)
for name in ('add_ellipse_tests.py','run_ellipse_red.py','finalize_ellipse_docs.py','correct_active_graphic_ids.py','active-graphic-id-correction.json','commit_ellipse.py','prepare_ellipse_delivery.py','package_slice8.py'):
 shutil.copy2(root/name,repro/name)
git('status','--short');git('add','--',*paths)
staged=list(filter(None,git('diff','--cached','--name-only','-z').decode().split('\0')));assert all(any(n==p or n.startswith(p+'/') for p in paths) for n in staged)
git('diff','--cached','--check','--','.',':(exclude)docs/stage5-evidence/native/',':(exclude)docs/stage5-evidence/ellipse-windows-r2/build-r1.log',':(exclude)docs/stage5-evidence/ellipse-windows-r2/build-r2.log',':(exclude)docs/stage5-evidence/ellipse-red-r1/ellipse-red.txt')
now=datetime.datetime.now(datetime.timezone(datetime.timedelta(hours=8)));day=now.strftime('%Y年%m月%d日');subjects=git('log','--all','-400','--format=%s').decode();sequence=max([int(m[1]) for m in re.finditer(re.escape(day)+r' \d\d:\d\d:\d\d，修改(\d+)：',subjects)],default=0)+1
message=now.strftime('%Y年%m月%d日 %H:%M:%S')+f'，修改{sequence}：新增椭圆定点背景编译并修正混色舍入与图形编号'
git('commit','-m',message);result=dict(commit=git('rev-parse','HEAD').decode().strip(),message=message,files=len(staged),stage5Complete=False)
(root/'ellipse-commit.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(json.dumps(result,ensure_ascii=False))
