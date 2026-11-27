/******************************************************************************
 * Function Name   : indexValidatedSource
 * Description     : Enumerate counted screens and the independent startup screen using verified root field spans.
 * Input Parameters: parsed source document
 * Return Value    : Project and page index.
 * Date            : 2026.11.27
 ******************************************************************************/
static NativeSourceIndex indexValidatedSource(const SourceDocument &document) //20261127-11 add
{
    NativeSourceIndex result;
    NativeSourcePage page;
    NativeSourceRoot root;
    QString prefix;
    QString objects;
    int ordinaryPages;
    int pages;
    int count;
    int p;
    int i;

    result.title = QFileInfo(document.value(QStringLiteral("trailer.settings.1")).toString()).completeBaseName();
    result.model = document.value(QStringLiteral("system.identity.1")).toString();
    result.version = document.value(QStringLiteral("header.1")).toInt();
    ordinaryPages = document.value(QStringLiteral("project.pages.count")).toInt();
    pages = ordinaryPages + int(document.values.contains(QStringLiteral("startup.head.18")));
    for (p = 0; p < pages; ++p) {
        prefix = p ? SourceDocument::indexed(QStringLiteral("page"), p) : QStringLiteral("page");
        if (p == ordinaryPages) { prefix = QStringLiteral("startup"); }
        objects = p ? prefix + QStringLiteral(".object") : QStringLiteral("object");
        page = NativeSourcePage();
        page.name = document.value(prefix + QStringLiteral(".head.18")).toString();
        page.number = document.value(prefix + QStringLiteral(".head.19")).toString();
        count = document.value(prefix + QStringLiteral(".objects.count")).toInt();
        for (i = 0; i < count; ++i) {
            root.prefix = SourceDocument::indexed(objects, i) + QLatin1Char('.');
            root.type = document.value(root.prefix + QStringLiteral("type")).toInt();
            root.movable = document.value(root.prefix + QStringLiteral("movable")).toInt() != 0;
            root.layer = document.value(root.prefix + QLatin1Char('z')).toDouble();
            root.name = document.value(root.prefix + QStringLiteral("group.metadata-name")).toString();
            if (root.name.isEmpty()) { root.name = document.value(root.prefix + QStringLiteral("box.name")).toString(); }
            page.roots.append(root);
        }
        result.pages.append(page);
    }
    return result;
}

/******************************************************************************
 * Function Name   : readNativeSourceIndex
 * Description     : Authenticate and index native page roots without projecting their private fields into Lite controls.
 * Input Parameters: source bytes, owner password, output index, error
 * Return Value    : True after validation through EOF and replacement of the index.
 * Date            : 2026.11.27
 ******************************************************************************/
bool readNativeSourceIndex(const QByteArray &data, const QString &password, NativeSourceIndex *index, QString *error) //20261127-11 add
{
    SourceDocument document(data);
    if (!index) { return fail(error, QStringLiteral("缺少原厂工程索引输出。")); }
    if (!document.parse()) { return fail(error, document.error); }
    if (!authenticatedSource(document, password, nullptr, nullptr, error)) { return false; }
    *index = indexValidatedSource(document);
    return true;
}

/******************************************************************************
 * Function Name   : editNativeSource
 * Description     : Re-encode only the document title and selected root lock or Z fields, and reparse the candidate.
 * Input Parameters: authenticated source, password, displayed title, root edits, output bytes, error
 * Return Value    : True after all requested edits validate; failure leaves the original output intact.
 * Date            : 2026.11.27
 ******************************************************************************/
bool editNativeSource(const QByteArray &data, const QString &password, const QString &title,
                      const QList<NativeSourceEdit> &edits, QByteArray *result, QString *error) //20261127-11 add
{
    NativeSourceIndex index;
    SourceDocument document(data);
    SourceUpdates changes(document);
    QHash<QString, NativeSourceRoot> roots;
    QSet<QString> seen;
    QByteArray candidate;
    QString original;
    QString fileName;
    int separator;
    int extension;

    if (!result) { return fail(error, QStringLiteral("缺少原厂工程输出。")); }
    if (!document.parse()) { return fail(error, document.error); }
    if (!authenticatedSource(document, password, nullptr, nullptr, error)) { return false; }
    index = indexValidatedSource(document);
    if (!title.isNull() && title != index.title) {
        if (title.trimmed().isEmpty() || title.size() > 256 || !title.isValidUtf16() ||
            title.contains(QRegularExpression(QStringLiteral(R"rx([\\/<>:"|?*\x00-\x1f])rx")))) {
            return fail(error, QStringLiteral("请输入有效的工程名称。"));
        }
        original = document.value(QStringLiteral("trailer.settings.1")).toString();
        separator = qMax(original.lastIndexOf(QLatin1Char('/')), original.lastIndexOf(QLatin1Char('\\')));
        fileName = original.mid(separator + 1);
        extension = fileName.lastIndexOf(QLatin1Char('.'));
        changes.set(QStringLiteral("trailer.settings.1"), original.left(separator + 1) + title +
                    (extension <= 0 ? QString() : fileName.mid(extension)));
    }
    for (const NativeSourcePage &page : std::as_const(index.pages)) {
        for (const NativeSourceRoot &root : page.roots) { roots.insert(root.prefix, root); }
    }
    for (const NativeSourceEdit &edit : edits) {
        const auto root = roots.constFind(edit.prefix);
        if (root == roots.constEnd() || seen.contains(edit.prefix) || !std::isfinite(edit.layer)) {
            return fail(error, QStringLiteral("原厂图元索引或层级无效。"));
        }
        seen.insert(edit.prefix);
        if (edit.layer != root->layer) { changes.set(edit.prefix + QLatin1Char('z'), edit.layer); }
        if (edit.movable != root->movable) { changes.set(edit.prefix + QStringLiteral("movable"), int(edit.movable)); }
    }
    candidate = changes.render();
    if (!changes.error.isEmpty()) { return fail(error, changes.error); }
    if (changes.replacements.isEmpty() && candidate != data) { return fail(error, QStringLiteral("原样保存验证失败。")); }
    SourceDocument verify(candidate);
    if (!verify.parse()) { return fail(error, verify.error); }
    *result = candidate;
    clearError(error);
    return true;
}

/******************************************************************************
 * Function Name   : saveNativeSourceFile
 * Description     : Atomically replace the destination only after the native stream is ready.
 * Input Parameters: target path, complete encoded bytes, error
 * Return Value    : True after filesystem commit.
 * Date            : 2026.11.27
 ******************************************************************************/
bool saveNativeSourceFile(const QString &path, const QByteArray &data, QString *error) //20261127-11 add
{
    QSaveFile dest(path);
    if (!dest.open(QIODevice::WriteOnly) || dest.write(data) != data.size() || !dest.commit()) {
        return fail(error, dest.errorString());
    }
    clearError(error);
    return true;
}
