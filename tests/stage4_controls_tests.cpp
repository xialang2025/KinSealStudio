#include "main_window.h"
#include "hmi_canvas.h"
#include "message_display_dialog.h"
#include "scale_dialog.h"
#include "qr_code_dialog.h" //20261112-0 add
#include "image_display_dialog.h" //20261112-0 add
#include "stage3_resources.h"
#include <QBuffer>
#include <QListWidget>
#include "hmi_property_widgets.h"
#include "native_project_codec.h"
#include "project_capabilities.h"
#include "independent_downloader.h"
#include "stage3_project_edits.h"
#include <QAction>
#include <QComboBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QMessageBox>
#include <QRadioButton>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTableWidget>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <limits>

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
    void scaleNativeBoundsAndDirectionLinkage();
    void scaleRangeErrorsAndEqualLimits();
    void scaleRenderingCountsAndClipping();
    void scaleTypedRoundTripMixedAndCreation();
    void qrDefaultsAndPermissionBranches();
    void qrDescriptionRasterAndCapacity();
    void qrStrictPersistenceAndBackendGates();
    void qrCopyAndCompactProperties();
    void qrCreationEditCancellationAndHistory();
    void imageDefaultsConditionsAndStateLifetime();
    void imageBatchOrderDeletionAndGalleryCancel();
    void imageResourceBindingsAndCopyCollisions();
    void imagePersistenceLegacyAndBackendGates();
    void imageActualRasterAndPlacementTransaction();
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
    QVector<QByteArray> payloads = {QByteArrayLiteral("unchanged")};
    QString error;
    QJsonObject capabilities;
    project.objects.append(messageObject()); capabilities = describeProjectCapabilities(project);
    QVERIFY(capabilities.value(QStringLiteral("modelPersistence")).toObject().value(QStringLiteral("allowed")).toBool());
    QCOMPARE(capabilities.value(QStringLiteral("nativeSource")).toObject().value(QStringLiteral("status")).toString(), QStringLiteral("blocked"));
    QCOMPARE(capabilities.value(QStringLiteral("deviceCompile")).toObject().value(QStringLiteral("status")).toString(), QStringLiteral("blocked"));
    QVERIFY(!exportNativeProjectSource(project, &source, &error)); QCOMPARE(source, QByteArrayLiteral("unchanged"));
    QVERIFY(!buildProjectPayload(project, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("unchanged")}); QVERIFY(!error.isEmpty());
}

/******************************************************************************
 * Function Name   : scaleObject
 * Description     : Provide a bounded scale record without borrowing a dynamic control subtype.
 * Input Parameters: None
 * Return Value    : Independent scale object.
 * Date            : 2026.11.12
 ******************************************************************************/
static HmiObject scaleObject() //20261112-0 add
{
    HmiObject object;
    object.id = QStringLiteral("stage4-scale"); object.type = QStringLiteral("scale"); object.name = QStringLiteral("刻度"); object.geometry = QRect(300, 50, 160, 110);
    return object;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::scaleNativeBoundsAndDirectionLinkage
 * Description     : Validate the native hard bounds and positional up/down versus left/right labels without permission pages.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::scaleNativeBoundsAndDirectionLinkage() //20261112-0 add
{
    ScaleDialog dialog(scaleObject());
    QComboBox *font;
    QComboBox *direction;
    QSpinBox *major;
    QSpinBox *minor;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    font = dialog.findChild<QComboBox *>(QStringLiteral("ScaleFontSize")); direction = dialog.findChild<QComboBox *>(QStringLiteral("ScaleDirection"));
    major = dialog.findChild<QSpinBox *>(QStringLiteral("ScaleMajorCount")); minor = dialog.findChild<QSpinBox *>(QStringLiteral("ScaleMinorCount"));
    QCOMPARE(font->count(), 16); QCOMPARE(font->itemText(0), QStringLiteral("5")); QCOMPARE(font->itemText(15), QStringLiteral("20")); QCOMPARE(font->currentText(), QStringLiteral("16"));
    QCOMPARE(major->minimum(), 2); QCOMPARE(major->maximum(), 20); QCOMPARE(major->value(), 4); QCOMPARE(minor->minimum(), 0); QCOMPARE(minor->maximum(), 5); QCOMPARE(minor->value(), 1);
    QCOMPARE(direction->itemText(0), QStringLiteral("向上")); direction->setCurrentIndex(1);
    dialog.findChild<QRadioButton *>(QStringLiteral("ScalePosition0"))->click(); QCOMPARE(direction->itemText(0), QStringLiteral("向左")); QCOMPARE(direction->currentIndex(), 1);
    dialog.findChild<QRadioButton *>(QStringLiteral("ScalePosition3"))->click(); QCOMPARE(direction->itemText(1), QStringLiteral("向下"));
    QCOMPARE(dialog.findChild<QTabWidget *>()->count(), 1); QVERIFY(dialog.findChild<HmiPermissionEditor *>() == nullptr);
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR"))
    { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("scale-general.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::scaleRangeErrorsAndEqualLimits
 * Description     : Reproduce the native invalid-range message and retain the dialog while accepting equal endpoints.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::scaleRangeErrorsAndEqualLimits() //20261112-0 add
{
    ScaleDialog dialog(scaleObject());
    QLineEdit *minimum;
    QLineEdit *maximum;
    bool explained = false;
    dialog.show(); minimum = dialog.findChild<QLineEdit *>(QStringLiteral("ScaleMinimum")); maximum = dialog.findChild<QLineEdit *>(QStringLiteral("ScaleMaximum"));
    minimum->setText(QStringLiteral("101"));
    QTimer::singleShot(0, &dialog, [&explained]() {
        QMessageBox *message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (message != nullptr) { explained = message->text() == QStringLiteral("最小值不能大于最大值"); message->accept(); }
    });
    dialog.findChild<QDialogButtonBox *>(QStringLiteral("ScaleButtons"))->button(QDialogButtonBox::Ok)->click(); QVERIFY(explained); QVERIFY(dialog.isVisible());
    minimum->setText(QStringLiteral("0")); maximum->setText(QStringLiteral("0"));
    dialog.findChild<QDialogButtonBox *>(QStringLiteral("ScaleButtons"))->button(QDialogButtonBox::Ok)->click(); QCOMPARE(dialog.result(), int(QDialog::Accepted)); QCOMPARE(dialog.object().scale.maximum, 0);
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::scaleRenderingCountsAndClipping
 * Description     : Count major/minor strokes in the real raster and check clipping, orientation, equal limits and tiny geometry.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::scaleRenderingCountsAndClipping() //20261112-0 add
{
    ScaleProperties properties;
    QImage image(150, 171, QImage::Format_ARGB32);
    QImage other;
    QPainter painter;
    int y;
    int major = 0;
    int all = 0;
    properties.showValues = false; properties.minorCount = 4;
    image.fill(Qt::white); painter.begin(&image); paintScale(&painter, QRectF(40, 30, 100, 116), properties); painter.end();
    for (y = 0; y < image.height(); ++y)
    {
        if (image.pixelColor(65, y) == QColor(Qt::black)) { ++major; }
        if (image.pixelColor(47, y) == QColor(Qt::black)) { ++all; }
    }
    QCOMPARE(major, 4); QCOMPARE(all, 16); QCOMPARE(image.pixelColor(0, 0), QColor(Qt::white));
    other = image; properties.position = 0; properties.direction = 1; properties.minimum = -100; properties.maximum = 100;
    image.fill(Qt::white); painter.begin(&image); paintScale(&painter, QRectF(10, 10, 120, 60), properties); painter.end(); QVERIFY(image != other);
    properties.minimum = properties.maximum; painter.begin(&image); paintScale(&painter, QRectF(1, 1, 1, 1), properties); painter.end();
    QCOMPARE(image.pixelColor(0, 0), QColor(Qt::white));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::scaleTypedRoundTripMixedAndCreation
 * Description     : Preserve all fields in mixed KSP, reject unknown types, create through the real command and undo atomically.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::scaleTypedRoundTripMixedAndCreation() //20261112-0 add
{
    HmiProject project;
    HmiProject loaded;
    HmiObject object = scaleObject();
    HmiObject parsed;
    QJsonObject json;
    QJsonObject properties;
    QByteArray source = QByteArrayLiteral("unchanged");
    QVector<QByteArray> payloads = {QByteArrayLiteral("unchanged")};
    QString error;
    MainWindow window;
    object.scale.minimum = std::numeric_limits<int>::min(); object.scale.maximum = std::numeric_limits<int>::max(); object.scale.decimals = 3; object.scale.position = 1; object.scale.direction = 1;
    project.objects = {messageObject(), object};
    QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.toJson(), project.toJson());
    json = object.toJson(); properties = json.value(QStringLiteral("scale")).toObject(); properties.insert(QStringLiteral("majorCount"), 4.5); json.insert(QStringLiteral("scale"), properties); QVERIFY(!HmiObject::fromJson(json, &parsed, &error));
    json = object.toJson(); json.insert(QStringLiteral("type"), QStringLiteral("text")); QVERIFY(!HmiObject::fromJson(json, &parsed, &error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY2(project.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("scale-mixed.ksp")), &error), qPrintable(error)); }
    project.objects.clear(); window.setProject(project, QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window));
    QTimer::singleShot(0, &window, []() {
        ScaleDialog *dialog = qobject_cast<ScaleDialog *>(QApplication::activeModalWidget());
        if (dialog != nullptr) { dialog->findChild<QDialogButtonBox *>(QStringLiteral("ScaleButtons"))->button(QDialogButtonBox::Ok)->click(); }
    });
    window.m_commands.value(QStringLiteral("action_mark"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement());
    QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20)));
    QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(180, 130)));
    QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(180, 130)));
    QCOMPARE(window.currentEditorProject().objects.size(), 1); QCOMPARE(window.currentEditorProject().objects.first().type, QStringLiteral("scale"));
    window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty()); window.restoreProjectHistory(1); QCOMPARE(window.currentEditorProject().objects.size(), 1);
    QCOMPARE(describeProjectCapabilities(window.currentEditorProject()).value(QStringLiteral("deviceCompile")).toObject().value(QStringLiteral("status")).toString(), QStringLiteral("blocked"));
    QVERIFY(!buildProjectPayload(window.currentEditorProject(), &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("unchanged")});
    QVERIFY(!exportNativeProjectSource(window.currentEditorProject(), &source, &error)); QCOMPARE(source, QByteArrayLiteral("unchanged"));
}

/******************************************************************************
 * Function Name   : qrObject
 * Description     : Provide a new independent QR family with the measured native editing defaults.
 * Input Parameters: None
 * Return Value    : Typed QR object.
 * Date            : 2026.11.12
 ******************************************************************************/
static HmiObject qrObject() //20261112-0 add
{
    HmiObject object;
    object.id = QStringLiteral("stage4-qr"); object.type = QStringLiteral("qrcode"); object.name = QStringLiteral("二维码"); object.address = QStringLiteral("LW0"); object.geometry = QRect(480, 20, 160, 110);
    return object;
}

/******************************************************************************
 * Function Name   : existingFiveObjects
 * Description     : Keep the established five independent families in each new control's mixed persistence fixture.
 * Input Parameters: None
 * Return Value    : Valid text, bit button, word button, lamp and system function objects.
 * Date            : 2026.11.12
 ******************************************************************************/
static QList<HmiObject> existingFiveObjects() //20261112-0 add
{
    QList<HmiObject> objects;
    HmiObject object;
    int index;
    for (index = 0; index < 5; ++index)
    {
        object = HmiObject(); object.id = QStringLiteral("existing-%1").arg(index); object.name = object.id; object.geometry = QRect(20 + index * 140, 220, 100, 50);
        object.type = index == 0 ? QStringLiteral("text") : QStringLiteral("switch");
        if (index == 0) { object.hasStaticTextProperties = true; object.text = QStringLiteral("原有文本"); }
        else { object.hasBitButtonProperties = true; object.address = index == 1 ? QStringLiteral("LB0") : QString(); }
        if (index == 2) { object.hasWordButtonProperties = true; object.address = QStringLiteral("LW0"); object.bitButton.indicator = false; object.bitButton.dualMonitor = false; }
        if (index == 3) { object.hasLampProperties = true; object.bitButton.monitorSame = false; }
        if (index == 4) { object.hasFunctionButtonProperties = true; object.bitButton.indicator = false; object.bitButton.dualMonitor = false; }
        objects.append(object);
    }
    return objects;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::qrDefaultsAndPermissionBranches
 * Description     : Exercise both QR pages and measured limits, visibility hierarchy, tolerance and dormant values.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::qrDefaultsAndPermissionBranches() //20261112-0 add
{
    QrCodeDialog dialog(qrObject(), HmiProject());
    QSpinBox *length;
    QLineEdit *description;
    HmiPermissionEditor *permission;
    QGroupBox *word;
    QComboBox *type;
    QComboBox *comparison;
    QLineEdit *tolerance;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    QCOMPARE(dialog.windowTitle(), QStringLiteral("二维码")); QCOMPARE(dialog.findChild<QTabWidget *>()->count(), 2);
    length = dialog.findChild<QSpinBox *>(QStringLiteral("QrCharacterLength")); description = dialog.findChild<QLineEdit *>(QStringLiteral("QrDescription"));
    QCOMPARE(length->minimum(), 0); QCOMPARE(length->maximum(), 255); QCOMPARE(length->value(), 25); QCOMPARE(description->maxLength(), 32767); QVERIFY(description->text().isEmpty());
    QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("QrEncoding"))->count(), 2); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("QrEncoding"))->itemText(1), QStringLiteral("GB2312"));
    QVERIFY(!dialog.findChild<QCheckBox *>(QStringLiteral("QrSwapBytes"))->isChecked()); QCOMPARE(dialog.findChildren<HmiPermissionEditor *>().size(), 1);
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("qr-general.png")))); }
    dialog.findChild<QTabWidget *>()->setCurrentIndex(1);
    permission = dialog.findChild<HmiPermissionEditor *>(QStringLiteral("QrVisibility")); word = dialog.findChild<QGroupBox *>(QStringLiteral("QrVisibilityWord"));
    type = dialog.findChild<QComboBox *>(QStringLiteral("QrVisibilityType")); comparison = dialog.findChild<QComboBox *>(QStringLiteral("QrVisibilityComparison")); tolerance = dialog.findChild<QLineEdit *>(QStringLiteral("QrVisibilityTolerance"));
    QVERIFY(!permission->isChecked()); QVERIFY(!word->isEnabled()); permission->setChecked(true); word->setChecked(true);
    QCOMPARE(type->itemText(0), QStringLiteral("16位正整数")); QCOMPARE(type->itemText(1), QStringLiteral("32位正整数")); QCOMPARE(type->itemText(2), QStringLiteral("16位整数")); QCOMPARE(type->itemText(3), QStringLiteral("32位整数")); QCOMPARE(type->itemText(4), QStringLiteral("32位浮点数"));
    comparison->setCurrentIndex(2); QVERIFY(tolerance->isVisible()); QVERIFY(tolerance->isEnabled()); tolerance->setText(QStringLiteral("2"));
    type->setCurrentIndex(4); comparison->setCurrentIndex(3); QVERIFY(tolerance->isVisible());
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("qr-permissions.png")))); }
    permission->setChecked(false); QVERIFY(!tolerance->isEnabled()); QCOMPARE(dialog.object().qrCode.visibility.tolerance, QStringLiteral("2"));
    length->setValue(0); QCOMPARE(dialog.object().qrCode.characterLength, 0); length->setValue(255); QCOMPARE(dialog.object().qrCode.characterLength, 255);
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::qrDescriptionRasterAndCapacity
 * Description     : Render scannable description fixtures, keep quiet zones and preserve unsupported long data without stale QR output.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::qrDescriptionRasterAndCapacity() //20261112-0 add
{
    QImage ascii = qrDescriptionPreview(QStringLiteral("HTTPS://KINSEAL.TEST/STAGE4"));
    QImage chinese = qrDescriptionPreview(QStringLiteral("金玺测试"));
    QImage empty = qrDescriptionPreview(QString());
    QImage painted(170, 120, QImage::Format_RGB32);
    QPainter painter;
    QrCodeProperties properties;
    QString error;
    QrCodeDialog dialog(qrObject(), HmiProject());
    QFile nativeModules(QFINDTESTDATA("../docs/stage4-evidence/qrcode/native/qr-preview-decoded.json"));
    QJsonArray rows;
    int index;
    int x;
    QVERIFY(!ascii.isNull()); QVERIFY(!chinese.isNull()); QVERIFY(!empty.isNull()); QVERIFY(ascii != chinese); QVERIFY(ascii != empty);
    QVERIFY(nativeModules.open(QIODevice::ReadOnly)); rows = QJsonDocument::fromJson(nativeModules.readAll()).object().value(QStringLiteral("records")).toArray().first().toObject().value(QStringLiteral("moduleRows")).toArray();
    QCOMPARE(rows.size(), 25); QCOMPARE(empty.size(), QSize(33, 33));
    for (index = 0; index < rows.size(); ++index)
    {
        QCOMPARE(rows.at(index).toString().size(), 25);
        for (x = 0; x < 25; ++x) { QCOMPARE(empty.pixelColor(x + 4, index + 4), QColor(rows.at(index).toString().at(x) == QLatin1Char('1') ? Qt::black : Qt::white)); }
    }
    for (index = 0; index < ascii.width(); ++index) { QCOMPARE(ascii.pixelColor(index, 0), QColor(Qt::white)); QCOMPARE(ascii.pixelColor(0, index), QColor(Qt::white)); }
    properties.description = QStringLiteral("金玺测试"); painted.fill(Qt::magenta); painter.begin(&painted); paintQrCode(&painter, QRectF(5, 5, 160, 110), properties); painter.end();
    QCOMPARE(painted.pixelColor(0, 0), QColor(Qt::magenta)); QCOMPARE(painted.pixelColor(6, 6), QColor(Qt::white));
    dialog.findChild<QLineEdit *>(QStringLiteral("QrDescription"))->setText(QString(32767, QLatin1Char('x')));
    QCOMPARE(dialog.object().qrCode.description.size(), 32767); QVERIFY(qrDescriptionPreview(dialog.object().qrCode.description, &error).isNull()); QVERIFY(!error.isEmpty());
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR"))
    {
        QVERIFY(ascii.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("qr-scan-ascii.png"))));
        QVERIFY(chinese.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("qr-scan-chinese.png"))));
        QVERIFY(empty.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("qr-scan-empty.png"))));
    }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::qrStrictPersistenceAndBackendGates
 * Description     : Round-trip all QR fields with other families and reject malformed records and unknown source/device generation atomically.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::qrStrictPersistenceAndBackendGates() //20261112-0 add
{
    HmiProject project;
    HmiProject loaded;
    HmiProject qrOnly;
    HmiObject object = qrObject();
    HmiObject parsed = messageObject();
    QJsonObject json;
    QJsonObject properties;
    QJsonObject visibility;
    QByteArray source = QByteArrayLiteral("unchanged");
    QVector<QByteArray> payloads = {QByteArrayLiteral("unchanged")};
    QString error;
    object.qrCode.encoding = 1; object.qrCode.swapBytes = true; object.qrCode.characterLength = 0; object.qrCode.description = QStringLiteral("金玺测试");
    object.qrCode.visibility.tolerance = QStringLiteral("0.25"); object.qrCode.visibility.wordAddress = QStringLiteral("LW42");
    qrOnly.objects = {object}; project.objects = existingFiveObjects(); project.objects.append({messageObject(), scaleObject(), object});
    QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.toJson(), project.toJson());
    json = object.toJson(); properties = json.value(QStringLiteral("qrCode")).toObject(); properties.insert(QStringLiteral("characterLength"), 1.5); json.insert(QStringLiteral("qrCode"), properties);
    QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); QCOMPARE(parsed.type, QStringLiteral("message"));
    json = object.toJson(); properties = json.value(QStringLiteral("qrCode")).toObject(); visibility = properties.value(QStringLiteral("visibility")).toObject(); visibility.insert(QStringLiteral("unknown"), true); properties.insert(QStringLiteral("visibility"), visibility); json.insert(QStringLiteral("qrCode"), properties);
    QVERIFY(!HmiObject::fromJson(json, &parsed, &error));
    json = object.toJson(); json.insert(QStringLiteral("type"), QStringLiteral("text")); QVERIFY(!HmiObject::fromJson(json, &parsed, &error));
    object.qrCode.monitorAddress = QStringLiteral("LB0"); QVERIFY(!object.qrCode.validateAddresses(project.connections, &error)); object.qrCode.monitorAddress = QStringLiteral("LW0");
    object.qrCode.visibility.visibilityEnabled = true; object.qrCode.visibility.wordEnabled = true; object.qrCode.visibility.dataType = 1; object.qrCode.visibility.wordAddress = QStringLiteral("LW4096"); QVERIFY(!object.qrCode.validateAddresses(project.connections, &error));
    QCOMPARE(loaded.objects.size(), 8);
    QVERIFY(!exportNativeProjectSource(qrOnly, &source, &error)); QCOMPARE(source, QByteArrayLiteral("unchanged")); QVERIFY(!buildProjectPayload(qrOnly, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("unchanged")});
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY2(project.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("qr-mixed.ksp")), &error), qPrintable(error)); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::qrCopyAndCompactProperties
 * Description     : Preserve dormant QR fields through real selection cloning and typed compact-panel description/address changes.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::qrCopyAndCompactProperties() //20261112-0 add
{
    MainWindow window;
    HmiProject project;
    HmiProject copied;
    HmiObject object = qrObject();
    Stage3Clipboard clipboard;
    QStringList created;
    QString error;
    object.qrCode.swapBytes = true; object.qrCode.encoding = 1; object.qrCode.characterLength = 255;
    project.objects = {messageObject(), scaleObject(), object};
    QVERIFY2(captureStage3Selection(project, {object.id}, &clipboard, &error), qPrintable(error));
    QVERIFY2(pasteStage3Selection(project, clipboard, QPoint(0, 120), &copied, &created, &error), qPrintable(error));
    QCOMPARE(copied.objects.last().qrCode.toJson(), object.qrCode.toJson()); QVERIFY(copied.objects.last().id != object.id);
    window.setProject(project, QString()); window.m_canvas->selectObject(object.id); window.m_textEdit->setPlainText(QStringLiteral("HTTPS://KINSEAL.TEST/EDIT")); window.m_addressEdit->setText(QStringLiteral("LW8")); window.applySelectedProperties();
    object = window.currentEditorProject().objects.last(); QCOMPARE(object.qrCode.description, QStringLiteral("HTTPS://KINSEAL.TEST/EDIT")); QCOMPARE(object.qrCode.monitorAddress, QStringLiteral("LW8")); QCOMPARE(object.qrCode.characterLength, 255); QVERIFY(object.qrCode.swapBytes);
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::qrCreationEditCancellationAndHistory
 * Description     : Exercise real command placement, creation cancellation, edit cancellation and one-step accepted edits with undo/redo.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::qrCreationEditCancellationAndHistory() //20261112-0 add
{
    MainWindow window;
    HmiProject project;
    int history;
    window.setProject(project, QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); history = window.m_projectHistory.size();
    QTimer::singleShot(0, &window, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } });
    window.m_commands.value(QStringLiteral("action_qrcode"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement()); QCOMPARE(window.m_projectHistory.size(), history);
    QTimer::singleShot(0, &window, []() {
        QrCodeDialog *dialog = qobject_cast<QrCodeDialog *>(QApplication::activeModalWidget());
        if (dialog != nullptr) { dialog->findChild<QLineEdit *>(QStringLiteral("QrDescription"))->setText(QStringLiteral("create")); dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click(); }
    });
    window.m_commands.value(QStringLiteral("action_qrcode"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QVERIFY(window.currentEditorProject().objects.isEmpty());
    QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20)));
    QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(180, 130)));
    QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(180, 130)));
    QCOMPARE(window.currentEditorProject().objects.size(), 1); QCOMPARE(window.currentEditorProject().objects.first().qrCode.description, QStringLiteral("create")); QCOMPARE(window.m_projectHistory.size(), history + 1);
    QTimer::singleShot(0, &window, []() {
        QrCodeDialog *dialog = qobject_cast<QrCodeDialog *>(QApplication::activeModalWidget());
        if (dialog != nullptr) { dialog->findChild<QLineEdit *>(QStringLiteral("QrDescription"))->setText(QStringLiteral("discard")); dialog->reject(); }
    });
    window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().qrCode.description, QStringLiteral("create"));
    QTimer::singleShot(0, &window, []() {
        QrCodeDialog *dialog = qobject_cast<QrCodeDialog *>(QApplication::activeModalWidget());
        if (dialog != nullptr) { dialog->findChild<QLineEdit *>(QStringLiteral("QrDescription"))->setText(QStringLiteral("edit")); dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click(); }
    });
    window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().qrCode.description, QStringLiteral("edit")); QCOMPARE(window.m_projectHistory.size(), history + 2);
    window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().objects.first().qrCode.description, QStringLiteral("create")); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty());
    window.restoreProjectHistory(1); window.restoreProjectHistory(1); QCOMPARE(window.currentEditorProject().objects.first().qrCode.description, QStringLiteral("edit"));
}

/******************************************************************************
 * Function Name   : imagePng
 * Description     : Create a deterministic tiny image source for actual renderer and binding tests.
 * Input Parameters: color
 * Return Value    : Valid PNG bytes.
 * Date            : 2026.11.12
 ******************************************************************************/
static QByteArray imagePng(const QColor &color) //20261112-0 add
{
    QImage image(8, 6, QImage::Format_ARGB32);
    QByteArray bytes;
    QBuffer buffer(&bytes);
    image.fill(color); buffer.open(QIODevice::WriteOnly); image.save(&buffer, "PNG"); return bytes;
}

/******************************************************************************
 * Function Name   : imageObject
 * Description     : Provide one valid static picture independently from old unconfigured image placeholders.
 * Input Parameters: None
 * Return Value    : Typed image object.
 * Date            : 2026.11.12
 ******************************************************************************/
static HmiObject imageObject() //20261112-0 add
{
    HmiObject object;
    object.id = QStringLiteral("stage4-image"); object.type = QStringLiteral("image"); object.name = QStringLiteral("图片显示器"); object.geometry = QRect(20, 320, 160, 100); object.hasImageDisplayProperties = true;
    object.imageDisplay.states[0] = {QStringLiteral("red.png"), imagePng(Qt::red)}; return object;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::imageDefaultsConditionsAndStateLifetime
 * Description     : Exercise measured defaults, all switching modes, address-role changes and destructive shrinking.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::imageDefaultsConditionsAndStateLifetime() //20261112-0 add
{
    HmiObject object = imageObject();
    ImageDisplayDialog *dialog;
    QSpinBox *count;
    QLineEdit *address;
    QSpinBox *interval;
    QGroupBox *conditions;
    QTableWidget *table;
    bool explained = false;
    object.imageDisplay.states[0] = ImageDisplayState(); dialog = new ImageDisplayDialog(object, HmiProject()); dialog->show(); QVERIFY(QTest::qWaitForWindowExposed(dialog));
    count = dialog->findChild<QSpinBox *>(QStringLiteral("ImageStateCount")); address = dialog->findChild<QLineEdit *>(QStringLiteral("ImageMonitorAddress")); interval = dialog->findChild<QSpinBox *>(QStringLiteral("ImageInterval")); conditions = dialog->findChild<QGroupBox *>(QStringLiteral("ImageConditions")); table = dialog->findChild<QTableWidget *>(QStringLiteral("ImageStates"));
    QCOMPARE(dialog->findChild<QTabWidget *>()->count(), 3); QCOMPARE(count->minimum(), 1); QCOMPARE(count->maximum(), 1); QVERIFY(!count->isEnabled()); QVERIFY(!conditions->isEnabled()); QVERIFY(!address->isEnabled()); QVERIFY(address->text().isEmpty()); QCOMPARE(interval->minimum(), 1); QCOMPARE(interval->maximum(), 1000); QCOMPARE(interval->value(), 1); QVERIFY(!interval->isEnabled());
    QTimer::singleShot(0, dialog, [&explained]() { QMessageBox *message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget()); if (message != nullptr) { explained = message->text() == QStringLiteral("状态没有选择图片!!"); message->accept(); } });
    dialog->findChild<QDialogButtonBox *>(QStringLiteral("ImageButtons"))->button(QDialogButtonBox::Ok)->click(); QVERIFY(explained); QVERIFY(dialog->isVisible());
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog->grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("image-general.png")))); }
    dialog->findChild<QRadioButton *>(QStringLiteral("ImageType1"))->click(); QCOMPARE(count->minimum(), 2); QCOMPARE(count->maximum(), 128); QCOMPARE(count->value(), 2); QVERIFY(interval->isEnabled()); QVERIFY(!conditions->isCheckable()); QCOMPARE(dialog->findChild<QSpinBox *>(QStringLiteral("ImageBatchEnd"))->value(), 1);
    count->setValue(3); QCOMPARE(table->rowCount(), 3); QCOMPARE(dialog->findChild<QSpinBox *>(QStringLiteral("ImageBatchEnd"))->value(), 1); dialog->findChild<QRadioButton *>(QStringLiteral("ImageCondition1"))->click(); QCOMPARE(address->text(), QStringLiteral("LW0")); QVERIFY(address->isEnabled()); QVERIFY(!interval->isEnabled());
    dialog->findChild<QRadioButton *>(QStringLiteral("ImageCondition2"))->click(); QCOMPARE(count->value(), 2); QCOMPARE(count->maximum(), 2); QCOMPARE(address->text(), QStringLiteral("LB0")); QVERIFY(!count->isEnabled());
    dialog->findChild<QRadioButton *>(QStringLiteral("ImageCondition0"))->click(); QCOMPARE(count->value(), 2); QCOMPARE(address->text(), QStringLiteral("LB0")); QVERIFY(!address->isEnabled());
    dialog->findChild<QRadioButton *>(QStringLiteral("ImageType0"))->click(); QCOMPARE(count->value(), 1); QVERIFY(conditions->isChecked()); QVERIFY(!conditions->isEnabled()); delete dialog;
    object = imageObject(); object.imageDisplay.displayType = 1; object.imageDisplay.conditionEnabled = true; object.imageDisplay.setStateCount(3); object.imageDisplay.states[2] = {QStringLiteral("green.png"), imagePng(Qt::green)};
    dialog = new ImageDisplayDialog(object, HmiProject()); count = dialog->findChild<QSpinBox *>(QStringLiteral("ImageStateCount")); count->setValue(2); count->setValue(3); QVERIFY(dialog->object().imageDisplay.states.at(2).picturePng.isEmpty()); delete dialog;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::imageBatchOrderDeletionAndGalleryCancel
 * Description     : Exercise actual child multi-selection, ordered/truncated filling, batch deletion, reversed intervals and cancellation.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::imageBatchOrderDeletionAndGalleryCancel() //20261112-0 add
{
    HmiProject project;
    HmiObject object = imageObject();
    ImageDisplayDialog *dialog;
    QTableWidget *table;
    QCheckBox *add;
    QCheckBox *remove;
    QSpinBox *start;
    QSpinBox *end;
    QTimer interactor;
    bool handled = false;
    bool modeMatched = false;
    bool explained = false;
    object.imageDisplay.displayType = 1; object.imageDisplay.conditionEnabled = true; object.imageDisplay.setStateCount(3); object.imageDisplay.states.fill(ImageDisplayState());
    project.pictures = {{QStringLiteral("a-red.png"), imagePng(Qt::red)}, {QStringLiteral("b-green.png"), imagePng(Qt::green)}};
    dialog = new ImageDisplayDialog(object, project); dialog->show(); dialog->findChild<QTabWidget *>()->setCurrentIndex(1); table = dialog->findChild<QTableWidget *>(QStringLiteral("ImageStates")); add = dialog->findChild<QCheckBox *>(QStringLiteral("ImageBatchAdd")); remove = dialog->findChild<QCheckBox *>(QStringLiteral("ImageBatchDelete")); start = dialog->findChild<QSpinBox *>(QStringLiteral("ImageBatchStart")); end = dialog->findChild<QSpinBox *>(QStringLiteral("ImageBatchEnd"));
    QCOMPARE(end->value(), 2); add->setChecked(true); remove->setChecked(true); add->setChecked(false); QVERIFY(start->isEnabled()); add->setChecked(true);
    interactor.setInterval(20); connect(&interactor, &QTimer::timeout, dialog, [&]() {
        QDialog *gallery = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QListWidget *list;
        if (gallery == nullptr || gallery->objectName() != QStringLiteral("BitGallery")) { return; }
        list = gallery->findChild<QListWidget *>(QStringLiteral("GalleryItems")); if (list == nullptr || list->count() != 2) { return; }
        modeMatched = list->selectionMode() == QAbstractItemView::MultiSelection; list->clearSelection(); list->item(1)->setSelected(true); list->item(0)->setSelected(true); handled = true; interactor.stop(); gallery->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
    });
    interactor.start(); dialog->findChild<QPushButton *>(QStringLiteral("ImagePictureAction2"))->click(); QVERIFY(handled); QVERIFY(modeMatched);
    QCOMPARE(dialog->object().imageDisplay.states.at(0).picture, QStringLiteral("a-red.png")); QCOMPARE(dialog->object().imageDisplay.states.at(1).picture, QStringLiteral("b-green.png")); QVERIFY(dialog->object().imageDisplay.states.at(2).picture.isEmpty());
    table->setCurrentCell(0, 0); dialog->findChild<QPushButton *>(QStringLiteral("ImageMove1"))->click(); QCOMPARE(dialog->object().imageDisplay.states.at(0).picture, QStringLiteral("b-green.png")); dialog->findChild<QPushButton *>(QStringLiteral("ImageMove0"))->click(); QCOMPARE(dialog->object().imageDisplay.states.at(0).picture, QStringLiteral("a-red.png"));
    start->setValue(1); end->setValue(1); dialog->findChild<QPushButton *>(QStringLiteral("ImagePictureAction0"))->click(); QVERIFY(dialog->object().imageDisplay.states.at(1).picture.isEmpty()); QCOMPARE(table->rowCount(), 3); QVERIFY(!dialog->object().imageDisplay.states.at(0).picture.isEmpty());
    start->setValue(2); end->setValue(0); dialog->findChild<QPushButton *>(QStringLiteral("ImagePictureAction0"))->click(); QVERIFY(!dialog->object().imageDisplay.states.at(0).picture.isEmpty());
    QTimer::singleShot(0, dialog, [&explained]() { QMessageBox *message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget()); if (message != nullptr) { explained = message->text() == QStringLiteral("批量加起始结束位置异常，请重新设置。"); message->accept(); } });
    dialog->findChild<QPushButton *>(QStringLiteral("ImagePictureAction2"))->click(); QVERIFY(explained);
    add->setChecked(false); remove->setChecked(false); QVERIFY(!start->isEnabled()); QTimer::singleShot(50, dialog, []() { QDialog *gallery = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (gallery != nullptr) { gallery->reject(); } });
    dialog->findChild<QPushButton *>(QStringLiteral("ImagePictureAction2"))->click(); QCOMPARE(dialog->object().imageDisplay.states.at(0).picture, QStringLiteral("a-red.png")); QCOMPARE(dialog->pictures(), project.pictures);
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog->grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("image-pictures.png")))); dialog->findChild<QTabWidget *>()->setCurrentIndex(2); QVERIFY(dialog->grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("image-permissions.png")))); }
    delete dialog;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::imageResourceBindingsAndCopyCollisions
 * Description     : Protect and remap picture bindings on active/inactive pages and preserve bytes during cross-project name collisions.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::imageResourceBindingsAndCopyCollisions() //20261112-0 add
{
    HmiProject project;
    HmiProject renamed;
    HmiProject replaced;
    HmiProject target;
    HmiProject copied;
    HmiObject object = imageObject();
    HmiPage page;
    Stage3Clipboard clipboard;
    QString error;
    project.objects = {object}; project.pictures.insert(QStringLiteral("red.png"), object.imageDisplay.states.first().picturePng);
    page.id = QStringLiteral("image-page2"); page.name = QStringLiteral("图片第二页"); object.id = QStringLiteral("image-on-page2"); page.objects = {object}; QVERIFY2(project.addPage(page, &error), qPrintable(error)); //20261112-0 change: Register page order through the actual model API.
    QCOMPARE(stage3PictureUsage(project, QStringLiteral("red.png")).boundStates, 2);
    QVERIFY2(renameStage3Picture(project, QStringLiteral("red.png"), QStringLiteral("renamed.png"), &renamed, &error), qPrintable(error)); QCOMPARE(renamed.objects.first().imageDisplay.states.first().picture, QStringLiteral("renamed.png")); QCOMPARE(renamed.inactivePages.first().objects.first().imageDisplay.states.first().picture, QStringLiteral("renamed.png"));
    QVERIFY(!deleteStage3Picture(renamed, QStringLiteral("renamed.png"), &target, &error)); QVERIFY(target.objects.isEmpty());
    QVERIFY2(replaceStage3Picture(renamed, QStringLiteral("renamed.png"), imagePng(Qt::green), &replaced, &error), qPrintable(error)); QCOMPARE(replaced.inactivePages.first().objects.first().imageDisplay.states.first().picturePng, imagePng(Qt::green));
    target.pictures.insert(QStringLiteral("renamed.png"), imagePng(Qt::blue)); QVERIFY2(captureStage3Selection(replaced, {replaced.objects.first().id}, &clipboard, &error), qPrintable(error)); QVERIFY2(pasteStage3Selection(target, clipboard, QPoint(), &copied, nullptr, &error), qPrintable(error));
    QVERIFY(copied.objects.first().imageDisplay.states.first().picture.startsWith(QStringLiteral("renamed.png~"))); QCOMPARE(copied.objects.first().imageDisplay.states.first().picturePng, imagePng(Qt::green)); QCOMPARE(copied.pictures.value(QStringLiteral("renamed.png")), imagePng(Qt::blue));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::imagePersistenceLegacyAndBackendGates
 * Description     : Preserve a nine-family mixed project, reject corrupt picture records and keep legacy placeholders and backend output safe.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::imagePersistenceLegacyAndBackendGates() //20261112-0 add
{
    HmiProject project;
    HmiProject loaded;
    HmiProject imageOnly;
    HmiObject object = imageObject();
    HmiObject parsed = qrObject();
    HmiObject legacy;
    QJsonObject json;
    QJsonObject properties;
    QJsonObject row;
    QJsonArray rows;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    project.objects = existingFiveObjects(); project.objects.append({messageObject(), scaleObject(), qrObject(), object}); QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.toJson(), project.toJson()); QCOMPARE(loaded.objects.size(), 9);
    json = object.toJson(); properties = json.value(QStringLiteral("imageDisplay")).toObject(); rows = properties.value(QStringLiteral("states")).toArray(); row = rows.first().toObject(); row.insert(QStringLiteral("picturePng"), QStringLiteral("not base64!")); rows[0] = row; properties.insert(QStringLiteral("states"), rows); json.insert(QStringLiteral("imageDisplay"), properties); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); QCOMPARE(parsed.type, QStringLiteral("qrcode"));
    legacy = object; legacy.hasImageDisplayProperties = false; QVERIFY2(HmiObject::fromJson(legacy.toJson(), &parsed, &error), qPrintable(error)); QVERIFY(!parsed.hasImageDisplayProperties);
    imageOnly.objects = {object}; QVERIFY(!exportNativeProjectSource(imageOnly, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(imageOnly, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")});
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY2(project.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("image-mixed.ksp")), &error), qPrintable(error)); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::imageActualRasterAndPlacementTransaction
 * Description     : Verify real state pixels and execute child gallery, accepted placement, edit cancellation and whole-object undo/redo.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::imageActualRasterAndPlacementTransaction() //20261112-0 add
{
    HmiObject object = imageObject();
    QImage raster(180, 120, QImage::Format_ARGB32);
    QPainter painter;
    HmiProject project;
    MainWindow window;
    QTimer parentTimer;
    QTimer childTimer;
    bool configured = false;
    bool childAccepted = false;
    int history;
    raster.fill(Qt::magenta); painter.begin(&raster); paintImageDisplay(&painter, QRectF(10, 10, 160, 100), object.imageDisplay, 0); painter.end(); QCOMPARE(raster.pixelColor(50, 50), QColor(Qt::red)); QCOMPARE(raster.pixelColor(0, 0), QColor(Qt::magenta));
    object.imageDisplay.states[0].picturePng = imagePng(Qt::green); painter.begin(&raster); paintImageDisplay(&painter, QRectF(10, 10, 160, 100), object.imageDisplay, 0); painter.end(); QCOMPARE(raster.pixelColor(50, 50), QColor(Qt::green));
    project.pictures.insert(QStringLiteral("green.png"), imagePng(Qt::green)); window.setProject(project, QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); history = window.m_projectHistory.size();
    QTimer::singleShot(0, &window, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } }); window.m_commands.value(QStringLiteral("pictruedisplay"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement()); QCOMPARE(window.m_projectHistory.size(), history);
    parentTimer.setInterval(20); childTimer.setInterval(20);
    connect(&childTimer, &QTimer::timeout, &window, [&]() {
        QDialog *gallery = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QListWidget *list;
        if (gallery == nullptr || gallery->objectName() != QStringLiteral("BitGallery")) { return; }
        list = gallery->findChild<QListWidget *>(QStringLiteral("GalleryItems")); if (list == nullptr || list->count() != 1) { return; } list->setCurrentRow(0); childAccepted = true; childTimer.stop(); gallery->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
    });
    connect(&parentTimer, &QTimer::timeout, &window, [&]() {
        ImageDisplayDialog *dialog = qobject_cast<ImageDisplayDialog *>(QApplication::activeModalWidget());
        if (dialog == nullptr || configured) { return; } configured = true; parentTimer.stop(); dialog->findChild<QPushButton *>(QStringLiteral("ImagePictureAction2"))->click(); dialog->findChild<QDialogButtonBox *>(QStringLiteral("ImageButtons"))->button(QDialogButtonBox::Ok)->click();
    });
    parentTimer.start(); childTimer.start(); window.m_commands.value(QStringLiteral("pictruedisplay"))->trigger(); QVERIFY(configured); QVERIFY(childAccepted); QVERIFY(window.m_canvas->hasPendingPlacement()); QVERIFY(window.currentEditorProject().objects.isEmpty());
    QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(180, 120))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(180, 120)));
    QCOMPARE(window.currentEditorProject().objects.size(), 1); QCOMPARE(window.currentEditorProject().objects.first().imageDisplay.states.first().picturePng, imagePng(Qt::green)); QCOMPARE(window.m_projectHistory.size(), history + 1);
    QTimer::singleShot(0, &window, []() { ImageDisplayDialog *dialog = qobject_cast<ImageDisplayDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } }); window.editStage4Control(); QCOMPARE(window.m_projectHistory.size(), history + 1);
    QTimer::singleShot(0, &window, []() { ImageDisplayDialog *dialog = qobject_cast<ImageDisplayDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QSpinBox *>(QStringLiteral("ImageInterval"))->setValue(7); dialog->findChild<QDialogButtonBox *>(QStringLiteral("ImageButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.editStage4Control(); QCOMPARE(window.m_projectHistory.size(), history + 2); QCOMPARE(window.currentEditorProject().objects.first().imageDisplay.interval, 7);
    window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().objects.first().imageDisplay.interval, 1); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty()); window.restoreProjectHistory(1); window.restoreProjectHistory(1); QVERIFY(window.currentEditorProject().objects.first().hasImageDisplayProperties); QCOMPARE(window.currentEditorProject().objects.first().imageDisplay.interval, 7);
}

QTEST_MAIN(Stage4ControlTests)
#include "stage4_controls_tests.moc"
