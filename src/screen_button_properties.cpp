#include "screen_button_properties.h"
#include "plc_native_catalog.h"
#include <cmath>

/******************************************************************************
 * Function Name   : ScreenButtonProperties::ScreenButtonProperties
 * Description     : Initialize measured screen-button defaults independently of bit-switch operating addresses.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
ScreenButtonProperties::ScreenButtonProperties() //20261112-0 add
{
    common.indicator = false; common.dualMonitor = true; common.monitorSame = false; common.bitIndex = 0;
    common.states[0].background = QColor(QStringLiteral("#f0f0f0")); common.states[1].background = QColor(QStringLiteral("#f0f0f0"));
}

/******************************************************************************
 * Function Name   : ScreenButtonProperties::actions
 * Description     : Return the four observed operation labels in native order.
 * Input Parameters: None
 * Return Value    : Editor labels only.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList ScreenButtonProperties::actions() //20261112-0 add
{
    return {QStringLiteral("跳转到指定画面"), QStringLiteral("返回上一个画面"), QStringLiteral("打开窗口"), QStringLiteral("关闭窗口")};
}

/******************************************************************************
 * Function Name   : ScreenButtonProperties::activeTarget
 * Description     : Identify only the currently active navigation dependency, retaining dormant screen/window drafts.
 * Input Parameters: None
 * Return Value    : Stable page ID or an empty value for return/close.
 * Date            : 2026.11.12
 ******************************************************************************/
QString ScreenButtonProperties::activeTarget() const //20261112-0 add
{
    return action == 0 ? screenPageId : action == 2 ? windowPageId : QString();
}

/******************************************************************************
 * Function Name   : ScreenButtonProperties::toJson
 * Description     : Serialize independent navigation and complete shared visual/permission drafts.
 * Input Parameters: None
 * Return Value    : Typed screen-button editor record.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject ScreenButtonProperties::toJson() const //20261112-0 add
{
    return {{QStringLiteral("common"), common.toJson()}, {QStringLiteral("action"), action}, {QStringLiteral("screenPageId"), screenPageId}, {QStringLiteral("windowPageId"), windowPageId}, {QStringLiteral("changeLevel"), changeLevel}, {QStringLiteral("newLevel"), newLevel}, {QStringLiteral("requireLogin"), requireLogin}};
}

/******************************************************************************
 * Function Name   : ScreenButtonProperties::fromJson
 * Description     : Reject incomplete, unknown, fractional or lossy records without changing the receiving object.
 * Input Parameters: json, output, error
 * Return Value    : True after full independent validation.
 * Date            : 2026.11.12
 ******************************************************************************/
bool ScreenButtonProperties::fromJson(const QJsonObject &json, ScreenButtonProperties *output, QString *error) //20261112-0 add
{
    ScreenButtonProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator item;
    double value;
    if (output == nullptr || json.keys() != shape.keys()) { if (error != nullptr) { *error = QStringLiteral("画面按钮字段缺失、未知或接收位置为空。"); } return false; }
    for (item = json.constBegin(); item != json.constEnd(); ++item)
    {
        if (item.value().type() != shape.value(item.key()).type()) { if (error != nullptr) { *error = QStringLiteral("画面按钮字段类型错误。"); } return false; }
        if (item.value().isDouble()) { value = item.value().toDouble(); if (!std::isfinite(value) || value != std::floor(value) || value < 0 || value > 8) { if (error != nullptr) { *error = QStringLiteral("画面按钮枚举或等级无效。"); } return false; } }
    }
    candidate.action = json.value(QStringLiteral("action")).toInt(); candidate.newLevel = json.value(QStringLiteral("newLevel")).toInt(); candidate.changeLevel = json.value(QStringLiteral("changeLevel")).toBool(); candidate.requireLogin = json.value(QStringLiteral("requireLogin")).toBool(); candidate.screenPageId = json.value(QStringLiteral("screenPageId")).toString(); candidate.windowPageId = json.value(QStringLiteral("windowPageId")).toString();
    if (!BitButtonProperties::fromJson(json.value(QStringLiteral("common")).toObject(), &candidate.common, error, true) || candidate.common.toJson() != json.value(QStringLiteral("common")).toObject() || !candidate.validate(error)) { return false; }
    *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : ScreenButtonProperties::validate
 * Description     : Validate native UI bounds while rejecting unrecovered active same-operation-address monitor semantics.
 * Input Parameters: error
 * Return Value    : True for the declared screen editor slice.
 * Date            : 2026.11.12
 ******************************************************************************/
bool ScreenButtonProperties::validate(QString *error) const //20261112-0 add
{
    if (action < 0 || action > 3 || newLevel < 1 || newLevel > 8 || screenPageId.size() > 256 || windowPageId.size() > 256 || common.macroEnabled || (common.noticeBit && common.noticeWord) || common.pressTime < 0 || common.pressTime > 255 || common.states[0].text.fontSize > 128 || common.states[1].text.fontSize > 128)
    { if (error != nullptr) { *error = QStringLiteral("画面按钮参数超出已核定编辑范围。"); } return false; }
    if (common.indicator && (!common.dualMonitor || common.monitorSame))
    { if (error != nullptr) { *error = QStringLiteral("画面按钮未恢复单态或同操作地址监视语义，请使用独立双态监视地址。"); } return false; }
    return common.validate(error, true);
}

/******************************************************************************
 * Function Name   : ScreenButtonProperties::validateAddresses
 * Description     : Validate active monitoring, visibility, touch and notice spans without inventing an operation carrier.
 * Input Parameters: connections, error
 * Return Value    : True for all configured active carriers.
 * Date            : 2026.11.12
 ******************************************************************************/
bool ScreenButtonProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261112-0 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress address;
    QList<QPair<QString, int>> words;
    QStringList bits;
    StaticTextProperties condition;
    QPair<QString, int> word;
    QString bit;
    int index;
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    if (common.indicator) { if (common.monitorType == 0) { bits.append(common.monitorBitAddress); } else { words.append({common.monitorWordAddress, 1}); } }
    for (index = 0; index < 2; ++index)
    {
        condition = index == 0 ? common.visibility : common.touch;
        if (!condition.visibilityEnabled) { continue; }
        if (condition.bitEnabled) { bits.append(condition.bitAddress); }
        if (condition.wordEnabled) { words.append({condition.wordAddress, condition.dataType == 0 || condition.dataType == 2 ? 1 : 2}); }
    }
    if (common.notice && common.noticeBit) { bits.append(common.noticeBitAddress); }
    if (common.notice && common.noticeWord) { words.append({common.noticeWordAddress, common.noticeWordType == 0 || common.noticeWordType == 2 ? 1 : 2}); }
    context.connections = connections; context.role = PlcAddressRole::Word;
    Q_FOREACH (word, words) { context.transferCount = word.second; if (!parseQualifiedPlcAddressText(word.first, context, catalog, &address, error)) { return false; } }
    context.role = PlcAddressRole::Bit; context.transferCount = 1U;
    Q_FOREACH (bit, bits) { if (!parseQualifiedPlcAddressText(bit, context, catalog, &address, error)) { return false; } }
    return true;
}
