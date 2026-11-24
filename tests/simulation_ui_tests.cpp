#include "main_window.h"
#include "simulation_window.h"
#include "plc_online_dialog.h" //20261114-8 add
#include <QComboBox>

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QFocusEvent>
#include <QElapsedTimer> //20261124-13 add
#include <QJsonDocument>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>
#include <QTimer>
#include <QThread> //20261124-13 add
#include <QtTest>

class SimulationUiTests : public QObject //20261114-6 add
{
    Q_OBJECT
private:
    QTemporaryDir m_settings;
private slots:
    void initTestCase();
    void sharedFrameFollowsAddress();
    void heldMomentaryMoveAndCaptureLoss();
    void stoppedSessionKeepsSource();
    void snapshotDoesNotDirtyEditor();
    void duplicateActionOwnsOneWindow();
    void unsupportedProjectDoesNotStart();
    void parentCloseEndsRuntime();
    void onlineToolRequiresExplicitPort(); //20261124-13 change
    void onlinePollsWritesAndRecovers();
    void onlineWaitsForFreshValuesAfterWrite(); //20261124-14 add
    void onlineRejectsUnsafeOrOutOfRangeMapping();
    void invalidNumericInputCanRecover(); //20261114-6 add
    void directOwnerDeletionWithHeldViewport();
    void ownerDeletionInsideInputDialog();
};

/******************************************************************************
 * Function Name   : simulationTestProject
 * Description     : Build internal toggle, momentary, and lamp controls with deterministic plain artwork.
 * Input Parameters: None
 * Return Value    : A five-family-compatible immutable fixture.
 * Date            : 2026.11.14
 ******************************************************************************/
static HmiProject simulationTestProject() //20261114-6 add
{
    HmiProject project;
    HmiObject toggle;
    HmiObject momentary;
    HmiObject lamp;
    int index;

    project.name = QStringLiteral("Simulation test");
    project.targetModel = QStringLiteral("AMT070W124-L800X480R");
    toggle.id = QStringLiteral("toggle");
    toggle.name = toggle.id;
    toggle.type = QStringLiteral("switch");
    toggle.address = QStringLiteral("LB0");
    toggle.geometry = QRect(20, 20, 101, 60);
    toggle.hasBitButtonProperties = true;
    toggle.bitButton.operation = 3;
    toggle.bitButton.monitorSame = true;
    for (index = 0; index < 2; ++index)
    {
        toggle.bitButton.states[index].useImage = false;
        toggle.bitButton.states[index].background = index == 0 ? QColor(Qt::red) : QColor(Qt::green);
        toggle.bitButton.states[index].text.languageTexts.insert(QStringLiteral("中文"), index == 0 ? QStringLiteral("OFF") : QStringLiteral("ON"));
    }
    momentary = toggle;
    momentary.id = QStringLiteral("momentary");
    momentary.name = momentary.id;
    momentary.address = QStringLiteral("LB1");
    momentary.geometry = QRect(20, 110, 101, 60);
    momentary.bitButton.operation = 2;
    lamp = toggle;
    lamp.id = QStringLiteral("lamp");
    lamp.name = lamp.id;
    lamp.address.clear();
    lamp.geometry = QRect(200, 20, 101, 60);
    lamp.hasLampProperties = true;
    lamp.bitButton.monitorSame = false;
    lamp.bitButton.monitorBitAddress = QStringLiteral("LB0");
    project.objects = {toggle, momentary, lamp};
    return project;
}

/******************************************************************************
 * Function Name   : SimulationUiTests::initTestCase
 * Description     : Isolate editor preferences and recent files from the user's application settings.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationUiTests::initTestCase() //20261114-6 add
{
    QVERIFY(m_settings.isValid());
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_settings.path());
}

/******************************************************************************
 * Function Name   : SimulationUiTests::sharedFrameFollowsAddress
 * Description     : Observe a toggle and its separately monitored odd-width lamp changing through real widget events.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationUiTests::sharedFrameFollowsAddress() //20261114-6 add
{
    SimulationWindow window;
    SimulationViewport *view;
    QImage before;
    QImage after;
    QString error;
    bool bit = false;

    QVERIFY2(window.initialize(simulationTestProject(), &error), qPrintable(error));
    window.show();
    QApplication::processEvents();
    view = window.findChild<SimulationViewport *>();
    QVERIFY(view != nullptr);
    before = simulationFrame(window.runtime());
    QTest::mouseClick(view, Qt::LeftButton, Qt::NoModifier, QPoint(50, 45));
    QVERIFY(window.runtime().readBit(QStringLiteral("LB0"), &bit, &error));
    QVERIFY(bit);
    after = simulationFrame(window.runtime());
    QCOMPARE(before.size(), QSize(800, 480));
    QVERIFY(before.copy(QRect(200, 20, 101, 60)) != after.copy(QRect(200, 20, 101, 60)));
    QCOMPARE(window.runtime().visualState(window.runtime().project().objects.at(2)), 1);
}

/******************************************************************************
 * Function Name   : SimulationUiTests::heldMomentaryMoveAndCaptureLoss
 * Description     : Clear a momentary value on move-out, capture loss, and normal release independently.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationUiTests::heldMomentaryMoveAndCaptureLoss() //20261114-6 add
{
    SimulationWindow window;
    SimulationViewport *view;
    QEvent lost(QEvent::UngrabMouse);
    QString error;
    bool bit = false;

    QVERIFY2(window.initialize(simulationTestProject(), &error), qPrintable(error));
    window.show();
    QApplication::processEvents();
    view = window.findChild<SimulationViewport *>();
    QTest::mousePress(view, Qt::LeftButton, Qt::NoModifier, QPoint(50, 135));
    QVERIFY(window.runtime().readBit(QStringLiteral("LB1"), &bit, &error));
    QVERIFY(bit);
    {
        QMouseEvent move(QEvent::MouseMove, QPointF(500, 300), QPointF(500, 300), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);

        QApplication::sendEvent(view, &move); //20261114-6 change: Deliver an owned widget event without a system cursor operation.
    }
    QVERIFY(window.runtime().readBit(QStringLiteral("LB1"), &bit, &error));
    QVERIFY(!bit);
    QTest::mousePress(view, Qt::LeftButton, Qt::NoModifier, QPoint(50, 135));
    QApplication::sendEvent(view, &lost);
    QVERIFY(window.runtime().readBit(QStringLiteral("LB1"), &bit, &error));
    QVERIFY(!bit);
    QTest::mousePress(view, Qt::LeftButton, Qt::NoModifier, QPoint(50, 135));
    QTest::mouseRelease(view, Qt::LeftButton, Qt::NoModifier, QPoint(50, 135));
    QVERIFY(window.runtime().readBit(QStringLiteral("LB1"), &bit, &error));
    QVERIFY(!bit);
}

/******************************************************************************
 * Function Name   : SimulationUiTests::stoppedSessionKeepsSource
 * Description     : Close a held runtime and prove its input model remains byte-equivalent.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationUiTests::stoppedSessionKeepsSource() //20261114-6 add
{
    HmiProject source = simulationTestProject();
    QJsonObject before = source.toJson();
    SimulationWindow window;
    QString error;

    QVERIFY(window.initialize(source, &error));
    QVERIFY(window.runtime().press(QStringLiteral("momentary"), &error));
    window.close();
    QVERIFY(!window.runtime().running());
    QCOMPARE(source.toJson(), before);
    QCOMPARE(window.runtime().project().toJson(), before);
    QVERIFY(!window.runtime().snapshot().value(QStringLiteral("hardwareIo")).toBool());
}

/******************************************************************************
 * Function Name   : SimulationUiTests::snapshotDoesNotDirtyEditor
 * Description     : Start from unsaved edits and operate a detached runtime without changing editor content or history.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationUiTests::snapshotDoesNotDirtyEditor() //20261114-6 add
{
    MainWindow window;
    HmiProject edited;
    QJsonObject before;
    int history;
    QString error;

    window.setProject(simulationTestProject(), QString());
    edited = window.currentEditorProject();
    edited.name = QStringLiteral("Unsaved edit");
    QVERIFY(window.commitProjectDraft(edited, &error));
    QVERIFY(window.m_dirty);
    before = window.currentEditorProject().toJson();
    history = window.m_projectHistoryIndex;
    window.m_commands.value(QStringLiteral("action_offline_simulation"))->trigger();
    QVERIFY(!window.m_simulationWindow.isNull());
    QVERIFY(window.m_simulationWindow->runtime().press(QStringLiteral("toggle"), &error));
    QVERIFY(window.m_simulationWindow->runtime().release(true, &error));
    QCOMPARE(window.currentEditorProject().toJson(), before);
    QCOMPARE(window.m_projectHistoryIndex, history);
    QVERIFY(window.m_dirty);
    window.m_simulationWindow->close();
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

/******************************************************************************
 * Function Name   : SimulationUiTests::duplicateActionOwnsOneWindow
 * Description     : Repeated shell actions reuse one session rather than restarting its store.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationUiTests::duplicateActionOwnsOneWindow() //20261114-6 add
{
    MainWindow window;
    SimulationWindow *first;
    QString error;
    bool bit = false;

    window.setProject(simulationTestProject(), QString());
    window.startOfflineSimulation();
    first = window.m_simulationWindow.data();
    QVERIFY(first != nullptr);
    QVERIFY(first->runtime().writeBit(QStringLiteral("LB0"), true, &error));
    window.startOfflineSimulation();
    QCOMPARE(window.m_simulationWindow.data(), first);
    QCOMPARE(window.findChildren<SimulationWindow *>().size(), 1);
    QVERIFY(first->runtime().readBit(QStringLiteral("LB0"), &bit, &error));
    QVERIFY(bit);
    first->close();
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QVERIFY(window.m_simulationWindow.isNull());
}

/******************************************************************************
 * Function Name   : SimulationUiTests::unsupportedProjectDoesNotStart
 * Description     : Reject a future control before creating a live window and preserve the editor transaction.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationUiTests::unsupportedProjectDoesNotStart() //20261114-6 add
{
    MainWindow window;
    HmiProject project = simulationTestProject();
    QJsonObject before;
    bool rejected = false;

    project.objects[0].bitButton.pressTimeEnabled = true;
    window.setProject(project, QString());
    before = window.currentEditorProject().toJson();
    QTimer::singleShot(0, &window, [&rejected]() {
        QMessageBox *message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());

        if (message != nullptr) { rejected = !message->text().isEmpty(); message->accept(); }
    });
    window.startOfflineSimulation();
    QVERIFY(rejected);
    QVERIFY(window.m_simulationWindow.isNull());
    QCOMPARE(window.currentEditorProject().toJson(), before);
}

/******************************************************************************
 * Function Name   : SimulationUiTests::parentCloseEndsRuntime
 * Description     : Close an unchanged editor and release a still-held child runtime touch.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationUiTests::parentCloseEndsRuntime() //20261114-6 add
{
    MainWindow window;
    QPointer<SimulationWindow> child;
    QString error;

    window.setProject(simulationTestProject(), QString());
    window.startOfflineSimulation();
    child = window.m_simulationWindow;
    QVERIFY(child->runtime().press(QStringLiteral("momentary"), &error));
    QVERIFY(window.close());
    if (!child.isNull()) { QVERIFY(!child->runtime().running()); }
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QVERIFY(child.isNull());
}

/******************************************************************************
 * Function Name   : onlineTestProject
 * Description     : Describe one configured fixture with a coil, a numeric monitor and a two-register write.
 * Input Parameters: None
 * Return Value    : Controlled Modbus project.
 * Date            : 2026.11.24
 ******************************************************************************/
static HmiProject onlineTestProject() //20261124-13 add
{
    HmiProject project = simulationTestProject();
    PlcConnection connection;
    HmiConfigurationNode serial;
    HmiObject number;
    HmiObject word;

    connection.id = QStringLiteral("test-rtu");
    connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
    connection.channel = PlcChannel::Com1;
    connection.station = QStringLiteral("7");
    connection.timeoutMs = 1000;
    project.connections = {connection};
    serial.id = QStringLiteral("serial-port-com1");
    serial.kind = QStringLiteral("serialPort");
    serial.name = QStringLiteral("COM1");
    serial.active = true;
    serial.implemented = true;
    serial.properties = PlcSerialPortSettings::defaults(serial.name);
    serial.properties.insert(QStringLiteral("onlinePort"), QStringLiteral("COM8"));
    serial.properties.insert(QStringLiteral("baudRate"), QStringLiteral("19200"));
    serial.properties.insert(QStringLiteral("parity"), QStringLiteral("EVEN"));
    project.configurationNodes.append(serial);
    project.objects[0].address = QStringLiteral("@test-rtu:0x17");
    project.objects[2].bitButton.monitorBitAddress = project.objects[0].address;

    number.id = QStringLiteral("numeric");
    number.name = number.id;
    number.type = QStringLiteral("number");
    number.address = QStringLiteral("@test-rtu:4x_dw24");
    number.geometry = QRect(350, 20, 125, 60);
    number.hasNumberDisplayProperties = true;
    number.numberDisplay.changeDataType(6);
    number.numberDisplay.allowInput = false;
    number.numberDisplay.monitorAddress = number.address;
    project.objects.append(number);

    word = project.objects.first();
    word.id = QStringLiteral("set-word");
    word.name = word.id;
    word.address = number.address;
    word.geometry = QRect(350, 100, 125, 60);
    word.hasWordButtonProperties = true;
    word.bitButton.indicator = false;
    word.bitButton.dualMonitor = false;
    word.wordButton.resetDefaultsForDataType(1);
    word.wordButton.constantValue = QStringLiteral("131077");
    project.objects.append(word);
    return project;
}

struct OnlineSlave //20261124-13 add: Shared observations are atomic because the real serial worker drives the fake.
{
    std::atomic_int opens{0};
    std::atomic_int closes{0};
    std::atomic_int coilWrites{0};
    std::atomic_int wordWrites{0};
    std::atomic_bool settingsMatch{false};
    std::atomic_bool framesMatch{true};
    std::atomic_bool holdCoilAck{true};
    std::atomic_bool failNextCoilAck{false};
    std::atomic_bool holdRegisterReads{false}; //20261124-14 add
    std::atomic_int firstPollAfterWordWrite{0};
    bool coil = false;               // Owned only by the one worker for this phase of the session.
    u16 low = 321;
    u16 high = 1;
};

/******************************************************************************
 * Function Name   : onlineSlave
 * Description     : Make a controlled RTU responder that checks actual station, offsets, CRC and write data.
 * Input Parameters: slave
 * Return Value    : Injected transport factory with no physical endpoint.
 * Date            : 2026.11.24
 ******************************************************************************/
static PlcSerialIoFactory onlineSlave(const std::shared_ptr<OnlineSlave> &slave) //20261124-13 add
{
    return [slave](const PlcSerialSettings &settings, const std::function<bool()> &cancelled) {
        struct Reply { QByteArray bytes; bool wait = false; bool holdRegisterRead = false; bool timeout = false; bool sent = false; };
        const auto reply = std::make_shared<Reply>();
        SimulationModbusIo io;

        io.open = [slave, settings](QString *) {
            slave->settingsMatch = settings.portName == QStringLiteral("TEST-RTU") && settings.baudRate == 19200 &&
                                   settings.parity == 2 && settings.timeoutMs == 1000;
            ++slave->opens;
            return true;
        };
        io.write = [slave, reply](const QByteArray &frame, int) {
            const QByteArray payload = frame.left(frame.size() - 2);
            const int function = frame.size() > 1 ? u8(frame[1]) : 0;
            const u16 crc = simulationModbusCrc16(payload);
            QByteArray data;
            int first = 0;

            *reply = {};
            if (frame.size() < 8 || u8(frame[0]) != 7 || u8(frame[frame.size() - 2]) != u8(crc) ||
                u8(frame.back()) != u8(crc >> 8))
            {
                slave->framesMatch = false;
                return -1;
            }
            if (slave->wordWrites.load() > 0 && function >= 1 && function <= 4)
            {
                slave->firstPollAfterWordWrite.compare_exchange_strong(first, function);
            }
            data = payload.left(2);
            if (function == 1 && payload.mid(2) == QByteArray::fromHex("00110001"))
            {
                data.append(char(1)); data.append(char(slave->coil));
            }
            else if (function == 3 && payload.mid(2) == QByteArray::fromHex("00180002"))
            {
                data.append(char(4)); data.append(char(slave->low >> 8)); data.append(char(slave->low));
                data.append(char(slave->high >> 8)); data.append(char(slave->high));
                reply->holdRegisterRead = slave->wordWrites.load() > 0;
            }
            else if (function == 3 && payload.mid(2) == QByteArray::fromHex("00190001"))
            {
                data.append(char(2)); data.append(char(slave->high >> 8)); data.append(char(slave->high));
                reply->holdRegisterRead = slave->wordWrites.load() > 0;
            }
            else if (function == 4 && payload.mid(2) == QByteArray::fromHex("00190001"))
            {
                data.append(char(2)); data.append(char(0)); data.append(char(77));
            }
            else if (function == 5 && payload.mid(2, 2) == QByteArray::fromHex("0011") &&
                     (payload.mid(4) == QByteArray::fromHex("ff00") || payload.mid(4) == QByteArray::fromHex("0000")))
            {
                slave->coil = u8(payload[4]) != 0;
                reply->wait = true;
                reply->timeout = slave->failNextCoilAck.exchange(false);
                data = payload;
                ++slave->coilWrites;
            }
            else if (function == 16 && payload.mid(2) == QByteArray::fromHex("001800020400050002"))
            {
                slave->low = 5; slave->high = 2;
                data = payload.left(6);
                ++slave->wordWrites;
            }
            else
            {
                slave->framesMatch = false;
                return -1;
            }
            const u16 replyCrc = simulationModbusCrc16(data);
            data.append(char(replyCrc)); data.append(char(replyCrc >> 8));
            reply->bytes = data;
            return int(frame.size());
        };
        io.read = [slave, reply, cancelled](int, int timeoutMs) {
            SimulationModbusReadChunk chunk;
            QElapsedTimer wait;
            const auto held = [&]() {
                return (reply->wait && slave->holdCoilAck) || (reply->holdRegisterRead && slave->holdRegisterReads);
            };

            if (reply->timeout) { chunk.timedOut = true; return chunk; }
            wait.start();
            while (held() && !cancelled() && wait.elapsed() < timeoutMs)
            {
                QThread::msleep(1);
            }
            if (held()) { chunk.timedOut = true; return chunk; }
            if (reply->sent) { chunk.endOfFrame = true; }
            else { chunk.bytes = reply->bytes; reply->sent = true; }
            return chunk;
        };
        io.close = [slave](QString *) { ++slave->closes; return true; };
        return io;
    };
}

/******************************************************************************
 * Function Name   : SimulationUiTests::onlineToolRequiresExplicitPort
 * Description     : Open the engineering simulation from the menu and wait for an explicit Connect click.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationUiTests::onlineToolRequiresExplicitPort() //20261124-13 change
{
    MainWindow window;
    QJsonObject before;
    SimulationWindow *online;
    SimulationWindow preflight;
    QString error;
    QComboBox *pcPort;
    QLabel *status;
    QAction *diagnostics;
    bool bit;
    bool explained = false;

    window.setProject(onlineTestProject(), QString());
    before = window.currentEditorProject().toJson();
    QVERIFY2(preflight.initializeOnline(window.currentEditorProject(), &error), qPrintable(error));
    window.m_commands.value(QStringLiteral("action_online_simulation"))->trigger();
    online = window.m_simulationWindow;
    QVERIFY(online != nullptr && online->runtime().online());
    pcPort = online->findChild<QComboBox *>(QStringLiteral("onlineSimulationPort"));
    status = online->findChild<QLabel *>(QStringLiteral("simulationStatus"));
    QVERIFY(pcPort != nullptr && status != nullptr);
    QCOMPARE(pcPort->currentText(), QStringLiteral("COM8"));
    QVERIFY(status->text().contains(QStringLiteral("PLC 未连接")));
    QVERIFY(!online->runtime().readBit(QStringLiteral("@test-rtu:0x17"), &bit));
    online->close();
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    diagnostics = window.findChild<QAction *>(QStringLiteral("PlcSerialDiagnostics"));
    QVERIFY(diagnostics != nullptr);
    QTimer::singleShot(0, &window, [&explained]() {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());

        if (dialog != nullptr)
        {
            PlcOnlineDialog *online = qobject_cast<PlcOnlineDialog *>(dialog);
            QComboBox *port = dialog->findChild<QComboBox *>(QStringLiteral("PlcPcPort"));

            explained = online != nullptr && port != nullptr && port->currentIndex() == -1 &&
                port->currentText().isEmpty() && !online->running() && online->lastReport().isEmpty();
            dialog->reject();
        }
    });
    diagnostics->trigger();
    QVERIFY(explained);
    QCOMPARE(window.currentEditorProject().toJson(), before);
    QVERIFY(!window.m_dirty);
}

/******************************************************************************
 * Function Name   : SimulationUiTests::onlinePollsWritesAndRecovers
 * Description     : Verify polling, confirmed writes and recovery through the actual online viewport.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.24
 ******************************************************************************/
void SimulationUiTests::onlinePollsWritesAndRecovers() //20261124-13 add
{
    const auto slave = std::make_shared<OnlineSlave>();
    const HmiProject project = onlineTestProject();
    SimulationWindow window;
    SimulationViewport *view;
    QPushButton *connect;
    QComboBox *port;
    QLabel *status;
    QString error;
    bool bit = false;

    QVERIFY2(window.initializeOnline(project, &error, onlineSlave(slave)), qPrintable(error));
    window.show();
    QApplication::processEvents();
    view = window.findChild<SimulationViewport *>();
    connect = window.findChild<QPushButton *>(QStringLiteral("onlineSimulationConnect"));
    port = window.findChild<QComboBox *>(QStringLiteral("onlineSimulationPort"));
    status = window.findChild<QLabel *>(QStringLiteral("simulationStatus"));
    QVERIFY(view != nullptr && connect != nullptr && port != nullptr && status != nullptr);
    QCOMPARE(slave->opens.load(), 0);
    port->setEditText(QStringLiteral("TEST-RTU"));
    QTest::mouseClick(connect, Qt::LeftButton);
    QTRY_COMPARE(window.runtime().numberDisplayValue(project.objects.at(3)), QStringLiteral("65857"));
    QVERIFY(window.runtime().readBit(project.objects[0].address, &bit) && !bit);
    QCOMPARE(slave->opens.load(), 1);
    QVERIFY(slave->settingsMatch.load());

    QTest::mouseClick(view, Qt::LeftButton, Qt::NoModifier, project.objects[0].geometry.center());
    QTRY_COMPARE(slave->coilWrites.load(), 1);
    QVERIFY(window.runtime().readBit(project.objects[0].address, &bit) && !bit);
    slave->holdCoilAck = false;
    QTRY_COMPARE(window.runtime().visualState(project.objects[2]), 1);
    QTest::mouseClick(view, Qt::LeftButton, Qt::NoModifier, project.objects.last().geometry.center());
    QTRY_COMPARE(slave->wordWrites.load(), 1);
    QTRY_COMPARE(window.runtime().numberDisplayValue(project.objects.at(3)), QStringLiteral("131077"));

    slave->failNextCoilAck = true;
    QTest::mouseClick(view, Qt::LeftButton, Qt::NoModifier, project.objects[0].geometry.center());
    QTRY_VERIFY(status->text().contains(QStringLiteral("写入结果未确认")));
    QVERIFY(!window.runtime().readBit(project.objects[0].address, &bit));
    QCOMPARE(window.runtime().numberDisplayValue(project.objects.at(3)), QStringLiteral("—"));
    QCOMPARE(slave->coilWrites.load(), 2);
    QCOMPARE(slave->closes.load(), 1);

    QTest::mouseClick(connect, Qt::LeftButton);
    QTRY_VERIFY(window.runtime().readBit(project.objects[0].address, &bit) && !bit);
    QCOMPARE(slave->opens.load(), 2);
    QCOMPARE(slave->coilWrites.load(), 2);

    // Disconnect while a sent write is still awaiting an acknowledgement.
    slave->holdCoilAck = true; //20261124-14 add
    QTest::mouseClick(view, Qt::LeftButton, Qt::NoModifier, project.objects[0].geometry.center());
    QTRY_COMPARE(slave->coilWrites.load(), 3);
    QTest::mouseClick(connect, Qt::LeftButton);
    QVERIFY(status->text().contains(QStringLiteral("上一次写入未确认")));
    QVERIFY(!window.runtime().readBit(project.objects[0].address, &bit));
    QCOMPARE(slave->closes.load(), 2);
    QTest::mouseClick(connect, Qt::LeftButton);
    QTRY_VERIFY(window.runtime().readBit(project.objects[0].address, &bit) && bit);
    QCOMPARE(slave->opens.load(), 3);
    QCOMPARE(slave->coilWrites.load(), 3);
    QVERIFY(slave->framesMatch.load());
    QTest::keyClick(&window, Qt::Key_Escape);
    QCOMPARE(slave->closes.load(), 3);
    QVERIFY(!window.runtime().running());
}

/******************************************************************************
 * Function Name   : SimulationUiTests::onlineWaitsForFreshValuesAfterWrite
 * Description     : Ensure a write acknowledgement invalidates all overlapping scans until readback.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.24
 ******************************************************************************/
void SimulationUiTests::onlineWaitsForFreshValuesAfterWrite() //20261124-14 add
{
    const auto slave = std::make_shared<OnlineSlave>();
    HmiProject project = onlineTestProject();
    SimulationWindow window;
    SimulationViewport *view;
    QPushButton *connectButton;
    QComboBox *port;
    QLabel *status;
    QString error;
    QString value;
    bool bit = false;
    HmiObject &coil = project.objects[0];
    const QRect writeButton = project.objects.last().geometry;
    HmiObject input = project.objects.at(3);

    coil.bitButton.touch.visibilityEnabled = true;
    coil.bitButton.touch.wordEnabled = true;
    coil.bitButton.touch.wordAddress = QStringLiteral("@test-rtu:4x25");
    coil.bitButton.touch.comparison = 2;
    coil.bitButton.touch.compareValue = QStringLiteral("1");
    project.objects[2].bitButton.monitorBitAddress = QStringLiteral("@test-rtu:4x_bit25.00");

    input.id = QStringLiteral("input-reg");
    input.name = input.id;
    input.address = QStringLiteral("@test-rtu:3x25");
    input.geometry.translate(160, 0);
    input.numberDisplay.changeDataType(1);
    input.numberDisplay.monitorAddress = input.address;
    project.objects.append(input);

    QVERIFY2(window.initializeOnline(project, &error, onlineSlave(slave)), qPrintable(error));
    window.show();
    view = window.findChild<SimulationViewport *>();
    connectButton = window.findChild<QPushButton *>(QStringLiteral("onlineSimulationConnect"));
    port = window.findChild<QComboBox *>(QStringLiteral("onlineSimulationPort"));
    status = window.findChild<QLabel *>(QStringLiteral("simulationStatus"));
    QVERIFY(view != nullptr && connectButton != nullptr && port != nullptr && status != nullptr);
    port->setEditText(QStringLiteral("TEST-RTU"));
    QTest::mouseClick(connectButton, Qt::LeftButton);
    QTRY_VERIFY(window.runtime().touchAllowed(project.objects[0]));
    QTRY_COMPARE(window.runtime().numberDisplayValue(project.objects.at(3)), QStringLiteral("65857"));
    QTRY_COMPARE(window.runtime().numberDisplayValue(input), QStringLiteral("77"));
    QCOMPARE(window.runtime().visualState(project.objects.at(2)), 1);

    slave->holdRegisterReads = true;
    QTest::mouseClick(view, Qt::LeftButton, Qt::NoModifier, writeButton.center());
    QTRY_VERIFY(status->text().contains(QStringLiteral("PLC 已确认写入")));
    QVERIFY(!window.runtime().readWord(QStringLiteral("@test-rtu:4x25"), 0, &value));
    QVERIFY(!window.runtime().readBit(QStringLiteral("@test-rtu:4x_bit25.00"), &bit));
    QCOMPARE(window.runtime().numberDisplayValue(project.objects.at(3)), QStringLiteral("—"));
    QCOMPARE(window.runtime().numberDisplayValue(input), QStringLiteral("77"));
    QVERIFY(!window.runtime().touchAllowed(project.objects[0]));
    QTRY_COMPARE(slave->firstPollAfterWordWrite.load(), 3);
    slave->holdRegisterReads = false;

    QTRY_VERIFY(window.runtime().readWord(QStringLiteral("@test-rtu:4x25"), 0, &value));
    QCOMPARE(value, QStringLiteral("2"));
    QTRY_COMPARE(window.runtime().visualState(project.objects.at(2)), 0);
    QTRY_COMPARE(window.runtime().numberDisplayValue(project.objects.at(3)), QStringLiteral("131077"));
    QVERIFY(!window.runtime().touchAllowed(project.objects[0]));
    QCOMPARE(slave->coilWrites.load(), 0);
    QVERIFY(slave->framesMatch.load());
}

/******************************************************************************
 * Function Name   : SimulationUiTests::onlineRejectsUnsafeOrOutOfRangeMapping
 * Description     : Refuse read-only targets and offsets that a 16-bit Modbus PDU cannot carry.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.24
 ******************************************************************************/
void SimulationUiTests::onlineRejectsUnsafeOrOutOfRangeMapping() //20261124-13 add
{
    const auto slave = std::make_shared<OnlineSlave>();
    HmiProject project = onlineTestProject();
    SimulationWindow window;
    QString error;

    project.objects[0].address = QStringLiteral("@test-rtu:1x17");
    QVERIFY(!window.initializeOnline(project, &error, onlineSlave(slave)));
    QVERIFY(error.contains(QStringLiteral("在线写入仅支持")));
    project.objects[0].address = QStringLiteral("@test-rtu:0x17");
    project.objects[2].bitButton.monitorBitAddress = QStringLiteral("@test-rtu:1x65536");
    QVERIFY(!window.initializeOnline(project, &error, onlineSlave(slave)));
    QVERIFY(error.contains(QStringLiteral("零起始地址")));
    QCOMPARE(slave->opens.load(), 0);
}

/******************************************************************************
 * Function Name   : SimulationUiTests::invalidNumericInputCanRecover
 * Description     : Retry or cancel an invalid keypad value without leaving every subsequent touch blocked.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationUiTests::invalidNumericInputCanRecover() //20261114-6 add
{
    HmiProject project;
    HmiObject word;
    SimulationViewport *view;
    QString error;
    QString value;
    bool cancel;
    int dialogs;

    for (int scenario = 0; scenario < 2; ++scenario)
    {
        SimulationWindow window;

        project = simulationTestProject();
        word = project.objects.first();
        word.id = QStringLiteral("input-word");
        word.name = word.id;
        word.address = QStringLiteral("LW10");
        word.geometry = QRect(400, 20, 150, 60);
        word.hasWordButtonProperties = true;
        word.wordButton.dataType = 0;
        word.wordButton.operation = 1;
        word.wordButton.minimumValue = QStringLiteral("0");
        word.wordButton.maximumValue = QStringLiteral("65535");
        word.bitButton.indicator = false;
        word.bitButton.dualMonitor = false; //20261114-6 change: Word buttons require single-state artwork in the existing model schema.
        project.objects.append(word);
        QVERIFY2(window.initialize(project, &error), qPrintable(error));
        window.show();
        QApplication::processEvents();
        view = window.findChild<SimulationViewport *>();
        cancel = scenario == 1;
        dialogs = 0;
        QTimer::singleShot(0, &window, [&window, &dialogs, cancel]() {
            QInputDialog *dialog = qobject_cast<QInputDialog *>(QApplication::activeModalWidget());

            if (dialog != nullptr)
            {
                ++dialogs;
                dialog->setTextValue(QStringLiteral("70000"));
                dialog->accept();
                QTimer::singleShot(0, &window, [&dialogs, cancel]() {
                    QInputDialog *retry = qobject_cast<QInputDialog *>(QApplication::activeModalWidget());

                    if (retry != nullptr)
                    {
                        ++dialogs;
                        if (cancel) { retry->reject(); }
                        else { retry->setTextValue(QStringLiteral("7")); retry->accept(); }
                    }
                });
            }
        });
        QTest::mouseClick(view, Qt::LeftButton, Qt::NoModifier, QPoint(450, 45));
        QCOMPARE(dialogs, 2);
        QVERIFY(window.runtime().pendingInputObjectId().isEmpty());
        QVERIFY(window.runtime().readWord(QStringLiteral("LW10"), 0, &value, &error));
        QCOMPARE(value, cancel ? QStringLiteral("0") : QStringLiteral("7"));
        QVERIFY(window.runtime().press(QStringLiteral("toggle"), &error));
        QVERIFY(window.runtime().release(true, &error));
    }
}

/******************************************************************************
 * Function Name   : SimulationUiTests::directOwnerDeletionWithHeldViewport
 * Description     : Delete the owner while the real viewport holds a momentary touch and verify child teardown is safe.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationUiTests::directOwnerDeletionWithHeldViewport() //20261114-6 add
{
    SimulationWindow *window = new SimulationWindow();
    QPointer<SimulationViewport> view;
    QString error;

    QVERIFY(window->initialize(simulationTestProject(), &error));
    window->show();
    QApplication::processEvents();
    view = window->findChild<SimulationViewport *>();
    QTest::mousePress(view, Qt::LeftButton, Qt::NoModifier, QPoint(50, 135));
    QVERIFY(view->pointerDown());
    delete window;
    QVERIFY(view.isNull());
}

/******************************************************************************
 * Function Name   : SimulationUiTests::ownerDeletionInsideInputDialog
 * Description     : Destroy a modal input's owner without returning into a dead runtime or viewport callback.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationUiTests::ownerDeletionInsideInputDialog() //20261114-6 add
{
    HmiProject project = simulationTestProject();
    HmiObject word = project.objects.first();
    QPointer<SimulationWindow> window = new SimulationWindow();
    QPointer<SimulationViewport> view;
    QString error;
    bool deleted = false;

    word.id = QStringLiteral("input-word");
    word.name = word.id;
    word.address = QStringLiteral("LW10");
    word.geometry = QRect(400, 20, 150, 60);
    word.hasWordButtonProperties = true;
    word.wordButton.dataType = 0;
    word.wordButton.operation = 1;
    word.wordButton.minimumValue = QStringLiteral("0");
    word.wordButton.maximumValue = QStringLiteral("65535");
    word.bitButton.indicator = false;
    word.bitButton.dualMonitor = false; //20261114-6 change: Keep this input fixture valid before testing modal owner destruction.
    project.objects.append(word);
    QVERIFY(window->initialize(project, &error));
    window->show();
    QApplication::processEvents();
    view = window->findChild<SimulationViewport *>();
    QTimer::singleShot(0, [&window, &deleted]() {
        delete window.data();
        deleted = true;
    });
    QTest::mouseClick(view, Qt::LeftButton, Qt::NoModifier, QPoint(450, 45));
    QVERIFY(deleted);
    QVERIFY(window.isNull());
    QVERIFY(view.isNull());
}

QTEST_MAIN(SimulationUiTests)
#include "simulation_ui_tests.moc"
