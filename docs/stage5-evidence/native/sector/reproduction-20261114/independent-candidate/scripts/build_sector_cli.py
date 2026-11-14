from pathlib import Path
import hashlib,json,os,subprocess
here=Path(__file__).resolve().parent;root=here.parent;work=root.parent/'kinseal-stage4-20261112/worktree'
qt=Path('G:/01_Professional/QT6/Installed/6.12.0/mingw_64');mingw=Path('G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin')
env=os.environ.copy();env['PATH']=str(mingw)+';'+str(qt/'bin')+';'+env.get('PATH','')
for key in ('TEMP','TMP','TMPDIR'):env[key]=str(here/'temp')
command=[str(mingw/'g++.exe'),'-std=c++17','-O2','-Wall','-Wextra','-Wconversion','-DQT_NO_DEBUG','-isystem',str(qt/'include'),'-isystem',str(qt/'include/QtCore'),'-isystem',str(qt/'include/QtGui'),str(here/'sector_curve_cli.cpp'),str(root/'ellipse_algorithm/ellipse_curve_candidate.cpp'),str(work/'src/device_polygon_raster.cpp'),'-L'+str(qt/'lib'),'-lQt6Gui','-lQt6Core','-o',str(here/'sector_curve_cli.exe')]
r=subprocess.run(command,env=env,capture_output=True,creationflags=subprocess.CREATE_NO_WINDOW);(here/'sector-cli-build-r2.log').write_bytes(r.stdout+r.stderr)
(here/'sector-cli-build-r2.json').write_text(json.dumps(dict(command=command,exitCode=r.returncode,localPathThenIntegerTranslation=True,originalProgramOrDllUsed=False,usbAccessed=False),indent=2)+'\n',encoding='utf-8')
if r.returncode:raise RuntimeError((r.returncode,(r.stdout+r.stderr).decode(errors='replace')))
print('build exit0; warnings bytes',len(r.stdout+r.stderr))
prefix=here/'cli-smoke-local-r2/default';command=[str(here/'sector_curve_cli.exe'),str(prefix),'100','100','200','200','0','1']
r=subprocess.run(command,env=env,capture_output=True,creationflags=subprocess.CREATE_NO_WINDOW)
if r.returncode:raise RuntimeError((r.returncode,r.stderr))
meta=json.loads(r.stdout);assert meta['pathElementCount']==8 and not meta['pathExplicitlyClosed'];assert meta['frameSha256']=='73653ee69d44d72a4ab9c6547c817423bdfae1e0c5da44b12da347caa5fe0251'
print(json.dumps(dict(defaultFrameSha256=meta['frameSha256'],elementCount=meta['pathElementCount'],transform=meta['transform'])))
