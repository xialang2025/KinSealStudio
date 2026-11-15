"""Reconstruct the observed ARM scatter-loading layout without running firmware.

Input: the explicit installed firmware; Output: owned memory image and manifest.
Date: 2026.11.15. #20261115-0 add
"""
from pathlib import Path
import hashlib
import json
import struct

ROOT = Path(__file__).resolve().parents[1]
SOURCE = Path('G:/01_Professional/触摸屏/金玺/KinSealStudio_Setup_V5.1/Installed/Firmware/AMT070W124-L800X480R.bin')
EXPECTED = '74d510f150dc8edadf8e967bdb3c00ca5790ca68b9da6f906125a5234a485c24'
BASE = 0x8001ffe0

def main():
    """Function Name   : main
    Description     : Verify source and reproduce the observed scatter data loop.
    Input Parameters: None; fixed immutable installation firmware path.
    Return Value    : None; writes private analysis memory image and manifest.
    Date            : 2026.11.15
    """
    source = SOURCE.read_bytes()
    assert hashlib.sha256(source).hexdigest() == EXPECTED
    assert struct.unpack_from('<I', source, 0x10)[0] == len(source) == 0xdd800
    entries = [struct.unpack_from('<4I', source, off) for off in range(0xdb2ac, 0xdb2dc, 16)]
    assert entries == [(0x800fb2e8, 0x8010085c, 0x2360, 0x80020128),
                       (0x8010085c, 0x800fb2e8, 0x5574, 0x800200c8),
                       (0x800fd648, 0x8010085c, 0x14ea78, 0x80020150)]
    src = entries[0][0] - BASE
    cur = src
    end = src + entries[0][2]
    decoded = bytearray()
    target_length = entries[1][2]
    tokens = 0
    # Translate the Thumb loop at file 0xf0..0x144, including its inclusive copy count.
    while len(decoded) < target_length:
        flag = source[cur]
        cur += 1
        literal, run = flag & 3, flag >> 4
        if literal == 0:
            literal = source[cur]
            cur += 1
        if run == 0:
            run = source[cur]
            cur += 1
        assert literal >= 1
        decoded.extend(source[cur:cur + literal - 1])
        cur += literal - 1
        if run:
            low = source[cur]
            cur += 1
            high = (flag >> 2) & 3
            if high == 3:
                high = source[cur]
                cur += 1
            distance = low + (high << 8)
            assert 0 < distance <= len(decoded)
            for _ in range(run + 2):
                decoded.append(decoded[-distance])
        tokens += 1
        assert len(decoded) <= target_length and cur <= end
    assert cur - src == 0x235f and source[cur:end] == b'\x00'
    assert hashlib.sha256(decoded).hexdigest() == '2880fba2e52e80abc614fc84926459f55f2e833557307ddd866294b602e4fcd0'
    # This is an analysis artifact, not a flashable firmware: initialized data replaces compression.
    memory = bytearray(source[:src]) + decoded + bytes(entries[2][2])
    assert BASE + len(memory) == 0x8024f2d4
    output = ROOT / 'temp' / 'firmware-memory.bin'
    output.write_bytes(memory)
    report = {
        'provider': 'Direct Ghidra headless fallback; not an REA evidence record',
        'source': str(SOURCE), 'sourceSize': len(source), 'sourceSha256': EXPECTED,
        'mapFormula': 'virtualAddress = fileOffset + 0x8001ffe0 before initialized RW data',
        'entryFileOffset': '0x20', 'entryVA': '0x80020000',
        'architecture': 'Observed little-endian ARM and Thumb interworking; CPU model unknown',
        'basis': ['entry LDR literal 0x80022bb8 maps to exception vectors file0x2bd8',
                  'vector branch returns to VA0x80020000; handlers map to their code stubs',
                  'scatter table and destination function addresses resolve consistently',
                  'reconstructed RW pointers resolve ControlPara.dat strings'],
        'scatterRecords': [[hex(x) for x in row] for row in entries],
        'compressedSourceFileOffset': hex(src), 'consumed': hex(cur-src), 'tokens': tokens,
        'initializedDataVA': '0x800fb2e8', 'initializedDataLength': hex(len(decoded)),
        'initializedDataSha256': hashlib.sha256(decoded).hexdigest(),
        'bssStart': '0x8010085c', 'bssLength': '0x14ea78',
        'memoryImage': str(output), 'memoryImageSize': len(memory),
        'memoryImageSha256': hashlib.sha256(memory).hexdigest(),
        'limitations': ['Not executed; source is an installation artifact, not verified device firmware',
                        'Link-time virtual mapping established; bootloader physical load procedure unknown',
                        'Header checksum/signature semantics not established',
                        'Memory image contains reconstructed data and synthesized zero BSS, is not flashable']}
    (ROOT / 'memory-map.json').write_text(json.dumps(report, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
    print(json.dumps({'sourceSha256':EXPECTED,'memorySize':len(memory),'decodedSize':len(decoded),'manifest':str(ROOT/'memory-map.json')}))

if __name__ == '__main__':
    main()
