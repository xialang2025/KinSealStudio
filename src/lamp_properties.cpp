#include "lamp_properties.h"

#include <QJsonArray>

namespace
{
/******************************************************************************
 * Function Name   : lampError
 * Description     : Report a lamp-property failure without replacing the saved draft.
 * Input Parameters: error, message
 * Return Value    : False
 * Date            : 2026.11.09
 ******************************************************************************/
bool lampError(QString *error, const QString &message) //20261109-8 add
{
    if (error != nullptr) { *error = message; }
    return false;
}
}

/******************************************************************************
 * Function Name   : LampProperties::dataTypes
 * Description     : Preserve the observed unsigned monitoring type order.
 * Input Parameters: None
 * Return Value    : Ordered type labels.
 * Date            : 2026.11.09
 ******************************************************************************/
QStringList LampProperties::dataTypes() //20261109-8 add
{
    return {QStringLiteral("16位正整数"), QStringLiteral("32位正整数")};
}

/******************************************************************************
 * Function Name   : LampProperties::stateCount
 * Description     : Return the number of appearances used by the active monitoring mode.
 * Input Parameters: None
 * Return Value    : Active appearance count.
 * Date            : 2026.11.09
 ******************************************************************************/
int LampProperties::stateCount() const //20261109-8 add
{
    return monitorMode == 2 ? multiStateCount : monitorMode == 1 ? 2 : 1;
}

/******************************************************************************
 * Function Name   : LampProperties::transferWordCount
 * Description     : Reserve both words for unsigned 32-bit multi-state monitoring.
 * Input Parameters: None
 * Return Value    : Register count.
 * Date            : 2026.11.09
 ******************************************************************************/
int LampProperties::transferWordCount() const //20261109-8 add
{
    return multiDataType == 1 ? 2 : 1;
}

/******************************************************************************
 * Function Name   : LampProperties::ensureStateCount
 * Description     : Resize additional appearances to the edited state count using native defaults for new rows.
 * Input Parameters: count, shared
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void LampProperties::ensureStateCount(int count, const BitButtonProperties &shared) //20261109-8 add
{
    BitButtonState next;

    if (count < 2 || count > 32) { return; }
    if (extraStates.size() > count - 2) { extraStates.resize(count - 2); }
    while (extraStates.size() < qMax(0, count - 2))
    {
        next = BitButtonState();
        next.picture = QStringLiteral("switch_ON.kvg");
        next.background = QColor(QStringLiteral("#a0a0a0"));
        next.effect3d = shared.states[0].effect3d;
        next.transparent = shared.states[0].transparent;
        next.sameLanguage = shared.states[0].sameLanguage; //20261109-8 add: Native lamp language sharing is global, while each new caption remains empty.
        extraStates.append(next);
    }
}

/******************************************************************************
 * Function Name   : LampProperties::state
 * Description     : Resolve an editable lamp appearance across shared and additional state storage.
 * Input Parameters: shared, index
 * Return Value    : Appearance pointer or null for a missing state.
 * Date            : 2026.11.09
 ******************************************************************************/
BitButtonState *LampProperties::state(BitButtonProperties &shared, int index) //20261109-8 add
{
    if (index < 0 || index >= extraStates.size() + 2) { return nullptr; }
    return index < 2 ? &shared.states[index] : &extraStates[index - 2];
}

/******************************************************************************
 * Function Name   : LampProperties::state
 * Description     : Resolve a saved lamp appearance without modifying the record.
 * Input Parameters: shared, index
 * Return Value    : Appearance pointer or null for a missing state.
 * Date            : 2026.11.09
 ******************************************************************************/
const BitButtonState *LampProperties::state(const BitButtonProperties &shared, int index) const //20261109-8 add
{
    if (index < 0 || index >= extraStates.size() + 2) { return nullptr; }
    return index < 2 ? &shared.states[index] : &extraStates[index - 2];
}

/******************************************************************************
 * Function Name   : LampProperties::validate
 * Description     : Check lamp monitoring selections and all retained additional appearances.
 * Input Parameters: error
 * Return Value    : True for a representable editor record.
 * Date            : 2026.11.09
 ******************************************************************************/
bool LampProperties::validate(QString *error) const //20261109-8 add
{
    BitButtonState appearance;
    BitButtonProperties appearanceGuard;

    if (monitorMode < 0 || monitorMode > 2 || multiDataType < 0 || multiDataType > 1 ||
        multiStateCount < 2 || multiStateCount > 32 || extraStates.size() > 30 ||
        (monitorMode == 2 && multiMonitorAddress.trimmed().isEmpty()))
    {
        return lampError(error, QStringLiteral("指示灯的监视模式、数据类型、状态数或活动监视地址无效。"));
    }
    if (monitorMode == 2 && extraStates.size() < multiStateCount - 2)
    {
        return lampError(error, QStringLiteral("多态指示灯的状态外观数量不足。"));
    }
    Q_FOREACH (appearance, extraStates)
    {
        appearanceGuard.states[0] = appearance;
        if (!appearanceGuard.validate(error) || (!appearance.picturePng.isEmpty() && !validateButtonImage(appearance.picturePng)))
        {
            return lampError(error, QStringLiteral("指示灯的附加状态外观无效。"));
        }
    }
    if (error != nullptr) { error->clear(); }
    return true;
}

/******************************************************************************
 * Function Name   : LampProperties::toJson
 * Description     : Persist monitoring selections and independent additional state drafts.
 * Input Parameters: None
 * Return Value    : Versioned lamp property record.
 * Date            : 2026.11.09
 ******************************************************************************/
QJsonObject LampProperties::toJson() const //20261109-8 add
{
    QJsonObject json;
    QJsonArray appearances;
    BitButtonState appearance;

    Q_FOREACH (appearance, extraStates) { appearances.append(appearance.toJson()); }
    json.insert(QStringLiteral("version"), 1);
    json.insert(QStringLiteral("monitorMode"), monitorMode);
    json.insert(QStringLiteral("multiDataType"), multiDataType);
    json.insert(QStringLiteral("multiStateCount"), multiStateCount);
    json.insert(QStringLiteral("multiMonitorSame"), multiMonitorSame);
    json.insert(QStringLiteral("multiMonitorAddress"), multiMonitorAddress);
    json.insert(QStringLiteral("extraStates"), appearances);
    return json;
}

/******************************************************************************
 * Function Name   : LampProperties::fromJson
 * Description     : Restore a lamp record transactionally and reject truncated appearance arrays.
 * Input Parameters: json, properties, error
 * Return Value    : True when the validated record replaces the output.
 * Date            : 2026.11.09
 ******************************************************************************/
bool LampProperties::fromJson(const QJsonObject &json, LampProperties *properties, QString *error) //20261109-8 add
{
    LampProperties result;
    QJsonArray appearances;
    QJsonValue appearanceJson;
    BitButtonState appearance;

    if (properties == nullptr || json.value(QStringLiteral("version")).toInt() != 1)
    {
        return lampError(error, QStringLiteral("不支持的指示灯属性版本或输出为空。"));
    }
    result.monitorMode = json.value(QStringLiteral("monitorMode")).toInt(1);
    result.multiDataType = json.value(QStringLiteral("multiDataType")).toInt();
    result.multiStateCount = json.value(QStringLiteral("multiStateCount")).toInt(2);
    result.multiMonitorSame = json.value(QStringLiteral("multiMonitorSame")).toBool();
    result.multiMonitorAddress = json.value(QStringLiteral("multiMonitorAddress")).toString(QStringLiteral("LW0"));
    appearances = json.value(QStringLiteral("extraStates")).toArray();
    if (appearances.size() > 30) { return lampError(error, QStringLiteral("指示灯附加外观数量超出32状态范围。")); }
    Q_FOREACH (appearanceJson, appearances)
    {
        if (!appearanceJson.isObject() || !BitButtonState::fromJson(appearanceJson.toObject(), &appearance, error)) { return false; }
        result.extraStates.append(appearance);
    }
    if (!result.validate(error)) { return false; }
    *properties = result;
    return true;
}
