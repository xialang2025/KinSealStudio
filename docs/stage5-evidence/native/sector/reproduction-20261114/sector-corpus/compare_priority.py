"""Give early full-frame evidence for the bounded solid-outline sector variants."""
from pathlib import Path
import hashlib
import json
import os
import subprocess
import numpy as np

HERE=Path(__file__).resolve().parent
ROOT=HERE.parent
WORK=ROOT.parent/'kinseal-stage4-20261112/worktree'
BASE=WORK/'docs/stage5-evidence/native/sector'
TEMP=HERE/'temp'
ENV=os.environ.copy()
ENV['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;'+ENV.get('PATH','')
for key in ('TEMP','TMP','TMPDIR'):
    ENV[key]=str(TEMP)


def call(command):
    """Invoke independent curve/LZO tools without any original process or transport."""
    result=subprocess.run(list(map(str,command)),capture_output=True,env=ENV,creationflags=subprocess.CREATE_NO_WINDOW)
    if result.returncode:
        raise RuntimeError((command,result.returncode,result.stdout,result.stderr))


def main():
    """Compare every currently bound line-one sample to the independent complete candidate page."""
    index=json.loads((BASE/'valid-target-index-r1.json').read_text(encoding='utf-8'))
    records=[]
    for row in index['runs']:
        if row['lineStyle']!=1:
            continue
        assert len(row['rects'])==1
        prefix=TEMP/('priority-'+str(row['lineStyle'])+'-'+str(row['fillStyle']))
        if not Path(str(prefix)+'.json').exists():
            call([ROOT/'sector_followup/sector_curve_cli.exe',prefix,*row['rects'][0],row['lineStyle'],row['fillStyle']])
        candidate=Path(str(prefix)+'.rgb565').read_bytes()
        folder=BASE/row['name']/'compiled'/row['compilationLabel']
        native=TEMP/(row['name']+'-early.rgb565')
        call([ROOT.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe',folder/'SheetPixel.dat',native,768000])
        expected=native.read_bytes()
        actual=np.frombuffer(candidate,'<u2').reshape(480,800)
        comparison=np.frombuffer(expected,'<u2').reshape(480,800)
        ys,xs=np.where(actual!=comparison)
        records.append(dict(run=row['name'],rects=row['rects'],lineStyle=row['lineStyle'],fillStyle=row['fillStyle'],sourceSha256=row['sourceSha256'],differentPixels=int(len(xs)),candidateSha256=hashlib.sha256(candidate).hexdigest(),nativeSha256=hashlib.sha256(expected).hexdigest(),first=[dict(x=int(x),y=int(y),native=int(comparison[y,x]),candidate=int(actual[y,x])) for y,x in zip(ys[:12],xs[:12])]))
    report=dict(cases=records,allEqual=all(row['differentPixels']==0 for row in records),candidateStrokeOrder='local path -> integer scene translation -> scene createStroke',nativeProcessLaunched=False,usbAccessed=False)
    (BASE/'line-one-priority-r1.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(report,ensure_ascii=False,indent=2))


if __name__=='__main__':
    main()
