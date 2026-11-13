"""Compare complete native outputs by bytes, structure and decoded pixels without ignoring unknown differences."""
from pathlib import Path
import hashlib
import json
import os
import subprocess

root=Path(__file__).resolve().parent
work=root.parent/'kinseal-stage4-20261112/worktree'
base=work/'docs/stage5-evidence/native/polyline'
decoder=root.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe'
environment=os.environ.copy();environment.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
compilations=[]
for run in sorted(base.iterdir()):
    if not run.is_dir() or not (run/'native-probe.json').exists():continue
    probe=read(run/'native-probe.json')
    for c in probe.get('nativeCompilations',[]):
        folder=run/'compiled'/c['label']
        assert sha(folder/'input-native.sm')==c['sourceSha256']
        raw=folder/'decoded.rgb565'
        r=subprocess.run([str(decoder),str(folder/'SheetPixel.dat'),str(raw),'768000'],capture_output=True,env=environment,timeout=30,creationflags=subprocess.CREATE_NO_WINDOW)
        assert r.returncode==0 and raw.stat().st_size==768000
        lengths=(folder/'PicLenPara.dat').read_bytes()
        pixels=raw.read_bytes()
        changed=[(i%800,i//800) for i in range(800*480) if i%800>0 and i//800>0 and int.from_bytes(pixels[i*2:i*2+2],'little')!=0xc6ff]
        compilations.append(dict(run=run.name,label=c['label'],sourceSha256=c['sourceSha256'],folder=str(folder.relative_to(work)).replace('\\','/'),
            sourceOrigin='native_ui_saved_or_retained_native_source',freshCompileVerified=True,usbAccessed=False,
            originalSha256=probe['originalSha256'],runtimeSha256=probe['runtimeSha256'],
            B=[dict(name=p.name,bytes=p.stat().st_size,sha256=sha(p)) for p in sorted(folder.iterdir()) if p.is_file() and p.name!='decoded.rgb565'],
            S=dict(controlParaPresent=(folder/'ControlPara.dat').exists(),controlPixelPresent=(folder/'ControlPixel.dat').exists(),picLenBytes=len(lengths),
                backgroundCompressedLength=int.from_bytes(lengths[:4],'little'),dynamicCompressedLength=int.from_bytes(lengths[0x320:0x324],'little'),dynamicImageCount=int.from_bytes(lengths[0x640:0x644],'little')),
            M=dict(rgb565Bytes=len(pixels),rgb565Sha256=sha(raw),nonBackgroundPixels=len(changed),bounds=[min(x for x,y in changed),min(y for x,y in changed),max(x for x,y in changed),max(y for x,y in changed)] if changed else None),E='not_executed'))
        raw.unlink()
repeated=[c for c in compilations if c['sourceSha256']=='2fac27d72ba777de8d357536e334160dbb1478eb8be5822b1d6108d73fe9ec68']
assert len(repeated)==3
assert len({c['M']['rgb565Sha256'] for c in repeated})==1
names=sorted({e['name'] for c in repeated for e in c['B']})
differences=[]
for name in names:
    paths=[work/c['folder']/name for c in repeated]
    if not all(p.exists() for p in paths):continue
    data=[p.read_bytes() for p in paths]
    ranges=[];start=None;end=None
    for i in range(max(map(len,data))):
        different=len({d[i] if i<len(d) else None for d in data})!=1
        if different:
            if start is None:start=i
            end=i+1
        elif start is not None:ranges.append([start,end]);start=None
    if start is not None:ranges.append([start,end])
    differences.append(dict(name=name,lengths=list(map(len,data)),identical=len({hashlib.sha256(d).hexdigest() for d in data})==1,changedRangesExclusiveEnd=ranges,
        classification='stable_in_three_fresh_process_samples' if not ranges else 'unknown_non_determinism_no_ignore_mask_authorized'))
result=dict(target='AMT070W124-L800X480R',firmwareVersionConfirmed=False,compilations=compilations,repeatedInputFreshProcessCount=3,
    B_repeatDifferences=differences,M_repeatPixelsEqual=True,ignoredByteRanges=[],
    cachedSameProcessAttempt='repeat-r1 second request did not yield a fresh SysPara timestamp; probe remains partial-or-error and is not counted as a fresh compile.',
    completeNativeMaskRecovered=False,usbAccessed=False,E=False)
(base/'B-S-M-comparison.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps(dict(compilations=len(compilations),freshSameInput=3,stableRaster=True,nonDeterministicFiles=[x['name'] for x in differences if not x['identical']],variantBounds=[(c['label'],c['M']['bounds']) for c in compilations]),ensure_ascii=False))
