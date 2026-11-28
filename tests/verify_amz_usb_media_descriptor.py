"""Check AMZ media-descriptor reads and predict full files across logged consecutive original builds."""
# 20261128-18 add
import argparse
from datetime import datetime
import gzip
import hashlib
import io
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import sys
from tempfile import TemporaryDirectory
import zipfile


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from rebuild_usb_metadata_fields import earliest_native_seed_time, render_descriptor
from stage6_media_bundle import pack_native_media


def sha(raw):
    """Identify each build and each complete predicted file."""
    return hashlib.sha256(raw).hexdigest()


def fw_reads():
    """Verify the decoded instructions and enumerate the direct data literals inside the copied header."""
    path = ROOT / "firmware_analysis/amz070w01-disasm-20261119/full_disassembly.asm"
    asm = path.read_bytes()
    literals = {}
    code = {}
    for line in asm.decode().splitlines():
        row = line.split(None, 4)
        if len(row) != 5 or not re.fullmatch("[0-9A-Fa-f]{8}", row[1]):
            continue
        addr = int(row[1], 16)
        if row[2] == "ARM":
            code[addr] = row[4].split("  ;")[0].strip()
        if row[2] == "DATA" and len(row[3]) == 8:
            val = int.from_bytes(bytes.fromhex(row[3]), "little")
            if 0x80348790 <= val < 0x80348b18:
                literals[addr] = val - 0x80348790
    expected = {0x8004eb2c:0, 0x80081754:0, 0x8008fbf0:0, 0x80096abc:0, 0x80096afc:0x162,
                0x800970f0:0, 0x800976e0:0, 0x800978b0:0, 0x80097e08:0, 0x800984b0:0, 0x80098b10:0}
    ops = {0x8004ead8:"mov r2,#0x388", 0x8004eae8:"bl 0x80020c10", 0x80081724:"ldrb r2,[r1,#0x2]",
           0x8008172c:"add r1,r1,#0x4c", 0x80081734:"mov r2,#0xa", 0x8008173c:"bl 0x80020c10",
           0x8008fba0:"ldr r1,[r1,#0xc]", 0x80096814:"ldrb r8,[r4,#0x1c]", 0x8009684c:"add r1,r4,#0x130",
           0x80096980:"ldr r1,[0x80096afc]", 0x80096f64:"ldrb r1,[r1,#0x0]", 0x80097578:"ldr r1,[r1,#0x14]",
           0x80097724:"ldr r1,[r1,#0x14]", 0x80097c58:"ldr r6,[r0,#0xf8]", 0x80098300:"ldr r6,[r0,#0xfc]",
           0x80098a14:"ldr r0,[r0,#0x4]"}
    assert literals == expected and all(code[a] == text for a, text in ops.items())
    references = {}
    for addr, instruction in code.items():
        literal = re.search(r"\[(0x[0-9a-f]+)\]", instruction)
        if literal and int(literal[1],16) in literals:
            references[addr] = literals[int(literal[1],16)]
    assert list(references) == [0x8004eae4,0x8008171c,0x8008fb9c,0x8009680c,0x80096980,0x80096f60,
                                0x80097574,0x80097720,0x80097c50,0x800982f8,0x80098a0c]
    return {"assemblySHA256":sha(asm), "headerBytes":904, "directLiterals":{hex(k):v for k,v in literals.items()},
            "directReferenceSites":{hex(k):v for k,v in references.items()},"instructions":{hex(k):v for k,v in ops.items()}}


def direct_values(data):
    """Read the width used by each confirmed instruction, including terminated COM1/COM2 basenames."""
    words = [(0,1),(2,1),(4,4),(12,4),(20,4),(28,1),(76,10),(248,4),(252,4)]
    values = {hex(off):data[off:off + size] for off,size in words}
    for off in (0x130,0x162):
        end = data.index(0, off, off+50)
        values[hex(off)] = data[off:end+1]
    return values


def clock_seconds(t):
    """Convert a civil timestamp to the input used by the native qrand seed."""
    return t.hour * 3600 + t.minute * 60 + t.second


def check_cli(raw):
    """Check the public disk-only command with fresh, conflicting, and incomplete input files."""
    with TemporaryDirectory(dir=ROOT / "build") as temp, zipfile.ZipFile(io.BytesIO(raw)) as archive:
        work = Path(temp)
        parts = work / "parts"
        parts.mkdir()
        infos = {i.filename.lower():i for i in archive.infolist()}
        for name in ("load.bin","syspara.dat","addrpara.dat","piclenpara.dat","sheetpixel.dat","controlpixel.dat"):
            if name in infos:
                (parts / name).write_bytes(archive.read(infos[name]))
        date = datetime(*infos["load.bin"].date_time)
        cmd = [sys.executable,"-B",str(ROOT/"tools/rebuild_usb_metadata_fields.py"),str(parts),
               "--preceding-load-time",date.isoformat()]
        predicted = work / "first.bin"
        p = subprocess.run(cmd+["--comparison-bin",str(predicted)],capture_output=True,check=True)
        assert json.loads(p.stdout)["seedSource"] == "estimated"
        field_map = json.loads(subprocess.check_output(cmd))
        assert field_map["estimatedClock"]["offset"] == 4 and all(v["offset"] != 4 for v in field_map["ranges"])
        metadata,lengths,seconds,_,_ = package_case(raw)
        assert predicted.read_bytes() == render_descriptor(metadata,lengths,seconds)
        existing = subprocess.run(cmd+["--comparison-bin",str(predicted)],capture_output=True)
        assert existing.returncode and predicted.read_bytes() == render_descriptor(metadata,lengths,seconds)
        previous = work / "prior.bin"
        previous.write_bytes(bytes([0x55])*1704)
        contextual = work / "context.bin"
        subprocess.run(cmd+["--prior-descriptor",str(previous),"--comparison-bin",str(contextual)],capture_output=True,check=True)
        assert contextual.read_bytes() == render_descriptor(metadata,lengths,seconds,prior=previous.read_bytes())
        previous.write_bytes(bytes(1703))
        absent = work / "invalid.bin"
        short = subprocess.run(cmd+["--prior-descriptor",str(previous),"--comparison-bin",str(absent)],capture_output=True)
        assert short.returncode and not absent.exists()
    return {"normalized":True,"estimatedFieldSeparated":True,"priorOverlay":True,"existingFilePreserved":True,
            "invalidPriorRejected":True}


def package_case(raw):
    """Predict before reading USB contents, using only companion files and the earlier load.bin's DOS time."""
    with zipfile.ZipFile(io.BytesIO(raw)) as archive:
        infos = {v.filename.lower():v for v in archive.infolist()}
        metadata = archive.read(infos["load.bin"])
        system = archive.read(infos["syspara.dat"])
        time = earliest_native_seed_time(system, infos["load.bin"].date_time)
        second = clock_seconds(time)
        lengths = {name:item.file_size for name,item in infos.items() if name != "usbload.bin"}
        specimen = render_descriptor(metadata, lengths, second)
        oracle = archive.read(infos["usbload.bin"])
    observed = direct_values(oracle)
    predicted = direct_values(specimen)
    differences = [k for k,v in observed.items() if v != predicted[k]]
    assert set(differences) <= {"0x4"} and len(oracle) == len(specimen) == 1704
    next_seed = struct.pack("<I", ((214013 * ((second + 1) % 86400) + 2531011) >> 16) & 32767)
    two_seconds = (predicted["0x4"], next_seed)
    return metadata, lengths, second, oracle, {
        "archive":sha(raw), "original":sha(oracle), "normalized":sha(specimen), "seedSecond":second,
        "normalizedExact":specimen == oracle, "directFieldsOtherThanClockExact":True,
        "systemClockSeedExact":oracle[4:8] == struct.pack("<I", ((214013 * sum(x * n for x,n in
                            zip(struct.unpack_from("<3I",system,324),(1,60,3600))) + 2531011) >> 16) & 32767),
        "clockExact":not differences, "observedWithinTwoSecondCandidates":observed["0x4"] in two_seconds,
        "commonSeedBytePositions":[i for i in range(4) if two_seconds[0][i] == two_seconds[1][i]],
        "knownReadBytes":sum(map(len, observed.values()))}


def repack(raw, predicted):
    """Pass other member bytes and original member times to the normal packer and compare the new archive."""
    with TemporaryDirectory(dir=ROOT / "build") as temp, zipfile.ZipFile(io.BytesIO(raw)) as archive:
        target = Path(temp) / "comparison.kns"
        parts = Path(temp) / "parts"
        parts.mkdir()
        for info in archive.infolist():
            assert Path(info.filename).name == info.filename
            dst = parts / info.filename
            dst.write_bytes(predicted if info.filename.lower() == "usbload.bin" else archive.read(info))
            mtime = datetime(*info.date_time).timestamp()
            os.utime(dst, (mtime,mtime))
        pack_native_media(parts,target)
        return target.read_bytes()


def audit(revision,extra):
    """Pair originals by the log's recorded sequence and immutable archive digest before measuring any result."""
    tree = subprocess.check_output(["git","ls-tree","-rz",revision],cwd=ROOT)
    files = {}
    for row in filter(None, tree.split(b"\0")):
        meta,path = row.split(b"\t",1)
        files[path.decode()] = meta.split()[2].decode()
    reader = subprocess.Popen(["git","cat-file","--batch"],cwd=ROOT,stdin=subprocess.PIPE,stdout=subprocess.PIPE)

    def read(oid):
        reader.stdin.write(oid.encode()+b"\n")
        reader.stdin.flush()
        n = int(reader.stdout.readline().split()[2])
        data = reader.stdout.read(n)
        assert len(data) == n and reader.stdout.read(1) == b"\n"
        return data

    inventory = {}
    allcases = []
    rows = []
    missing = []
    try:
        for oid in sorted({oid for name,oid in files.items() if name.lower().endswith("/load.kns")}):
            raw = read(oid)
            *_, row = package_case(raw)
            inventory[row["archive"]] = oid
            allcases.append(row)
        for path in extra:
            *_, row = package_case(path.read_bytes())
            if row["archive"] not in inventory:
                allcases.append(row)
        seen = set()
        for name,oid in sorted(files.items()):
            if (not name.startswith("docs/stage") or "/compiled/" in name or
                    not ("probe" in name or "capture" in name) or not name.endswith((".json",".json.gz"))):
                continue
            rawlog = read(oid)
            if rawlog.startswith(b"\x1f\x8b"):
                rawlog = gzip.decompress(rawlog)
            if b'"nativeCompilations"' not in rawlog:
                continue
            log = json.loads(rawlog)
            builds = log.get("nativeCompilations") or []
            if type(log.get("pid")) is not int:
                continue
            for prev,cur in zip(builds,builds[1:]):
                refs = [{f["path"].lower():f["sha256"] for f in r.get("files",[]) if "sha256" in f} for r in (prev,cur)]
                if any("load.kns" not in ref or "usbload.bin" not in ref for ref in refs):
                    continue
                key = tuple(ref["load.kns"] for ref in refs)
                if key in seen or key[0] == key[1]:
                    continue
                seen.add(key)
                row = {"log":name,"pid":log["pid"],"previousBuild":prev["label"],"currentBuild":cur["label"],
                       "previousSM":prev["sourceSha256"],"currentSM":cur["sourceSha256"],"previousArchive":key[0],"currentArchive":key[1]}
                if any(k not in inventory for k in key):
                    missing.append(row)
                    continue
                with zipfile.ZipFile(io.BytesIO(read(inventory[key[0]]))) as archive:
                    prior = archive.read("Usbload.bin")
                raw = read(inventory[key[1]])
                metadata,lengths,seconds,oracle,_ = package_case(raw)
                assert sha(prior) == refs[0]["usbload.bin"] and sha(oracle) == refs[1]["usbload.bin"]
                result = render_descriptor(metadata,lengths,seconds,prior=prior)
                diff = [i for i,(x,y) in enumerate(zip(result,oracle)) if x != y]
                packed = repack(raw,result)
                assert not (set(diff)-set(range(4,8))-set(range(314,324))) and (not diff) == (raw == packed)
                row.update(predictedDescriptor=sha(result),originalDescriptor=sha(oracle),differenceOffsets=diff,
                           predictedArchive=sha(packed),archiveExact=packed==raw)
                rows.append(row)
    finally:
        reader.stdin.close()
        reader.wait()
    return {"revision":revision,"firmware":fw_reads(),"archives":len(allcases),
            "cli":check_cli(extra[0].read_bytes()) if extra else None,
            "normalizedWholeExact":sum(c["normalizedExact"] for c in allcases),"directNonClockExact":len(allcases),
            "predictedClockExact":sum(c["clockExact"] for c in allcases),
            "systemClockAloneExact":sum(c["systemClockSeedExact"] for c in allcases),
            "twoClockCandidatesContainOriginal":sum(c["observedWithinTwoSecondCandidates"] for c in allcases),
            "transitions":len(rows), "earlierBuildWholeFilesExact":sum(r["archiveExact"] for r in rows),
            "allCases":allcases,"chronologicalPairs":rows,"missingPairMembers":missing}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--revision",default="e3b72657")
    parser.add_argument("--report",type=Path)
    parser.add_argument("extra",nargs="*",type=Path)
    args = parser.parse_args()
    report = audit(args.revision,args.extra)
    if args.report:
        args.report.write_bytes((json.dumps(report,ensure_ascii=False,indent=1)+"\n").encode())
    print({k:v for k,v in report.items() if not isinstance(v,(dict,list))})
