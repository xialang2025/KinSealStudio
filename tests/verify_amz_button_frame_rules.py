"""Compare Lite switch image layouts with the V5.1 frame-selection loop and AMZ070W01 reader."""
# 20261128-16 add
import argparse
from copy import deepcopy
import hashlib
import itertools
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile

from verify_native_drawing_drafts import ROOT

NATIVE_SHA = "305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c"
FIRMWARE_SHA = "7f625b7a967c1ecc0ff5db1fba4d6ed68cb1d9474216e87009395f1d53a88db5"
U32 = struct.Struct("<I")


def native_frames(pe, subtype, shared, languages, two, flash0, flash1):
    """Run the original x86 loop and capture its language, state and flash at each bitmap append."""
    from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
    from unicorn.x86_const import UC_X86_REG_EBP, UC_X86_REG_ESP, UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EIP
    cpu = Uc(UC_ARCH_X86, UC_MODE_32)
    frame, obj, appearance = 0x1e000, 0x12000, 0x14000
    cpu.mem_map(0x10000, 0x10000)
    cpu.mem_map(0x62c000, 0x2000)
    cpu.mem_write(0x62c000, pe.get_data(0x62c000 - pe.OPTIONAL_HEADER.ImageBase, 0x2000))
    cpu.mem_map(0xb90000, 0x1000)
    # Only graphics selection and bitmap emission are substituted. All loop conditions and counters execute intact.
    for call in (0xb9048c, 0xb904dc, 0xb9052c):
        cpu.mem_write(call, b"\xc3")
    cpu.reg_write(UC_X86_REG_EBP, frame)
    cpu.reg_write(UC_X86_REG_ESP, frame - 0x180)
    cpu.reg_write(UC_X86_REG_EBX, 40)
    cpu.reg_write(UC_X86_REG_ESI, 24)
    for addr, val in ((frame - 0x58, obj), (frame - 0x6c, languages), (obj + 0x128, subtype),
                      (obj + 0x244, 1 if two else 2), (obj + 0x294, appearance), (appearance, 1),
                      (appearance + 8, 2 if two else 1), (appearance + 0x18, flash0), (appearance + 0x38, flash1)):
        cpu.mem_write(addr, U32.pack(val))
    cpu.mem_write(obj + 0x22f, bytes([shared]))
    cpu.mem_write(obj + 0x230, bytes([two]))
    sequence = []
    selected = []

    def graphics(machine, pc, size, context):
        """Record the state requested before each emission, without rendering or storing target images."""
        if pc in (0xb9048c, 0xb904dc):
            stack = machine.reg_read(UC_X86_REG_ESP)
            state, lang = struct.unpack("<II", machine.mem_read(stack + 12, 8))
            selected[:] = [lang, state, int(pc == 0xb9048c)]
        elif pc == 0xb9052c:
            if len(selected) != 3:
                raise ValueError("bitmap emission lacks its preceding state selection")
            sequence.append(tuple(selected))
            selected.clear()

    cpu.hook_add(UC_HOOK_CODE, graphics, begin=0xb9048c, end=0xb9052c)
    cpu.emu_start(0x62c2f9, 0x62c4dc, count=80000)
    if cpu.reg_read(UC_X86_REG_EIP) != 0x62c4dc:
        raise ValueError("native frame loop failed to terminate")
    return sequence


def firmware_image_offsets(firmware, record, languages, selections):
    """Execute the original ARM RGB565 byte-offset calculation, stopping before its GUI call."""
    from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM
    from unicorn.arm_const import UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_SP, UC_ARM_REG_PC
    cpu = Uc(UC_ARCH_ARM, UC_MODE_ARM)
    cpu.mem_map(0x80020000, 0x200000)
    cpu.mem_write(0x80020000, firmware[0x20:])
    cpu.mem_write(0x8019c5e4, record[:0x1c0])
    cpu.mem_write(0x8019c5e4 + 0x284, record[:1])
    offsets = []
    for language, frame in selections:
        cpu.mem_write(0x80158352, struct.pack("<HH", language, languages))
        cpu.reg_write(UC_ARM_REG_R0, 0)
        cpu.reg_write(UC_ARM_REG_R1, frame)
        cpu.reg_write(UC_ARM_REG_R2, 0)
        cpu.reg_write(UC_ARM_REG_SP, 0x80200000)
        cpu.emu_start(0x80047640, 0x8004773c, count=1000)
        if cpu.reg_read(UC_ARM_REG_PC) != 0x8004773c:
            raise ValueError("firmware offset loop did not reach its bitmap output")
        offsets.append(cpu.reg_read(UC_ARM_REG_R1))
    return offsets


def run(exe, path, *args):
    """Compile or export one ordinary project in the caller's offscreen or private desktop environment."""
    result = subprocess.run([str(exe), *map(str, args)], cwd=path, timeout=90, capture_output=True,
                            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
                            env=dict(os.environ, TEMP=str(path), TMP=str(path), TMPDIR=str(path)))
    if result.returncode:
        raise ValueError(result.stdout.decode("utf8", "replace") + result.stderr.decode("utf8", "replace"))


def project(languages, mixed):
    """Combine separated bit and word buttons with independently chosen state, language and flashing settings."""
    paths = ("bit-button/双语位按钮.ksp", "word-button/双语按下效果与闪烁.ksp")
    models = [json.loads((ROOT / "samples" / path).read_bytes()) for path in paths]
    base = deepcopy(models[1])
    names = ["中文", "English"] + [f"Language{i}" for i in range(2, languages)]
    names = names[:languages]
    base["languages"], base["activeLanguage"] = names, names[0]
    objects, specifications = [], []
    for sub, two, shared in itertools.product((0, 1), repeat=3):
        obj = deepcopy(models[sub]["pages"][0]["objects"][0])
        index = len(objects)
        obj.update(id=f"button-{index}", name=f"Button {index}", x=30 + index % 4 * 150, y=30 + index // 4 * 90,
                   width=90, height=40, layer=index)
        button = obj["bitButton"]
        button.update(indicator=not sub, dualMonitor=bool(two) and not sub)
        if sub:
            obj["wordButton"]["pressedEffect"] = bool(two)
        for state, row in enumerate(button["states"]):
            flag = shared if not mixed or state == 0 else 1 - shared
            row.update(sameLanguage=bool(flag), sharedText=f"ROW{state}", useImage=False, flash=bool(state == sub))
            row["text"]["texts"] = {lang: (row["sharedText"] if i == 0 else f"L{i}-{state}") for i, lang in enumerate(names)}
        objects.append(obj)
        shared_states = tuple(state["sameLanguage"] for state in button["states"])
        specifications.append((sub, shared and (not mixed or not two), two, int(sub == 0), int(sub == 1), shared_states))
    base["pages"][0]["objects"] = objects
    return base, specifications


def inspect(folder, specifications, langs, pe, fw):
    """Check the produced controls and packed image bounds against the original compiler and screen instructions."""
    table = (folder / "ControlPara.dat").read_bytes()
    indices = (folder / "PicLenPara.dat").read_bytes()[2400:]
    pixels = (folder / "ControlPixel.dat").read_bytes()
    if len(table) != len(specifications) * 640:
        raise ValueError("incorrect number of controls")
    raw_offset = frame_index = packed_offset = observations = 0
    result = []
    for idx, (sub, shared, two, flash0, flash1, shared_states) in enumerate(specifications):
        rec = table[idx * 640:(idx + 1) * 640]
        seq = native_frames(pe, sub, shared, langs, two, flash0, flash1)
        w, h, offset, raw = struct.unpack_from("<HHII", rec, 8)
        frame_size = w * h * 2
        if not (rec[0] == 10 and struct.unpack_from("<H", rec, 0x106)[0] == sub and rec[0x1bc] == shared
                and offset == raw_offset and raw == frame_size * len(seq)):
            raise ValueError(f"control {idx}, subtype {sub}, shared {shared}, languages {langs}: "
                             f"observed flag={rec[0x1bc]}, bytes={raw}, expected flag={int(shared)}, bytes={frame_size * len(seq)}")
        raw_offset += raw
        language_slots = 1 if shared else langs
        per_language = len(seq) // language_slots
        selections = list(itertools.product(range(langs), range(per_language)))
        offsets = firmware_image_offsets(fw, rec, langs, selections)
        for (lang, state), read in zip(selections, offsets):
            expected_index = (0 if shared else lang * per_language) + state
            if read != offset + expected_index * frame_size:
                raise ValueError("firmware selected a different bitmap byte offset")
        observations += len(selections)
        frames = []
        for lang, state, hidden in seq:
            reserved, aligned, size = struct.unpack_from("<III", indices, frame_index * 12)
            if reserved or size < 1 or aligned != ((size + 31) & ~31) or packed_offset + aligned > len(pixels):
                raise ValueError("invalid compressed frame index")
            frames.append(hashlib.sha256(pixels[packed_offset:packed_offset + size]).hexdigest())
            frame_index += 1
            packed_offset += aligned
        if os.environ.get("QT_QPA_PLATFORM") == "windows":
            # Real Windows fonts distinguish the eight controlled captions. The offscreen platform may omit glyphs.
            for state in range(1 + two):
                visible = {digest for (_, index, hidden), digest in zip(seq, frames) if index == state and not hidden}
                if len(visible) != (1 if shared_states[state] else langs):
                    raise ValueError(f"control {idx}, state {state}: expected localized captions in visible RGB565 frames")
        result.append(dict(subtype=sub, shared=bool(shared), states=1 + two,
                           nativeEmissionSequence=seq, compressedFrameSha256=frames))
    if frame_index * 12 != len(indices) or packed_offset != len(pixels):
        raise ValueError("orphaned byte range or frame index")
    return dict(records=len(specifications), frames=frame_index, firmwareOffsetChecks=observations, objects=result)


def main():
    """Re-run the firmware and original authoring-machine opcodes for every verified switch layout."""
    import pefile
    import unicorn
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--native-exe", type=Path, required=True)
    parser.add_argument("--firmware", type=Path, default=ROOT / "bin/AMZ070W01.bin")
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    exe = args.exe.resolve()
    native = args.native_exe.read_bytes()
    firmware = args.firmware.read_bytes()
    if hashlib.sha256(native).hexdigest() != NATIVE_SHA or hashlib.sha256(firmware).hexdigest() != FIRMWARE_SHA:
        raise ValueError("unexpected program revision")
    pe = pefile.PE(data=native, fast_load=True)
    native_cases = 0
    for langs in (1, 3, 8):
        for sub, shared, two, flash0, flash1 in itertools.product((0, 1), repeat=5):
            seq = native_frames(pe, sub, shared, langs, two, flash0, flash1)
            states = ([0, 1] if sub else [1, 0]) if two else [0]
            expected = [(lang, state, hidden) for lang in range(1 if shared else langs)
                        for state in states for hidden in range(1 + (flash0, flash1)[state])]
            if seq != expected:
                raise ValueError(f"original compiler image order differs: {seq} != {expected}")
            native_cases += 1
    rows = []
    with tempfile.TemporaryDirectory(prefix="amz-button-frames-", dir=ROOT / "build") as temp:
        directory = Path(temp)
        for langs, mixed in itertools.product((1, 3, 8), (False, True)):
            model, spec = project(langs, mixed)
            name = f"language-{langs}-mixed-{int(mixed)}"
            ksp = directory / (name + ".ksp")
            ksp.write_text(json.dumps(model, ensure_ascii=False), encoding="utf8")
            out = directory / (name + "-direct")
            run(exe, directory, "--compile-project", ksp, out)
            info = inspect(out, spec, langs, pe, firmware)
            item = dict(languages=langs, mixed=mixed, direct=info)
            if not mixed:
                sm = directory / (name + ".sm")
                run(exe, directory, "--export-native-project", ksp, sm)
                for cmd in ("--compile-project", "--compile-native-media-parts"):
                    dest = directory / (name + cmd)
                    run(exe, directory, cmd, sm, dest)
                    actual = inspect(dest, spec, langs, pe, firmware)
                    if actual != info:
                        raise ValueError("source/compiler route changed a frame")
                    item[cmd] = True
            rows.append(item)
    report = dict(exeSha256=hashlib.sha256(exe.read_bytes()).hexdigest(), nativeSha256=NATIVE_SHA,
                  firmwareSha256=FIRMWARE_SHA, emulator=unicorn.__version__, nativeSelectorCases=native_cases,
                  projectCases=rows, localizedCaptionHashesChecked=os.environ.get("QT_QPA_PLATFORM") == "windows")
    if args.receipt:
        args.receipt.write_bytes((json.dumps(report, ensure_ascii=False, indent=1) + "\n").encode())
    print("Native selector cases:", native_cases, "project shapes:", len(rows), "records through direct entry:",
          sum(row["direct"]["records"] for row in rows))


if __name__ == "__main__":
    main()
