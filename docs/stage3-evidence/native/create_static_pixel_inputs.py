"""#20261110-8 add: Author explicit source-only probes for independent native compilation."""
from pathlib import Path
import copy
import hashlib
import json
import os
import subprocess


def main():
    """Export odd-width static controls without using the product device compiler."""
    root = Path(r'Z:/Project/vscode_prj_temp/REA/KinSealStudio')
    output = root / 'docs/stage3-evidence/native/inputs'
    seed = output / 'product-five-compact-all-address-roles.ksp'
    exporter = root / 'dist/KinSealStudioLite-Stage2-0.9.0/KinSealStudioLite.exe'
    project = json.loads(seed.read_text(encoding='utf8'))
    text = copy.deepcopy(project['pages'][0]['objects'][0])
    text.update(id='static-base-odd', name='Explicit odd-width static base', text='', x=41, y=53, width=101, height=61, layer=1)
    text['staticText'].update(texts={'中文': '', 'English': ''}, style=2, opacity=255, foregroundColor='#ff193aaf', backgroundColor='#ff67c02b', fontSize=13, spacing=0, visibilityEnabled=False, bitEnabled=False, wordEnabled=False)
    top = copy.deepcopy(text)
    top.update(id='static-transparent-top-odd', name='Explicit transparent top', text='T', x=54, y=64, width=55, height=29, layer=2)
    top['staticText'].update(texts={'中文': 'T', 'English': 'T'}, style=0, opacity=255, textColor='#ffffd020', backgroundColor='#ffe02020', foregroundColor='#ff4060a0')
    manifest = {'inputOrigin': 'Explicit product-authored KSP then source-only export; not native UI-created objects and not a product C oracle', 'exporter': str(exporter), 'exporterSha256': hashlib.sha256(exporter.read_bytes()).hexdigest(), 'seedSha256': hashlib.sha256(seed.read_bytes()).hexdigest(), 'variants': []}
    for name in ('base-only', 'transparent-top', 'opacity128-top'):
        sample = copy.deepcopy(project)
        sample['projectName'] = 'Stage3 native pixel benchmark ' + name
        sample['macros'] = {}
        sample['macroIds'] = {}
        sample['pages'] = [copy.deepcopy(project['pages'][0])]
        sample['pages'][0]['objects'] = [copy.deepcopy(text)]
        if name != 'base-only':
            sample['pages'][0]['objects'].append(copy.deepcopy(top))
        if name == 'opacity128-top':
            sample['pages'][0]['objects'][1]['staticText'].update(style=1, opacity=128)
        ksp = output / ('static-odd2-' + name + '.ksp')
        source = ksp.with_suffix('.sm')
        assert not ksp.exists() and not source.exists()
        ksp.write_text(json.dumps(sample, ensure_ascii=False, indent=2) + '\n', encoding='utf8')
        run = subprocess.run([str(exporter), '--export-native-project', str(ksp), str(source)], cwd=str(exporter.parent), env={**os.environ, 'QT_QPA_PLATFORM': 'windows'}, creationflags=0x08000000, capture_output=True, timeout=45)
        (output / ('static-odd2-' + name + '-export.txt')).write_bytes(run.stdout + b'\nSTDERR\n' + run.stderr)
        assert run.returncode == 0 and source.is_file(), run.stdout
        manifest['variants'].append({'name': name, 'ksp': ksp.name, 'kspSha256': hashlib.sha256(ksp.read_bytes()).hexdigest(), 'source': source.name, 'sourceSha256': hashlib.sha256(source.read_bytes()).hexdigest(), 'objects': sample['pages'][0]['objects']})
    (output / 'static-pixel-input-provenance.json').write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + '\n', encoding='utf8')
    print(json.dumps({'exported': [v['source'] for v in manifest['variants']]}))


if __name__ == '__main__':
    main()
