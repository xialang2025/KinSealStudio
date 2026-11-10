"""Verify native language selection preservation with a frozen portable program.

The default is offline. --download writes the three-language native UI sample
only after checking immutable inputs and exactly one online matching screen.
"""
#20261110-6 add: Reuse the existing identity, CLI logging and device enumeration contracts.
import argparse
import json
from pathlib import Path
import sys
from stage1_model_verification import ROOT, cli, devices, now, save, sha


def verify(executable, output, download):
    """Input: Portable EXE, new result folder and transfer opt-in. Return: Actual verification status."""
    if output.exists():
        raise RuntimeError('Each verification requires a new output directory.')
    output.mkdir(parents=True)
    record = dict(startedAtUtc=now(), status='running', steps=[], usbAccessed=False,
                  firmwareVersion=None, physicalTouchConfirmed=False)
    try:
        if sha(executable) != sha(ROOT / 'build/KinSealStudioLite.exe'):
            raise RuntimeError('Portable and build executables differ.')
        frozen = {str(executable): sha(executable)}
        record['executable'] = dict(path=str(executable), sha256=sha(executable), bytes=executable.stat().st_size)
        for name, expected_index, count in [('settings1-editor0',0,2),('settings0-editor1',1,2),('settings2-editor1-three',1,3)]:
            source = executable.parent / 'tests/fixtures/native-source/language' / (name+'-native.sm')
            oracle = source.with_name(name+'-SysPara.dat')
            frozen[str(source)] = sha(source)
            frozen[str(oracle)] = sha(oracle)
            local = output / source.name
            local.write_bytes(source.read_bytes())
            restored = output / (name+'-restored.sm')
            compiled = output / (name+'-compiled')
            record['steps'].append(cli(executable,['--export-native-project',local,restored],output,name+'-export'))
            if sha(local) != sha(restored):
                raise RuntimeError('Unedited native source bytes changed: '+name)
            record['steps'].append(cli(executable,['--compile-project',local,compiled],output,name+'-compile'))
            system = (compiled/'SysPara.dat').read_bytes()
            native = oracle.read_bytes()
            if len(system) != len(native) or system[2172:2176] != native[2172:2176]:
                raise RuntimeError('Recovered system language fields differ from native output: '+name)
            if int.from_bytes(system[2172:2174],'little') != expected_index or int.from_bytes(system[2174:2176],'little') != count:
                raise RuntimeError('Unexpected native language index/count: '+name)
        save(output/'input-identities.json',frozen)
        if any(sha(Path(path)) != value for path,value in frozen.items()):
            raise RuntimeError('A frozen input changed during verification.')
        if download:
            current = devices()
            current['observedAtUtc'] = now()
            save(output/'pre-transfer-devices.json',current)
            online = [item for item in current['devices'] if item['Status']=='OK']
            if len(online)!=1:
                raise RuntimeError('Expected exactly one online matching USB screen.')
            record['usbDevice'] = online[0]
            record['usbAccessed'] = True
            result = cli(executable,['--download-project',output/'settings2-editor1-three-native.sm'],output,'usb-download')
            record['steps'].append(result)
            if not (result.get('result') or {}).get('ok'):
                raise RuntimeError('Transport did not report accepted segments and commit.')
            record['transportAccepted'] = True
        record['status'] = 'passed_with_physical_observation_pending'
    except Exception as error:
        record.update(status='failed',error=str(error))
    record['completedAtUtc'] = now()
    save(output/'run.json',record)
    return record


def main():
    """Input: Command-line arguments. Return: Nonzero for an actual failure."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable',type=Path,default=ROOT/'dist/KinSealStudioLite-Stage1-0.8.1/KinSealStudioLite.exe')
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--download',action='store_true')
    args = parser.parse_args()
    result = verify(args.executable.resolve(),args.output.resolve(),args.download)
    print(json.dumps({key:result.get(key) for key in ['status','error','transportAccepted']},ensure_ascii=False))
    return result['status']=='failed'


if __name__=='__main__':
    sys.exit(main())
