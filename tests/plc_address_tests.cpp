#include "plc_address.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>
#include <QTemporaryFile>
#include <QtEndian> //20261109-0 add
#include <QtTest>

//20261109-0 add: Static-format tests do not open serial ports or execute vendor code.
class PlcAddressTests : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void completeCatalog();
    void catalogLoadIsTransactional();
    void structuredJson();
    void internalAddresses();
    void qualifiedAddresses();
    void modbusFamilies_data();
    void modbusFamilies();
    void numericBoundaries_data();
    void numericBoundaries();
    void rejectsUnknownNativeContext();
    void stationIsNotNode();
    void maskAndTemplate();
    void recoveredProtocolIntegration(); //20261109-0 add

private:
    PlcCatalog m_catalog;
    PlcAddressContext m_context;
    PlcNativeBinding m_binding;
};

/******************************************************************************
 * Function Name   : PlcAddressTests::initTestCase
 * Description     : Load the shipped catalog and explicitly bind a synthetic verified route.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void PlcAddressTests::initTestCase()
{
    PlcConnection connection;
    QString error;

    QVERIFY2(m_catalog.load(QStringLiteral(":/plc/plc-catalog.json"), &error), qPrintable(error));
    connection.id = QStringLiteral("com1");
    connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
    connection.channel = PlcChannel::Com1;
    connection.station = QStringLiteral("1");
    connection.nodeKnown = true;
    connection.node = 7;
    m_context.connections.append(connection);
    m_binding.connectionId = connection.id;
    m_binding.channel = connection.channel;
    m_binding.commonV51CodecVerified = true;
    m_binding.routeVerified = true;
    m_binding.channelByte = 2;                 // Deliberately no automatic COM1 -> byte 1 assumption.
    m_binding.evidence = QStringLiteral("Synthetic test binding; no hardware verification claimed");
}

/******************************************************************************
 * Function Name   : PlcAddressTests::completeCatalog
 * Description     : Check complete observed metadata, source defects and role filtering.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void PlcAddressTests::completeCatalog()
{
    const PlcProfile *profile;
    QVector<PlcProfile> current;
    QSet<QString> names;
    int index;
    int familyCount;

    QCOMPARE(m_catalog.profiles().size(), 139);
    QCOMPARE(m_catalog.metadata().value(QStringLiteral("models")).toArray().size(), 80);
    current = m_catalog.profilesForModel(QStringLiteral("AMZ028W06"));
    QCOMPARE(current.size(), 70);
    QVERIFY(m_catalog.profilesForModel(QStringLiteral("unknown-model")).isEmpty());
    familyCount = 0;
    for (index = 0; index < m_catalog.profiles().size(); ++index)
    {
        profile = &m_catalog.profiles()[index];
        names.insert(profile->name);
        QVERIFY(profile->metadata.value(QStringLiteral("sourceHashMatchesOriginal")).toBool());
        QVERIFY(!profile->rawFields.isEmpty());
        if (profile->route == QStringLiteral("serial_amz"))
        {
            familyCount += profile->families.size();
        }
    }
    QCOMPARE(names.size(), 78);
    QCOMPARE(familyCount, 883);
    profile = m_catalog.profile(QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini"));
    QVERIFY(profile != nullptr);
    QCOMPARE(profile->rawFields.value(QStringLiteral("Parity")).toString(), QStringLiteral("1"));
    QCOMPARE(profile->metadata.value(QStringLiteral("rawLists")).toObject().value(QStringLiteral("ranges")).toArray().size(), 13);
    QVERIFY(!profile->warnings.isEmpty());
    QCOMPARE(m_catalog.families(profile->id, PlcAddressRole::Bit).size(), 6);
    QCOMPARE(m_catalog.families(profile->id, PlcAddressRole::Word).size(), 6);
    QCOMPARE(m_catalog.families(profile->id, PlcAddressRole::Dword).size(), 2);
    QVERIFY(plcProfileHasObservedDriver(*profile, PlcChannel::Com1));
    QVERIFY(!plcProfileHasObservedDriver(*profile, PlcChannel::Unspecified));
    profile = m_catalog.profile(QStringLiteral("Link/serial_amz/POWTRAN/Powtran_ASCII.ini"));
    QVERIFY(profile != nullptr);
    QCOMPARE(profile->families.size(), 22);
    QVERIFY(profile->rawFields.value(QStringLiteral("ALL_ADDR_REG")).toString().isEmpty());
    QVERIFY(!profile->warnings.isEmpty());
    profile = m_catalog.profile(QStringLiteral("Link/serial/HOLLYSYS/Hollysys_LE_Series.ini"));
    QVERIFY(profile != nullptr);
    QCOMPARE(profile->metadata.value(QStringLiteral("rawLists")).toObject().value(QStringLiteral("radices")).toArray().size(), 7);
    QCOMPARE(profile->families.size(), 6);
}

/******************************************************************************
 * Function Name   : PlcAddressTests::catalogLoadIsTransactional
 * Description     : Failed reloads must not erase the working catalog.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void PlcAddressTests::catalogLoadIsTransactional()
{
    QTemporaryFile file;
    QString error;

    QVERIFY(file.open());
    file.write(QByteArrayLiteral("{\"version\":1,\"profileCount\":2,\"profiles\":[]}"));
    file.flush();
    QVERIFY(!m_catalog.load(file.fileName(), &error));
    QVERIFY(!error.isEmpty());
    QCOMPARE(m_catalog.profiles().size(), 139);
}

/******************************************************************************
 * Function Name   : PlcAddressTests::structuredJson
 * Description     : Round-trip offline addresses and keep unknown nodes independent of station.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void PlcAddressTests::structuredJson()
{
    PlcConnection connection;
    PlcConnection decoded;
    PlcAddress address;
    PlcAddress decodedAddress;
    QJsonObject json;
    QJsonObject before;
    QString error;

    connection = m_context.connections[0];
    connection.profileId = QStringLiteral("future-driver");
    connection.station = QStringLiteral("station-raw-unverified");
    connection.nodeKnown = false;
    json = connection.toJson();
    QVERIFY(json.value(QStringLiteral("node")).isNull());
    QVERIFY(PlcConnection::fromJson(json, &decoded, &error));
    QCOMPARE(decoded.toJson(), json);
    QVERIFY(!decoded.nodeKnown);
    before = decoded.toJson();
    json.insert(QStringLiteral("node"), 1.5);
    QVERIFY(!PlcConnection::fromJson(json, &decoded, &error));
    QCOMPARE(decoded.toJson(), before);
    json.insert(QStringLiteral("node"), 256);
    QVERIFY(!PlcConnection::fromJson(json, &decoded, &error));
    json.insert(QStringLiteral("node"), QStringLiteral("1"));
    QVERIFY(!PlcConnection::fromJson(json, &decoded, &error));
    address.space = PlcAddressSpace::External;
    address.connectionId = QStringLiteral("com1");
    address.family = QStringLiteral("future-bank");
    address.value = QStringLiteral("DB9:12.F");
    QVERIFY(PlcAddress::fromJson(address.toJson(), &decodedAddress, &error));
    QCOMPARE(decodedAddress.toJson(), address.toJson());
    QCOMPARE(plcAddressText(decodedAddress), QStringLiteral("future-bankDB9:12.F"));
    json = address.toJson();
    json.insert(QStringLiteral("space"), QStringLiteral("internal"));
    QVERIFY(!PlcAddress::fromJson(json, &decodedAddress, &error));
    QCOMPARE(decodedAddress.toJson(), address.toJson());
}

/******************************************************************************
 * Function Name   : PlcAddressTests::internalAddresses
 * Description     : Preserve LB/LW, inclusive legacy bounds and real display-radix conversion.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void PlcAddressTests::internalAddresses()
{
    PlcAddress address;
    PlcAddressContext context;
    PlcNativeAddressRecord record;
    QString error;
    QJsonObject before;

    context.role = PlcAddressRole::Bit;
    QVERIFY(parsePlcAddressText(QStringLiteral("lb4096"), QString(), context, m_catalog, &address, &error));
    QCOMPARE(plcAddressText(address), QStringLiteral("LB4096"));
    QVERIFY(encodePlcNativeAddress(address, context, m_catalog, PlcNativeBinding(), &record, &error));
    QCOMPARE(u8(record.knownBytes[1]), u8(0x42));
    QCOMPARE(u8(record.knownBytes[0]), u8(0));
    before = address.toJson();
    QVERIFY(!parsePlcAddressText(QStringLiteral("LB4097"), QString(), context, m_catalog, &address, &error));
    QCOMPARE(address.toJson(), before);
    QVERIFY(!parsePlcAddressText(QStringLiteral("LW1"), QString(), context, m_catalog, &address, &error));
    QVERIFY(parsePlcAddress(QStringLiteral("LB"), QStringLiteral("FF"), QString(), context,
                             m_catalog, &address, &error, 16));
    QCOMPARE(address.value, QStringLiteral("255"));
    QCOMPARE(plcAddressText(address, 16), QStringLiteral("LBFF"));
    context.role = PlcAddressRole::Word;
    QVERIFY(parsePlcAddressText(QStringLiteral("LW0"), QString(), context, m_catalog, &address, &error));
    QVERIFY(encodePlcNativeAddress(address, context, m_catalog, PlcNativeBinding(), &record, &error));
    QCOMPARE(u8(record.knownBytes[1]), u8(0x57));
    context.localAddressCount = 0;
    QVERIFY(!validatePlcAddress(address, context, m_catalog, &error));
}

/******************************************************************************
 * Function Name   : PlcAddressTests::qualifiedAddresses
 * Description     : Keep external connection identity explicit through text serialization.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void PlcAddressTests::qualifiedAddresses()
{
    PlcAddress address;
    QString error;

    QVERIFY(parseQualifiedPlcAddressText(QStringLiteral("@com1:4x_bit12.03"), m_context,
                                          m_catalog, &address, &error));
    QCOMPARE(plcQualifiedAddressText(address), QStringLiteral("@com1:4x_bit12.03"));
    QVERIFY(!parseQualifiedPlcAddressText(QStringLiteral("4x12"), m_context, m_catalog, &address, &error));
    QVERIFY(!parseQualifiedPlcAddressText(QStringLiteral("@:4x12"), m_context, m_catalog, &address, &error));
    QVERIFY(!parseQualifiedPlcAddressText(QStringLiteral("@missing:4x12"), m_context, m_catalog, &address, &error));
    QVERIFY(parseQualifiedPlcAddressText(QStringLiteral("LW4096"), m_context, m_catalog, &address, &error));
    QCOMPARE(plcQualifiedAddressText(address), QStringLiteral("LW4096"));
}

/******************************************************************************
 * Function Name   : PlcAddressTests::modbusFamilies_data
 * Description     : Define independent observed type-byte expectations for all twelve families.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void PlcAddressTests::modbusFamilies_data()
{
    QTest::addColumn<QString>("family");
    QTest::addColumn<QString>("value");
    QTest::addColumn<int>("type");
    QTest::addColumn<u32>("numeric");
    QTest::newRow("0x") << QStringLiteral("0x") << QStringLiteral("123") << 0x4d << u32(123);
    QTest::newRow("1x") << QStringLiteral("1x") << QStringLiteral("123") << 0x58 << u32(123);
    QTest::newRow("3x_bit") << QStringLiteral("3x_bit") << QStringLiteral("12.03") << 0x48 << u32(195);
    QTest::newRow("4x_bit") << QStringLiteral("4x_bit") << QStringLiteral("12.03") << 0x4b << u32(195);
    QTest::newRow("6x_bit") << QStringLiteral("6x_bit") << QStringLiteral("12.03") << 0x56 << u32(195);
    QTest::newRow("10x_bit") << QStringLiteral("10x_bit") << QStringLiteral("12.03") << 0x4c << u32(195);
    QTest::newRow("3x") << QStringLiteral("3x") << QStringLiteral("123") << 0x52 << u32(123);
    QTest::newRow("4x") << QStringLiteral("4x") << QStringLiteral("123") << 0x44 << u32(123);
    QTest::newRow("3x_dw") << QStringLiteral("3x_dw") << QStringLiteral("123") << 0x74 << u32(123);
    QTest::newRow("4x_dw") << QStringLiteral("4x_dw") << QStringLiteral("123") << 0x63 << u32(123);
    QTest::newRow("6x") << QStringLiteral("6x") << QStringLiteral("123") << 0x65 << u32(123);
    QTest::newRow("10x") << QStringLiteral("10x") << QStringLiteral("123") << 0x66 << u32(123);
}

/******************************************************************************
 * Function Name   : PlcAddressTests::modbusFamilies
 * Description     : Verify type bytes, LE numeric values, explicit routes and exact known masks.
 * Input Parameters: Qt test row
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void PlcAddressTests::modbusFamilies()
{
    QFETCH(QString, family);
    QFETCH(QString, value);
    QFETCH(int, type);
    QFETCH(u32, numeric);
    PlcAddress address;
    PlcNativeAddressRecord record;
    QString error;
    u32 observed;
    int index;

    QVERIFY2(parsePlcAddressText(family + value, QStringLiteral("com1"), m_context,
                                 m_catalog, &address, &error), qPrintable(error));
    QVERIFY2(encodePlcNativeAddress(address, m_context, m_catalog, m_binding, &record, &error), qPrintable(error));
    QCOMPARE(u8(record.knownBytes[0]), u8(2));
    QCOMPARE(u8(record.knownBytes[1]), u8(type));
    QCOMPARE(u8(record.knownBytes[2]), u8(7));
    QCOMPARE(u8(record.knownBytes[5]), u8(0));
    observed = 0;
    for (index = 0; index < 4; ++index)
    {
        observed |= u32(u8(record.knownBytes[8 + index])) << (index * 8);
    }
    QCOMPARE(observed, numeric);
    QCOMPARE(record.knownMask.toHex(), QByteArrayLiteral("ffffff0000ff0000ffffffff000000000000000000000000000000000000000000000000"));
}

/******************************************************************************
 * Function Name   : PlcAddressTests::numericBoundaries_data
 * Description     : Cover recovered asymmetric boundaries and strict canonical syntax rejection.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void PlcAddressTests::numericBoundaries_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<u32>("count");
    QTest::addColumn<bool>("accepted");
    QTest::newRow("coil-max") << QStringLiteral("0x65535") << u32(1) << true;
    QTest::newRow("coil-over") << QStringLiteral("0x65536") << u32(1) << false;
    QTest::newRow("input-extra") << QStringLiteral("1x65536") << u32(1) << true;
    QTest::newRow("input-over") << QStringLiteral("1x65537") << u32(1) << false;
    QTest::newRow("input-count") << QStringLiteral("1x65536") << u32(2) << false;
    QTest::newRow("word-max") << QStringLiteral("4x65535") << u32(1) << true;
    QTest::newRow("word-count") << QStringLiteral("4x65535") << u32(2) << false;
    QTest::newRow("word-count-ok") << QStringLiteral("4x65534") << u32(2) << true;
    QTest::newRow("bit-max") << QStringLiteral("4x_bit65535.15") << u32(1) << true;
    QTest::newRow("bit-count") << QStringLiteral("4x_bit65535.15") << u32(2) << false;
    QTest::newRow("bit-over") << QStringLiteral("4x_bit65536.00") << u32(1) << false;
    QTest::newRow("minor-over") << QStringLiteral("3x_bit1.16") << u32(1) << false;
    QTest::newRow("minor-width") << QStringLiteral("3x_bit1.015") << u32(1) << false;
    QTest::newRow("minor-missing") << QStringLiteral("3x_bit1.") << u32(1) << false;
    QTest::newRow("major-missing") << QStringLiteral("3x_bit.1") << u32(1) << false;
    QTest::newRow("extra-dot") << QStringLiteral("3x_bit1.2.3") << u32(1) << false;
    QTest::newRow("minus") << QStringLiteral("4x-1") << u32(1) << false;
    QTest::newRow("fraction") << QStringLiteral("4x1.5") << u32(1) << false;
    QTest::newRow("decimal-only") << QStringLiteral("4xFFFF") << u32(1) << false;
    QTest::newRow("overflow") << QStringLiteral("4x42949672960") << u32(1) << false;
    QTest::newRow("count-zero") << QStringLiteral("4x0") << u32(0) << false;
    QTest::newRow("dword-no-stride") << QStringLiteral("3x_dw65535") << u32(1) << true;
    QTest::newRow("dword-count-explicit") << QStringLiteral("3x_dw65535") << u32(2) << false;
}

/******************************************************************************
 * Function Name   : PlcAddressTests::numericBoundaries
 * Description     : Verify accepted/rejected numeric cases and transactional failure behavior.
 * Input Parameters: Qt test row
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void PlcAddressTests::numericBoundaries()
{
    QFETCH(QString, text);
    QFETCH(u32, count);
    QFETCH(bool, accepted);
    PlcAddressContext context;
    PlcAddress address;
    QJsonObject before;
    QString error;
    bool result;

    context = m_context;
    context.transferCount = count;
    before = address.toJson();
    result = parsePlcAddressText(text, QStringLiteral("com1"), context, m_catalog, &address, &error);
    QCOMPARE(result, accepted);
    if (!accepted)
    {
        QVERIFY(!error.isEmpty());
        QCOMPARE(address.toJson(), before);
    }
}

/******************************************************************************
 * Function Name   : PlcAddressTests::rejectsUnknownNativeContext
 * Description     : Unknown profiles, ambiguous connections, routes and drivers must fail closed.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void PlcAddressTests::rejectsUnknownNativeContext()
{
    PlcAddressContext context;
    PlcNativeBinding binding;
    PlcNativeAddressRecord record;
    PlcAddress address;
    QString error;

    QVERIFY(parsePlcAddressText(QStringLiteral("4x_bit12.03"), QStringLiteral("com1"), m_context,
                                 m_catalog, &address, &error));
    record.knownBytes = QByteArrayLiteral("sentinel");
    binding = m_binding;
    binding.routeVerified = false;
    QVERIFY(!encodePlcNativeAddress(address, m_context, m_catalog, binding, &record, &error));
    QCOMPARE(record.knownBytes, QByteArrayLiteral("sentinel"));
    binding = m_binding;
    binding.commonV51CodecVerified = false;
    QVERIFY(!encodePlcNativeAddress(address, m_context, m_catalog, binding, &record, &error));
    binding = m_binding;
    binding.channelByte = 0;
    QVERIFY(!encodePlcNativeAddress(address, m_context, m_catalog, binding, &record, &error));
    binding = m_binding;
    binding.channel = PlcChannel::Com2;
    QVERIFY(!encodePlcNativeAddress(address, m_context, m_catalog, binding, &record, &error));
    context = m_context;
    context.connections[0].channel = PlcChannel::Unspecified;
    QVERIFY(!encodePlcNativeAddress(address, context, m_catalog, m_binding, &record, &error));
    context = m_context;
    context.connections[0].profileId = QStringLiteral("Link/serial/MODBUS/Modbus_RTU.ini");
    QVERIFY(parsePlcAddressText(QStringLiteral("4x1"), QStringLiteral("com1"), context, m_catalog, &address, &error));
    QVERIFY(!encodePlcNativeAddress(address, context, m_catalog, m_binding, &record, &error));
    context = m_context;
    context.connections[0].profileId = QStringLiteral("Link/serial_amz/AIBUS/AIBUS.ini");
    QVERIFY(parsePlcAddressText(QStringLiteral("Parameter1"), QStringLiteral("com1"), context, m_catalog, &address, &error)); //20261109-0 change
    context.connections[0].profileId = QStringLiteral("Link/serial_amz/Graphic/Graphic_KS.ini");
    QVERIFY(!parsePlcAddressText(QStringLiteral("GA1"), QStringLiteral("com1"), context, m_catalog, &address, &error));
    context.connections[0].profileId = QStringLiteral("unknown");
    QVERIFY(!parsePlcAddressText(QStringLiteral("4x1"), QStringLiteral("com1"), context, m_catalog, &address, &error));
    context = m_context;
    context.connections.append(context.connections[0]);
    QVERIFY(!parsePlcAddressText(QStringLiteral("4x1"), QStringLiteral("com1"), context, m_catalog, &address, &error));
}

/******************************************************************************
 * Function Name   : PlcAddressTests::stationIsNotNode
 * Description     : Station edits cannot manufacture or overwrite the native source node byte.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void PlcAddressTests::stationIsNotNode()
{
    PlcAddressContext context;
    PlcNativeAddressRecord first;
    PlcNativeAddressRecord second;
    PlcAddress address;
    QString error;

    QVERIFY(parsePlcAddressText(QStringLiteral("4x_bit12.03"), QStringLiteral("com1"), m_context,
                                 m_catalog, &address, &error));
    context = m_context;
    context.connections[0].nodeKnown = false;
    context.connections[0].station = QStringLiteral("1");
    QVERIFY(!encodePlcNativeAddress(address, context, m_catalog, m_binding, &first, &error));
    QVERIFY(error.contains(QStringLiteral("站号")));
    context.connections[0].nodeKnown = true;
    QVERIFY(encodePlcNativeAddress(address, context, m_catalog, m_binding, &first, &error));
    context.connections[0].station = QStringLiteral("250");
    QVERIFY(encodePlcNativeAddress(address, context, m_catalog, m_binding, &second, &error));
    QCOMPARE(first.knownBytes, second.knownBytes);
    QCOMPARE(u8(second.knownBytes[2]), u8(7));
    QCOMPARE(plcAddressText(address, 16), QStringLiteral("4x_bitC.03"));
    QVERIFY(parsePlcAddress(QStringLiteral("4x_bit"), QStringLiteral("C.03"), QStringLiteral("com1"),
                             context, m_catalog, &address, &error, 16));
    QCOMPARE(address.value, QStringLiteral("12.03"));
}

/******************************************************************************
 * Function Name   : PlcAddressTests::maskAndTemplate
 * Description     : Unknown native bytes survive overlay and incomplete templates are rejected.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void PlcAddressTests::maskAndTemplate()
{
    PlcNativeAddressRecord encoded;
    PlcAddress address;
    QByteArray materialized;
    QString error;
    int index;

    QVERIFY(parsePlcAddressText(QStringLiteral("4x_bit12.03"), QStringLiteral("com1"), m_context,
                                 m_catalog, &address, &error));
    QVERIFY(encodePlcNativeAddress(address, m_context, m_catalog, m_binding, &encoded, &error));
    materialized = QByteArrayLiteral("sentinel");
    QVERIFY(!encoded.overlay(QByteArray(), &materialized, &error));
    QCOMPARE(materialized, QByteArrayLiteral("sentinel"));
    QVERIFY(encoded.overlay(QByteArray(36, char(0xa5)), &materialized, &error));
    for (index = 0; index < 36; ++index)
    {
        QCOMPARE(u8(materialized[index]), u8(encoded.knownMask[index]) == 0xff ?
                 u8(encoded.knownBytes[index]) : u8(0xa5));
    }
    QCOMPARE(materialized.mid(8, 4).toHex(), QByteArrayLiteral("c3000000"));
    encoded.knownMask[3] = char(1);
    materialized = QByteArrayLiteral("sentinel");
    QVERIFY(!encoded.overlay(QByteArray(36, char(0)), &materialized, &error));
    QCOMPARE(materialized, QByteArrayLiteral("sentinel"));
}

/******************************************************************************
 * Function Name   : PlcAddressTests::recoveredProtocolIntegration
 * Description     : Exercise five distinct native grammars through the public model and materializer.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void PlcAddressTests::recoveredProtocolIntegration() //20261109-0 add
{
    const QString profiles[5] = {
        QStringLiteral("Link/serial_amz/MITSUBISHI/Mitsubishi_Fx.ini"),
        QStringLiteral("Link/serial_amz/SIEMENS/Siemens_S7_300_MPI.ini"),
        QStringLiteral("Link/serial_amz/KEYENCE/KV1000.ini"),
        QStringLiteral("Link/serial_amz/LG/LG_K10S1.ini"),
        QStringLiteral("Link/serial_amz/PANASONIC/PANASONIC_FP.ini")};
    const QString inputs[5] = {QStringLiteral("X17"), QStringLiteral("DB_W1:12"),
        QStringLiteral("CR3915"), QStringLiteral("P01"), QStringLiteral("R9999F")};
    const u32 numbers[5] = {15, 8205, 639, 16, 159999};
    const u8 types[5] = {88, 68, 72, 89, 77};
    PlcAddressContext context;
    PlcAddress address;
    PlcNativeAddressRecord encoded;
    PlcNativeBinding binding;
    QByteArray record;
    QString error;
    QString formatted;
    int index;

    context = m_context; binding = m_binding;
    for (index = 0; index < 5; ++index)
    {
        context.connections[0].profileId = profiles[index];
        QVERIFY2(parsePlcAddressText(inputs[index], QStringLiteral("com1"), context, m_catalog,
                                    &address, &error), qPrintable(error));
        formatted = QStringLiteral("@com1:") + inputs[index];
        QCOMPARE(plcQualifiedAddressText(address), formatted);
        QVERIFY2(encodePlcNativeAddress(address, context, m_catalog, binding, &encoded, &error), qPrintable(error));
        QVERIFY(encoded.overlay(QByteArray(36, char(0)), &record, &error));
        QCOMPARE(u8(record[1]), types[index]);
        QCOMPARE(qFromLittleEndian<u32>(record.constData() + 8), numbers[index]);
        QVERIFY(parseQualifiedPlcAddressText(formatted, context, m_catalog, &address, &error));
    }
    context.connections[0].profileId = profiles[3];
    QVERIFY(parsePlcAddressText(QStringLiteral("P1"), QStringLiteral("com1"), context, m_catalog, &address, &error));
    QVERIFY(encodePlcNativeAddress(address, context, m_catalog, binding, &encoded, &error));
    QVERIFY(encoded.overlay(QByteArray(36, char(0)), &record, &error));
    QCOMPARE(qFromLittleEndian<u32>(record.constData() + 8), u32(1));
    QVERIFY(!parsePlcAddressText(QStringLiteral("P01"), QStringLiteral("com1"), context, m_catalog, &address, &error, 16));
}

QTEST_GUILESS_MAIN(PlcAddressTests)
#include "plc_address_tests.moc"
