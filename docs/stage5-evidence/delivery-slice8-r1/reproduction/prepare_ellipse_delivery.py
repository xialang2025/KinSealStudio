"""Prepare the next portable recipe, retaining both ellipse rounding regression cases."""
from pathlib import Path
root=Path(__file__).resolve().parent
text=(root/'package_slice7.py').read_text(encoding='utf-8').replace('independent cell-area polygon compiler','fixed-point ellipse compiler').replace('slice7','slice8').replace('polygon-windows-r1','ellipse-windows-r2').replace('mixed-twelve','mixed-thirteen').replace('十二类控件混合示例','十三类控件混合示例').replace('十二对象','十三对象')
marker='polygon=copy.deepcopy(rectangle)'
addition="""ellipse=copy.deepcopy(rectangle);ellipse['projectName']='圆/椭圆默认样例';ellipse_object=ellipse['pages'][0]['objects'][0];ellipse_object.update(type='ellipse',id='stage5-ellipse',name='圆/椭圆1');write(samples/'ellipse-default.ksp',ellipse)
ellipse_circle=copy.deepcopy(ellipse);ellipse_circle['projectName']='圆形样例';ellipse_circle['pages'][0]['objects'][0].update(width=200,height=200);write(samples/'ellipse-circle.ksp',ellipse_circle)
ellipse_wide=copy.deepcopy(ellipse);ellipse_wide['projectName']='宽椭圆舍入回归';ellipse_wide['pages'][0]['objects'][0].update(width=300,height=150);write(samples/'ellipse-wide.ksp',ellipse_wide)
ellipse_fill=copy.deepcopy(ellipse);ellipse_fill['projectName']='无描边白填充椭圆舍入回归';ellipse_fill['pages'][0]['objects'][0]['graphic']['lineStyle']=0;write(samples/'ellipse-fill-only.ksp',ellipse_fill)
"""
assert marker in text;text=text.replace(marker,addition+marker,1)
marker="mixed['pages'][0]['objects'].append(polygon_mixed);write(samples/'mixed-thirteen.ksp',mixed)"
assert marker in text;text=text.replace(marker,"mixed['pages'][0]['objects'].append(polygon_mixed);ellipse_mixed=copy.deepcopy(ellipse_object);ellipse_mixed.update(x=260,y=190,width=100,height=100);mixed['pages'][0]['objects'].append(ellipse_mixed);write(samples/'mixed-thirteen.ksp',mixed)")
text=text.replace("[('polygon-default',10)","[('ellipse-default',10),('ellipse-circle',10),('ellipse-wide',10),('ellipse-fill-only',10),('polygon-default',10)")
marker="    check('polygon-source-gate'"
addition="""    check('ellipse-source-gate',['--export-native-project',str(samples/'ellipse-default.ksp'),str(out/'unpublished-ellipse.sm')],20)
    check('ellipse-legacy-gate',['--compile-single-text',str(samples/'ellipse-default.ksp'),str(out/'unpublished-ellipse-legacy')],19)
    bad_ellipse=copy.deepcopy(ellipse);bad_ellipse['pages'][0]['objects'][0]['graphic']['alpha']=128;write(out/'unsupported-ellipse-alpha.ksp',bad_ellipse)
    check('ellipse-alpha-gate',['--compile-project',str(out/'unsupported-ellipse-alpha.ksp'),str(out/'unpublished-ellipse-alpha')],19)
    for refused in ('unpublished-ellipse.sm','unpublished-ellipse-legacy','unpublished-ellipse-alpha'):assert not (out/refused).exists()
"""
assert marker in text;text=text.replace(marker,addition+marker,1)
text=text.replace("check('unsupported-ellipse',","check('unsupported-ellipse-style',")
text=text.replace('阶段5第七批：多边形、圆角矩形','阶段5第八批：圆/椭圆、多边形、圆角矩形')
text=text.replace('多边形仅3至8点',r'编号以A03为准：G05圆角矩形，G06圆/椭圆。\n椭圆X/Y至少2、宽高至少100、右端不超过698、下端不超过378；黑边/黑前景/白背景、宽1、不透明、线型0/1与填充0/1，拒绝补片相交或接触。独立定点细分和8位子像素积分，混色先右移再加128。\n多边形仅3至8点')
destination=root/'package_slice8.py';assert not destination.exists();destination.write_text(text,encoding='utf-8');print(destination)
