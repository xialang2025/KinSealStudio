"""Recheck retained native sector output with the two independently rounded integer blend terms.

No original program or original DLL is loaded. The external coverage oracle links
Qt6 and retained public Qt4 raster source solely for offline diagnostic comparison.
"""
from pathlib import Path
import gzip
import hashlib
import json
import os
import re
import shutil
import struct
import subprocess
import numpy as np
from area_candidate_256 import coverage
from historical_curve_reference import make_flat

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
WORK = ROOT.parent / 'kinseal-stage4-20261112/worktree'
EVIDENCE = WORK / 'docs/stage5-evidence/native/sector-rounding-20261114'
TEMP = HERE / 'temp'
ENV = os.environ.copy()
QT = Path('G:/01_Professional/QT6/Installed/6.12.0/mingw_64')
MINGW = Path('G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin')
ENV['PATH'] = str(QT / 'bin') + ';' + str(MINGW) + ';' + ENV.get('PATH', '')
for key in ('TEMP', 'TMP', 'TMPDIR'):
    ENV[key] = str(TEMP)


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def call(command):
    result = subprocess.run(list(map(str, command)), env=ENV, capture_output=True,
                            creationflags=subprocess.CREATE_NO_WINDOW)
    if result.returncode:
        raise RuntimeError((command, result.returncode, result.stdout, result.stderr))
    return result


def parse(raw):
    count = struct.unpack_from('>I', raw)[0]
    elements = [struct.unpack_from('>Idd', raw, 4 + index * 20) for index in range(count)]
    tail = struct.unpack_from('>II', raw, 4 + count * 20)
    return elements, tail


def serialize(elements, tail):
    return struct.pack('>I', len(elements)) + b''.join(struct.pack('>Idd', *point) for point in elements) + struct.pack('>II', *tail)


def retained_gzip(raw, folder, suffix):
    folder.mkdir(parents=True, exist_ok=True)
    target = folder / (sha(raw) + suffix + '.gz')
    if target.exists():
        if gzip.decompress(target.read_bytes()) != raw:
            raise ValueError('Existing evidence archive has different contents: ' + str(target))
    else:
        target.write_bytes(gzip.compress(raw, compresslevel=9, mtime=0))
    if gzip.decompress(target.read_bytes()) != raw:
        raise ValueError('Evidence gzip round-trip failed.')
    return dict(path=str(target.relative_to(EVIDENCE)), rawBytes=len(raw),
                uncompressedSha256=sha(raw), compressedSha256=sha(target.read_bytes()))


def blend_frame(alpha, rounding):
    """Apply source and destination D separately; never round their combined numerator."""
    alpha = alpha.astype(np.int64)[..., None]
    image = np.zeros((480, 800, 3), dtype=np.int64)
    image[1:, 1:] = [197, 222, 255]
    source_numerator = 255 * alpha
    destination_numerator = image * (255 - alpha)
    if rounding == 'qt4-shift-before-128':
        divide = lambda value: (value + (value >> 8) + 128) >> 8
    elif rounding == 'previous-shift-after-128':
        divide = lambda value: (value + 128 + ((value + 128) >> 8)) >> 8
    else:
        raise ValueError('Unknown diagnostic blend order.')
    image = divide(source_numerator) + divide(destination_numerator)
    return (((image[..., 0] >> 3) << 11) | ((image[..., 1] >> 2) << 5) | (image[..., 2] >> 3)).astype('<u2')


def main():
    if (EVIDENCE / 'sector-rounding-r1.json').exists():
        raise ValueError('Do not overwrite this evidence version.')
    retained = EVIDENCE / 'retained-native'
    retained.mkdir(parents=True, exist_ok=True)
    source_probe = WORK / 'docs/stage5-evidence/native/audit/paint-sector-r1/native-probe.json.gz'
    probe_raw_gzip = source_probe.read_bytes()
    probe = json.loads(gzip.decompress(probe_raw_gzip))
    payload = next(message['payload'] for message in probe['instrumentationMessages']
                   if 'pathStreamHex' in message.get('payload', {}))
    if payload['pen']['style'] != 0 or payload['transform'] != [1, 0, 0, 1, 100, 100, 0, 0, 1]:
        raise ValueError('Retained default source is not the expected fill-only translated path.')
    (retained / 'native-probe.json.gz').write_bytes(probe_raw_gzip)
    (retained / 'paint-payload.json').write_text(json.dumps(payload, indent=2) + '\n', encoding='utf-8')
    local = bytes.fromhex(payload['pathStreamHex'])
    elements, tail = parse(local)
    absolute = serialize([(tag, x + 100, y + 100) for tag, x, y in elements], tail)
    (retained / 'native-local.pathstream').write_bytes(local)
    (retained / 'native-absolute.pathstream').write_bytes(absolute)
    native_compressed = WORK / 'docs/stage5-evidence/native/audit/paint-sector-r1/compiled/fresh-process-3/SheetPixel.dat'
    shutil.copy2(native_compressed, retained / 'SheetPixel.dat')
    decoder = ROOT.parents[1] / 'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe'
    native_decoded = TEMP / 'native.rgb565'
    commands = [[decoder, native_compressed, native_decoded, 768000]]
    call(commands[-1])
    native_raw = native_decoded.read_bytes()
    if len(native_raw) != 768000:
        raise ValueError('Native decoded frame length differs.')
    native = np.frombuffer(native_raw, '<u2').reshape(480, 800)
    native_archive = retained_gzip(native_raw, EVIDENCE / 'frames', '.rgb565')
    oracle = ROOT / 'legacy_coverage_probe.exe'
    imports = call([MINGW / 'objdump.exe', '-p', oracle]).stdout.decode(errors='replace')
    libraries = re.findall(r'DLL Name:\s*(\S+)', imports)
    if any(name.lower().startswith(('qtcore4', 'qtgui4')) for name in libraries):
        raise ValueError('Diagnostic oracle unexpectedly imports original Qt4 DLLs.')
    generated_mask = TEMP / 'independent.mask'
    commands.append([oracle, generated_mask, 'sector', 'fill'])
    call(commands[-1])
    independent = Path(str(generated_mask) + '.pathstream').read_bytes()
    independent_file = EVIDENCE / 'independent-qt6.pathstream'
    independent_file.write_bytes(independent)
    generated_elements, generated_tail = parse(independent)
    quantized_equal = [(tag, int(x * 64), int(y * 64)) for tag, x, y in parse(absolute)[0]] == [
        (tag, int(x * 64), int(y * 64)) for tag, x, y in generated_elements]
    records = []
    for label, raw_path, input_path in (
            ('independent-qt6-arcTo45-span90', independent, independent_file),
            ('retained-native-eight-elements', absolute, retained / 'native-absolute.pathstream')):
        mask_file = TEMP / (label + '.mask')
        commands.append([oracle, mask_file, 'sector', 'fill', input_path])
        call(commands[-1])
        oracle_raw = mask_file.read_bytes()
        oracle_mask = np.frombuffer(oracle_raw, np.uint8).reshape(480, 800)
        flattened, details = make_flat(raw_path)
        (EVIDENCE / (label + '-flattened.pathstream')).write_bytes(flattened)
        independent_mask = coverage(flattened)
        mask_difference = int(np.count_nonzero(independent_mask != oracle_mask))
        mask_archive = retained_gzip(independent_mask.tobytes(), EVIDENCE / 'masks', '.mask')
        formulas = []
        for order in ('previous-shift-after-128', 'qt4-shift-before-128'):
            candidate = blend_frame(independent_mask, order)
            ys, xs = np.where(candidate != native)
            frame_raw = candidate.tobytes()
            formulas.append(dict(rounding=order, differentPixels=int(len(xs)), bytes=len(frame_raw),
                                 frameSha256=sha(frame_raw), frame=retained_gzip(frame_raw, EVIDENCE / 'frames', '.rgb565'),
                                 first=[dict(x=int(x), y=int(y), native=int(native[y, x]),
                                             candidate=int(candidate[y, x]), coverage=int(independent_mask[y, x]))
                                        for y, x in zip(ys[:12], xs[:12])]))
        records.append(dict(pathSource=label, inputPathSha256=sha(raw_path), curveDetails=details,
                            oracleCoverageSha256=sha(oracle_raw), independentCoverageSha256=sha(independent_mask.tobytes()),
                            differentOracleCoveragePixels=mask_difference, mask=mask_archive, frameComparisons=formulas))
    result = dict(scope='Retained default sector only: geometry 100,100,200,200; center 200,200; start45 span90; white fill; no pen; alpha255.',
                  cases=records, nativeFrame=native_archive, nativeProbeSha256=sha(probe_raw_gzip),
                  nativeCompressedSheetPixelSha256=sha(native_compressed.read_bytes()),
                  quantizedNativeAndIndependentPathsEqual=quantized_equal,
                  independentPathElements=len(generated_elements), nativePathElements=len(elements),
                  nativeFillRule=tail[1], independentFillRule=generated_tail[1],
                  diagnosticOracle=dict(path=str(oracle), sha256=sha(oracle.read_bytes()), importedLibraries=libraries,
                                        publicLibrarySourceOnly=True, originalQt4DllImported=False),
                  previousOnePixelFailurePreserved=True, productionSupported=False,
                  nativeProcessLaunched=False, originalDllLoaded=False, usbAccessed=False, productionChanged=False)
    (EVIDENCE / 'sector-rounding-r1.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    (EVIDENCE / 'commands-r1.json').write_text(json.dumps(dict(commands=[list(map(str, command)) for command in commands]), indent=2) + '\n', encoding='utf-8')
    print(json.dumps(dict(pathCandidates=len(records), coverageDifferences=[item['differentOracleCoveragePixels'] for item in records],
                          frameDifferences=[(item['pathSource'], [(f['rounding'], f['differentPixels']) for f in item['frameComparisons']]) for item in records],
                          nativeFrameSha256=sha(native_raw), quantizedPathsEqual=quantized_equal)))
    if any(item['differentOracleCoveragePixels'] or item['frameComparisons'][1]['differentPixels'] for item in records):
        raise SystemExit(1)


if __name__ == '__main__':
    main()
