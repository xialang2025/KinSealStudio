"""Correct active document identities from A03 and stage4 reports without rewriting historical raw evidence."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree'
authority=(work/'design_plan/A03_基础图形与表格规范.md').read_text(encoding='utf-8')
assert '| G05 | 圆角矩形 |' in authority and '| G06 | 圆/椭圆 |' in authority
paths=['docs/阶段5_圆角矩形定点背景编译与差分验收.md','docs/阶段5逐项进度.md','docs/阶段5_多边形单元格背景编译与差分验收.md','docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md','docs/stage5-evidence/progress.json','design_plan/06_阶段5_设备编译与格式差分.md','design_plan/A05_源格式与设备编译契约.md','design_plan/复刻待验线索台账.csv','design_plan/00_复刻执行总入口.md']
changes=[]
for name in paths:
 p=work/name;old=p.read_text(encoding='utf-8-sig');text=old.replace('G06圆角','G05圆角').replace('G06便携','G05便携').replace('G05椭圆','G06椭圆').replace('Q-S05-G06-C,阶段5圆角','Q-S05-G05-C,阶段5圆角')
 if '圆角矩形' in p.name:text=text.replace('C切片G06','C切片G05')
 if text!=old:p.write_text(text,encoding='utf-8');changes.append(name)
p=work/'design_plan/A03_基础图形与表格规范.md';text=authority.replace('| G05 | 圆角矩形 | 编辑/KSP切片；源/设备阻塞 |','| G05 | 圆角矩形 | 编辑/KSP切片；半径10受限背景C；完整源阻塞 |');p.write_text(text,encoding='utf-8');changes.append(str(p.relative_to(work)))
(root/'active-graphic-id-correction.json').write_text(json.dumps(dict(authority='A03 current table + stage4 named reports',G05='roundrect',G06='ellipse',activeFiles=changes,historicalRawAndReproductionUnchanged=True,priorLog83NeedsSupersedingEntry=True),ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps(dict(correctedFiles=len(changes),historicalRawUnchanged=True)))
