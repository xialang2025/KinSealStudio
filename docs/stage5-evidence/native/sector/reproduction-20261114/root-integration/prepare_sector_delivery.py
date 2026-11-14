"""Generate the slice 9 packaging recipe; never build, launch or package the application."""

from pathlib import Path


ROOT = Path(__file__).resolve().parent


def replace_once(source, old, new):
    """Refuse a changed template rather than silently emitting an incomplete recipe."""
    if source.count(old) != 1:
        raise ValueError(f'Expected one template marker: {old!r}')
    return source.replace(old, new, 1)


def build_recipe(source):
    """Derive the sector samples, guards and reviewable 14-object layout from slice 8."""
    result = source.replace('fixed-point ellipse compiler', 'default-angle sector compiler')
    result = result.replace('slice8', 'slice9').replace('ellipse-windows-r2', 'sector-windows-r1')
    result = result.replace('mixed-thirteen', 'mixed-fourteen')
    result = result.replace('十三类控件混合示例', '十四类控件混合示例').replace('十三对象', '十四对象')

    sector_samples = """sector=copy.deepcopy(rectangle);sector['projectName']='扇形默认角度样例';sector_object=sector['pages'][0]['objects'][0];sector_object.update(type='sector',id='stage5-sector',name='扇形1',width=200,height=200)
sector_object['graphic']=dict(lineStyle=0,lineWidth=1,lineColor='#ff000000',alpha=255,fillStyle=1,foreground='#ff000000',background='#ffffffff');write(samples/'sector-default.ksp',sector)
sector_outline=copy.deepcopy(sector);sector_outline['projectName']='扇形实线无填充';sector_outline['pages'][0]['objects'][0]['graphic'].update(lineStyle=1,fillStyle=0);write(samples/'sector-outline.ksp',sector_outline)
for label,side,title in (('odd',201,'扇形奇数尺寸'),('threshold168',168,'扇形168像素细分回归'),('threshold170',170,'扇形170像素细分回归')):
    variant=copy.deepcopy(sector);variant['projectName']=title;variant['pages'][0]['objects'][0].update(width=side,height=side);write(samples/('sector-'+label+'.ksp'),variant)

def assert_no_touching_patches(objects):
    '''Record every clipped physical patch and reject intersection or boundary contact before sample publication.'''
    bounds=[]
    for current in objects:
        bounds.append(dict(id=current['id'],type=current['type'],left=max(0,current['x']-1),top=max(0,current['y']-1),right=min(800,current['x']+current['width']+2),bottom=min(480,current['y']+current['height']+2)))
    for index,first in enumerate(bounds):
        for second in bounds[index+1:]:
            touches=(first['left']<=second['right'] and second['left']<=first['right'] and first['top']<=second['bottom'] and second['top']<=first['bottom'])
            assert not touches,('Touching portable sample patches',first,second)
    return dict(passed=True,physicalBoundaryContactRejected=True,objects=bounds,pairsChecked=len(bounds)*(len(bounds)-1)//2)

"""
    result = replace_once(result, 'polygon=copy.deepcopy(rectangle)', sector_samples + 'polygon=copy.deepcopy(rectangle)')
    mixed_marker = "mixed['pages'][0]['objects'].append(ellipse_mixed);write(samples/'mixed-fourteen.ksp',mixed)"
    mixed_addition = """mixed['pages'][0]['objects'].append(ellipse_mixed)
rectangles=[existing for existing in mixed['pages'][0]['objects'] if existing['id']=='stage5-rectangle'];assert len(rectangles)==1;rectangles[0].update(x=130,y=300)
sector_mixed=copy.deepcopy(sector_object);sector_mixed.update(x=10,y=205,width=100,height=100);mixed['pages'][0]['objects'].append(sector_mixed)
assert len(mixed['pages'][0]['objects'])==14
mixed_patch_separation=assert_no_touching_patches(mixed['pages'][0]['objects']);write(samples/'mixed-fourteen.ksp',mixed)"""
    result = replace_once(result, mixed_marker, mixed_addition)
    result = replace_once(result, "[('ellipse-default',10)", "[('sector-default',10),('sector-outline',10),('sector-odd',10),('sector-threshold168',10),('sector-threshold170',10),('ellipse-default',10)")

    sector_guards = """    check('sector-source-gate',['--export-native-project',str(samples/'sector-default.ksp'),str(out/'unpublished-sector.sm')],20)
    check('sector-legacy-gate',['--compile-single-text',str(samples/'sector-default.ksp'),str(out/'unpublished-sector-legacy')],19)
    bad_sector=copy.deepcopy(sector);bad_sector['pages'][0]['objects'][0]['height']=199;write(out/'unsupported-sector-nonsquare.ksp',bad_sector)
    check('sector-nonsquare-gate',['--compile-project',str(out/'unsupported-sector-nonsquare.ksp'),str(out/'unpublished-sector-nonsquare')],19)
    bad_sector=copy.deepcopy(sector);bad_sector['pages'][0]['objects'][0]['graphic']['alpha']=128;write(out/'unsupported-sector-alpha.ksp',bad_sector)
    check('sector-alpha-gate',['--compile-project',str(out/'unsupported-sector-alpha.ksp'),str(out/'unpublished-sector-alpha')],19)
    for refused in ('unpublished-sector.sm','unpublished-sector-legacy','unpublished-sector-nonsquare','unpublished-sector-alpha'):assert not (out/refused).exists()
"""
    result = replace_once(result, "    check('ellipse-source-gate'", sector_guards + "    check('ellipse-source-gate'")
    result = replace_once(result, 'runs=[],usbAccessed=False', 'runs=[],mixedPatchSeparation=mixed_patch_separation,usbAccessed=False')
    result = replace_once(result, '    record.update(passed=True,onlyBundledQtDllsObserved=True,qtDlls=qt)', "    assert len(record['runs'])==136,len(record['runs'])\n    record.update(passed=True,onlyBundledQtDllsObserved=True,qtDlls=qt)")
    result = replace_once(result, 'checksPassed=len(record[\'runs\']),completeStage5=False', "checksPassed=len(record['runs']),mixedPatchSeparation=mixed_patch_separation,completeStage5=False")
    result = result.replace('阶段5第八批：圆/椭圆', '阶段5第九批：默认角度扇形、圆/椭圆')
    result = replace_once(result, 'G05圆角矩形，G06圆/椭圆。', 'G05圆角矩形，G06圆/椭圆，G07扇形。')
    sector_readme = r'扇形仅固定45度起角/90度扫角，正方形边长100..240，X/Y至少2、右端不超过698、下端不超过378；黑边/黑前景/白背景、宽1/Alpha255、线型0/1与填充0/1，拒绝补片相交或接触。路径为moveTo圆心+arcTo(45,90)的开放8元素；填充隐闭合，描边不补第9条末端径向边。JSON只有闭合家族七键，不含角度、半径、points或线条专用端点键；完整角度编辑及非方框源语义仍未恢复。\n'
    result = replace_once(result, '椭圆X/Y至少2、宽高至少100', sector_readme + '椭圆X/Y至少2、宽高至少100')
    result = replace_once(result, "shutil.copy2(root/'package_slice9.py',repro/'package_slice9.py')", "shutil.copy2(root/'package_slice9.py',repro/'package_slice9.py');shutil.copy2(root/'prepare_sector_delivery.py',repro/'prepare_sector_delivery.py')")
    return result


def main():
    """Write a new recipe only; equal reruns are idempotent and differing files are preserved."""
    destination = ROOT / 'package_slice9.py'
    candidate = build_recipe((ROOT / 'package_slice8.py').read_text(encoding='utf-8'))
    if destination.exists():
        if destination.read_text(encoding='utf-8') != candidate:
            raise FileExistsError(f'Existing recipe differs; refusing overwrite: {destination}')
    else:
        destination.write_text(candidate, encoding='utf-8')
    print(destination)


if __name__ == '__main__':
    main()
