"""Derive a new seventh portable recipe while preserving all earlier delivery identities."""
from pathlib import Path
root=Path(__file__).resolve().parent
text=(root.parent/'kinseal-stage5-20261113/package_slice6.py').read_text(encoding='utf-8')
text=text.replace('fixed-scanline roundrect compiler','independent cell-area polygon compiler').replace('slice6','slice7').replace('roundrect-windows-r2','polygon-windows-r1').replace('mixed-eleven','mixed-twelve').replace('十一类控件混合示例','十二类控件混合示例').replace('十一对象','十二对象')
marker="mixed=read(work/'samples/stage5/mixed-nine.ksp')"
addition="""polygon=copy.deepcopy(rectangle);polygon['projectName']='多边形默认样例';polygon_object=polygon['pages'][0]['objects'][0];polygon_object.update(type='polygon',id='stage5-polygon',name='多边形1',height=120);polygon_object['graphic'].update(foreground='#ffffffff',background='#ff000000',points=[[0,0],[1,0],[.75,1]]);write(samples/'polygon-default.ksp',polygon)
polygon_outline=copy.deepcopy(polygon);polygon_outline['projectName']='多边形线框';polygon_outline['pages'][0]['objects'][0]['graphic']['fillStyle']=0;write(samples/'polygon-outline.ksp',polygon_outline)
polygon_concave=copy.deepcopy(polygon);polygon_concave['projectName']='凹五边形';polygon_concave['pages'][0]['objects'][0].update(height=130);polygon_concave['pages'][0]['objects'][0]['graphic']['points']=[[0,0],[1,0],[.55,50/130],[.9,1],[.1,1]];write(samples/'polygon-concave.ksp',polygon_concave)
"""
assert marker in text;text=text.replace(marker,addition+marker)
marker="mixed['pages'][0]['objects'].append(rounded_mixed);write(samples/'mixed-twelve.ksp',mixed)"
assert marker in text;text=text.replace(marker,"mixed['pages'][0]['objects'].append(rounded_mixed);polygon_mixed=copy.deepcopy(polygon_object);polygon_mixed.update(x=590,y=80,width=100,height=100);mixed['pages'][0]['objects'].append(polygon_mixed);write(samples/'mixed-twelve.ksp',mixed)")
text=text.replace("[('roundrect-default',10)","[('polygon-default',10),('polygon-outline',10),('polygon-concave',10),('roundrect-default',10)")
marker="    check('roundrect-source-gate'"
addition="""    check('polygon-source-gate',['--export-native-project',str(samples/'polygon-default.ksp'),str(out/'unpublished-polygon.sm')],20)
    check('polygon-legacy-gate',['--compile-single-text',str(samples/'polygon-default.ksp'),str(out/'unpublished-polygon-legacy')],19)
    bad_polygon=copy.deepcopy(polygon);bad_polygon['pages'][0]['objects'][0]['graphic']['points']=[[0,0],[1,1],[0,1],[1,0]];write(out/'unsupported-polygon-self-cross.ksp',bad_polygon)
    check('polygon-self-cross-gate',['--compile-project',str(out/'unsupported-polygon-self-cross.ksp'),str(out/'unpublished-polygon-self-cross')],19)
    for refused in ('unpublished-polygon.sm','unpublished-polygon-legacy','unpublished-polygon-self-cross'):assert not (out/refused).exists()
"""
assert marker in text;text=text.replace(marker,addition+marker)
text=text.replace('阶段5第六批：圆角矩形、两端点自由直线','阶段5第七批：多边形、圆角矩形、两端点自由直线')
text=text.replace('圆角矩形支持半径10/10',r'多边形仅3至8点简单整数闭合形，X为2至698，Y为2至398，宽高至少100；黑边/白前景/黑背景、宽1、不透明；拒绝自交、重复、共线、相交及接触。\n圆角矩形支持半径10/10')
destination=root/'package_slice7.py';assert not destination.exists();destination.write_text(text,encoding='utf-8')
print(destination)
