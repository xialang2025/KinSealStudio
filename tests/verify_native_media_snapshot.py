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
    standalone = path / "separate.knr"
    run(exe, path, "--build-native-media-snapshot", source_path, standalone)
    original_bytes = struct.pack(">I", len(expected)) + zlib.compress(expected, 9)
    if standalone.read_bytes() != original_bytes or source_path.read_bytes() != source:
        raise ValueError(tag + " standalone archive or source is incorrect")
    evidence = dict(case=tag, initialSelectionWasNull=old == bytes.fromhex("ffffffff"),
                    page=name[4:].decode("utf-16-be"), sourceSHA256=hashlib.sha256(source).hexdigest(),
                    mediaKnrSHA256=hashlib.sha256(original_bytes).hexdigest(), standaloneCompressedBytesExact=True)
    try:
        run(exe, path, "--compile-native-media-parts", project, media, "--without-com1-driver")
    except ValueError as error:
        if tag == "new" or "系统、通信、语言或工程配置" not in str(error) or media.exists():
            raise
        evidence.update(completeMediaCompilation=False, unsupportedSystemDefaults=True, outputPublished=False)
        return evidence
    run(exe, path, "--compile-project", project, usb)
    if (media / "upLoad.knr").read_bytes() != original_bytes or zlib.decompress((usb / "upLoad.knr").read_bytes()[4:]) != source:
        raise ValueError(tag + " complete compiler disagrees with the standalone or USB source")
    evidence.update(completeMediaCompilation=True, mediaCompressedBytesExact=True, usbDecodedEqualsSource=True)
    return evidence


def explicit_empty(exe, path, sm):
    """Check that a non-null zero-length Qt string remains distinct from the null-selection sentinel."""
    source = sm.read_bytes()
    fields = {v["path"]: v for v in json.loads(run(exe, path, "--inspect-native-source", sm))["fields"]}
    span = fields["config.head.12"]
    changed = source[:span["start"]] + bytes(4) + source[span["end"]:]
    marker = path / "explicit-empty.sm"
    out = path / "explicit-empty.knr"
    marker.write_bytes(changed)
    run(exe, path, "--build-native-media-snapshot", marker, out)
    if zlib.decompress(out.read_bytes()[4:]) != changed:
        raise ValueError("a non-null empty selection was replaced")
    return dict(case="explicitEmpty", standalonePreservedNonNullEmpty=True)



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
            row = check_archive(exe, path, project, tag)
            if tag in ("new", "preselected"):
                if (path / "separate.knr").read_bytes() != git_blob(reference, "upLoad.knr"):
                    raise ValueError(tag + " archived vendor file differs")
                row["originalVendorArchiveExact"] = True
            if tag == "unicode" and row["page"] != "控制α":
                raise ValueError("the test did not exercise a renamed page")
            rows.append(row)
            if tag == "new":
                rows.append(explicit_empty(exe, path, sm))
    result = dict(releaseSHA256=hashlib.sha256(exe.read_bytes()).hexdigest(), cases=rows)
    args.receipt.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf8")
    print(sum(row.get("mediaCompressedBytesExact", False) for row in rows), "compiled cases of", len(rows))


if __name__ == "__main__":
    main()
