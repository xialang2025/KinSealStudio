"""Verify V122 single-message compilation from the standard production entry points."""
# 20261128-24 add
import argparse
import hashlib
import io
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
REV = "e3b72657"
SOURCE = "docs/stage4-evidence/message/native/"
CORE = ("SheetPara.dat", "AddrPara.dat", "PicLenPara.dat", "SheetPixel.dat", "ControlPixel.dat", "Message.dat")


def verify(exe, directory):
    """Compare branch-specific native bytes and exercise independent edits and rejected inputs."""
    def git(*args):
        return subprocess.check_output(["git", *args], cwd=ROOT)

    def run(*args, success=True):
        env = dict(os.environ, QT_QPA_PLATFORM="offscreen", TMP=str(directory), TEMP=str(directory), TMPDIR=str(directory))
        p = subprocess.run([exe, *args], env=env, capture_output=True, timeout=60,
                           creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        if (p.returncode == 0) != success:
            raise AssertionError((args, p.returncode, p.stdout.decode("utf-8", "replace")))

    listing = git("ls-tree", "-r", "--name-only", REV, SOURCE).decode().splitlines()
    # Read the immutable references in one Git process; starting one process for every file dominates Windows runs.
    members = [p for p in listing if "/compiled/" in p or ("/native-probe-" in p and p.endswith(".json"))]
    requests = "".join(f"{REV}:{p}\n" for p in members).encode()
    batch = subprocess.check_output(["git", "cat-file", "--batch"], cwd=ROOT, input=requests)
    stream = io.BytesIO(batch)
    references = {}
    for path in members:
        _, kind, size = stream.readline().split()
        assert kind == b"blob"
        references[path] = stream.read(int(size))
        assert len(references[path]) == int(size) and stream.read(1) == b"\n"
    assert not stream.read(1)
    original = references.__getitem__
    profiles = {}
    for path in listing:
        if "/native-probe-" in path and path.endswith(".json"):
            for n, build in enumerate(json.loads(original(path)).get("nativeCompilations", [])):
                profiles[build["label"]] = ("download-defaults" if n else "fresh", build["sourceSha256"])
    rows = []
    for path in listing:
        if not path.endswith("/ControlPara.dat"):
            continue
        folder = path.rsplit("/", 1)[0]
        name = folder.rsplit("/", 1)[1]
        profile, expected_sha = profiles[name]
        source = directory / (name + ".sm")
        source.write_bytes(original(folder + "/input-native.sm"))
        assert hashlib.sha256(source.read_bytes()).hexdigest() == expected_sha
        for media in (False, True):
            out = directory / (name + ("-media" if media else "-disk"))
            args = ["--compile-native-media-parts" if media else "--compile-project", source, out]
            if media:
                args += ["--without-com1-driver", "--simulator-session", profile]
            run(*args)
            hashes = {}
            for member in CORE + (("SimMessage.dat", "load.bin") if media else ()):
                raw = (out / member).read_bytes()
                reference = original(folder + "/" + member)
                if raw != reference:
                    raise AssertionError(f"{name}/{out.name}/{member} differs from V5.1")
                hashes[member] = hashlib.sha256(raw).hexdigest()
            # Classify every frozen compiler member, including missing filenames and remaining byte differences.
            originals = {p.rsplit("/", 1)[1] for p in listing
                         if p.startswith(folder + "/") and not p.endswith("/input-native.sm")}
            generated = {f.name for f in out.iterdir() if f.is_file()}
            different = []
            for member in sorted(originals.intersection(generated).difference(hashes)):
                raw = (out / member).read_bytes()
                reference = original(folder + "/" + member)
                if raw == reference:
                    hashes[member] = hashlib.sha256(raw).hexdigest()
                else:
                    different.append(member)
            control = (out / "ControlPara.dat").read_bytes()
            expected = original(path)
            live = [(0, 20), (0x100, 0x10e), (0x110, 0x124), (0x128, 0x12c), (0x144, 0x148)]
            if name == "message-bilingual-size24":
                live.append((0x148, 0x14c))
            if name == "message-visibility-bit-level":
                live += [(0x15, 0x16), (0x1e, 0x20), (0x22, 0x23), (0x94, 0x96), (0x9c, 0xa0)]
            assert len(expected) == len(control) == 640
            for start, end in live:
                assert control[start:end] == expected[start:end], (name, start)
            differences = [i for i in range(640) if control[i] != expected[i]]
            rows.append(dict(case=name, route="media" if media else "disk", sourceSHA256=expected_sha, profile=profile,
                             exactFiles=hashes, differentFiles=different, missingFiles=sorted(originals - generated),
                             additionalGeneratedFiles=sorted(generated - originals), activeControlBytes=sum(e - b for b, e in live),
                             entireControlEqual=not differences, differingControlBytes=len(differences),
                             firstDifference=differences[0] if differences else None))
    assert len(rows) == 18
    seed = directory / "message-default-r1.sm"
    edited = directory / "edited.ksp"
    run("--migrate-project", seed, edited)
    project = json.loads(edited.read_bytes())
    obj = project["pages"][0]["objects"][0]
    message = obj["messageDisplay"]
    obj.update(address="LW27", x=79, y=30, width=250, height=150)
    message.update(monitorAddress="LW27", movement=2, speed=8, dataType=1)
    style = message["languageAppearances"][project["languages"][0]]
    style.update(textColor="#7f1234ab", fontSize=24)
    message["states"][1][project["languages"][0]] = "C" * 99 + "中"
    for pristine in (False, True):
        if pristine:
            project.pop("nativeSourceSnapshot", None)
            project.pop("nativeSourceWarnings", None)
        edited.write_text(json.dumps(project, ensure_ascii=False), encoding="utf-8")
        out = directory / ("new" if pristine else "resized")
        run("--compile-project", edited, out)
        ctrl = (out / "ControlPara.dat").read_bytes()
        assert struct.unpack_from("<4H", ctrl, 4) == (79, 30, 250, 150)
        assert struct.unpack_from("<4H", ctrl, 0x100) == (1, 2, 2, 8)
        assert struct.unpack_from("<4I", ctrl, 0x110) == (89, 36, 230, 138)
        assert ctrl[0x121] == ord("W") and struct.unpack_from("<I", ctrl, 0x128)[0] == 27
        assert struct.unpack_from("<I", ctrl, 0x144)[0] == 24
        assert (out / "Message.dat").read_bytes()[104:] == bytes.fromhex("12 34 ab 00") + b"C" * 99 + bytes.fromhex("d6")
    for key, value in (("style", 2), ("fontFamily", "Unmeasured"), ("backgroundColor", "#ff00887f")):
        model = json.loads(edited.read_bytes())
        model["pages"][0]["objects"][0]["messageDisplay"]["languageAppearances"][project["languages"][0]][key] = value
        invalid = directory / ("invalid-" + key + ".ksp")
        out = invalid.with_suffix(".output")
        invalid.write_text(json.dumps(model, ensure_ascii=False), encoding="utf-8")
        run("--compile-project", invalid, out, success=False)
        assert not out.exists()
    matrix = json.loads(edited.read_bytes())
    matrix_message = matrix["pages"][0]["objects"][0]["messageDisplay"]
    first = matrix["languages"][0]
    matrix["languages"] = [first, "L2", "L3", "L4", "L5", "L6"]
    sizes = (12, 16, 24, 48, 64, 192)
    for n, name in enumerate(matrix["languages"]):
        matrix_message["languageAppearances"][name] = dict(matrix_message["languageAppearances"][first], fontSize=sizes[n])
        matrix_message["languageFrameColors"][name] = matrix_message["languageFrameColors"][first]
    six = directory / "six.ksp"
    six.write_text(json.dumps(matrix, ensure_ascii=False), encoding="utf-8")
    run("--compile-project", six, directory / "six")
    ctrl = (directory / "six" / "ControlPara.dat").read_bytes()
    assert struct.unpack_from("<6I", ctrl, 0x144) == sizes
    assert len((directory / "six" / "Message.dat").read_bytes()) == 6 * 2 * 104
    matrix["languages"].append("L7")
    seven = directory / "seven.ksp"
    seven.write_text(json.dumps(matrix, ensure_ascii=False), encoding="utf-8")
    run("--compile-project", seven, directory / "seven", success=False)
    assert not (directory / "seven").exists()
    subpixel = json.loads(edited.read_bytes())
    subpixel["pages"][0]["objects"][0].update(subpixelX=0.5, subpixelY=0.5)
    fraction = directory / "subpixel.ksp"
    fraction.write_text(json.dumps(subpixel, ensure_ascii=False), encoding="utf-8")
    run("--compile-project", fraction, directory / "fraction", success=False)
    assert not (directory / "fraction").exists()
    return dict(nativeRevision=REV, cases=rows, editFromSource=True, editWithoutSource=True,
                rejectedUnsupportedAppearances=3, sixLanguageFontSlots=True, seventhLanguageRejected=True,
                fractionalLayoutRejected=True, completeFileMatches=sum(len(r["exactFiles"]) for r in rows))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--report", type=Path)
    opts = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="message-device-", dir=ROOT / "build") as temp:
        result = verify(opts.exe.resolve(), Path(temp))
    if opts.report:
        opts.report.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"9 original projects, 18 standard compilations, {result['completeFileMatches']} complete file matches")
