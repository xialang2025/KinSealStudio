from pathlib import Path
import gzip,hashlib,json,os,subprocess,tarfile
here=Path(__file__).resolve().parent;follow=here.parent;root=follow.parent;work=root.parent/'kinseal-stage4-20261112/worktree'
qt=Path('G:/01_Professional/QT6/Installed/6.12.0/mingw_64');mingw=Path('G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin');env=os.environ.copy();env['PATH']=str(mingw)+';'+str(qt/'bin')+';'+env.get('PATH','')
for key in ('TEMP','TMP','TMPDIR'):env[key]=str(here/'temp')
def invoke(args):
 r=subprocess.run(list(map(str,args)),env=env,capture_output=True,creationflags=subprocess.CREATE_NO_WINDOW)
 if r.returncode:raise RuntimeError((args,r.returncode,r.stdout,r.stderr))
 return r
old=here/'old-ellipse';old.mkdir(exist_ok=True)
with tarfile.open(root/'ellipse_algorithm/module_crosscheck/module-crosscheck-r1.tar.gz','r:gz') as archive:
 for name in ('device_ellipse_raster.cpp','device_ellipse_raster.h'):
  (old/name).write_bytes(archive.extractfile('production-source-snapshot/'+name).read())
commands=[]
for mode in ('old','current'):
 args=[mingw/'g++.exe','-std=c++17','-O2','-Wall','-Wextra','-Wconversion','-DQT_NO_DEBUG','-isystem',qt/'include','-isystem',qt/'include/QtCore','-isystem',qt/'include/QtGui','-I'+str(work/'src'),here/'paint_probe.cpp']
 if mode=='old':args+=['-DOLD_ELLIPSE',old/'device_ellipse_raster.cpp']
 else:args+=[work/'src/device_ellipse_raster.cpp',work/'src/device_sector_raster.cpp']
 args+=[work/'src/device_polygon_raster.cpp','-L'+str(qt/'lib'),'-lQt6Gui','-lQt6Core','-o',here/('paint-'+mode+'.exe')]
 r=invoke(args);(here/(mode+'-build-r1.log')).write_bytes(r.stdout+r.stderr);commands.append(list(map(str,args)))
records=[]
ellipses=[('default',(100,100,200,100)),('shift',(75,65,160,120)),('wide',(100,100,300,150)),('tall',(100,100,120,250)),('odd',(100,100,201,101)),('min100',(100,100,100,100)),('far',(498,278,200,100)),('near',(2,2,200,100))]
for label,geometry in ellipses:
 for line,fill in ((0,0),(0,1),(1,0),(1,1)):
  frames=[]
  for mode in ('old','current'):
   out=here/'temp'/('ellipse-'+label+'-'+str(line)+str(fill)+'-'+mode+'.rgb565');args=[here/('paint-'+mode+'.exe'),'ellipse',*geometry,line,fill,out];invoke(args);commands.append(list(map(str,args)));frames.append(out.read_bytes())
  records.append(dict(family='ellipse',case=label,geometry=geometry,line=line,fill=fill,reference='retained pre-sharing production module',oldSha256=hashlib.sha256(frames[0]).hexdigest(),currentSha256=hashlib.sha256(frames[1]).hexdigest(),differentPixels=sum(a!=b for a,b in zip(memoryview(frames[0]).cast('H'),memoryview(frames[1]).cast('H')))))
sectors=[('default',(100,100,200,200)),('shift',(75,65,160,160)),('side240',(100,100,240,240)),('odd',(100,100,201,201)),('min100',(100,100,100,100)),('far',(498,178,200,200)),('near',(2,2,200,200))]
for label,geometry in sectors:
 for line,fill in ((0,0),(0,1),(1,0),(1,1)):
  prefix=here/'temp'/('sector-'+label+'-'+str(line)+str(fill)+'-cli');args=[follow/'sector_curve_cli.exe',prefix,*geometry,line,fill];invoke(args);commands.append(list(map(str,args)));reference=Path(str(prefix)+'.rgb565').read_bytes()
  out=here/'temp'/('sector-'+label+'-'+str(line)+str(fill)+'-current.rgb565');args=[here/'paint-current.exe','sector',*geometry,line,fill,out];invoke(args);commands.append(list(map(str,args)));actual=out.read_bytes()
  records.append(dict(family='sector',case=label,geometry=geometry,line=line,fill=fill,reference='independent local8 -> integer translation Qt6 CLI candidate; not native acceptance',cliSha256=hashlib.sha256(reference).hexdigest(),currentSha256=hashlib.sha256(actual).hexdigest(),differentPixels=sum(a!=b for a,b in zip(memoryview(reference).cast('H'),memoryview(actual).cast('H')))))
result=dict(cases=records,ellipseCases=32,sectorCandidateCases=28,allEqual=all(r['differentPixels']==0 for r in records),buildLogBytes={mode:(here/(mode+'-build-r1.log')).stat().st_size for mode in ('old','current')},productionSha256={name:hashlib.sha256((work/'src'/name).read_bytes()).hexdigest() for name in ('device_ellipse_raster.cpp','device_ellipse_raster.h','device_sector_raster.cpp','device_sector_raster.h')},nativeProcessLaunched=False,usbAccessed=False,mainBuildInvoked=False,sectorNativeAcceptancePending=True)
(here/'production-sharing-crosscheck-r1.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');(here/'commands-r1.json').write_text(json.dumps(dict(commands=commands),indent=2)+'\n',encoding='utf-8');print(json.dumps(dict(ellipseCases=32,sectorCandidateCases=28,allEqual=result['allEqual'],buildLogBytes=result['buildLogBytes'],differences=[(r['family'],r['case'],r['line'],r['fill'],r['differentPixels']) for r in records if r['differentPixels']])))
if not result['allEqual']:raise SystemExit(1)
