/******************************************************************************
 * Function Name   : nativeMessageInterior
 * Description     : Resolve the V122 background corners and the independent text bounds from the actual edited source.
 * Input Parameters: single-message project, device bounds, interior rectangle, diagnostic
 * Return Value    : True when both source children agree on an integral rectangle inside the control.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool nativeMessageInterior(const HmiProject &project, const QRect &outer, QRect *interior, QString *error) //20261128-24 add
{
    QByteArray source;
    QList<NativeSourceFieldSpan> fields;
    QHash<QString, double> values;
    const QString back = QStringLiteral("object[0].child[1].rectangle.");
    const QString text = QStringLiteral("object[0].child[2].text.head.");
    const QString pos = QStringLiteral("object[0].child[2].text.tail.");
    QRectF background;
    QRectF bounds;
    double number;
    if (!exportNativeProjectSource(project, &source, error) || !inspectNativeProjectSourceFields(source, &fields, error)) {
        return false;
    }
    for (const NativeSourceFieldSpan &field : fields) {
        if (field.kind == 'd' && (field.path.startsWith(back) || field.path.startsWith(text) || field.path.startsWith(pos))) {
            QDataStream stream(source.mid(field.start, field.end - field.start));
            stream >> number;
            if (stream.status() != QDataStream::Ok || !qIsFinite(number) || qAbs(number) > 32768) {
                if (error) { *error = QStringLiteral("消息子图元的坐标无效。"); }
                return false;
            }
            values.insert(field.path, number);
        }
    }
    // The rectangle child stores two opposite local corners; the text child stores x, y, width and height.
    const auto field = [&values](const QString &prefix, int index) {
        return values.value(prefix + QString::number(index), qQNaN());
    };
    background = QRectF(QPointF(field(back, 0), field(back, 1)), QPointF(field(back, 2), field(back, 3)));
    background.translate(field(back, 9), field(back, 10));
    bounds = QRectF(field(text, 0) + field(pos, 10), field(text, 1) + field(pos, 11),
                    field(text, 2), field(text, 3));
    if (!bounds.isValid() || background != bounds || !QRectF(outer).contains(bounds) || QRectF(bounds.toRect()) != bounds) {
        if (error) { *error = QStringLiteral("消息设备底图需要位于框内、互相吻合的整数背景和文字矩形。"); }
        return false;
    }
    *interior = bounds.toRect();
    return true;
}

/******************************************************************************
 * Function Name   : buildNativeMessagePayload
 * Description     : Encode a single local type-35 control and its text-free RGB565 substrate for offline compilation.
 * Input Parameters: control, complete project, address compiler, raw offset, control/frame/page outputs, diagnostic
 * Return Value    : True after verifying the source geometry and constructing all measured active fields.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool buildNativeMessagePayload(const HmiObject &object, const HmiProject &project, const DeviceAddressCompiler &compiler,
                                      u32 rawOffset, QByteArray *record, BitButtonSpriteSet *sprites, QImage *pagePreview,
                                      QString *error) //20261128-24 add
{
    const MessageDisplayProperties &message = object.messageDisplay;
    const StaticTextProperties &gate = message.appearance;
    const QRect bounds = object.geometry;
    const QStringList &languages = project.languages;
    const QList<int> nativeSizes = {12, 16, 24, 48, 64, 96, 192};
    const u32 rawSize = u32(bounds.width()) * u32(bounds.height()) * 2U;
    QByteArray encoded(640, char(0));
    QByteArray address;
    QByteArray raw;
    QByteArray compressed;
    QByteArray index(12, char(0));
    BitButtonSpriteSet result;
    QRect interior;
    QRect local;
    u32 padded;
    int x, y, language;
    if (project.objects.size() != 1 || object.type != QStringLiteral("message") || languages.isEmpty() || languages.size() > 6 ||
        languages.size() * message.stateCount * 104 > 65535 ||
        bounds.isEmpty() || !QRect(0, 0, 800, 480).contains(bounds) || quint64(rawOffset) + rawSize > 0xffffffffULL ||
        object.hasAppearanceFlip() || !object.subpixelOffset.isNull() || (gate.visibilityEnabled && gate.wordEnabled)) {
        if (error) { *error = QStringLiteral("消息设备记录支持单控件、最多六种语言、65535字节内的消息表、整数布局和本地位或等级显现。"); }
        return false;
    }
    for (language = 0; language < languages.size(); ++language) {
        const QString name = languages.at(language);
        const StaticTextProperties style = message.languageAppearance(name);
        if (!nativeSizes.contains(int(style.fontSize)) || style.fontSize != int(style.fontSize) || style.fontFamily != QStringLiteral("宋体") ||
            style.style != 1 || !style.textColor.isValid() || style.foregroundColor != QColor(Qt::white) ||
            style.backgroundColor != QColor(Qt::white) || message.languageFrameColor(name) != QColor(224, 224, 224)) {
            if (error) { *error = QStringLiteral("消息设备外观支持原厂宋体字号、RGB文字、白色纯色底和灰色边框。"); }
            return false;
        }
        qToLittleEndian<u32>(u32(style.fontSize), encoded.data() + 0x144 + 4 * language);
    }
    if (!nativeMessageInterior(project, bounds, &interior, error) ||
        !compiler.encode(message.monitorAddress, true, false, &address, error, message.dataType == 1 ? 2 : 1)) {
        return false;
    }
    if (address.size() != 36 || address.at(0) != 0 || address.at(1) != 'W') {
        if (error) { *error = QStringLiteral("消息设备监视仅核实本地 LW 字地址。"); }
        return false;
    }
    // The word address is followed by the runtime value cache. Only the local tag and the register index are live here.
    encoded[0x121] = 'W';
    encoded.replace(0x128, 4, address.constData() + 8, 4);
    if (gate.visibilityEnabled) {
        encoded[0x15] = 1;
        encoded[0x22] = char((gate.bitEnabled ? 4 : 0) | (gate.levelEnabled ? 1 : 0));
        if (gate.levelEnabled) { encoded[0x1f] = char(gate.minimumLevel - 1); }
        if (gate.bitEnabled) {
            if (!compiler.encode(gate.bitAddress, false, false, &address, error)) { return false; }
            if (address.size() != 36 || address.at(0) != 0 || address.at(1) != 'B') {
                if (error) { *error = QStringLiteral("消息设备位显现仅核实本地 LB 位地址。"); }
                return false;
            }
            encoded[0x1e] = char(gate.bitState);
            encoded[0x95] = 'B';
            encoded.replace(0x9c, 4, address.constData() + 8, 4);
        }
    }
    encoded[0] = 35;
    qToLittleEndian<u16>(u16(bounds.x()), encoded.data() + 4);
    qToLittleEndian<u16>(u16(bounds.y()), encoded.data() + 6);
    qToLittleEndian<u16>(u16(bounds.width()), encoded.data() + 8);
    qToLittleEndian<u16>(u16(bounds.height()), encoded.data() + 10);
    qToLittleEndian<u32>(rawOffset, encoded.data() + 12);
    qToLittleEndian<u32>(rawSize, encoded.data() + 16);
    encoded[0x100] = char(message.dataType);
    // 0x101 is the zero-based index into the simulator/device message directory, zero for this single message.
    qToLittleEndian<u16>(u16(message.stateCount), encoded.data() + 0x102);
    qToLittleEndian<u16>(u16(message.movement), encoded.data() + 0x104);
    qToLittleEndian<u16>(u16(message.speed), encoded.data() + 0x106);
    qToLittleEndian<u32>(u32(interior.x()), encoded.data() + 0x110);
    qToLittleEndian<u32>(u32(interior.y()), encoded.data() + 0x114);
    qToLittleEndian<u32>(u32(interior.width()), encoded.data() + 0x118);
    qToLittleEndian<u32>(u32(interior.height()), encoded.data() + 0x11c);
    // The frozen originals contain one shared substrate per object, independent of language and state count.
    local = interior.translated(-bounds.topLeft());
    raw.resize(rawSize);
    for (y = 0; y < bounds.height(); ++y) {
        for (x = 0; x < bounds.width(); ++x) {
            const u16 color = local.contains(x, y) ? 0xffff : 0xe71c;
            qToLittleEndian<u16>(color, raw.data() + 2 * (y * bounds.width() + x));
        }
    }
    if (!compressRgb565Pixels(raw, &compressed, error)) { return false; }
    padded = (u32(compressed.size()) + 31U) & ~31U;
    qToLittleEndian<u32>(padded, index.data() + 4);
    qToLittleEndian<u32>(u32(compressed.size()), index.data() + 8);
    result.rawLength = rawSize;
    result.lengthRecords = index;
    result.pixelData = compressed + QByteArray(padded - compressed.size(), char(0));
    *record = encoded;
    *sprites = result;
    *pagePreview = QImage();
    return true;
}
