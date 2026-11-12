"""Generate distinct recipe definitions and display drafts from C31's observed contract."""
from pathlib import Path
root=Path(__file__).resolve().parent/'worktree'
def put(path,text):(root/path).write_text(text,encoding='utf-8',newline='\n')
def f(name,desc,signature,body,params='None',returns='None.'):
    return f'\n/******************************************************************************\n * Function Name   : {name}\n * Description     : {desc}\n * Input Parameters: {params}\n * Return Value    : {returns}\n * Date            : 2026.11.13\n ******************************************************************************/\n{signature} //20261113-2 add\n{{\n{body}\n}}\n'
put('src/recipe_properties.h','''#ifndef RECIPE_PROPERTIES_H
#define RECIPE_PROPERTIES_H
#include "plc_address.h"
//20261113-2 add: Visible dimensions are independent of retained shrink/regrow drafts.
struct RecipeProperties
{
    QString description;
    int length = 4;
    int count = 1;
    int dataType = 0;
    bool writeEnabled = false;
    bool readEnabled = false;
    bool sameReadWrite = true;
    QString writeAddress = QStringLiteral("LW0");
    QString readAddress = QStringLiteral("LW0");
    QStringList itemNames;
    QStringList recipeNames;
    QVector<QStringList> values;
    RecipeProperties();
    static QStringList dataTypes();
    static int maximumLength(int type);
    int wordSpan() const;
    void resizeTable(int rows, int columns);
    void changeType(int type);
    void copyValues(int source, int target);
    static bool validValue(const QString &value, int type);
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, RecipeProperties *output, QString *error = nullptr);
};
#endif
''')
s='''#include "recipe_properties.h"
#include "data_collection_properties.h"
#include <QJsonArray>
#include <QLocale>
#include <QRegularExpression>
#include <cmath>
'''
s+=f('recipeError','Report a typed recipe failure without replacing caller state.','static bool recipeError(QString *error, const QString &message)','    if (error != nullptr) { *error = message; } return false;','error, message','False.')
s+=f('recipeShape','Require exact field types and lossless bounded integral dimensions.','static bool recipeShape(const QJsonObject &json, const QJsonObject &shape)', '''    QJsonObject::const_iterator field;
    double number;
    if (json.keys() != shape.keys()) { return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field) { if (field.value().type() != shape.value(field.key()).type()) { return false; } if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || number < 0 || number > 200 || number != std::floor(number)) { return false; } } }
    return true;''','json, shape','True for an exact recipe record shape.')
s+=f('RecipeProperties::RecipeProperties','Seed the measured four-item, one-recipe table and common header names.','RecipeProperties::RecipeProperties()', '    resizeTable(4, 1);')
s+=f('RecipeProperties::dataTypes','Keep the observed five scalar types in native order.','QStringList RecipeProperties::dataTypes()', '    return DataCollectionProperties::dataTypes();',returns='Ordered type labels.')
s+=f('RecipeProperties::maximumLength','Apply the measured 200-row sixteen-bit and 99-row thirty-two-bit editor limits.','int RecipeProperties::maximumLength(int type)', '    return type == 0 || type == 2 ? 200 : 99;','type','Measured active row limit.')
s+=f('RecipeProperties::wordSpan','Resolve the complete scalar width for active recipe transfers and readonly range labels.','int RecipeProperties::wordSpan() const','    return dataType == 0 || dataType == 2 ? 1 : 2;',returns='Words per scalar.')
s+=f('RecipeProperties::resizeTable','Retain hidden values and names on shrink and rebuild exposed default item-address labels on regrowth.','void RecipeProperties::resizeTable(int rows, int columns)', '''    int index;
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
    length = rows; count = columns;''','rows, columns')
s+=f('RecipeProperties::changeType','Retain existing scalar drafts and headers while clamping only the active row count.','void RecipeProperties::changeType(int type)', '''    if (type < 0 || type > 4) { return; } dataType = type; if (length > maximumLength(type)) { length = maximumLength(type); }''','type')
s+=f('RecipeProperties::copyValues','Copy active values from the zero-based source recipe without copying either header set.','void RecipeProperties::copyValues(int source, int target)', '''    int row;
    if (source < 0 || source >= count || target < 0 || target >= count) { return; }
    for (row = 0; row < length; ++row) { values[row][target] = values[row][source]; }''','source, target')
s+=f('RecipeProperties::validValue','Require a finite representable decimal scalar instead of silently truncating recipe data.','bool RecipeProperties::validValue(const QString &value, int type)', '''    bool valid;
    qlonglong integer;
    double real;
    if (value.isEmpty() || value.size() > 128 || value != value.trimmed() || type < 0 || type > 4) { return false; }
    if (type == 4) { real = QLocale::c().toDouble(value, &valid); return valid && !value.contains(QLatin1Char(',')) && std::isfinite(real) && std::isfinite(float(real)); }
    if (!QRegularExpression(QStringLiteral("^[+-]?[0-9]+$")).match(value).hasMatch()) { return false; }
    integer = value.toLongLong(&valid, 10); if (!valid) { return false; }
    if (type == 0) { return integer >= 0 && integer <= 65535; } if (type == 1) { return integer >= 0 && integer <= 4294967295LL; } if (type == 2) { return integer >= -32768 && integer <= 32767; } return integer >= -2147483648LL && integer <= 2147483647LL;''','value, type','True for a finite scalar of the selected type.')
s+=f('RecipeProperties::validate','Check visible scalar semantics and bounded retained matrix drafts without discarding inactive shrink data.','bool RecipeProperties::validate(QString *error) const', '''    int row;
    int column;
    QString name;
    if (dataType < 0 || dataType > 4 || length < 1 || length > maximumLength(dataType) || count < 1 || count > 200 || description.size() > 32767 || writeAddress.size() > 1024 || readAddress.size() > 1024 || itemNames.size() < length || itemNames.size() > 200 || recipeNames.size() < count || recipeNames.size() > 200 || values.size() != itemNames.size()) { return recipeError(error, QStringLiteral("配方长度、总数、类型或保留表格大小无效。")); }
    Q_FOREACH (name, itemNames + recipeNames) { if (name.size() > 32767) { return recipeError(error, QStringLiteral("配方表头名称过长。")); } }
    for (row = 0; row < values.size(); ++row)
    {
        if (values[row].size() != recipeNames.size()) { return recipeError(error, QStringLiteral("配方数据矩阵列数不一致。")); }
        for (column = 0; column < values[row].size(); ++column) { if (values[row][column].size() > 128 || (row < length && column < count && !validValue(values[row][column], dataType))) { return recipeError(error, QStringLiteral("配方数据必须符合所选数据类型的有效范围。")); } }
    }
    return true;''','error','True for a complete bounded recipe definition.')
s+=f('RecipeProperties::validateAddresses','Validate complete enabled read/write spans while retaining disabled address drafts.','bool RecipeProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const', '''    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress address;
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; } context.connections = connections; context.role = PlcAddressRole::Word; context.transferCount = u32(length * wordSpan());
    if (writeEnabled && !parseQualifiedPlcAddressText(writeAddress, context, catalog, &address, error)) { return false; }
    if (readEnabled && !parseQualifiedPlcAddressText(sameReadWrite ? writeAddress : readAddress, context, catalog, &address, error)) { return false; } return true;''','connections, error','True for every active complete transfer span.')
strings=['description','writeAddress','readAddress'];ints=['length','count','dataType'];bools=['writeEnabled','readEnabled','sameReadWrite']
s+=f('RecipeProperties::toJson','Persist visible dimensions and all retained values and common header names.','QJsonObject RecipeProperties::toJson() const', '''    QJsonArray data;
    QStringList row;
    Q_FOREACH (row, values) { data.append(QJsonArray::fromStringList(row)); }
    return {'''+', '.join('{QStringLiteral("'+k+'"), '+k+'}' for k in strings+ints+bools)+''', {QStringLiteral("itemNames"), QJsonArray::fromStringList(itemNames)}, {QStringLiteral("recipeNames"), QJsonArray::fromStringList(recipeNames)}, {QStringLiteral("values"), data}};''',returns='Exact typed recipe JSON.')
s+=f('RecipeProperties::fromJson','Reject malformed keys, scalar types and nested matrix cells before replacing the output.','bool RecipeProperties::fromJson(const QJsonObject &json, RecipeProperties *output, QString *error)', '''    RecipeProperties candidate;
    QJsonValue value;
    QJsonValue cell;
    QStringList row;
    QString key;
    if (output == nullptr || !recipeShape(json, candidate.toJson())) { return recipeError(error, QStringLiteral("配方字段缺失、未知或类型错误。")); }
'''+''.join(f'    candidate.{k} = json.value(QStringLiteral("{k}")).to{kind}();\n' for keys,kind in [(strings,'String'),(ints,'Int'),(bools,'Bool')] for k in keys)+'''    candidate.itemNames.clear(); candidate.recipeNames.clear(); candidate.values.clear();
    Q_FOREACH (key, (QStringList{QStringLiteral("itemNames"), QStringLiteral("recipeNames")}))
    {
        if (json.value(key).toArray().size() > 200) { return false; }
        Q_FOREACH (value, json.value(key).toArray()) { if (!value.isString()) { return false; } if (key == QStringLiteral("itemNames")) { candidate.itemNames.append(value.toString()); } else { candidate.recipeNames.append(value.toString()); } }
    }
    if (json.value(QStringLiteral("values")).toArray().size() > 200) { return false; }
    Q_FOREACH (value, json.value(QStringLiteral("values")).toArray()) { if (!value.isArray() || value.toArray().size() > 200) { return false; } row.clear(); Q_FOREACH (cell, value.toArray()) { if (!cell.isString()) { return false; } row.append(cell.toString()); } candidate.values.append(row); }
    if (!candidate.validate(error)) { return false; } *output = candidate; return true;''','json, output, error','True after exact complete recipe validation.')
put('src/recipe_properties.cpp',s)
print('Created bounded recipe definition with common names, full transfer spans and retained shrink/regrow values.')
