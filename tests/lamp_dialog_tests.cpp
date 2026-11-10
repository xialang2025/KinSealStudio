#include "bit_button_dialog.h"
#include "lamp_properties.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QGroupBox>
#include <QLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTimer>
#include <QtTest>

/******************************************************************************
 * Function Name   : lampObject
 * Description     : Build a detached display-only lamp with stored bit index zero.
 * Input Parameters: existing
 * Return Value    : Lamp fixture.
 * Date            : 2026.11.09
 ******************************************************************************/
static HmiObject lampObject(bool existing = false) //20261109-8 add
{
    HmiObject object;

    object.type = QStringLiteral("switch"); object.geometry = QRect(20, 20, 120, 60);
    object.hasBitButtonProperties = true; object.hasLampProperties = true;
    object.bitButton.monitorSame = false; object.bitButton.bitIndex = 0;
    if (existing) { object.id = QStringLiteral("lamp-existing"); }
    return object;
}

/******************************************************************************
 * Function Name   : saveLampShot
 * Description     : Render optional dialog evidence entirely offscreen.
 * Input Parameters: dialog, name
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
static void saveLampShot(BitButtonDialog *dialog, const QString &name) //20261109-8 add
{
    QString directory;
    QPixmap pixels;

    directory = qEnvironmentVariable("KINSEAL_LAMP_UI_SNAPSHOT_DIR");
    if (directory.isEmpty()) { return; }
    QDir().mkpath(directory); dialog->ensurePolished(); dialog->layout()->activate();
    pixels = QPixmap(dialog->size()); pixels.fill(Qt::white); dialog->render(&pixels);
    pixels.save(directory + QChar(u'/') + name + QStringLiteral(".png"));
}

class LampDialogTests : public QObject
{
    Q_OBJECT
private slots:
    void propertyRoundTripAndBounds();
    void initialPageAndStoredBitQuirk();
    void monitorGatesAndAddressCaches();
    void multiStateAppearanceTransaction();
    void displayOnlyGeneralAndPermissions();
    void notificationsAndVisibility();
    void bitAndWordRemainIndependent();
    void monitorAddressPicker();
    void acceptedValuesReopen();
    void uncheckedMonitorAcceptsAsMulti();
    void sharedLanguageIsGlobalWithIndependentCaptions();
};

/******************************************************************************
 * Function Name   : LampDialogTests::propertyRoundTripAndBounds
 * Description     : Check observed native count limits and lossless additional-state serialization.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void LampDialogTests::propertyRoundTripAndBounds() //20261109-8 add
{
    LampProperties properties;
    LampProperties restored;
    BitButtonProperties shared;
    QJsonObject invalid;
    QString error;

    QCOMPARE(properties.monitorMode, 1); QCOMPARE(properties.multiStateCount, 2);
    QVERIFY(properties.validate(&error)); QCOMPARE(properties.stateCount(), 2);
    properties.monitorMode = 2; properties.multiStateCount = 5;
    QVERIFY(!properties.validate(&error)); properties.ensureStateCount(5, shared);
    QVERIFY(properties.validate(&error)); QCOMPARE(properties.extraStates.size(), 3);
    QCOMPARE(properties.state(shared, 2)->picture, QStringLiteral("switch_ON.kvg"));
    QCOMPARE(properties.state(shared, 2)->background, QColor(QStringLiteral("#a0a0a0")));
    QVERIFY(properties.state(shared, 2)->text.languageTexts.isEmpty());
    QVERIFY(properties.state(shared, -1) == nullptr); QVERIFY(properties.state(shared, 5) == nullptr);
    properties.state(shared, 4)->text.languageTexts.insert(QStringLiteral("中文"), QStringLiteral("状态四"));
    QVERIFY(LampProperties::fromJson(properties.toJson(), &restored, &error)); QCOMPARE(restored.toJson(), properties.toJson());
    invalid = properties.toJson(); invalid.insert(QStringLiteral("multiStateCount"), 33);
    QVERIFY(!LampProperties::fromJson(invalid, &restored, &error)); QCOMPARE(restored.toJson(), properties.toJson());
    properties.ensureStateCount(33, shared); QCOMPARE(properties.extraStates.size(), 3);
    properties.multiStateCount = 32; properties.ensureStateCount(32, shared); QVERIFY(properties.validate(&error));
    QCOMPARE(properties.extraStates.size(), 30); properties.multiStateCount = 2;
    properties.ensureStateCount(2, shared); QVERIFY(properties.validate(&error)); QCOMPARE(properties.extraStates.size(), 0);
    properties.multiDataType = 1; QCOMPARE(properties.transferWordCount(), 2);
    properties.multiDataType = 2; QVERIFY(!properties.validate(&error));
}

/******************************************************************************
 * Function Name   : LampDialogTests::initialPageAndStoredBitQuirk
 * Description     : Reproduce the native new-dialog fifteen versus saved zero distinction.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void LampDialogTests::initialPageAndStoredBitQuirk() //20261109-8 add
{
    HmiProject project;
    BitButtonDialog dialog(lampObject(), project);
    QTabWidget *tabs;
    QSpinBox *bitIndex;
    QGroupBox *container;
    HmiObject draft;

    tabs = dialog.findChild<QTabWidget *>(QStringLiteral("BitButtonTabs"));
    bitIndex = dialog.findChild<QSpinBox *>(QStringLiteral("BitMonitorIndex"));
    container = dialog.findChild<QGroupBox *>(QStringLiteral("LampMonitorContainer"));
    QCOMPARE(dialog.windowTitle(), QStringLiteral("位指示灯")); QCOMPARE(tabs->currentIndex(), 1);
    QVERIFY(container && !container->isCheckable());
    QCOMPARE(bitIndex->value(), 15); QVERIFY(!bitIndex->isEnabled());
    draft = dialog.object(); QCOMPARE(draft.bitButton.bitIndex, 0); draft.id = QStringLiteral("saved");
    BitButtonDialog reopened(draft, project);
    QCOMPARE(reopened.findChild<QSpinBox *>(QStringLiteral("BitMonitorIndex"))->value(), 0);
    saveLampShot(&dialog, QStringLiteral("monitor-default"));
}

/******************************************************************************
 * Function Name   : LampDialogTests::monitorGatesAndAddressCaches
 * Description     : Exercise dual/multi exclusivity, independent address drafts and inactive field gates.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void LampDialogTests::monitorGatesAndAddressCaches() //20261109-8 add
{
    HmiProject project;
    BitButtonDialog dialog(lampObject(), project);
    QGroupBox *dual;
    QGroupBox *multi;
    QComboBox *type;
    QLineEdit *address;
    QSpinBox *bitIndex;
    QComboBox *dataType;
    QSpinBox *count;

    dual = dialog.findChild<QGroupBox *>(QStringLiteral("BitDualMonitor"));
    multi = dialog.findChild<QGroupBox *>(QStringLiteral("LampMultiMonitor"));
    type = dialog.findChild<QComboBox *>(QStringLiteral("BitMonitorType"));
    address = dialog.findChild<QLineEdit *>(QStringLiteral("BitMonitorAddress"));
    bitIndex = dialog.findChild<QSpinBox *>(QStringLiteral("BitMonitorIndex"));
    dataType = dialog.findChild<QComboBox *>(QStringLiteral("LampMultiDataType"));
    count = dialog.findChild<QSpinBox *>(QStringLiteral("LampMultiStateCount"));
    QVERIFY(dual->isChecked() && !multi->isChecked()); QCOMPARE(type->count(), 2);
    QCOMPARE(address->text(), QStringLiteral("LB0")); QVERIFY(address->isEnabled());
    QVERIFY(!dialog.findChild<QCheckBox *>(QStringLiteral("BitMonitorSame"))->isEnabled());
    QVERIFY(!dialog.findChild<QCheckBox *>(QStringLiteral("LampMultiSame"))->isEnabled());
    dual->setChecked(false); QVERIFY(!address->isEnabled()); dual->setChecked(true); QVERIFY(address->isEnabled());
    address->setText(QStringLiteral("LB8")); type->setCurrentIndex(1);
    QCOMPARE(address->text(), QStringLiteral("LW0")); QVERIFY(bitIndex->isEnabled());
    QCOMPARE(dialog.object().bitButton.bitIndex, 15);
    address->setText(QStringLiteral("LW12")); bitIndex->setValue(13); type->setCurrentIndex(0);
    QCOMPARE(address->text(), QStringLiteral("LB8")); QVERIFY(!bitIndex->isEnabled()); type->setCurrentIndex(1);
    QCOMPARE(address->text(), QStringLiteral("LW12")); QCOMPARE(bitIndex->value(), 13);
    multi->setChecked(true); QVERIFY(!dual->isChecked()); QVERIFY(!address->isEnabled());
    QCOMPARE(dialog.windowTitle(), QStringLiteral("多态指示灯"));
    QVERIFY(dataType->isEnabled() && count->isEnabled()); QCOMPARE(dataType->count(), 2);
    QCOMPARE(count->minimum(), 2); QCOMPARE(count->maximum(), 32);
    QVERIFY(!dialog.findChild<QCheckBox *>(QStringLiteral("LampMultiSame"))->isEnabled());
    saveLampShot(&dialog, QStringLiteral("monitor-multi"));
    multi->setChecked(false); QVERIFY(dual->isChecked()); QVERIFY(address->isEnabled());
    QCOMPARE(dialog.windowTitle(), QStringLiteral("位指示灯"));
    multi->setChecked(true); dual->setChecked(true); QVERIFY(!multi->isChecked());
}

/******************************************************************************
 * Function Name   : LampDialogTests::multiStateAppearanceTransaction
 * Description     : Preserve drafts across monitor-mode changes and delete removed states on explicit count reduction.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void LampDialogTests::multiStateAppearanceTransaction() //20261109-8 add
{
    HmiProject project;
    HmiObject original;
    HmiObject draft;
    BitButtonDialog dialog(lampObject(), project);
    QGroupBox *multi;
    QSpinBox *count;
    QSpinBox *state;
    QPlainTextEdit *text;
    QCheckBox *effect;
    QTabWidget *tabs;

    multi = dialog.findChild<QGroupBox *>(QStringLiteral("LampMultiMonitor"));
    count = dialog.findChild<QSpinBox *>(QStringLiteral("LampMultiStateCount"));
    state = dialog.findChild<QSpinBox *>(QStringLiteral("BitState")); text = dialog.findChild<QPlainTextEdit *>(QStringLiteral("BitText"));
    effect = dialog.findChild<QCheckBox *>(QStringLiteral("Bit3D")); tabs = dialog.findChild<QTabWidget *>(QStringLiteral("BitButtonTabs"));
    original = dialog.object(); multi->setChecked(true); count->setValue(5); QCOMPARE(state->maximum(), 4);
    state->setValue(4); QVERIFY(text->toPlainText().isEmpty()); text->setPlainText(QStringLiteral("第四状态"));
    state->setValue(2); QVERIFY(text->toPlainText().isEmpty()); text->setPlainText(QStringLiteral("第二状态"));
    state->setValue(4); QCOMPARE(text->toPlainText(), QStringLiteral("第四状态"));
    effect->setChecked(true); QVERIFY(!dialog.findChild<QPushButton *>(QStringLiteral("BitBackground"))->isEnabled());
    tabs->setCurrentIndex(2); saveLampShot(&dialog, QStringLiteral("appearance-state4"));
    dialog.findChild<QGroupBox *>(QStringLiteral("BitDualMonitor"))->setChecked(true);
    multi->setChecked(true); state->setValue(4); QCOMPARE(text->toPlainText(), QStringLiteral("第四状态"));
    count->setValue(2); QCOMPARE(state->maximum(), 1); count->setValue(5); state->setValue(4);
    QVERIFY(text->toPlainText().isEmpty());
    draft = dialog.object(); QVERIFY(draft.lamp.validate(nullptr)); QCOMPARE(draft.lamp.extraStates.size(), 3);
    QVERIFY(draft.lamp.state(draft.bitButton, 4)->effect3d); QCOMPARE(original.lamp.extraStates.size(), 0);
    dialog.reject(); QCOMPARE(dialog.result(), int(QDialog::Rejected)); QCOMPARE(original.lamp.multiStateCount, 2);
}

/******************************************************************************
 * Function Name   : LampDialogTests::displayOnlyGeneralAndPermissions
 * Description     : Check locked lamp family and the absence of touch-operation controls from visible pages.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void LampDialogTests::displayOnlyGeneralAndPermissions() //20261109-8 add
{
    HmiProject project;
    BitButtonDialog dialog(lampObject(), project);
    QTabWidget *tabs;
    QRadioButton *radio;
    bool found;

    tabs = dialog.findChild<QTabWidget *>(QStringLiteral("BitButtonTabs")); tabs->setCurrentIndex(0);
    QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("BitOperatingAddress"))->parentWidget()->isHidden());
    QVERIFY(dialog.findChild<QComboBox *>(QStringLiteral("BitOperation"))->parentWidget()->isHidden());
    found = false;
    Q_FOREACH (radio, dialog.findChildren<QRadioButton *>())
    {
        if (radio->text() == QStringLiteral("指示灯")) { found = true; QVERIFY(radio->isChecked()); QVERIFY(!radio->isEnabled()); }
        if (radio->text() == QStringLiteral("位按钮")) { radio->click(); QVERIFY(!radio->isChecked()); }
    }
    QVERIFY(found); saveLampShot(&dialog, QStringLiteral("general")); tabs->setCurrentIndex(3);
    QVERIFY(!dialog.findChild<QGroupBox *>(QStringLiteral("BitMacro"))->isEnabled());
    QVERIFY(dialog.findChild<QCheckBox *>(QStringLiteral("BitPressEnabled"))->isHidden());
    QVERIFY(dialog.findChild<QGroupBox *>(QStringLiteral("BitTouch"))->isHidden());
    QVERIFY(!dialog.findChild<QGroupBox *>(QStringLiteral("BitVisibility"))->isHidden());
    saveLampShot(&dialog, QStringLiteral("permissions"));
}

/******************************************************************************
 * Function Name   : LampDialogTests::notificationsAndVisibility
 * Description     : Preserve shared visibility and notification behavior without exposing a touch column.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void LampDialogTests::notificationsAndVisibility() //20261109-8 add
{
    HmiProject project;
    BitButtonDialog dialog(lampObject(), project);
    QGroupBox *notice;
    QCheckBox *noticeBit;
    QCheckBox *noticeWord;
    QLineEdit *address;
    HmiObject draft;

    notice = dialog.findChild<QGroupBox *>(QStringLiteral("BitNotice"));
    noticeBit = dialog.findChild<QCheckBox *>(QStringLiteral("BitNoticeBit"));
    noticeWord = dialog.findChild<QCheckBox *>(QStringLiteral("BitNoticeWord"));
    address = dialog.findChild<QLineEdit *>(QStringLiteral("BitNoticeBitAddress"));
    QVERIFY(!notice->isEnabled()); QVERIFY(!noticeBit->isEnabled()); QVERIFY(!noticeWord->isEnabled()); QVERIFY(address->isHidden());
    draft = dialog.object(); QVERIFY(!draft.bitButton.notice && !draft.bitButton.noticeWord);
    QVERIFY(!draft.bitButton.touch.visibilityEnabled && !draft.bitButton.pressTimeEnabled);
}

/******************************************************************************
 * Function Name   : LampDialogTests::bitAndWordRemainIndependent
 * Description     : Guard family-specific pages and default state gates after adding lamps.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void LampDialogTests::bitAndWordRemainIndependent() //20261109-8 add
{
    HmiProject project;
    HmiObject bit;
    HmiObject word;

    bit.type = QStringLiteral("switch"); bit.address = QStringLiteral("LB0"); bit.hasBitButtonProperties = true;
    BitButtonDialog bitDialog(bit, project);
    QCOMPARE(bitDialog.windowTitle(), QStringLiteral("位按钮"));
    QVERIFY(bitDialog.findChild<QCheckBox *>(QStringLiteral("BitIndicator")) != nullptr);
    QVERIFY(bitDialog.findChild<QGroupBox *>(QStringLiteral("LampMultiMonitor")) == nullptr);
    word = bit; word.hasWordButtonProperties = true; word.address = QStringLiteral("LW0");
    BitButtonDialog wordDialog(word, project);
    QCOMPARE(wordDialog.windowTitle(), QStringLiteral("字按钮"));
    QCOMPARE(wordDialog.findChild<QSpinBox *>(QStringLiteral("BitState"))->maximum(), 0);
    QVERIFY(wordDialog.findChild<QCheckBox *>(QStringLiteral("WordPressedEffect")) != nullptr);
    QVERIFY(wordDialog.findChild<QGroupBox *>(QStringLiteral("LampMultiMonitor")) == nullptr);
}

/******************************************************************************
 * Function Name   : LampDialogTests::monitorAddressPicker
 * Description     : Open the shared qualified-address picker from each lamp monitoring role.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void LampDialogTests::monitorAddressPicker() //20261109-8 add
{
    HmiProject project;
    BitButtonDialog dialog(lampObject(), project);
    bool found;

    found = false;
    QTimer::singleShot(0, [&found]() {
        QDialog *picker;
        picker = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (picker != nullptr) { found = true; picker->reject(); }
    });
    dialog.findChild<QPushButton *>(QStringLiteral("BitMonitorPicker"))->click(); QVERIFY(found);
    dialog.findChild<QGroupBox *>(QStringLiteral("LampMultiMonitor"))->setChecked(true);
    dialog.findChild<QComboBox *>(QStringLiteral("LampMultiDataType"))->setCurrentIndex(1);
    found = false;
    QTimer::singleShot(0, [&found]() {
        QDialog *picker;
        picker = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (picker != nullptr) { found = true; picker->reject(); }
    });
    dialog.findChild<QPushButton *>(QStringLiteral("LampMultiPicker"))->click(); QVERIFY(found);
}

/******************************************************************************
 * Function Name   : LampDialogTests::acceptedValuesReopen
 * Description     : Accept a multi-state word monitor and recover its selected fields and appearances.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void LampDialogTests::acceptedValuesReopen() //20261109-8 add
{
    HmiProject project;
    BitButtonDialog dialog(lampObject(), project);
    HmiObject draft;
    QDialogButtonBox *buttons;

    dialog.findChild<QGroupBox *>(QStringLiteral("LampMultiMonitor"))->setChecked(true);
    dialog.findChild<QSpinBox *>(QStringLiteral("LampMultiStateCount"))->setValue(5);
    dialog.findChild<QComboBox *>(QStringLiteral("LampMultiDataType"))->setCurrentIndex(1);
    dialog.findChild<QLineEdit *>(QStringLiteral("LampMultiAddress"))->setText(QStringLiteral("LW20"));
    dialog.findChild<QSpinBox *>(QStringLiteral("BitState"))->setValue(3);
    dialog.findChild<QPlainTextEdit *>(QStringLiteral("BitText"))->setPlainText(QStringLiteral("第三状态"));
    buttons = dialog.findChild<QDialogButtonBox *>(); buttons->button(QDialogButtonBox::Ok)->click();
    QCOMPARE(dialog.result(), int(QDialog::Accepted)); draft = dialog.object(); draft.id = QStringLiteral("saved");
    QCOMPARE(draft.lamp.monitorMode, 2); QCOMPARE(draft.lamp.multiStateCount, 5); QCOMPARE(draft.lamp.multiDataType, 1);
    QCOMPARE(draft.lamp.multiMonitorAddress, QStringLiteral("LW20")); QVERIFY(draft.address.isEmpty());
    BitButtonDialog reopened(draft, project);
    QCOMPARE(reopened.findChild<QTabWidget *>(QStringLiteral("BitButtonTabs"))->currentIndex(), 1);
    QCOMPARE(reopened.findChild<QSpinBox *>(QStringLiteral("BitState"))->maximum(), 4);
    reopened.findChild<QSpinBox *>(QStringLiteral("BitState"))->setValue(3);
    QCOMPARE(reopened.findChild<QPlainTextEdit *>(QStringLiteral("BitText"))->toPlainText(), QStringLiteral("第三状态"));
}

/******************************************************************************
 * Function Name   : LampDialogTests::uncheckedMonitorAcceptsAsMulti
 * Description     : Preserve the visible unchecked draft while normalizing acceptance as native multi monitoring.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void LampDialogTests::uncheckedMonitorAcceptsAsMulti() //20261109-8 add
{
    HmiProject project;
    BitButtonDialog dialog(lampObject(), project);
    HmiObject draft;
    QDialogButtonBox *buttons;

    dialog.findChild<QGroupBox *>(QStringLiteral("BitDualMonitor"))->setChecked(false);
    QVERIFY(!dialog.findChild<QGroupBox *>(QStringLiteral("LampMultiMonitor"))->isChecked());
    buttons = dialog.findChild<QDialogButtonBox *>(); buttons->button(QDialogButtonBox::Ok)->click();
    QCOMPARE(dialog.result(), int(QDialog::Accepted)); draft = dialog.object();
    QCOMPARE(draft.lamp.monitorMode, 2); QCOMPARE(draft.lamp.multiStateCount, 2);
    QVERIFY(draft.bitButton.indicator && !draft.bitButton.dualMonitor);
    QCOMPARE(draft.lamp.multiMonitorAddress, QStringLiteral("LW0"));
}

/******************************************************************************
 * Function Name   : LampDialogTests::sharedLanguageIsGlobalWithIndependentCaptions
 * Description     : Share the lamp-wide language flag without copying captions between independent states.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void LampDialogTests::sharedLanguageIsGlobalWithIndependentCaptions() //20261109-8 add
{
    HmiProject project;
    HmiObject object;
    HmiObject draft;
    QSpinBox *state;
    QPlainTextEdit *text;
    QCheckBox *shared;
    QComboBox *language;
    int index;

    project.languages = {QStringLiteral("中文"), QStringLiteral("English")};
    project.activeLanguage = QStringLiteral("中文");
    object = lampObject(true); object.lamp.monitorMode = 2; object.lamp.multiStateCount = 5;
    object.lamp.ensureStateCount(5, object.bitButton);
    object.bitButton.states[0].text.languageTexts.insert(QStringLiteral("中文"), QStringLiteral("零"));
    object.bitButton.states[0].text.languageTexts.insert(QStringLiteral("English"), QStringLiteral("zero"));
    object.bitButton.states[1].text.languageTexts.insert(QStringLiteral("中文"), QStringLiteral("一"));
    object.bitButton.states[1].text.languageTexts.insert(QStringLiteral("English"), QStringLiteral("one"));
    object.lamp.state(object.bitButton, 4)->text.languageTexts.insert(QStringLiteral("中文"), QStringLiteral("四"));
    object.lamp.state(object.bitButton, 4)->text.languageTexts.insert(QStringLiteral("English"), QStringLiteral("four"));
    BitButtonDialog dialog(object, project);
    state = dialog.findChild<QSpinBox *>(QStringLiteral("BitState"));
    text = dialog.findChild<QPlainTextEdit *>(QStringLiteral("BitText"));
    shared = dialog.findChild<QCheckBox *>(QStringLiteral("BitSameLanguage"));
    language = dialog.findChild<QComboBox *>(QStringLiteral("BitLanguage"));
    shared->setChecked(true); draft = dialog.object();
    for (index = 0; index < 5; ++index) { QVERIFY(draft.lamp.state(draft.bitButton, index)->sameLanguage); }
    QCOMPARE(draft.bitButton.states[0].sharedText, QStringLiteral("零"));
    QCOMPARE(draft.bitButton.states[1].sharedText, QStringLiteral("一"));
    QCOMPARE(draft.lamp.state(draft.bitButton, 4)->sharedText, QStringLiteral("四"));
    QVERIFY(draft.lamp.state(draft.bitButton, 2)->sharedText.isEmpty());
    language->setCurrentText(QStringLiteral("English")); state->setValue(4);
    QVERIFY(shared->isChecked()); QCOMPARE(text->toPlainText(), QStringLiteral("四"));
    text->setPlainText(QStringLiteral("shared four"));
    dialog.findChild<QSpinBox *>(QStringLiteral("LampMultiStateCount"))->setValue(6);
    state->setValue(5); QVERIFY(shared->isChecked()); QVERIFY(text->toPlainText().isEmpty());
    state->setValue(4); QCOMPARE(text->toPlainText(), QStringLiteral("shared four"));
    shared->setChecked(false); QCOMPARE(text->toPlainText(), QStringLiteral("four"));
    state->setValue(0); QVERIFY(!shared->isChecked()); QCOMPARE(text->toPlainText(), QStringLiteral("zero"));
    language->setCurrentText(QStringLiteral("中文")); QCOMPARE(text->toPlainText(), QStringLiteral("零"));
    state->setValue(4); QCOMPARE(text->toPlainText(), QStringLiteral("四"));
    draft = dialog.object();
    for (index = 0; index < 6; ++index) { QVERIFY(!draft.lamp.state(draft.bitButton, index)->sameLanguage); }
}

QTEST_MAIN(LampDialogTests)
#include "lamp_dialog_tests.moc"
