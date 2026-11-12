#include "message_display_properties.h"
#include <QJsonArray>
#include <cmath>

namespace {
/******************************************************************************
 * Function Name   : messageError
 * Description     : Report a bounded message-property error without modifying a caller's object.
 * Input Parameters: error, text
 * Return Value    : False.
 * Date            : 2026.11.12
 ******************************************************************************/
bool messageError(QString *error, const QString &text) //20261112-0 add
{
    if (error != nullptr) { *error = text; }
    return false;
}
}

/******************************************************************************
 * Function Name   : MessageDisplayProperties::MessageDisplayProperties
 * Description     : Initialize the observed message dialog defaults independently of static text defaults.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
MessageDisplayProperties::MessageDisplayProperties() //20261112-0 add
{
    appearance.fontFamily = QStringLiteral("宋体");
    appearance.style = 1;
    appearance.foregroundColor = Qt::white;
    ensureStates(stateCount);
}

/******************************************************************************
 * Function Name   : MessageDisplayProperties::ensureStates
 * Description     : Resize state drafts and reset removed rows when they are added again, as observed natively.
 * Input Parameters: count
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void MessageDisplayProperties::ensureStates(int count) //20261112-0 add
{
    states.resize(qBound(1, count, 128));
}

/******************************************************************************
 * Function Name   : MessageDisplayProperties::text
 * Description     : Resolve each state's language independently, using the native numeric default for absent text.
 * Input Parameters: state, language
 * Return Value    : State text; an explicitly empty translation remains empty.
 * Date            : 2026.11.12
 ******************************************************************************/
QString MessageDisplayProperties::text(int state, const QString &language) const //20261112-0 add
{
    if (state < 0 || state >= stateCount || state >= states.size()) { return QString(); }
    return states.at(state).value(language, QString::number(state));
}

/******************************************************************************
 * Function Name   : MessageDisplayProperties::languageAppearance
 * Description     : Resolve an independent language appearance without leaking edits from the previous language.
 * Input Parameters: language
 * Return Value    : Saved language appearance, or observed defaults.
 * Date            : 2026.11.12
 ******************************************************************************/
StaticTextProperties MessageDisplayProperties::languageAppearance(const QString &language) const //20261112-0 add
{
    return languageAppearances.value(language, appearance);
}

/******************************************************************************
 * Function Name   : MessageDisplayProperties::languageFrameColor
 * Description     : Resolve the separately saved border color of one message language.
 * Input Parameters: language
 * Return Value    : Language border color, or the native default.
 * Date            : 2026.11.12
 ******************************************************************************/
QColor MessageDisplayProperties::languageFrameColor(const QString &language) const //20261112-0 add
{
    return languageFrameColors.value(language, frameColor);
}

/******************************************************************************
 * Function Name   : MessageDisplayProperties::toJson
 * Description     : Serialize all active and dormant message fields into the versioned Lite project.
 * Input Parameters: None
 * Return Value    : Lossless message-property JSON, not a native source or device record.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject MessageDisplayProperties::toJson() const //20261112-0 add
{
    QJsonObject result;
    QJsonObject row;
    QJsonArray rows;
    QJsonObject appearances;
    QJsonObject frameColors;
    QMap<QString, StaticTextProperties>::const_iterator visual;
    QMap<QString, QColor>::const_iterator frame;
    QMap<QString, QString> translations;
    QMap<QString, QString>::const_iterator it;

    result.insert(QStringLiteral("stateCount"), stateCount);
    result.insert(QStringLiteral("dataType"), dataType);
    result.insert(QStringLiteral("monitorAddress"), monitorAddress);
    result.insert(QStringLiteral("movement"), movement);
    result.insert(QStringLiteral("speed"), speed);
    result.insert(QStringLiteral("appearance"), appearance.toJson());
    result.insert(QStringLiteral("frameColor"), frameColor.name(QColor::HexArgb));
    for (visual = languageAppearances.constBegin(); visual != languageAppearances.constEnd(); ++visual) { appearances.insert(visual.key(), visual.value().toJson()); }
    result.insert(QStringLiteral("languageAppearances"), appearances);
    for (frame = languageFrameColors.constBegin(); frame != languageFrameColors.constEnd(); ++frame) { frameColors.insert(frame.key(), frame.value().name(QColor::HexArgb)); }
    result.insert(QStringLiteral("languageFrameColors"), frameColors);
    Q_FOREACH (translations, states)
    {
        row = QJsonObject();
        for (it = translations.constBegin(); it != translations.constEnd(); ++it) { row.insert(it.key(), it.value()); }
        rows.append(row);
    }
    result.insert(QStringLiteral("states"), rows);
    return result;
}

/******************************************************************************
 * Function Name   : MessageDisplayProperties::fromJson
 * Description     : Reject malformed and unknown message fields before publishing a complete typed value.
 * Input Parameters: json, output, error
 * Return Value    : True only after all message fields are preserved and validated.
 * Date            : 2026.11.12
 ******************************************************************************/
bool MessageDisplayProperties::fromJson(const QJsonObject &json, MessageDisplayProperties *output, QString *error) //20261112-0 add
{
    MessageDisplayProperties candidate;
    QJsonObject defaults;
    QJsonObject row;
    QJsonObject::const_iterator it;
    QJsonObject::const_iterator translation;
    QJsonArray rows;
    QJsonObject appearances;
    QJsonObject frameColors;
    StaticTextProperties visual;
    QJsonValue value;
    QMap<QString, QString> texts;
    double number;

    if (output == nullptr) { return messageError(error, QStringLiteral("消息属性接收位置为空。")); }
    defaults = candidate.toJson();
    if (json.keys() != defaults.keys()) { return messageError(error, QStringLiteral("消息属性缺少字段或含未知字段。")); }
    for (it = json.constBegin(); it != json.constEnd(); ++it)
    {
        if (it.value().type() != defaults.value(it.key()).type()) { return messageError(error, QStringLiteral("消息属性字段类型无效：%1").arg(it.key())); }
        if (it.value().isDouble())
        {
            number = it.value().toDouble();
            if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 65535)
            { return messageError(error, QStringLiteral("消息属性数值必须为保护范围内的整数。")); }
        }
    }
    candidate.stateCount = json.value(QStringLiteral("stateCount")).toInt();
    candidate.dataType = json.value(QStringLiteral("dataType")).toInt();
    candidate.monitorAddress = json.value(QStringLiteral("monitorAddress")).toString();
    candidate.movement = json.value(QStringLiteral("movement")).toInt();
    candidate.speed = json.value(QStringLiteral("speed")).toInt();
    candidate.frameColor = QColor(json.value(QStringLiteral("frameColor")).toString());
    if (!StaticTextProperties::fromJson(json.value(QStringLiteral("appearance")).toObject(), &candidate.appearance, error)) { return false; }
    // Re-serialization catches unknown appearance fields and conversions that a legacy parser would discard.
    if (candidate.appearance.toJson() != json.value(QStringLiteral("appearance")).toObject())
    { return messageError(error, QStringLiteral("消息外观含未知、缺失或非规范字段。")); }
    appearances = json.value(QStringLiteral("languageAppearances")).toObject();
    if (appearances.size() > 64) { return messageError(error, QStringLiteral("消息外观超过Lite的64份语言草稿保护边界。")); }
    for (it = appearances.constBegin(); it != appearances.constEnd(); ++it)
    {
        if (it.key().isEmpty() || it.key().size() > 256 || !it.value().isObject() ||
            !StaticTextProperties::fromJson(it.value().toObject(), &visual, error) || visual.toJson() != it.value().toObject())
        { return messageError(error, QStringLiteral("消息语言外观无效或含未保全字段。")); }
        candidate.languageAppearances.insert(it.key(), visual);
    }
    frameColors = json.value(QStringLiteral("languageFrameColors")).toObject();
    if (frameColors.size() > 64) { return messageError(error, QStringLiteral("消息边框超过Lite的64份语言草稿保护边界。")); }
    for (it = frameColors.constBegin(); it != frameColors.constEnd(); ++it)
    {
        if (it.key().isEmpty() || it.key().size() > 256 || !it.value().isString() || !QColor(it.value().toString()).isValid())
        { return messageError(error, QStringLiteral("消息语言边框颜色无效。")); }
        candidate.languageFrameColors.insert(it.key(), QColor(it.value().toString()));
    }
    rows = json.value(QStringLiteral("states")).toArray();
    if (rows.size() < 1 || rows.size() > 128) { return messageError(error, QStringLiteral("消息状态草稿必须为1..128行。")); }
    candidate.states.clear();
    Q_FOREACH (value, rows)
    {
        if (!value.isObject()) { return messageError(error, QStringLiteral("消息翻译行必须为对象。")); }
        row = value.toObject(); texts.clear();
        for (translation = row.constBegin(); translation != row.constEnd(); ++translation)
        {
            if (translation.key().isEmpty() || translation.key().size() > 256 || !translation.value().isString() || translation.value().toString().size() > 65536)
            { return messageError(error, QStringLiteral("消息语言或文本超出Lite保护边界。")); }
            texts.insert(translation.key(), translation.value().toString());
        }
        candidate.states.append(texts);
    }
    if (!candidate.validate(error)) { return false; }
    *output = candidate;
    return true;
}

/******************************************************************************
 * Function Name   : MessageDisplayProperties::validate
 * Description     : Bound editor resources without claiming unobserved native speed or text limits.
 * Input Parameters: error
 * Return Value    : True for the supported message editing slice.
 * Date            : 2026.11.12
 ******************************************************************************/
bool MessageDisplayProperties::validate(QString *error) const //20261112-0 add
{
    StaticTextProperties visual;
    QMap<QString, QString> translations;
    QMap<QString, QString>::const_iterator text;
    if (stateCount < 1 || stateCount > 128 || states.size() != stateCount || languageAppearances.size() > 64 || languageFrameColors.size() > 64 ||
        dataType < 0 || dataType > 1 || movement < 0 || movement > 2 || speed < 1 || speed > 10 ||
        monitorAddress.trimmed().isEmpty() || monitorAddress.size() > 256 || !frameColor.isValid() ||
        appearance.fontSize < 1 || appearance.fontSize > 192 || !appearance.validate(error))
    { return messageError(error, QStringLiteral("消息显示属性超出当前编辑器支持范围。")); }
    Q_FOREACH (visual, languageAppearances)
    {
        if (!visual.validate(error) || visual.fontSize > 192) { return false; }
    }
    Q_FOREACH (translations, states)
    {
        if (translations.size() > 64) { return messageError(error, QStringLiteral("消息文本超过Lite的语言草稿数量保护。")); }
        for (text = translations.constBegin(); text != translations.constEnd(); ++text)
        {
            if (text.key().isEmpty() || text.key().size() > 256 || text.value().size() > 65536)
            { return messageError(error, QStringLiteral("消息文本超过Lite的字段长度保护。")); }
        }
    }
    return true;
}

/******************************************************************************
 * Function Name   : MessageDisplayProperties::validateAddresses
 * Description     : Validate each active read address and its full transfer span against the selected project connections.
 * Input Parameters: connections, error
 * Return Value    : True when monitor and active visibility addresses fit their observed address families.
 * Date            : 2026.11.12
 ******************************************************************************/
bool MessageDisplayProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261112-0 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress address;
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    context.connections = connections; context.role = PlcAddressRole::Word; context.transferCount = dataType == 0 ? 1U : 2U;
    if (!parseQualifiedPlcAddressText(monitorAddress, context, catalog, &address, error)) { return false; }
    if (appearance.visibilityEnabled && appearance.bitEnabled)
    {
        context.role = PlcAddressRole::Bit; context.transferCount = 1;
        if (!parseQualifiedPlcAddressText(appearance.bitAddress, context, catalog, &address, error)) { return false; }
    }
    if (appearance.visibilityEnabled && appearance.wordEnabled)
    {
        context.role = PlcAddressRole::Word; context.transferCount = appearance.dataType == 0 || appearance.dataType == 2 ? 1U : 2U;
        if (!parseQualifiedPlcAddressText(appearance.wordAddress, context, catalog, &address, error)) { return false; }
    }
    return true;
}
