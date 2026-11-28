#include <QDataStream>
#include <QMap>
#include <QRegularExpression>
#include <QSet>
#include <QtEndian>
#include <cmath> //20261128-33 add

//20261128-22 add: The original opened path may supply both trailer fields or its directory only.
struct NativeMediaSourceLocation
{
    QString path;
    bool projectFile = false;
};

/******************************************************************************
 * Function Name   : encodeNativeMediaLocation
 * Description     : Encode an offline Windows directory and the optional actual SM filename as Qt 4 strings.
 * Input Parameters: original source location, directory and filename outputs, optional diagnostic
 * Return Value    : True for a validated absolute drive or UNC location.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool encodeNativeMediaLocation(const NativeMediaSourceLocation &location, QByteArray *wire,
                                      QByteArray *filenameWire, QString *error) //20261128-22 change
{
    const QChar slash = QLatin1Char('\\');
    const QRegularExpression reserved(QStringLiteral("^(?:CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])(?:\\..*)?$"),
                                      QRegularExpression::CaseInsensitiveOption);
    QString path = location.path;
    QString rest, canonical, filename;
    QStringList parts;
    QDataStream stream(wire, QIODevice::WriteOnly);
    QDataStream filenameStream(filenameWire, QIODevice::WriteOnly);
    bool drive, unc, valid;

    path.replace(QLatin1Char('/'), slash);
    drive = path.size() >= 3 && ((path[0] >= QLatin1Char('A') && path[0] <= QLatin1Char('Z')) ||
                                 (path[0] >= QLatin1Char('a') && path[0] <= QLatin1Char('z'))) &&
            path[1] == QLatin1Char(':') && path[2] == slash;
    unc = path.startsWith(QStringLiteral("\\\\"));
    rest = path.mid(drive ? 3 : 2);
    while (rest.endsWith(slash)) { rest.chop(1); }
    if (!rest.isEmpty()) { parts = rest.split(slash, Qt::KeepEmptyParts); }
    valid = (drive || (unc && parts.size() >= (location.projectFile ? 3 : 2))) && path.size() < 32767;
    for (const QString &part : std::as_const(parts)) {
        if (part.isEmpty() || part.endsWith(QLatin1Char(' ')) || part.endsWith(QLatin1Char('.')) ||
            reserved.match(part).hasMatch()) { valid = false; }
        for (QChar c : part) {
            if (c.unicode() < 32 || QStringLiteral("<>:\"|?*").contains(c)) { valid = false; }
        }
    }
    if (location.projectFile) {
        if (parts.isEmpty() || path.endsWith(slash) || parts.last().size() <= 3 ||
            !parts.last().endsWith(QStringLiteral(".sm"), Qt::CaseInsensitive)) { valid = false; }
        if (!parts.isEmpty()) { filename = parts.takeLast(); }
    }
    // These are offline path components, including UNC shares without a running server.
    canonical = (drive ? path.left(3) : QStringLiteral("\\\\")) + parts.join(slash);
    if (!location.projectFile && (canonical.endsWith(QStringLiteral(".sm"), Qt::CaseInsensitive) ||
        canonical.endsWith(QStringLiteral(".ksp"), Qt::CaseInsensitive))) { valid = false; }

    if (!valid) {
        if (error != nullptr) { *error = QStringLiteral("原厂路径须为有效的绝对 Windows 盘符或 UNC 路径；打开的工程须含 .sm 文件名。"); }
        return false;
    }
    if (!canonical.endsWith(slash)) { canonical += slash; }
    stream.setVersion(QDataStream::Qt_4_8);
    stream << canonical;
    if (location.projectFile) {
        filenameStream.setVersion(QDataStream::Qt_4_8);
        filenameStream << filename;
    }
    return stream.status() == QDataStream::Ok && filenameStream.status() == QDataStream::Ok;
}

/******************************************************************************
 * Function Name   : pruneNativeMediaPictureGallery
 * Description     : Rebuild a picture widget's archive gallery in native resource-key order.
 * Input Parameters: original SM, validated field spans, gallery count span and mutable archive copy
 * Return Value    : None.
 * Date            : 2026.11.28
 ******************************************************************************/
static void pruneNativeMediaPictureGallery(const QByteArray &source, const QList<NativeSourceFieldSpan> &fields,
                                         const NativeSourceFieldSpan &count, QByteArray *snapshot) //20261128-22 change
{
    HmiProject project;
    QSet<QString> used;
    QMap<QString, QByteArray> records;
    QSet<QByteArray> otherKeys;
    QString key;
    QByteArray rawKey, gallery;
    qsizetype end = count.end, start = -1;

    // The complete picture importer checks bindings, PNG records, preview children and state indices.
    if (count.kind != 'i' || count.end - count.start != 4 || qFromBigEndian<u32>(source.constData() + count.start) <= 1 ||
        !importNativeProjectSource(source, &project) || !project.animations.isEmpty() || project.objects.size() != 1 ||
        !project.objects.first().hasImageDisplayProperties) { return; }
    for (const ImageDisplayState &state : project.objects.first().imageDisplay.states) {
        if (state.picture.isEmpty() || state.picturePng.isEmpty()) { return; }
        used.insert(state.picture);
    }
    if (used.isEmpty()) { return; }
    for (const NativeSourceFieldSpan &field : fields) {
        if (!field.path.startsWith(QStringLiteral("config.284["))) { continue; }
        if (field.path.endsWith(QStringLiteral("].0"))) {
            rawKey = source.mid(field.start, field.end - field.start);
            QDataStream decoder(rawKey);
            decoder.setVersion(QDataStream::Qt_4_8);
            decoder >> key;
            start = field.start;
            if (!used.contains(key)) { otherKeys.insert(rawKey); }
        } else if (field.path.endsWith(QStringLiteral("].5"))) {
            end = field.end;
            if (used.contains(key) && start >= count.end) {
                records.insert(key, source.mid(start, end - start));
            }
        }
    }
    if (records.size() != used.size()) { return; }
    // A reference anywhere else in the source may belong to a page background or another native subsystem.
    for (const NativeSourceFieldSpan &field : fields) {
        if (field.kind == 'q' && (field.end <= count.start || field.start >= end) &&
            otherKeys.contains(source.mid(field.start, field.end - field.start))) { return; }
    }
    // The original serializer walks the QString map forward and copies the full record for each registered ID.
    gallery.resize(sizeof(u32));
    qToBigEndian<u32>(static_cast<u32>(records.size()), gallery.data());
    for (const QByteArray &record : std::as_const(records)) { gallery.append(record); }
    snapshot->replace(count.start, end - count.start, gallery);
}

/******************************************************************************
 * Function Name   : refreshReopenedNativeLamp
 * Description     : Replay V122 switch loading and the built-in SVG preview on a single-lamp media copy.
 * Input Parameters: original source bytes, validated source spans and the media working copy
 * Return Value    : None.
 * Date            : 2026.11.28
 ******************************************************************************/
static void refreshReopenedNativeLamp(const QByteArray &source, const QList<NativeSourceFieldSpan> &fields,
                                      QByteArray *snapshot) //20261128-33 add
{
    const QString prefix = QStringLiteral("object[0].");
    const QByteArray zero(4, '\0');
    const QByteArray one = QByteArray::fromHex("00000001");
    QMap<QString, NativeSourceFieldSpan> object;
    QMap<qsizetype, QPair<qsizetype, QByteArray>> edits;
    bool singleObject = false;
    QString resource, selectedText, selectedPicture;
    int state;
    double extent, cached, natural;
    QByteArray value, encoded;
    const auto wire = [&](const QString &key) {
        const NativeSourceFieldSpan field = object.value(key);
        return source.mid(field.start, field.end - field.start);
    };
    const auto replace = [&](const QString &key, const QByteArray &bytes) {
        const NativeSourceFieldSpan field = object.value(key);
        if (field.kind != 0 && !bytes.isEmpty() && wire(key) != bytes) {
            edits.insert(field.start, qMakePair(field.end - field.start, bytes));
        }
    };
    const auto number = [&](const QString &key) {
        double result = 0.0;
        QDataStream stream(wire(key));
        stream >> result;
        return result;
    };
    const auto switchSvg = [&](const QString &key) {
        QString id;
        QDataStream stream(wire(key));
        stream.setVersion(QDataStream::Qt_4_8);
        stream >> id;
        return id == QStringLiteral(":/btn/images/switch_OFF.kvg") || id == QStringLiteral(":/btn/images/switch_ON.kvg");
    };

    for (const NativeSourceFieldSpan &field : fields) {
        if (field.path == QStringLiteral("page.objects.count")) {
            singleObject = source.mid(field.start, field.end - field.start) == one;
            if (!singleObject) { return; }
        } else if (field.path.startsWith(prefix)) {
            object.insert(field.path.mid(prefix.size()), field);
        }
    }
    if (!singleObject || wire(QStringLiteral("type")) != QByteArray::fromHex("0000000a") ||
        wire(QStringLiteral("switch.mode")) != QByteArray::fromHex("00000004")) { return; }

    // V5.1 retains the constructor's access mode for values outside the two accepted enumeration cases.
    if (wire(QStringLiteral("common.58.0")) != one) { replace(QStringLiteral("common.58.0"), zero); }
    for (const QString &key : {QStringLiteral("switch.22c.0"), QStringLiteral("switch.22c.1"), QStringLiteral("switch.22c.4")}) {
        value = wire(key);
        if (value.size() == 1) { replace(key, QByteArray(1, value[0] != 0)); }
    }
    // The V122 reader replaces the saved monitor selector with bit-address flag +3 of its address structure.
    value = wire(QStringLiteral("switch.254.6"));
    if (value.size() == 1) { replace(QStringLiteral("switch.248.0"), QByteArray(1, value[0] != 0)); }

    if (switchSvg(QStringLiteral("pixmap.0"))) {
        // Qt 4 scales the integer SVG defaultSize(), then maps the bounds back through the scene transform.
        // Preserve cropped or independently transformed children and only refresh equal parent/preview extents.
        for (int axis = 0; axis < 2; ++axis) {
            resource = QStringLiteral("pixmap.%1").arg(4 + axis);
            extent = number(QStringLiteral("rectangle.%1").arg(2 + axis)) - number(QStringLiteral("rectangle.%1").arg(axis));
            cached = number(resource);
            if (extent < 1 || extent > 65535 || std::floor(extent) != extent || !qFuzzyCompare(extent, cached)) { continue; }
            natural = axis == 0 ? 123.0 : 58.0;
            encoded.clear();
            QDataStream stream(&encoded, QIODevice::WriteOnly);
            stream << ((extent / natural) * natural);
            replace(resource, encoded);
        }
        value = wire(QStringLiteral("switch.274.1"));
        state = value.size() == 4 ? qFromBigEndian<qint32>(value.constData()) : 0;
        selectedText = QStringLiteral("switch.290[%1]").arg(state);
        selectedPicture = QStringLiteral("switch.294[%1]").arg(state);
        // Reopen selects state zero. Reconstruct this preview only when both states use compatible single-language
        // text styling and built-in artwork and the existing children still match the saved editor selection.
        if (state > 0 && wire(selectedText + QStringLiteral(".count")) == one && wire(QStringLiteral("switch.290[0].count")) == one &&
            wire(selectedText + QStringLiteral("[0].0")) == wire(QStringLiteral("switch.290[0][0].0")) &&
            wire(selectedText + QStringLiteral("[0].2")) == wire(QStringLiteral("switch.290[0][0].2")) &&
            wire(selectedText + QStringLiteral("[0].3")) == wire(QStringLiteral("switch.290[0][0].3")) &&
            wire(selectedText + QStringLiteral("[0].1")) == wire(QStringLiteral("font.family")) && wire(selectedText + QStringLiteral("[0].4")) == wire(QStringLiteral("text.head.4")) &&
            wire(selectedPicture + QStringLiteral(".0")) == one && wire(QStringLiteral("switch.294[0].0")) == one &&
            wire(selectedPicture + QStringLiteral(".3")) == wire(QStringLiteral("switch.294[0].3")) &&
            wire(selectedPicture + QStringLiteral(".1")) == wire(QStringLiteral("pixmap.0")) && switchSvg(QStringLiteral("switch.294[0].1"))) {
            replace(QStringLiteral("switch.274.1"), zero);
            replace(QStringLiteral("pixmap.0"), wire(QStringLiteral("switch.294[0].1")));
            replace(QStringLiteral("font.family"), wire(QStringLiteral("switch.290[0][0].1")));
            replace(QStringLiteral("text.head.4"), wire(QStringLiteral("switch.290[0][0].4")));
        }
    }
    // The preview strings can change length, so write all spans from the end towards the header.
    for (auto it = edits.constEnd(); it != edits.constBegin();) {
        --it;
        snapshot->replace(it.key(), it.value().first, it.value().second);
    }
}

/******************************************************************************
 * Function Name   : buildNativeMediaSnapshot
 * Description     : Reproduce selected-page and optional run-directory fields on an independent V122 source copy.
 * Input Parameters: exported SM, destination archive, optional diagnostic and original editor working directory
 * Return Value    : True after the complete reconstructed archive has been compressed and verified.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool buildNativeMediaSnapshot(const QByteArray &source, QByteArray *archive, QString *error,
                                     const NativeMediaSourceLocation &location) //20261128-22 change
{
    NativeProjectEnvelope envelope;
    QList<NativeSourceFieldSpan> fields;
    NativeSourceFieldSpan currentPage, pageName, pageCount, directory, filename, galleryCount;
    QByteArray snapshot = source;
    QByteArray compressed, directoryWire, filenameWire;
    bool startupPage = false;

    if (archive == nullptr || !inspectNativeProjectEnvelope(source, &envelope, error) ||
        !inspectNativeProjectSourceFields(source, &fields, error)) {
        return false;
    }
    for (const NativeSourceFieldSpan &field : fields) {
        if (field.path == QStringLiteral("config.head.12")) { currentPage = field; }
        if (field.path == QStringLiteral("config.284.count")) { galleryCount = field; } //20261128-13 add
        if (field.path == QStringLiteral("page.head.18")) { pageName = field; }
        if (field.path == QStringLiteral("project.pages.count")) { pageCount = field; }
        if (field.path == QStringLiteral("trailer.settings.0")) { directory = field; }
        if (field.path == QStringLiteral("trailer.settings.1")) { filename = field; }
        if (field.path == QStringLiteral("startup.head.0")) { startupPage = true; }
    }
    if (envelope.version != 122 || startupPage || pageCount.end - pageCount.start != 4 ||
        source.mid(pageCount.start, 4) != QByteArray::fromHex("00000001")) {
        if (error != nullptr) { *error = QStringLiteral("原厂介质工程归档仅核定 V122 单个普通画面。"); }
        return false;
    }
    if (currentPage.kind != 'q' || pageName.kind != 'q' || directory.kind != 'q') {
        if (error != nullptr) { *error = QStringLiteral("介质工程归档缺少原厂页面名或目录字段。"); }
        return false;
    }
    // Apply the later variable-length field first so original offsets remain valid for the selection field.
    if (!location.path.isEmpty()) {
        if (!encodeNativeMediaLocation(location, &directoryWire, &filenameWire, error)) { return false; }
        if (location.projectFile) {
            if (filename.kind != 'q' || filename.start < directory.end) {
                if (error != nullptr) { *error = QStringLiteral("归档缺少原厂打开的工程文件名字段。"); }
                return false;
            }
            snapshot.replace(filename.start, filename.end - filename.start, filenameWire);
        }
        snapshot.replace(directory.start, directory.end - directory.start, directoryWire);
    }
    if (location.projectFile) { refreshReopenedNativeLamp(source, fields, &snapshot); } //20261128-33 add
    pruneNativeMediaPictureGallery(source, fields, galleryCount, &snapshot); //20261128-13 add
    if (source.mid(currentPage.start, currentPage.end - currentPage.start) == QByteArray(4, char(0xff)) &&
        pageName.end - pageName.start > 4) {
        snapshot.replace(currentPage.start, currentPage.end - currentPage.start,
                         source.mid(pageName.start, pageName.end - pageName.start));
    }
    compressed = qCompress(snapshot, 9);
    if (compressed.isEmpty() || qUncompress(compressed) != snapshot) {
        if (error != nullptr) { *error = QStringLiteral("介质可编辑工程归档压缩核验失败。"); }
        return false;
    }
    *archive = std::move(compressed);
    return true;
}
