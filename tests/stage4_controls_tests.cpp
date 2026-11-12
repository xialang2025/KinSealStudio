#include "main_window.h"
#include "hmi_canvas.h"
#include "message_display_dialog.h"
#include "scale_dialog.h"
#include "qr_code_dialog.h" //20261112-0 add
#include "image_display_dialog.h" //20261112-0 add
#include "gif_display_dialog.h" //20261112-0 add
#include "animation_resources.h"
#include "number_display_dialog.h" //20261112-0 add
#include "bit_button_dialog.h" //20261112-0 add
#include "dropdown_dialog.h" //20261112-0 add
#include "bit_button_renderer.h" //20261112-0 add
#include <QImageReader>
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
#include <QColorDialog> //20261112-0 add
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
    void gifDefaultsGuardsAndIndependentPermissions();
    void gifOriginalBytesFramesAndStrictPersistence();
    void gifGalleryImportCancelAndResourceProtection();
    void gifActualPlaybackPauseAndCanvasPoster();
    void gifCrossProjectReferencesAndAddressIncrement();
    void gifPlacementCancellationAndUndoAreAtomic();
    void numberNativeDefaultsAndInputLinkage();
    void numberAllDataTypeTransitions();
    void numberRangesAndActiveAddressSpans();
    void numberPermissionNotificationAndKeyboardLifecycle();
    void numberStrictPersistenceLegacyAndBackendGates();
    void numberPicturesCopiesAndAddressRoles();
    void numberActualRasterAndHiddenAppearanceDrafts();
    void numberShellPlacementEditAndUndo();
    void asciiNativeDefaultsAndDistinctFields();
    void asciiNumericSwitchAndDormantDrafts();
    void asciiStrictMixedPersistenceAndBackendGates();
    void asciiResourcesAddressesAndGalleryTransaction();
    void asciiSampleRasterAndOptionIsolation();
    void asciiShellPlacementConversionAndHistory();
    void timeNativeDefaultsAndThreePages();
    void timeFormatsImmediateGuardAndDraftRetention();
    void timeFamilySwitchPreservesIndependentDrafts();
    void timeStrictMixedPersistenceAndBackendGates();
    void timeResourcesVisibilityAndNoMonitorCarrier();
    void timeRasterClippingAndFormatSamples();
    void timeShellPlacementConversionAndUndo();
    void screenDefaultsOperationsAndStableTargets();
    void screenMonitorAppearanceAndPermissions();
    void screenStrictMixedPersistenceAndBackendGates();
    void screenPageLifecycleAndClipboardDependencies();
    void screenResourcesAndRealAddressRoles();
    void screenShellCreationCancellationAndUndo();
    void screenLanguageCompactProperties();
    void multiDefaultsAndMirroredAddressLifetime();
    void multiCountShrinkRegrowAndSelection();
    void multiAllStatesLanguagesAndFlags();
    void multiStrictMixedPersistenceAndBackendGates();
    void multiResourcesAndFullAddressRoles();
    void multiShellPlacementStatePreviewAndUndo();
    void multiLanguageCompactProperties();
    void mfDefaultsAndCommandSelection(); //20261112-0 add
    void mfCommandLimitDeletionAndEmptyDraft();
    void mfDataTypeMappingAndFullSpans();
    void mfPermissionsUnchosenAndActiveSelection();
    void mfPressedAppearanceLanguageAndCopies();
    void mfStrictMixedPersistenceAndBackendGates();
    void mfNavigationLifecycleAndClipboard();
    void mfResourcesAndAddressIncrements();
    void mfShellPlacementEditCompactAndUndo();
    void dropdownDefaultsCountsAndRowLifetime(); //20261112-0 add
    void dropdownTableInlineEditingAndColorTransaction();
    void dropdownTypesPermissionsAndFullCarriers();
    void dropdownMacroDraftLifecycleAndClipboard();
    void dropdownStrictMixedPersistenceAndBackendGates();
    void dropdownActualFaceAndDirectionRaster();
    void dropdownShellMacroPlacementCompactAndUndo();
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

/******************************************************************************
 * Function Name   : gifFixture
 * Description     : Load the original retained native animation stimulus with its exact delays and loop bytes.
 * Input Parameters: alternate
 * Return Value    : Original GIF bytes.
 * Date            : 2026.11.12
 ******************************************************************************/
static QByteArray gifFixture(bool alternate = false) //20261112-0 add
{
    QFile file(alternate ? QFINDTESTDATA("../docs/stage4-evidence/gif/native/probe-blue-yellow.gif") : QFINDTESTDATA("../docs/stage4-evidence/gif/native/probe-red-green.gif"));
    if (!file.open(QIODevice::ReadOnly)) { return QByteArray(); } return file.readAll();
}

/******************************************************************************
 * Function Name   : gifObject
 * Description     : Build a valid independent GIF control rather than a static PNG appearance.
 * Input Parameters: None
 * Return Value    : Complete editor GIF object.
 * Date            : 2026.11.12
 ******************************************************************************/
static HmiObject gifObject() //20261112-0 add
{
    HmiObject object;
    object.type = QStringLiteral("gif"); object.id = QStringLiteral("gif-object"); object.name = QStringLiteral("GIF显示器"); object.geometry = QRect(20, 20, 120, 60);
    object.gifDisplay.animation = QStringLiteral("red-green.gif"); object.gifDisplay.gifData = gifFixture(); return object;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::gifDefaultsGuardsAndIndependentPermissions
 * Description     : Exercise exact empty-resource messages, playback-bit linkage and independent visibility drafts.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::gifDefaultsGuardsAndIndependentPermissions() //20261112-0 add
{
    HmiObject empty = gifObject();
    GifDisplayDialog *dialog;
    QStringList messages;
    QTimer guard;
    HmiPermissionEditor *visibility;
    empty.gifDisplay.gifData.clear(); empty.gifDisplay.animation.clear(); dialog = new GifDisplayDialog(empty, HmiProject()); dialog->show(); QVERIFY(QTest::qWaitForWindowExposed(dialog));
    QCOMPARE(dialog->findChild<QTabWidget *>(QStringLiteral("GifTabs"))->count(), 2); QVERIFY(!dialog->findChild<QCheckBox *>(QStringLiteral("GifBitControl"))->isChecked());
    QCOMPARE(dialog->findChild<QLineEdit *>(QStringLiteral("GifControlAddress"))->text(), QStringLiteral("LB0")); QVERIFY(!dialog->findChild<QLineEdit *>(QStringLiteral("GifControlAddress"))->isEnabled());
    QVERIFY(dialog->findChild<QRadioButton *>(QStringLiteral("GifState1"))->isChecked()); guard.setInterval(10);
    connect(&guard, &QTimer::timeout, dialog, [&]() { QMessageBox *message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget()); if (message != nullptr) { messages.append(message->text()); message->accept(); } }); guard.start();
    dialog->findChild<QPushButton *>(QStringLiteral("GifRun"))->click(); dialog->findChild<QDialogButtonBox *>(QStringLiteral("GifButtons"))->button(QDialogButtonBox::Ok)->click(); guard.stop();
    QCOMPARE(messages, (QStringList{QStringLiteral("GIF数据为空，请重新选择。"), QStringLiteral("没有选择图片或图片已损坏！")})); QVERIFY(dialog->isVisible());
    dialog->findChild<QCheckBox *>(QStringLiteral("GifBitControl"))->setChecked(true); QVERIFY(dialog->findChild<QLineEdit *>(QStringLiteral("GifControlAddress"))->isEnabled());
    dialog->findChild<QLineEdit *>(QStringLiteral("GifControlAddress"))->setText(QStringLiteral("LB17")); dialog->findChild<QRadioButton *>(QStringLiteral("GifState0"))->click();
    dialog->findChild<QCheckBox *>(QStringLiteral("GifBitControl"))->setChecked(false); QCOMPARE(dialog->object().gifDisplay.controlAddress, QStringLiteral("LB17")); QCOMPARE(dialog->object().gifDisplay.activeState, 0); QVERIFY(!dialog->object().gifDisplay.visibility.visibilityEnabled);
    visibility = dialog->findChild<HmiPermissionEditor *>(QStringLiteral("GifVisibility")); QCOMPARE(dialog->findChildren<HmiPermissionEditor *>().size(), 1); visibility->setChecked(true);
    dialog->findChild<QGroupBox *>(QStringLiteral("GifVisibilityWord"))->setChecked(true); QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("GifVisibilityType"))->count(), 5); QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("GifVisibilityComparison"))->count(), 6); QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("GifVisibilityMinimum"))->count(), 8);
    dialog->findChild<QComboBox *>(QStringLiteral("GifVisibilityType"))->setCurrentIndex(4); dialog->findChild<QLineEdit *>(QStringLiteral("GifVisibilityWordAddress"))->setText(QStringLiteral("LW10")); visibility->setChecked(false); QCOMPARE(dialog->object().gifDisplay.visibility.dataType, 4); QCOMPARE(dialog->object().gifDisplay.visibility.wordAddress, QStringLiteral("LW10"));
    dialog->reject(); delete dialog;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::gifOriginalBytesFramesAndStrictPersistence
 * Description     : Verify raw bytes, frame order, delays and loop fields survive a ten-family mixed project and reject corrupt records atomically.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::gifOriginalBytesFramesAndStrictPersistence() //20261112-0 add
{
    HmiProject project;
    HmiProject loaded;
    HmiProject sentinel;
    HmiObject object = gifObject();
    HmiObject parsed = qrObject();
    QBuffer buffer;
    QImageReader reader;
    QImage frame;
    QJsonObject json;
    QJsonObject props;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    QVERIFY2(validateGifData(object.gifDisplay.gifData, &error), qPrintable(error)); buffer.setData(object.gifDisplay.gifData); buffer.open(QIODevice::ReadOnly); reader.setDevice(&buffer);
    QCOMPARE(reader.imageCount(), 2); QCOMPARE(reader.loopCount(), 3); frame = reader.read(); QCOMPARE(frame.pixelColor(5, 5), QColor(255, 0, 0)); QCOMPARE(reader.nextImageDelay(), 110);
    frame = reader.read(); QCOMPARE(frame.pixelColor(5, 5), QColor(0, 255, 0)); QCOMPARE(reader.nextImageDelay(), 270);
    project.objects = existingFiveObjects(); project.objects.append({messageObject(), scaleObject(), qrObject(), imageObject(), object}); project.animations.insert(object.gifDisplay.animation, object.gifDisplay.gifData);
    QCOMPARE(project.toJson().value(QStringLiteral("schemaVersion")).toInt(), 12); QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.toJson(), project.toJson()); QCOMPARE(loaded.objects.size(), 10); QCOMPARE(loaded.animations, project.animations);
    json = project.toJson(); json.insert(QStringLiteral("schemaVersion"), 11); QVERIFY(!HmiProject::fromJson(json, &sentinel, &error)); QVERIFY(sentinel.objects.isEmpty());
    json = object.toJson(); props = json.value(QStringLiteral("gifDisplay")).toObject(); props.insert(QStringLiteral("activeState"), 0.5); json.insert(QStringLiteral("gifDisplay"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); QCOMPARE(parsed.type, QStringLiteral("qrcode"));
    props = object.gifDisplay.toJson(); props.insert(QStringLiteral("gifData"), QStringLiteral("!!!!")); json.insert(QStringLiteral("gifDisplay"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); QVERIFY(!validateGifData(object.gifDisplay.gifData.chopped(8))); QVERIFY(!validateGifData(imagePng(Qt::red)));
    project.objects = {object}; QVERIFY(!exportNativeProjectSource(project, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(project, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")});
    project.objects = existingFiveObjects(); QVERIFY(!exportNativeProjectSource(project, &source, &error)); QVERIFY(error.contains(QStringLiteral("GIF"))); QCOMPARE(source, QByteArrayLiteral("keep"));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR"))
    { QVERIFY2(loaded.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("gif-mixed.ksp")), &error), qPrintable(error)); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::gifGalleryImportCancelAndResourceProtection
 * Description     : Test actual file import atomicity, nested cancellation and resource remapping on inactive pages.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::gifGalleryImportCancelAndResourceProtection() //20261112-0 add
{
    HmiProject project;
    HmiProject renamed;
    HmiProject replaced;
    HmiProject result;
    HmiPage page;
    HmiObject object = gifObject();
    AnimationGalleryDialog gallery(project, QString());
    QString error;
    QString path = QFINDTESTDATA("../docs/stage4-evidence/gif/native/probe-red-green.gif");
    QTimer nested;
    GifDisplayDialog *dialog;
    QVERIFY2(gallery.importFiles({path}, &error), qPrintable(error)); QCOMPARE(gallery.draftProject().animations.first(), gifFixture()); QVERIFY(project.animations.isEmpty());
    QVERIFY(!gallery.importFiles({path, QStringLiteral("does-not-exist.gif")}, &error)); QCOMPARE(gallery.draftProject().animations.size(), 1); gallery.reject(); QVERIFY(project.animations.isEmpty());
    project.objects = {object}; project.animations.insert(object.gifDisplay.animation, object.gifDisplay.gifData); page.id = QStringLiteral("gif-page"); page.name = QStringLiteral("GIF第二页"); object.id = QStringLiteral("inactive-gif"); page.objects.append(object); QVERIFY2(project.addPage(page, &error), qPrintable(error));
    QCOMPARE(animationUsage(project, QStringLiteral("red-green.gif")), 2); QVERIFY(!deleteAnimation(project, QStringLiteral("red-green.gif"), &result, &error));
    QVERIFY2(renameAnimation(project, QStringLiteral("red-green.gif"), QStringLiteral("renamed.gif"), &renamed, &error), qPrintable(error)); QCOMPARE(renamed.inactivePages.first().objects.first().gifDisplay.animation, QStringLiteral("renamed.gif"));
    QVERIFY2(replaceAnimation(renamed, QStringLiteral("renamed.gif"), gifFixture(true), &replaced, &error), qPrintable(error)); QCOMPARE(replaced.objects.first().gifDisplay.gifData, gifFixture(true)); QCOMPARE(project.objects.first().gifDisplay.gifData, gifFixture());
    dialog = new GifDisplayDialog(project.objects.first(), project); nested.setInterval(10);
    connect(&nested, &QTimer::timeout, dialog, [&]() { AnimationGalleryDialog *child = qobject_cast<AnimationGalleryDialog *>(QApplication::activeModalWidget()); if (child != nullptr) { child->reject(); } }); nested.start();
    dialog->findChild<QPushButton *>(QStringLiteral("GifChoose"))->click(); nested.stop(); QCOMPARE(dialog->object().gifDisplay.toJson(), project.objects.first().gifDisplay.toJson()); dialog->reject(); delete dialog;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::gifActualPlaybackPauseAndCanvasPoster
 * Description     : Observe real frame changes, pause stability, resumed playback and clipped first-frame canvas pixels.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::gifActualPlaybackPauseAndCanvasPoster() //20261112-0 add
{
    GifDisplayDialog dialog(gifObject(), HmiProject());
    QLabel *preview = dialog.findChild<QLabel *>(QStringLiteral("GifPreview"));
    QPushButton *run = dialog.findChild<QPushButton *>(QStringLiteral("GifRun"));
    QImage paused;
    QImage raster(180, 100, QImage::Format_ARGB32);
    QPainter painter;
    QJsonObject before = dialog.object().toJson();
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(preview->pixmap().toImage().pixelColor(5, 5), QColor(255, 0, 0));
    run->click(); QCOMPARE(run->text(), QStringLiteral("停止运行")); QTRY_COMPARE_WITH_TIMEOUT(preview->pixmap().toImage().pixelColor(5, 5), QColor(0, 255, 0), 1500);
    run->click(); paused = preview->pixmap().toImage(); QTest::qWait(400); QCOMPARE(preview->pixmap().toImage(), paused); QCOMPARE(run->text(), QStringLiteral("运行测试"));
    run->click(); QTRY_COMPARE_WITH_TIMEOUT(preview->pixmap().toImage().pixelColor(5, 5), QColor(255, 0, 0), 1500); run->click(); QCOMPARE(dialog.object().toJson(), before);
    raster.fill(Qt::magenta); painter.begin(&raster); paintGifDisplay(&painter, QRectF(10, 10, 160, 60), gifObject().gifDisplay); painter.end(); QCOMPARE(raster.pixelColor(0, 0), QColor(Qt::magenta)); QCOMPARE(raster.pixelColor(100, 30), QColor(255, 0, 0));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR"))
    {
        QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("gif-general.png"))));
        dialog.findChild<QTabWidget *>(QStringLiteral("GifTabs"))->setCurrentIndex(1); QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("gif-permissions.png"))));
    }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::gifCrossProjectReferencesAndAddressIncrement
 * Description     : Preserve exact bytes through collisions and explicit references while incrementing only active typed bit slots.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::gifCrossProjectReferencesAndAddressIncrement() //20261112-0 add
{
    HmiProject source;
    HmiProject target;
    HmiProject copied;
    HmiObject object = gifObject();
    HmiReference reference;
    Stage3Clipboard clipboard;
    Stage3DuplicateOptions options;
    HmiProject duplicated;
    QString error;
    QString newPage;
    object.gifDisplay.bitControl = true; object.gifDisplay.controlAddress = QStringLiteral("LB17"); object.gifDisplay.activeState = 0;
    source.objects = {object}; source.animations.insert(object.gifDisplay.animation, object.gifDisplay.gifData);
    reference.id = QStringLiteral("gif-resource-reference"); reference.ownerKind = QStringLiteral("object"); reference.ownerId = object.id; reference.field = QStringLiteral("animation"); reference.targetKind = QStringLiteral("animation"); reference.targetId = object.gifDisplay.animation; source.references.append(reference);
    target.animations.insert(object.gifDisplay.animation, gifFixture(true)); QVERIFY2(captureStage3Selection(source, {object.id}, &clipboard, &error), qPrintable(error)); QVERIFY2(pasteStage3Selection(target, clipboard, QPoint(), &copied, nullptr, &error), qPrintable(error));
    QCOMPARE(copied.objects.first().gifDisplay.gifData, gifFixture()); QCOMPARE(copied.animations.value(object.gifDisplay.animation), gifFixture(true)); QVERIFY(copied.objects.first().gifDisplay.animation.startsWith(QStringLiteral("red-green.gif~"))); QCOMPARE(copied.references.first().targetId, copied.objects.first().gifDisplay.animation);
    QVERIFY2(copied.copyPage(copied.activePageId, QStringLiteral("复制GIF页"), &newPage, &error), qPrintable(error)); QCOMPARE(animationUsage(copied, copied.objects.first().gifDisplay.animation), 4);
    options.columns = 2; options.increments.append({Stage3AddressField::MonitorBit, 1, 0}); QVERIFY2(duplicateStage3Selection(source, {object.id}, options, &duplicated, nullptr, &error), qPrintable(error)); QCOMPARE(duplicated.objects.last().gifDisplay.controlAddress, QStringLiteral("LB18")); QCOMPARE(duplicated.objects.first().gifDisplay.controlAddress, QStringLiteral("LB17"));
    object.gifDisplay.controlAddress = QStringLiteral("LW0"); QVERIFY(!object.gifDisplay.validateAddresses(source.connections, &error)); object.gifDisplay.bitControl = false; QVERIFY2(object.gifDisplay.validateAddresses(source.connections, &error), qPrintable(error));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::gifPlacementCancellationAndUndoAreAtomic
 * Description     : Create through the real shell action, import in the child gallery and verify cancel, placement, edit and history resource atomicity.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::gifPlacementCancellationAndUndoAreAtomic() //20261112-0 add
{
    MainWindow window;
    HmiProject project;
    QTimer parentTimer;
    QTimer childTimer;
    bool configured = false;
    bool childAccepted = false;
    int history;
    QString path = QFINDTESTDATA("../docs/stage4-evidence/gif/native/probe-red-green.gif");
    window.setProject(project, QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); history = window.m_projectHistory.size();
    parentTimer.setInterval(15); childTimer.setInterval(15);
    connect(&childTimer, &QTimer::timeout, &window, [&]() {
        AnimationGalleryDialog *gallery = qobject_cast<AnimationGalleryDialog *>(QApplication::activeModalWidget());
        QString error;
        if (gallery == nullptr) { return; } QVERIFY2(gallery->importFiles({path}, &error), qPrintable(error)); childAccepted = true; childTimer.stop(); gallery->findChild<QDialogButtonBox *>(QStringLiteral("AnimationButtons"))->button(QDialogButtonBox::Ok)->click();
    });
    connect(&parentTimer, &QTimer::timeout, &window, [&]() {
        GifDisplayDialog *dialog = qobject_cast<GifDisplayDialog *>(QApplication::activeModalWidget());
        if (dialog == nullptr || configured) { return; } configured = true; parentTimer.stop(); dialog->findChild<QPushButton *>(QStringLiteral("GifChoose"))->click(); dialog->findChild<QDialogButtonBox *>(QStringLiteral("GifButtons"))->button(QDialogButtonBox::Ok)->click();
    });
    parentTimer.start(); childTimer.start(); window.m_commands.value(QStringLiteral("action_GIFoutput"))->trigger(); QVERIFY(configured); QVERIFY(childAccepted); QVERIFY(window.m_canvas->hasPendingPlacement()); QVERIFY(window.currentEditorProject().animations.isEmpty());
    QTest::keyClick(window.m_canvas, Qt::Key_Escape); QVERIFY(!window.m_canvas->hasPendingPlacement()); QVERIFY(window.currentEditorProject().objects.isEmpty()); QVERIFY(window.currentEditorProject().animations.isEmpty()); QCOMPARE(window.m_projectHistory.size(), history);
    configured = false; childAccepted = false; parentTimer.start(); childTimer.start(); window.m_commands.value(QStringLiteral("action_GIFoutput"))->trigger(); QVERIFY(configured); QVERIFY(childAccepted);
    QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(180, 120))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(180, 120)));
    QCOMPARE(window.currentEditorProject().objects.size(), 1); QCOMPARE(window.currentEditorProject().animations.size(), 1); QCOMPARE(window.currentEditorProject().objects.first().gifDisplay.gifData, gifFixture()); QCOMPARE(window.m_projectHistory.size(), history + 1);
    QTimer::singleShot(0, &window, []() { GifDisplayDialog *dialog = qobject_cast<GifDisplayDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QCheckBox *>(QStringLiteral("GifBitControl"))->setChecked(true); dialog->findChild<QLineEdit *>(QStringLiteral("GifControlAddress"))->setText(QStringLiteral("LB17")); dialog->findChild<QDialogButtonBox *>(QStringLiteral("GifButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().gifDisplay.controlAddress, QStringLiteral("LB17")); QCOMPARE(window.m_projectHistory.size(), history + 2);
    QTimer::singleShot(0, &window, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } }); window.editStage4Control(); QCOMPARE(window.m_projectHistory.size(), history + 2);
    window.restoreProjectHistory(-1); QVERIFY(!window.currentEditorProject().objects.first().gifDisplay.bitControl); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty()); QVERIFY(window.currentEditorProject().animations.isEmpty()); window.restoreProjectHistory(1); window.restoreProjectHistory(1); QCOMPARE(window.currentEditorProject().objects.first().gifDisplay.controlAddress, QStringLiteral("LB17"));
}

/******************************************************************************
 * Function Name   : numberObject
 * Description     : Build a typed numeric object distinct from the legacy number placeholder and switch family.
 * Input Parameters: None
 * Return Value    : Default numeric editor object.
 * Date            : 2026.11.12
 ******************************************************************************/
static HmiObject numberObject() //20261112-0 add
{
    HmiObject object;
    object.type = QStringLiteral("number"); object.id = QStringLiteral("number-object"); object.name = QStringLiteral("数值显示器"); object.geometry = QRect(40, 100, 180, 80); object.address = QStringLiteral("LW0"); object.hasNumberDisplayProperties = true; return object;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::numberNativeDefaultsAndInputLinkage
 * Description     : Check the corrected native defaults and input/monitor/offset draft enablement through real widgets.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::numberNativeDefaultsAndInputLinkage() //20261112-0 add
{
    NumberDisplayDialog dialog(numberObject(), HmiProject());
    QLineEdit *monitor = dialog.findChild<QLineEdit *>(QStringLiteral("Number_monitorAddress"));
    QLineEdit *input = dialog.findChild<QLineEdit *>(QStringLiteral("Number_inputAddress"));
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(dialog.windowTitle(), QStringLiteral("数值输入")); QCOMPARE(dialog.findChild<QTabWidget *>(QStringLiteral("NumberTabs"))->count(), 4);
    QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("Number_dataType"))->count(), 14); QVERIFY(dialog.findChild<QCheckBox *>(QStringLiteral("Number_displayAddress"))->isChecked()); QVERIFY(!dialog.findChild<QGroupBox *>(QStringLiteral("Number_macroEnabled"))->isEnabled());
    QCOMPARE(dialog.object().numberDisplay.backgroundColor, QStringLiteral("#a0a0ff")); QVERIFY(!input->isEnabled()); monitor->setText(QStringLiteral("LW17")); QCOMPARE(input->text(), QStringLiteral("LW17"));
    dialog.findChild<QCheckBox *>(QStringLiteral("Number_sameMonitor"))->setChecked(false); QVERIFY(input->isEnabled()); input->setText(QStringLiteral("LW25")); dialog.findChild<QCheckBox *>(QStringLiteral("Number_sameMonitor"))->setChecked(true); QCOMPARE(input->text(), QStringLiteral("LW17")); QCOMPARE(dialog.object().numberDisplay.inputAddress, QStringLiteral("LW25"));
    dialog.findChild<QCheckBox *>(QStringLiteral("Number_sameMonitor"))->setChecked(false); QCOMPARE(input->text(), QStringLiteral("LW25")); dialog.findChild<QCheckBox *>(QStringLiteral("Number_allowInput"))->setChecked(false); QCOMPARE(dialog.windowTitle(), QStringLiteral("数值显示")); QVERIFY(!input->isEnabled()); QVERIFY(!dialog.findChild<QCheckBox *>(QStringLiteral("Number_password"))->isEnabled());
    dialog.findChild<QCheckBox *>(QStringLiteral("Number_offsetEnabled"))->setChecked(true); QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("Number_offsetAddress"))->isVisible());
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("number-basic.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::numberAllDataTypeTransitions
 * Description     : Compare every measured transition, including stale VP drafts and differing 32-bit octal input/display ceilings.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::numberAllDataTypeTransitions() //20261112-0 add
{
    NumberDisplayDialog dialog(numberObject(), HmiProject());
    QComboBox *type = dialog.findChild<QComboBox *>(QStringLiteral("Number_dataType"));
    QSpinBox *digits = dialog.findChild<QSpinBox *>(QStringLiteral("Number_totalDigits"));
    QSpinBox *decimals = dialog.findChild<QSpinBox *>(QStringLiteral("Number_decimalDigits"));
    QFile file(QFINDTESTDATA("../docs/stage4-evidence/number/native/findings.json"));
    QJsonArray matrix;
    QJsonValue value;
    QJsonObject row;
    int index;
    QVERIFY(file.open(QIODevice::ReadOnly)); matrix = QJsonDocument::fromJson(file.readAll()).object().value(QStringLiteral("dataTypeTransitionObservations")).toArray(); QCOMPARE(matrix.size(), 14);
    Q_FOREACH (value, matrix)
    {
        row = value.toObject(); index = row.value(QStringLiteral("uiIndex")).toInt(); type->setCurrentIndex(index); QCOMPARE(type->currentText(), row.value(QStringLiteral("label")).toString());
        QCOMPARE(dialog.object().numberDisplay.inputMinimum, row.value(QStringLiteral("inputMinimumText")).toString()); QCOMPARE(dialog.object().numberDisplay.inputMaximum, row.value(QStringLiteral("inputMaximumText")).toString());
        QCOMPARE(digits->maximum(), row.value(QStringLiteral("totalDigits")).toObject().value(QStringLiteral("maximum")).toString().toInt()); QCOMPARE(digits->value(), row.value(QStringLiteral("totalDigits")).toObject().value(QStringLiteral("value")).toString().toInt());
        QCOMPARE(decimals->isEnabled(), row.value(QStringLiteral("decimalDigits")).toObject().value(QStringLiteral("enabled")).toInt() == 1);
    }
    type->setCurrentIndex(9); QCOMPARE(dialog.object().numberDisplay.inputMaximum, QStringLiteral("17777777777")); QCOMPARE(dialog.object().numberDisplay.displayMaximum, QStringLiteral("37777777777"));
    QCOMPARE(decimals->maximum(), 99); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("Number_fontSize"))->count(), 8); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("Number_font"))->count(), 3);
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::numberRangesAndActiveAddressSpans
 * Description     : Validate signed/unsigned/BCD/radix/float ranges and active 32-bit carriers without rejecting disabled address drafts.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::numberRangesAndActiveAddressSpans() //20261112-0 add
{
    NumberDisplayProperties properties;
    QString error;
    int index;
    for (index = 0; index < 12; ++index) { properties.changeDataType(index); QVERIFY2(properties.validate(&error), qPrintable(error)); }
    properties.changeDataType(6); properties.inputMaximum = QStringLiteral("4294967296"); QVERIFY(!properties.validate(&error)); properties.inputMaximum = QStringLiteral("4294967295"); properties.monitorAddress = QStringLiteral("LW4096"); QVERIFY(!properties.validateAddresses({}, &error));
    properties.changeDataType(1); QVERIFY2(properties.validateAddresses({}, &error), qPrintable(error)); properties.sameMonitor = false; properties.inputAddress = QStringLiteral("invalid"); QVERIFY(!properties.validateAddresses({}, &error)); properties.allowInput = false; QVERIFY2(properties.validateAddresses({}, &error), qPrintable(error));
    properties.scaling = true; properties.gain = QStringLiteral("0"); QVERIFY(!properties.validate(&error)); properties.gain = QStringLiteral("2.5"); QVERIFY2(properties.validate(&error), qPrintable(error)); properties.offsetEnabled = true; properties.offsetAddress = QStringLiteral("LB0"); QVERIFY(!properties.validateAddresses({}, &error));
    properties.offsetEnabled = false; properties.displayRange = true; properties.displayMinimum = QStringLiteral("20"); properties.displayMaximum = QStringLiteral("10"); QVERIFY(!properties.validate(&error)); properties.displayRange = false; QVERIFY2(properties.validate(&error), qPrintable(error));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::numberPermissionNotificationAndKeyboardLifecycle
 * Description     : Check permission levels, active notifications and stable keyboard references through page rename/delete and cross-project copy.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::numberPermissionNotificationAndKeyboardLifecycle() //20261112-0 add
{
    HmiProject project;
    HmiProject copied;
    HmiPage keyboard;
    HmiObject object = numberObject();
    Stage3Clipboard clipboard;
    NumberDisplayDialog *dialog;
    QString error;
    keyboard.id = QStringLiteral("number-keyboard"); keyboard.name = QStringLiteral("数字键盘"); keyboard.kind = QStringLiteral("keyboard"); keyboard.width = 300; keyboard.height = 200; QVERIFY2(project.addPage(keyboard, &error), qPrintable(error));
    dialog = new NumberDisplayDialog(object, project); QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("NumberVisibilityMinimum"))->currentText(), QStringLiteral("1")); QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("NumberTouchMinimum"))->currentText(), QStringLiteral("2"));
    dialog->findChild<QGroupBox *>(QStringLiteral("Number_notice"))->setChecked(true); QVERIFY(dialog->findChild<QCheckBox *>(QStringLiteral("Number_noticeBit"))->isEnabled()); dialog->findChild<QCheckBox *>(QStringLiteral("Number_noticeBit"))->setChecked(true); dialog->findChild<QCheckBox *>(QStringLiteral("Number_noticeWord"))->setChecked(true); QVERIFY(!dialog->findChild<QCheckBox *>(QStringLiteral("Number_noticeBit"))->isChecked());
    dialog->findChild<QCheckBox *>(QStringLiteral("Number_customKeyboard"))->setChecked(true); dialog->findChild<QComboBox *>(QStringLiteral("Number_keyboardPageId"))->setCurrentIndex(0); object = dialog->object(); delete dialog;
    project.objects = {object}; QVERIFY2(project.validateModel(&error), qPrintable(error)); QVERIFY2(project.renamePage(keyboard.id, QStringLiteral("重命名键盘"), &error), qPrintable(error)); QCOMPARE(project.objects.first().numberDisplay.keyboardPageId, keyboard.id); QVERIFY(!project.removePage(keyboard.id, &error));
    QVERIFY2(captureStage3Selection(project, {object.id}, &clipboard, &error), qPrintable(error)); QVERIFY(!pasteStage3Selection(HmiProject(), clipboard, QPoint(), &copied, nullptr, &error)); QVERIFY(copied.objects.isEmpty());
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::numberStrictPersistenceLegacyAndBackendGates
 * Description     : Preserve all eleven mixed control families and numeric drafts while rejecting malformed fields and unrecovered native output.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::numberStrictPersistenceLegacyAndBackendGates() //20261112-0 add
{
    HmiProject project;
    HmiProject loaded;
    HmiObject object = numberObject();
    HmiObject parsed = qrObject();
    HmiObject legacy;
    QJsonObject json;
    QJsonObject properties;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    object.numberDisplay.inputAddress = QStringLiteral("LW25"); object.numberDisplay.decimalDigits = 7; object.numberDisplay.customText = QStringLiteral("草稿123");
    project.objects = existingFiveObjects(); project.objects.append({messageObject(), scaleObject(), qrObject(), imageObject(), gifObject(), object}); QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.objects.size(), 11); QCOMPARE(loaded.toJson(), project.toJson());
    json = object.toJson(); properties = json.value(QStringLiteral("numberDisplay")).toObject(); properties.insert(QStringLiteral("dataType"), 8.5); json.insert(QStringLiteral("numberDisplay"), properties); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); QCOMPARE(parsed.type, QStringLiteral("qrcode"));
    properties = object.numberDisplay.toJson(); properties.remove(QStringLiteral("inputMaximum")); json.insert(QStringLiteral("numberDisplay"), properties); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); legacy = object; legacy.hasNumberDisplayProperties = false; QVERIFY2(HmiObject::fromJson(legacy.toJson(), &parsed, &error), qPrintable(error)); QVERIFY(!parsed.hasNumberDisplayProperties);
    project.objects = {numberObject()}; QVERIFY(!exportNativeProjectSource(project, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(project, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")});
    json = project.toJson(); json.insert(QStringLiteral("schemaVersion"), 11); json.remove(QStringLiteral("animations")); QVERIFY(!HmiProject::fromJson(json, &project, &error)); QCOMPARE(project.objects.first().type, QStringLiteral("number"));
    object.numberDisplay.keyboardPageId = QStringLiteral("disabled-missing-keyboard");
    { NumberDisplayDialog dormant(object, HmiProject()); QCOMPARE(dormant.object().numberDisplay.keyboardPageId, object.numberDisplay.keyboardPageId); }
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY2(loaded.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("number-mixed.ksp")), &error), qPrintable(error)); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::numberPicturesCopiesAndAddressRoles
 * Description     : Protect numeric artwork on inactive pages and preserve independent monitor/input/range address increments.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::numberPicturesCopiesAndAddressRoles() //20261112-0 add
{
    HmiProject source;
    HmiProject result;
    HmiProject target;
    HmiPage page;
    HmiObject object = numberObject();
    Stage3Clipboard clipboard;
    Stage3DuplicateOptions options;
    QString error;
    object.numberDisplay.appearance = {QStringLiteral("number.png"), imagePng(Qt::green)}; object.numberDisplay.sameMonitor = false; object.numberDisplay.inputAddress = QStringLiteral("LW25"); object.numberDisplay.inputRangeSource = 1; object.numberDisplay.inputMinimumAddress = QStringLiteral("LW30"); source.objects = {object}; source.pictures.insert(QStringLiteral("number.png"), imagePng(Qt::green));
    page.id = QStringLiteral("number-second-page"); page.name = QStringLiteral("数值第二页"); object.id = QStringLiteral("inactive-number"); page.objects.append(object); QVERIFY2(source.addPage(page, &error), qPrintable(error)); QCOMPARE(stage3PictureUsage(source, QStringLiteral("number.png")).boundStates, 2); QVERIFY(!deleteStage3Picture(source, QStringLiteral("number.png"), &result, &error));
    QVERIFY2(renameStage3Picture(source, QStringLiteral("number.png"), QStringLiteral("renamed-number.png"), &result, &error), qPrintable(error)); QCOMPARE(result.inactivePages.first().objects.first().numberDisplay.appearance.picture, QStringLiteral("renamed-number.png"));
    options.columns = 2; options.increments = {{Stage3AddressField::MonitorWord, 2, 0}, {Stage3AddressField::Operation, 3, 0}, {Stage3AddressField::InputMinimumWord, 4, 0}}; QVERIFY2(duplicateStage3Selection(source, {source.objects.first().id}, options, &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.objects.last().numberDisplay.monitorAddress, QStringLiteral("LW2")); QCOMPARE(result.objects.last().numberDisplay.inputAddress, QStringLiteral("LW28")); QCOMPARE(result.objects.last().numberDisplay.inputMinimumAddress, QStringLiteral("LW34"));
    target.pictures.insert(QStringLiteral("number.png"), imagePng(Qt::red)); QVERIFY2(captureStage3Selection(source, {source.objects.first().id}, &clipboard, &error), qPrintable(error)); QVERIFY2(pasteStage3Selection(target, clipboard, QPoint(), &result, nullptr, &error), qPrintable(error)); QVERIFY(result.objects.first().numberDisplay.appearance.picture.startsWith(QStringLiteral("number.png~"))); QCOMPARE(result.objects.first().numberDisplay.appearance.picturePng, imagePng(Qt::green));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::numberActualRasterAndHiddenAppearanceDrafts
 * Description     : Verify the measured background sample, clipping and actual font/password/custom rendering without claiming firmware pixels.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::numberActualRasterAndHiddenAppearanceDrafts() //20261112-0 add
{
    NumberDisplayDialog dialog(numberObject(), HmiProject());
    QImage raster(200, 120, QImage::Format_ARGB32);
    QPainter painter;
    NumberDisplayProperties properties;
    QImage before;
    raster.fill(Qt::magenta); painter.begin(&raster); paintNumberDisplay(&painter, QRectF(10, 10, 180, 100), properties); painter.end(); QCOMPARE(raster.pixelColor(12, 12), QColor(QStringLiteral("#a0a0ff"))); QCOMPARE(raster.pixelColor(0, 0), QColor(Qt::magenta)); before = raster;
    properties.password = true; painter.begin(&raster); paintNumberDisplay(&painter, QRectF(10, 10, 180, 100), properties); painter.end(); QVERIFY(raster != before);
    dialog.findChild<QTabWidget *>(QStringLiteral("NumberTabs"))->setCurrentIndex(1); dialog.findChild<QCheckBox *>(QStringLiteral("Number_customDisplay"))->setChecked(true); dialog.findChild<QLineEdit *>(QStringLiteral("Number_customText"))->setText(QStringLiteral("12345")); dialog.findChild<QCheckBox *>(QStringLiteral("Number_customDisplay"))->setChecked(false); QCOMPARE(dialog.object().numberDisplay.customText, QStringLiteral("12345"));
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR"))
    {
        QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("number-display.png")))); dialog.findChild<QTabWidget *>(QStringLiteral("NumberTabs"))->setCurrentIndex(2); QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("number-permissions.png")))); dialog.findChild<QTabWidget *>(QStringLiteral("NumberTabs"))->setCurrentIndex(3); QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("number-other.png"))));
    }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::numberShellPlacementEditAndUndo
 * Description     : Execute the real native-shell action, create/edit/cancel and undo, including the compact typed monitor editor.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::numberShellPlacementEditAndUndo() //20261112-0 add
{
    MainWindow window;
    int history;
    window.setProject(HmiProject(), QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); history = window.m_projectHistory.size();
    QTimer::singleShot(0, &window, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } }); window.m_commands.value(QStringLiteral("action_dataDisplay"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement());
    QTimer::singleShot(0, &window, []() { NumberDisplayDialog *dialog = qobject_cast<NumberDisplayDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QDialogButtonBox *>(QStringLiteral("NumberButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.m_commands.value(QStringLiteral("action_dataDisplay"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement());
    QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(180, 120))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(180, 120))); QCOMPARE(window.currentEditorProject().objects.size(), 1); QVERIFY(window.currentEditorProject().objects.first().hasNumberDisplayProperties); QCOMPARE(window.m_projectHistory.size(), history + 1);
    QTimer::singleShot(0, &window, []() { NumberDisplayDialog *dialog = qobject_cast<NumberDisplayDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QCheckBox *>(QStringLiteral("Number_allowInput"))->setChecked(false); dialog->findChild<QDialogButtonBox *>(QStringLiteral("NumberButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.editStage4Control(); QVERIFY(!window.currentEditorProject().objects.first().numberDisplay.allowInput); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.first().numberDisplay.allowInput);
    window.m_canvas->selectObject(window.currentEditorProject().objects.first().id); QVERIFY(window.m_addressEdit->isEnabled()); //20261112-0 change: Restoring document history clears selection; explicitly select the object before editing its compact fields.
    window.m_addressEdit->setText(QStringLiteral("LW17")); window.applySelectedProperties(); QCOMPARE(window.currentEditorProject().objects.first().numberDisplay.monitorAddress, QStringLiteral("LW17")); QVERIFY(!window.m_textEdit->isEnabled()); window.restoreProjectHistory(-1); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty());
}

/******************************************************************************
 * Function Name   : asciiObject
 * Description     : Build a standalone ASCII control with independent string options and a shared visual/access draft.
 * Input Parameters: None
 * Return Value    : Default typed ASCII object.
 * Date            : 2026.11.12
 ******************************************************************************/
static HmiObject asciiObject() //20261112-0 add
{
    HmiObject object;
    object.type = QStringLiteral("ascii"); object.id = QStringLiteral("ascii-object"); object.name = QStringLiteral("ASCII显示器"); object.geometry = QRect(260, 100, 180, 80); object.address = QStringLiteral("LW0"); return object;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::asciiNativeDefaultsAndDistinctFields
 * Description     : Verify corrected native defaults, ASCII-specific bounds and enablement without exposing numeric-only controls.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::asciiNativeDefaultsAndDistinctFields() //20261112-0 add
{
    NumberDisplayDialog dialog(asciiObject(), HmiProject());
    QTabWidget *tabs = dialog.findChild<QTabWidget *>(QStringLiteral("NumberTabs"));
    QSpinBox *characters = dialog.findChild<QSpinBox *>(QStringLiteral("Number_asciiCharacterCount"));
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(dialog.windowTitle(), QStringLiteral("ASCII输入")); QCOMPARE(tabs->count(), 4); QCOMPARE(dialog.object().type, QStringLiteral("ascii")); QVERIFY(!dialog.object().hasNumberDisplayProperties);
    QVERIFY(dialog.findChild<QCheckBox *>(QStringLiteral("Number_asciiSwapBytes"))->isChecked()); QVERIFY(!dialog.findChild<QCheckBox *>(QStringLiteral("Number_asciiAutoWrap"))->isChecked()); QVERIFY(dialog.findChild<QCheckBox *>(QStringLiteral("Number_displayAddress"))->isChecked());
    QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("Number_asciiEncoding"))->count(), 2); QVERIFY(!dialog.findChild<QComboBox *>(QStringLiteral("Number_dataType"))->isVisible()); QVERIFY(!dialog.findChild<QGroupBox *>(QStringLiteral("Number_scaling"))->isVisible()); QVERIFY(!dialog.findChild<QGroupBox *>(QStringLiteral("Number_macroEnabled"))->isEnabled());
    dialog.findChild<QCheckBox *>(QStringLiteral("Number_allowInput"))->setChecked(false); QCOMPARE(dialog.windowTitle(), QStringLiteral("ASCII显示")); QVERIFY(!dialog.findChild<QComboBox *>(QStringLiteral("Number_asciiEncoding"))->isEnabled());
    tabs->setCurrentIndex(2); QVERIFY(!dialog.findChild<QGroupBox *>(QStringLiteral("Number_notice"))->isVisible()); dialog.findChild<QCheckBox *>(QStringLiteral("Number_allowInput"))->setChecked(true); QVERIFY(dialog.findChild<QGroupBox *>(QStringLiteral("Number_notice"))->isVisible());
    tabs->setCurrentIndex(1); QCOMPARE(characters->value(), 5); QCOMPARE(characters->minimum(), 1); QCOMPARE(characters->maximum(), 256); QVERIFY(!dialog.findChild<QSpinBox *>(QStringLiteral("Number_decimalDigits"))->isEnabled()); QVERIFY(!dialog.findChild<QComboBox *>(QStringLiteral("Number_adjustment"))->isEnabled()); QVERIFY(!dialog.findChild<QGroupBox *>(QStringLiteral("Number_displayRange"))->isVisible()); QVERIFY(!dialog.findChild<QPushButton *>(QStringLiteral("Number_lowColorPicker"))->isEnabled());
    characters->setValue(256); QCOMPARE(dialog.object().asciiDisplay.characterCount, 256); dialog.findChild<QComboBox *>(QStringLiteral("Number_asciiEncoding"))->setCurrentIndex(1); QVERIFY(dialog.object().asciiDisplay.swapBytes); QCOMPARE(dialog.object().asciiDisplay.encoding, 1);
    QCOMPARE(dialog.findChild<QLabel *>(QStringLiteral("Number_textColorLabel"))->text(), QStringLiteral("文本颜色:")); QCOMPARE(dialog.findChild<QLabel *>(QStringLiteral("Number_backgroundColorLabel"))->text(), QStringLiteral("文本背景颜色:")); //20261112-0 add
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("ascii-display.png")))); tabs->setCurrentIndex(0); QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("ascii-basic.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::asciiNumericSwitchAndDormantDrafts
 * Description     : Switch actual family radios without losing string options or invalid but inactive numeric range/scaling drafts.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::asciiNumericSwitchAndDormantDrafts() //20261112-0 add
{
    NumberDisplayDialog dialog(numberObject(), HmiProject());
    HmiObject object;
    HmiObject parsed;
    QString error;
    dialog.findChild<QComboBox *>(QStringLiteral("Number_dataType"))->setCurrentIndex(8); dialog.findChild<QLineEdit *>(QStringLiteral("Number_inputMinimum"))->setText(QStringLiteral("dormant-range")); dialog.findChild<QGroupBox *>(QStringLiteral("Number_scaling"))->setChecked(true); dialog.findChild<QLineEdit *>(QStringLiteral("Number_gain"))->setText(QStringLiteral("0"));
    dialog.findChild<QRadioButton *>(QStringLiteral("NumberFamily1"))->click(); QCOMPARE(dialog.windowTitle(), QStringLiteral("ASCII输入")); dialog.findChild<QSpinBox *>(QStringLiteral("Number_asciiCharacterCount"))->setValue(128); dialog.findChild<QCheckBox *>(QStringLiteral("Number_asciiSwapBytes"))->setChecked(false); dialog.findChild<QCheckBox *>(QStringLiteral("Number_asciiModifyInput"))->setChecked(true);
    object = dialog.object(); QVERIFY2(object.asciiDisplay.validate(&error), qPrintable(error)); QCOMPARE(object.asciiDisplay.common.inputMinimum, QStringLiteral("dormant-range")); QVERIFY2(HmiObject::fromJson(object.toJson(), &parsed, &error), qPrintable(error)); QCOMPARE(parsed.toJson(), object.toJson());
    dialog.findChild<QRadioButton *>(QStringLiteral("NumberFamily0"))->click(); object = dialog.object(); QVERIFY(object.hasNumberDisplayProperties); QCOMPARE(object.numberDisplay.dataType, 8); QCOMPARE(object.numberDisplay.gain, QStringLiteral("0")); QVERIFY(!object.numberDisplay.validate(&error));
    dialog.findChild<QRadioButton *>(QStringLiteral("NumberFamily1"))->click(); QCOMPARE(dialog.object().asciiDisplay.characterCount, 128); QVERIFY(!dialog.object().asciiDisplay.swapBytes); QVERIFY(dialog.object().asciiDisplay.modifyInput);
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::asciiStrictMixedPersistenceAndBackendGates
 * Description     : Round-trip twelve control families and reject malformed ASCII records, version downgrades and unrecovered native output atomically.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::asciiStrictMixedPersistenceAndBackendGates() //20261112-0 add
{
    HmiProject project;
    HmiProject loaded;
    HmiProject only;
    HmiObject object = asciiObject();
    HmiObject parsed = qrObject();
    QJsonObject json;
    QJsonObject props;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    object.asciiDisplay.encoding = 1; object.asciiDisplay.characterCount = 255; object.asciiDisplay.autoWrap = true; object.asciiDisplay.modifyInput = true; object.asciiDisplay.common.keyboardPageId = QStringLiteral("dormant-keyboard");
    project.objects = existingFiveObjects(); project.objects.append({messageObject(), scaleObject(), qrObject(), imageObject(), gifObject(), numberObject(), object}); QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.objects.size(), 12); QCOMPARE(loaded.toJson(), project.toJson());
    json = object.toJson(); props = object.asciiDisplay.toJson(); props.insert(QStringLiteral("characterCount"), 4.5); json.insert(QStringLiteral("asciiDisplay"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); QCOMPARE(parsed.type, QStringLiteral("qrcode"));
    props = object.asciiDisplay.toJson(); props.insert(QStringLiteral("encoding"), 2); json.insert(QStringLiteral("asciiDisplay"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error));
    only.objects = {object}; json = only.toJson(); json.insert(QStringLiteral("schemaVersion"), 11); json.remove(QStringLiteral("animations")); QVERIFY(!HmiProject::fromJson(json, &only, &error)); QCOMPARE(only.objects.first().type, QStringLiteral("ascii"));
    QVERIFY(!exportNativeProjectSource(only, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(only, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")});
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY2(loaded.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("ascii-mixed.ksp")), &error), qPrintable(error)); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::asciiResourcesAddressesAndGalleryTransaction
 * Description     : Exercise active address roles, cross-page resource protection and the actual nested ASCII artwork selector.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::asciiResourcesAddressesAndGalleryTransaction() //20261112-0 add
{
    HmiProject source;
    HmiProject result;
    HmiPage page;
    HmiObject object = asciiObject();
    Stage3DuplicateOptions options;
    NumberDisplayDialog *dialog;
    QTimer child;
    bool accepted = false;
    QString error;
    object.asciiDisplay.common.appearance = {QStringLiteral("ascii.png"), imagePng(Qt::green)}; object.asciiDisplay.common.sameMonitor = false; object.asciiDisplay.common.inputAddress = QStringLiteral("LW20"); object.asciiDisplay.common.offsetEnabled = true; object.asciiDisplay.common.offsetAddress = QStringLiteral("LW30"); source.pictures.insert(QStringLiteral("ascii.png"), imagePng(Qt::green)); source.objects.append(object);
    page.id = QStringLiteral("ascii-second-page"); page.name = QStringLiteral("ASCII第二页"); object.id = QStringLiteral("ascii-inactive"); page.objects.append(object); QVERIFY2(source.addPage(page, &error), qPrintable(error)); QCOMPARE(stage3PictureUsage(source, QStringLiteral("ascii.png")).boundStates, 2); QVERIFY(!deleteStage3Picture(source, QStringLiteral("ascii.png"), &result, &error));
    QVERIFY2(renameStage3Picture(source, QStringLiteral("ascii.png"), QStringLiteral("renamed-ascii.png"), &result, &error), qPrintable(error)); QCOMPARE(result.inactivePages.first().objects.first().asciiDisplay.common.appearance.picture, QStringLiteral("renamed-ascii.png"));
    options.columns = 2; options.increments = {{Stage3AddressField::MonitorWord, 1, 0}, {Stage3AddressField::Operation, 2, 0}, {Stage3AddressField::OffsetWord, 3, 0}}; QVERIFY2(duplicateStage3Selection(source, {source.objects.first().id}, options, &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.objects.last().asciiDisplay.common.monitorAddress, QStringLiteral("LW1")); QCOMPARE(result.objects.last().asciiDisplay.common.inputAddress, QStringLiteral("LW22")); QCOMPARE(result.objects.last().asciiDisplay.common.offsetAddress, QStringLiteral("LW33"));
    dialog = new NumberDisplayDialog(source.objects.first(), source); dialog->findChild<QPushButton *>(QStringLiteral("NumberPicture2"))->click(); child.setInterval(15);
    connect(&child, &QTimer::timeout, dialog, [&]() { QDialog *gallery = qobject_cast<QDialog *>(QApplication::activeModalWidget()); QListWidget *items; if (gallery == nullptr || gallery->objectName() != QStringLiteral("BitGallery")) { return; } items = gallery->findChild<QListWidget *>(QStringLiteral("GalleryItems")); if (items == nullptr || items->count() != 1) { return; } items->setCurrentRow(0); accepted = true; child.stop(); gallery->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click(); }); child.start();
    dialog->findChild<QPushButton *>(QStringLiteral("NumberPicture1"))->click(); QVERIFY(accepted); QCOMPARE(dialog->object().asciiDisplay.common.appearance.picturePng, imagePng(Qt::green)); dialog->reject(); delete dialog; QCOMPARE(source.objects.first().asciiDisplay.common.appearance.picture, QStringLiteral("ascii.png"));
    object = asciiObject(); object.asciiDisplay.common.visibility.visibilityEnabled = true; object.asciiDisplay.common.visibility.wordEnabled = true; object.asciiDisplay.common.visibility.dataType = 1; object.asciiDisplay.common.visibility.wordAddress = QStringLiteral("LW4096"); QVERIFY(!object.asciiDisplay.validateAddresses({}, &error)); object.asciiDisplay.common.visibility.visibilityEnabled = false; QVERIFY2(object.asciiDisplay.validateAddresses({}, &error), qPrintable(error));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::asciiSampleRasterAndOptionIsolation
 * Description     : Check real sample pixels, clipping and wrap/password changes while keeping unobserved byte/encoding runtime behavior out of the preview.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::asciiSampleRasterAndOptionIsolation() //20261112-0 add
{
    AsciiDisplayProperties properties;
    QImage raster(120, 100, QImage::Format_ARGB32);
    QImage original;
    QPainter painter;
    raster.fill(Qt::magenta); painter.begin(&raster); paintAsciiDisplay(&painter, QRectF(10, 10, 100, 80), properties); painter.end(); QCOMPARE(raster.pixelColor(0, 0), QColor(Qt::magenta)); QCOMPARE(raster.pixelColor(12, 12), QColor(QStringLiteral("#a0a0ff"))); original = raster;
    properties.swapBytes = false; properties.encoding = 1; painter.begin(&raster); paintAsciiDisplay(&painter, QRectF(10, 10, 100, 80), properties); painter.end(); QCOMPARE(raster, original);
    properties.common.password = true; painter.begin(&raster); paintAsciiDisplay(&painter, QRectF(10, 10, 100, 80), properties); painter.end(); QVERIFY(raster != original); properties.common.password = false; properties.autoWrap = true; properties.characterCount = 32; painter.begin(&raster); paintAsciiDisplay(&painter, QRectF(10, 10, 100, 80), properties); painter.end(); QVERIFY(raster != original);
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::asciiShellPlacementConversionAndHistory
 * Description     : Execute the real ASCII entry, placement and accepted family conversion as one undoable object transaction.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::asciiShellPlacementConversionAndHistory() //20261112-0 add
{
    MainWindow window;
    int history;
    window.setProject(HmiProject(), QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); history = window.m_projectHistory.size();
    QTimer::singleShot(0, &window, []() { NumberDisplayDialog *dialog = qobject_cast<NumberDisplayDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QDialogButtonBox *>(QStringLiteral("NumberButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.m_commands.value(QStringLiteral("action_asciiDisplay"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement());
    QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(180, 120))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(180, 120))); QCOMPARE(window.currentEditorProject().objects.first().type, QStringLiteral("ascii")); QCOMPARE(window.m_projectHistory.size(), history + 1);
    QTimer::singleShot(0, &window, []() { NumberDisplayDialog *dialog = qobject_cast<NumberDisplayDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QRadioButton *>(QStringLiteral("NumberFamily0"))->click(); dialog->findChild<QDialogButtonBox *>(QStringLiteral("NumberButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().type, QStringLiteral("number")); QVERIFY(window.currentEditorProject().objects.first().hasNumberDisplayProperties); QCOMPARE(window.m_projectHistory.size(), history + 2);
    window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().objects.first().type, QStringLiteral("ascii")); window.m_canvas->selectObject(window.currentEditorProject().objects.first().id); window.m_addressEdit->setText(QStringLiteral("LW9")); window.applySelectedProperties(); QCOMPARE(window.currentEditorProject().objects.first().asciiDisplay.common.monitorAddress, QStringLiteral("LW9")); window.restoreProjectHistory(-1); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty());
}

/******************************************************************************
 * Function Name   : timeObject
 * Description     : Create a standalone RTC display with no numeric input or monitor carrier.
 * Input Parameters: None
 * Return Value    : Bounded time object.
 * Date            : 2026.11.12
 ******************************************************************************/
static HmiObject timeObject() //20261112-0 add
{
    HmiObject object;
    object.id = QStringLiteral("time-object"); object.type = QStringLiteral("time"); object.name = QStringLiteral("时间显示器"); object.geometry = QRect(450, 200, 220, 70); return object;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::timeNativeDefaultsAndThreePages
 * Description     : Verify measured clock-only pages, seven font sizes and absence of numeric/touch/input controls.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::timeNativeDefaultsAndThreePages() //20261112-0 add
{
    NumberDisplayDialog dialog(timeObject(), HmiProject());
    QTabWidget *tabs = dialog.findChild<QTabWidget *>(QStringLiteral("NumberTabs"));
    QComboBox *sizes = dialog.findChild<QComboBox *>(QStringLiteral("Number_timeFontSize"));
    int index;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(dialog.windowTitle(), QStringLiteral("时间显示")); QCOMPARE(tabs->count(), 3); QCOMPARE(dialog.object().type, QStringLiteral("time")); QVERIFY(dialog.object().address.isEmpty()); QVERIFY(!dialog.object().hasNumberDisplayProperties);
    QVERIFY(dialog.findChild<QCheckBox *>(QStringLiteral("Number_timeShowDate"))->isChecked()); QVERIFY(!dialog.findChild<QCheckBox *>(QStringLiteral("Number_timeShowWeek"))->isChecked()); QVERIFY(!dialog.findChild<QCheckBox *>(QStringLiteral("Number_timeShowTime"))->isChecked());
    QVERIFY(!dialog.findChild<QLineEdit *>(QStringLiteral("Number_monitorAddress"))->isVisible()); QVERIFY(!dialog.findChild<QCheckBox *>(QStringLiteral("Number_allowInput"))->isVisible()); QVERIFY(!dialog.findChild<QGroupBox *>(QStringLiteral("Number_macroEnabled"))->isVisible()); QCOMPARE(dialog.findChild<QLabel *>(QStringLiteral("TimeFormatSample"))->text(), QStringLiteral("YYYY/MM/DD"));
    tabs->setCurrentIndex(1); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("Number_timeFont"))->count(), 1); QCOMPARE(sizes->count(), 7); QCOMPARE(sizes->currentText(), QStringLiteral("16"));
    for (index = 0; index < 7; ++index) { QCOMPARE(sizes->itemText(index), TimeDisplayProperties::fontSizes().at(index)); }
    QVERIFY(!dialog.findChild<QSpinBox *>(QStringLiteral("Number_timeTotalDigits"))->isEnabled()); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("Number_timeTotalDigits"))->value(), 1); QVERIFY(!dialog.findChild<QSpinBox *>(QStringLiteral("Number_decimalDigits"))->isEnabled()); QVERIFY(!dialog.findChild<QGroupBox *>(QStringLiteral("Number_displayRange"))->isVisible());
    tabs->setCurrentIndex(2); QVERIFY(!dialog.findChild<HmiPermissionEditor *>(QStringLiteral("NumberTouch"))->isVisible()); QVERIFY(!dialog.findChild<QGroupBox *>(QStringLiteral("Number_notice"))->isVisible()); QVERIFY(!dialog.findChild<QCheckBox *>(QStringLiteral("Number_pressTimeEnabled"))->isVisible());
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("time-permissions.png")))); tabs->setCurrentIndex(0); QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("time-basic.png")))); tabs->setCurrentIndex(1); QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("time-display.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::timeFormatsImmediateGuardAndDraftRetention
 * Description     : Verify date/time combinations, enabled unchecked format drafts and the immediate all-hidden message.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::timeFormatsImmediateGuardAndDraftRetention() //20261112-0 add
{
    NumberDisplayDialog dialog(timeObject(), HmiProject());
    QCheckBox *date = dialog.findChild<QCheckBox *>(QStringLiteral("Number_timeShowDate"));
    QCheckBox *week = dialog.findChild<QCheckBox *>(QStringLiteral("Number_timeShowWeek"));
    QCheckBox *clock = dialog.findChild<QCheckBox *>(QStringLiteral("Number_timeShowTime"));
    QComboBox *dateFormat = dialog.findChild<QComboBox *>(QStringLiteral("Number_timeDateFormat"));
    QComboBox *timeFormat = dialog.findChild<QComboBox *>(QStringLiteral("Number_timeTimeFormat"));
    QLabel *sample = dialog.findChild<QLabel *>(QStringLiteral("TimeFormatSample"));
    QString last;
    QString error;
    bool immediate = false;
    int d;
    int t;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); week->setChecked(true); clock->setChecked(true);
    for (d = 0; d < 9; ++d) { for (t = 0; t < 4; ++t) { dateFormat->setCurrentIndex(d); timeFormat->setCurrentIndex(t); QCOMPARE(sample->text(), TimeDisplayProperties::dateFormats().at(d) + QStringLiteral(" (Mon) ") + TimeDisplayProperties::timeFormats().at(t)); } }
    date->setChecked(false); QVERIFY(dateFormat->isEnabled()); QCOMPARE(sample->text(), QStringLiteral("(Mon) HH-MM-SS")); week->setChecked(false); last = sample->text();
    QTimer::singleShot(0, &dialog, [&immediate]() { QMessageBox *message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget()); if (message != nullptr) { immediate = message->text() == QStringLiteral("不能隐藏所有时间和星期"); message->accept(); } }); clock->setChecked(false); QVERIFY(immediate); QCOMPARE(sample->text(), last); QVERIFY(!dialog.object().timeDisplay.validate(&error)); QVERIFY(timeFormat->isEnabled()); QVERIFY(dialog.isVisible());
    clock->setChecked(true); QCOMPARE(dialog.object().timeDisplay.dateFormat, 8); QCOMPARE(dialog.object().timeDisplay.timeFormat, 3); QVERIFY2(dialog.object().timeDisplay.validate(&error), qPrintable(error));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::timeFamilySwitchPreservesIndependentDrafts
 * Description     : Retain numeric, ASCII and time-only fields across all three family transitions in one dialog.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::timeFamilySwitchPreservesIndependentDrafts() //20261112-0 add
{
    NumberDisplayDialog dialog(numberObject(), HmiProject());
    QTabWidget *tabs = dialog.findChild<QTabWidget *>(QStringLiteral("NumberTabs"));
    QString error;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); dialog.findChild<QLineEdit *>(QStringLiteral("Number_inputMinimum"))->setText(QStringLiteral("-123")); dialog.findChild<QComboBox *>(QStringLiteral("Number_fontSize"))->setCurrentIndex(2);
    dialog.findChild<QRadioButton *>(QStringLiteral("NumberFamily2"))->click(); QCOMPARE(tabs->count(), 3); QCOMPARE(dialog.windowTitle(), QStringLiteral("时间显示")); dialog.findChild<QCheckBox *>(QStringLiteral("Number_timeShowWeek"))->setChecked(true); dialog.findChild<QComboBox *>(QStringLiteral("Number_timeDateFormat"))->setCurrentIndex(8); dialog.findChild<QComboBox *>(QStringLiteral("Number_timeFontSize"))->setCurrentIndex(0); QCOMPARE(dialog.object().timeDisplay.fontSize, 12); QVERIFY2(dialog.object().timeDisplay.validate(&error), qPrintable(error));
    dialog.findChild<QRadioButton *>(QStringLiteral("NumberFamily1"))->click(); QCOMPARE(tabs->count(), 4); dialog.findChild<QSpinBox *>(QStringLiteral("Number_asciiCharacterCount"))->setValue(19); dialog.findChild<QCheckBox *>(QStringLiteral("Number_asciiModifyInput"))->setChecked(true);
    dialog.findChild<QRadioButton *>(QStringLiteral("NumberFamily0"))->click(); QCOMPARE(dialog.object().numberDisplay.inputMinimum, QStringLiteral("-123")); QCOMPARE(dialog.object().numberDisplay.fontSize, 32);
    dialog.findChild<QRadioButton *>(QStringLiteral("NumberFamily2"))->click(); QCOMPARE(dialog.object().timeDisplay.dateFormat, 8); QCOMPARE(dialog.object().timeDisplay.fontSize, 12); QVERIFY(dialog.object().timeDisplay.showWeek); QVERIFY(!dialog.object().toJson().contains(QStringLiteral("numberDisplay"))); QVERIFY(!dialog.object().toJson().contains(QStringLiteral("asciiDisplay")));
    dialog.findChild<QRadioButton *>(QStringLiteral("NumberFamily1"))->click(); QCOMPARE(dialog.object().asciiDisplay.characterCount, 19); QVERIFY(dialog.object().asciiDisplay.modifyInput);
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::timeStrictMixedPersistenceAndBackendGates
 * Description     : Round-trip thirteen families and reject malformed clock records, schema downgrades and unknown output formats atomically.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::timeStrictMixedPersistenceAndBackendGates() //20261112-0 add
{
    HmiProject project;
    HmiProject loaded;
    HmiProject only;
    HmiObject object = timeObject();
    HmiObject parsed = qrObject();
    QJsonObject json;
    QJsonObject props;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    object.timeDisplay.showWeek = true; object.timeDisplay.showTime = true; object.timeDisplay.dateFormat = 8; object.timeDisplay.fontSize = 12;
    project.objects = existingFiveObjects(); project.objects.append({messageObject(), scaleObject(), qrObject(), imageObject(), gifObject(), numberObject(), asciiObject(), object}); QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.objects.size(), 13); QCOMPARE(loaded.toJson(), project.toJson());
    json = object.toJson(); props = object.timeDisplay.toJson(); props.insert(QStringLiteral("fontSize"), 32); json.insert(QStringLiteral("timeDisplay"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); QCOMPARE(parsed.type, QStringLiteral("qrcode")); props.insert(QStringLiteral("fontSize"), 12); props.insert(QStringLiteral("dateFormat"), 1.5); json.insert(QStringLiteral("timeDisplay"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error));
    json = object.toJson(); json.insert(QStringLiteral("address"), QStringLiteral("LW0")); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); json = object.toJson(); json.remove(QStringLiteral("timeDisplay")); QVERIFY(!HmiObject::fromJson(json, &parsed, &error));
    only.objects = {object}; json = only.toJson(); QCOMPARE(json.value(QStringLiteral("schemaVersion")).toInt(), 12); QVERIFY2(HmiProject::fromJson(json, &only, &error), qPrintable(error)); QCOMPARE(only.toJson(), json); json.insert(QStringLiteral("schemaVersion"), 11); json.remove(QStringLiteral("animations")); QVERIFY(!HmiProject::fromJson(json, &only, &error)); QCOMPARE(only.objects.first().type, QStringLiteral("time"));
    QVERIFY(!exportNativeProjectSource(only, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(only, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")});
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY2(loaded.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("time-mixed.ksp")), &error), qPrintable(error)); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::timeResourcesVisibilityAndNoMonitorCarrier
 * Description     : Protect artwork across pages, preserve only real visibility address roles and merge picture collisions.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::timeResourcesVisibilityAndNoMonitorCarrier() //20261112-0 add
{
    HmiProject source;
    HmiProject result;
    HmiProject target;
    HmiPage page;
    HmiObject object = timeObject();
    Stage3DuplicateOptions options;
    Stage3Clipboard clipboard;
    QStringList ids;
    QString error;
    object.timeDisplay.appearance = {QStringLiteral("clock.png"), imagePng(Qt::green)}; object.timeDisplay.visibility.visibilityEnabled = true; object.timeDisplay.visibility.bitEnabled = true; object.timeDisplay.visibility.bitAddress = QStringLiteral("LB10"); source.pictures.insert(QStringLiteral("clock.png"), imagePng(Qt::green)); source.objects = {object};
    page.id = QStringLiteral("clock-page"); page.name = QStringLiteral("时钟副页"); object.id = QStringLiteral("clock-inactive"); page.objects = {object}; QVERIFY2(source.addPage(page, &error), qPrintable(error)); QCOMPARE(stage3PictureUsage(source, QStringLiteral("clock.png")).boundStates, 2); QVERIFY(!deleteStage3Picture(source, QStringLiteral("clock.png"), &result, &error)); QVERIFY2(renameStage3Picture(source, QStringLiteral("clock.png"), QStringLiteral("clock-new.png"), &result, &error), qPrintable(error)); QCOMPARE(result.inactivePages.first().objects.first().timeDisplay.appearance.picture, QStringLiteral("clock-new.png"));
    options.columns = 2; options.increments = {{Stage3AddressField::MonitorWord, 100, 0}}; result = source; QVERIFY(!duplicateStage3Selection(source, {source.objects.first().id}, options, &result, nullptr, &error)); QCOMPARE(result.toJson(), source.toJson()); options.increments = {{Stage3AddressField::VisibilityBit, 2, 0}}; QVERIFY2(duplicateStage3Selection(source, {source.objects.first().id}, options, &result, nullptr, &error), qPrintable(error)); QVERIFY(result.objects.last().address.isEmpty()); QCOMPARE(result.objects.last().timeDisplay.visibility.bitAddress, QStringLiteral("LB12"));
    target.pictures.insert(QStringLiteral("clock.png"), imagePng(Qt::red)); QVERIFY2(captureStage3Selection(source, {source.objects.first().id}, &clipboard, &error), qPrintable(error)); QVERIFY2(pasteStage3Selection(target, clipboard, QPoint(10, 10), &result, &ids, &error), qPrintable(error)); QCOMPARE(result.objects.last().timeDisplay.appearance.picturePng, imagePng(Qt::green)); QVERIFY(result.objects.last().timeDisplay.appearance.picture != QStringLiteral("clock.png"));
    object.timeDisplay.visibility.wordEnabled = true; object.timeDisplay.visibility.dataType = 1; object.timeDisplay.visibility.wordAddress = QStringLiteral("LW4096"); QVERIFY(!object.timeDisplay.validateAddresses({}, &error)); object.timeDisplay.visibility.visibilityEnabled = false; QVERIFY2(object.timeDisplay.validateAddresses({}, &error), qPrintable(error));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::timeRasterClippingAndFormatSamples
 * Description     : Verify shared clock sample painting, the actual default background and clipping without host-clock substitution.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::timeRasterClippingAndFormatSamples() //20261112-0 add
{
    TimeDisplayProperties properties;
    QImage raster(200, 100, QImage::Format_ARGB32);
    QImage original;
    QPainter painter;
    raster.fill(Qt::magenta); painter.begin(&raster); paintTimeDisplay(&painter, QRectF(10, 10, 180, 80), properties); painter.end(); QCOMPARE(raster.pixelColor(0, 0), QColor(Qt::magenta)); QCOMPARE(raster.pixelColor(12, 12), QColor(QStringLiteral("#a0a0ff"))); original = raster;
    properties.showWeek = true; properties.showTime = true; properties.fontSize = 12; painter.begin(&raster); paintTimeDisplay(&painter, QRectF(10, 10, 180, 80), properties); painter.end(); QVERIFY(raster != original); QCOMPARE(properties.sample(), QStringLiteral("YYYY/MM/DD (Mon) HH:MM")); QCOMPARE(raster.pixelColor(0, 0), QColor(Qt::magenta));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::timeShellPlacementConversionAndUndo
 * Description     : Exercise the actual clock command, creation cancellation, family conversion and whole-project undo.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::timeShellPlacementConversionAndUndo() //20261112-0 add
{
    MainWindow window;
    int history;
    window.setProject(HmiProject(), QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); history = window.m_projectHistory.size();
    QTimer::singleShot(0, &window, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } }); window.m_commands.value(QStringLiteral("action_timeDisplay"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement()); QCOMPARE(window.m_projectHistory.size(), history);
    QTimer::singleShot(0, &window, []() { NumberDisplayDialog *dialog = qobject_cast<NumberDisplayDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QCheckBox *>(QStringLiteral("Number_timeShowWeek"))->setChecked(true); dialog->findChild<QDialogButtonBox *>(QStringLiteral("NumberButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.m_commands.value(QStringLiteral("action_timeDisplay"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement());
    QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(210, 100))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(210, 100))); QCOMPARE(window.currentEditorProject().objects.first().type, QStringLiteral("time")); QVERIFY(window.currentEditorProject().objects.first().timeDisplay.showWeek); QVERIFY(!window.m_addressEdit->isEnabled()); QCOMPARE(window.m_projectHistory.size(), history + 1);
    QTimer::singleShot(0, &window, []() { NumberDisplayDialog *dialog = qobject_cast<NumberDisplayDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QRadioButton *>(QStringLiteral("NumberFamily1"))->click(); dialog->findChild<QDialogButtonBox *>(QStringLiteral("NumberButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().type, QStringLiteral("ascii")); QCOMPARE(window.m_projectHistory.size(), history + 2); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().objects.first().type, QStringLiteral("time")); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty());
}

/******************************************************************************
 * Function Name   : screenObject
 * Description     : Create an independent navigation control with a stable normal-page target.
 * Input Parameters: project
 * Return Value    : Default screen button without ordinary switch properties.
 * Date            : 2026.11.12
 ******************************************************************************/
static HmiObject screenObject(const HmiProject &project) //20261112-0 add
{
    HmiObject object;
    object.id = QStringLiteral("screen-object"); object.type = QStringLiteral("screen"); object.name = QStringLiteral("画面按钮"); object.geometry = QRect(600, 300, 120, 60); object.text = QStringLiteral("OFF"); object.screenButton.screenPageId = project.activePageId; return object;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::screenDefaultsOperationsAndStableTargets
 * Description     : Verify native four-page defaults, target filtering, operation row visibility and independent dormant IDs.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::screenDefaultsOperationsAndStableTargets() //20261112-0 add
{
    HmiProject project;
    HmiPage page;
    QString error;
    page.id = QStringLiteral("window-target"); page.name = QStringLiteral("窗口甲"); page.kind = QStringLiteral("window"); QVERIFY2(project.addPage(page, &error), qPrintable(error));
    BitButtonDialog dialog(screenObject(project), project);
    QComboBox *action = dialog.findChild<QComboBox *>(QStringLiteral("ScreenAction"));
    QComboBox *target = dialog.findChild<QComboBox *>(QStringLiteral("ScreenTarget"));
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(dialog.windowTitle(), QStringLiteral("画面按钮")); QCOMPARE(dialog.findChild<QTabWidget *>(QStringLiteral("BitButtonTabs"))->count(), 4); QCOMPARE(action->count(), 4); QCOMPARE(target->currentData().toString(), project.activePageId); QVERIFY(dialog.findChild<QGroupBox *>(QStringLiteral("ScreenChangeLevel"))->isChecked()); QVERIFY(dialog.findChild<QCheckBox *>(QStringLiteral("ScreenRequireLogin"))->isChecked()); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("ScreenNewLevel"))->count(), 8); QVERIFY(!dialog.findChild<QGroupBox *>(QStringLiteral("BitMacro"))->isEnabled()); QVERIFY(!dialog.object().hasBitButtonProperties); QVERIFY(dialog.object().address.isEmpty());
    action->setCurrentIndex(1); QVERIFY(!target->isVisible()); action->setCurrentIndex(2); QCOMPARE(target->count(), 1); QCOMPARE(target->currentData().toString(), page.id); action->setCurrentIndex(3); QVERIFY(!target->isVisible()); action->setCurrentIndex(0); QCOMPARE(target->currentData().toString(), project.activePageId); QCOMPARE(dialog.findChild<QLabel *>(QStringLiteral("ScreenTargetLabel"))->text(), QStringLiteral("窗口名称:")); action->setCurrentIndex(2); QCOMPARE(dialog.object().screenButton.windowPageId, page.id);
    dialog.findChild<QGroupBox *>(QStringLiteral("ScreenChangeLevel"))->setChecked(false); QVERIFY(!dialog.findChild<QComboBox *>(QStringLiteral("ScreenNewLevel"))->isEnabled());
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("screen-general.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::screenMonitorAppearanceAndPermissions
 * Description     : Verify screen-specific monitoring defaults, both visual drafts and notification visibility without claiming runtime navigation.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::screenMonitorAppearanceAndPermissions() //20261112-0 add
{
    HmiProject project;
    BitButtonDialog dialog(screenObject(project), project);
    QTabWidget *tabs = dialog.findChild<QTabWidget *>(QStringLiteral("BitButtonTabs"));
    QString error;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); tabs->setCurrentIndex(1); QVERIFY(!dialog.findChild<QCheckBox *>(QStringLiteral("BitIndicator"))->isChecked()); QVERIFY(!dialog.findChild<QCheckBox *>(QStringLiteral("BitMonitorSame"))->isChecked()); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("BitMonitorIndex"))->value(), 0); dialog.findChild<QCheckBox *>(QStringLiteral("BitIndicator"))->setChecked(true); dialog.findChild<QComboBox *>(QStringLiteral("BitMonitorType"))->setCurrentIndex(1); dialog.findChild<QLineEdit *>(QStringLiteral("BitMonitorAddress"))->setText(QStringLiteral("LW12")); QCOMPARE(dialog.object().screenButton.common.monitorWordAddress, QStringLiteral("LW12")); QVERIFY2(dialog.object().screenButton.validateAddresses({}, &error), qPrintable(error));
    tabs->setCurrentIndex(2); QVERIFY(!dialog.findChild<QCheckBox *>(QStringLiteral("BitFlash"))->isEnabled()); QCOMPARE(dialog.object().screenButton.common.states[0].background, QColor(QStringLiteral("#f0f0f0"))); QVERIFY(!dialog.findChild<QSpinBox *>(QStringLiteral("BitOpacity"))->isEnabled()); dialog.findChild<QPlainTextEdit *>(QStringLiteral("BitText"))->setPlainText(QStringLiteral("页面OFF")); dialog.findChild<QSpinBox *>(QStringLiteral("BitState"))->setValue(1); dialog.findChild<QPlainTextEdit *>(QStringLiteral("BitText"))->setPlainText(QStringLiteral("页面ON")); dialog.findChild<QSpinBox *>(QStringLiteral("BitState"))->setValue(0); QCOMPARE(dialog.findChild<QPlainTextEdit *>(QStringLiteral("BitText"))->toPlainText(), QStringLiteral("页面OFF")); QCOMPARE(bitButtonContent(dialog.object().screenButton.common.states[1], QStringLiteral("中文")), QStringLiteral("页面ON"));
    tabs->setCurrentIndex(3); dialog.findChild<QGroupBox *>(QStringLiteral("BitNotice"))->setChecked(true); QVERIFY(dialog.findChild<QCheckBox *>(QStringLiteral("BitNoticeBit"))->isEnabled()); dialog.findChild<QCheckBox *>(QStringLiteral("BitNoticeBit"))->setChecked(true); QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("BitNoticeBitAddress"))->isVisible()); dialog.findChild<QCheckBox *>(QStringLiteral("BitNoticeWord"))->setChecked(true); QVERIFY(!dialog.findChild<QCheckBox *>(QStringLiteral("BitNoticeBit"))->isChecked()); QVERIFY(dialog.object().screenButton.common.noticeWord); QVERIFY2(dialog.object().screenButton.validate(&error), qPrintable(error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("screen-permissions.png")))); tabs->setCurrentIndex(2); QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("screen-appearance.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::screenStrictMixedPersistenceAndBackendGates
 * Description     : Prove screen-only and fourteen-family persistence while keeping unknown native outputs closed atomically.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::screenStrictMixedPersistenceAndBackendGates() //20261112-0 add
{
    HmiProject project;
    HmiProject loaded;
    HmiProject only;
    HmiObject object = screenObject(project);
    HmiObject parsed = qrObject();
    QJsonObject json;
    QJsonObject props;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    project.objects = existingFiveObjects(); project.objects.append({messageObject(), scaleObject(), qrObject(), imageObject(), gifObject(), numberObject(), asciiObject(), timeObject(), object}); QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.objects.size(), 14); QCOMPARE(loaded.toJson(), project.toJson());
    only.objects = {object}; json = only.toJson(); QCOMPARE(json.value(QStringLiteral("schemaVersion")).toInt(), 12); QVERIFY2(HmiProject::fromJson(json, &only, &error), qPrintable(error)); QVERIFY(!exportNativeProjectSource(only, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(only, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")});
    json = object.toJson(); props = object.screenButton.toJson(); props.insert(QStringLiteral("action"), 1.5); json.insert(QStringLiteral("screenButton"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); QCOMPARE(parsed.type, QStringLiteral("qrcode")); json = object.toJson(); json.remove(QStringLiteral("screenButton")); QVERIFY(!HmiObject::fromJson(json, &parsed, &error));
    object.screenButton.common.indicator = true; object.screenButton.common.monitorSame = true; QVERIFY(!object.screenButton.validate(&error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY2(loaded.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("screen-mixed.ksp")), &error), qPrintable(error)); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::screenPageLifecycleAndClipboardDependencies
 * Description     : Protect active targets on rename/delete, remap self-navigation on page copy and reject missing external dependencies.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::screenPageLifecycleAndClipboardDependencies() //20261112-0 add
{
    HmiProject project;
    HmiProject before;
    HmiProject result;
    HmiProject target;
    HmiPage page;
    Stage3Clipboard clipboard;
    QString copyId;
    QString error;
    page.id = QStringLiteral("normal-target"); page.name = QStringLiteral("画面乙"); QVERIFY2(project.addPage(page, &error), qPrintable(error)); project.objects = {screenObject(project)}; project.objects[0].screenButton.screenPageId = page.id; QVERIFY2(project.validateModel(&error), qPrintable(error)); QVERIFY2(project.renamePage(page.id, QStringLiteral("已改名"), &error), qPrintable(error)); QCOMPARE(project.objects.first().screenButton.screenPageId, page.id); before = project; QVERIFY(!project.removePage(page.id, &error)); QCOMPARE(project.toJson(), before.toJson());
    QVERIFY2(captureStage3Selection(project, {project.objects.first().id}, &clipboard, &error), qPrintable(error)); result = target; QVERIFY(!pasteStage3Selection(target, clipboard, QPoint(1, 1), &result, nullptr, &error)); QCOMPARE(result.toJson(), target.toJson());
    project.objects[0].screenButton.screenPageId = project.activePageId; QVERIFY2(project.copyPage(project.activePageId, QStringLiteral("复制自跳转"), &copyId, &error), qPrintable(error)); QCOMPARE(project.inactivePages.last().objects.first().screenButton.screenPageId, copyId);
    project.objects[0].screenButton.action = 1; QVERIFY2(project.removePage(page.id, &error), qPrintable(error));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::screenResourcesAndRealAddressRoles
 * Description     : Protect both state images across pages and apply only screen monitoring/permission/notice increments.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::screenResourcesAndRealAddressRoles() //20261112-0 add
{
    HmiProject project;
    HmiProject result;
    HmiPage page;
    HmiObject object = screenObject(project);
    Stage3DuplicateOptions options;
    QString error;
    object.screenButton.common.indicator = true; object.screenButton.common.monitorBitAddress = QStringLiteral("LB10"); object.screenButton.common.notice = true; object.screenButton.common.noticeWord = true; object.screenButton.common.noticeWordAddress = QStringLiteral("LW20"); object.screenButton.common.states[1].picture = QStringLiteral("screen.png"); object.screenButton.common.states[1].picturePng = imagePng(Qt::green); project.pictures.insert(QStringLiteral("screen.png"), imagePng(Qt::green)); project.objects = {object}; page.id = QStringLiteral("screen-inactive"); page.name = QStringLiteral("画面按钮副页"); object.id = QStringLiteral("screen-other"); page.objects = {object}; QVERIFY2(project.addPage(page, &error), qPrintable(error)); QCOMPARE(stage3PictureUsage(project, QStringLiteral("screen.png")).boundStates, 2); QVERIFY(!deleteStage3Picture(project, QStringLiteral("screen.png"), &result, &error)); QVERIFY2(renameStage3Picture(project, QStringLiteral("screen.png"), QStringLiteral("screen-renamed.png"), &result, &error), qPrintable(error)); QCOMPARE(result.inactivePages.first().objects.first().screenButton.common.states[1].picture, QStringLiteral("screen-renamed.png"));
    options.columns = 2; options.increments = {{Stage3AddressField::MonitorBit, 2, 0}, {Stage3AddressField::NoticeWord, 3, 0}}; QVERIFY2(duplicateStage3Selection(project, {project.objects.first().id}, options, &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.objects.last().screenButton.common.monitorBitAddress, QStringLiteral("LB12")); QCOMPARE(result.objects.last().screenButton.common.noticeWordAddress, QStringLiteral("LW23")); QVERIFY(result.objects.last().address.isEmpty());
    options.increments = {{Stage3AddressField::Operation, 2, 0}}; result = project; QVERIFY(!duplicateStage3Selection(project, {project.objects.first().id}, options, &result, nullptr, &error)); QCOMPARE(result.toJson(), project.toJson());
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::screenShellCreationCancellationAndUndo
 * Description     : Exercise actual screen-button creation, cancellation, stable-target editing and whole-project undo.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::screenShellCreationCancellationAndUndo() //20261112-0 add
{
    MainWindow window;
    int history;
    window.setProject(HmiProject(), QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); history = window.m_projectHistory.size();
    QTimer::singleShot(0, &window, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } }); window.m_commands.value(QStringLiteral("action_screenbutton"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement()); QCOMPARE(window.m_projectHistory.size(), history);
    QTimer::singleShot(0, &window, []() { BitButtonDialog *dialog = qobject_cast<BitButtonDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click(); } }); window.m_commands.value(QStringLiteral("action_screenbutton"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement());
    QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(200, 110))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(200, 110))); QCOMPARE(window.currentEditorProject().objects.first().type, QStringLiteral("screen")); QVERIFY(!window.currentEditorProject().objects.first().hasBitButtonProperties); QVERIFY(!window.m_addressEdit->isEnabled()); QCOMPARE(window.m_projectHistory.size(), history + 1);
    QTimer::singleShot(0, &window, []() { BitButtonDialog *dialog = qobject_cast<BitButtonDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QComboBox *>(QStringLiteral("ScreenAction"))->setCurrentIndex(1); dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click(); } }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().screenButton.action, 1); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().objects.first().screenButton.action, 0); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty());
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::screenLanguageCompactProperties
 * Description     : Prevent a name or geometry edit in English from overwriting its translated screen-button label with Chinese.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::screenLanguageCompactProperties() //20261112-0 add
{
    HmiProject project;
    HmiObject object = screenObject(project);
    MainWindow window;
    QString error;
    project.languages = {QStringLiteral("中文"), QStringLiteral("English")}; project.activeLanguage = QStringLiteral("English"); object.text = QStringLiteral("中文"); object.screenButton.common.states[0].text.languageTexts = {{QStringLiteral("中文"), QStringLiteral("中文")}, {QStringLiteral("English"), QStringLiteral("English")}}; object.screenButton.common.macroName = QStringLiteral("dormant macro"); object.screenButton.common.pressTime = 0; project.objects = {object};
    QVERIFY2(HmiProject::fromJson(project.toJson(), &project, &error), qPrintable(error)); window.setProject(project, QString()); window.m_canvas->selectObject(object.id); QCOMPARE(window.m_textEdit->toPlainText(), QStringLiteral("English")); window.m_nameEdit->setText(QStringLiteral("重命名")); window.applySelectedProperties(); QCOMPARE(bitButtonContent(window.currentEditorProject().objects.first().screenButton.common.states[0], QStringLiteral("English")), QStringLiteral("English"));
    window.m_textEdit->setPlainText(QStringLiteral("English edited")); window.applySelectedProperties(); object = window.currentEditorProject().objects.first(); QCOMPARE(bitButtonContent(object.screenButton.common.states[0], QStringLiteral("中文")), QStringLiteral("中文")); QCOMPARE(bitButtonContent(object.screenButton.common.states[0], QStringLiteral("English")), QStringLiteral("English edited"));
    BitButtonDialog dialog(object, project);
    QCOMPARE(dialog.object().screenButton.common.macroName, QStringLiteral("dormant macro")); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("BitPressTime"))->minimum(), 0); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("BitPressTime"))->maximum(), 255); QCOMPARE(dialog.object().screenButton.common.pressTime, 0); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("BitSize"))->maximum(), 128); QVERIFY(!dialog.findChild<QPushButton *>(QStringLiteral("BitBackground"))->isEnabled());
}

/******************************************************************************
 * Function Name   : multiObject
 * Description     : Create an independent three-state input record without activating shared lamp or bit-switch layouts.
 * Input Parameters: None
 * Return Value    : Bounded multi-state button.
 * Date            : 2026.11.12
 ******************************************************************************/
static HmiObject multiObject() //20261112-0 add
{
    HmiObject object;
    object.id = QStringLiteral("multi-object"); object.type = QStringLiteral("multistate"); object.name = QStringLiteral("多态按钮"); object.geometry = QRect(100, 350, 130, 70); object.text = QStringLiteral("OFF"); return object;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::multiDefaultsAndMirroredAddressLifetime
 * Description     : Verify native three-state defaults, six action drafts and the documented same-address mirror/uncheck lifetime.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::multiDefaultsAndMirroredAddressLifetime() //20261112-0 add
{
    BitButtonDialog dialog(multiObject(), HmiProject());
    QTabWidget *tabs = dialog.findChild<QTabWidget *>(QStringLiteral("BitButtonTabs"));
    QLineEdit *monitor = dialog.findChild<QLineEdit *>(QStringLiteral("MultiMonitor"));
    QLineEdit *write = dialog.findChild<QLineEdit *>(QStringLiteral("MultiWrite"));
    QCheckBox *same = dialog.findChild<QCheckBox *>(QStringLiteral("MultiSameAddress"));
    int index;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(dialog.windowTitle(), QStringLiteral("多态按钮")); QCOMPARE(tabs->count(), 4); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("MultiDataType"))->count(), 2); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("MultiStateCount"))->value(), 3); QVERIFY(!same->isChecked()); QVERIFY(!dialog.findChild<QGroupBox *>(QStringLiteral("BitMacro"))->isEnabled()); QVERIFY(!dialog.object().hasBitButtonProperties); QVERIFY(!dialog.object().hasLampProperties); QVERIFY(dialog.object().address.isEmpty());
    monitor->setText(QStringLiteral("LW12")); write->setText(QStringLiteral("LW25")); same->setChecked(true); QCOMPARE(write->text(), QStringLiteral("LW12")); QVERIFY(!write->isEnabled()); monitor->setText(QStringLiteral("LW14")); QCOMPARE(write->text(), QStringLiteral("LW14")); same->setChecked(false); QVERIFY(write->isEnabled()); QCOMPARE(write->text(), QStringLiteral("LW14"));
    for (index = 0; index < 6; ++index) { dialog.findChild<QRadioButton *>(QStringLiteral("MultiAction%1").arg(index))->click(); QCOMPARE(dialog.object().multiStateButton.action, index); }
    tabs->setCurrentIndex(1); QCOMPARE(tabs->widget(1)->findChildren<QWidget *>().size(), 0);
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { tabs->setCurrentIndex(0); QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("multi-general.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::multiCountShrinkRegrowAndSelection
 * Description     : Reproduce the successful native lifetime experiment and prevent deleted state data from returning.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::multiCountShrinkRegrowAndSelection() //20261112-0 add
{
    HmiObject original = multiObject();
    BitButtonDialog dialog(original, HmiProject());
    QTabWidget *tabs = dialog.findChild<QTabWidget *>(QStringLiteral("BitButtonTabs"));
    QSpinBox *count = dialog.findChild<QSpinBox *>(QStringLiteral("MultiStateCount"));
    QSpinBox *state = dialog.findChild<QSpinBox *>(QStringLiteral("BitState"));
    QPlainTextEdit *text = dialog.findChild<QPlainTextEdit *>(QStringLiteral("BitText"));
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); tabs->setCurrentIndex(2); state->setValue(2); QCOMPARE(text->toPlainText(), QString()); text->setPlainText(QStringLiteral("C15_STATE2_KEEP")); dialog.findChild<QPushButton *>(QStringLiteral("BitNoImage"))->click(); QVERIFY(!dialog.object().multiStateButton.state(2)->useImage); QVERIFY(dialog.findChild<QSpinBox *>(QStringLiteral("BitOpacity"))->isEnabled());
    count->setValue(2); QCOMPARE(state->maximum(), 1); QCOMPARE(state->value(), 1); QCOMPARE(text->toPlainText(), QStringLiteral("ON")); count->setValue(3); state->setValue(2); QCOMPARE(text->toPlainText(), QString()); QVERIFY(dialog.object().multiStateButton.state(2)->useImage); QCOMPARE(dialog.object().multiStateButton.state(2)->picture, QStringLiteral("switch_ON.kvg")); QCOMPARE(dialog.object().multiStateButton.state(2)->background, QColor(QStringLiteral("#a0a0a0"))); QVERIFY(!dialog.findChild<QSpinBox *>(QStringLiteral("BitOpacity"))->isEnabled());
    count->setValue(32); state->setValue(31); text->setPlainText(QStringLiteral("tail31")); count->setValue(3); QCOMPARE(state->value(), 2); count->setValue(32); state->setValue(31); QCOMPARE(text->toPlainText(), QString()); dialog.reject(); QCOMPARE(original.multiStateButton.stateCount, 3); QCOMPARE(bitButtonContent(*original.multiStateButton.state(2), QStringLiteral("中文")), QString());
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::multiAllStatesLanguagesAndFlags
 * Description     : Keep thirty-two independent language/text/image drafts and state-local visual flags through shared editor access.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::multiAllStatesLanguagesAndFlags() //20261112-0 add
{
    HmiProject project;
    HmiObject object = multiObject();
    BitButtonDialog *dialog;
    QSpinBox *state;
    QComboBox *language;
    QPlainTextEdit *text;
    int index;
    project.languages = {QStringLiteral("中文"), QStringLiteral("English")}; object.multiStateButton.setStateCount(32); dialog = new BitButtonDialog(object, project); dialog->show(); QVERIFY(QTest::qWaitForWindowExposed(dialog)); dialog->findChild<QTabWidget *>(QStringLiteral("BitButtonTabs"))->setCurrentIndex(2); state = dialog->findChild<QSpinBox *>(QStringLiteral("BitState")); language = dialog->findChild<QComboBox *>(QStringLiteral("BitLanguage")); text = dialog->findChild<QPlainTextEdit *>(QStringLiteral("BitText"));
    for (index = 0; index < 32; ++index) { state->setValue(index); language->setCurrentText(QStringLiteral("中文")); text->setPlainText(QStringLiteral("中文%1").arg(index)); language->setCurrentText(QStringLiteral("English")); text->setPlainText(QStringLiteral("English %1").arg(index)); }
    state->setValue(2); dialog->findChild<QCheckBox *>(QStringLiteral("Bit3D"))->setChecked(true); object = dialog->object(); QVERIFY(object.multiStateButton.state(2)->effect3d); QVERIFY(!object.multiStateButton.state(0)->effect3d); QVERIFY(!object.multiStateButton.state(1)->effect3d); QVERIFY(dialog->findChild<QCheckBox *>(QStringLiteral("BitFlash"))->isEnabled());
    for (index = 0; index < 32; ++index) { QCOMPARE(bitButtonContent(*object.multiStateButton.state(index), QStringLiteral("中文")), QStringLiteral("中文%1").arg(index)); QCOMPARE(bitButtonContent(*object.multiStateButton.state(index), QStringLiteral("English")), QStringLiteral("English %1").arg(index)); }
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog->grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("multi-appearance.png")))); } delete dialog;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::multiStrictMixedPersistenceAndBackendGates
 * Description     : Round-trip fifteen families and a multi-only project while rejecting bad state lists and unrecovered native outputs atomically.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::multiStrictMixedPersistenceAndBackendGates() //20261112-0 add
{
    HmiProject project;
    HmiProject loaded;
    HmiProject only;
    HmiObject object = multiObject();
    HmiObject parsed = qrObject();
    QJsonObject json;
    QJsonObject props;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    object.multiStateButton.setStateCount(32); object.multiStateButton.extraStates.last().text.languageTexts.insert(QStringLiteral("中文"), QStringLiteral("state31")); project.objects = existingFiveObjects(); project.objects.append({messageObject(), scaleObject(), qrObject(), imageObject(), gifObject(), numberObject(), asciiObject(), timeObject(), screenObject(project), object}); QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.objects.size(), 15); QCOMPARE(loaded.toJson(), project.toJson());
    only.objects = {object}; json = only.toJson(); QCOMPARE(json.value(QStringLiteral("schemaVersion")).toInt(), 12); QVERIFY2(HmiProject::fromJson(json, &only, &error), qPrintable(error)); QVERIFY(!exportNativeProjectSource(only, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(only, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")});
    json = object.toJson(); props = object.multiStateButton.toJson(); props.insert(QStringLiteral("extraStates"), QJsonArray()); json.insert(QStringLiteral("multiStateButton"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); QCOMPARE(parsed.type, QStringLiteral("qrcode")); props = object.multiStateButton.toJson(); props.insert(QStringLiteral("dataType"), 2); json.insert(QStringLiteral("multiStateButton"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY2(loaded.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("multi-mixed.ksp")), &error), qPrintable(error)); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::multiResourcesAndFullAddressRoles
 * Description     : Protect extra-state resources across pages and validate independent full-width monitor/write/notice increments.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::multiResourcesAndFullAddressRoles() //20261112-0 add
{
    HmiProject project;
    HmiProject result;
    HmiPage page;
    HmiObject object = multiObject();
    Stage3DuplicateOptions options;
    QString error;
    object.multiStateButton.setStateCount(32); object.multiStateButton.dataType = 1; object.multiStateButton.monitorAddress = QStringLiteral("LW10"); object.multiStateButton.writeAddress = QStringLiteral("LW20"); object.multiStateButton.common.notice = true; object.multiStateButton.common.noticeBit = true; object.multiStateButton.common.noticeBitAddress = QStringLiteral("LB30"); object.multiStateButton.state(31)->picture = QStringLiteral("multi.png"); object.multiStateButton.state(31)->picturePng = imagePng(Qt::green); project.pictures.insert(QStringLiteral("multi.png"), imagePng(Qt::green)); project.objects = {object}; page.id = QStringLiteral("multi-page"); page.name = QStringLiteral("多态副页"); object.id = QStringLiteral("multi-other"); page.objects = {object}; QVERIFY2(project.addPage(page, &error), qPrintable(error)); QCOMPARE(stage3PictureUsage(project, QStringLiteral("multi.png")).boundStates, 2); QVERIFY(!deleteStage3Picture(project, QStringLiteral("multi.png"), &result, &error)); QVERIFY2(renameStage3Picture(project, QStringLiteral("multi.png"), QStringLiteral("multi-new.png"), &result, &error), qPrintable(error)); QCOMPARE(result.inactivePages.first().objects.first().multiStateButton.state(31)->picture, QStringLiteral("multi-new.png"));
    options.columns = 2; options.increments = {{Stage3AddressField::MonitorWord, 2, 0}, {Stage3AddressField::Operation, 4, 0}, {Stage3AddressField::NoticeBit, 3, 0}}; QVERIFY2(duplicateStage3Selection(project, {project.objects.first().id}, options, &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.objects.last().multiStateButton.monitorAddress, QStringLiteral("LW12")); QCOMPARE(result.objects.last().multiStateButton.writeAddress, QStringLiteral("LW24")); QCOMPARE(result.objects.last().multiStateButton.common.noticeBitAddress, QStringLiteral("LB33")); QVERIFY(result.objects.last().address.isEmpty());
    object.multiStateButton.monitorAddress = QStringLiteral("LW4096"); QVERIFY(!object.multiStateButton.validateAddresses({}, &error)); object.multiStateButton.dataType = 0; QVERIFY2(object.multiStateButton.validateAddresses({}, &error), qPrintable(error));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::multiShellPlacementStatePreviewAndUndo
 * Description     : Exercise actual creation/cancellation, thirty-two-state preview, editing, shrinking and undo without PLC writes.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::multiShellPlacementStatePreviewAndUndo() //20261112-0 add
{
    MainWindow window;
    int history;
    window.setProject(HmiProject(), QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); history = window.m_projectHistory.size(); QTimer::singleShot(0, &window, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } }); window.m_commands.value(QStringLiteral("action_polyBtn"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement());
    QTimer::singleShot(0, &window, []() { BitButtonDialog *dialog = qobject_cast<BitButtonDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QSpinBox *>(QStringLiteral("MultiStateCount"))->setValue(32); dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click(); } }); window.m_commands.value(QStringLiteral("action_polyBtn"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(200, 110))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(200, 110))); QCOMPARE(window.currentEditorProject().objects.first().multiStateButton.stateCount, 32); QCOMPARE(window.findChild<QComboBox *>(QStringLiteral("NativeStatePreview"))->count(), 32); QCOMPARE(window.m_projectHistory.size(), history + 1);
    QTimer::singleShot(0, &window, []() { BitButtonDialog *dialog = qobject_cast<BitButtonDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QSpinBox *>(QStringLiteral("MultiStateCount"))->setValue(2); dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click(); } }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().multiStateButton.stateCount, 2); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().objects.first().multiStateButton.stateCount, 32); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty());
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::multiLanguageCompactProperties
 * Description     : Preserve English/Chinese OFF labels and real monitor identity during compact edits and whole-object undo.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::multiLanguageCompactProperties() //20261112-0 add
{
    HmiProject project;
    HmiObject object = multiObject();
    MainWindow window;
    project.languages = {QStringLiteral("中文"), QStringLiteral("English")}; project.activeLanguage = QStringLiteral("English"); object.text = QStringLiteral("中文"); object.multiStateButton.common.states[0].text.languageTexts = {{QStringLiteral("中文"), QStringLiteral("中文")}, {QStringLiteral("English"), QStringLiteral("English")}}; project.objects = {object}; window.setProject(project, QString()); window.m_canvas->selectObject(object.id); QCOMPARE(window.m_textEdit->toPlainText(), QStringLiteral("English")); window.m_nameEdit->setText(QStringLiteral("multi renamed")); window.m_addressEdit->setText(QStringLiteral("LW8")); window.applySelectedProperties(); object = window.currentEditorProject().objects.first(); QCOMPARE(object.multiStateButton.monitorAddress, QStringLiteral("LW8")); QVERIFY(object.address.isEmpty()); QCOMPARE(bitButtonContent(object.multiStateButton.common.states[0], QStringLiteral("English")), QStringLiteral("English")); QCOMPARE(bitButtonContent(object.multiStateButton.common.states[0], QStringLiteral("中文")), QStringLiteral("中文")); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().objects.first().multiStateButton.monitorAddress, QStringLiteral("LW0"));
}

/******************************************************************************
 * Function Name   : mfObject
 * Description     : Create an independent MF editor fixture without ordinary Switch flags or a generic operation address.
 * Input Parameters: None
 * Return Value    : Typed default MF object.
 * Date            : 2026.11.12
 ******************************************************************************/
static HmiObject mfObject() //20261112-0 add
{
    HmiObject object;
    object.id = QStringLiteral("mf-object"); object.type = QStringLiteral("multifunction"); object.name = QStringLiteral("多功能按钮"); object.geometry = QRect(260, 350, 160, 80); return object;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::mfDefaultsAndCommandSelection
 * Description     : Check all native menu orders, new-item defaults and mutually exclusive editor groups with row-specific round trips.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::mfDefaultsAndCommandSelection() //20261112-0 add
{
    HmiObject object = mfObject();
    BitButtonDialog dialog(object, HmiProject());
    QListWidget *list = dialog.findChild<QListWidget *>(QStringLiteral("MfCommands"));
    QTabWidget *tabs = dialog.findChild<QTabWidget *>(QStringLiteral("BitButtonTabs"));
    QComboBox *bit = dialog.findChild<QComboBox *>(QStringLiteral("MfBitOperation"));
    QComboBox *word = dialog.findChild<QComboBox *>(QStringLiteral("MfWordOperation"));
    QComboBox *type = dialog.findChild<QComboBox *>(QStringLiteral("MfDataType"));
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(dialog.windowTitle(), QStringLiteral("多功能按钮")); QCOMPARE(tabs->count(), 3); QCOMPARE(tabs->tabText(0), QStringLiteral("基本设置")); QCOMPARE(list->count(), 1); QCOMPARE(list->currentRow(), 0); QCOMPARE(list->item(0)->text(), QStringLiteral("线圈操作 置位"));
    QVERIFY(dialog.findChild<QGroupBox *>(QStringLiteral("MfBitGroup"))->isEnabled()); QVERIFY(!dialog.findChild<QGroupBox *>(QStringLiteral("MfScreenGroup"))->isEnabled()); QVERIFY(!dialog.findChild<QGroupBox *>(QStringLiteral("MfWordGroup"))->isEnabled()); QVERIFY(!dialog.findChild<QGroupBox *>(QStringLiteral("BitMacro"))->isEnabled()); QCOMPARE(type->currentIndex(), 0); QCOMPARE(bit->itemText(2), QStringLiteral("交替")); QCOMPARE(bit->itemText(3), QStringLiteral("点动"));
    bit->setCurrentIndex(2); dialog.findChild<QLineEdit *>(QStringLiteral("MfBitAddress"))->setText(QStringLiteral("LB20")); dialog.findChild<QPushButton *>(QStringLiteral("MfAddWord"))->click(); QCOMPARE(list->currentRow(), 1); QCOMPARE(type->currentIndex(), 2); QCOMPARE(type->itemText(0), QStringLiteral("16位整数")); QCOMPARE(type->itemText(2), QStringLiteral("16位正整数")); QCOMPARE(word->currentIndex(), 1); QCOMPARE(word->itemText(0), QStringLiteral("用户输入"));
    dialog.findChild<QLineEdit *>(QStringLiteral("MfValue"))->setText(QStringLiteral("123")); word->setCurrentIndex(0); QVERIFY(!dialog.findChild<QLineEdit *>(QStringLiteral("MfValue"))->isVisible()); word->setCurrentIndex(1); QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("MfValue"))->text(), QStringLiteral("123"));
    dialog.findChild<QPushButton *>(QStringLiteral("MfAddScreen"))->click(); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("MfTarget"))->currentIndex(), -1); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("MfScreenOperation"))->count(), 4); dialog.findChild<QComboBox *>(QStringLiteral("MfScreenOperation"))->setCurrentIndex(1);
    list->setCurrentRow(0); QCOMPARE(bit->currentIndex(), 2); QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("MfBitAddress"))->text(), QStringLiteral("LB20")); object = dialog.object(); QVERIFY(!object.hasBitButtonProperties); QVERIFY(object.address.isEmpty()); QCOMPARE(object.multiFunctionButton.commands.at(1).value, QStringLiteral("123")); QCOMPARE(object.multiFunctionButton.commands.at(2).operation, 1);
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("mf-general.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::mfCommandLimitDeletionAndEmptyDraft
 * Description     : Verify silent seven-item truncation, selected-row deletion, empty Lite persistence and draft cancellation.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::mfCommandLimitDeletionAndEmptyDraft() //20261112-0 add
{
    HmiObject original = mfObject();
    BitButtonDialog dialog(original, HmiProject());
    QListWidget *list = dialog.findChild<QListWidget *>(QStringLiteral("MfCommands"));
    HmiProject project;
    HmiProject loaded;
    QString error;
    int index;
    for (index = 0; index < 12; ++index) { dialog.findChild<QPushButton *>(QStringLiteral("MfAddBit"))->click(); }
    QCOMPARE(list->count(), 7); QCOMPARE(dialog.object().multiFunctionButton.commands.size(), 7);
    while (list->count() > 0) { dialog.findChild<QPushButton *>(QStringLiteral("MfRemove"))->click(); }
    QCOMPARE(list->currentRow(), -1); QVERIFY(!dialog.findChild<QGroupBox *>(QStringLiteral("MfBitGroup"))->isEnabled()); project.objects = {dialog.object()}; QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QVERIFY(loaded.objects.first().multiFunctionButton.commands.isEmpty());
    dialog.findChild<QPushButton *>(QStringLiteral("MfAddWord"))->click(); QCOMPARE(list->count(), 1); QCOMPARE(dialog.object().multiFunctionButton.commands.first().kind, 2); dialog.reject(); QCOMPARE(original.multiFunctionButton.commands.size(), 1); QCOMPARE(original.multiFunctionButton.commands.first().kind, 0);
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::mfDataTypeMappingAndFullSpans
 * Description     : Catch signed/unsigned enum drift and validate every type's complete 16/32-bit address carrier.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::mfDataTypeMappingAndFullSpans() //20261112-0 add
{
    MultiFunctionCommand command;
    MultiFunctionButtonProperties properties;
    QString error;
    QStringList valid = {QStringLiteral("-32768"), QStringLiteral("-2147483648"), QStringLiteral("65535"), QStringLiteral("4294967295"), QStringLiteral("9999"), QStringLiteral("99999999"), QStringLiteral("-1.25")};
    int index;
    command.kind = 2; command.operation = 1; command.address = QStringLiteral("LW0");
    for (index = 0; index < 7; ++index) { command.dataType = index; command.value = valid.at(index); QVERIFY2(command.validate(&error), qPrintable(error)); properties.commands = {command}; QVERIFY2(properties.validateAddresses({}, &error), qPrintable(error)); }
    command.dataType = 0; command.value = QStringLiteral("65535"); QVERIFY(!command.validate(&error)); command.dataType = 2; command.value = QStringLiteral("-1"); QVERIFY(!command.validate(&error)); command.dataType = 4; command.value = QStringLiteral("10000"); QVERIFY(!command.validate(&error)); command.operation = 0; QVERIFY2(command.validate(&error), qPrintable(error));
    command.address = QStringLiteral("LW4096"); command.dataType = 3; properties.commands = {command}; QVERIFY(!properties.validateAddresses({}, &error)); command.dataType = 2; properties.commands = {command}; QVERIFY2(properties.validateAddresses({}, &error), qPrintable(error)); command.address = QStringLiteral("LB0"); properties.commands = {command}; QVERIFY(!properties.validateAddresses({}, &error));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::mfPermissionsUnchosenAndActiveSelection
 * Description     : Preserve initial empty radios, auto-select an enabled condition and round-trip a real chosen zero independently.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::mfPermissionsUnchosenAndActiveSelection() //20261112-0 add
{
    HmiObject original = mfObject();
    BitButtonDialog dialog(original, HmiProject());
    HmiObject object;
    HmiObject loaded;
    QString error;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); dialog.findChild<QTabWidget *>(QStringLiteral("BitButtonTabs"))->setCurrentIndex(2);
    QVERIFY(!dialog.findChild<QRadioButton *>(QStringLiteral("BitVisibilityBitOn"))->isChecked()); QVERIFY(!dialog.findChild<QRadioButton *>(QStringLiteral("BitVisibilityBitOff"))->isChecked()); QVERIFY(!dialog.findChild<QRadioButton *>(QStringLiteral("BitTouchBitOn"))->isChecked()); QVERIFY(!dialog.findChild<QRadioButton *>(QStringLiteral("BitTouchBitOff"))->isChecked()); QVERIFY(!dialog.object().multiFunctionButton.visibilityBitChosen); QVERIFY(!dialog.findChild<QGroupBox *>(QStringLiteral("BitNotice"))->isVisible()); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("BitPressTime"))->value(), 0); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("BitPressTime"))->maximum(), 255); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("BitTouchMinimum"))->currentIndex(), 1);
    dialog.findChild<QGroupBox *>(QStringLiteral("BitVisibility"))->setChecked(true); dialog.findChild<QGroupBox *>(QStringLiteral("BitVisibilityBit"))->setChecked(true); QVERIFY(dialog.findChild<QRadioButton *>(QStringLiteral("BitVisibilityBitOn"))->isChecked()); QVERIFY(dialog.object().multiFunctionButton.visibilityBitChosen); QVERIFY(!dialog.object().multiFunctionButton.touchBitChosen);
    dialog.findChild<QGroupBox *>(QStringLiteral("BitTouch"))->setChecked(true); dialog.findChild<QGroupBox *>(QStringLiteral("BitTouchBit"))->setChecked(true); dialog.findChild<QRadioButton *>(QStringLiteral("BitTouchBitOff"))->click(); object = dialog.object(); QVERIFY(object.multiFunctionButton.touchBitChosen); QVERIFY(!object.multiFunctionButton.common.touch.bitState); QVERIFY2(HmiObject::fromJson(object.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.toJson(), object.toJson());
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("mf-permissions.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::mfPressedAppearanceLanguageAndCopies
 * Description     : Verify native single/two-state gating, default ON artwork and separate language/text/shape drafts.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::mfPressedAppearanceLanguageAndCopies() //20261112-0 add
{
    HmiProject project;
    HmiObject object = mfObject();
    BitButtonDialog *dialog;
    QSpinBox *state;
    QPlainTextEdit *text;
    QComboBox *language;
    project.languages = {QStringLiteral("中文"), QStringLiteral("English")}; dialog = new BitButtonDialog(object, project); dialog->show(); QVERIFY(QTest::qWaitForWindowExposed(dialog)); dialog->findChild<QTabWidget *>(QStringLiteral("BitButtonTabs"))->setCurrentIndex(1); state = dialog->findChild<QSpinBox *>(QStringLiteral("BitState")); text = dialog->findChild<QPlainTextEdit *>(QStringLiteral("BitText")); language = dialog->findChild<QComboBox *>(QStringLiteral("BitLanguage"));
    QVERIFY(!state->isEnabled()); QCOMPARE(state->maximum(), 0); QVERIFY(!dialog->findChild<QPushButton *>(QStringLiteral("BitCopyText"))->isEnabled()); QCOMPARE(text->toPlainText(), QString()); QCOMPARE(dialog->findChild<QLineEdit *>(QStringLiteral("BitPicture"))->text(), QStringLiteral("switch_ON.kvg")); QCOMPARE(dialog->findChild<QSpinBox *>(QStringLiteral("BitSize"))->maximum(), 128);
    text->setPlainText(QStringLiteral("常态")); language->setCurrentText(QStringLiteral("English")); text->setPlainText(QStringLiteral("Normal")); dialog->findChild<QCheckBox *>(QStringLiteral("MfPressedEffect"))->setChecked(true); QVERIFY(state->isEnabled()); state->setValue(1); language->setCurrentText(QStringLiteral("中文")); QCOMPARE(text->toPlainText(), QStringLiteral("ON")); text->setPlainText(QStringLiteral("按下")); dialog->findChild<QPushButton *>(QStringLiteral("BitNoImage"))->click(); dialog->findChild<QCheckBox *>(QStringLiteral("Bit3D"))->setChecked(true); state->setValue(0); QCOMPARE(text->toPlainText(), QStringLiteral("常态")); QVERIFY(!dialog->findChild<QCheckBox *>(QStringLiteral("Bit3D"))->isChecked()); QVERIFY(dialog->object().multiFunctionButton.common.states[0].useImage);
    dialog->findChild<QPushButton *>(QStringLiteral("BitCopyShape"))->click(); QVERIFY(!dialog->object().multiFunctionButton.common.states[0].useImage); state->setValue(1); dialog->findChild<QCheckBox *>(QStringLiteral("MfPressedEffect"))->setChecked(false); QCOMPARE(state->value(), 0); dialog->findChild<QCheckBox *>(QStringLiteral("MfPressedEffect"))->setChecked(true); state->setValue(1); QCOMPARE(text->toPlainText(), QStringLiteral("按下")); object = dialog->object(); QCOMPARE(bitButtonContent(object.multiFunctionButton.common.states[0], QStringLiteral("English")), QStringLiteral("Normal"));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog->grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("mf-appearance.png")))); } delete dialog;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::mfStrictMixedPersistenceAndBackendGates
 * Description     : Round-trip sixteen families, enforce typed schema/command limits and retain outputs when native backends reject MF.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::mfStrictMixedPersistenceAndBackendGates() //20261112-0 add
{
    HmiProject project;
    HmiProject loaded;
    HmiProject only;
    HmiObject object = mfObject();
    HmiObject parsed = qrObject();
    QJsonObject json;
    QJsonObject props;
    QJsonArray commands;
    MultiFunctionCommand command;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    command.kind = 2; command.address = QStringLiteral("LW10"); command.operation = 1; command.dataType = 2; command.value = QStringLiteral("65535"); object.multiFunctionButton.commands.append(command); command.kind = 1; command.operation = 0; command.screenPageId = project.activePageId; object.multiFunctionButton.commands.append(command);
    project.objects = existingFiveObjects(); project.objects.append({messageObject(), scaleObject(), qrObject(), imageObject(), gifObject(), numberObject(), asciiObject(), timeObject(), screenObject(project), multiObject(), object}); QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.objects.size(), 16); QCOMPARE(loaded.toJson(), project.toJson());
    only.objects = {object}; json = only.toJson(); QCOMPARE(json.value(QStringLiteral("schemaVersion")).toInt(), 12); QVERIFY2(HmiProject::fromJson(json, &only, &error), qPrintable(error)); json.insert(QStringLiteral("schemaVersion"), 11); QVERIFY(!HmiProject::fromJson(json, &only, &error)); QVERIFY(!exportNativeProjectSource(only, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(only, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")});
    json = object.toJson(); props = object.multiFunctionButton.toJson(); commands = props.value(QStringLiteral("commands")).toArray(); while (commands.size() < 8) { commands.append(MultiFunctionCommand().toJson()); } props.insert(QStringLiteral("commands"), commands); json.insert(QStringLiteral("multiFunctionButton"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); QCOMPARE(parsed.type, QStringLiteral("qrcode"));
    props = object.multiFunctionButton.toJson(); props.insert(QStringLiteral("pressedEffect"), 1); json.insert(QStringLiteral("multiFunctionButton"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); props = object.multiFunctionButton.toJson(); props.insert(QStringLiteral("future"), true); json.insert(QStringLiteral("multiFunctionButton"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY2(loaded.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("mf-mixed.ksp")), &error), qPrintable(error)); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::mfNavigationLifecycleAndClipboard
 * Description     : Protect every command target during rename/delete, page duplication and cross-project clipboard transactions.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::mfNavigationLifecycleAndClipboard() //20261112-0 add
{
    HmiProject project;
    HmiProject before;
    HmiProject target;
    HmiProject result;
    HmiPage page;
    HmiObject object = mfObject();
    MultiFunctionCommand command;
    Stage3Clipboard clipboard;
    QString copyId;
    QString error;
    page.id = QStringLiteral("mf-target"); page.name = QStringLiteral("目标画面"); QVERIFY2(project.addPage(page, &error), qPrintable(error)); command.kind = 1; command.operation = 0; command.screenPageId = page.id; object.multiFunctionButton.commands.append(command); project.objects = {object}; QVERIFY2(project.validateModel(&error), qPrintable(error)); QVERIFY2(project.renamePage(page.id, QStringLiteral("已改名"), &error), qPrintable(error)); QCOMPARE(project.objects.first().multiFunctionButton.commands.last().screenPageId, page.id); before = project; QVERIFY(!project.removePage(page.id, &error)); QCOMPARE(project.toJson(), before.toJson());
    QVERIFY2(captureStage3Selection(project, {object.id}, &clipboard, &error), qPrintable(error)); result = target; QVERIFY(!pasteStage3Selection(target, clipboard, QPoint(1, 1), &result, nullptr, &error)); QCOMPARE(result.toJson(), target.toJson());
    project.objects[0].multiFunctionButton.commands.last().screenPageId = project.activePageId; QVERIFY2(project.copyPage(project.activePageId, QStringLiteral("MF自跳转副本"), &copyId, &error), qPrintable(error)); QCOMPARE(project.inactivePages.last().objects.first().multiFunctionButton.commands.last().screenPageId, copyId);
    project.objects[0].multiFunctionButton.commands.last().operation = 2; QVERIFY(!project.validateModel(&error)); project.objects[0].multiFunctionButton.commands.last().operation = 1; QVERIFY2(project.removePage(page.id, &error), qPrintable(error));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::mfResourcesAndAddressIncrements
 * Description     : Protect dormant pressed-state images on all pages and increment each bit/word command with atomic span checks.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::mfResourcesAndAddressIncrements() //20261112-0 add
{
    HmiProject project;
    HmiProject result;
    HmiPage page;
    HmiObject object = mfObject();
    MultiFunctionCommand command;
    Stage3DuplicateOptions options;
    QString error;
    command.kind = 2; command.address = QStringLiteral("LW10"); command.operation = 1; command.dataType = 3; object.multiFunctionButton.commands.append(command); object.multiFunctionButton.common.states[1].picture = QStringLiteral("mf.png"); object.multiFunctionButton.common.states[1].picturePng = imagePng(Qt::green); project.pictures.insert(QStringLiteral("mf.png"), imagePng(Qt::green)); project.objects = {object}; page.id = QStringLiteral("mf-other-page"); page.name = QStringLiteral("MF副页"); object.id = QStringLiteral("mf-other"); page.objects = {object}; QVERIFY2(project.addPage(page, &error), qPrintable(error)); QCOMPARE(stage3PictureUsage(project, QStringLiteral("mf.png")).boundStates, 2); QVERIFY(!deleteStage3Picture(project, QStringLiteral("mf.png"), &result, &error)); QVERIFY2(renameStage3Picture(project, QStringLiteral("mf.png"), QStringLiteral("mf-new.png"), &result, &error), qPrintable(error)); QCOMPARE(result.inactivePages.first().objects.first().multiFunctionButton.common.states[1].picture, QStringLiteral("mf-new.png"));
    options.columns = 2; options.increments = {{Stage3AddressField::Operation, 2, 0}}; QVERIFY2(duplicateStage3Selection(project, {project.objects.first().id}, options, &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.objects.last().multiFunctionButton.commands.first().address, QStringLiteral("LB2")); QCOMPARE(result.objects.last().multiFunctionButton.commands.last().address, QStringLiteral("LW12")); QVERIFY(result.objects.last().address.isEmpty());
    project.objects[0].multiFunctionButton.commands.last().address = QStringLiteral("LW4095"); result = project; QVERIFY(!duplicateStage3Selection(project, {project.objects.first().id}, options, &result, nullptr, &error)); QCOMPARE(result.toJson(), project.toJson());
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::mfShellPlacementEditCompactAndUndo
 * Description     : Exercise real toolbox creation/cancel, two-state preview, compact bilingual edits and whole-document undo.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::mfShellPlacementEditCompactAndUndo() //20261112-0 add
{
    MainWindow window;
    HmiProject project;
    int history;
    project.languages = {QStringLiteral("中文"), QStringLiteral("English")}; project.activeLanguage = QStringLiteral("English"); window.setProject(project, QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); history = window.m_projectHistory.size(); QVERIFY(window.m_commands.contains(QStringLiteral("action_MFbtn")));
    QTimer::singleShot(0, &window, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } }); window.m_commands.value(QStringLiteral("action_MFbtn"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement());
    QTimer::singleShot(0, &window, []() { BitButtonDialog *dialog = qobject_cast<BitButtonDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QCheckBox *>(QStringLiteral("MfPressedEffect"))->setChecked(true); dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click(); } }); window.m_commands.value(QStringLiteral("action_MFbtn"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(200, 110))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(200, 110))); QCOMPARE(window.currentEditorProject().objects.first().type, QStringLiteral("multifunction")); QCOMPARE(window.findChild<QComboBox *>(QStringLiteral("NativeStatePreview"))->count(), 2); QCOMPARE(window.findChild<QLabel *>(QStringLiteral("NativeSelectionType"))->text(), QStringLiteral("多功能按钮")); QVERIFY(!window.m_addressEdit->isEnabled()); QCOMPARE(window.m_projectHistory.size(), history + 1);
    window.m_textEdit->setPlainText(QStringLiteral("MF English")); window.applySelectedProperties(); QCOMPARE(bitButtonContent(window.currentEditorProject().objects.first().multiFunctionButton.common.states[0], QStringLiteral("English")), QStringLiteral("MF English")); QCOMPARE(bitButtonContent(window.currentEditorProject().objects.first().multiFunctionButton.common.states[0], QStringLiteral("中文")), QString());
    QTimer::singleShot(0, &window, []() { BitButtonDialog *dialog = qobject_cast<BitButtonDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QPushButton *>(QStringLiteral("MfAddWord"))->click(); dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click(); } }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().multiFunctionButton.commands.size(), 2); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().objects.first().multiFunctionButton.commands.size(), 1); window.restoreProjectHistory(-1); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty());
}

/******************************************************************************
 * Function Name   : dropdownObject
 * Description     : Provide a dropdown fixture with independent rows and no generic button address or flags.
 * Input Parameters: None
 * Return Value    : Typed default dropdown object.
 * Date            : 2026.11.12
 ******************************************************************************/
static HmiObject dropdownObject() //20261112-0 add
{
    HmiObject object;
    object.id = QStringLiteral("dropdown-object"); object.type = QStringLiteral("dropdown"); object.name = QStringLiteral("下拉框"); object.geometry = QRect(450, 350, 160, 80); return object;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::dropdownDefaultsCountsAndRowLifetime
 * Description     : Check observed bounds and menu order, surviving row drafts, fresh regrowth and cancel isolation.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::dropdownDefaultsCountsAndRowLifetime() //20261112-0 add
{
    HmiObject original = dropdownObject();
    DropdownDialog dialog(original, HmiProject());
    QSpinBox *count = dialog.findChild<QSpinBox *>(QStringLiteral("DropdownCount"));
    QTableWidget *table = dialog.findChild<QTableWidget *>(QStringLiteral("DropdownTable"));
    QSpinBox *font = dialog.findChild<QSpinBox *>(QStringLiteral("DropdownFontSize"));
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(dialog.windowTitle(), QStringLiteral("下拉框")); QCOMPARE(dialog.findChild<QTabWidget *>(QStringLiteral("DropdownTabs"))->count(), 3); QCOMPARE(count->minimum(), 2); QCOMPARE(count->maximum(), 15); QCOMPARE(count->value(), 2); QCOMPARE(font->minimum(), 8); QCOMPARE(font->maximum(), 40); QCOMPARE(font->value(), 16); QCOMPARE(dialog.object().dropdown.background, QStringLiteral("#646464")); QCOMPARE(dialog.object().dropdown.textColor, QStringLiteral("#ffffff")); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("DropdownAlignment"))->currentIndex(), 2); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("DropdownDirection"))->itemText(1), QStringLiteral("向上")); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("DropdownDataType"))->count(), 7); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("DropdownDataType"))->itemText(0), QStringLiteral("16位正整数"));
    QCOMPARE(table->item(1, 2)->text(), QStringLiteral("state_1")); QVERIFY(!(table->item(0, 0)->flags() & Qt::ItemIsEditable)); table->item(0, 2)->setText(QStringLiteral("保留的第一行")); count->setValue(15); QCOMPARE(table->rowCount(), 15); QCOMPARE(table->item(14, 1)->text(), QStringLiteral("14")); table->item(14, 2)->setText(QStringLiteral("删除的尾行")); count->setValue(2); count->setValue(15); QCOMPARE(table->item(0, 2)->text(), QStringLiteral("保留的第一行")); QCOMPARE(table->item(14, 2)->text(), QStringLiteral("state_14"));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("dropdown-general.png")))); } dialog.reject(); QCOMPARE(original.dropdown.entries.size(), 2); QCOMPARE(original.dropdown.entries.first().label, QStringLiteral("state_0"));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::dropdownTableInlineEditingAndColorTransaction
 * Description     : Exercise a real table delegate, Enter commit without premature acceptance, and accepted/cancelled colors.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::dropdownTableInlineEditingAndColorTransaction() //20261112-0 add
{
    DropdownDialog dialog(dropdownObject(), HmiProject());
    QTableWidget *table = dialog.findChild<QTableWidget *>(QStringLiteral("DropdownTable"));
    QLineEdit *editor;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); dialog.findChild<QTabWidget *>(QStringLiteral("DropdownTabs"))->setCurrentIndex(1); table->setCurrentCell(0, 2); table->editItem(table->item(0, 2)); editor = table->findChild<QLineEdit *>(); QVERIFY(editor != nullptr); editor->selectAll(); QTest::keyClicks(editor, QStringLiteral("Edited A")); QCOMPARE(editor->text(), QStringLiteral("Edited A")); QTest::keyClick(editor, Qt::Key_Return); QTRY_COMPARE(dialog.object().dropdown.entries.first().label, QStringLiteral("Edited A")); QVERIFY(dialog.isVisible()); //20261112-0 change: Qt queues delegate commit after the Enter key event.
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("dropdown-list.png")))); }
    dialog.findChild<QTabWidget *>(QStringLiteral("DropdownTabs"))->setCurrentIndex(0); QTimer::singleShot(0, &dialog, []() { QDialog *color = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (color != nullptr) { color->reject(); } }); dialog.findChild<QPushButton *>(QStringLiteral("DropdownBackground"))->click(); QCOMPARE(dialog.object().dropdown.background, QStringLiteral("#646464"));
    QTimer::singleShot(0, &dialog, []() { QColorDialog *color = qobject_cast<QColorDialog *>(QApplication::activeModalWidget()); if (color != nullptr) { color->setCurrentColor(QColor(Qt::red)); color->accept(); } }); dialog.findChild<QPushButton *>(QStringLiteral("DropdownBackground"))->click(); QCOMPARE(dialog.object().dropdown.background, QStringLiteral("#ff0000"));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::dropdownTypesPermissionsAndFullCarriers
 * Description     : Validate signed and unsigned rows, unchosen/selected permissions, notification exclusivity and complete word spans.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::dropdownTypesPermissionsAndFullCarriers() //20261112-0 add
{
    HmiObject object = dropdownObject();
    DropdownDialog dialog(object, HmiProject());
    QString error;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); dialog.findChild<QTabWidget *>(QStringLiteral("DropdownTabs"))->setCurrentIndex(2); QVERIFY(dialog.findChild<QGroupBox *>(QStringLiteral("DropdownNotice"))->isVisible()); QVERIFY(!dialog.findChild<QCheckBox *>(QStringLiteral("DropdownNoticeBit"))->isEnabled()); QVERIFY(!dialog.findChild<QRadioButton *>(QStringLiteral("DropdownVisibilityBitOn"))->isChecked()); QVERIFY(!dialog.findChild<QRadioButton *>(QStringLiteral("DropdownTouchBitOff"))->isChecked()); QVERIFY(dialog.findChild<QCheckBox *>(QStringLiteral("BitPressEnabled")) == nullptr); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("DropdownTouchMinimum"))->currentIndex(), 1);
    dialog.findChild<QGroupBox *>(QStringLiteral("DropdownVisibility"))->setChecked(true); dialog.findChild<QGroupBox *>(QStringLiteral("DropdownVisibilityBit"))->setChecked(true); QVERIFY(dialog.findChild<QRadioButton *>(QStringLiteral("DropdownVisibilityBitOn"))->isChecked()); dialog.findChild<QRadioButton *>(QStringLiteral("DropdownVisibilityBitOff"))->click(); object = dialog.object(); QVERIFY(object.dropdown.visibilityBitChosen); QVERIFY(!object.dropdown.visibility.bitState);
    dialog.findChild<QGroupBox *>(QStringLiteral("DropdownNotice"))->setChecked(true); dialog.findChild<QCheckBox *>(QStringLiteral("DropdownNoticeBit"))->setChecked(true); dialog.findChild<QCheckBox *>(QStringLiteral("DropdownNoticeWord"))->setChecked(true); QVERIFY(!dialog.findChild<QCheckBox *>(QStringLiteral("DropdownNoticeBit"))->isChecked()); dialog.findChild<QComboBox *>(QStringLiteral("DropdownNoticeWordType"))->setCurrentIndex(1); dialog.findChild<QLineEdit *>(QStringLiteral("DropdownNoticeWordAddress"))->setText(QStringLiteral("LW4096")); QVERIFY(!dialog.object().dropdown.validateAddresses({}, &error));
    object = dropdownObject(); object.dropdown.entries[0].value = QStringLiteral("-32768"); object.dropdown.dataType = 2; QVERIFY2(object.dropdown.validate(&error), qPrintable(error)); object.dropdown.dataType = 0; QVERIFY(!object.dropdown.validate(&error)); object.dropdown.entries[0].value = QStringLiteral("65535"); QVERIFY2(object.dropdown.validate(&error), qPrintable(error)); object.dropdown.address = QStringLiteral("LW4096"); object.dropdown.dataType = 1; QVERIFY(!object.dropdown.validateAddresses({}, &error)); object.dropdown.dataType = 0; QVERIFY2(object.dropdown.validateAddresses({}, &error), qPrintable(error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("dropdown-permissions.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::dropdownMacroDraftLifecycleAndClipboard
 * Description     : Keep child saves local, protect cross-page macro identity and reject clipboard source collisions atomically.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::dropdownMacroDraftLifecycleAndClipboard() //20261112-0 add
{
    HmiProject project;
    HmiProject target;
    HmiProject result;
    HmiPage page;
    HmiObject object = dropdownObject();
    DropdownDialog dialog(object, project);
    Stage3Clipboard clipboard;
    QString error;
    dialog.findChild<QGroupBox *>(QStringLiteral("DropdownMacro"))->setChecked(true); QVERIFY(dialog.findChild<QPushButton *>(QStringLiteral("DropdownWriteMacro"))->isEnabled());
    QTimer::singleShot(0, &dialog, []() { QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (editor != nullptr) { editor->findChild<QLineEdit *>(QStringLiteral("MacroName"))->setText(QStringLiteral("DropdownMacroA")); editor->findChild<QPlainTextEdit *>(QStringLiteral("MacroSource"))->setPlainText(QStringLiteral("void Macro_Entry() {}")); editor->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Save)->click(); } }); dialog.findChild<QPushButton *>(QStringLiteral("DropdownWriteMacro"))->click(); QVERIFY(dialog.macros().contains(QStringLiteral("DropdownMacroA"))); QVERIFY(project.macros.isEmpty());
    object = dialog.object(); project.macros = dialog.macros(); project.objects = {object}; page.id = QStringLiteral("dropdown-other-page"); page.name = QStringLiteral("下拉框副页"); object.id = QStringLiteral("dropdown-other"); page.objects = {object}; QVERIFY2(project.addPage(page, &error), qPrintable(error)); QVERIFY2(project.renameMacro(QStringLiteral("DropdownMacroA"), QStringLiteral("DropdownMacroB"), &error), qPrintable(error)); QCOMPARE(project.objects.first().dropdown.macroName, QStringLiteral("DropdownMacroB")); QCOMPARE(project.inactivePages.first().objects.first().dropdown.macroName, QStringLiteral("DropdownMacroB")); result = project; result.macros.clear(); result.macroIds.clear(); QVERIFY(!result.validateModel(&error));
    QVERIFY2(captureStage3Selection(project, {project.objects.first().id}, &clipboard, &error), qPrintable(error)); QVERIFY2(pasteStage3Selection(target, clipboard, QPoint(1, 1), &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.macros.value(QStringLiteral("DropdownMacroB")), QStringLiteral("void Macro_Entry() {}")); target.macros.insert(QStringLiteral("DropdownMacroB"), QStringLiteral("different")); result = target; QVERIFY(!pasteStage3Selection(target, clipboard, QPoint(1, 1), &result, nullptr, &error)); QCOMPARE(result.toJson(), target.toJson()); dialog.reject(); QVERIFY(object.dropdown.macroEnabled);
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::dropdownStrictMixedPersistenceAndBackendGates
 * Description     : Round-trip seventeen control families and reject malformed rows/schema and unrecovered native outputs without data loss.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::dropdownStrictMixedPersistenceAndBackendGates() //20261112-0 add
{
    HmiProject project;
    HmiProject loaded;
    HmiProject only;
    HmiObject object = dropdownObject();
    HmiObject parsed = qrObject();
    QJsonObject json;
    QJsonObject props;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    object.dropdown.setCount(15); object.dropdown.entries[14].label = QStringLiteral("中文项目14"); project.objects = existingFiveObjects(); project.objects.append({messageObject(), scaleObject(), qrObject(), imageObject(), gifObject(), numberObject(), asciiObject(), timeObject(), screenObject(project), multiObject(), mfObject(), object}); QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.objects.size(), 17); QCOMPARE(loaded.toJson(), project.toJson());
    only.objects = {object}; json = only.toJson(); QCOMPARE(json.value(QStringLiteral("schemaVersion")).toInt(), 12); QVERIFY2(HmiProject::fromJson(json, &only, &error), qPrintable(error)); json.insert(QStringLiteral("schemaVersion"), 11); QVERIFY(!HmiProject::fromJson(json, &only, &error)); QVERIFY(!exportNativeProjectSource(only, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(only, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")});
    json = object.toJson(); props = object.dropdown.toJson(); props.insert(QStringLiteral("entries"), QJsonArray{QJsonObject{{QStringLiteral("value"), 1}, {QStringLiteral("label"), QStringLiteral("bad")}}}); json.insert(QStringLiteral("dropdown"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); QCOMPARE(parsed.type, QStringLiteral("qrcode")); props = object.dropdown.toJson(); props.insert(QStringLiteral("fontSize"), 16.5); json.insert(QStringLiteral("dropdown"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY2(loaded.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("dropdown-mixed.ksp")), &error), qPrintable(error)); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::dropdownActualFaceAndDirectionRaster
 * Description     : Check bounded actual pixels, the native blank editor face and directional arrow geometry without a simulated PLC selection.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::dropdownActualFaceAndDirectionRaster() //20261112-0 add
{
    DropdownProperties properties;
    QImage down(180, 100, QImage::Format_ARGB32);
    QImage up(180, 100, QImage::Format_ARGB32);
    QPainter painter;
    down.fill(Qt::transparent); up.fill(Qt::transparent); painter.begin(&down); paintDropdown(&painter, QRectF(10, 10, 160, 80), properties); painter.end(); properties.direction = 1; painter.begin(&up); paintDropdown(&painter, QRectF(10, 10, 160, 80), properties); painter.end(); QCOMPARE(down.pixelColor(25, 50), QColor(QStringLiteral("#646464"))); QCOMPARE(down.pixelColor(160, 75), QColor(QStringLiteral("#c0c0c0"))); QCOMPARE(down.pixelColor(139, 20), QColor(Qt::black)); QCOMPARE(up.pixelColor(139, 20), QColor(QStringLiteral("#c0c0c0"))); QCOMPARE(up.pixelColor(139, 47), QColor(Qt::black)); QCOMPARE(down.pixelColor(5, 5).alpha(), 0);
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(down.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("dropdown-down.png")))); QVERIFY(up.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("dropdown-up.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::dropdownShellMacroPlacementCompactAndUndo
 * Description     : Keep accepted macro resources pending until placement completes, then test compact addresses, edits and undo as whole transactions.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::dropdownShellMacroPlacementCompactAndUndo() //20261112-0 add
{
    MainWindow window;
    HmiProject project;
    HmiObject object;
    HmiProject result;
    Stage3DuplicateOptions options;
    QString error;
    auto acceptWithNewMacro = []() {
        DropdownDialog *dialog = qobject_cast<DropdownDialog *>(QApplication::activeModalWidget());
        if (dialog == nullptr) { return; }
        dialog->findChild<QGroupBox *>(QStringLiteral("DropdownMacro"))->setChecked(true);
        QTimer::singleShot(0, dialog, []() { QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (editor != nullptr) { editor->findChild<QLineEdit *>(QStringLiteral("MacroName"))->setText(QStringLiteral("PendingDropdownMacro")); editor->findChild<QPlainTextEdit *>(QStringLiteral("MacroSource"))->setPlainText(QStringLiteral("void Macro_Entry() {}")); editor->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Save)->click(); } });
        dialog->findChild<QPushButton *>(QStringLiteral("DropdownWriteMacro"))->click(); dialog->findChild<QLineEdit *>(QStringLiteral("DropdownAddress"))->setText(QStringLiteral("LW10")); dialog->findChild<QDialogButtonBox *>(QStringLiteral("DropdownButtons"))->button(QDialogButtonBox::Ok)->click();
    };
    project.macros.insert(QStringLiteral("ExistingDropdownMacro"), QStringLiteral("void Macro_Entry() {}")); window.setProject(project, QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); QVERIFY(window.m_commands.contains(QStringLiteral("action_droplist")));
    QTimer::singleShot(0, &window, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } }); window.m_commands.value(QStringLiteral("action_droplist"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement());
    QTimer::singleShot(0, &window, acceptWithNewMacro); window.m_commands.value(QStringLiteral("action_droplist"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QVERIFY(window.currentEditorProject().objects.isEmpty()); QVERIFY(!window.currentEditorProject().macros.contains(QStringLiteral("PendingDropdownMacro"))); QTest::keyClick(window.m_canvas->viewport(), Qt::Key_Escape); QVERIFY(!window.m_canvas->hasPendingPlacement()); QVERIFY(window.currentEditorProject().objects.isEmpty()); QVERIFY(!window.currentEditorProject().macros.contains(QStringLiteral("PendingDropdownMacro")));
    QTimer::singleShot(0, &window, acceptWithNewMacro); window.m_commands.value(QStringLiteral("action_droplist"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(200, 110))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(200, 110))); QVERIFY(window.currentEditorProject().macros.contains(QStringLiteral("PendingDropdownMacro"))); QCOMPARE(window.findChild<QLabel *>(QStringLiteral("NativeSelectionType"))->text(), QStringLiteral("下拉框")); QVERIFY(!window.m_textEdit->isEnabled()); QCOMPARE(window.m_addressEdit->text(), QStringLiteral("LW10")); window.m_addressEdit->setText(QStringLiteral("LW12")); window.applySelectedProperties(); QVERIFY(window.currentEditorProject().objects.first().address.isEmpty()); QCOMPARE(window.currentEditorProject().objects.first().dropdown.address, QStringLiteral("LW12"));
    QTimer::singleShot(0, &window, []() { DropdownDialog *dialog = qobject_cast<DropdownDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QSpinBox *>(QStringLiteral("DropdownCount"))->setValue(7); dialog->findChild<QDialogButtonBox *>(QStringLiteral("DropdownButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().dropdown.entries.size(), 7); options.columns = 2; options.increments = {{Stage3AddressField::Operation, 2, 0}}; QVERIFY2(duplicateStage3Selection(window.currentEditorProject(), {window.currentEditorProject().objects.first().id}, options, &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.objects.last().dropdown.address, QStringLiteral("LW14")); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().objects.first().dropdown.entries.size(), 2); window.restoreProjectHistory(-1); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty()); QVERIFY(!window.currentEditorProject().macros.contains(QStringLiteral("PendingDropdownMacro")));
}

QTEST_MAIN(Stage4ControlTests)
#include "stage4_controls_tests.moc"
