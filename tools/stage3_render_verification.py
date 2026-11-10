"""Compare the frozen stage-3 renderer with the previous release and verify geometry/overlap device guards."""
#20261110-8 add: These are product regression comparisons, not new vendor-native pixel oracles.
import argparse
import copy
from pathlib import Path
from stage1_model_verification import ROOT, cli, now, read, save, sha


def main():
    """Input: Frozen portable EXE and new evidence directory. Return: Observed regression status."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    executable, output = args.executable.resolve(), args.output.resolve()
    previous = ROOT / "dist/KinSealStudioLite-Stage2-0.9.0/KinSealStudioLite.exe"
    output.mkdir(parents=True, exist_ok=False)
    record = dict(startedAtUtc=now(), scope="product-regression; vendor Qt4 coverage remains separate",
        previousSha256=sha(previous), executableSha256=sha(executable), comparisons=[], passed=False, usbAccessed=False)
    try:
        if sha(executable) != sha(ROOT / "build/KinSealStudioLite.exe"):
            raise RuntimeError("The portable EXE differs from this build.")
        base = read(ROOT / "samples/stage3/stage3-single-page.ksp")
        for size in (16,17.5,17.9,23.2):
            project = copy.deepcopy(base)
            for obj in project["pages"][0]["objects"]:
                if "staticText" in obj:
                    obj["staticText"]["fontSize"] = size
                for state in obj.get("bitButton",{}).get("states",[]):
                    state["text"]["fontSize"] = size
            stem = "font-"+str(size).replace(".","_")
            source = output / (stem+".ksp")
            save(source,project)
            for label, program in (("previous",previous),("current",executable)):
                cli(program,["--compile-project",source,output/(stem+"-"+label)],output,stem+"-"+label)
            entries = []
            for name in ("ControlPara.dat","ControlPixel.dat","SheetPara.dat","SheetPixel.dat","PicLenPara.dat","AddrPara.dat"):
                left,right = output/(stem+"-previous")/name,output/(stem+"-current")/name
                entries.append(dict(name=name,previous=sha(left),current=sha(right),equal=sha(left)==sha(right)))
            record["comparisons"].append(dict(fontSize=size,files=entries))
            if not all(item["equal"] for item in entries):
                raise RuntimeError("Existing 96-DPI product output changed at font size "+str(size))
        source = output / "resized.ksp"
        # Compile an exact size-16 counterpart of the source geometry before evaluating invalidation.
        baseline = read(output/"font-16.ksp")
        baseline["pages"][0]["objects"][1]["width"] += 2
        save(source,baseline)
        cli(executable,["--compile-project",source,output/"resized-16"],output,"compile-resized-16")
        record["geometryInvalidation"] = {name:sha(output/"resized-16"/name)!=sha(output/"font-16-current"/name)
            for name in ("ControlPara.dat","ControlPixel.dat","PicLenPara.dat")}
        if not all(record["geometryInvalidation"][name] for name in ("ControlPara.dat","ControlPixel.dat")):
            raise RuntimeError("Changing width did not invalidate device geometry or image data.")
        # A freshly recomputed length index may legitimately remain byte-identical when compression lengths coincide.
        record["lengthIndexPolicy"] = "Recomputed by a fresh compile; equality is allowed when compressed lengths coincide."
        overlap = copy.deepcopy(base)
        first,second = overlap["pages"][0]["objects"][1:3]
        second.update(x=first["x"],y=first["y"])
        for state in second["bitButton"]["states"]:
            state["transparent"] = True
        save(output/"dynamic-overlap.ksp",overlap)
        refused = cli(executable,["--compile-project",output/"dynamic-overlap.ksp",output/"overlap-rejected"],output,"dynamic-overlap-refusal",19)
        record["dynamicOverlapRefusal"] = refused
        record["passed"] = True
    except Exception as error:
        record["error"] = repr(error)
    record["identitiesUnchanged"] = sha(previous)==record["previousSha256"] and sha(executable)==record["executableSha256"]
    record["passed"] &= record["identitiesUnchanged"]
    record["completedAtUtc"] = now()
    save(output/"run.json",record)
    print({key:record.get(key) for key in ("passed","error","geometryInvalidation")})
    return 0 if record["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
