#include "history_trend_properties.h"
#include <QColor>
#include <QJsonArray>
#include <QSet>
#include <cmath>

/******************************************************************************
 * Function Name   : historyScalar
 * Description     : Bound finite display scalars independently of the collection raw type and future scaling runtime.
 * Input Parameters: text
 * Return Value    : True for a bounded finite editor value.
 * Date            : 2026.11.12
 ******************************************************************************/
static bool historyScalar(const QString &text) //20261112-1 add
{
    bool ok;
    double value = text.toDouble(&ok);
    return !text.isEmpty() && text.size() <= 80 && ok && std::isfinite(value) && std::abs(value) <= 3.402823466e38;
}

/******************************************************************************
 * Function Name   : HistoryTrendProperties::validate
 * Description     : Validate measured time/axis limits and all curve shapes while checking only active bindings and ranges.
 * Input Parameters: error
 * Return Value    : True for the declared history editor slice.
 * Date            : 2026.11.12
 ******************************************************************************/
bool HistoryTrendProperties::validate(QString *error) const //20261112-1 add
{
    TrendAxis axis;
    QStringList colors = {bottomColor, backgroundColor};
    QString color;
    int index;
    if (collectionId.trimmed().isEmpty() || collectionId.size() > 256 || curveCount < 1 || curveCount > 8 || currentCurve < 0 || currentCurve >= curveCount || hours < 0 || hours > 48 || minutes < 0 || minutes > 59 || seconds < 0 || seconds > 59 || hours * 3600 + minutes * 60 + seconds == 0 || timeFormat < 0 || timeFormat > 1 || totalDigits < 1 || totalDigits > 10 || fractionalDigits < 0 || fractionalDigits > 3 || (visibility.visibilityEnabled && visibility.bitEnabled && !visibilityBitChosen)) { if (error != nullptr) { *error = QStringLiteral("历史趋势图采集引用、时间范围、曲线或显现状态无效。"); } return false; }
    for (index = 0; index < 2; ++index)
    {
        axis = index == 0 ? xAxis : yAxis;
        if (axis.major < 3 || axis.major > 10 || axis.minor < 1 || axis.minor > 3 || !QList<int>({12, 16, 24, 48}).contains(axis.fontSize)) { return false; }
        colors.append(axis.axisColor); colors.append(axis.gridColor);
    }
    for (index = 0; index < 8; ++index)
    {
        colors.append(curves[index].color);
        if (curves[index].lineStyle < 0 || curves[index].lineStyle > 4 || curves[index].sourceItemId.size() > 128) { return false; }
        if (index < curveCount && (curves[index].sourceItemId.trimmed().isEmpty() || !historyScalar(curves[index].minimum) || !historyScalar(curves[index].maximum) || curves[index].minimum.toDouble() >= curves[index].maximum.toDouble())) { if (error != nullptr) { *error = QStringLiteral("本版要求活动曲线具有数据来源和有效递增显示范围。"); } return false; }
    }
    Q_FOREACH (color, colors) { if (color.size() > 32 || !QColor(color).isValid()) { return false; } }
    if (!historyScalar(yMinimum) || !historyScalar(yMaximum) || yMinimum.toDouble() >= yMaximum.toDouble()) { if (error != nullptr) { *error = QStringLiteral("本版要求Y轴最小值小于最大值。"); } return false; }
    return visibility.validate(error);
}

/******************************************************************************
 * Function Name   : HistoryTrendProperties::validateCollection
 * Description     : Resolve stable item identities inside the selected collection, independent of renamed display labels.
 * Input Parameters: collection, error
 * Return Value    : True when every active curve remains bound to an existing sampled item.
 * Date            : 2026.11.12
 ******************************************************************************/
bool HistoryTrendProperties::validateCollection(const DataCollectionProperties &collection, QString *error) const //20261112-1 add
{
    QSet<QString> itemIds;
    DataCollectionItem item;
    int index;
    if (curveCount > qMin(8, int(collection.items.size()))) { if (error != nullptr) { *error = QStringLiteral("历史曲线数量超过数据采集项目数。"); } return false; }
    Q_FOREACH (item, collection.items) { itemIds.insert(item.id); }
    for (index = 0; index < curveCount; ++index) { if (!itemIds.contains(curves[index].sourceItemId)) { if (error != nullptr) { *error = QStringLiteral("历史趋势图数据来源已删除或属于其他采集。"); } return false; } }
    return true;
}

/******************************************************************************
 * Function Name   : HistoryTrendProperties::validateAddresses
 * Description     : Validate only display permission carriers; the owning project validates the collection's sampled span.
 * Input Parameters: connections, error
 * Return Value    : True for active visibility carriers.
 * Date            : 2026.11.12
 ******************************************************************************/
bool HistoryTrendProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261112-1 add
{
    ScreenButtonProperties shared;
    shared.common.indicator = false; shared.common.visibility = visibility; return shared.validateAddresses(connections, error);
}

/******************************************************************************
 * Function Name   : HistoryTrendCurve::toJson
 * Description     : Preserve active and dormant curve, axis and permission drafts independently of native serialization.
 * Input Parameters: None
 * Return Value    : Typed editor JSON record.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject HistoryTrendCurve::toJson() const //20261112-1 add
{
    QJsonObject json;
    json.insert(QStringLiteral("sourceItemId"), sourceItemId);
    json.insert(QStringLiteral("minimum"), minimum);
    json.insert(QStringLiteral("maximum"), maximum);
    json.insert(QStringLiteral("lineStyle"), lineStyle);
    json.insert(QStringLiteral("color"), color);
    return json;
}

/******************************************************************************
 * Function Name   : HistoryTrendCurve::fromJson
 * Description     : Reject incomplete, unknown, mistyped and fractional records before replacing caller state.
 * Input Parameters: json, output, error
 * Return Value    : True for a complete supported record.
 * Date            : 2026.11.12
 ******************************************************************************/
bool HistoryTrendCurve::fromJson(const QJsonObject &json, HistoryTrendCurve *output, QString *error) //20261112-1 add
{
    HistoryTrendCurve candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator field;
    double number;
    if (output == nullptr || json.keys() != shape.keys()) { if (error != nullptr) { *error = QStringLiteral("历史趋势图字段缺失或未知。"); } return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != shape.value(field.key()).type()) { return false; }
        if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 1000) { return false; } }
        if (field.value().isString() && field.value().toString().size() > 1024) { return false; }
    }
    candidate.sourceItemId = json.value(QStringLiteral("sourceItemId")).toString();
    candidate.minimum = json.value(QStringLiteral("minimum")).toString();
    candidate.maximum = json.value(QStringLiteral("maximum")).toString();
    candidate.lineStyle = json.value(QStringLiteral("lineStyle")).toInt();
    candidate.color = json.value(QStringLiteral("color")).toString();
    *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : HistoryTrendProperties::toJson
 * Description     : Preserve active and dormant curve, axis and permission drafts independently of native serialization.
 * Input Parameters: None
 * Return Value    : Typed editor JSON record.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject HistoryTrendProperties::toJson() const //20261112-1 add
{
    QJsonObject json;
    QJsonArray rows;
    const HistoryTrendCurve *curve;
    for (curve = curves.data(); curve != curves.data() + curves.size(); ++curve) { rows.append(curve->toJson()); }
    json.insert(QStringLiteral("curves"), rows); json.insert(QStringLiteral("xAxis"), xAxis.toJson()); json.insert(QStringLiteral("yAxis"), yAxis.toJson()); json.insert(QStringLiteral("visibility"), visibility.toJson());
    json.insert(QStringLiteral("collectionId"), collectionId);
    json.insert(QStringLiteral("curveCount"), curveCount);
    json.insert(QStringLiteral("currentCurve"), currentCurve);
    json.insert(QStringLiteral("bottomColor"), bottomColor);
    json.insert(QStringLiteral("backgroundColor"), backgroundColor);
    json.insert(QStringLiteral("hours"), hours);
    json.insert(QStringLiteral("minutes"), minutes);
    json.insert(QStringLiteral("seconds"), seconds);
    json.insert(QStringLiteral("timeFormat"), timeFormat);
    json.insert(QStringLiteral("yMinimum"), yMinimum);
    json.insert(QStringLiteral("yMaximum"), yMaximum);
    json.insert(QStringLiteral("totalDigits"), totalDigits);
    json.insert(QStringLiteral("fractionalDigits"), fractionalDigits);
    json.insert(QStringLiteral("visibilityBitChosen"), visibilityBitChosen);
    return json;
}

/******************************************************************************
 * Function Name   : HistoryTrendProperties::fromJson
 * Description     : Reject incomplete, unknown, mistyped and fractional records before replacing caller state.
 * Input Parameters: json, output, error
 * Return Value    : True for a complete supported record.
 * Date            : 2026.11.12
 ******************************************************************************/
bool HistoryTrendProperties::fromJson(const QJsonObject &json, HistoryTrendProperties *output, QString *error) //20261112-1 add
{
    HistoryTrendProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator field;
    double number;
    QJsonArray rows;
    int index;
    if (output == nullptr || json.keys() != shape.keys()) { if (error != nullptr) { *error = QStringLiteral("历史趋势图字段缺失或未知。"); } return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != shape.value(field.key()).type()) { return false; }
        if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 1000) { return false; } }
        if (field.value().isString() && field.value().toString().size() > 1024) { return false; }
    }
    candidate.collectionId = json.value(QStringLiteral("collectionId")).toString();
    candidate.curveCount = json.value(QStringLiteral("curveCount")).toInt();
    candidate.currentCurve = json.value(QStringLiteral("currentCurve")).toInt();
    candidate.bottomColor = json.value(QStringLiteral("bottomColor")).toString();
    candidate.backgroundColor = json.value(QStringLiteral("backgroundColor")).toString();
    candidate.hours = json.value(QStringLiteral("hours")).toInt();
    candidate.minutes = json.value(QStringLiteral("minutes")).toInt();
    candidate.seconds = json.value(QStringLiteral("seconds")).toInt();
    candidate.timeFormat = json.value(QStringLiteral("timeFormat")).toInt();
    candidate.yMinimum = json.value(QStringLiteral("yMinimum")).toString();
    candidate.yMaximum = json.value(QStringLiteral("yMaximum")).toString();
    candidate.totalDigits = json.value(QStringLiteral("totalDigits")).toInt();
    candidate.fractionalDigits = json.value(QStringLiteral("fractionalDigits")).toInt();
    candidate.visibilityBitChosen = json.value(QStringLiteral("visibilityBitChosen")).toBool();
    rows = json.value(QStringLiteral("curves")).toArray(); if (rows.size() != 8) { return false; }
    for (index = 0; index < 8; ++index) { if (!rows.at(index).isObject() || !HistoryTrendCurve::fromJson(rows.at(index).toObject(), &candidate.curves[index], error)) { return false; } }
    if (!TrendAxis::fromJson(json.value(QStringLiteral("xAxis")).toObject(), &candidate.xAxis, error) || !TrendAxis::fromJson(json.value(QStringLiteral("yAxis")).toObject(), &candidate.yAxis, error) || !StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() || !candidate.validate(error)) { return false; }
    *output = candidate; return true;
}

