#include "main_window.h"
#include "hmi_canvas.h"
#include "message_display_dialog.h"
#include "hmi_property_widgets.h"
#include "native_project_codec.h"
#include "project_capabilities.h"
#include "stage3_project_edits.h"
#include <QAction>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTableWidget>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>

//20261112-0 add: Exercise real control dialogs, persisted values and whole-document transactions.
class Stage4ControlTests : public QObject
{
    Q_OBJECT
private:
    QTemporaryDir m_settings;
private slots:
    void initTestCase();
    void messageDefaultsAndNativeBounds();
    void messageLanguageAndHiddenStateDrafts();
    void messageLanguageAppearanceAndCompactProperties();
    void messagePermissionsRetainDisabledValues();
    void messageTypedRoundTripAndInvalidFields();
    void messageAddressSpanIsValidated();
    void messageCanvasAndPreviewShareRenderer();
    void messageCopyAndMixedPersistence();
    void messageCreationAndCancellationAreAtomic();
    void messageUnknownDeviceAndSourceAreRejected();
};

/******************************************************************************
 * Function Name   : messageObject
 * Description     : Provide a bounded typed message object, independent of the old static-text or lamp subtype.
 * Input Parameters: None
 * Return Value    : Default message object.
 * Date            : 2026.11.12
 ******************************************************************************/
static HmiObject messageObject() //20261112-0 add
{
    HmiObject object;
    object.id = QStringLiteral("stage4-message"); object.type = QStringLiteral("message");
    object.name = QStringLiteral("消息显示器"); object.text = QStringLiteral("0"); object.address = QStringLiteral("LW0");
    object.geometry = QRect(20, 20, 200, 110);
    return object;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::initTestCase
 * Description     : Keep test preferences away from the user's editor configuration and recent files.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::initTestCase() //20261112-0 add
{
    QVERIFY(m_settings.isValid()); QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_settings.path());
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::messageDefaultsAndNativeBounds
 * Description     : Check fresh native observations, including the state minimum that contradicts the old help.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::messageDefaultsAndNativeBounds() //20261112-0 add
{
    MessageDisplayDialog dialog(messageObject(), HmiProject());
    QSpinBox *count;
    QSpinBox *speed;
    QComboBox *movement;
    QTableWidget *table;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    QCOMPARE(dialog.windowTitle(), QStringLiteral("消息显示器"));
    count = dialog.findChild<QSpinBox *>(QStringLiteral("MessageStateCount"));
    speed = dialog.findChild<QSpinBox *>(QStringLiteral("MessageSpeed"));
    movement = dialog.findChild<QComboBox *>(QStringLiteral("MessageMovement"));
    table = dialog.findChild<QTableWidget *>(QStringLiteral("MessageStates"));
    QCOMPARE(count->value(), 2); QCOMPARE(count->minimum(), 1); QCOMPARE(count->maximum(), 128);
    QCOMPARE(speed->value(), 5); QCOMPARE(speed->minimum(), 1); QCOMPARE(speed->maximum(), 10); QVERIFY(!speed->isVisible());
    QCOMPARE(table->rowCount(), 2); QCOMPARE(table->item(1, 1)->text(), QStringLiteral("1"));
    movement->setCurrentIndex(1); QVERIFY(speed->isVisible()); speed->setValue(8);
    movement->setCurrentIndex(0); QVERIFY(!speed->isVisible()); QCOMPARE(dialog.object().messageDisplay.speed, 8);
    count->setValue(1); QCOMPARE(table->rowCount(), 1); count->setValue(128); QCOMPARE(table->rowCount(), 128);
    QCOMPARE(table->item(127, 1)->text(), QStringLiteral("127"));
    count->setValue(2); movement->setCurrentIndex(0);
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR"))
    {
        QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("message-general.png"))));
        dialog.findChild<QTabWidget *>(QStringLiteral("MessageTabs"))->setCurrentIndex(1);
        QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("message-permissions.png"))));
    }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::messageLanguageAppearanceAndCompactProperties
 * Description     : Keep every language's size/style independent and route compact property edits into typed message fields.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::messageLanguageAppearanceAndCompactProperties() //20261112-0 add
{
    HmiProject project;
    HmiObject object = messageObject();
    MessageDisplayDialog *dialog;
    QComboBox *language;
    QComboBox *size;
    QComboBox *style;
    MainWindow window;
    project.languages = {QStringLiteral("中文"), QStringLiteral("English")};
    dialog = new MessageDisplayDialog(object, project);
    language = dialog->findChild<QComboBox *>(QStringLiteral("MessageLanguage")); size = dialog->findChild<QComboBox *>(QStringLiteral("MessageFontSize")); style = dialog->findChild<QComboBox *>(QStringLiteral("MessageStyle"));
    language->setCurrentText(QStringLiteral("English")); size->setCurrentText(QStringLiteral("24")); style->setCurrentIndex(2);
    language->setCurrentText(QStringLiteral("中文")); QCOMPARE(size->currentText(), QStringLiteral("16")); QCOMPARE(style->currentIndex(), 1);
    language->setCurrentText(QStringLiteral("English")); QCOMPARE(size->currentText(), QStringLiteral("24")); QCOMPARE(style->currentIndex(), 2);
    object = dialog->object(); delete dialog; project.objects.append(object); window.setProject(project, QString()); window.m_canvas->selectObject(object.id);
    window.m_textEdit->setPlainText(QStringLiteral("快捷编辑")); window.m_addressEdit->setText(QStringLiteral("LW8")); window.applySelectedProperties();
    object = window.currentEditorProject().objects.first(); QCOMPARE(object.messageDisplay.text(0, QStringLiteral("中文")), QStringLiteral("快捷编辑"));
    QCOMPARE(object.messageDisplay.monitorAddress, QStringLiteral("LW8")); QCOMPARE(object.messageDisplay.languageAppearance(QStringLiteral("English")).fontSize, 24.0);
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::messageLanguageAndHiddenStateDrafts
 * Description     : Verify per-state translations, replacement selection and temporary count changes without live mutations.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::messageLanguageAndHiddenStateDrafts() //20261112-0 add
{
    HmiProject project;
    HmiObject original = messageObject();
    MessageDisplayDialog *dialog;
    QSpinBox *count;
    QTableWidget *table;
    QPlainTextEdit *text;
    QComboBox *language;
    project.languages = {QStringLiteral("中文"), QStringLiteral("English")};
    dialog = new MessageDisplayDialog(original, project);
    count = dialog->findChild<QSpinBox *>(QStringLiteral("MessageStateCount")); table = dialog->findChild<QTableWidget *>(QStringLiteral("MessageStates"));
    text = dialog->findChild<QPlainTextEdit *>(QStringLiteral("MessageTextEditor")); language = dialog->findChild<QComboBox *>(QStringLiteral("MessageLanguage"));
    count->setValue(3); table->setCurrentCell(2, 0); QCOMPARE(text->textCursor().selectedText(), QStringLiteral("2"));
    text->setPlainText(QStringLiteral("中文消息")); language->setCurrentText(QStringLiteral("English")); QCOMPARE(text->toPlainText(), QStringLiteral("2"));
    text->setPlainText(QStringLiteral("English message")); language->setCurrentText(QStringLiteral("中文")); QCOMPARE(text->toPlainText(), QStringLiteral("中文消息"));
    QCOMPARE(dialog->object().messageDisplay.text(2, QStringLiteral("English")), QStringLiteral("English message"));
    count->setValue(1); count->setValue(3); table->setCurrentCell(2, 0); QCOMPARE(text->toPlainText(), QStringLiteral("2"));
    dialog->reject(); QCOMPARE(original.messageDisplay.stateCount, 2); QCOMPARE(original.messageDisplay.states.size(), 2); delete dialog;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::messagePermissionsRetainDisabledValues
 * Description     : Exercise visibility-only hierarchy and equality tolerance while retaining disabled drafts.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::messagePermissionsRetainDisabledValues() //20261112-0 add
{
    MessageDisplayDialog dialog(messageObject(), HmiProject());
    HmiPermissionEditor *permission;
    QGroupBox *word;
    QLineEdit *address;
    QComboBox *comparison;
    QLineEdit *tolerance;
    dialog.show();
    permission = dialog.findChild<HmiPermissionEditor *>(QStringLiteral("MessageVisibility"));
    word = dialog.findChild<QGroupBox *>(QStringLiteral("MessageVisibilityWord")); address = dialog.findChild<QLineEdit *>(QStringLiteral("MessageVisibilityWordAddress"));
    comparison = dialog.findChild<QComboBox *>(QStringLiteral("MessageVisibilityComparison")); tolerance = dialog.findChild<QLineEdit *>(QStringLiteral("MessageVisibilityTolerance"));
    QVERIFY(!permission->isChecked()); QVERIFY(!word->isEnabled());
    permission->setChecked(true); word->setChecked(true); QVERIFY(address->isEnabled()); address->setText(QStringLiteral("LW8"));
    comparison->setCurrentIndex(2); QVERIFY(!tolerance->isHidden()); tolerance->setText(QStringLiteral("2"));
    comparison->setCurrentIndex(0); QVERIFY(tolerance->isHidden()); permission->setChecked(false);
    QCOMPARE(dialog.object().messageDisplay.appearance.wordAddress, QStringLiteral("LW8")); QCOMPARE(dialog.object().messageDisplay.appearance.tolerance, QStringLiteral("2"));
    QVERIFY(dialog.findChild<QGroupBox *>(QStringLiteral("MessageTouch")) == nullptr);
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::messageTypedRoundTripAndInvalidFields
 * Description     : Reopen real KSP data and reject unknown, fractional, malformed and mismatched message records atomically.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::messageTypedRoundTripAndInvalidFields() //20261112-0 add
{
    HmiProject project;
    HmiProject loaded;
    HmiObject object = messageObject();
    QTemporaryDir directory;
    QJsonObject json;
    QJsonObject properties;
    QString error;
    object.messageDisplay.stateCount = 128; object.messageDisplay.ensureStates(128);
    object.messageDisplay.states[127].insert(QStringLiteral("中文"), QStringLiteral("边界消息")); project.objects.append(object);
    QVERIFY2(project.save(directory.filePath(QStringLiteral("message.ksp")), &error), qPrintable(error));
    QVERIFY2(HmiProject::load(directory.filePath(QStringLiteral("message.ksp")), &loaded, &error), qPrintable(error));
    QCOMPARE(loaded.toJson(), project.toJson()); QCOMPARE(project.toJson().value(QStringLiteral("schemaVersion")).toInt(), 11);
    json = object.toJson(); properties = json.value(QStringLiteral("messageDisplay")).toObject(); properties.insert(QStringLiteral("speed"), 1.5); json.insert(QStringLiteral("messageDisplay"), properties);
    QVERIFY(!HmiObject::fromJson(json, &object, &error)); QCOMPARE(object.messageDisplay.speed, 5);
    json = project.objects.first().toJson(); json.insert(QStringLiteral("unknownActiveField"), true); QVERIFY(!HmiObject::fromJson(json, &object, &error));
    json = project.objects.first().toJson(); json.insert(QStringLiteral("type"), QStringLiteral("text")); QVERIFY(!HmiObject::fromJson(json, &object, &error));
    json = project.objects.first().toJson(); json.remove(QStringLiteral("messageDisplay")); QVERIFY(!HmiObject::fromJson(json, &object, &error));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::messageAddressSpanIsValidated
 * Description     : Enforce the monitor's U16/U32 tail address without normalizing qualified or dormant permission addresses.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::messageAddressSpanIsValidated() //20261112-0 add
{
    MessageDisplayProperties properties;
    QString error;
    properties.monitorAddress = QStringLiteral("LW4096"); QVERIFY(properties.validateAddresses({}, &error));
    properties.dataType = 1; QVERIFY(!properties.validateAddresses({}, &error));
    properties.monitorAddress = QStringLiteral("LW4095"); QVERIFY(properties.validateAddresses({}, &error));
    properties.monitorAddress = QStringLiteral("LB0"); QVERIFY(!properties.validateAddresses({}, &error));
    properties.monitorAddress = QStringLiteral("@missing:LW0"); QVERIFY(!properties.validateAddresses({}, &error));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::messageCanvasAndPreviewShareRenderer
 * Description     : Check clipped state and language painting without treating host pixels as native or device equivalence.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::messageCanvasAndPreviewShareRenderer() //20261112-0 add
{
    MessageDisplayProperties properties;
    QImage image(202, 112, QImage::Format_ARGB32);
    QImage before;
    QPainter painter;
    properties.states[0].insert(QStringLiteral("中文"), QStringLiteral("0")); properties.states[1].insert(QStringLiteral("中文"), QStringLiteral("报警"));
    image.fill(Qt::magenta); painter.begin(&image); paintMessageDisplay(&painter, QRectF(1, 1, 200, 110), properties, QStringLiteral("中文"), 0); painter.end(); before = image;
    QCOMPARE(image.pixelColor(0, 0), QColor(Qt::magenta)); QCOMPARE(image.pixelColor(10, 10), QColor(Qt::white));
    painter.begin(&image); paintMessageDisplay(&painter, QRectF(1, 1, 200, 110), properties, QStringLiteral("中文"), 1); painter.end(); QVERIFY(image != before);
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::messageCopyAndMixedPersistence
 * Description     : Mix a new message with an established static text, copy it and reopen without changing the old object.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::messageCopyAndMixedPersistence() //20261112-0 add
{
    HmiProject project;
    HmiProject copied;
    HmiProject loaded;
    HmiObject old;
    Stage3Clipboard clipboard;
    QString error;
    old.id = QStringLiteral("old-static"); old.type = QStringLiteral("text"); old.name = QStringLiteral("已有文本"); old.text = QStringLiteral("保持"); old.geometry = QRect(300, 20, 100, 40); old.hasStaticTextProperties = true;
    project.objects = {old, messageObject()};
    QVERIFY2(captureStage3Selection(project, {QStringLiteral("stage4-message")}, &clipboard, &error), qPrintable(error));
    QVERIFY2(pasteStage3Selection(project, clipboard, QPoint(0, 150), &copied, nullptr, &error), qPrintable(error));
    QCOMPARE(copied.objects.size(), 3); QCOMPARE(copied.objects.first().toJson(), old.toJson());
    QCOMPARE(copied.objects.last().messageDisplay.toJson(), project.objects.last().messageDisplay.toJson());
    QVERIFY2(HmiProject::fromJson(copied.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.toJson(), copied.toJson());
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR"))
    { QVERIFY2(loaded.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("message-mixed.ksp")), &error), qPrintable(error)); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::messageCreationAndCancellationAreAtomic
 * Description     : Trigger the real toolbox action, cancel a draft, then place once and undo/redo the complete object.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::messageCreationAndCancellationAreAtomic() //20261112-0 add
{
    MainWindow window;
    HmiProject empty;
    int history;
    window.setProject(empty, QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window));
    history = window.m_projectHistory.size();
    QTimer::singleShot(0, &window, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } });
    window.m_commands.value(QStringLiteral("action_messagedisplay"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement()); QCOMPARE(window.m_projectHistory.size(), history);
    QTimer::singleShot(0, &window, []() {
        MessageDisplayDialog *dialog;
        dialog = qobject_cast<MessageDisplayDialog *>(QApplication::activeModalWidget());
        if (dialog != nullptr) { dialog->findChild<QDialogButtonBox *>(QStringLiteral("MessageButtons"))->button(QDialogButtonBox::Ok)->click(); }
    });
    window.m_commands.value(QStringLiteral("action_messagedisplay"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QVERIFY(window.currentEditorProject().objects.isEmpty());
    QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20)));
    QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(220, 130)));
    QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(220, 130)));
    QCOMPARE(window.currentEditorProject().objects.size(), 1); QCOMPARE(window.currentEditorProject().objects.first().type, QStringLiteral("message"));
    QCOMPARE(window.m_projectHistory.size(), history + 1); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty());
    window.restoreProjectHistory(1); QCOMPARE(window.currentEditorProject().objects.size(), 1);
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::messageUnknownDeviceAndSourceAreRejected
 * Description     : Ensure editor support never bypasses the unrecovered source or device backend gate.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::messageUnknownDeviceAndSourceAreRejected() //20261112-0 add
{
    HmiProject project;
    QByteArray source = QByteArrayLiteral("unchanged");
    QString error;
    QJsonObject capabilities;
    project.objects.append(messageObject()); capabilities = describeProjectCapabilities(project);
    QVERIFY(capabilities.value(QStringLiteral("modelPersistence")).toObject().value(QStringLiteral("allowed")).toBool());
    QCOMPARE(capabilities.value(QStringLiteral("nativeSource")).toObject().value(QStringLiteral("status")).toString(), QStringLiteral("blocked"));
    QCOMPARE(capabilities.value(QStringLiteral("deviceCompile")).toObject().value(QStringLiteral("status")).toString(), QStringLiteral("blocked"));
    QVERIFY(!exportNativeProjectSource(project, &source, &error)); QCOMPARE(source, QByteArrayLiteral("unchanged"));
}

QTEST_MAIN(Stage4ControlTests)
#include "stage4_controls_tests.moc"
