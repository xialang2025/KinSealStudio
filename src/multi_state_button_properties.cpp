#include "multi_state_button_properties.h"
#include "plc_native_catalog.h"
#include <QJsonArray>
#include <cmath>

/******************************************************************************
 * Function Name   : MultiStateButtonProperties::MultiStateButtonProperties
 * Description     : Initialize the native default three-state editor independently of lamp monitoring unions.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
MultiStateButtonProperties::MultiStateButtonProperties() //20261112-0 add
{
    common.indicator = false; common.dualMonitor = false; common.monitorSame = false; common.bitIndex = 0;
    common.states[0].background = QColor(QStringLiteral("#f0f0f0")); common.states[1].background = QColor(QStringLiteral("#f0f0f0")); extraStates.append(freshExtraState());
}

/******************************************************************************
 * Function Name   : MultiStateButtonProperties::actions
 * Description     : Preserve six native action labels without mapping UI positions to unverified firmware enums.
 * Input Parameters: None
 * Return Value    : Six labels in the documented UI grouping.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList MultiStateButtonProperties::actions() //20261112-0 add
{
    return {QStringLiteral("键盘输入"), QStringLiteral("递增"), QStringLiteral("递减"), QStringLiteral("递增（绕回）"), QStringLiteral("递减（绕回）"), QStringLiteral("保持")};
}

/******************************************************************************
 * Function Name   : MultiStateButtonProperties::freshExtraState
 * Description     : Recreate the observed empty-text, ON-artwork state after a destructive shrink and regrow.
 * Input Parameters: None
 * Return Value    : Independent native-bounded appearance draft.
 * Date            : 2026.11.12
 ******************************************************************************/
BitButtonState MultiStateButtonProperties::freshExtraState() //20261112-0 add
{
    BitButtonProperties defaults;
    BitButtonState result = defaults.states[1];
    result.sharedText.clear(); result.text.languageTexts.clear(); result.background = QColor(QStringLiteral("#a0a0a0")); return result;
}

/******************************************************************************
 * Function Name   : MultiStateButtonProperties::setStateCount
 * Description     : Delete truncated states permanently within the draft and create measured fresh defaults when growing.
 * Input Parameters: count
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void MultiStateButtonProperties::setStateCount(int count) //20261112-0 add
{
    stateCount = qBound(2, count, 32);
    while (extraStates.size() > stateCount - 2) { extraStates.removeLast(); }
    while (extraStates.size() < stateCount - 2) { extraStates.append(freshExtraState()); }
}

/******************************************************************************
 * Function Name   : MultiStateButtonProperties::state
 * Description     : Resolve a mutable appearance without aliasing state two onto the shared two-state editor storage.
 * Input Parameters: index
 * Return Value    : Appearance pointer or nullptr for an invalid index.
 * Date            : 2026.11.12
 ******************************************************************************/
BitButtonState *MultiStateButtonProperties::state(int index) //20261112-0 add
{
    if (index < 0 || index >= stateCount) { return nullptr; }
    return index < 2 ? &common.states[index] : index - 2 < extraStates.size() ? &extraStates[index - 2] : nullptr;
}

/******************************************************************************
 * Function Name   : MultiStateButtonProperties::state
 * Description     : Resolve a read-only appearance across shared and extra state storage.
 * Input Parameters: index
 * Return Value    : Const appearance pointer or nullptr.
 * Date            : 2026.11.12
 ******************************************************************************/
const BitButtonState *MultiStateButtonProperties::state(int index) const //20261112-0 add
{
    if (index < 0 || index >= stateCount) { return nullptr; }
    return index < 2 ? &common.states[index] : index - 2 < extraStates.size() ? &extraStates[index - 2] : nullptr;
}

/******************************************************************************
 * Function Name   : MultiStateButtonProperties::toJson
 * Description     : Persist independent read/write actions and every surviving state draft.
 * Input Parameters: None
 * Return Value    : Complete typed multi-state button record.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject MultiStateButtonProperties::toJson() const //20261112-0 add
{
    QJsonArray states;
    BitButtonState appearance;
    Q_FOREACH (appearance, extraStates) { states.append(appearance.toJson()); }
    return {{QStringLiteral("common"), common.toJson()}, {QStringLiteral("dataType"), dataType}, {QStringLiteral("monitorAddress"), monitorAddress}, {QStringLiteral("writeAddress"), writeAddress}, {QStringLiteral("sameAddress"), sameAddress}, {QStringLiteral("stateCount"), stateCount}, {QStringLiteral("action"), action}, {QStringLiteral("extraStates"), states}};
}

/******************************************************************************
 * Function Name   : MultiStateButtonProperties::fromJson
 * Description     : Strictly parse field/state counts and reject lossy nested button drafts atomically.
 * Input Parameters: json, output, error
 * Return Value    : True after complete typed validation.
 * Date            : 2026.11.12
 ******************************************************************************/
bool MultiStateButtonProperties::fromJson(const QJsonObject &json, MultiStateButtonProperties *output, QString *error) //20261112-0 add
{
    MultiStateButtonProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator item;
    QJsonValue value;
    BitButtonState appearance;
    double number;
    if (output == nullptr || json.keys() != shape.keys()) { if (error != nullptr) { *error = QStringLiteral("多态按钮字段缺失、未知或接收位置为空。"); } return false; }
    for (item = json.constBegin(); item != json.constEnd(); ++item)
    {
        if (item.value().type() != shape.value(item.key()).type()) { if (error != nullptr) { *error = QStringLiteral("多态按钮字段类型错误。"); } return false; }
        if (item.value().isDouble()) { number = item.value().toDouble(); if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 32) { if (error != nullptr) { *error = QStringLiteral("多态按钮枚举或状态数无效。"); } return false; } }
    }
    candidate.dataType = json.value(QStringLiteral("dataType")).toInt(); candidate.stateCount = json.value(QStringLiteral("stateCount")).toInt(); candidate.action = json.value(QStringLiteral("action")).toInt(); candidate.sameAddress = json.value(QStringLiteral("sameAddress")).toBool(); candidate.monitorAddress = json.value(QStringLiteral("monitorAddress")).toString(); candidate.writeAddress = json.value(QStringLiteral("writeAddress")).toString();
    if (!BitButtonProperties::fromJson(json.value(QStringLiteral("common")).toObject(), &candidate.common, error, true) || candidate.common.toJson() != json.value(QStringLiteral("common")).toObject() || json.value(QStringLiteral("extraStates")).toArray().size() > 30) { return false; }
    candidate.extraStates.clear();
    Q_FOREACH (value, json.value(QStringLiteral("extraStates")).toArray())
    { if (!value.isObject() || !BitButtonState::fromJson(value.toObject(), &appearance, error) || appearance.toJson() != value.toObject()) { return false; } candidate.extraStates.append(appearance); }
    if (!candidate.validate(error)) { return false; } *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : MultiStateButtonProperties::validate
 * Description     : Validate measured two-type, six-action and state lifetime bounds with per-state visual checks.
 * Input Parameters: error
 * Return Value    : True for the declared editor model.
 * Date            : 2026.11.12
 ******************************************************************************/
bool MultiStateButtonProperties::validate(QString *error) const //20261112-0 add
{
    ScreenButtonProperties shared;
    int index;
    if (dataType < 0 || dataType > 1 || stateCount < 2 || stateCount > 32 || extraStates.size() != stateCount - 2 || action < 0 || action > 5 || monitorAddress.size() > 1024 || writeAddress.size() > 1024 || common.indicator || common.dualMonitor)
    { if (error != nullptr) { *error = QStringLiteral("多态按钮类型、功能或独立状态集合无效。"); } return false; }
    shared.common = common; if (!shared.validate(error)) { return false; }
    for (index = 0; index < extraStates.size(); ++index) { shared.common.states[0] = extraStates.at(index); if (!shared.validate(error)) { return false; } }
    return true;
}

/******************************************************************************
 * Function Name   : MultiStateButtonProperties::validateAddresses
 * Description     : Validate full unsigned-word monitor/write spans and reuse only independent shared permission/notice roles.
 * Input Parameters: connections, error
 * Return Value    : True for active carriers; runtime writes remain unobserved.
 * Date            : 2026.11.12
 ******************************************************************************/
bool MultiStateButtonProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261112-0 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress address;
    ScreenButtonProperties shared;
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    context.connections = connections; context.role = PlcAddressRole::Word; context.transferCount = dataType == 0 ? 1U : 2U;
    if (!parseQualifiedPlcAddressText(monitorAddress, context, catalog, &address, error) || (!sameAddress && !parseQualifiedPlcAddressText(writeAddress, context, catalog, &address, error))) { return false; }
    shared.common = common; return shared.validateAddresses(connections, error);
}
