#include <QDataStream>
#include <QMap>
#include <QRegularExpression>
#include <QSet>
#include <QtEndian>

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
