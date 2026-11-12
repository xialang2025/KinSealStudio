"""Independently decode retained QR screenshots and fresh product module rasters with OpenCV."""
#20261112-0 add: Decoder output is an offline observation, not a physical phone or HMI acceptance result.
import argparse
import hashlib
import json
from pathlib import Path
import cv2
import numpy as np


def decode(path, expected):
    """Input: Explicit PNG path and expected text. Return: Source identity, corners and independently decoded payload."""
    data = path.read_bytes()
    raster = cv2.imdecode(np.frombuffer(data, dtype=np.uint8), cv2.IMREAD_COLOR)
    # Upscale tiny one-pixel modules only for this decoder; the original fixture bytes remain untouched.
    if raster.shape[0] < 180:
        raster = cv2.resize(raster, None, fx=8, fy=8, interpolation=cv2.INTER_NEAREST)
    text, corners, modules = cv2.QRCodeDetector().detectAndDecode(raster)
    if text != expected:
        raise AssertionError((str(path), expected, text))
    return dict(path=path.as_posix(), sha256=hashlib.sha256(data).hexdigest(), decodedText=text,
                modules=int(modules.shape[0]), corners=corners.tolist(), expectedMatched=True)


def main():
    """Input: A fresh test output directory. Return: Successful decodes, retained as evidence alongside its exact rasters."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("run", type=Path)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    cases = [(args.run / "qr-scan-ascii.png", "HTTPS://KINSEAL.TEST/STAGE4"),
             (args.run / "qr-scan-chinese.png", "金玺测试"),
             (args.run / "qr-scan-empty.png", "http://www.kinseal.com/")]
    cases += [(root / "docs/stage4-evidence/qrcode/native" / name, "http://www.kinseal.com/")
              for name in ("qrcode-default-r1.png", "qrcode-chinese-ascii-r1.png", "qrcode-chinese-gb2312-r1.png")]
    results = [decode(path, expected) for path, expected in cases]
    record = dict(decoder="OpenCV QRCodeDetector", version=cv2.__version__, passed=True, records=results,
                  scope="离线QR载荷与默认点阵核验；不代表设备编码、手机实扫或中文原厂textEdited路径已通过")
    (args.run / "qr-decoding.json").write_text(json.dumps(record, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(dict(passed=True, decoded=len(results)), ensure_ascii=False))


if __name__ == "__main__":
    main()
