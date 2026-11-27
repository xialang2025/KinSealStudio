/******************************************************************************
 * Function Name   : buildNativeMediaSnapshot
 * Description     : Fill the V122 single-page media archive's missing page-name QString on an independent source copy.
 * Input Parameters: exported source SM bytes, destination archive, optional error text
 * Return Value    : True when the complete edited source is compressed and verified.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool buildNativeMediaSnapshot(const QByteArray &source, QByteArray *archive, QString *error) //20261128-7 add
{
    NativeProjectEnvelope envelope;
    QList<NativeSourceFieldSpan> fields;
    NativeSourceFieldSpan currentPage, pageName, pageCount;
    QByteArray snapshot = source;
    QByteArray compressed;

    if (archive == nullptr || !inspectNativeProjectEnvelope(source, &envelope, error) ||
        !inspectNativeProjectSourceFields(source, &fields, error)) {
        return false;
    }
    if (envelope.version == 122) {
        for (const NativeSourceFieldSpan &field : fields) {
            if (field.path == QStringLiteral("config.head.12")) { currentPage = field; }
            if (field.path == QStringLiteral("page.head.18")) { pageName = field; }
            if (field.path == QStringLiteral("project.pages.count")) { pageCount = field; }
        }
        if (currentPage.kind != 'q' || pageName.kind != 'q' || pageCount.end - pageCount.start != 4) {
            if (error != nullptr) { *error = QStringLiteral("介质工程归档缺少页面名称或页面数量字段。"); }
            return false;
        }
        // The compiler snapshots the selected page after opening a newly saved single-page project.
        // Copy the serialized QString to preserve its exact UTF-16 representation and leave other fields untouched.
        if (source.mid(pageCount.start, 4) == QByteArray::fromHex("00000001") &&
            source.mid(currentPage.start, currentPage.end - currentPage.start) == QByteArray(4, char(0xff)) &&
            pageName.end - pageName.start > 4) {
            snapshot.replace(currentPage.start, currentPage.end - currentPage.start,
                             source.mid(pageName.start, pageName.end - pageName.start));
        }
    }
    compressed = qCompress(snapshot, 9);
    if (compressed.isEmpty() || qUncompress(compressed) != snapshot) {
        if (error != nullptr) { *error = QStringLiteral("介质可编辑工程归档压缩核验失败。"); }
        return false;
    }
    *archive = std::move(compressed);
    return true;
}
