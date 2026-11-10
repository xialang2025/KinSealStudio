#include "device_transfer_plan.h"

#include <QtTest>

//20261109-0 add: Expected orders come from recorded native transfers, not USB hardware in tests.
class DeviceTransferTests : public QObject
{
    Q_OBJECT
private slots:
    void nativeStaticOrder();
    void nativeBitButtonOrder();
    void nativeMacroHostOrder(); //20261109-0 add
    void incompletePayloadIsRejected();
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

QTEST_APPLESS_MAIN(DeviceTransferTests)
#include "device_transfer_tests.moc"
