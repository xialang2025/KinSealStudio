"""Materialize a standalone rectangle delivery recipe from the verified second-slice recipe."""
from pathlib import Path
import ast

root=Path(__file__).resolve().parent
source=(root/'package_slice2.py').read_text(encoding='utf-8')
source=source.replace('delivery-slice2-','delivery-slice3-').replace('compile-slice2-','compile-slice3-').replace('package_slice2.py','package_slice3.py').replace('table-windows-r1','rect-windows-r1')
old="mixed=read(work/'samples/stage5/mixed-six.ksp');mixed['projectName']='七类控件混合示例';item=copy.deepcopy(obj);item.update(x=400,y=200);mixed['pages'][0]['objects'].append(item);write(samples/'mixed-seven.ksp',mixed)"
new="""rectangle=copy.deepcopy(table);rectangle['projectName']='矩形默认样例';rect=rectangle['pages'][0]['objects'][0];rect.update(type='rect',id='stage5-rectangle',name='矩形1')
rect['graphic']=dict(lineStyle=1,lineWidth=1,lineColor='#ff000000',alpha=255,fillStyle=1,foreground='#ff000000',background='#ffffffff');write(samples/'rectangle-default.ksp',rectangle)
outline=copy.deepcopy(rectangle);outline['projectName']='矩形线框样例';outline['pages'][0]['objects'][0]['graphic']['fillStyle']=0;write(samples/'rectangle-outline.ksp',outline)
mixed=read(work/'samples/stage5/mixed-seven.ksp');mixed['projectName']='八类控件混合示例';item=copy.deepcopy(rect);item.update(x=100,y=300);mixed['pages'][0]['objects'].append(item);write(samples/'mixed-eight.ksp',mixed)"""
assert source.count(old)==1;source=source.replace(old,new)
source=source.replace("('mixed-seven',12)","('mixed-eight',12),('rectangle-default',10),('rectangle-outline',10)")
source=source.replace("str(samples/'mixed-seven.ksp')","str(samples/'mixed-eight.ksp')")
source=source.replace('阶段5第二批：纯静态表格、正交折线及既有五类控件','阶段5第三批：矩形、纯静态表格、正交折线及既有五类控件').replace('samples/mixed-seven.ksp查看七对象','samples/mixed-eight.ksp查看八对象')
source=source.replace('表格支持1..30行列','矩形支持宽高至少100、黑白、宽1、Alpha255、线型0/1与填充0/1，拒绝小尺寸/其它样式/重叠或接触。\\n表格支持1..30行列')
ast.parse(source);target=root/'package_slice3.py';assert not target.exists();target.write_text(source,encoding='utf-8',newline='\n')
print(str(target))
