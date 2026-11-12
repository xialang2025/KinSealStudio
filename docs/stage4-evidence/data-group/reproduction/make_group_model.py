"""Generate C26's single-channel typed model with separate sample and maximum-address drafts."""
from pathlib import Path
root=Path(__file__).resolve().parent/'worktree'
fields=[('QString','triggerAddress','QStringLiteral("LB0")'),('QString','readAddress','QStringLiteral("LW0")'),('int','dataType','0'),('int','sampleMode','0'),('int','samplePoints','100'),('QString','sampleAddress','QStringLiteral("LW0")'),('int','lineStyle','0'),('QString','channelColor','QStringLiteral("#ff0000")'),('QString','bottomColor','QStringLiteral("#0000ff")'),('QString','frameColor','QStringLiteral("#0000ff")'),('QString','backgroundColor','QStringLiteral("#ffffff")'),('int','rangeSource','0'),('QString','yMinimum','QStringLiteral("0")'),('QString','yMaximum','QStringLiteral("1000")'),('QString','maximumAddress','QStringLiteral("LW0")'),('int','totalDigits','4'),('int','fractionalDigits','0'),('bool','visibilityBitChosen','false')]
header='''#ifndef DATA_GROUP_PROPERTIES_H
#define DATA_GROUP_PROPERTIES_H
#include "trend_properties.h"

//20261113-1 add: A data group is one buffered curve; it has no history-collection or paired-X/Y identity.
struct DataGroupProperties
{
'''+''.join(f'    {typ} {name} = {value};\n' for typ,name,value in fields)+'''    TrendAxis xAxis;
    TrendAxis yAxis;
    StaticTextProperties visibility;
    static QStringList dataTypes();
    static u32 wordCount(int type);
    u32 readWordSpan() const;
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, DataGroupProperties *output, QString *error = nullptr);
};
#endif
'''
(root/'src/data_group_properties.h').write_text(header,encoding='utf-8',newline='\n')
def function(name,description,signature,body,inputs='None',returns='None.'):
 return f'/******************************************************************************\n * Function Name   : {name}\n * Description     : {description}\n * Input Parameters: {inputs}\n * Return Value    : {returns}\n * Date            : 2026.11.13\n ******************************************************************************/\n{signature} //20261113-1 add\n{{\n{body}\n}}\n\n'
s='#include "data_group_properties.h"\n#include "plc_native_catalog.h"\n#include <QColor>\n#include <cmath>\n\n'
s+=function('DataGroupProperties::dataTypes','Use the observed signed-first seven-type order rather than the older handbook order.','QStringList DataGroupProperties::dataTypes()','    return TrendProperties::dataTypes();',returns='Ordered native type labels.')
s+=function('DataGroupProperties::wordCount','Protect the complete scalar word width independently of unverified firmware byte ordering.','u32 DataGroupProperties::wordCount(int type)','    return TrendProperties::wordCount(type);',inputs='type',returns='One or two words.')
s+=function('DataGroupProperties::readWordSpan','Protect the entire declared point buffer in both sample modes; the runtime sample-address meaning remains separate.','u32 DataGroupProperties::readWordSpan() const','    return u32(samplePoints) * wordCount(dataType);',returns='Declared contiguous point-buffer word span.')
s+=function('groupScalar','Bound active display scalars while allowing the editor to retain an inactive maximum draft.','static bool groupScalar(const QString &text)','''    bool ok;
    double value = text.toDouble(&ok);
    return !text.isEmpty() && text.size() <= 80 && ok && std::isfinite(value) && std::abs(value) <= 3.402823466e38;
''',inputs='text',returns='True for a bounded finite scalar.')
s+=function('DataGroupProperties::validate','Enforce observed single-curve limits and active range semantics without inventing hidden channel fields.','bool DataGroupProperties::validate(QString *error) const','''    TrendAxis axis;
    QString color;
    QStringList colors = {channelColor, bottomColor, frameColor, backgroundColor};
    int index;
    if (dataType < 0 || dataType > 6 || sampleMode < 0 || sampleMode > 1 || samplePoints < 10 || samplePoints > 800 || lineStyle < 0 || lineStyle > 4 || rangeSource < 0 || rangeSource > 1 || totalDigits < 1 || totalDigits > 10 || fractionalDigits < 0 || fractionalDigits > 3 || triggerAddress.size() > 1024 || readAddress.size() > 1024 || sampleAddress.size() > 1024 || maximumAddress.size() > 1024 || yMinimum.size() > 80 || yMaximum.size() > 80 || (visibility.visibilityEnabled && visibility.bitEnabled && !visibilityBitChosen)) { if (error != nullptr) { *error = QStringLiteral("数据群组采样、范围、位数或显现状态无效。"); } return false; }
    for (index = 0; index < 2; ++index) { axis = index == 0 ? xAxis : yAxis; if (axis.major < 3 || axis.major > 10 || axis.minor < 1 || axis.minor > 3 || !QList<int>({12, 16, 24, 48}).contains(axis.fontSize)) { return false; } colors.append(axis.axisColor); colors.append(axis.gridColor); }
    Q_FOREACH (color, colors) { if (color.size() > 32 || !QColor(color).isValid()) { return false; } }
    if (!groupScalar(yMinimum) || (rangeSource == 0 && (!groupScalar(yMaximum) || yMinimum.toDouble() >= yMaximum.toDouble()))) { if (error != nullptr) { *error = QStringLiteral("本版要求活动常量范围有效，最小值小于最大值。"); } return false; }
    return visibility.validate(error);
''',inputs='error',returns='True for the supported editor slice.')
s+=function('DataGroupProperties::validateAddresses','Validate the declared buffer span and only the active sample and maximum source carriers.','bool DataGroupProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const','''    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress parsed;
    ScreenButtonProperties shared;
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    context.connections = connections; context.role = PlcAddressRole::Word; context.transferCount = readWordSpan();
    if (!parseQualifiedPlcAddressText(readAddress, context, catalog, &parsed, error)) { return false; }
    context.transferCount = 1U;
    if (sampleMode == 1 && !parseQualifiedPlcAddressText(sampleAddress, context, catalog, &parsed, error)) { return false; }
    context.transferCount = wordCount(dataType);
    if (rangeSource == 1 && !parseQualifiedPlcAddressText(maximumAddress, context, catalog, &parsed, error)) { return false; }
    context.role = PlcAddressRole::Bit; context.transferCount = 1U;
    if (!parseQualifiedPlcAddressText(triggerAddress, context, catalog, &parsed, error)) { return false; }
    shared.common.indicator = false; shared.common.visibility = visibility; return shared.validateAddresses(connections, error);
''',inputs='connections, error',returns='True for all active editor carriers.')
body='    QJsonObject json;\n'+''.join(f'    json.insert(QStringLiteral("{name}"), {name});\n' for typ,name,value in fields)+'    json.insert(QStringLiteral("xAxis"), xAxis.toJson()); json.insert(QStringLiteral("yAxis"), yAxis.toJson()); json.insert(QStringLiteral("visibility"), visibility.toJson()); return json;'
s+=function('DataGroupProperties::toJson','Persist active and inactive drafts as explicit typed editor data.','QJsonObject DataGroupProperties::toJson() const',body,returns='Complete data-group JSON record.')
body='''    DataGroupProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator field;
    double number;
    if (output == nullptr || json.keys() != shape.keys()) { if (error != nullptr) { *error = QStringLiteral("数据群组字段缺失或未知。"); } return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != shape.value(field.key()).type()) { return false; }
        if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 65535) { return false; } }
        if (field.value().isString() && field.value().toString().size() > 1024) { return false; }
    }
'''+''.join(f'    candidate.{name} = json.value(QStringLiteral("{name}")).to'+{'QString':'String','int':'Int','bool':'Bool'}[typ]+'();\n' for typ,name,value in fields)+'''    if (!TrendAxis::fromJson(json.value(QStringLiteral("xAxis")).toObject(), &candidate.xAxis, error) || !TrendAxis::fromJson(json.value(QStringLiteral("yAxis")).toObject(), &candidate.yAxis, error) || !StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() || !candidate.validate(error)) { return false; }
    *output = candidate; return true;
'''
s+=function('DataGroupProperties::fromJson','Reject incomplete, unknown and mistyped fields before replacing the caller state.','bool DataGroupProperties::fromJson(const QJsonObject &json, DataGroupProperties *output, QString *error)',body,inputs='json, output, error',returns='True for an exact validated record.')
(root/'src/data_group_properties.cpp').write_text(s,encoding='utf-8',newline='\n')
print('Created independent C26 single-channel properties and active-only source validation.')
