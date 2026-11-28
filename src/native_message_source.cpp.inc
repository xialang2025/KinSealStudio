//20261128-23 add
/******************************************************************************
 * Function Name   : sourceMessageDisplay
 * Description     : Read both V122 message tables, preserving native-only preview and row metadata in the snapshot.
 * Input Parameters: parsed source, object prefix and ordinal, ordered project languages, output and diagnostics
 * Return Value    : True when the language and state matrix can be edited without losing its meaning.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool sourceMessageDisplay(const SourceDocument &d, const QString &p, int index, const QStringList &languages,
                                 HmiObject *object, QStringList *warnings, bool *external, QString *error)
{
    HmiObject result;
    MessageDisplayProperties &m = result.messageDisplay;
    const QString base = p + QStringLiteral("private.");
    const QString rect = p + QStringLiteral("child[0].rectangle.");
    const int count = d.value(base + QStringLiteral("states.count")).toInt();
    const double x = d.value(rect + QStringLiteral("0")).toDouble() + d.value(rect + QStringLiteral("9")).toDouble();
    const double y = d.value(rect + QStringLiteral("1")).toDouble() + d.value(rect + QStringLiteral("10")).toDouble();
    const double w = d.value(rect + QStringLiteral("2")).toDouble(), h = d.value(rect + QStringLiteral("3")).toDouble();
    QString simple, ext, item, full, language;
    StaticTextProperties visual;
    QColor border;
    int i, j, k;

    if (languages.isEmpty() || count < 1 || count > 128 || count != d.value(base + QStringLiteral("head.4")).toInt() ||
        d.value(base + QStringLiteral("head.0")).toInt() < 0 || d.value(base + QStringLiteral("head.0")).toInt() >= count ||
        d.value(base + QStringLiteral("extended")).toInt() < 0 || d.value(base + QStringLiteral("extended")).toInt() >= languages.size() ||
        d.value(p + QStringLiteral("children.count")).toInt() != 3 || d.value(p + QStringLiteral("child[0].rectangle.13")).toString() != QStringLiteral("mFramRect") ||
        d.value(p + QStringLiteral("child[1].rectangle.13")).toString() != QStringLiteral("mBackRect") || d.value(p + QStringLiteral("child[2].text.tail.8")).toString() != QStringLiteral("mText") ||
        w < 1 || h < 1 || w > 4096 || h > 4096 || std::abs(x) > 32768 || std::abs(y) > 32768) {
        return fail(error, QStringLiteral("消息控件的预览、状态或语言范围无法映射。"));
    }
    m.stateCount = count;
    m.ensureStates(count);
    m.dataType = d.value(base + QStringLiteral("head.2")).toInt();
    m.movement = d.value(base + QStringLiteral("head.5")).toInt();
    m.speed = d.value(base + QStringLiteral("head.6")).toInt();
    if (!modelSourceAddress(d, base + QStringLiteral("head.3"), &m.monitorAddress, external, error)) { return false; }
    m.appearance = sourceCondition(d, p, false, warnings);
    for (i = 0; i < count; ++i) {
        simple = SourceDocument::indexed(base + QStringLiteral("state"), i);
        ext = SourceDocument::indexed(base + QStringLiteral("extendedState"), i);
        if (d.value(simple + QStringLiteral(".languages.count")).toInt() != languages.size() ||
            d.value(ext + QStringLiteral(".languages.count")).toInt() != languages.size()) {
            return fail(error, QStringLiteral("消息源的语言矩阵与工程不符。"));
        }
        for (j = 0; j < languages.size(); ++j) {
            language = languages.at(j);
            item = SourceDocument::indexed(simple + QStringLiteral(".language"), j) + QStringLiteral(".");
            full = SourceDocument::indexed(ext + QStringLiteral(".language"), j) + QStringLiteral(".");
            for (k = 0; k < 4; ++k) {
                if (d.value(item + QString::number(k)) != d.value(full + QString::number(k))) {
                    return fail(error, QStringLiteral("消息兼容表和扩展表的活动字段不一致。"));
                }
            }
            if (d.value(full + QStringLiteral("0")).toInt() != i) { return fail(error, QStringLiteral("消息源包含非连续状态编号。")); }
            m.states[i].insert(language, d.value(full + QStringLiteral("1")).toString());
            visual.fontSize = d.value(full + QStringLiteral("2")).toInt();
            visual.textColor = d.value(full + QStringLiteral("3")).value<QColor>();
            visual.fontFamily = d.value(full + QStringLiteral("4")).toString();
            visual.backgroundColor = d.value(full + QStringLiteral("5")).value<QColor>();
            visual.foregroundColor = d.value(full + QStringLiteral("6")).value<QColor>();
            border = d.value(full + QStringLiteral("7")).value<QColor>();
            visual.style = d.value(full + QStringLiteral("8")).toInt();
            if (i && (m.languageAppearances.value(language).toJson() != visual.toJson() ||
                      m.languageFrameColors.value(language) != border)) {
                return fail(error, QStringLiteral("原厂各状态外观不同，无法合并为一套语言样式。"));
            }
            m.languageAppearances.insert(language, visual);
            m.languageFrameColors.insert(language, border);
        }
    }
    if (!m.validate(error)) { return false; }
    result.id = QStringLiteral("native-message-%1").arg(index);
    result.type = QStringLiteral("message");
    result.name = d.value(p + QStringLiteral("group.metadata-name")).toString();
    if (result.name.isEmpty()) { result.name = QStringLiteral("消息显示器%1").arg(index + 1); }
    result.geometry = QRect(int(std::floor(x)), int(std::floor(y)), qRound(w), qRound(h));
    result.subpixelOffset = QPointF(x - std::floor(x), y - std::floor(y));
    result.text = m.text(0, languages.first());
    result.address = m.monitorAddress;
    *object = result;
    return true;
}

/******************************************************************************
 * Function Name   : writeMessageRows
 * Description     : Rebuild the counted state/language lists in source order while retaining untouched row bytes.
 * Input Parameters: source, private prefix, old and new models, original and edited languages, cached language, output
 * Return Value    : True when every language style and translation has a native representation.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool writeMessageRows(const SourceDocument &d, const QString &base, const MessageDisplayProperties &old,
                             const MessageDisplayProperties &m, const QStringList &oldNames, const QStringList &names,
                             int selected, QByteArray *bytes, QString *error)
{
    QString row, cell, last;
    int pass, i, j, seedState, seedLang;
    QByteArray data;
    const auto patch = [&](SourceUpdates &u, const QString &name, const QVariant &value) {
        if (d.value(name) != value) { u.set(name, value); }
    };
    appendWord(&data, u32(m.stateCount));
    for (pass = 0; pass < 2; ++pass) {
        if (pass) { data.append(char(selected >> 8)); data.append(char(selected)); }
        for (i = 0; i < m.stateCount; ++i) {
            seedState = qMin(i, old.stateCount - 1);
            row = SourceDocument::indexed(base + (pass ? QStringLiteral("extendedState") : QStringLiteral("state")), seedState);
            SourceUpdates head(d);
            const StaticTextProperties selectedStyle = m.languageAppearance(names.at(selected));
            const int oldSelected = d.value(base + QStringLiteral("extended")).toInt();
            const QString oldName = oldNames.at(oldSelected);
            const StaticTextProperties oldStyle = old.languageAppearance(oldName);
            // The redundant row header describes the saved preview; retain its raw cache until that style changes.
            if (old.languageFrameColor(oldName) != m.languageFrameColor(names.at(selected))) { head.set(row + QStringLiteral(".head.0"), m.languageFrameColor(names.at(selected))); }
            if (oldStyle.foregroundColor != selectedStyle.foregroundColor) { head.set(row + QStringLiteral(".head.1"), selectedStyle.foregroundColor); }
            if (oldStyle.backgroundColor != selectedStyle.backgroundColor) { head.set(row + QStringLiteral(".head.2"), selectedStyle.backgroundColor); }
            if (oldStyle.style != selectedStyle.style) { head.set(row + QStringLiteral(".head.3"), selectedStyle.style); }
            patch(head, row + QStringLiteral(".languages.count"), int(names.size()));
            data.append(head.render(d.values.value(row + QStringLiteral(".languages.count")).start, d.values.value(row + QStringLiteral(".language[0].0")).start));
            if (!head.error.isEmpty()) { return fail(error, head.error); }
            for (j = 0; j < names.size(); ++j) {
                const QString name = names.at(j);
                const StaticTextProperties visual = m.languageAppearance(name);
                seedLang = qMax(0, int(oldNames.indexOf(name)));
                cell = SourceDocument::indexed(row + QStringLiteral(".language"), seedLang) + QStringLiteral(".");
                SourceUpdates u(d);
                if (visual.fontSize != int(visual.fontSize)) { return fail(error, QStringLiteral("原厂消息字号必须是整数。")); }
                patch(u, cell + QStringLiteral("0"), i);
                patch(u, cell + QStringLiteral("1"), m.text(i, name));
                patch(u, cell + QStringLiteral("2"), int(visual.fontSize));
                patch(u, cell + QStringLiteral("3"), visual.textColor);
                if (pass) {
                    patch(u, cell + QStringLiteral("4"), visual.fontFamily);
                    patch(u, cell + QStringLiteral("5"), visual.backgroundColor);
                    patch(u, cell + QStringLiteral("6"), visual.foregroundColor);
                    patch(u, cell + QStringLiteral("7"), m.languageFrameColor(name));
                    patch(u, cell + QStringLiteral("8"), visual.style);
                }
                last = cell + (pass ? QStringLiteral("9") : QStringLiteral("3"));
                data.append(u.render(d.values.value(cell + QStringLiteral("0")).start, d.values.value(last).end));
                if (!u.error.isEmpty()) { return fail(error, u.error); }
            }
        }
    }
    *bytes = data;
    return true;
}

/******************************************************************************
 * Function Name   : writeNativeMessage
 * Description     : Save edited message records, their preview children and common visibility predicate.
 * Input Parameters: edited and original objects, source or default seed, source prefix, language lists, output, diagnostic
 * Return Value    : True after the complete type-35 object body is emitted.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool writeNativeMessage(const HmiObject &o, const HmiObject *prior, const SourceDocument &source, QString p,
                               QStringList oldNames, const QStringList &names, QByteArray *output, QString *error)
{
    QFile seedFile(QStringLiteral(":/native/message-default.sm"));
    SourceDocument seed(QByteArray{});
    const SourceDocument *d = &source;
    const MessageDisplayProperties &m = o.messageDisplay;
    const MessageDisplayProperties authoredDefaults;
    HmiObject baseline;
    QStringList warnings;
    QString base, child, pos, local, key, oldSelected;
    QByteArray rows;
    bool external = false;
    qsizetype begin, end;
    int selected, oldState, previewState, i, j;
    double scale, before, delta;
    if (!prior) {
        if (!seedFile.open(QIODevice::ReadOnly)) { return fail(error, seedFile.errorString()); }
        seed.bytes = seedFile.readAll();
        if (!seed.parse() || seed.objectPrefixes.size() != 1) { return fail(error, seed.error); }
        d = &seed; p = seed.objectPrefixes.first(); oldNames = QStringList{QStringLiteral("中文")};
        if (!sourceMessageDisplay(seed, p, 0, oldNames, &baseline, &warnings, &external, error)) { return false; }
        prior = &baseline;
    }
    const MessageDisplayProperties &old = prior->messageDisplay;
    SourceUpdates u(*d);
    if (names.isEmpty() || names.size() > 32 || o.hasAppearanceFlip() || !o.extensions.isEmpty() ||
        o.address != m.monitorAddress || (!o.text.isEmpty() && o.text != m.text(0, names.first()))) {
        return fail(error, QStringLiteral("消息控件的根文字、地址、翻转或扩展与原厂结构不符。"));
    }
    if (!m.validate(error)) { return false; }
    // A V122 row encodes just the registered languages in their project order. An unregistered KSP draft has no slot.
    const auto registered = [&names](const auto &entries) {
        for (auto it = entries.cbegin(); it != entries.cend(); ++it) {
            if (!names.contains(it.key())) { return false; }
        }
        return true;
    };
    if (!registered(m.languageAppearances) || !registered(m.languageFrameColors)) {
        return fail(error, QStringLiteral("消息外观包含未注册的工程语言，无法写入原厂语言表。"));
    }
    for (const auto &translations : m.states) {
        if (!registered(translations)) {
            return fail(error, QStringLiteral("消息文本包含未注册的工程语言，无法写入原厂语言表。"));
        }
    }
    base = p + QStringLiteral("private.");
    oldState = d->value(base + QStringLiteral("head.0")).toInt(); //20261128-23 change
    previewState = qMin(oldState, m.stateCount - 1);
    if (previewState != oldState) { u.set(base + QStringLiteral("head.0"), previewState); }
    oldSelected = oldNames.at(d->value(base + QStringLiteral("extended")).toInt());
    selected = qMax(0, int(names.indexOf(oldSelected)));
    const QString name = names.at(selected);
    const StaticTextProperties visual = m.languageAppearance(name), oldVisual = old.languageAppearance(oldSelected);
    if (o.name != prior->name || d == &seed) { u.set(p + QStringLiteral("group.metadata-name"), o.name); }
    if (m.dataType != old.dataType) { u.set(base + QStringLiteral("head.2"), m.dataType); }
    if (m.monitorAddress != old.monitorAddress) { u.address(base + QStringLiteral("head.3"), m.monitorAddress); }
    if (m.stateCount != old.stateCount) { u.set(base + QStringLiteral("head.4"), m.stateCount); }
    if (m.movement != old.movement) { u.set(base + QStringLiteral("head.5"), m.movement); }
    if (m.speed != old.speed) { u.set(base + QStringLiteral("head.6"), m.speed); }
    QJsonObject oldGate = old.appearance.toJson(), newGate = m.appearance.toJson();
    const QStringList visualKeys = {QStringLiteral("fontFamily"),QStringLiteral("fontSize"),QStringLiteral("textColor"),QStringLiteral("foregroundColor"),QStringLiteral("backgroundColor"),QStringLiteral("style")};
    const bool allStyleOverrides = std::all_of(names.cbegin(), names.cend(),
        [&m](const QString &language) { return m.languageAppearances.contains(language); });
    const bool allFrameOverrides = std::all_of(names.cbegin(), names.cend(),
        [&m](const QString &language) { return m.languageFrameColors.contains(language); });
    const QJsonObject editorDefaults = authoredDefaults.appearance.toJson();
    if (allStyleOverrides) {
        for (const QString &v : visualKeys) {
            if (oldGate.value(v) != newGate.value(v) && (d != &seed || newGate.value(v) != editorDefaults.value(v))) {
                return fail(error, QStringLiteral("所有消息语言都已有独立样式，修改的基准样式在原厂源中没有单独字段。"));
            }
        }
    }
    if (allFrameOverrides && old.frameColor != m.frameColor && (d != &seed || m.frameColor != authoredDefaults.frameColor)) {
        return fail(error, QStringLiteral("所有消息语言都已有独立框色，修改的基准框色在原厂源中没有单独字段。"));
    }
    for (const QString &v : visualKeys) { oldGate.remove(v); newGate.remove(v); }
    if (!patchCondition(oldGate, newGate, p, false, &u)) { return fail(error, QStringLiteral("消息显现条件或样式含未映射的设置。")); }
    for (const QString &language : names) {
        QJsonObject a = old.languageAppearance(oldNames.contains(language) ? language : oldNames.first()).toJson();
        QJsonObject b = m.languageAppearance(language).toJson();
        for (const QString &v : visualKeys) { a.remove(v); b.remove(v); }
        // A language entry has no private visibility predicate; changing nonvisual per-language fields would lose data.
        if (a != b && m.languageAppearances.contains(language)) { return fail(error, QStringLiteral("消息语言外观包含无法编码的属性。")); }
    }
    for (i = 0; i < 3; ++i) {
        child = SourceDocument::indexed(p + QStringLiteral("child"), i) + QStringLiteral(".");
        local = child + (i == 2 ? QStringLiteral("text.head.") : QStringLiteral("rectangle."));
        pos = child + (i == 2 ? QStringLiteral("text.tail.") : QStringLiteral("rectangle."));
        for (j = 0; j < 2; ++j) {
            delta = j ? o.geometry.y() + o.subpixelOffset.y() - prior->geometry.y() - prior->subpixelOffset.y()
                      : o.geometry.x() + o.subpixelOffset.x() - prior->geometry.x() - prior->subpixelOffset.x();
            key = pos + QString::number((i == 2 ? 10 : 9) + j);
            if (delta) { u.set(key, d->value(key).toDouble() + delta); }
            scale = j ? double(o.geometry.height()) / prior->geometry.height() : double(o.geometry.width()) / prior->geometry.width();
            if (scale != 1) {
                key = local + QString::number(j);
                before = d->value(key).toDouble();
                u.set(key, before * scale);
                key = local + QString::number(j + 2);
                u.set(key, d->value(key).toDouble() * scale);
            }
        }
    }
    child = p + QStringLiteral("child[2].");
    // V122 stores both the active preview row and a separate text child rendered by the native editor.
    if (previewState != oldState || old.text(oldState, oldSelected) != m.text(previewState, name)) { //20261128-23 change
        u.set(child + QStringLiteral("text.head.4"), m.text(previewState, name));
    }
    if (oldVisual.fontFamily != visual.fontFamily) { u.set(child + QStringLiteral("font.family"), visual.fontFamily); }
    if (oldVisual.fontSize != visual.fontSize) { u.set(child + QStringLiteral("font.tail.1"), int(visual.fontSize)); }
    if (oldVisual.textColor != visual.textColor) { u.set(child + QStringLiteral("text.tail.0"), visual.textColor); }
    if (oldVisual.foregroundColor != visual.foregroundColor) { u.set(p + QStringLiteral("child[1].rectangle.7"), visual.foregroundColor); }
    if (oldVisual.backgroundColor != visual.backgroundColor) { u.set(p + QStringLiteral("child[1].rectangle.6"), visual.backgroundColor); }
    if (oldVisual.style != visual.style) { u.set(p + QStringLiteral("child[1].rectangle.8"), visual.style); }
    if (old.languageFrameColor(oldSelected) != m.languageFrameColor(name)) { u.set(p + QStringLiteral("child[0].rectangle.6"), m.languageFrameColor(name)); }
    if (!writeMessageRows(*d, base, old, m, oldNames, names, selected, &rows, error)) { return false; }
    begin = d->values.value(base + QStringLiteral("states.count")).start;
    end = d->values.value(base + QStringLiteral("extendedState[%1].language[%2].9").arg(old.stateCount - 1).arg(oldNames.size() - 1)).end;
    *output = u.render(d->values.value(p + QStringLiteral("type")).start, begin) + rows;
    if (!u.error.isEmpty() || end <= begin) { return fail(error, u.error.isEmpty() ? QStringLiteral("消息记录边界无效。") : u.error); }
    return true;
}
