#ifndef STAGE2_CLOCK_PANEL_H
#define STAGE2_CLOCK_PANEL_H

#include "plc_address.h"

#include <QDate>
#include <QJsonArray>
#include <QRegularExpression>
#include <QTime>

class QWidget;
struct HmiProject;

namespace Stage2ClockPanelDetail {

/******************************************************************************
 * Function Name   : error
 * Description     : Report a configuration validation error to the caller.
 * Input Parameters: output, message
 * Return Value    : False.
 * Date            : 2026.11.23
 ******************************************************************************/
inline bool error(QString *output, const QString &message) //20261123-16 add
{
    if (output != nullptr)
    {
        *output = message;
    }
    return false;
}

/******************************************************************************
 * Function Name   : integerInRange
 * Description     : Recognize an exact integer JSON value in the requested bounds.
 * Input Parameters: value, minimum, maximum
 * Return Value    : True for a valid integer.
 * Date            : 2026.11.23
 ******************************************************************************/
inline bool integerInRange(const QJsonValue &value, int minimum, int maximum) //20261123-16 add
{
    const int integer = value.toInt(minimum - 1);

    return value.isDouble() && integer >= minimum && integer <= maximum;
}

/******************************************************************************
 * Function Name   : validClockChannel
 * Description     : Validate the exact shape of one stored clock direction.
 * Input Parameters: value
 * Return Value    : True when both active and inactive fields are well formed.
 * Date            : 2026.11.23
 ******************************************************************************/
inline bool validClockChannel(const QJsonValue &value) //20261123-16 add
{
    const QJsonObject entry = value.toObject();
    const QJsonValue dataAddress = entry.value(QStringLiteral("dataAddress"));
    const QJsonValue triggerAddress = entry.value(QStringLiteral("triggerAddress"));

    return value.isObject() && entry.size() == 5 &&
        entry.value(QStringLiteral("enabled")).isBool() &&
        entry.value(QStringLiteral("trigger")).isBool() &&
        integerInRange(entry.value(QStringLiteral("interval")), 1, 255) &&
        dataAddress.isString() && dataAddress.toString().size() <= 512 &&
        triggerAddress.isString() && triggerAddress.toString().size() <= 512;
}

} // namespace Stage2ClockPanelDetail

/******************************************************************************
 * Function Name   : defaultStage2ClockPanel
 * Description     : Construct the original desktop defaults for either settings window.
 * Input Parameters: kind
 * Return Value    : Default property object, empty for an unknown kind.
 * Date            : 2026.11.23
 ******************************************************************************/
inline QJsonObject defaultStage2ClockPanel(const QString &kind) //20261123-16 add
{
    QJsonObject channel;
    QJsonObject level;
    QJsonArray levels;
    int index;

    if (kind == QStringLiteral("clockSettings"))
    {
        channel = {{QStringLiteral("enabled"), false},
            {QStringLiteral("trigger"), false}, {QStringLiteral("interval"), 60},
            {QStringLiteral("dataAddress"), QStringLiteral("LW0")},
            {QStringLiteral("triggerAddress"), QStringLiteral("LB0")}};
        return {{QStringLiteral("downloadToHmi"), false},
            {QStringLiteral("write"), channel}, {QStringLiteral("read"), channel}};
    }
    if (kind == QStringLiteral("panelProtection")) //20261123-18 add
    {
        level = {{QStringLiteral("date"), QStringLiteral("2021-01-01")},
            {QStringLiteral("time"), QStringLiteral("12:00:00")},
            {QStringLiteral("pageId"), QString()}, {QStringLiteral("password"), QString()}};
        for (index = 0; index < 10; ++index)
        {
            levels.append(level);
        }
        return {{QStringLiteral("enabled"), false}, {QStringLiteral("levelCount"), 1},
            {QStringLiteral("levels"), levels}};
    }
    return {};
}

/******************************************************************************
 * Function Name   : validateStage2ClockPanel
 * Description     : Check the versioned property shape and never accept two active clock directions.
 * Input Parameters: kind, properties, error
 * Return Value    : True when the property record can be retained in a project.
 * Date            : 2026.11.23
 ******************************************************************************/
inline bool validateStage2ClockPanel(const QString &kind, const QJsonObject &properties,
                                     QString *error = nullptr) //20261123-16 add
{
    static const QRegularExpression decimalPassword(QStringLiteral("\\A[0-9]{0,8}\\z"));
    const QJsonValue read = properties.value(QStringLiteral("read"));
    const QJsonValue write = properties.value(QStringLiteral("write"));
    const QJsonValue entries = properties.value(QStringLiteral("levels"));
    const QJsonArray levels = entries.toArray();
    const QJsonValue count = properties.value(QStringLiteral("levelCount"));
    const QJsonValue enabled = properties.value(QStringLiteral("enabled"));
    QJsonObject level;
    QJsonValue date;
    QJsonValue time;
    QJsonValue pageId;
    QJsonValue password;
    QDate parsedDate;
    QTime parsedTime;
    int index;

    if (kind == QStringLiteral("clockSettings"))
    {
        if (properties.size() == 3 && properties.value(QStringLiteral("downloadToHmi")).isBool() &&
            Stage2ClockPanelDetail::validClockChannel(read) && Stage2ClockPanelDetail::validClockChannel(write) &&
            !(read.toObject().value(QStringLiteral("enabled")).toBool() &&
              write.toObject().value(QStringLiteral("enabled")).toBool()))
        {
            return true;
        }
        return Stage2ClockPanelDetail::error(error, QStringLiteral("时钟配置无效。写时间到PLC和与PLC时间同步不能同时启用。"));
    }
    if (kind != QStringLiteral("panelProtection") || properties.size() != 3 || !enabled.isBool() ||
        !Stage2ClockPanelDetail::integerInRange(count, 1, 10) || !entries.isArray() || levels.size() != 10)
    {
        return Stage2ClockPanelDetail::error(error, QStringLiteral("HMI保护必须包含1–10级及对应的期限设置。"));
    }
    for (index = 0; index < levels.size(); ++index) //20261123-18 add: Keep inactive levels as editable drafts.
    {
        level = levels.at(index).toObject();
        date = level.value(QStringLiteral("date"));
        time = level.value(QStringLiteral("time"));
        pageId = level.value(QStringLiteral("pageId"));
        password = level.value(QStringLiteral("password"));
        parsedDate = QDate::fromString(date.toString(), Qt::ISODate);
        parsedTime = QTime::fromString(time.toString(), Qt::ISODate);
        if (!levels.at(index).isObject() || level.size() != 4 ||
            !date.isString() || !parsedDate.isValid() || parsedDate.toString(Qt::ISODate) != date.toString() ||
            !time.isString() || !parsedTime.isValid() || parsedTime.toString(Qt::ISODate) != time.toString() ||
            !pageId.isString() || pageId.toString().size() > 256 ||
            (enabled.toBool() && index < count.toInt() && pageId.toString().trimmed().isEmpty()) ||
            !password.isString() || !decimalPassword.match(password.toString()).hasMatch())
        {
            return Stage2ClockPanelDetail::error(error, QStringLiteral("第%1级保护的日期、时间、提示画面或密码无效。").arg(index + 1));
        }
        if (enabled.toBool() && index < count.toInt() && password.toString().isEmpty()) //20261123-18 add
        {
            return Stage2ClockPanelDetail::error(error, QStringLiteral("第%1级密码为空！").arg(index + 1));
        }
    }
    return true;
}

/******************************************************************************
 * Function Name   : stage2ClockPanelActive
 * Description     : Identify whether a saved draft would change device or simulation behavior.
 * Input Parameters: kind, properties
 * Return Value    : True only when a device operation is enabled.
 * Date            : 2026.11.23
 ******************************************************************************/
inline bool stage2ClockPanelActive(const QString &kind, const QJsonObject &properties) //20261123-16 add
{
    if (kind == QStringLiteral("clockSettings"))
    {
        return properties.value(QStringLiteral("downloadToHmi")).toBool() ||
            properties.value(QStringLiteral("write")).toObject().value(QStringLiteral("enabled")).toBool() ||
            properties.value(QStringLiteral("read")).toObject().value(QStringLiteral("enabled")).toBool();
    }
    return kind == QStringLiteral("panelProtection") && properties.value(QStringLiteral("enabled")).toBool();
}

/******************************************************************************
 * Function Name   : stage2ActivePanelTargets
 * Description     : Return each active protection level's stable expiry-screen ID in level order.
 * Input Parameters: properties
 * Return Value    : IDs for enabled levels, empty when protection is off.
 * Date            : 2026.11.23
 ******************************************************************************/
inline QStringList stage2ActivePanelTargets(const QJsonObject &properties) //20261123-18 add
{
    const QJsonArray levels = properties.value(QStringLiteral("levels")).toArray();
    const int count = qMin(properties.value(QStringLiteral("levelCount")).toInt(), int(levels.size()));
    QStringList targets;
    int index;

    if (properties.value(QStringLiteral("enabled")).toBool())
    {
        for (index = 0; index < count; ++index)
        {
            targets.append(levels.at(index).toObject().value(QStringLiteral("pageId")).toString());
        }
    }
    return targets;
}

/******************************************************************************
 * Function Name   : validateStage2ClockPanelAddresses
 * Description     : Validate an enabled clock path's 7-word range and optional bit trigger.
 * Input Parameters: kind, properties, connections, error
 * Return Value    : True when each used address has the correct family and valid full span.
 * Date            : 2026.11.23
 ******************************************************************************/
inline bool validateStage2ClockPanelAddresses(const QString &kind, const QJsonObject &properties,
                                              const QVector<PlcConnection> &connections,
                                              QString *error = nullptr) //20261123-16 add
{
    const QStringList directions = {QStringLiteral("write"), QStringLiteral("read")};
    PlcAddressContext context;
    PlcCatalog catalog;
    PlcAddress parsed;
    QJsonObject channel;
    QString direction;

    if (kind != QStringLiteral("clockSettings"))
    {
        return true;
    }
    context.connections = connections;
    context.localAddressCount = 65536;
    for (const QString &id : directions)
    {
        channel = properties.value(id).toObject();
        if (!channel.value(QStringLiteral("enabled")).toBool())
        {
            continue;
        }
        direction = id == QStringLiteral("write") ? QStringLiteral("写入") : QStringLiteral("读取");
        context.role = PlcAddressRole::Word;
        context.transferCount = 7;
        if (!catalog.load() ||
            !parseQualifiedPlcAddressText(channel.value(QStringLiteral("dataAddress")).toString(),
                context, catalog, &parsed))
        {
            return Stage2ClockPanelDetail::error(error, QStringLiteral("时钟%1地址必须支持连续7字。").arg(direction));
        }
        context.role = PlcAddressRole::Bit;
        context.transferCount = 1;
        if (channel.value(QStringLiteral("trigger")).toBool() &&
            !parseQualifiedPlcAddressText(channel.value(QStringLiteral("triggerAddress")).toString(),
                context, catalog, &parsed))
        {
            return Stage2ClockPanelDetail::error(error, QStringLiteral("时钟%1触发位地址无效。").arg(direction));
        }
    }
    return true;
}

bool editStage2ClockPanel(QWidget *parent, const HmiProject &project, const QString &kind,
                          const QJsonObject &initial, QJsonObject *result); //20261123-16 add

#endif
