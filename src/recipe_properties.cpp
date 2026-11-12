#include "recipe_properties.h"
#include "data_collection_properties.h"
#include <QJsonArray>
#include <QLocale>
#include <QRegularExpression>
#include <cmath>
#include <limits>

/******************************************************************************
 * Function Name   : recipeError
 * Description     : Report a typed recipe failure without replacing caller state.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.13
 ******************************************************************************/
static bool recipeError(QString *error, const QString &message) //20261113-2 add
{
    if (error != nullptr) { *error = message; } return false;
}

/******************************************************************************
 * Function Name   : recipeShape
 * Description     : Require exact field types and lossless bounded integral dimensions.
 * Input Parameters: json, shape
 * Return Value    : True for an exact recipe record shape.
 * Date            : 2026.11.13
 ******************************************************************************/
static bool recipeShape(const QJsonObject &json, const QJsonObject &shape) //20261113-2 add
{
    QJsonObject::const_iterator field;
    double number;
    if (json.keys() != shape.keys()) { return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field) { if (field.value().type() != shape.value(field.key()).type()) { return false; } if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || number < 0 || number > 200 || number != std::floor(number)) { return false; } } }
    return true;
}

/******************************************************************************
 * Function Name   : RecipeProperties::RecipeProperties
 * Description     : Seed the measured four-item, one-recipe table and common header names.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
RecipeProperties::RecipeProperties() //20261113-2 add
{
    resizeTable(4, 1);
}

/******************************************************************************
 * Function Name   : RecipeProperties::dataTypes
 * Description     : Keep the observed five scalar types in native order.
 * Input Parameters: None
 * Return Value    : Ordered type labels.
 * Date            : 2026.11.13
 ******************************************************************************/
QStringList RecipeProperties::dataTypes() //20261113-2 add
{
    return DataCollectionProperties::dataTypes();
}

/******************************************************************************
 * Function Name   : RecipeProperties::maximumLength
 * Description     : Apply the measured 200-row sixteen-bit and 99-row thirty-two-bit editor limits.
 * Input Parameters: type
 * Return Value    : Measured active row limit.
 * Date            : 2026.11.13
 ******************************************************************************/
int RecipeProperties::maximumLength(int type) //20261113-2 add
{
    return type == 0 || type == 2 ? 200 : 99;
}

/******************************************************************************
 * Function Name   : RecipeProperties::wordSpan
 * Description     : Resolve the complete scalar width for active recipe transfers and readonly range labels.
 * Input Parameters: None
 * Return Value    : Words per scalar.
 * Date            : 2026.11.13
 ******************************************************************************/
int RecipeProperties::wordSpan() const //20261113-2 add
{
    return dataType == 0 || dataType == 2 ? 1 : 2;
}

/******************************************************************************
 * Function Name   : RecipeProperties::resizeTable
 * Description     : Retain hidden values and names on shrink and rebuild exposed default item-address labels on regrowth.
 * Input Parameters: rows, columns
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void RecipeProperties::resizeTable(int rows, int columns) //20261113-2 add
{
    int index;
    int column;
    int previousLength = length;
    QStringList data;
    QRegularExpression automatic(QStringLiteral("^RWI:[0-9]+$"));
    if (rows < 1 || rows > 200 || columns < 1 || columns > 200) { return; }
    while (recipeNames.size() < columns) { recipeNames.append(QStringLiteral("RECIPE%1").arg(recipeNames.size() + 1)); }
    while (itemNames.size() < rows) { itemNames.append(QStringLiteral("RWI:%1").arg((itemNames.size() + 1) * wordSpan())); }
    while (values.size() < itemNames.size()) { data.clear(); for (column = 0; column < recipeNames.size(); ++column) { data.append(QStringLiteral("0")); } values.append(data); }
    for (index = 0; index < values.size(); ++index) { while (values[index].size() < recipeNames.size()) { values[index].append(QStringLiteral("0")); } }
    for (index = previousLength; index < rows; ++index) { if (automatic.match(itemNames[index]).hasMatch()) { itemNames[index] = QStringLiteral("RWI:%1").arg((index + 1) * wordSpan()); } }
    length = rows; count = columns;
}

/******************************************************************************
 * Function Name   : RecipeProperties::changeType
 * Description     : Retain existing scalar drafts and headers while clamping only the active row count.
 * Input Parameters: type
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void RecipeProperties::changeType(int type) //20261113-2 add
{
    if (type < 0 || type > 4) { return; } dataType = type; if (length > maximumLength(type)) { length = maximumLength(type); }
}

/******************************************************************************
 * Function Name   : RecipeProperties::copyValues
 * Description     : Copy active values from the zero-based source recipe without copying either header set.
 * Input Parameters: source, target
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void RecipeProperties::copyValues(int source, int target) //20261113-2 add
{
    int row;
    if (source < 0 || source >= count || target < 0 || target >= count) { return; }
    for (row = 0; row < length; ++row) { values[row][target] = values[row][source]; }
}

/******************************************************************************
 * Function Name   : RecipeProperties::validValue
 * Description     : Require a finite representable decimal scalar instead of silently truncating recipe data.
 * Input Parameters: value, type
 * Return Value    : True for a finite scalar of the selected type.
 * Date            : 2026.11.13
 ******************************************************************************/
bool RecipeProperties::validValue(const QString &value, int type) //20261113-2 add
{
    bool valid;
    qlonglong integer;
    double real;
    if (value.isEmpty() || value.size() > 128 || value != value.trimmed() || type < 0 || type > 4) { return false; }
    if (type == 4) { real = QLocale::c().toDouble(value, &valid); return valid && !value.contains(QLatin1Char(',')) && std::isfinite(real) && std::fabs(real) <= double(std::numeric_limits<float>::max()); }
    if (!QRegularExpression(QStringLiteral("^[+-]?[0-9]+$")).match(value).hasMatch()) { return false; }
    integer = value.toLongLong(&valid, 10); if (!valid) { return false; }
    if (type == 0) { return integer >= 0 && integer <= 65535; } if (type == 1) { return integer >= 0 && integer <= 4294967295LL; } if (type == 2) { return integer >= -32768 && integer <= 32767; } return integer >= -2147483648LL && integer <= 2147483647LL;
}

/******************************************************************************
 * Function Name   : RecipeProperties::validate
 * Description     : Check visible scalar semantics and bounded retained matrix drafts without discarding inactive shrink data.
 * Input Parameters: error
 * Return Value    : True for a complete bounded recipe definition.
 * Date            : 2026.11.13
 ******************************************************************************/
bool RecipeProperties::validate(QString *error) const //20261113-2 add
{
    int row;
    int column;
    QString name;
    if (dataType < 0 || dataType > 4 || length < 1 || length > maximumLength(dataType) || count < 1 || count > 200 || description.size() > 32767 || writeAddress.size() > 1024 || readAddress.size() > 1024 || itemNames.size() < length || itemNames.size() > 200 || recipeNames.size() < count || recipeNames.size() > 200 || values.size() != itemNames.size()) { return recipeError(error, QStringLiteral("配方长度、总数、类型或保留表格大小无效。")); }
    Q_FOREACH (name, itemNames + recipeNames) { if (name.size() > 32767) { return recipeError(error, QStringLiteral("配方表头名称过长。")); } }
    for (row = 0; row < values.size(); ++row)
    {
        if (values[row].size() != recipeNames.size()) { return recipeError(error, QStringLiteral("配方数据矩阵列数不一致。")); }
        for (column = 0; column < values[row].size(); ++column) { if (values[row][column].size() > 128 || (row < length && column < count && !validValue(values[row][column], dataType))) { return recipeError(error, QStringLiteral("配方数据必须符合所选数据类型的有效范围。")); } }
    }
    return true;
}

/******************************************************************************
 * Function Name   : RecipeProperties::validateAddresses
 * Description     : Validate complete enabled read/write spans while retaining disabled address drafts.
 * Input Parameters: connections, error
 * Return Value    : True for every active complete transfer span.
 * Date            : 2026.11.13
 ******************************************************************************/
bool RecipeProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261113-2 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress address;
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; } context.connections = connections; context.role = PlcAddressRole::Word; context.transferCount = u32(length * wordSpan());
    if (writeEnabled && !parseQualifiedPlcAddressText(writeAddress, context, catalog, &address, error)) { return false; }
    if (readEnabled && !parseQualifiedPlcAddressText(sameReadWrite ? writeAddress : readAddress, context, catalog, &address, error)) { return false; } return true;
}

/******************************************************************************
 * Function Name   : RecipeProperties::toJson
 * Description     : Persist visible dimensions and all retained values and common header names.
 * Input Parameters: None
 * Return Value    : Exact typed recipe JSON.
 * Date            : 2026.11.13
 ******************************************************************************/
QJsonObject RecipeProperties::toJson() const //20261113-2 add
{
    QJsonArray data;
    QStringList row;
    Q_FOREACH (row, values) { data.append(QJsonArray::fromStringList(row)); }
    return {{QStringLiteral("description"), description}, {QStringLiteral("writeAddress"), writeAddress}, {QStringLiteral("readAddress"), readAddress}, {QStringLiteral("length"), length}, {QStringLiteral("count"), count}, {QStringLiteral("dataType"), dataType}, {QStringLiteral("writeEnabled"), writeEnabled}, {QStringLiteral("readEnabled"), readEnabled}, {QStringLiteral("sameReadWrite"), sameReadWrite}, {QStringLiteral("itemNames"), QJsonArray::fromStringList(itemNames)}, {QStringLiteral("recipeNames"), QJsonArray::fromStringList(recipeNames)}, {QStringLiteral("values"), data}};
}

/******************************************************************************
 * Function Name   : RecipeProperties::fromJson
 * Description     : Reject malformed keys, scalar types and nested matrix cells before replacing the output.
 * Input Parameters: json, output, error
 * Return Value    : True after exact complete recipe validation.
 * Date            : 2026.11.13
 ******************************************************************************/
bool RecipeProperties::fromJson(const QJsonObject &json, RecipeProperties *output, QString *error) //20261113-2 add
{
    RecipeProperties candidate;
    QJsonValue value;
    QJsonValue cell;
    QStringList row;
    QString key;
    if (output == nullptr || !recipeShape(json, candidate.toJson())) { return recipeError(error, QStringLiteral("配方字段缺失、未知或类型错误。")); }
    candidate.description = json.value(QStringLiteral("description")).toString();
    candidate.writeAddress = json.value(QStringLiteral("writeAddress")).toString();
    candidate.readAddress = json.value(QStringLiteral("readAddress")).toString();
    candidate.length = json.value(QStringLiteral("length")).toInt();
    candidate.count = json.value(QStringLiteral("count")).toInt();
    candidate.dataType = json.value(QStringLiteral("dataType")).toInt();
    candidate.writeEnabled = json.value(QStringLiteral("writeEnabled")).toBool();
    candidate.readEnabled = json.value(QStringLiteral("readEnabled")).toBool();
    candidate.sameReadWrite = json.value(QStringLiteral("sameReadWrite")).toBool();
    candidate.itemNames.clear(); candidate.recipeNames.clear(); candidate.values.clear();
    Q_FOREACH (key, (QStringList{QStringLiteral("itemNames"), QStringLiteral("recipeNames")}))
    {
        if (json.value(key).toArray().size() > 200) { return false; }
        Q_FOREACH (value, json.value(key).toArray()) { if (!value.isString()) { return false; } if (key == QStringLiteral("itemNames")) { candidate.itemNames.append(value.toString()); } else { candidate.recipeNames.append(value.toString()); } }
    }
    if (json.value(QStringLiteral("values")).toArray().size() > 200) { return false; }
    Q_FOREACH (value, json.value(QStringLiteral("values")).toArray()) { if (!value.isArray() || value.toArray().size() > 200) { return false; } row.clear(); Q_FOREACH (cell, value.toArray()) { if (!cell.isString()) { return false; } row.append(cell.toString()); } candidate.values.append(row); }
    if (!candidate.validate(error)) { return false; } *output = candidate; return true;
}
