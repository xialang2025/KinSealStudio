"""Exercise the real single-page compiler with renamed pages and compare the full 72-byte descriptor."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile
import zlib


ROOT = Path(__file__).resolve().parents[1]


def run(exe, directory, *args, reject=False):
    """Run the Lite compiler offscreen and retain its diagnostic when probing rejection."""  # 20261128-10 add
    result = subprocess.run([str(exe), *map(str, args)], cwd=directory, capture_output=True, timeout=60,
                            env=dict(os.environ, QT_QPA_PLATFORM=os.environ.get("QT_QPA_PLATFORM", "offscreen"),
                                     TEMP=str(directory), TMP=str(directory)),
                            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
    if reject == (result.returncode == 0):
        raise AssertionError((args, result.returncode, result.stdout, result.stderr))
    return json.loads(result.stdout)


def verify(exe, scratch):
    """Compare KSP and SM entry points, including native truncation in the middle of a multibyte character."""
    fixtures = ("samples/native-device-test/CodexProbeTest.ksp", "samples/stage5/rectangle-default.ksp")
    reference = (ROOT / "samples/native-device-test/compile-test/SheetPara.dat").read_bytes()
    if len(reference) != 72:
        raise AssertionError("expected one original sheet")
    for fixture in fixtures:
        for name in ("画面1", "工艺A", "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abc测"):
            with tempfile.TemporaryDirectory(dir=scratch) as temp:
                directory = Path(temp)
                project = json.loads((ROOT / fixture).read_bytes())
                project["pages"][0]["name"] = name
                ksp, sm = directory / "page.ksp", directory / "page.sm"
                ksp.write_text(json.dumps(project, ensure_ascii=False), encoding="utf8")
                run(exe, directory, "--export-native-project", ksp, sm)
                # The frozen reference and the independent expected name use the Simplified Chinese Windows code page.
                expected = reference[:7] + name.encode("cp936")[:40].ljust(40, b"\0") + reference[47:]
                for source in (ksp, sm):
                    output = directory / source.suffix[1:]
                    run(exe, directory, "--compile-project", source, output)
                    actual = (output / "SheetPara.dat").read_bytes()
                    if actual != expected:
                        raise AssertionError((fixture, name, source.suffix, actual.hex(), expected.hex()))
                if fixture == fixtures[0]:
                    diagnostic = directory / "diagnostic"
                    run(exe, directory, "--compile-single-text", ksp, diagnostic)
                    if (diagnostic / "SheetPara.dat").read_bytes() != expected:
                        raise AssertionError("legacy compiler lost the page title")
                    archive = directory / "archived.sm"
                    archive.write_bytes(zlib.decompress((diagnostic / "upLoad.knr").read_bytes()[4:]))
                    if name != "画面1" and archive.read_bytes() != sm.read_bytes():
                        # The legacy text style adapter can change other native fields; verify the actual archived title.
                        archived = run(exe, directory, "--inspect-native-source", archive)["fields"]
                        title = next(f for f in archived if f["path"] == "page.head.18")
                        if archive.read_bytes()[title["start"] + 4:title["end"]].decode("utf-16-be") != name:
                            raise AssertionError("legacy source archive and device title use different page names")
                fields = run(exe, directory, "--inspect-native-source", sm)["fields"]
                spans = {v["path"]: v for v in fields}
                # A page reference to another name cannot use the single-page normalization rule.
                for key in ("config.head.4", "config.head.11"):
                    data = bytearray(sm.read_bytes())
                    data[spans[key]["end"] - 1] ^= 1
                    wrong = directory / (key + ".sm")
                    wrong.write_bytes(data)
                    target = directory / ("reject-" + key)
                    run(exe, directory, "--compile-project", wrong, target, reject=True)
                    if (target / "SheetPara.dat").exists():
                        raise AssertionError("failed compilation published a partial sheet")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    parser.add_argument("--scratch", type=Path, default=ROOT / "build")
    options = parser.parse_args()
    verify(options.exe.resolve(), options.scratch.resolve())
    print("Two compiler paths, three names, KSP/SM exports and mismatched-reference controls passed.")
