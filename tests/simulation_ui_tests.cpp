#include "main_window.h"
#include "simulation_window.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QFocusEvent>
#include <QJsonDocument>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>
#include <QTimer>
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
    void onlineScopeHasNoConnectAction();
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
 * Function Name   : SimulationUiTests::onlineScopeHasNoConnectAction
 * Description     : Explain PC transport limits with no enabled port-connect path or project mutation.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationUiTests::onlineScopeHasNoConnectAction() //20261114-6 add
{
    MainWindow window;
    QJsonObject before;
    bool explained = false;

    window.setProject(simulationTestProject(), QString());
    before = window.currentEditorProject().toJson();
    QTimer::singleShot(0, &window, [&explained]() {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());

        if (dialog != nullptr && dialog->objectName() == QStringLiteral("onlineSimulationScope"))
        {
            explained = dialog->findChild<QLabel *>()->text().contains(QStringLiteral("不会连接PLC"));
            dialog->reject();
        }
    });
    window.m_commands.value(QStringLiteral("action_online_simulation"))->trigger();
    QVERIFY(explained);
    QCOMPARE(window.currentEditorProject().toJson(), before);
    QVERIFY(!window.m_dirty);
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
