#include "plc_address_dialog.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QtTest>
#include <functional>

//20261114-8 add: Real modal widgets exercise the public transactions without device or vendor calls.
class Stage9AddressUiTests : public QObject
{
    Q_OBJECT
private slots:
    void initialInvalidDraft();
    void correctInvalidDraft();
    void radixAndMinorRoundTrip();
    void familyAndMemoryReset();
    void cancelKeepsOriginal();
    void boundary_data();
    void boundary();
    void nativeRadixAndUnknownProfile();
    void connectionsConfirmAndCancel();
    void connectionProfileInvalidatesNode();
    void connectionRemovalAndInvalidInput();
    void ownerDestruction_data();
    void ownerDestruction();
    void unvalidatedAcceptCannotCommit();
};

/******************************************************************************
 * Function Name   : findDialog
 * Description     : Find only a dialog owned by this Qt test process.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
static QDialog *findDialog(const QString &name) //20261114-8 add
{
    QWidget *widget;

    Q_FOREACH (widget, QApplication::topLevelWidgets())
    {
        if (widget->objectName() == name) { return qobject_cast<QDialog *>(widget); }
    }
    return nullptr;
}

/******************************************************************************
 * Function Name   : driveDialog
 * Description     : Run one callback inside the real nested modal loop with an owned watchdog.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
static bool driveDialog(const QString &name, const std::function<void(QDialog *)> &action,
                        const std::function<bool()> &invoke) //20261114-8 add
{
    QTimer watchdog;
    bool result;

    watchdog.setSingleShot(true);
    QObject::connect(&watchdog, &QTimer::timeout, &watchdog, [name]() {
        QDialog *dialog;

        dialog = findDialog(name);
        if (dialog != nullptr) { dialog->reject(); }
    });
    QTimer::singleShot(0, &watchdog, [name, action]() { action(findDialog(name)); });
    watchdog.start(3000);
    result = invoke();
    watchdog.stop();
    return result;
}

/******************************************************************************
 * Function Name   : defaultConnections
 * Description     : Provide an explicit synthetic connection; station and native node remain independent.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
static QVector<PlcConnection> defaultConnections() //20261114-8 add
{
    PlcConnection connection;

    connection.id = QStringLiteral("com1");
    connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
    connection.channel = PlcChannel::Com1;
    connection.station = QStringLiteral("1");
    connection.nodeKnown = true;
    connection.node = 7;
    return {connection};
}

/******************************************************************************
 * Function Name   : connectionsJson
 * Description     : Snapshot complete connection fields to detect unintended transaction writes.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
static QJsonArray connectionsJson(const QVector<PlcConnection> &connections) //20261114-8 add
{
    QJsonArray result;
    PlcConnection connection;

    Q_FOREACH (connection, connections) { result.append(connection.toJson()); }
    return result;
}

/******************************************************************************
 * Function Name   : okButton
 * Description     : Access the actual standard confirmation button by the existing stable object name.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
static QPushButton *okButton(QDialog *dialog, bool connection = false) //20261114-8 add
{
    return dialog->findChild<QDialogButtonBox *>(connection ? QStringLiteral("PlcConnectionButtons") :
        QStringLiteral("PlcAddressButtons"))->button(QDialogButtonBox::Ok);
}

/******************************************************************************
 * Function Name   : Stage9AddressUiTests::initialInvalidDraft
 * Description     : Keep an invalid initial address editable and show the parser failure before any confirmation.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9AddressUiTests::initialInvalidDraft() //20261114-8 add
{
    QString text = QStringLiteral("LW4097");
    bool visibleError = false;
    bool draftKept = false;
    bool invalidDisabled = false;
    bool result;

    result = driveDialog(QStringLiteral("PlcAddressDialog"), [&](QDialog *dialog) {
        if (dialog == nullptr) { return; }
        draftKept = dialog->findChild<QLineEdit *>(QStringLiteral("PlcValue"))->text() == QStringLiteral("4097");
        visibleError = !dialog->findChild<QLabel *>(QStringLiteral("PlcAddressError"))->text().isEmpty();
        invalidDisabled = !okButton(dialog)->isEnabled();
        dialog->reject();
    }, [&]() { return editPlcAddress(nullptr, false, {}, &text); });
    QVERIFY(!result); QCOMPARE(text, QStringLiteral("LW4097"));
    QVERIFY(draftKept); QVERIFY(visibleError); QVERIFY(invalidDisabled);
}

/******************************************************************************
 * Function Name   : Stage9AddressUiTests::correctInvalidDraft
 * Description     : Correct a live invalid draft and commit only its canonical parser-approved value.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9AddressUiTests::correctInvalidDraft() //20261114-8 add
{
    QString text = QStringLiteral("LW0");
    bool errorShown = false;
    bool corrected = false;
    bool result;

    result = driveDialog(QStringLiteral("PlcAddressDialog"), [&](QDialog *dialog) {
        QLineEdit *value;
        QLabel *error;

        if (dialog == nullptr) { return; }
        value = dialog->findChild<QLineEdit *>(QStringLiteral("PlcValue"));
        error = dialog->findChild<QLabel *>(QStringLiteral("PlcAddressError"));
        value->setText(QStringLiteral("4097"));
        errorShown = !error->text().isEmpty() && !okButton(dialog)->isEnabled();
        value->setText(QStringLiteral("0004096"));
        corrected = error->text().isEmpty() && okButton(dialog)->isEnabled();
        okButton(dialog)->click();
    }, [&]() { return editPlcAddress(nullptr, false, {}, &text); });
    QVERIFY(result); QCOMPARE(text, QStringLiteral("LW4096")); QVERIFY(errorShown); QVERIFY(corrected);
}

/******************************************************************************
 * Function Name   : Stage9AddressUiTests::radixAndMinorRoundTrip
 * Description     : Switch the real radix widget without changing the decimal bit component or losing malformed text.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9AddressUiTests::radixAndMinorRoundTrip() //20261114-8 add
{
    QString text = QStringLiteral("@com1:4x_bit12.03");
    QVector<PlcConnection> connections = defaultConnections();
    bool converted = false;
    bool rejectedSwitch = false;
    bool result;

    result = driveDialog(QStringLiteral("PlcAddressDialog"), [&](QDialog *dialog) {
        QComboBox *radix;
        QLineEdit *value;

        if (dialog == nullptr) { return; }
        radix = dialog->findChild<QComboBox *>(QStringLiteral("PlcRadix"));
        value = dialog->findChild<QLineEdit *>(QStringLiteral("PlcValue"));
        radix->setCurrentIndex(1);
        converted = value->text() == QStringLiteral("C.03");
        radix->setCurrentIndex(0);
        value->setText(QStringLiteral("12.16"));
        radix->setCurrentIndex(1);
        rejectedSwitch = radix->currentIndex() == 0 && value->text() == QStringLiteral("12.16");
        value->setText(QStringLiteral("12.15"));
        radix->setCurrentIndex(1);
        converted = converted && value->text() == QStringLiteral("C.15");
        okButton(dialog)->click();
    }, [&]() { return editPlcAddress(nullptr, true, connections, &text); });
    QVERIFY(result); QVERIFY(converted); QVERIFY(rejectedSwitch);
    QCOMPARE(text, QStringLiteral("@com1:4x_bit12.15"));
}

/******************************************************************************
 * Function Name   : Stage9AddressUiTests::familyAndMemoryReset
 * Description     : Lock the existing Lite family/connection reset rule without presenting it as vendor behavior.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9AddressUiTests::familyAndMemoryReset() //20261114-8 add
{
    QString text = QStringLiteral("@com1:4x123");
    QVector<PlcConnection> connections = defaultConnections();
    bool reset = false;

    QVERIFY(!driveDialog(QStringLiteral("PlcAddressDialog"), [&](QDialog *dialog) {
        QComboBox *family;
        QComboBox *memory;
        QLineEdit *value;

        if (dialog == nullptr) { return; }
        family = dialog->findChild<QComboBox *>(QStringLiteral("PlcFamily"));
        memory = dialog->findChild<QComboBox *>(QStringLiteral("PlcMemory"));
        value = dialog->findChild<QLineEdit *>(QStringLiteral("PlcValue"));
        family->setCurrentIndex(family->findData(QStringLiteral("3x")));
        reset = value->text() == QStringLiteral("0");
        memory->setCurrentIndex(memory->findData(QString()));
        reset = reset && family->currentData().toString() == QStringLiteral("LW") && value->text() == QStringLiteral("0");
        dialog->reject();
    }, [&]() { return editPlcAddress(nullptr, false, connections, &text); }));
    QVERIFY(reset); QCOMPARE(text, QStringLiteral("@com1:4x123"));
}

/******************************************************************************
 * Function Name   : Stage9AddressUiTests::cancelKeepsOriginal
 * Description     : Cancel after field changes without canonicalizing or writing the caller's original text.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9AddressUiTests::cancelKeepsOriginal() //20261114-8 add
{
    QString text = QStringLiteral("  lw00015  ");

    QVERIFY(!driveDialog(QStringLiteral("PlcAddressDialog"), [](QDialog *dialog) {
        if (dialog == nullptr) { return; }
        dialog->findChild<QLineEdit *>(QStringLiteral("PlcValue"))->setText(QStringLiteral("31"));
        dialog->reject();
    }, [&]() { return editPlcAddress(nullptr, false, {}, &text); }));
    QCOMPARE(text, QStringLiteral("  lw00015  "));
}

/******************************************************************************
 * Function Name   : Stage9AddressUiTests::boundary_data
 * Description     : Cover actual picker role/count boundaries without inferring a DWORD stride.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9AddressUiTests::boundary_data() //20261114-8 add
{
    QTest::addColumn<QString>("input"); QTest::addColumn<bool>("bit");
    QTest::addColumn<int>("count"); QTest::addColumn<bool>("valid");
    QTest::newRow("local-last-n1") << QStringLiteral("LW4096") << false << 1 << true;
    QTest::newRow("local-last-n2") << QStringLiteral("LW4096") << false << 2 << false;
    QTest::newRow("local-before-last-n2") << QStringLiteral("LW4095") << false << 2 << true;
    QTest::newRow("word-last-n2") << QStringLiteral("@com1:4x65535") << false << 2 << false;
    QTest::newRow("word-before-last-n2") << QStringLiteral("@com1:4x65534") << false << 2 << true;
    QTest::newRow("input-extra-n1") << QStringLiteral("@com1:1x65536") << true << 1 << true;
    QTest::newRow("input-extra-n2") << QStringLiteral("@com1:1x65536") << true << 2 << false;
    QTest::newRow("packed-last-n2") << QStringLiteral("@com1:4x_bit65535.15") << true << 2 << false;
    QTest::newRow("dword-not-implicit-n2") << QStringLiteral("@com1:3x_dw65535") << false << 1 << true;
    QTest::newRow("dword-explicit-n2") << QStringLiteral("@com1:3x_dw65535") << false << 2 << false;
}

/******************************************************************************
 * Function Name   : Stage9AddressUiTests::boundary
 * Description     : Confirm or refuse the real widget draft using the explicit requested transfer count.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9AddressUiTests::boundary() //20261114-8 add
{
    QFETCH(QString, input); QFETCH(bool, bit); QFETCH(int, count); QFETCH(bool, valid);
    QString text = input;
    QVector<PlcConnection> connections = defaultConnections();
    bool stateMatched = false;
    bool result;

    result = driveDialog(QStringLiteral("PlcAddressDialog"), [&](QDialog *dialog) {
        if (dialog == nullptr) { return; }
        stateMatched = okButton(dialog)->isEnabled() == valid;
        if (valid) { okButton(dialog)->click(); } else { dialog->reject(); }
    }, [&]() { return editPlcAddress(nullptr, bit, connections, &text, count); });
    QCOMPARE(result, valid); QVERIFY(stateMatched); QCOMPARE(text, input);
}

/******************************************************************************
 * Function Name   : Stage9AddressUiTests::nativeRadixAndUnknownProfile
 * Description     : Retain verified native grammar and refuse unknown connection profiles in the real picker.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9AddressUiTests::nativeRadixAndUnknownProfile() //20261114-8 add
{
    QVector<PlcConnection> connections = defaultConnections();
    QString text = QStringLiteral("@com1:X17");
    bool nativeOnly = false;
    bool refused = false;

    connections[0].profileId = QStringLiteral("Link/serial_amz/MITSUBISHI/Mitsubishi_Fx.ini");
    QVERIFY(driveDialog(QStringLiteral("PlcAddressDialog"), [&](QDialog *dialog) {
        if (dialog == nullptr) { return; }
        nativeOnly = !dialog->findChild<QComboBox *>(QStringLiteral("PlcRadix"))->isEnabled();
        okButton(dialog)->click();
    }, [&]() { return editPlcAddress(nullptr, true, connections, &text); }));
    QVERIFY(nativeOnly); QCOMPARE(text, QStringLiteral("@com1:X17"));
    connections[0].profileId = QStringLiteral("missing-profile");
    text = QStringLiteral("@com1:4x12");
    QVERIFY(!driveDialog(QStringLiteral("PlcAddressDialog"), [&](QDialog *dialog) {
        if (dialog == nullptr) { return; }
        refused = !okButton(dialog)->isEnabled();
        dialog->reject();
    }, [&]() { return editPlcAddress(nullptr, false, connections, &text); }));
    QVERIFY(refused); QCOMPARE(text, QStringLiteral("@com1:4x12"));
}

/******************************************************************************
 * Function Name   : Stage9AddressUiTests::connectionsConfirmAndCancel
 * Description     : Validate station edits transactionally while keeping the independent verified source node.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9AddressUiTests::connectionsConfirmAndCancel() //20261114-8 add
{
    QVector<PlcConnection> connections = defaultConnections();
    QJsonArray before = connectionsJson(connections);
    bool invalidKept = false;

    QVERIFY(driveDialog(QStringLiteral("PlcConnectionDialog"), [&](QDialog *dialog) {
        QLineEdit *station;

        if (dialog == nullptr) { return; }
        station = dialog->findChild<QLineEdit *>(QStringLiteral("PlcCom1Station"));
        station->setText(QStringLiteral("-1")); okButton(dialog, true)->click();
        invalidKept = connectionsJson(connections) == before && !dialog->findChild<QLabel *>(QStringLiteral("PlcConnectionError"))->text().isEmpty();
        station->setText(QStringLiteral("9")); okButton(dialog, true)->click();
    }, [&]() { return editPlcConnections(nullptr, QStringLiteral("AMT070W124-L800X480R"), &connections); }));
    QVERIFY(invalidKept); QCOMPARE(connections[0].station, QStringLiteral("9"));
    QVERIFY(connections[0].nodeKnown); QCOMPARE(connections[0].node, u8(7));
    before = connectionsJson(connections);
    QVERIFY(!driveDialog(QStringLiteral("PlcConnectionDialog"), [](QDialog *dialog) {
        if (dialog == nullptr) { return; }
        dialog->findChild<QCheckBox *>(QStringLiteral("PlcCom1Enabled"))->setChecked(false); dialog->reject();
    }, [&]() { return editPlcConnections(nullptr, QStringLiteral("AMT070W124-L800X480R"), &connections); }));
    QCOMPARE(connectionsJson(connections), before);
}

/******************************************************************************
 * Function Name   : Stage9AddressUiTests::connectionProfileInvalidatesNode
 * Description     : Change an available profile without carrying a source node into a different protocol.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9AddressUiTests::connectionProfileInvalidatesNode() //20261114-8 add
{
    QVector<PlcConnection> connections = defaultConnections();
    QString selectedProfile;

    QVERIFY(driveDialog(QStringLiteral("PlcConnectionDialog"), [&](QDialog *dialog) {
        QComboBox *profile;

        if (dialog == nullptr) { return; }
        profile = dialog->findChild<QComboBox *>(QStringLiteral("PlcCom1Profile"));
        profile->setCurrentIndex(profile->currentIndex() == 0 ? 1 : 0);
        selectedProfile = profile->currentData().toString();
        dialog->findChild<QLineEdit *>(QStringLiteral("PlcCom1Station"))->setText(QStringLiteral("2"));
        okButton(dialog, true)->click();
    }, [&]() { return editPlcConnections(nullptr, QStringLiteral("AMT070W124-L800X480R"), &connections); }));
    QCOMPARE(connections[0].profileId, selectedProfile); QVERIFY(!connections[0].nodeKnown); QCOMPARE(connections[0].node, u8(0));
}

/******************************************************************************
 * Function Name   : Stage9AddressUiTests::connectionRemovalAndInvalidInput
 * Description     : Commit explicit connection removal and reject malformed port identity before a dialog opens.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9AddressUiTests::connectionRemovalAndInvalidInput() //20261114-8 add
{
    QVector<PlcConnection> connections = defaultConnections();
    QJsonArray before;
    QString error;

    QVERIFY(driveDialog(QStringLiteral("PlcConnectionDialog"), [](QDialog *dialog) {
        if (dialog == nullptr) { return; }
        dialog->findChild<QCheckBox *>(QStringLiteral("PlcCom1Enabled"))->setChecked(false);
        okButton(dialog, true)->click();
    }, [&]() { return editPlcConnections(nullptr, QStringLiteral("AMT070W124-L800X480R"), &connections); }));
    QVERIFY(connections.isEmpty());
    connections = defaultConnections(); connections[0].id = QStringLiteral("com2"); before = connectionsJson(connections);
    QVERIFY(!editPlcConnections(nullptr, QStringLiteral("AMT070W124-L800X480R"), &connections, &error));
    QCOMPARE(connectionsJson(connections), before); QVERIFY(!error.isEmpty());
}

/******************************************************************************
 * Function Name   : Stage9AddressUiTests::ownerDestruction_data
 * Description     : Destroy the real modal owner before or synchronously during confirmation.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9AddressUiTests::ownerDestruction_data() //20261114-8 add
{
    QTest::addColumn<bool>("connection"); QTest::addColumn<bool>("duringClick");
    QTest::newRow("address-before") << false << false;
    QTest::newRow("address-confirm-click") << false << true;
    QTest::newRow("connections-before") << true << false;
    QTest::newRow("connections-confirm-click") << true << true;
}

/******************************************************************************
 * Function Name   : Stage9AddressUiTests::ownerDestruction
 * Description     : Refuse publication and survive owner deletion inside the modal event loop.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9AddressUiTests::ownerDestruction() //20261114-8 add
{
    QFETCH(bool, connection); QFETCH(bool, duringClick);
    QPointer<QWidget> owner = new QWidget;
    QVector<PlcConnection> connections = defaultConnections();
    QJsonArray before = connectionsJson(connections);
    QString text = QStringLiteral("LW0015");
    bool result;

    result = driveDialog(connection ? QStringLiteral("PlcConnectionDialog") : QStringLiteral("PlcAddressDialog"),
        [&](QDialog *dialog) {
            QPushButton *button;

            if (dialog == nullptr) { return; }
            if (!duringClick) { delete owner.data(); return; }
            button = okButton(dialog, connection);
            QObject::connect(button, &QPushButton::clicked, qApp, [&]() { delete owner.data(); });
            button->click();
        }, [&]() {
            return connection ? editPlcConnections(owner.data(), QStringLiteral("AMT070W124-L800X480R"), &connections) :
                editPlcAddress(owner.data(), false, connections, &text);
        });
    QVERIFY(owner.isNull()); QVERIFY(!result); QCOMPARE(text, QStringLiteral("LW0015"));
    QCOMPARE(connectionsJson(connections), before);
}

/******************************************************************************
 * Function Name   : Stage9AddressUiTests::unvalidatedAcceptCannotCommit
 * Description     : Do not publish an empty default result when external code bypasses the validation button.
 * Input Parameters: See signature.
 * Return Value    : See signature.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage9AddressUiTests::unvalidatedAcceptCannotCommit() //20261114-8 add
{
    QString text = QStringLiteral("LW0015");
    QVector<PlcConnection> connections = defaultConnections();
    QJsonArray before = connectionsJson(connections);

    QVERIFY(!driveDialog(QStringLiteral("PlcAddressDialog"), [](QDialog *dialog) {
        if (dialog != nullptr) { dialog->accept(); }
    }, [&]() { return editPlcAddress(nullptr, false, connections, &text); }));
    QCOMPARE(text, QStringLiteral("LW0015"));
    QVERIFY(!driveDialog(QStringLiteral("PlcConnectionDialog"), [](QDialog *dialog) {
        if (dialog != nullptr) { dialog->accept(); }
    }, [&]() { return editPlcConnections(nullptr, QStringLiteral("AMT070W124-L800X480R"), &connections); }));
    QCOMPARE(connectionsJson(connections), before);
}

QTEST_MAIN(Stage9AddressUiTests)
#include "stage9_address_ui_tests.moc"

