"""Keep raw file and structural differences separate from full RGB565 equality."""
from pathlib import Path
import hashlib
import json
import os
import subprocess
import argparse

root = Path(__file__).resolve().parent
work = root.parent/'kinseal-stage4-20261112/worktree'
base = work/'docs/stage5-evidence/native/table'
decoder = root.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe'
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
environment = os.environ.copy()
environment.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))
parser=argparse.ArgumentParser();parser.add_argument('--flat',action='store_true');args=parser.parse_args()
cases = []
for run in sorted(base.iterdir()):
    probe = run/'native-probe.json'
    if not probe.is_file():
        continue
    data = json.loads(probe.read_text(encoding='utf-8'))
    assert data['ownedProcessTerminated'] and not data['usbOrPlc']
    boundary = next((v for v in ('one-cell','three-by-two','thirty-boundary') if run.name.startswith('table-'+v+'-')),None)
    if boundary is not None and not args.flat:
        continue
    for compiled in data.get('nativeCompilations', []):
        folder = run/'compiled'/compiled['label']
        assert sha(folder/'input-native.sm') == compiled['sourceSha256']
        raw = root/'temp'/(run.name+'.rgb565')
        subprocess.run([str(decoder),str(folder/'SheetPixel.dat'),str(raw),'768000'],check=True,capture_output=True,env=environment,timeout=30,creationflags=subprocess.CREATE_NO_WINDOW)
        mask = int(run.name.split('-')[2]) if run.name.startswith('table-flags-') else 4 if 'line-zero' in run.name else 7
        actual = root/'temp'/f'table-{"flat" if args.flat else "actual"}-{mask}.rgb565'
        if boundary is not None:
            geometry = {'one-cell':[100,100,40,40,1,1],'three-by-two':[75,65,160,120,3,2],'thirty-boundary':[0,0,600,300,30,30]}[boundary]
            actual = root/'temp'/('table-boundary-'+boundary+'.rgb565')
            subprocess.run([str(root/'table_boundary_raster_probe.exe'),str(actual),*map(str,geometry)],check=True,capture_output=True,env=environment,timeout=30,creationflags=subprocess.CREATE_NO_WINDOW)
        assert raw.stat().st_size == actual.stat().st_size == 768000
        pixels, expected = actual.read_bytes(), raw.read_bytes()
        difference = sum(pixels[i:i+2] != expected[i:i+2] for i in range(0,len(pixels),2))
        lengths = (folder/'PicLenPara.dat').read_bytes()
        assert len(lengths)==2400 and not (folder/'ControlPara.dat').exists() and not (folder/'ControlPixel.dat').exists()
        cases.append(dict(run=run.name,mask=mask,geometryAndCounts=geometry if boundary is not None else [100,100,200,100,4,4],sourceSha256=compiled['sourceSha256'],folder=str(folder.relative_to(work)).replace('\\','/'),
                          B=[dict(name=p.name,bytes=p.stat().st_size,sha256=sha(p)) for p in sorted(folder.iterdir()) if p.is_file()],
                          S=dict(controlParaPresent=False,controlPixelPresent=False,picLenBytes=len(lengths),backgroundCompressedLength=int.from_bytes(lengths[:4],'little'),dynamicImageCount=int.from_bytes(lengths[0x640:0x644],'little')),
                          M=dict(rgb565Bytes=len(pixels),nativeSha256=sha(raw),productRendererSha256=sha(actual),differentPixels=difference),E='not_executed',usbAccessed=False))
        raw.unlink()
repeated = [c for c in cases if c['sourceSha256']=='f2e1f7e14a72773c0a337df74b36bf23033ebccdea6644a7d210669d2cd9d729']
differences = []
for name in sorted({x['name'] for c in repeated for x in c['B']}):
    files = [(work/c['folder']/name).read_bytes() for c in repeated]
    ranges = []
    start = None
    for index in range(max(map(len,files))):
        changed = len({f[index] if index<len(f) else None for f in files}) != 1
        if changed and start is None:
            start=index
        if start is not None and (not changed or index==max(map(len,files))-1):
            ranges.append([start,index+1 if changed else index]);start=None
    differences.append(dict(name=name,lengths=list(map(len,files)),identical=not ranges,changedRangesExclusiveEnd=ranges,classification='unknown_no_ignore_mask' if ranges else 'stable_in_observed_fresh_processes'))
record = dict(target='AMT070W124-L800X480R',firmwareVersionConfirmed=False,cases=cases,repeatedInputFreshProcessCount=len(repeated),
              allRasterComparisonsEqual=all(c['M']['differentPixels']==0 for c in cases),B_repeatDifferences=differences,ignoredByteRanges=[],usbAccessed=False,E=False)
(base/('B-S-M-flat-candidate.json' if args.flat else 'B-S-M-comparison.json')).write_text(json.dumps(record,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps(dict(cases=len(cases),sameSourceCompiles=len(repeated),allEqual=record['allRasterComparisonsEqual'],differentFiles=[d['name'] for d in differences if not d['identical']]),ensure_ascii=False))
assert record['allRasterComparisonsEqual']
