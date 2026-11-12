#include "time_display_properties.h"
#include "plc_native_catalog.h"
#include <cmath>

/******************************************************************************
 * Function Name   : TimeDisplayProperties::dateFormats
 * Description     : Return the measured nine date format options in native order.
 * Input Parameters: None
 * Return Value    : Editor date formats, not firmware enum values.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList TimeDisplayProperties::dateFormats() //20261112-0 add
{
    return {QStringLiteral("YYYY/MM/DD"), QStringLiteral("YYYY.MM.DD"), QStringLiteral("YYYY-MM-DD"), QStringLiteral("MM/DD/YYYY"), QStringLiteral("MM.DD.YYYY"), QStringLiteral("MM-DD-YYYY"), QStringLiteral("DD/MM/YYYY"), QStringLiteral("DD.MM.YYYY"), QStringLiteral("DD-MM-YYYY")};
}

/******************************************************************************
 * Function Name   : TimeDisplayProperties::timeFormats
 * Description     : Return the four independently observed clock format options.
 * Input Parameters: None
 * Return Value    : Native ordered labels.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList TimeDisplayProperties::timeFormats() //20261112-0 add
{
    return {QStringLiteral("HH:MM"), QStringLiteral("HH:MM:SS"), QStringLiteral("HH-MM"), QStringLiteral("HH-MM-SS")};
}

/******************************************************************************
 * Function Name   : TimeDisplayProperties::fontSizes
 * Description     : Keep the measured time-only font sizes independent of numeric and ASCII lists.
 * Input Parameters: None
 * Return Value    : Seven measured bitmap-font size labels.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList TimeDisplayProperties::fontSizes() //20261112-0 add
{
    return {QStringLiteral("12"), QStringLiteral("16"), QStringLiteral("24"), QStringLiteral("48"), QStringLiteral("64"), QStringLiteral("96"), QStringLiteral("192")};
}

/******************************************************************************
 * Function Name   : TimeDisplayProperties::sample
 * Description     : Compose the editor format placeholder without reading or modifying any system RTC.
 * Input Parameters: None
 * Return Value    : Date, week and time tokens in native sample order.
 * Date            : 2026.11.12
 ******************************************************************************/
QString TimeDisplayProperties::sample() const //20261112-0 add
{
    QStringList parts;
    if (showDate) { parts.append(dateFormats().value(dateFormat)); }
    if (showWeek) { parts.append(QStringLiteral("(Mon)")); }
    if (showTime) { parts.append(timeFormats().value(timeFormat)); }
    return parts.join(QLatin1Char(' '));
}

/******************************************************************************
 * Function Name   : TimeDisplayProperties::toJson
 * Description     : Persist a standalone clock-display record with no fabricated numeric input or RTC address.
 * Input Parameters: None
 * Return Value    : Complete editor record.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject TimeDisplayProperties::toJson() const //20261112-0 add
{
    return {{QStringLiteral("showDate"), showDate}, {QStringLiteral("showWeek"), showWeek}, {QStringLiteral("showTime"), showTime}, {QStringLiteral("dateFormat"), dateFormat}, {QStringLiteral("timeFormat"), timeFormat},
            {QStringLiteral("fontSize"), fontSize}, {QStringLiteral("alignment"), alignment}, {QStringLiteral("opacity"), opacity}, {QStringLiteral("effect3d"), effect3d}, {QStringLiteral("transparent"), transparent},
            {QStringLiteral("textColor"), textColor}, {QStringLiteral("backgroundColor"), backgroundColor}, {QStringLiteral("appearance"), appearance.toJson()}, {QStringLiteral("visibility"), visibility.toJson()}};
}

/******************************************************************************
 * Function Name   : TimeDisplayProperties::fromJson
 * Description     : Strictly parse complete clock-display fields and keep the destination intact on failure.
 * Input Parameters: json, output, error
 * Return Value    : True only for a valid complete record.
 * Date            : 2026.11.12
 ******************************************************************************/
bool TimeDisplayProperties::fromJson(const QJsonObject &json, TimeDisplayProperties *output, QString *error) //20261112-0 add
{
    TimeDisplayProperties candidate;
    QJsonObject expected = candidate.toJson();
    QJsonObject::const_iterator item;
    QJsonObject artwork;
    QString encoded;
    double value;
    if (output == nullptr || json.keys() != expected.keys()) { if (error != nullptr) { *error = QStringLiteral("时间显示字段缺失、未知或接收位置为空。"); } return false; }
    for (item = json.constBegin(); item != json.constEnd(); ++item)
    {
        if (item.value().type() != expected.value(item.key()).type()) { if (error != nullptr) { *error = QStringLiteral("时间显示字段类型错误。"); } return false; }
        if (item.value().isDouble())
        {
            value = item.value().toDouble();
            if (!std::isfinite(value) || value != std::floor(value) || value < 0 || value > 255) { if (error != nullptr) { *error = QStringLiteral("时间显示枚举或数值无效。"); } return false; }
        }
    }
    candidate.showDate = json.value(QStringLiteral("showDate")).toBool(); candidate.showWeek = json.value(QStringLiteral("showWeek")).toBool(); candidate.showTime = json.value(QStringLiteral("showTime")).toBool();
    candidate.dateFormat = json.value(QStringLiteral("dateFormat")).toInt(); candidate.timeFormat = json.value(QStringLiteral("timeFormat")).toInt(); candidate.fontSize = json.value(QStringLiteral("fontSize")).toInt(); candidate.alignment = json.value(QStringLiteral("alignment")).toInt(); candidate.opacity = json.value(QStringLiteral("opacity")).toInt();
    candidate.effect3d = json.value(QStringLiteral("effect3d")).toBool(); candidate.transparent = json.value(QStringLiteral("transparent")).toBool(); candidate.textColor = json.value(QStringLiteral("textColor")).toString(); candidate.backgroundColor = json.value(QStringLiteral("backgroundColor")).toString();
    artwork = json.value(QStringLiteral("appearance")).toObject();
    if (artwork.keys() != candidate.appearance.toJson().keys() || !artwork.value(QStringLiteral("picture")).isString() || !artwork.value(QStringLiteral("picturePng")).isString()) { if (error != nullptr) { *error = QStringLiteral("时间外形字段无效。"); } return false; }
    candidate.appearance.picture = artwork.value(QStringLiteral("picture")).toString(); encoded = artwork.value(QStringLiteral("picturePng")).toString();
    if (encoded.size() > 24 * 1024 * 1024) { if (error != nullptr) { *error = QStringLiteral("时间外形编码超限。"); } return false; }
    candidate.appearance.picturePng = QByteArray::fromBase64(encoded.toLatin1());
    if (QString::fromLatin1(candidate.appearance.picturePng.toBase64()) != encoded) { if (error != nullptr) { *error = QStringLiteral("时间外形Base64无效。"); } return false; }
    if (!StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() || !candidate.validate(error)) { return false; }
    *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : TimeDisplayProperties::validate
 * Description     : Enforce measured clock UI bounds and reuse only artwork/access validation from the numeric family.
 * Input Parameters: error
 * Return Value    : True for a valid clock editor record.
 * Date            : 2026.11.12
 ******************************************************************************/
bool TimeDisplayProperties::validate(QString *error) const //20261112-0 add
{
    NumberDisplayProperties visual;
    if (!showDate && !showWeek && !showTime) { if (error != nullptr) { *error = QStringLiteral("不能隐藏所有时间和星期"); } return false; }
    if (dateFormat < 0 || dateFormat > 8 || timeFormat < 0 || timeFormat > 3 || !fontSizes().contains(QString::number(fontSize))) { if (error != nullptr) { *error = QStringLiteral("时间格式或字号超出已核定范围。"); } return false; }
    visual.allowInput = false; visual.alignment = alignment; visual.opacity = opacity; visual.textColor = textColor; visual.backgroundColor = backgroundColor; visual.appearance = appearance; visual.visibility = visibility;
    return visual.validate(error, false);
}

/******************************************************************************
 * Function Name   : TimeDisplayProperties::validateAddresses
 * Description     : Validate only visibility carriers; the clock has no invented LW input or monitor address.
 * Input Parameters: connections, error
 * Return Value    : True for enabled visibility address spans.
 * Date            : 2026.11.12
 ******************************************************************************/
bool TimeDisplayProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261112-0 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress address;
    if (!visibility.visibilityEnabled) { return true; }
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    context.connections = connections;
    if (visibility.bitEnabled) { context.role = PlcAddressRole::Bit; context.transferCount = 1U; if (!parseQualifiedPlcAddressText(visibility.bitAddress, context, catalog, &address, error)) { return false; } }
    if (visibility.wordEnabled) { context.role = PlcAddressRole::Word; context.transferCount = visibility.dataType == 0 || visibility.dataType == 2 ? 1U : 2U; if (!parseQualifiedPlcAddressText(visibility.wordAddress, context, catalog, &address, error)) { return false; } }
    return true;
}
