#include <QDataStream>
#include <QRegularExpression>

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
    NativeSourceFieldSpan currentPage, pageName, pageCount, directory;
    QByteArray snapshot = source;
    QByteArray compressed, directoryWire;
    bool startupPage = false;

    if (archive == nullptr || !inspectNativeProjectEnvelope(source, &envelope, error) ||
        !inspectNativeProjectSourceFields(source, &fields, error)) {
        return false;
    }
    for (const NativeSourceFieldSpan &field : fields) {
        if (field.path == QStringLiteral("config.head.12")) { currentPage = field; }
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
