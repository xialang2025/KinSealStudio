#include "qr_code_properties.h"
#include <cmath>

/******************************************************************************
 * Function Name   : QrCodeProperties::toJson
 * Description     : Persist every QR field, including inactive permission drafts, without a synthetic device record.
 * Input Parameters: None
 * Return Value    : Complete editor QR record.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject QrCodeProperties::toJson() const //20261112-0 add
{
    return {{QStringLiteral("monitorAddress"), monitorAddress}, {QStringLiteral("encoding"), encoding},
            {QStringLiteral("swapBytes"), swapBytes}, {QStringLiteral("characterLength"), characterLength},
            {QStringLiteral("description"), description}, {QStringLiteral("visibility"), visibility.toJson()}};
}

/******************************************************************************
 * Function Name   : QrCodeProperties::fromJson
 * Description     : Reject missing, foreign and lossy fields before replacing the caller's QR draft.
 * Input Parameters: json, output, error
 * Return Value    : True for a complete validated editor record.
 * Date            : 2026.11.12
 ******************************************************************************/
bool QrCodeProperties::fromJson(const QJsonObject &json, QrCodeProperties *output, QString *error) //20261112-0 add
{
    QrCodeProperties candidate;
    QJsonObject expected = candidate.toJson();
    QJsonObject::const_iterator item;
    double number;
    if (output == nullptr || json.keys() != expected.keys())
    { if (error != nullptr) { *error = QStringLiteral("二维码字段缺失、未知或接收位置为空。"); } return false; }
    for (item = json.constBegin(); item != json.constEnd(); ++item)
    {
        if (item.value().type() != expected.value(item.key()).type())
        { if (error != nullptr) { *error = QStringLiteral("二维码字段类型错误：%1").arg(item.key()); } return false; }
        if (item.value().isDouble())
        {
            number = item.value().toDouble();
            if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 255)
            { if (error != nullptr) { *error = QStringLiteral("二维码枚举或长度必须为0..255的整数。"); } return false; }
        }
    }
    candidate.monitorAddress = json.value(QStringLiteral("monitorAddress")).toString();
    candidate.encoding = json.value(QStringLiteral("encoding")).toInt(); candidate.swapBytes = json.value(QStringLiteral("swapBytes")).toBool();
    candidate.characterLength = json.value(QStringLiteral("characterLength")).toInt(); candidate.description = json.value(QStringLiteral("description")).toString();
    if (!StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error)) { return false; }
    if (candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject())
    { if (error != nullptr) { *error = QStringLiteral("二维码显现字段不完整或含未知字段。"); } return false; }
    if (!candidate.validate(error)) { return false; }
    *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : QrCodeProperties::validate
 * Description     : Apply measured native field limits while keeping unknown PLC encoding independent of preview capacity.
 * Input Parameters: error
 * Return Value    : True for the observed editing fields.
 * Date            : 2026.11.12
 ******************************************************************************/
bool QrCodeProperties::validate(QString *error) const //20261112-0 add
{
    if (encoding < 0 || encoding > 1 || characterLength < 0 || characterLength > 255 ||
        description.size() > 32767 || monitorAddress.isEmpty() || monitorAddress.size() > 256)
    { if (error != nullptr) { *error = QStringLiteral("二维码属性超出已核定范围。"); } return false; }
    return visibility.validate(error);
}

/******************************************************************************
 * Function Name   : QrCodeProperties::validateAddresses
 * Description     : Validate the monitor word and enabled visibility reads without guessing string byte spans.
 * Input Parameters: connections, error
 * Return Value    : True when each known address carrier is valid.
 * Date            : 2026.11.12
 ******************************************************************************/
bool QrCodeProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261112-0 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress address;
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    context.connections = connections; context.role = PlcAddressRole::Word; context.transferCount = 1;
    if (!parseQualifiedPlcAddressText(monitorAddress, context, catalog, &address, error)) { return false; }
    if (visibility.visibilityEnabled && visibility.bitEnabled)
    {
        context.role = PlcAddressRole::Bit;
        if (!parseQualifiedPlcAddressText(visibility.bitAddress, context, catalog, &address, error)) { return false; }
    }
    if (visibility.visibilityEnabled && visibility.wordEnabled)
    {
        context.role = PlcAddressRole::Word; context.transferCount = visibility.dataType == 0 || visibility.dataType == 2 ? 1U : 2U;
        if (!parseQualifiedPlcAddressText(visibility.wordAddress, context, catalog, &address, error)) { return false; }
    }
    return true;
}
