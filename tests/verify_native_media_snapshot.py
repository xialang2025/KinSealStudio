"""Test the original media snapshot and the ordinary USB archive through public compiler commands."""
# 20261128-7 add
import argparse
import hashlib
import json
from pathlib import Path
import struct
import tempfile
import zlib

from verify_native_drawing_drafts import git_blob, run

DRAFT = "docs/stage4-evidence/freeline/native/click-investigation/compiled/freeline-click-candidate-r1"
PRESELECTED = "docs/stage2-evidence/native/compiled/final-system-baseline-repeat-fresh"


def check_archive(exe, path, project, tag):
    """Check one archive, including the exact serialized page name, full zlib output, and unchanged USB source."""
    source_path = path / "exported.sm"
    run(exe, path, "--export-native-project", project, source_path)
    source = source_path.read_bytes()
    fields = {f["path"]: f for f in json.loads(run(exe, path, "--inspect-native-source", source_path))["fields"]}
    current = fields["config.head.12"]
    page = fields["page.head.18"]
    old = source[current["start"]:current["end"]]
    name = source[page["start"]:page["end"]]
    expected = source if old != bytes.fromhex("ffffffff") else source[:current["start"]] + name + source[current["end"]:]
    media = path / "media"
    usb = path / "usb"
    run(exe, path, "--compile-native-media-parts", project, media, "--without-com1-driver")
    run(exe, path, "--compile-project", project, usb)
    actual = (media / "upLoad.knr").read_bytes()
    if actual != struct.pack(">I", len(expected)) + zlib.compress(expected, 9):
        raise ValueError(tag + " media archive differs from the independently constructed compressed bytes")
    if zlib.decompress((usb / "upLoad.knr").read_bytes()[4:]) != source:
        raise ValueError(tag + " ordinary archive lost source data")
    if project.suffix.lower() == ".sm" and project.read_bytes() != source:
        raise ValueError(tag + " native source changed")
    return dict(case=tag, initialSelectionWasNull=old==bytes.fromhex("ffffffff"), page=name[4:].decode("utf-16-be"),
                sourceSHA256=hashlib.sha256(source).hexdigest(), mediaKnrSHA256=hashlib.sha256(actual).hexdigest(),
                mediaCompressedBytesExact=True, usbDecodedEqualsSource=True)


def main():
    """Exercise two vendor projects and an independently renamed Lite page without a retained source snapshot."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    parser.add_argument("--scratch", type=Path, required=True)
    parser.add_argument("--receipt", type=Path, required=True)
    args = parser.parse_args()
    exe = args.exe.resolve()
    rows = []
    with tempfile.TemporaryDirectory(dir=args.scratch) as space:
        root = Path(space).resolve()
        for tag, reference in [("new", DRAFT), ("preselected", PRESELECTED), ("unicode", DRAFT)]:
            path = root / tag
            path.mkdir()
            sm = path / "original.sm"
            sm.write_bytes(git_blob(reference, "input-native.sm"))
            project = sm
            if tag == "unicode":
                project = path / "named.ksp"
                run(exe, path, "--migrate-project", sm, project)
                model = json.loads(project.read_bytes())
                del model["nativeSourceSnapshot"]
                model.pop("nativeSourceWarnings", None)
                model["pages"][0]["name"] = "控制α"
                project.write_text(json.dumps(model, ensure_ascii=False), encoding="utf8")
            try:
                row = check_archive(exe, path, project, tag)
            except ValueError as error:
                # The full V122 project compiler can reject independent system settings before the media writer is reached.
                if tag == "new" or "系统、通信、语言或工程配置" not in str(error) or (path / "media").exists():
                    raise
                row = dict(case=tag, completeMediaCompilation=False, unsupportedSystemDefaults=True,
                           outputPublished=False, sourceBytesUnchanged=sm.read_bytes() == git_blob(reference, "input-native.sm"))
                rows.append(row)
                continue
            if tag in ("new", "preselected"):
                if (path / "media/upLoad.knr").read_bytes() != git_blob(reference, "upLoad.knr"):
                    raise ValueError(tag + " archived vendor file differs")
                row["originalVendorArchiveExact"] = True
            if tag == "unicode" and row["page"] != "控制α":
                raise ValueError("the test did not exercise a renamed page")
            rows.append(row)
    result = dict(releaseSHA256=hashlib.sha256(exe.read_bytes()).hexdigest(), cases=rows)
    args.receipt.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf8")
    print(sum(row.get("mediaCompressedBytesExact", False) for row in rows), "compiled cases of", len(rows))


if __name__ == "__main__":
    main()
