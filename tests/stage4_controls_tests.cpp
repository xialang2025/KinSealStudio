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
#include "flow_block_dialog.h" //20261112-0 add
#include "slider_dialog.h" //20261112-0 add
#include "wheel_dialog.h" //20261112-0 add
#include "history_data_dialog.h" //20261113-2 add
#include "alarm_display_dialog.h" //20261113-2 add
#include "alarm_bar_dialog.h" //20261113-2 add
#include "alarm_registration_dialog.h"
#include "data_group_dialog.h" //20261113-1 add
#include "xy_curve_dialog.h" //20261113-1 add
#include "history_trend_dialog.h" //20261112-1 add
#include "data_collection_dialog.h"
#include <QGroupBox>
#include <QTreeWidget>
#include "trend_dialog.h" //20261112-1 add
#include "chart_dialog.h" //20261112-1 add
#include "move_track_dialog.h" //20261112-0 add
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
    void flowDefaultsAndMeasuredOrientationResets(); //20261112-0 add
    void flowColorPickersAndNativeBorderEnablement();
    void flowVisibilityAcceptanceAndAddressRoles();
    void flowStrictMixedPersistenceAndBackendGates();
    void flowActualRasterCountPatternsAndClipping();
    void flowShellPlacementEditingCompactAndUndo();
    void sliderDefaultsStylesColorsAndTransparency(); //20261112-0 add
    void sliderRangeCacheAndPointingGeometry();
    void sliderFiveTypeMappingAndAddressSpans();
    void sliderPermissionsNotificationsAndAddressCopies();
    void sliderMacroDependenciesAndClipboard();
    void sliderStrictMixedPersistenceAndBackendGates();
    void sliderCenteredRasterAndClipping();
    void sliderShellMacroPlacementGeometryAndUndo();
    void wheelNativeBoundsAndIndependentDrafts(); //20261112-0 add
    void historyDataMeasuredDefaultsAndZeroRows(); //20261113-2 add
    void historyDataAllColumnLanguageSynchronization(); //20261113-2 add
    void historyDataVisibleColumnGuard(); //20261113-2 add
    void historyDataStrictRoundTripAndBackendGates(); //20261113-2 add
    void historyDataCollectorReconciliationAcrossPages(); //20261113-2 add
    void historyDataClipboardAndFullVisibilitySpans(); //20261113-2 add
    void historyDataRendererGridAndZeroRowClipping(); //20261113-2 add
    void historyDataShellPlacementCollectionEditAndUndo(); //20261113-2 add
    void historyAlarmSharedDialogKeepsDistinctIdentity(); //20261113-2 add
    void historyAlarmStrictTypeLanguagesAndBackendGates(); //20261113-2 add
    void historyAlarmWordVisibilityAndCopyDependencies(); //20261113-2 add
    void historyAlarmShellEntryPlacementEditAndUndo(); //20261113-2 add
    void alarmDisplayDefaultsAndStrictRecords(); //20261113-2 add
    void alarmDisplayCaptionLanguagesAndCommonStyles(); //20261113-2 add
    void alarmDisplaySharedPrerequisitesCopyAndVisibility(); //20261113-2 add
    void alarmDisplayPersistenceLanguagesAndBackendGates(); //20261113-2 add
    void alarmDisplayStaticColumnsFontsAndClipping(); //20261113-2 add
    void alarmDisplayShellGatePlacementEditingAndUndo(); //20261113-2 add
    void alarmDefinitionsModesBoundsAndStrictRows(); //20261113-2 add
    void alarmRegistrationRowsLanguagesAndCommonType(); //20261113-2 add
    void alarmPrerequisitesNavigationAndLanguageTransactions(); //20261113-2 add
    void alarmBarDialogFieldsPermissionsAndCancel(); //20261113-2 add
    void alarmStrictProjectPersistenceAndBackendGates(); //20261113-2 add
    void alarmClipboardDefinitionsAndIndependentVisibility(); //20261113-2 add
    void alarmStaticSamplePatternsAndClipping(); //20261113-2 add
    void alarmShellGateRegistrationPlacementAndUndo(); //20261113-2 add
    void groupNativeDefaultsAndSingleChannelLimits(); //20261113-1 add
    void groupSampleAndMaximumDraftLifetime(); //20261113-1 add
    void groupPointBufferAndIndependentAddressRoles(); //20261113-1 add
    void groupStrictPersistenceAndBackendGates(); //20261113-1 add
    void groupStaticFillOwnershipAndClipping(); //20261113-1 add
    void groupShellPlacementEditAndUndo(); //20261113-1 add
    void xyNativeBoundsAndDataTypeLifetime(); //20261113-1 add
    void xyIndependentChannelsAndRemovalReset(); //20261113-1 add
    void xyAxisHiddenFieldsAndAccessSpans(); //20261113-1 add
    void xyStrictPersistenceAndBackendGates(); //20261113-1 add
    void xyStaticRasterAndClipping(); //20261113-1 add
    void xyShellPlacementEditAndUndo(); //20261113-1 add
    void collectionSharedTypeMemoryAndFullAddressSpan(); //20261112-1 add
    void collectionDialogItemsAndInactiveDrafts(); //20261112-1 add
    void historyIndependentCurvesAndStableNames(); //20261112-1 add
    void historyCheckableAxesAndTimeBounds(); //20261112-1 add
    void historyReferenceLifetimeAndSingleCollectorLimit(); //20261112-1 add
    void historyCrossProjectPasteAndVisibilityRoles(); //20261112-1 add
    void historyStrictPersistenceAndBackendGates(); //20261112-1 add
    void historyStaticRasterAndTimeFormatClipping(); //20261112-1 add
    void historyShellGateCollectionPlacementAndUndo(); //20261112-1 add
    void trendNativeBoundsAndDataTypeLifetime(); //20261112-1 add
    void trendIndependentChannelsAndRemovalReset();
    void trendAxisHiddenFieldsAndAccessSpans();
    void trendStrictPersistenceAndBackendGates();
    void trendStaticRasterAndClipping();
    void trendShellPlacementEditAndUndo();
    void chartDefaultsAndIndependentFormDrafts(); //20261112-1 add
    void chartTypeRangesAndNativeLinks();
    void chartAccessSpansAndAddressCopies();
    void chartStrictPersistenceAndBackendGates();
    void chartStaticRasterAndClipping();
    void chartShellPlacementEditAndUndo();
    void trackDefaultsAndRangeDraftLifetime(); //20261112-0 add
    void trackPermissionsNotificationsAndAddressCopies();
    void trackResourceTransactionsAndCopyConflicts();
    void trackStrictMixedPersistenceAndBackendGates();
    void trackStaticRasterAndClipping();
    void trackShellPlacementEditAndUndo();
    void wheelPermissionsAcceptanceAndAddressRoles();
    void wheelGalleryBindingsCopiesAndResourceProtection();
    void wheelStrictMixedPersistenceAndBackendGates();
    void wheelMidpointRasterFontsAlphaAndClipping();
    void wheelShellPlacementPicturesCompactAndUndo();
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

/******************************************************************************
 * Function Name   : flowObject
 * Description     : Provide a display-only flow fixture with an independent bit trigger and no button carrier.
 * Input Parameters: None
 * Return Value    : Typed fresh flow object.
 * Date            : 2026.11.12
 ******************************************************************************/
static HmiObject flowObject() //20261112-0 add
{
    HmiObject object;
    object.id = QStringLiteral("flow-object"); object.type = QStringLiteral("flow"); object.name = QStringLiteral("流动块"); object.geometry = QRect(620, 350, 160, 60); return object;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::flowDefaultsAndMeasuredOrientationResets
 * Description     : Regress real measured bounds and reset direction/style instead of incorrectly preserving their old indices.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::flowDefaultsAndMeasuredOrientationResets() //20261112-0 add
{
    FlowBlockDialog dialog(flowObject(), HmiProject());
    QComboBox *pattern = dialog.findChild<QComboBox *>(QStringLiteral("FlowPattern"));
    QComboBox *direction = dialog.findChild<QComboBox *>(QStringLiteral("FlowDirection"));
    QSpinBox *count = dialog.findChild<QSpinBox *>(QStringLiteral("FlowCount"));
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(dialog.windowTitle(), QStringLiteral("流动块")); QCOMPARE(dialog.findChild<QTabWidget *>(QStringLiteral("FlowTabs"))->count(), 2); QCOMPARE(count->minimum(), 1); QCOMPARE(count->maximum(), 30); QCOMPARE(count->value(), 3); count->setValue(0); QCOMPARE(count->value(), 1); count->setValue(31); QCOMPARE(count->value(), 30); QCOMPARE(pattern->count(), 13); QCOMPARE(pattern->itemText(11), QStringLiteral("左上角辐射")); QCOMPARE(pattern->currentIndex(), 5); QCOMPARE(direction->itemText(0), QStringLiteral("向左")); QVERIFY(dialog.findChild<QRadioButton *>(QStringLiteral("FlowSpeed1"))->isChecked());
    direction->setCurrentIndex(1); pattern->setCurrentIndex(2); dialog.findChild<QRadioButton *>(QStringLiteral("FlowVertical"))->click(); QCOMPARE(direction->currentIndex(), 0); QCOMPARE(direction->itemText(0), QStringLiteral("向上")); QCOMPARE(pattern->currentIndex(), 3); pattern->setCurrentIndex(9); direction->setCurrentIndex(1); dialog.findChild<QRadioButton *>(QStringLiteral("FlowHorizontal"))->click(); QCOMPARE(direction->currentIndex(), 0); QCOMPARE(pattern->currentIndex(), 5); dialog.findChild<QRadioButton *>(QStringLiteral("FlowSpeed0"))->click(); QCOMPARE(dialog.object().flowBlock.speed, 0); QVERIFY(!dialog.object().hasBitButtonProperties); QVERIFY(dialog.object().address.isEmpty());
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("flow-appearance.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::flowColorPickersAndNativeBorderEnablement
 * Description     : Check all five independent color slots and preserve native frame-color editing even when no border is selected.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::flowColorPickersAndNativeBorderEnablement() //20261112-0 add
{
    HmiObject original = flowObject();
    FlowBlockDialog dialog(original, HmiProject());
    QPushButton *frame = dialog.findChild<QPushButton *>(QStringLiteral("FlowColor4"));
    int index;
    QVERIFY(frame->isEnabled()); QVERIFY(dialog.findChild<QCheckBox *>(QStringLiteral("FlowNoBorder"))->isChecked()); dialog.findChild<QCheckBox *>(QStringLiteral("FlowNoBorder"))->setChecked(false); QVERIFY(frame->isEnabled());
    for (index = 0; index < 5; ++index)
    {
        QTimer::singleShot(0, &dialog, []() { QColorDialog *color = qobject_cast<QColorDialog *>(QApplication::activeModalWidget()); if (color != nullptr) { color->setCurrentColor(QColor(Qt::blue)); color->accept(); } }); dialog.findChild<QPushButton *>(QStringLiteral("FlowColor%1").arg(index))->click();
    }
    QCOMPARE(dialog.object().flowBlock.flowFront, QStringLiteral("#0000ff")); QCOMPARE(dialog.object().flowBlock.flowBack, QStringLiteral("#0000ff")); QCOMPARE(dialog.object().flowBlock.pipeFront, QStringLiteral("#0000ff")); QCOMPARE(dialog.object().flowBlock.pipeBack, QStringLiteral("#0000ff")); QCOMPARE(dialog.object().flowBlock.borderColor, QStringLiteral("#0000ff"));
    QTimer::singleShot(0, &dialog, []() { QDialog *color = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (color != nullptr) { color->reject(); } }); frame->click(); QCOMPARE(dialog.object().flowBlock.borderColor, QStringLiteral("#0000ff")); dialog.reject(); QCOMPARE(original.flowBlock.flowBack, QStringLiteral("#ff0000")); QCOMPARE(original.flowBlock.borderColor, QStringLiteral("#000000"));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::flowVisibilityAcceptanceAndAddressRoles
 * Description     : Distinguish fresh empty radios from the observed accepted default, protect chosen zero and validate pure display carriers.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::flowVisibilityAcceptanceAndAddressRoles() //20261112-0 add
{
    HmiProject project;
    HmiProject result;
    HmiObject object = flowObject();
    FlowBlockDialog dialog(object, project);
    Stage3DuplicateOptions options;
    QString error;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); dialog.findChild<QTabWidget *>(QStringLiteral("FlowTabs"))->setCurrentIndex(1); QVERIFY(!dialog.findChild<QRadioButton *>(QStringLiteral("FlowVisibilityBitOn"))->isChecked()); QVERIFY(!dialog.findChild<QRadioButton *>(QStringLiteral("FlowVisibilityBitOff"))->isChecked()); QVERIFY(dialog.findChild<QGroupBox *>(QStringLiteral("DropdownNotice")) == nullptr); QVERIFY(!dialog.object().flowBlock.visibilityBitChosen);
    dialog.findChild<QDialogButtonBox *>(QStringLiteral("FlowButtons"))->button(QDialogButtonBox::Ok)->click(); object = dialog.object(); QVERIFY(object.flowBlock.visibilityBitChosen); QVERIFY(object.flowBlock.visibility.bitState); FlowBlockDialog reopened(object, project); reopened.show(); QVERIFY(QTest::qWaitForWindowExposed(&reopened)); reopened.findChild<QTabWidget *>(QStringLiteral("FlowTabs"))->setCurrentIndex(1); QCOMPARE(reopened.findChildren<HmiPermissionEditor *>().size(), 1); QVERIFY(reopened.findChild<QRadioButton *>(QStringLiteral("FlowVisibilityBitOn"))->isChecked()); reopened.findChild<QGroupBox *>(QStringLiteral("FlowVisibility"))->setChecked(true); reopened.findChild<QGroupBox *>(QStringLiteral("FlowVisibilityBit"))->setChecked(true); reopened.findChild<QRadioButton *>(QStringLiteral("FlowVisibilityBitOff"))->click(); object = reopened.object(); QVERIFY(!object.flowBlock.visibility.bitState); object.flowBlock.triggerAddress = QStringLiteral("LB10"); object.flowBlock.visibility.bitAddress = QStringLiteral("LB20"); project.objects = {object}; options.columns = 2; options.offsetX = -16; options.increments = {{Stage3AddressField::MonitorBit, 2, 0}, {Stage3AddressField::VisibilityBit, 3, 0}}; QVERIFY2(duplicateStage3Selection(project, {object.id}, options, &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.objects.last().flowBlock.triggerAddress, QStringLiteral("LB12")); QCOMPARE(result.objects.last().flowBlock.visibility.bitAddress, QStringLiteral("LB23"));
    object.flowBlock.triggerAddress = QStringLiteral("LW0"); QVERIFY(!object.flowBlock.validateAddresses({}, &error)); object.flowBlock.triggerAddress = QStringLiteral("LB0"); object.flowBlock.visibility.wordEnabled = true; object.flowBlock.visibility.dataType = 1; object.flowBlock.visibility.wordAddress = QStringLiteral("LW4096"); QVERIFY(!object.flowBlock.validateAddresses({}, &error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(reopened.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("flow-permissions.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::flowStrictMixedPersistenceAndBackendGates
 * Description     : Round-trip eighteen families and flow-only schema12 while refusing malformed enums/colors and unrecovered source/device output.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::flowStrictMixedPersistenceAndBackendGates() //20261112-0 add
{
    HmiProject project;
    HmiProject loaded;
    HmiProject only;
    HmiObject object = flowObject();
    HmiObject parsed = qrObject();
    QJsonObject json;
    QJsonObject props;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    object.flowBlock.setVertical(true); object.flowBlock.direction = 1; object.flowBlock.pattern = 11; object.flowBlock.visibilityBitChosen = true; project.objects = existingFiveObjects(); project.objects.append({messageObject(), scaleObject(), qrObject(), imageObject(), gifObject(), numberObject(), asciiObject(), timeObject(), screenObject(project), multiObject(), mfObject(), dropdownObject(), object}); QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.objects.size(), 18); QCOMPARE(loaded.toJson(), project.toJson()); only.objects = {object}; json = only.toJson(); QCOMPARE(json.value(QStringLiteral("schemaVersion")).toInt(), 12); QVERIFY2(HmiProject::fromJson(json, &only, &error), qPrintable(error)); json.insert(QStringLiteral("schemaVersion"), 11); QVERIFY(!HmiProject::fromJson(json, &only, &error)); QVERIFY(!exportNativeProjectSource(only, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(only, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")});
    json = object.toJson(); props = object.flowBlock.toJson(); props.insert(QStringLiteral("count"), 31); json.insert(QStringLiteral("flowBlock"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); QCOMPARE(parsed.type, QStringLiteral("qrcode")); props = object.flowBlock.toJson(); props.insert(QStringLiteral("flowBack"), QStringLiteral("bad-color")); json.insert(QStringLiteral("flowBlock"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); props = object.flowBlock.toJson(); props.insert(QStringLiteral("pattern"), 1.5); json.insert(QStringLiteral("flowBlock"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY2(loaded.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("flow-mixed.ksp")), &error), qPrintable(error)); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::flowActualRasterCountPatternsAndClipping
 * Description     : Count real static blocks and exercise all named PC brushes while checking border/transparent behavior and clipping.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::flowActualRasterCountPatternsAndClipping() //20261112-0 add
{
    FlowBlockProperties properties;
    QImage image(380, 80, QImage::Format_ARGB32);
    QPainter painter;
    int x;
    int groups = 0;
    int pattern;
    bool previous = false;
    bool red;
    image.fill(Qt::transparent); properties.pattern = 1; properties.count = 30; painter.begin(&image); paintFlowBlock(&painter, QRectF(10, 10, 360, 60), properties); painter.end();
    for (x = 10; x < 370; ++x) { red = image.pixelColor(x, 40) == QColor(Qt::red); if (red && !previous) { ++groups; } previous = red; } QCOMPARE(groups, 30); QCOMPARE(image.pixelColor(5, 5).alpha(), 0);
    for (pattern = 0; pattern < 13; ++pattern) { image.fill(Qt::transparent); properties.pattern = pattern; painter.begin(&image); paintFlowBlock(&painter, QRectF(10, 10, 360, 60), properties); painter.end(); QCOMPARE(image.pixelColor(5, 5).alpha(), 0); }
    image.fill(Qt::transparent); properties.pattern = 0; properties.noBorder = false; painter.begin(&image); paintFlowBlock(&painter, QRectF(10, 10, 360, 60), properties); painter.end(); QCOMPARE(image.pixelColor(100, 40).alpha(), 0); QVERIFY(image.pixelColor(100, 10) == QColor(Qt::black) || image.pixelColor(100, 11) == QColor(Qt::black));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(image.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("flow-transparent-frame.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::flowShellPlacementEditingCompactAndUndo
 * Description     : Exercise real control entry, cancellation, display-only sidebars, orientation editing and whole-document undo.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::flowShellPlacementEditingCompactAndUndo() //20261112-0 add
{
    MainWindow window;
    window.setProject(HmiProject(), QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); QVERIFY(window.m_commands.contains(QStringLiteral("action_flow_block"))); QTimer::singleShot(0, &window, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } }); window.m_commands.value(QStringLiteral("action_flow_block"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement());
    QTimer::singleShot(0, &window, []() { FlowBlockDialog *dialog = qobject_cast<FlowBlockDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QDialogButtonBox *>(QStringLiteral("FlowButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.m_commands.value(QStringLiteral("action_flow_block"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(200, 110))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(200, 110))); QCOMPARE(window.findChild<QLabel *>(QStringLiteral("NativeSelectionType"))->text(), QStringLiteral("流动块")); QVERIFY(!window.m_textEdit->isEnabled()); QCOMPARE(window.m_addressEdit->text(), QStringLiteral("LB0")); window.m_addressEdit->setText(QStringLiteral("LB8")); window.applySelectedProperties(); QCOMPARE(window.currentEditorProject().objects.first().flowBlock.triggerAddress, QStringLiteral("LB8")); QVERIFY(window.currentEditorProject().objects.first().address.isEmpty());
    QTimer::singleShot(0, &window, []() { FlowBlockDialog *dialog = qobject_cast<FlowBlockDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QRadioButton *>(QStringLiteral("FlowVertical"))->click(); dialog->findChild<QDialogButtonBox *>(QStringLiteral("FlowButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.editStage4Control(); QVERIFY(window.currentEditorProject().objects.first().flowBlock.vertical); window.restoreProjectHistory(-1); QVERIFY(!window.currentEditorProject().objects.first().flowBlock.vertical); window.restoreProjectHistory(-1); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty());
}

/******************************************************************************
 * Function Name   : sliderObject
 * Description     : Provide an independent slider fixture with the measured default perpendicular envelope.
 * Input Parameters: None
 * Return Value    : Typed slider object.
 * Date            : 2026.11.12
 ******************************************************************************/
static HmiObject sliderObject() //20261112-0 add
{
    HmiObject object;
    object.id = QStringLiteral("slider-object"); object.type = QStringLiteral("slider"); object.name = QStringLiteral("滑动开关"); object.geometry = QRect(20, 260, 200, 54); return object;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::sliderDefaultsStylesColorsAndTransparency
 * Description     : Exercise two observed pages, native defaults, all editable style dimensions and local color transactions.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::sliderDefaultsStylesColorsAndTransparency() //20261112-0 add
{
    SliderDialog dialog(sliderObject(), HmiProject());
    QComboBox *style = dialog.findChild<QComboBox *>(QStringLiteral("SliderStyle"));
    QSpinBox *width = dialog.findChild<QSpinBox *>(QStringLiteral("SliderThumbWidth"));
    QSpinBox *height = dialog.findChild<QSpinBox *>(QStringLiteral("SliderThumbHeight"));
    QSpinBox *rail = dialog.findChild<QSpinBox *>(QStringLiteral("SliderRailHeight"));
    int index;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(dialog.windowTitle(), QStringLiteral("滑动开关")); QCOMPARE(dialog.findChild<QTabWidget *>(QStringLiteral("SliderTabs"))->count(), 2); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("SliderDataType"))->count(), 5); QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("SliderAddress"))->text(), QStringLiteral("LW0")); QCOMPARE(style->currentIndex(), 1); QCOMPARE(width->value(), 10); QCOMPARE(height->value(), 50); QCOMPARE(rail->value(), 10); QCOMPARE(width->minimum(), 1); QCOMPARE(height->maximum(), 800); QCOMPARE(rail->maximum(), 800); QVERIFY(dialog.findChild<QLabel *>(QStringLiteral("SliderPreview")) == nullptr);
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("slider-general.png")))); }
    for (index = 0; index < 4; ++index) { style->setCurrentIndex(index); QVERIFY(width->isEnabled()); QVERIFY(height->isEnabled()); dialog.findChild<QCheckBox *>(QStringLiteral("SliderTransparent%1").arg(index))->setChecked(true); }
    for (index = 0; index < 7; ++index) { QVERIFY(dialog.findChild<QPushButton *>(QStringLiteral("SliderColor%1").arg(index))->isEnabled()); }
    QTimer::singleShot(0, &dialog, []() { QDialog *child = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (child != nullptr) { child->reject(); } }); dialog.findChild<QPushButton *>(QStringLiteral("SliderColor0"))->click(); QCOMPARE(dialog.object().slider.background, QStringLiteral("#f0f0f0"));
    QTimer::singleShot(0, &dialog, []() { QColorDialog *child = qobject_cast<QColorDialog *>(QApplication::activeModalWidget()); if (child != nullptr) { child->setCurrentColor(QColor(Qt::red)); child->accept(); } }); dialog.findChild<QPushButton *>(QStringLiteral("SliderColor0"))->click(); QCOMPARE(dialog.object().slider.background, QStringLiteral("#ff0000")); QCOMPARE(dialog.object().slider.railBackground, QStringLiteral("#00ff00")); QVERIFY(dialog.object().slider.backgroundTransparent); dialog.reject();
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::sliderRangeCacheAndPointingGeometry
 * Description     : Reproduce measured constant caching, discarded dynamic edits, overlapping U32 defaults and axis geometry changes.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::sliderRangeCacheAndPointingGeometry() //20261112-0 add
{
    SliderDialog dialog(sliderObject(), HmiProject());
    QLineEdit *minimum = dialog.findChild<QLineEdit *>(QStringLiteral("SliderMinimum"));
    QLineEdit *maximum = dialog.findChild<QLineEdit *>(QStringLiteral("SliderMaximum"));
    QCheckBox *dynamic = dialog.findChild<QCheckBox *>(QStringLiteral("SliderDynamic"));
    QComboBox *direction = dialog.findChild<QComboBox *>(QStringLiteral("SliderDirection"));
    SliderProperties properties;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); minimum->setText(QStringLiteral("7")); maximum->setText(QStringLiteral("88")); dynamic->setChecked(true); QCOMPARE(minimum->text(), QStringLiteral("LW2")); QCOMPARE(maximum->text(), QStringLiteral("LW3")); QVERIFY(dialog.findChild<QPushButton *>(QStringLiteral("SliderMinimumPicker"))->isVisible()); minimum->setText(QStringLiteral("LW20")); maximum->setText(QStringLiteral("LW24")); dynamic->setChecked(false); QCOMPARE(minimum->text(), QStringLiteral("7")); QCOMPARE(maximum->text(), QStringLiteral("88")); QVERIFY(!dialog.findChild<QPushButton *>(QStringLiteral("SliderMinimumPicker"))->isVisible()); dynamic->setChecked(true); QCOMPARE(minimum->text(), QStringLiteral("LW2")); QCOMPARE(maximum->text(), QStringLiteral("LW3")); dialog.findChild<QComboBox *>(QStringLiteral("SliderDataType"))->setCurrentIndex(1); QCOMPARE(maximum->text(), QStringLiteral("LW3")); dynamic->setChecked(false); dialog.findChild<QComboBox *>(QStringLiteral("SliderDataType"))->setCurrentIndex(4); QCOMPARE(minimum->text(), QStringLiteral("7"));
    QCOMPARE(direction->currentIndex(), 1); dialog.findChild<QRadioButton *>(QStringLiteral("SliderPoint2"))->click(); QCOMPARE(direction->itemText(0), QStringLiteral("向上")); QCOMPARE(direction->currentIndex(), 0); QCOMPARE(dialog.object().geometry.size(), QSize(54, 54)); properties = dialog.object().slider; QCOMPARE(properties.constrainedGeometry(QRect(160, 120, 200, 100)), QRect(160, 120, 54, 100)); properties.pointing = 0; QCOMPARE(properties.constrainedGeometry(QRect(160, 120, 200, 100)), QRect(160, 120, 200, 54));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::sliderFiveTypeMappingAndAddressSpans
 * Description     : Protect signed limits, explicit F32 mapping, finite ranges and full two-word dynamic/operation address spans.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::sliderFiveTypeMappingAndAddressSpans() //20261112-0 add
{
    SliderProperties properties;
    QString error;
    int type;
    properties.maximum = QStringLiteral("65535"); QVERIFY2(properties.validate(&error), qPrintable(error)); properties.maximum = QStringLiteral("65536"); QVERIFY(!properties.validate(&error)); properties.dataType = 2; properties.minimum = QStringLiteral("-32768"); properties.maximum = QStringLiteral("32767"); QVERIFY2(properties.validate(&error), qPrintable(error)); properties.minimum = QStringLiteral("-32769"); QVERIFY(!properties.validate(&error)); properties.dataType = 4; properties.minimum = QStringLiteral("-0.5"); properties.maximum = QStringLiteral("2.5"); QVERIFY2(properties.validate(&error), qPrintable(error)); properties.maximum = QStringLiteral("nan"); QVERIFY(!properties.validate(&error)); properties.maximum = QStringLiteral("-0.5"); QVERIFY(!properties.validate(&error));
    for (type = 0; type < 5; ++type) { properties = SliderProperties(); properties.dataType = type; properties.address = QStringLiteral("LW4096"); QCOMPARE(properties.validateAddresses({}, &error), type == 0 || type == 2); }
    properties = SliderProperties(); properties.dataType = 1; properties.setDynamicRange(true); QVERIFY2(properties.validateAddresses({}, &error), qPrintable(error)); properties.maximumAddress = QStringLiteral("LW4096"); QVERIFY(!properties.validateAddresses({}, &error)); properties.setDynamicRange(false); QVERIFY2(properties.validateAddresses({}, &error), qPrintable(error));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::sliderPermissionsNotificationsAndAddressCopies
 * Description     : Preserve fresh selected-one radios and independent active operation, dynamic, visibility and notification addresses.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::sliderPermissionsNotificationsAndAddressCopies() //20261112-0 add
{
    SliderDialog dialog(sliderObject(), HmiProject());
    HmiObject object;
    HmiProject project;
    HmiProject result;
    Stage3DuplicateOptions options;
    QString error;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); dialog.findChild<QTabWidget *>(QStringLiteral("SliderTabs"))->setCurrentIndex(1); QVERIFY(dialog.findChild<QRadioButton *>(QStringLiteral("SliderVisibilityBitOn"))->isChecked()); QVERIFY(dialog.findChild<QRadioButton *>(QStringLiteral("SliderTouchBitOn"))->isChecked()); QCOMPARE(dialog.object().slider.touch.minimumLevel, 2); dialog.findChild<QGroupBox *>(QStringLiteral("SliderVisibility"))->setChecked(true); dialog.findChild<QGroupBox *>(QStringLiteral("SliderVisibilityBit"))->setChecked(true); dialog.findChild<QRadioButton *>(QStringLiteral("SliderVisibilityBitOff"))->click(); dialog.findChild<QGroupBox *>(QStringLiteral("SliderNotice"))->setChecked(true); dialog.findChild<QCheckBox *>(QStringLiteral("SliderNoticeBit"))->setChecked(true); dialog.findChild<QCheckBox *>(QStringLiteral("SliderNoticeWord"))->setChecked(true); QVERIFY(!dialog.findChild<QCheckBox *>(QStringLiteral("SliderNoticeBit"))->isChecked()); dialog.findChild<QComboBox *>(QStringLiteral("SliderNoticeWordType"))->setCurrentIndex(1); dialog.findChild<QLineEdit *>(QStringLiteral("SliderNoticeWordAddress"))->setText(QStringLiteral("LW4096")); QVERIFY(!dialog.object().slider.validateAddresses({}, &error)); dialog.findChild<QLineEdit *>(QStringLiteral("SliderNoticeWordAddress"))->setText(QStringLiteral("LW30")); object = dialog.object(); object.slider.setDynamicRange(true); project.objects = {object}; options.columns = 2; options.increments = {{Stage3AddressField::Operation, 2, 0}, {Stage3AddressField::DisplayMinimumWord, 4, 0}, {Stage3AddressField::DisplayMaximumWord, 6, 0}, {Stage3AddressField::VisibilityBit, 3, 0}, {Stage3AddressField::NoticeWord, 8, 0}}; QVERIFY2(duplicateStage3Selection(project, {object.id}, options, &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.objects.last().slider.address, QStringLiteral("LW2")); QCOMPARE(result.objects.last().slider.minimumAddress, QStringLiteral("LW6")); QCOMPARE(result.objects.last().slider.maximumAddress, QStringLiteral("LW9")); QCOMPARE(result.objects.last().slider.visibility.bitAddress, QStringLiteral("LB3")); QCOMPARE(result.objects.last().slider.noticeWordAddress, QStringLiteral("LW38")); QVERIFY(!result.objects.last().slider.visibility.bitState);
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("slider-permissions.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::sliderMacroDependenciesAndClipboard
 * Description     : Test actual child-editor cancellation, local save, cross-page rename and collision-safe cross-project macro transfer.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::sliderMacroDependenciesAndClipboard() //20261112-0 add
{
    HmiProject project;
    HmiProject result;
    HmiProject target;
    HmiPage page;
    HmiObject object = sliderObject();
    SliderDialog dialog(object, project);
    Stage3Clipboard clipboard;
    QString error;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); dialog.findChild<QGroupBox *>(QStringLiteral("SliderMacro"))->setChecked(true); QTimer::singleShot(0, &dialog, []() { QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (editor != nullptr) { editor->reject(); } }); dialog.findChild<QPushButton *>(QStringLiteral("SliderWriteMacro"))->click(); QVERIFY(dialog.macros().isEmpty());
    QTimer::singleShot(0, &dialog, []() { QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (editor != nullptr) { editor->findChild<QLineEdit *>(QStringLiteral("MacroName"))->setText(QStringLiteral("SliderMacroA")); editor->findChild<QPlainTextEdit *>(QStringLiteral("MacroSource"))->setPlainText(QStringLiteral("void Macro_Entry() {}")); editor->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Save)->click(); } }); dialog.findChild<QPushButton *>(QStringLiteral("SliderWriteMacro"))->click(); QVERIFY(dialog.macros().contains(QStringLiteral("SliderMacroA"))); QVERIFY(project.macros.isEmpty()); object = dialog.object(); project.macros = dialog.macros(); project.objects = {object}; page.id = QStringLiteral("slider-page2"); page.name = QStringLiteral("滑块副页"); object.id = QStringLiteral("slider-other"); page.objects = {object}; QVERIFY2(project.addPage(page, &error), qPrintable(error)); QVERIFY2(project.renameMacro(QStringLiteral("SliderMacroA"), QStringLiteral("SliderMacroB"), &error), qPrintable(error)); QCOMPARE(project.objects.first().slider.macroName, QStringLiteral("SliderMacroB")); QCOMPARE(project.inactivePages.first().objects.first().slider.macroName, QStringLiteral("SliderMacroB")); QVERIFY2(captureStage3Selection(project, {project.objects.first().id}, &clipboard, &error), qPrintable(error)); QVERIFY2(pasteStage3Selection(target, clipboard, QPoint(10, 10), &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.macros.value(QStringLiteral("SliderMacroB")), QStringLiteral("void Macro_Entry() {}")); target.macros.insert(QStringLiteral("SliderMacroB"), QStringLiteral("different")); result = target; QVERIFY(!pasteStage3Selection(target, clipboard, QPoint(10, 10), &result, nullptr, &error)); QCOMPARE(result.toJson(), target.toJson());
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::sliderStrictMixedPersistenceAndBackendGates
 * Description     : Round-trip nineteen families and slider-only schema12; reject malformed fields without enabling unknown native outputs.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::sliderStrictMixedPersistenceAndBackendGates() //20261112-0 add
{
    HmiProject project;
    HmiProject loaded;
    HmiProject only;
    HmiObject object = sliderObject();
    HmiObject parsed = qrObject();
    QJsonObject json;
    QJsonObject props;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    object.slider.minimum = QStringLiteral("7"); object.slider.maximum = QStringLiteral("88"); object.slider.setDynamicRange(true); object.slider.minimumAddress = QStringLiteral("LW20"); object.slider.maximumAddress = QStringLiteral("LW24"); project.objects = existingFiveObjects(); project.objects.append({messageObject(), scaleObject(), qrObject(), imageObject(), gifObject(), numberObject(), asciiObject(), timeObject(), screenObject(project), multiObject(), mfObject(), dropdownObject(), flowObject(), object}); QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.objects.size(), 19); QCOMPARE(loaded.toJson(), project.toJson()); only.objects = {object}; json = only.toJson(); QCOMPARE(json.value(QStringLiteral("schemaVersion")).toInt(), 12); QVERIFY2(HmiProject::fromJson(json, &only, &error), qPrintable(error)); json.insert(QStringLiteral("schemaVersion"), 11); QVERIFY(!HmiProject::fromJson(json, &only, &error)); QVERIFY(!exportNativeProjectSource(only, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(only, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")});
    json = object.toJson(); props = object.slider.toJson(); props.insert(QStringLiteral("thumbWidth"), 10.5); json.insert(QStringLiteral("slider"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); QCOMPARE(parsed.type, QStringLiteral("qrcode")); props = object.slider.toJson(); props.insert(QStringLiteral("pointing"), 4); json.insert(QStringLiteral("slider"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); props = object.slider.toJson(); props.insert(QStringLiteral("invented"), true); json.insert(QStringLiteral("slider"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY2(loaded.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("slider-mixed.ksp")), &error), qPrintable(error)); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::sliderCenteredRasterAndClipping
 * Description     : Verify measured default rail/thumb pixels, opposite fill directions and clipping across four PC style approximations.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::sliderCenteredRasterAndClipping() //20261112-0 add
{
    SliderProperties properties;
    QImage image(220, 74, QImage::Format_ARGB32);
    QPainter painter;
    int style;
    image.fill(Qt::transparent); painter.begin(&image); paintSlider(&painter, QRectF(10, 10, 200, 54), properties); painter.end(); QCOMPARE(image.pixelColor(30, 35), QColor(QStringLiteral("#00f0f0"))); QCOMPARE(image.pixelColor(180, 35), QColor(QStringLiteral("#00ff00"))); QCOMPARE(image.pixelColor(110, 20), QColor(QStringLiteral("#c8d2dc"))); QCOMPARE(image.pixelColor(5, 5).alpha(), 0);
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(image.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("slider-default-raster.png")))); }
    properties.direction = 0; image.fill(Qt::transparent); painter.begin(&image); paintSlider(&painter, QRectF(10, 10, 200, 54), properties); painter.end(); QCOMPARE(image.pixelColor(30, 35), QColor(QStringLiteral("#00ff00"))); QCOMPARE(image.pixelColor(180, 35), QColor(QStringLiteral("#00f0f0")));
    for (style = 0; style < 4; ++style) { properties.style = style; properties.pointing = style; properties.backgroundTransparent = true; properties.borderTransparent = true; image.fill(Qt::transparent); painter.begin(&image); paintSlider(&painter, QRectF(10, 10, 54, 54), properties); painter.end(); QCOMPARE(image.pixelColor(5, 5).alpha(), 0); QCOMPARE(image.pixelColor(70, 30).alpha(), 0); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::sliderShellMacroPlacementGeometryAndUndo
 * Description     : Exercise real toolbox entry, pending macro cancellation, constrained placement, compact edits and axis-change undo.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::sliderShellMacroPlacementGeometryAndUndo() //20261112-0 add
{
    MainWindow window;
    auto acceptWithMacro = []() {
        SliderDialog *dialog = qobject_cast<SliderDialog *>(QApplication::activeModalWidget());
        if (dialog == nullptr) { return; }
        dialog->findChild<QGroupBox *>(QStringLiteral("SliderMacro"))->setChecked(true); QTimer::singleShot(0, dialog, []() { QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (editor != nullptr) { editor->findChild<QLineEdit *>(QStringLiteral("MacroName"))->setText(QStringLiteral("PendingSliderMacro")); editor->findChild<QPlainTextEdit *>(QStringLiteral("MacroSource"))->setPlainText(QStringLiteral("void Macro_Entry() {}")); editor->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Save)->click(); } }); dialog->findChild<QPushButton *>(QStringLiteral("SliderWriteMacro"))->click(); dialog->findChild<QDialogButtonBox *>(QStringLiteral("SliderButtons"))->button(QDialogButtonBox::Ok)->click();
    };
    window.setProject(HmiProject(), QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); QVERIFY(window.m_commands.contains(QStringLiteral("action_Slider"))); QTimer::singleShot(0, &window, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } }); window.m_commands.value(QStringLiteral("action_Slider"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement());
    QTimer::singleShot(0, &window, acceptWithMacro); window.m_commands.value(QStringLiteral("action_Slider"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QVERIFY(!window.currentEditorProject().macros.contains(QStringLiteral("PendingSliderMacro"))); QTest::keyClick(window.m_canvas->viewport(), Qt::Key_Escape); QVERIFY(window.currentEditorProject().objects.isEmpty()); QVERIFY(!window.currentEditorProject().macros.contains(QStringLiteral("PendingSliderMacro")));
    QTimer::singleShot(0, &window, acceptWithMacro); window.m_commands.value(QStringLiteral("action_Slider"))->trigger(); QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(220, 120))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(220, 120))); QCOMPARE(window.currentEditorProject().objects.first().geometry.size(), QSize(200, 54)); QVERIFY(window.currentEditorProject().macros.contains(QStringLiteral("PendingSliderMacro"))); QCOMPARE(window.findChild<QLabel *>(QStringLiteral("NativeSelectionType"))->text(), QStringLiteral("滑动开关")); QVERIFY(!window.m_textEdit->isEnabled()); window.m_addressEdit->setText(QStringLiteral("LW10")); window.applySelectedProperties(); QCOMPARE(window.currentEditorProject().objects.first().slider.address, QStringLiteral("LW10")); QVERIFY(window.currentEditorProject().objects.first().address.isEmpty());
    QTimer::singleShot(0, &window, []() { SliderDialog *dialog = qobject_cast<SliderDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QRadioButton *>(QStringLiteral("SliderPoint2"))->click(); dialog->findChild<QDialogButtonBox *>(QStringLiteral("SliderButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().geometry.size(), QSize(54, 54)); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().objects.first().geometry.size(), QSize(200, 54)); window.restoreProjectHistory(-1); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty()); QVERIFY(!window.currentEditorProject().macros.contains(QStringLiteral("PendingSliderMacro")));
}

/******************************************************************************
 * Function Name   : wheelObject
 * Description     : Provide an independent wheel fixture using the actually observed free-size canvas rectangle.
 * Input Parameters: None
 * Return Value    : Typed wheel object.
 * Date            : 2026.11.12
 ******************************************************************************/
static HmiObject wheelObject() //20261112-0 add
{
    HmiObject object;
    object.id = QStringLiteral("wheel-object"); object.type = QStringLiteral("wheel"); object.name = QStringLiteral("滚轮控件"); object.geometry = QRect(240, 260, 200, 100); return object;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::wheelNativeBoundsAndIndependentDrafts
 * Description     : Regress measured sensitivity, row/font menus, unsigned scalar bounds and independent rollover/alpha/color drafts.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::wheelNativeBoundsAndIndependentDrafts() //20261112-0 add
{
    WheelDialog dialog(wheelObject(), HmiProject());
    QSpinBox *sensitivity = dialog.findChild<QSpinBox *>(QStringLiteral("WheelSensitivity"));
    QSpinBox *rows = dialog.findChild<QSpinBox *>(QStringLiteral("WheelRows"));
    QComboBox *low = dialog.findChild<QComboBox *>(QStringLiteral("WheelLowFont"));
    WheelProperties properties;
    QString error;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(dialog.windowTitle(), QStringLiteral("滚轮控件")); QCOMPARE(dialog.findChild<QTabWidget *>(QStringLiteral("WheelTabs"))->count(), 2); QCOMPARE(sensitivity->minimum(), 1); QCOMPARE(sensitivity->maximum(), 99); QCOMPARE(sensitivity->value(), 20); QCOMPARE(rows->minimum(), 2); QCOMPARE(rows->maximum(), 5); QCOMPARE(rows->value(), 3); QCOMPARE(low->count(), 7); QCOMPARE(low->itemText(1), QStringLiteral("12")); QCOMPARE(low->currentText(), QStringLiteral("16")); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("WheelHighFont"))->currentText(), QStringLiteral("24")); QCOMPARE(dialog.object().wheel.background, QStringLiteral("#55aaff")); QVERIFY(!dialog.object().wheel.rollover); QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("WheelMinimum"))->maxLength(), 15);
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("wheel-appearance.png")))); }
    sensitivity->setValue(0); QCOMPARE(sensitivity->value(), 1); sensitivity->setValue(100); QCOMPARE(sensitivity->value(), 99); rows->setValue(1); QCOMPARE(rows->value(), 2); rows->setValue(6); QCOMPARE(rows->value(), 5); dialog.findChild<QCheckBox *>(QStringLiteral("WheelRollover"))->setChecked(true); QVERIFY(dialog.object().wheel.rollover);
    properties.maximum = QStringLiteral("65535"); QVERIFY2(properties.validate(&error), qPrintable(error)); properties.maximum = QStringLiteral("65536"); QVERIFY(!properties.validate(&error)); properties.maximum = QStringLiteral("100"); properties.minimum = QStringLiteral("-1"); QVERIFY(!properties.validate(&error)); properties.minimum = QStringLiteral("0"); properties.step = QStringLiteral("0.5"); QVERIFY(!properties.validate(&error)); properties.step = QStringLiteral("0"); QVERIFY(!properties.validate(&error));
    QTimer::singleShot(0, &dialog, []() { QDialog *child = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (child != nullptr) { child->reject(); } }); dialog.findChild<QPushButton *>(QStringLiteral("WheelColor0"))->click(); QCOMPARE(dialog.object().wheel.background, QStringLiteral("#55aaff"));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::wheelPermissionsAcceptanceAndAddressRoles
 * Description     : Preserve fresh empty and accepted-one bit states, chosen zero and independent operation/permission address increments.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::wheelPermissionsAcceptanceAndAddressRoles() //20261112-0 add
{
    WheelDialog dialog(wheelObject(), HmiProject());
    HmiObject object;
    HmiProject project;
    HmiProject result;
    Stage3DuplicateOptions options;
    QString error;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); dialog.findChild<QTabWidget *>(QStringLiteral("WheelTabs"))->setCurrentIndex(1); QVERIFY(!dialog.findChild<QRadioButton *>(QStringLiteral("WheelVisibilityBitOn"))->isChecked()); QVERIFY(!dialog.findChild<QRadioButton *>(QStringLiteral("WheelTouchBitOn"))->isChecked()); QCOMPARE(dialog.findChildren<HmiPermissionEditor *>().size(), 2); QVERIFY(dialog.findChild<QGroupBox *>(QStringLiteral("SliderNotice")) == nullptr); dialog.findChild<QDialogButtonBox *>(QStringLiteral("WheelButtons"))->button(QDialogButtonBox::Ok)->click(); object = dialog.object(); QVERIFY(object.wheel.visibilityBitChosen); QVERIFY(object.wheel.touchBitChosen); WheelDialog reopened(object, HmiProject()); reopened.show(); QVERIFY(QTest::qWaitForWindowExposed(&reopened)); reopened.findChild<QTabWidget *>(QStringLiteral("WheelTabs"))->setCurrentIndex(1); QVERIFY(reopened.findChild<QRadioButton *>(QStringLiteral("WheelVisibilityBitOn"))->isChecked()); reopened.findChild<QGroupBox *>(QStringLiteral("WheelVisibility"))->setChecked(true); reopened.findChild<QGroupBox *>(QStringLiteral("WheelVisibilityBit"))->setChecked(true); reopened.findChild<QRadioButton *>(QStringLiteral("WheelVisibilityBitOff"))->click(); object = reopened.object(); project.objects = {object}; options.columns = 2; options.increments = {{Stage3AddressField::Operation, 2, 0}, {Stage3AddressField::VisibilityBit, 3, 0}}; QVERIFY2(duplicateStage3Selection(project, {object.id}, options, &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.objects.last().wheel.address, QStringLiteral("LW2")); QCOMPARE(result.objects.last().wheel.visibility.bitAddress, QStringLiteral("LB3")); QVERIFY(!result.objects.last().wheel.visibility.bitState); object.wheel.touch.visibilityEnabled = true; object.wheel.touch.wordEnabled = true; object.wheel.touch.dataType = 1; object.wheel.touch.wordAddress = QStringLiteral("LW4096"); QVERIFY(!object.wheel.validateAddresses({}, &error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(reopened.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("wheel-permissions.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::wheelGalleryBindingsCopiesAndResourceProtection
 * Description     : Exercise actual nested gallery accept/cancel/clear and protect exact image bindings across every page and copied project.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::wheelGalleryBindingsCopiesAndResourceProtection() //20261112-0 add
{
    HmiProject project;
    HmiProject result;
    HmiProject target;
    HmiPage page;
    HmiObject object = wheelObject();
    Stage3Clipboard clipboard;
    QTimer interactor;
    bool handled = false;
    QString error;
    project.pictures.insert(QStringLiteral("wheel.png"), imagePng(Qt::red)); WheelDialog dialog(object, project); dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QTimer::singleShot(0, &dialog, []() { QDialog *gallery = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (gallery != nullptr) { gallery->reject(); } }); dialog.findChild<QPushButton *>(QStringLiteral("WheelSystemGallery"))->click(); QVERIFY(dialog.object().wheel.highlight.picture.isEmpty());
    interactor.setInterval(20); connect(&interactor, &QTimer::timeout, &dialog, [&]() { QDialog *gallery = qobject_cast<QDialog *>(QApplication::activeModalWidget()); QListWidget *list = gallery == nullptr ? nullptr : gallery->findChild<QListWidget *>(QStringLiteral("GalleryItems")); if (gallery == nullptr || gallery->objectName() != QStringLiteral("BitGallery") || list == nullptr || list->count() != 1) { return; } list->setCurrentRow(0); list->item(0)->setSelected(true); handled = true; interactor.stop(); gallery->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click(); }); //20261112-0 change: The shared gallery fills its list on a timer; wait for actual rows.
    QTimer::singleShot(5000, &dialog, [&handled]() { QDialog *gallery = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (!handled && gallery != nullptr && gallery->objectName() == QStringLiteral("BitGallery")) { gallery->reject(); } }); interactor.start(); dialog.findChild<QPushButton *>(QStringLiteral("WheelProjectGallery"))->click(); interactor.stop(); QVERIFY(handled); QCOMPARE(dialog.object().wheel.highlight.picture, QStringLiteral("wheel.png")); QCOMPARE(dialog.object().wheel.highlight.picturePng, imagePng(Qt::red)); object = dialog.object(); dialog.findChild<QPushButton *>(QStringLiteral("WheelClearPicture"))->click(); QVERIFY(dialog.object().wheel.highlight.picturePng.isEmpty()); QVERIFY(dialog.pictures().contains(QStringLiteral("wheel.png")));
    project.objects = {object}; page.id = QStringLiteral("wheel-page2"); page.name = QStringLiteral("滚轮副页"); object.id = QStringLiteral("wheel-other"); page.objects = {object}; QVERIFY2(project.addPage(page, &error), qPrintable(error)); QCOMPARE(stage3PictureUsage(project, QStringLiteral("wheel.png")).boundStates, 2); QVERIFY(!deleteStage3Picture(project, QStringLiteral("wheel.png"), &result, &error)); QVERIFY2(renameStage3Picture(project, QStringLiteral("wheel.png"), QStringLiteral("wheel-new.png"), &result, &error), qPrintable(error)); QCOMPARE(result.inactivePages.first().objects.first().wheel.highlight.picture, QStringLiteral("wheel-new.png")); QVERIFY2(replaceStage3Picture(result, QStringLiteral("wheel-new.png"), imagePng(Qt::green), &project, &error), qPrintable(error)); QCOMPARE(project.objects.first().wheel.highlight.picturePng, imagePng(Qt::green)); QVERIFY2(captureStage3Selection(project, {project.objects.first().id}, &clipboard, &error), qPrintable(error)); target.pictures.insert(QStringLiteral("wheel-new.png"), imagePng(Qt::blue)); QVERIFY2(pasteStage3Selection(target, clipboard, QPoint(5, 5), &result, nullptr, &error), qPrintable(error)); QVERIFY(result.objects.first().wheel.highlight.picture != QStringLiteral("wheel-new.png")); QCOMPARE(result.objects.first().wheel.highlight.picturePng, imagePng(Qt::green));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::wheelStrictMixedPersistenceAndBackendGates
 * Description     : Round-trip twenty typed families and wheel-only schema12 while refusing malformed fields/images and unknown backend formats.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::wheelStrictMixedPersistenceAndBackendGates() //20261112-0 add
{
    HmiProject project;
    HmiProject loaded;
    HmiProject only;
    HmiObject object = wheelObject();
    HmiObject parsed = qrObject();
    QJsonObject json;
    QJsonObject props;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    object.wheel.minimum = QStringLiteral("7"); object.wheel.maximum = QStringLiteral("88"); object.wheel.step = QStringLiteral("2"); object.wheel.rowCount = 5; object.wheel.opacity = 128; object.wheel.sensitivity = 99; project.objects = existingFiveObjects(); project.objects.append({messageObject(), scaleObject(), qrObject(), imageObject(), gifObject(), numberObject(), asciiObject(), timeObject(), screenObject(project), multiObject(), mfObject(), dropdownObject(), flowObject(), sliderObject(), object}); QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.objects.size(), 20); QCOMPARE(loaded.toJson(), project.toJson()); only.objects = {object}; json = only.toJson(); QCOMPARE(json.value(QStringLiteral("schemaVersion")).toInt(), 12); QVERIFY2(HmiProject::fromJson(json, &only, &error), qPrintable(error)); json.insert(QStringLiteral("schemaVersion"), 11); QVERIFY(!HmiProject::fromJson(json, &only, &error)); QVERIFY(!exportNativeProjectSource(only, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(only, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")});
    json = object.toJson(); props = object.wheel.toJson(); props.insert(QStringLiteral("rowCount"), 3.5); json.insert(QStringLiteral("wheel"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); QCOMPARE(parsed.type, QStringLiteral("qrcode")); props = object.wheel.toJson(); props.insert(QStringLiteral("rowCount"), 6); json.insert(QStringLiteral("wheel"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); props = object.wheel.toJson(); props.insert(QStringLiteral("highlight"), QJsonObject{{QStringLiteral("picture"), QStringLiteral("bad")}, {QStringLiteral("picturePng"), QStringLiteral("!!")}}); json.insert(QStringLiteral("wheel"), props); QVERIFY(!HmiObject::fromJson(json, &parsed, &error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY2(loaded.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("wheel-mixed.ksp")), &error), qPrintable(error)); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::wheelMidpointRasterFontsAlphaAndClipping
 * Description     : Check actual background alpha independently of text, measured midpoint samples and clipping over all font sizes and row counts.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::wheelMidpointRasterFontsAlphaAndClipping() //20261112-0 add
{
    WheelProperties properties;
    QImage image(220, 140, QImage::Format_ARGB32);
    QPainter painter;
    int x;
    int y;
    int visible = 0;
    int rows;
    int size;
    QCOMPARE(properties.editorSampleValue(), 50); properties.minimum = QStringLiteral("7"); properties.maximum = QStringLiteral("88"); properties.step = QStringLiteral("2"); QCOMPARE(properties.editorSampleValue(), 47); properties.opacity = 128; image.fill(Qt::transparent); painter.begin(&image); paintWheel(&painter, QRectF(10, 10, 200, 100), properties); painter.end(); QCOMPARE(image.pixelColor(20, 20).alpha(), 128); QCOMPARE(image.pixelColor(5, 5).alpha(), 0); properties.opacity = 0; image.fill(Qt::transparent); painter.begin(&image); paintWheel(&painter, QRectF(10, 10, 200, 100), properties); painter.end(); QCOMPARE(image.pixelColor(20, 20).alpha(), 0);
    for (y = 10; y < 110; ++y) { for (x = 10; x < 210; ++x) { if (image.pixelColor(x, y).alpha() > 0) { ++visible; } } } QVERIFY(visible > 20);
    for (rows = 2; rows <= 5; ++rows) { Q_FOREACH (size, WheelProperties::fontSizes()) { properties.rowCount = rows; properties.lowFontSize = size; properties.highFontSize = size; image.fill(Qt::transparent); painter.begin(&image); paintWheel(&painter, QRectF(10, 10, 200, 100), properties); painter.end(); QCOMPARE(image.pixelColor(5, 5).alpha(), 0); QCOMPARE(image.pixelColor(215, 50).alpha(), 0); } }
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(image.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("wheel-clipped-raster.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::wheelShellPlacementPicturesCompactAndUndo
 * Description     : Exercise actual entry, pending cancellation, free geometry, compact carrier edits and whole-document undo.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::wheelShellPlacementPicturesCompactAndUndo() //20261112-0 add
{
    MainWindow window;
    auto acceptWheel = []() { WheelDialog *dialog = qobject_cast<WheelDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QDialogButtonBox *>(QStringLiteral("WheelButtons"))->button(QDialogButtonBox::Ok)->click(); } };
    window.setProject(HmiProject(), QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); QVERIFY(window.m_commands.contains(QStringLiteral("action_wheelView"))); QTimer::singleShot(0, &window, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } }); window.m_commands.value(QStringLiteral("action_wheelView"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement()); QTimer::singleShot(0, &window, acceptWheel); window.m_commands.value(QStringLiteral("action_wheelView"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QTest::keyClick(window.m_canvas->viewport(), Qt::Key_Escape); QVERIFY(window.currentEditorProject().objects.isEmpty());
    QTimer::singleShot(0, &window, acceptWheel); window.m_commands.value(QStringLiteral("action_wheelView"))->trigger(); QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(220, 120))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(220, 120))); QCOMPARE(window.currentEditorProject().objects.first().geometry.size(), QSize(200, 100)); QCOMPARE(window.findChild<QLabel *>(QStringLiteral("NativeSelectionType"))->text(), QStringLiteral("滚轮控件")); QVERIFY(!window.m_textEdit->isEnabled()); window.m_addressEdit->setText(QStringLiteral("LW10")); window.applySelectedProperties(); QCOMPARE(window.currentEditorProject().objects.first().wheel.address, QStringLiteral("LW10")); QVERIFY(window.currentEditorProject().objects.first().address.isEmpty());
    QTimer::singleShot(0, &window, []() { WheelDialog *dialog = qobject_cast<WheelDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QSpinBox *>(QStringLiteral("WheelRows"))->setValue(5); dialog->findChild<QDialogButtonBox *>(QStringLiteral("WheelButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().wheel.rowCount, 5); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().objects.first().wheel.rowCount, 3); window.restoreProjectHistory(-1); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty());
}

/******************************************************************************
 * Function Name   : trackObject
 * Description     : Build a standalone track fixture with a real embedded system graphic binding.
 * Input Parameters: None
 * Return Value    : Typed track object.
 * Date            : 2026.11.12
 ******************************************************************************/
static HmiObject trackObject() //20261112-0 add
{
    HmiObject object;
    object.id = QStringLiteral("track-object"); object.type = QStringLiteral("track"); object.name = QStringLiteral("移动轨迹"); object.geometry = QRect(100, 100, 200, 100); return object;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::trackDefaultsAndRangeDraftLifetime
 * Description     : Exercise observed direction/distance bounds and repeated constant/address switching without draft resets.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::trackDefaultsAndRangeDraftLifetime() //20261112-0 add
{
    MoveTrackDialog dialog(trackObject(), HmiProject());
    QSpinBox *distance = dialog.findChild<QSpinBox *>(QStringLiteral("TrackDistance"));
    QComboBox *direction = dialog.findChild<QComboBox *>(QStringLiteral("TrackDirection"));
    QComboBox *range = dialog.findChild<QComboBox *>(QStringLiteral("TrackRange"));
    QLineEdit *minimum = dialog.findChild<QLineEdit *>(QStringLiteral("TrackMinimum"));
    QLineEdit *minimumAddress = dialog.findChild<QLineEdit *>(QStringLiteral("TrackMinimumAddress"));
    MoveTrackProperties properties;
    QString error;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(dialog.findChild<QTabWidget *>(QStringLiteral("TrackTabs"))->count(), 2); QCOMPARE(distance->value(), 100); QCOMPARE(distance->minimum(), 0); QCOMPARE(distance->maximum(), 900); QCOMPARE(direction->count(), 4); QCOMPARE(direction->itemText(0), QStringLiteral("向上")); QCOMPARE(direction->currentText(), QStringLiteral("向右")); QCOMPARE(dialog.object().moveTrack.appearance.picture, QStringLiteral(":/system-gallery/default/gray.kvg"));
    minimum->setText(QStringLiteral("7")); dialog.findChild<QLineEdit *>(QStringLiteral("TrackMaximum"))->setText(QStringLiteral("88")); QVERIFY(minimum->isEnabled()); QVERIFY(!minimumAddress->isEnabled()); range->setCurrentIndex(1); QVERIFY(!minimum->isEnabled()); QVERIFY(minimumAddress->isEnabled()); minimumAddress->setText(QStringLiteral("LW20")); range->setCurrentIndex(0); QCOMPARE(minimum->text(), QStringLiteral("7")); range->setCurrentIndex(1); QCOMPARE(minimumAddress->text(), QStringLiteral("LW20")); QCOMPARE(dialog.object().moveTrack.maximum, QStringLiteral("88"));
    properties.maximum = QStringLiteral("65535"); QVERIFY2(properties.validate(&error), qPrintable(error)); properties.maximum = QStringLiteral("65536"); QVERIFY(!properties.validate(&error)); properties.maximum = QStringLiteral("100"); properties.minimum = QStringLiteral("-1"); QVERIFY(!properties.validate(&error)); properties.minimum = QStringLiteral("1.5"); QVERIFY(!properties.validate(&error)); properties.minimum = QStringLiteral("0"); properties.distance = 0; QVERIFY(properties.validate(&error)); properties.distance = 901; QVERIFY(!properties.validate(&error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { range->setCurrentIndex(0); QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("track-appearance.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::trackPermissionsNotificationsAndAddressCopies
 * Description     : Check fresh/accepted access state, exclusive notifications and independently incremented dynamic address roles.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::trackPermissionsNotificationsAndAddressCopies() //20261112-0 add
{
    MoveTrackDialog dialog(trackObject(), HmiProject());
    HmiObject object;
    HmiProject project;
    HmiProject result;
    Stage3DuplicateOptions options;
    QString error;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); dialog.findChild<QTabWidget *>(QStringLiteral("TrackTabs"))->setCurrentIndex(1); QVERIFY(!dialog.findChild<QRadioButton *>(QStringLiteral("TrackVisibilityBitOn"))->isChecked()); QVERIFY(!dialog.findChild<QRadioButton *>(QStringLiteral("TrackTouchBitOn"))->isChecked()); dialog.findChild<QGroupBox *>(QStringLiteral("TrackNotice"))->setChecked(true); dialog.findChild<QCheckBox *>(QStringLiteral("TrackNoticeBit"))->click(); dialog.findChild<QCheckBox *>(QStringLiteral("TrackNoticeWord"))->click(); QVERIFY(!dialog.findChild<QCheckBox *>(QStringLiteral("TrackNoticeBit"))->isChecked()); dialog.findChild<QLineEdit *>(QStringLiteral("TrackNoticeWordValue"))->setText(QStringLiteral("12")); dialog.findChild<QGroupBox *>(QStringLiteral("TrackNotice"))->setChecked(false); QCOMPARE(dialog.object().moveTrack.noticeWordValue, QStringLiteral("12")); dialog.findChild<QDialogButtonBox *>(QStringLiteral("TrackButtons"))->button(QDialogButtonBox::Ok)->click(); object = dialog.object(); QVERIFY(object.moveTrack.visibilityBitChosen); QVERIFY(object.moveTrack.touchBitChosen);
    object.moveTrack.dynamicRange = true; object.moveTrack.minimumAddress = QStringLiteral("LW20"); object.moveTrack.maximumAddress = QStringLiteral("LW24"); project.objects = {object}; options.columns = 2; options.increments = {{Stage3AddressField::Operation, 2, 0}, {Stage3AddressField::DisplayMinimumWord, 3, 0}, {Stage3AddressField::DisplayMaximumWord, 5, 0}}; QVERIFY2(duplicateStage3Selection(project, {object.id}, options, &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.objects.last().moveTrack.address, QStringLiteral("LW2")); QCOMPARE(result.objects.last().moveTrack.minimumAddress, QStringLiteral("LW23")); QCOMPARE(result.objects.last().moveTrack.maximumAddress, QStringLiteral("LW29"));
    object.moveTrack.maximumAddress = QStringLiteral("LB0"); QVERIFY(!object.moveTrack.validateAddresses(project.connections, &error)); object.moveTrack.maximumAddress = QStringLiteral("LW0"); object.moveTrack.notice = true; object.moveTrack.noticeWord = true; object.moveTrack.noticeWordType = 1; object.moveTrack.noticeWordAddress = QStringLiteral("LW65535"); QVERIFY(!object.moveTrack.validateAddresses(project.connections, &error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("track-permissions.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::trackResourceTransactionsAndCopyConflicts
 * Description     : Protect track images across inactive pages and preserve content under a cross-project name collision.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::trackResourceTransactionsAndCopyConflicts() //20261112-0 add
{
    HmiObject object = trackObject();
    MoveTrackDialog dialog(trackObject(), HmiProject());
    HmiProject project;
    HmiProject target;
    HmiProject result;
    HmiPage page;
    Stage3Clipboard clipboard;
    QString error;
    project.pictures.insert(QStringLiteral("track.png"), imagePng(Qt::red)); object.moveTrack.appearance.picture = QStringLiteral("track.png"); object.moveTrack.appearance.picturePng = project.pictures.value(QStringLiteral("track.png")); project.objects = {object}; page.id = QStringLiteral("track-page2"); page.name = QStringLiteral("轨迹副页"); object.id = QStringLiteral("track-other"); page.objects = {object}; QVERIFY(project.addPage(page, &error)); QCOMPARE(stage3PictureUsage(project, QStringLiteral("track.png")).boundStates, 2); QVERIFY(!deleteStage3Picture(project, QStringLiteral("track.png"), &result, &error)); QVERIFY(renameStage3Picture(project, QStringLiteral("track.png"), QStringLiteral("track-new.png"), &result, &error)); QCOMPARE(result.inactivePages.first().objects.first().moveTrack.appearance.picture, QStringLiteral("track-new.png")); QVERIFY(captureStage3Selection(result, {result.objects.first().id}, &clipboard, &error)); target.pictures.insert(QStringLiteral("track-new.png"), imagePng(Qt::blue)); QVERIFY2(pasteStage3Selection(target, clipboard, QPoint(5, 5), &project, nullptr, &error), qPrintable(error)); QVERIFY(project.objects.first().moveTrack.appearance.picture != QStringLiteral("track-new.png")); QCOMPARE(project.objects.first().moveTrack.appearance.picturePng, imagePng(Qt::red));
    QTimer::singleShot(0, &dialog, []() { QDialog *child = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (child != nullptr) { child->reject(); } }); dialog.findChild<QPushButton *>(QStringLiteral("TrackProjectGallery"))->click(); QCOMPARE(dialog.object().moveTrack.appearance.picture, QStringLiteral(":/system-gallery/default/gray.kvg")); QVERIFY(dialog.pictures().isEmpty());
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::trackStrictMixedPersistenceAndBackendGates
 * Description     : Round-trip 21 families and standalone schema12; preserve caller outputs when rejecting unknown encodings.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::trackStrictMixedPersistenceAndBackendGates() //20261112-0 add
{
    HmiProject project;
    HmiProject loaded;
    HmiProject only;
    HmiObject object = trackObject();
    HmiObject parsed = qrObject();
    QJsonObject json;
    QJsonObject properties;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    project.objects = existingFiveObjects(); project.objects.append({messageObject(), scaleObject(), qrObject(), imageObject(), gifObject(), numberObject(), asciiObject(), timeObject(), screenObject(project), multiObject(), mfObject(), dropdownObject(), flowObject(), sliderObject(), wheelObject(), object}); QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.objects.size(), 21); QCOMPARE(loaded.toJson(), project.toJson()); only.objects = {object}; json = only.toJson(); QCOMPARE(json.value(QStringLiteral("schemaVersion")).toInt(), 12); QVERIFY(HmiProject::fromJson(json, &only, &error)); json.insert(QStringLiteral("schemaVersion"), 11); QVERIFY(!HmiProject::fromJson(json, &only, &error)); QVERIFY(!exportNativeProjectSource(only, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(only, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")});
    json = object.toJson(); properties = object.moveTrack.toJson(); properties.insert(QStringLiteral("distance"), 100.5); json.insert(QStringLiteral("moveTrack"), properties); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); QCOMPARE(parsed.type, QStringLiteral("qrcode")); properties = object.moveTrack.toJson(); properties.insert(QStringLiteral("extra"), true); json.insert(QStringLiteral("moveTrack"), properties); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); properties = object.moveTrack.toJson(); properties.insert(QStringLiteral("appearance"), QJsonObject{{QStringLiteral("picture"), QStringLiteral("bad")}, {QStringLiteral("picturePng"), QStringLiteral("!!")}}); json.insert(QStringLiteral("moveTrack"), properties); QVERIFY(!HmiObject::fromJson(json, &parsed, &error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY2(loaded.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("track-mixed.ksp")), &error), qPrintable(error)); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::trackStaticRasterAndClipping
 * Description     : Render actual embedded pixels with clipping while avoiding a fabricated direction-dependent live position.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::trackStaticRasterAndClipping() //20261112-0 add
{
    MoveTrackProperties properties;
    QImage image(120, 90, QImage::Format_ARGB32);
    QPainter painter;
    properties.appearance.picture = QStringLiteral("red.png"); properties.appearance.picturePng = imagePng(Qt::red); image.fill(Qt::transparent); painter.begin(&image); paintMoveTrack(&painter, QRectF(10, 10, 100, 60), properties); painter.end(); QCOMPARE(image.pixelColor(50, 30), QColor(Qt::red)); QCOMPARE(image.pixelColor(5, 5).alpha(), 0); QCOMPARE(image.pixelColor(115, 50).alpha(), 0);
    properties.direction = 0; properties.distance = 900; image.fill(Qt::transparent); painter.begin(&image); paintMoveTrack(&painter, QRectF(10, 10, 100, 60), properties); painter.end(); QCOMPARE(image.pixelColor(50, 30), QColor(Qt::red));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::trackShellPlacementEditAndUndo
 * Description     : Exercise toolbox cancellation, pending placement, compact address edits and one-transaction property undo.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::trackShellPlacementEditAndUndo() //20261112-0 add
{
    MainWindow window;
    auto acceptTrack = []() { MoveTrackDialog *dialog = qobject_cast<MoveTrackDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QDialogButtonBox *>(QStringLiteral("TrackButtons"))->button(QDialogButtonBox::Ok)->click(); } };
    window.setProject(HmiProject(), QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); QVERIFY(window.m_commands.contains(QStringLiteral("action_moveTrack"))); QTimer::singleShot(0, &window, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } }); window.m_commands.value(QStringLiteral("action_moveTrack"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement()); QTimer::singleShot(0, &window, acceptTrack); window.m_commands.value(QStringLiteral("action_moveTrack"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QTest::keyClick(window.m_canvas->viewport(), Qt::Key_Escape); QVERIFY(window.currentEditorProject().objects.isEmpty());
    QTimer::singleShot(0, &window, acceptTrack); window.m_commands.value(QStringLiteral("action_moveTrack"))->trigger(); QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(220, 120))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(220, 120))); QCOMPARE(window.currentEditorProject().objects.first().geometry.size(), QSize(200, 100)); QCOMPARE(window.findChild<QLabel *>(QStringLiteral("NativeSelectionType"))->text(), QStringLiteral("移动轨迹")); QVERIFY(!window.m_textEdit->isEnabled()); window.m_addressEdit->setText(QStringLiteral("LW10")); window.applySelectedProperties(); QCOMPARE(window.currentEditorProject().objects.first().moveTrack.address, QStringLiteral("LW10"));
    QTimer::singleShot(0, &window, []() { MoveTrackDialog *dialog = qobject_cast<MoveTrackDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QSpinBox *>(QStringLiteral("TrackDistance"))->setValue(900); dialog->findChild<QDialogButtonBox *>(QStringLiteral("TrackButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().moveTrack.distance, 900); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().objects.first().moveTrack.distance, 100); window.restoreProjectHistory(-1); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty());
}

/******************************************************************************
 * Function Name   : chartObject
 * Description     : Create an isolated typed chart fixture without legacy generic address fields.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
static HmiObject chartObject() //20261112-1 add
{
    HmiObject object;
    object.id = QStringLiteral("chart-object"); object.type = QStringLiteral("chart"); object.name = QStringLiteral("图表"); object.geometry = QRect(100, 100, 170, 130); return object;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::chartDefaultsAndIndependentFormDrafts
 * Description     : Keep all three forms and inactive field drafts across repeated shape and tab changes.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::chartDefaultsAndIndependentFormDrafts() //20261112-1 add
{
    ChartDialog dialog(chartObject(), HmiProject());
    QTabWidget *tabs = dialog.findChild<QTabWidget *>(QStringLiteral("ChartTabs"));
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(dialog.object().chart.bar.dataType, 2); QCOMPARE(dialog.object().chart.meter.dataType, 2); QCOMPARE(dialog.object().chart.donut.dataType, 0); QCOMPARE(dialog.object().chart.meter.shape, 1); QCOMPARE(tabs->count(), 3);
    dialog.findChild<QLineEdit *>(QStringLiteral("ChartBarMinimum"))->setText(QStringLiteral("7")); dialog.findChild<QLineEdit *>(QStringLiteral("ChartSourceMaximum"))->setText(QStringLiteral("88")); dialog.findChild<QComboBox *>(QStringLiteral("ChartRangeType"))->setCurrentIndex(1); QVERIFY(!dialog.findChild<QLineEdit *>(QStringLiteral("ChartSourceMaximum"))->isEnabled()); dialog.findChild<QLineEdit *>(QStringLiteral("ChartMinimumAddress"))->setText(QStringLiteral("LW20"));
    dialog.findChild<QRadioButton *>(QStringLiteral("ChartShape1"))->click(); dialog.findChild<QLineEdit *>(QStringLiteral("ChartMeterAddress"))->setText(QStringLiteral("LW10")); dialog.findChild<QRadioButton *>(QStringLiteral("ChartShape2"))->click(); QVERIFY(!tabs->isTabVisible(1)); dialog.findChild<QRadioButton *>(QStringLiteral("ChartArc"))->click(); dialog.findChild<QLineEdit *>(QStringLiteral("ChartSpanAngle"))->setText(QStringLiteral("270")); dialog.findChild<QRadioButton *>(QStringLiteral("ChartShape0"))->click(); QVERIFY(tabs->isTabVisible(1)); QCOMPARE(dialog.object().chart.bar.minimum, QStringLiteral("7")); QCOMPARE(dialog.object().chart.bar.sourceMaximum, QStringLiteral("88")); QCOMPARE(dialog.object().chart.bar.minimumAddress, QStringLiteral("LW20")); QCOMPARE(dialog.object().chart.meter.address, QStringLiteral("LW10")); QCOMPARE(dialog.object().chart.donut.spanAngle, QStringLiteral("270"));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("chart-bar-basic.png")))); dialog.findChild<QRadioButton *>(QStringLiteral("ChartShape1"))->click(); tabs->setCurrentIndex(1); QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("chart-meter-appearance.png")))); tabs->setCurrentIndex(0); dialog.findChild<QRadioButton *>(QStringLiteral("ChartShape2"))->click(); QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("chart-donut-basic.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::chartTypeRangesAndNativeLinks
 * Description     : Exercise every native range reset, bipolar midpoint, pattern and subscale dependency.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::chartTypeRangesAndNativeLinks() //20261112-1 add
{
    ChartDialog dialog(chartObject(), HmiProject());
    QComboBox *type = dialog.findChild<QComboBox *>(QStringLiteral("ChartBarDataType"));
    QComboBox *pattern = dialog.findChild<QComboBox *>(QStringLiteral("ChartPattern"));
    QCheckBox *bipolar = dialog.findChild<QCheckBox *>(QStringLiteral("ChartTwoPoles"));
    QString error;
    ChartProperties properties;
    int index;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    for (index = 0; index < 7; ++index) { type->setCurrentIndex(index); QCOMPARE(dialog.object().chart.bar.minimum, ChartProperties::typeRange(index).first); QCOMPARE(dialog.object().chart.bar.sourceMaximum, ChartProperties::typeRange(index).second); QVERIFY(!bipolar->isChecked()); QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("ChartBarAddress"))->property("chartWords").toInt(), int(ChartProperties::wordCount(index))); }
    type->setCurrentIndex(0); bipolar->click(); QCOMPARE(dialog.object().chart.bar.midpoint, QStringLiteral("32767")); QCOMPARE(pattern->count(), 13); pattern->setCurrentIndex(2); QVERIFY(dialog.findChild<QPushButton *>(QStringLiteral("ChartPatternColor"))->isEnabled()); pattern->setCurrentIndex(1); QVERIFY(dialog.findChild<QPushButton *>(QStringLiteral("ChartPatternColor"))->isEnabled());
    dialog.findChild<QRadioButton *>(QStringLiteral("ChartShape1"))->click(); dialog.findChild<QCheckBox *>(QStringLiteral("ChartShowSubScale"))->setChecked(false); QVERIFY(!dialog.findChild<QSpinBox *>(QStringLiteral("ChartSubDivisions"))->isEnabled()); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("ChartMainDivisions"))->minimum(), 2); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("ChartMainDivisions"))->maximum(), 20); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("ChartSubDivisions"))->maximum(), 5); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("ChartFontSize"))->minimum(), 5); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("ChartFillWidth"))->count(), 32);
    properties.bar.minimum = QStringLiteral("200"); QVERIFY(!properties.validate(&error)); QCOMPARE(error, QStringLiteral("显示范围最大值小于最小值")); properties.bar.minimum = QStringLiteral("0"); properties.bar.maximum = QStringLiteral("0"); QVERIFY(properties.validate(&error)); properties.bar.twoPoles = true; properties.bar.midpoint = QStringLiteral("101"); QVERIFY(!properties.validate(&error)); properties.bar.midpoint = QStringLiteral("nan"); QVERIFY(!properties.validate(&error)); properties.bar.twoPoles = false; properties.bar.dataType = 0; properties.bar.maximum = QStringLiteral("65536"); QVERIFY(!properties.validate(&error));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::chartAccessSpansAndAddressCopies
 * Description     : Validate full active register widths and independent display permissions without touching inactive form addresses.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::chartAccessSpansAndAddressCopies() //20261112-1 add
{
    HmiObject object = chartObject();
    HmiProject project;
    HmiProject result;
    Stage3DuplicateOptions options;
    ChartDialog dialog(object, project);
    QString error;
    object.chart.bar.dataType = 1; object.chart.bar.address = QStringLiteral("LW4096"); QVERIFY(!object.chart.validateAddresses(project.connections, &error)); object.chart.bar.address = QStringLiteral("LW0"); object.chart.bar.dynamicRange = true; object.chart.bar.minimumAddress = QStringLiteral("LW20"); object.chart.bar.maximumAddress = QStringLiteral("LW24"); object.chart.meter.address = QStringLiteral("LB0"); QVERIFY(object.chart.validateAddresses(project.connections, &error)); project.objects = {object}; options.columns = 2; options.increments = {{Stage3AddressField::Operation, 2, 0}, {Stage3AddressField::DisplayMinimumWord, 3, 0}, {Stage3AddressField::DisplayMaximumWord, 5, 0}}; QVERIFY2(duplicateStage3Selection(project, {object.id}, options, &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.objects.last().chart.bar.address, QStringLiteral("LW2")); QCOMPARE(result.objects.last().chart.bar.minimumAddress, QStringLiteral("LW23")); QCOMPARE(result.objects.last().chart.meter.address, QStringLiteral("LB0"));
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QVERIFY(dialog.findChildren<HmiPermissionEditor *>().size() == 1); QVERIFY(!dialog.findChild<QRadioButton *>(QStringLiteral("ChartVisibleBitOn"))->isChecked()); dialog.findChild<QGroupBox *>(QStringLiteral("ChartVisible"))->setChecked(true); dialog.findChild<QGroupBox *>(QStringLiteral("ChartVisibleBit"))->setChecked(true); dialog.findChild<QRadioButton *>(QStringLiteral("ChartVisibleBitOff"))->click(); QVERIFY(dialog.object().chart.visibilityBitChosen); QVERIFY(!dialog.object().chart.visibility.bitState);
    object.chart.visibility.visibilityEnabled = true; object.chart.visibility.wordEnabled = true; object.chart.visibility.dataType = 1; object.chart.visibility.wordAddress = QStringLiteral("LW4096"); QVERIFY(!object.chart.validateAddresses(project.connections, &error));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::chartStrictPersistenceAndBackendGates
 * Description     : Round-trip mixed and standalone charts while rejecting malformed drafts and unsupported source/device output atomically.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::chartStrictPersistenceAndBackendGates() //20261112-1 add
{
    HmiProject project;
    HmiProject loaded;
    HmiProject only;
    HmiObject object = chartObject();
    ChartProperties parsed;
    QJsonObject json;
    QJsonObject form;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    object.chart.shape = 2; object.chart.bar.minimum = QStringLiteral("7"); object.chart.meter.address = QStringLiteral("LW20"); object.chart.donut.arc = true; object.chart.donut.spanAngle = QStringLiteral("270");
    project.objects = existingFiveObjects(); project.objects.append({messageObject(), scaleObject(), qrObject(), imageObject(), gifObject(), numberObject(), asciiObject(), timeObject(), screenObject(project), multiObject(), mfObject(), dropdownObject(), flowObject(), sliderObject(), wheelObject(), trackObject(), object}); QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.objects.size(), 22); QCOMPARE(loaded.toJson(), project.toJson());
    only.objects = {object}; json = only.toJson(); QCOMPARE(json.value(QStringLiteral("schemaVersion")).toInt(), 12); QVERIFY(HmiProject::fromJson(json, &only, &error)); json.insert(QStringLiteral("schemaVersion"), 11); QVERIFY(!HmiProject::fromJson(json, &only, &error)); QVERIFY(!exportNativeProjectSource(only, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(only, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")});
    json = object.chart.toJson(); json.insert(QStringLiteral("shape"), .5); parsed.shape = 1; QVERIFY(!ChartProperties::fromJson(json, &parsed, &error)); QCOMPARE(parsed.shape, 1); json = object.chart.toJson(); form = json.value(QStringLiteral("donut")).toObject(); form.remove(QStringLiteral("radius")); json.insert(QStringLiteral("donut"), form); QVERIFY(!ChartProperties::fromJson(json, &parsed, &error)); form = object.chart.meter.toJson(); form.insert(QStringLiteral("mainDivisions"), 4.5); json = object.chart.toJson(); json.insert(QStringLiteral("meter"), form); QVERIFY(!ChartProperties::fromJson(json, &parsed, &error)); json = object.chart.toJson(); json.insert(QStringLiteral("unknown"), false); QVERIFY(!ChartProperties::fromJson(json, &parsed, &error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY2(loaded.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("chart-mixed.ksp")), &error), qPrintable(error)); QVERIFY2(only.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("chart-only.ksp")), &error), qPrintable(error)); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::chartStaticRasterAndClipping
 * Description     : Check real clipped bar pixels, meter transparency and invisible donut rendering on the shared painter.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::chartStaticRasterAndClipping() //20261112-1 add
{
    ChartProperties properties;
    QImage image(120, 90, QImage::Format_ARGB32);
    QImage meter;
    QPainter painter;
    image.fill(Qt::transparent); painter.begin(&image); paintChart(&painter, QRectF(10, 10, 100, 60), properties); painter.end(); QCOMPARE(image.pixelColor(50, 20), QColor(Qt::yellow)); QCOMPARE(image.pixelColor(50, 60), QColor(Qt::red)); QCOMPARE(image.pixelColor(5, 5).alpha(), 0);
    properties.shape = 1; image.fill(Qt::transparent); painter.begin(&image); paintChart(&painter, QRectF(10, 10, 100, 60), properties); painter.end(); meter = image; properties.meter.transparent = true; image.fill(Qt::transparent); painter.begin(&image); paintChart(&painter, QRectF(10, 10, 100, 60), properties); painter.end(); QVERIFY(image != meter); QCOMPARE(image.pixelColor(115, 50).alpha(), 0);
    properties.shape = 2; properties.donut.lineType = 0; image.fill(Qt::transparent); painter.begin(&image); paintChart(&painter, QRectF(10, 10, 100, 60), properties); painter.end(); QCOMPARE(image.pixelColor(60, 10).alpha(), 0); properties.donut.lineType = 1; properties.donut.opacity = 0; image.fill(Qt::transparent); painter.begin(&image); paintChart(&painter, QRectF(10, 10, 100, 60), properties); painter.end(); QCOMPARE(image.pixelColor(60, 10).alpha(), 0);
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::chartShellPlacementEditAndUndo
 * Description     : Exercise the verified graphicsitem command, cancellation, geometry, compact address and property history.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::chartShellPlacementEditAndUndo() //20261112-1 add
{
    MainWindow window;
    auto acceptChart = []() { ChartDialog *dialog = qobject_cast<ChartDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QDialogButtonBox *>(QStringLiteral("ChartButtons"))->button(QDialogButtonBox::Ok)->click(); } };
    window.setProject(HmiProject(), QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); QVERIFY(window.m_commands.contains(QStringLiteral("graphicsitem"))); QTimer::singleShot(0, &window, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } }); window.m_commands.value(QStringLiteral("graphicsitem"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement()); QTimer::singleShot(0, &window, acceptChart); window.m_commands.value(QStringLiteral("graphicsitem"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QTest::keyClick(window.m_canvas->viewport(), Qt::Key_Escape); QVERIFY(window.currentEditorProject().objects.isEmpty());
    QTimer::singleShot(0, &window, acceptChart); window.m_commands.value(QStringLiteral("graphicsitem"))->trigger(); QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(190, 150))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(190, 150))); QCOMPARE(window.currentEditorProject().objects.first().geometry.size(), QSize(170, 130)); QCOMPARE(window.findChild<QLabel *>(QStringLiteral("NativeSelectionType"))->text(), QStringLiteral("图表")); QVERIFY(!window.m_textEdit->isEnabled()); window.m_addressEdit->setText(QStringLiteral("LW10")); window.applySelectedProperties(); QCOMPARE(window.currentEditorProject().objects.first().chart.bar.address, QStringLiteral("LW10"));
    QTimer::singleShot(0, &window, []() { ChartDialog *dialog = qobject_cast<ChartDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QRadioButton *>(QStringLiteral("ChartShape2"))->click(); dialog->findChild<QDialogButtonBox *>(QStringLiteral("ChartButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().chart.shape, 2); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().objects.first().chart.shape, 0); window.restoreProjectHistory(-1); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty());
}

/******************************************************************************
 * Function Name   : trendObject
 * Description     : Create a distinct sampling-control fixture with no generic carrier aliases.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
static HmiObject trendObject() //20261112-1 add
{
    HmiObject object;
    object.id = QStringLiteral("trend-object"); object.type = QStringLiteral("trend"); object.name = QStringLiteral("趋势图"); object.geometry = QRect(100, 100, 240, 160); return object;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::trendNativeBoundsAndDataTypeLifetime
 * Description     : Check signed-first type ordering, measured sampling limits and range retention across type changes.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::trendNativeBoundsAndDataTypeLifetime() //20261112-1 add
{
    TrendDialog dialog(trendObject(), HmiProject());
    QComboBox *type = dialog.findChild<QComboBox *>(QStringLiteral("TrendDataType"));
    TrendProperties properties;
    QString error;
    int index;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(dialog.findChild<QTabWidget *>(QStringLiteral("TrendTabs"))->count(), 3); QCOMPARE(type->count(), 7); QCOMPARE(type->itemText(0), QStringLiteral("16位整数")); QCOMPARE(type->itemText(2), QStringLiteral("16位正整数")); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("TrendSamplePoints"))->maximum(), 100); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("TrendSampleTime"))->maximum(), 255); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("TrendChannelCount"))->maximum(), 8);
    for (index = 0; index < 7; ++index) { type->setCurrentIndex(index); QCOMPARE(dialog.object().trend.channels[0].maximum, QStringLiteral("1000")); QCOMPARE(dialog.object().trend.channels[0].minimum, QStringLiteral("0")); }
    properties.sampleTime = 256; QVERIFY(!properties.validate(&error)); properties.sampleTime = 255; properties.samplePoints = 100; QVERIFY(properties.validate(&error)); properties.samplePoints = 0; QVERIFY(!properties.validate(&error)); properties.samplePoints = 60; properties.channels[0].maximum = QStringLiteral("nan"); QVERIFY(!properties.validate(&error));
    type->setCurrentIndex(0);
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("trend-channel.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::trendIndependentChannelsAndRemovalReset
 * Description     : Keep completed edits independent, reset removed channels and reopen the native first-channel editor selection.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::trendIndependentChannelsAndRemovalReset() //20261112-1 add
{
    TrendDialog dialog(trendObject(), HmiProject());
    QSpinBox *count = dialog.findChild<QSpinBox *>(QStringLiteral("TrendChannelCount"));
    QComboBox *current = dialog.findChild<QComboBox *>(QStringLiteral("TrendCurrentChannel"));
    HmiObject object;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); count->setValue(3); QCOMPARE(current->count(), 3); current->setCurrentIndex(2); dialog.findChild<QLineEdit *>(QStringLiteral("TrendChannelMinimum"))->setText(QStringLiteral("-10")); dialog.findChild<QLineEdit *>(QStringLiteral("TrendChannelMaximum"))->setText(QStringLiteral("2000")); dialog.findChild<QComboBox *>(QStringLiteral("TrendLineStyle"))->setCurrentIndex(2); current->setCurrentIndex(0); QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("TrendChannelMinimum"))->text(), QStringLiteral("0")); current->setCurrentIndex(2); QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("TrendChannelMaximum"))->text(), QStringLiteral("2000")); object = dialog.object();
    { TrendDialog reopened(object, HmiProject()); QCOMPARE(reopened.object().trend.currentChannel, 0); QCOMPARE(reopened.object().trend.channels[2].minimum, QStringLiteral("-10")); QCOMPARE(reopened.object().trend.channels[2].lineStyle, 2); }
    count->setValue(1); count->setValue(3); current->setCurrentIndex(2); QCOMPARE(dialog.object().trend.channels[2].minimum, QStringLiteral("0")); QCOMPARE(dialog.object().trend.channels[2].maximum, QStringLiteral("1000")); QCOMPARE(dialog.object().trend.channels[2].lineStyle, 0); QCOMPARE(dialog.object().trend.currentChannel, 2);
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::trendAxisHiddenFieldsAndAccessSpans
 * Description     : Preserve hidden axis values and independently increment trigger, clear and full read spans.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::trendAxisHiddenFieldsAndAccessSpans() //20261112-1 add
{
    TrendDialog dialog(trendObject(), HmiProject());
    HmiObject object = trendObject();
    HmiProject project;
    HmiProject result;
    Stage3DuplicateOptions options;
    QString error;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); dialog.findChild<QTabWidget *>(QStringLiteral("TrendTabs"))->setCurrentIndex(1); dialog.findChild<QLineEdit *>(QStringLiteral("TrendYMinimum"))->setText(QStringLiteral("-20")); dialog.findChild<QLineEdit *>(QStringLiteral("TrendYMaximum"))->setText(QStringLiteral("5000")); dialog.findChild<QCheckBox *>(QStringLiteral("TrendYMarks"))->setChecked(false); QVERIFY(!dialog.findChild<QLineEdit *>(QStringLiteral("TrendYMaximum"))->isVisible()); QVERIFY(!dialog.findChild<QSpinBox *>(QStringLiteral("TrendTotalDigits"))->isVisible()); QVERIFY(dialog.findChild<QComboBox *>(QStringLiteral("TrendXFontSize"))->isVisible()); dialog.findChild<QCheckBox *>(QStringLiteral("TrendYMarks"))->setChecked(true); QCOMPARE(dialog.object().trend.yMaximum, QStringLiteral("5000")); dialog.findChild<QCheckBox *>(QStringLiteral("TrendXTicks"))->setChecked(false); QVERIFY(!dialog.findChild<QPushButton *>(QStringLiteral("TrendXAxisColor"))->isVisible()); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("TrendXMinor"))->minimum(), 1); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("TrendYMinor"))->maximum(), 3); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("TrendTotalDigits"))->maximum(), 10); QCOMPARE(dialog.findChildren<HmiPermissionEditor *>().size(), 1);
    object.trend.dataType = 1; object.trend.channelCount = 8; object.trend.readAddress = QStringLiteral("LW4082"); QVERIFY(!object.trend.validateAddresses(project.connections, &error)); object.trend.readAddress = QStringLiteral("LW4081"); QVERIFY2(object.trend.validateAddresses(project.connections, &error), qPrintable(error)); object.trend.readAddress = QStringLiteral("LW0"); object.trend.triggerAddress = QStringLiteral("LB10"); object.trend.clearAddress = QStringLiteral("LB20"); project.objects = {object}; options.columns = 2; options.increments = {{Stage3AddressField::Operation, 16, 0}, {Stage3AddressField::TriggerBit, 2, 0}, {Stage3AddressField::ClearBit, 3, 0}}; QVERIFY2(duplicateStage3Selection(project, {object.id}, options, &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.objects.last().trend.readAddress, QStringLiteral("LW16")); QCOMPARE(result.objects.last().trend.triggerAddress, QStringLiteral("LB12")); QCOMPARE(result.objects.last().trend.clearAddress, QStringLiteral("LB23"));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("trend-display.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::trendStrictPersistenceAndBackendGates
 * Description     : Round-trip 23 families and strict nested axis/channel drafts while preserving refused backend outputs.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::trendStrictPersistenceAndBackendGates() //20261112-1 add
{
    HmiProject project;
    HmiProject loaded;
    HmiProject only;
    HmiObject object = trendObject();
    TrendProperties parsed;
    QJsonObject json;
    QJsonObject nested;
    QJsonArray channels;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    object.trend.channelCount = 3; object.trend.currentChannel = 2; object.trend.channels[2].minimum = QStringLiteral("-10"); object.trend.channels[2].maximum = QStringLiteral("2000"); object.trend.channels[2].lineStyle = 2; object.trend.channels[2].color = QStringLiteral("#0000ff"); object.trend.yAxis.marks = false; object.trend.yMaximum = QStringLiteral("5000");
    project.objects = existingFiveObjects(); project.objects.append({messageObject(), scaleObject(), qrObject(), imageObject(), gifObject(), numberObject(), asciiObject(), timeObject(), screenObject(project), multiObject(), mfObject(), dropdownObject(), flowObject(), sliderObject(), wheelObject(), trackObject(), chartObject(), object}); QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.objects.size(), 23); QCOMPARE(loaded.toJson(), project.toJson()); only.objects = {object}; json = only.toJson(); QCOMPARE(json.value(QStringLiteral("schemaVersion")).toInt(), 12); QVERIFY(HmiProject::fromJson(json, &only, &error)); json.insert(QStringLiteral("schemaVersion"), 11); QVERIFY(!HmiProject::fromJson(json, &only, &error)); QVERIFY(!exportNativeProjectSource(only, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(only, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")});
    json = object.trend.toJson(); json.insert(QStringLiteral("sampleTime"), 1.5); parsed.sampleTime = 9; QVERIFY(!TrendProperties::fromJson(json, &parsed, &error)); QCOMPARE(parsed.sampleTime, 9); json = object.trend.toJson(); channels = json.value(QStringLiteral("channels")).toArray(); channels.removeLast(); json.insert(QStringLiteral("channels"), channels); QVERIFY(!TrendProperties::fromJson(json, &parsed, &error)); nested = object.trend.xAxis.toJson(); nested.insert(QStringLiteral("major"), true); json = object.trend.toJson(); json.insert(QStringLiteral("xAxis"), nested); QVERIFY(!TrendProperties::fromJson(json, &parsed, &error)); json = object.trend.toJson(); json.insert(QStringLiteral("unknown"), false); QVERIFY(!TrendProperties::fromJson(json, &parsed, &error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY2(loaded.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("trend-mixed.ksp")), &error), qPrintable(error)); QVERIFY2(only.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("trend-only.ksp")), &error), qPrintable(error)); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::trendStaticRasterAndClipping
 * Description     : Verify configured fills, deterministic direction, independent curves and clipping without inventing acquisition.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::trendStaticRasterAndClipping() //20261112-1 add
{
    TrendProperties properties;
    QImage image(240, 160, QImage::Format_ARGB32);
    QImage forward;
    QPainter painter;
    image.fill(Qt::transparent); painter.begin(&image); paintTrend(&painter, QRectF(10, 10, 220, 140), properties); painter.end(); QCOMPARE(image.pixelColor(5, 5).alpha(), 0); QCOMPARE(image.pixelColor(235, 155).alpha(), 0); forward = image;
    properties.rightToLeft = true; image.fill(Qt::transparent); painter.begin(&image); paintTrend(&painter, QRectF(10, 10, 220, 140), properties); painter.end(); QVERIFY(image != forward); properties.channelCount = 2; properties.channels[1].color = QStringLiteral("#0000ff"); properties.bottomColor = QStringLiteral("#00ff00"); image.fill(Qt::transparent); painter.begin(&image); paintTrend(&painter, QRectF(10, 10, 220, 140), properties); painter.end(); QVERIFY(image != forward); QCOMPARE(image.pixelColor(5, 5).alpha(), 0);
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::trendShellPlacementEditAndUndo
 * Description     : Use the real trend action through cancel, pending placement, compact read edits and one-transaction history.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::trendShellPlacementEditAndUndo() //20261112-1 add
{
    MainWindow window;
    auto acceptTrend = []() { TrendDialog *dialog = qobject_cast<TrendDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QDialogButtonBox *>(QStringLiteral("TrendButtons"))->button(QDialogButtonBox::Ok)->click(); } };
    window.setProject(HmiProject(), QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); QVERIFY(window.m_commands.contains(QStringLiteral("action_trendchart"))); QTimer::singleShot(0, &window, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } }); window.m_commands.value(QStringLiteral("action_trendchart"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement()); QTimer::singleShot(0, &window, acceptTrend); window.m_commands.value(QStringLiteral("action_trendchart"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QTest::keyClick(window.m_canvas->viewport(), Qt::Key_Escape); QVERIFY(window.currentEditorProject().objects.isEmpty());
    QTimer::singleShot(0, &window, acceptTrend); window.m_commands.value(QStringLiteral("action_trendchart"))->trigger(); QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(260, 180))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(260, 180))); QCOMPARE(window.currentEditorProject().objects.first().geometry.size(), QSize(240, 160)); QCOMPARE(window.findChild<QLabel *>(QStringLiteral("NativeSelectionType"))->text(), QStringLiteral("趋势图")); QVERIFY(!window.m_textEdit->isEnabled()); window.m_addressEdit->setText(QStringLiteral("LW10")); window.applySelectedProperties(); QCOMPARE(window.currentEditorProject().objects.first().trend.readAddress, QStringLiteral("LW10"));
    QTimer::singleShot(0, &window, []() { TrendDialog *dialog = qobject_cast<TrendDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QSpinBox *>(QStringLiteral("TrendSampleTime"))->setValue(255); dialog->findChild<QDialogButtonBox *>(QStringLiteral("TrendButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().trend.sampleTime, 255); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().objects.first().trend.sampleTime, 1); window.restoreProjectHistory(-1); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty());
}

/******************************************************************************
 * Function Name   : historyProject
 * Description     : Create one actual collector with three named cache items and one independently bound history object.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
static HmiProject historyProject() //20261112-1 add
{
    HmiProject project;
    DataCollectionProperties properties;
    HmiConfigurationNode collection;
    HmiObject object;
    int index;
    properties.sampleLength = 3; properties.items.clear(); for (index = 0; index < 3; ++index) { properties.items.append(DataCollectionProperties::createItem(index)); properties.items[index].name = QStringList{QStringLiteral("First"), QStringLiteral("Second"), QStringLiteral("Third")}.at(index); }
    collection.id = QStringLiteral("history-collection"); collection.name = QStringLiteral("C24"); collection.kind = QStringLiteral("dataCollection"); collection.implemented = true; collection.active = true; collection.properties = properties.toJson(); project.configurationNodes = {collection};
    object.id = QStringLiteral("history-object"); object.name = QStringLiteral("历史趋势图"); object.type = QStringLiteral("historytrend"); object.geometry = QRect(100, 100, 240, 160); object.historyTrend.collectionId = collection.id; object.historyTrend.curveCount = 3; for (index = 0; index < 8; ++index) { object.historyTrend.curves[index].sourceItemId = properties.items.first().id; } project.objects = {object}; return project;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::collectionSharedTypeMemoryAndFullAddressSpan
 * Description     : Check the measured common type, memory formula, strict byte cap and full active PLC carrier spans.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::collectionSharedTypeMemoryAndFullAddressSpan() //20261112-1 add
{
    DataCollectionProperties properties;
    QString error;
    int index;
    QCOMPARE(DataCollectionProperties::dataTypes().size(), 5); QCOMPARE(DataCollectionProperties::displayTypes().size(), 3);
    properties.sampleLength = 3; properties.sampleCount = 1000; properties.items.clear(); for (index = 0; index < 3; ++index) { properties.items.append(DataCollectionProperties::createItem(index)); } QCOMPARE(properties.memoryBytes(), 26000U); QVERIFY(properties.validate(&error)); properties.dataType = 1; QCOMPARE(properties.memoryBytes(), 32000U); QCOMPARE(properties.wordCount(), 2U);
    properties.sampleLength = 32; properties.items.clear(); for (index = 0; index < 32; ++index) { properties.items.append(DataCollectionProperties::createItem(index)); } properties.dataType = 0; properties.sampleCount = 768; QCOMPARE(properties.memoryBytes(), 63U * 1024U); QVERIFY(properties.validate(&error)); properties.sampleCount = 769; QVERIFY(!properties.validate(&error)); properties.sampleCount = 1; properties.dataType = 1; properties.readAddress = QStringLiteral("LW4033"); QVERIFY(properties.validateAddresses({}, &error)); properties.readAddress = QStringLiteral("LW4034"); QVERIFY(!properties.validateAddresses({}, &error)); properties.readAddress = QStringLiteral("LW0"); properties.triggerAddress = QStringLiteral("invalid"); properties.noticeAddress = QStringLiteral("invalid"); QVERIFY(properties.validateAddresses({}, &error)); properties.samplingMode = 1; QVERIFY(!properties.validateAddresses({}, &error)); properties.samplingMode = 0; properties.notice = true; QVERIFY(!properties.validateAddresses({}, &error));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::collectionDialogItemsAndInactiveDrafts
 * Description     : Keep item names and scaling independent while changing the collection-wide type and fixed LH row.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::collectionDialogItemsAndInactiveDrafts() //20261112-1 add
{
    HmiProject project;
    HmiConfigurationNode node;
    DataCollectionDialog dialog(node, project);
    DataCollectionProperties parsed;
    QTabWidget *tabs = dialog.findChild<QTabWidget *>(QStringLiteral("CollectionTabs"));
    QTableWidget *items = dialog.findChild<QTableWidget *>(QStringLiteral("CollectionItems"));
    QString error;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(tabs->count(), 2); dialog.findChild<QLineEdit *>(QStringLiteral("CollectionName"))->setText(QStringLiteral("Collector")); dialog.findChild<QSpinBox *>(QStringLiteral("CollectionLength"))->setValue(3); QCOMPARE(items->rowCount(), 3); tabs->setCurrentIndex(1); QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("CollectionCacheAddress"))->isReadOnly()); dialog.findChild<QLineEdit *>(QStringLiteral("CollectionItemName"))->setText(QStringLiteral("Alpha")); dialog.findChild<QComboBox *>(QStringLiteral("CollectionDataType"))->setCurrentIndex(1); items->setCurrentCell(1, 0); QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("CollectionCacheAddress"))->text(), QStringLiteral("LH1")); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("CollectionDataType"))->currentIndex(), 1); dialog.findChild<QLineEdit *>(QStringLiteral("CollectionItemName"))->setText(QStringLiteral("Beta")); dialog.findChild<QCheckBox *>(QStringLiteral("CollectionItemScaled"))->setChecked(true); QVERIFY(!DataCollectionProperties::fromJson(dialog.node().properties, &parsed, &error)); dialog.findChild<QLineEdit *>(QStringLiteral("CollectionItemGain"))->setText(QStringLiteral("2.5")); dialog.findChild<QLineEdit *>(QStringLiteral("CollectionItemOffset"))->setText(QStringLiteral("-4")); items->setCurrentCell(0, 0); QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("CollectionItemName"))->text(), QStringLiteral("Alpha")); QVERIFY(!dialog.findChild<QCheckBox *>(QStringLiteral("CollectionItemScaled"))->isChecked()); QVERIFY(DataCollectionProperties::fromJson(dialog.node().properties, &parsed, &error)); QCOMPARE(parsed.items[1].gain, QStringLiteral("2.5")); QVERIFY(node.properties.isEmpty());
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("collection-items.png")))); tabs->setCurrentIndex(0); QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("collection-general.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::historyIndependentCurvesAndStableNames
 * Description     : Bind every default curve to the first item and retain independent source, range and style across renames.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::historyIndependentCurvesAndStableNames() //20261112-1 add
{
    HmiProject project = historyProject();
    HmiObject object = project.objects.first();
    HistoryTrendDialog dialog(object, project);
    QComboBox *current = dialog.findChild<QComboBox *>(QStringLiteral("HistoryCurrentCurve"));
    QComboBox *source = dialog.findChild<QComboBox *>(QStringLiteral("HistorySource"));
    HistoryTrendDialog *reopened;
    DataCollectionProperties collection;
    QString error;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(dialog.findChild<QTabWidget *>(QStringLiteral("HistoryTabs"))->count(), 3); QCOMPARE(source->currentText(), QStringLiteral("First")); current->setCurrentIndex(2); QCOMPARE(source->currentText(), QStringLiteral("First")); source->setCurrentIndex(2); dialog.findChild<QLineEdit *>(QStringLiteral("HistoryCurveMinimum"))->setText(QStringLiteral("7")); dialog.findChild<QLineEdit *>(QStringLiteral("HistoryCurveMaximum"))->setText(QStringLiteral("900")); dialog.findChild<QComboBox *>(QStringLiteral("HistoryLineStyle"))->setCurrentIndex(2); current->setCurrentIndex(0); QCOMPARE(source->currentText(), QStringLiteral("First")); QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("HistoryCurveMinimum"))->text(), QStringLiteral("0")); object = dialog.object(); QCOMPARE(object.historyTrend.curves[2].maximum, QStringLiteral("900"));
    QVERIFY(DataCollectionProperties::fromJson(project.configurationNodes.first().properties, &collection, &error)); collection.items[2].name = QStringLiteral("Renamed"); project.configurationNodes[0].properties = collection.toJson(); project.objects[0] = object; QVERIFY(project.validateModel(&error)); reopened = new HistoryTrendDialog(object, project); QCOMPARE(reopened->object().historyTrend.currentCurve, 0); reopened->findChild<QComboBox *>(QStringLiteral("HistoryCurrentCurve"))->setCurrentIndex(2); QCOMPARE(reopened->findChild<QComboBox *>(QStringLiteral("HistorySource"))->currentText(), QStringLiteral("Renamed")); QCOMPARE(reopened->object().historyTrend.curves[2].minimum, QStringLiteral("7")); delete reopened;
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("history-general.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::historyCheckableAxesAndTimeBounds
 * Description     : Measure the distinct history mark groups and retained Y limits and enforce the observed 48:59:59 upper fields.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::historyCheckableAxesAndTimeBounds() //20261112-1 add
{
    HmiProject project = historyProject();
    HistoryTrendDialog dialog(project.objects.first(), project);
    HistoryTrendProperties properties = project.objects.first().historyTrend;
    QString error;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); dialog.findChild<QTabWidget *>(QStringLiteral("HistoryTabs"))->setCurrentIndex(1); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("HistoryHours"))->maximum(), 48); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("HistoryMinutes"))->maximum(), 59); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("HistorySeconds"))->maximum(), 59); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("HistoryTimeFormat"))->count(), 2); dialog.findChild<QLineEdit *>(QStringLiteral("HistoryYMaximum"))->setText(QStringLiteral("5000")); dialog.findChild<QGroupBox *>(QStringLiteral("HistoryYMarks"))->setChecked(false); QVERIFY(!dialog.findChild<QSpinBox *>(QStringLiteral("HistoryTotalDigits"))->isVisible()); QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("HistoryYMaximum"))->isVisible()); QCOMPARE(dialog.object().historyTrend.yMaximum, QStringLiteral("5000")); dialog.findChild<QGroupBox *>(QStringLiteral("HistoryXMarks"))->setChecked(false); QVERIFY(!dialog.findChild<QComboBox *>(QStringLiteral("HistoryTimeFormat"))->isVisible()); QVERIFY(dialog.findChild<QSpinBox *>(QStringLiteral("HistoryHours"))->isVisible()); dialog.findChild<QGroupBox *>(QStringLiteral("HistoryYMarks"))->setChecked(true); dialog.findChild<QGroupBox *>(QStringLiteral("HistoryXMarks"))->setChecked(true); QCOMPARE(dialog.findChildren<HmiPermissionEditor *>().size(), 1); properties.hours = 48; properties.minutes = 59; properties.seconds = 59; QVERIFY(properties.validate(&error)); properties.hours = 49; QVERIFY(!properties.validate(&error)); properties.hours = 0; properties.minutes = 0; properties.seconds = 0; QVERIFY(!properties.validate(&error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("history-axes.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::historyReferenceLifetimeAndSingleCollectorLimit
 * Description     : Protect cross-page source identities, item-count limits and the measured one-collector target.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::historyReferenceLifetimeAndSingleCollectorLimit() //20261112-1 add
{
    HmiProject project = historyProject();
    HmiProject candidate;
    HmiPage page;
    HmiConfigurationNode second = project.configurationNodes.first();
    DataCollectionProperties collection;
    QString error;
    QVERIFY(DataCollectionProperties::fromJson(second.properties, &collection, &error)); project.objects[0].historyTrend.curves[2].sourceItemId = collection.items[2].id; page.id = QStringLiteral("history-page-2"); page.name = QStringLiteral("History2"); page.objects = project.objects; page.objects[0].id = QStringLiteral("history-object-2"); QVERIFY(project.addPage(page, &error)); project.objects.clear(); QVERIFY(project.validateModel(&error)); candidate = project; candidate.configurationNodes.clear(); QVERIFY(!candidate.validateModel(&error)); candidate = project; collection.items.removeLast(); collection.sampleLength = 2; candidate.configurationNodes[0].properties = collection.toJson(); QVERIFY(!candidate.validateModel(&error)); candidate = project; second.id = QStringLiteral("other-collector"); candidate.configurationNodes.append(second); QVERIFY(!candidate.validateModel(&error));
    { DataCollectionDialog dialog(project.configurationNodes.first(), project); QTimer::singleShot(0, []() { QMessageBox *warning = qobject_cast<QMessageBox *>(QApplication::activeModalWidget()); if (warning != nullptr) { warning->accept(); } }); dialog.findChild<QSpinBox *>(QStringLiteral("CollectionLength"))->setValue(2); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("CollectionLength"))->value(), 3); QCOMPARE(dialog.node().properties, project.configurationNodes.first().properties); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::historyCrossProjectPasteAndVisibilityRoles
 * Description     : Copy required definitions into an empty project, reuse identical definitions and refuse target conflicts atomically.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::historyCrossProjectPasteAndVisibilityRoles() //20261112-1 add
{
    HmiProject source = historyProject();
    HmiProject target;
    HmiProject result;
    HmiProject sentinel;
    Stage3Clipboard clipboard;
    Stage3DuplicateOptions options;
    QString error;
    source.objects[0].historyTrend.visibility.visibilityEnabled = true; source.objects[0].historyTrend.visibility.bitEnabled = true; source.objects[0].historyTrend.visibilityBitChosen = true; source.objects[0].historyTrend.visibility.bitAddress = QStringLiteral("LB10"); QVERIFY(captureStage3Selection(source, {source.objects.first().id}, &clipboard, &error)); target.objects.clear(); QVERIFY2(pasteStage3Selection(target, clipboard, QPoint(0, 0), &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.configurationNodes.first().toJson(), source.configurationNodes.first().toJson()); QCOMPARE(result.objects.first().historyTrend.collectionId, source.configurationNodes.first().id); QVERIFY(result.validateModel(&error)); target = source; target.configurationNodes[0].name = QStringLiteral("Conflict"); sentinel = source; QVERIFY(!pasteStage3Selection(target, clipboard, QPoint(0, 0), &sentinel, nullptr, &error)); QCOMPARE(sentinel.toJson(), source.toJson()); options.columns = 2; options.increments = {{Stage3AddressField::VisibilityBit, 2, 0}}; QVERIFY(duplicateStage3Selection(source, {source.objects.first().id}, options, &result, nullptr, &error)); QCOMPARE(result.configurationNodes.size(), 1); QCOMPARE(result.objects.last().historyTrend.visibility.bitAddress, QStringLiteral("LB12"));
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::historyStrictPersistenceAndBackendGates
 * Description     : Round-trip 24 families plus a standalone collector and reject malformed nested source or device records.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::historyStrictPersistenceAndBackendGates() //20261112-1 add
{
    HmiProject project = historyProject();
    HmiProject loaded;
    HmiProject only = project;
    DataCollectionProperties collection;
    HistoryTrendProperties properties = project.objects.first().historyTrend;
    QJsonObject json;
    QJsonArray curves;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    project.objects = existingFiveObjects(); project.objects.append({messageObject(), scaleObject(), qrObject(), imageObject(), gifObject(), numberObject(), asciiObject(), timeObject(), screenObject(project), multiObject(), mfObject(), dropdownObject(), flowObject(), sliderObject(), wheelObject(), trackObject(), chartObject(), trendObject(), only.objects.first()}); QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.objects.size(), 24); QCOMPARE(loaded.toJson(), project.toJson()); json = only.toJson(); json.insert(QStringLiteral("schemaVersion"), 11); QVERIFY(!HmiProject::fromJson(json, &loaded, &error)); QVERIFY(!exportNativeProjectSource(only, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(only, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")});
    json = properties.toJson(); json.insert(QStringLiteral("hours"), 1.5); properties.hours = 9; QVERIFY(!HistoryTrendProperties::fromJson(json, &properties, &error)); QCOMPARE(properties.hours, 9); json = only.objects.first().historyTrend.toJson(); curves = json.value(QStringLiteral("curves")).toArray(); curves.removeLast(); json.insert(QStringLiteral("curves"), curves); QVERIFY(!HistoryTrendProperties::fromJson(json, &properties, &error)); json = only.configurationNodes.first().properties; json.insert(QStringLiteral("sampleLength"), true); QVERIFY(!DataCollectionProperties::fromJson(json, &collection, &error)); json = only.configurationNodes.first().properties; json.insert(QStringLiteral("unknown"), false); QVERIFY(!DataCollectionProperties::fromJson(json, &collection, &error)); loaded = only; loaded.objects.clear(); QCOMPARE(loaded.toJson().value(QStringLiteral("schemaVersion")).toInt(), 12); QVERIFY(HmiProject::fromJson(loaded.toJson(), &loaded, &error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(project.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("history-mixed.ksp")), &error)); QVERIFY(only.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("history-only.ksp")), &error)); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::historyStaticRasterAndTimeFormatClipping
 * Description     : Keep the fixed editor illustration clipped and make explicit time-format and per-curve colors visible.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::historyStaticRasterAndTimeFormatClipping() //20261112-1 add
{
    HistoryTrendProperties properties = historyProject().objects.first().historyTrend;
    QImage image(240, 160, QImage::Format_ARGB32);
    QImage first;
    QPainter painter;
    image.fill(Qt::transparent); painter.begin(&image); paintHistoryTrend(&painter, QRectF(10, 10, 220, 140), properties); painter.end(); QCOMPARE(image.pixelColor(5, 5).alpha(), 0); QCOMPARE(image.pixelColor(235, 155).alpha(), 0); first = image; properties.timeFormat = 1; properties.curves[1].color = QStringLiteral("#0000ff"); image.fill(Qt::transparent); painter.begin(&image); paintHistoryTrend(&painter, QRectF(10, 10, 220, 140), properties); painter.end(); QVERIFY(image != first); painter.begin(&image); paintHistoryTrend(&painter, QRectF(1, 1, 1, 1), properties); painter.end(); QCOMPARE(image.pixelColor(5, 5).alpha(), 0);
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::historyShellGateCollectionPlacementAndUndo
 * Description     : Exercise actual no-collection warning, collector action, accepted placement, tree identity and one-step undo.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void Stage4ControlTests::historyShellGateCollectionPlacementAndUndo() //20261112-1 add
{
    MainWindow window;
    auto closeWarning = []() { QMessageBox *warning = qobject_cast<QMessageBox *>(QApplication::activeModalWidget()); if (warning != nullptr) { warning->accept(); } };
    auto acceptHistory = []() { HistoryTrendDialog *dialog = qobject_cast<HistoryTrendDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QDialogButtonBox *>(QStringLiteral("HistoryButtons"))->button(QDialogButtonBox::Ok)->click(); } };
    window.setProject(HmiProject(), QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); QVERIFY(window.m_commands.contains(QStringLiteral("action_DatagroupDisplay"))); QVERIFY(window.m_commands.contains(QStringLiteral("action_datasamp"))); QTimer::singleShot(0, &window, closeWarning); window.m_commands.value(QStringLiteral("action_DatagroupDisplay"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement());
    QTimer::singleShot(0, &window, []() { DataCollectionDialog *dialog = qobject_cast<DataCollectionDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QLineEdit *>(QStringLiteral("CollectionName"))->setText(QStringLiteral("ShellCollector")); dialog->findChild<QDialogButtonBox *>(QStringLiteral("CollectionButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.m_commands.value(QStringLiteral("action_datasamp"))->trigger(); QCOMPARE(window.currentEditorProject().configurationNodes.size(), 1); QVERIFY(!window.m_systemSettingsTree->findItems(QStringLiteral("ShellCollector"), Qt::MatchExactly | Qt::MatchRecursive).isEmpty());
    QTimer::singleShot(0, &window, acceptHistory); window.m_commands.value(QStringLiteral("action_DatagroupDisplay"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QTest::keyClick(window.m_canvas->viewport(), Qt::Key_Escape); QVERIFY(window.currentEditorProject().objects.isEmpty()); QTimer::singleShot(0, &window, acceptHistory); window.m_commands.value(QStringLiteral("action_DatagroupDisplay"))->trigger(); QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(260, 180))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(260, 180))); QCOMPARE(window.currentEditorProject().objects.size(), 1); QCOMPARE(window.findChild<QLabel *>(QStringLiteral("NativeSelectionType"))->text(), QStringLiteral("历史趋势图")); QVERIFY(!window.m_addressEdit->isEnabled()); QVERIFY(!window.m_textEdit->isEnabled()); QTimer::singleShot(0, &window, closeWarning); window.deleteDataCollection(window.currentEditorProject().configurationNodes.first().id); QCOMPARE(window.currentEditorProject().configurationNodes.size(), 1); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty()); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().configurationNodes.isEmpty());
}

/******************************************************************************
 * Function Name   : xyObject
 * Description     : Create an independent paired-coordinate fixture without generic operation-address aliases.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
static HmiObject xyObject() //20261113-1 add
{
    HmiObject object;
    object.id = QStringLiteral("xy-object"); object.type = QStringLiteral("xycurve"); object.name = QStringLiteral("X-Y曲线"); object.geometry = QRect(100, 100, 240, 160); return object;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::xyNativeBoundsAndDataTypeLifetime
 * Description     : Check signed-first type ordering, measured sampling limits and range retention across type changes.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::xyNativeBoundsAndDataTypeLifetime() //20261113-1 add
{
    XyCurveDialog dialog(xyObject(), HmiProject());
    QComboBox *type = dialog.findChild<QComboBox *>(QStringLiteral("XyDataType"));
    XyCurveProperties properties;
    QString error;
    int index;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(dialog.findChild<QTabWidget *>(QStringLiteral("XyTabs"))->count(), 3); QCOMPARE(type->count(), 7); QCOMPARE(type->itemText(0), QStringLiteral("16位整数")); QCOMPARE(type->itemText(2), QStringLiteral("16位正整数")); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("XySamplePoints"))->maximum(), 100); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("XySampleTime"))->maximum(), 255); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("XyChannelCount"))->maximum(), 8);
    for (index = 0; index < 7; ++index) { type->setCurrentIndex(index); QCOMPARE(dialog.object().xyCurve.channels[0].maximum, QStringLiteral("1000")); QCOMPARE(dialog.object().xyCurve.channels[0].minimum, QStringLiteral("0")); }
    properties.sampleTime = 256; QVERIFY(!properties.validate(&error)); properties.sampleTime = 255; properties.samplePoints = 100; QVERIFY(properties.validate(&error)); properties.samplePoints = 0; QVERIFY(!properties.validate(&error)); properties.samplePoints = 60; properties.channels[0].maximum = QStringLiteral("nan"); QVERIFY(!properties.validate(&error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("xy-channel.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::xyIndependentChannelsAndRemovalReset
 * Description     : Keep completed edits independent, reset removed channels and reopen the native first-channel editor selection.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::xyIndependentChannelsAndRemovalReset() //20261113-1 add
{
    XyCurveDialog dialog(xyObject(), HmiProject());
    QSpinBox *count = dialog.findChild<QSpinBox *>(QStringLiteral("XyChannelCount"));
    QComboBox *current = dialog.findChild<QComboBox *>(QStringLiteral("XyCurrentChannel"));
    HmiObject object;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); count->setValue(3); QCOMPARE(current->count(), 3); current->setCurrentIndex(2); dialog.findChild<QLineEdit *>(QStringLiteral("XyChannelMinimum"))->setText(QStringLiteral("-10")); dialog.findChild<QLineEdit *>(QStringLiteral("XyChannelMaximum"))->setText(QStringLiteral("2000")); dialog.findChild<QComboBox *>(QStringLiteral("XyLineStyle"))->setCurrentIndex(2); current->setCurrentIndex(0); QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("XyChannelMinimum"))->text(), QStringLiteral("0")); current->setCurrentIndex(2); QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("XyChannelMaximum"))->text(), QStringLiteral("2000")); object = dialog.object();
    { XyCurveDialog reopened(object, HmiProject()); QCOMPARE(reopened.object().xyCurve.currentChannel, 0); QCOMPARE(reopened.object().xyCurve.channels[2].minimum, QStringLiteral("-10")); QCOMPARE(reopened.object().xyCurve.channels[2].lineStyle, 2); }
    count->setValue(1); count->setValue(3); current->setCurrentIndex(2); QCOMPARE(dialog.object().xyCurve.channels[2].minimum, QStringLiteral("0")); QCOMPARE(dialog.object().xyCurve.channels[2].maximum, QStringLiteral("1000")); QCOMPARE(dialog.object().xyCurve.channels[2].lineStyle, 0); QCOMPARE(dialog.object().xyCurve.currentChannel, 2);
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::xyAxisHiddenFieldsAndAccessSpans
 * Description     : Preserve hidden axis values and independently increment trigger, clear and full read spans.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::xyAxisHiddenFieldsAndAccessSpans() //20261113-1 add
{
    XyCurveDialog dialog(xyObject(), HmiProject());
    HmiObject object = xyObject();
    HmiProject project;
    HmiProject result;
    Stage3DuplicateOptions options;
    QString error;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); dialog.findChild<QTabWidget *>(QStringLiteral("XyTabs"))->setCurrentIndex(1); dialog.findChild<QLineEdit *>(QStringLiteral("XyYMinimum"))->setText(QStringLiteral("-20")); dialog.findChild<QLineEdit *>(QStringLiteral("XyYMaximum"))->setText(QStringLiteral("5000")); dialog.findChild<QCheckBox *>(QStringLiteral("XyYMarks"))->setChecked(false); QVERIFY(!dialog.findChild<QLineEdit *>(QStringLiteral("XyYMaximum"))->isVisible()); QVERIFY(!dialog.findChild<QSpinBox *>(QStringLiteral("XyYTotalDigits"))->isVisible()); QVERIFY(dialog.findChild<QComboBox *>(QStringLiteral("XyXFontSize"))->isVisible()); dialog.findChild<QCheckBox *>(QStringLiteral("XyYMarks"))->setChecked(true); QCOMPARE(dialog.object().xyCurve.yMaximum, QStringLiteral("5000")); dialog.findChild<QCheckBox *>(QStringLiteral("XyXTicks"))->setChecked(false); QVERIFY(!dialog.findChild<QPushButton *>(QStringLiteral("XyXAxisColor"))->isVisible()); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("XyXMinor"))->minimum(), 1); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("XyYMinor"))->maximum(), 3); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("XyYTotalDigits"))->maximum(), 10); QCOMPARE(dialog.findChildren<HmiPermissionEditor *>().size(), 1);
    dialog.findChild<QLineEdit *>(QStringLiteral("XyXMinimum"))->setText(QStringLiteral("-100")); dialog.findChild<QLineEdit *>(QStringLiteral("XyXMaximum"))->setText(QStringLiteral("4000")); dialog.findChild<QSpinBox *>(QStringLiteral("XyXFractionalDigits"))->setValue(2); dialog.findChild<QCheckBox *>(QStringLiteral("XyXMarks"))->setChecked(false); QVERIFY(!dialog.findChild<QLineEdit *>(QStringLiteral("XyXMaximum"))->isVisible()); QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("XyYMaximum"))->isVisible()); dialog.findChild<QCheckBox *>(QStringLiteral("XyXMarks"))->setChecked(true); QCOMPARE(dialog.object().xyCurve.xMaximum, QStringLiteral("4000")); dialog.findChild<QCheckBox *>(QStringLiteral("XyXGrid"))->setChecked(false); QVERIFY(!dialog.findChild<QPushButton *>(QStringLiteral("XyXGridColor"))->isVisible()); dialog.findChild<QCheckBox *>(QStringLiteral("XyXGrid"))->setChecked(true);
    object.xyCurve.dataType = 1; object.xyCurve.channelCount = 8; object.xyCurve.readAddress = QStringLiteral("LW4066"); QVERIFY(!object.xyCurve.validateAddresses(project.connections, &error)); object.xyCurve.readAddress = QStringLiteral("LW4065"); QVERIFY(object.xyCurve.validateAddresses(project.connections, &error)); object.xyCurve.readAddress = QStringLiteral("LW0"); object.xyCurve.triggerAddress = QStringLiteral("LB10"); object.xyCurve.clearAddress = QStringLiteral("LB20"); project.objects = {object}; options.columns = 2; options.increments = {{Stage3AddressField::Operation, 32, 0}, {Stage3AddressField::TriggerBit, 2, 0}, {Stage3AddressField::ClearBit, 3, 0}}; QVERIFY2(duplicateStage3Selection(project, {object.id}, options, &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.objects.last().xyCurve.readAddress, QStringLiteral("LW32")); QCOMPARE(result.objects.last().xyCurve.triggerAddress, QStringLiteral("LB12")); QCOMPARE(result.objects.last().xyCurve.clearAddress, QStringLiteral("LB23"));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("xy-display.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::xyStrictPersistenceAndBackendGates
 * Description     : Round-trip 25 families and strict nested axis/channel drafts while preserving refused backend outputs.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::xyStrictPersistenceAndBackendGates() //20261113-1 add
{
    HmiProject project = historyProject();
    HmiObject history = project.objects.first();
    HmiProject loaded;
    HmiProject only;
    HmiObject object = xyObject();
    XyCurveProperties parsed;
    QJsonObject json;
    QJsonObject nested;
    QJsonArray channels;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    object.xyCurve.xMinimum = QStringLiteral("-100"); object.xyCurve.xMaximum = QStringLiteral("4000"); object.xyCurve.xTotalDigits = 6; object.xyCurve.xFractionalDigits = 2;
    object.xyCurve.channelCount = 3; object.xyCurve.currentChannel = 2; object.xyCurve.channels[2].minimum = QStringLiteral("-10"); object.xyCurve.channels[2].maximum = QStringLiteral("2000"); object.xyCurve.channels[2].lineStyle = 2; object.xyCurve.channels[2].color = QStringLiteral("#0000ff"); object.xyCurve.yAxis.marks = false; object.xyCurve.yMaximum = QStringLiteral("5000");
    project.objects = existingFiveObjects(); project.objects.append({messageObject(), scaleObject(), qrObject(), imageObject(), gifObject(), numberObject(), asciiObject(), timeObject(), screenObject(project), multiObject(), mfObject(), dropdownObject(), flowObject(), sliderObject(), wheelObject(), trackObject(), chartObject(), trendObject(), history, object}); QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.objects.size(), 25); QCOMPARE(loaded.toJson(), project.toJson()); only.objects = {object}; json = only.toJson(); QCOMPARE(json.value(QStringLiteral("schemaVersion")).toInt(), 12); QVERIFY(HmiProject::fromJson(json, &only, &error)); json.insert(QStringLiteral("schemaVersion"), 11); QVERIFY(!HmiProject::fromJson(json, &only, &error)); QVERIFY(!exportNativeProjectSource(only, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(only, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")});
    QVERIFY(!object.xyCurve.toJson().contains(QStringLiteral("rightToLeft")));
    json = object.xyCurve.toJson(); json.insert(QStringLiteral("sampleTime"), 1.5); parsed.sampleTime = 9; QVERIFY(!XyCurveProperties::fromJson(json, &parsed, &error)); QCOMPARE(parsed.sampleTime, 9); json = object.xyCurve.toJson(); channels = json.value(QStringLiteral("channels")).toArray(); channels.removeLast(); json.insert(QStringLiteral("channels"), channels); QVERIFY(!XyCurveProperties::fromJson(json, &parsed, &error)); nested = object.xyCurve.xAxis.toJson(); nested.insert(QStringLiteral("major"), true); json = object.xyCurve.toJson(); json.insert(QStringLiteral("xAxis"), nested); QVERIFY(!XyCurveProperties::fromJson(json, &parsed, &error)); json = object.xyCurve.toJson(); json.insert(QStringLiteral("rightToLeft"), false); QVERIFY(!XyCurveProperties::fromJson(json, &parsed, &error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY2(loaded.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("xy-mixed.ksp")), &error), qPrintable(error)); QVERIFY2(only.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("xy-only.ksp")), &error), qPrintable(error)); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::xyStaticRasterAndClipping
 * Description     : Verify configured fills, independent numeric axis labels, independent curves and clipping without inventing acquisition.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::xyStaticRasterAndClipping() //20261113-1 add
{
    XyCurveProperties properties;
    QImage image(240, 160, QImage::Format_ARGB32);
    QImage forward;
    QPainter painter;
    image.fill(Qt::transparent); painter.begin(&image); paintXyCurve(&painter, QRectF(10, 10, 220, 140), properties); painter.end(); QCOMPARE(image.pixelColor(5, 5).alpha(), 0); QCOMPARE(image.pixelColor(235, 155).alpha(), 0); forward = image;
    properties.xMaximum = QStringLiteral("4000"); properties.xFractionalDigits = 2; image.fill(Qt::transparent); painter.begin(&image); paintXyCurve(&painter, QRectF(10, 10, 220, 140), properties); painter.end(); QVERIFY(image != forward); properties.channelCount = 2; properties.channels[1].color = QStringLiteral("#0000ff"); properties.bottomColor = QStringLiteral("#00ff00"); image.fill(Qt::transparent); painter.begin(&image); paintXyCurve(&painter, QRectF(10, 10, 220, 140), properties); painter.end(); QVERIFY(image != forward); QCOMPARE(image.pixelColor(5, 5).alpha(), 0);
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::xyShellPlacementEditAndUndo
 * Description     : Use the real X/Y action through cancel, pending placement, compact read edits and one-transaction history.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::xyShellPlacementEditAndUndo() //20261113-1 add
{
    MainWindow window;
    auto acceptTrend = []() { XyCurveDialog *dialog = qobject_cast<XyCurveDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QDialogButtonBox *>(QStringLiteral("XyButtons"))->button(QDialogButtonBox::Ok)->click(); } };
    window.setProject(HmiProject(), QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); QVERIFY(window.m_commands.contains(QStringLiteral("action_X_Y"))); QTimer::singleShot(0, &window, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } }); window.m_commands.value(QStringLiteral("action_X_Y"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement()); QTimer::singleShot(0, &window, acceptTrend); window.m_commands.value(QStringLiteral("action_X_Y"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QTest::keyClick(window.m_canvas->viewport(), Qt::Key_Escape); QVERIFY(window.currentEditorProject().objects.isEmpty());
    QTimer::singleShot(0, &window, acceptTrend); window.m_commands.value(QStringLiteral("action_X_Y"))->trigger(); QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(260, 180))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(260, 180))); QCOMPARE(window.currentEditorProject().objects.first().geometry.size(), QSize(240, 160)); QCOMPARE(window.findChild<QLabel *>(QStringLiteral("NativeSelectionType"))->text(), QStringLiteral("X-Y曲线")); QVERIFY(!window.m_textEdit->isEnabled()); window.m_addressEdit->setText(QStringLiteral("LW10")); window.applySelectedProperties(); QCOMPARE(window.currentEditorProject().objects.first().xyCurve.readAddress, QStringLiteral("LW10"));
    QTimer::singleShot(0, &window, []() { XyCurveDialog *dialog = qobject_cast<XyCurveDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QSpinBox *>(QStringLiteral("XySampleTime"))->setValue(255); dialog->findChild<QDialogButtonBox *>(QStringLiteral("XyButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().xyCurve.sampleTime, 255); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().objects.first().xyCurve.sampleTime, 1); window.restoreProjectHistory(-1); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty());
}

/******************************************************************************
 * Function Name   : groupObject
 * Description     : Create one distinct data-group point buffer without trend timing or paired-coordinate fields.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
static HmiObject groupObject() //20261113-1 add
{
    HmiObject object;
    object.id = QStringLiteral("group-object"); object.name = QStringLiteral("数据群组"); object.type = QStringLiteral("datagroup"); object.geometry = QRect(100, 100, 240, 160); return object;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::groupNativeDefaultsAndSingleChannelLimits
 * Description     : Match the measured signed-first types, 10..800 points, fixed one channel and missing unsupported fields.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::groupNativeDefaultsAndSingleChannelLimits() //20261113-1 add
{
    DataGroupDialog dialog(groupObject(), HmiProject());
    DataGroupProperties properties;
    QString error;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(dialog.findChild<QTabWidget *>(QStringLiteral("GroupTabs"))->count(), 3); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("GroupSamplePoints"))->minimum(), 10); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("GroupSamplePoints"))->maximum(), 800); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("GroupSamplePoints"))->value(), 100); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("GroupChannelCount"))->minimum(), 1); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("GroupChannelCount"))->maximum(), 1); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("GroupCurrentChannel"))->count(), 1); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("GroupDataType"))->itemText(1), QStringLiteral("32位整数")); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("GroupDataType"))->count(), 7); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("GroupLineStyle"))->count(), 5); QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("GroupClearAddress")) == nullptr); QVERIFY(!properties.toJson().contains(QStringLiteral("sampleTime"))); QVERIFY(!properties.toJson().contains(QStringLiteral("channelCount"))); properties.samplePoints = 9; QVERIFY(!properties.validate(&error)); properties.samplePoints = 800; QVERIFY(properties.validate(&error)); properties.samplePoints = 801; QVERIFY(!properties.validate(&error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("group-channel.png")))); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::groupSampleAndMaximumDraftLifetime
 * Description     : Keep the sample point count enabled in both modes and preserve independent constant and address maximum drafts.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::groupSampleAndMaximumDraftLifetime() //20261113-1 add
{
    DataGroupDialog dialog(groupObject(), HmiProject());
    QComboBox *mode = dialog.findChild<QComboBox *>(QStringLiteral("GroupSampleMode"));
    QComboBox *source = dialog.findChild<QComboBox *>(QStringLiteral("GroupRangeSource"));
    QLineEdit *sample = dialog.findChild<QLineEdit *>(QStringLiteral("GroupSampleAddress"));
    QTabWidget *tabs = dialog.findChild<QTabWidget *>(QStringLiteral("GroupTabs"));
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QVERIFY(!sample->isEnabled()); mode->setCurrentIndex(1); QVERIFY(sample->isEnabled()); QVERIFY(dialog.findChild<QSpinBox *>(QStringLiteral("GroupSamplePoints"))->isEnabled()); dialog.findChild<QSpinBox *>(QStringLiteral("GroupSamplePoints"))->setValue(250); sample->setText(QStringLiteral("LW10")); mode->setCurrentIndex(0); QVERIFY(!sample->isEnabled()); QCOMPARE(dialog.object().dataGroup.sampleAddress, QStringLiteral("LW10")); QCOMPARE(dialog.object().dataGroup.samplePoints, 250); mode->setCurrentIndex(1);
    tabs->setCurrentIndex(1); QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("GroupYMaximum"))->isVisible()); QVERIFY(!dialog.findChild<QLineEdit *>(QStringLiteral("GroupMaximumAddress"))->isVisible()); dialog.findChild<QLineEdit *>(QStringLiteral("GroupYMinimum"))->setText(QStringLiteral("-20")); dialog.findChild<QLineEdit *>(QStringLiteral("GroupYMaximum"))->setText(QStringLiteral("5000")); source->setCurrentIndex(1); QVERIFY(!dialog.findChild<QLineEdit *>(QStringLiteral("GroupYMaximum"))->isVisible()); QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("GroupMaximumAddress"))->isVisible()); QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("GroupYMinimum"))->isVisible()); dialog.findChild<QLineEdit *>(QStringLiteral("GroupMaximumAddress"))->setText(QStringLiteral("LW20")); source->setCurrentIndex(0); QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("GroupYMaximum"))->text(), QStringLiteral("5000")); source->setCurrentIndex(1); QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("GroupMaximumAddress"))->text(), QStringLiteral("LW20")); dialog.findChild<QCheckBox *>(QStringLiteral("GroupYMarks"))->setChecked(false); QVERIFY(!source->isVisible()); QVERIFY(!dialog.findChild<QLineEdit *>(QStringLiteral("GroupYMinimum"))->isVisible()); dialog.findChild<QCheckBox *>(QStringLiteral("GroupYMarks"))->setChecked(true); QVERIFY(source->isVisible()); QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("GroupMaximumAddress"))->isVisible()); QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("GroupXMinimum")) == nullptr); QVERIFY(dialog.findChild<QSpinBox *>(QStringLiteral("GroupXTotalDigits")) == nullptr); QCOMPARE(dialog.findChildren<HmiPermissionEditor *>().size(), 1);
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("group-display.png")))); }
    dialog.findChild<QDialogButtonBox *>(QStringLiteral("GroupButtons"))->button(QDialogButtonBox::Ok)->click(); QCOMPARE(dialog.result(), int(QDialog::Accepted)); QCOMPARE(dialog.object().dataGroup.yMinimum, QStringLiteral("0")); QCOMPARE(dialog.object().dataGroup.yMaximum, QStringLiteral("1000")); QCOMPARE(dialog.object().dataGroup.maximumAddress, QStringLiteral("LW20"));
    { HmiObject raw = groupObject(); raw.dataGroup.rangeSource = 1; raw.dataGroup.yMinimum = QStringLiteral("-20"); raw.dataGroup.yMaximum = QStringLiteral("5000"); DataGroupDialog reopened(raw, HmiProject()); QCOMPARE(reopened.object().dataGroup.yMinimum, QStringLiteral("0")); QCOMPARE(reopened.object().dataGroup.yMaximum, QStringLiteral("1000")); }

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::groupPointBufferAndIndependentAddressRoles
 * Description     : Validate the whole declared point buffer and independently increment trigger, sample and dynamic-maximum carriers.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::groupPointBufferAndIndependentAddressRoles() //20261113-1 add
{
    HmiObject object = groupObject();
    HmiProject project;
    HmiProject result;
    HmiProject sentinel;
    Stage3DuplicateOptions options;
    QString error;
    object.dataGroup.dataType = 1; object.dataGroup.samplePoints = 800; QCOMPARE(object.dataGroup.readWordSpan(), 1600U); object.dataGroup.readAddress = QStringLiteral("LW2497"); QVERIFY(object.dataGroup.validateAddresses(project.connections, &error)); object.dataGroup.readAddress = QStringLiteral("LW2498"); QVERIFY(!object.dataGroup.validateAddresses(project.connections, &error)); object.dataGroup.readAddress = QStringLiteral("LW0"); object.dataGroup.sampleAddress = QStringLiteral("invalid"); object.dataGroup.maximumAddress = QStringLiteral("invalid"); QVERIFY(object.dataGroup.validateAddresses(project.connections, &error)); object.dataGroup.sampleMode = 1; QVERIFY(!object.dataGroup.validateAddresses(project.connections, &error)); object.dataGroup.sampleAddress = QStringLiteral("LW10"); object.dataGroup.rangeSource = 1; object.dataGroup.maximumAddress = QStringLiteral("LW4096"); QVERIFY(!object.dataGroup.validateAddresses(project.connections, &error)); object.dataGroup.maximumAddress = QStringLiteral("LW4095"); QVERIFY(object.dataGroup.validateAddresses(project.connections, &error)); object.dataGroup.maximumAddress = QStringLiteral("LW20"); object.dataGroup.triggerAddress = QStringLiteral("LB10"); project.objects = {object}; options.columns = 2; options.increments = {{Stage3AddressField::Operation, 1600, 0}, {Stage3AddressField::TriggerBit, 2, 0}, {Stage3AddressField::SamplingWord, 1, 0}, {Stage3AddressField::DisplayMaximumWord, 2, 0}}; QVERIFY2(duplicateStage3Selection(project, {object.id}, options, &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.objects.last().dataGroup.readAddress, QStringLiteral("LW1600")); QCOMPARE(result.objects.last().dataGroup.triggerAddress, QStringLiteral("LB12")); QCOMPARE(result.objects.last().dataGroup.sampleAddress, QStringLiteral("LW11")); QCOMPARE(result.objects.last().dataGroup.maximumAddress, QStringLiteral("LW22")); project.objects[0].dataGroup.sampleAddress = QStringLiteral("LW4096"); sentinel = result; QVERIFY(!duplicateStage3Selection(project, {object.id}, options, &sentinel, nullptr, &error)); QCOMPARE(sentinel.toJson(), result.toJson());
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::groupStrictPersistenceAndBackendGates
 * Description     : Round-trip 26 families and reject trend-only fields and malformed source modes atomically.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::groupStrictPersistenceAndBackendGates() //20261113-1 add
{
    HmiProject project = historyProject();
    HmiObject history = project.objects.first();
    HmiObject object = groupObject();
    HmiProject loaded;
    HmiProject only;
    DataGroupProperties parsed;
    QJsonObject json;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    object.dataGroup.dataType = 1; object.dataGroup.sampleMode = 1; object.dataGroup.samplePoints = 250; object.dataGroup.sampleAddress = QStringLiteral("LW10"); object.dataGroup.rangeSource = 1; object.dataGroup.maximumAddress = QStringLiteral("LW20"); object.dataGroup.yMinimum = QStringLiteral("-20"); object.dataGroup.yMaximum = QStringLiteral("5000"); object.dataGroup.lineStyle = 3;
    project.objects = existingFiveObjects(); project.objects.append({messageObject(), scaleObject(), qrObject(), imageObject(), gifObject(), numberObject(), asciiObject(), timeObject(), screenObject(project), multiObject(), mfObject(), dropdownObject(), flowObject(), sliderObject(), wheelObject(), trackObject(), chartObject(), trendObject(), history, xyObject(), object}); QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.objects.size(), 26); QCOMPARE(loaded.toJson(), project.toJson()); only.objects = {object}; json = only.toJson(); QCOMPARE(json.value(QStringLiteral("schemaVersion")).toInt(), 12); QVERIFY(HmiProject::fromJson(json, &only, &error)); json.insert(QStringLiteral("schemaVersion"), 11); QVERIFY(!HmiProject::fromJson(json, &only, &error)); QVERIFY(!exportNativeProjectSource(only, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(only, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")}); json = object.dataGroup.toJson(); json.insert(QStringLiteral("samplePoints"), 1.5); parsed.samplePoints = 10; QVERIFY(!DataGroupProperties::fromJson(json, &parsed, &error)); QCOMPARE(parsed.samplePoints, 10); json = object.dataGroup.toJson(); json.insert(QStringLiteral("channelCount"), 2); QVERIFY(!DataGroupProperties::fromJson(json, &parsed, &error)); json = object.dataGroup.toJson(); json.insert(QStringLiteral("clearAddress"), QStringLiteral("LB0")); QVERIFY(!DataGroupProperties::fromJson(json, &parsed, &error)); parsed = object.dataGroup; parsed.yMaximum = QStringLiteral("draft"); QVERIFY(parsed.validate(&error)); parsed.rangeSource = 0; QVERIFY(!parsed.validate(&error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(project.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("group-mixed.ksp")), &error)); QVERIFY(only.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("group-only.ksp")), &error)); }
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::groupStaticFillOwnershipAndClipping
 * Description     : Keep blue outer fill and white plot ownership explicit without interpreting live sample-address values.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::groupStaticFillOwnershipAndClipping() //20261113-1 add
{
    DataGroupProperties properties;
    QImage image(240, 160, QImage::Format_ARGB32);
    QImage first;
    QPainter painter;
    image.fill(Qt::transparent); painter.begin(&image); paintDataGroup(&painter, QRectF(10, 10, 220, 140), properties); painter.end(); QCOMPARE(image.pixelColor(5, 5).alpha(), 0); QCOMPARE(image.pixelColor(11, 11), QColor(QStringLiteral("#0000ff"))); QCOMPARE(image.pixelColor(60, 60), QColor(QStringLiteral("#ffffff"))); first = image; properties.channelColor = QStringLiteral("#00ff00"); properties.samplePoints = 800; properties.rangeSource = 1; properties.yMaximum = QStringLiteral("unfinished"); image.fill(Qt::transparent); painter.begin(&image); paintDataGroup(&painter, QRectF(10, 10, 220, 140), properties); paintDataGroup(&painter, QRectF(1, 1, 1, 1), properties); painter.end(); QVERIFY(image != first); QCOMPARE(image.pixelColor(5, 5).alpha(), 0);
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::groupShellPlacementEditAndUndo
 * Description     : Use the actual dependency-free data-group action through accepted placement, compact buffer edits and undo.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::groupShellPlacementEditAndUndo() //20261113-1 add
{
    MainWindow window;
    auto acceptGroup = []() { DataGroupDialog *dialog = qobject_cast<DataGroupDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QDialogButtonBox *>(QStringLiteral("GroupButtons"))->button(QDialogButtonBox::Ok)->click(); } };
    window.setProject(HmiProject(), QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); QVERIFY(window.m_commands.contains(QStringLiteral("action_dataGroup"))); QTimer::singleShot(0, &window, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } }); window.m_commands.value(QStringLiteral("action_dataGroup"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement()); QTimer::singleShot(0, &window, acceptGroup); window.m_commands.value(QStringLiteral("action_dataGroup"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QTest::keyClick(window.m_canvas->viewport(), Qt::Key_Escape); QVERIFY(window.currentEditorProject().objects.isEmpty());
    QTimer::singleShot(0, &window, acceptGroup); window.m_commands.value(QStringLiteral("action_dataGroup"))->trigger(); QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(260, 180))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(260, 180))); QCOMPARE(window.currentEditorProject().objects.first().geometry.size(), QSize(240, 160)); QCOMPARE(window.findChild<QLabel *>(QStringLiteral("NativeSelectionType"))->text(), QStringLiteral("数据群组")); QVERIFY(!window.m_textEdit->isEnabled()); QVERIFY(window.currentEditorProject().configurationNodes.isEmpty()); window.m_addressEdit->setText(QStringLiteral("LW10")); window.applySelectedProperties(); QCOMPARE(window.currentEditorProject().objects.first().dataGroup.readAddress, QStringLiteral("LW10"));
    QTimer::singleShot(0, &window, []() { DataGroupDialog *dialog = qobject_cast<DataGroupDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QSpinBox *>(QStringLiteral("GroupSamplePoints"))->setValue(800); dialog->findChild<QDialogButtonBox *>(QStringLiteral("GroupButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().dataGroup.samplePoints, 800); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().objects.first().dataGroup.samplePoints, 100); window.restoreProjectHistory(-1); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty());
}

/******************************************************************************
 * Function Name   : alarmProject
 * Description     : Provide a typed alarm display and its two real multilingual definition rows.
 * Input Parameters: None
 * Return Value    : Complete C27 editor project.
 * Date            : 2026.11.13
 ******************************************************************************/
static HmiProject alarmProject() //20261113-2 add
{
    HmiProject project;
    HmiObject object;
    HmiConfigurationNode node;
    AlarmRegistrationProperties properties;
    object.id = QStringLiteral("alarmbar-1"); object.type = QStringLiteral("alarmbar"); object.name = QStringLiteral("动态报警条"); object.geometry = QRect(20, 20, 320, 60); properties.resizeRows(2, project.languages); properties.rows[0].messages[QStringLiteral("中文")] = QStringLiteral("ALM1"); node.id = QStringLiteral("alarm-bit"); node.kind = QStringLiteral("alarm"); node.name = QStringLiteral("数位地址报警"); node.active = true; node.implemented = true; node.properties = properties.toJson(); project.configurationNodes.append(node); project.objects = {object}; return project;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::alarmDefinitionsModesBoundsAndStrictRows
 * Description     : Cover all three bit families and four-level unsigned-first condition definitions with exact JSON atomicity.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::alarmDefinitionsModesBoundsAndStrictRows() //20261113-2 add
{
    AlarmRegistrationProperties properties;
    AlarmRegistrationProperties loaded;
    QJsonObject json;
    QString error;
    properties.resizeRows(511, {QStringLiteral("中文")}); QCOMPARE(properties.rows.size(), 511); QCOMPARE(properties.readSpan(), 511U); QVERIFY(properties.rows.first().used); properties.bitType = 1; QCOMPARE(properties.readSpan(), 32U); properties.bitType = 2; QCOMPARE(properties.readSpan(), 1U); properties.count = 512; QVERIFY(!properties.validate(&error)); properties.count = 511; QVERIFY(properties.validate(&error)); properties.family = 1; properties.resizeRows(31, {QStringLiteral("中文")}); QCOMPARE(properties.rows.size(), 124); QCOMPARE(AlarmRegistrationProperties::dataTypes().at(0), QStringLiteral("16位正整数")); properties.dataType = 3; QCOMPARE(properties.readSpan(), 62U); properties.rows[0].limit = QStringLiteral("-123"); QVERIFY(properties.validate(&error)); json = properties.toJson(); QVERIFY(AlarmRegistrationProperties::fromJson(json, &loaded, &error)); QCOMPARE(loaded.toJson(), json); json.insert(QStringLiteral("dataType"), 1.5); QVERIFY(!AlarmRegistrationProperties::fromJson(json, &loaded, &error)); QCOMPARE(loaded.toJson(), properties.toJson()); json = properties.toJson(); json.insert(QStringLiteral("extra"), true); QVERIFY(!AlarmRegistrationProperties::fromJson(json, &loaded, &error)); properties.rows[0].limit = QStringLiteral("nan"); QVERIFY(!properties.validate(&error)); properties.rows[0].used = false; QVERIFY(properties.validate(&error));

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::alarmRegistrationRowsLanguagesAndCommonType
 * Description     : Verify selected-row language isolation, used flags and common condition type without premature project writes.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::alarmRegistrationRowsLanguagesAndCommonType() //20261113-2 add
{
    HmiProject project;
    HmiConfigurationNode node;
    AlarmRegistrationProperties properties;
    QString error;
    project.languages = {QStringLiteral("中文"), QStringLiteral("English")}; AlarmRegistrationDialog dialog(node, project, 1); dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); dialog.findChild<QComboBox *>(QStringLiteral("AlarmRegistrationCount"))->setCurrentIndex(2); QCOMPARE(dialog.findChild<QTableWidget *>(QStringLiteral("AlarmRegistrationTable"))->rowCount(), 8); dialog.findChild<QComboBox *>(QStringLiteral("AlarmConditionDataType"))->setCurrentIndex(3); dialog.findChild<QLineEdit *>(QStringLiteral("AlarmMessage"))->setText(QStringLiteral("LOW")); QVERIFY(QMetaObject::invokeMethod(dialog.findChild<QLineEdit *>(QStringLiteral("AlarmMessage")), "textEdited", Q_ARG(QString, QStringLiteral("LOW")))); dialog.findChild<QLineEdit *>(QStringLiteral("AlarmConditionLimit"))->setText(QStringLiteral("-123")); QVERIFY(QMetaObject::invokeMethod(dialog.findChild<QLineEdit *>(QStringLiteral("AlarmConditionLimit")), "textEdited", Q_ARG(QString, QStringLiteral("-123")))); dialog.findChild<QTableWidget *>(QStringLiteral("AlarmRegistrationTable"))->setCurrentCell(4, 1); QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("AlarmRowAddress"))->text(), QStringLiteral("LW2")); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("AlarmConditionDataType"))->currentIndex(), 3); dialog.findChild<QTableWidget *>(QStringLiteral("AlarmRegistrationTable"))->setCurrentCell(0, 1); QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("AlarmMessage"))->text(), QStringLiteral("LOW")); dialog.findChild<QComboBox *>(QStringLiteral("AlarmLanguage"))->setCurrentText(QStringLiteral("English")); QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("AlarmMessage"))->text().isEmpty()); dialog.findChild<QLineEdit *>(QStringLiteral("AlarmMessage"))->setText(QStringLiteral("LOW EN")); QVERIFY(QMetaObject::invokeMethod(dialog.findChild<QLineEdit *>(QStringLiteral("AlarmMessage")), "textEdited", Q_ARG(QString, QStringLiteral("LOW EN")))); dialog.findChild<QTableWidget *>(QStringLiteral("AlarmRegistrationTable"))->item(1, 0)->setCheckState(Qt::Unchecked); QVERIFY(AlarmRegistrationProperties::fromJson(dialog.node().properties, &properties, &error)); QCOMPARE(properties.rows[0].messages.value(QStringLiteral("中文")), QStringLiteral("LOW")); QCOMPARE(properties.rows[0].messages.value(QStringLiteral("English")), QStringLiteral("LOW EN")); QCOMPARE(properties.rows[0].limit, QStringLiteral("-123")); QVERIFY(!properties.rows[1].used); QVERIFY(project.configurationNodes.isEmpty()); if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("alarm-condition.png")))); }

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::alarmPrerequisitesNavigationAndLanguageTransactions
 * Description     : Protect all-page alarm consumers, active navigation, full address spans and language changes.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::alarmPrerequisitesNavigationAndLanguageTransactions() //20261113-2 add
{
    HmiProject project = alarmProject();
    HmiProject sentinel;
    HmiPage page;
    AlarmRegistrationProperties definitions;
    QString error;
    QVERIFY(project.validateModel(&error)); QVERIFY(AlarmRegistrationProperties::fromJson(project.configurationNodes.first().properties, &definitions, &error)); definitions.readAddress = QStringLiteral("LB4096"); QVERIFY(!definitions.validateContext(project, &error)); definitions.bitType = 1; definitions.readAddress = QStringLiteral("LW4096"); QVERIFY(definitions.validateContext(project, &error)); definitions.family = 1; definitions.dataType = 3; definitions.resizeRows(2, project.languages); QVERIFY(!definitions.validateContext(project, &error)); definitions.readAddress = QStringLiteral("LW4093"); QVERIFY(definitions.validateContext(project, &error));
    page.width = 800; page.height = 480; page.id = QStringLiteral("alarm-target"); page.kind = QStringLiteral("normal"); page.name = QStringLiteral("报警目标"); QVERIFY(project.addPage(page, &error)); QVERIFY(AlarmRegistrationProperties::fromJson(project.configurationNodes.first().properties, &definitions, &error)); definitions.rows[0].jumpPageId = page.id; project.configurationNodes[0].properties = definitions.toJson(); QVERIFY(project.validateModel(&error)); sentinel = project; QVERIFY(!project.removePage(page.id, &error)); QCOMPARE(project.toJson(), sentinel.toJson()); definitions.rows[0].used = false; project.configurationNodes[0].properties = definitions.toJson(); QVERIFY(project.removePage(page.id, &error));
    QVERIFY(project.setLanguages({QStringLiteral("中文"), QStringLiteral("English")}, &error)); QVERIFY(AlarmRegistrationProperties::fromJson(project.configurationNodes.first().properties, &definitions, &error)); QVERIFY(definitions.rows[0].messages.contains(QStringLiteral("English"))); project.configurationNodes.clear(); QVERIFY(!project.validateModel(&error));

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::alarmBarDialogFieldsPermissionsAndCancel
 * Description     : Exercise measured speed and patterns, independent radio groups, display-only permissions and draft cancellation.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::alarmBarDialogFieldsPermissionsAndCancel() //20261113-2 add
{
    HmiProject project = alarmProject();
    HmiObject original = project.objects.first();
    AlarmBarDialog dialog(original, project);
    QSpinBox *speed = dialog.findChild<QSpinBox *>(QStringLiteral("AlarmSpeed"));
    QComboBox *pattern = dialog.findChild<QComboBox *>(QStringLiteral("AlarmPattern"));
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(speed->minimum(), 0); QCOMPARE(speed->maximum(), 30); speed->setValue(99); QCOMPARE(speed->value(), 30); QCOMPARE(pattern->count(), 13); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("AlarmFontSize"))->count(), 5); dialog.findChild<QRadioButton *>(QStringLiteral("AlarmRight"))->setChecked(true); dialog.findChild<QRadioButton *>(QStringLiteral("AlarmReverseTime"))->setChecked(true); QCOMPARE(dialog.object().alarmBar.direction, 1); QCOMPARE(dialog.object().alarmBar.sortOrder, 1); QVERIFY(!dialog.findChild<QRadioButton *>(QStringLiteral("AlarmLeft"))->isChecked()); QCOMPARE(dialog.findChildren<HmiPermissionEditor *>().size(), 1); QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("AlarmVisibleBitAddress")) != nullptr); if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("alarm-basic.png")))); } dialog.reject(); QCOMPARE(project.objects.first().toJson(), original.toJson());

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::alarmStrictProjectPersistenceAndBackendGates
 * Description     : Round-trip registration-only and mixed typed files while preserving source/device refusal and sentinel outputs.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::alarmStrictProjectPersistenceAndBackendGates() //20261113-2 add
{
    HmiProject project = alarmProject();
    HmiProject loaded;
    HmiProject mixed = historyProject();
    QJsonObject json;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    QVERIFY(HmiProject::fromJson(project.toJson(), &loaded, &error)); QCOMPARE(loaded.toJson(), project.toJson()); json = project.toJson(); QCOMPARE(json.value(QStringLiteral("schemaVersion")).toInt(), 12); project.objects.clear(); QCOMPARE(project.toJson().value(QStringLiteral("schemaVersion")).toInt(), 12); QVERIFY(HmiProject::fromJson(project.toJson(), &loaded, &error)); project = alarmProject(); mixed.configurationNodes.append(project.configurationNodes.first()); mixed.objects.append(project.objects.first()); QVERIFY2(HmiProject::fromJson(mixed.toJson(), &loaded, &error), qPrintable(error)); QVERIFY(!describeProjectCapabilities(project).value(QStringLiteral("deviceCompile")).toObject().value(QStringLiteral("status")).toString().contains(QStringLiteral("candidate"))); QVERIFY(!exportNativeProjectSource(project, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(project, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")}); json = project.toJson(); json.insert(QStringLiteral("schemaVersion"), 11); json.remove(QStringLiteral("animations")); QVERIFY(!HmiProject::fromJson(json, &loaded, &error)); if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(project.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("alarm-only.ksp")), &error)); QVERIFY(mixed.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("alarm-mixed.ksp")), &error)); }

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::alarmClipboardDefinitionsAndIndependentVisibility
 * Description     : Keep global definitions shared on duplicate, merge exact dependencies on paste and reject conflicts atomically.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::alarmClipboardDefinitionsAndIndependentVisibility() //20261113-2 add
{
    HmiProject project = alarmProject();
    HmiProject result;
    HmiProject target;
    HmiProject sentinel;
    Stage3DuplicateOptions options;
    Stage3Clipboard clipboard;
    AlarmRegistrationProperties definitions;
    QString error;
    project.objects[0].alarmBar.visibility.visibilityEnabled = true; project.objects[0].alarmBar.visibility.bitEnabled = true; project.objects[0].alarmBar.visibility.bitAddress = QStringLiteral("LB10"); project.objects[0].alarmBar.visibilityBitChosen = true; options.columns = 2; options.increments = {{Stage3AddressField::VisibilityBit, 2, 0}}; QVERIFY2(duplicateStage3Selection(project, {project.objects.first().id}, options, &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.objects.last().alarmBar.visibility.bitAddress, QStringLiteral("LB12")); QCOMPARE(result.configurationNodes.size(), 1); QVERIFY(captureStage3Selection(project, {project.objects.first().id}, &clipboard, &error)); QVERIFY2(pasteStage3Selection(target, clipboard, QPoint(10, 10), &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.configurationNodes.size(), 1); QVERIFY(result.validateModel(&error)); target = alarmProject(); target.objects.clear(); QVERIFY(AlarmRegistrationProperties::fromJson(target.configurationNodes.first().properties, &definitions, &error)); definitions.rows[0].messages[QStringLiteral("中文")] = QStringLiteral("different"); target.configurationNodes[0].properties = definitions.toJson(); sentinel = result; QVERIFY(!pasteStage3Selection(target, clipboard, QPoint(10, 10), &sentinel, nullptr, &error)); QCOMPARE(sentinel.toJson(), result.toJson());

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::alarmStaticSamplePatternsAndClipping
 * Description     : Verify every fill and all font sizes remain inside the supplied canvas rectangle without runtime alarm input.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::alarmStaticSamplePatternsAndClipping() //20261113-2 add
{
    AlarmBarProperties properties;
    QImage image(160, 100, QImage::Format_ARGB32);
    QPainter painter;
    int pattern;
    for (pattern = 0; pattern < 13; ++pattern) { image.fill(Qt::magenta); properties.pattern = pattern; properties.fontSize = 64; painter.begin(&image); paintAlarmBar(&painter, QRectF(20, 20, 120, 60), properties); painter.end(); QCOMPARE(image.pixelColor(1, 1), QColor(Qt::magenta)); QCOMPARE(image.pixelColor(159, 99), QColor(Qt::magenta)); QVERIFY(image.pixelColor(20, 20) != QColor(Qt::magenta)); } properties.speed = -1; QVERIFY(!properties.validate()); properties.speed = 0; properties.fontSize = 15; QVERIFY(!properties.validate());

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::alarmShellGateRegistrationPlacementAndUndo
 * Description     : Verify the actual shell gate, configuration transaction, placement, compact fields and undo.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::alarmShellGateRegistrationPlacementAndUndo() //20261113-2 add
{
    MainWindow window;
    auto acceptAlarm = []() { AlarmBarDialog *dialog = qobject_cast<AlarmBarDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QDialogButtonBox *>(QStringLiteral("AlarmButtons"))->button(QDialogButtonBox::Ok)->click(); } };
    window.setProject(HmiProject(), QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); QVERIFY(window.m_commands.contains(QStringLiteral("action_dynamicarlarm"))); QTimer::singleShot(0, &window, []() { QMessageBox *message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget()); if (message != nullptr) { QCOMPARE(message->text(), QStringLiteral("位报警和条件报警都不存在")); message->accept(); } }); window.m_commands.value(QStringLiteral("action_dynamicarlarm"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement()); QTimer::singleShot(0, &window, []() { AlarmRegistrationDialog *dialog = qobject_cast<AlarmRegistrationDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QComboBox *>(QStringLiteral("AlarmRegistrationCount"))->setCurrentIndex(2); dialog->findChild<QDialogButtonBox *>(QStringLiteral("AlarmRegistrationButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.configureAlarmRegistration(0); QCOMPARE(window.currentEditorProject().configurationNodes.size(), 1); QVERIFY(hasAlarmRegistrations(window.currentEditorProject())); QTimer::singleShot(0, &window, acceptAlarm); window.m_commands.value(QStringLiteral("action_dynamicarlarm"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20, 20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(340, 80))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(340, 80))); QCOMPARE(window.currentEditorProject().objects.size(), 1); QCOMPARE(window.currentEditorProject().objects.first().geometry.size(), QSize(320, 60)); QCOMPARE(window.findChild<QLabel *>(QStringLiteral("NativeSelectionType"))->text(), QStringLiteral("动态报警条")); QVERIFY(!window.m_textEdit->isEnabled()); QVERIFY(!window.m_addressEdit->isEnabled()); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty()); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().configurationNodes.isEmpty());

}

/******************************************************************************
 * Function Name   : alarmDisplayProject
 * Description     : Provide one real registered alarm table with two independent caption languages.
 * Input Parameters: None
 * Return Value    : Complete C28 editor project.
 * Date            : 2026.11.13
 ******************************************************************************/
static HmiProject alarmDisplayProject() //20261113-2 add
{
    HmiProject project = alarmProject();
    project.languages = {QStringLiteral("中文"), QStringLiteral("English")}; project.objects[0].type = QStringLiteral("alarmdisplay"); project.objects[0].id = QStringLiteral("alarm-display-1"); project.objects[0].name = QStringLiteral("报警显示器"); project.objects[0].geometry = QRect(20,20,320,160); project.objects[0].alarmDisplay.setLanguages(project.languages); return project;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::alarmDisplayDefaultsAndStrictRecords
 * Description     : Validate seven sizes, measured bounds and complete nested caption records atomically.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::alarmDisplayDefaultsAndStrictRecords() //20261113-2 add
{
    AlarmDisplayProperties properties;
    AlarmDisplayProperties parsed;
    QJsonObject json;
    QJsonObject captions;
    QString error;
    QCOMPARE(properties.rows, 5); QCOMPARE(properties.fontSize, 16); QCOMPARE(properties.backgroundColor, QStringLiteral("#e0e0e0")); QCOMPARE(AlarmDisplayProperties::fontSizes(), QList<int>({12,16,24,48,64,96,192})); QVERIFY(properties.validate(&error)); properties.fontSize = 192; properties.rows = 20; QVERIFY(properties.validate(&error)); json = properties.toJson(); QVERIFY(AlarmDisplayProperties::fromJson(json, &parsed, &error)); QCOMPARE(parsed.toJson(), json); json.insert(QStringLiteral("rows"), 1.5); QVERIFY(!AlarmDisplayProperties::fromJson(json, &parsed, &error)); QCOMPARE(parsed.toJson(), properties.toJson()); json = properties.toJson(); json.insert(QStringLiteral("extra"), true); QVERIFY(!AlarmDisplayProperties::fromJson(json, &parsed, &error)); properties.rows = 0; QVERIFY(!properties.validate(&error)); properties.rows = 21; QVERIFY(!properties.validate(&error)); properties.rows = 5; properties.fontSize = 15; QVERIFY(!properties.validate(&error)); properties.fontSize = 16; captions = properties.toJson().value(QStringLiteral("captions")).toObject(); captions.insert(QStringLiteral("English"), QJsonObject{{QStringLiteral("message"), QStringLiteral("Alarm")}}); json = properties.toJson(); json.insert(QStringLiteral("captions"), captions); QVERIFY(!AlarmDisplayProperties::fromJson(json, &parsed, &error));

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::alarmDisplayCaptionLanguagesAndCommonStyles
 * Description     : Verify only three captions change with language while font, row count and column switches remain common.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::alarmDisplayCaptionLanguagesAndCommonStyles() //20261113-2 add
{
    HmiProject project = alarmDisplayProject();
    AlarmDisplayDialog dialog(project.objects.first(), project);
    QComboBox *language = dialog.findChild<QComboBox *>(QStringLiteral("AlarmDisplayLanguage"));
    QLineEdit *message = dialog.findChild<QLineEdit *>(QStringLiteral("AlarmDisplayMessageCaption"));
    QLineEdit *date = dialog.findChild<QLineEdit *>(QStringLiteral("AlarmDisplayDateCaption"));
    QLineEdit *time = dialog.findChild<QLineEdit *>(QStringLiteral("AlarmDisplayTimeCaption"));
    QComboBox *font = dialog.findChild<QComboBox *>(QStringLiteral("AlarmDisplayFontSize"));
    QSpinBox *rows = dialog.findChild<QSpinBox *>(QStringLiteral("AlarmDisplayRows"));
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("AlarmDisplayFont"))->count(), 1); QCOMPARE(font->count(), 7); QCOMPARE(rows->minimum(), 1); QCOMPARE(rows->maximum(), 20); message->setText(QStringLiteral("ZH Message")); date->setText(QStringLiteral("ZH Date")); time->setText(QStringLiteral("ZH Time")); font->setCurrentText(QStringLiteral("24")); rows->setValue(20); dialog.findChild<QCheckBox *>(QStringLiteral("AlarmDisplayShowDate"))->setChecked(false); QVERIFY(!date->isEnabled()); language->setCurrentText(QStringLiteral("English")); QCOMPARE(message->text(), QStringLiteral("消息")); QCOMPARE(font->currentText(), QStringLiteral("24")); QCOMPARE(rows->value(), 20); QVERIFY(!date->isEnabled()); message->setText(QStringLiteral("EN Message")); dialog.findChild<QCheckBox *>(QStringLiteral("AlarmDisplayShowDate"))->setChecked(true); date->setText(QStringLiteral("EN Date")); time->setText(QStringLiteral("EN Time")); language->setCurrentText(QStringLiteral("中文")); QCOMPARE(message->text(), QStringLiteral("ZH Message")); QCOMPARE(date->text(), QStringLiteral("ZH Date")); QCOMPARE(time->text(), QStringLiteral("ZH Time")); QVERIFY(date->isEnabled()); QCOMPARE(dialog.object().alarmDisplay.captionsFor(QStringLiteral("English")).date, QStringLiteral("EN Date")); QCOMPARE(dialog.findChildren<HmiPermissionEditor *>().size(), 1); if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("alarm-display-basic.png")))); } dialog.reject(); QCOMPARE(project.objects.first().alarmDisplay.rows, 5);

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::alarmDisplaySharedPrerequisitesCopyAndVisibility
 * Description     : Apply global alarm dependencies and independent visibility carriers to table displays across duplicate and paste.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::alarmDisplaySharedPrerequisitesCopyAndVisibility() //20261113-2 add
{
    HmiProject project = alarmDisplayProject();
    HmiProject result;
    HmiProject target;
    HmiPage page;
    Stage3DuplicateOptions options;
    Stage3Clipboard clipboard;
    HmiProject sentinel;
    QString error;
    project.objects[0].alarmDisplay.visibility.visibilityEnabled = true; project.objects[0].alarmDisplay.visibility.bitEnabled = true; project.objects[0].alarmDisplay.visibility.bitAddress = QStringLiteral("LB10"); project.objects[0].alarmDisplay.visibilityBitChosen = true; options.columns = 2; options.increments = {{Stage3AddressField::VisibilityBit, 3, 0}}; QVERIFY2(duplicateStage3Selection(project, {project.objects.first().id}, options, &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.objects.last().alarmDisplay.visibility.bitAddress, QStringLiteral("LB13")); QCOMPARE(result.configurationNodes.size(), 1); QVERIFY(captureStage3Selection(project, {project.objects.first().id}, &clipboard, &error)); sentinel = result; QVERIFY(!pasteStage3Selection(target, clipboard, QPoint(10,10), &result, nullptr, &error)); QCOMPARE(result.toJson(), sentinel.toJson()); target.languages = project.languages; QVERIFY2(pasteStage3Selection(target, clipboard, QPoint(10,10), &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.configurationNodes.size(), 1); QVERIFY(result.validateModel(&error)); page.id = QStringLiteral("inactive-alarm-page"); page.name = QStringLiteral("Inactive"); page.objects = project.objects; project.objects.clear(); QVERIFY(project.addPage(page, &error)); project.configurationNodes.clear(); QVERIFY(!project.validateModel(&error));

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::alarmDisplayPersistenceLanguagesAndBackendGates
 * Description     : Round-trip typed displays and common configurations while keeping language updates transactional and unsupported output immutable.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::alarmDisplayPersistenceLanguagesAndBackendGates() //20261113-2 add
{
    HmiProject project = alarmDisplayProject();
    HmiProject loaded;
    HmiProject mixed = alarmProject();
    QJsonObject json;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    project.objects[0].alarmDisplay.captions[QStringLiteral("English")].message = QStringLiteral("Alarm"); QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.toJson(), project.toJson()); json = project.toJson(); QCOMPARE(json.value(QStringLiteral("schemaVersion")).toInt(), 12); json.insert(QStringLiteral("schemaVersion"), 11); json.remove(QStringLiteral("animations")); QVERIFY(!HmiProject::fromJson(json, &loaded, &error)); QVERIFY(project.setLanguages({QStringLiteral("English")}, &error)); QCOMPARE(project.objects.first().alarmDisplay.captions.size(), 1); QCOMPARE(project.objects.first().alarmDisplay.captionsFor(QStringLiteral("English")).message, QStringLiteral("Alarm")); QVERIFY(!exportNativeProjectSource(project, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(project, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")}); mixed.languages = project.languages; mixed.activeLanguage = project.activeLanguage; mixed.objects.append(project.objects.first()); QVERIFY2(mixed.validateModel(&error), qPrintable(error)); if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(project.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("alarm-display-only.ksp")), &error)); QVERIFY(mixed.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("alarm-display-mixed.ksp")), &error)); }

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::alarmDisplayStaticColumnsFontsAndClipping
 * Description     : Exercise every column combination and all measured sizes without claiming timestamps, paging or live alarm content.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::alarmDisplayStaticColumnsFontsAndClipping() //20261113-2 add
{
    AlarmDisplayProperties properties;
    QImage image(200, 160, QImage::Format_ARGB32);
    QPainter painter;
    int combination;
    int font;
    for (combination = 0; combination < 4; ++combination) { properties.showDate = (combination & 1) != 0; properties.showTime = (combination & 2) != 0; Q_FOREACH (font, AlarmDisplayProperties::fontSizes()) { properties.fontSize = font; properties.rows = 20; image.fill(Qt::magenta); painter.begin(&image); paintAlarmDisplay(&painter, QRectF(20,20,160,120), properties, QStringLiteral("中文")); painter.end(); QCOMPARE(image.pixelColor(1,1), QColor(Qt::magenta)); QCOMPARE(image.pixelColor(199,159), QColor(Qt::magenta)); QVERIFY(image.pixelColor(25,25) != QColor(Qt::magenta)); } }

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::alarmDisplayShellGatePlacementEditingAndUndo
 * Description     : Verify actual action gating, detached cancellation, table placement, compact field ownership and undo.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::alarmDisplayShellGatePlacementEditingAndUndo() //20261113-2 add
{
    MainWindow window;
    HmiProject project = alarmDisplayProject();
    auto acceptTable = []() { AlarmDisplayDialog *dialog = qobject_cast<AlarmDisplayDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QDialogButtonBox *>(QStringLiteral("AlarmDisplayButtons"))->button(QDialogButtonBox::Ok)->click(); } };
    window.setProject(HmiProject(), QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); QVERIFY(window.m_commands.contains(QStringLiteral("action_arlamcontrol"))); QTimer::singleShot(0, &window, []() { QMessageBox *message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget()); if (message != nullptr) { QCOMPARE(message->text(), QStringLiteral("位报警和条件报警都不存在")); message->accept(); } }); window.m_commands.value(QStringLiteral("action_arlamcontrol"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement()); project.objects.clear(); window.setProject(project, QString()); QTimer::singleShot(0, &window, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } }); window.m_commands.value(QStringLiteral("action_arlamcontrol"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement()); QTimer::singleShot(0, &window, acceptTable); window.m_commands.value(QStringLiteral("action_arlamcontrol"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20,20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(340,180))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(340,180))); QCOMPARE(window.currentEditorProject().objects.first().geometry.size(), QSize(320,160)); QCOMPARE(window.findChild<QLabel *>(QStringLiteral("NativeSelectionType"))->text(), QStringLiteral("报警显示器")); QVERIFY(!window.m_addressEdit->isEnabled()); QVERIFY(!window.m_textEdit->isEnabled()); QTimer::singleShot(0, &window, []() { AlarmDisplayDialog *dialog = qobject_cast<AlarmDisplayDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QSpinBox *>(QStringLiteral("AlarmDisplayRows"))->setValue(20); dialog->findChild<QDialogButtonBox *>(QStringLiteral("AlarmDisplayButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().alarmDisplay.rows, 20); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().objects.first().alarmDisplay.rows, 5); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty()); QCOMPARE(window.currentEditorProject().configurationNodes.size(), 1);

}

/******************************************************************************
 * Function Name   : historyAlarmProject
 * Description     : Create a history table with the same editable columns and a separate object identity.
 * Input Parameters: None
 * Return Value    : Complete C29 editor project.
 * Date            : 2026.11.13
 ******************************************************************************/
static HmiProject historyAlarmProject() //20261113-2 add
{
    HmiProject project = alarmDisplayProject();
    project.objects[0].type = QStringLiteral("historyalarm"); project.objects[0].id = QStringLiteral("history-alarm-1"); project.objects[0].name = QStringLiteral("历史报警显示器"); return project;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::historyAlarmSharedDialogKeepsDistinctIdentity
 * Description     : Retain historical identity through the shared table editor and detached cancellation.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::historyAlarmSharedDialogKeepsDistinctIdentity() //20261113-2 add
{
    HmiProject project = historyAlarmProject();
    HmiObject original = project.objects.first();
    AlarmDisplayDialog dialog(original, project);
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(dialog.windowTitle(), QStringLiteral("历史报警显示器")); QCOMPARE(dialog.object().type, QStringLiteral("historyalarm")); QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("AlarmDisplayFontSize"))->count(), 7); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("AlarmDisplayRows"))->value(), 5); if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("history-alarm-basic.png")))); } dialog.findChild<QComboBox *>(QStringLiteral("AlarmDisplayFontSize"))->setCurrentText(QStringLiteral("192")); dialog.findChild<QSpinBox *>(QStringLiteral("AlarmDisplayRows"))->setValue(20); dialog.findChild<QCheckBox *>(QStringLiteral("AlarmDisplayShowDate"))->setChecked(false); QCOMPARE(dialog.object().type, QStringLiteral("historyalarm")); QCOMPARE(dialog.object().alarmDisplay.fontSize, 192); QVERIFY(!dialog.findChild<QLineEdit *>(QStringLiteral("AlarmDisplayDateCaption"))->isEnabled()); dialog.reject(); QCOMPARE(project.objects.first().toJson(), original.toJson());

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::historyAlarmStrictTypeLanguagesAndBackendGates
 * Description     : Keep current and historical records distinct in schema12 and maintain source/device refusal with immutable outputs.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::historyAlarmStrictTypeLanguagesAndBackendGates() //20261113-2 add
{
    HmiProject project = historyAlarmProject();
    HmiProject loaded;
    HmiProject mixed = alarmDisplayProject();
    HmiObject parsed;
    QJsonObject json;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    project.objects[0].alarmDisplay.captions[QStringLiteral("English")].message = QStringLiteral("History"); QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.objects.first().type, QStringLiteral("historyalarm")); QCOMPARE(loaded.toJson(), project.toJson()); json = project.objects.first().toJson(); QVERIFY(json.contains(QStringLiteral("alarmDisplay"))); json.insert(QStringLiteral("type"), QStringLiteral("text")); QVERIFY(!HmiObject::fromJson(json, &parsed, &error)); json = project.toJson(); json.insert(QStringLiteral("schemaVersion"), 11); json.remove(QStringLiteral("animations")); QVERIFY(!HmiProject::fromJson(json, &loaded, &error)); QVERIFY(project.setLanguages({QStringLiteral("English")}, &error)); QCOMPARE(project.objects.first().alarmDisplay.captions.size(), 1); QCOMPARE(project.objects.first().alarmDisplay.captionsFor(QStringLiteral("English")).message, QStringLiteral("History")); QVERIFY(!exportNativeProjectSource(project, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(project, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")}); mixed.objects.append(historyAlarmProject().objects.first()); mixed.objects.append(alarmProject().objects.first()); QVERIFY2(HmiProject::fromJson(mixed.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.objects.size(), 3); QCOMPARE(loaded.objects[0].type, QStringLiteral("alarmdisplay")); QCOMPARE(loaded.objects[1].type, QStringLiteral("historyalarm")); QCOMPARE(loaded.objects[2].type, QStringLiteral("alarmbar")); if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(project.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("history-alarm-only.ksp")), &error)); QVERIFY(mixed.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("history-alarm-mixed.ksp")), &error)); }

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::historyAlarmWordVisibilityAndCopyDependencies
 * Description     : Validate full visibility word width, global alarm prerequisites and atomic language-compatible pasting for history tables.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::historyAlarmWordVisibilityAndCopyDependencies() //20261113-2 add
{
    HmiProject project = historyAlarmProject();
    HmiProject result;
    HmiProject target;
    HmiProject sentinel;
    Stage3DuplicateOptions options;
    Stage3Clipboard clipboard;
    QString error;
    project.objects[0].alarmDisplay.visibility.visibilityEnabled = true; project.objects[0].alarmDisplay.visibility.wordEnabled = true; project.objects[0].alarmDisplay.visibility.dataType = 1; project.objects[0].alarmDisplay.visibility.wordAddress = QStringLiteral("LW4096"); QVERIFY(!project.validateModel(&error)); project.objects[0].alarmDisplay.visibility.wordAddress = QStringLiteral("LW4095"); QVERIFY(project.validateModel(&error)); project.objects[0].alarmDisplay.visibility.wordAddress = QStringLiteral("LW10"); options.columns = 2; options.increments = {{Stage3AddressField::VisibilityWord, 2, 0}}; QVERIFY2(duplicateStage3Selection(project, {project.objects.first().id}, options, &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.objects.last().alarmDisplay.visibility.wordAddress, QStringLiteral("LW12")); QCOMPARE(result.objects.last().type, QStringLiteral("historyalarm")); QCOMPARE(result.configurationNodes.size(), 1); QVERIFY(captureStage3Selection(project, {project.objects.first().id}, &clipboard, &error)); sentinel = result; QVERIFY(!pasteStage3Selection(target, clipboard, QPoint(10,10), &result, nullptr, &error)); QCOMPARE(result.toJson(), sentinel.toJson()); target.languages = project.languages; QVERIFY2(pasteStage3Selection(target, clipboard, QPoint(10,10), &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.objects.first().type, QStringLiteral("historyalarm")); QCOMPARE(result.configurationNodes.size(), 1); result.configurationNodes.clear(); QVERIFY(!result.validateModel(&error));

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::historyAlarmShellEntryPlacementEditAndUndo
 * Description     : Exercise the distinct history action, warning title, shared dialog, placement, properties and undo.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::historyAlarmShellEntryPlacementEditAndUndo() //20261113-2 add
{
    MainWindow window;
    HmiProject project = historyAlarmProject();
    auto acceptHistory = []() { AlarmDisplayDialog *dialog = qobject_cast<AlarmDisplayDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { QCOMPARE(dialog->windowTitle(), QStringLiteral("历史报警显示器")); QCOMPARE(dialog->object().type, QStringLiteral("historyalarm")); dialog->findChild<QDialogButtonBox *>(QStringLiteral("AlarmDisplayButtons"))->button(QDialogButtonBox::Ok)->click(); } };
    window.setProject(HmiProject(), QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); QVERIFY(window.m_commands.contains(QStringLiteral("action_histotyarlarm"))); QTimer::singleShot(0, &window, []() { QMessageBox *message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget()); if (message != nullptr) { QCOMPARE(message->windowTitle(), QStringLiteral("历史报警显示器")); QCOMPARE(message->text(), QStringLiteral("位报警和条件报警都不存在")); message->accept(); } }); window.m_commands.value(QStringLiteral("action_histotyarlarm"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement()); project.objects.clear(); window.setProject(project, QString()); QTimer::singleShot(0, &window, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->reject(); } }); window.m_commands.value(QStringLiteral("action_histotyarlarm"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement()); QTimer::singleShot(0, &window, acceptHistory); window.m_commands.value(QStringLiteral("action_histotyarlarm"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20,20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(340,180))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(340,180))); QCOMPARE(window.currentEditorProject().objects.first().type, QStringLiteral("historyalarm")); QCOMPARE(window.findChild<QLabel *>(QStringLiteral("NativeSelectionType"))->text(), QStringLiteral("历史报警显示器")); QVERIFY(!window.m_addressEdit->isEnabled()); QVERIFY(!window.m_textEdit->isEnabled()); QTimer::singleShot(0, &window, []() { AlarmDisplayDialog *dialog = qobject_cast<AlarmDisplayDialog *>(QApplication::activeModalWidget()); if (dialog != nullptr) { dialog->findChild<QSpinBox *>(QStringLiteral("AlarmDisplayRows"))->setValue(20); dialog->findChild<QDialogButtonBox *>(QStringLiteral("AlarmDisplayButtons"))->button(QDialogButtonBox::Ok)->click(); } }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().type, QStringLiteral("historyalarm")); QCOMPARE(window.currentEditorProject().objects.first().alarmDisplay.rows, 20); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().objects.first().alarmDisplay.rows, 5); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty());

}


/******************************************************************************
 * Function Name   : historyDataProject
 * Description     : Build a three-channel collector and independent table with three project languages.
 * Input Parameters: None
 * Return Value    : Complete C30 editor project.
 * Date            : 2026.11.13
 ******************************************************************************/
static HmiProject historyDataProject() //20261113-2 add
{
    HmiProject project = historyProject();
    DataCollectionProperties collection;
    HmiObject object;
    DataCollectionProperties::fromJson(project.configurationNodes.first().properties, &collection); project.languages = {QStringLiteral("中文"), QStringLiteral("English"), QStringLiteral("Deutsch")}; object.id = QStringLiteral("history-data-1"); object.name = QStringLiteral("历史数据显示器"); object.type = QStringLiteral("historydata"); object.geometry = QRect(100,100,320,180); object.historyData.collectionId = project.configurationNodes.first().id; object.historyData.reconcileCollection(collection, project.languages); project.objects = {object}; return project;
}

/******************************************************************************
 * Function Name   : Stage4ControlTests::historyDataMeasuredDefaultsAndZeroRows
 * Description     : Keep native C30 defaults separate from alarm-table limits and retain editable hidden date fields.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::historyDataMeasuredDefaultsAndZeroRows() //20261113-2 add
{
    HmiProject project = historyDataProject();
    HistoryDataDialog dialog(project.objects.first(), project);
    QSpinBox *rows = dialog.findChild<QSpinBox *>(QStringLiteral("HistoryDataRows"));
    QComboBox *size = dialog.findChild<QComboBox *>(QStringLiteral("HistoryDataFontSize"));
    HistoryDataProperties properties = project.objects.first().historyData;
    QString error;
    dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); QCOMPARE(dialog.windowTitle(), QStringLiteral("历史数据显示器")); QCOMPARE(dialog.findChild<QTabWidget *>(QStringLiteral("HistoryDataTabs"))->count(), 3); QCOMPARE(dialog.findChildren<HmiPermissionEditor *>().size(), 1); QCOMPARE(rows->minimum(), 0); QCOMPARE(rows->maximum(), 20); QCOMPARE(rows->value(), 5); QCOMPARE(size->count(), 4); QCOMPARE(size->currentText(), QStringLiteral("16")); QCOMPARE(properties.lineColor, QStringLiteral("#00c000")); QCOMPARE(properties.columns.size(), 3); QCOMPARE(properties.columns.first().names.value(QStringLiteral("中文")), QStringLiteral("First"));
    dialog.findChild<QCheckBox *>(QStringLiteral("HistoryDataShowDate"))->setChecked(false); dialog.findChild<QCheckBox *>(QStringLiteral("HistoryDataShowTime"))->setChecked(false); QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("HistoryDataDateCaption"))->isEnabled()); QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("HistoryDataTimeCaption"))->isEnabled()); rows->setValue(0); size->setCurrentText(QStringLiteral("48")); QVERIFY(dialog.object().historyData.validate(&error)); QCOMPARE(dialog.object().historyData.rows, 0); QCOMPARE(dialog.object().historyData.fontSize, 48); properties.rows = 21; QVERIFY(!properties.validate(&error)); properties.rows = 0; properties.fontSize = 64; QVERIFY(!properties.validate(&error));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("history-data-general.png")))); }

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::historyDataAllColumnLanguageSynchronization
 * Description     : Synchronize all first-language names to current or all languages while preserving date/time captions, shared flags and project language.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::historyDataAllColumnLanguageSynchronization() //20261113-2 add
{
    HmiProject project = historyDataProject();
    HistoryDataDialog dialog(project.objects.first(), project);
    QComboBox *language = dialog.findChild<QComboBox *>(QStringLiteral("HistoryDataDataLanguage"));
    QComboBox *general = dialog.findChild<QComboBox *>(QStringLiteral("HistoryDataLanguage"));
    QTableWidget *table = dialog.findChild<QTableWidget *>(QStringLiteral("HistoryDataColumns"));
    QLineEdit *name = dialog.findChild<QLineEdit *>(QStringLiteral("HistoryDataColumnName"));
    int row;
    for (row = 0; row < 3; ++row) { table->setCurrentCell(row, 0); name->setText(QStringLiteral("C%1").arg(row)); } dialog.findChild<QLineEdit *>(QStringLiteral("HistoryDataDateCaption"))->setText(QStringLiteral("D0")); table->item(1, 1)->setCheckState(Qt::Unchecked); language->setCurrentText(QStringLiteral("English")); QCOMPARE(general->currentText(), QStringLiteral("English"));
    for (row = 0; row < 3; ++row) { table->setCurrentCell(row, 0); name->setText(QStringLiteral("E%1").arg(row)); } dialog.findChild<QLineEdit *>(QStringLiteral("HistoryDataDateCaption"))->setText(QStringLiteral("D1")); language->setCurrentText(QStringLiteral("Deutsch")); for (row = 0; row < 3; ++row) { table->setCurrentCell(row, 0); name->setText(QStringLiteral("G%1").arg(row)); }
    language->setCurrentText(QStringLiteral("English")); table->setCurrentCell(1, 0); dialog.findChild<QPushButton *>(QStringLiteral("HistoryDataSyncCurrent"))->click(); for (row = 0; row < 3; ++row) { QCOMPARE(dialog.object().historyData.columns[row].names.value(QStringLiteral("English")), QStringLiteral("C%1").arg(row)); QCOMPARE(dialog.object().historyData.columns[row].names.value(QStringLiteral("Deutsch")), QStringLiteral("G%1").arg(row)); } QCOMPARE(table->currentRow(), 1); QCOMPARE(name->text(), QStringLiteral("C1"));
    dialog.findChild<QPushButton *>(QStringLiteral("HistoryDataSyncAll"))->click(); for (row = 0; row < 3; ++row) { QCOMPARE(dialog.object().historyData.columns[row].names.value(QStringLiteral("Deutsch")), QStringLiteral("C%1").arg(row)); } QVERIFY(!dialog.object().historyData.columns[1].shown); QCOMPARE(dialog.object().historyData.captionsFor(QStringLiteral("中文")).date, QStringLiteral("D0")); QCOMPARE(dialog.object().historyData.captionsFor(QStringLiteral("English")).date, QStringLiteral("D1")); QCOMPARE(project.activeLanguage, QStringLiteral("中文")); dialog.findChild<QTabWidget *>(QStringLiteral("HistoryDataTabs"))->setCurrentIndex(1); dialog.show(); QVERIFY(QTest::qWaitForWindowExposed(&dialog)); if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("history-data-columns.png")))); } dialog.reject(); QCOMPARE(project.objects.first().historyData.columns.first().names.value(QStringLiteral("中文")), QStringLiteral("First"));

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::historyDataVisibleColumnGuard
 * Description     : Accept date-only and zero-row tables but reject the measured no-visible-column case without closing the parent editor.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::historyDataVisibleColumnGuard() //20261113-2 add
{
    HmiProject project = historyDataProject();
    HistoryDataProperties properties = project.objects.first().historyData;
    HistoryDataDialog dialog(project.objects.first(), project);
    QTableWidget *table = dialog.findChild<QTableWidget *>(QStringLiteral("HistoryDataColumns"));
    QString error;
    int index;
    for (index = 0; index < properties.columns.size(); ++index) { properties.columns[index].shown = false; table->item(index, 1)->setCheckState(Qt::Unchecked); } properties.showTime = false; properties.rows = 0; QVERIFY(properties.validate(&error)); properties.showDate = false; QVERIFY(!properties.validate(&error)); QCOMPARE(error, QStringLiteral("未选择任何显示数据")); dialog.findChild<QCheckBox *>(QStringLiteral("HistoryDataShowDate"))->setChecked(false); dialog.findChild<QCheckBox *>(QStringLiteral("HistoryDataShowTime"))->setChecked(false);
    QTimer::singleShot(0, []() { QMessageBox *message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget()); QVERIFY(message != nullptr); QCOMPARE(message->windowTitle(), QStringLiteral("设置错误")); QCOMPARE(message->text(), QStringLiteral("未选择任何显示数据")); message->accept(); }); dialog.findChild<QDialogButtonBox *>(QStringLiteral("HistoryDataButtons"))->button(QDialogButtonBox::Ok)->click(); QCOMPARE(dialog.result(), int(QDialog::Rejected)); dialog.findChild<QCheckBox *>(QStringLiteral("HistoryDataShowDate"))->setChecked(true); dialog.findChild<QDialogButtonBox *>(QStringLiteral("HistoryDataButtons"))->button(QDialogButtonBox::Ok)->click(); QCOMPARE(dialog.result(), int(QDialog::Accepted));

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::historyDataStrictRoundTripAndBackendGates
 * Description     : Round-trip distinct typed tables and reject unknown, fractional, malformed, stale-source and unsupported backend inputs atomically.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::historyDataStrictRoundTripAndBackendGates() //20261113-2 add
{
    HmiProject project = historyDataProject();
    HmiProject loaded;
    HmiProject mixed = historyProject();
    HistoryDataProperties parsed = project.objects.first().historyData;
    QJsonObject json;
    QJsonArray columns;
    QJsonObject column;
    HmiObject object;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    int index;
    project.objects[0].historyData.rows = 0; project.objects[0].historyData.columns[1].shown = false; QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.toJson(), project.toJson()); json = project.objects.first().historyData.toJson(); json.insert(QStringLiteral("rows"), 1.5); parsed.rows = 17; QVERIFY(!HistoryDataProperties::fromJson(json, &parsed, &error)); QCOMPARE(parsed.rows, 17); json = project.objects.first().historyData.toJson(); json.insert(QStringLiteral("unknown"), true); QVERIFY(!HistoryDataProperties::fromJson(json, &parsed, &error)); json.remove(QStringLiteral("unknown")); columns = json.value(QStringLiteral("columns")).toArray(); column = columns[0].toObject(); column.insert(QStringLiteral("shown"), 1); columns[0] = column; json.insert(QStringLiteral("columns"), columns); QVERIFY(!HistoryDataProperties::fromJson(json, &parsed, &error));
    json = project.objects.first().toJson(); json.insert(QStringLiteral("type"), QStringLiteral("text")); QVERIFY(!HmiObject::fromJson(json, &object, &error)); json = project.toJson(); json.insert(QStringLiteral("schemaVersion"), 11); QVERIFY(!HmiProject::fromJson(json, &loaded, &error)); loaded = project; loaded.objects[0].historyData.columns.swapItemsAt(0, 1); QVERIFY(!loaded.validateModel(&error)); loaded = project; loaded.configurationNodes.clear(); QVERIFY(!loaded.validateModel(&error)); QVERIFY(!exportNativeProjectSource(project, &source, &error)); QCOMPARE(source, QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(project, &payloads, &error)); QCOMPARE(payloads, QVector<QByteArray>{QByteArrayLiteral("keep")}); mixed.languages = project.languages; mixed.configurationNodes = project.configurationNodes; for (index = 0; index < 8; ++index) { mixed.objects[0].historyTrend.curves[index].sourceItemId = project.objects.first().historyData.columns.first().sourceItemId; } mixed.objects.append(project.objects.first()); mixed.objects.append(existingFiveObjects()); QVERIFY2(HmiProject::fromJson(mixed.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.objects.size(), 7);
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(project.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("history-data-only.ksp")), &error)); QVERIFY(mixed.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("history-data-mixed.ksp")), &error)); }

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::historyDataCollectorReconciliationAcrossPages
 * Description     : Shrink, grow and rename collector items by stable identity over active and inactive pages with atomic invalid-update refusal.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::historyDataCollectorReconciliationAcrossPages() //20261113-2 add
{
    HmiProject project = historyDataProject();
    HmiProject before;
    HmiPage page;
    HmiConfigurationNode node = project.configurationNodes.first();
    DataCollectionProperties collection;
    QString error;
    QString firstId = project.objects.first().historyData.columns.first().sourceItemId;
    project.objects[0].historyData.columns[0].names[QStringLiteral("English")] = QStringLiteral("Kept"); project.objects[0].historyData.columns[1].shown = false; page.id = QStringLiteral("data-page-2"); page.name = QStringLiteral("Second page"); page.objects = project.objects; page.objects[0].id = QStringLiteral("data-other"); QVERIFY(project.addPage(page, &error)); QVERIFY(DataCollectionProperties::fromJson(node.properties, &collection, &error)); collection.items.removeLast(); collection.sampleLength = 2; collection.items[0].name = QStringLiteral("Renamed source"); node.properties = collection.toJson(); QVERIFY2(project.replaceDataCollection(node, &error), qPrintable(error)); QCOMPARE(project.objects.first().historyData.columns.size(), 2); QCOMPARE(project.inactivePages.first().objects.first().historyData.columns.size(), 2); QCOMPARE(project.objects.first().historyData.columns.first().sourceItemId, firstId); QCOMPARE(project.objects.first().historyData.columns.first().names.value(QStringLiteral("English")), QStringLiteral("Kept")); QVERIFY(!project.objects.first().historyData.columns[1].shown);
    collection.sampleLength = 3; collection.items.append(DataCollectionProperties::createItem(2)); collection.items[2].name = QStringLiteral("New source"); node.properties = collection.toJson(); QVERIFY(project.replaceDataCollection(node, &error)); QCOMPARE(project.inactivePages.first().objects.first().historyData.columns.last().names.value(QStringLiteral("English")), QStringLiteral("New source")); QVERIFY(project.setLanguages({QStringLiteral("English"), QStringLiteral("日本語")}, &error)); QCOMPARE(project.objects.first().historyData.columns.first().names.size(), 2); QCOMPARE(project.inactivePages.first().objects.first().historyData.captions.size(), 2);
    project.objects[0].historyData.showDate = false; project.objects[0].historyData.showTime = false; project.objects[0].historyData.columns[0].shown = false; project.objects[0].historyData.columns[1].shown = false; before = project; collection.sampleLength = 2; collection.items.removeLast(); node.properties = collection.toJson(); QVERIFY(!project.replaceDataCollection(node, &error)); QCOMPARE(project.toJson(), before.toJson());

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::historyDataClipboardAndFullVisibilitySpans
 * Description     : Copy the real collector only into compatible projects, preserve all languages and increment full visibility carriers.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::historyDataClipboardAndFullVisibilitySpans() //20261113-2 add
{
    HmiProject project = historyDataProject();
    HmiProject result;
    HmiProject sentinel;
    HmiProject target;
    Stage3Clipboard clipboard;
    Stage3DuplicateOptions options;
    QString error;
    project.objects[0].historyData.visibility.visibilityEnabled = true; project.objects[0].historyData.visibility.wordEnabled = true; project.objects[0].historyData.visibility.dataType = 1; project.objects[0].historyData.visibility.wordAddress = QStringLiteral("LW4096"); QVERIFY(!project.validateModel(&error)); project.objects[0].historyData.visibility.wordAddress = QStringLiteral("LW4095"); QVERIFY(project.validateModel(&error)); project.objects[0].historyData.visibility.wordAddress = QStringLiteral("LW10"); options.columns = 2; options.increments = {{Stage3AddressField::VisibilityWord, 2, 0}}; QVERIFY2(duplicateStage3Selection(project, {project.objects.first().id}, options, &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.objects.last().historyData.visibility.wordAddress, QStringLiteral("LW12")); QCOMPARE(result.configurationNodes.size(), 1); QVERIFY(captureStage3Selection(project, {project.objects.first().id}, &clipboard, &error)); sentinel = result; QVERIFY(!pasteStage3Selection(target, clipboard, QPoint(10,10), &result, nullptr, &error)); QCOMPARE(result.toJson(), sentinel.toJson()); target.languages = project.languages; QVERIFY2(pasteStage3Selection(target, clipboard, QPoint(10,10), &result, nullptr, &error), qPrintable(error)); QCOMPARE(result.configurationNodes.size(), project.configurationNodes.size()); QCOMPARE(result.configurationNodes.first().toJson(), project.configurationNodes.first().toJson()); target = project; target.configurationNodes[0].name = QStringLiteral("conflict"); sentinel = result; QVERIFY(!pasteStage3Selection(target, clipboard, QPoint(10,10), &result, nullptr, &error)); QCOMPARE(result.toJson(), sentinel.toJson());

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::historyDataRendererGridAndZeroRowClipping
 * Description     : Verify independent grid flags, header-only zero rows and clipping without fabricating historical values.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::historyDataRendererGridAndZeroRowClipping() //20261113-2 add
{
    HistoryDataProperties properties = historyDataProject().objects.first().historyData;
    QImage grid(330, 150, QImage::Format_ARGB32);
    QImage plain = grid;
    QImage zero = grid;
    QPainter painter;
    grid.fill(Qt::magenta); painter.begin(&grid); paintHistoryData(&painter, QRectF(10,10,300,120), properties, QStringLiteral("中文")); painter.end(); QCOMPARE(grid.pixelColor(0,0), QColor(Qt::magenta)); QCOMPARE(grid.pixelColor(70,90), QColor(properties.lineColor)); QCOMPARE(grid.pixelColor(90,50), QColor(properties.lineColor)); properties.verticalLines = false; properties.horizontalLines = false; plain.fill(Qt::magenta); painter.begin(&plain); paintHistoryData(&painter, QRectF(10,10,300,120), properties, QStringLiteral("中文")); painter.end(); QCOMPARE(plain.pixelColor(70,90), QColor(properties.backgroundColor)); QCOMPARE(plain.pixelColor(90,50), QColor(properties.backgroundColor)); properties.rows = 0; zero.fill(Qt::magenta); painter.begin(&zero); paintHistoryData(&painter, QRectF(10,10,300,120), properties, QStringLiteral("中文")); painter.end(); QCOMPARE(zero.pixelColor(20,20), QColor(properties.titleBackground)); QCOMPARE(zero.pixelColor(0,0), QColor(Qt::magenta)); QVERIFY(zero != plain);

}

/******************************************************************************
 * Function Name   : Stage4ControlTests::historyDataShellPlacementCollectionEditAndUndo
 * Description     : Use real menu creation, cancellation, placement and property edits, then reconcile collector shrink and undo the whole transaction.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage4ControlTests::historyDataShellPlacementCollectionEditAndUndo() //20261113-2 add
{
    MainWindow window;
    HmiProject project = historyDataProject();
    auto acceptData = []() { HistoryDataDialog *dialog = qobject_cast<HistoryDataDialog *>(QApplication::activeModalWidget()); QVERIFY(dialog != nullptr); dialog->findChild<QDialogButtonBox *>(QStringLiteral("HistoryDataButtons"))->button(QDialogButtonBox::Ok)->click(); };
    window.setProject(HmiProject(), QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); QVERIFY(window.m_commands.contains(QStringLiteral("action_historyrecord"))); QTimer::singleShot(0, []() { QMessageBox *message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget()); QVERIFY(message != nullptr); QCOMPARE(message->text(), QStringLiteral("没有建立数据采集")); message->accept(); }); window.m_commands.value(QStringLiteral("action_historyrecord"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement()); project.objects.clear(); window.setProject(project, QString()); QTimer::singleShot(0, []() { QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget()); QVERIFY(dialog != nullptr); dialog->reject(); }); window.m_commands.value(QStringLiteral("action_historyrecord"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement()); QTimer::singleShot(0, acceptData); window.m_commands.value(QStringLiteral("action_historyrecord"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(20,20))); QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(QPointF(340,200))); QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(QPointF(340,200))); QCOMPARE(window.currentEditorProject().objects.first().type, QStringLiteral("historydata")); QCOMPARE(window.findChild<QLabel *>(QStringLiteral("NativeSelectionType"))->text(), QStringLiteral("历史数据显示器")); QVERIFY(!window.m_addressEdit->isEnabled()); QVERIFY(!window.m_textEdit->isEnabled());
    QTimer::singleShot(0, []() { HistoryDataDialog *dialog = qobject_cast<HistoryDataDialog *>(QApplication::activeModalWidget()); QVERIFY(dialog != nullptr); dialog->findChild<QSpinBox *>(QStringLiteral("HistoryDataRows"))->setValue(0); dialog->findChild<QDialogButtonBox *>(QStringLiteral("HistoryDataButtons"))->button(QDialogButtonBox::Ok)->click(); }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().historyData.rows, 0); QTimer::singleShot(0, []() { DataCollectionDialog *dialog = qobject_cast<DataCollectionDialog *>(QApplication::activeModalWidget()); QVERIFY(dialog != nullptr); dialog->findChild<QSpinBox *>(QStringLiteral("CollectionLength"))->setValue(2); QMetaObject::invokeMethod(dialog, "accept", Qt::DirectConnection); }); window.configureDataCollection(project.configurationNodes.first().id); QCOMPARE(window.currentEditorProject().objects.first().historyData.columns.size(), 2); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().objects.first().historyData.columns.size(), 3); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().objects.first().historyData.rows, 5); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty());

}
QTEST_MAIN(Stage4ControlTests)
#include "stage4_controls_tests.moc"
