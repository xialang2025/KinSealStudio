#include "ascii_display_properties.h"
#include <cmath>

/******************************************************************************
 * Function Name   : AsciiDisplayProperties::toJson
 * Description     : Preserve the independent ASCII options and every shared or hidden numeric editor draft.
 * Input Parameters: None
 * Return Value    : Complete ASCII editor record.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject AsciiDisplayProperties::toJson() const //20261112-0 add
{
    return {{QStringLiteral("common"), common.toJson()}, {QStringLiteral("characterCount"), characterCount}, {QStringLiteral("encoding"), encoding},
            {QStringLiteral("autoWrap"), autoWrap}, {QStringLiteral("swapBytes"), swapBytes}, {QStringLiteral("modifyInput"), modifyInput}};
}

/******************************************************************************
 * Function Name   : AsciiDisplayProperties::fromJson
 * Description     : Strictly parse ASCII records while retaining inactive numeric ranges without validating them as active ASCII inputs.
 * Input Parameters: json, output, error
 * Return Value    : True after complete validation; failure leaves output untouched.
 * Date            : 2026.11.12
 ******************************************************************************/
bool AsciiDisplayProperties::fromJson(const QJsonObject &json, AsciiDisplayProperties *output, QString *error) //20261112-0 add
{
    AsciiDisplayProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator item;
    double value;
    if (output == nullptr || json.keys() != shape.keys()) { if (error != nullptr) { *error = QStringLiteral("ASCII字段缺失、未知或接收位置为空。"); } return false; }
    for (item = json.constBegin(); item != json.constEnd(); ++item)
    {
        if (item.value().type() != shape.value(item.key()).type()) { if (error != nullptr) { *error = QStringLiteral("ASCII字段类型错误。"); } return false; }
        if (item.value().isDouble())
        {
            value = item.value().toDouble();
            if (!std::isfinite(value) || value != std::floor(value) || value < 0 || value > 256) { if (error != nullptr) { *error = QStringLiteral("ASCII字符数或编码枚举无效。"); } return false; }
        }
    }
    if (!NumberDisplayProperties::fromJson(json.value(QStringLiteral("common")).toObject(), &candidate.common, error, false)) { return false; }
    candidate.characterCount = json.value(QStringLiteral("characterCount")).toInt(); candidate.encoding = json.value(QStringLiteral("encoding")).toInt();
    candidate.autoWrap = json.value(QStringLiteral("autoWrap")).toBool(); candidate.swapBytes = json.value(QStringLiteral("swapBytes")).toBool(); candidate.modifyInput = json.value(QStringLiteral("modifyInput")).toBool();
    if (!candidate.validate(error)) { return false; } *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : AsciiDisplayProperties::validate
 * Description     : Apply measured ASCII bounds while leaving hidden numeric-only values dormant and lossless.
 * Input Parameters: error
 * Return Value    : True for a complete bounded ASCII editor model.
 * Date            : 2026.11.12
 ******************************************************************************/
bool AsciiDisplayProperties::validate(QString *error) const //20261112-0 add
{
    if (characterCount < 1 || characterCount > 256 || encoding < 0 || encoding > 1)
    { if (error != nullptr) { *error = QStringLiteral("ASCII字符数必须为1..256，编码必须为ASCII或GB2312。"); } return false; }
    return common.validate(error, false);
}

/******************************************************************************
 * Function Name   : AsciiDisplayProperties::validateAddresses
 * Description     : Validate declared base carriers and permission/notification roles without guessing ASCII/GB2312 string span or byte order.
 * Input Parameters: connections, error
 * Return Value    : True for configured base addresses; complete string spans remain unrecovered.
 * Date            : 2026.11.12
 ******************************************************************************/
bool AsciiDisplayProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261112-0 add
{
    NumberDisplayProperties base = common;
    base.dataType = 0; base.inputRangeSource = 0; base.displayRange = false; // Only shared roles are active in ASCII mode; one word validates a carrier, not the unknown full string span.
    return base.validateAddresses(connections, error);
}
