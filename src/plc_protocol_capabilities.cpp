#include "plc_protocol_capabilities.h"
#include "plc_address.h"
#include "plc_native_catalog.h"
#include <QCryptographicHash>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>

/******************************************************************************
 * Function Name   : plcProtocolCapabilities
 * Description     : Report grammar coverage and conservative device gates independently from file presence and raw PC RTU tools.
 * Input Parameters: model
 * Return Value    : Bounded report; no vendor library or physical port is opened.
 * Date            : 2026.11.14
 ******************************************************************************/
QJsonObject plcProtocolCapabilities(const QString &model) //20261114-8 add
{
    PlcCatalog catalog;
    QString error;
    QFile file(QStringLiteral(":/plc/stage9-file-audit.json"));
    QFile catalogFile(QStringLiteral(":/plc/plc-catalog.json"));
    QByteArray catalogBytes;
    QJsonObject result;
    QJsonObject audit;
    QJsonObject source;
    QJsonObject row;
    QJsonObject familyRow;
    QJsonArray rows;
    QJsonArray families;
    QMap<QString, QJsonObject> observations;
    PlcNativeFamilyRule rule;
    bool defaultCodec;
    bool eligible;
    bool familySupported;
    int supportedCount;

    result.insert(QStringLiteral("format"), QStringLiteral("KinSealLite-plc-capabilities-v1"));
    result.insert(QStringLiteral("model"), model);
    result.insert(QStringLiteral("hardwareIo"), false);
    result.insert(QStringLiteral("hmiExecutionConfirmed"), false);
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), &error) || !file.open(QIODevice::ReadOnly) ||
        file.size() > 2 * 1024 * 1024 || !catalogFile.open(QIODevice::ReadOnly))
    {
        result.insert(QStringLiteral("ok"), false); result.insert(QStringLiteral("error"), error); return result;
    }
    catalogBytes = catalogFile.readAll();
    audit = QJsonDocument::fromJson(file.readAll()).object();
    if (audit.value(QStringLiteral("catalogSha256")).toString() !=
        QString::fromLatin1(QCryptographicHash::hash(catalogBytes, QCryptographicHash::Sha256).toHex()))
    {
        result.insert(QStringLiteral("ok"), false); result.insert(QStringLiteral("error"), QStringLiteral("协议目录与文件身份快照不匹配。")); return result;
    }
    for (const QJsonValue &value : audit.value(QStringLiteral("rows")).toArray())
    {
        source = value.toObject(); observations.insert(source.value(QStringLiteral("profileId")).toString(), source);
    }
    for (const PlcProfile &profile : catalog.profiles())
    {
        source = observations.value(profile.id);
        defaultCodec = plcProfileHasModbusCodec(profile);
        supportedCount = 0; families = QJsonArray();
        for (const PlcFamily &family : profile.families)
        {
            familySupported = defaultCodec || plcNativeFamilyRule(profile, family.name, &rule, nullptr);
            if (familySupported) { ++supportedCount; }
            familyRow = QJsonObject{{QStringLiteral("name"), family.name}, {QStringLiteral("addressGrammarImplemented"), familySupported},
                {QStringLiteral("rawRange"), family.rawRange}, {QStringLiteral("rawRadix"), family.rawRadix}};
            families.append(familyRow);
        }
        eligible = model == QStringLiteral("AMT070W124-L800X480R") &&
            profile.id == QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini") && defaultCodec &&
            source.value(QStringLiteral("sourceMatches")).toBool() && source.value(QStringLiteral("com1")).toObject().value(QStringLiteral("present")).toBool();
        row = QJsonObject{{QStringLiteral("profileId"), profile.id}, {QStringLiteral("name"), profile.name},
            {QStringLiteral("driverName"), profile.driverName}, {QStringLiteral("catalogVisible"), true},
            {QStringLiteral("connectionMetadataStorable"), true}, {QStringLiteral("families"), families},
            {QStringLiteral("grammarFamilyCount"), supportedCount}, {QStringLiteral("familyCount"), profile.families.size()},
            {QStringLiteral("deviceCompileEligible"), eligible}, {QStringLiteral("pcProfileMapped"), false},
            {QStringLiteral("physicalPlcVerified"), false}, {QStringLiteral("hmiPlcVerified"), false},
            {QStringLiteral("nativeDllAbiImplemented"), false}, {QStringLiteral("filesObserved"), source},
            {QStringLiteral("serialDefaultsRaw"), profile.serialDefaults},
            {QStringLiteral("nextEvidence"), eligible ? QStringLiteral("单页COM1站1/node1已核C；后续专用从站验证HMI读写/异常/字序。") :
                QStringLiteral("核原厂地址/模块ABI/型号模板；目录和文件存在不代表通信支持。")}};
        if (eligible)
        {
            row.insert(QStringLiteral("deviceScope"), QJsonObject{{QStringLiteral("channel"), QStringLiteral("COM1")},
                {QStringLiteral("stationText"), QStringLiteral("1")}, {QStringLiteral("nodeKnown"), true},
                {QStringLiteral("node"), 1}, {QStringLiteral("route"), 1}, {QStringLiteral("singleConnectionSinglePage"), true}});
        }
        rows.append(row);
    }
    result.insert(QStringLiteral("ok"), true); result.insert(QStringLiteral("profiles"), rows);
    result.insert(QStringLiteral("profileCount"), rows.size()); result.insert(QStringLiteral("catalogSha256"), audit.value(QStringLiteral("catalogSha256")));
    result.insert(QStringLiteral("fileAuditModel"), audit.value(QStringLiteral("model")));
    result.insert(QStringLiteral("pcRawRtu"), QJsonObject{{QStringLiteral("functions"), QJsonArray{1, 2, 3, 4, 5, 6, 15, 16}},
        {QStringLiteral("explicitZeroBasedPduAddress"), true}, {QStringLiteral("serialBackendImplemented"), true},
        {QStringLiteral("automaticProjectMapping"), false}, {QStringLiteral("physicalEndpointVerified"), false},
        {QStringLiteral("automaticRetries"), 0}});
    return result;
}
