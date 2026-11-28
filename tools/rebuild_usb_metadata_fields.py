"""Rebuild the documented fields of a KinSeal V5.1 Usbload.bin without reading that file."""
# 20261128-6 add
import argparse
import json
import math
from pathlib import Path
import struct
from datetime import datetime


class Stream:
    """Read the bounded big-endian Qt 4.8 metadata used by load.bin."""
    def __init__(self, data):
        self.data = data
        self.pos = 0

    def take(self, size):
        if size < 0 or size > len(self.data) - self.pos:
            raise ValueError("Truncated load.bin")
        value = self.data[self.pos:self.pos + size]
        self.pos += size
        return value

    def integer(self):
        return int.from_bytes(self.take(4), "big", signed=True)

    def count(self, maximum=4096):
        count = self.integer()
        if not 0 <= count <= maximum:
            raise ValueError("Invalid load.bin collection count")
        return count

    def string(self):
        size = self.integer()
        if size == -1:
            return ""
        if not 0 <= size <= 8192 or size % 2:
            raise ValueError("Invalid Qt string length")
        return self.take(size).decode("utf-16be")


def read_metadata(data):
    """Read V122 project/compiler inputs independently of the USB descriptor."""
    r = Stream(data)
    signature = r.string()
    if signature != "KinSealStudio-V1.0.2" or r.integer() != 122:
        raise ValueError("Unsupported load.bin signature or version")
    model, count = r.string(), r.count(200)
    if not model or count < 1:
        raise ValueError("Missing panel or ordinary pages")
    startup, image, flags = r.take(1)[0], r.string(), r.take(4)
    if any(flag > 1 for flag in flags):
        raise ValueError("Invalid boolean")
    macros = r.count()
    if macros != r.count():
        raise ValueError("Conflicting macro counts")
    for _ in range(macros):
        r.string()
    drivers = [r.string() for _ in range(r.count(4))]
    for _ in range(r.count()):
        r.string()
        r.string()
    panel, option = r.integer(), r.string()
    r.integer()  # The Chinese-library download option belongs to the media descriptor.
    connections, messages = r.integer(), r.integer()
    if r.count(200) != count:
        raise ValueError("Conflicting page counts")
    sizes = []
    for _ in range(count):
        width, height = struct.unpack(">dd", r.take(16))
        if not all(math.isfinite(v) and 0 < v <= 65536 for v in [width, height]) or 2 * width * height > 2147483647:
            raise ValueError("Unsupported native page size")
        sizes.append(int(2 * width * height))
    history = r.integer()
    if r.pos != len(data) or min(history, connections, messages, panel) < 0:
        raise ValueError("Invalid trailing metadata")
    return dict(signature=signature, model=model, count=count, startup=startup, image=image, flags=flags,
                macros=macros, drivers=drivers, panel=panel, option=option, connections=connections,
                messages=messages, sizes=sizes, history=history)


LENGTH_FILES = ("sheetpixel.dat", "controlpixel.dat", "addrpara.dat", "piclenpara.dat")


def native_seed_word(seconds):  # 20261128-18 change
    """Encode the native Windows Qt4 qrand result for one known local second."""
    if not 0 <= seconds < 86400:
        raise ValueError("Seconds must be local seconds after midnight")
    return struct.pack("<I", ((214013 * seconds + 2531011) >> 16) & 32767)


def build_fields(metadata, lengths, seconds=None, encoding="cp936"):
    """Return (offset, byte string) fragments. Omitted bytes have no zero-fill contract."""
    m = read_metadata(metadata)
    flags = m["flags"]
    fields = {0: bytes((m["startup"], flags[2], flags[1] | (flags[0] << 1), flags[3]))}
    if seconds is not None:
        fields[4] = native_seed_word(seconds)
    fields[8] = struct.pack("<7I", m["messages"], 123, m["count"], m["macros"],
                            m["panel"], m["connections"], m["history"])
    for offset, width, key in [(0x42, 10, "model"), (0x4c, 10, "option"),
                                (0x56, 32, "signature"), (0x76, 128, "image")]:
        fields[offset] = m[key].encode(encoding)[:width].ljust(width, bytes(1))
    fields[0xf8] = struct.pack("<4I", *(lengths.get(name, 0) for name in LENGTH_FILES))
    for i in range(4):
        offset = 0x130 + 50 * i
        if i >= len(m["drivers"]):
            fields[offset] = bytes(50)
            continue
        path = m["drivers"][i].replace("\\", "/")
        name = path.rsplit("/", 1)[-1].encode(encoding)
        if not name or len(name) > 50 or bytes(1) in name:
            raise ValueError("Unsupported native driver slot")
        if len(name) >= 19:
            fields[offset] = name.ljust(50, bytes(1))
        else:
            # A short active slot has a defined C string and zero tail; the gap has no stable content.
            fields[offset] = name + bytes(1)
            fields[offset + 20] = bytes(30)
    fields[0x388] = struct.pack("<" + "I" * len(m["sizes"]), *m["sizes"])
    return fields


# 20261128-18 add
def earliest_native_seed_time(system, load_dos_time):
    """Estimate the later USB clock from the earlier system clock and preceding load.bin write."""
    if len(system) != 4998:
        raise ValueError("Expected a V122 SysPara.dat")
    second, minute, hour, weekday, day, month, year = struct.unpack_from("<7I", system, 324)
    system_time = datetime(year, month, day, hour, minute, second)
    if system_time.isoweekday() != weekday:
        raise ValueError("Inconsistent native weekday")
    return max(system_time, datetime(*load_dos_time))


def render_descriptor(metadata, lengths, seconds, encoding="cp936", prior=None):
    """Render a disk comparison specimen using explicit seconds and optional earlier-build context."""
    required = {"sheetpixel.dat", "piclenpara.dat", "addrpara.dat"}
    if seconds is None or not required <= lengths.keys():
        raise ValueError("A file specimen needs the clock and three mandatory resource lengths")
    if prior is not None and len(prior) != 1704:
        raise ValueError("Expected a 1704-byte earlier-build descriptor")
    data = bytearray(prior) if prior is not None else bytearray(1704)
    for start, field in build_fields(metadata, lengths, seconds, encoding).items():
        data[start:start + len(field)] = field
    return bytes(data)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path, help="Directory containing independent compiler parts")
    parser.add_argument("--load-metadata", type=Path, help="Use a separately generated load.bin")
    clock = parser.add_mutually_exclusive_group()
    clock.add_argument("--seconds", type=int, help="Known local compiler time in seconds after midnight")
    clock.add_argument("--preceding-load-time", type=datetime.fromisoformat, metavar="YYYY-MM-DDTHH:MM:SS",
                       help="Predict the USB clock from SysPara.dat and the earlier load.bin's local DOS timestamp")
    parser.add_argument("--encoding", default="cp936", help="Native Windows ANSI encoding")
    output = parser.add_mutually_exclusive_group()
    output.add_argument("--output", type=Path, help="Write the known byte ranges as JSON")
    output.add_argument("--comparison-bin", type=Path, help="Create a new 1704-byte offline comparison specimen; uncovered bytes default to zero")
    parser.add_argument("--prior-descriptor", type=Path, help="In comparison mode retain uncovered bytes from an explicitly supplied earlier build")
    options = parser.parse_args()
    directory = {p.name.lower(): p for p in options.directory.iterdir() if p.is_file()}
    lengths = {name: directory[name].stat().st_size for name in LENGTH_FILES if name in directory}
    data = (options.load_metadata or directory["load.bin"]).read_bytes()
    if options.preceding_load_time is not None:
        timestamp = options.preceding_load_time
        if timestamp.tzinfo is not None or timestamp.microsecond or timestamp.second % 2 or "syspara.dat" not in directory:
            parser.error("--preceding-load-time requires an even local DOS second and SysPara.dat")
        estimate = earliest_native_seed_time(directory["syspara.dat"].read_bytes(), timestamp.timetuple()[:6])
        options.seconds = estimate.hour * 3600 + estimate.minute * 60 + estimate.second
    if options.prior_descriptor and options.comparison_bin is None:
        parser.error("--prior-descriptor requires --comparison-bin")
    if options.comparison_bin is not None:
        if options.comparison_bin.suffix.lower() != ".bin":
            parser.error("--comparison-bin requires a new .bin filename")
        prior = options.prior_descriptor.read_bytes() if options.prior_descriptor else None
        result = render_descriptor(data, lengths, options.seconds, options.encoding, prior)
        from stage6_media_bundle import _ordinary_path, _publish_archive_bytes
        target = options.comparison_bin.absolute()
        _publish_archive_bytes(target, result, _ordinary_path(target.parent))
        print(json.dumps({"bytes": len(result), "uncoveredSource": "prior-build" if prior is not None else "zero",
                          "seedSource": "estimated" if options.preceding_load_time else "explicit", "seconds": options.seconds}))
        return
    fields = build_fields(data, lengths, options.seconds, options.encoding)
    estimated = fields.pop(4) if options.preceding_load_time is not None else None
    result = {"format": "Usbload.bin", "size": 1704, "knownBytes": sum(map(len, fields.values())),
              "ranges": [{"offset": offset, "hex": value.hex()} for offset, value in sorted(fields.items())]}
    if estimated is not None:
        following = native_seed_word((options.seconds + 1) % 86400)
        result["estimatedClock"] = {"offset": 4, "hex": estimated.hex(), "seconds": options.seconds,
                                    "nextSecondHex": following.hex(),
                                    "commonOffsets": [4 + i for i in range(4) if estimated[i] == following[i]]}
    text = json.dumps(result, indent=2)
    if options.output:
        if options.output.suffix.lower() != ".json":
            parser.error("The output is a field map and requires a .json filename")
        options.output.write_text(text + "\n", encoding="utf8")
    else:
        print(text)


if __name__ == "__main__":
    main()
