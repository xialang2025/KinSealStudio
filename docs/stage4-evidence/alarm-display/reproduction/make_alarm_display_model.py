"""Implement measured common styling and language-specific captions for C28."""
from pathlib import Path
import ast
root=Path(__file__).resolve().parent/'worktree'
tree=ast.parse(Path(__file__).with_name('make_alarm_models.py').read_text(encoding='utf-8'));ns={};exec(compile(ast.Module(body=[n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name=='f'],type_ignores=[]),'<documented-function>','exec'),ns);f=ns['f']
def put(path,text):(root/path).write_text(text,encoding='utf-8',newline='\n')
fields=[('QString','titleBackground','QStringLiteral("#ffff00")'),('QString','titleColor','QStringLiteral("#000000")'),('QString','frameColor','QStringLiteral("#000000")'),('QString','backgroundColor','QStringLiteral("#e0e0e0")'),('QString','textColor','QStringLiteral("#000000")'),('int','fontSize','16'),('int','rows','5'),('bool','showDate','true'),('bool','showTime','true'),('bool','visibilityBitChosen','false')]
put('src/alarm_display_properties.h','''#ifndef ALARM_DISPLAY_PROPERTIES_H
#define ALARM_DISPLAY_PROPERTIES_H
#include "alarm_bar_properties.h"
#include <QMap>
//20261113-2 add: Only the three captions are localized; all observed styling and column flags are common.
struct AlarmDisplayCaptions
{
    QString message = QStringLiteral("消息");
    QString date = QStringLiteral("日期");
    QString time = QStringLiteral("时间");
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, AlarmDisplayCaptions *output, QString *error = nullptr);
};
struct AlarmDisplayProperties
{
'''+''.join(f'    {t} {n} = {v};\n' for t,n,v in fields)+'''    QMap<QString, AlarmDisplayCaptions> captions;
    StaticTextProperties visibility;
    AlarmDisplayProperties();
    static QList<int> fontSizes();
    AlarmDisplayCaptions captionsFor(const QString &language) const;
    void setLanguages(const QStringList &languages);
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, AlarmDisplayProperties *output, QString *error = nullptr);
};
#endif
''')
s='#include "alarm_display_properties.h"\n#include <QColor>\n#include <cmath>\n\n'
s+=f('displayShape','Check exact record keys and bounded integral numbers before any conversion.','static bool displayShape(const QJsonObject &json, const QJsonObject &shape)','''    QJsonObject::const_iterator field;
    double number;
    if (json.keys() != shape.keys()) { return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field) { if (field.value().type() != shape.value(field.key()).type()) { return false; } if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || number < 0 || number > 192 || std::floor(number) != number) { return false; } } }
    return true;''','json, shape','True for an exact scalar shape.')
s+=f('AlarmDisplayCaptions::toJson','Persist three independent column captions, including captions for currently hidden columns.','QJsonObject AlarmDisplayCaptions::toJson() const','    return {{QStringLiteral("message"), message}, {QStringLiteral("date"), date}, {QStringLiteral("time"), time}};',returns='Complete caption JSON.')
s+=f('AlarmDisplayCaptions::fromJson','Reject unknown, non-string and oversized captions before replacing caller state.','bool AlarmDisplayCaptions::fromJson(const QJsonObject &json, AlarmDisplayCaptions *output, QString *error)','''    AlarmDisplayCaptions candidate;
    if (output == nullptr || !displayShape(json, candidate.toJson())) { if (error != nullptr) { *error = QStringLiteral("报警列名字段缺失、未知或类型错误。"); } return false; }
    candidate.message = json.value(QStringLiteral("message")).toString(); candidate.date = json.value(QStringLiteral("date")).toString(); candidate.time = json.value(QStringLiteral("time")).toString();
    if (candidate.message.size() > 32767 || candidate.date.size() > 32767 || candidate.time.size() > 32767) { return false; } *output = candidate; return true;''','json, output, error','True after complete caption validation.')
s+=f('AlarmDisplayProperties::AlarmDisplayProperties','Seed the observed default Chinese captions without making an editor language index persistent.','AlarmDisplayProperties::AlarmDisplayProperties()','    captions.insert(QStringLiteral("中文"), AlarmDisplayCaptions());')
s+=f('AlarmDisplayProperties::fontSizes','Expose all seven observed sizes rather than copying the five alarm-bar sizes.','QList<int> AlarmDisplayProperties::fontSizes()','    return {12, 16, 24, 48, 64, 96, 192};',returns='Measured font-size list.')
s+=f('AlarmDisplayProperties::captionsFor','Use native default Chinese captions for a newly introduced language until it is edited.','AlarmDisplayCaptions AlarmDisplayProperties::captionsFor(const QString &language) const','    return captions.value(language, AlarmDisplayCaptions());','language','Effective detached captions.')
s+=f('AlarmDisplayProperties::setLanguages','Preserve surviving caption languages and initialize new languages as one bounded project transaction.','void AlarmDisplayProperties::setLanguages(const QStringList &languages)','''    QMap<QString, AlarmDisplayCaptions> next;
    QString language;
    Q_FOREACH (language, languages) { next.insert(language, captionsFor(language)); } captions = next;''','languages')
s+=f('AlarmDisplayProperties::validate','Enforce measured row/font limits, five common colors, bounded caption maps and explicit active visibility state.','bool AlarmDisplayProperties::validate(QString *error) const','''    QString color;
    QMap<QString, AlarmDisplayCaptions>::const_iterator language;
    AlarmDisplayCaptions parsed;
    if (rows < 1 || rows > 20 || !fontSizes().contains(fontSize) || captions.isEmpty() || captions.size() > 8 || (visibility.visibilityEnabled && visibility.bitEnabled && !visibilityBitChosen)) { if (error != nullptr) { *error = QStringLiteral("报警显示行数、字号、语言或显现状态无效。"); } return false; }
    Q_FOREACH (color, (QStringList{titleBackground, titleColor, frameColor, backgroundColor, textColor})) { if (color.size() > 32 || !QColor(color).isValid()) { return false; } }
    for (language = captions.constBegin(); language != captions.constEnd(); ++language) { if (language.key().trimmed().isEmpty() || language.key().size() > 128 || !AlarmDisplayCaptions::fromJson(language.value().toJson(), &parsed, error)) { return false; } }
    return visibility.validate(error);''','error','True for the supported table-display editor slice.')
s+=f('AlarmDisplayProperties::validateAddresses','Validate only active display conditions; alarm inputs remain in global registration definitions.','bool AlarmDisplayProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const','''    AlarmBarProperties shared;
    shared.visibility = visibility; shared.visibilityBitChosen = visibilityBitChosen; return shared.validateAddresses(connections, error);''','connections, error','True for all active visibility carriers.')
s+=f('AlarmDisplayProperties::toJson','Store common appearance and complete caption maps without mixing in device alarm runtime state.','QJsonObject AlarmDisplayProperties::toJson() const','''    QJsonObject localized;
    QMap<QString, AlarmDisplayCaptions>::const_iterator language;
    for (language = captions.constBegin(); language != captions.constEnd(); ++language) { localized.insert(language.key(), language.value().toJson()); }
    return {'''+', '.join('{QStringLiteral("'+n+'"), '+n+'}' for t,n,v in fields)+', {QStringLiteral("captions"), localized}, {QStringLiteral("visibility"), visibility.toJson()}};','None','Complete typed display record.')
s+=f('AlarmDisplayProperties::fromJson','Reject unknown, fractional, mistyped and malformed nested records atomically.','bool AlarmDisplayProperties::fromJson(const QJsonObject &json, AlarmDisplayProperties *output, QString *error)','''    AlarmDisplayProperties candidate;
    QJsonObject localized;
    QJsonObject::const_iterator language;
    AlarmDisplayCaptions captions;
    if (output == nullptr || !displayShape(json, candidate.toJson())) { if (error != nullptr) { *error = QStringLiteral("报警显示字段缺失、未知或类型错误。"); } return false; }
'''+''.join(f'    candidate.{n} = json.value(QStringLiteral("{n}")).'+{'QString':'toString','int':'toInt','bool':'toBool'}[t]+'();\n' for t,n,v in fields)+'''    localized = json.value(QStringLiteral("captions")).toObject(); if (localized.size() > 8) { return false; } candidate.captions.clear(); for (language = localized.constBegin(); language != localized.constEnd(); ++language) { if (!language.value().isObject() || !AlarmDisplayCaptions::fromJson(language.value().toObject(), &captions, error)) { return false; } candidate.captions.insert(language.key(), captions); }
    if (!StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() || !candidate.validate(error)) { return false; } *output = candidate; return true;''','json, output, error','True after exact complete validation.')
put('src/alarm_display_properties.cpp',s)
print('Created C28 common styling, seven sizes and three-caption language records.')
