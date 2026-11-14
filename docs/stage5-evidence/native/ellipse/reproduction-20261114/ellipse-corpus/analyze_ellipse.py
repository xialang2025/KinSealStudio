"""Bind 27 native ellipse compilations to independent C++ subdivision and full-frame cell-area output."""
from pathlib import Path
import gzip
import hashlib
import json
import os
import subprocess
import sys
import numpy as np

HERE=Path(__file__).resolve().parent
ROOT=HERE.parent
WORK=ROOT.parent/'kinseal-stage4-20261112/worktree'
BASE=WORK/'docs/stage5-evidence/native/ellipse'
ALGORITHM=ROOT/'ellipse_algorithm'
TEMP=HERE/'temp'
sys.path.insert(0,str(ALGORITHM))
from area_candidate_256 import coverage
from ellipse_reference import compose

ENV=os.environ.copy()
ENV['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;'+ENV.get('PATH','')
for key in ('TEMP','TMP','TMPDIR'):
    ENV[key]=str(TEMP)


def digest(raw):
    """Identify complete candidate/native bytes without masks or ignored ranges."""
    return hashlib.sha256(raw).hexdigest()


def read_json(path):
    """Read one retained source-bound evidence record."""
    return json.loads(path.read_text(encoding='utf-8-sig'))


def call(command):
    """Run only offline path or LZO diagnostics with Z-owned temporary storage."""
    result=subprocess.run(list(map(str,command)),capture_output=True,env=ENV,creationflags=subprocess.CREATE_NO_WINDOW)
    if result.returncode:
        raise RuntimeError((command,result.returncode,result.stdout,result.stderr))


def blend(rgb,alpha,source):
    """Preserve Qt4 byte multiplication's correction before its rounding constant."""
    alpha=alpha.astype(np.int64)[...,None]
    destination=rgb*(255-alpha)
    foreground=source*alpha
    return ((foreground+(foreground>>8)+128)>>8)+((destination+(destination>>8)+128)>>8)


def main():
    """Generate exact B/S/M and keep any failed comparisons visible in the result."""
    TEMP.mkdir(exist_ok=True)
    destination=BASE/'B-S-M-ellipse-cell-area-r2.json'
    if destination.exists():
        raise RuntimeError('Refuse to replace a completed comparison: '+str(destination))
    manifest=read_json(BASE/'valid-target-index-r1.json')
    assert manifest['complete'] and manifest['passed'] and len(manifest['runs'])==27
    executable=ALGORITHM/'ellipse_curve_probe.exe'
    executable_sha=digest(executable.read_bytes())
    candidate_sources={name:digest((ALGORITHM/name).read_bytes()) for name in ('ellipse_curve_probe.cpp','ellipse_curve_candidate.cpp','ellipse_curve_candidate.h','area_candidate_256.py','ellipse_reference.py')}
    masks={}
    generated=[]
    geometry_evidence=[]
    results=[]
    failures=BASE/'independent-failures-r2'
    for row in manifest['runs']:
        assert row['passed'] and row['targetGeometryBound']
        folder=BASE/row['name']
        probe=read_json(folder/'native-probe.json')
        assert probe['ownedProcessTerminated'] and not probe['usbOrPlc'] and not probe['systemInput']
        traces=[message['payload'] for message in probe['instrumentationMessages'] if message.get('payload',{}).get('tag')=='stage5-native-paint']
        for rect in row['rects']:
            assert any('drawEllipse' in trace['symbol'] and trace.get('rect')==rect and trace['pen']['style']==row['lineStyle'] and trace['transform']==[1,0,0,1,0,0,0,0,1] for trace in traces),(row['name'],'unbound native rectangle')
        assert len(probe['nativeCompilations'])==1
        compilation=probe['nativeCompilations'][0]
        target=folder/'compiled'/compilation['label']
        assert digest((target/'input-native.sm').read_bytes())==compilation['sourceSha256']==row['sourceSha256']
        rgb=np.zeros((480,800,3),dtype=np.int64)
        rgb[1:,1:]=[197,222,255]
        coverage_checks=[]
        for object_index,rect in enumerate(row['rects']):
            key=tuple(rect)
            if key not in masks:
                label='geometry-'+'-'.join(map(str,rect))
                prefix=TEMP/label
                call([executable,prefix,*rect])
                detail_path=Path(str(prefix)+'-details.json')
                details=read_json(detail_path)
                generated.append(detail_path)
                masks[key]={}
                geometry=dict(rect=rect,details=details,layers=[])
                for layer in ('fill','stroke'):
                    original=Path(str(prefix)+'-'+layer+'.pathstream')
                    flat=Path(str(prefix)+'-'+layer+'-flat.pathstream')
                    generated.extend([original,flat])
                    own=coverage(flat.read_bytes())
                    oracle_path=TEMP/(label+'-'+layer+'.mask')
                    call([ROOT/'legacy_coverage_probe.exe',oracle_path,'ellipse','fill',original])
                    oracle_raw=oracle_path.read_bytes()
                    oracle=np.frombuffer(oracle_raw,np.uint8).reshape(480,800)
                    differences=int(np.count_nonzero(own!=oracle))
                    record=dict(layer=layer,independentPathSha256=digest(original.read_bytes()),independentFlatPathSha256=digest(flat.read_bytes()),candidateCoverageSha256=digest(own.tobytes()),diagnosticOracleCoverageSha256=digest(oracle_raw),diagnosticOracleDifferentPixels=differences)
                    if differences:
                        failures.mkdir(exist_ok=True)
                        for suffix,raw in (('candidate',own.tobytes()),('oracle',oracle_raw)):
                            (failures/(label+'-'+layer+'-'+suffix+'.mask.gz')).write_bytes(gzip.compress(raw,mtime=0))
                    masks[key][layer]=(own,record)
                    geometry['layers'].append(record)
                    oracle_path.unlink()
                    Path(str(oracle_path)+'.pathstream').unlink()
                geometry_evidence.append(geometry)
            for layer,enabled,source in (('fill',row['fillStyle'],255),('stroke',row['lineStyle'],0)):
                mask,record=masks[key][layer]
                coverage_checks.append(dict(object=object_index,enabled=bool(enabled),**record))
                if enabled:
                    rgb=blend(rgb,mask,source)
        candidate=(((rgb[...,0]>>3)<<11)|((rgb[...,1]>>2)<<5)|(rgb[...,2]>>3)).astype('<u2').tobytes()
        reference_difference=None
        # Retain the previous candidate's distinct rounding as an explicit failed-reference diagnostic.
        if len(row['rects'])==1:
            pair=masks[tuple(row['rects'][0])]
            empty=np.zeros((480,800),dtype=np.uint8)
            old=compose(pair['fill'][0] if row['fillStyle'] else empty,pair['stroke'][0] if row['lineStyle'] else empty)
            reference_difference=int(np.count_nonzero(np.frombuffer(candidate,'<u2').reshape(480,800)!=old))
        native_path=TEMP/(row['name']+'.rgb565')
        call([ROOT.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe',target/'SheetPixel.dat',native_path,768000])
        native=native_path.read_bytes()
        assert len(native)==len(candidate)==768000
        expected=np.frombuffer(native,'<u2').reshape(480,800)
        actual=np.frombuffer(candidate,'<u2').reshape(480,800)
        ys,xs=np.where(expected!=actual)
        difference=int(len(xs))
        if difference:
            failures.mkdir(exist_ok=True)
            for suffix,raw in (('candidate',candidate),('native',native)):
                (failures/(row['name']+'-'+suffix+'.rgb565.gz')).write_bytes(gzip.compress(raw,mtime=0))
        lengths=(target/'PicLenPara.dat').read_bytes()
        structure=dict(controlParaPresent=(target/'ControlPara.dat').exists(),controlPixelPresent=(target/'ControlPixel.dat').exists(),picLenBytes=len(lengths),dynamicImageBytes=int.from_bytes(lengths[800:804],'little'),dynamicImageCount=int.from_bytes(lengths[1600:1604],'little'),sheetControlCount=(target/'SheetPara.dat').read_bytes()[2])
        structure['passed']=not structure['controlParaPresent'] and not structure['controlPixelPresent'] and len(lengths)==2400 and structure['dynamicImageBytes']==0 and structure['dynamicImageCount']==0 and structure['sheetControlCount']==0
        results.append(dict(run=row['name'],rects=row['rects'],lineStyle=row['lineStyle'],fillStyle=row['fillStyle'],sourceSha256=compilation['sourceSha256'],inputSourceSha256=row['inputSourceSha256'],folder=str(target.relative_to(WORK)).replace('\\','/'),B=[dict(name=path.name,bytes=path.stat().st_size,sha256=digest(path.read_bytes())) for path in sorted(target.iterdir()) if path.is_file()],S=structure,M=dict(rgb565Bytes=768000,nativeSha256=digest(native),candidateSha256=digest(candidate),differentPixels=difference,previousReferenceDifferentPixels=reference_difference,coverageChecks=coverage_checks,firstDifferences=[dict(x=int(x),y=int(y),native=int(expected[y,x]),candidate=int(actual[y,x])) for y,x in zip(ys[:12],xs[:12])]),usbAccessed=False,E=False))
        native_path.unlink()
        print(row['name'],difference,flush=True)
    repeated=[case for case in results if 'default-repeat' in case['run']]
    assert len(repeated)==3 and len({case['sourceSha256'] for case in repeated})==1
    repeat_differences=[]
    for name in sorted({file['name'] for case in repeated for file in case['B']}):
        buffers=[(WORK/case['folder']/name).read_bytes() for case in repeated]
        ranges=[]
        start=None
        for index in range(max(map(len,buffers))):
            changed=len({raw[index] if index<len(raw) else None for raw in buffers})!=1
            if changed and start is None:
                start=index
            if start is not None and (not changed or index==max(map(len,buffers))-1):
                ranges.append([start,index+1 if changed else index])
                start=None
        repeat_differences.append(dict(name=name,lengths=list(map(len,buffers)),changedRangesExclusiveEnd=ranges,classification='unknown_no_ignore_mask' if ranges else 'stable_in_three_observed_fresh_processes'))
    assert digest(executable.read_bytes())==executable_sha
    archive=BASE/'independent-paths-r2'
    archive.mkdir(exist_ok=False)
    cleanup=[]
    for path in generated:
        raw=path.read_bytes()
        compressed=gzip.compress(raw,mtime=0)
        saved=archive/(path.name+'.gz')
        saved.write_bytes(compressed)
        assert gzip.decompress(saved.read_bytes())==raw
        cleanup.append(dict(path=str(path),bytes=len(raw),sha256=digest(raw),archive=str(saved.relative_to(BASE)),archiveBytes=len(compressed),archiveSha256=digest(compressed)))
    result=dict(cases=results,canonicalCases=15,independentScenes=13,formalCompilations=27,B_repeatDifferences=repeat_differences,ignoredByteRanges=[],allRasterComparisonsEqual=all(case['M']['differentPixels']==0 for case in results),allStructuresValid=all(case['S']['passed'] for case in results),allDiagnosticCoverageEqual=all(layer['diagnosticOracleDifferentPixels']==0 for geometry in geometry_evidence for layer in geometry['layers']),independentGeometry=geometry_evidence,candidateExecutableSha256=executable_sha,candidateSourceSha256=candidate_sources,compositionRounding='each_term=(v+(v>>8)+128)>>8; source and destination corrected before rounding',previousFailureReport='B-S-M-ellipse-cell-area-r1.json',productionLibraryLinked=False,externalQt4OracleDiagnosticOnly=True,nativeProcessLaunched=False,usbAccessed=False,E=False)
    destination.write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    for path in generated:
        assert path.resolve().is_relative_to(TEMP.resolve())
        path.unlink()
    (BASE/'independent-temp-cleanup-r2.json').write_text(json.dumps(dict(files=cleanup,removedFiles=len(cleanup),removedLogicalBytes=sum(item['bytes'] for item in cleanup),gzipRoundtripVerified=True,scope=str(TEMP)),ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(dict(cases=len(results),allRasterComparisonsEqual=result['allRasterComparisonsEqual'],allStructuresValid=result['allStructuresValid'],allDiagnosticCoverageEqual=result['allDiagnosticCoverageEqual'],nonzeroCases=[case['run'] for case in results if case['M']['differentPixels']],temporaryFilesRemoved=len(cleanup)),ensure_ascii=False))
    if not result['allRasterComparisonsEqual'] or not result['allStructuresValid'] or not result['allDiagnosticCoverageEqual']:
        raise SystemExit(1)


if __name__=='__main__':
    main()
