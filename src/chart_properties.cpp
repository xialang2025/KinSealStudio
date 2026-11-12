#include "chart_properties.h"
#include "plc_native_catalog.h"
#include <QColor>
#include <QRegularExpression>
#include <cmath>

/******************************************************************************
 * Function Name   : ChartBarProperties::toJson
 * Description     : Preserve every active and inactive field in this independent form draft.
 * Input Parameters: None
 * Return Value    : Complete editor JSON record.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject ChartBarProperties::toJson() const //20261112-1 add
{
    QJsonObject json;
    json.insert(QStringLiteral("dataType"), dataType);
    json.insert(QStringLiteral("address"), address);
    json.insert(QStringLiteral("minimum"), minimum);
    json.insert(QStringLiteral("maximum"), maximum);
    json.insert(QStringLiteral("twoPoles"), twoPoles);
    json.insert(QStringLiteral("midpoint"), midpoint);
    json.insert(QStringLiteral("dynamicRange"), dynamicRange);
    json.insert(QStringLiteral("sourceMinimum"), sourceMinimum);
    json.insert(QStringLiteral("sourceMaximum"), sourceMaximum);
    json.insert(QStringLiteral("minimumAddress"), minimumAddress);
    json.insert(QStringLiteral("maximumAddress"), maximumAddress);
    json.insert(QStringLiteral("foreground"), foreground);
    json.insert(QStringLiteral("background"), background);
    json.insert(QStringLiteral("patternColor"), patternColor);
    json.insert(QStringLiteral("pattern"), pattern);
    json.insert(QStringLiteral("direction"), direction);
    return json;
}

/******************************************************************************
 * Function Name   : ChartBarProperties::fromJson
 * Description     : Reject missing, unknown, mistyped and fractional enumeration fields atomically.
 * Input Parameters: json, output
 * Return Value    : True when the record has an exact supported shape.
 * Date            : 2026.11.12
 ******************************************************************************/
bool ChartBarProperties::fromJson(const QJsonObject &json, ChartBarProperties *output) //20261112-1 add
{
    ChartBarProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator field;
    double number;
    if (output == nullptr || json.keys() != shape.keys()) { return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != shape.value(field.key()).type()) { return false; }
        if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 65535) { return false; } }
        if (field.value().isString() && field.value().toString().size() > 1024) { return false; }
    }
    candidate.dataType = json.value(QStringLiteral("dataType")).toInt();
    candidate.address = json.value(QStringLiteral("address")).toString();
    candidate.minimum = json.value(QStringLiteral("minimum")).toString();
    candidate.maximum = json.value(QStringLiteral("maximum")).toString();
    candidate.twoPoles = json.value(QStringLiteral("twoPoles")).toBool();
    candidate.midpoint = json.value(QStringLiteral("midpoint")).toString();
    candidate.dynamicRange = json.value(QStringLiteral("dynamicRange")).toBool();
    candidate.sourceMinimum = json.value(QStringLiteral("sourceMinimum")).toString();
    candidate.sourceMaximum = json.value(QStringLiteral("sourceMaximum")).toString();
    candidate.minimumAddress = json.value(QStringLiteral("minimumAddress")).toString();
    candidate.maximumAddress = json.value(QStringLiteral("maximumAddress")).toString();
    candidate.foreground = json.value(QStringLiteral("foreground")).toString();
    candidate.background = json.value(QStringLiteral("background")).toString();
    candidate.patternColor = json.value(QStringLiteral("patternColor")).toString();
    candidate.pattern = json.value(QStringLiteral("pattern")).toInt();
    candidate.direction = json.value(QStringLiteral("direction")).toInt();
    *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : ChartMeterProperties::toJson
 * Description     : Preserve every active and inactive field in this independent form draft.
 * Input Parameters: None
 * Return Value    : Complete editor JSON record.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject ChartMeterProperties::toJson() const //20261112-1 add
{
    QJsonObject json;
    json.insert(QStringLiteral("dataType"), dataType);
    json.insert(QStringLiteral("address"), address);
    json.insert(QStringLiteral("minimum"), minimum);
    json.insert(QStringLiteral("maximum"), maximum);
    json.insert(QStringLiteral("direction"), direction);
    json.insert(QStringLiteral("transparent"), transparent);
    json.insert(QStringLiteral("foreground"), foreground);
    json.insert(QStringLiteral("background"), background);
    json.insert(QStringLiteral("pointer"), pointer);
    json.insert(QStringLiteral("shape"), shape);
    json.insert(QStringLiteral("mainDivisions"), mainDivisions);
    json.insert(QStringLiteral("showValues"), showValues);
    json.insert(QStringLiteral("showAxis"), showAxis);
    json.insert(QStringLiteral("fontSize"), fontSize);
    json.insert(QStringLiteral("scaleColor"), scaleColor);
    json.insert(QStringLiteral("showSubScale"), showSubScale);
    json.insert(QStringLiteral("subDivisions"), subDivisions);
    return json;
}

/******************************************************************************
 * Function Name   : ChartMeterProperties::fromJson
 * Description     : Reject missing, unknown, mistyped and fractional enumeration fields atomically.
 * Input Parameters: json, output
 * Return Value    : True when the record has an exact supported shape.
 * Date            : 2026.11.12
 ******************************************************************************/
bool ChartMeterProperties::fromJson(const QJsonObject &json, ChartMeterProperties *output) //20261112-1 add
{
    ChartMeterProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator field;
    double number;
    if (output == nullptr || json.keys() != shape.keys()) { return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != shape.value(field.key()).type()) { return false; }
        if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 65535) { return false; } }
        if (field.value().isString() && field.value().toString().size() > 1024) { return false; }
    }
    candidate.dataType = json.value(QStringLiteral("dataType")).toInt();
    candidate.address = json.value(QStringLiteral("address")).toString();
    candidate.minimum = json.value(QStringLiteral("minimum")).toString();
    candidate.maximum = json.value(QStringLiteral("maximum")).toString();
    candidate.direction = json.value(QStringLiteral("direction")).toInt();
    candidate.transparent = json.value(QStringLiteral("transparent")).toBool();
    candidate.foreground = json.value(QStringLiteral("foreground")).toString();
    candidate.background = json.value(QStringLiteral("background")).toString();
    candidate.pointer = json.value(QStringLiteral("pointer")).toInt();
    candidate.shape = json.value(QStringLiteral("shape")).toInt();
    candidate.mainDivisions = json.value(QStringLiteral("mainDivisions")).toInt();
    candidate.showValues = json.value(QStringLiteral("showValues")).toBool();
    candidate.showAxis = json.value(QStringLiteral("showAxis")).toBool();
    candidate.fontSize = json.value(QStringLiteral("fontSize")).toInt();
    candidate.scaleColor = json.value(QStringLiteral("scaleColor")).toString();
    candidate.showSubScale = json.value(QStringLiteral("showSubScale")).toBool();
    candidate.subDivisions = json.value(QStringLiteral("subDivisions")).toInt();
    *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : ChartDonutProperties::toJson
 * Description     : Preserve every active and inactive field in this independent form draft.
 * Input Parameters: None
 * Return Value    : Complete editor JSON record.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject ChartDonutProperties::toJson() const //20261112-1 add
{
    QJsonObject json;
    json.insert(QStringLiteral("dataType"), dataType);
    json.insert(QStringLiteral("address"), address);
    json.insert(QStringLiteral("minimum"), minimum);
    json.insert(QStringLiteral("maximum"), maximum);
    json.insert(QStringLiteral("arc"), arc);
    json.insert(QStringLiteral("lineType"), lineType);
    json.insert(QStringLiteral("lineWidth"), lineWidth);
    json.insert(QStringLiteral("radius"), radius);
    json.insert(QStringLiteral("startAngle"), startAngle);
    json.insert(QStringLiteral("spanAngle"), spanAngle);
    json.insert(QStringLiteral("opacity"), opacity);
    json.insert(QStringLiteral("lineColor"), lineColor);
    json.insert(QStringLiteral("fillWidth"), fillWidth);
    return json;
}

/******************************************************************************
 * Function Name   : ChartDonutProperties::fromJson
 * Description     : Reject missing, unknown, mistyped and fractional enumeration fields atomically.
 * Input Parameters: json, output
 * Return Value    : True when the record has an exact supported shape.
 * Date            : 2026.11.12
 ******************************************************************************/
bool ChartDonutProperties::fromJson(const QJsonObject &json, ChartDonutProperties *output) //20261112-1 add
{
    ChartDonutProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator field;
    double number;
    if (output == nullptr || json.keys() != shape.keys()) { return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != shape.value(field.key()).type()) { return false; }
        if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 65535) { return false; } }
        if (field.value().isString() && field.value().toString().size() > 1024) { return false; }
    }
    candidate.dataType = json.value(QStringLiteral("dataType")).toInt();
    candidate.address = json.value(QStringLiteral("address")).toString();
    candidate.minimum = json.value(QStringLiteral("minimum")).toString();
    candidate.maximum = json.value(QStringLiteral("maximum")).toString();
    candidate.arc = json.value(QStringLiteral("arc")).toBool();
    candidate.lineType = json.value(QStringLiteral("lineType")).toInt();
    candidate.lineWidth = json.value(QStringLiteral("lineWidth")).toInt();
    candidate.radius = json.value(QStringLiteral("radius")).toString();
    candidate.startAngle = json.value(QStringLiteral("startAngle")).toString();
    candidate.spanAngle = json.value(QStringLiteral("spanAngle")).toString();
    candidate.opacity = json.value(QStringLiteral("opacity")).toInt();
    candidate.lineColor = json.value(QStringLiteral("lineColor")).toString();
    candidate.fillWidth = json.value(QStringLiteral("fillWidth")).toInt();
    *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : ChartProperties::dataTypes
 * Description     : Return the native enumerated labels in observed order.
 * Input Parameters: None
 * Return Value    : Ordered labels.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList ChartProperties::dataTypes() //20261112-1 add
{
    return {QStringLiteral("16位正整数"), QStringLiteral("32位正整数"), QStringLiteral("16位整数"), QStringLiteral("32位整数"), QStringLiteral("16位BCD码"), QStringLiteral("32位BCD码"), QStringLiteral("32位浮点数")};
}

/******************************************************************************
 * Function Name   : ChartProperties::patterns
 * Description     : Return the native enumerated labels in observed order.
 * Input Parameters: None
 * Return Value    : Ordered labels.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList ChartProperties::patterns() //20261112-1 add
{
    return {QStringLiteral("透明"), QStringLiteral("纯色"), QStringLiteral("横向过渡"), QStringLiteral("横向对称过渡"), QStringLiteral("纵向过渡"), QStringLiteral("纵向对称过渡"), QStringLiteral("斜上过渡"), QStringLiteral("斜上对称过渡"), QStringLiteral("斜下过渡"), QStringLiteral("斜下对称过渡"), QStringLiteral("右上角辐射"), QStringLiteral("左上角辐射"), QStringLiteral("中心辐射")};
}

/******************************************************************************
 * Function Name   : ChartProperties::lineTypes
 * Description     : Return the native enumerated labels in observed order.
 * Input Parameters: None
 * Return Value    : Ordered labels.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList ChartProperties::lineTypes() //20261112-1 add
{
    return {QStringLiteral("透明线"), QStringLiteral("实线"), QStringLiteral("虚线"), QStringLiteral("短划线"), QStringLiteral("点划线"), QStringLiteral("两点划线")};
}

/******************************************************************************
 * Function Name   : ChartProperties::directions
 * Description     : Return the native enumerated labels in observed order.
 * Input Parameters: None
 * Return Value    : Ordered labels.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList ChartProperties::directions() //20261112-1 add
{
    return {QStringLiteral("向上"), QStringLiteral("向下"), QStringLiteral("向左"), QStringLiteral("向右")};
}

/******************************************************************************
 * Function Name   : ChartProperties::meterShapes
 * Description     : Return the native enumerated labels in observed order.
 * Input Parameters: None
 * Return Value    : Ordered labels.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList ChartProperties::meterShapes() //20261112-1 add
{
    return {QStringLiteral("半圆指针向上"), QStringLiteral("全圆指针向上"), QStringLiteral("全圆指针向下"), QStringLiteral("四分之三圆指针向上")};
}

/******************************************************************************
 * Function Name   : ChartProperties::pointers
 * Description     : Return the native enumerated labels in observed order.
 * Input Parameters: None
 * Return Value    : Ordered labels.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList ChartProperties::pointers() //20261112-1 add
{
    return {QStringLiteral("细直线"), QStringLiteral("粗直线"), QStringLiteral("菱形"), QStringLiteral("三角形")};
}

/******************************************************************************
 * Function Name   : ChartProperties::typeRange
 * Description     : Use the native scalar ranges observed on explicit data-type activation.
 * Input Parameters: type
 * Return Value    : Lower and upper range strings.
 * Date            : 2026.11.12
 ******************************************************************************/
QPair<QString, QString> ChartProperties::typeRange(int type) //20261112-1 add
{
    switch (type) {
    case 0: return {QStringLiteral("0"), QStringLiteral("65535")};
    case 1: return {QStringLiteral("0"), QStringLiteral("4294967295")};
    case 2: return {QStringLiteral("-32768"), QStringLiteral("32767")};
    case 4: return {QStringLiteral("0"), QStringLiteral("9999")};
    case 5: return {QStringLiteral("0"), QStringLiteral("99999999")};
    default: return {QStringLiteral("-2147483648"), QStringLiteral("2147483647")};
    }
}

/******************************************************************************
 * Function Name   : ChartProperties::wordCount
 * Description     : Validate the entire active two-word span for U32, I32, BCD32 and F32.
 * Input Parameters: type
 * Return Value    : Register span.
 * Date            : 2026.11.12
 ******************************************************************************/
unsigned int ChartProperties::wordCount(int type) //20261112-1 add
{
    return type == 0 || type == 2 || type == 4 ? 1U : 2U;
}

/******************************************************************************
 * Function Name   : ChartProperties::address
 * Description     : Expose only the selected form monitor to the compact editor.
 * Input Parameters: None
 * Return Value    : Current monitor address.
 * Date            : 2026.11.12
 ******************************************************************************/
QString ChartProperties::address() const //20261112-1 add
{
    return shape == 0 ? bar.address : shape == 1 ? meter.address : donut.address;
}

/******************************************************************************
 * Function Name   : ChartProperties::setAddress
 * Description     : Update only the active monitor without destroying inactive form drafts.
 * Input Parameters: text
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void ChartProperties::setAddress(const QString &text) //20261112-1 add
{
    if (shape == 0) { bar.address = text; } else if (shape == 1) { meter.address = text; } else { donut.address = text; }
}

/******************************************************************************
 * Function Name   : ChartProperties::toJson
 * Description     : Persist shape, independent form drafts and display-only permissions.
 * Input Parameters: None
 * Return Value    : Typed chart editor record.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject ChartProperties::toJson() const //20261112-1 add
{
    QJsonObject json;
    json.insert(QStringLiteral("shape"), shape); json.insert(QStringLiteral("bar"), bar.toJson()); json.insert(QStringLiteral("meter"), meter.toJson()); json.insert(QStringLiteral("donut"), donut.toJson()); json.insert(QStringLiteral("visibility"), visibility.toJson()); json.insert(QStringLiteral("visibilityBitChosen"), visibilityBitChosen); return json;
}

/******************************************************************************
 * Function Name   : ChartProperties::fromJson
 * Description     : Decode atomically with exact keys and explicit integer and permission checks.
 * Input Parameters: json, output, error
 * Return Value    : True for a complete supported chart record.
 * Date            : 2026.11.12
 ******************************************************************************/
bool ChartProperties::fromJson(const QJsonObject &json, ChartProperties *output, QString *error) //20261112-1 add
{
    ChartProperties candidate;
    double value = json.value(QStringLiteral("shape")).toDouble(-1);
    if (output == nullptr || json.keys() != candidate.toJson().keys() || !json.value(QStringLiteral("shape")).isDouble() || !std::isfinite(value) || std::floor(value) != value || value < 0 || value > 2 || !json.value(QStringLiteral("visibilityBitChosen")).isBool()) { return false; }
    candidate.shape = int(value); candidate.visibilityBitChosen = json.value(QStringLiteral("visibilityBitChosen")).toBool();
    if (!json.value(QStringLiteral("bar")).isObject() || !ChartBarProperties::fromJson(json.value(QStringLiteral("bar")).toObject(), &candidate.bar) || !json.value(QStringLiteral("meter")).isObject() || !ChartMeterProperties::fromJson(json.value(QStringLiteral("meter")).toObject(), &candidate.meter) || !json.value(QStringLiteral("donut")).isObject() || !ChartDonutProperties::fromJson(json.value(QStringLiteral("donut")).toObject(), &candidate.donut) || !json.value(QStringLiteral("visibility")).isObject() || !StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() || !candidate.validate(error)) { return false; }
    *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : chartScalar
 * Description     : Reject malformed, nonfinite, fractional integer or out-of-range editor values.
 * Input Parameters: text, type
 * Return Value    : True for an admissible scalar.
 * Date            : 2026.11.12
 ******************************************************************************/
static bool chartScalar(const QString &text, int type) //20261112-1 add
{
    bool ok;
    double value = text.toDouble(&ok);
    QPair<QString, QString> range = ChartProperties::typeRange(type);
    if (text.isEmpty() || text.size() > 80 || !ok || !std::isfinite(value) || value < range.first.toDouble() || value > range.second.toDouble()) { return false; }
    return type == 6 || QRegularExpression(QStringLiteral("^[+-]?[0-9]+$")).match(text).hasMatch();
}

/******************************************************************************
 * Function Name   : ChartProperties::validate
 * Description     : Check all structural drafts and only active range semantics; preserve dormant values.
 * Input Parameters: error
 * Return Value    : True for the declared editor slice.
 * Date            : 2026.11.12
 ******************************************************************************/
bool ChartProperties::validate(QString *error) const //20261112-1 add
{
    QString color;
    bool ok;
    double radius = donut.radius.toDouble(&ok);
    if (shape < 0 || shape > 2 || bar.dataType < 0 || bar.dataType > 6 || meter.dataType < 0 || meter.dataType > 6 || donut.dataType < 0 || donut.dataType > 6 || bar.direction < 0 || bar.direction > 3 || bar.pattern < 0 || bar.pattern > 12 || meter.direction < 0 || meter.direction > 1 || meter.pointer < 0 || meter.pointer > 3 || meter.shape < 0 || meter.shape > 3 || meter.mainDivisions < 2 || meter.mainDivisions > 20 || meter.subDivisions < 2 || meter.subDivisions > 5 || meter.fontSize < 5 || meter.fontSize > 100 || donut.lineType < 0 || donut.lineType > 5 || donut.lineWidth < 1 || donut.lineWidth > 32 || donut.fillWidth < 1 || donut.fillWidth > 32 || donut.opacity < 0 || donut.opacity > 255 || bar.address.size() > 1024 || meter.address.size() > 1024 || donut.address.size() > 1024 || bar.minimumAddress.size() > 1024 || bar.maximumAddress.size() > 1024 || !ok || !std::isfinite(radius) || radius < 0 || radius > 10000) { if (error != nullptr) { *error = QStringLiteral("图表枚举、尺寸或字段超出本版编辑范围。"); } return false; }
    Q_FOREACH (color, QStringList({bar.foreground, bar.background, bar.patternColor, meter.foreground, meter.background, meter.scaleColor, donut.lineColor})) { if (color.size() > 32 || !QColor(color).isValid()) { return false; } }
    if (!chartScalar(donut.startAngle, 3) || !chartScalar(donut.spanAngle, 3)) { return false; }
    if ((shape == 0 && (!chartScalar(bar.minimum, bar.dataType) || !chartScalar(bar.maximum, bar.dataType))) || (shape == 1 && (!chartScalar(meter.minimum, meter.dataType) || !chartScalar(meter.maximum, meter.dataType))) || (shape == 2 && (!chartScalar(donut.minimum, donut.dataType) || !chartScalar(donut.maximum, donut.dataType)))) { if (error != nullptr) { *error = QStringLiteral("显示范围格式错误或超出数据类型范围。"); } return false; }
    if ((shape == 0 && bar.minimum.toDouble() > bar.maximum.toDouble()) || (shape == 1 && meter.minimum.toDouble() > meter.maximum.toDouble()) || (shape == 2 && donut.minimum.toDouble() > donut.maximum.toDouble())) { if (error != nullptr) { *error = QStringLiteral("显示范围最大值小于最小值"); } return false; }
    if (shape == 0 && bar.twoPoles && !chartScalar(bar.midpoint, bar.dataType)) { if (error != nullptr) { *error = QStringLiteral("两极柱中点格式错误。"); } return false; }
    if (shape == 0 && !bar.dynamicRange)
    {
        if (!chartScalar(bar.sourceMinimum, bar.dataType) || !chartScalar(bar.sourceMaximum, bar.dataType) || bar.sourceMinimum.toDouble() > bar.sourceMaximum.toDouble()) { if (error != nullptr) { *error = QStringLiteral("源范围最大最小值格式错误或顺序无效。"); } return false; }
        if (bar.twoPoles && (!chartScalar(bar.midpoint, bar.dataType) || bar.midpoint.toDouble() < bar.sourceMinimum.toDouble() || bar.midpoint.toDouble() > bar.sourceMaximum.toDouble())) { if (error != nullptr) { *error = QStringLiteral("两极柱中点应在源范围最大最小值之间"); } return false; }
    }
    if (visibility.visibilityEnabled && visibility.bitEnabled && !visibilityBitChosen) { if (error != nullptr) { *error = QStringLiteral("请选择显现状态。"); } return false; }
    return visibility.validate(error);
}

/******************************************************************************
 * Function Name   : ChartProperties::validateAddresses
 * Description     : Validate active monitor, source range and display permissions with full register spans.
 * Input Parameters: connections, error
 * Return Value    : True for all active addresses.
 * Date            : 2026.11.12
 ******************************************************************************/
bool ChartProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261112-1 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress parsed;
    ScreenButtonProperties shared;
    QStringList words;
    QString word;
    int type = shape == 0 ? bar.dataType : shape == 1 ? meter.dataType : donut.dataType;
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    words.append(address());
    if (shape == 0 && bar.dynamicRange) { words.append(bar.minimumAddress); words.append(bar.maximumAddress); }
    context.connections = connections; context.role = PlcAddressRole::Word; context.transferCount = wordCount(type);
    Q_FOREACH (word, words) { if (!parseQualifiedPlcAddressText(word, context, catalog, &parsed, error)) { return false; } }
    shared.common.indicator = false; shared.common.visibility = visibility; return shared.validateAddresses(connections, error);
}

