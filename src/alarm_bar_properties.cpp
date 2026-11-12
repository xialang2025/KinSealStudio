#include "alarm_bar_properties.h"
#include <QColor>
#include <cmath>

/******************************************************************************
 * Function Name   : AlarmBarProperties::toJson
 * Description     : Keep fill, text, direction, ordering and visibility drafts independent.
 * Input Parameters: None
 * Return Value    : Complete alarm-bar JSON.
 * Date            : 2026.11.13
 ******************************************************************************/
QJsonObject AlarmBarProperties::toJson() const //20261113-2 add
{
    return {{QStringLiteral("frameColor"), frameColor}, {QStringLiteral("foregroundColor"), foregroundColor}, {QStringLiteral("backgroundColor"), backgroundColor}, {QStringLiteral("textColor"), textColor}, {QStringLiteral("pattern"), pattern}, {QStringLiteral("speed"), speed}, {QStringLiteral("direction"), direction}, {QStringLiteral("sortOrder"), sortOrder}, {QStringLiteral("fontSize"), fontSize}, {QStringLiteral("visibilityBitChosen"), visibilityBitChosen}, {QStringLiteral("visibility"), visibility.toJson()}};
}

/******************************************************************************
 * Function Name   : AlarmBarProperties::validate
 * Description     : Enforce all measured ranges and a deliberately selected active visibility bit state.
 * Input Parameters: error
 * Return Value    : True for a bounded display-only draft.
 * Date            : 2026.11.13
 ******************************************************************************/
bool AlarmBarProperties::validate(QString *error) const //20261113-2 add
{
    QString color;
    if (pattern < 0 || pattern > 12 || speed < 0 || speed > 30 || direction < 0 || direction > 1 || sortOrder < 0 || sortOrder > 1 || !QList<int>({12, 16, 24, 48, 64}).contains(fontSize) || (visibility.visibilityEnabled && visibility.bitEnabled && !visibilityBitChosen)) { if (error != nullptr) { *error = QStringLiteral("动态报警条样式、速度、字号或显现状态无效。"); } return false; }
    Q_FOREACH (color, (QStringList{frameColor, foregroundColor, backgroundColor, textColor})) { if (color.size() > 32 || !QColor(color).isValid()) { return false; } }
    return visibility.validate(error);
}

/******************************************************************************
 * Function Name   : AlarmBarProperties::validateAddresses
 * Description     : Validate only active display visibility carriers, including complete scalar word widths.
 * Input Parameters: connections, error
 * Return Value    : True for all active visibility addresses.
 * Date            : 2026.11.13
 ******************************************************************************/
bool AlarmBarProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261113-2 add
{
    ScreenButtonProperties shared;
    shared.common.indicator = false; shared.common.visibility = visibility; return shared.validateAddresses(connections, error);
}

/******************************************************************************
 * Function Name   : AlarmBarProperties::fromJson
 * Description     : Require exact fields and lossless types before publishing a typed alarm-bar draft.
 * Input Parameters: json, output, error
 * Return Value    : True after complete validation.
 * Date            : 2026.11.13
 ******************************************************************************/
bool AlarmBarProperties::fromJson(const QJsonObject &json, AlarmBarProperties *output, QString *error) //20261113-2 add
{
    AlarmBarProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator field;
    double value;
    if (output == nullptr || json.keys() != shape.keys()) { return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field) { if (field.value().type() != shape.value(field.key()).type()) { return false; } if (field.value().isDouble()) { value = field.value().toDouble(); if (!std::isfinite(value) || value < 0 || value > 64 || std::floor(value) != value) { return false; } } }
    candidate.frameColor = json.value(QStringLiteral("frameColor")).toString();
    candidate.foregroundColor = json.value(QStringLiteral("foregroundColor")).toString();
    candidate.backgroundColor = json.value(QStringLiteral("backgroundColor")).toString();
    candidate.textColor = json.value(QStringLiteral("textColor")).toString();
    candidate.pattern = json.value(QStringLiteral("pattern")).toInt();
    candidate.speed = json.value(QStringLiteral("speed")).toInt();
    candidate.direction = json.value(QStringLiteral("direction")).toInt();
    candidate.sortOrder = json.value(QStringLiteral("sortOrder")).toInt();
    candidate.fontSize = json.value(QStringLiteral("fontSize")).toInt();
    candidate.visibilityBitChosen = json.value(QStringLiteral("visibilityBitChosen")).toBool();
    if (!StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() || !candidate.validate(error)) { return false; } *output = candidate; return true;
}

