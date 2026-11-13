"""Build and run three real Windows targets with only Z-owned temporary storage."""
from pathlib import Path
import argparse
import os
import subprocess
import sys
root = Path(__file__).resolve().parent
p = argparse.ArgumentParser(); p.add_argument('family'); p.add_argument('--run', default='windows-r1'); a=p.parse_args()
assert a.family in ['polygon','freeline','curve','sector','table']
temporary = root/'task-temp'/(a.family+'-validation'); temporary.mkdir(parents=True,exist_ok=True)
environment=os.environ.copy()
environment.update(TEMP=str(temporary), TMP=str(temporary), TMPDIR=str(temporary), PYTHONDONTWRITEBYTECODE='1', KINSEAL_ARM_SDK='G:/01_Professional/触摸屏/金玺/KinSealStudio_Setup_V5.1/Installed/script')
environment['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;'+environment.get('PATH','')
with (root/(a.family+'-build-'+a.run.replace('windows-','')+'.log')).open('wb') as stream:
    subprocess.run(['G:/01_Professional/QT6/Installed/Tools/CMake_64/bin/cmake.exe','--build',str(root/'worktree/build'),'--target','KinSealStudioLite','KinSealStage4ControlTests','KinSealNativeSourceTests','KinSealDeviceMacroTests','-j3'],env=environment,stdout=stream,stderr=subprocess.STDOUT,check=True,creationflags=subprocess.CREATE_NO_WINDOW)
subprocess.run([sys.executable,'-B',str(root/'worktree/tools/run_stage4_private_tests.py'),'--build-dir',str(root/'worktree/build'),'--output',str(root/'worktree/docs/stage4-evidence'/a.family/a.run),'KinSealStage4ControlTests','KinSealNativeSourceTests','KinSealDeviceMacroTests'],env=environment,check=True,creationflags=subprocess.CREATE_NO_WINDOW)
