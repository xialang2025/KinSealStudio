//20261127-8 add: Keep V122 ordinary-page source editing independent of the single-page device compiler.
namespace {
/******************************************************************************
 * Function Name   : nativePagePath
 * Description     : Return the path assigned by the bounded native source parser.
 * Input Parameters: page index
 * Return Value    : Parsed field prefix for the requested page.
 * Date            : 2026.11.27
 ******************************************************************************/
QString nativePagePath(int index)
{
    return index == 0 ? QStringLiteral("page") : SourceDocument::indexed(QStringLiteral("page"), index);
}

/******************************************************************************
 * Function Name   : nativePageEnd
 * Description     : Find the exclusive end of a complete page including grouped controls.
 * Input Parameters: document, page index
 * Return Value    : Offset of the next page or the macro history.
 * Date            : 2026.11.27
 ******************************************************************************/
qsizetype nativePageEnd(const SourceDocument &document, int index)
{
    const QString next = index + 1 == document.value(QStringLiteral("project.pages.count")).toInt()
        ? QStringLiteral("history.1e4.count") : nativePagePath(index + 1) + QStringLiteral(".head.0");
    return document.values.value(next).start;
}

/******************************************************************************
 * Function Name   : nativePageProjection
 * Description     : Decode one ordinary page with the same checked controls and global data used for single-page SM.
 * Input Parameters: document, page index, output project, error
 * Return Value    : True when all records of the isolated page were read.
 * Date            : 2026.11.27
 ******************************************************************************/
bool nativePageProjection(const SourceDocument &document, int index, HmiProject *project, QString *error)
{
    SourceUpdates updates(document);
    const QString prefix = nativePagePath(index);
    const QString name = document.value(prefix + QStringLiteral(".head.18")).toString();
    const qsizetype start = document.values.value(prefix + QStringLiteral(".head.0")).start;
    const qsizetype allStart = document.values.value(QStringLiteral("page.head.0")).start;
    const qsizetype allEnd = document.values.value(QStringLiteral("history.1e4.count")).start;
    const qsizetype mapStart = document.values.value(QStringLiteral("trailer.page-map.count")).start;
    QByteArray localMap = QByteArray::fromHex("000000010000000000000000");
    SourceDocument single(QByteArray{});

    if (document.value(prefix + QStringLiteral(".head.17")).toInt() != 0 ||
        document.value(prefix + QStringLiteral(".head.2")).toDouble() != 800 || document.value(prefix + QStringLiteral(".head.3")).toDouble() != 480)
    {
        return fail(error, QStringLiteral("多页源工程含窗口、键盘或非 800×480 普通画面，无法无损映射。"));
    }
    if (document.value(prefix + QStringLiteral(".head.4")).value<QColor>() != HmiPageBackground::defaultColor() ||
        document.value(prefix + QStringLiteral(".head.5")).value<QColor>() != HmiPageBackground::defaultColor() ||
        document.value(prefix + QStringLiteral(".head.6")).toInt() != 0 || document.value(prefix + QStringLiteral(".head.7")).toInt() != 0 ||
        !document.value(prefix + QStringLiteral(".head.8")).toString().isEmpty() || document.value(prefix + QStringLiteral(".94.count")).toInt() != 0)
    {
        return fail(error, QStringLiteral("原厂多页含非默认背景或画面级地址记录，当前不能无损还原其编辑属性。"));
    }
    for (int slot = 5; slot <= 8; ++slot)
    {
        if (document.value(prefix + QStringLiteral(".tail.") + QString::number(slot)).toInt() != 0)
        {
            return fail(error, QStringLiteral("原厂普通画面含活动导航设置，当前不能无损还原该配置。"));
        }
    }
    // The temporary projection is never returned as the user's source snapshot.
    updates.set(QStringLiteral("project.pages.count"), 1);
    updates.replacements.insert(allStart, qMakePair(allEnd, document.bytes.mid(start, nativePageEnd(document, index) - start)));
    updates.replacements.insert(mapStart, qMakePair(document.bytes.size(), localMap));
    for (const QString &key : {QStringLiteral("config.head.4"), QStringLiteral("config.head.11"), QStringLiteral("trailer.settings.11")})
    {
        updates.set(key, name);
    }
    single.bytes = updates.render();
    if (!updates.error.isEmpty() || !single.parse()) { return fail(error, updates.error.isEmpty() ? single.error : updates.error); }
    return sourceProject(single, project, error);
}

/******************************************************************************
 * Function Name   : importOrdinaryPages
 * Description     : Assemble all editable pages in serialized order, with globally unique editor object IDs.
 * Input Parameters: document, output project, error
 * Return Value    : True after validating the complete model and its untouched original snapshot.
 * Date            : 2026.11.27
 ******************************************************************************/
bool importOrdinaryPages(const SourceDocument &document, HmiProject *project, QString *error)
{
    const int count = document.value(QStringLiteral("project.pages.count")).toInt();
    const QString first = document.value(QStringLiteral("page.head.18")).toString();
    HmiProject model;
    HmiProject local;
    HmiPage page;
    QSet<int> numbers;
    QString map;

    if (document.value(QStringLiteral("trailer.page-map.count")).toInt() != count || document.value(QStringLiteral("config.head.4")).toString() != first ||
        document.value(QStringLiteral("config.head.11")).toString() != first || document.value(QStringLiteral("trailer.settings.11")).toString() != first)
    {
        return fail(error, QStringLiteral("多页源工程的启动画面或索引表不符合已核实的原厂布局。"));
    }
    for (int i = 0; i < count; ++i)
    {
        map = SourceDocument::indexed(QStringLiteral("trailer.page-map"), i);
        if (document.value(map + QStringLiteral(".0")).toInt() != count - i - 1 || document.value(map + QStringLiteral(".1")).toInt() != count - i - 1)
        {
            return fail(error, QStringLiteral("多页源工程的页索引对含尚未核实的排列。"));
        }
        if (!nativePageProjection(document, i, &local, error)) { return false; }
        page = local.allPages().first();
        page.number = local.activePageNumber.value_or(0);
        if (numbers.contains(*page.number)) { return fail(error, QStringLiteral("原厂普通画面编号重复。")); }
        numbers.insert(*page.number);
        if (i == 0)
        {
            model = local;
            model.activePageNumber = page.number;
            continue;
        }
        page.id = QStringLiteral("page-%1").arg(i + 1);
        for (HmiObject &object : page.objects) { object.id.prepend(page.id + QLatin1Char('/')); }
        model.inactivePages.append(page);
        model.pageOrder.append(page.id);
        if (model.connections.isEmpty()) { model.connections = local.connections; }
        for (const QString &warning : local.nativeSourceWarnings)
        {
            if (model.nativeSourceWarnings.size() < 128 && !model.nativeSourceWarnings.contains(warning)) { model.nativeSourceWarnings.append(warning); }
        }
    }
    model.nativeSourceSnapshot = document.bytes;
    if (!model.validateModel(error)) { return false; }
    *project = model;
    clearError(error);
    return true;
}

/******************************************************************************
 * Function Name   : sourceGlobalProperties
 * Description     : Compare all global settings outside the page and project-name edits handled here.
 * Input Parameters: project
 * Return Value    : Comparable global editor settings.
 * Date            : 2026.11.27
 ******************************************************************************/
QJsonObject sourceGlobalProperties(const HmiProject &project)
{
    QJsonObject root = project.toJson();
    for (const QString &key : {QStringLiteral("pages"), QStringLiteral("activePageId"), QStringLiteral("projectName"),
                               QStringLiteral("nativeSourceWarnings"), QStringLiteral("nativeSourceSnapshot"), QStringLiteral("schemaVersion")})
    {
        root.remove(key);
    }
    if (root.value(QStringLiteral("animations")).isObject() && root.value(QStringLiteral("animations")).toObject().isEmpty()) { root.remove(QStringLiteral("animations")); }
    return root;
}

/******************************************************************************
 * Function Name   : exportOrdinaryPages
 * Description     : Patch page headers and encoded control lists at verified spans while preserving all unrelated bytes.
 * Input Parameters: project, fully parsed original document, output bytes, error
 * Return Value    : True after checking that the complete edited project can be reopened.
 * Date            : 2026.11.27
 ******************************************************************************/
bool exportOrdinaryPages(const HmiProject &project, const SourceDocument &document, QByteArray *data, QString *error)
{
    HmiProject original;
    HmiProject local;
    HmiProject verified;
    SourceUpdates updates(document);
    QList<HmiPage> previous;
    QList<HmiPage> pages;
    QJsonObject oldMetadata;
    QJsonObject newMetadata;
    QMap<QString, QString> pageLabels;
    QMap<QString, QString> localIds;
    QString nativeNumber;
    QString prefix;
    QString key;
    qsizetype start;
    QByteArray encoded;
    QByteArray result;

    if (document.value(QStringLiteral("config.270.0")).toInt() != 0)
    {
        if (!authenticateNativeProjectFile(document.bytes, project.filePassword, nullptr, nullptr, error)) { return false; }
    }
    else if (!project.filePassword.isEmpty())
    {
        return fail(error, QStringLiteral("当前文件密码无法保留到这份原厂 .sm；请保存为受保护的 .ksp。"));
    }
    if (!project.validateModel(error) || !importOrdinaryPages(document, &original, error)) { return false; }
    if (sourceGlobalProperties(original) != sourceGlobalProperties(project))
    {
        return fail(error, QStringLiteral("多页源写回支持原有普通画面及控件编辑；本次全局设置或页面结构变更尚未核实。"));
    }
    pages = project.allPages();
    previous = original.allPages();
    if (previous.size() != pages.size()) { return fail(error, QStringLiteral("多页源工程的页面数量已改变。")); }
    if (project.name != original.name) { updates.set(QStringLiteral("trailer.settings.1"), project.name + QStringLiteral(".sm")); }
    for (int i = 0; i < pages.size(); ++i)
    {
        const HmiPage &old = previous.at(i);
        const HmiPage &page = pages.at(i);
        oldMetadata = old.toJson();
        newMetadata = page.toJson();
        if (oldMetadata == newMetadata) { continue; }
        for (const QString &field : {QStringLiteral("name"), QStringLiteral("number"), QStringLiteral("objects")})
        {
            oldMetadata.remove(field);
            newMetadata.remove(field);
        }
        if (oldMetadata != newMetadata || !page.number.has_value())
        {
            return fail(error, QStringLiteral("多页源写回尚未核实画面类型、扩展属性、尺寸或背景编辑。"));
        }
        prefix = nativePagePath(i);
        nativeNumber = QStringLiteral("%1").arg(*page.number, 3, 10, QLatin1Char('0'));
        if (old.name != page.name || old.number != page.number)
        {
            pageLabels.insert(document.value(prefix + QStringLiteral(".head.19")).toString() + QStringLiteral(":") + old.name, nativeNumber + QStringLiteral(":") + page.name);
        }
        if (old.number != page.number) { updates.set(prefix + QStringLiteral(".head.19"), nativeNumber); }
        if (old.name != page.name)
        {
            if (std::count_if(previous.cbegin(), previous.cend(), [&](const HmiPage &entry) { return entry.name == old.name; }) != 1)
            {
                return fail(error, QStringLiteral("原厂画面重名，无法确定不带编号的旧名称引用。"));
            }
            // The startup page stores bare names in its four navigation slots; later pages use NNN:name.
            pageLabels.insert(old.name, page.name);
            updates.set(prefix + QStringLiteral(".head.18"), page.name);
            if (i == 0)
            {
                updates.set(QStringLiteral("config.head.4"), page.name);
                updates.set(QStringLiteral("config.head.11"), page.name);
                updates.set(QStringLiteral("trailer.settings.11"), page.name);
            }
        }
        if (old.toJson().value(QStringLiteral("objects")) == page.toJson().value(QStringLiteral("objects"))) { continue; }
        if (!nativePageProjection(document, i, &local, error)) { return false; }
        local.filePassword = project.filePassword;
        localIds.clear();
        for (int j = 0; j < old.objects.size(); ++j) { localIds.insert(old.objects.at(j).id, local.objects.at(j).id); }
        local.objects = page.objects;
        for (HmiObject &object : local.objects) { object.id = localIds.value(object.id, object.id); }
        if (!exportNativeProjectSource(local, &encoded, error)) { return false; }
        SourceDocument patched(encoded);
        if (!patched.parse()) { return fail(error, patched.error); }
        start = patched.values.value(QStringLiteral("page.objects.count")).start;
        updates.replacements.insert(document.values.value(prefix + QStringLiteral(".objects.count")).start,
                                    qMakePair(nativePageEnd(document, i), encoded.mid(start, nativePageEnd(patched, 0) - start)));
    }
    for (int i = 0; i < pages.size() && !pageLabels.isEmpty(); ++i)
    {
        for (int slot = 9; slot <= 12; ++slot)
        {
            key = nativePagePath(i) + QStringLiteral(".tail.") + QString::number(slot);
            if (pageLabels.contains(document.value(key).toString())) { updates.set(key, pageLabels.value(document.value(key).toString())); }
        }
    }
    result = updates.render();
    if (!updates.error.isEmpty()) { return fail(error, updates.error); }
    if (!importNativeProjectSource(result, &verified, error)) { return false; }
    *data = result;
    clearError(error);
    return true;
}

/******************************************************************************
 * Function Name   : extendOrdinaryPageSource
 * Description     : Append verified default ordinary-page records to a newly encoded or preserved native page list.
 * Input Parameters: project, retained page count, output bytes, error
 * Return Value    : True after validating the assembled SM with its full set of pages.
 * Date            : 2026.11.27
 ******************************************************************************/
bool extendOrdinaryPageSource(const HmiProject &project, int retained, QByteArray *data, QString *error)
{
    const QList<HmiPage> pages = project.allPages();
    const QMap<QString, int> numbers = project.screenAndWindowNumbers();
    QFile seedFile(QStringLiteral(":/native/ordinary-pages.sm"));
    SourceDocument seed(QByteArray{});
    SourceDocument baseline(QByteArray{});
    SourceUpdates combined(baseline);
    HmiProject base = project;
    HmiProject local;
    HmiProject checked;
    QByteArray encoded;
    QByteArray map;
    QByteArray blocks;
    QByteArray result;
    QString startupLabel;
    qsizetype objects;
    int i;
    int slot;

    if (!project.validateModel(error)) { return false; }
    if (retained < 1 || retained >= pages.size() || pages.first().id != project.startupPageId ||
        !project.references.isEmpty() || !project.configurationNodes.isEmpty())
    {
        return fail(error, QStringLiteral("追加普通多页源要求首个画面为启动页，且不含跨页引用或配置节点。"));
    }
    for (i = 0; i < pages.size(); ++i)
    {
        const HmiPage &page = pages.at(i);
        if (page.kind != QStringLiteral("normal") || page.width != 800 || page.height != 480 ||
            !page.background.isDefault() || !page.extensions.isEmpty() || page.keyboardType.has_value())
        {
            return fail(error, QStringLiteral("追加多页源仅支持默认背景的 800×480 普通画面。"));
        }
    }
    if (!seedFile.open(QIODevice::ReadOnly)) { return fail(error, QStringLiteral("缺少原厂普通多页源样本。")); }
    seed.bytes = seedFile.readAll();
    if (!seed.parse() || seed.value(QStringLiteral("project.pages.count")).toInt() != 2)
    {
        return fail(error, QStringLiteral("内置普通多页源样本结构无效。"));
    }
    if (!base.selectPage(pages.first().id, error)) { return false; }
    base.pageOrder = project.pageOrder.mid(0, retained);
    base.inactivePages.removeIf([&](const HmiPage &p) { return !base.pageOrder.contains(p.id); });
    if (project.nativeSourceSnapshot.isEmpty()) { base.activePageNumber = numbers.value(base.activePageId); }
    if (!exportNativeProjectSource(base, &baseline.bytes, error)) { return false; }
    if (!baseline.parse() || baseline.value(QStringLiteral("project.pages.count")).toInt() != retained)
    {
        return fail(error, QStringLiteral("已编码画面数与原厂源索引不符。"));
    }
    if (retained == 1 && !project.nativeSourceSnapshot.isEmpty() &&
        (pages.first().id != QStringLiteral("page-1") || baseline.value(QStringLiteral("trailer.page-map.count")).toInt() != 1 ||
         baseline.value(QStringLiteral("trailer.page-map[0].0")).toInt() != 0 || baseline.value(QStringLiteral("trailer.page-map[0].1")).toInt() != 0))
    {
        return fail(error, QStringLiteral("尚未核实此原厂单页的页面索引，不能追加画面。"));
    }
    if (!nativePageProjection(baseline, 0, &local, error)) { return false; }
    local.filePassword = project.filePassword;
    startupLabel = QStringLiteral("%1:%2").arg(numbers.value(pages.first().id), 3, 10, QLatin1Char('0')).arg(pages.first().name);
    blocks = baseline.bytes.mid(baseline.values.value(QStringLiteral("page.head.0")).start,
                                nativePageEnd(baseline, retained - 1) - baseline.values.value(QStringLiteral("page.head.0")).start);
    if (project.nativeSourceSnapshot.isEmpty())
    {
        SourceUpdates startup(baseline);
        // Native B stores bare startup labels in the first page and numbered labels in each new page.
        for (slot = 9; slot <= 12; ++slot) { startup.set(QStringLiteral("page.tail.") + QString::number(slot), pages.first().name); }
        blocks = startup.render(baseline.values.value(QStringLiteral("page.head.0")).start, nativePageEnd(baseline, 0));
        if (!startup.error.isEmpty()) { return fail(error, startup.error); }
    }
    appendWord(&map, static_cast<u32>(pages.size()));
    for (i = 0; i < pages.size(); ++i)
    {
        appendWord(&map, static_cast<u32>(pages.size() - i - 1));
        appendWord(&map, static_cast<u32>(pages.size() - i - 1));
        if (i < retained) { continue; }
        const HmiPage &page = pages.at(i);
        SourceUpdates secondary(seed);
        local.objects = page.objects;
        for (int j = 0; j < local.objects.size(); ++j)
        {
            // Temporary IDs select new records in the single-page encoder, independently of long editor IDs.
            local.objects[j].id = QStringLiteral("added-page-object-%1").arg(j);
        }
        local.pageName = page.name;
        local.activePageNumber = numbers.value(page.id);
        if (!exportNativeProjectSource(local, &encoded, error)) { return false; }
        SourceDocument generated(encoded);
        if (!generated.parse()) { return fail(error, generated.error); }
        secondary.set(QStringLiteral("page[1].head.18"), page.name);
        secondary.set(QStringLiteral("page[1].head.19"), QStringLiteral("%1").arg(numbers.value(page.id), 3, 10, QLatin1Char('0')));
        for (slot = 9; slot <= 12; ++slot) { secondary.set(QStringLiteral("page[1].tail.") + QString::number(slot), startupLabel); }
        blocks.append(secondary.render(seed.values.value(QStringLiteral("page[1].head.0")).start, seed.values.value(QStringLiteral("page[1].objects.count")).start));
        if (!secondary.error.isEmpty()) { return fail(error, secondary.error); }
        objects = generated.values.value(QStringLiteral("page.objects.count")).start;
        blocks.append(encoded.mid(objects, nativePageEnd(generated, 0) - objects));
        if (blocks.size() > 16 * 1024 * 1024) { return fail(error, QStringLiteral("普通多页源超过工程快照容量。")); }
    }
    combined.set(QStringLiteral("project.pages.count"), pages.size());
    combined.replacements.insert(baseline.values.value(QStringLiteral("page.head.0")).start, qMakePair(nativePageEnd(baseline, retained - 1), blocks));
    combined.replacements.insert(baseline.values.value(QStringLiteral("trailer.page-map.count")).start, qMakePair(baseline.bytes.size(), map));
    result = combined.render();
    if (!combined.error.isEmpty()) { return fail(error, combined.error); }
    if (!importNativeProjectSource(result, &checked, error)) { return false; }
    *data = result;
    clearError(error);
    return true;
}
} // namespace
