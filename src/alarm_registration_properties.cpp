#include "alarm_registration_properties.h"
#include "project_model.h"
#include <QJsonArray>
#include <QSet>
#include <cmath>

/******************************************************************************
 * Function Name   : alarmError
 * Description     : Return a useful domain error without publishing a partially parsed alarm draft.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.13
 ******************************************************************************/
static bool alarmError(QString *error, const QString &message) //20261113-2 add
{
    if (error != nullptr) { *error = message; } return false;
}

/******************************************************************************
 * Function Name   : alarmShape
 * Description     : Require exact keys and integral numeric fields before converting untrusted JSON.
 * Input Parameters: json, shape
 * Return Value    : True for an exact record shape.
 * Date            : 2026.11.13
 ******************************************************************************/
static bool alarmShape(const QJsonObject &json, const QJsonObject &shape) //20261113-2 add
{
    QJsonObject::const_iterator field;
    double number;
    if (json.keys() != shape.keys()) { return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != shape.value(field.key()).type()) { return false; }
        if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || number < 0 || number > 3600 || std::floor(number) != number) { return false; } }
    }
    return true;
}

/******************************************************************************
 * Function Name   : AlarmRegistrationRow::toJson
 * Description     : Preserve independent multilingual messages and stable optional navigation drafts.
 * Input Parameters: None
 * Return Value    : Complete row JSON.
 * Date            : 2026.11.13
 ******************************************************************************/
QJsonObject AlarmRegistrationRow::toJson() const //20261113-2 add
{
    QJsonObject messagesJson;
    QMap<QString, QString>::const_iterator message;
    for (message = messages.constBegin(); message != messages.constEnd(); ++message) { messagesJson.insert(message.key(), message.value()); }
    return {{QStringLiteral("used"), used}, {QStringLiteral("messages"), messagesJson}, {QStringLiteral("limit"), limit}, {QStringLiteral("jumpPageId"), jumpPageId}, {QStringLiteral("backPageId"), backPageId}};
}

/******************************************************************************
 * Function Name   : AlarmRegistrationRow::fromJson
 * Description     : Reject malformed or oversized message and navigation records atomically.
 * Input Parameters: json, output, error
 * Return Value    : True after complete row validation.
 * Date            : 2026.11.13
 ******************************************************************************/
bool AlarmRegistrationRow::fromJson(const QJsonObject &json, AlarmRegistrationRow *output, QString *error) //20261113-2 add
{
    AlarmRegistrationRow candidate;
    QJsonObject messagesJson;
    QJsonObject::const_iterator message;
    if (output == nullptr || !alarmShape(json, candidate.toJson())) { return alarmError(error, QStringLiteral("报警条目字段缺失、未知或类型错误。")); }
    candidate.used = json.value(QStringLiteral("used")).toBool(); candidate.limit = json.value(QStringLiteral("limit")).toString(); candidate.jumpPageId = json.value(QStringLiteral("jumpPageId")).toString(); candidate.backPageId = json.value(QStringLiteral("backPageId")).toString(); messagesJson = json.value(QStringLiteral("messages")).toObject();
    if (messagesJson.size() > 8 || candidate.limit.size() > 80 || candidate.jumpPageId.size() > 128 || candidate.backPageId.size() > 128) { return false; }
    for (message = messagesJson.constBegin(); message != messagesJson.constEnd(); ++message) { if (message.key().trimmed().isEmpty() || message.key().size() > 128 || !message.value().isString() || message.value().toString().size() > 32767) { return alarmError(error, QStringLiteral("报警消息超出编辑范围。")); } candidate.messages.insert(message.key(), message.value().toString()); }
    *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : AlarmRegistrationProperties::dataTypes
 * Description     : Preserve the measured unsigned-first alarm order, independent of trend type indices.
 * Input Parameters: None
 * Return Value    : Seven observed labels.
 * Date            : 2026.11.13
 ******************************************************************************/
QStringList AlarmRegistrationProperties::dataTypes() //20261113-2 add
{
    return {QStringLiteral("16位正整数"), QStringLiteral("32位正整数"), QStringLiteral("16位整数"), QStringLiteral("32位整数"), QStringLiteral("16位BCD码"), QStringLiteral("32位BCD码"), QStringLiteral("32位浮点数")};
}

/******************************************************************************
 * Function Name   : AlarmRegistrationProperties::rowCount
 * Description     : Map one condition register to four levels while every bit-mode count means rows.
 * Input Parameters: None
 * Return Value    : Expected table rows.
 * Date            : 2026.11.13
 ******************************************************************************/
int AlarmRegistrationProperties::rowCount() const //20261113-2 add
{
    return count * (family == 1 ? 4 : 1);
}

/******************************************************************************
 * Function Name   : AlarmRegistrationProperties::readSpan
 * Description     : Protect the complete active registration input buffer without claiming firmware sampling order.
 * Input Parameters: None
 * Return Value    : Required active bit or word span.
 * Date            : 2026.11.13
 ******************************************************************************/
u32 AlarmRegistrationProperties::readSpan() const //20261113-2 add
{
    if (family == 1) { return u32(count) * (dataType == 0 || dataType == 2 || dataType == 4 ? 1U : 2U); }
    return bitType == 0 ? u32(count) : bitType == 1 ? (u32(count) + 15U) / 16U : 1U;
}

/******************************************************************************
 * Function Name   : AlarmRegistrationProperties::resizeRows
 * Description     : Retain surviving messages, remove truncated rows and seed new rows as used in every language.
 * Input Parameters: value, languages
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void AlarmRegistrationProperties::resizeRows(int value, const QStringList &languages) //20261113-2 add
{
    AlarmRegistrationRow row;
    QString language;
    if (value < 0 || value > (family == 1 ? 31 : 511)) { return; }
    count = value;
    while (rows.size() > rowCount()) { rows.removeLast(); }
    while (rows.size() < rowCount()) { row = AlarmRegistrationRow(); Q_FOREACH (language, languages) { row.messages.insert(language, QString()); } rows.append(row); }

}

/******************************************************************************
 * Function Name   : AlarmRegistrationProperties::validate
 * Description     : Validate exact table shape and active numeric thresholds with bounded multilingual message storage.
 * Input Parameters: error
 * Return Value    : True for the supported registration editor slice.
 * Date            : 2026.11.13
 ******************************************************************************/
bool AlarmRegistrationProperties::validate(QString *error) const //20261113-2 add
{
    AlarmRegistrationRow row;
    AlarmRegistrationRow parsed;
    bool ok;
    double limit;
    int units = 0;
    QString message;
    if (family < 0 || family > 1 || bitType < 0 || bitType > 2 || count < 0 || count > (family == 1 ? 31 : 511) || scanSeconds < 1 || scanSeconds > 3600 || dataType < 0 || dataType > 6 || readAddress.size() > 1024 || rows.size() != rowCount()) { return alarmError(error, QStringLiteral("报警类型、条数、扫描时间或表格不完整。")); }
    Q_FOREACH (row, rows)
    {
        if (!AlarmRegistrationRow::fromJson(row.toJson(), &parsed, error)) { return false; }
        Q_FOREACH (message, row.messages) { units += message.size(); }
        if (units > 4 * 1024 * 1024) { return alarmError(error, QStringLiteral("报警消息总量超过Lite编辑保护上限。")); }
        if (family == 1 && row.used) { limit = row.limit.toDouble(&ok); if (!ok || !std::isfinite(limit) || std::abs(limit) > 3.402823466e38) { return alarmError(error, QStringLiteral("活动条件报警阈值必须是有限数值。")); } }
    }
    return true;
}

/******************************************************************************
 * Function Name   : AlarmRegistrationProperties::validateContext
 * Description     : Resolve only active address spans and used navigation targets against the complete project.
 * Input Parameters: project, error
 * Return Value    : True for active carriers and dependencies.
 * Date            : 2026.11.13
 ******************************************************************************/
bool AlarmRegistrationProperties::validateContext(const HmiProject &project, QString *error) const //20261113-2 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress parsed;
    AlarmRegistrationRow row;
    HmiPage page;
    QSet<QString> pages;
    QString id;
    if (!validate(error)) { return false; }
    if (count > 0)
    {
        if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
        context.connections = project.connections; context.role = family == 0 && bitType == 0 ? PlcAddressRole::Bit : PlcAddressRole::Word; context.transferCount = readSpan();
        if (!parseQualifiedPlcAddressText(readAddress, context, catalog, &parsed, error)) { return false; }
    }
    Q_FOREACH (page, project.allPages()) { if (page.kind == QStringLiteral("normal")) { pages.insert(page.id); } }
    if (family == 0) { Q_FOREACH (row, rows) { if (!row.used) { continue; } Q_FOREACH (id, (QStringList{row.jumpPageId, row.backPageId})) { if (!id.isEmpty() && !pages.contains(id)) { return alarmError(error, QStringLiteral("报警跳转或跳回画面已不存在。")); } } } }
    return true;
}

/******************************************************************************
 * Function Name   : AlarmRegistrationProperties::toJson
 * Description     : Store explicit registration fields and ordered rows separately from configuration-node state.
 * Input Parameters: None
 * Return Value    : Complete registration JSON.
 * Date            : 2026.11.13
 ******************************************************************************/
QJsonObject AlarmRegistrationProperties::toJson() const //20261113-2 add
{
    QJsonArray entries;
    AlarmRegistrationRow row;
    Q_FOREACH (row, rows) { entries.append(row.toJson()); }
    return {{QStringLiteral("family"), family}, {QStringLiteral("bitType"), bitType}, {QStringLiteral("count"), count}, {QStringLiteral("scanSeconds"), scanSeconds}, {QStringLiteral("dataType"), dataType}, {QStringLiteral("readAddress"), readAddress}, {QStringLiteral("rows"), entries}};
}

/******************************************************************************
 * Function Name   : AlarmRegistrationProperties::fromJson
 * Description     : Reject missing, unknown, fractional and mismatched table fields before replacing caller state.
 * Input Parameters: json, output, error
 * Return Value    : True for an exact registration record.
 * Date            : 2026.11.13
 ******************************************************************************/
bool AlarmRegistrationProperties::fromJson(const QJsonObject &json, AlarmRegistrationProperties *output, QString *error) //20261113-2 add
{
    AlarmRegistrationProperties candidate;
    AlarmRegistrationRow row;
    QJsonArray entries;
    QJsonValue entry;
    if (output == nullptr || !alarmShape(json, candidate.toJson())) { return alarmError(error, QStringLiteral("报警登录字段不完整或类型错误。")); }
    candidate.family = json.value(QStringLiteral("family")).toInt(); candidate.bitType = json.value(QStringLiteral("bitType")).toInt(); candidate.count = json.value(QStringLiteral("count")).toInt(); candidate.scanSeconds = json.value(QStringLiteral("scanSeconds")).toInt(); candidate.dataType = json.value(QStringLiteral("dataType")).toInt(); candidate.readAddress = json.value(QStringLiteral("readAddress")).toString(); entries = json.value(QStringLiteral("rows")).toArray();
    if (entries.size() > 511) { return false; }
    Q_FOREACH (entry, entries) { if (!entry.isObject() || !AlarmRegistrationRow::fromJson(entry.toObject(), &row, error)) { return false; } candidate.rows.append(row); }
    if (!candidate.validate(error)) { return false; } *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : hasAlarmRegistrations
 * Description     : Apply the observed nonempty registration gate independently of used-row runtime state.
 * Input Parameters: project
 * Return Value    : True when an implemented registration contains rows.
 * Date            : 2026.11.13
 ******************************************************************************/
bool hasAlarmRegistrations(const HmiProject &project) //20261113-2 add
{
    HmiConfigurationNode node;
    AlarmRegistrationProperties properties;
    Q_FOREACH (node, project.configurationNodes) { if (node.kind == QStringLiteral("alarm") && node.active && node.implemented && AlarmRegistrationProperties::fromJson(node.properties, &properties) && properties.count > 0) { return true; } }
    return false;
}

/******************************************************************************
 * Function Name   : validateAlarmRegistrations
 * Description     : Protect global registration uniqueness, all-page alarm consumers and active page references.
 * Input Parameters: project, error
 * Return Value    : True after dependency validation.
 * Date            : 2026.11.13
 ******************************************************************************/
bool validateAlarmRegistrations(const HmiProject &project, QString *error) //20261113-2 add
{
    HmiConfigurationNode node;
    AlarmRegistrationProperties properties;
    QSet<int> families;
    HmiPage page;
    HmiObject object;
    Q_FOREACH (node, project.configurationNodes)
    {
        if (node.kind != QStringLiteral("alarm") || !node.active || !node.implemented) { continue; }
        if (!AlarmRegistrationProperties::fromJson(node.properties, &properties, error) || !properties.validateContext(project, error)) { return false; }
        if (families.contains(properties.family)) { return alarmError(error, QStringLiteral("同类报警登录只能配置一份。")); } families.insert(properties.family);
    }
    Q_FOREACH (page, project.allPages()) { Q_FOREACH (object, page.objects) { if (object.type == QStringLiteral("alarmbar") && !hasAlarmRegistrations(project)) { return alarmError(error, QStringLiteral("位报警和条件报警都不存在")); } } }
    return true;
}

