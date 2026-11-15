"""Export narrow original-firmware ARM ranges without trusting Ghidra function bodies.

//20261115-0 add
"""
from pathlib import Path
import hashlib
import json
import struct
import capstone

ROOT = Path(__file__).resolve().parents[1]
SOURCE = Path('G:/01_Professional/触摸屏/金玺/KinSealStudio_Setup_V5.1/Installed/Firmware/AMT070W124-L800X480R.bin')
EXPECTED = '74d510f150dc8edadf8e967bdb3c00ca5790ca68b9da6f906125a5234a485c24'
BASE = 0x8001ffe0


def main():
    """Function Name   : main
    Description     : Verify immutable source and export bounded code/literal evidence.
    Input Parameters: None (fixed source and virtual ranges below).
    Return Value    : None; writes raw-evidence.json.
    Date            : 2026.11.15
    """
    source = SOURCE.read_bytes()
    assert hashlib.sha256(source).hexdigest() == EXPECTED
    decoder = capstone.Cs(capstone.CS_ARCH_ARM, capstone.CS_MODE_ARM | capstone.CS_MODE_LITTLE_ENDIAN)
    ranges = [
        ('release_dispatch_and_literals', 0x8007f430, 0x8007f600),
        ('release_list_consumer', 0x8003a670, 0x8003a884),
        ('local_bit_write_helper', 0x8007bdb0, 0x8007c0dc),
        ('release_flag_dispatch', 0x8005fd28, 0x8005fe20),
        ('release_flag_consume_and_reset', 0x800606b0, 0x80060810),
        ('notify_type_dispatch_and_cases', 0x80060e5c, 0x800610c4),
        ('touch_word_compare_dispatch', 0x80057258, 0x80057428),
        ('double_subtract_tail_to_add', 0x800a3efc, 0x800a4100),
        ('double_add_body_and_normalization', 0x800a32ac, 0x800a3400),
        ('double_compare_flags_forward', 0x800a3984, 0x800a3a00),
        ('double_compare_flags_reverse', 0x800a3c58, 0x800a3ce4),
    ]
    output = {'provider': 'Direct Python Capstone disassembly; no REA evidence ID',
              'source': str(SOURCE), 'sourceSha256': EXPECTED,
              'capstoneVersion': capstone.__version__,
              'limitations': 'Literal pools are also decoded as instructions; raw words are authoritative. Static installation image, not executed or verified on current screen.',
              'ranges': []}
    for name, start, end in ranges:
        data = source[start - BASE:end - BASE]
        output['ranges'].append({
            'name': name, 'start': hex(start), 'endExclusive': hex(end),
            'rawWords': [{'address': hex(start + offset), 'word': hex(struct.unpack_from('<I', data, offset)[0])}
                         for offset in range(0, len(data), 4)],
            'assembly': [f'{insn.address:08x} {insn.bytes.hex()} {insn.mnemonic} {insn.op_str}'
                         for offset in range(0, len(data), 4)
                         for insn in decoder.disasm(data[offset:offset + 4], start + offset)],
        })
    output['u16ThresholdMathCheck'] = {
        'domain': 'all 65536 integers x in [0,65535]',
        'formula': '(x * 0xCCCD) >> 20 == x // 20',
        'allPass': all((x * 0xcccd) >> 20 == x // 20 for x in range(65536)),
        'limitation': 'Pure arithmetic verification only; does not execute firmware or establish milliseconds per OS tick.',
    }
    (ROOT / 'raw-evidence.json').write_text(json.dumps(output, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(json.dumps({'ranges': len(ranges), 'thresholdMathAllPass': output['u16ThresholdMathCheck']['allPass']}))


if __name__ == '__main__':
    main()
