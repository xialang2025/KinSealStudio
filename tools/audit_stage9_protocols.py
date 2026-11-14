"""Freeze observed profile/driver identities without loading vendor code or opening ports."""
#20261114-8 add: Catalog visibility is distinct from every execution capability.
import argparse
import hashlib
import json
from pathlib import Path


def audit(catalog_path, installation, model):
    """Input: Explicit catalog, installation and model. Return: Bounded file identities and reference checks."""
    data = json.loads(catalog_path.read_text(encoding="utf-8-sig"))
    root = installation.resolve()
    cache = {}

    def identity(relative):
        """Input: An installation-relative reference. Return: Presence/hash, never executable behavior."""
        if not relative:
            return dict(path=None, present=False, referenceDeclared=False)
        path = (root / relative).resolve()
        if not path.is_relative_to(root):
            raise ValueError("Reference leaves the explicitly supplied installation")
        if relative not in cache:
            cache[relative] = dict(path=relative, present=path.is_file(), referenceDeclared=True)
            if path.is_file():
                raw = path.read_bytes()
                cache[relative].update(bytes=len(raw), sha256=hashlib.sha256(raw).hexdigest())
        return cache[relative].copy()

    selected = next(item for item in data["models"] if item["name"] == model)
    rows = []
    for profile in data["profiles"]:
        source = identity(profile["id"])
        observed = profile["driverFilesObserved"]
        row = dict(profileId=profile["id"], driverName=profile["driverName"],
                   source=source, expectedSourceSha256=profile["sourceSha256"],
                   sourceMatches=source.get("sha256") == profile["sourceSha256"],
                   modelDriverTree=selected["drivePath"],
                   modelApplicable=profile["id"].startswith(selected["drivePath"]),
                   windowsDll=identity(observed["windowsDll"]),
                   com1=identity(observed["com1"]), com2=identity(observed["com2"]),
                   serialDefaultsRaw=profile.get("serialDefaults", {}))
        row["historicalPresenceMatches"] = all(
            row[key]["present"] == bool(observed[key + "Present"])
            for key in ("windowsDll", "com1", "com2"))
        rows.append(row)
    return dict(schemaVersion=1, model=model, catalogSha256=hashlib.sha256(catalog_path.read_bytes()).hexdigest(),
                sourceVersion=data["provenance"]["sourceVersion"], installation=str(root),
                modelRecord=selected, profileCount=len(rows), rows=rows,
                sourceMatches=sum(row["sourceMatches"] for row in rows),
                historicalPresenceMatches=sum(row["historicalPresenceMatches"] for row in rows),
                modelApplicableProfiles=sum(row["modelApplicable"] for row in rows),
                uniqueObservedFiles=len(cache), method="read-only complete file hashing; no DLL execution",
                physicalEndpointUsed=False, hmiExecutionConfirmed=False)


def main():
    """Input: Explicit read-only target and two owned outputs. Return: Nonzero for changed profile sources."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--installation", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--resource", type=Path, required=True)
    args = parser.parse_args()
    result = audit(Path(__file__).resolve().parents[1] / "resources/plc-catalog.json",
                   args.installation, "AMT070W124-L800X480R")
    for path in (args.output, args.resource):
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({key: result[key] for key in ("profileCount", "sourceMatches", "historicalPresenceMatches",
                                                "modelApplicableProfiles", "uniqueObservedFiles")}, ensure_ascii=False))
    return 0 if result["sourceMatches"] == result["profileCount"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
