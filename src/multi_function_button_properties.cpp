#include "multi_function_button_properties.h"
#include "word_button_properties.h"
#include "plc_native_catalog.h"
#include <QJsonArray>
#include <cmath>

namespace
{
/******************************************************************************
 * Function Name   : mfError
 * Description     : Report an MF model failure without modifying the destination draft.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.12
 ******************************************************************************/
bool mfError(QString *error, const QString &message) //20261112-0 add
{
    if (error != nullptr) { *error = message; }
    return false;
}

/******************************************************************************
 * Function Name   : mfShape
 * Description     : Reject unknown, missing, mistyped and fractional MF JSON fields before conversion.
 * Input Parameters: json, shape, error
 * Return Value    : True for a complete typed object.
 * Date            : 2026.11.12
 ******************************************************************************/
bool mfShape(const QJsonObject &json, const QJsonObject &shape, QString *error) //20261112-0 add
{
    QJsonObject::const_iterator item;
    double number;
    if (json.keys() != shape.keys()) { return mfError(error, QStringLiteral("多功能按钮字段缺失或未知。")); }
    for (item = json.constBegin(); item != json.constEnd(); ++item)
    {
        if (item.value().type() != shape.value(item.key()).type()) { return mfError(error, QStringLiteral("多功能按钮字段类型错误。")); }
        if (item.value().isDouble())
        {
            number = item.value().toDouble();
            if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 6) { return mfError(error, QStringLiteral("多功能按钮枚举无效。")); }
        }
    }
    return true;
}
}

/******************************************************************************
 * Function Name   : MultiFunctionCommand::bitOperations
 * Description     : Keep toggle before momentary, as observed in the MF dialog.
 * Input Parameters: None
 * Return Value    : Four ordered labels.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList MultiFunctionCommand::bitOperations() //20261112-0 add
{
    return {QStringLiteral("置位"), QStringLiteral("复位"), QStringLiteral("交替"), QStringLiteral("点动")};
}

/******************************************************************************
 * Function Name   : MultiFunctionCommand::wordOperations
 * Description     : Preserve input and set-data command order independently of ordinary word buttons.
 * Input Parameters: None
 * Return Value    : Two ordered labels.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList MultiFunctionCommand::wordOperations() //20261112-0 add
{
    return {QStringLiteral("用户输入"), QStringLiteral("设置数据")};
}

/******************************************************************************
 * Function Name   : MultiFunctionCommand::dataTypes
 * Description     : Expose the measured signed-first MF type order.
 * Input Parameters: None
 * Return Value    : Seven ordered labels.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList MultiFunctionCommand::dataTypes() //20261112-0 add
{
    return {QStringLiteral("16位整数"), QStringLiteral("32位整数"), QStringLiteral("16位正整数"), QStringLiteral("32位正整数"), QStringLiteral("16位BCD码"), QStringLiteral("32位BCD码"), QStringLiteral("32位浮点数")};
}

/******************************************************************************
 * Function Name   : MultiFunctionCommand::activeTarget
 * Description     : Resolve the active navigation dependency while retaining dormant target drafts.
 * Input Parameters: None
 * Return Value    : Page ID or empty for non-navigation and return/close commands.
 * Date            : 2026.11.12
 ******************************************************************************/
QString MultiFunctionCommand::activeTarget() const //20261112-0 add
{
    return kind != 1 ? QString() : operation == 0 ? screenPageId : operation == 2 ? windowPageId : QString();
}

/******************************************************************************
 * Function Name   : MultiFunctionCommand::transferWordCount
 * Description     : Validate the complete command carrier instead of only its base address.
 * Input Parameters: None
 * Return Value    : One or two words.
 * Date            : 2026.11.12
 ******************************************************************************/
int MultiFunctionCommand::transferWordCount() const //20261112-0 add
{
    return dataType == 1 || dataType == 3 || dataType == 5 || dataType == 6 ? 2 : 1;
}

/******************************************************************************
 * Function Name   : MultiFunctionCommand::label
 * Description     : Generate a command-list caption without interpreting it as a firmware opcode.
 * Input Parameters: None
 * Return Value    : Kind and current operation label.
 * Date            : 2026.11.12
 ******************************************************************************/
QString MultiFunctionCommand::label() const //20261112-0 add
{
    QStringList labels = kind == 0 ? bitOperations() : kind == 1 ? ScreenButtonProperties::actions() : wordOperations();
    return (kind == 0 ? QStringLiteral("线圈操作 ") : kind == 1 ? QStringLiteral("画面操作-") : QStringLiteral("字操作-")) + labels.value(operation);
}

/******************************************************************************
 * Function Name   : MultiFunctionCommand::toJson
 * Description     : Serialize a typed editor command and its independent navigation drafts.
 * Input Parameters: None
 * Return Value    : Complete MF command JSON.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject MultiFunctionCommand::toJson() const //20261112-0 add
{
    return {{QStringLiteral("kind"), kind}, {QStringLiteral("operation"), operation}, {QStringLiteral("address"), address}, {QStringLiteral("dataType"), dataType}, {QStringLiteral("value"), value}, {QStringLiteral("screenPageId"), screenPageId}, {QStringLiteral("windowPageId"), windowPageId}};
}

/******************************************************************************
 * Function Name   : MultiFunctionCommand::fromJson
 * Description     : Parse and validate a command atomically, preserving the destination on failure.
 * Input Parameters: json, output, error
 * Return Value    : True after complete command validation.
 * Date            : 2026.11.12
 ******************************************************************************/
bool MultiFunctionCommand::fromJson(const QJsonObject &json, MultiFunctionCommand *output, QString *error) //20261112-0 add
{
    MultiFunctionCommand candidate;
    if (output == nullptr || !mfShape(json, candidate.toJson(), error)) { return false; }
    candidate.kind = json.value(QStringLiteral("kind")).toInt(); candidate.operation = json.value(QStringLiteral("operation")).toInt();
    candidate.dataType = json.value(QStringLiteral("dataType")).toInt(); candidate.address = json.value(QStringLiteral("address")).toString(); candidate.value = json.value(QStringLiteral("value")).toString();
    candidate.screenPageId = json.value(QStringLiteral("screenPageId")).toString(); candidate.windowPageId = json.value(QStringLiteral("windowPageId")).toString();
    if (!candidate.validate(error)) { return false; }
    *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : MultiFunctionCommand::validate
 * Description     : Check kind-specific bounds and explicitly translate MF types for shared constant validation.
 * Input Parameters: error
 * Return Value    : True for the bounded PC editor command.
 * Date            : 2026.11.12
 ******************************************************************************/
bool MultiFunctionCommand::validate(QString *error) const //20261112-0 add
{
    static const int wordType[7] = {2, 3, 0, 1, 4, 5, 6};
    WordButtonProperties constant;
    if (kind < 0 || kind > 2 || operation < 0 || operation > (kind == 2 ? 1 : 3) || dataType < 0 || dataType > 6 || address.size() > 1024 || value.size() > 256 || screenPageId.size() > 256 || windowPageId.size() > 256)
    { return mfError(error, QStringLiteral("多功能按钮命令类型、操作或字段长度无效。")); }
    if (kind == 2 && operation == 1)
    {
        constant.resetDefaultsForDataType(wordType[dataType]); constant.operation = 0; constant.constantValue = value;
        if (!constant.validate(error)) { return false; }
    }
    return true;
}

/******************************************************************************
 * Function Name   : MultiFunctionButtonProperties::MultiFunctionButtonProperties
 * Description     : Initialize one set-bit command, two ON artworks and the measured unchosen permission radios.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
MultiFunctionButtonProperties::MultiFunctionButtonProperties() //20261112-0 add
{
    commands.append(MultiFunctionCommand()); common.indicator = false; common.dualMonitor = false; common.monitorSame = false; common.bitIndex = 0; common.pressTime = 0;
    common.states[0].picture = common.states[1].picture; common.states[0].sharedText.clear(); common.states[0].text.languageTexts.clear();
    common.states[0].background = QColor(QStringLiteral("#f0f0f0")); common.states[1].background = QColor(QStringLiteral("#a0a0a0"));
}

/******************************************************************************
 * Function Name   : MultiFunctionButtonProperties::toJson
 * Description     : Store ordered commands, press appearance and explicit permission selection identity.
 * Input Parameters: None
 * Return Value    : Typed MF property record.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject MultiFunctionButtonProperties::toJson() const //20261112-0 add
{
    QJsonArray list;
    MultiFunctionCommand command;
    Q_FOREACH (command, commands) { list.append(command.toJson()); }
    return {{QStringLiteral("common"), common.toJson()}, {QStringLiteral("commands"), list}, {QStringLiteral("pressedEffect"), pressedEffect}, {QStringLiteral("visibilityBitChosen"), visibilityBitChosen}, {QStringLiteral("touchBitChosen"), touchBitChosen}};
}

/******************************************************************************
 * Function Name   : MultiFunctionButtonProperties::fromJson
 * Description     : Reject incomplete or lossy nested records and publish only a valid complete draft.
 * Input Parameters: json, output, error
 * Return Value    : True for a complete MF record.
 * Date            : 2026.11.12
 ******************************************************************************/
bool MultiFunctionButtonProperties::fromJson(const QJsonObject &json, MultiFunctionButtonProperties *output, QString *error) //20261112-0 add
{
    MultiFunctionButtonProperties candidate;
    MultiFunctionCommand command;
    QJsonValue item;
    if (output == nullptr || !mfShape(json, candidate.toJson(), error) || json.value(QStringLiteral("commands")).toArray().size() > 7) { return false; }
    if (!BitButtonProperties::fromJson(json.value(QStringLiteral("common")).toObject(), &candidate.common, error, true) || candidate.common.toJson() != json.value(QStringLiteral("common")).toObject()) { return false; }
    candidate.commands.clear();
    Q_FOREACH (item, json.value(QStringLiteral("commands")).toArray())
    { if (!item.isObject() || !MultiFunctionCommand::fromJson(item.toObject(), &command, error)) { return false; } candidate.commands.append(command); }
    candidate.pressedEffect = json.value(QStringLiteral("pressedEffect")).toBool(); candidate.visibilityBitChosen = json.value(QStringLiteral("visibilityBitChosen")).toBool(); candidate.touchBitChosen = json.value(QStringLiteral("touchBitChosen")).toBool();
    if (!candidate.validate(error)) { return false; } *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : MultiFunctionButtonProperties::validate
 * Description     : Bound ordered command drafts and reject active features absent from the observed MF UI.
 * Input Parameters: error
 * Return Value    : True for the supported editor slice; empty lists remain explicit Lite drafts.
 * Date            : 2026.11.12
 ******************************************************************************/
bool MultiFunctionButtonProperties::validate(QString *error) const //20261112-0 add
{
    ScreenButtonProperties shared;
    MultiFunctionCommand command;
    if (commands.size() > 7 || common.indicator || common.dualMonitor || common.notice || common.noticeBit || common.noticeWord || (common.visibility.visibilityEnabled && common.visibility.bitEnabled && !visibilityBitChosen) || (common.touch.visibilityEnabled && common.touch.bitEnabled && !touchBitChosen))
    { return mfError(error, QStringLiteral("多功能按钮命令超限、包含未支持功能或活动权限位未选择状态。")); }
    shared.common = common; if (!shared.validate(error)) { return false; }
    Q_FOREACH (command, commands) { if (!command.validate(error)) { return false; } }
    return true;
}

/******************************************************************************
 * Function Name   : MultiFunctionButtonProperties::validateAddresses
 * Description     : Check every bit/word command with its full carrier width and independent active permissions.
 * Input Parameters: connections, error
 * Return Value    : True when all active addresses are valid.
 * Date            : 2026.11.12
 ******************************************************************************/
bool MultiFunctionButtonProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261112-0 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress address;
    ScreenButtonProperties shared;
    MultiFunctionCommand command;
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    context.connections = connections;
    Q_FOREACH (command, commands)
    {
        if (command.kind == 1) { continue; }
        context.role = command.kind == 0 ? PlcAddressRole::Bit : PlcAddressRole::Word; context.transferCount = command.kind == 0 ? 1U : u32(command.transferWordCount());
        if (!parseQualifiedPlcAddressText(command.address, context, catalog, &address, error)) { return false; }
    }
    shared.common = common; return shared.validateAddresses(connections, error);
}
