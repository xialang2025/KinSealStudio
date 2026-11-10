#include "device_scan_table.h"

#include <QtEndian>
#include <QtTest>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

//20261109-0 add: The fixture contains direct Common.dll 64149e70 results, not this encoder's output.
static const char commonOracleJson[] = R"json([
{"name":"family-0","reads":[[0,17,1]],"maximum":32,"ok":1,"result":[{"family":0,"start":17,"count":1,"node":1,"flags":8}]},
{"name":"family-1","reads":[[1,17,1]],"maximum":32,"ok":1,"result":[{"family":1,"start":17,"count":1,"node":1,"flags":8}]},
{"name":"family-2","reads":[[2,17,1]],"maximum":32,"ok":1,"result":[{"family":2,"start":16,"count":1,"node":1,"flags":8}]},
{"name":"family-3","reads":[[3,17,1]],"maximum":32,"ok":1,"result":[{"family":3,"start":16,"count":1,"node":1,"flags":8}]},
{"name":"family-4","reads":[[4,17,1]],"maximum":32,"ok":1,"result":[{"family":4,"start":16,"count":1,"node":1,"flags":8}]},
{"name":"family-5","reads":[[5,17,1]],"maximum":32,"ok":1,"result":[{"family":5,"start":16,"count":1,"node":1,"flags":8}]},
{"name":"family-6","reads":[[6,17,1]],"maximum":32,"ok":1,"result":[{"family":6,"start":17,"count":1,"node":1,"flags":8}]},
{"name":"family-7","reads":[[7,17,1]],"maximum":32,"ok":1,"result":[{"family":7,"start":17,"count":1,"node":1,"flags":8}]},
{"name":"family-8","reads":[[8,17,1]],"maximum":32,"ok":1,"result":[{"family":8,"start":16,"count":2,"node":1,"flags":8}]},
{"name":"family-9","reads":[[9,17,1]],"maximum":32,"ok":1,"result":[{"family":9,"start":16,"count":2,"node":1,"flags":8}]},
{"name":"family-10","reads":[[10,17,1]],"maximum":32,"ok":1,"result":[{"family":10,"start":17,"count":1,"node":1,"flags":8}]},
{"name":"family-11","reads":[[11,17,1]],"maximum":32,"ok":1,"result":[{"family":11,"start":17,"count":1,"node":1,"flags":8}]},
{"name":"reverse-families","reads":[[11,0,1],[10,0,1],[9,0,1],[8,0,1],[7,0,1],[6,0,1],[5,0,1],[4,0,1],[3,0,1],[2,0,1],[1,0,1],[0,0,1]],"maximum":32,"ok":1,"result":[{"family":0,"start":0,"count":1,"node":1,"flags":8},{"family":1,"start":0,"count":1,"node":1,"flags":8},{"family":2,"start":0,"count":1,"node":1,"flags":8},{"family":3,"start":0,"count":1,"node":1,"flags":8},{"family":4,"start":0,"count":1,"node":1,"flags":8},{"family":5,"start":0,"count":1,"node":1,"flags":8},{"family":6,"start":0,"count":1,"node":1,"flags":8},{"family":7,"start":0,"count":1,"node":1,"flags":8},{"family":8,"start":0,"count":2,"node":1,"flags":8},{"family":9,"start":0,"count":2,"node":1,"flags":8},{"family":10,"start":0,"count":1,"node":1,"flags":8},{"family":11,"start":0,"count":1,"node":1,"flags":8}]},
{"name":"bit-gap-1","reads":[[0,0,1],[0,2,1]],"maximum":32,"ok":1,"result":[{"family":0,"start":0,"count":3,"node":1,"flags":8}]},
{"name":"bit-gap-32","reads":[[0,0,1],[0,33,1]],"maximum":32,"ok":1,"result":[{"family":0,"start":0,"count":34,"node":1,"flags":8}]},
{"name":"bit-gap-33","reads":[[0,0,1],[0,34,1]],"maximum":32,"ok":1,"result":[{"family":0,"start":0,"count":1,"node":1,"flags":8},{"family":0,"start":34,"count":1,"node":1,"flags":8}]},
{"name":"bit-max-64","reads":[[0,0,1],[0,32,1],[0,63,1]],"maximum":32,"ok":1,"result":[{"family":0,"start":0,"count":64,"node":1,"flags":8}]},
{"name":"bit-large-65","reads":[[0,0,65]],"maximum":32,"ok":1,"result":[{"family":0,"start":0,"count":64,"node":1,"flags":8},{"family":0,"start":64,"count":1,"node":1,"flags":8}]},
{"name":"word-max-32","reads":[[7,0,1],[7,31,1]],"maximum":32,"ok":1,"result":[{"family":7,"start":0,"count":32,"node":1,"flags":8}]},
{"name":"word-max-33","reads":[[7,0,1],[7,32,1]],"maximum":32,"ok":1,"result":[{"family":7,"start":0,"count":1,"node":1,"flags":8},{"family":7,"start":32,"count":1,"node":1,"flags":8}]},
{"name":"word-contained","reads":[[7,0,10],[7,2,1],[7,0,2]],"maximum":32,"ok":1,"result":[{"family":7,"start":0,"count":10,"node":1,"flags":8}]},
{"name":"packed-bit-boundary","reads":[[3,15,17]],"maximum":32,"ok":1,"result":[{"family":3,"start":0,"count":2,"node":1,"flags":8}]},
{"name":"packed-bit-two","reads":[[3,15,1],[3,16,1]],"maximum":32,"ok":1,"result":[{"family":3,"start":0,"count":2,"node":1,"flags":8}]},
{"name":"packed-bit-gap-512","reads":[[3,0,1],[3,513,1]],"maximum":32,"ok":1,"result":[{"family":3,"start":0,"count":1,"node":1,"flags":8},{"family":3,"start":512,"count":1,"node":1,"flags":8}]},
{"name":"dw-normal","reads":[[9,2,2],[9,4,2]],"maximum":32,"ok":1,"result":[{"family":9,"start":2,"count":4,"node":1,"flags":8}]},
{"name":"dw-odd","reads":[[9,3,1]],"maximum":32,"ok":1,"result":[{"family":9,"start":2,"count":2,"node":1,"flags":8}]},
{"name":"profile-clamp","reads":[[7,0,600]],"maximum":1024,"ok":1,"result":[{"family":7,"start":0,"count":256,"node":1,"flags":8},{"family":7,"start":256,"count":256,"node":1,"flags":8},{"family":7,"start":512,"count":88,"node":1,"flags":8}]},
{"name":"contained-after-split","reads":[[7,0,100],[7,10,1]],"maximum":32,"ok":1,"result":[{"family":7,"start":0,"count":32,"node":1,"flags":8},{"family":7,"start":32,"count":32,"node":1,"flags":8},{"family":7,"start":64,"count":32,"node":1,"flags":8},{"family":7,"start":96,"count":4,"node":1,"flags":8}]},
{"name":"packed-contained-after-split","reads":[[3,0,1000],[3,10,1]],"maximum":32,"ok":1,"result":[{"family":3,"start":0,"count":32,"node":1,"flags":8},{"family":3,"start":512,"count":2,"node":1,"flags":8}]},
{"name":"packed-contained","reads":[[3,0,32],[3,1,1]],"maximum":32,"ok":1,"result":[{"family":3,"start":0,"count":1,"node":1,"flags":8}]},
{"name":"equal-start-sort","reads":[[3,20,1],[3,20,17],[3,0,1]],"maximum":32,"ok":1,"result":[{"family":3,"start":0,"count":2,"node":1,"flags":8}]}
])json";

class DeviceScanTableTests : public QObject
{
    Q_OBJECT
private slots:
    void nativeUiTables();
    void commonSorterCases_data();
    void commonSorterCases();
    void tupleIsolation();
    void boundsAreTransactional();
    void segmentCountBounds();
};

/******************************************************************************
 * Function Name   : DeviceScanTableTests::nativeUiTables
 * Description     : Compare original UI-compiled toggle, monitor and 32-bit permission subscriptions.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void DeviceScanTableTests::nativeUiTables() //20261109-0 add
{
    QVector<DeviceScanRead> reads;
    QByteArray table;
    QString error;

    reads = {{0x4d, 1, 1, 0, 1}, {0x4d, 1, 1, 0, 1}};
    QVERIFY2(buildDeviceScanTable(reads, &table, &error), qPrintable(error));
    QCOMPARE(table.size(), 212);
    QCOMPARE(table[0], char(1));
    QCOMPARE(table.mid(200).toHex(), QByteArray("4d0100010100000000000000"));
    reads[1].start = 1;
    QVERIFY2(buildDeviceScanTable(reads, &table, &error), qPrintable(error));
    QCOMPARE(table.mid(200).toHex(), QByteArray("4d0100010200000000000000"));
    reads[1] = {0x44, 1, 1, 0, 1};
    QVERIFY2(buildDeviceScanTable(reads, &table, &error), qPrintable(error));
    QCOMPARE(table[0], char(2));
    QCOMPARE(table.mid(200).toHex(), QByteArray("4d0100010100000000000000440100010100000000000000"));
    reads = {{0x44, 1, 1, 0, 2}};
    QVERIFY2(buildDeviceScanTable(reads, &table, &error), qPrintable(error));
    QCOMPARE(table.mid(200).toHex(), QByteArray("440100010200000000000000"));
}

/******************************************************************************
 * Function Name   : DeviceScanTableTests::commonSorterCases_data
 * Description     : Register independently observed native sorting and merge boundary fixtures.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void DeviceScanTableTests::commonSorterCases_data() //20261109-0 add
{
    QJsonArray cases;
    QJsonObject test;
    int index;

    QTest::addColumn<QByteArray>("testJson");
    cases = QJsonDocument::fromJson(QByteArray(commonOracleJson)).array();
    for (index = 0; index < cases.size(); ++index)
    {
        test = cases[index].toObject();
        QTest::newRow(test[QStringLiteral("name")].toString().toUtf8().constData())
            << QJsonDocument(test).toJson(QJsonDocument::Compact);
    }
}

/******************************************************************************
 * Function Name   : DeviceScanTableTests::commonSorterCases
 * Description     : Compare aligned starts and read counts with direct original Common.dll calls.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void DeviceScanTableTests::commonSorterCases() //20261109-0 add
{
    const u8 types[12] = {0x4d, 0x58, 0x48, 0x4b, 0x56, 0x4c,
                          0x52, 0x44, 0x74, 0x63, 0x65, 0x66};
    QJsonObject test;
    QJsonObject expected;
    QJsonArray inputs;
    QJsonArray input;
    QJsonArray outputs;
    QVector<DeviceScanRead> reads;
    QByteArray table;
    QByteArray record;
    QString error;
    int index;

    QFETCH(QByteArray, testJson);
    test = QJsonDocument::fromJson(testJson).object();
    inputs = test[QStringLiteral("reads")].toArray();
    outputs = test[QStringLiteral("result")].toArray();
    for (index = 0; index < inputs.size(); ++index)
    {
        input = inputs[index].toArray();
        reads.append({types[input[0].toInt()], 1, 1, u32(input[1].toDouble()), u32(input[2].toDouble())});
    }
    QVERIFY2(buildDeviceScanTable(reads, &table, &error,
                                  u32(test[QStringLiteral("maximum")].toInt())), qPrintable(error));
    QCOMPARE(table.size(), 200 + outputs.size() * 12);
    QCOMPARE(u8(table[0]), u8(outputs.size()));
    QCOMPARE(table.mid(1, 199), QByteArray(199, char(0)));
    for (index = 0; index < outputs.size(); ++index)
    {
        expected = outputs[index].toObject();
        record = table.mid(200 + index * 12, 12);
        QCOMPARE(u8(record[0]), types[expected[QStringLiteral("family")].toInt()]);
        QCOMPARE(u8(record[1]), u8(1));
        QCOMPARE(u8(record[2]), u8(0));
        QCOMPARE(u8(record[3]), u8(1));
        QCOMPARE(record.mid(6, 2), QByteArray(2, char(0)));
        QCOMPARE(qFromLittleEndian<u16>(record.constData() + 4), u16(expected[QStringLiteral("count")].toInt()));
        QCOMPARE(qFromLittleEndian<u32>(record.constData() + 8), u32(expected[QStringLiteral("start")].toDouble()));
    }
}

/******************************************************************************
 * Function Name   : DeviceScanTableTests::tupleIsolation
 * Description     : Keep equal address ranges separate when their COM route or station differs.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void DeviceScanTableTests::tupleIsolation() //20261109-0 add
{
    QVector<DeviceScanRead> reads;
    QByteArray table;
    QString error;

    reads = {{0x44, 2, 3, 0, 1}, {0x4d, 1, 2, 0, 1}, {0x4d, 2, 3, 0, 1}, {0x4d, 2, 4, 0, 1}};
    QVERIFY2(buildDeviceScanTable(reads, &table, &error), qPrintable(error));
    QCOMPARE(u8(table[0]), u8(4));
    QCOMPARE(table.mid(200).toHex(), QByteArray("4d02000301000000000000004402000301000000000000004d01000201000000000000004d0200040100000000000000"));
}

/******************************************************************************
 * Function Name   : DeviceScanTableTests::boundsAreTransactional
 * Description     : Preserve prior output when a family, route, count or address range is invalid.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void DeviceScanTableTests::boundsAreTransactional() //20261109-0 add
{
    QVector<DeviceScanRead> reads;
    QByteArray table;
    QString error;

    table = QByteArray("keep");
    reads = {{0x99, 1, 1, 0, 1}};
    QVERIFY(!buildDeviceScanTable(reads, &table, &error));
    QCOMPARE(table, QByteArray("keep"));
    reads = {{0x4d, 0, 1, 0, 1}};
    QVERIFY(!buildDeviceScanTable(reads, &table, &error));
    reads = {{0x4d, 1, 1, 0, 0}};
    QVERIFY(!buildDeviceScanTable(reads, &table, &error));
    reads = {{0x4d, 1, 1, 0xffffffffU, 2}};
    QVERIFY(!buildDeviceScanTable(reads, &table, &error));
    QVERIFY(!buildDeviceScanTable({}, &table, &error, 0));
    QVERIFY(!buildDeviceScanTable({}, nullptr, &error));
    QCOMPARE(table, QByteArray("keep"));
    QVERIFY(buildDeviceScanTable({}, &table, &error));
    QCOMPARE(table, QByteArray(200, char(0)));
    QVERIFY(error.isEmpty());
}

/******************************************************************************
 * Function Name   : DeviceScanTableTests::segmentCountBounds
 * Description     : Accept 255 disjoint segments and reject a 256th without wrapping the page byte.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void DeviceScanTableTests::segmentCountBounds() //20261109-0 add
{
    QVector<DeviceScanRead> reads;
    QByteArray table;
    QString error;
    int index;

    for (index = 0; index < 255; ++index)
    {
        reads.append({0x4d, 1, 1, u32(index * 100), 1});
    }
    QVERIFY2(buildDeviceScanTable(reads, &table, &error), qPrintable(error));
    QCOMPARE(u8(table[0]), u8(255));
    QCOMPARE(table.size(), 3260);
    table = QByteArray("keep");
    reads.append({0x4d, 1, 1, 25500, 1});
    QVERIFY(!buildDeviceScanTable(reads, &table, &error));
    QCOMPARE(table, QByteArray("keep"));
}

QTEST_APPLESS_MAIN(DeviceScanTableTests)
#include "device_scan_table_tests.moc"
