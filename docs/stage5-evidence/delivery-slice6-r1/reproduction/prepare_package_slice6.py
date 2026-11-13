"""Derive the sixth actual portable verification recipe without replacing any previous package."""
from pathlib import Path

root=Path(__file__).resolve().parent
text=(root/'package_slice5.py').read_text(encoding='utf-8')
text=text.replace('two-endpoint freeline compiler','fixed-scanline roundrect compiler').replace('slice5','slice6').replace('freeline-windows-r1','roundrect-windows-r2').replace('mixed-ten','mixed-eleven').replace('十类控件混合示例','十一类控件混合示例')
marker="mixed=read(work/'samples/stage5/mixed-nine.ksp')"
new="""rounded=copy.deepcopy(rectangle);rounded['projectName']='圆角矩形默认样例';rounded_object=rounded['pages'][0]['objects'][0];rounded_object.update(type='roundrect',id='stage5-roundrect',name='圆角矩形1');rounded_object['graphic'].update(radiusX=10,radiusY=10);write(samples/'roundrect-default.ksp',rounded)
rounded_outline=copy.deepcopy(rounded);rounded_outline['projectName']='圆角矩形线框';rounded_outline['pages'][0]['objects'][0]['graphic']['fillStyle']=0;write(samples/'roundrect-outline.ksp',rounded_outline)
rounded_odd=copy.deepcopy(rounded);rounded_odd['projectName']='圆角矩形奇数尺寸';rounded_odd['pages'][0]['objects'][0].update(width=201,height=101);write(samples/'roundrect-odd.ksp',rounded_odd)
"""
assert marker in text;text=text.replace(marker,new+marker)
marker="mixed['pages'][0]['objects'].append(item);write(samples/'mixed-eleven.ksp',mixed)"
assert marker in text;text=text.replace(marker,"mixed['pages'][0]['objects'].append(item);rounded_mixed=copy.deepcopy(rounded_object);rounded_mixed.update(x=350,y=80);mixed['pages'][0]['objects'].append(rounded_mixed);write(samples/'mixed-eleven.ksp',mixed)")
text=text.replace("[('table-default',10)","[('roundrect-default',10),('roundrect-outline',10),('roundrect-odd',10),('table-default',10)")
marker="    check('capture',['--capture-preview'"
extra="""    check('roundrect-source-gate',['--export-native-project',str(samples/'roundrect-default.ksp'),str(out/'unpublished-roundrect.sm')],20)
    check('roundrect-legacy-gate',['--compile-single-text',str(samples/'roundrect-default.ksp'),str(out/'unpublished-roundrect-legacy')],19)
    bad_round=copy.deepcopy(rounded);bad_round['pages'][0]['objects'][0]['graphic']['radiusX']=20;write(out/'unsupported-roundrect-radius.ksp',bad_round)
    check('roundrect-radius-gate',['--compile-project',str(out/'unsupported-roundrect-radius.ksp'),str(out/'unpublished-roundrect-radius')],19)
    for refused in ('unpublished-roundrect.sm','unpublished-roundrect-legacy','unpublished-roundrect-radius'):assert not (out/refused).exists()
"""
assert marker in text;text=text.replace(marker,extra+marker)
text=text.replace('阶段5第五批：两端点自由直线','阶段5第六批：圆角矩形、两端点自由直线').replace('查看十对象混合示例','查看十一对象混合示例')
text=text.replace('自由直线仅两端点，',r'圆角矩形支持半径10/10、X/Y至少1、宽高至少100、黑白/宽1/Alpha255、线型0/1×填充0/1，拒绝其它样式/半径/相交或接触；设备采用独立定点扫描。\n自由直线仅两端点，')
destination=root/'package_slice6.py';assert not destination.exists();destination.write_text(text,encoding='utf-8')
print(str(destination))
