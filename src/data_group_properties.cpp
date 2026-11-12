#include "data_group_properties.h"
#include "plc_native_catalog.h"
#include <QColor>
#include <cmath>

/******************************************************************************
 * Function Name   : DataGroupProperties::dataTypes
 * Description     : Use the observed signed-first seven-type order rather than the older handbook order.
 * Input Parameters: None
 * Return Value    : Ordered native type labels.
 * Date            : 2026.11.13
 ******************************************************************************/
QStringList DataGroupProperties::dataTypes() //20261113-1 add
{
    return TrendProperties::dataTypes();
}

/******************************************************************************
 * Function Name   : DataGroupProperties::wordCount
 * Description     : Protect the complete scalar word width independently of unverified firmware byte ordering.
 * Input Parameters: type
 * Return Value    : One or two words.
 * Date            : 2026.11.13
 ******************************************************************************/
u32 DataGroupProperties::wordCount(int type) //20261113-1 add
{
    return TrendProperties::wordCount(type);
}

/******************************************************************************
 * Function Name   : DataGroupProperties::readWordSpan
 * Description     : Protect the entire declared point buffer in both sample modes; the runtime sample-address meaning remains separate.
 * Input Parameters: None
 * Return Value    : Declared contiguous point-buffer word span.
 * Date            : 2026.11.13
 ******************************************************************************/
u32 DataGroupProperties::readWordSpan() const //20261113-1 add
{
    return u32(samplePoints) * wordCount(dataType);
}

/******************************************************************************
 * Function Name   : groupScalar
 * Description     : Bound active display scalars while allowing the editor to retain an inactive maximum draft.
 * Input Parameters: text
 * Return Value    : True for a bounded finite scalar.
 * Date            : 2026.11.13
 ******************************************************************************/
static bool groupScalar(const QString &text) //20261113-1 add
{
    bool ok;
    double value = text.toDouble(&ok);
    return !text.isEmpty() && text.size() <= 80 && ok && std::isfinite(value) && std::abs(value) <= 3.402823466e38;

}

/******************************************************************************
 * Function Name   : DataGroupProperties::validate
 * Description     : Enforce observed single-curve limits and active range semantics without inventing hidden channel fields.
 * Input Parameters: error
 * Return Value    : True for the supported editor slice.
 * Date            : 2026.11.13
 ******************************************************************************/
bool DataGroupProperties::validate(QString *error) const //20261113-1 add
{
    TrendAxis axis;
    QString color;
    QStringList colors = {channelColor, bottomColor, frameColor, backgroundColor};
    int index;
    if (dataType < 0 || dataType > 6 || sampleMode < 0 || sampleMode > 1 || samplePoints < 10 || samplePoints > 800 || lineStyle < 0 || lineStyle > 4 || rangeSource < 0 || rangeSource > 1 || totalDigits < 1 || totalDigits > 10 || fractionalDigits < 0 || fractionalDigits > 3 || triggerAddress.size() > 1024 || readAddress.size() > 1024 || sampleAddress.size() > 1024 || maximumAddress.size() > 1024 || yMinimum.size() > 80 || yMaximum.size() > 80 || (visibility.visibilityEnabled && visibility.bitEnabled && !visibilityBitChosen)) { if (error != nullptr) { *error = QStringLiteral("数据群组采样、范围、位数或显现状态无效。"); } return false; }
    for (index = 0; index < 2; ++index) { axis = index == 0 ? xAxis : yAxis; if (axis.major < 3 || axis.major > 10 || axis.minor < 1 || axis.minor > 3 || !QList<int>({12, 16, 24, 48}).contains(axis.fontSize)) { return false; } colors.append(axis.axisColor); colors.append(axis.gridColor); }
    Q_FOREACH (color, colors) { if (color.size() > 32 || !QColor(color).isValid()) { return false; } }
    if (!groupScalar(yMinimum) || (rangeSource == 0 && (!groupScalar(yMaximum) || yMinimum.toDouble() >= yMaximum.toDouble()))) { if (error != nullptr) { *error = QStringLiteral("本版要求活动常量范围有效，最小值小于最大值。"); } return false; }
    return visibility.validate(error);

}

/******************************************************************************
 * Function Name   : DataGroupProperties::validateAddresses
 * Description     : Validate the declared buffer span and only the active sample and maximum source carriers.
 * Input Parameters: connections, error
 * Return Value    : True for all active editor carriers.
 * Date            : 2026.11.13
 ******************************************************************************/
bool DataGroupProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261113-1 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress parsed;
    ScreenButtonProperties shared;
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    context.connections = connections; context.role = PlcAddressRole::Word; context.transferCount = readWordSpan();
    if (!parseQualifiedPlcAddressText(readAddress, context, catalog, &parsed, error)) { return false; }
    context.transferCount = 1U;
    if (sampleMode == 1 && !parseQualifiedPlcAddressText(sampleAddress, context, catalog, &parsed, error)) { return false; }
    context.transferCount = wordCount(dataType);
    if (rangeSource == 1 && !parseQualifiedPlcAddressText(maximumAddress, context, catalog, &parsed, error)) { return false; }
    context.role = PlcAddressRole::Bit; context.transferCount = 1U;
    if (!parseQualifiedPlcAddressText(triggerAddress, context, catalog, &parsed, error)) { return false; }
    shared.common.indicator = false; shared.common.visibility = visibility; return shared.validateAddresses(connections, error);

}

/******************************************************************************
 * Function Name   : DataGroupProperties::toJson
 * Description     : Persist active and inactive drafts as explicit typed editor data.
 * Input Parameters: None
 * Return Value    : Complete data-group JSON record.
 * Date            : 2026.11.13
 ******************************************************************************/
QJsonObject DataGroupProperties::toJson() const //20261113-1 add
{
    QJsonObject json;
    json.insert(QStringLiteral("triggerAddress"), triggerAddress);
    json.insert(QStringLiteral("readAddress"), readAddress);
    json.insert(QStringLiteral("dataType"), dataType);
    json.insert(QStringLiteral("sampleMode"), sampleMode);
    json.insert(QStringLiteral("samplePoints"), samplePoints);
    json.insert(QStringLiteral("sampleAddress"), sampleAddress);
    json.insert(QStringLiteral("lineStyle"), lineStyle);
    json.insert(QStringLiteral("channelColor"), channelColor);
    json.insert(QStringLiteral("bottomColor"), bottomColor);
    json.insert(QStringLiteral("frameColor"), frameColor);
    json.insert(QStringLiteral("backgroundColor"), backgroundColor);
    json.insert(QStringLiteral("rangeSource"), rangeSource);
    json.insert(QStringLiteral("yMinimum"), yMinimum);
    json.insert(QStringLiteral("yMaximum"), yMaximum);
    json.insert(QStringLiteral("maximumAddress"), maximumAddress);
    json.insert(QStringLiteral("totalDigits"), totalDigits);
    json.insert(QStringLiteral("fractionalDigits"), fractionalDigits);
    json.insert(QStringLiteral("visibilityBitChosen"), visibilityBitChosen);
    json.insert(QStringLiteral("xAxis"), xAxis.toJson()); json.insert(QStringLiteral("yAxis"), yAxis.toJson()); json.insert(QStringLiteral("visibility"), visibility.toJson()); return json;
}

/******************************************************************************
 * Function Name   : DataGroupProperties::fromJson
 * Description     : Reject incomplete, unknown and mistyped fields before replacing the caller state.
 * Input Parameters: json, output, error
 * Return Value    : True for an exact validated record.
 * Date            : 2026.11.13
 ******************************************************************************/
bool DataGroupProperties::fromJson(const QJsonObject &json, DataGroupProperties *output, QString *error) //20261113-1 add
{
    DataGroupProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator field;
    double number;
    if (output == nullptr || json.keys() != shape.keys()) { if (error != nullptr) { *error = QStringLiteral("数据群组字段缺失或未知。"); } return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != shape.value(field.key()).type()) { return false; }
        if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 65535) { return false; } }
        if (field.value().isString() && field.value().toString().size() > 1024) { return false; }
    }
    candidate.triggerAddress = json.value(QStringLiteral("triggerAddress")).toString();
    candidate.readAddress = json.value(QStringLiteral("readAddress")).toString();
    candidate.dataType = json.value(QStringLiteral("dataType")).toInt();
    candidate.sampleMode = json.value(QStringLiteral("sampleMode")).toInt();
    candidate.samplePoints = json.value(QStringLiteral("samplePoints")).toInt();
    candidate.sampleAddress = json.value(QStringLiteral("sampleAddress")).toString();
    candidate.lineStyle = json.value(QStringLiteral("lineStyle")).toInt();
    candidate.channelColor = json.value(QStringLiteral("channelColor")).toString();
    candidate.bottomColor = json.value(QStringLiteral("bottomColor")).toString();
    candidate.frameColor = json.value(QStringLiteral("frameColor")).toString();
    candidate.backgroundColor = json.value(QStringLiteral("backgroundColor")).toString();
    candidate.rangeSource = json.value(QStringLiteral("rangeSource")).toInt();
    candidate.yMinimum = json.value(QStringLiteral("yMinimum")).toString();
    candidate.yMaximum = json.value(QStringLiteral("yMaximum")).toString();
    candidate.maximumAddress = json.value(QStringLiteral("maximumAddress")).toString();
    candidate.totalDigits = json.value(QStringLiteral("totalDigits")).toInt();
    candidate.fractionalDigits = json.value(QStringLiteral("fractionalDigits")).toInt();
    candidate.visibilityBitChosen = json.value(QStringLiteral("visibilityBitChosen")).toBool();
    if (!TrendAxis::fromJson(json.value(QStringLiteral("xAxis")).toObject(), &candidate.xAxis, error) || !TrendAxis::fromJson(json.value(QStringLiteral("yAxis")).toObject(), &candidate.yAxis, error) || !StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() || !candidate.validate(error)) { return false; }
    *output = candidate; return true;

}

