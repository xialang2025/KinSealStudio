"""Verify V122 media picture-library pruning against unmodified V5.1 output."""
# 20261128-13 add
import argparse
import hashlib
import json
from pathlib import Path
import tempfile
import zlib

from verify_native_drawing_drafts import git_blob, run


def fields(exe, work, raw):
    """Read typed spans in the file that is about to be edited or checked."""
    saved = work / "inspect.sm"
    saved.write_bytes(raw)
    rows = json.loads(run(exe, work, "--inspect-native-source", saved))["fields"]
    return {row["path"]: slice(row["start"], row["end"]) for row in rows}


def gallery_controls(exe, work, reference):
    """Verify a variable-length replacement, an earlier deleted row, and a live reference outside the image."""
    cases = []
    for name in ("picture-multiple-bit", "picture-distinct-red-green"):
        raw = git_blob(reference + name, "input-native.sm")
        fs = fields(exe, work, raw)
        data = [raw[fs[f"config.284[{i}].0"].start:fs[f"config.284[{i}].5"].stop] for i in range(2)]
        span = slice(fs["config.284.count"].start, fs["config.284[1].5"].stop)
        if name == "picture-distinct-red-green":
            cases.append(("both row bindings in native ID order", raw, [data[1], data[0]]))
            cases.append(("source gallery already sorted", raw[:span.start+4] + data[1] + data[0] + raw[span.stop:],
                          [data[1], data[0]]))
            # Give the red resource a key that sorts first despite its longer wire length and later filename.
            red, green = (raw[fs[f"config.284[{i}].0"]] for i in range(2))
            keys = [s.encode("utf-16-be") for s in ("Ared-longer", "z")]
            new_red, new_green = (len(key).to_bytes(4, "big") + key for key in keys)
            renamed = raw.replace(red, new_red).replace(green, new_green)
            cases.append(("renamed IDs take precedence over filenames and length prefixes", renamed,
                          [data[0].replace(red, new_red), data[1].replace(green, new_green)]))
            continue
        green = raw[fs["config.284[1].0"]]
        # An equal QString outside this object's image state rows also stops pruning.
        outside = fs["trailer.settings.1"]
        cases.append(("outside reference", raw[:outside.start] + green + raw[outside.stop:], data))
        # Append a second real picture object whose preview and state rows reference the other PNG.
        obj = [bounds for field, bounds in fs.items() if field.startswith("object[0].")]
        start, end = min(s.start for s in obj), max(s.stop for s in obj)
        second = raw[start:end].replace(raw[fs["config.284[0].0"]], green)
        total = fs["page.objects.count"]
        mixed = raw[:total.start] + (2).to_bytes(4, "big") + raw[total.stop:end] + second + raw[end:]
        inspected = fields(exe, work, mixed)
        if mixed[inspected["object[1].image.row[0].0"]] != green:
            raise ValueError("second picture fixture does not reference the expected PNG")
        cases.append(("second picture uses green", mixed, data))
        cases.append(("unused first record", raw[:span.start+4] + data[1] + data[0] + raw[span.stop:], [data[0]]))
    for label, source, records in cases:
        sm, archive = work / "test.sm", work / "test.knr"
        sm.write_bytes(source)
        archive.unlink(missing_ok=True)
        run(exe, work, "--build-native-media-snapshot", sm, archive, "--native-work-directory", "Z:/长目录/Archive")
        snapshot = zlib.decompress(archive.read_bytes()[4:])
        fs = fields(exe, work, snapshot)
        actual_count = int.from_bytes(snapshot[fs["config.284.count"]], "big")
        actual = [snapshot[fs[f"config.284[{i}].0"].start:fs[f"config.284[{i}].5"].stop] for i in range(actual_count)]
        directory = snapshot[fs["trailer.settings.0"]][4:].decode("utf-16be")
        if (actual != records or sm.read_bytes() != source
                or directory != "Z:\\长目录\\Archive\\"
                or snapshot[fs["config.head.12"]] != snapshot[fs["page.head.18"]]):
            raise ValueError("media gallery control failed: " + label)
    return [case[0] for case in cases]


def verify(exe, scratch):
    """Compare complete compressed output and check that unused images survive normal source editing."""
    reference = "docs/stage4-evidence/picture/native/compiled/"
    reports = []
    with tempfile.TemporaryDirectory(dir=scratch) as temp:
        work = Path(temp).resolve()
        for name in ("picture-multiple-bit", "picture-multiple-numeric", "picture-multiple-time",
                     "picture-static-red", "picture-distinct-red-green"):  # 20261128-22 change
            source = git_blob(reference + name, "input-native.sm")
            sm, archive = work / "input.sm", work / "generated.knr"
            sm.write_bytes(source)
            archive.unlink(missing_ok=True)
            run(exe, work, "--build-native-media-snapshot", sm, archive)
            actual = archive.read_bytes()
            expected = git_blob(reference + name, "upLoad.knr")
            if actual != expected or source != sm.read_bytes():
                raise ValueError("compressed archive differs or the original gallery was modified: " + name)
            reports.append(dict(project=name, sourceBytes=len(source), snapshotBytes=len(zlib.decompress(actual[4:])),
                                fullArchiveSHA256=hashlib.sha256(actual).hexdigest()))
        controls = gallery_controls(exe, work, reference)
    return dict(vendorWholeFiles=reports, editedControls=controls)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    parser.add_argument("--scratch", required=True, type=Path)
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    exe = args.exe.resolve()
    result = json.dumps(dict(executableSHA256=hashlib.sha256(exe.read_bytes()).hexdigest(),
                             gitRevision="e3b72657", cases=verify(exe, args.scratch)), ensure_ascii=False, indent=2)
    if args.receipt:
        args.receipt.write_text(result + "\n", encoding="utf8")
    print(result)
