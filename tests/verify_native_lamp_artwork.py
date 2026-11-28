"""Compare complete native lamp streams, indices, and per-frame firmware-decoded RGB565 pixels."""
# 20261128-20 add
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import tempfile

from verify_amz_button_frame_rules import run
from verify_amz_firmware_page_images import firmware_frame
from verify_native_drawing_drafts import ROOT, REVISION, git_blob
from verify_native_lamp_monitor_words import FOLDER, ROUTES, SAMPLES

FILES = ("ControlPixel.dat", "PicLenPara.dat")
FIRMWARE_SHA = "7f625b7a967c1ecc0ff5db1fba4d6ed68cb1d9474216e87009395f1d53a88db5"


def sha(data):
    """Identify a complete source, reference file or compiler under test."""
    return hashlib.sha256(data).hexdigest()


def frames(firmware, files, frame_bytes):
    """Validate the 12-byte indices and decode the effective LZO lengths with the AMZ reader."""
    packed, table = (files[name] for name in FILES)
    offset = 0
    pixels = []
    for codec, stored, used in struct.iter_unpack("<III", table[2400:]):
        if codec or not used or stored != (used + 31) & ~31 or offset + stored > len(packed):
            raise ValueError("invalid native frame bounds")
        pixels.append(firmware_frame(firmware, packed[offset:offset + used], frame_bytes))
        offset += stored
    if offset != len(packed) or struct.unpack_from("<I", table, 1600)[0] != len(pixels):
        raise ValueError("invalid original single-page frame count or final offset")
    return pixels


def compare(exe, route, source, work, originals, native_frames, firmware, frame_bytes):
    """Use the production compiler entry, including its native page-background padding bytes."""
    with tempfile.TemporaryDirectory(dir=work) as temporary:
        output = Path(temporary) / "parts"
        run(exe, work, route, source, output)
        files = {name: (Path(output) / name).read_bytes() for name in FILES}
    output_frames = frames(firmware, files, frame_bytes)
    if len(native_frames) != len(output_frames):
        raise ValueError("the emitted state count changed")
    differences = []
    for original, generated in zip(native_frames, output_frames):
        differences.append(sum(original[i:i+2] != generated[i:i+2] for i in range(0, frame_bytes, 2)))
    return dict(equalFiles={name: files[name] == originals[name] for name in FILES},
                sha256={name: sha(files[name]) for name in FILES}, differentPixelsByFrame=differences)


def main():
    """Check the original renderer and both compiler routes on a private Windows desktop."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--before-exe", type=Path)
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    exe = args.exe.resolve()
    previous = args.before_exe.resolve() if args.before_exe else None
    firmware = (ROOT / "bin/AMZ070W01.bin").read_bytes()
    if sha(firmware) != FIRMWARE_SHA or os.environ.get("QT_QPA_PLATFORM") != "windows":
        raise ValueError("requires the pinned firmware and Windows font backend on a private desktop")
    rows = []
    with tempfile.TemporaryDirectory(prefix="native-lamp-art-", dir=ROOT / "build") as tmp:
        work = Path(tmp)
        for name in SAMPLES:
            directory = FOLDER + name
            source = work / (name + ".sm")
            source.write_bytes(git_blob(directory, "input-native.sm"))
            record = git_blob(directory, "ControlPara.dat")
            width, height = struct.unpack_from("<HH", record, 8)
            frame_bytes = width * height * 2
            native = {member: git_blob(directory, member) for member in FILES}
            native_frames = frames(firmware, native, frame_bytes)
            row = dict(name=name, dimensions=[width, height], sourceSha256=sha(source.read_bytes()),
                       nativeSha256={member: sha(native[member]) for member in FILES})
            for route in ROUTES:
                row[route] = compare(exe, route, source, work, native, native_frames, firmware, frame_bytes)
            if row[ROUTES[0]] != row[ROUTES[1]]:
                raise ValueError(name + ": USB and media artwork differ")
            result = row[ROUTES[0]]
            if name != "product-shared-lamp-new-native" and not all(result["equalFiles"].values()):
                raise ValueError(name + ": complete original pixel or index file differs")
            if (not result["equalFiles"]["PicLenPara.dat"] or sum(result["differentPixelsByFrame"]) > 2):
                raise ValueError(name + ": original index or previously attained pixel accuracy regressed")
            if previous:
                row["previous"] = compare(previous, ROUTES[0], source, work, native, native_frames, firmware, frame_bytes)
                if sum(result["differentPixelsByFrame"]) >= sum(row["previous"]["differentPixelsByFrame"]):
                    raise ValueError(name + ": did not improve RGB565 equality")
            rows.append(row)
    report = dict(referenceRevision=REVISION, firmwareSha256=FIRMWARE_SHA, exeSha256=sha(exe.read_bytes()),
                  frames=sum(len(row[ROUTES[0]]["differentPixelsByFrame"]) for row in rows),
                  matchingFrames=sum(count == 0 for row in rows for count in row[ROUTES[0]]["differentPixelsByFrame"]),
                  matchedFilePairs=sum(all(row[ROUTES[0]]["equalFiles"].values()) for row in rows), cases=rows)
    if previous:
        report["previousExeSha256"] = sha(previous.read_bytes())
    if args.receipt:
        args.receipt.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf8")
    print(f'{report["matchedFilePairs"]}/{len(rows)} complete file pairs, '
          f'{report["matchingFrames"]}/{report["frames"]} RGB565 frames match per route.')


if __name__ == "__main__":
    main()
