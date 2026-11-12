"""Implement typed C27 alarm registration and a separate display-only alarm-bar draft."""
from pathlib import Path
root=Path(__file__).resolve().parent/'worktree'
def put(path,text): (root/path).write_text(text,encoding='utf-8',newline='\n')
def f(name,desc,sig,body,inputs='None',returns='None.'):
    return f'/******************************************************************************\n * Function Name   : {name}\n * Description     : {desc}\n * Input Parameters: {inputs}\n * Return Value    : {returns}\n * Date            : 2026.11.13\n ******************************************************************************/\n{sig} //20261113-2 add\n{{\n{body}\n}}\n\n'
put('src/alarm_registration_properties.h','''#ifndef ALARM_REGISTRATION_PROPERTIES_H
#define ALARM_REGISTRATION_PROPERTIES_H
#include "plc_address.h"
#include <QMap>
struct HmiProject;
//20261113-2 add: Stable editor rows have no claim to recovered firmware alarm record identities.
struct AlarmRegistrationRow
{
    bool used = true;
    QMap<QString, QString> messages;
    QString limit = QStringLiteral("0");
    QString jumpPageId;
    QString backPageId;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, AlarmRegistrationRow *output, QString *error = nullptr);
};
struct AlarmRegistrationProperties
{
    int family = 0;
    int bitType = 0;
    int count = 0;
    int scanSeconds = 1;
    int dataType = 0;
    QString readAddress = QStringLiteral("LB0");
    QVector<AlarmRegistrationRow> rows;
    static QStringList dataTypes();
    int rowCount() const;
    u32 readSpan() const;
    void resizeRows(int value, const QStringList &languages);
    bool validate(QString *error = nullptr) const;
    bool validateContext(const HmiProject &project, QString *error = nullptr) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, AlarmRegistrationProperties *output, QString *error = nullptr);
};
bool hasAlarmRegistrations(const HmiProject &project);
bool validateAlarmRegistrations(const HmiProject &project, QString *error = nullptr);
#endif
''')
s='''#include "alarm_registration_properties.h"
#include "project_model.h"
#include <QJsonArray>
#include <QSet>
#include <cmath>

'''
s+=f('alarmError','Return a useful domain error without publishing a partially parsed alarm draft.','static bool alarmError(QString *error, const QString &message)','    if (error != nullptr) { *error = message; } return false;','error, message','False.')
s+=f('alarmShape','Require exact keys and integral numeric fields before converting untrusted JSON.','static bool alarmShape(const QJsonObject &json, const QJsonObject &shape)','''    QJsonObject::const_iterator field;
    double number;
    if (json.keys() != shape.keys()) { return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != shape.value(field.key()).type()) { return false; }
        if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || number < 0 || number > 3600 || std::floor(number) != number) { return false; } }
    }
    return true;''','json, shape','True for an exact record shape.')
s+=f('AlarmRegistrationRow::toJson','Preserve independent multilingual messages and stable optional navigation drafts.','QJsonObject AlarmRegistrationRow::toJson() const','''    QJsonObject messagesJson;
    QMap<QString, QString>::const_iterator message;
    for (message = messages.constBegin(); message != messages.constEnd(); ++message) { messagesJson.insert(message.key(), message.value()); }
    return {{QStringLiteral("used"), used}, {QStringLiteral("messages"), messagesJson}, {QStringLiteral("limit"), limit}, {QStringLiteral("jumpPageId"), jumpPageId}, {QStringLiteral("backPageId"), backPageId}};''',returns='Complete row JSON.')
s+=f('AlarmRegistrationRow::fromJson','Reject malformed or oversized message and navigation records atomically.','bool AlarmRegistrationRow::fromJson(const QJsonObject &json, AlarmRegistrationRow *output, QString *error)','''    AlarmRegistrationRow candidate;
    QJsonObject messagesJson;
    QJsonObject::const_iterator message;
    if (output == nullptr || !alarmShape(json, candidate.toJson())) { return alarmError(error, QStringLiteral("报警条目字段缺失、未知或类型错误。")); }
    candidate.used = json.value(QStringLiteral("used")).toBool(); candidate.limit = json.value(QStringLiteral("limit")).toString(); candidate.jumpPageId = json.value(QStringLiteral("jumpPageId")).toString(); candidate.backPageId = json.value(QStringLiteral("backPageId")).toString(); messagesJson = json.value(QStringLiteral("messages")).toObject();
    if (messagesJson.size() > 8 || candidate.limit.size() > 80 || candidate.jumpPageId.size() > 128 || candidate.backPageId.size() > 128) { return false; }
    for (message = messagesJson.constBegin(); message != messagesJson.constEnd(); ++message) { if (message.key().trimmed().isEmpty() || message.key().size() > 128 || !message.value().isString() || message.value().toString().size() > 32767) { return alarmError(error, QStringLiteral("报警消息超出编辑范围。")); } candidate.messages.insert(message.key(), message.value().toString()); }
    *output = candidate; return true;''','json, output, error','True after complete row validation.')
s+=f('AlarmRegistrationProperties::dataTypes','Preserve the measured unsigned-first alarm order, independent of trend type indices.','QStringList AlarmRegistrationProperties::dataTypes()','    return {QStringLiteral("16位正整数"), QStringLiteral("32位正整数"), QStringLiteral("16位整数"), QStringLiteral("32位整数"), QStringLiteral("16位BCD码"), QStringLiteral("32位BCD码"), QStringLiteral("32位浮点数")};',returns='Seven observed labels.')
s+=f('AlarmRegistrationProperties::rowCount','Map one condition register to four levels while every bit-mode count means rows.','int AlarmRegistrationProperties::rowCount() const','    return count * (family == 1 ? 4 : 1);',returns='Expected table rows.')
s+=f('AlarmRegistrationProperties::readSpan','Protect the complete active registration input buffer without claiming firmware sampling order.','u32 AlarmRegistrationProperties::readSpan() const','''    if (family == 1) { return u32(count) * (dataType == 0 || dataType == 2 || dataType == 4 ? 1U : 2U); }
    return bitType == 0 ? u32(count) : bitType == 1 ? (u32(count) + 15U) / 16U : 1U;''',returns='Required active bit or word span.')
s+=f('AlarmRegistrationProperties::resizeRows','Retain surviving messages, remove truncated rows and seed new rows as used in every language.','void AlarmRegistrationProperties::resizeRows(int value, const QStringList &languages)','''    AlarmRegistrationRow row;
    QString language;
    if (value < 0 || value > (family == 1 ? 31 : 511)) { return; }
    count = value;
    while (rows.size() > rowCount()) { rows.removeLast(); }
    while (rows.size() < rowCount()) { row = AlarmRegistrationRow(); Q_FOREACH (language, languages) { row.messages.insert(language, QString()); } rows.append(row); }
''','value, languages')
s+=f('AlarmRegistrationProperties::validate','Validate exact table shape and active numeric thresholds with bounded multilingual message storage.','bool AlarmRegistrationProperties::validate(QString *error) const','''    AlarmRegistrationRow row;
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
    return true;''','error','True for the supported registration editor slice.')
s+=f('AlarmRegistrationProperties::validateContext','Resolve only active address spans and used navigation targets against the complete project.','bool AlarmRegistrationProperties::validateContext(const HmiProject &project, QString *error) const','''    PlcCatalog catalog;
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
    return true;''','project, error','True for active carriers and dependencies.')
s+=f('AlarmRegistrationProperties::toJson','Store explicit registration fields and ordered rows separately from configuration-node state.','QJsonObject AlarmRegistrationProperties::toJson() const','''    QJsonArray entries;
    AlarmRegistrationRow row;
    Q_FOREACH (row, rows) { entries.append(row.toJson()); }
    return {{QStringLiteral("family"), family}, {QStringLiteral("bitType"), bitType}, {QStringLiteral("count"), count}, {QStringLiteral("scanSeconds"), scanSeconds}, {QStringLiteral("dataType"), dataType}, {QStringLiteral("readAddress"), readAddress}, {QStringLiteral("rows"), entries}};''',returns='Complete registration JSON.')
s+=f('AlarmRegistrationProperties::fromJson','Reject missing, unknown, fractional and mismatched table fields before replacing caller state.','bool AlarmRegistrationProperties::fromJson(const QJsonObject &json, AlarmRegistrationProperties *output, QString *error)','''    AlarmRegistrationProperties candidate;
    AlarmRegistrationRow row;
    QJsonArray entries;
    QJsonValue entry;
    if (output == nullptr || !alarmShape(json, candidate.toJson())) { return alarmError(error, QStringLiteral("报警登录字段不完整或类型错误。")); }
    candidate.family = json.value(QStringLiteral("family")).toInt(); candidate.bitType = json.value(QStringLiteral("bitType")).toInt(); candidate.count = json.value(QStringLiteral("count")).toInt(); candidate.scanSeconds = json.value(QStringLiteral("scanSeconds")).toInt(); candidate.dataType = json.value(QStringLiteral("dataType")).toInt(); candidate.readAddress = json.value(QStringLiteral("readAddress")).toString(); entries = json.value(QStringLiteral("rows")).toArray();
    if (entries.size() > 511) { return false; }
    Q_FOREACH (entry, entries) { if (!entry.isObject() || !AlarmRegistrationRow::fromJson(entry.toObject(), &row, error)) { return false; } candidate.rows.append(row); }
    if (!candidate.validate(error)) { return false; } *output = candidate; return true;''','json, output, error','True for an exact registration record.')
s+=f('hasAlarmRegistrations','Apply the observed nonempty registration gate independently of used-row runtime state.','bool hasAlarmRegistrations(const HmiProject &project)','''    HmiConfigurationNode node;
    AlarmRegistrationProperties properties;
    Q_FOREACH (node, project.configurationNodes) { if (node.kind == QStringLiteral("alarm") && node.active && node.implemented && AlarmRegistrationProperties::fromJson(node.properties, &properties) && properties.count > 0) { return true; } }
    return false;''','project','True when an implemented registration contains rows.')
s+=f('validateAlarmRegistrations','Protect global registration uniqueness, all-page alarm consumers and active page references.','bool validateAlarmRegistrations(const HmiProject &project, QString *error)','''    HmiConfigurationNode node;
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
    return true;''','project, error','True after dependency validation.')
put('src/alarm_registration_properties.cpp',s)
fields=[('QString','frameColor','QStringLiteral("#000000")'),('QString','foregroundColor','QStringLiteral("#000000")'),('QString','backgroundColor','QStringLiteral("#c0c0c0")'),('QString','textColor','QStringLiteral("#000000")'),('int','pattern','1'),('int','speed','3'),('int','direction','0'),('int','sortOrder','0'),('int','fontSize','16'),('bool','visibilityBitChosen','false')]
put('src/alarm_bar_properties.h','''#ifndef ALARM_BAR_PROPERTIES_H
#define ALARM_BAR_PROPERTIES_H
#include "screen_button_properties.h"
//20261113-2 add: Alarm text comes from global registrations; this display owns no operation address.
struct AlarmBarProperties
{
'''+''.join(f'    {t} {n} = {v};\n' for t,n,v in fields)+'''    StaticTextProperties visibility;
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, AlarmBarProperties *output, QString *error = nullptr);
};
#endif
''')
s='#include "alarm_bar_properties.h"\n#include <QColor>\n#include <cmath>\n\n'
s+=f('AlarmBarProperties::toJson','Keep fill, text, direction, ordering and visibility drafts independent.','QJsonObject AlarmBarProperties::toJson() const','    return {'+', '.join('{QStringLiteral("'+n+'"), '+n+'}' for t,n,v in fields)+', {QStringLiteral("visibility"), visibility.toJson()}};',returns='Complete alarm-bar JSON.')
s+=f('AlarmBarProperties::validate','Enforce all measured ranges and a deliberately selected active visibility bit state.','bool AlarmBarProperties::validate(QString *error) const','''    QString color;
    if (pattern < 0 || pattern > 12 || speed < 0 || speed > 30 || direction < 0 || direction > 1 || sortOrder < 0 || sortOrder > 1 || !QList<int>({12, 16, 24, 48, 64}).contains(fontSize) || (visibility.visibilityEnabled && visibility.bitEnabled && !visibilityBitChosen)) { if (error != nullptr) { *error = QStringLiteral("动态报警条样式、速度、字号或显现状态无效。"); } return false; }
    Q_FOREACH (color, (QStringList{frameColor, foregroundColor, backgroundColor, textColor})) { if (color.size() > 32 || !QColor(color).isValid()) { return false; } }
    return visibility.validate(error);''','error','True for a bounded display-only draft.')
s+=f('AlarmBarProperties::validateAddresses','Validate only active display visibility carriers, including complete scalar word widths.','bool AlarmBarProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const','    ScreenButtonProperties shared;\n    shared.common.indicator = false; shared.common.visibility = visibility; return shared.validateAddresses(connections, error);','connections, error','True for all active visibility addresses.')
s+=f('AlarmBarProperties::fromJson','Require exact fields and lossless types before publishing a typed alarm-bar draft.','bool AlarmBarProperties::fromJson(const QJsonObject &json, AlarmBarProperties *output, QString *error)','''    AlarmBarProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator field;
    double value;
    if (output == nullptr || json.keys() != shape.keys()) { return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field) { if (field.value().type() != shape.value(field.key()).type()) { return false; } if (field.value().isDouble()) { value = field.value().toDouble(); if (!std::isfinite(value) || value < 0 || value > 64 || std::floor(value) != value) { return false; } } }
'''+''.join(f'    candidate.{n} = json.value(QStringLiteral("{n}")).'+{'QString':'toString','int':'toInt','bool':'toBool'}[t]+'();\n' for t,n,v in fields)+'''    if (!StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() || !candidate.validate(error)) { return false; } *output = candidate; return true;''','json, output, error','True after complete validation.')
put('src/alarm_bar_properties.cpp',s)
print('Created strict alarm definitions and independent display properties.')
