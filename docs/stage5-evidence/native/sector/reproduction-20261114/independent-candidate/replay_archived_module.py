"""Rebuild the frozen candidate and compare 60 retained reference frames, without native software."""
from pathlib import Path
import argparse
import gzip
import hashlib
import json
import os
import subprocess


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--qt', default='G:/01_Professional/QT6/Installed/6.12.0/mingw_64')
    parser.add_argument('--mingw', default='G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin')
    parser.add_argument('--output', required=True, help='A fresh output directory on a drive with sufficient space.')
    args = parser.parse_args()
    base = Path(__file__).resolve().parent
    output = Path(args.output).resolve()
    output.mkdir(parents=True, exist_ok=False)
    temp = output / 'temp'
    temp.mkdir()
    qt, mingw = Path(args.qt), Path(args.mingw)
    env = os.environ.copy()
    env['PATH'] = str(mingw) + ';' + str(qt / 'bin') + ';' + env.get('PATH', '')
    for key in ('TEMP', 'TMP', 'TMPDIR'):
        env[key] = str(temp)
    manifest = json.loads((base / 'manifest-r1.json').read_text(encoding='utf-8'))
    for record in manifest['files']:
        raw = (base / record['path']).read_bytes()
        assert len(raw) == record['bytes'] and sha(raw) == record['sha256'], record['path']
    source = base / 'source/current'
    command = [str(mingw / 'g++.exe'), '-std=c++17', '-O2', '-Wall', '-Wextra', '-Wconversion',
               '-DQT_NO_DEBUG', '-isystem', str(qt / 'include'), '-isystem', str(qt / 'include/QtCore'),
               '-isystem', str(qt / 'include/QtGui'), '-I' + str(source), str(base / 'source/paint_probe.cpp'),
               str(source / 'device_ellipse_raster.cpp'), str(source / 'device_sector_raster.cpp'),
               str(source / 'device_polygon_raster.cpp'), '-L' + str(qt / 'lib'), '-lQt6Gui', '-lQt6Core',
               '-o', str(output / 'paint-current.exe')]
    run = subprocess.run(command, env=env, capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
    (output / 'build.log').write_bytes(run.stdout + run.stderr)
    if run.returncode:
        raise RuntimeError(('build', run.returncode, run.stdout, run.stderr))
    result = json.loads((base / 'reports/production-sharing-crosscheck-r2.json').read_text(encoding='utf-8'))
    index = json.loads((base / 'artifact-index-r1.json').read_text(encoding='utf-8'))
    lookup = {item['sha256']: item for item in index['blobs']}
    records = []
    commands = [command]
    for case in result['cases']:
        blob = lookup[case['referenceSha256']]
        reference = gzip.decompress((base / blob['path']).read_bytes())
        assert len(reference) == 768000 and sha(reference) == case['referenceSha256']
        target = temp / (case['family'] + '-' + case['case'] + '-' + str(case['line']) + str(case['fill']) + '.rgb565')
        command = [str(output / 'paint-current.exe'), case['family'], *map(str, case['geometry']),
                   str(case['line']), str(case['fill']), str(target)]
        run = subprocess.run(command, env=env, capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
        if run.returncode:
            raise RuntimeError((command, run.returncode, run.stdout, run.stderr))
        actual = target.read_bytes()
        assert len(actual) == 768000
        different = sum(a != b for a, b in zip(memoryview(reference).cast('H'), memoryview(actual).cast('H')))
        records.append(dict(family=case['family'], case=case['case'], line=case['line'], fill=case['fill'],
                            differentPixels=different, sha256=sha(actual)))
        commands.append(command)
    final = dict(cases=records, count=len(records), allEqual=all(x['differentPixels'] == 0 for x in records),
                 buildLogBytes=(output / 'build.log').stat().st_size,
                 nativeProcessLaunched=False, originalDllLoaded=False, usbAccessed=False)
    (output / 'result.json').write_text(json.dumps(final, indent=2) + '\n', encoding='utf-8')
    (output / 'commands.json').write_text(json.dumps(commands, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(dict(count=final['count'], allEqual=final['allEqual'], buildLogBytes=final['buildLogBytes'])))
    if not final['allEqual']:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
