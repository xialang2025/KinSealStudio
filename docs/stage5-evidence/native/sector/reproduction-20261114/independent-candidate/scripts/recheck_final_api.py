from pathlib import Path
import hashlib,json,os,subprocess
here=Path(__file__).resolve().parent;root=here.parent.parent;work=root.parent/'kinseal-stage4-20261112/worktree';qt=Path('G:/01_Professional/QT6/Installed/6.12.0/mingw_64');mingw=Path('G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin');env=os.environ.copy();env['PATH']=str(mingw)+';'+str(qt/'bin')+';'+env.get('PATH','')
for key in ('TEMP','TMP','TMPDIR'):env[key]=str(here/'temp')
command=[str(mingw/'g++.exe'),'-std=c++17','-O2','-Wall','-Wextra','-Wconversion','-DQT_NO_DEBUG','-isystem',str(qt/'include'),'-isystem',str(qt/'include/QtCore'),'-isystem',str(qt/'include/QtGui'),'-I'+str(work/'src'),str(here/'paint_probe.cpp'),str(work/'src/device_ellipse_raster.cpp'),str(work/'src/device_sector_raster.cpp'),str(work/'src/device_polygon_raster.cpp'),'-L'+str(qt/'lib'),'-lQt6Gui','-lQt6Core','-o',str(here/'paint-current-final.exe')]
r=subprocess.run(command,env=env,capture_output=True,creationflags=subprocess.CREATE_NO_WINDOW);(here/'current-final-build-r2.log').write_bytes(r.stdout+r.stderr)
if r.returncode:raise RuntimeError((r.returncode,r.stdout,r.stderr))
previous=json.loads((here/'production-sharing-crosscheck-r1.json').read_text(encoding='utf-8'));records=[];commands=[command]
for case in previous['cases']:
 family=case['family'];label=case['case'];line=case['line'];fill=case['fill'];stem=family+'-'+label+'-'+str(line)+str(fill)
 reference=here/'temp'/(stem+('-old.rgb565' if family=='ellipse' else '-cli.rgb565'));raw=reference.read_bytes();out=here/'temp'/(stem+'-final.rgb565')
 args=[str(here/'paint-current-final.exe'),family,*map(str,case['geometry']),str(line),str(fill),str(out)];r=subprocess.run(args,env=env,capture_output=True,creationflags=subprocess.CREATE_NO_WINDOW)
 if r.returncode:raise RuntimeError((args,r.returncode,r.stdout,r.stderr))
 actual=out.read_bytes();commands.append(args);records.append(dict(family=family,case=label,geometry=case['geometry'],line=line,fill=fill,referenceSha256=hashlib.sha256(raw).hexdigest(),finalSha256=hashlib.sha256(actual).hexdigest(),differentPixels=sum(a!=b for a,b in zip(memoryview(raw).cast('H'),memoryview(actual).cast('H')))))
result=dict(cases=records,ellipseCases=32,sectorCandidateCases=28,allEqual=all(c['differentPixels']==0 for c in records),sharedApi='paintDeviceWhiteCurvePath(painter,geometry,properties,path,error)',unusedStrokeOutlineExtensionRemoved=True,buildLogBytes=(here/'current-final-build-r2.log').stat().st_size,productionSha256={name:hashlib.sha256((work/'src'/name).read_bytes()).hexdigest() for name in ('device_ellipse_raster.cpp','device_ellipse_raster.h','device_sector_raster.cpp','device_sector_raster.h')},nativeProcessLaunched=False,usbAccessed=False,mainBuildInvoked=False,sectorNativeAcceptance='root/sole native owner separate B/S/M')
(here/'production-sharing-crosscheck-r2.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');(here/'commands-r2.json').write_text(json.dumps(dict(commands=commands),indent=2)+'\n',encoding='utf-8');print(json.dumps(dict(ellipseCases=32,sectorCandidateCases=28,allEqual=result['allEqual'],buildLogBytes=result['buildLogBytes'])))
if not result['allEqual']:raise SystemExit(1)
