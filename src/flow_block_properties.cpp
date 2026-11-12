#include "flow_block_properties.h"
#include "plc_native_catalog.h"
#include <cmath>

/******************************************************************************
 * Function Name   : FlowBlockProperties::patterns
 * Description     : Preserve all thirteen observed labels, including the corrected left-upper radial entry.
 * Input Parameters: None
 * Return Value    : Ordered pattern labels.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList FlowBlockProperties::patterns() //20261112-0 add
{
    return {QStringLiteral("透明"), QStringLiteral("纯色"), QStringLiteral("横向过渡"), QStringLiteral("横向对称过渡"), QStringLiteral("纵向过渡"), QStringLiteral("纵向对称过渡"), QStringLiteral("斜上过渡"), QStringLiteral("斜上对称过渡"), QStringLiteral("斜下过渡"), QStringLiteral("斜下对称过渡"), QStringLiteral("右上角辐射"), QStringLiteral("左上角辐射"), QStringLiteral("中心辐射")};
}

/******************************************************************************
 * Function Name   : FlowBlockProperties::directions
 * Description     : Resolve the two native direction labels for the active orientation.
 * Input Parameters: None
 * Return Value    : Left/right or up/down labels.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList FlowBlockProperties::directions() const //20261112-0 add
{
    return vertical ? QStringList{QStringLiteral("向上"), QStringLiteral("向下")} : QStringList{QStringLiteral("向左"), QStringLiteral("向右")};
}

/******************************************************************************
 * Function Name   : FlowBlockProperties::setVertical
 * Description     : Reset direction and pattern on an actual orientation change, as measured with nondefault direction/style drafts.
 * Input Parameters: value
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void FlowBlockProperties::setVertical(bool value) //20261112-0 add
{
    if (value == vertical) { return; }
    vertical = value; direction = 0; pattern = vertical ? 3 : 5;
}

/******************************************************************************
 * Function Name   : FlowBlockProperties::toJson
 * Description     : Persist exact colors, orientation, trigger and visibility drafts independently of device records.
 * Input Parameters: None
 * Return Value    : Complete typed flow JSON.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject FlowBlockProperties::toJson() const //20261112-0 add
{
    return {{QStringLiteral("triggerAddress"), triggerAddress}, {QStringLiteral("flowFront"), flowFront}, {QStringLiteral("flowBack"), flowBack}, {QStringLiteral("pipeFront"), pipeFront}, {QStringLiteral("pipeBack"), pipeBack}, {QStringLiteral("borderColor"), borderColor}, {QStringLiteral("pattern"), pattern}, {QStringLiteral("direction"), direction}, {QStringLiteral("count"), count}, {QStringLiteral("speed"), speed}, {QStringLiteral("vertical"), vertical}, {QStringLiteral("noBorder"), noBorder}, {QStringLiteral("visibilityBitChosen"), visibilityBitChosen}, {QStringLiteral("visibility"), visibility.toJson()}};
}

/******************************************************************************
 * Function Name   : FlowBlockProperties::fromJson
 * Description     : Reject unknown, missing, fractional or lossy records before replacing the destination model.
 * Input Parameters: json, output, error
 * Return Value    : True after strict complete validation.
 * Date            : 2026.11.12
 ******************************************************************************/
bool FlowBlockProperties::fromJson(const QJsonObject &json, FlowBlockProperties *output, QString *error) //20261112-0 add
{
    FlowBlockProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator field;
    double number;
    if (output == nullptr || json.keys() != shape.keys()) { if (error != nullptr) { *error = QStringLiteral("流动块字段缺失或未知。"); } return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != shape.value(field.key()).type()) { if (error != nullptr) { *error = QStringLiteral("流动块字段类型错误。"); } return false; }
        if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 30) { return false; } }
    }
    candidate.triggerAddress = json.value(QStringLiteral("triggerAddress")).toString(); candidate.flowFront = json.value(QStringLiteral("flowFront")).toString(); candidate.flowBack = json.value(QStringLiteral("flowBack")).toString(); candidate.pipeFront = json.value(QStringLiteral("pipeFront")).toString(); candidate.pipeBack = json.value(QStringLiteral("pipeBack")).toString(); candidate.borderColor = json.value(QStringLiteral("borderColor")).toString();
    candidate.pattern = json.value(QStringLiteral("pattern")).toInt(); candidate.direction = json.value(QStringLiteral("direction")).toInt(); candidate.count = json.value(QStringLiteral("count")).toInt(); candidate.speed = json.value(QStringLiteral("speed")).toInt(); candidate.vertical = json.value(QStringLiteral("vertical")).toBool(); candidate.noBorder = json.value(QStringLiteral("noBorder")).toBool(); candidate.visibilityBitChosen = json.value(QStringLiteral("visibilityBitChosen")).toBool();
    if (!StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() || !candidate.validate(error)) { return false; }
    *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : FlowBlockProperties::validate
 * Description     : Check measured UI bounds, five color drafts and explicitly selected active visibility conditions.
 * Input Parameters: error
 * Return Value    : True for a bounded PC editor flow model.
 * Date            : 2026.11.12
 ******************************************************************************/
bool FlowBlockProperties::validate(QString *error) const //20261112-0 add
{
    QString color;
    if (count < 1 || count > 30 || pattern < 0 || pattern > 12 || direction < 0 || direction > 1 || speed < 0 || speed > 2 || triggerAddress.size() > 1024 || (visibility.visibilityEnabled && visibility.bitEnabled && !visibilityBitChosen))
    { if (error != nullptr) { *error = QStringLiteral("流动块数量、样式、方向、速度或活动显现位状态无效。"); } return false; }
    Q_FOREACH (color, QStringList({flowFront, flowBack, pipeFront, pipeBack, borderColor})) { if (color.size() > 32 || !QColor(color).isValid()) { if (error != nullptr) { *error = QStringLiteral("流动块颜色无效。"); } return false; } }
    return visibility.validate(error);
}

/******************************************************************************
 * Function Name   : FlowBlockProperties::validateAddresses
 * Description     : Validate one bit trigger and full-width independent active visibility carriers without accessing PLC hardware.
 * Input Parameters: connections, error
 * Return Value    : True for all active trigger/condition addresses.
 * Date            : 2026.11.12
 ******************************************************************************/
bool FlowBlockProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261112-0 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress parsed;
    ScreenButtonProperties shared;
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    context.connections = connections; context.role = PlcAddressRole::Bit;
    if (!parseQualifiedPlcAddressText(triggerAddress, context, catalog, &parsed, error)) { return false; }
    shared.common.indicator = false; shared.common.visibility = visibility; return shared.validateAddresses(connections, error);
}
