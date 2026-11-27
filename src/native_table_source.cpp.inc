//20261127-9 add: V122 type 36 keeps both table attributes and cached rectangle/grid painter items.
/******************************************************************************
 * Function Name   : readTableFields
 * Description     : Traverse every table child, shared ContainerItem field and private V122 attribute.
 * Input Parameters: document, object prefix
 * Return Value    : None. Malformed records latch the parser's first error.
 * Date            : 2026.11.27
 ******************************************************************************/
static void readTableFields(SourceDocument &d, const QString &p)
{
    const int children = d.count(p + QStringLiteral("children"), 59);
    QString item;
    int index;
    int points;
    int j;

    if (children < 1 || d.read('i', p + QStringLiteral("rectangle.type")).toInt() != 0x1000f)
    {
        d.reject(p, QStringLiteral("表格缺少原厂矩形背景子图元。")); return;
    }
    d.fields("ddddcicciddibqii", p + QStringLiteral("rectangle"));
    d.list(p + QStringLiteral("rectangle.labels")); d.read('i', p + QStringLiteral("rectangle.end"));
    for (index = 1; index < children && d.error.isEmpty(); ++index)
    {
        item = SourceDocument::indexed(p + QStringLiteral("grid"), index);
        if (d.read('i', item + QStringLiteral(".type")).toInt() != 0x10011)
        {
            d.reject(item, QStringLiteral("表格含不支持的缓存网格线子图元。")); return;
        }
        points = d.count(item + QStringLiteral(".points"), 16384);
        for (j = 0; j < points; ++j) { d.fields("idd", SourceDocument::indexed(item + QStringLiteral(".point"), j)); }
        if (points != 0) { d.fields("ii", item + QStringLiteral(".pathEnd")); }
        d.fields("iicddbqiiii", item + QStringLiteral(".style"));
        d.list(item + QStringLiteral(".labels")); d.read('i', item + QStringLiteral(".end"));
    }
    if (d.read('i', p + QStringLiteral("group.metadata-type")).toInt() != 105)
    {
        d.reject(p, QStringLiteral("表格容器元数据子类型不匹配。")); return;
    }
    d.read('q', p + QStringLiteral("group.metadata-name")); d.read('i', p + QStringLiteral("group.1c"));
    d.fields("bi", p + QStringLiteral("common.touch")); d.address(p + QStringLiteral("common.40"));
    d.fields("iibi", p + QStringLiteral("common.58")); d.address(p + QStringLiteral("common.e4"));
    d.fields("bi", p + QStringLiteral("common.visibility")); d.address(p + QStringLiteral("common.90"));
    d.fields("iii", p + QStringLiteral("common.a8")); d.address(p + QStringLiteral("common.60"));
    d.fields("aqqi", p + QStringLiteral("common.78")); d.address(p + QStringLiteral("common.b0"));
    d.fields("aqqii", p + QStringLiteral("common.c8")); d.address(p + QStringLiteral("common.fc"));
    d.fields("qbi", p + QStringLiteral("common.118"));
    d.fields("ssbbbcccssb", p + QStringLiteral("table")); d.address(p + QStringLiteral("table.rowAddress"));
    d.read('b', p + QStringLiteral("table.columnControlled")); d.address(p + QStringLiteral("table.columnAddress"));
}

/******************************************************************************
 * Function Name   : sourceTable
 * Description     : Expose the native geometry and static table settings without activating dormant addresses.
 * Input Parameters: document, object prefix, index, output object, error
 * Return Value    : True for a representable native static table.
 * Date            : 2026.11.27
 ******************************************************************************/
static bool sourceTable(const SourceDocument &d, const QString &p, int index, HmiObject *output, QString *error)
{
    HmiObject object;
    GraphicProperties &g = object.graphic;
    const double x = d.value(p + QStringLiteral("rectangle.9")).toDouble();
    const double y = d.value(p + QStringLiteral("rectangle.10")).toDouble();
    const double w = d.value(p + QStringLiteral("rectangle.2")).toDouble();
    const double h = d.value(p + QStringLiteral("rectangle.3")).toDouble();
    int expected;
    int i;
    QString line;
    QString label;
    QColor cachedBackground;
    bool horizontal;
    double offset;

    if (d.value(p + QStringLiteral("table.10")).toBool() || d.value(p + QStringLiteral("table.columnControlled")).toBool() ||
        d.value(p + QStringLiteral("common.touch.0")).toBool() || d.value(p + QStringLiteral("common.visibility.0")).toBool() ||
        d.value(p + QStringLiteral("common.58.2")).toBool())
    {
        return fail(error, QStringLiteral("表格含动态行列地址、通知或活动显现/触控条件，尚未映射到纯图像表格。"));
    }
    if (d.value(p + QStringLiteral("rectangle.0")).toDouble() != 0 || d.value(p + QStringLiteral("rectangle.1")).toDouble() != 0 ||
        std::abs(x) > 32768 || std::abs(y) > 32768 || w < 1 || h < 1 || w > 32768 || h > 32768 ||
        std::trunc(w) != w || std::trunc(h) != h)
    {
        return fail(error, QStringLiteral("表格的原厂局部原点或边长尚不能无损表示。"));
    }
    object.type = QStringLiteral("table");
    object.id = QStringLiteral("native-table-%1").arg(index);
    object.name = d.value(p + QStringLiteral("group.metadata-name")).toString();
    if (object.name.isEmpty()) { object.name = graphicName(object.type) + QString::number(index + 1); }
    object.geometry = QRect(int(std::floor(x)), int(std::floor(y)), int(w), int(h));
    object.subpixelOffset = QPointF(x - std::floor(x), y - std::floor(y));
    g.rows = d.value(p + QStringLiteral("table.0")).toInt(); g.columns = d.value(p + QStringLiteral("table.1")).toInt();
    g.showOutline = d.value(p + QStringLiteral("table.2")).toBool();
    g.showHorizontal = d.value(p + QStringLiteral("table.3")).toBool();
    g.showVertical = d.value(p + QStringLiteral("table.4")).toBool();
    g.background = d.value(p + QStringLiteral("table.5")).value<QColor>();
    g.frameColor = d.value(p + QStringLiteral("table.6")).value<QColor>();
    g.lineColor = d.value(p + QStringLiteral("table.7")).value<QColor>();
    g.lineStyle = d.value(p + QStringLiteral("table.8")).toInt(); g.alpha = d.value(p + QStringLiteral("table.9")).toInt();
    if (!g.validate(object.type, error)) { return false; }
    cachedBackground = g.background;
    cachedBackground.setAlpha(g.alpha);
    if (!d.value(p + QStringLiteral("rectangle.12")).toBool() || d.value(p + QStringLiteral("rectangle.11")).toInt() != int(g.showOutline) ||
        d.value(p + QStringLiteral("rectangle.4")).value<QColor>() != g.frameColor ||
        d.value(p + QStringLiteral("rectangle.6")).value<QColor>() != cachedBackground)
    {
        return fail(error, QStringLiteral("表格缓存矩形的颜色、描边或显现状态与其属性不一致。"));
    }
    expected = 1 + (g.showHorizontal ? g.rows - 1 : 0) + (g.showVertical ? g.columns - 1 : 0);
    if (d.value(p + QStringLiteral("children.count")).toInt() != expected)
    {
        return fail(error, QStringLiteral("表格缓存线数与其行列属性不一致。"));
    }
    for (i = 1; i < expected; ++i)
    {
        line = SourceDocument::indexed(p + QStringLiteral("grid"), i);
        horizontal = g.showHorizontal && i < g.rows;
        label = horizontal ? QStringLiteral("HLine%1").arg(i) : QStringLiteral("VLine%1").arg(i - (g.showHorizontal ? g.rows - 1 : 0));
        offset = horizontal ? (h / g.rows) * i : (w / g.columns) * (i - (g.showHorizontal ? g.rows - 1 : 0));
        if (d.value(line + QStringLiteral(".style.6")).toString() != label ||
            d.value(line + QStringLiteral(".points.count")).toInt() != 2 || d.value(line + QStringLiteral(".point[0].0")).toInt() != 0 ||
            d.value(line + QStringLiteral(".point[1].0")).toInt() != 1 || d.value(line + QStringLiteral(".style.7")).toInt() != g.lineStyle ||
            !d.value(line + QStringLiteral(".style.5")).toBool() || d.value(line + QStringLiteral(".style.2")).value<QColor>() != g.lineColor ||
            std::abs(d.value(line + QStringLiteral(".style.3")).toDouble() - x) > 1e-7 ||
            std::abs(d.value(line + QStringLiteral(".style.4")).toDouble() - y) > 1e-7 ||
            std::abs(d.value(line + QStringLiteral(".point[0].1")).toDouble() - (horizontal ? 0.0 : offset)) > 1e-7 ||
            std::abs(d.value(line + QStringLiteral(".point[0].2")).toDouble() - (horizontal ? offset : 0.0)) > 1e-7 ||
            std::abs(d.value(line + QStringLiteral(".point[1].1")).toDouble() - (horizontal ? w : offset)) > 1e-7 ||
            std::abs(d.value(line + QStringLiteral(".point[1].2")).toDouble() - (horizontal ? offset : h)) > 1e-7)
        {
            return fail(error, QStringLiteral("表格缓存线的名称、位置或线型与其属性不一致。"));
        }
    }
    object.layer = d.value(p + QStringLiteral("z")).toDouble();
    *output = object;
    return true;
}

/******************************************************************************
 * Function Name   : setTableField
 * Description     : Keep untouched native QColor bytes and null strings when their decoded values already match.
 * Input Parameters: update transaction, field path, desired value
 * Return Value    : None. Encoding errors remain in the transaction.
 * Date            : 2026.11.27
 ******************************************************************************/
static void setTableField(SourceUpdates &updates, const QString &key, const QVariant &value)
{
    if (updates.document.value(key) != value) { updates.set(key, value); }
}

/******************************************************************************
 * Function Name   : tableGridLine
 * Description     : Patch one native cached two-point line in local coordinates with a separate scene translation.
 * Input Parameters: source document, child prefix, line name and endpoints, translation, table style
 * Return Value    : Complete native child record, including its type.
 * Date            : 2026.11.27
 ******************************************************************************/
static QByteArray tableGridLine(const SourceDocument &source, const QString &p, const QString &name,
                                const QLineF &line, const QPointF &position, const GraphicProperties &graphic)
{
    SourceUpdates updates(source);
    setTableField(updates, p + QStringLiteral(".point[0].1"), line.x1());
    setTableField(updates, p + QStringLiteral(".point[0].2"), line.y1());
    setTableField(updates, p + QStringLiteral(".point[1].1"), line.x2());
    setTableField(updates, p + QStringLiteral(".point[1].2"), line.y2());
    setTableField(updates, p + QStringLiteral(".style.2"), graphic.lineColor);
    setTableField(updates, p + QStringLiteral(".style.3"), position.x());
    setTableField(updates, p + QStringLiteral(".style.4"), position.y());
    setTableField(updates, p + QStringLiteral(".style.6"), name);
    setTableField(updates, p + QStringLiteral(".style.7"), graphic.lineStyle);
    return updates.render(source.values.value(p + QStringLiteral(".type")).start, source.values.value(p + QStringLiteral(".end")).end);
}

/******************************************************************************
 * Function Name   : writeNativeTable
 * Description     : Encode both static table attributes and their painter cache, retaining untouched shared-control metadata.
 * Input Parameters: object, original object if available, source document, prefix, output bytes, error
 * Return Value    : True after a complete table body has been assembled.
 * Date            : 2026.11.27
 ******************************************************************************/
static bool writeNativeTable(const HmiObject &object, const HmiObject *original, const SourceDocument &document,
                             QString prefix, QByteArray *output, QString *error)
{
    const GraphicProperties &g = object.graphic;
    const QPointF position = QPointF(object.geometry.topLeft()) + object.subpixelOffset;
    const double width = object.geometry.width();
    const double height = object.geometry.height();
    QFile fixture(QStringLiteral(":/native/table-default.sm"));
    SourceDocument seed(QByteArray{});
    const SourceDocument *source = &document;
    QMap<QString, QString> existing;
    QString fallback;
    QString item;
    QString label;
    QColor back = g.background;
    QColor foreground;
    QLineF line;
    QByteArray children;
    QByteArray child;
    double offset;
    int i;
    int j;
    int count;

    if (!g.validate(QStringLiteral("table"), error)) { return false; }
    if (!object.geometry.isValid() || width > 32768 || height > 32768 || std::abs(position.x()) > 32768 || std::abs(position.y()) > 32768 ||
        !object.extensions.isEmpty() || !object.address.isEmpty() || !object.text.isEmpty())
    {
        return fail(error, QStringLiteral("表格含未映射的源属性或几何范围无效。"));
    }
    if (original && original->name == object.name && original->geometry == object.geometry && original->subpixelOffset == object.subpixelOffset &&
        original->graphic.toJson(object.type) == g.toJson(object.type))
    {
        *output = document.bytes.mid(document.values.value(prefix + QStringLiteral("type")).start,
                                     document.values.value(prefix + QStringLiteral("table.columnAddress.8")).end - document.values.value(prefix + QStringLiteral("type")).start);
        return true;
    }
    if (!fixture.open(QIODevice::ReadOnly)) { return fail(error, QStringLiteral("缺少原厂纯图像表格种子。")); }
    seed.bytes = fixture.readAll();
    if (!seed.parse() || seed.objectPrefixes.size() != 1) { return fail(error, QStringLiteral("原厂表格种子的子记录无效。")); }
    if (!original) { source = &seed; prefix = seed.objectPrefixes.first(); }
    SourceUpdates updates(*source);
    SourceUpdates rect(*source);
    fallback = SourceDocument::indexed(seed.objectPrefixes.first() + QStringLiteral("grid"), 1);
    count = source->value(prefix + QStringLiteral("children.count")).toInt();
    for (i = 1; i < count; ++i)
    {
        item = SourceDocument::indexed(prefix + QStringLiteral("grid"), i);
        existing.insert(source->value(item + QStringLiteral(".style.6")).toString(), item);
    }
    if (!original || original->name != object.name) { setTableField(updates, prefix + QStringLiteral("group.metadata-name"), object.name); }
    setTableField(updates, prefix + QStringLiteral("table.0"), g.rows); setTableField(updates, prefix + QStringLiteral("table.1"), g.columns);
    setTableField(updates, prefix + QStringLiteral("table.2"), int(g.showOutline));
    setTableField(updates, prefix + QStringLiteral("table.3"), int(g.showHorizontal));
    setTableField(updates, prefix + QStringLiteral("table.4"), int(g.showVertical));
    setTableField(updates, prefix + QStringLiteral("table.5"), g.background);
    setTableField(updates, prefix + QStringLiteral("table.6"), g.frameColor);
    setTableField(updates, prefix + QStringLiteral("table.7"), g.lineColor);
    setTableField(updates, prefix + QStringLiteral("table.8"), g.lineStyle);
    setTableField(updates, prefix + QStringLiteral("table.9"), g.alpha);
    setTableField(rect, prefix + QStringLiteral("rectangle.2"), width); setTableField(rect, prefix + QStringLiteral("rectangle.3"), height);
    setTableField(rect, prefix + QStringLiteral("rectangle.9"), position.x()); setTableField(rect, prefix + QStringLiteral("rectangle.10"), position.y());
    setTableField(rect, prefix + QStringLiteral("rectangle.11"), int(g.showOutline)); setTableField(rect, prefix + QStringLiteral("rectangle.4"), g.frameColor);
    back.setAlpha(g.alpha);
    foreground = source->value(prefix + QStringLiteral("rectangle.7")).value<QColor>();
    foreground.setAlpha(g.alpha);
    setTableField(rect, prefix + QStringLiteral("rectangle.6"), back); setTableField(rect, prefix + QStringLiteral("rectangle.7"), foreground);
    appendWord(&children, 1 + (g.showHorizontal ? g.rows - 1 : 0) + (g.showVertical ? g.columns - 1 : 0));
    children.append(rect.render(source->values.value(prefix + QStringLiteral("rectangle.type")).start, source->values.value(prefix + QStringLiteral("rectangle.end")).end));
    if (!rect.error.isEmpty()) { return fail(error, rect.error); }
    for (i = 0; i < 2; ++i)
    {
        count = i == 0 ? (g.showHorizontal ? g.rows : 0) : (g.showVertical ? g.columns : 0);
        for (j = 1; j < count; ++j)
        {
            // V5.1 subdivides the scene in doubles, including non-integral cell widths and heights.
            offset = ((i == 0 ? height : width) / count) * j;
            line = i == 0 ? QLineF(0, offset, width, offset) : QLineF(offset, 0, offset, height);
            label = (i == 0 ? QStringLiteral("HLine") : QStringLiteral("VLine")) + QString::number(j);
            item = existing.value(label);
            child = tableGridLine(item.isEmpty() ? seed : *source, item.isEmpty() ? fallback : item, label, line, position, g);
            if (child.isEmpty()) { return fail(error, QStringLiteral("表格缓存网格线编码失败。")); }
            children.append(child);
        }
    }
    updates.replacements.insert(source->values.value(prefix + QStringLiteral("children.count")).start,
                                qMakePair(source->values.value(prefix + QStringLiteral("group.metadata-type")).start, children));
    *output = updates.render(source->values.value(prefix + QStringLiteral("type")).start, source->values.value(prefix + QStringLiteral("table.columnAddress.8")).end);
    if (!updates.error.isEmpty()) { return fail(error, updates.error); }
    return true;
}
