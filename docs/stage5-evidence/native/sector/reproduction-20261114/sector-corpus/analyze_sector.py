"""Compare every valid fixed-angle sector source with independent local-path construction and full-frame area integration."""
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
BASE=WORK/'docs/stage5-evidence/native/sector'
TEMP=HERE/'temp'
CLI=ROOT/'sector_followup/sector_curve_cli.exe'
sys.path.insert(0,str(ROOT/'sector_followup'))
from area_candidate_256 import coverage

ENV=os.environ.copy()
ENV['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;'+ENV.get('PATH','')
for key in ('TEMP','TMP','TMPDIR'):
    ENV[key]=str(TEMP)


def sha(raw):
    """Identify complete bytes without ignored regions."""
    return hashlib.sha256(raw).hexdigest()


def read(path):
    """Resolve full raw evidence before or after verified gzip compaction."""
    result=json.loads(path.read_text(encoding='utf-8-sig'))
    if 'rawEvidence' in result:
        reference=result['rawEvidence']
        archive=path.parent/reference['file']
        assert archive.parent.resolve()==path.parent.resolve()
        compressed=archive.read_bytes()
        assert sha(compressed)==reference['archiveSha256']
        raw=gzip.decompress(compressed)
        assert len(raw)==reference['uncompressedBytes'] and sha(raw)==reference['uncompressedSha256']
        return json.loads(raw)
    return result


def call(command):
    """Run only independent offline CLI or decompression tools with owned Z temporary files."""
    result=subprocess.run(list(map(str,command)),capture_output=True,env=ENV,creationflags=subprocess.CREATE_NO_WINDOW)
    if result.returncode:
        raise RuntimeError((command,result.returncode,result.stdout,result.stderr))


def blend(rgb,alpha,source):
    """Keep Qt4 correction before rounding in each separate source and destination term."""
    alpha=alpha.astype(np.int64)[...,None]
    source_term=source*alpha
    destination=rgb*(255-alpha)
    return ((source_term+(source_term>>8)+128)>>8)+((destination+(destination>>8)+128)>>8)


def frame(rgb):
    """Truncate RGB32 channels to one little-endian RGB565 frame after all composition."""
    return (((rgb[...,0]>>3)<<11)|((rgb[...,1]>>2)<<5)|(rgb[...,2]>>3)).astype('<u2').tobytes()


def main():
    """Write formal B/S/M and retain any failed complete frames without masking them."""
    destination=BASE/'B-S-M-sector-cell-area-r1.json'
    if destination.exists():
        raise RuntimeError('Refuse to replace a retained comparison: '+str(destination))
    index=read(BASE/'valid-target-index-r1.json')
    assert index['complete'] and index['passed'] and len(index['runs'])==27
    executable_sha=sha(CLI.read_bytes())
    TEMP.mkdir(exist_ok=True)
    cache={}
    geometries=[]
    generated=[]
    results=[]
    failed=BASE/'independent-failures-r1'
    for row in index['runs']:
        assert row['passed'] and row['targetGeometryBound'] and row['ownedProcessTerminated'] and not row['usbOrPlc']
        folder=BASE/row['name']
        probe=read(folder/'native-probe.json')
        assert probe['ownedProcessTerminated'] and not probe['usbOrPlc'] and not probe['systemInput']
        assert len(probe['nativeCompilations'])==1
        compilation=probe['nativeCompilations'][0]
        target=folder/'compiled'/compilation['label']
        assert sha((target/'input-native.sm').read_bytes())==compilation['sourceSha256']==row['sourceSha256']
        rgb=np.zeros((480,800,3),dtype=np.int64)
        rgb[1:,1:]=[197,222,255]
        checks=[]
        for object_index,rect in enumerate(row['rects']):
            key=tuple(rect)
            source_path=row['sectorPaths'][object_index]
            expected_transform=[1,0,0,1,rect[0],rect[1],0,0,1]
            assert source_path['itemTransform']==source_path['painterTransform']==expected_transform
            assert source_path['quantized26_6ElementsEqual'] and not source_path['pathExplicitlyClosed']
            if key not in cache:
                prefix=TEMP/('bsm-'+'-'.join(map(str,rect)))
                call([CLI,prefix,*rect,1,1])
                details=read(Path(str(prefix)+'.json'))
                assert details['pathElementCount']==8 and not details['pathExplicitlyClosed'] and details['transform']==expected_transform
                assert len(details['localElements'])==len(details['elements'])==8
                curve_path=Path(str(prefix)+'-path.pathstream').read_bytes()
                assert sha(curve_path)==source_path['independentScenePathSha256']
                cache[key]={}
                geometry=dict(rect=rect,pathSha256=sha(curve_path),localPathSha256=sha(Path(str(prefix)+'-local.pathstream').read_bytes()),independentConstruction=details,coverage=[])
                for layer in ('fill','stroke'):
                    flat=Path(str(prefix)+'-'+layer+'-flat.pathstream').read_bytes()
                    own=coverage(flat)
                    cli_mask=Path(str(prefix)+'-'+layer+'.mask').read_bytes()
                    difference=int(np.count_nonzero(own!=np.frombuffer(cli_mask,np.uint8).reshape(480,800)))
                    check=dict(layer=layer,independentFlatPathSha256=sha(flat),pythonCoverageSha256=sha(own.tobytes()),cppCoverageSha256=sha(cli_mask),pythonCppDifferentCoveragePixels=difference)
                    if difference:
                        failed.mkdir(exist_ok=True)
                        (failed/(prefix.name+'-'+layer+'-python.mask.gz')).write_bytes(gzip.compress(own.tobytes(),mtime=0))
                        (failed/(prefix.name+'-'+layer+'-cpp.mask.gz')).write_bytes(gzip.compress(cli_mask,mtime=0))
                    geometry['coverage'].append(check)
                    cache[key][layer]=(own,check)
                combined=np.zeros((480,800,3),dtype=np.int64)
                combined[1:,1:]=[197,222,255]
                combined=blend(combined,cache[key]['fill'][0],255)
                combined=blend(combined,cache[key]['stroke'][0],0)
                assert frame(combined)==Path(str(prefix)+'.rgb565').read_bytes()
                geometry['pythonCppFullFrameEqual']=True
                geometries.append(geometry)
                generated.extend(sorted(TEMP.glob(prefix.name+'*')))
            for layer,enabled,source in (('fill',row['fillStyle'],255),('stroke',row['lineStyle'],0)):
                mask,check=cache[key][layer]
                checks.append(dict(object=object_index,enabled=bool(enabled),**check))
                if enabled:
                    rgb=blend(rgb,mask,source)
        actual=frame(rgb)
        decoded=TEMP/(row['name']+'-bsm.rgb565')
        call([ROOT.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe',target/'SheetPixel.dat',decoded,768000])
        native=decoded.read_bytes()
        assert len(native)==len(actual)==768000
        expected_pixels=np.frombuffer(native,'<u2').reshape(480,800)
        actual_pixels=np.frombuffer(actual,'<u2').reshape(480,800)
        ys,xs=np.where(expected_pixels!=actual_pixels)
        if len(xs):
            failed.mkdir(exist_ok=True)
            (failed/(row['name']+'-native.rgb565.gz')).write_bytes(gzip.compress(native,mtime=0))
            (failed/(row['name']+'-candidate.rgb565.gz')).write_bytes(gzip.compress(actual,mtime=0))
        lengths=(target/'PicLenPara.dat').read_bytes()
        structure=dict(controlParaPresent=(target/'ControlPara.dat').exists(),controlPixelPresent=(target/'ControlPixel.dat').exists(),picLenBytes=len(lengths),dynamicImageBytes=int.from_bytes(lengths[800:804],'little'),dynamicImageCount=int.from_bytes(lengths[1600:1604],'little'),sheetControlCount=(target/'SheetPara.dat').read_bytes()[2])
        structure['passed']=not structure['controlParaPresent'] and not structure['controlPixelPresent'] and len(lengths)==2400 and structure['dynamicImageBytes']==structure['dynamicImageCount']==structure['sheetControlCount']==0
        results.append(dict(run=row['name'],rects=row['rects'],lineStyle=row['lineStyle'],fillStyle=row['fillStyle'],sourceSha256=row['sourceSha256'],inputSourceSha256=row['inputSourceSha256'],folder=str(target.relative_to(WORK)).replace('\\','/'),B=[dict(name=file.name,bytes=file.stat().st_size,sha256=sha(file.read_bytes())) for file in sorted(target.iterdir()) if file.is_file()],S=structure,M=dict(rgb565Bytes=768000,nativeSha256=sha(native),candidateSha256=sha(actual),differentPixels=int(len(xs)),coverageChecks=checks,firstDifferences=[dict(x=int(x),y=int(y),native=int(expected_pixels[y,x]),candidate=int(actual_pixels[y,x])) for y,x in zip(ys[:12],xs[:12])]),observedSectorPaths=row['sectorPaths'],usbAccessed=False,E=False))
        decoded.unlink()
        print(row['name'],int(len(xs)),flush=True)
    repeated=[case for case in results if 'default-repeat' in case['run']]
    assert len(repeated)==3 and len({case['sourceSha256'] for case in repeated})==1
    changes=[]
    for name in sorted({file['name'] for case in repeated for file in case['B']}):
        buffers=[(WORK/case['folder']/name).read_bytes() for case in repeated]
        ranges=[]
        start=None
        length=max(map(len,buffers))
        for position in range(length):
            changed=len({raw[position] if position<len(raw) else None for raw in buffers})!=1
            if changed and start is None:
                start=position
            if start is not None and (not changed or position==length-1):
                ranges.append([start,position+1 if changed else position])
                start=None
        changes.append(dict(name=name,lengths=list(map(len,buffers)),changedRangesExclusiveEnd=ranges,classification='unknown_no_ignore_mask' if ranges else 'stable_in_three_observed_fresh_processes'))
    assert sha(CLI.read_bytes())==executable_sha
    archive=BASE/'independent-paths-r1'
    archive.mkdir(exist_ok=False)
    retained=[]
    for file in generated:
        raw=file.read_bytes()
        target=archive/(file.name+'.gz')
        target.write_bytes(gzip.compress(raw,mtime=0))
        assert gzip.decompress(target.read_bytes())==raw
        retained.append(dict(path=str(file),bytes=len(raw),sha256=sha(raw),archive=str(target.relative_to(BASE)),archiveBytes=target.stat().st_size,archiveSha256=sha(target.read_bytes())))
    result=dict(cases=results,canonicalCases=15,independentScenes=13,formalCompilations=27,B_repeatDifferences=changes,ignoredByteRanges=[],allRasterComparisonsEqual=all(case['M']['differentPixels']==0 for case in results),allStructuresValid=all(case['S']['passed'] for case in results),allPythonCppCoverageEqual=all(check['pythonCppDifferentCoveragePixels']==0 for geometry in geometries for check in geometry['coverage']),independentGeometry=geometries,candidateExecutableSha256=executable_sha,localPathAndSceneTransformPreserved=True,sectorExplicitClosureAdded=False,composition='D(v)=(v+(v>>8)+128)>>8; independent white-fill then black-stroke terms',nativeProcessLaunched=False,usbAccessed=False,E=False)
    destination.write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    for file in generated:
        assert file.resolve().is_relative_to(TEMP.resolve())
        file.unlink()
    (BASE/'independent-temp-cleanup-r1.json').write_text(json.dumps(dict(files=retained,removedFiles=len(retained),removedLogicalBytes=sum(item['bytes'] for item in retained),gzipRoundtripVerified=True,scope=str(TEMP)),ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(dict(cases=len(results),allRasterComparisonsEqual=result['allRasterComparisonsEqual'],allStructuresValid=result['allStructuresValid'],allPythonCppCoverageEqual=result['allPythonCppCoverageEqual']),ensure_ascii=False))
    if not result['allRasterComparisonsEqual'] or not result['allStructuresValid'] or not result['allPythonCppCoverageEqual']:
        raise SystemExit(1)


if __name__=='__main__':
    main()
