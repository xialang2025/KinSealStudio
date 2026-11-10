#include "plc_address.h"
#include "plc_native_catalog.h" //20261109-0 add

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSet>
#include <cmath>
#include <limits>

//20261109-0 add: No vendor library is loaded; only explicitly recovered native fields are emitted.
namespace
{
/******************************************************************************
 * Function Name   : addressError
 * Description     : Report a validation failure without touching the caller's result.
 * Input Parameters: error, message
 * Return Value    : False
 * Date            : 2026.11.09
 ******************************************************************************/
bool addressError(QString *error, const QString &message)
{
    if (error != nullptr)
    {
        *error = message;
    }
    return false;
}

/******************************************************************************
 * Function Name   : clearAddressError
 * Description     : Clear stale diagnostics after a successful operation.
 * Input Parameters: error
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void clearAddressError(QString *error)
{
    if (error != nullptr)
    {
        error->clear();
    }
}

/******************************************************************************
 * Function Name   : jsonStrings
 * Description     : Extract an observed list of strings from catalog metadata.
 * Input Parameters: value
 * Return Value    : The string list
 * Date            : 2026.11.09
 ******************************************************************************/
QStringList jsonStrings(const QJsonValue &value)
{
    QStringList result;
    QJsonArray array;
    int index;

    array = value.toArray();
    for (index = 0; index < array.size(); ++index)
    {
        if (array[index].isString())
        {
            result.append(array[index].toString());
        }
    }
    return result;
}

/******************************************************************************
 * Function Name   : channelText
 * Description     : Serialize a project channel without inferring its native route byte.
 * Input Parameters: channel
 * Return Value    : A stable channel identifier
 * Date            : 2026.11.09
 ******************************************************************************/
QString channelText(PlcChannel channel)
{
    if (channel == PlcChannel::Com1)
    {
        return QStringLiteral("COM1");
    }
    if (channel == PlcChannel::Com2)
    {
        return QStringLiteral("COM2");
    }
    return QStringLiteral("unspecified");
}

/******************************************************************************
 * Function Name   : modbusFamilyIndex
 * Description     : Locate the twelve family indices recovered from Common V5.1.
 * Input Parameters: family
 * Return Value    : Index 0..11, or -1 for an unsupported family
 * Date            : 2026.11.09
 ******************************************************************************/
int modbusFamilyIndex(const QString &family)
{
    const QString names[12] = {
        QStringLiteral("0x"), QStringLiteral("1x"), QStringLiteral("3x_bit"),
        QStringLiteral("4x_bit"), QStringLiteral("6x_bit"), QStringLiteral("10x_bit"),
        QStringLiteral("3x"), QStringLiteral("4x"), QStringLiteral("3x_dw"),
        QStringLiteral("4x_dw"), QStringLiteral("6x"), QStringLiteral("10x")
    };
    int index;

    for (index = 0; index < 12; ++index)
    {
        if (family == names[index])
        {
            return index;
        }
    }
    return -1;
}

/******************************************************************************
 * Function Name   : readUnsigned
 * Description     : Accept a bounded ASCII integer, excluding negative and libc-specific forms.
 * Input Parameters: text, radix, number
 * Return Value    : True for a decimal or hexadecimal unsigned 32-bit value
 * Date            : 2026.11.09
 ******************************************************************************/
bool readUnsigned(const QString &text, int radix, u32 *number)
{
    unsigned long long result;
    unsigned long long digit;
    int index;
    int start;
    u16 code;

    if (number == nullptr || text.isEmpty() || (radix != 10 && radix != 16))
    {
        return false;
    }
    result = 0;
    start = text.startsWith(QChar(u'+')) ? 1 : 0;
    if (start == text.size())
    {
        return false;
    }
    for (index = start; index < text.size(); ++index)
    {
        code = text[index].unicode();
        if (code >= u'0' && code <= u'9')
        {
            digit = code - u'0';
        }
        else if (radix == 16 && code >= u'a' && code <= u'f')
        {
            digit = code - u'a' + 10;
        }
        else if (radix == 16 && code >= u'A' && code <= u'F')
        {
            digit = code - u'A' + 10;
        }
        else
        {
            return false;
        }
        if (digit >= static_cast<unsigned long long>(radix))
        {
            return false;
        }
        result = result * static_cast<unsigned long long>(radix) + digit;
        if (result > std::numeric_limits<u32>::max())
        {
            return false;
        }
    }
    *number = u32(result);
    return true;
}

/******************************************************************************
 * Function Name   : numericValue
 * Description     : Apply the observed transfer-count bounds and A*16+B conversion.
 * Input Parameters: text, familyIndex, internal, radix, count, localCount, number, canonical, error
 * Return Value    : True with an unchanged-on-failure canonical numeric result
 * Date            : 2026.11.09
 ******************************************************************************/
bool numericValue(const QString &text, int familyIndex, bool internal, int radix,
                   u32 count, u32 localCount, u32 *number, QString *canonical,
                   QString *error)
{
    QStringList parts;
    QString normalized;
    unsigned long long limit;
    unsigned long long encoded;
    u32 major;
    u32 minor;

    if (count == 0 || (radix != 10 && radix != 16))
    {
        return addressError(error, QStringLiteral("地址访问数量必须大于零，显示进制仅支持十进制或十六进制。"));
    }
    major = 0;
    minor = 0;
    if (!internal && familyIndex >= 2 && familyIndex <= 5)
    {
        parts = text.split(QChar(u'.'), Qt::KeepEmptyParts);
        if (parts.size() != 2 || parts[1].size() >= 3 ||
            !readUnsigned(parts[0], radix, &major) || !readUnsigned(parts[1], 10, &minor) ||
            minor > 15)
        {
            return addressError(error, QStringLiteral("寄存器位地址必须是 A.B，B 为 0..15 的一位或两位十进制数。"));
        }
        encoded = static_cast<unsigned long long>(major) * 16 + minor;
        limit = 0x100000;
        normalized = QString::number(major) + QChar(u'.') +
                     QString::number(minor).rightJustified(2, QChar(u'0'));
    }
    else
    {
        if (!readUnsigned(text, radix, &major))
        {
            return addressError(error, QStringLiteral("地址必须是非负整数，不支持负数、小数或进制前缀。"));
        }
        encoded = major;
        // Common allows one extra 1x address; this is a static encoding fact, not a PDU claim.
        limit = internal ? localCount : (familyIndex == 1 ? 0x10001 : 0x10000);
        normalized = QString::number(major);
    }
    if (limit == 0 || encoded + count > limit)
    {
        return addressError(error, QStringLiteral("地址和访问数量超出已验证范围。"));
    }
    *number = u32(encoded);
    *canonical = normalized;
    return true;
}
}

/******************************************************************************
 * Function Name   : PlcCatalog::load
 * Description     : Validate the complete shipped profile inventory before publishing it.
 * Input Parameters: path, error
 * Return Value    : True when the catalog is valid; previous metadata survives failures
 * Date            : 2026.11.09
 ******************************************************************************/
bool PlcCatalog::load(const QString &path, QString *error) //20261109-0 add
{
    QFile file;
    QJsonParseError parseError;
    QJsonDocument document;
    QJsonObject root;
    QJsonObject rawProfile;
    QJsonObject rawFamily;
    QJsonArray profiles;
    QJsonArray families;
    QVector<PlcProfile> parsed;
    QSet<QString> ids;
    QSet<QString> names;
    PlcProfile profile;
    PlcFamily family;
    int profileIndex;
    int familyIndex;

    file.setFileName(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        return addressError(error, QStringLiteral("无法读取 PLC 目录：%1").arg(file.errorString()));
    }
    document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
    {
        return addressError(error, QStringLiteral("PLC 目录 JSON 无效：%1").arg(parseError.errorString()));
    }
    root = document.object();
    profiles = root.value(QStringLiteral("profiles")).toArray();
    if (root.value(QStringLiteral("version")).toInt() != 1 || profiles.isEmpty() ||
        root.value(QStringLiteral("profileCount")).toInt() != profiles.size())
    {
        return addressError(error, QStringLiteral("PLC 目录版本或配置总数不一致。"));
    }
    for (profileIndex = 0; profileIndex < profiles.size(); ++profileIndex)
    {
        rawProfile = profiles[profileIndex].toObject();
        profile = PlcProfile();
        profile.id = rawProfile.value(QStringLiteral("id")).toString();
        profile.name = rawProfile.value(QStringLiteral("name")).toString();
        profile.manufacturer = rawProfile.value(QStringLiteral("manufacturerDirectory")).toString();
        profile.route = rawProfile.value(QStringLiteral("route")).toString();
        profile.driverName = rawProfile.value(QStringLiteral("driverName")).toString();
        profile.sourceSha256 = rawProfile.value(QStringLiteral("sourceSha256")).toString();
        profile.serialDefaults = rawProfile.value(QStringLiteral("serialDefaults")).toObject();
        profile.rawFields = rawProfile.value(QStringLiteral("rawFields")).toObject();
        profile.warnings = jsonStrings(rawProfile.value(QStringLiteral("warnings")));
        profile.metadata = rawProfile;
        if (profile.id.isEmpty() || profile.name.isEmpty() || ids.contains(profile.id) ||
            !rawProfile.value(QStringLiteral("families")).isArray())
        {
            return addressError(error, QStringLiteral("PLC 目录包含空名称、重复 ID 或无效地址族。"));
        }
        ids.insert(profile.id);
        families = rawProfile.value(QStringLiteral("families")).toArray();
        names.clear();
        for (familyIndex = 0; familyIndex < families.size(); ++familyIndex)
        {
            rawFamily = families[familyIndex].toObject();
            family = PlcFamily();
            family.name = rawFamily.value(QStringLiteral("name")).toString();
            family.roles = jsonStrings(rawFamily.value(QStringLiteral("roles")));
            family.rawRadix = rawFamily.value(QStringLiteral("rawRadix")).toString();
            family.rawRange = rawFamily.value(QStringLiteral("rawRange")).toString();
            family.grammarCandidate = rawFamily.value(QStringLiteral("grammarCandidate")).toString();
            family.majorRadix = rawFamily.value(QStringLiteral("majorRadixCandidate")).toInt();
            family.minorRadix = rawFamily.value(QStringLiteral("minorRadixCandidate")).toInt();
            family.metadata = rawFamily;
            if (family.name.isEmpty() || names.contains(family.name))
            {
                return addressError(error, QStringLiteral("PLC 目录包含空地址族或重复地址族。"));
            }
            names.insert(family.name);
            profile.families.append(family);
        }
        parsed.append(profile);
    }
    m_profiles = parsed;
    m_metadata = root;
    clearAddressError(error);
    return true;
}

/******************************************************************************
 * Function Name   : PlcCatalog::profiles
 * Description     : Expose all observed profiles without filtering away legacy entries.
 * Input Parameters: None
 * Return Value    : The catalog profiles
 * Date            : 2026.11.09
 ******************************************************************************/
const QVector<PlcProfile> &PlcCatalog::profiles() const
{
    return m_profiles;
}

/******************************************************************************
 * Function Name   : PlcCatalog::profile
 * Description     : Resolve a stable source-path identifier.
 * Input Parameters: id
 * Return Value    : Matching profile, or nullptr
 * Date            : 2026.11.09
 ******************************************************************************/
const PlcProfile *PlcCatalog::profile(const QString &id) const
{
    int index;

    for (index = 0; index < m_profiles.size(); ++index)
    {
        if (m_profiles[index].id == id)
        {
            return &m_profiles[index];
        }
    }
    return nullptr;
}

/******************************************************************************
 * Function Name   : PlcCatalog::profilesForModel
 * Description     : Follow the observed model driver tree; unknown models have no guessed route.
 * Input Parameters: modelName
 * Return Value    : Profiles on the model's observed driver tree
 * Date            : 2026.11.09
 ******************************************************************************/
QVector<PlcProfile> PlcCatalog::profilesForModel(const QString &modelName) const
{
    QVector<PlcProfile> result;
    QJsonArray models;
    QJsonObject model;
    QString drivePath;
    QString route;
    int index;

    models = m_metadata.value(QStringLiteral("models")).toArray();
    for (index = 0; index < models.size(); ++index)
    {
        model = models[index].toObject();
        if (model.value(QStringLiteral("name")).toString() == modelName)
        {
            drivePath = model.value(QStringLiteral("drivePath")).toString();
            break;
        }
    }
    for (index = 0; index < m_profiles.size(); ++index)
    {
        route = QStringLiteral("Link/") + m_profiles[index].route + QChar(u'/');
        if (!drivePath.isEmpty() && drivePath == route)
        {
            result.append(m_profiles[index]);
        }
    }
    return result;
}

/******************************************************************************
 * Function Name   : PlcCatalog::families
 * Description     : Query declared role membership without inventing another PLC's syntax.
 * Input Parameters: profileId, role
 * Return Value    : Matching family metadata
 * Date            : 2026.11.09
 ******************************************************************************/
QVector<PlcFamily> PlcCatalog::families(const QString &profileId, PlcAddressRole role) const
{
    QVector<PlcFamily> result;
    const PlcProfile *selected;
    int index;

    selected = profile(profileId);
    if (selected != nullptr)
    {
        for (index = 0; index < selected->families.size(); ++index)
        {
            if (plcFamilyHasRole(selected->families[index], role))
            {
                result.append(selected->families[index]);
            }
        }
    }
    return result;
}

/******************************************************************************
 * Function Name   : PlcCatalog::metadata
 * Description     : Preserve raw warnings, source provenance and observed model routes.
 * Input Parameters: None
 * Return Value    : The complete JSON metadata
 * Date            : 2026.11.09
 ******************************************************************************/
QJsonObject PlcCatalog::metadata() const
{
    return m_metadata;
}

/******************************************************************************
 * Function Name   : PlcConnection::toJson
 * Description     : Store station and nullable node as separate project fields.
 * Input Parameters: None
 * Return Value    : A structured connection object
 * Date            : 2026.11.09
 ******************************************************************************/
QJsonObject PlcConnection::toJson() const
{
    QJsonObject json;

    json.insert(QStringLiteral("id"), id);
    json.insert(QStringLiteral("profileId"), profileId);
    json.insert(QStringLiteral("channel"), channelText(channel));
    json.insert(QStringLiteral("station"), station);
    json.insert(QStringLiteral("node"), nodeKnown ? QJsonValue(int(node)) : QJsonValue(QJsonValue::Null));
    return json;
}

/******************************************************************************
 * Function Name   : PlcConnection::fromJson
 * Description     : Read offline connections without treating station as native node evidence.
 * Input Parameters: json, connection, error
 * Return Value    : True for structurally valid data, including unknown profile identifiers
 * Date            : 2026.11.09
 ******************************************************************************/
bool PlcConnection::fromJson(const QJsonObject &json, PlcConnection *connection, QString *error)
{
    PlcConnection parsed;
    QString channel;
    QJsonValue node;
    double numericNode;

    if (connection == nullptr || !json.value(QStringLiteral("id")).isString() ||
        !json.value(QStringLiteral("profileId")).isString() ||
        !json.value(QStringLiteral("station")).isString() ||
        !json.value(QStringLiteral("channel")).isString())
    {
        return addressError(error, QStringLiteral("PLC 连接缺少字符串 ID、配置、通道或独立站号。"));
    }
    parsed.id = json.value(QStringLiteral("id")).toString();
    parsed.profileId = json.value(QStringLiteral("profileId")).toString();
    parsed.station = json.value(QStringLiteral("station")).toString();
    channel = json.value(QStringLiteral("channel")).toString();
    node = json.value(QStringLiteral("node"));
    if (parsed.id.isEmpty() || parsed.profileId.isEmpty())
    {
        return addressError(error, QStringLiteral("PLC 连接 ID 和配置 ID 不能为空。"));
    }
    if (channel == QStringLiteral("COM1"))
    {
        parsed.channel = PlcChannel::Com1;
    }
    else if (channel == QStringLiteral("COM2"))
    {
        parsed.channel = PlcChannel::Com2;
    }
    else if (channel != QStringLiteral("unspecified"))
    {
        return addressError(error, QStringLiteral("PLC 通道标识无效。"));
    }
    if (!node.isNull() && !node.isUndefined())
    {
        numericNode = node.toDouble(-1);
        if (!node.isDouble() || numericNode < 0 || numericNode > 255 || std::floor(numericNode) != numericNode)
        {
            return addressError(error, QStringLiteral("PLC 节点必须是独立的 0..255 整数或 null。"));
        }
        parsed.nodeKnown = true;
        parsed.node = u8(numericNode);
    }
    *connection = parsed;
    clearAddressError(error);
    return true;
}

/******************************************************************************
 * Function Name   : PlcAddress::toJson
 * Description     : Keep source scope, connection identity, family and value independent.
 * Input Parameters: None
 * Return Value    : A structured address object
 * Date            : 2026.11.09
 ******************************************************************************/
QJsonObject PlcAddress::toJson() const
{
    QJsonObject json;

    json.insert(QStringLiteral("space"), space == PlcAddressSpace::Internal ?
                QStringLiteral("internal") : QStringLiteral("external"));
    json.insert(QStringLiteral("connectionId"), connectionId);
    json.insert(QStringLiteral("family"), family);
    json.insert(QStringLiteral("value"), value);
    return json;
}

/******************************************************************************
 * Function Name   : PlcAddress::fromJson
 * Description     : Preserve unsupported offline family syntax for later verified codecs.
 * Input Parameters: json, address, error
 * Return Value    : True for structurally consistent data; numeric support is checked separately
 * Date            : 2026.11.09
 ******************************************************************************/
bool PlcAddress::fromJson(const QJsonObject &json, PlcAddress *address, QString *error)
{
    PlcAddress parsed;
    QString space;

    if (address == nullptr || !json.value(QStringLiteral("space")).isString() ||
        !json.value(QStringLiteral("connectionId")).isString() ||
        !json.value(QStringLiteral("family")).isString() || !json.value(QStringLiteral("value")).isString())
    {
        return addressError(error, QStringLiteral("结构化 PLC 地址字段不完整。"));
    }
    space = json.value(QStringLiteral("space")).toString();
    parsed.connectionId = json.value(QStringLiteral("connectionId")).toString();
    parsed.family = json.value(QStringLiteral("family")).toString();
    parsed.value = json.value(QStringLiteral("value")).toString();
    if (space == QStringLiteral("external"))
    {
        parsed.space = PlcAddressSpace::External;
    }
    else if (space != QStringLiteral("internal"))
    {
        return addressError(error, QStringLiteral("PLC 地址空间无效。"));
    }
    if (parsed.family.isEmpty() || parsed.value.isEmpty() ||
        (parsed.space == PlcAddressSpace::Internal && !parsed.connectionId.isEmpty()) ||
        (parsed.space == PlcAddressSpace::External && parsed.connectionId.isEmpty()))
    {
        return addressError(error, QStringLiteral("PLC 地址的空间、连接或地址族不一致。"));
    }
    *address = parsed;
    clearAddressError(error);
    return true;
}

/******************************************************************************
 * Function Name   : plcFamilyHasRole
 * Description     : Use observed BIT/WORT/DWORT membership for picker filtering.
 * Input Parameters: family, role
 * Return Value    : True if the role is declared, or any role is requested
 * Date            : 2026.11.09
 ******************************************************************************/
bool plcFamilyHasRole(const PlcFamily &family, PlcAddressRole role)
{
    if (role == PlcAddressRole::Any)
    {
        return true;
    }
    if (role == PlcAddressRole::Bit)
    {
        return family.roles.contains(QStringLiteral("bit"));
    }
    if (role == PlcAddressRole::Word)
    {
        return family.roles.contains(QStringLiteral("word"));
    }
    return family.roles.contains(QStringLiteral("dword"));
}

/******************************************************************************
 * Function Name   : plcProfileHasObservedDriver
 * Description     : Check the actual target file inventory for an explicit project channel.
 * Input Parameters: profile, channel
 * Return Value    : True when the selected target binary was observed
 * Date            : 2026.11.09
 ******************************************************************************/
bool plcProfileHasObservedDriver(const PlcProfile &profile, PlcChannel channel)
{
    QJsonObject observed;

    observed = profile.metadata.value(QStringLiteral("driverFilesObserved")).toObject();
    if (profile.driverName.isEmpty())
    {
        return false;
    }
    if (channel == PlcChannel::Com1)
    {
        return observed.value(QStringLiteral("com1Present")).toBool();
    }
    if (channel == PlcChannel::Com2)
    {
        return observed.value(QStringLiteral("com2Present")).toBool();
    }
    return false;
}

/******************************************************************************
 * Function Name   : plcProfileHasModbusCodec
 * Description     : Restrict numeric translation to the exact observed Modbus RTU configuration.
 * Input Parameters: profile
 * Return Value    : True for either hash-identical default Modbus RTU source path
 * Date            : 2026.11.09
 ******************************************************************************/
bool plcProfileHasModbusCodec(const PlcProfile &profile)
{
    return (profile.id == QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini") ||
            profile.id == QStringLiteral("Link/serial/MODBUS/Modbus_RTU.ini")) &&
           profile.driverName == QStringLiteral("Modbus_RTU") &&
           profile.sourceSha256 == QStringLiteral("78fdc0aa284d545c0a1a55bfa98fa0bb3bb49e464f670212996ee818eb01a477");
}

/******************************************************************************
 * Function Name   : plcConnection
 * Description     : Resolve exactly one connection, rejecting ambiguous duplicate IDs.
 * Input Parameters: context, id
 * Return Value    : Matching connection, or nullptr
 * Date            : 2026.11.09
 ******************************************************************************/
const PlcConnection *plcConnection(const PlcAddressContext &context, const QString &id)
{
    const PlcConnection *found;
    int index;

    found = nullptr;
    for (index = 0; index < context.connections.size(); ++index)
    {
        if (context.connections[index].id == id)
        {
            if (found != nullptr)
            {
                return nullptr;
            }
            found = &context.connections[index];
        }
    }
    return found;
}

/******************************************************************************
 * Function Name   : parsePlcAddress
 * Description     : Normalize supported syntax and enforce family roles and transfer bounds.
 * Input Parameters: family, value, connectionId, context, catalog, address, error, displayRadix
 * Return Value    : True on supported input, preserving the result on failure
 * Date            : 2026.11.09
 ******************************************************************************/
bool parsePlcAddress(const QString &family, const QString &value, const QString &connectionId,
                     const PlcAddressContext &context, const PlcCatalog &catalog,
                     PlcAddress *address, QString *error, int displayRadix) //20261109-0 add
{
    PlcAddress parsed;
    PlcFamily localFamily;
    PlcNativeParsedAddress nativeAddress; //20261109-0 add
    const PlcConnection *connection;
    const PlcProfile *profile = nullptr; //20261109-0 change
    const PlcFamily *selectedFamily;
    QString canonical;
    QString familyName;
    u32 numeric;
    int index;
    int familyIndex;

    if (address == nullptr)
    {
        return addressError(error, QStringLiteral("PLC 地址输出为空。"));
    }
    parsed.connectionId = connectionId;
    familyName = family.trimmed();
    familyIndex = -1;
    selectedFamily = nullptr;
    if (connectionId.isEmpty())
    {
        parsed.family = familyName.toUpper();
        if (parsed.family != QStringLiteral("LB") && parsed.family != QStringLiteral("LW"))
        {
            return addressError(error, QStringLiteral("内部地址仅支持 LB 和 LW。"));
        }
        localFamily.roles = parsed.family == QStringLiteral("LB") ? QStringList{QStringLiteral("bit")} :
                            QStringList{QStringLiteral("word"), QStringLiteral("dword")};
        selectedFamily = &localFamily;
    }
    else
    {
        parsed.space = PlcAddressSpace::External;
        connection = plcConnection(context, connectionId);
        profile = connection == nullptr ? nullptr : catalog.profile(connection->profileId);
        if (profile == nullptr)
        {
            return addressError(error, QStringLiteral("PLC 连接未知、重复，或其配置未在目录中。"));
        }
        for (index = 0; index < profile->families.size(); ++index)
        {
            if (profile->families[index].name.compare(familyName, Qt::CaseInsensitive) == 0)
            {
                selectedFamily = &profile->families[index];
                parsed.family = selectedFamily->name;
                break;
            }
        }
        familyIndex = modbusFamilyIndex(parsed.family);
        if (selectedFamily == nullptr)
        {
            return addressError(error, QStringLiteral("所选PLC地址族未知。"));
        }
    }
    if (!plcFamilyHasRole(*selectedFamily, context.role))
    {
        return addressError(error, QStringLiteral("所选地址族不适用于当前位/字/双字用途。"));
    }
    if (parsed.space == PlcAddressSpace::External && !plcProfileHasModbusCodec(*profile)) //20261109-0 change
    {
        if (displayRadix != 10)
        {
            return addressError(error, QStringLiteral("该PLC地址使用其原生进制和分段格式，不能转换为通用十六进制显示。"));
        }
        if (!parseNativePlcAddress(*profile, parsed.family, value.trimmed(), context.transferCount,
                                   &nativeAddress, error)) { return false; }
        canonical = nativeAddress.canonicalValue;
    }
    else if (!numericValue(value.trimmed(), familyIndex, parsed.space == PlcAddressSpace::Internal,
                      displayRadix, context.transferCount, context.localAddressCount,
                      &numeric, &canonical, error))
    {
        return false;
    }
    parsed.value = canonical;
    *address = parsed;
    clearAddressError(error);
    return true;
}

/******************************************************************************
 * Function Name   : parsePlcAddressText
 * Description     : Split an address using the longest family prefix, including 4x_bit and 4x_dw.
 * Input Parameters: text, connectionId, context, catalog, address, error, displayRadix
 * Return Value    : True for a supported address string
 * Date            : 2026.11.09
 ******************************************************************************/
bool parsePlcAddressText(const QString &text, const QString &connectionId,
                         const PlcAddressContext &context, const PlcCatalog &catalog,
                         PlcAddress *address, QString *error, int displayRadix)
{
    QString trimmed;
    QString prefix;
    const PlcConnection *connection;
    const PlcProfile *profile;
    int index;

    trimmed = text.trimmed();
    if (connectionId.isEmpty())
    {
        prefix = trimmed.left(2);
    }
    else
    {
        connection = plcConnection(context, connectionId);
        profile = connection == nullptr ? nullptr : catalog.profile(connection->profileId);
        if (profile != nullptr)
        {
            for (index = 0; index < profile->families.size(); ++index)
            {
                if (trimmed.startsWith(profile->families[index].name, Qt::CaseInsensitive) &&
                    profile->families[index].name.size() > prefix.size())
                {
                    prefix = profile->families[index].name;
                }
            }
        }
    }
    if (prefix.isEmpty())
    {
        return addressError(error, QStringLiteral("PLC 地址没有已知的地址族前缀。"));
    }
    return parsePlcAddress(prefix, trimmed.mid(prefix.size()), connectionId, context,
                           catalog, address, error, displayRadix);
}

/******************************************************************************
 * Function Name   : validatePlcAddress
 * Description     : Check scope consistency and canonical supported numeric semantics.
 * Input Parameters: address, context, catalog, error
 * Return Value    : True for an address that can be represented by an implemented codec
 * Date            : 2026.11.09
 ******************************************************************************/
bool validatePlcAddress(const PlcAddress &address, const PlcAddressContext &context,
                        const PlcCatalog &catalog, QString *error)
{
    PlcAddress parsed;

    if ((address.space == PlcAddressSpace::Internal && !address.connectionId.isEmpty()) ||
        (address.space == PlcAddressSpace::External && address.connectionId.isEmpty()))
    {
        return addressError(error, QStringLiteral("PLC 地址空间与连接 ID 不一致。"));
    }
    return parsePlcAddress(address.family, address.value, address.connectionId,
                           context, catalog, &parsed, error);
}

/******************************************************************************
 * Function Name   : parseQualifiedPlcAddressText
 * Description     : Resolve @connectionId:familyValue without guessing a connection for bare text.
 * Input Parameters: text, context, catalog, address, error, displayRadix
 * Return Value    : True for a supported internal or explicitly qualified external address
 * Date            : 2026.11.09
 ******************************************************************************/
bool parseQualifiedPlcAddressText(const QString &text, const PlcAddressContext &context,
                                  const PlcCatalog &catalog, PlcAddress *address,
                                  QString *error, int displayRadix)
{
    QString trimmed;
    QString connectionId;
    QString body;
    int delimiter;

    trimmed = text.trimmed();
    body = trimmed;
    if (trimmed.startsWith(QChar(u'@')))
    {
        delimiter = trimmed.indexOf(QChar(u':'));
        if (delimiter <= 1 || delimiter == trimmed.size() - 1)
        {
            return addressError(error, QStringLiteral("外部地址格式必须是 @连接ID:地址族与数值。"));
        }
        connectionId = trimmed.mid(1, delimiter - 1);
        body = trimmed.mid(delimiter + 1);
    }
    return parsePlcAddressText(body, connectionId, context, catalog, address, error, displayRadix);
}

/******************************************************************************
 * Function Name   : plcAddressValueText
 * Description     : Format canonical decimal storage in an explicitly chosen display radix.
 * Input Parameters: address, displayRadix
 * Return Value    : Formatted value, or an empty string for unsupported conversion
 * Date            : 2026.11.09
 ******************************************************************************/
QString plcAddressValueText(const PlcAddress &address, int displayRadix)
{
    QStringList parts;
    u32 major;

    if (displayRadix == 10)
    {
        return address.value;                 // Offline unsupported syntax is preserved verbatim.
    }
    if (displayRadix != 16)
    {
        return QString();
    }
    parts = address.value.split(QChar(u'.'), Qt::KeepEmptyParts);
    if (!readUnsigned(parts[0], 10, &major) || parts.size() > 2)
    {
        return QString();
    }
    return QString::number(major, 16).toUpper() +
           (parts.size() == 2 ? QChar(u'.') + parts[1] : QString());
}

/******************************************************************************
 * Function Name   : plcAddressText
 * Description     : Render family and value while keeping connection selection explicit.
 * Input Parameters: address, displayRadix
 * Return Value    : Address text, or an empty string for unsupported conversion
 * Date            : 2026.11.09
 ******************************************************************************/
QString plcAddressText(const PlcAddress &address, int displayRadix)
{
    QString value;

    value = plcAddressValueText(address, displayRadix);
    return value.isEmpty() ? QString() : address.family + value;
}

/******************************************************************************
 * Function Name   : plcQualifiedAddressText
 * Description     : Format a stable external connection qualifier or a bare internal address.
 * Input Parameters: address, displayRadix
 * Return Value    : Qualified text, or empty for inconsistent address scope or connection IDs
 * Date            : 2026.11.09
 ******************************************************************************/
QString plcQualifiedAddressText(const PlcAddress &address, int displayRadix)
{
    QString text;

    text = plcAddressText(address, displayRadix);
    if (text.isEmpty())
    {
        return QString();
    }
    if (address.space == PlcAddressSpace::Internal)
    {
        return address.connectionId.isEmpty() ? text : QString();
    }
    if (address.connectionId.isEmpty() || address.connectionId.contains(QChar(u':')) ||
        address.connectionId.contains(QChar(u'@')))
    {
        return QString();
    }
    return QChar(u'@') + address.connectionId + QChar(u':') + text;
}

/******************************************************************************
 * Function Name   : encodePlcNativeAddress
 * Description     : Produce known fields only, requiring independent external node and route evidence.
 * Input Parameters: address, context, catalog, binding, record, error
 * Return Value    : True when the assigned native fields can be emitted safely
 * Date            : 2026.11.09
 ******************************************************************************/
bool encodePlcNativeAddress(const PlcAddress &address, const PlcAddressContext &context,
                            const PlcCatalog &catalog, const PlcNativeBinding &binding,
                            PlcNativeAddressRecord *record, QString *error) //20261109-0 add
{
    const int assigned[8] = {0, 1, 2, 5, 8, 9, 10, 11};
    PlcNativeAddressRecord encoded;
    PlcNativeParsedAddress nativeAddress; //20261109-0 add
    PlcAddress canonical;
    const PlcConnection *connection;
    const PlcProfile *profile;
    QString normalized;
    u32 numeric;
    u8 type;
    u8 channel;
    u8 node;
    int familyIndex;
    int index;

    if (record == nullptr || !validatePlcAddress(address, context, catalog, error) ||
        !parsePlcAddress(address.family, address.value, address.connectionId,
                          context, catalog, &canonical, error))
    {
        return record == nullptr ? addressError(error, QStringLiteral("PLC 原生地址输出为空。")) : false;
    }
    familyIndex = modbusFamilyIndex(canonical.family);
    channel = 0;
    node = 0;                                // The recovered local LB0 constructor initializes node to zero.
    type = canonical.family == QStringLiteral("LW") ? 0x57 : 0x42;
    if (canonical.space == PlcAddressSpace::External)
    {
        connection = plcConnection(context, canonical.connectionId);
        profile = connection == nullptr ? nullptr : catalog.profile(connection->profileId);
        if (connection == nullptr || profile == nullptr ||
            !plcProfileHasObservedDriver(*profile, connection->channel))
        {
            return addressError(error, QStringLiteral("PLC 配置、通道或目标驱动文件未知，无法生成设备地址。"));
        }
        if (!binding.commonV51CodecVerified || !binding.routeVerified || binding.evidence.trimmed().isEmpty() ||
            binding.connectionId != canonical.connectionId || binding.channel != connection->channel ||
            (binding.channelByte != 1 && binding.channelByte != 2))
        {
            return addressError(error, QStringLiteral("外部地址需要已验证的 Common V5.1 编码和独立通道路由绑定。"));
        }
        if (!connection->nodeKnown)
        {
            return addressError(error, QStringLiteral("PLC 节点未知；站号不能替代原生地址节点。"));
        }
        channel = binding.channelByte;
        node = connection->node;
        if (!parseNativePlcAddress(*profile, canonical.family, canonical.value, context.transferCount,
                                   &nativeAddress, error)) { return false; } //20261109-0 change
        type = nativeAddress.typeByte;
        numeric = nativeAddress.numeric;
    }
    if (canonical.space == PlcAddressSpace::Internal &&
        !numericValue(canonical.value, familyIndex, true,
                       10, context.transferCount, context.localAddressCount,
                       &numeric, &normalized, error))
    {
        return false;
    }
    encoded.knownBytes = QByteArray(36, char(0));
    encoded.knownMask = QByteArray(36, char(0));
    for (index = 0; index < 8; ++index)
    {
        encoded.knownMask[assigned[index]] = char(0xff);
    }
    encoded.knownBytes[0] = char(channel);
    encoded.knownBytes[1] = char(type);
    encoded.knownBytes[2] = char(node);
    encoded.knownBytes[5] = char(0);            // Generic address field; control operations handle toggling separately.
    for (index = 0; index < 4; ++index)
    {
        encoded.knownBytes[8 + index] = char((numeric >> (index * 8)) & 0xffU);
    }
    *record = encoded;
    clearAddressError(error);
    return true;
}

/******************************************************************************
 * Function Name   : PlcNativeAddressRecord::overlay
 * Description     : Preserve template bytes where the vendor writer's values are unknown.
 * Input Parameters: verifiedTemplate, record, error
 * Return Value    : True for a complete 36-byte template and a valid byte mask
 * Date            : 2026.11.09
 ******************************************************************************/
bool PlcNativeAddressRecord::overlay(const QByteArray &verifiedTemplate, QByteArray *record,
                                     QString *error) const
{
    QByteArray encoded;
    u8 mask;
    int index;

    if (record == nullptr || verifiedTemplate.size() != 36 || knownBytes.size() != 36 || knownMask.size() != 36)
    {
        return addressError(error, QStringLiteral("物化原生地址必须提供完整的已验证 36 字节模板及 36 字节掩码。"));
    }
    encoded = verifiedTemplate;
    for (index = 0; index < 36; ++index)
    {
        mask = u8(knownMask[index]);
        if (mask != 0 && mask != 0xff)
        {
            return addressError(error, QStringLiteral("原生地址已知掩码必须逐字节为 0 或 0xff。"));
        }
        if (mask == 0xff)
        {
            encoded[index] = knownBytes[index];
        }
    }
    *record = encoded;
    clearAddressError(error);
    return true;
}
