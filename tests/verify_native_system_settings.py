"""Exercise the verified V122 system settings through the ordinary device and offline media compilers."""
# 20261128-10 add
import argparse
import hashlib
import json
from pathlib import Path
import tempfile

from verify_native_drawing_drafts import ROOT, REVISION, git_blob, run


BASE = "docs/stage2-evidence/native/compiled/final-system-"
OFFSETS = (3, 4, 8, 22)


def source_fields(folder):
    """Load a typed source map only when its recorded hash matches the independently archived SM."""
    raw = git_blob(folder, "input-native.sm")
    grammar = json.loads(git_blob(folder, "input-native-fields.json"))
    if grammar["sha256"] != hashlib.sha256(raw).hexdigest():
        raise ValueError("field map does not belong to the archived native source")
    return raw, {f["path"]: (f["start"], f["end"]) for f in grammar["fields"]}


def source_values(raw, fields):
    """Read source inputs separately from the generated device bytes."""
    return [int.from_bytes(raw[slice(*fields[key])], "big") for key in
            ("config.head.5", "config.head.6", "config.head.8", "config.head.14", "config.flags.1")]


def check_reference(exe, work, folder, mode):
    """Check source-to-DAT transfer for the four native byte stores and the editor-only hexadecimal flag."""
    source, fields = source_fields(folder)
    sm = work / "original.sm"
    sm.write_bytes(source)
    out = work / "compiled"
    run(exe, work, mode, sm, out)
    original = git_blob(folder, "SysPara.dat")
    generated = (out / "SysPara.dat").read_bytes()
    saver, blink, sound, level, hex_word = source_values(source, fields)
    expected = [saver, blink, sound ^ 1, level]
    if len(generated) != 4998 or any(expected[i] != generated[off] or expected[i] != original[off]
                                      for i, off in enumerate(OFFSETS)):
        raise ValueError("system options differ for " + folder)
    return dict(case=folder, mode=mode, sourceSha256=hashlib.sha256(source).hexdigest(), atOffsets=expected,
                hexWord=hex_word, nativeSysSha256=hashlib.sha256(original).hexdigest(),
                generatedSysSha256=hashlib.sha256(generated).hexdigest(), wholeSysExact=generated == original)


def controlled(exe, work):
    """Use single-field rejections and a combined valid change to catch unsafe widening or default substitution."""
    folder = BASE + "baseline-repeat-fresh"
    source, fields = source_fields(folder)
    sm = work / "input.sm"
    invalid = (("config.head.5", 61), ("config.head.6", 0), ("config.head.8", 2), ("config.head.14", 9),
               ("config.flags.1", 2), ("config.48.6", 12345), ("object[0].common.visibility.0", 1),
               ("object[0].common.40.1", 1))
    for field, value in invalid:
        a, b = fields[field]
        sm.write_bytes(source[:a] + value.to_bytes(b-a, "big") + source[b:])
        dest = work / "invalid"
        result = json.loads(run(exe, work, "--compile-project", sm, dest, success=False))
        if result.get("ok") is not False or (dest.is_dir() and list(dest.iterdir())):
            raise ValueError("unexpected output for " + field)
    limits = {"config.head.5": 60, "config.head.6": 50, "config.head.8": 0, "config.head.14": 8, "config.flags.1": 1}
    edited = bytearray(source)
    for key, value in limits.items():
        a, b = fields[key]
        edited[a:b] = value.to_bytes(b-a, "big")
    sm.write_bytes(source)
    run(exe, work, "--compile-project", sm, work / "base")
    expected = bytearray((work / "base/SysPara.dat").read_bytes())
    for off, value in zip(OFFSETS, (60, 50, 1, 8)):
        expected[off] = value
    sm.write_bytes(edited)
    run(exe, work, "--compile-project", sm, work / "limits")
    if (work / "limits/SysPara.dat").read_bytes() != expected:
        raise ValueError("valid settings did not change exactly the four output bytes")
    return dict(rejectedFields=[k for k, _ in invalid], limitCombinationChangedOnlyKnownBytes=True)


def main():
    """Verify original baseline, repeats and restore, then exercise validation boundaries through the public CLI."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    parser.add_argument("--scratch", type=Path, default=ROOT / "build")
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    exe = args.exe.resolve()
    variants = ["baseline-r1", "baseline-repeat", "baseline-restored"]
    variants += [base + "-" + rep for base in ("screensaver2", "flick11", "touch-off", "user2", "hex-on")
                 for rep in ("r1", "repeat")]
    rows = []
    for name in variants:
        for mode in ("--compile-project", "--compile-native-media-parts"):
            with tempfile.TemporaryDirectory(dir=args.scratch) as folder:
                rows.append(check_reference(exe, Path(folder).resolve(), BASE + name + "-fresh", mode))
    with tempfile.TemporaryDirectory(dir=args.scratch) as folder:
        controls = controlled(exe, Path(folder).resolve())
    report = dict(sourceRevision=REVISION, exeSha256=hashlib.sha256(exe.read_bytes()).hexdigest(),
                  originalRuns=len(variants), compiledOutputs=len(rows), offsets=list(OFFSETS),
                  comparedOptionBytes=len(rows)*len(OFFSETS), controlled=controls, rows=rows)
    if args.receipt:
        args.receipt.write_text(json.dumps(report, ensure_ascii=False, indent=1) + "\n", encoding="utf8")
    print(json.dumps({k: v for k, v in report.items() if k != "rows"}))


if __name__ == "__main__":
    main()
