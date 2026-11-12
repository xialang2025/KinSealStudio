#include "scale_properties.h"
#include <cmath>
#include <limits>

/******************************************************************************
 * Function Name   : ScaleProperties::toJson
 * Description     : Persist the scale's independent appearance and numeric fields without inventing device records.
 * Input Parameters: None
 * Return Value    : Typed Lite scale JSON.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject ScaleProperties::toJson() const //20261112-0 add
{
    return {{QStringLiteral("fontSize"), fontSize}, {QStringLiteral("direction"), direction},
            {QStringLiteral("position"), position}, {QStringLiteral("minimum"), minimum}, {QStringLiteral("maximum"), maximum},
            {QStringLiteral("decimals"), decimals}, {QStringLiteral("textColor"), textColor.name(QColor::HexArgb)},
            {QStringLiteral("lineColor"), lineColor.name(QColor::HexArgb)}, {QStringLiteral("showValues"), showValues},
            {QStringLiteral("majorCount"), majorCount}, {QStringLiteral("minorCount"), minorCount}};
}

/******************************************************************************
 * Function Name   : ScaleProperties::fromJson
 * Description     : Reject unknown or lossy fields and restore a complete scale only after validation.
 * Input Parameters: json, output, error
 * Return Value    : True on a complete typed replacement.
 * Date            : 2026.11.12
 ******************************************************************************/
bool ScaleProperties::fromJson(const QJsonObject &json, ScaleProperties *output, QString *error) //20261112-0 add
{
    ScaleProperties candidate;
    QJsonObject expected = candidate.toJson();
    QJsonObject::const_iterator item;
    double number;
    if (output == nullptr || json.keys() != expected.keys())
    { if (error != nullptr) { *error = QStringLiteral("刻度字段缺失、未知或接收位置为空。"); } return false; }
    for (item = json.constBegin(); item != json.constEnd(); ++item)
    {
        if (item.value().type() != expected.value(item.key()).type())
        { if (error != nullptr) { *error = QStringLiteral("刻度字段类型错误：%1").arg(item.key()); } return false; }
        if (item.value().isDouble())
        {
            number = item.value().toDouble();
            if (!std::isfinite(number) || std::floor(number) != number || number < std::numeric_limits<int>::min() || number > std::numeric_limits<int>::max())
            { if (error != nullptr) { *error = QStringLiteral("刻度数值必须为32位整数。"); } return false; }
        }
    }
    candidate.fontSize = json.value(QStringLiteral("fontSize")).toInt(); candidate.direction = json.value(QStringLiteral("direction")).toInt();
    candidate.position = json.value(QStringLiteral("position")).toInt(); candidate.minimum = json.value(QStringLiteral("minimum")).toInt(); candidate.maximum = json.value(QStringLiteral("maximum")).toInt();
    candidate.decimals = json.value(QStringLiteral("decimals")).toInt(); candidate.textColor = QColor(json.value(QStringLiteral("textColor")).toString()); candidate.lineColor = QColor(json.value(QStringLiteral("lineColor")).toString());
    candidate.showValues = json.value(QStringLiteral("showValues")).toBool(); candidate.majorCount = json.value(QStringLiteral("majorCount")).toInt(); candidate.minorCount = json.value(QStringLiteral("minorCount")).toInt();
    if (!candidate.validate(error)) { return false; }
    *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : ScaleProperties::validate
 * Description     : Apply freshly observed native bounds and allow equal endpoints without a division by zero.
 * Input Parameters: error
 * Return Value    : True for the supported static-scale editing fields.
 * Date            : 2026.11.12
 ******************************************************************************/
bool ScaleProperties::validate(QString *error) const //20261112-0 add
{
    if (minimum > maximum)
    { if (error != nullptr) { *error = QStringLiteral("最小值不能大于最大值"); } return false; }
    if (fontSize < 5 || fontSize > 20 || direction < 0 || direction > 1 || position < 0 || position > 3 ||
        decimals < 0 || decimals > 3 || majorCount < 2 || majorCount > 20 || minorCount < 0 || minorCount > 5 || !textColor.isValid() || !lineColor.isValid())
    { if (error != nullptr) { *error = QStringLiteral("刻度属性超出已核定范围。"); } return false; }
    return true;
}
