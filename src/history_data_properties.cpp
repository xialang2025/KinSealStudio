#include "history_data_properties.h"
#include <QColor>
#include <QJsonArray>
#include <QSet>
#include <cmath>

/******************************************************************************
 * Function Name   : historyDataError
 * Description     : Report an actionable typed-record error without mutating caller state.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.13
 ******************************************************************************/
static bool historyDataError(QString *error, const QString &message) //20261113-2 add
{
    if (error != nullptr) { *error = message; } return false;
}

/******************************************************************************
 * Function Name   : historyDataShape
 * Description     : Reject unknown keys and lossy scalar conversions before nested validation.
 * Input Parameters: json, shape
 * Return Value    : True for an exact scalar shape.
 * Date            : 2026.11.13
 ******************************************************************************/
static bool historyDataShape(const QJsonObject &json, const QJsonObject &shape) //20261113-2 add
{
    QJsonObject::const_iterator field;
    double number;
    if (json.keys() != shape.keys()) { return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != shape.value(field.key()).type()) { return false; }
        if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || number < 0 || number > 48 || number != std::floor(number)) { return false; } }
    }
    return true;
}

/******************************************************************************
 * Function Name   : historyDataNames
 * Description     : Require bounded language names and string captions without coercion.
 * Input Parameters: names
 * Return Value    : True for a bounded localized name map.
 * Date            : 2026.11.13
 ******************************************************************************/
static bool historyDataNames(const QMap<QString, QString> &names) //20261113-2 add
{
    QMap<QString, QString>::const_iterator name;
    if (names.isEmpty() || names.size() > 8) { return false; }
    for (name = names.constBegin(); name != names.constEnd(); ++name) { if (name.key().trimmed().isEmpty() || name.key().size() > 128 || name.value().size() > 32767) { return false; } }
    return true;
}

/******************************************************************************
 * Function Name   : HistoryDataColumn::toJson
 * Description     : Keep stable source identity, shared visibility and each localized caption.
 * Input Parameters: None
 * Return Value    : Complete column JSON.
 * Date            : 2026.11.13
 ******************************************************************************/
QJsonObject HistoryDataColumn::toJson() const //20261113-2 add
{
    QJsonObject localized;
    QMap<QString, QString>::const_iterator name;
    for (name = names.constBegin(); name != names.constEnd(); ++name) { localized.insert(name.key(), name.value()); }
    return {{QStringLiteral("sourceItemId"), sourceItemId}, {QStringLiteral("shown"), shown}, {QStringLiteral("names"), localized}};
}

/******************************************************************************
 * Function Name   : HistoryDataColumn::fromJson
 * Description     : Parse one exact source column atomically, including currently hidden captions.
 * Input Parameters: json, output, error
 * Return Value    : True after complete column validation.
 * Date            : 2026.11.13
 ******************************************************************************/
bool HistoryDataColumn::fromJson(const QJsonObject &json, HistoryDataColumn *output, QString *error) //20261113-2 add
{
    HistoryDataColumn candidate;
    QJsonObject names;
    QJsonObject::const_iterator name;
    if (output == nullptr || !historyDataShape(json, candidate.toJson())) { return historyDataError(error, QStringLiteral("历史数据列字段缺失、未知或类型错误。")); }
    candidate.sourceItemId = json.value(QStringLiteral("sourceItemId")).toString(); candidate.shown = json.value(QStringLiteral("shown")).toBool(); names = json.value(QStringLiteral("names")).toObject();
    for (name = names.constBegin(); name != names.constEnd(); ++name) { if (!name.value().isString()) { return false; } candidate.names.insert(name.key(), name.value().toString()); }
    if (candidate.sourceItemId.trimmed().isEmpty() || candidate.sourceItemId.size() > 128 || !historyDataNames(candidate.names)) { return historyDataError(error, QStringLiteral("历史数据列来源或语言名称无效。")); }
    *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : HistoryDataCaptions::toJson
 * Description     : Store date and time captions even when their columns are hidden.
 * Input Parameters: None
 * Return Value    : Localized date/time JSON.
 * Date            : 2026.11.13
 ******************************************************************************/
QJsonObject HistoryDataCaptions::toJson() const //20261113-2 add
{
    return {{QStringLiteral("date"), date}, {QStringLiteral("time"), time}};
}

/******************************************************************************
 * Function Name   : HistoryDataProperties::HistoryDataProperties
 * Description     : Seed measured default captions while leaving the required collection unresolved.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
HistoryDataProperties::HistoryDataProperties() //20261113-2 add
{
    captions.insert(QStringLiteral("中文"), HistoryDataCaptions());
}

/******************************************************************************
 * Function Name   : HistoryDataProperties::fontSizes
 * Description     : Expose the four observed title sizes for this distinct table family.
 * Input Parameters: None
 * Return Value    : Measured sizes.
 * Date            : 2026.11.13
 ******************************************************************************/
QList<int> HistoryDataProperties::fontSizes() //20261113-2 add
{
    return {12, 16, 24, 48};
}

/******************************************************************************
 * Function Name   : HistoryDataProperties::captionsFor
 * Description     : Return the selected language captions or measured initial labels.
 * Input Parameters: language
 * Return Value    : Detached effective captions.
 * Date            : 2026.11.13
 ******************************************************************************/
HistoryDataCaptions HistoryDataProperties::captionsFor(const QString &language) const //20261113-2 add
{
    return captions.value(language, HistoryDataCaptions());
}

/******************************************************************************
 * Function Name   : HistoryDataProperties::setLanguages
 * Description     : Retain surviving localized fields and initialize new languages without changing common flags.
 * Input Parameters: languages
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void HistoryDataProperties::setLanguages(const QStringList &languages) //20261113-2 add
{
    QMap<QString, HistoryDataCaptions> next;
    QMap<QString, QString> names;
    HistoryDataColumn *column;
    QString language;
    QString initial;
    int index;
    Q_FOREACH (language, languages) { next.insert(language, captionsFor(language)); } captions = next;
    for (index = 0; index < columns.size(); ++index)
    {
        column = &columns[index];
        initial = column->names.value(QStringLiteral("中文"), column->names.isEmpty() ? QString() : column->names.first()); names.clear();
        Q_FOREACH (language, languages) { names.insert(language, column->names.value(language, initial)); } column->names = names;
    }
}

/******************************************************************************
 * Function Name   : HistoryDataProperties::reconcileCollection
 * Description     : Rebuild columns in collector order while retaining surviving source identities and localized drafts.
 * Input Parameters: collection, languages
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void HistoryDataProperties::reconcileCollection(const DataCollectionProperties &collection, const QStringList &languages) //20261113-2 add
{
    QVector<HistoryDataColumn> next;
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
    columns = next; setLanguages(languages);
}

/******************************************************************************
 * Function Name   : HistoryDataProperties::synchronizeNames
 * Description     : Copy every first-language data caption to selected languages without changing date/time captions or flags.
 * Input Parameters: first, targets
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void HistoryDataProperties::synchronizeNames(const QString &first, const QStringList &targets) //20261113-2 add
{
    HistoryDataColumn *column;
    QString language;
    QString name;
    int index;
    for (index = 0; index < columns.size(); ++index)
    {
        column = &columns[index];
        if (!column->names.contains(first)) { continue; } name = column->names.value(first);
        Q_FOREACH (language, targets) { if (column->names.contains(language)) { column->names.insert(language, name); } }
    }
}

/******************************************************************************
 * Function Name   : HistoryDataProperties::validate
 * Description     : Enforce exact fonts, zero-to-twenty rows, six colors, unique source columns and at least one visible column.
 * Input Parameters: error
 * Return Value    : True for the supported editor slice.
 * Date            : 2026.11.13
 ******************************************************************************/
bool HistoryDataProperties::validate(QString *error) const //20261113-2 add
{
    QString color;
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
    return visibility.validate(error);
}

/******************************************************************************
 * Function Name   : HistoryDataProperties::validateCollection
 * Description     : Match every source item and its order against the actual collection definition.
 * Input Parameters: collection, error
 * Return Value    : True for exact stable source references.
 * Date            : 2026.11.13
 ******************************************************************************/
bool HistoryDataProperties::validateCollection(const DataCollectionProperties &collection, QString *error) const //20261113-2 add
{
    int index;
    if (!collection.validate(error) || columns.size() != collection.items.size()) { return historyDataError(error, QStringLiteral("历史数据列与采集通道数量不一致。")); }
    for (index = 0; index < columns.size(); ++index) { if (columns[index].sourceItemId != collection.items[index].id) { return historyDataError(error, QStringLiteral("历史数据列的采集通道引用或顺序无效。")); } }
    return true;
}

/******************************************************************************
 * Function Name   : HistoryDataProperties::validateAddresses
 * Description     : Validate only active visibility carriers with the full selected word width.
 * Input Parameters: connections, error
 * Return Value    : True for valid visibility addresses.
 * Date            : 2026.11.13
 ******************************************************************************/
bool HistoryDataProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261113-2 add
{
    AlarmBarProperties shared;
    shared.visibility = visibility; shared.visibilityBitChosen = visibilityBitChosen; return shared.validateAddresses(connections, error);
}

/******************************************************************************
 * Function Name   : HistoryDataProperties::toJson
 * Description     : Persist the complete typed table without any fabricated history records or query state.
 * Input Parameters: None
 * Return Value    : Complete typed historical-data JSON.
 * Date            : 2026.11.13
 ******************************************************************************/
QJsonObject HistoryDataProperties::toJson() const //20261113-2 add
{
    QJsonObject localized;
    QJsonArray data;
    QMap<QString, HistoryDataCaptions>::const_iterator language;
    HistoryDataColumn column;
    for (language = captions.constBegin(); language != captions.constEnd(); ++language) { localized.insert(language.key(), language.value().toJson()); }
    Q_FOREACH (column, columns) { data.append(column.toJson()); }
    return {{QStringLiteral("collectionId"), collectionId}, {QStringLiteral("textColor"), textColor}, {QStringLiteral("lineColor"), lineColor}, {QStringLiteral("titleBackground"), titleBackground}, {QStringLiteral("titleColor"), titleColor}, {QStringLiteral("backgroundColor"), backgroundColor}, {QStringLiteral("frameColor"), frameColor}, {QStringLiteral("fontSize"), fontSize}, {QStringLiteral("rows"), rows}, {QStringLiteral("verticalLines"), verticalLines}, {QStringLiteral("horizontalLines"), horizontalLines}, {QStringLiteral("showDate"), showDate}, {QStringLiteral("showTime"), showTime}, {QStringLiteral("visibilityBitChosen"), visibilityBitChosen}, {QStringLiteral("captions"), localized}, {QStringLiteral("columns"), data}, {QStringLiteral("visibility"), visibility.toJson()}};
}

/******************************************************************************
 * Function Name   : HistoryDataProperties::fromJson
 * Description     : Reject malformed, unknown and nested mistyped fields before atomically replacing the output.
 * Input Parameters: json, output, error
 * Return Value    : True after complete strict validation.
 * Date            : 2026.11.13
 ******************************************************************************/
bool HistoryDataProperties::fromJson(const QJsonObject &json, HistoryDataProperties *output, QString *error) //20261113-2 add
{
    HistoryDataProperties candidate;
    QJsonObject localized;
    QJsonObject caption;
    QJsonObject::const_iterator language;
    QJsonValue value;
    HistoryDataColumn column;
    HistoryDataCaptions titles;
    if (output == nullptr || !historyDataShape(json, candidate.toJson())) { return historyDataError(error, QStringLiteral("历史数据显示字段缺失、未知或类型错误。")); }
    candidate.collectionId = json.value(QStringLiteral("collectionId")).toString();
    candidate.textColor = json.value(QStringLiteral("textColor")).toString();
    candidate.lineColor = json.value(QStringLiteral("lineColor")).toString();
    candidate.titleBackground = json.value(QStringLiteral("titleBackground")).toString();
    candidate.titleColor = json.value(QStringLiteral("titleColor")).toString();
    candidate.backgroundColor = json.value(QStringLiteral("backgroundColor")).toString();
    candidate.frameColor = json.value(QStringLiteral("frameColor")).toString();
    candidate.fontSize = json.value(QStringLiteral("fontSize")).toInt();
    candidate.rows = json.value(QStringLiteral("rows")).toInt();
    candidate.verticalLines = json.value(QStringLiteral("verticalLines")).toBool();
    candidate.horizontalLines = json.value(QStringLiteral("horizontalLines")).toBool();
    candidate.showDate = json.value(QStringLiteral("showDate")).toBool();
    candidate.showTime = json.value(QStringLiteral("showTime")).toBool();
    candidate.visibilityBitChosen = json.value(QStringLiteral("visibilityBitChosen")).toBool();
    localized = json.value(QStringLiteral("captions")).toObject(); if (localized.size() > 8) { return false; } candidate.captions.clear();
    for (language = localized.constBegin(); language != localized.constEnd(); ++language)
    {
        if (!language.value().isObject() || !historyDataShape(language.value().toObject(), titles.toJson())) { return false; }
        caption = language.value().toObject(); titles.date = caption.value(QStringLiteral("date")).toString(); titles.time = caption.value(QStringLiteral("time")).toString(); candidate.captions.insert(language.key(), titles);
    }
    if (json.value(QStringLiteral("columns")).toArray().size() > 32) { return false; }
    Q_FOREACH (value, json.value(QStringLiteral("columns")).toArray()) { if (!value.isObject() || !HistoryDataColumn::fromJson(value.toObject(), &column, error)) { return false; } candidate.columns.append(column); }
    if (!StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() || !candidate.validate(error)) { return false; }
    *output = candidate; return true;
}
