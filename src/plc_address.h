#ifndef PLC_ADDRESS_H
#define PLC_ADDRESS_H

#include <QByteArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QVector>

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

//20261109-0 add: Catalog visibility, offline address storage and native codec support are independent.
enum class PlcAddressRole { Any, Bit, Word, Dword };
enum class PlcAddressSpace { Internal, External };
enum class PlcChannel { Unspecified, Com1, Com2 };

struct PlcFamily
{
    QString name;
    QStringList roles;
    QString rawRadix;
    QString rawRange;
    QString grammarCandidate;
    int majorRadix = 0;
    int minorRadix = 0;
    QJsonObject metadata;
};

struct PlcProfile
{
    QString id;
    QString name;
    QString manufacturer;
    QString route;
    QString driverName;
    QString sourceSha256;
    QJsonObject serialDefaults;
    QJsonObject rawFields;
    QStringList warnings;
    QVector<PlcFamily> families;
    QJsonObject metadata;
};

class PlcCatalog
{
public:
    /******************************************************************************
     * Function Name   : load
     * Description     : Load shipped observed metadata without reading a vendor installation.
     * Input Parameters: path, error
     * Return Value    : True after the entire catalog has been validated and replaced.
     * Date            : 2026.11.09
     ******************************************************************************/
    bool load(const QString &path = QStringLiteral(":/plc/plc-catalog.json"),
              QString *error = nullptr); //20261109-0 add

    const QVector<PlcProfile> &profiles() const;
    const PlcProfile *profile(const QString &id) const;
    QVector<PlcProfile> profilesForModel(const QString &modelName) const;
    QVector<PlcFamily> families(const QString &profileId,
                                PlcAddressRole role = PlcAddressRole::Any) const;
    QJsonObject metadata() const;

private:
    QVector<PlcProfile> m_profiles;
    QJsonObject m_metadata;
};

struct PlcConnection
{
    QString id;
    QString profileId;
    PlcChannel channel = PlcChannel::Unspecified;
    QString station;                         // Raw project station text; no observed station bounds.
    bool nodeKnown = false;
    u8 node = 0;                             // A node is independent of station and may remain unknown.

    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, PlcConnection *connection,
                         QString *error = nullptr);
};

struct PlcAddress
{
    PlcAddressSpace space = PlcAddressSpace::Internal;
    QString connectionId;
    QString family = QStringLiteral("LB");
    QString value = QStringLiteral("0");     // Canonical native syntax; protocol families may use mixed radices or banks.

    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, PlcAddress *address,
                         QString *error = nullptr);
};

struct PlcAddressContext
{
    QVector<PlcConnection> connections;
    u32 localAddressCount = 4097;              // Inclusive editor range 0..4096; model addrRange semantics are unverified.
    u32 transferCount = 1;                     // Native numeric request +4; not inferred from DWORD role.
    PlcAddressRole role = PlcAddressRole::Any;
};

struct PlcNativeBinding
{
    QString connectionId;
    PlcChannel channel = PlcChannel::Unspecified;
    bool commonV51CodecVerified = false;        // Must not be used for the unverified NewCommon variant.
    bool routeVerified = false;
    u8 channelByte = 0;                        // Explicit observed external route value 1 or 2.
    QString evidence;
};

struct PlcNativeAddressRecord
{
    QByteArray knownBytes;
    QByteArray knownMask;                      // 0xff marks assigned bytes; other bytes remain unknown.

    bool overlay(const QByteArray &verifiedTemplate, QByteArray *record,
                 QString *error = nullptr) const;
};

bool plcFamilyHasRole(const PlcFamily &family, PlcAddressRole role);
bool plcProfileHasObservedDriver(const PlcProfile &profile, PlcChannel channel);
bool plcProfileHasModbusCodec(const PlcProfile &profile);
const PlcConnection *plcConnection(const PlcAddressContext &context, const QString &id);

/******************************************************************************
 * Function Name   : parsePlcAddress
 * Description     : Parse LB/LW or the observed default Modbus RTU decimal families.
 * Input Parameters: family, value, connectionId, context, catalog, address, error, displayRadix
 * Return Value    : True with a canonical address; outputs are unchanged on failure.
 * Date            : 2026.11.09
 ******************************************************************************/
bool parsePlcAddress(const QString &family, const QString &value, const QString &connectionId,
                     const PlcAddressContext &context, const PlcCatalog &catalog,
                     PlcAddress *address, QString *error = nullptr,
                     int displayRadix = 10); //20261109-0 add
bool parsePlcAddressText(const QString &text, const QString &connectionId,
                         const PlcAddressContext &context, const PlcCatalog &catalog,
                         PlcAddress *address, QString *error = nullptr,
                         int displayRadix = 10);
bool parseQualifiedPlcAddressText(const QString &text, const PlcAddressContext &context,
                                  const PlcCatalog &catalog, PlcAddress *address,
                                  QString *error = nullptr, int displayRadix = 10);
bool validatePlcAddress(const PlcAddress &address, const PlcAddressContext &context,
                        const PlcCatalog &catalog, QString *error = nullptr);
QString plcAddressValueText(const PlcAddress &address, int displayRadix = 10);
QString plcAddressText(const PlcAddress &address, int displayRadix = 10);
QString plcQualifiedAddressText(const PlcAddress &address, int displayRadix = 10);

/******************************************************************************
 * Function Name   : encodePlcNativeAddress
 * Description     : Encode assigned 36-byte fields while preserving all unknown bytes in a mask.
 * Input Parameters: address, context, catalog, binding, record, error
 * Return Value    : True for a verified local or default Modbus RTU native address.
 * Date            : 2026.11.09
 ******************************************************************************/
bool encodePlcNativeAddress(const PlcAddress &address, const PlcAddressContext &context,
                            const PlcCatalog &catalog, const PlcNativeBinding &binding,
                            PlcNativeAddressRecord *record, QString *error = nullptr); //20261109-0 add

#endif
