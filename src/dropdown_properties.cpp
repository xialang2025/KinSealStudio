#include "dropdown_properties.h"
#include "word_button_properties.h"
#include "plc_native_catalog.h"
#include <QJsonArray>
#include <cmath>

/******************************************************************************
 * Function Name   : DropdownProperties::DropdownProperties
 * Description     : Initialize two measured default rows and independent visibility/touch conditions.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
DropdownProperties::DropdownProperties() //20261112-0 add
{
    touch.minimumLevel = 2; setCount(2);
}

/******************************************************************************
 * Function Name   : DropdownProperties::setCount
 * Description     : Resize a bounded Lite row draft, dropping the truncated tail and creating fresh indexed defaults.
 * Input Parameters: count
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void DropdownProperties::setCount(int count) //20261112-0 add
{
    int bounded = qBound(2, count, 15);
    int index;
    while (entries.size() > bounded) { entries.removeLast(); }
    while (entries.size() < bounded) { index = entries.size(); entries.append({QString::number(index), QStringLiteral("state_%1").arg(index)}); }
}

/******************************************************************************
 * Function Name   : DropdownProperties::transferWordCount
 * Description     : Match the observed unsigned-first seven-type order to its complete carrier width.
 * Input Parameters: None
 * Return Value    : One or two words.
 * Date            : 2026.11.12
 ******************************************************************************/
int DropdownProperties::transferWordCount() const //20261112-0 add
{
    return dataType == 1 || dataType == 3 || dataType == 5 || dataType == 6 ? 2 : 1;
}

/******************************************************************************
 * Function Name   : DropdownProperties::accessDraft
 * Description     : Adapt only shared permission and notification validation without persisting invented button appearances.
 * Input Parameters: None
 * Return Value    : Temporary common-access validation carrier.
 * Date            : 2026.11.12
 ******************************************************************************/
BitButtonProperties DropdownProperties::accessDraft() const //20261112-0 add
{
    BitButtonProperties access;
    access.indicator = false; access.dualMonitor = false; access.monitorSame = false; access.bitIndex = 0; access.pressTime = 0;
    access.visibility = visibility; access.touch = touch; access.notice = notice; access.noticeBit = noticeBit; access.noticeWord = noticeWord; access.noticeBitValue = noticeBitValue; access.noticeBitAddress = noticeBitAddress; access.noticeWordAddress = noticeWordAddress; access.noticeWordType = noticeWordType; access.noticeWordValue = noticeWordValue;
    return access;
}

/******************************************************************************
 * Function Name   : DropdownProperties::toJson
 * Description     : Persist exact scalar, permission and ordered row drafts under an independent dropdown record.
 * Input Parameters: None
 * Return Value    : Complete typed editor JSON.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject DropdownProperties::toJson() const //20261112-0 add
{
    QJsonObject json;
    QJsonArray rows;
    DropdownEntry entry;
#define SAVE_DROPDOWN(name, initial) json.insert(QStringLiteral(#name), name);
    DROPDOWN_BOOL_FIELDS(SAVE_DROPDOWN) DROPDOWN_INT_FIELDS(SAVE_DROPDOWN) DROPDOWN_STRING_FIELDS(SAVE_DROPDOWN)
#undef SAVE_DROPDOWN
    Q_FOREACH (entry, entries) { rows.append(QJsonObject{{QStringLiteral("value"), entry.value}, {QStringLiteral("label"), entry.label}}); }
    json.insert(QStringLiteral("entries"), rows); json.insert(QStringLiteral("visibility"), visibility.toJson()); json.insert(QStringLiteral("touch"), touch.toJson()); return json;
}

/******************************************************************************
 * Function Name   : DropdownProperties::fromJson
 * Description     : Reject missing, unknown, lossy or fractional records atomically before publishing a complete table.
 * Input Parameters: json, output, error
 * Return Value    : True for a complete validated dropdown model.
 * Date            : 2026.11.12
 ******************************************************************************/
bool DropdownProperties::fromJson(const QJsonObject &json, DropdownProperties *output, QString *error) //20261112-0 add
{
    DropdownProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator field;
    QJsonValue row;
    QJsonObject item;
    double number;
    if (output == nullptr || json.keys() != shape.keys()) { if (error != nullptr) { *error = QStringLiteral("下拉框字段缺失或未知。"); } return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != shape.value(field.key()).type()) { if (error != nullptr) { *error = QStringLiteral("下拉框字段类型错误。"); } return false; }
        if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 40) { return false; } }
    }
#define READ_DROPDOWN_BOOL(name, initial) candidate.name = json.value(QStringLiteral(#name)).toBool();
    DROPDOWN_BOOL_FIELDS(READ_DROPDOWN_BOOL)
#undef READ_DROPDOWN_BOOL
#define READ_DROPDOWN_INT(name, initial) candidate.name = json.value(QStringLiteral(#name)).toInt();
    DROPDOWN_INT_FIELDS(READ_DROPDOWN_INT)
#undef READ_DROPDOWN_INT
#define READ_DROPDOWN_STRING(name, initial) candidate.name = json.value(QStringLiteral(#name)).toString();
    DROPDOWN_STRING_FIELDS(READ_DROPDOWN_STRING)
#undef READ_DROPDOWN_STRING
    if (!StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() || !StaticTextProperties::fromJson(json.value(QStringLiteral("touch")).toObject(), &candidate.touch, error) || candidate.touch.toJson() != json.value(QStringLiteral("touch")).toObject() || json.value(QStringLiteral("entries")).toArray().size() > 15) { return false; }
    candidate.entries.clear();
    Q_FOREACH (row, json.value(QStringLiteral("entries")).toArray())
    {
        item = row.toObject();
        if (!row.isObject() || item.keys() != QStringList({QStringLiteral("label"), QStringLiteral("value")}) || !item.value(QStringLiteral("label")).isString() || !item.value(QStringLiteral("value")).isString()) { return false; }
        candidate.entries.append({item.value(QStringLiteral("value")).toString(), item.value(QStringLiteral("label")).toString()});
    }
    if (!candidate.validate(error)) { return false; } *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : DropdownProperties::validate
 * Description     : Validate observed UI bounds, bounded text/typed values and explicitly chosen active permission states.
 * Input Parameters: error
 * Return Value    : True for the declared PC editor slice.
 * Date            : 2026.11.12
 ******************************************************************************/
bool DropdownProperties::validate(QString *error) const //20261112-0 add
{
    ScreenButtonProperties shared;
    WordButtonProperties scalar;
    DropdownEntry entry;
    if (entries.size() < 2 || entries.size() > 15 || fontSize < 8 || fontSize > 40 || alignment < 0 || alignment > 2 || direction < 0 || direction > 1 || dataType < 0 || dataType > 6 || !QColor(background).isValid() || !QColor(textColor).isValid() || background.size() > 32 || textColor.size() > 32 || address.size() > 1024 || macroName.size() > 256 || (macroEnabled && macroName.trimmed().isEmpty()) || (visibility.visibilityEnabled && visibility.bitEnabled && !visibilityBitChosen) || (touch.visibilityEnabled && touch.bitEnabled && !touchBitChosen))
    { if (error != nullptr) { *error = QStringLiteral("下拉框项目数、字号、类型、颜色或活动权限/宏选择无效。"); } return false; }
    shared.common = accessDraft(); if (!shared.validate(error)) { return false; }
    scalar.resetDefaultsForDataType(dataType); scalar.operation = 0;
    Q_FOREACH (entry, entries)
    {
        if (entry.value.size() > 256 || entry.label.size() > 32767) { if (error != nullptr) { *error = QStringLiteral("下拉框行文本超过本版编辑保护范围。"); } return false; }
        scalar.constantValue = entry.value; if (!scalar.validate(error)) { return false; }
    }
    return true;
}

/******************************************************************************
 * Function Name   : DropdownProperties::validateAddresses
 * Description     : Check the entire operation word span and independently enabled permission/notice carriers.
 * Input Parameters: connections, error
 * Return Value    : True for all active carriers without performing a device write.
 * Date            : 2026.11.12
 ******************************************************************************/
bool DropdownProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261112-0 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress parsed;
    ScreenButtonProperties shared;
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    context.connections = connections; context.role = PlcAddressRole::Word; context.transferCount = u32(transferWordCount());
    if (!parseQualifiedPlcAddressText(address, context, catalog, &parsed, error)) { return false; }
    shared.common = accessDraft(); return shared.validateAddresses(connections, error);
}
