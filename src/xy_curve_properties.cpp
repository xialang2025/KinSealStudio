#include "xy_curve_properties.h"
#include "plc_native_catalog.h"
#include <QJsonArray>
#include <QColor>
#include <QRegularExpression>
#include <cmath>

/******************************************************************************
 * Function Name   : XyCurveProperties::dataTypes
 * Description     : Preserve the trend-specific signed-first native data-type order.
 * Input Parameters: None
 * Return Value    : Seven native labels.
 * Date            : 2026.11.13
 ******************************************************************************/
QStringList XyCurveProperties::dataTypes() //20261113-1 add
{
    return {QStringLiteral("16位整数"), QStringLiteral("32位整数"), QStringLiteral("16位正整数"), QStringLiteral("32位正整数"), QStringLiteral("16位BCD码"), QStringLiteral("32位BCD码"), QStringLiteral("32位浮点数")};
}

/******************************************************************************
 * Function Name   : XyCurveProperties::wordCount
 * Description     : Cover the complete contiguous channel read span for 16-bit and 32-bit types.
 * Input Parameters: type
 * Return Value    : Words occupied by one channel.
 * Date            : 2026.11.13
 ******************************************************************************/
u32 XyCurveProperties::wordCount(int type) //20261113-1 add
{
    return type == 0 || type == 2 || type == 4 ? 1U : 2U;
}

/******************************************************************************
 * Function Name   : XyCurveProperties::typeRange
 * Description     : Bound editable channel scalars without resetting them when the type changes.
 * Input Parameters: type
 * Return Value    : Supported lower and upper scalar strings.
 * Date            : 2026.11.13
 ******************************************************************************/
QPair<QString, QString> XyCurveProperties::typeRange(int type) //20261113-1 add
{
    switch (type) {
    case 0: return {QStringLiteral("-32768"), QStringLiteral("32767")};
    case 2: return {QStringLiteral("0"), QStringLiteral("65535")};
    case 3: return {QStringLiteral("0"), QStringLiteral("4294967295")};
    case 4: return {QStringLiteral("0"), QStringLiteral("9999")};
    case 5: return {QStringLiteral("0"), QStringLiteral("99999999")};
    default: return {QStringLiteral("-2147483648"), QStringLiteral("2147483647")};
    }
}

/******************************************************************************
 * Function Name   : xyScalar
 * Description     : Reject malformed, nonfinite and out-of-range values before editor acceptance.
 * Input Parameters: text, type
 * Return Value    : True for the supported scalar range.
 * Date            : 2026.11.13
 ******************************************************************************/
static bool xyScalar(const QString &text, int type) //20261113-1 add
{
    bool ok;
    double value = text.toDouble(&ok);
    QPair<QString, QString> range = XyCurveProperties::typeRange(type);
    return text.size() <= 80 && !text.isEmpty() && ok && std::isfinite(value) && value >= range.first.toDouble() && value <= range.second.toDouble() && (type == 6 || QRegularExpression(QStringLiteral("^[+-]?[0-9]+$")).match(text).hasMatch());
}

/******************************************************************************
 * Function Name   : XyCurveProperties::validate
 * Description     : Validate measured sampling and axis limits, channel structure and active scalar semantics.
 * Input Parameters: error
 * Return Value    : True for the bounded editor slice.
 * Date            : 2026.11.13
 ******************************************************************************/
bool XyCurveProperties::validate(QString *error) const //20261113-1 add
{
    int index;
    TrendAxis axis;
    QString color;
    QStringList colors = {bottomColor, frameColor, backgroundColor};
    if (dataType < 0 || dataType > 6 || samplePoints < 1 || samplePoints > 100 || sampleTime < 1 || sampleTime > 255 || channelCount < 1 || channelCount > 8 || currentChannel < 0 || currentChannel >= channelCount || xTotalDigits < 1 || xTotalDigits > 10 || xFractionalDigits < 0 || xFractionalDigits > 3 || totalDigits < 1 || totalDigits > 10 || fractionalDigits < 0 || fractionalDigits > 3 || triggerAddress.size() > 1024 || readAddress.size() > 1024 || clearAddress.size() > 1024 || (visibility.visibilityEnabled && visibility.bitEnabled && !visibilityBitChosen)) { if (error != nullptr) { *error = QStringLiteral("X-Y曲线采样、通道、数值位数或显现状态无效。"); } return false; }
    for (index = 0; index < 2; ++index)
    {
        axis = index == 0 ? xAxis : yAxis;
        if (axis.major < 3 || axis.major > 10 || axis.minor < 1 || axis.minor > 3 || !QList<int>({12, 16, 24, 48}).contains(axis.fontSize)) { return false; }
        colors.append(axis.axisColor); colors.append(axis.gridColor);
    }
    for (index = 0; index < 8; ++index)
    {
        colors.append(channels[index].color);
        if (channels[index].lineStyle < 0 || channels[index].lineStyle > 4) { return false; }
        if (index < channelCount && (!xyScalar(channels[index].minimum, dataType) || !xyScalar(channels[index].maximum, dataType) || channels[index].minimum.toDouble() >= channels[index].maximum.toDouble())) { if (error != nullptr) { *error = QStringLiteral("本版要求活动通道范围有效且最小值小于最大值。"); } return false; }
    }
    Q_FOREACH (color, colors) { if (color.size() > 32 || !QColor(color).isValid()) { return false; } }
    if (!xyScalar(xMinimum, 6) || !xyScalar(xMaximum, 6) || xMinimum.toDouble() >= xMaximum.toDouble()) { if (error != nullptr) { *error = QStringLiteral("本版要求X刻度范围有效且最小值小于最大值。"); } return false; }
    if (!xyScalar(yMinimum, 6) || !xyScalar(yMaximum, 6) || yMinimum.toDouble() >= yMaximum.toDouble()) { if (error != nullptr) { *error = QStringLiteral("本版要求Y刻度范围有效且最小值小于最大值。"); } return false; }
    return visibility.validate(error);
}

/******************************************************************************
 * Function Name   : XyCurveProperties::validateAddresses
 * Description     : Check trigger/clear bits, all contiguous read words and display-only permissions.
 * Input Parameters: connections, error
 * Return Value    : True for every active carrier span.
 * Date            : 2026.11.13
 ******************************************************************************/
bool XyCurveProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261113-1 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress parsed;
    ScreenButtonProperties shared;
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    context.connections = connections; context.role = PlcAddressRole::Word; context.transferCount = wordCount(dataType) * u32(channelCount) * 2U; // Conservative full X/Y-pair guard; firmware ordering remains unverified.
    if (!parseQualifiedPlcAddressText(readAddress, context, catalog, &parsed, error)) { return false; }
    context.role = PlcAddressRole::Bit; context.transferCount = 1U;
    if (!parseQualifiedPlcAddressText(triggerAddress, context, catalog, &parsed, error) || !parseQualifiedPlcAddressText(clearAddress, context, catalog, &parsed, error)) { return false; }
    shared.common.indicator = false; shared.common.visibility = visibility; return shared.validateAddresses(connections, error);
}

/******************************************************************************
 * Function Name   : XyCurveProperties::toJson
 * Description     : Preserve all supported fields and dormant axis values in a typed editor record.
 * Input Parameters: None
 * Return Value    : Complete JSON record.
 * Date            : 2026.11.13
 ******************************************************************************/
QJsonObject XyCurveProperties::toJson() const //20261113-1 add
{
    QJsonObject json;
    QJsonArray entries;
    const TrendChannel *channel;
    for (channel = channels.data(); channel != channels.data() + channels.size(); ++channel) { entries.append(channel->toJson()); }
    json.insert(QStringLiteral("channels"), entries); json.insert(QStringLiteral("xAxis"), xAxis.toJson()); json.insert(QStringLiteral("yAxis"), yAxis.toJson()); json.insert(QStringLiteral("visibility"), visibility.toJson());
    json.insert(QStringLiteral("triggerAddress"), triggerAddress);
    json.insert(QStringLiteral("readAddress"), readAddress);
    json.insert(QStringLiteral("clearAddress"), clearAddress);
    json.insert(QStringLiteral("dataType"), dataType);
    json.insert(QStringLiteral("samplePoints"), samplePoints);
    json.insert(QStringLiteral("sampleTime"), sampleTime);
    json.insert(QStringLiteral("channelCount"), channelCount);
    json.insert(QStringLiteral("currentChannel"), currentChannel);
    json.insert(QStringLiteral("bottomColor"), bottomColor);
    json.insert(QStringLiteral("frameColor"), frameColor);
    json.insert(QStringLiteral("backgroundColor"), backgroundColor);
    json.insert(QStringLiteral("xMinimum"), xMinimum);
    json.insert(QStringLiteral("xMaximum"), xMaximum);
    json.insert(QStringLiteral("xTotalDigits"), xTotalDigits);
    json.insert(QStringLiteral("xFractionalDigits"), xFractionalDigits);
    json.insert(QStringLiteral("yMinimum"), yMinimum);
    json.insert(QStringLiteral("yMaximum"), yMaximum);
    json.insert(QStringLiteral("totalDigits"), totalDigits);
    json.insert(QStringLiteral("fractionalDigits"), fractionalDigits);
    json.insert(QStringLiteral("visibilityBitChosen"), visibilityBitChosen);
    return json;
}

/******************************************************************************
 * Function Name   : XyCurveProperties::fromJson
 * Description     : Reject unknown, missing, mistyped or lossy values before replacing the destination.
 * Input Parameters: json, output, error
 * Return Value    : True for an exact supported record.
 * Date            : 2026.11.13
 ******************************************************************************/
bool XyCurveProperties::fromJson(const QJsonObject &json, XyCurveProperties *output, QString *error) //20261113-1 add
{
    XyCurveProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator field;
    double number;
    int index;
    QJsonArray entries;
    if (output == nullptr || json.keys() != shape.keys()) { if (error != nullptr) { *error = QStringLiteral("X-Y曲线字段缺失或未知。"); } return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != shape.value(field.key()).type()) { return false; }
        if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 1000) { return false; } }
        if (field.value().isString() && field.value().toString().size() > 1024) { return false; }
    }
    candidate.triggerAddress = json.value(QStringLiteral("triggerAddress")).toString();
    candidate.readAddress = json.value(QStringLiteral("readAddress")).toString();
    candidate.clearAddress = json.value(QStringLiteral("clearAddress")).toString();
    candidate.dataType = json.value(QStringLiteral("dataType")).toInt();
    candidate.samplePoints = json.value(QStringLiteral("samplePoints")).toInt();
    candidate.sampleTime = json.value(QStringLiteral("sampleTime")).toInt();
    candidate.channelCount = json.value(QStringLiteral("channelCount")).toInt();
    candidate.currentChannel = json.value(QStringLiteral("currentChannel")).toInt();
    candidate.bottomColor = json.value(QStringLiteral("bottomColor")).toString();
    candidate.frameColor = json.value(QStringLiteral("frameColor")).toString();
    candidate.backgroundColor = json.value(QStringLiteral("backgroundColor")).toString();
    candidate.xMinimum = json.value(QStringLiteral("xMinimum")).toString();
    candidate.xMaximum = json.value(QStringLiteral("xMaximum")).toString();
    candidate.xTotalDigits = json.value(QStringLiteral("xTotalDigits")).toInt();
    candidate.xFractionalDigits = json.value(QStringLiteral("xFractionalDigits")).toInt();
    candidate.yMinimum = json.value(QStringLiteral("yMinimum")).toString();
    candidate.yMaximum = json.value(QStringLiteral("yMaximum")).toString();
    candidate.totalDigits = json.value(QStringLiteral("totalDigits")).toInt();
    candidate.fractionalDigits = json.value(QStringLiteral("fractionalDigits")).toInt();
    candidate.visibilityBitChosen = json.value(QStringLiteral("visibilityBitChosen")).toBool();
    entries = json.value(QStringLiteral("channels")).toArray(); if (entries.size() != 8) { return false; }
    for (index = 0; index < 8; ++index) { if (!entries.at(index).isObject() || !TrendChannel::fromJson(entries.at(index).toObject(), &candidate.channels[index], error)) { return false; } }
    if (!TrendAxis::fromJson(json.value(QStringLiteral("xAxis")).toObject(), &candidate.xAxis, error) || !TrendAxis::fromJson(json.value(QStringLiteral("yAxis")).toObject(), &candidate.yAxis, error) || !StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() || !candidate.validate(error)) { return false; }
    *output = candidate; return true;
}

