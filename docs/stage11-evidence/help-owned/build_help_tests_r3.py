from pathlib import Path
import os,subprocess,json,hashlib
ROOT=Path.cwd(); EVIDENCE=ROOT/'docs/stage11-evidence/help-owned'
OUTPUT=Path(r'Z:/Project/vscode_prj_temp/REA/.stage11-run-20261114/help-owned/build-r3'); OUTPUT.mkdir(parents=True,exist_ok=False)
TEMP=OUTPUT/'temp'; TEMP.mkdir()
QT=Path(r'G:/01_Professional/QT6/Installed/6.12.0/mingw_64'); CC=Path(r'G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin/g++.exe')
ENV=os.environ.copy(); ENV['PATH']=os.pathsep.join((str(CC.parent),str(QT/'bin'),ENV.get('SYSTEMROOT',r'C:\Windows')+r'\System32'))
for n in ('TEMP','TMP','TMPDIR'):ENV[n]=str(TEMP)
ENV['QT_QPA_PLATFORM']='offscreen'; ENV['KINSEAL_HELP_SOURCE_ROOT']=str(ROOT/'help')
record={'steps':[],'sourceHashes':{n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in ['src/local_help.cpp','src/local_help.h','tests/local_help_tests.cpp']},'physicalDevicesOpened':False,'visibleWindowCreated':False,'platform':'offscreen'}
def step(name,argv):
 p=subprocess.run([str(a) for a in argv],cwd=ROOT,env=ENV,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=60)
 (EVIDENCE/(name+'.log')).write_bytes(p.stdout); record['steps'].append({'name':name,'argv':[str(a) for a in argv],'exitCode':p.returncode,'log':name+'.log'})
 (EVIDENCE/'build-r3.json').write_text(json.dumps(record,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 if p.returncode: print(p.stdout.decode('utf-8',errors='replace'));raise SystemExit(p.returncode)
step('moc-r3',[QT/'bin/moc.exe',ROOT/'tests/local_help_tests.cpp','-o',OUTPUT/'local_help_tests.moc'])
flags=['-std=c++17','-Wall','-Wextra','-Wpedantic','-DQT_WIDGETS_LIB','-DQT_GUI_LIB','-DQT_CORE_LIB','-DQT_NO_CAST_FROM_ASCII','-DQT_NO_CAST_TO_ASCII','-Isrc','-I'+str(OUTPUT)]
for mod in ('','QtCore','QtGui','QtWidgets','QtTest'): flags+=['-isystem',str(QT/'include'/mod)]
step('compile-r3',[CC]+flags+[ROOT/'src/local_help.cpp',ROOT/'tests/local_help_tests.cpp','-L'+str(QT/'lib'),'-lQt6Widgets','-lQt6Gui','-lQt6Test','-lQt6Core','-o',OUTPUT/'LocalHelpTests.exe'])
step('test-r3',[OUTPUT/'LocalHelpTests.exe','-o',str(EVIDENCE/'tests-r3.txt')+',txt'])
record['executable']=str(OUTPUT/'LocalHelpTests.exe'); record['executableSha256']=hashlib.sha256((OUTPUT/'LocalHelpTests.exe').read_bytes()).hexdigest()
(EVIDENCE/'build-r3.json').write_text(json.dumps(record,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps({'executable':record['executable'],'steps':record['steps'][-1:]}))
