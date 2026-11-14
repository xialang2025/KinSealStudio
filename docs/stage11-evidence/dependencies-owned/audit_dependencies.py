#!/usr/bin/env python3
"""Read static dependency and licence identities without loading inspected modules."""
#20261114-10 add: Keep the dependency audit independent of Qt and vendor executables.
import hashlib
import argparse
import json
import re
import shutil
import struct
import xml.etree.ElementTree as ET
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
OUT = Path(__file__).resolve().parent
QT = Path('G:/01_Professional/QT6/Installed/6.12.0/mingw_64')
QT_DOC = Path('G:/01_Professional/QT6/Installed/Docs/Qt-6.12.0')
MINGW = Path('G:/01_Professional/QT6/Installed/Tools/mingw1310_64')
PACKAGE = Path('Z:/Project/vscode_prj_temp/REA/KinSealStudio/dist/KinSealStudioLite-Stage9-plc-r2')


def identity(path):
    """Return byte identities for a readable file; do not infer authenticity."""
    data = path.read_bytes()
    return {'path': path.as_posix(), 'bytes': len(data),
            'sha256': hashlib.sha256(data).hexdigest(), 'sha1': hashlib.sha1(data).hexdigest()}


def pe_imports(path):
    """Parse bounded PE regular and delay import descriptors without loading code."""
    data = path.read_bytes()
    def unpack(fmt, offset):
        if offset < 0 or offset + struct.calcsize(fmt) > len(data):
            raise ValueError('PE read outside file')
        return struct.unpack_from(fmt, data, offset)
    def cstring(offset):
        end = data.find(b'\0', offset, min(offset + 512, len(data)))
        if end < 0:
            raise ValueError('Unterminated import name')
        return data[offset:end].decode('ascii')
    if data[:2] != b'MZ':
        raise ValueError('Missing MZ')
    pe = unpack('<I', 0x3c)[0]
    if data[pe:pe + 4] != b'PE\0\0':
        raise ValueError('Missing PE')
    machine, section_count = unpack('<HH', pe + 4)
    optional_size = unpack('<H', pe + 20)[0]
    optional = pe + 24
    magic = unpack('<H', optional)[0]
    if magic not in (0x10b, 0x20b):
        raise ValueError('Unsupported optional header')
    directory = optional + (112 if magic == 0x20b else 96)
    image_base = unpack('<Q' if magic == 0x20b else '<I', optional + (24 if magic == 0x20b else 28))[0]
    directory_count = unpack('<I', directory - 4)[0]
    headers_size = unpack('<I', optional + 60)[0]
    sections = []
    for i in range(section_count):
        offset = optional + optional_size + i * 40
        size, rva, raw_size, raw_offset = unpack('<IIII', offset + 8)
        sections.append((rva, max(size, raw_size), raw_offset, raw_size))
    def rva_offset(rva):
        if rva < headers_size and rva < len(data):
            return rva
        for start, length, raw, raw_size in sections:
            if start <= rva < start + length and rva - start < raw_size:
                return raw + rva - start
        raise ValueError('Unmapped import RVA')
    result = {'machine': hex(machine), 'architecture': {0x8664: 'x86_64', 0x14c: 'x86'}.get(machine, 'unknown'),
              'regularImports': [], 'delayImports': [], 'inspection': 'static PE headers only; no DLL loaded'}
    for index, field, size in ((1, 'regularImports', 20), (13, 'delayImports', 32)):
        if index >= directory_count:
            continue
        rva, directory_size = unpack('<II', directory + index * 8)
        if not rva:
            continue
        offset = rva_offset(rva)
        for i in range(min(directory_size // size + 1, 4096)):
            values = unpack('<' + 'I' * (size // 4), offset + i * size)
            if not any(values):
                break
            name_rva = values[3] if index == 1 else values[1]
            if index == 13 and values[0] & 1 == 0:
                name_rva -= image_base
            result[field].append(cstring(rva_offset(name_rva)))
        else:
            raise ValueError('Import directory exceeds limit')
    return result


def write_json(path, value):
    """Write deterministic UTF-8 audit output under the allocated directories."""
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8', newline='\n')


def main():
    """Inventory package PE files, resource inputs and locally available licence texts."""
    global OUT, PACKAGE
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--package', type=Path, default=PACKAGE)
    parser.add_argument('--output', type=Path, default=OUT)
    args = parser.parse_args()
    output = args.output.resolve()
    if not output.is_relative_to(OUT.resolve()):
        parser.error('output must remain under dependencies-owned')
    OUT = output
    PACKAGE = args.package.resolve(strict=True)
    OUT.mkdir(parents=True, exist_ok=True)
    license_dir = OUT / 'licenses'
    license_dir.mkdir(exist_ok=True)
    license_sources = [
        ('Qt-GPL3-LGPL3-LICENSE.txt', Path('G:/01_Professional/QT6/Installed/Licenses/LICENSE')),
        ('Qt-Documentation-FDL.txt', Path('G:/01_Professional/QT6/Installed/Licenses/LICENSE.FDL')),
        ('GCC-GPL3.txt', MINGW / 'licenses/gcc/COPYING3'),
        ('GCC-Runtime-Exception-3.1.txt', MINGW / 'licenses/gcc/COPYING.RUNTIME'),
        ('MinGW-w64-runtime.txt', MINGW / 'licenses/mingw-w64/COPYING.MinGW-w64-runtime.txt'),
        ('winpthreads-COPYING.txt', MINGW / 'licenses/winpthreads/COPYING'),
        ('LZO-MIT-LICENSE.txt', ROOT / 'third_party/lzokay/LICENSE'),
        ('QR-MIT-LICENSE.txt', PACKAGE / 'QR-MIT-LICENSE.txt'),
    ]
    for section in ('qtcore', 'qtgui', 'qtnetwork'):
        for source in sorted((QT_DOC / section).glob('*-attribution-*.html')):
            license_sources.append((source.name, source))
    for name in ('qt-attribution-llvmpipe.html', 'qt-attribution-llvm.html', 'licenses-used-in-qt.html', 'lgpl.html'):
        license_sources.append((name, QT_DOC / 'qtdoc' / name))
    license_records = []
    for name, source in license_sources:
        if not source.is_file():
            license_records.append({'source': source.as_posix(), 'state': 'missing'})
            continue
        dest = license_dir / name
        shutil.copyfile(source, dest)
        license_records.append({'source': identity(source), 'retained': identity(dest),
                                'sameBytes': source.read_bytes() == dest.read_bytes(),
                                'meaning': 'local notice text; not a grant covering unrelated vendor resources'})
    sboms = []
    spdx_files = {}
    for name in ('qtbase', 'qtsvg', 'qtserialport'):
        source = QT / 'sbom' / (name + '-6.12.0.spdx.json')
        document = json.loads(source.read_text(encoding='utf-8'))
        sboms.append(identity(source))
        for record in document.get('files', []):
            spdx_files[record['fileName'].lstrip('./')] = {'sourceSbom': source.as_posix(), **record}
    package_files = {p.name.lower(): p for p in PACKAGE.rglob('*') if p.is_file() and p.suffix.lower() in ('.exe', '.dll')}
    pe_records = []
    for path in sorted(package_files.values()):
        item = identity(path)
        item['relativePath'] = path.relative_to(PACKAGE).as_posix()
        item.update(pe_imports(path))
        imports = item['regularImports'] + item['delayImports']
        item['importsPresentSomewhereInPackage'] = [n for n in imports if n.lower() in package_files]
        item['importsOutsidePackage'] = [n for n in imports if n.lower() not in package_files]
        item['outsidePackageMeaning'] = 'declared imports; Windows/API-set candidates, not a runtime DLL search-path proof'
        qt_rel = ('plugins/' + item['relativePath']) if '/' in item['relativePath'] else ('bin/' + path.name)
        local_source = QT / qt_rel
        if not local_source.is_file():
            local_source = MINGW / 'bin' / path.name
        if local_source.is_file():
            item['installedCandidate'] = identity(local_source)
            item['matchesInstalledCandidate'] = item['sha256'] == item['installedCandidate']['sha256']
        if qt_rel in spdx_files:
            item['qtSbom'] = spdx_files[qt_rel]
            item['matchesSbomSha1'] = any(c['algorithm'] == 'SHA1' and c['checksumValue'] == item['sha1']
                                            for c in item['qtSbom'].get('checksums', []))
        pe_records.append(item)
    resource_groups = []
    for name in ('native_shell.qrc', 'system_gallery.qrc', 'macro_library.qrc', 'native_test_template.qrc', 'plc_catalog.qrc'):
        qrc = ROOT / name
        entries = []
        for entry in ET.fromstring(qrc.read_bytes()).iter('file'):
            source = (ROOT / entry.text).resolve()
            data = identity(source)
            data['relativePath'] = source.relative_to(ROOT).as_posix()
            data['alias'] = entry.attrib.get('alias')
            entries.append(data)
        aggregate = hashlib.sha256(json.dumps([{k: e[k] for k in ('relativePath', 'bytes', 'sha256')} for e in entries],
                                              ensure_ascii=False, sort_keys=True, separators=(',', ':')).encode()).hexdigest()
        resource_groups.append({'qrc': identity(qrc), 'fileCount': len(entries), 'totalBytes': sum(x['bytes'] for x in entries),
                                'inputSetSha256': aggregate, 'files': entries,
                                'distributionAuthority': 'not established by technical provenance'})
    system_usb = []
    for name in ('C:/Windows/System32/libusb0.dll', 'C:/Windows/SysWOW64/libusb0.dll'):
        source = Path(name)
        if source.is_file():
            system_usb.append({**identity(source), **pe_imports(source), 'redistributed': False,
                               'policy': 'Use compatible preinstalled system driver/runtime; no copy or driver installation'})
    font_files = [p.relative_to(PACKAGE).as_posix() for p in PACKAGE.rglob('*') if p.is_file() and p.suffix.lower() in ('.ttf', '.otf', '.ttc', '.woff', '.woff2')]
    result = {'format': 'KinSealLite-stage11-dependency-audit-v1', 'observedAtUtc': datetime.now(timezone.utc).isoformat(),
              'auditKind': 'read-only static inventory; no hardware I/O or vendor module execution',
              'package': PACKAGE.as_posix(), 'sourceRoot': ROOT.as_posix(),
              'stage9Manifest': identity(PACKAGE / 'manifest.json'),
              'executable': identity(PACKAGE / 'KinSealStudioLite.exe'),
              'peCount': len(pe_records), 'peFiles': pe_records, 'qtSbomSources': sboms,
              'localLicenseTexts': license_records, 'resources': resource_groups,
              'fontFilesInPackage': font_files, 'systemUsbCandidates': system_usb,
              'limitations': ['Static imports do not enumerate all runtime load events.',
                              'Qt SBOM is local package metadata, not an independent authenticity certification.',
                              'Original gallery/icons/source fragments/test payloads have provenance but no verified redistribution grant.',
                              'Host fonts/driver/SDK are external and must not be copied based on this audit.',
                              'No clean-environment missing-dependency run was performed by this audit.']}
    write_json(OUT / 'inventory.json', result)
    summary = {'peCount': len(pe_records), 'qtSbomMatches': sum(x.get('matchesSbomSha1', False) for x in pe_records),
               'installedCandidateMatches': sum(x.get('matchesInstalledCandidate', False) for x in pe_records),
               'licenseTexts': len(license_records), 'licenseTextBytes': sum(x.get('retained', {}).get('bytes', 0) for x in license_records),
               'resourceGroups': [{'qrc': Path(x['qrc']['path']).name, 'files': x['fileCount'], 'bytes': x['totalBytes']} for x in resource_groups],
               'fontFilesInPackage': font_files, 'inventory': identity(OUT / 'inventory.json')}
    write_json(OUT / 'summary.json', summary)
    print(json.dumps(summary, ensure_ascii=False))


if __name__ == '__main__':
    main()
