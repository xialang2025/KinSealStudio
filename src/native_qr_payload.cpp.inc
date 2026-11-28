/******************************************************************************
 * Function Name   : nativeQrGalleryImage
 * Description     : Decode the original gallery's embedded grayscale PNG at its native resolution.
 * Input Parameters: None
 * Return Value    : The QR startup artwork, or a null image if the resource cannot be read.
 * Date            : 2026.11.28
 ******************************************************************************/
static QImage nativeQrGalleryImage() //20261128-26 add
{
    QFile file(QStringLiteral(":/system-gallery/default/KinSeal_QuickMark.kvg"));
    QXmlStreamReader xml;
    QString href;
    const QString prefix = QStringLiteral("data:image/png;base64,");
    if (!file.open(QIODevice::ReadOnly)) { return {}; }
    xml.setDevice(&file);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == QStringLiteral("image")) {
            href = xml.attributes().value(QStringLiteral("http://www.w3.org/1999/xlink"), QStringLiteral("href")).toString();
            if (href.startsWith(prefix)) {
                return decodedControlPng(QByteArray::fromBase64(href.sliced(prefix.size()).toLatin1()))
                    .convertToFormat(QImage::Format_Grayscale8);
            }
        }
    }
    return {};
}

/******************************************************************************
 * Function Name   : packNativeQrFrame
 * Description     : Scale the gallery image with the original writer's fixed-point bilinear sampler.
 * Input Parameters: output size and RGB565 byte buffer
 * Return Value    : True when all pixels were packed, excluding any scan-line padding.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool packNativeQrFrame(const QSize &size, QByteArray *frame) //20261128-26 add
{
    static const QImage source = nativeQrGalleryImage();
    const int width = size.width(), height = size.height();
    const qint64 sourceX = qint64(source.width()) << 16, sourceY = qint64(source.height()) << 16;
    qint64 stepX, stepY, startX, posX, posY;
    int x, y, cellX, cellY, left, right, fx, fy, upper, lower, gray;
    const u8 *top;
    const u8 *bottom;
    u16 pixel;
    if (source.isNull() || width <= 0 || height <= 0 || qint64(width) * height > 0x3fffffff) {
        return false;
    }
    stepX = sourceX / width;
    stepY = sourceY / height;
    startX = sourceX / (2LL * width) - 32768;
    posY = sourceY / (2LL * height) - 32768;
    frame->resize(qsizetype(width) * height * 2);
    for (y = 0; y < height; ++y, posY += stepY) {
        cellY = int(posY >> 16);
        fy = int((posY >> 8) & 255);
        top = source.constScanLine(qBound(0, cellY, source.height() - 1));
        bottom = source.constScanLine(qBound(0, cellY + 1, source.height() - 1));
        for (x = 0, posX = startX; x < width; ++x, posX += stepX) {
            cellX = int(posX >> 16);
            fx = int((posX >> 8) & 255);
            left = qBound(0, cellX, source.width() - 1);
            right = qBound(0, cellX + 1, source.width() - 1);
            // Truncating each horizontal blend before the vertical blend reproduces the native RGB565 thresholds.
            upper = (top[left] * (256 - fx) + top[right] * fx) >> 8;
            lower = (bottom[left] * (256 - fx) + bottom[right] * fx) >> 8;
            gray = (upper * (256 - fy) + lower * fy) >> 8;
            pixel = u16(((gray >> 3) << 11) | ((gray >> 2) << 5) | (gray >> 3));
            qToLittleEndian<u16>(pixel, frame->data() + 2 * (y * width + x));
        }
    }
    return true;
}

/******************************************************************************
 * Function Name   : buildNativeQrPayload
 * Description     : Emit the type-52 local-word monitor, CP936 description, and one startup frame.
 * Input Parameters: object, prepared address compiler, raw offset, control record, sprites, error
 * Return Value    : True after every output is complete.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool buildNativeQrPayload(const HmiObject &object, const DeviceAddressCompiler &addressCompiler, u32 rawOffset,
                                 QByteArray *record, BitButtonSpriteSet *sprites, QString *error) //20261128-26 add
{
    const QrCodeProperties &qr = object.qrCode;
    const QRect bounds = object.geometry;
    QByteArray encoded(640, char(0)), index(12, char(0));
    QByteArray description, address, frame, compressed;
    BitButtonSpriteSet result;
    u32 padded;
    if (!qr.validate(error) ||
        !addressCompiler.encode(qr.monitorAddress, true, false, &address, error,
                                u32(QrCodeProperties::monitorAddressSpan(qr.characterLength)))) { return false; }
    if (address.size() != 36 || address.at(0) != 0 || address.at(1) != 'W' ||
        qr.visibility.visibilityEnabled || object.hasAppearanceFlip()) {
        if (error) { *error = QStringLiteral("二维码设备编译仅核实内部 LW 监视、关闭显现且未翻转的画面。"); }
        return false;
    }
    if ((!qr.description.isEmpty() && !encodeNativeCp936(qr.description, &description)) ||
        description.size() > 255 || description.contains(char(0))) {
        if (error) { *error = QStringLiteral("二维码说明须可用 CP936 表示，最多 255 字节，且不能含空字符。"); }
        return false;
    }
    if (!packNativeQrFrame(bounds.size(), &frame) || quint64(rawOffset) + quint64(frame.size()) > 0xffffffffULL) {
        if (error) { *error = QStringLiteral("二维码图库图像读取失败或像素范围越界。"); }
        return false;
    }
    if (!compressRgb565Pixels(frame, &compressed, error)) { return false; }
    encoded[0] = 52;
    encoded[3] = 6;
    qToLittleEndian<u16>(u16(bounds.x()), encoded.data() + 4);
    qToLittleEndian<u16>(u16(bounds.y()), encoded.data() + 6);
    qToLittleEndian<u16>(u16(bounds.width()), encoded.data() + 8);
    qToLittleEndian<u16>(u16(bounds.height()), encoded.data() + 10);
    qToLittleEndian<u32>(rawOffset, encoded.data() + 12);
    qToLittleEndian<u32>(u32(frame.size()), encoded.data() + 16);
    encoded.replace(0x104, address.size(), address);
    encoded[0x107] = 1; // The original assigns this same tag for all six local-word variants.
    encoded.replace(0x128, description.size(), description);
    qToLittleEndian<u32>(u32(qr.characterLength), encoded.data() + 0x228);
    encoded[0x22c] = char(qr.swapBytes);
    encoded[0x22d] = char(qr.encoding);
    padded = (u32(compressed.size()) + 31U) & ~31U;
    qToLittleEndian<u32>(padded, index.data() + 4);
    qToLittleEndian<u32>(u32(compressed.size()), index.data() + 8);
    result.rawLength = u32(frame.size());
    result.lengthRecords = index;
    result.pixelData = compressed + QByteArray(padded - compressed.size(), char(0));
    *record = encoded;
    *sprites = result;
    return true;
}
