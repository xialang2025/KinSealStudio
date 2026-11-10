"""Compare new product sprites to three independently captured vendor compilations without normalizing unexplained bytes."""
#20261110-8 add: Pixel matches and structure differences are reported separately; neither proves screen behavior.
import argparse
from pathlib import Path
import struct
import subprocess
from stage1_model_verification import ROOT, cli, now, save, sha


def main():
    """Input: Frozen EXE, independent LZO decoder and new output path. Return: Process integrity status; differences remain explicit."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--decoder", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    executable, decoder, output = args.executable.resolve(), args.decoder.resolve(), args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    evidence = ROOT / "docs/stage3-evidence/native"
    record = dict(startedAtUtc=now(), executableSha256=sha(executable), decoderSha256=sha(decoder),
        variants=[], usbAccessed=False, physicalDisplayConfirmed=False, completed=False)
    try:
        if sha(executable) != sha(ROOT / "build/KinSealStudioLite.exe"):
            raise RuntimeError("Portable executable differs from this build.")
        for variant in ("base-only","transparent-top","opacity128-top"):
            source = evidence / "inputs" / ("static-odd2-"+variant+".ksp")
            native = evidence / "compiled" / ("static-odd2-"+variant)
            destination = output / variant
            if variant != "base-only":
                refused = cli(executable,["--compile-project",source,destination],output,variant+"-compile-refusal",19)
                record["variants"].append(dict(variant=variant,inputSha256=sha(source),status="blocked_by_existing_overlap_guard",result=refused))
                continue
            cli(executable,["--compile-project",source,destination],output,variant+"-compile")
            control = (destination/"ControlPara.dat").read_bytes()
            original = (native/"ControlPara.dat").read_bytes()
            pixels = (destination/"ControlPixel.dat").read_bytes()
            lengths = (destination/"PicLenPara.dat").read_bytes()
            result = dict(variant=variant,inputSha256=sha(source), productControlCount=len(control)//640,
                nativeControlCount=len(original)//640, controlBytesEqual=control==original,
                recordDifferences=[],frames=[])
            if len(control) != len(original) or len(control)%640:
                raise RuntimeError("Product and native control cardinalities differ for "+variant)
            offset = 0
            frame = 0
            for index in range(len(control)//640):
                left,right = original[index*640:(index+1)*640],control[index*640:(index+1)*640]
                result["recordDifferences"].append(dict(control=index,nativeTypeAndFlags=left[:4].hex(),
                    productTypeAndFlags=right[:4].hex(), geometryAndPixelRangesEqual=left[4:20]==right[4:20],
                    differingByteOffsets=[i for i in range(640) if left[i]!=right[i]]))
                width,height = struct.unpack_from("<HH",right,8)
                for language in range(2):
                    _,padded,compressed = struct.unpack_from("<III",lengths,2400+frame*12)
                    encoded,decoded = destination/("frame-"+str(frame)+".lzo"),destination/("frame-"+str(frame)+".rgb565")
                    encoded.write_bytes(pixels[offset:offset+compressed])
                    process = subprocess.run([str(decoder),str(encoded),str(decoded),str(width*height*2)],capture_output=True,
                        timeout=30,creationflags=subprocess.CREATE_NO_WINDOW)
                    if process.returncode:
                        raise RuntimeError("Independent decoder rejected product frame "+str(frame))
                    actual,expected = decoded.read_bytes(),(native/("frame-"+str(frame)+".rgb565")).read_bytes()
                    if len(actual)!=len(expected):
                        raise RuntimeError("Raw frame byte counts differ")
                    difference = [i//2 for i in range(0,len(actual),2) if actual[i:i+2]!=expected[i:i+2]]
                    result["frames"].append(dict(index=frame,control=index,language=language,width=width,height=height,
                        rawBytes=len(actual),differentPixels=len(difference),totalPixels=width*height,
                        nativeSha256=sha(native/("frame-"+str(frame)+".rgb565")),productSha256=sha(decoded),
                        firstDifferencePositions=difference[:24]))
                    offset += padded
                    frame += 1
            if offset!=len(pixels):
                raise RuntimeError("Product compressed frame indices leave unaccounted bytes")
            record["variants"].append(result)
        record["completed"] = True
        record["allComparedPixelsEqual"] = all(frame["differentPixels"]==0 for result in record["variants"] for frame in result.get("frames",[]))
        record["allControlBytesEqual"] = all(result.get("controlBytesEqual",False) for result in record["variants"])
        record["refusedVariants"] = sum(result.get("status")=="blocked_by_existing_overlap_guard" for result in record["variants"])
    except Exception as error:
        record["error"] = repr(error)
    record["identitiesUnchanged"] = sha(executable)==record["executableSha256"] and sha(decoder)==record["decoderSha256"]
    record["completed"] &= record["identitiesUnchanged"]
    record["completedAtUtc"] = now()
    save(output/"run.json",record)
    print({key:record.get(key) for key in ("completed","error","allComparedPixelsEqual","allControlBytesEqual")})
    return 0 if record["completed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
