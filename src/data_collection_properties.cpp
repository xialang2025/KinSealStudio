#include "data_collection_properties.h"
#include "plc_native_catalog.h"
#include <QJsonArray>
#include <QSet>
#include <QUuid>
#include <cmath>
#include <limits>

/******************************************************************************
 * Function Name   : DataCollectionProperties::DataCollectionProperties
 * Description     : Seed one named cache item without inventing native source or device identifiers.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
DataCollectionProperties::DataCollectionProperties() //20261112-1 add
{
    items.append(createItem(0));
}

/******************************************************************************
 * Function Name   : DataCollectionProperties::createItem
 * Description     : Assign a stable editor identity independently of the read-only native LH index and display name.
 * Input Parameters: index
 * Return Value    : Fresh cache item.
 * Date            : 2026.11.12
 ******************************************************************************/
DataCollectionItem DataCollectionProperties::createItem(int index) //20261112-1 add
{
    DataCollectionItem item;
    item.id = QStringLiteral("collection-item-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)); item.name = QStringLiteral("data_%1").arg(index); item.cacheIndex = index; return item;
}

/******************************************************************************
 * Function Name   : DataCollectionProperties::dataTypes
 * Description     : Preserve the five observed collection-wide data types, excluding chart-only BCD choices.
 * Input Parameters: None
 * Return Value    : Ordered native labels.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList DataCollectionProperties::dataTypes() //20261112-1 add
{
    return {QStringLiteral("16位正整数"), QStringLiteral("32位正整数"), QStringLiteral("16位整数"), QStringLiteral("32位整数"), QStringLiteral("32位浮点数")};
}

/******************************************************************************
 * Function Name   : DataCollectionProperties::displayTypes
 * Description     : Keep the native decimal, hexadecimal and octal display order.
 * Input Parameters: None
 * Return Value    : Three format labels.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList DataCollectionProperties::displayTypes() //20261112-1 add
{
    return {QStringLiteral("十进制"), QStringLiteral("十六进制"), QStringLiteral("八进制")};
}

/******************************************************************************
 * Function Name   : DataCollectionProperties::wordCount
 * Description     : Map the shared scalar type to the full monitored word span of one sampled value.
 * Input Parameters: None
 * Return Value    : One or two words.
 * Date            : 2026.11.12
 ******************************************************************************/
u32 DataCollectionProperties::wordCount() const //20261112-1 add
{
    return dataType == 0 || dataType == 2 ? 1U : 2U;
}

/******************************************************************************
 * Function Name   : DataCollectionProperties::memoryBytes
 * Description     : Apply the observed common-type memory formula with bounded operands and an invalid sentinel.
 * Input Parameters: None
 * Return Value    : Required bytes or the largest u32 for invalid operands.
 * Date            : 2026.11.12
 ******************************************************************************/
u32 DataCollectionProperties::memoryBytes() const //20261112-1 add
{
    if (dataType < 0 || dataType > 4 || sampleLength < 1 || sampleLength > 32 || sampleCount < 1 || sampleCount > 65535) { return std::numeric_limits<u32>::max(); }
    return (20U + u32(sampleLength) * wordCount() * 2U) * u32(sampleCount);
}

/******************************************************************************
 * Function Name   : DataCollectionProperties::validate
 * Description     : Enforce measured UI limits, stable item identity and a stated strict 63-KiB editor guard.
 * Input Parameters: error
 * Return Value    : True for the declared collection editor slice.
 * Date            : 2026.11.12
 ******************************************************************************/
bool DataCollectionProperties::validate(QString *error) const //20261112-1 add
{
    QSet<QString> ids;
    QSet<QString> names;
    DataCollectionItem item;
    int index = 0;
    bool gainOk;
    bool offsetOk;
    double gain;
    double offset;
    if (dataType < 0 || dataType > 4 || sampleLength < 1 || sampleLength > 32 || sampleCount < 1 || sampleCount > 65535 || samplingMode < 0 || samplingMode > 2 || interval < 1 || interval > 3600 || triggerMinute < 1 || triggerMinute > 255 || fullPercent < 0 || fullPercent > 100 || items.size() != sampleLength || readAddress.size() > 1024 || triggerAddress.size() > 1024 || noticeAddress.size() > 1024) { if (error != nullptr) { *error = QStringLiteral("数据采集取样、方式、数据项或通知字段无效。"); } return false; }
    if (memoryBytes() > 63U * 1024U) { if (error != nullptr) { *error = QStringLiteral("本版采集内存不得超过63 kbyte。"); } return false; }
    Q_FOREACH (item, items)
    {
        if (item.id.trimmed().isEmpty() || item.id.size() > 128 || ids.contains(item.id) || item.name.trimmed().isEmpty() || item.name.size() > 48 || names.contains(item.name) || item.cacheIndex != index++ || item.displayType < 0 || item.displayType > 2 || item.digits < 1 || item.digits > 10 || item.fractionalDigits < 0 || item.fractionalDigits > 9 || item.gain.size() > 80 || item.offset.size() > 80) { if (error != nullptr) { *error = QStringLiteral("采集数据项名称、标识、格式或LH顺序无效。"); } return false; }
        ids.insert(item.id); names.insert(item.name); gain = item.gain.toDouble(&gainOk); offset = item.offset.toDouble(&offsetOk);
        if (item.scaled && (!gainOk || !offsetOk || !std::isfinite(gain) || !std::isfinite(offset))) { if (error != nullptr) { *error = QStringLiteral("请输入有效的缩放增益和偏移量。"); } return false; }
    }
    return true;
}

/******************************************************************************
 * Function Name   : DataCollectionProperties::validateAddresses
 * Description     : Validate the whole sampled word span and only the active trigger/notification bit carriers.
 * Input Parameters: connections, error
 * Return Value    : True for every active carrier.
 * Date            : 2026.11.12
 ******************************************************************************/
bool DataCollectionProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261112-1 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress parsed;
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    context.connections = connections; context.role = PlcAddressRole::Word; context.transferCount = wordCount() * u32(sampleLength);
    if (!parseQualifiedPlcAddressText(readAddress, context, catalog, &parsed, error)) { return false; }
    context.role = PlcAddressRole::Bit; context.transferCount = 1U;
    if (samplingMode == 1 && !parseQualifiedPlcAddressText(triggerAddress, context, catalog, &parsed, error)) { return false; }
    if (notice && !parseQualifiedPlcAddressText(noticeAddress, context, catalog, &parsed, error)) { return false; }
    return true;
}

/******************************************************************************
 * Function Name   : DataCollectionItem::toJson
 * Description     : Preserve exact editor drafts without projecting a native collector binary layout.
 * Input Parameters: None
 * Return Value    : Complete typed JSON record.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject DataCollectionItem::toJson() const //20261112-1 add
{
    QJsonObject json;
    json.insert(QStringLiteral("id"), id);
    json.insert(QStringLiteral("name"), name);
    json.insert(QStringLiteral("cacheIndex"), cacheIndex);
    json.insert(QStringLiteral("displayType"), displayType);
    json.insert(QStringLiteral("digits"), digits);
    json.insert(QStringLiteral("fractionalDigits"), fractionalDigits);
    json.insert(QStringLiteral("scaled"), scaled);
    json.insert(QStringLiteral("gain"), gain);
    json.insert(QStringLiteral("offset"), offset);
    return json;
}

/******************************************************************************
 * Function Name   : DataCollectionItem::fromJson
 * Description     : Reject unknown, missing, fractional or mistyped fields atomically.
 * Input Parameters: json, output, error
 * Return Value    : True for a complete supported record.
 * Date            : 2026.11.12
 ******************************************************************************/
bool DataCollectionItem::fromJson(const QJsonObject &json, DataCollectionItem *output, QString *error) //20261112-1 add
{
    DataCollectionItem candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator field;
    double number;
    if (output == nullptr || json.keys() != shape.keys()) { if (error != nullptr) { *error = QStringLiteral("数据采集字段缺失或未知。"); } return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != shape.value(field.key()).type()) { return false; }
        if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 65535) { return false; } }
        if (field.value().isString() && field.value().toString().size() > 1024) { return false; }
    }
    candidate.id = json.value(QStringLiteral("id")).toString();
    candidate.name = json.value(QStringLiteral("name")).toString();
    candidate.cacheIndex = json.value(QStringLiteral("cacheIndex")).toInt();
    candidate.displayType = json.value(QStringLiteral("displayType")).toInt();
    candidate.digits = json.value(QStringLiteral("digits")).toInt();
    candidate.fractionalDigits = json.value(QStringLiteral("fractionalDigits")).toInt();
    candidate.scaled = json.value(QStringLiteral("scaled")).toBool();
    candidate.gain = json.value(QStringLiteral("gain")).toString();
    candidate.offset = json.value(QStringLiteral("offset")).toString();
    *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : DataCollectionProperties::toJson
 * Description     : Preserve exact editor drafts without projecting a native collector binary layout.
 * Input Parameters: None
 * Return Value    : Complete typed JSON record.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject DataCollectionProperties::toJson() const //20261112-1 add
{
    QJsonObject json;
    QJsonArray rows;
    DataCollectionItem item;
    Q_FOREACH (item, items) { rows.append(item.toJson()); }
    json.insert(QStringLiteral("items"), rows);
    json.insert(QStringLiteral("upload"), upload);
    json.insert(QStringLiteral("sampleLength"), sampleLength);
    json.insert(QStringLiteral("sampleCount"), sampleCount);
    json.insert(QStringLiteral("dataType"), dataType);
    json.insert(QStringLiteral("readAddress"), readAddress);
    json.insert(QStringLiteral("samplingMode"), samplingMode);
    json.insert(QStringLiteral("interval"), interval);
    json.insert(QStringLiteral("triggerAddress"), triggerAddress);
    json.insert(QStringLiteral("triggerMinute"), triggerMinute);
    json.insert(QStringLiteral("fullStop"), fullStop);
    json.insert(QStringLiteral("notice"), notice);
    json.insert(QStringLiteral("noticeAddress"), noticeAddress);
    json.insert(QStringLiteral("fullPercent"), fullPercent);
    return json;
}

/******************************************************************************
 * Function Name   : DataCollectionProperties::fromJson
 * Description     : Reject unknown, missing, fractional or mistyped fields atomically.
 * Input Parameters: json, output, error
 * Return Value    : True for a complete supported record.
 * Date            : 2026.11.12
 ******************************************************************************/
bool DataCollectionProperties::fromJson(const QJsonObject &json, DataCollectionProperties *output, QString *error) //20261112-1 add
{
    DataCollectionProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator field;
    double number;
    QJsonArray rows;
    QJsonValue value;
    DataCollectionItem item;
    if (output == nullptr || json.keys() != shape.keys()) { if (error != nullptr) { *error = QStringLiteral("数据采集字段缺失或未知。"); } return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != shape.value(field.key()).type()) { return false; }
        if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 65535) { return false; } }
        if (field.value().isString() && field.value().toString().size() > 1024) { return false; }
    }
    candidate.upload = json.value(QStringLiteral("upload")).toBool();
    candidate.sampleLength = json.value(QStringLiteral("sampleLength")).toInt();
    candidate.sampleCount = json.value(QStringLiteral("sampleCount")).toInt();
    candidate.dataType = json.value(QStringLiteral("dataType")).toInt();
    candidate.readAddress = json.value(QStringLiteral("readAddress")).toString();
    candidate.samplingMode = json.value(QStringLiteral("samplingMode")).toInt();
    candidate.interval = json.value(QStringLiteral("interval")).toInt();
    candidate.triggerAddress = json.value(QStringLiteral("triggerAddress")).toString();
    candidate.triggerMinute = json.value(QStringLiteral("triggerMinute")).toInt();
    candidate.fullStop = json.value(QStringLiteral("fullStop")).toBool();
    candidate.notice = json.value(QStringLiteral("notice")).toBool();
    candidate.noticeAddress = json.value(QStringLiteral("noticeAddress")).toString();
    candidate.fullPercent = json.value(QStringLiteral("fullPercent")).toInt();
    rows = json.value(QStringLiteral("items")).toArray(); if (rows.isEmpty() || rows.size() > 32) { return false; }
    candidate.items.clear(); Q_FOREACH (value, rows) { if (!value.isObject() || !DataCollectionItem::fromJson(value.toObject(), &item, error)) { return false; } candidate.items.append(item); }
    if (!candidate.validate(error)) { return false; }
    *output = candidate; return true;
}

