#ifndef STAGE2_IO_SETTINGS_H
#define STAGE2_IO_SETTINGS_H

#include "plc_address.h"

struct HmiProject;
class QWidget;

//20261123-15 add: Retain the editor's 16-bit address space while downstream capabilities are checked separately.
inline constexpr u32 stage2IoLocalAddressCount = 65536;

/******************************************************************************
 * Function Name   : stage2IoSettingsKeys
 * Description     : Identify the distinct word-address rows in each global dialog.
 * Input Parameters: kind
 * Return Value    : Ordered property keys, or an empty list for an unknown dialog.
 * Date            : 2026.11.23
 ******************************************************************************/
inline QStringList stage2IoSettingsKeys(const QString &kind)
{
    if (kind == QStringLiteral("hmiStatus"))
    {
        return {QStringLiteral("screenIndex"), QStringLiteral("recipeIndex"), QStringLiteral("userLevel")};
    }
    if (kind == QStringLiteral("plcControl"))
    {
        return {QStringLiteral("switchScreen"), QStringLiteral("selectRecipe"),
                QStringLiteral("writeRecipe"), QStringLiteral("setUserLevel")};
    }
    return {};
}

/******************************************************************************
 * Function Name   : defaultStage2IoSettings
 * Description     : Initialize each row independently to the original disabled LW0 state.
 * Input Parameters: kind
 * Return Value    : Configuration properties in the editor's named-row schema.
 * Date            : 2026.11.23
 ******************************************************************************/
inline QJsonObject defaultStage2IoSettings(const QString &kind)
{
    QJsonObject properties;

    for (const QString &key : stage2IoSettingsKeys(kind))
    {
        properties.insert(key, QJsonObject{{QStringLiteral("enabled"), false},
                                           {QStringLiteral("address"), QStringLiteral("LW0")}});
    }
    return properties;
}

/******************************************************************************
 * Function Name   : validateStage2IoSettings
 * Description     : Reject missing, extra or invalid types before accepting a global I/O record.
 * Input Parameters: kind, properties, error
 * Return Value    : True for a complete row schema.
 * Date            : 2026.11.23
 ******************************************************************************/
inline bool validateStage2IoSettings(const QString &kind, const QJsonObject &properties, QString *error = nullptr)
{
    const QStringList keys = stage2IoSettingsKeys(kind);
    QJsonObject row;
    QJsonValue enabled;
    QJsonValue address;

    if (keys.isEmpty() || properties.size() != keys.size())
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("HMI 状态或 PLC 控制配置缺少地址行。");
        }
        return false;
    }
    for (const QString &key : keys)
    {
        row = properties.value(key).toObject();
        enabled = row.value(QStringLiteral("enabled"));
        address = row.value(QStringLiteral("address"));
        if (row.size() != 2 || !enabled.isBool() || !address.isString() || address.toString().size() > 512 ||
            (enabled.toBool() && address.toString().trimmed().isEmpty()))
        {
            if (error != nullptr)
            {
                *error = QStringLiteral("HMI 状态或 PLC 控制的地址行无效。");
            }
            return false;
        }
    }
    return true;
}

/******************************************************************************
 * Function Name   : stage2IoSettingsActive
 * Description     : Determine whether any saved row consumes an address at runtime.
 * Input Parameters: kind, properties
 * Return Value    : True if at least one row is enabled.
 * Date            : 2026.11.23
 ******************************************************************************/
inline bool stage2IoSettingsActive(const QString &kind, const QJsonObject &properties)
{
    for (const QString &key : stage2IoSettingsKeys(kind))
    {
        if (properties.value(key).toObject().value(QStringLiteral("enabled")).toBool())
        {
            return true;
        }
    }
    return false;
}

/******************************************************************************
 * Function Name   : validateStage2IoAddresses
 * Description     : Validate active rows against the project's word-address grammar and connection IDs.
 * Input Parameters: kind, properties, connections, error
 * Return Value    : True when every enabled row resolves to a word address.
 * Date            : 2026.11.23
 ******************************************************************************/
inline bool validateStage2IoAddresses(const QString &kind, const QJsonObject &properties,
                                       const QVector<PlcConnection> &connections, QString *error = nullptr)
{
    PlcAddressContext context;
    PlcCatalog catalog;
    PlcAddress address;
    QJsonObject row;
    QString text;
    bool catalogLoaded = false;

    if (!validateStage2IoSettings(kind, properties, error))
    {
        return false;
    }
    context.connections = connections;
    context.role = PlcAddressRole::Word;
    context.localAddressCount = stage2IoLocalAddressCount;
    for (const QString &key : stage2IoSettingsKeys(kind))
    {
        row = properties.value(key).toObject();
        if (!row.value(QStringLiteral("enabled")).toBool())
        {
            continue;
        }
        text = row.value(QStringLiteral("address")).toString().trimmed();
        if (text.startsWith(QLatin1Char('@')) && !catalogLoaded)
        {
            if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error))
            {
                return false;
            }
            catalogLoaded = true;
        }
        if (!parseQualifiedPlcAddressText(text, context, catalog, &address, error))
        {
            return false;
        }
    }
    return true;
}

bool editStage2IoSettings(QWidget *parent, const HmiProject &project, const QString &kind,
                          const QJsonObject &initial, QJsonObject *result);

#endif
