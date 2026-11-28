"""Compare SM-derived alarm fields with original Git blobs and logged V5.1 compilations."""
# 20261128-10 add
import argparse
from collections import Counter, defaultdict
from functools import lru_cache
import hashlib
import itertools
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from native_alarm_fields import BIT_BYTES, CONDITION_BYTES, MESSAGE_BYTES, encode_alarm, read_fields, write_messages

REVISION = "e3b72657"


def sha(data):
    """Return the SHA-256 identity recorded by the native compiler logger."""
    return hashlib.sha256(data).hexdigest()


def verify_writer(path):
    """Pin the installed V5.1 writer's block copies, strides and separately serialized flags."""
    import pefile
    image = path.read_bytes()
    identity = sha(image)
    if identity != "305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c":
        raise ValueError("the selected binary differs from the original V5.1 reference")
    anchors = {
        0x91299f: "8db53444ffffb90c2600008bbd2044fffff3a5",
        0x9129b8: "05309800008db564dcffff66b9ac0889c7f3a5",
        0x9129e6: "c7442408e0ba0000", 0x9124d6: "83850c44ffff4c",
        0x91256e: "8b85f443ffffc1e0040385f443ffff8b95f443ffff8d0482c1e00289850044ffff",
        0x9127c9: "83850c44ffff1083851444ffff4083bd2444ffff04", 0x912730: "dd5b04",
        0x91b3a9: "0fb68634010000", 0x91b3c0: "0fb686fc000000",
        0x912bf9: "0fb681fc000000898560dcffff", 0x912b74: "0fb68234010000898510ffffff",
    }
    with pefile.PE(data=image, fast_load=True) as pe:
        for address, expected in anchors.items():
            start = pe.get_offset_from_rva(address - pe.OPTIONAL_HEADER.ImageBase)
            value = bytes.fromhex(expected)
            if image[start:start + len(value)] != value:
                raise ValueError(f"unexpected native instruction at {address:x}")
    return {"sha256": identity, "anchors": [hex(addr) for addr in sorted(anchors)]}


def compare_fields(blob, length, fields):
    """Check the exact offset and size of each encoded range against the independent original."""
    if len(blob) != length:
        raise ValueError("wrong original length")
    previous = 0
    for offset, value in sorted(fields.items()):
        if offset < previous or offset + len(value) > length or blob[offset:offset + len(value)] != value:
            raise ValueError(f"field mismatch at {offset}")
        previous = offset + len(value)
    return sum(map(len, fields.values()))


def run(exe, scratch):
    """Read the fixed corpus once; supply only the native SM to the field encoder."""
    tree = {}
    listing = subprocess.check_output(["git", "ls-tree", "-rz", REVISION], cwd=ROOT)
    for entry in filter(None, listing.split(b"\0")):
        info, name = entry.split(b"\t", 1)
        tree[name.decode()] = info.split()[2]
    pipe = subprocess.Popen(["git", "cat-file", "--batch"], cwd=ROOT, stdin=subprocess.PIPE, stdout=subprocess.PIPE)

    @lru_cache(maxsize=2048)
    def blob(oid):
        """Retrieve a blob by immutable Git identity from the local batch reader."""
        pipe.stdin.write(oid + b"\n")
        pipe.stdin.flush()
        length = int(pipe.stdout.readline().split()[2])
        data = pipe.stdout.read(length)
        if len(data) != length or pipe.stdout.read(1) != b"\n":
            raise ValueError("incomplete Git blob")
        return data

    @lru_cache(maxsize=512)
    def compile_source(oid):
        """Use a validated Qt field map for the source only; no comparison file enters the writer."""
        data = blob(oid)
        source = scratch / "source.sm"
        source.write_bytes(data)
        child = subprocess.run([str(exe), "--inspect-native-source", str(source)], timeout=30,
                               env=dict(os.environ, TEMP=str(scratch), TMP=str(scratch), QT_QPA_PLATFORM="offscreen"),
                               creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0), capture_output=True, check=True)
        fields = read_fields(data, json.loads(child.stdout))
        return encode_alarm(fields), fields["config.ec.0"], fields["config.124.0"]

    compiled = {}
    same_source = defaultdict(list)
    try:
        for name, oid in tree.items():
            folder, _, basename = name.rpartition("/")
            if basename == "input-native.sm" and folder + "/AlarmPara.dat" in tree:
                compiled[folder] = oid
                same_source[oid].append(folder)
        adjacent = len(compiled)
        # Stage 4's GUI logger separately records the SHA of the edited source and each compiled file.
        for path, log_oid in tree.items():
            if "/native/native-probe-" not in path or not path.endswith(".json"):
                continue
            if not any("/" + family + "/" in path for family in ("alarm-bar", "alarm-display", "history-alarm")):
                continue
            log = json.loads(blob(log_oid))
            root = path.rsplit("/", 1)[0]
            for record in log.get("nativeCompilations", []):
                files = {f["path"]: f["sha256"] for f in record.get("files", [])}
                if not {"AlarmPara.dat", "AlarmMuti.dat"} <= files.keys():
                    continue
                folder = root + "/compiled/" + record["label"]
                source = next(f for f in log["outputs"] if f["sha256"] == record["sourceSha256"])
                oid = tree[root + "/" + source["path"]]
                if sha(blob(oid)) != record["sourceSha256"] or any(
                    sha(blob(tree[folder + "/" + n])) != files[n] for n in ("AlarmPara.dat", "AlarmMuti.dat")
                ):
                    raise ValueError("native source/output identities differ from the original log")
                compiled[folder] = oid
        samples = []
        counts = Counter()
        for folder, oid in sorted(compiled.items()):
            (para, messages), bits, groups = compile_source(oid)
            original = blob(tree[folder + "/AlarmPara.dat"])
            multilingual = blob(tree[folder + "/AlarmMuti.dat"])
            known = compare_fields(original, BIT_BYTES + CONDITION_BYTES, para)
            text_known = compare_fields(multilingual, len(messages) * MESSAGE_BYTES, messages)
            if not bits:
                # This is a corpus observation, measured separately from explicit native assignments.
                counts["inactiveBitZeroPrefix"] += int(not any(original[:BIT_BYTES]))
                counts["inactiveBitTables"] += 1
            counts["completeAlarmMuti"] += int(text_known == len(multilingual))
            counts["activeBit"] += bool(bits)
            counts["activeCondition"] += bool(groups)
            if bits or groups:
                samples.append(dict(folder=folder, sourceSHA256=sha(blob(oid)), bits=bits, groups=groups,
                                    alarmBytesVerified=known, messageBytesVerified=text_known,
                                    messageLength=len(multilingual), alarmSHA256=sha(original), messageSHA256=sha(multilingual)))
        differences = []
        for folders in same_source.values():
            for a, b in itertools.combinations(folders, 2):
                lhs = blob(tree[a + "/AlarmPara.dat"])
                rhs = blob(tree[b + "/AlarmPara.dat"])
                differences.append(sum(x != y for x, y in zip(lhs, rhs)))
        if not samples or not counts["activeCondition"] or len(compiled) <= adjacent or not min(differences):
            raise ValueError("missing original positive or same-source negative controls")
        return dict(revision=REVISION, adjacent=adjacent, logged=len(compiled)-adjacent,
                    sourceBodies=compile_source.cache_info().currsize, comparisons=len(compiled), counts=dict(counts),
                    sameSourceGroups=sum(len(v) > 1 for v in same_source.values()), sameSourcePairs=len(differences),
                    fullAlarmEqualPairs=differences.count(0), minPairChanged=min(differences), maxPairChanged=max(differences),
                    activeSamples=samples)
    finally:
        pipe.stdin.close()
        pipe.wait()


def slot_boundaries(scratch):
    """Check the complete-writer boundary, code-page truncation and retention of a previous output."""
    path = scratch / "AlarmMuti.dat"
    values = {"config.ec.0": 1, "config.124.0": 0, "config.ec.1": 300, "config.d0": 0, "config.134.1": 1,
              "config.100.count": 1, "config.104.count": 1, "config.108.count": 1, "config.108[0].0": 1,
              "config.100[0]": "未使用", "config.104[0]": "未使用", "system.164.count": 1, "system.identity.15": 1,
              "project.pages.count": 1, "page.head.18": "画面1"}
    for text in ("A"*49, "A"*48 + "中", "A"*49 + "中", "A"*60):
        values["config.108[0].1"] = text
        para, slots = encode_alarm(values)
        write_messages(path, slots)
        expected = (text.encode("cp936") + b"\0")[:50]
        if (path.read_bytes() != expected or para[68] != expected
                or para[1] != bytes([44]) or para[BIT_BYTES-4] != b"\1\0\0\0"):
            raise ValueError("wrong complete message slot")
    before = path.read_bytes()
    values["config.108[0].1"] = "短"
    _, slots = encode_alarm(values)
    try:
        write_messages(path, slots)
    except ValueError:
        if path.read_bytes() != before:
            raise ValueError("rejected short message changed output")
    else:
        raise ValueError("unverified trailing bytes were accepted")
    write_messages(path, {})
    if path.read_bytes():
        raise ValueError("empty table did not compile exactly")


def main():
    """Run offscreen and write a compact receipt with the historical active cases."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--native-exe", type=Path, help="verify the installed original writer's static instructions")
    parser.add_argument("--scratch", type=Path, default=ROOT / "build")
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(dir=args.scratch.resolve()) as temp:
        scratch = Path(temp)
        slot_boundaries(scratch)
        result = run(args.exe.resolve(), scratch)
    if args.native_exe:
        result["originalWriter"] = verify_writer(args.native_exe)
    text = json.dumps(result, indent=2, ensure_ascii=False) + "\n"
    if args.report:
        args.report.write_text(text, encoding="utf8")
    print(text if not args.report else json.dumps({k:v for k,v in result.items() if k != "activeSamples"}))


if __name__ == "__main__":
    main()
