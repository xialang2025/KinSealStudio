#include "device_transfer_plan.h"
#include "device_transfer_contract.h" //20261114-5 add

#include <QtTest>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QTemporaryDir>
#include <QFile>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

//20261109-0 add: Expected orders come from recorded native transfers, not USB hardware in tests.
class DeviceTransferTests : public QObject
{
    Q_OBJECT
private slots:
    void nativeStaticOrder();
    void nativeBitButtonOrder();
    void nativeMacroHostOrder(); //20261109-0 add
    void incompletePayloadIsRejected();
    void offlineContracts(); //20261114-5 add
    void rejectedContractPreservesOutput();
    void contractSourceAliases(); //20261114-5 add
};

/******************************************************************************
 * Function Name   : markerPayloads
 * Description     : Give each source block a distinct size and contents for wire checks.
 * Input Parameters: count
 * Return Value    : Ordered compiler blocks.
 * Date            : 2026.11.09
 ******************************************************************************/
static QVector<QByteArray> markerPayloads(int count) //20261109-0 add
{
    QVector<QByteArray> payloads;
    int index;

    for (index = 0; index < count; ++index)
    {
        payloads.append(QByteArray(257 + index, char(index + 1)));
    }
    return payloads;
}

/******************************************************************************
 * Function Name   : DeviceTransferTests::nativeStaticOrder
 * Description     : Preserve the twelve-step, ten-data-block static protocol from six captures.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void DeviceTransferTests::nativeStaticOrder() //20261109-0 add
{
    QVector<QByteArray> payloads;
    QVector<HmiDownloadStep> steps;
    QByteArray commands;
    HmiDownloadStep step;
    QString error;
    int count;

    payloads = markerPayloads(10);
    QVERIFY2(buildDeviceTransferPlan(payloads, &steps, &error), qPrintable(error));
    count = 0;
    Q_FOREACH (step, steps)
    {
        commands.append(char(step.command));
        if (!step.bytes.isEmpty())
        {
            ++count;
        }
    }
    QCOMPARE(commands.toHex(), QByteArray("0a0c0e16151912140d1e0f11"));
    QCOMPARE(count, 10);
    QCOMPARE(steps.at(3).bytes.size(), 0);
    QCOMPARE(steps.at(5).bytes.size(), 0);
    QCOMPARE(steps.at(9).bytes.left(4).toHex(), QByteArray("00000108"));
    QCOMPARE(steps.at(9).bytes.mid(4), payloads.at(7));
    QCOMPARE(steps.last().options.toHex(), QByteArray("0a010000"));
}

/******************************************************************************
 * Function Name   : DeviceTransferTests::nativeBitButtonOrder
 * Description     : Match the 2026-11-09 LB0 toggle capture including the two additional segments.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void DeviceTransferTests::nativeBitButtonOrder() //20261109-0 add
{
    QVector<QByteArray> payloads;
    QVector<HmiDownloadStep> steps;
    HmiDownloadStep step;
    QByteArray commands;
    QString error;
    int count;

    payloads = markerPayloads(12);
    QVERIFY2(buildDeviceTransferPlan(payloads, &steps, &error), qPrintable(error));
    count = 0;
    Q_FOREACH (step, steps)
    {
        commands.append(char(step.command));
        if (!step.bytes.isEmpty())
        {
            ++count;
        }
    }
    QCOMPARE(commands.toHex(), QByteArray("0a0b0c0e16151912140d1e100f11"));
    QCOMPARE(count, 12);
    QCOMPARE(steps.at(1).bytes, payloads.at(10));
    QCOMPARE(steps.at(11).bytes, payloads.at(11));
    QCOMPARE(steps.at(4).bytes.size(), 0);
    QCOMPARE(steps.at(6).bytes.size(), 0);
    QVERIFY(steps.at(1).options.isEmpty());
    QVERIFY(steps.at(11).options.isEmpty());
    QCOMPARE(steps.at(10).bytes.mid(4), payloads.at(7));
}

/******************************************************************************
 * Function Name   : DeviceTransferTests::incompletePayloadIsRejected
 * Description     : Leave an existing plan intact when one compiler block is missing.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void DeviceTransferTests::incompletePayloadIsRejected() //20261109-0 add
{
    QVector<QByteArray> payloads;
    QVector<HmiDownloadStep> steps;
    QString error;

    payloads = markerPayloads(10);
    QVERIFY(buildDeviceTransferPlan(payloads, &steps, &error));
    payloads.append(QByteArray("control-only"));
    QVERIFY(!buildDeviceTransferPlan(payloads, &steps, &error));
    QCOMPARE(steps.size(), 12);
    payloads = markerPayloads(12);
    payloads[11].clear();
    QVERIFY(!buildDeviceTransferPlan(payloads, &steps, &error));
    QCOMPARE(steps.size(), 12);
    QVERIFY(!buildDeviceTransferPlan(markerPayloads(10), nullptr, &error));
}

/******************************************************************************
 * Function Name   : DeviceTransferTests::nativeMacroHostOrder
 * Description     : Match the instrumented native host macro sequence without claiming hardware ACKs.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void DeviceTransferTests::nativeMacroHostOrder() //20261109-0 add
{
    QVector<QByteArray> payloads;
    QVector<HmiDownloadStep> steps;
    HmiDownloadStep step;
    QByteArray commands;
    QString error;

    payloads = markerPayloads(14);
    QVERIFY2(buildDeviceTransferPlan(payloads, &steps, &error), qPrintable(error));
    Q_FOREACH (step, steps)
    {
        commands.append(char(step.command));
    }
    QCOMPARE(commands.toHex(), QByteArray("0a0b0c361d0e16151912140d1e100f11"));
    QCOMPARE(steps.at(3).bytes, payloads.at(12));
    QCOMPARE(steps.at(4).bytes, payloads.at(13));
}

/******************************************************************************
 * Function Name   : DeviceTransferTests::offlineContracts
 * Description     : Match all three packet schedules, framing digests, and post-reset completion.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferTests::offlineContracts() //20261114-5 add
{
    QVector<QByteArray> payloads;
    QJsonObject identity;
    QJsonObject contract;
    QJsonArray operations;
    QJsonObject row;
    QByteArray expected;
    QString error;
    int count;
    int payloadCount;
    int index;

    identity.insert(QStringLiteral("targetModel"), QStringLiteral("AMT070W124-L800X480R"));
    for (count = 10; count <= 14; count += 2)
    {
        payloads = markerPayloads(count);
        QVERIFY2(buildDeviceTransferContract(payloads, identity, &contract, &error), qPrintable(error));
        QCOMPARE(contract.value(QStringLiteral("payloadCount")).toInt(), count);
        QVERIFY(!contract.value(QStringLiteral("usbAccessed")).toBool());
        operations = contract.value(QStringLiteral("operations")).toArray();
        QCOMPARE(operations.size(), 3 * count + 11);
        QCOMPARE(operations.first().toObject().value(QStringLiteral("frameHex")).toString(),
                 QStringLiteral("4b696e5365616c53747564696f4b697373310200000000000000000000000000"));
        QCOMPARE(operations.at(2).toObject().value(QStringLiteral("optionsHex")).toString(), QStringLiteral("000000"));
        payloadCount = 0;
        for (index = 0; index < operations.size(); ++index)
        {
            row = operations.at(index).toObject();
            QCOMPARE(row.value(QStringLiteral("sequence")).toInt(), index + 1);
            QCOMPARE(row.value(QStringLiteral("nextSequence")).toInt(), index + 1 == operations.size() ? 0 : index + 2);
            if (row.value(QStringLiteral("kind")).toString() != QStringLiteral("payload"))
            {
                continue;
            }
            ++payloadCount;
            if (row.value(QStringLiteral("name")).toString() == QStringLiteral("upLoad.knr"))
            {
                expected = payloads.at(7);
                expected.prepend(QByteArray::fromHex("00000108"));
                QCOMPARE(row.value(QStringLiteral("wireSha256")).toString(), QString::fromLatin1(QCryptographicHash::hash(expected, QCryptographicHash::Sha256).toHex()));
            }
        }
        QCOMPARE(payloadCount, count);
        QCOMPARE(operations.at(operations.size() - 2).toObject().value(QStringLiteral("commandHex")).toString(), QStringLiteral("1a"));
        QCOMPARE(operations.last().toObject().value(QStringLiteral("observedReturn")).toInt(), -5);
    }
}

/******************************************************************************
 * Function Name   : DeviceTransferTests::rejectedContractPreservesOutput
 * Description     : Keep previous output intact for invalid target, options, or missing segments.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferTests::rejectedContractPreservesOutput() //20261114-5 add
{
    QJsonObject identity;
    QJsonObject output;
    QJsonObject before;
    QString error;

    identity.insert(QStringLiteral("targetModel"), QStringLiteral("AMT070W124-L800X480R"));
    output.insert(QStringLiteral("old"), true);
    before = output;
    QVERIFY(!buildDeviceTransferContract(markerPayloads(11), identity, &output, &error));
    QCOMPARE(output, before);
    QVERIFY(!buildDeviceTransferContract(markerPayloads(10), identity, &output, &error, QByteArray::fromHex("020000")));
    QCOMPARE(output, before);
    identity.insert(QStringLiteral("targetModel"), QStringLiteral("other"));
    QVERIFY(!buildDeviceTransferContract(markerPayloads(10), identity, &output, &error));
    QCOMPARE(output, before);
}

/******************************************************************************
 * Function Name   : DeviceTransferTests::contractSourceAliases
 * Description     : Reject Windows case aliases and hard links while permitting separate destinations.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferTests::contractSourceAliases() //20261114-5 add
{
    QTemporaryDir directory;
    QString source;
    QString separate;
    QString alias;
    QFile file;

    QVERIFY(directory.isValid());
    source = directory.filePath(QStringLiteral("source.json"));
    separate = directory.filePath(QStringLiteral("output.json"));
    alias = directory.filePath(QStringLiteral("linked.json"));
    file.setFileName(source);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QCOMPARE(file.write("source"), qint64(6));
    file.close();
    QVERIFY(transferContractWouldReplaceInput(source, source));
    QVERIFY(transferContractWouldReplaceInput(source, directory.filePath(QStringLiteral("./source.json"))));
    QVERIFY(!transferContractWouldReplaceInput(source, separate));
    file.setFileName(separate);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QCOMPARE(file.write("other"), qint64(5));
    file.close();
    QVERIFY(!transferContractWouldReplaceInput(source, separate));
#ifdef Q_OS_WIN
    QVERIFY(transferContractWouldReplaceInput(source, directory.filePath(QStringLiteral("SOURCE.JSON"))));
    QVERIFY(CreateHardLinkW(reinterpret_cast<LPCWSTR>(alias.utf16()), reinterpret_cast<LPCWSTR>(source.utf16()), nullptr) != 0);
    QVERIFY(transferContractWouldReplaceInput(source, alias));
#endif
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), QByteArray("other"));
}

QTEST_APPLESS_MAIN(DeviceTransferTests)
#include "device_transfer_tests.moc"
