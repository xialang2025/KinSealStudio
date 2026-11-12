#include "alarm_display_properties.h"
#include <QColor>
#include <cmath>

/******************************************************************************
 * Function Name   : displayShape
 * Description     : Check exact record keys and bounded integral numbers before any conversion.
 * Input Parameters: json, shape
 * Return Value    : True for an exact scalar shape.
 * Date            : 2026.11.13
 ******************************************************************************/
static bool displayShape(const QJsonObject &json, const QJsonObject &shape) //20261113-2 add
{
    QJsonObject::const_iterator field;
    double number;
    if (json.keys() != shape.keys()) { return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field) { if (field.value().type() != shape.value(field.key()).type()) { return false; } if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || number < 0 || number > 192 || std::floor(number) != number) { return false; } } }
    return true;
}

/******************************************************************************
 * Function Name   : AlarmDisplayCaptions::toJson
 * Description     : Persist three independent column captions, including captions for currently hidden columns.
 * Input Parameters: None
 * Return Value    : Complete caption JSON.
 * Date            : 2026.11.13
 ******************************************************************************/
QJsonObject AlarmDisplayCaptions::toJson() const //20261113-2 add
{
    return {{QStringLiteral("message"), message}, {QStringLiteral("date"), date}, {QStringLiteral("time"), time}};
}

/******************************************************************************
 * Function Name   : AlarmDisplayCaptions::fromJson
 * Description     : Reject unknown, non-string and oversized captions before replacing caller state.
 * Input Parameters: json, output, error
 * Return Value    : True after complete caption validation.
 * Date            : 2026.11.13
 ******************************************************************************/
bool AlarmDisplayCaptions::fromJson(const QJsonObject &json, AlarmDisplayCaptions *output, QString *error) //20261113-2 add
{
    AlarmDisplayCaptions candidate;
    if (output == nullptr || !displayShape(json, candidate.toJson())) { if (error != nullptr) { *error = QStringLiteral("报警列名字段缺失、未知或类型错误。"); } return false; }
    candidate.message = json.value(QStringLiteral("message")).toString(); candidate.date = json.value(QStringLiteral("date")).toString(); candidate.time = json.value(QStringLiteral("time")).toString();
    if (candidate.message.size() > 32767 || candidate.date.size() > 32767 || candidate.time.size() > 32767) { return false; } *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : AlarmDisplayProperties::AlarmDisplayProperties
 * Description     : Seed the observed default Chinese captions without making an editor language index persistent.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
AlarmDisplayProperties::AlarmDisplayProperties() //20261113-2 add
{
    captions.insert(QStringLiteral("中文"), AlarmDisplayCaptions());
}

/******************************************************************************
 * Function Name   : AlarmDisplayProperties::fontSizes
 * Description     : Expose all seven observed sizes rather than copying the five alarm-bar sizes.
 * Input Parameters: None
 * Return Value    : Measured font-size list.
 * Date            : 2026.11.13
 ******************************************************************************/
QList<int> AlarmDisplayProperties::fontSizes() //20261113-2 add
{
    return {12, 16, 24, 48, 64, 96, 192};
}

/******************************************************************************
 * Function Name   : AlarmDisplayProperties::captionsFor
 * Description     : Use native default Chinese captions for a newly introduced language until it is edited.
 * Input Parameters: language
 * Return Value    : Effective detached captions.
 * Date            : 2026.11.13
 ******************************************************************************/
AlarmDisplayCaptions AlarmDisplayProperties::captionsFor(const QString &language) const //20261113-2 add
{
    return captions.value(language, AlarmDisplayCaptions());
}

/******************************************************************************
 * Function Name   : AlarmDisplayProperties::setLanguages
 * Description     : Preserve surviving caption languages and initialize new languages as one bounded project transaction.
 * Input Parameters: languages
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void AlarmDisplayProperties::setLanguages(const QStringList &languages) //20261113-2 add
{
    QMap<QString, AlarmDisplayCaptions> next;
    QString language;
    Q_FOREACH (language, languages) { next.insert(language, captionsFor(language)); } captions = next;
}

/******************************************************************************
 * Function Name   : AlarmDisplayProperties::validate
 * Description     : Enforce measured row/font limits, five common colors, bounded caption maps and explicit active visibility state.
 * Input Parameters: error
 * Return Value    : True for the supported table-display editor slice.
 * Date            : 2026.11.13
 ******************************************************************************/
bool AlarmDisplayProperties::validate(QString *error) const //20261113-2 add
{
    QString color;
    QMap<QString, AlarmDisplayCaptions>::const_iterator language;
    AlarmDisplayCaptions parsed;
    if (rows < 1 || rows > 20 || !fontSizes().contains(fontSize) || captions.isEmpty() || captions.size() > 8 || (visibility.visibilityEnabled && visibility.bitEnabled && !visibilityBitChosen)) { if (error != nullptr) { *error = QStringLiteral("报警显示行数、字号、语言或显现状态无效。"); } return false; }
    Q_FOREACH (color, (QStringList{titleBackground, titleColor, frameColor, backgroundColor, textColor})) { if (color.size() > 32 || !QColor(color).isValid()) { return false; } }
    for (language = captions.constBegin(); language != captions.constEnd(); ++language) { if (language.key().trimmed().isEmpty() || language.key().size() > 128 || !AlarmDisplayCaptions::fromJson(language.value().toJson(), &parsed, error)) { return false; } }
    return visibility.validate(error);
}

/******************************************************************************
 * Function Name   : AlarmDisplayProperties::validateAddresses
 * Description     : Validate only active display conditions; alarm inputs remain in global registration definitions.
 * Input Parameters: connections, error
 * Return Value    : True for all active visibility carriers.
 * Date            : 2026.11.13
 ******************************************************************************/
bool AlarmDisplayProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261113-2 add
{
    AlarmBarProperties shared;
    shared.visibility = visibility; shared.visibilityBitChosen = visibilityBitChosen; return shared.validateAddresses(connections, error);
}

/******************************************************************************
 * Function Name   : AlarmDisplayProperties::toJson
 * Description     : Store common appearance and complete caption maps without mixing in device alarm runtime state.
 * Input Parameters: None
 * Return Value    : Complete typed display record.
 * Date            : 2026.11.13
 ******************************************************************************/
QJsonObject AlarmDisplayProperties::toJson() const //20261113-2 add
{
    QJsonObject localized;
    QMap<QString, AlarmDisplayCaptions>::const_iterator language;
    for (language = captions.constBegin(); language != captions.constEnd(); ++language) { localized.insert(language.key(), language.value().toJson()); }
    return {{QStringLiteral("titleBackground"), titleBackground}, {QStringLiteral("titleColor"), titleColor}, {QStringLiteral("frameColor"), frameColor}, {QStringLiteral("backgroundColor"), backgroundColor}, {QStringLiteral("textColor"), textColor}, {QStringLiteral("fontSize"), fontSize}, {QStringLiteral("rows"), rows}, {QStringLiteral("showDate"), showDate}, {QStringLiteral("showTime"), showTime}, {QStringLiteral("visibilityBitChosen"), visibilityBitChosen}, {QStringLiteral("captions"), localized}, {QStringLiteral("visibility"), visibility.toJson()}};
}

/******************************************************************************
 * Function Name   : AlarmDisplayProperties::fromJson
 * Description     : Reject unknown, fractional, mistyped and malformed nested records atomically.
 * Input Parameters: json, output, error
 * Return Value    : True after exact complete validation.
 * Date            : 2026.11.13
 ******************************************************************************/
bool AlarmDisplayProperties::fromJson(const QJsonObject &json, AlarmDisplayProperties *output, QString *error) //20261113-2 add
{
    AlarmDisplayProperties candidate;
    QJsonObject localized;
    QJsonObject::const_iterator language;
    AlarmDisplayCaptions captions;
    if (output == nullptr || !displayShape(json, candidate.toJson())) { if (error != nullptr) { *error = QStringLiteral("报警显示字段缺失、未知或类型错误。"); } return false; }
    candidate.titleBackground = json.value(QStringLiteral("titleBackground")).toString();
    candidate.titleColor = json.value(QStringLiteral("titleColor")).toString();
    candidate.frameColor = json.value(QStringLiteral("frameColor")).toString();
    candidate.backgroundColor = json.value(QStringLiteral("backgroundColor")).toString();
    candidate.textColor = json.value(QStringLiteral("textColor")).toString();
    candidate.fontSize = json.value(QStringLiteral("fontSize")).toInt();
    candidate.rows = json.value(QStringLiteral("rows")).toInt();
    candidate.showDate = json.value(QStringLiteral("showDate")).toBool();
    candidate.showTime = json.value(QStringLiteral("showTime")).toBool();
    candidate.visibilityBitChosen = json.value(QStringLiteral("visibilityBitChosen")).toBool();
    localized = json.value(QStringLiteral("captions")).toObject(); if (localized.size() > 8) { return false; } candidate.captions.clear(); for (language = localized.constBegin(); language != localized.constEnd(); ++language) { if (!language.value().isObject() || !AlarmDisplayCaptions::fromJson(language.value().toObject(), &captions, error)) { return false; } candidate.captions.insert(language.key(), captions); }
    if (!StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() || !candidate.validate(error)) { return false; } *output = candidate; return true;
}

