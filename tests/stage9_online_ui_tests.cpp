#include "plc_online_dialog.h"
#include "plc_protocol_capabilities.h"

#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QSignalSpy>
#include <QSpinBox>
#include <QTableWidget>
#include <QTabWidget>
#include <QThread>
#include <QTimer>
#include <QtTest>

//20261114-8 add: No callback captures a widget or accesses an actual serial endpoint.
struct OnlineUiFixture
{
    std::atomic_int factoryCalls{0};
    std::atomic_int openCalls{0};
    std::atomic_int writeCalls{0};
    std::atomic_int readCalls{0};
    std::atomic_int closeCalls{0};
    std::atomic_bool releaseRead{true};
    std::atomic_bool releaseClose{true};
    std::atomic_bool readEntered{false};
    std::atomic_bool closeEntered{false};
    std::atomic_bool closeFinished{false};
    std::atomic_bool cancellationObserved{false};
    std::atomic_bool callbackOnGuiThread{false};
    std::atomic<QThread *> worker{nullptr};
    Qt::HANDLE guiThread = QThread::currentThreadId();
    QByteArray request;
    QByteArray pending;
    PlcSerialSettings settings;
};

class Stage9OnlineUiTests : public QObject //20261114-8 add
{
    Q_OBJECT
private slots:
    void initialNoImplicitPortOrIo();
    void invalidRequestNeverCreatesEndpoint_data();
    void invalidRequestNeverCreatesEndpoint();
    void operationControls_data();
    void operationControls();
    void maximumReadKeepsCompleteDisplayAndReport();
    void successfulWorkerFreezesAndRestores();
    void cancelledWorkerFreezesAndRestores();
    void rejectWaitsForCancelledClose();
    void ownerDestructionFinishesWorkerWithoutWidgetAccess();
    void capabilitiesRemainConservative();
};

/******************************************************************************
 * Function Name   : fixtureU16
 * Description     : Read a bounded request quantity for the controlled UI fixture, independent of native address profiles.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
static int fixtureU16(const QByteArray &bytes, int offset) //20261114-8 add
{
    return (int(u8(bytes.at(offset))) << 8) | int(u8(bytes.at(offset + 1)));
}

/******************************************************************************
 * Function Name   : fixtureReply
 * Description     : Create a legal single-request response so widget tests exercise the real session and transport.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
static QByteArray fixtureReply(const QByteArray &request) //20261114-8 add
{
    QByteArray result;
    int operation;
    int count;
    int index;
    int byteIndex;
    u16 crc;

    operation = int(u8(request.at(1)));
    count = fixtureU16(request, 4);
    result.append(request.at(0)); result.append(request.at(1));
    if (operation == 1 || operation == 2)
    {
        result.append(char((count + 7) / 8));
        result.append(QByteArray((count + 7) / 8, char(0)));
        for (index = 0; index < count; ++index)
        {
            if ((index % 2) != 0)
            {
                byteIndex = 3 + index / 8;
                result[byteIndex] = char(u8(result.at(byteIndex)) | u8(1U << unsigned(index % 8)));
            }
        }
    }
    else if (operation == 3 || operation == 4)
    {
        result.append(char(count * 2));
        for (index = 0; index < count; ++index)
        { result.append(char((100 + index) >> 8)); result.append(char((100 + index) & 0xff)); }
    }
    else { result = request.left(6); }
    crc = simulationModbusCrc16(result); // Fixture construction only; CRC conformance is tested by the transport suite.
    result.append(char(crc & 0xffU)); result.append(char(crc >> 8));
    return result;
}

/******************************************************************************
 * Function Name   : controlledFactory
 * Description     : Provide only worker-owned in-memory I/O with explicit release gates and observable cleanup.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
static PlcSerialIoFactory controlledFactory(const std::shared_ptr<OnlineUiFixture> &state) //20261114-8 add
{
    return [state](const PlcSerialSettings &settings, const std::function<bool()> &cancelled) {
        SimulationModbusIo io;

        state->settings = settings; state->worker.store(QThread::currentThread());
        state->callbackOnGuiThread.store(QThread::currentThreadId() == state->guiThread);
        state->factoryCalls.fetch_add(1);
        io.open = [state, cancelled](QString *) {
            state->openCalls.fetch_add(1);
            if (QThread::currentThreadId() == state->guiThread) { state->callbackOnGuiThread.store(true); }
            return !cancelled();
        };
        io.write = [state](const QByteArray &bytes, int) {
            state->request = bytes; state->pending = fixtureReply(bytes); state->writeCalls.fetch_add(1);
            if (QThread::currentThreadId() == state->guiThread) { state->callbackOnGuiThread.store(true); }
            return int(bytes.size());
        };
        io.read = [state, cancelled](int maximum, int timeoutMs) {
            SimulationModbusReadChunk chunk;
            QElapsedTimer elapsed;

            state->readCalls.fetch_add(1); state->readEntered.store(true); elapsed.start();
            while (!state->releaseRead.load() && !cancelled() && elapsed.elapsed() < timeoutMs) { QThread::msleep(1); }
            if (QThread::currentThreadId() == state->guiThread) { state->callbackOnGuiThread.store(true); }
            if (cancelled())
            {
                state->cancellationObserved.store(true); chunk.error = QStringLiteral("Controlled read cancelled."); return chunk;
            }
            if (elapsed.elapsed() >= timeoutMs) { chunk.timedOut = true; return chunk; }
            if (state->pending.isEmpty()) { chunk.endOfFrame = true; }
            else { chunk.bytes = state->pending.left(maximum); state->pending.remove(0, chunk.bytes.size()); }
            return chunk;
        };
        io.close = [state](QString *error) {
            QElapsedTimer elapsed;

            state->closeCalls.fetch_add(1); state->closeEntered.store(true); elapsed.start();
            while (!state->releaseClose.load() && elapsed.elapsed() < 3000) { QThread::msleep(1); }
            if (QThread::currentThreadId() == state->guiThread) { state->callbackOnGuiThread.store(true); }
            state->closeFinished.store(true);
            if (!state->releaseClose.load() && error != nullptr) { *error = QStringLiteral("Controlled close gate timed out."); }
            return state->releaseClose.load();
        };
        return io;
    };
}

/******************************************************************************
 * Function Name   : combo
 * Description     : Find a real named combo box in the production dialog.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
static QComboBox *combo(PlcOnlineDialog &dialog, const char *name) //20261114-8 add
{
    return dialog.findChild<QComboBox *>(QString::fromLatin1(name));
}

/******************************************************************************
 * Function Name   : spin
 * Description     : Find a real named numeric input in the production dialog.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
static QSpinBox *spin(PlcOnlineDialog &dialog, const char *name) //20261114-8 add
{
    return dialog.findChild<QSpinBox *>(QString::fromLatin1(name));
}

/******************************************************************************
 * Function Name   : button
 * Description     : Find a real named action button in the production dialog.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
static QPushButton *button(PlcOnlineDialog &dialog, const char *name) //20261114-8 add
{
    return dialog.findChild<QPushButton *>(QString::fromLatin1(name));
}

/******************************************************************************
 * Function Name   : output
 * Description     : Read the actual displayed output rather than constructing a replacement view.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
static QPlainTextEdit *output(PlcOnlineDialog &dialog) //20261114-8 add
{
    return dialog.findChild<QPlainTextEdit *>(QStringLiteral("PlcOutput"));
}

/******************************************************************************
 * Function Name   : configureRequest
 * Description     : Enter an explicit controlled-only endpoint and standard PDU fields through real widgets.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
static void configureRequest(PlcOnlineDialog &dialog, int operation, int count = 1, int address = 0) //20261114-8 add
{
    combo(dialog, "PlcPcPort")->setEditText(QStringLiteral("CONTROLLED-ONLY"));
    combo(dialog, "PlcOperation")->setCurrentIndex(combo(dialog, "PlcOperation")->findData(operation));
    spin(dialog, "PlcStation")->setValue(7); spin(dialog, "PlcPduAddress")->setValue(address);
    spin(dialog, "PlcCount")->setValue(count); spin(dialog, "PlcTimeout")->setValue(5000);
}

/******************************************************************************
 * Function Name   : inputsFrozen
 * Description     : Verify every exposed request input is frozen while only the stop action remains enabled.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
static bool inputsFrozen(PlcOnlineDialog &dialog) //20261114-8 add
{
    const QStringList names = {QStringLiteral("PlcPcPort"), QStringLiteral("PlcBaud"), QStringLiteral("PlcParity"),
        QStringLiteral("PlcStopBits"), QStringLiteral("PlcStation"), QStringLiteral("PlcOperation"),
        QStringLiteral("PlcPduAddress"), QStringLiteral("PlcCount"), QStringLiteral("PlcValues"), QStringLiteral("PlcTimeout")};
    QWidget *widget;

    for (const QString &name : names)
    {
        widget = dialog.findChild<QWidget *>(name);
        if (widget == nullptr || widget->isEnabled()) { return false; }
    }
    return !button(dialog, "PlcExecute")->isEnabled() && button(dialog, "PlcCancel")->isEnabled();
}

/******************************************************************************
 * Function Name   : captureWidget
 * Description     : Save an owned widget capture only to the explicit acceptance capture directory.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
static void captureWidget(QWidget &widget, const QString &name) //20261114-8 add
{
    QString directory;

    directory = qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR");
    if (directory.isEmpty()) { return; }
    QVERIFY(QDir().mkpath(directory)); widget.show(); QApplication::processEvents();
    QVERIFY(widget.grab().save(QDir(directory).filePath(name)));
}

/******************************************************************************
 * Function Name   : Stage9OnlineUiTests::initialNoImplicitPortOrIo
 * Description     : Require an explicit port and keep construction independent of endpoint creation.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9OnlineUiTests::initialNoImplicitPortOrIo() //20261114-8 add
{
    std::shared_ptr<OnlineUiFixture> state = std::make_shared<OnlineUiFixture>();
    PlcOnlineDialog dialog(controlledFactory(state));

    QCOMPARE(combo(dialog, "PlcPcPort")->currentIndex(), -1);
    QVERIFY(combo(dialog, "PlcPcPort")->currentText().isEmpty()); QVERIFY(!dialog.running());
    QVERIFY(dialog.lastReport().isEmpty()); QCOMPARE(state->factoryCalls.load(), 0);
    QCOMPARE(state->openCalls.load(), 0); QVERIFY(!button(dialog, "PlcCancel")->isEnabled());
    button(dialog, "PlcExecute")->click();
    QVERIFY(!dialog.running()); QCOMPARE(state->factoryCalls.load(), 0);
    QVERIFY(!output(dialog)->toPlainText().isEmpty()); QVERIFY(dialog.lastReport().isEmpty());
}

/******************************************************************************
 * Function Name   : Stage9OnlineUiTests::invalidRequestNeverCreatesEndpoint_data
 * Description     : Exercise invalid widget input at the UI seam before any factory can run.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9OnlineUiTests::invalidRequestNeverCreatesEndpoint_data() //20261114-8 add
{
    QTest::addColumn<int>("operation"); QTest::addColumn<QString>("port"); QTest::addColumn<QString>("values");
    QTest::addColumn<int>("count"); QTest::addColumn<int>("address");
    QTest::newRow("no-port") << 3 << QString() << QStringLiteral("0") << 1 << 0;
    QTest::newRow("untrimmed-port") << 3 << QStringLiteral(" CONTROLLED ") << QStringLiteral("0") << 1 << 0;
    QTest::newRow("fraction") << 6 << QStringLiteral("CONTROLLED-ONLY") << QStringLiteral("1.5") << 1 << 0;
    QTest::newRow("coil-range") << 5 << QStringLiteral("CONTROLLED-ONLY") << QStringLiteral("2") << 1 << 0;
    QTest::newRow("word-range") << 6 << QStringLiteral("CONTROLLED-ONLY") << QStringLiteral("65536") << 1 << 0;
    QTest::newRow("multi-coil-shape") << 15 << QStringLiteral("CONTROLLED-ONLY") << QStringLiteral("1") << 2 << 0;
    QTest::newRow("multi-word-shape") << 16 << QStringLiteral("CONTROLLED-ONLY") << QStringLiteral("1") << 2 << 0;
    QTest::newRow("span-crosses-end") << 3 << QStringLiteral("CONTROLLED-ONLY") << QStringLiteral("0") << 2 << 65535;
    QTest::newRow("uint-overflow") << 6 << QStringLiteral("CONTROLLED-ONLY") << QStringLiteral("4294967296") << 1 << 0;
    QTest::newRow("negative") << 6 << QStringLiteral("CONTROLLED-ONLY") << QStringLiteral("-1") << 1 << 0;
    QTest::newRow("duplicate-comma") << 16 << QStringLiteral("CONTROLLED-ONLY") << QStringLiteral("1,,0") << 2 << 0;
    QTest::newRow("trailing-comma") << 15 << QStringLiteral("CONTROLLED-ONLY") << QStringLiteral("1,0,") << 2 << 0;
    QTest::newRow("leading-plus") << 16 << QStringLiteral("CONTROLLED-ONLY") << QStringLiteral("1,+0") << 2 << 0;
    QTest::newRow("whitespace-only") << 15 << QStringLiteral("CONTROLLED-ONLY") << QStringLiteral("   ") << 1 << 0;
}

/******************************************************************************
 * Function Name   : Stage9OnlineUiTests::invalidRequestNeverCreatesEndpoint
 * Description     : Validate actual action-button behavior without invoking a physical adapter.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9OnlineUiTests::invalidRequestNeverCreatesEndpoint() //20261114-8 add
{
    QFETCH(int, operation); QFETCH(QString, port); QFETCH(QString, values); QFETCH(int, count); QFETCH(int, address);
    std::shared_ptr<OnlineUiFixture> state = std::make_shared<OnlineUiFixture>();
    PlcOnlineDialog dialog(controlledFactory(state));

    configureRequest(dialog, operation, count, address); combo(dialog, "PlcPcPort")->setEditText(port);
    dialog.findChild<QLineEdit *>(QStringLiteral("PlcValues"))->setText(values); button(dialog, "PlcExecute")->click();
    QTRY_VERIFY_WITH_TIMEOUT(!dialog.running(), 2000);
    QCOMPARE(state->factoryCalls.load(), 0); QCOMPARE(state->openCalls.load(), 0);
    QVERIFY(!output(dialog)->toPlainText().trimmed().isEmpty()); QVERIFY(dialog.lastReport().isEmpty());
    QVERIFY(button(dialog, "PlcExecute")->isEnabled()); QVERIFY(!button(dialog, "PlcCancel")->isEnabled());
}

/******************************************************************************
 * Function Name   : Stage9OnlineUiTests::operationControls_data
 * Description     : List the eight explicit standard-PDU operation controls and adopted protocol limits.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9OnlineUiTests::operationControls_data() //20261114-8 add
{
    QTest::addColumn<int>("operation"); QTest::addColumn<int>("maximum");
    QTest::addColumn<bool>("countEnabled"); QTest::addColumn<bool>("valuesEnabled");
    QTest::newRow("01") << 1 << 2000 << true << false;
    QTest::newRow("02") << 2 << 2000 << true << false;
    QTest::newRow("03") << 3 << 125 << true << false;
    QTest::newRow("04") << 4 << 125 << true << false;
    QTest::newRow("05") << 5 << 1 << false << true;
    QTest::newRow("06") << 6 << 1 << false << true;
    QTest::newRow("15") << 15 << 1968 << true << true;
    QTest::newRow("16") << 16 << 123 << true << true;
}

/******************************************************************************
 * Function Name   : Stage9OnlineUiTests::operationControls
 * Description     : Change the real operation combo and inspect its count/value input affordances.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9OnlineUiTests::operationControls() //20261114-8 add
{
    QFETCH(int, operation); QFETCH(int, maximum); QFETCH(bool, countEnabled); QFETCH(bool, valuesEnabled);
    std::shared_ptr<OnlineUiFixture> state = std::make_shared<OnlineUiFixture>();
    PlcOnlineDialog dialog(controlledFactory(state));

    combo(dialog, "PlcOperation")->setCurrentIndex(combo(dialog, "PlcOperation")->findData(operation));
    QCOMPARE(spin(dialog, "PlcCount")->minimum(), 1); QCOMPARE(spin(dialog, "PlcCount")->maximum(), maximum);
    QCOMPARE(spin(dialog, "PlcCount")->isEnabled(), countEnabled);
    QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("PlcValues"))->isEnabled(), valuesEnabled);
    spin(dialog, "PlcCount")->setValue(maximum + 1); QCOMPARE(spin(dialog, "PlcCount")->value(), maximum);
    QCOMPARE(state->factoryCalls.load(), 0);
}

/******************************************************************************
 * Function Name   : Stage9OnlineUiTests::maximumReadKeepsCompleteDisplayAndReport
 * Description     : Keep all 2000 read values and the success heading in both the human view and the full receipt.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9OnlineUiTests::maximumReadKeepsCompleteDisplayAndReport() //20261114-8 add
{
    std::shared_ptr<OnlineUiFixture> state = std::make_shared<OnlineUiFixture>();
    PlcOnlineDialog dialog(controlledFactory(state));
    QJsonObject report;
    QJsonObject result;
    QJsonArray values;
    QStringList expected;
    QString displayed;
    int index;

    configureRequest(dialog, 1, 2000); button(dialog, "PlcExecute")->click();
    QTRY_VERIFY_WITH_TIMEOUT(!dialog.running(), 5000);
    report = dialog.lastReport(); QVERIFY(report.value(QStringLiteral("ok")).toBool());
    QVERIFY(report.value(QStringLiteral("opened")).toBool()); QVERIFY(report.value(QStringLiteral("closed")).toBool());
    QCOMPARE(report.value(QStringLiteral("requestedCount")).toInt(), 1); QCOMPARE(report.value(QStringLiteral("completedCount")).toInt(), 1);
    QCOMPARE(report.value(QStringLiteral("endpointKind")).toString(), QStringLiteral("injected-controlled-endpoint"));
    QVERIFY(!report.value(QStringLiteral("physicalEndpointUsed")).toBool());
    QVERIFY(!report.value(QStringLiteral("projectAddressMapping")).toBool());
    QVERIFY(!report.value(QStringLiteral("hmiExecutionConfirmed")).toBool());
    QCOMPARE(report.value(QStringLiteral("results")).toArray().size(), 1);
    result = report.value(QStringLiteral("results")).toArray().first().toObject(); values = result.value(QStringLiteral("values")).toArray();
    QCOMPARE(values.size(), 2000);
    for (index = 0; index < 2000; ++index) { QCOMPARE(values.at(index).toInt(), index % 2); expected.append(QString::number(index % 2)); }
    QCOMPARE(result.value(QStringLiteral("requestHex")).toString(), QString::fromLatin1(state->request.toHex()));
    displayed = output(dialog)->toPlainText(); QVERIFY(displayed.startsWith(QStringLiteral("收发完成。")));
    QVERIFY(displayed.contains(QStringLiteral("读取值（2000项）：") + expected.join(QStringLiteral(", "))));
    QVERIFY(displayed.contains(QStringLiteral("已关闭本次PC串口会话。")));
    QCOMPARE(state->factoryCalls.load(), 1); QCOMPARE(state->openCalls.load(), 1); QCOMPARE(state->closeCalls.load(), 1);
    QVERIFY(!state->callbackOnGuiThread.load()); captureWidget(dialog, QStringLiteral("online-max-read.png"));
}

/******************************************************************************
 * Function Name   : Stage9OnlineUiTests::successfulWorkerFreezesAndRestores
 * Description     : Freeze all request widgets until a controlled successful worker completes and restores read controls.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9OnlineUiTests::successfulWorkerFreezesAndRestores() //20261114-8 add
{
    std::shared_ptr<OnlineUiFixture> state = std::make_shared<OnlineUiFixture>();
    PlcOnlineDialog dialog(controlledFactory(state));

    state->releaseRead.store(false); configureRequest(dialog, 3, 3, 123); button(dialog, "PlcExecute")->click();
    QTRY_VERIFY_WITH_TIMEOUT(state->readEntered.load(), 3000); QVERIFY(dialog.running()); QVERIFY(inputsFrozen(dialog));
    spin(dialog, "PlcPduAddress")->setValue(999); // Programmatic changes cannot rewrite the already frozen request.
    dialog.executeRequest(); QCOMPARE(state->factoryCalls.load(), 1);
    state->releaseRead.store(true); QTRY_VERIFY_WITH_TIMEOUT(!dialog.running(), 3000);
    QVERIFY(dialog.lastReport().value(QStringLiteral("ok")).toBool());
    QVERIFY(state->request.startsWith(QByteArray::fromHex("0703007b0003")));
    QVERIFY(combo(dialog, "PlcPcPort")->isEnabled()); QVERIFY(spin(dialog, "PlcCount")->isEnabled());
    QVERIFY(!dialog.findChild<QLineEdit *>(QStringLiteral("PlcValues"))->isEnabled());
    QVERIFY(button(dialog, "PlcExecute")->isEnabled()); QVERIFY(!button(dialog, "PlcCancel")->isEnabled());
    QCOMPARE(state->closeCalls.load(), 1); QVERIFY(state->closeFinished.load()); QVERIFY(!state->callbackOnGuiThread.load());
}

/******************************************************************************
 * Function Name   : Stage9OnlineUiTests::cancelledWorkerFreezesAndRestores
 * Description     : Keep inputs frozen throughout cancellation and endpoint close, then restore single-write controls.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9OnlineUiTests::cancelledWorkerFreezesAndRestores() //20261114-8 add
{
    std::shared_ptr<OnlineUiFixture> state = std::make_shared<OnlineUiFixture>();
    PlcOnlineDialog dialog(controlledFactory(state));

    state->releaseRead.store(false); state->releaseClose.store(false); configureRequest(dialog, 6);
    dialog.findChild<QLineEdit *>(QStringLiteral("PlcValues"))->setText(QStringLiteral("42"));
    button(dialog, "PlcExecute")->click(); QTRY_VERIFY_WITH_TIMEOUT(state->readEntered.load(), 3000);
    QVERIFY(inputsFrozen(dialog)); button(dialog, "PlcCancel")->click();
    QTRY_VERIFY_WITH_TIMEOUT(state->closeEntered.load(), 3000); QVERIFY(dialog.running()); QVERIFY(inputsFrozen(dialog));
    state->releaseClose.store(true); QTRY_VERIFY_WITH_TIMEOUT(!dialog.running(), 3000);
    QVERIFY(!dialog.lastReport().value(QStringLiteral("ok")).toBool());
    QVERIFY(dialog.lastReport().value(QStringLiteral("cancelled")).toBool());
    QVERIFY(dialog.lastReport().value(QStringLiteral("closed")).toBool());
    QVERIFY(state->cancellationObserved.load()); QCOMPARE(state->closeCalls.load(), 1);
    QVERIFY(combo(dialog, "PlcPcPort")->isEnabled()); QVERIFY(!spin(dialog, "PlcCount")->isEnabled());
    QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("PlcValues"))->isEnabled());
    QVERIFY(button(dialog, "PlcExecute")->isEnabled()); QVERIFY(!button(dialog, "PlcCancel")->isEnabled());
}

/******************************************************************************
 * Function Name   : Stage9OnlineUiTests::rejectWaitsForCancelledClose
 * Description     : Return from the real modal loop only after cancellation and the owned endpoint close finish.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9OnlineUiTests::rejectWaitsForCancelledClose() //20261114-8 add
{
    std::shared_ptr<OnlineUiFixture> state = std::make_shared<OnlineUiFixture>();
    PlcOnlineDialog dialog(controlledFactory(state));
    QTimer controller;
    bool requested = false;
    bool deferred = false;
    int modalResult;

    state->releaseRead.store(false); state->releaseClose.store(false); configureRequest(dialog, 3);
    QObject::connect(&controller, &QTimer::timeout, &dialog, [&]() {
        if (state->readEntered.load() && !requested) { requested = true; dialog.reject(); }
        if (requested && state->closeEntered.load())
        {
            deferred = dialog.running() && dialog.isVisible(); dialog.reject();
            deferred = deferred && dialog.isVisible(); state->releaseClose.store(true); controller.stop();
        }
    });
    QTimer::singleShot(0, &dialog, [&]() { button(dialog, "PlcExecute")->click(); });
    QTimer::singleShot(5000, &dialog, [&]() { state->releaseRead.store(true); state->releaseClose.store(true); dialog.reject(); });
    controller.start(1); modalResult = dialog.exec(); controller.stop();
    QCOMPARE(modalResult, int(QDialog::Rejected)); QVERIFY(requested); QVERIFY(deferred); QVERIFY(!dialog.running());
    QVERIFY(state->closeFinished.load()); QCOMPARE(state->closeCalls.load(), 1);
    QVERIFY(dialog.lastReport().value(QStringLiteral("closed")).toBool());
    QVERIFY(dialog.lastReport().value(QStringLiteral("cancelled")).toBool());
}

/******************************************************************************
 * Function Name   : Stage9OnlineUiTests::ownerDestructionFinishesWorkerWithoutWidgetAccess
 * Description     : Delete the actual owner while blocked I/O is active and observe worker close and thread destruction.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9OnlineUiTests::ownerDestructionFinishesWorkerWithoutWidgetAccess() //20261114-8 add
{
    std::shared_ptr<OnlineUiFixture> state = std::make_shared<OnlineUiFixture>();
    std::unique_ptr<QWidget> owner = std::make_unique<QWidget>();
    QPointer<PlcOnlineDialog> dialog = new PlcOnlineDialog(controlledFactory(state), owner.get());
    QPointer<QThread> worker;

    state->releaseRead.store(false); configureRequest(*dialog, 3); dialog->show();
    button(*dialog, "PlcExecute")->click(); QTRY_VERIFY_WITH_TIMEOUT(state->readEntered.load(), 3000);
    worker = state->worker.load(); QVERIFY(!worker.isNull()); QVERIFY(dialog->running()); QVERIFY(inputsFrozen(*dialog));
    owner.reset(); QVERIFY(dialog.isNull());
    QTRY_VERIFY_WITH_TIMEOUT(state->closeFinished.load(), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(worker.isNull(), 3000);
    QVERIFY(state->cancellationObserved.load()); QCOMPARE(state->closeCalls.load(), 1);
    QVERIFY(!state->callbackOnGuiThread.load());
}

/******************************************************************************
 * Function Name   : Stage9OnlineUiTests::capabilitiesRemainConservative
 * Description     : Show all observed profiles while separating raw PC operations from one restricted AMT device capability.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9OnlineUiTests::capabilitiesRemainConservative() //20261114-8 add
{
    std::shared_ptr<OnlineUiFixture> state = std::make_shared<OnlineUiFixture>();
    PlcOnlineDialog dialog(controlledFactory(state));
    QJsonObject report = plcProtocolCapabilities();
    QJsonArray profiles = report.value(QStringLiteral("profiles")).toArray();
    QJsonObject profile;
    QJsonObject scope;
    QJsonObject raw;
    QTableWidget *table = dialog.findChild<QTableWidget *>(QStringLiteral("PlcCapabilities"));
    int eligible = 0;
    int eligibleRow = -1;
    int index;

    QVERIFY(report.value(QStringLiteral("ok")).toBool()); QCOMPARE(profiles.size(), 139);
    QCOMPARE(table->rowCount(), 139); QCOMPARE(table->columnCount(), 6);
    for (index = 0; index < profiles.size(); ++index)
    {
        profile = profiles.at(index).toObject();
        QCOMPARE(table->item(index, 0)->text(), profile.value(QStringLiteral("profileId")).toString());
        QCOMPARE(table->item(index, 4)->text(), QStringLiteral("未接入"));
        QVERIFY(!profile.value(QStringLiteral("pcProfileMapped")).toBool());
        QVERIFY(!profile.value(QStringLiteral("physicalPlcVerified")).toBool()); QVERIFY(!profile.value(QStringLiteral("hmiPlcVerified")).toBool());
        if (profile.value(QStringLiteral("deviceCompileEligible")).toBool())
        {
            ++eligible; eligibleRow = index;
            QCOMPARE(profile.value(QStringLiteral("profileId")).toString(), QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini"));
            scope = profile.value(QStringLiteral("deviceScope")).toObject();
            QCOMPARE(scope.value(QStringLiteral("channel")).toString(), QStringLiteral("COM1"));
            QCOMPARE(scope.value(QStringLiteral("stationText")).toString(), QStringLiteral("1"));
            QVERIFY(scope.value(QStringLiteral("nodeKnown")).toBool()); QCOMPARE(scope.value(QStringLiteral("node")).toInt(), 1);
            QVERIFY(scope.value(QStringLiteral("singleConnectionSinglePage")).toBool());
            QCOMPARE(table->item(index, 3)->text(), QStringLiteral("受限COM1/站1"));
        }
        else { QCOMPARE(table->item(index, 3)->text(), QStringLiteral("未开放")); }
    }
    QCOMPARE(eligible, 1); raw = report.value(QStringLiteral("pcRawRtu")).toObject();
    QCOMPARE(raw.value(QStringLiteral("functions")).toArray(), (QJsonArray{1, 2, 3, 4, 5, 6, 15, 16}));
    QVERIFY(raw.value(QStringLiteral("explicitZeroBasedPduAddress")).toBool());
    QVERIFY(!raw.value(QStringLiteral("automaticProjectMapping")).toBool());
    QVERIFY(!raw.value(QStringLiteral("physicalEndpointVerified")).toBool());
    QCOMPARE(raw.value(QStringLiteral("automaticRetries")).toInt(), 0);
    for (const QJsonValue &value : plcProtocolCapabilities(QStringLiteral("AMZ070W01")).value(QStringLiteral("profiles")).toArray())
    { QVERIFY(!value.toObject().value(QStringLiteral("deviceCompileEligible")).toBool()); }
    QCOMPARE(state->factoryCalls.load(), 0); dialog.findChild<QTabWidget *>()->setCurrentIndex(1);
    table->selectRow(eligibleRow); table->scrollToItem(table->item(eligibleRow, 0));
    captureWidget(dialog, QStringLiteral("online-capabilities.png"));
}

QTEST_MAIN(Stage9OnlineUiTests)
#include "stage9_online_ui_tests.moc"

