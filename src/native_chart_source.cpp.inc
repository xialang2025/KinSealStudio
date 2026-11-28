//20261128-32 add: V122 chart snapshots and new controls share the same verified object writers.

/******************************************************************************
 * Function Name   : loadNativeChartSeed
 * Description     : Parse one factory chart once. Only its object body is ever copied into an authored project.
 * Input Parameters: resource path
 * Return Value    : Parsed factory source, or null when the resource is invalid.
 * Date            : 2026.11.28
 ******************************************************************************/
static std::unique_ptr<SourceDocument> loadNativeChartSeed(const QString &path)
{
    QFile file(path);
    std::unique_ptr<SourceDocument> seed;

    if (!file.open(QIODevice::ReadOnly)) { return {}; }
    seed = std::make_unique<SourceDocument>(file.readAll());
    if (!seed->parse() || seed->objectPrefixes.size() != 1) { return {}; }
    return seed;
}

/******************************************************************************
 * Function Name   : nativeChartSeed
 * Description     : Keep a separate immutable source graph for each native chart mode.
 * Input Parameters: model shape
 * Return Value    : Source containing one chart, or null.
 * Date            : 2026.11.28
 ******************************************************************************/
static const SourceDocument *nativeChartSeed(int shape)
{
    static const std::unique_ptr<SourceDocument> seeds[] = {
        loadNativeChartSeed(QStringLiteral(":/native/chart-bar.sm")),
        loadNativeChartSeed(QStringLiteral(":/native/chart-meter.sm")),
        loadNativeChartSeed(QStringLiteral(":/native/chart-donut.sm"))
    };
    return shape >= 0 && shape < 3 ? seeds[shape].get() : nullptr;
}

/******************************************************************************
 * Function Name   : nativeChartBaseline
 * Description     : Bind the nominal factory placement and private defaults to the chosen seed's own name.
 * Input Parameters: shape, parsed seed
 * Return Value    : Model of the unmodified chart.
 * Date            : 2026.11.28
 ******************************************************************************/
static HmiObject nativeChartBaseline(int shape, const SourceDocument &seed)
{
    HmiObject obj;
    obj.type = QStringLiteral("chart");
    obj.chart.shape = shape;
    obj.name = seed.value(seed.objectPrefixes.first() + QStringLiteral("group.metadata-name")).toString();
    obj.geometry = QRect(160, 120, 200, shape == 0 ? 160 : 200);
    return obj;
}

/******************************************************************************
 * Function Name   : patchNativeChart
 * Description     : Update the private monitor and preview fields; retain every other byte in the donor object.
 * Input Parameters: old model, edited model, typed source edits, record prefix, diagnostic
 * Return Value    : True after validating all requested changes.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool patchNativeChart(const HmiObject &old, const HmiObject &obj, SourceUpdates &edits,
                             const QString &p, QString *error)
{
    const ChartProperties &chart = obj.chart;
    ChartProperties writable = old.chart;
    const SourceDocument &d = edits.document;
    const int shape = chart.shape;
    const QString body = p + QStringLiteral("private.");
    const QString root = p + QStringLiteral("child[");
    const QString rect = root + QStringLiteral("0].rectangle.");
    const QString fill = root + QStringLiteral("1].rectangle.");
    const QString ellipse = root + QStringLiteral("0].ellipse.");
    const QString address = chart.address();
    const double dx = obj.geometry.x() + obj.subpixelOffset.x() - old.geometry.x() - old.subpixelOffset.x();
    const double dy = obj.geometry.y() + obj.subpixelOffset.y() - old.geometry.y() - old.subpixelOffset.y();
    const bool resized = old.geometry.size() != obj.geometry.size();
    const int w = obj.geometry.width(), h = obj.geometry.height();
    const QColor red = QColor(chart.bar.foreground), background = QColor(chart.bar.background), line = QColor(chart.donut.lineColor);
    const auto local = QRegularExpression(QStringLiteral("^LW(0|[1-9][0-9]{0,3})$")).match(address);
    QString key, structured, mirror;
    double delta;

    if (shape != old.chart.shape || !local.hasMatch() || local.captured(1).toInt() > 4095 ||
        !obj.address.isEmpty() || !obj.text.isEmpty() || obj.hasAppearanceFlip() || !obj.extensions.isEmpty() ||
        !chart.validate(error) || w < 1 || h < 1 || w > 4096 || h > 4096 ||
        !std::isfinite(dx) || !std::isfinite(dy) || (resized && (shape == 1 || (shape == 2 && w != h)))) {
        return fail(error, QStringLiteral("图表的地址、形态、尺寸或外观超出原厂源文件的已核实范围。"));
    }
    writable.setAddress(address);
    if (shape == 0) {
        writable.bar.foreground = red.name(); writable.bar.background = background.name();
    } else if (shape == 2) {
        writable.donut.lineColor = line.name();
        writable.donut.minimum = QString::number(chart.donut.minimum.toUInt());
        writable.donut.maximum = QString::number(chart.donut.maximum.toUInt());
    }
    if (writable.toJson() != chart.toJson()) {
        return fail(error, QStringLiteral("此次图表属性修改需要其他原厂私有字段，请保存在 KSP 中。"));
    }
    if (address != old.chart.address()) {
        mirror = body + (shape == 2 ? QStringLiteral("1") : QStringLiteral("2"));
        structured = body + (shape == 0 ? QStringLiteral("3") : shape == 1 ? QStringLiteral("34") : QStringLiteral("10"));
        edits.set(mirror, address); edits.address(structured, address);
    }
    if (old.name != obj.name) { edits.set(p + QStringLiteral("group.metadata-name"), obj.name); }
    if (shape == 0) {
        patchDisplayColor(edits, body + QStringLiteral("9"), fill + QStringLiteral("6"), old.chart.bar.foreground, chart.bar.foreground);
        patchDisplayColor(edits, body + QStringLiteral("10"), rect + QStringLiteral("6"), old.chart.bar.background, chart.bar.background);
        if (resized) {
            // Rectangle records contain the two corners. The default editor shows the upper-moving column at half height.
            edits.set(rect + QStringLiteral("2"), w); edits.set(rect + QStringLiteral("3"), h);
            edits.set(fill + QStringLiteral("1"), h / 2.0); edits.set(fill + QStringLiteral("2"), w); edits.set(fill + QStringLiteral("3"), h);
        }
    } else if (shape == 2) {
        if (old.chart.donut.lineColor != chart.donut.lineColor) {
            edits.set(body + QStringLiteral("9"), line); edits.set(ellipse + QStringLiteral("0"), line);
        }
        if (old.chart.donut.minimum != chart.donut.minimum) { edits.set(body + QStringLiteral("12"), chart.donut.minimum.toDouble()); }
        if (old.chart.donut.maximum != chart.donut.maximum) { edits.set(body + QStringLiteral("13"), chart.donut.maximum.toDouble()); }
        if (resized) { edits.set(ellipse + QStringLiteral("4"), w); edits.set(ellipse + QStringLiteral("5"), h); }
    }
    // Nested meter labels, strokes and ellipses each save their own scene offset; translating only the outer face splits them.
    if (dx || dy) {
        for (auto it = d.values.lowerBound(root); it != d.values.end() && it.key().startsWith(root); ++it) {
            key = it.key();
            if (key.endsWith(QStringLiteral(".rectangle.9")) || key.endsWith(QStringLiteral(".ellipse.pos.0")) ||
                key.endsWith(QStringLiteral(".text.tail.10")) || key.endsWith(QStringLiteral(".stroke.3"))) { delta = dx; }
            else if (key.endsWith(QStringLiteral(".rectangle.10")) || key.endsWith(QStringLiteral(".ellipse.pos.1")) ||
                     key.endsWith(QStringLiteral(".text.tail.11")) || key.endsWith(QStringLiteral(".stroke.4"))) { delta = dy; }
            else { continue; }
            if (delta) { edits.set(key, it.value().value.toDouble() + delta); }
        }
    }
    return edits.error.isEmpty() || fail(error, edits.error);
}

/******************************************************************************
 * Function Name   : sourceNativeChart
 * Description     : Restore a typed chart only if recovered properties also account for its private data and cached preview graph.
 * Input Parameters: parsed project, chart prefix and index, output model, diagnostic
 * Return Value    : True when the original preview and private record match their reconstructed byte sequences.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool sourceNativeChart(const SourceDocument &d, const QString &p, int index, HmiObject *out, QString *error)
{
    const QString body = p + QStringLiteral("private.");
    const int mode = d.value(body + QStringLiteral("mode")).toInt();
    const int shape = mode == 0 ? 0 : mode == 2 ? 1 : mode == 3 ? 2 : -1;
    const SourceDocument *seed = nativeChartSeed(shape);
    const QString relative = shape == 0 ? QStringLiteral("child[0].rectangle.") : QStringLiteral("child[0].ellipse.");
    const QString px = relative + (shape == 0 ? QStringLiteral("9") : QStringLiteral("pos.0"));
    const QString py = relative + (shape == 0 ? QStringLiteral("10") : QStringLiteral("pos.1"));
    const QString first = QStringLiteral("children.count"), previewEnd = QStringLiteral("group.1c"), privateStart = QStringLiteral("private.mode");
    HmiObject base, obj;
    QString donor, address, last;
    double x, y, w, h;

    if (!seed || d.value(QStringLiteral("header.1")).toInt() != 122 ||
        d.value(p + QStringLiteral("common.visibility.0")).toInt() || d.value(p + QStringLiteral("common.touch.0")).toInt() ||
        d.value(p + QStringLiteral("common.58.2")).toInt()) {
        return fail(error, QStringLiteral("当前图表的版本、权限、通知或类型没有原厂样本对照。"));
    }
    donor = seed->objectPrefixes.first();
    base = nativeChartBaseline(shape, *seed);
    obj = base;
    obj.id = QStringLiteral("native-chart-%1").arg(index);
    obj.name = d.value(p + QStringLiteral("group.metadata-name")).toString();
    address = d.value(body + (shape == 2 ? QStringLiteral("1") : QStringLiteral("2"))).toString();
    obj.chart.setAddress(address);
    x = base.geometry.x() + (d.value(p + px).toDouble() - seed->value(donor + px).toDouble());
    y = base.geometry.y() + (d.value(p + py).toDouble() - seed->value(donor + py).toDouble());
    w = base.geometry.width(); h = base.geometry.height();
    if (shape == 0) {
        obj.chart.bar.foreground = dataDisplayColor(d.value(body + QStringLiteral("9")).toString()).name();
        obj.chart.bar.background = dataDisplayColor(d.value(body + QStringLiteral("10")).toString()).name();
        w = d.value(p + relative + QStringLiteral("2")).toDouble(); h = d.value(p + relative + QStringLiteral("3")).toDouble();
    } else if (shape == 2) {
        obj.chart.donut.lineColor = d.value(body + QStringLiteral("9")).value<QColor>().name();
        obj.chart.donut.minimum = QString::number(d.value(body + QStringLiteral("12")).toDouble(), 'g', 16);
        obj.chart.donut.maximum = QString::number(d.value(body + QStringLiteral("13")).toDouble(), 'g', 16);
        w = d.value(p + relative + QStringLiteral("4")).toDouble(); h = d.value(p + relative + QStringLiteral("5")).toDouble();
    }
    if (!std::isfinite(x) || !std::isfinite(y) || std::abs(x) > 65535 || std::abs(y) > 65535 ||
        !(w >= 1 && w <= 4096 && h >= 1 && h <= 4096) || std::trunc(w) != w || std::trunc(h) != h) {
        return fail(error, QStringLiteral("图表原厂图元坐标超出画布范围。"));
    }
    obj.geometry = QRect(int(std::floor(x)), int(std::floor(y)), int(w), int(h));
    obj.subpixelOffset = QPointF(x - std::floor(x), y - std::floor(y));
    SourceUpdates expected(*seed);
    if (!patchNativeChart(base, obj, expected, donor, error)) { return false; }
    last = QStringLiteral("private.") + (shape == 0 ? QStringLiteral("66") : shape == 1 ? QStringLiteral("38") : QStringLiteral("14"));
    const auto matches = [&](const QString &a, const QString &b) {
        const qsizetype begin = d.values.value(p + a).start, end = d.values.value(p + b).end;
        return expected.render(seed->values.value(donor + a).start, seed->values.value(donor + b).end) == d.bytes.mid(begin, end-begin);
    };
    if (!matches(first, previewEnd) || !matches(privateStart, last)) {
        return fail(error, QStringLiteral("图表预览、监控地址副本或私有属性与已核实的原厂结构不同。"));
    }
    if (obj.name.isEmpty()) { obj.name = QStringLiteral("图表%1").arg(index + 1); }
    *out = obj;
    return true;
}

/******************************************************************************
 * Function Name   : writeNativeChart
 * Description     : Save the original object's bytes or construct one from the correct factory form, excluding the donor project shell.
 * Input Parameters: edited object, optional prior object, native document, original object prefix, output bytes, diagnostic
 * Return Value    : True after serializing exactly one type-11 body.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool writeNativeChart(const HmiObject &obj, const HmiObject *old, const SourceDocument &d,
                             const QString &prefix, QByteArray *out, QString *error)
{
    const SourceDocument *source = old ? &d : nativeChartSeed(obj.chart.shape);
    HmiObject baseline;
    QString p = prefix;

    if (!source) { return fail(error, QStringLiteral("图表原厂模板不可用。")); }
    if (!old) { p = source->objectPrefixes.first(); baseline = nativeChartBaseline(obj.chart.shape, *source); old = &baseline; }
    SourceUpdates changes(*source);
    if (!patchNativeChart(*old, obj, changes, p, error)) { return false; }
    // The host project owns the root flags, Z order, configuration, resources and footer.
    const QString last = QStringLiteral("private.") + (obj.chart.shape == 0 ? QStringLiteral("66") : obj.chart.shape == 1 ? QStringLiteral("38") : QStringLiteral("14"));
    *out = changes.render(source->values.value(p + QStringLiteral("type")).start, source->values.value(p + last).end);
    return !out->isEmpty();
}
