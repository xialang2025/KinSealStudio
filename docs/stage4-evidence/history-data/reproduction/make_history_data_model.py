"""Produce C30's independent typed historical-data editor contract."""
from pathlib import Path
root = Path(__file__).resolve().parent / 'worktree'
def put(path, text):
    (root/path).write_text(text, encoding='utf-8', newline='\n')
def f(name, desc, signature, body, params='None', returns='None.'):
    return f'\n/******************************************************************************\n * Function Name   : {name}\n * Description     : {desc}\n * Input Parameters: {params}\n * Return Value    : {returns}\n * Date            : 2026.11.13\n ******************************************************************************/\n{signature} //20261113-2 add\n{{\n{body}\n}}\n'
put('src/history_data_properties.h', '''#ifndef HISTORY_DATA_PROPERTIES_H
#define HISTORY_DATA_PROPERTIES_H
#include "alarm_bar_properties.h"
#include "data_collection_properties.h"
#include <QMap>
//20261113-2 add: Column identities follow the collector; captions are localized and visibility is shared.
struct HistoryDataColumn
{
    QString sourceItemId;
    bool shown = true;
    QMap<QString, QString> names;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, HistoryDataColumn *output, QString *error = nullptr);
};
struct HistoryDataCaptions
{
    QString date = QStringLiteral("日期");
    QString time = QStringLiteral("时间");
    QJsonObject toJson() const;
};
struct HistoryDataProperties
{
    QString collectionId;
    QString textColor = QStringLiteral("#000000");
    QString lineColor = QStringLiteral("#00c000");
    QString titleBackground = QStringLiteral("#ffff00");
    QString titleColor = QStringLiteral("#000000");
    QString backgroundColor = QStringLiteral("#e0e0e0");
    QString frameColor = QStringLiteral("#000000");
    int fontSize = 16;
    int rows = 5;
    bool verticalLines = true;
    bool horizontalLines = true;
    bool showDate = true;
    bool showTime = true;
    bool visibilityBitChosen = false;
    QMap<QString, HistoryDataCaptions> captions;
    QVector<HistoryDataColumn> columns;
    StaticTextProperties visibility;
    HistoryDataProperties();
    static QList<int> fontSizes();
    HistoryDataCaptions captionsFor(const QString &language) const;
    void setLanguages(const QStringList &languages);
    void reconcileCollection(const DataCollectionProperties &collection, const QStringList &languages);
    void synchronizeNames(const QString &first, const QStringList &targets);
    bool validate(QString *error = nullptr) const;
    bool validateCollection(const DataCollectionProperties &collection, QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, HistoryDataProperties *output, QString *error = nullptr);
};
#endif
''')
s = '#include "history_data_properties.h"\n#include <QColor>\n#include <QJsonArray>\n#include <QSet>\n#include <cmath>\n'
s += f('historyDataError', 'Report an actionable typed-record error without mutating caller state.', 'static bool historyDataError(QString *error, const QString &message)', '''    if (error != nullptr) { *error = message; } return false;''', 'error, message', 'False.')
s += f('historyDataShape', 'Reject unknown keys and lossy scalar conversions before nested validation.', 'static bool historyDataShape(const QJsonObject &json, const QJsonObject &shape)', '''    QJsonObject::const_iterator field;
    double number;
    if (json.keys() != shape.keys()) { return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != shape.value(field.key()).type()) { return false; }
        if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || number < 0 || number > 48 || number != std::floor(number)) { return false; } }
    }
    return true;''', 'json, shape', 'True for an exact scalar shape.')
s += f('historyDataNames', 'Require bounded language names and string captions without coercion.', 'static bool historyDataNames(const QMap<QString, QString> &names)', '''    QMap<QString, QString>::const_iterator name;
    if (names.isEmpty() || names.size() > 8) { return false; }
    for (name = names.constBegin(); name != names.constEnd(); ++name) { if (name.key().trimmed().isEmpty() || name.key().size() > 128 || name.value().size() > 32767) { return false; } }
    return true;''', 'names', 'True for a bounded localized name map.')
s += f('HistoryDataColumn::toJson', 'Keep stable source identity, shared visibility and each localized caption.', 'QJsonObject HistoryDataColumn::toJson() const', '''    QJsonObject localized;
    QMap<QString, QString>::const_iterator name;
    for (name = names.constBegin(); name != names.constEnd(); ++name) { localized.insert(name.key(), name.value()); }
    return {{QStringLiteral("sourceItemId"), sourceItemId}, {QStringLiteral("shown"), shown}, {QStringLiteral("names"), localized}};''', returns='Complete column JSON.')
s += f('HistoryDataColumn::fromJson', 'Parse one exact source column atomically, including currently hidden captions.', 'bool HistoryDataColumn::fromJson(const QJsonObject &json, HistoryDataColumn *output, QString *error)', '''    HistoryDataColumn candidate;
    QJsonObject names;
    QJsonObject::const_iterator name;
    if (output == nullptr || !historyDataShape(json, candidate.toJson())) { return historyDataError(error, QStringLiteral("历史数据列字段缺失、未知或类型错误。")); }
    candidate.sourceItemId = json.value(QStringLiteral("sourceItemId")).toString(); candidate.shown = json.value(QStringLiteral("shown")).toBool(); names = json.value(QStringLiteral("names")).toObject();
    for (name = names.constBegin(); name != names.constEnd(); ++name) { if (!name.value().isString()) { return false; } candidate.names.insert(name.key(), name.value().toString()); }
    if (candidate.sourceItemId.trimmed().isEmpty() || candidate.sourceItemId.size() > 128 || !historyDataNames(candidate.names)) { return historyDataError(error, QStringLiteral("历史数据列来源或语言名称无效。")); }
    *output = candidate; return true;''', 'json, output, error', 'True after complete column validation.')
s += f('HistoryDataCaptions::toJson', 'Store date and time captions even when their columns are hidden.', 'QJsonObject HistoryDataCaptions::toJson() const', '''    return {{QStringLiteral("date"), date}, {QStringLiteral("time"), time}};''', returns='Localized date/time JSON.')
s += f('HistoryDataProperties::HistoryDataProperties', 'Seed measured default captions while leaving the required collection unresolved.', 'HistoryDataProperties::HistoryDataProperties()', '''    captions.insert(QStringLiteral("中文"), HistoryDataCaptions());''')
s += f('HistoryDataProperties::fontSizes', 'Expose the four observed title sizes for this distinct table family.', 'QList<int> HistoryDataProperties::fontSizes()', '''    return {12, 16, 24, 48};''', returns='Measured sizes.')
s += f('HistoryDataProperties::captionsFor', 'Return the selected language captions or measured initial labels.', 'HistoryDataCaptions HistoryDataProperties::captionsFor(const QString &language) const', '''    return captions.value(language, HistoryDataCaptions());''', 'language', 'Detached effective captions.')
s += f('HistoryDataProperties::setLanguages', 'Retain surviving localized fields and initialize new languages without changing common flags.', 'void HistoryDataProperties::setLanguages(const QStringList &languages)', '''    QMap<QString, HistoryDataCaptions> next;
    QMap<QString, QString> names;
    HistoryDataColumn *column;
    QString language;
    QString initial;
    Q_FOREACH (language, languages) { next.insert(language, captionsFor(language)); } captions = next;
    for (column = columns.begin(); column != columns.end(); ++column)
    {
        initial = column->names.value(QStringLiteral("中文"), column->names.isEmpty() ? QString() : column->names.first()); names.clear();
        Q_FOREACH (language, languages) { names.insert(language, column->names.value(language, initial)); } column->names = names;
    }''', 'languages')
s += f('HistoryDataProperties::reconcileCollection', 'Rebuild columns in collector order while retaining surviving source identities and localized drafts.', 'void HistoryDataProperties::reconcileCollection(const DataCollectionProperties &collection, const QStringList &languages)', '''    QVector<HistoryDataColumn> next;
    HistoryDataColumn column;
    HistoryDataColumn previous;
    DataCollectionItem item;
    QString language;
    Q_FOREACH (item, collection.items)
    {
        column = HistoryDataColumn(); column.sourceItemId = item.id;
        Q_FOREACH (language, languages) { column.names.insert(language, item.name); }
        Q_FOREACH (previous, columns) { if (previous.sourceItemId == item.id) { column = previous; break; } }
        next.append(column);
    }
    columns = next; setLanguages(languages);''', 'collection, languages')
s += f('HistoryDataProperties::synchronizeNames', 'Copy every first-language data caption to selected languages without changing date/time captions or flags.', 'void HistoryDataProperties::synchronizeNames(const QString &first, const QStringList &targets)', '''    HistoryDataColumn *column;
    QString language;
    QString name;
    for (column = columns.begin(); column != columns.end(); ++column)
    {
        if (!column->names.contains(first)) { continue; } name = column->names.value(first);
        Q_FOREACH (language, targets) { if (column->names.contains(language)) { column->names.insert(language, name); } }
    }''', 'first, targets')
s += f('HistoryDataProperties::validate', 'Enforce exact fonts, zero-to-twenty rows, six colors, unique source columns and at least one visible column.', 'bool HistoryDataProperties::validate(QString *error) const', '''    QString color;
    QMap<QString, HistoryDataCaptions>::const_iterator language;
    QSet<QString> ids;
    HistoryDataColumn column;
    bool any = showDate || showTime;
    if (collectionId.trimmed().isEmpty() || collectionId.size() > 128 || rows < 0 || rows > 20 || !fontSizes().contains(fontSize) || captions.isEmpty() || captions.size() > 8 || columns.isEmpty() || columns.size() > 32 || (visibility.visibilityEnabled && visibility.bitEnabled && !visibilityBitChosen)) { return historyDataError(error, QStringLiteral("历史数据采集、行数、字号、列或显现状态无效。")); }
    Q_FOREACH (color, (QStringList{textColor, lineColor, titleBackground, titleColor, backgroundColor, frameColor})) { if (color.size() > 32 || !QColor(color).isValid()) { return historyDataError(error, QStringLiteral("历史数据表颜色无效。")); } }
    for (language = captions.constBegin(); language != captions.constEnd(); ++language) { if (language.key().trimmed().isEmpty() || language.key().size() > 128 || language.value().date.size() > 32767 || language.value().time.size() > 32767) { return false; } }
    Q_FOREACH (column, columns)
    {
        if (column.sourceItemId.trimmed().isEmpty() || column.sourceItemId.size() > 128 || ids.contains(column.sourceItemId) || !historyDataNames(column.names) || column.names.keys() != captions.keys()) { return historyDataError(error, QStringLiteral("历史数据列来源重复或语言集合不一致。")); }
        ids.insert(column.sourceItemId); any = any || column.shown;
    }
    if (!any) { return historyDataError(error, QStringLiteral("未选择任何显示数据")); }
    return visibility.validate(error);''', 'error', 'True for the supported editor slice.')
s += f('HistoryDataProperties::validateCollection', 'Match every source item and its order against the actual collection definition.', 'bool HistoryDataProperties::validateCollection(const DataCollectionProperties &collection, QString *error) const', '''    int index;
    if (!collection.validate(error) || columns.size() != collection.items.size()) { return historyDataError(error, QStringLiteral("历史数据列与采集通道数量不一致。")); }
    for (index = 0; index < columns.size(); ++index) { if (columns[index].sourceItemId != collection.items[index].id) { return historyDataError(error, QStringLiteral("历史数据列的采集通道引用或顺序无效。")); } }
    return true;''', 'collection, error', 'True for exact stable source references.')
s += f('HistoryDataProperties::validateAddresses', 'Validate only active visibility carriers with the full selected word width.', 'bool HistoryDataProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const', '''    AlarmBarProperties shared;
    shared.visibility = visibility; shared.visibilityBitChosen = visibilityBitChosen; return shared.validateAddresses(connections, error);''', 'connections, error', 'True for valid visibility addresses.')
strings = ['collectionId','textColor','lineColor','titleBackground','titleColor','backgroundColor','frameColor']
ints = ['fontSize','rows']
bools = ['verticalLines','horizontalLines','showDate','showTime','visibilityBitChosen']
s += f('HistoryDataProperties::toJson', 'Persist the complete typed table without any fabricated history records or query state.', 'QJsonObject HistoryDataProperties::toJson() const', '''    QJsonObject localized;
    QJsonArray data;
    QMap<QString, HistoryDataCaptions>::const_iterator language;
    HistoryDataColumn column;
    for (language = captions.constBegin(); language != captions.constEnd(); ++language) { localized.insert(language.key(), language.value().toJson()); }
    Q_FOREACH (column, columns) { data.append(column.toJson()); }
    return {''' + ', '.join('{QStringLiteral("'+key+'"), '+key+'}' for key in strings+ints+bools) + ''', {QStringLiteral("captions"), localized}, {QStringLiteral("columns"), data}, {QStringLiteral("visibility"), visibility.toJson()}};''', returns='Complete typed historical-data JSON.')
s += f('HistoryDataProperties::fromJson', 'Reject malformed, unknown and nested mistyped fields before atomically replacing the output.', 'bool HistoryDataProperties::fromJson(const QJsonObject &json, HistoryDataProperties *output, QString *error)', '''    HistoryDataProperties candidate;
    QJsonObject localized;
    QJsonObject caption;
    QJsonObject::const_iterator language;
    QJsonValue value;
    HistoryDataColumn column;
    HistoryDataCaptions titles;
    if (output == nullptr || !historyDataShape(json, candidate.toJson())) { return historyDataError(error, QStringLiteral("历史数据显示字段缺失、未知或类型错误。")); }
''' + ''.join(f'    candidate.{key} = json.value(QStringLiteral("{key}")).to{kind}();\n' for keys,kind in [(strings,'String'),(ints,'Int'),(bools,'Bool')] for key in keys) + '''    localized = json.value(QStringLiteral("captions")).toObject(); if (localized.size() > 8) { return false; } candidate.captions.clear();
    for (language = localized.constBegin(); language != localized.constEnd(); ++language)
    {
        if (!language.value().isObject() || !historyDataShape(language.value().toObject(), titles.toJson())) { return false; }
        caption = language.value().toObject(); titles.date = caption.value(QStringLiteral("date")).toString(); titles.time = caption.value(QStringLiteral("time")).toString(); candidate.captions.insert(language.key(), titles);
    }
    if (json.value(QStringLiteral("columns")).toArray().size() > 32) { return false; }
    Q_FOREACH (value, json.value(QStringLiteral("columns")).toArray()) { if (!value.isObject() || !HistoryDataColumn::fromJson(value.toObject(), &column, error)) { return false; } candidate.columns.append(column); }
    if (!StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() || !candidate.validate(error)) { return false; }
    *output = candidate; return true;''', 'json, output, error', 'True after complete strict validation.')
put('src/history_data_properties.cpp', s)
print('Created independent historical-data model with collection reconciliation and measured edge limits.')
