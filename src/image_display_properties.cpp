#include "image_display_properties.h"
#include "bit_button_properties.h"
#include <QFile>
#include <QJsonArray>
#include <cmath>

/******************************************************************************
 * Function Name   : ImageDisplayState::toJson
 * Description     : Persist both resource identity and embedded PNG bytes for deterministic copy and detached-image handling.
 * Input Parameters: None
 * Return Value    : One picture row.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject ImageDisplayState::toJson() const //20261112-0 add
{
    return {{QStringLiteral("picture"), picture}, {QStringLiteral("picturePng"), QString::fromLatin1(picturePng.toBase64())}};
}

/******************************************************************************
 * Function Name   : ImageDisplayProperties::setStateCount
 * Description     : Discard removed rows and reconstruct newly added states empty, matching the measured native lifetime.
 * Input Parameters: count
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void ImageDisplayProperties::setStateCount(int count) //20261112-0 add
{
    stateCount = qBound(1, count, 128); states.resize(stateCount);
}

/******************************************************************************
 * Function Name   : ImageDisplayProperties::toJson
 * Description     : Serialize every picture condition, state and inactive permission draft in the editor schema.
 * Input Parameters: None
 * Return Value    : Independent image-display record.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject ImageDisplayProperties::toJson() const //20261112-0 add
{
    QJsonArray rows;
    ImageDisplayState state;
    Q_FOREACH (state, states) { rows.append(state.toJson()); }
    return {{QStringLiteral("displayType"), displayType}, {QStringLiteral("conditionEnabled"), conditionEnabled},
            {QStringLiteral("condition"), condition}, {QStringLiteral("stateCount"), stateCount}, {QStringLiteral("monitorAddress"), monitorAddress},
            {QStringLiteral("interval"), interval}, {QStringLiteral("states"), rows}, {QStringLiteral("visibility"), visibility.toJson()}};
}

/******************************************************************************
 * Function Name   : ImageDisplayProperties::fromJson
 * Description     : Validate the complete state table and canonical permission fields before replacing the caller's value.
 * Input Parameters: json, output, error
 * Return Value    : True for a complete, bounded image-display record.
 * Date            : 2026.11.12
 ******************************************************************************/
bool ImageDisplayProperties::fromJson(const QJsonObject &json, ImageDisplayProperties *output, QString *error) //20261112-0 add
{
    ImageDisplayProperties candidate;
    QJsonObject expected = candidate.toJson();
    QJsonObject::const_iterator item;
    QJsonArray rows;
    QJsonValue row;
    ImageDisplayState state;
    QByteArray::FromBase64Result decoded;
    double number;
    if (output == nullptr || json.keys() != expected.keys())
    { if (error != nullptr) { *error = QStringLiteral("图片显示字段缺失、未知或接收位置为空。"); } return false; }
    for (item = json.constBegin(); item != json.constEnd(); ++item)
    {
        if (item.value().type() != expected.value(item.key()).type())
        { if (error != nullptr) { *error = QStringLiteral("图片显示字段类型错误。"); } return false; }
        if (item.value().isDouble())
        {
            number = item.value().toDouble();
            if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 1000)
            { if (error != nullptr) { *error = QStringLiteral("图片显示数值字段必须为有界整数。"); } return false; }
        }
    }
    candidate.displayType = json.value(QStringLiteral("displayType")).toInt(); candidate.conditionEnabled = json.value(QStringLiteral("conditionEnabled")).toBool();
    candidate.condition = json.value(QStringLiteral("condition")).toInt(); candidate.stateCount = json.value(QStringLiteral("stateCount")).toInt();
    candidate.monitorAddress = json.value(QStringLiteral("monitorAddress")).toString(); candidate.interval = json.value(QStringLiteral("interval")).toInt();
    if (!StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject())
    { if (error != nullptr) { *error = QStringLiteral("图片显现字段无效、缺失或含未知字段。"); } return false; }
    rows = json.value(QStringLiteral("states")).toArray();
    if (rows.size() < 1 || rows.size() > 128) { if (error != nullptr) { *error = QStringLiteral("图片状态必须为1..128行。"); } return false; }
    candidate.states.clear();
    Q_FOREACH (row, rows)
    {
        if (!row.isObject() || row.toObject().keys() != state.toJson().keys() || !row.toObject().value(QStringLiteral("picture")).isString() || !row.toObject().value(QStringLiteral("picturePng")).isString() || row.toObject().value(QStringLiteral("picturePng")).toString().size() > 24 * 1024 * 1024)
        { if (error != nullptr) { *error = QStringLiteral("图片状态含未知或超限字段。"); } return false; }
        state.picture = row.toObject().value(QStringLiteral("picture")).toString();
        decoded = QByteArray::fromBase64Encoding(row.toObject().value(QStringLiteral("picturePng")).toString().toLatin1(), QByteArray::AbortOnBase64DecodingErrors);
        if (!decoded) { if (error != nullptr) { *error = QStringLiteral("图片状态PNG的Base64无效。"); } return false; }
        state.picturePng = decoded.decoded; candidate.states.append(state);
    }
    if (!candidate.validate(error)) { return false; }
    *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : ImageDisplayProperties::validate
 * Description     : Enforce measured mode/count/interval limits and require an actual image in every active state.
 * Input Parameters: error
 * Return Value    : True when the picture table can be accepted or saved.
 * Date            : 2026.11.12
 ******************************************************************************/
bool ImageDisplayProperties::validate(QString *error) const //20261112-0 add
{
    ImageDisplayState state;
    if (displayType < 0 || displayType > 1 || condition < 0 || condition > 2 || stateCount < 1 || stateCount > 128 ||
        states.size() != stateCount || interval < 1 || interval > 1000 || monitorAddress.size() > 256 ||
        (displayType == 0 && stateCount != 1) || (displayType == 1 && (stateCount < 2 || !conditionEnabled || (condition == 2 && stateCount != 2))))
    { if (error != nullptr) { *error = QStringLiteral("图片显示模式、状态数或间隔超出已核定范围。"); } return false; }
    Q_FOREACH (state, states)
    {
        if (state.picture.isEmpty() || state.picture.size() > 1024 ||
            (state.picturePng.isEmpty() ? !state.picture.startsWith(QStringLiteral(":/system-gallery/")) || !QFile::exists(state.picture) : !validateButtonImage(state.picturePng)))
        { if (error != nullptr) { *error = QStringLiteral("状态没有选择图片!!"); } return false; }
    }
    return visibility.validate(error);
}

/******************************************************************************
 * Function Name   : ImageDisplayProperties::validateAddresses
 * Description     : Validate only the active condition and visibility reads, preserving disabled address drafts verbatim.
 * Input Parameters: connections, error
 * Return Value    : True for address families backed by the project connections.
 * Date            : 2026.11.12
 ******************************************************************************/
bool ImageDisplayProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261112-0 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress address;
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    context.connections = connections; context.transferCount = 1;
    if (conditionEnabled && condition != 0)
    {
        context.role = condition == 2 ? PlcAddressRole::Bit : PlcAddressRole::Word;
        if (!parseQualifiedPlcAddressText(monitorAddress, context, catalog, &address, error)) { return false; }
    }
    if (visibility.visibilityEnabled && visibility.bitEnabled)
    {
        context.role = PlcAddressRole::Bit;
        if (!parseQualifiedPlcAddressText(visibility.bitAddress, context, catalog, &address, error)) { return false; }
    }
    if (visibility.visibilityEnabled && visibility.wordEnabled)
    {
        context.role = PlcAddressRole::Word; context.transferCount = visibility.dataType == 0 || visibility.dataType == 2 ? 1U : 2U;
        if (!parseQualifiedPlcAddressText(visibility.wordAddress, context, catalog, &address, error)) { return false; }
    }
    return true;
}
