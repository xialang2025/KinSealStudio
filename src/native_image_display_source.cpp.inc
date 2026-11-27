//20261127-14 add: Editable V122 picture and QR records retain each original graphical child and dormant field.
/******************************************************************************
 * Function Name   : isImageDisplayType
 * Description     : Identify source records using the recovered picture, QR or GIF container grammar.
 * Input Parameters: type
 * Return Value    : True when this reader owns the V122 body.
 * Date            : 2026.11.27
 ******************************************************************************/
static bool isImageDisplayType(int type)
{
    return type == 15 || type == 34 || type == 52;
}

/******************************************************************************
 * Function Name   : readNativeImageDisplay
 * Description     : Traverse display records and record individual stream spans for safe updates.
 * Input Parameters: d, p, type
 * Return Value    : None
 * Date            : 2026.11.27
 ******************************************************************************/
static void readNativeImageDisplay(SourceDocument &d, const QString &p, int type)
{
    int i;
    int count;

    d.container(p);
    if (type == 15) {
        d.fields("ssbsq", p + QStringLiteral("image.head"));
        d.address(p + QStringLiteral("image.address"));
        d.fields("ssbs", p + QStringLiteral("image.tail"));
        count = d.count(p + QStringLiteral("image.rows"), 128);
        for (i = 0; i < count && d.error.isEmpty(); ++i) {
            d.fields("qqis", SourceDocument::indexed(p + QStringLiteral("image.row"), i));
        }
    } else if (type == 52) {
        d.fields("qqi", p + QStringLiteral("qr.head"));
        d.address(p + QStringLiteral("qr.address"));
        d.fields("iqiicaa", p + QStringLiteral("qr.tail"));
    } else {
        d.fields("ssq", p + QStringLiteral("gif.head"));
        d.address(p + QStringLiteral("gif.address"));
        d.fields("qssi", p + QStringLiteral("gif.tail"));
    }
}

/******************************************************************************
 * Function Name   : nativeKvgImagePath
 * Description     : Resolve an original gallery reference exclusively inside the bundled gallery.
 * Input Parameters: vendorPath
 * Return Value    : Bundled Qt resource key or an empty string.
 * Date            : 2026.11.27
 ******************************************************************************/
static QString nativeKvgImagePath(QString vendorPath)
{
    const QString marker = QStringLiteral("/ExternAdd/");
    const QString builtin = QStringLiteral(":/btn/images/");
    QString key;
    int pos;

    vendorPath.replace(QLatin1Char('\\'), QLatin1Char('/'));
    while (vendorPath.contains(QStringLiteral("//"))) { vendorPath.replace(QStringLiteral("//"), QStringLiteral("/")); }
    pos = vendorPath.lastIndexOf(marker, -1, Qt::CaseInsensitive);
    if (vendorPath.startsWith(builtin)) {
        key = QStringLiteral(":/system-gallery/default/") + vendorPath.mid(builtin.size());
    } else if (pos >= 0) {
        key = QStringLiteral(":/system-gallery/externadd/") + vendorPath.mid(pos + marker.size());
    } else {
        return {};
    }
    if (QDir::cleanPath(key) != key || !key.endsWith(QStringLiteral(".kvg"), Qt::CaseInsensitive) || !QFile::exists(key)) { return {}; }
    return key;
}

/******************************************************************************
 * Function Name   : sourceNativeImageDisplay
 * Description     : Restore image state rows, selected preview geometry and typed QR inputs.
 * Input Parameters: d, p, index, pictures, output, warnings, external, error
 * Return Value    : True after validating the complete object.
 * Date            : 2026.11.27
 ******************************************************************************/
static bool sourceNativeImageDisplay(const SourceDocument &d, const QString &p, int index,
                                     const QMap<QString, QByteArray> &pictures, HmiObject *output,
                                     QStringList *warnings, bool *external, QString *error)
{
    HmiObject object;
    const int type = d.value(p + QStringLiteral("type")).toInt();
    const bool qr = type == 52;
    const int count = d.value(p + QStringLiteral("children.count")).toInt();
    QString child = SourceDocument::indexed(p + QStringLiteral("child"), qr ? 1 : 0) + QLatin1Char('.');
    const int kind = d.value(child + QStringLiteral("type")).toInt();
    const bool projectImage = kind == 0x10018;
    QString row;
    QString first;
    QString second;
    QString monitored;
    ImageDisplayState state;
    int selected;
    int mode;
    int wireCondition;
    int binding;
    int i;
    double x;
    double y;
    double width;
    double height;
    StaticTextProperties visible;

    if (type == 34) { return fail(error, QStringLiteral("原厂 GIF 工程保存的是帧资源表，尚未映射为完整动画及帧延时。")); }
    if ((qr && (kind != 13 || count != 2 || d.value(p + QStringLiteral("child[0].type")).toInt() != 0x1000f)) ||
        (!qr && (count != 1 || (!projectImage && kind != 13)))) {
        return fail(error, QStringLiteral("图片或二维码的原厂缓存子图元布局尚未核实。"));
    }
    child += projectImage ? QStringLiteral("project-picture.") : QStringLiteral("pixmap.");
    x = d.value(child + QStringLiteral("6")).toDouble();
    y = d.value(child + QStringLiteral("7")).toDouble();
    width = d.value(child + (projectImage ? QStringLiteral("3") : QStringLiteral("4"))).toDouble();
    height = d.value(child + (projectImage ? QStringLiteral("4") : QStringLiteral("5"))).toDouble();
    for (const double value : {x, y, width, height}) {
        if (!std::isfinite(value) || std::abs(value) > 32768) {
            return fail(error, QStringLiteral("图片控件尺寸或原点超出已核实的整数几何范围。"));
        }
    }
    if (std::abs(width - std::round(width)) > 1e-7 || std::abs(height - std::round(height)) > 1e-7 ||
        width < 1 || height < 1 || d.value(p + QStringLiteral("common.touch.0")).toBool() ||
        d.value(p + QStringLiteral("common.58.2")).toBool()) {
        return fail(error, QStringLiteral("图片对象含无效尺寸或未映射的活动触控及通知选项。"));
    }
    object.geometry = QRect(qRound(x), qRound(y), qRound(width), qRound(height));
    object.subpixelOffset = QPointF(x - object.geometry.x(), y - object.geometry.y());
    object.type = qr ? QStringLiteral("qrcode") : QStringLiteral("image");
    object.id = QStringLiteral("native-%1-%2").arg(object.type).arg(index);
    object.name = d.value(p + QStringLiteral("group.metadata-name")).toString();
    if (object.name.isEmpty()) { object.name = (qr ? QStringLiteral("二维码") : QStringLiteral("图片显示")) + QString::number(index + 1); }
    // Disabled visibility fields sometimes contain uninitialized native integers. The full record remains in the source.
    visible = sourceCondition(d, p, false, warnings);
    if (!visible.validate(error)) { return false; }
    if (qr) {
        if (!modelSourceAddress(d, p + QStringLiteral("qr.address"), &monitored, external, error)) { return false; }
        object.address = object.qrCode.monitorAddress = monitored;
        object.qrCode.description = d.value(p + QStringLiteral("qr.head.1")).toString();
        object.qrCode.characterLength = d.value(p + QStringLiteral("qr.head.2")).toInt();
        object.qrCode.swapBytes = d.value(p + QStringLiteral("qr.tail.5")).toBool();
        object.qrCode.encoding = d.value(p + QStringLiteral("qr.tail.6")).toInt();
        object.qrCode.visibility = visible;
        if (!object.qrCode.validate(error)) { return false; }
    } else {
        object.hasImageDisplayProperties = true;
        ImageDisplayProperties &properties = object.imageDisplay;
        properties.visibility = visible;
        selected = d.value(p + QStringLiteral("image.head.0")).toInt();
        mode = d.value(p + QStringLiteral("image.head.1")).toInt();
        wireCondition = d.value(p + QStringLiteral("image.head.3")).toInt();
        if (wireCondition < 0 || wireCondition > 2) { return fail(error, QStringLiteral("原厂图片切换条件未映射。")); }
        properties.displayType = mode;
        properties.conditionEnabled = mode == 1 || d.value(p + QStringLiteral("image.head.2")).toBool();
        properties.condition = wireCondition == 1 ? 2 : wireCondition == 2 ? 1 : 0;
        properties.interval = d.value(p + QStringLiteral("image.tail.1")).toInt();
        properties.stateCount = d.value(p + QStringLiteral("image.rows.count")).toInt();
        if (selected < 0 || selected >= properties.stateCount ||
            properties.stateCount != d.value(p + QStringLiteral("image.tail.0")).toInt()) {
            return fail(error, QStringLiteral("原厂图片状态数量或预览状态索引不一致。"));
        }
        properties.monitorAddress = d.value(p + QStringLiteral("image.head.4")).toString();
        if (properties.conditionEnabled && properties.condition != 0) {
            if (!modelSourceAddress(d, p + QStringLiteral("image.address"), &properties.monitorAddress, external, error)) { return false; }
        }
        object.address = properties.monitorAddress;
        properties.states.clear();
        for (i = 0; i < properties.stateCount; ++i) {
            row = SourceDocument::indexed(p + QStringLiteral("image.row"), i);
            first = d.value(row + QStringLiteral(".0")).toString();
            second = d.value(row + QStringLiteral(".1")).toString();
            binding = d.value(row + QStringLiteral(".2")).toInt();
            state = ImageDisplayState();
            if (binding == 0 && first == second && pictures.contains(first)) {
                state.picture = first; state.picturePng = pictures.value(first);
            } else if (binding == 1 && first == QFileInfo(QDir::fromNativeSeparators(second)).fileName()) {
                state.picture = nativeKvgImagePath(second);
            }
            if (state.picture.isEmpty() || d.value(row + QStringLiteral(".3")).toInt() != i) {
                return fail(error, QStringLiteral("原厂图片状态 %1 的图库类型、资源或顺序尚未映射。").arg(i));
            }
            if (i == selected && (projectImage != (binding == 0) ||
                d.value(child + (projectImage ? QStringLiteral("5") : QStringLiteral("0"))).toString() != second)) {
                return fail(error, QStringLiteral("原厂图片预览缓存与活动资源不一致。"));
            }
            properties.states.append(state);
        }
        if (!properties.validate(error)) { return false; }
    }
    *output = object;
    return true;
}

/******************************************************************************
 * Function Name   : nativeImageScalar
 * Description     : Preserve null strings, inactive values and imprecise native coordinates on unchanged fields.
 * Input Parameters: updates, key, value
 * Return Value    : None
 * Date            : 2026.11.27
 ******************************************************************************/
static void nativeImageScalar(SourceUpdates &updates, const QString &key, const QVariant &value)
{
    if (updates.document.value(key) != value) { updates.set(key, value); }
}

/******************************************************************************
 * Function Name   : writeNativeImageDisplay
 * Description     : Encode a new or edited display from the matching V122 object template.
 * Input Parameters: object, original, document, prefix, pictures, output, error
 * Return Value    : True after all active fields and cache geometry were checked.
 * Date            : 2026.11.27
 ******************************************************************************/
static bool writeNativeImageDisplay(const HmiObject &object, const HmiObject *original, const SourceDocument &document,
                                    QString prefix, const QMap<QString, QByteArray> &pictures, QByteArray *output, QString *error)
{
    const bool qr = object.type == QStringLiteral("qrcode");
    QFile file(qr ? QStringLiteral(":/native/qr-display-seed.sm") : QStringLiteral(":/native/image-display-seed.sm"));
    SourceDocument seed{QByteArray()};
    const SourceDocument *doc = &document;
    const HmiObject *previous = original;
    HmiObject baseline;
    QStringList warnings;
    QMap<QString,QByteArray> seedPictures;
    bool external = false;
    QString child;
    QString field;
    QString row;
    QString selectedKey;
    QString selectedPath;
    QString first;
    QString second;
    QString vendor;
    QByteArray rows;
    QByteArray preview;
    QDataStream stream(&rows,QIODevice::WriteOnly);
    QDataStream cache(&preview,QIODevice::WriteOnly);
    int oldKind;
    int newKind;
    int binding;
    int selected;
    int i;
    int n;
    double x;
    double y;
    double dx;
    double dy;
    bool redraw;
    qsizetype end;

    if (!object.extensions.isEmpty() || object.hasAppearanceFlip() || !object.text.isEmpty() || !object.geometry.isValid()) {
        return fail(error, QStringLiteral("图片或二维码存在未映射的变换、文字、扩展字段或尺寸。"));
    }
    if (!previous) {
        if (!file.open(QIODevice::ReadOnly)) { return fail(error, QStringLiteral("缺少原厂图片或二维码种子。")); }
        seed.bytes = file.readAll();
        if (!seed.parse() || seed.objectPrefixes.size() != 1 || !sourcePictures(seed, &seedPictures, error)) { return false; }
        prefix = seed.objectPrefixes.first();
        if (!sourceNativeImageDisplay(seed, prefix, 0, seedPictures, &baseline, &warnings, &external, error)) { return false; }
        previous = &baseline; doc = &seed;
    }
    const SourceDocument &d = *doc;
    SourceUpdates updates(d);
    redraw = object.geometry != previous->geometry || object.subpixelOffset != previous->subpixelOffset;
    x = object.geometry.x() + object.subpixelOffset.x();
    y = object.geometry.y() + object.subpixelOffset.y();
    dx = x - previous->geometry.x() - previous->subpixelOffset.x();
    dy = y - previous->geometry.y() - previous->subpixelOffset.y();
    if (!std::isfinite(x) || !std::isfinite(y) || std::abs(x) > 32768 || std::abs(y) > 32768 ||
        object.geometry.width() > 32768 || object.geometry.height() > 32768) {
        return fail(error, QStringLiteral("图片控件坐标或尺寸超出可重新读取的范围。"));
    }
    if (object.name != previous->name || !original) { nativeImageScalar(updates, prefix + QStringLiteral("group.metadata-name"), object.name); }
    n = d.value(prefix + QStringLiteral("children.count")).toInt();
    if (redraw) {
        for (i = 0; i < n; ++i) {
            child = SourceDocument::indexed(prefix + QStringLiteral("child"), i) + QLatin1Char('.');
            oldKind = d.value(child + QStringLiteral("type")).toInt();
            if (oldKind == 0x1000f) {
                field = child + QStringLiteral("rectangle.");
                if (object.geometry.width() != previous->geometry.width()) {
                    nativeImageScalar(updates, field + QStringLiteral("2"), object.geometry.width());
                }
                if (object.geometry.height() != previous->geometry.height()) {
                    nativeImageScalar(updates, field + QStringLiteral("3"), object.geometry.height());
                }
                nativeImageScalar(updates, field + QStringLiteral("9"), d.value(field + QStringLiteral("9")).toDouble() + dx);
                nativeImageScalar(updates, field + QStringLiteral("10"), d.value(field + QStringLiteral("10")).toDouble() + dy);
            } else {
                field = child + (oldKind == 13 ? QStringLiteral("pixmap.") : QStringLiteral("project-picture."));
                const int firstAxis = oldKind == 13 ? 2 : 1;
                for (int axis = 0; axis < 2; ++axis) {
                    const QString scene = field + QString::number(6 + axis);
                    const QString cached = field + QString::number(firstAxis + axis);
                    // The initial cached local origin may be zero; only move an independently stored scene-coordinate mirror.
                    if (d.value(cached).toDouble() == d.value(scene).toDouble()) {
                        nativeImageScalar(updates, cached, d.value(cached).toDouble() + (axis ? dy : dx));
                    }
                    nativeImageScalar(updates, scene, d.value(scene).toDouble() + (axis ? dy : dx));
                    if ((axis ? object.geometry.height() != previous->geometry.height() : object.geometry.width() != previous->geometry.width())) {
                        nativeImageScalar(updates, field + QString::number(firstAxis + 2 + axis), axis ? object.geometry.height() : object.geometry.width());
                    }
                }
            }
        }
    }
    if (qr) {
        const QrCodeProperties &a = previous->qrCode;
        const QrCodeProperties &z = object.qrCode;
        if (!z.validate(error) || object.address != z.monitorAddress ||
            !patchCondition(a.visibility.toJson(), z.visibility.toJson(), prefix, false, &updates)) {
            return fail(error, QStringLiteral("二维码地址与监视地址不一致或显现设置未映射。"));
        }
        if (a.monitorAddress != z.monitorAddress) { updates.address(prefix + QStringLiteral("qr.address"), z.monitorAddress); updates.set(prefix + QStringLiteral("qr.head.0"), z.monitorAddress); }
        if (a.description != z.description) { updates.set(prefix + QStringLiteral("qr.head.1"), z.description); }
        if (a.characterLength != z.characterLength) { updates.set(prefix + QStringLiteral("qr.head.2"), z.characterLength); }
        if (a.swapBytes != z.swapBytes) { updates.set(prefix + QStringLiteral("qr.tail.5"), z.swapBytes); }
        if (a.encoding != z.encoding) { updates.set(prefix + QStringLiteral("qr.tail.6"), z.encoding); }
        end = d.values.value(prefix + QStringLiteral("qr.tail.6")).end;
    } else {
        const ImageDisplayProperties &a = previous->imageDisplay;
        const ImageDisplayProperties &z = object.imageDisplay;
        if (!object.hasImageDisplayProperties || !z.validate(error) || object.address != z.monitorAddress ||
            !patchCondition(a.visibility.toJson(), z.visibility.toJson(), prefix, false, &updates)) {
            return fail(error, QStringLiteral("图片状态、显现或监视地址尚不能无损写回。"));
        }
        if (z.displayType != a.displayType) { updates.set(prefix + QStringLiteral("image.head.1"), z.displayType); }
        if (z.displayType == 0 && z.conditionEnabled != a.conditionEnabled) { updates.set(prefix + QStringLiteral("image.head.2"), z.conditionEnabled); }
        if (z.condition != a.condition) { updates.set(prefix + QStringLiteral("image.head.3"), z.condition == 2 ? 1 : z.condition == 1 ? 2 : 0); }
        if (z.monitorAddress != a.monitorAddress || (z.conditionEnabled && z.condition != 0 && (!a.conditionEnabled || z.condition != a.condition))) {
            updates.set(prefix + QStringLiteral("image.head.4"), z.monitorAddress);
            if (!z.monitorAddress.isEmpty()) { updates.address(prefix + QStringLiteral("image.address"), z.monitorAddress); }
        }
        if (z.interval != a.interval) { updates.set(prefix + QStringLiteral("image.tail.1"), z.interval); }
        selected = qMin(d.value(prefix + QStringLiteral("image.head.0")).toInt(), z.stateCount - 1);
        end = d.values.value(SourceDocument::indexed(prefix + QStringLiteral("image.row"), a.stateCount - 1) + QStringLiteral(".3")).end;
        if (!original || z.states.size() != a.states.size() ||
            !std::equal(z.states.cbegin(),z.states.cend(),a.states.cbegin(),[](const ImageDisplayState &l,const ImageDisplayState &r){
                return l.picture == r.picture && l.picturePng == r.picturePng;
            })) {
            stream.setVersion(QDataStream::Qt_4_8);
            stream << z.stateCount;
            for (i = 0; i < z.stateCount; ++i) {
                const ImageDisplayState &entry = z.states.at(i);
                if (!entry.picturePng.isEmpty() && pictures.value(entry.picture) == entry.picturePng) {
                    first = second = entry.picture; binding = 0;
                } else if (entry.picturePng.isEmpty() && entry.picture.startsWith(QStringLiteral(":/system-gallery/"))) {
                    vendor.clear();
                    for (int old = 0; old < a.states.size() && vendor.isEmpty(); ++old) {
                        if (a.states.at(old).picture == entry.picture) {
                            vendor = d.value(SourceDocument::indexed(prefix + QStringLiteral("image.row"), old) + QStringLiteral(".1")).toString();
                        }
                    }
                    if (vendor.isEmpty() && entry.picture.startsWith(QStringLiteral(":/system-gallery/default/"))) {
                        vendor = QStringLiteral(":/btn/images/") + QFileInfo(entry.picture).fileName();
                    }
                    if (vendor.isEmpty()) { return fail(error, QStringLiteral("新外部图库引用缺少原厂工程中的有效 KVG 路径。")); }
                    first = QFileInfo(vendor).fileName(); second = vendor; binding = 1;
                } else { return fail(error, QStringLiteral("图片状态引用的 PNG 内容与工程图库不一致。")); }
                stream << first << second << binding << static_cast<short>(i);
                if (i == selected) { selectedKey = first; selectedPath = second; }
            }
            if (stream.status() != QDataStream::Ok) { return fail(error, QStringLiteral("写入图片状态表失败。")); }
            updates.replacements.insert(d.values.value(prefix + QStringLiteral("image.rows.count")).start,qMakePair(end,rows));
            nativeImageScalar(updates,prefix + QStringLiteral("image.head.0"),selected);
            nativeImageScalar(updates,prefix + QStringLiteral("image.tail.0"),z.stateCount);
            child = prefix + QStringLiteral("child[0].");
            oldKind = d.value(child + QStringLiteral("type")).toInt();
            newKind = z.states.at(selected).picturePng.isEmpty() ? 13 : 0x10018;
            if (newKind == oldKind) {
                nativeImageScalar(updates,child + (newKind == 13 ? QStringLiteral("pixmap.0") : QStringLiteral("project-picture.5")),selectedPath);
            } else {
                // Changing library switches the type and serialized shape of the complete sole preview child.
                cache.setVersion(QDataStream::Qt_4_8);
                if (newKind == 13) { cache << newKind << selectedPath << 0.0 << x << y << double(object.geometry.width()) << double(object.geometry.height()) << x << y << true << 0 << QStringList{} << 1; }
                else { cache << newKind << 0.0 << x << y << double(object.geometry.width()) << double(object.geometry.height()) << selectedKey << x << y << true << -1 << QStringList{} << 0; }
                if (cache.status() != QDataStream::Ok) { return fail(error, QStringLiteral("图片预览缓存写入失败。")); }
                const qsizetype begin = d.values.value(child + QStringLiteral("type")).start;
                const qsizetype stop = d.values.value(prefix + QStringLiteral("group.metadata-type")).start;
                auto it = updates.replacements.lowerBound(begin);
                while (it != updates.replacements.end() && it.key() < stop) { it = updates.replacements.erase(it); }
                updates.replacements.insert(begin,qMakePair(stop,preview));
            }
        }
    }
    if (!updates.error.isEmpty()) { return fail(error, updates.error); }
    *output = updates.render(d.values.value(prefix + QStringLiteral("type")).start,end);
    return updates.error.isEmpty() || fail(error, updates.error);
}