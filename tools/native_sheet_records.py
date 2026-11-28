"""Build ordinary V122 SheetPara records from SM fields and the compiler's per-page control records."""
import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import struct
import subprocess
import tempfile


def read_page_fields(exe, path):
    """Read the native parser's verified field boundaries, then decode the original source scalars."""  # 20261128-10 add
    source = path.read_bytes()
    result = subprocess.run([str(exe), "--inspect-native-source", str(path)], check=True, capture_output=True,
                            timeout=120, env=dict(os.environ, QT_QPA_PLATFORM="offscreen"),
                            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
    index = json.loads(result.stdout)
    if not index["bodyValidated"] or index["sha256"] != hashlib.sha256(source).hexdigest():
        raise ValueError("inspection does not describe this source")
    spans = {field["path"]: field for field in index["fields"]}

    def field(name):
        span = spans[name]
        data = source[span["start"]:span["end"]]
        if span["kind"] == "q":
            length, = struct.unpack_from(">I", data)
            if length == 0xffffffff and len(data) == 4:
                return ""
            if length != len(data) - 4:
                raise ValueError("string length differs from the indexed field")
            return data[4:].decode("utf-16-be")
        return struct.unpack({"d": ">d", "i": ">i", "a": ">b", "b": ">B"}[span["kind"]], data)[0]

    if field("header.1") != 122 or field("config.270.0") != 0:
        raise ValueError("expected an unprotected V122 project")
    pages = []
    count = field("project.pages.count")
    if not 1 <= count <= 200:
        raise ValueError("unsupported ordinary page count")
    for i in range(count):
        prefix = "page" if i == 0 else f"page[{i}]"
        if field(prefix + ".head.17") != 0 or field(prefix + ".tail.1") != 0:
            raise ValueError("only ordinary pages with inactive page actions are supported")
        number = field(prefix + ".head.19")
        if not number.isascii() or not number.isdecimal() or int(number) > 255:
            raise ValueError("expected an ordinary page number in the supported byte range")
        pages.append(dict(number=int(number), name=field(prefix + ".head.18"),
                          width=field(prefix + ".head.2"), height=field(prefix + ".head.3"),
                          mode=field(prefix + ".tail.13"), interval=field(prefix + ".tail.2")))
    return pages


def encode_sheet(page, controls, encoding):
    """Encode one page with deterministic zeroes in the two alignment bytes and four inactive indices."""
    count, rest = divmod(len(controls), 640)
    if count > 255 or rest:
        raise ValueError("expected complete 640-byte control records for this page")
    raw_size = sum(struct.unpack_from("<I", controls, i + 16)[0] for i in range(0, len(controls), 640))
    sizes = (page["width"], page["height"])
    if any(not math.isfinite(v) or not 1 <= v < 65535.5 for v in sizes) or raw_size > 0xffffffff:
        raise ValueError("page geometry or picture total exceeds the record")
    # The native writer copies a counted local-code-page byte string. Preserve a split final multibyte character.
    name = page["name"].encode(encoding)[:40]
    record = bytearray(72)
    record[1] = page["mode"] & 0xff
    record[2] = count
    record[4] = page["number"]
    record[7:7 + len(name)] = name
    struct.pack_into("<HH", record, 48, *(int(size + 0.5) for size in sizes))
    struct.pack_into("<H", record, 56, page["interval"] & 0xffff)
    struct.pack_into("<4hI", record, 60, -1, -1, -1, -1, raw_size)
    return bytes(record)


def main():
    """Write a fresh page table using one ordered control block (or an explicit empty block) per page."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--page-controls", action="append", required=True, metavar="FILE|-",
                        help="control records emitted for one page, in source page order; '-' means no controls")
    parser.add_argument("--encoding", default="cp936", help="local code page of the original compiler")
    args = parser.parse_args()
    pages = read_page_fields(args.exe.resolve(), args.source.resolve())
    if len(args.page_controls) != len(pages):
        parser.error("supply the control block for every ordinary page")
    result = bytearray()
    for page, name in zip(pages, args.page_controls):
        result.extend(encode_sheet(page, b"" if name == "-" else Path(name).read_bytes(), args.encoding))
    # Publish a complete new table. Both supported operations refuse an existing destination atomically.
    with tempfile.NamedTemporaryFile(dir=args.output.parent, delete=False) as output:
        temporary = Path(output.name)
    try:
        temporary.write_bytes(result)
        if os.name == "nt":
            temporary.rename(args.output)
        else:
            os.link(temporary, args.output)
    finally:
        temporary.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
