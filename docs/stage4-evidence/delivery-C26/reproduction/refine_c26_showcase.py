"""Fit the three newest showcase controls inside the default viewport and recheck only the changed sample."""
from pathlib import Path
import hashlib
import importlib.util
import json
import os
import shutil
root=Path(__file__).resolve().parent;work=root/'worktree';package=root.parents[1]/'KinSealStudio/dist/KinSealStudioLite-Stage4-C26-0.10.1';out=work/'docs/stage4-evidence/delivery-C26'
def read(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def write(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
sample=package/'samples/stage4/stage4-controls-C26.ksp';data=read(sample);shutil.copy2(sample,out/'portable-r1/showcase-before-layout.ksp')
for page in data['pages']:
 if page['id']!='page-curves-C26':continue
 for obj in page['objects']:
  obj['width']=320
  if obj['type']=='xycurve':obj['x']=370
write(sample,data)
spec=importlib.util.spec_from_file_location('c26_showcase',work/'docs/stage1-evidence/gui-portable/probe.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
smoke=out/'portable-r2';smoke.mkdir(exist_ok=False);temporary=smoke/'temp';temporary.mkdir();desktop=module.PrivateDesktop(smoke);environment=module.clean_environment(package);environment.update(TEMP=str(temporary),TMP=str(temporary),TMPDIR=str(temporary));result=dict(usbAccessed=False,desktopSwitched=False,systemInput=False,runs=[],passed=False)
try:
 for name,argv in [('showcase-model',['--validate-project',str(sample)]),('capture-main',['--capture-preview',str(smoke/'main-window.png'),str(sample)])]:
  run=desktop.run(name,package/'KinSealStudioLite.exe',argv,environment,timeout=60);result['runs'].append(run);assert run['passed']
 result.update(passed=True,showcaseSha256=sha(sample),previewSha256=sha(smoke/'main-window.png'))
finally:
 desktop.close();result['privateDesktopClosed']=True;result['temporaryFilesRemaining']=[str(p.relative_to(temporary)) for p in temporary.rglob('*') if p.is_file()];write(smoke/'run.json',result)
shutil.copy2(out/'package-manifest.json',out/'package-manifest-r1.json');manifest=read(out/'package-manifest.json');manifest['files']=[dict(path=str(p.relative_to(package)),bytes=p.stat().st_size,sha256=sha(p)) for p in sorted(package.rglob('*')) if p.is_file()];manifest['showcaseFollowupChecksPassed']=2;write(out/'package-manifest.json',manifest)
shutil.copy2(root/'refine_c26_showcase.py',out/'reproduction/refine_c26_showcase.py')
with (work/'docs/阶段4_C26增量包交付.md').open('a',encoding='utf-8',newline='\n') as stream:stream.write('\n新增曲线页的3个控件宽度收为320像素，X-Y曲线左移到370，使默认视口可完整查看；仅重验变更样例模型和主窗口截图2项，见[追加验证](stage4-evidence/delivery-C26/portable-r2/run.json)。初始10项、旧样例和包清单均保留。\n')
print(json.dumps(dict(layoutChecksPassed=2,package=str(package))))
