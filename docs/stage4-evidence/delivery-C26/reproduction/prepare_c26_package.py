"""Adapt the verified portable recipe to C26 with an uncluttered second page for its three newest families."""
from pathlib import Path
root=Path(__file__).resolve().parent
s=(root/'package_c23.py').read_text(encoding='utf-8').replace('C23','C26').replace('c23','c26').replace('3989d184a64b24821be84bbbad7ea10d96ba0b9b','6a3c1eb88dceba42c3319b3ae448e660086dddf4').replace('stage4-evidence/trend/windows-final','stage4-evidence/data-group/windows-final')
a=s.index("sample=read(work/'samples/stage4/trend.ksp')");b=s.index('shutil.copy2(work/\'third_party/lzokay/LICENSE\'',a)
block='''sample=read(main/'dist/KinSealStudioLite-Stage4-C23-0.10.1/samples/stage4/stage4-controls-C23.ksp')
sample['projectName']='阶段4 C01～C26 编辑切片'
newest=read(work/'samples/stage4/data-group.ksp')
page=json.loads(json.dumps(newest['pages'][0],ensure_ascii=False))
page['id']='page-curves-C26'; page['name']='新增曲线与采集'
page['objects']=[o for o in page['objects'] if o['type'] in ('historytrend','xycurve','datagroup')]
assert len(page['objects'])==3
for obj in page['objects']:
 if obj['type']=='historytrend': obj.update(x=20,y=20,width=360,height=210,layer=1)
 elif obj['type']=='xycurve': obj.update(x=420,y=20,width=360,height=210,layer=2)
 else: obj.update(x=20,y=250,width=360,height=210,layer=3)
sample['pages'].append(page);sample['pageOrder'].append(page['id']);sample['activePageId']=page['id'];sample['configurationNodes']=newest['configurationNodes']
assert sum(len(p['objects']) for p in sample['pages'])==26
showcase=samples/'stage4-controls-C26.ksp';write(showcase,sample)
'''
s=s[:a]+block+s[b:]
a=s.index(' for name,args,expected in [');b=s.index('  run=desktop.run(',a)
s=s[:a]+''' checks=[('self-test',['--self-test'],0),('payload',['--verify-independent-payload'],0),('showcase-model',['--validate-project',str(showcase)],0),('capture-main',['--capture-preview',str(smoke/'main-window.png'),str(showcase)],0)]
 for family,filename in [('history-trend','history-only.ksp'),('xy-curve','xy-only.ksp'),('data-group','group-only.ksp')]:
  input_file=work/'docs/stage4-evidence'/family/'inputs'/filename
  checks += [(family+'-source-gate',['--export-native-project',str(input_file),str(smoke/'unpublished.sm')],20),(family+'-device-gate',['--compile-project',str(input_file),str(smoke/'unpublished-package')],19)]
 for name,args,expected in checks:
'''+s[b:]
s=s.replace('阶段4 C26增量：23类编辑切片（包括图表三形态、趋势图通道与坐标轴）。','阶段4 C26增量：26类编辑切片，新增历史采集/历史趋势、X-Y曲线、数据群组。').replace('controls=121','controls=142').replace('portableChecksPassed=6','portableChecksPassed=10').replace('checksPassed=6','checksPassed=10')
s=s.replace('控件121/0/0','控件142/0/0').replace('23类工程验证','26类两页工程验证').replace('趋势图源拒绝和设备拒绝共6项通过','历史趋势、X-Y、数据群组源与设备拒绝共10项通过').replace('18类新增控件','21类新增控件').replace('图表三形态和趋势图原厂基线均已实物下载','图表三形态、趋势图、历史趋势、X-Y及数据群组原厂基线均已实物下载')
s=s.replace("(package/'使用说明.txt').write_text(","(package/'使用说明.txt').write_text(")
s=s.replace('后台验证]','后台验证]')
s += '''\nprogress_path=work/'docs/stage4-evidence/progress.json';progress=read(progress_path)
progress['latestDelivery']=dict(evidence='docs/stage4-evidence/delivery-C26/package-manifest.json',package=str(package),sourceTree='isolated_verified_committed_checkout',sourceCommit=commit,processed=26,total=41,completeStage4=False,mainUnacceptedStage3Failures=2)
write(progress_path,progress)
shutil.copy2(root/'prepare_c26_package.py',out/'reproduction/prepare_c26_package.py')
'''
(root/'package_c26.py').write_text(s,encoding='utf-8',newline='\n')
print('Prepared the C26 portable recipe with 26 controls on two pages and 10 actual background checks.')
