"""Compare coverage and quantization candidates; never open a product compile gate."""
from pathlib import Path
import collections
import hashlib
import json
import os
import subprocess
import numpy as np

root = Path(__file__).resolve().parent
work = root.parent / 'kinseal-stage4-20261112/worktree'
base = work / 'docs/stage5-evidence/native/audit'
environment = os.environ.copy()
environment.update(TEMP=str(root/'temp'), TMP=str(root/'temp'), TMPDIR=str(root/'temp'))
environment['PATH'] = 'G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;' + environment.get('PATH', '')
decoder = root.parents[1] / 'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe'

def invoke(args):
    subprocess.run(list(map(str, args)), check=True, capture_output=True, env=environment, creationflags=subprocess.CREATE_NO_WINDOW)

def rgb565(rgb):
    return ((rgb[..., 0] >> 3) << 11) | ((rgb[..., 1] >> 2) << 5) | (rgb[..., 2] >> 3)

def expand565(values, replicate=True):
    red = (values >> 11) & 31
    green = (values >> 5) & 63
    blue = values & 31
    return np.stack(((red << 3) | (red >> 2) if replicate else red << 3,
                     (green << 2) | (green >> 4) if replicate else green << 2,
                     (blue << 3) | (blue >> 2) if replicate else blue << 3), axis=-1)

def mix(destination, source, alpha, mode):
    a = alpha.astype(np.int32)[..., None]
    if mode.startswith('components'):
        drgb = np.stack(((destination >> 11) & 31, (destination >> 5) & 63, destination & 31), axis=-1)
        srgb = np.array([31, 63, 31]) if source else np.array([0, 0, 0])
        if mode == 'components-5bit':
            aq = (a + 1) >> 3
            result = (srgb * aq + drgb * (32 - aq)) >> 5
        elif mode == 'components-256':
            result = (srgb * a + drgb * (256 - a)) >> 8
            result = np.where(a == 255, srgb, result)
        else:
            result = (srgb * a + drgb * (255 - a) + (127 if mode.endswith('round') else 0)) // 255
        return (result[..., 0] << 11) | (result[..., 1] << 5) | result[..., 2]
    dst = expand565(destination, not mode.startswith('truncate'))
    value = (255 if source else 0) * a + dst * (255 - a)
    if mode.endswith('qt'):
        value += 128
        value = (value + (value >> 8)) >> 8
    else:
        value = (value + (127 if mode.endswith('round') else 0)) // 255
    return rgb565(value)

records = []
for family in ('ellipse', 'polygon', 'roundrect', 'sector'):
    folder = base / ('audit-' + family + '-default-r1/compiled/canonical-default')
    native = root / ('temp/aa-' + family + '-native.rgb565')
    invoke([decoder, folder/'SheetPixel.dat', native, 768000])
    expected_bytes = native.read_bytes()
    expected = np.frombuffer(expected_bytes, dtype='<u2').reshape(480, 800).astype(np.int32)
    background = np.full((480, 800), 0, dtype=np.int32)
    background[1:, 1:] = int(rgb565(np.array([197, 223, 255], dtype=np.int32)))
    masks = {}
    candidates = []
    for aa in ('aa', 'alias'):
        for layer in ('fill', 'stroke', 'combined'):
            path = root / ('temp/aa-' + family + '-' + aa + '-' + layer + '.argb32')
            invoke([root/'aa_mask_probe.exe', path, family, layer, aa])
            data = path.read_bytes()
            masks[aa, layer] = np.frombuffer(data, dtype=np.uint8).reshape(480, 800, 4)[..., 3].astype(np.int32)
            path.unlink()
        for mode in ('expand-floor','expand-round','expand-qt','truncate-floor','truncate-round','components-floor','components-round','components-256','components-5bit'):
            actual = mix(background, family != 'polygon', masks[aa, 'fill'], mode)
            if family != 'sector':
                actual = mix(actual, False, masks[aa, 'stroke'], mode)
            diff = actual != expected
            ys, xs = np.where(diff)
            candidates.append(dict(aa=aa, mode=mode, differentPixels=int(diff.sum()), first=[dict(x=int(x), y=int(y), native=int(expected[y,x]), candidate=int(actual[y,x]), fill=int(masks[aa,'fill'][y,x]), stroke=int(masks[aa,'stroke'][y,x])) for y,x in zip(ys[:8],xs[:8])]))
    candidates.sort(key=lambda c:c['differentPixels'])
    palette = collections.Counter(map(int, expected.flatten()))
    records.append(dict(family=family, nativeRawSha256=hashlib.sha256(expected_bytes).hexdigest(), paletteSize=len(palette), candidates=candidates))
    native.unlink()
    print(family, [(c['aa'], c['mode'], c['differentPixels']) for c in candidates[:5]], flush=True)
destination = base / 'aa-coverage-candidates-r1.json'
assert not destination.exists()
destination.write_text(json.dumps(dict(cases=records, productCompilerOpened=False, usbAccessed=False, E=False), indent=2)+'\n', encoding='utf-8')
