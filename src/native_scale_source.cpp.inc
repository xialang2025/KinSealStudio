/******************************************************************************
 * Function Name   : nativeScaleLabel
 * Description     : Format the integer-range value assigned to one major graduation in display order.
 * Input Parameters: scale properties, major graduation index
 * Return Value    : Native numeric caption.
 * Date            : 2026.11.28
 ******************************************************************************/
static QString nativeScaleLabel(const ScaleProperties &scale, int index) //20261128-28 add
{
    const double fraction = double(index) / (scale.majorCount - 1);
    const double distance = double(scale.maximum) - scale.minimum;
    const double value = scale.direction == 0 ? scale.maximum - distance * fraction : scale.minimum + distance * fraction;
    return QString::number(value, 'f', scale.decimals);
}

/******************************************************************************
 * Function Name   : sourceNativeScale
 * Description     : Read the label and stroke graph of a saved right-side or top-side V122 scale.
 * Input Parameters: native document, object prefix and ordinal, model output, diagnostic
 * Return Value    : True when cached captions, item count, geometry and the private scale settings agree.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool sourceNativeScale(const SourceDocument &d, const QString &p, int ordinal, HmiObject *out, QString *error) //20261128-28 add
{
    HmiObject obj;
    ScaleProperties &s = obj.scale;
    const QString body = p + QStringLiteral("private.");
    const int count = d.value(p + QStringLiteral("children.count")).toInt();
    const int edge = d.value(body + QLatin1Char('3')).toInt();
    QString child, direction, name;
    double x, y, width, height, left = 1e9, right = -1e9, top = 1e9, bottom = -1e9;
    int i, k, label = 0;
    bool numberOk, maximumOk, sizeOk;
    const auto includePoint = [&](double px, double py) {
        if (!std::isfinite(px) || !std::isfinite(py) || std::abs(px) > 65535 || std::abs(py) > 65535) { return false; }
        left = qMin(left, px); right = qMax(right, px); top = qMin(top, py); bottom = qMax(bottom, py);
        return true;
    };
    s.position = edge == 2 ? 3 : edge == 3 ? 0 : -1;
    s.direction = d.value(body + QStringLiteral("12")).toInt();
    direction = (edge == 2 ? (s.direction ? QStringLiteral("向下") : QStringLiteral("向上")) :
                              (s.direction ? QStringLiteral("向右") : QStringLiteral("向左")));
    s.minimum = d.value(body + QStringLiteral("10")).toString().toInt(&numberOk);
    s.maximum = d.value(body + QLatin1Char('9')).toString().toInt(&maximumOk);
    s.fontSize = d.value(body + QStringLiteral("17")).toString().toInt(&sizeOk);
    s.majorCount = d.value(body + QLatin1Char('8')).toInt();
    s.minorCount = d.value(body + QStringLiteral("11")).toInt();
    s.decimals = d.value(body + QLatin1Char('6')).toInt();
    s.showValues = d.value(body + QLatin1Char('0')).toBool();
    s.lineColor = dataDisplayColor(d.value(body + QLatin1Char('7')).toString());
    s.textColor = dataDisplayColor(d.value(body + QStringLiteral("15")).toString());
    // The two private color strings share the same verified value in every saved witness.
    if (!numberOk || !maximumOk || !sizeOk || !s.showValues || s.decimals != 0 || s.minimum < 0 ||
        s.lineColor != s.textColor || !s.validate(error) || direction != d.value(body + QLatin1Char('1')).toString() ||
        d.value(p + QStringLiteral("common.visibility.0")).toInt() != 0 ||
        d.value(p + QStringLiteral("common.touch.0")).toInt() != 0 ||
        d.value(p + QStringLiteral("common.58.2")).toInt() != 0 ||
        count != 1 + 2 * s.majorCount + (s.majorCount - 1) * s.minorCount) {
        return fail(error, QStringLiteral("刻度的方向、颜色、主副线或标签组合超出原厂源对照。"));
    }
    for (i = 0; i < count; ++i) {
        child = SourceDocument::indexed(p + QStringLiteral("child"), i) + QLatin1Char('.');
        if (i > 0 && i <= 2 * s.majorCount && i % 2 == 0) {
            name = QStringLiteral("Txt") + QString::number(label);
            if (d.value(child + QStringLiteral("type")).toInt() != 0x10016 ||
                d.value(child + QStringLiteral("text.tail.8")).toString() != name ||
                d.value(child + QStringLiteral("text.head.4")).toString() != nativeScaleLabel(s, label) ||
                d.value(child + QStringLiteral("font.family")).toString() != QStringLiteral("宋体") ||
                d.value(child + QStringLiteral("font.tail.1")).toInt() != s.fontSize ||
                d.value(child + QStringLiteral("text.tail.0")).value<QColor>() != s.textColor) {
                return fail(error, QStringLiteral("刻度的缓存标签与原始属性不一致。"));
            }
            x = d.value(child + QStringLiteral("text.head.0")).toDouble() + d.value(child + QStringLiteral("text.tail.10")).toDouble();
            y = d.value(child + QStringLiteral("text.head.1")).toDouble() + d.value(child + QStringLiteral("text.tail.11")).toDouble();
            width = d.value(child + QStringLiteral("text.head.2")).toDouble();
            height = d.value(child + QStringLiteral("text.head.3")).toDouble();
            if (!(width > 0 && height > 0) || !includePoint(x, y) || !includePoint(x + width, y + height)) {
                return fail(error, QStringLiteral("刻度标签的场景几何超出范围。"));
            }
            ++label;
        } else {
            name = d.value(child + QStringLiteral("stroke.6")).toString();
            if (d.value(child + QStringLiteral("type")).toInt() != 0x10011 ||
                d.value(child + QStringLiteral("path.count")).toInt() != 2 ||
                d.value(child + QStringLiteral("element[0].0")).toInt() != 0 ||
                d.value(child + QStringLiteral("element[1].0")).toInt() != 1 ||
                d.value(child + QStringLiteral("stroke.2")).value<QColor>() != s.lineColor ||
                (i == 0 ? name != QStringLiteral("MainLine") : i <= 2*s.majorCount ?
                 name != QStringLiteral("SubLine") + QString::number((i-1)/2) : !name.startsWith(QStringLiteral("SLine")))) {
                return fail(error, QStringLiteral("刻度的缓存线段与已核对的原厂结构不同。"));
            }
            for (k = 0; k < 2; ++k) {
                const QString element = SourceDocument::indexed(child + QStringLiteral("element"), k);
                x = d.value(child + QStringLiteral("stroke.3")).toDouble() + d.value(element + QStringLiteral(".1")).toDouble();
                y = d.value(child + QStringLiteral("stroke.4")).toDouble() + d.value(element + QStringLiteral(".2")).toDouble();
                if (!includePoint(x, y)) { return fail(error, QStringLiteral("刻度线端点超出场景范围。")); }
            }
        }
    }
    width = right-left; height = bottom-top;
    if (width < 1 || height < 1 || width > 4096 || height > 4096 ||
        std::abs(width-qRound(width)) > 0.00001 || std::abs(height-qRound(height)) > 0.00001) {
        return fail(error, QStringLiteral("刻度子图元的包围矩形未还原为整数尺寸。"));
    }
    // Snap exact editor coordinates hidden by floating point rounding in scaled preview rectangles.
    if (std::abs(left-qRound(left)) < 0.00001) { left = qRound(left); }
    if (std::abs(top-qRound(top)) < 0.00001) { top = qRound(top); }
    obj.geometry = QRect(int(std::floor(left)), int(std::floor(top)), qRound(width), qRound(height));
    obj.subpixelOffset = QPointF(left - obj.geometry.x(), top - obj.geometry.y());
    obj.type = QStringLiteral("scale");
    obj.id = QStringLiteral("native-scale-%1").arg(ordinal);
    obj.name = d.value(p + QStringLiteral("group.metadata-name")).toString();
    if (obj.name.isEmpty()) { obj.name = QStringLiteral("刻度%1").arg(ordinal + 1); }
    *out = obj;
    return true;
}

/******************************************************************************
 * Function Name   : writeNativeScale
 * Description     : Preserve the raw preview graph and apply only supported translations and range-label edits.
 * Input Parameters: current and previous objects, native document, prefix, serialized control output, diagnostic
 * Return Value    : True after every edited property has a verified carrier in the original record.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool writeNativeScale(const HmiObject &obj, const HmiObject &prior, const SourceDocument &d, const QString &p,
                             QByteArray *output, QString *error) //20261128-28 add
{
    const auto &s = obj.scale;
    const auto &old = prior.scale;
    const double dx = obj.geometry.x() + obj.subpixelOffset.x() - prior.geometry.x() - prior.subpixelOffset.x();
    const double dy = obj.geometry.y() + obj.subpixelOffset.y() - prior.geometry.y() - prior.subpixelOffset.y();
    const QString body = p + QStringLiteral("private.");
    const int count = d.value(p + QStringLiteral("children.count")).toInt();
    SourceUpdates changes(d);
    QString child, tag, oldLabel, newLabel, location;
    QMap<int, double> widths;
    int i, previousMaximum = 0, newMaximum = 0;
    double value;
    if (!s.validate(error) || !obj.address.isEmpty() || !obj.text.isEmpty() || obj.hasAppearanceFlip() ||
        !obj.extensions.isEmpty() || obj.geometry.size() != prior.geometry.size() ||
        s.fontSize != old.fontSize || s.position != old.position || s.showValues != old.showValues ||
        s.decimals != old.decimals || s.majorCount != old.majorCount || s.minorCount != old.minorCount ||
        s.lineColor != old.lineColor || s.textColor != old.textColor || s.minimum < 0) {
        return fail(error, QStringLiteral("这项刻度修改需要重新生成原厂子图元或字形，请保存在 KSP 中。"));
    }
    if (obj.name != prior.name) { changes.set(p + QStringLiteral("group.metadata-name"), obj.name); }
    for (i = 0; i < old.majorCount; ++i) {
        child = SourceDocument::indexed(p + QStringLiteral("child"), 2*i+2) + QLatin1Char('.');
        oldLabel = nativeScaleLabel(old, i);
        newLabel = nativeScaleLabel(s, i);
        value = d.value(child + QStringLiteral("text.head.2")).toDouble();
        if (widths.contains(oldLabel.size()) && std::abs(widths.value(oldLabel.size())-value) > 0.00001) {
            return fail(error, QStringLiteral("同位数刻度标签具有不同缓存宽度。"));
        }
        widths.insert(oldLabel.size(), value);
        previousMaximum = qMax(previousMaximum, int(oldLabel.size()));
        newMaximum = qMax(newMaximum, int(newLabel.size()));
    }
    if (previousMaximum != newMaximum) {
        return fail(error, QStringLiteral("最大刻度文字宽度改变，需要重算原厂线段布局。"));
    }
    for (i = 0; i < count; ++i) {
        child = SourceDocument::indexed(p + QStringLiteral("child"), i) + QLatin1Char('.');
        if (d.value(child + QStringLiteral("type")).toInt() == 0x10016) {
            oldLabel = nativeScaleLabel(old, i/2-1);
            newLabel = nativeScaleLabel(s, i/2-1);
            if (oldLabel != newLabel) {
                if (!widths.contains(newLabel.size()) || (s.position == 0 && oldLabel.size() != newLabel.size())) {
                    return fail(error, QStringLiteral("新刻度文字尺寸缺少当前图元的原厂宽度对照。"));
                }
                changes.set(child + QStringLiteral("text.head.4"), newLabel);
                if (oldLabel.size() != newLabel.size()) { changes.set(child + QStringLiteral("text.head.2"), widths.value(newLabel.size())); }
            }
            location = child + QStringLiteral("text.tail.");
            tag = location + QStringLiteral("10");
            location += QStringLiteral("11");
        } else {
            location = child + QStringLiteral("stroke.");
            tag = location + QLatin1Char('3');
            location += QLatin1Char('4');
        }
        if (dx) { changes.set(tag, d.value(tag).toDouble() + dx); }
        if (dy) { changes.set(location, d.value(location).toDouble() + dy); }
    }
    if (old.minimum != s.minimum) { changes.set(body + QStringLiteral("10"), QString::number(s.minimum)); }
    if (old.maximum != s.maximum) { changes.set(body + QLatin1Char('9'), QString::number(s.maximum)); }
    if (old.direction != s.direction) {
        changes.set(body + QStringLiteral("12"), s.direction);
        changes.set(body + QLatin1Char('1'), s.position == 3 ? (s.direction ? QStringLiteral("向下") : QStringLiteral("向上")) :
                                                               (s.direction ? QStringLiteral("向右") : QStringLiteral("向左")));
    }
    if (!changes.error.isEmpty()) { return fail(error, changes.error); }
    *output = changes.render(d.values.value(p + QStringLiteral("type")).start, d.values.value(body + QStringLiteral("17")).end);
    return !output->isEmpty();
}
