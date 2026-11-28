/******************************************************************************
 * Function Name   : packNativePictureFrame
 * Description     : Pack an opaque native-size PNG or a solid scaled PNG into rows of RGB565.
 * Input Parameters: source PNG, destination size, packed frame and preview outputs, error
 * Return Value    : True for a representable tightly packed frame.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool packNativePictureFrame(const QByteArray &png, const QSize &size, QByteArray *frame,
                                   QImage *preview, QString *error) //20261128-21 add
{
    QImage source = decodedControlPng(png, error);
    QImage image;
    QRgb first;
    QRgb pixel;
    u16 color;
    int x;
    int y;
    if (source.isNull()) { return false; }
    source = source.convertToFormat(QImage::Format_ARGB32);
    first = source.pixel(0, 0);
    for (y = 0; y < source.height(); ++y) {
        const QRgb *row = reinterpret_cast<const QRgb *>(source.constScanLine(y));
        for (x = 0; x < source.width(); ++x) {
            if (qAlpha(row[x]) != 255 || (source.size() != size && row[x] != first)) {
                if (error) { *error = QStringLiteral("图片设备编码支持原像素大小的不透明 PNG 和可缩放的纯色 PNG；透明及彩色缩放尚待原厂对照。"); }
                return false;
            }
        }
    }
    image = QImage(size, QImage::Format_RGB16);
    frame->resize(size.width() * size.height() * 2);
    for (y = 0; y < size.height(); ++y) {
        u16 *row = reinterpret_cast<u16 *>(image.scanLine(y));
        for (x = 0; x < size.width(); ++x) {
            pixel = source.size() == size ? source.pixel(x, y) : first;
            color = u16((qRed(pixel) >> 3) << 11 | (qGreen(pixel) >> 2) << 5 | (qBlue(pixel) >> 3));
            row[x] = color;
            // QImage rows have four-byte alignment; the file contains no per-line padding.
            qToLittleEndian<u16>(color, frame->data() + 2 * (y * size.width() + x));
        }
    }
    *preview = image;
    return true;
}

/******************************************************************************
 * Function Name   : buildNativePicturePayload
 * Description     : Encode type 15 or 34 and their ordered PNG sprites. Type 34 leaves the page background untouched.
 * Input Parameters: validated object, address compiler, raw offset, record, sprites, first image, error
 * Return Value    : True after packing every state and validating its raw range.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool buildNativePicturePayload(const HmiObject &object, const DeviceAddressCompiler &compiler, u32 rawOffset,
                                      QByteArray *record, BitButtonSpriteSet *sprites, QImage *firstImage,
                                      QString *error) //20261128-21 add
{
    const ImageDisplayProperties &picture = object.imageDisplay;
    const bool gif = object.type == QStringLiteral("gif");
    const u16 condition = picture.condition == 1 ? 2 : picture.condition == 2 ? 1 : 0;
    QList<QByteArray> frames;
    u32 rawSize;
    QByteArray encoded(640, char(0));
    BitButtonSpriteSet result;
    QByteArray address;
    QByteArray frame;
    QByteArray compressed;
    QByteArray index(12, char(0));
    QImage image;
    QImage initial;
    u32 padded;

    if (gif) {
        if (!nativeGifPngFrames(object.gifDisplay.gifData, &frames)) {
            if (error) { *error = QStringLiteral("设备动画需要原厂 PNG 帧表。"); }
            return false;
        }
    } else {
        for (const ImageDisplayState &state : picture.states) {
            if (state.picturePng.isEmpty()) {
                if (error) { *error = QStringLiteral("图片显示的设备帧需使用工程 PNG，系统 KVG 的缩放待核实。"); }
                return false;
            }
            frames.append(state.picturePng);
        }
    }
    rawSize = u32(object.geometry.width() * object.geometry.height() * 2) * u32(frames.size());
    if (quint64(rawOffset) + rawSize > 0xffffffffULL) {
        if (error) { *error = QStringLiteral("图片的解压像素累计长度超出32位字段。"); }
        return false;
    }
    encoded[0] = gif ? 34 : 15;
    encoded[3] = 1;
    qToLittleEndian<u16>(u16(object.geometry.x()), encoded.data() + 4);
    qToLittleEndian<u16>(u16(object.geometry.y()), encoded.data() + 6);
    qToLittleEndian<u16>(u16(object.geometry.width()), encoded.data() + 8);
    qToLittleEndian<u16>(u16(object.geometry.height()), encoded.data() + 10);
    qToLittleEndian<u32>(rawOffset, encoded.data() + 12);
    qToLittleEndian<u32>(rawSize, encoded.data() + 16);
    if (gif) {
        qToLittleEndian<u16>(u16(frames.size()), encoded.data() + 0x100);
        qToLittleEndian<u16>(u16(object.gifDisplay.bitControl), encoded.data() + 0x108);
        qToLittleEndian<u16>(object.gifDisplay.bitControl ? u16(object.gifDisplay.activeState) : u16(0xff00), encoded.data() + 0x10a);
    } else {
        qToLittleEndian<u16>(u16(picture.displayType), encoded.data() + 0x100);
        qToLittleEndian<u16>(condition, encoded.data() + 0x102);
        qToLittleEndian<u16>(u16(picture.stateCount), encoded.data() + 0x106);
        qToLittleEndian<u16>(u16(picture.interval), encoded.data() + 0x10a);
    }
    encoded[0x10d] = 'B'; // Original time and single-picture controls retain an inactive local bit slot.
    if (gif ? object.gifDisplay.bitControl : (picture.conditionEnabled && condition != 0)) {
        if (!compiler.encode(gif ? object.gifDisplay.controlAddress : picture.monitorAddress,
                             !gif && condition == 2, false, &address, error)) { return false; }
        if (address.size() != 36 || address.at(0) != 0) {
            if (error) { *error = QStringLiteral("图片显示的设备监视仅核实内部 LB / LW。"); }
            return false;
        }
        encoded.replace(0x10c, 36, address);
    }
    if (gif) { encoded[0x10f] = 1; } // Type 34 uses the same local bit tag for gated and continuous playback.
    for (const QByteArray &png : frames) {
        if (!packNativePictureFrame(png, object.geometry.size(), &frame, &image, error) ||
            !compressRgb565Pixels(frame, &compressed, error)) { return false; }
        if (initial.isNull()) { initial = image; }
        padded = (u32(compressed.size()) + 31U) & ~31U;
        qToLittleEndian<u32>(padded, index.data() + 4);
        qToLittleEndian<u32>(u32(compressed.size()), index.data() + 8);
        result.lengthRecords.append(index);
        result.pixelData.append(compressed);
        result.pixelData.append(QByteArray(padded - compressed.size(), char(0)));
    }
    result.rawLength = rawSize;
    *record = encoded;
    *sprites = result;
    *firstImage = gif ? QImage() : initial;
    return true;
}
