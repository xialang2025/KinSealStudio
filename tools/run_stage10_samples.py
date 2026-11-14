"""Prepare bounded HMI fixtures or execute one explicitly selected download on an owned desktop."""
#20261114-9 add: Keep PC checks, physical transfer, human observations and recovery separate.
import argparse
import hashlib
import importlib.util
import json
import sys
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
DIST=Path('Z:/Project/vscode_prj_temp/REA/KinSealStudio/dist')
CURRENT=DIST/'KinSealStudioLite-Stage9-plc-r2/KinSealStudioLite.exe'
RESTORE=DIST/'KinSealStudioLite-Stage6-transfer-r1/KinSealStudioLite.exe'
CURRENT_SHA='aee3c13a5b3ad36ea67c4d68e4033858b3f1d31e952dbf02520186819c962709'
RESTORE_SHA='7dd94b3ccb01b3fa695c773cf6a4e59bbb47ee384277ef242cded1233188c2ef'
SOURCES={'bit-lamp-local':ROOT/'samples/stage10/bit-lamp-local.ksp',
         'word-lamp-local':ROOT/'samples/stage10/word-lamp-local.ksp',
         'function-local':ROOT/'samples/stage10/function-local.ksp',
         'restore-macro-14':ROOT/'samples/stage6/macro-14.ksp'}


def sha(path):
    """Input: regular file. Return: exact SHA256."""
    return hashlib.sha256(path.read_bytes()).hexdigest()


def save(path,value):
    """Input: owned evidence path and bounded value. Return: None after UTF8 serialization."""
    path.write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')


def main():
    """Input: mode, new output, and a selected case for device operations. Return: actual process/contract status."""
    #20261114-9 add: Refuse optimization before any output or device access because contract assertions are safety gates.
    if sys.flags.optimize:
        raise SystemExit('Run with Python optimization disabled; contract assertions must remain active')
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('mode',choices=['prepare','download'])
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--case',choices=list(SOURCES))
    parser.add_argument('--prepared',type=Path)
    args=parser.parse_args()
    output=args.output.resolve()
    if output.drive.lower()!='z:' or output.exists():
        parser.error('Require a new Z-drive output')
    if args.mode=='download' and (not args.case or not args.prepared):
        parser.error('A single case and its prepared evidence are required')
    assert sha(CURRENT)==CURRENT_SHA and sha(RESTORE)==RESTORE_SHA
    output.mkdir(parents=True)
    temporary=output/'temp'
    temporary.mkdir()
    spec=importlib.util.spec_from_file_location('stage10_private',ROOT/'docs/stage1-evidence/gui-portable/probe.py')
    probe=importlib.util.module_from_spec(spec)
    spec.loader.exec_module(probe)
    desktop=probe.PrivateDesktop(output)
    record=dict(mode=args.mode,case=args.case,usbAccessed=False,screenDisplayAndTouchConfirmed=False,
        firmwareVersion='not-observed',systemInput=False,desktopSwitched=False,runs=[],cases={},passed=False)

    def run(name,exe,argv):
        """Input: frozen binary and explicit arguments. Return: bounded actual private process result."""
        environment=probe.clean_environment(exe.parent)
        for key in ('TEMP','TMP','TMPDIR'):
            environment[key]=str(temporary)
        environment['KINSEAL_ARM_SDK']='G:/01_Professional/触摸屏/金玺/KinSealStudio_Setup_V5.1/Installed/script'
        environment['KINSEAL_MACRO_TEMP_ROOT']=str(temporary)
        result=desktop.run(name,exe,argv,environment,timeout=180)
        result.update(executableSha256=sha(exe))
        record['runs'].append(result)
        save(output/'run.json',record)
        if not result['passed']:
            raise RuntimeError(f'{name} failed with exit {result.get("exitCode")}; see {result["stdout"]} and {result["stderr"]}')
        return result

    try:
        if args.mode=='prepare':
            for case,source in SOURCES.items():
                exe=RESTORE if case.startswith('restore') else CURRENT
                contract=output/(case+'-contract.json')
                compiled=output/(case+'-compiled')
                run(case+'-validate',exe,['--validate-project',str(source)])
                run(case+'-compile',exe,['--compile-project',str(source),str(compiled)])
                run(case+'-verify',exe,['--verify-compiled-package',str(compiled)])
                run(case+'-contract',exe,['--transfer-contract',str(source),str(contract)])
                document=json.loads(contract.read_text(encoding='utf-8'))
                assert document['identity']['sourceSha256']==sha(source) and not document['usbAccessed']
                if case.startswith('restore'):
                    old=json.loads((ROOT/'docs/stage6-evidence/physical-r2/macro-14-contract.json').read_text(encoding='utf-8'))
                    assert {s['name']:s['wireSha256'] for s in old['segments']}=={s['name']:s['wireSha256'] for s in document['segments']}
                    assert document['payloadCount']==14
                else:
                    run(case+'-capture',exe,['--capture-preview',str(output/(case+'.png')),str(source)])
                record['cases'][case]=dict(source=str(source),sourceSha256=sha(source),contract=str(contract),
                    contractSha256=sha(contract),executable=str(exe),executableSha256=sha(exe),payloadCount=document['payloadCount'])
        else:
            prepared=json.loads((args.prepared/'run.json').read_text(encoding='utf-8'))
            assert prepared['passed'] and 'restore-macro-14' in prepared['cases']
            identity=prepared['cases'][args.case]
            source=Path(identity['source'])
            exe=Path(identity['executable'])
            assert exe.resolve()==(RESTORE if args.case.startswith('restore') else CURRENT).resolve()
            contract=Path(identity['contract'])
            assert source.resolve()==SOURCES[args.case].resolve()
            assert sha(source)==identity['sourceSha256'] and sha(exe)==identity['executableSha256'] and sha(contract)==identity['contractSha256']
            record['usbAccessed']=True
            result=run(args.case+'-download',exe,['--download-project',str(source)])
            lines=[json.loads(line) for line in (output/result['stdout']).read_text(encoding='utf-8-sig').splitlines() if line.strip()]
            final=lines[-1]
            assert final['kind']=='transfer-result'
            assert all(final[k] for k in ('ok','compiled','transferAccepted','commitAccepted','resetWritten','postResetReadCompleted','interfaceReleased','closed'))
            assert final['postResetReadResult']==-5 and not final['screenDisplayAndTouchConfirmed']
            expected={s['name']:s['wireSha256'] for s in json.loads(contract.read_text(encoding='utf-8'))['segments']}
            events=[line for line in lines if line.get('kind')=='transfer-event' and line.get('operation')=='payload-write']
            assert len(events)==len(expected)
            assert {line['segment']:line['sha256'] for line in events}==expected
            assert all(line['accepted'] and line['requested']==line['returned'] for line in events)
            record['finalResult']=final
            record['preparedIdentity']=identity
            record['expectedPayloads']=expected
            record['rawEventCount']=len(lines)-1
        record['passed']=True
    except Exception as error:
        record['error']=repr(error)
    finally:
        desktop.close()
        record['privateDesktopClosed']=True
        record['temporaryFilesRemaining']=[str(p.relative_to(temporary)) for p in temporary.rglob('*') if p.is_file()]
        save(output/'run.json',record)
    print(json.dumps(dict(passed=record['passed'],mode=args.mode,runs=len(record['runs']),error=record.get('error')),ensure_ascii=False))
    return 0 if record['passed'] else 1


if __name__=='__main__':
    raise SystemExit(main())
