"""Encode the corroborated V122 alarm fields from an SM and the Lite inspector's field map."""
# 20261128-10 add
import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import re
import struct
import tempfile

BIT_BYTES = 38960
CONDITION_BYTES = 8880
MESSAGE_BYTES = 50


def read_fields(source, layout):
    """Read alarm inputs from Qt big-endian spans belonging to this specific SM."""
    if not layout.get("bodyValidated") or layout["sha256"] != hashlib.sha256(source).hexdigest():
        raise ValueError("the validated field map must belong to the supplied SM")
    values = {}
    prefixes = ("config.d0", "config.ec", "config.100", "config.104", "config.108",
                "config.124", "config.134", "config.138", "page.head.18", "page[")
    keys = {"header.1", "config.270.0", "system.164.count", "system.identity.15", "project.pages.count"}
    for field in layout["fields"]:
        name, kind = field["path"], field["kind"]
        if name not in keys and not name.startswith(prefixes):
            continue
        if name.startswith("page[") and not name.endswith("].head.18"):
            continue
        start, end = field["start"], field["end"]
        if name in values or not 0 <= start < end <= len(source):
            raise ValueError("invalid source field: " + name)
        raw = source[start:end]
        if kind == "q":
            size = int.from_bytes(raw[:4], "big")
            if len(raw) < 4 or (size if size != 0xffffffff else 0) != len(raw) - 4:
                raise ValueError("invalid native QString: " + name)
            values[name] = raw[4:].decode("utf-16-be")
        elif kind in ("a", "b", "s", "i"):
            if len(raw) != {"a": 1, "b": 1, "s": 2, "i": 4}[kind]:
                raise ValueError("invalid native integer: " + name)
            values[name] = int.from_bytes(raw, "big", signed=kind in ("i", "s"))
    if values["header.1"] != 122 or values["config.270.0"] != 0:
        raise ValueError("only unprotected V122 sources are supported by this inspector")
    return values


def threshold(value):
    """Encode finite C-locale decimal thresholds accepted by the native QString conversion."""
    text = value.strip()
    if text and not re.fullmatch(r"[+-]?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][+-]?[0-9]+)?", text):
        raise ValueError("unsupported threshold syntax")
    number = float(text or 0)
    if not math.isfinite(number):
        raise ValueError("unsupported non-finite threshold")
    return struct.pack("<d", number)


def encode_alarm(values, encoding="cp936"):
    """Return known file offsets and bytes; omit all unproven bytes in each language slot."""
    bits = values["config.ec.0"]
    groups = values["config.124.0"]
    languages = values["system.164.count"]
    if not (0 <= bits <= 511 and 0 <= groups <= 31 and 1 <= languages <= 8):
        raise ValueError("invalid native alarm table dimensions")
    if values["system.identity.15"] != languages:
        raise ValueError("project language declarations disagree")
    if bits and bits * languages != values["config.108.count"]:
        raise ValueError("incomplete bit alarm language rows")
    if groups and groups * languages != values["config.138.count"]:
        raise ValueError("incomplete condition alarm language rows")
    if bits and (bits != values["config.100.count"] or bits != values["config.104.count"]):
        raise ValueError("incomplete bit alarm page references")
    pages = {values["page.head.18"]} if values["project.pages.count"] else set()
    pages.update(values[f"page[{i}].head.18"] for i in range(1, values["project.pages.count"]))
    para = {0: bytes([bool(bits)]), 4: struct.pack("<H", bits), BIT_BYTES: bytes([bool(groups)])}
    messages = {}

    def write_message(text):
        """Record text and its terminator, bounded by the original 50-byte slot."""
        raw = text.encode(encoding)[:MESSAGE_BYTES]
        messages[len(messages) * MESSAGE_BYTES] = (raw + b"\0")[:MESSAGE_BYTES]
        return raw.ljust(MESSAGE_BYTES, b"\0")

    if bits:
        para[1] = bytes([values["config.ec.1"] & 255])
        para[3] = bytes([values["config.d0"] & 255])
        para[BIT_BYTES - 4] = struct.pack("<I", values["config.134.1"])
    for row in range(bits):
        start = 44 + 76 * row
        for source_key, offset in (("config.100", 2), ("config.104", 4)):
            page = values[f"{source_key}[{row}]"]
            # Unused jumps resolve to -1. Other names require an independent positive target fixture.
            if page == "未使用" and page not in pages:
                para[start + offset] = b"\xff\xff"
        for language in range(languages):
            prefix = f"config.108[{row * languages + language}]"
            text = write_message(values[prefix + ".1"])
            if language == 0:
                para[start] = bytes([values[prefix + ".0"]])
                para[start + 24] = text
    if groups:
        para[BIT_BYTES + 1] = bytes([groups, values["config.124.1"] & 255])
        para[BIT_BYTES + CONDITION_BYTES - 4] = struct.pack("<I", values["config.134.0"])
    for group in range(groups):
        for language in range(languages):
            for level in range(4):
                start = BIT_BYTES + 64 + group * 276 + level * 64
                prefix = f"config.138[{group * languages + language}][{level}]"
                text = write_message(values[prefix + ".2"])
                if language == 0:
                    para[start] = bytes([values[prefix + ".1"], values[prefix + ".0"] & 255])
                    para[start + 4] = threshold(values[prefix + ".3"])
                    para[start + 12] = text
    return para, messages


def write_messages(path, slots):
    """Publish a complete language table only if every byte of every slot is known."""
    if any(len(part) != MESSAGE_BYTES for part in slots.values()):
        raise ValueError("at least one short message has unresolved native trailing bytes")
    temp = None
    try:
        with tempfile.NamedTemporaryFile(dir=path.parent, delete=False) as out:
            temp = out.name
            for offset in sorted(slots):
                out.write(slots[offset])
        os.replace(temp, path)
    finally:
        if temp is not None:
            Path(temp).unlink(missing_ok=True)


def main():
    """Print the known ranges or publish a fully determined AlarmMuti.dat."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("layout", type=Path, help="JSON produced by --inspect-native-source for the same SM")
    parser.add_argument("--encoding", default="cp936", help="the original compiler's Windows code page")
    parser.add_argument("--write-complete-messages", type=Path)
    args = parser.parse_args()
    source = args.source.read_bytes()
    para, messages = encode_alarm(read_fields(source, json.loads(args.layout.read_bytes())), args.encoding)
    if args.write_complete_messages:
        if args.write_complete_messages.name.lower() != "alarmmuti.dat":
            raise ValueError("the output must be named AlarmMuti.dat")
        if args.write_complete_messages.resolve() in (args.source.resolve(), args.layout.resolve()):
            raise ValueError("the output must be separate from both inputs")
        write_messages(args.write_complete_messages, messages)
    print(json.dumps({name: {"length": size, "known": sum(map(len, spans.values())),
                             "fields": {offset: data.hex() for offset, data in sorted(spans.items())}}
                      for name, size, spans in (("AlarmPara.dat", BIT_BYTES + CONDITION_BYTES, para),
                                                ("AlarmMuti.dat", len(messages) * MESSAGE_BYTES, messages))}))


if __name__ == "__main__":
    try:
        main()
    except (ValueError, LookupError, OSError) as exc:
        raise SystemExit(str(exc))
