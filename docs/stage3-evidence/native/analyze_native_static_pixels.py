"""#20261110-8 add: Inspect independent native C records and decoded LZO sprites."""
from pathlib import Path
from collections import Counter
import argparse
import hashlib
import json
import struct
import subprocess
from PIL import Image


def main():
    """Decode captured buffers using a standalone LZO decoder, preserving odd rows."""
    output = Path(__file__).parent
    parser = argparse.ArgumentParser()
    parser.add_argument('--decoder', type=Path, default=output / 'tools/lzo_decode.exe')
    args = parser.parse_args()
    decoder = args.decoder.resolve()
    assert decoder.is_file(), 'An explicit standalone LZO decoder is required'
    report = {'method': 'Independent native compiler output; standalone MIT LZO decoder only, no product renderer/device compiler', 'decoder': {'path': str(decoder), 'sha256': hashlib.sha256(decoder.read_bytes()).hexdigest()}, 'variants': []}
    for variant in ('base-only', 'transparent-top', 'opacity128-top'):
        folder = output / 'compiled' / ('static-odd2-' + variant)
        control = (folder / 'ControlPara.dat').read_bytes()
        pixels = (folder / 'ControlPixel.dat').read_bytes()
        lengths = (folder / 'PicLenPara.dat').read_bytes()
        record = {'name': variant, 'controls': [], 'frames': [], 'hashes': {n: hashlib.sha256((folder / n).read_bytes()).hexdigest() for n in ('ControlPara.dat', 'ControlPixel.dat', 'PicLenPara.dat')}}
        image_index = 0
        offset = 0
        for index in range(len(control) // 640):
            raw = control[index * 640:(index + 1) * 640]
            x, y, width, height = struct.unpack_from('<HHHH', raw, 4)
            record['controls'].append({'index': index, 'typeAndFlags': raw[:4].hex(), 'logicalRect': [x, y, width, height], 'rawPixelOffset': struct.unpack_from('<I', raw, 12)[0], 'rawPixelLength': struct.unpack_from('<I', raw, 16)[0], 'recordHex': raw.hex()})
            for language in range(2):
                entry = struct.unpack_from('<III', lengths, 2400 + image_index * 12)
                reserved, padded, size = entry
                compressed = folder / ('frame-' + str(image_index) + '.lzo')
                decoded = compressed.with_suffix('.rgb565')
                compressed.write_bytes(pixels[offset:offset + size])
                run = subprocess.run([str(decoder), str(compressed), str(decoded), str(width * height * 2)], capture_output=True, creationflags=0x08000000)
                assert run.returncode == 0, (variant, image_index, run.returncode)
                image_bytes = decoded.read_bytes()
                words = struct.unpack('<' + 'H' * (width * height), image_bytes)
                rgb = bytes(c for w in words for c in (((w >> 11) & 31) * 255 // 31, ((w >> 5) & 63) * 255 // 63, (w & 31) * 255 // 31))
                image = Image.frombytes('RGB', (width, height), rgb)
                image.save(compressed.with_suffix('.png'))
                record['frames'].append({'index': image_index, 'control': index, 'languageIndex': language, 'offset': offset, 'paddedLength': padded, 'compressedLength': size, 'rawLength': len(image_bytes), 'rawSha256': hashlib.sha256(image_bytes).hexdigest(), 'colors': [{'rgb565': hex(v), 'count': count} for v, count in Counter(words).most_common(12)], 'firstPixel': hex(words[0]), 'lastPixel': hex(words[-1]), 'topLeft4x4': [hex(words[y * width + x]) for y in range(min(4, height)) for x in range(min(4, width))]})
                image_index += 1
                offset += padded
        assert offset == len(pixels)
        report['variants'].append(record)
    top = report['variants'][1]['frames'][2]
    alpha = report['variants'][2]['frames'][2]
    report['observations'] = {'transparentAndOpacity128TopSpritesEqual': top['rawSha256'] == alpha['rawSha256'], 'baseFirstFrameAcrossAllVariantsEqual': len({v['frames'][0]['rawSha256'] for v in report['variants']}) == 1, 'noNativeDownloadButtonsInvoked': True, 'staticOnly': True, 'limitations': 'No claim about dynamic overlapping controls or runtime display follows from these compiler buffers.'}
    (output / 'native-static-pixels.json').write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf8')
    print(json.dumps({'observations': report['observations'], 'firstFrames': [{k: f[k] for k in ('index', 'rawLength', 'compressedLength', 'firstPixel', 'colors')} for v in report['variants'] for f in v['frames'][::2]]}))


if __name__ == '__main__':
    main()
