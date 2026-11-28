#include <QDataStream>
#include <QRegularExpression>
#include <QSet>
#include <QtEndian>

/******************************************************************************
 * Function Name   : nativeMediaWorkDirectory
 * Description     : Encode the Windows directory of the opened project without accessing a historical network share.
 * Input Parameters: Windows drive or UNC directory, output QString wire bytes, optional diagnostic
 * Return Value    : True after deriving an absolute directory with a single terminal backslash.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool nativeMediaWorkDirectory(const QString &input, QByteArray *wire, QString *error) //20261128-8 add
{
    const QChar slash = QLatin1Char('\\');
    const QRegularExpression reserved(QStringLiteral("^(?:CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])(?:\\..*)?$"),
                                      QRegularExpression::CaseInsensitiveOption);
    QString path = input;
    QString rest, canonical;
    QStringList parts;
    QDataStream stream(wire, QIODevice::WriteOnly);
    bool drive, unc, valid;

    path.replace(QLatin1Char('/'), slash);
    drive = path.size() >= 3 && ((path[0] >= QLatin1Char('A') && path[0] <= QLatin1Char('Z')) ||
                                 (path[0] >= QLatin1Char('a') && path[0] <= QLatin1Char('z'))) &&
            path[1] == QLatin1Char(':') && path[2] == slash;
    unc = path.startsWith(QStringLiteral("\\\\"));
    rest = path.mid(drive ? 3 : 2);
    while (rest.endsWith(slash)) { rest.chop(1); }
    if (!rest.isEmpty()) { parts = rest.split(slash, Qt::KeepEmptyParts); }
    valid = (drive || (unc && parts.size() >= 2)) && path.size() < 32767;
    for (const QString &part : std::as_const(parts)) {
        if (part.isEmpty() || part.endsWith(QLatin1Char(' ')) || part.endsWith(QLatin1Char('.')) ||
            reserved.match(part).hasMatch()) { valid = false; }
        for (QChar c : part) {
            if (c.unicode() < 32 || QStringLiteral("<>:\"|?*").contains(c)) { valid = false; }
        }
    }
    // Callers supply the directory, not the .sm or .ksp working file. Never resolve a UNC share on the network.
    canonical = (drive ? path.left(3) : QStringLiteral("\\\\")) + parts.join(slash);
    if (canonical.endsWith(QStringLiteral(".sm"), Qt::CaseInsensitive) ||
        canonical.endsWith(QStringLiteral(".ksp"), Qt::CaseInsensitive)) { valid = false; }

    if (!valid) {
        if (error != nullptr) { *error = QStringLiteral("原厂工作目录必须为绝对 Windows 盘符目录或 UNC 共享目录，不能包含工程文件名、设备名、上级目录或非法字符。"); }
        return false;
    }
    if (!canonical.endsWith(slash)) { canonical += slash; }
    stream.setVersion(QDataStream::Qt_4_8);
    stream << canonical;
    return stream.status() == QDataStream::Ok;
}

/******************************************************************************
 * Function Name   : pruneNativeMediaPictureGallery
 * Description     : Retain a single referenced project PNG in an image-only V122 archive copy.
 * Input Parameters: original SM, validated field spans, gallery count span and mutable archive copy
 * Return Value    : None.
 * Date            : 2026.11.28
 ******************************************************************************/
static void pruneNativeMediaPictureGallery(const QByteArray &source, const QList<NativeSourceFieldSpan> &fields,
                                         const NativeSourceFieldSpan &count, QByteArray *snapshot) //20261128-13 add
{
    HmiProject project;
    QByteArray key, record;
    QSet<QByteArray> otherKeys;
    QDataStream encodedKey(&key, QIODevice::WriteOnly);
    qsizetype end = count.end, start = -1;
    bool keep = false;

    // The complete picture importer checks bindings, PNG records, preview children and state indices.
    if (count.kind != 'i' || count.end - count.start != 4 || qFromBigEndian<u32>(source.constData() + count.start) <= 1 ||
        !importNativeProjectSource(source, &project) || project.objects.size() != 1 ||
        !project.objects.first().hasImageDisplayProperties) { return; }
    const QList<ImageDisplayState> &states = project.objects.first().imageDisplay.states;
    if (states.isEmpty() || states.first().picturePng.isEmpty()) { return; }
    for (const ImageDisplayState &state : states) {
        if (state.picture != states.first().picture || state.picturePng.isEmpty()) { return; }
    }
    encodedKey.setVersion(QDataStream::Qt_4_8);
    encodedKey << states.first().picture;
    for (const NativeSourceFieldSpan &field : fields) {
        if (!field.path.startsWith(QStringLiteral("config.284["))) { continue; }
        if (field.path.endsWith(QStringLiteral("].0"))) {
            const QByteArray id = source.mid(field.start, field.end - field.start);
            start = field.start;
            keep = id == key;
            if (!keep) { otherKeys.insert(id); }
        } else if (field.path.endsWith(QStringLiteral("].5"))) {
            end = field.end;
            if (keep && start >= count.end) { record = source.mid(start, end - start); }
        }
    }
    if (record.isEmpty() || otherKeys.isEmpty()) { return; }
    // A reference anywhere else in the source may belong to a page background or another native subsystem.
    for (const NativeSourceFieldSpan &field : fields) {
        if (field.kind == 'q' && (field.end <= count.start || field.start >= end) &&
            otherKeys.contains(source.mid(field.start, field.end - field.start))) { return; }
    }
    snapshot->replace(count.start, end - count.start, QByteArray::fromHex("00000001") + record);
}

/******************************************************************************
 * Function Name   : buildNativeMediaSnapshot
 * Description     : Reproduce selected-page and optional run-directory fields on an independent V122 source copy.
 * Input Parameters: exported SM, destination archive, optional diagnostic and original editor working directory
 * Return Value    : True after the complete reconstructed archive has been compressed and verified.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool buildNativeMediaSnapshot(const QByteArray &source, QByteArray *archive, QString *error,
                                     const QString &nativeWorkDirectory = QString()) //20261128-8 change
{
    NativeProjectEnvelope envelope;
    QList<NativeSourceFieldSpan> fields;
    NativeSourceFieldSpan currentPage, pageName, pageCount, directory, galleryCount;
    QByteArray snapshot = source;
    QByteArray compressed, directoryWire;
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
    if (!nativeWorkDirectory.isEmpty()) {
        if (!nativeMediaWorkDirectory(nativeWorkDirectory, &directoryWire, error)) { return false; }
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
