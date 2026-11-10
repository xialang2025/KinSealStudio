#include "bit_button_dialog.h"
#include "function_button_properties.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QEvent>
#include <QEventLoop>
#include <QGroupBox>
#include <QLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QScopedPointer>
#include <QSpinBox>
#include <QTabWidget>
#include <QtTest>

/******************************************************************************
 * Function Name   : functionObject
 * Description     : Build an address-free system-function fixture with the shared single appearance.
 * Input Parameters: None
 * Return Value    : Detached function-button object.
 * Date            : 2026.11.10
 ******************************************************************************/
static HmiObject functionObject() //20261110-0 add
{
    HmiObject object;

    object.id = QStringLiteral("function-test"); object.name = QStringLiteral("功能按钮1");
    object.type = QStringLiteral("switch"); object.geometry = QRect(20, 20, 120, 60);
    object.hasBitButtonProperties = true; object.hasFunctionButtonProperties = true;
    object.bitButton.indicator = false; object.bitButton.dualMonitor = false;
    return object;
}

/******************************************************************************
 * Function Name   : saveFunctionShot
 * Description     : Render optional UI evidence without showing a desktop window or taking focus.
 * Input Parameters: dialog, name
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
static void saveFunctionShot(BitButtonDialog *dialog, const QString &name) //20261110-0 add
{
    QString directory;
    QPixmap pixels;

    dialog->ensurePolished(); dialog->layout()->activate();
    pixels = QPixmap(dialog->size()); pixels.fill(Qt::white);
    dialog->render(&pixels); // The first hidden render polishes child widgets and queues their final size-hint updates.
    // Deliver the nested layout request before recording pixels after the keyboard replaces the type row.
    QApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    dialog->layout()->activate();
    pixels.fill(Qt::white); dialog->render(&pixels); // Complete hidden layout even when evidence files are not requested.
    directory = qEnvironmentVariable("KINSEAL_FUNCTION_UI_SNAPSHOT_DIR");
    if (directory.isEmpty()) { return; }
    QDir().mkpath(directory);
    pixels.save(directory + QChar(u'/') + name + QStringLiteral(".png"));
}

class FunctionButtonDialogTests : public QObject
{
    Q_OBJECT
private slots:
    void propertiesRoundTripAndGuards();
    void projectSchemaAndAddressFreeRecord();
    void generalDefaultsAndFamilyLock();
    void functionMatrix_data();
    void functionMatrix();
    void typesResetAndProjectLanguages();
    void keyboardMatrix_data();
    void keyboardMatrix();
    void keyboardTextBoundaries();
    void keyboardLayoutRemainsUnclipped();
    void singleStateAndAppearance();
    void permissionAndNotificationGates();
    void acceptedValuesReopenAndCancel();
    void otherButtonFamiliesRemainIndependent();
};

/******************************************************************************
 * Function Name   : FunctionButtonDialogTests::propertiesRoundTripAndGuards
 * Description     : Check transactional persistence and reject malformed function/type/key fields.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void FunctionButtonDialogTests::propertiesRoundTripAndGuards() //20261110-0 add
{
    FunctionButtonProperties properties;
    FunctionButtonProperties restored;
    QJsonObject json;
    QString error;

    QCOMPARE(properties.function, 0); QCOMPARE(properties.type, 0); QCOMPARE(properties.keyboardKey, 0);
    QVERIFY(properties.keyboardText.isEmpty()); QVERIFY(!properties.pressedEffect); QVERIFY(properties.validate(&error));
    QCOMPARE(FunctionButtonProperties::functions().size(), 18); QCOMPARE(FunctionButtonProperties::keyboardKeys().size(), 10);
    properties.function = 14; properties.keyboardKey = 9; properties.keyboardText = QStringLiteral("A"); properties.pressedEffect = true;
    QVERIFY(FunctionButtonProperties::fromJson(properties.toJson(), &restored, &error)); QCOMPARE(restored.toJson(), properties.toJson());
    json = properties.toJson(); json.insert(QStringLiteral("function"), 18);
    QVERIFY(!FunctionButtonProperties::fromJson(json, &restored, &error)); QCOMPARE(restored.toJson(), properties.toJson());
    json = properties.toJson(); json.insert(QStringLiteral("keyboardKey"), -1); QVERIFY(!FunctionButtonProperties::fromJson(json, &restored, &error));
    json = properties.toJson(); json.insert(QStringLiteral("function"), 0); json.insert(QStringLiteral("type"), 1);
    QVERIFY(!FunctionButtonProperties::fromJson(json, &restored, &error));
    json = properties.toJson(); json.insert(QStringLiteral("function"), 10); json.insert(QStringLiteral("type"), 7);
    QVERIFY(!FunctionButtonProperties::fromJson(json, &restored, &error));
    json = properties.toJson(); json.insert(QStringLiteral("function"), 1.5); QVERIFY(!FunctionButtonProperties::fromJson(json, &restored, &error));
    json = properties.toJson(); json.insert(QStringLiteral("keyboardText"), 65); QVERIFY(!FunctionButtonProperties::fromJson(json, &restored, &error));
    json = properties.toJson(); json.insert(QStringLiteral("pressedEffect"), 1); QVERIFY(!FunctionButtonProperties::fromJson(json, &restored, &error));
    json = properties.toJson(); json.insert(QStringLiteral("version"), 2); QVERIFY(!FunctionButtonProperties::fromJson(json, &restored, &error));
    json = properties.toJson(); json.remove(QStringLiteral("version")); QVERIFY(!FunctionButtonProperties::fromJson(json, &restored, &error));
    json = properties.toJson(); json.insert(QStringLiteral("version"), 1.5); QVERIFY(!FunctionButtonProperties::fromJson(json, &restored, &error));
    properties.keyboardText = QStringLiteral("中"); QVERIFY(properties.validate(&error));
    properties.keyboardText = QStringLiteral("Ω"); QVERIFY(properties.validate(&error));
    properties.keyboardText.clear(); QVERIFY(properties.validate(&error));
    properties.keyboardText = QStringLiteral("AB"); QVERIFY(!properties.validate(&error));
    properties.keyboardText = QString(1, QChar(char16_t(0xd83d))); QVERIFY(!properties.validate(&error));
}

/******************************************************************************
 * Function Name   : FunctionButtonDialogTests::projectSchemaAndAddressFreeRecord
 * Description     : Round-trip schema eight and reject older schemas that would discard system functions.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void FunctionButtonDialogTests::projectSchemaAndAddressFreeRecord() //20261110-0 add
{
    HmiProject project;
    HmiProject restored;
    HmiObject object;
    QJsonObject json;
    QString error;

    object = functionObject(); project.objects.append(object);
    QVERIFY(project.validatePlcConfiguration(&error)); json = project.toJson();
    QCOMPARE(json.value(QStringLiteral("schemaVersion")).toInt(), 8);
    QVERIFY(HmiProject::fromJson(json, &restored, &error)); QVERIFY(restored.objects.first().hasFunctionButtonProperties);
    QVERIFY(restored.objects.first().address.isEmpty()); QCOMPARE(restored.objects.first().functionButton.toJson(), object.functionButton.toJson());
    json.insert(QStringLiteral("schemaVersion"), 7); QVERIFY(!HmiProject::fromJson(json, &restored, &error));
    project.objects[0].bitButton.pressTime = 0; json = project.toJson();
    QVERIFY(HmiProject::fromJson(json, &restored, &error)); QCOMPARE(restored.objects.first().bitButton.pressTime, 0);
    project.objects[0].bitButton.pressTime = 255;
    object.functionButton.function = 13; object.functionButton.type = 1;
    project.objects[0] = object; QVERIFY(!project.validatePlcConfiguration(&error));
    project.languages.append(QStringLiteral("English")); QVERIFY(project.validatePlcConfiguration(&error));
}

/******************************************************************************
 * Function Name   : FunctionButtonDialogTests::generalDefaultsAndFamilyLock
 * Description     : Check native family locking, four tabs and the lack of operating-address controls.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void FunctionButtonDialogTests::generalDefaultsAndFamilyLock() //20261110-0 add
{
    HmiProject project;
    BitButtonDialog dialog(functionObject(), project);
    QComboBox *function;
    QComboBox *type;
    QTabWidget *tabs;
    QRadioButton *radio;
    HmiObject draft;
    bool found;

    function = dialog.findChild<QComboBox *>(QStringLiteral("FunctionSelection"));
    type = dialog.findChild<QComboBox *>(QStringLiteral("FunctionType")); tabs = dialog.findChild<QTabWidget *>(QStringLiteral("BitButtonTabs"));
    QCOMPARE(dialog.windowTitle(), QStringLiteral("功能按钮")); QCOMPARE(tabs->count(), 4); QCOMPARE(tabs->currentIndex(), 0);
    QCOMPARE(function->count(), 18); QCOMPARE(function->currentIndex(), 0); QCOMPARE(type->count(), 0); QVERIFY(type->isEnabled());
    QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("BitOperatingAddress")) == nullptr);
    QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("WordAddress")) == nullptr);
    QVERIFY(!dialog.findChild<QGroupBox *>(QStringLiteral("BitMacro"))->isEnabled());
    QVERIFY(tabs->widget(1)->findChildren<QWidget *>().isEmpty());
    found = false;
    Q_FOREACH (radio, tabs->widget(0)->findChildren<QRadioButton *>())
    {
        if (radio->text() == QStringLiteral("功能按钮")) { found = true; QVERIFY(radio->isChecked()); QVERIFY(!radio->isEnabled()); }
        if (radio->text() == QStringLiteral("位按钮")) { radio->click(); QVERIFY(!radio->isChecked()); }
    }
    QVERIFY(found); draft = dialog.object(); QVERIFY(draft.address.isEmpty()); QVERIFY(!draft.bitButton.indicator && !draft.bitButton.dualMonitor);
    saveFunctionShot(&dialog, QStringLiteral("general-default"));
}

/******************************************************************************
 * Function Name   : FunctionButtonDialogTests::functionMatrix_data
 * Description     : Cover every native function and its observed number of type choices.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void FunctionButtonDialogTests::functionMatrix_data() //20261110-0 add
{
    static const int counts[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 13, 14, 1, 0, 0, 0, 2};
    QByteArray name;
    int function;

    QTest::addColumn<int>("function"); QTest::addColumn<int>("count");
    for (function = 0; function < 18; ++function)
    {
        name = QByteArray("function-") + QByteArray::number(function); QTest::newRow(name.constData()) << function << counts[function];
    }
}

/******************************************************************************
 * Function Name   : FunctionButtonDialogTests::functionMatrix
 * Description     : Verify type ordering, keyboard replacement and valid selected-function drafts.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void FunctionButtonDialogTests::functionMatrix() //20261110-0 add
{
    QFETCH(int, function);
    QFETCH(int, count);
    HmiProject project;
    BitButtonDialog dialog(functionObject(), project);
    QComboBox *selection;
    QComboBox *type;
    QStringList expected;
    HmiObject draft;
    int index;

    selection = dialog.findChild<QComboBox *>(QStringLiteral("FunctionSelection")); type = dialog.findChild<QComboBox *>(QStringLiteral("FunctionType"));
    selection->setCurrentIndex(function); QCOMPARE(type->count(), count);
    expected = FunctionButtonProperties::types(function, project.languages);
    for (index = 0; index < count; ++index) { QCOMPARE(type->itemText(index), expected.at(index)); }
    QVERIFY(dialog.findChild<QWidget *>(QStringLiteral("FunctionKeyboard"))->isHidden() == (function != 14));
    draft = dialog.object(); QCOMPARE(draft.functionButton.function, function); QCOMPARE(draft.functionButton.type, 0);
    QVERIFY(draft.functionButton.validate(nullptr)); QVERIFY(draft.address.isEmpty());
}

/******************************************************************************
 * Function Name   : FunctionButtonDialogTests::typesResetAndProjectLanguages
 * Description     : Reset the dependent type on function changes and preserve all project language choices.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void FunctionButtonDialogTests::typesResetAndProjectLanguages() //20261110-0 add
{
    HmiProject project;
    QScopedPointer<BitButtonDialog> dialog;
    QComboBox *function;
    QComboBox *type;

    project.languages = {QStringLiteral("中文"), QStringLiteral("English"), QStringLiteral("繁體中文")};
    dialog.reset(new BitButtonDialog(functionObject(), project));
    function = dialog->findChild<QComboBox *>(QStringLiteral("FunctionSelection")); type = dialog->findChild<QComboBox *>(QStringLiteral("FunctionType"));
    function->setCurrentIndex(11); type->setCurrentIndex(12); QCOMPARE(dialog->object().functionButton.type, 12);
    function->setCurrentIndex(12); QCOMPARE(type->currentIndex(), 0); type->setCurrentIndex(13);
    function->setCurrentIndex(11); QCOMPARE(type->currentIndex(), 0);
    function->setCurrentIndex(0); QCOMPARE(type->currentIndex(), -1); QCOMPARE(dialog->object().functionButton.type, 0);
    function->setCurrentIndex(13); QCOMPARE(type->count(), 3); QCOMPARE(type->itemText(1), QStringLiteral("English"));
    type->setCurrentIndex(2); QCOMPARE(dialog->object().functionButton.type, 2); saveFunctionShot(dialog.data(), QStringLiteral("general-languages"));
}

/******************************************************************************
 * Function Name   : FunctionButtonDialogTests::keyboardMatrix_data
 * Description     : Cover all ten native keyboard radio choices.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void FunctionButtonDialogTests::keyboardMatrix_data() //20261110-0 add
{
    QByteArray name;
    int key;

    QTest::addColumn<int>("key");
    for (key = 0; key < 10; ++key) { name = QByteArray("key-") + QByteArray::number(key); QTest::newRow(name.constData()) << key; }
}

/******************************************************************************
 * Function Name   : FunctionButtonDialogTests::keyboardMatrix
 * Description     : Select real radio buttons while preserving the always-editable native character draft.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void FunctionButtonDialogTests::keyboardMatrix() //20261110-0 add
{
    QFETCH(int, key);
    HmiProject project;
    BitButtonDialog dialog(functionObject(), project);
    QRadioButton *radio;
    QLineEdit *text;
    HmiObject draft;

    dialog.findChild<QComboBox *>(QStringLiteral("FunctionSelection"))->setCurrentIndex(14);
    radio = dialog.findChild<QRadioButton *>(QStringLiteral("FunctionKey%1").arg(key));
    text = dialog.findChild<QLineEdit *>(QStringLiteral("FunctionKeyboardText"));
    QVERIFY(radio != nullptr); radio->click(); QVERIFY(radio->isChecked()); QVERIFY(text->isEnabled()); QCOMPARE(text->maxLength(), 1);
    if (key == 9) { text->setText(QStringLiteral("A")); saveFunctionShot(&dialog, QStringLiteral("general-keyboard")); }
    draft = dialog.object(); QCOMPARE(draft.functionButton.keyboardKey, key);
    QCOMPARE(draft.functionButton.keyboardText, key == 9 ? QStringLiteral("A") : QString());
}

/******************************************************************************
 * Function Name   : FunctionButtonDialogTests::keyboardTextBoundaries
 * Description     : Match the native one-unit edit limit and reject an active truncated surrogate without replacement.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void FunctionButtonDialogTests::keyboardTextBoundaries() //20261110-0 add
{
    HmiProject project;
    BitButtonDialog dialog(functionObject(), project);
    QLineEdit *text;
    HmiObject draft;

    dialog.findChild<QComboBox *>(QStringLiteral("FunctionSelection"))->setCurrentIndex(14);
    dialog.findChild<QRadioButton *>(QStringLiteral("FunctionKey9"))->click();
    text = dialog.findChild<QLineEdit *>(QStringLiteral("FunctionKeyboardText"));
    text->setText(QStringLiteral("AB")); QCOMPARE(text->text(), QStringLiteral("A"));
    text->setText(QStringLiteral("中")); QCOMPARE(dialog.object().functionButton.keyboardText, QStringLiteral("中"));
    text->setText(QStringLiteral("Ω")); QCOMPARE(dialog.object().functionButton.keyboardText, QStringLiteral("Ω"));
    text->clear(); QVERIFY(dialog.object().functionButton.validate(nullptr));
    text->setText(QStringLiteral("😀")); draft = dialog.object();
    QCOMPARE(draft.functionButton.keyboardText.size(), 1); QVERIFY(draft.functionButton.keyboardText.at(0).isHighSurrogate());
    QVERIFY(!draft.functionButton.validate(nullptr));
}

/******************************************************************************
 * Function Name   : FunctionButtonDialogTests::keyboardLayoutRemainsUnclipped
 * Description     : Check all eleven keyboard controls after hidden rendering and on an actual visible Qt dialog.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void FunctionButtonDialogTests::keyboardLayoutRemainsUnclipped() //20261110-0 add
{
    HmiProject project;
    BitButtonDialog dialog(functionObject(), project);
    QWidget *frame;
    QWidget *control;
    QWidget *other;
    QStringList names;
    QString name;
    QByteArray diagnostic;
    QSize minimum;
    int index;
    int otherIndex;
    int pass;

    dialog.findChild<QComboBox *>(QStringLiteral("FunctionSelection"))->setCurrentIndex(14);
    dialog.findChild<QRadioButton *>(QStringLiteral("FunctionKey9"))->click();
    dialog.findChild<QLineEdit *>(QStringLiteral("FunctionKeyboardText"))->setText(QStringLiteral("A"));
    frame = dialog.findChild<QWidget *>(QStringLiteral("FunctionKeyboard")); QVERIFY(frame != nullptr);
    for (index = 0; index < 10; ++index) { names.append(QStringLiteral("FunctionKey%1").arg(index)); }
    names.append(QStringLiteral("FunctionKeyboardText"));
    for (pass = 0; pass < 2; ++pass)
    {
        if (pass == 0) { saveFunctionShot(&dialog, QStringLiteral("general-keyboard")); }
        else
        {
            dialog.show(); QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            QVERIFY(dialog.isVisible());
        }
        QVERIFY(frame->height() >= frame->minimumSizeHint().height());
        for (index = 0; index < names.size(); ++index)
        {
            name = names.at(index); control = frame->findChild<QWidget *>(name); QVERIFY(control != nullptr);
            minimum = control->minimumSizeHint(); diagnostic = QStringLiteral("%1: actual %2x%3; minimum %4x%5")
                .arg(name).arg(control->width()).arg(control->height()).arg(minimum.width()).arg(minimum.height()).toUtf8();
            QVERIFY2(control->width() >= minimum.width() && control->height() >= minimum.height(), diagnostic.constData());
            QVERIFY2(frame->rect().contains(control->geometry()), qPrintable(name));
            if (pass == 1)
            {
                QVERIFY2(control->isVisibleTo(&dialog), qPrintable(name));
                QVERIFY2(control->visibleRegion().contains(control->rect()), qPrintable(name));
            }
            for (otherIndex = index + 1; otherIndex < names.size(); ++otherIndex)
            {
                other = frame->findChild<QWidget *>(names.at(otherIndex)); QVERIFY(other != nullptr);
                QVERIFY2(!control->geometry().intersects(other->geometry()), qPrintable(name));
            }
        }
    }
}

/******************************************************************************
 * Function Name   : FunctionButtonDialogTests::singleStateAndAppearance
 * Description     : Keep appearance state zero fixed while retaining independent pressed, flash and language flags.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void FunctionButtonDialogTests::singleStateAndAppearance() //20261110-0 add
{
    HmiProject project;
    BitButtonDialog dialog(functionObject(), project);
    QSpinBox *state;
    QCheckBox *pressed;
    QPlainTextEdit *text;
    HmiObject draft;

    state = dialog.findChild<QSpinBox *>(QStringLiteral("BitState")); pressed = dialog.findChild<QCheckBox *>(QStringLiteral("FunctionPressedEffect"));
    text = dialog.findChild<QPlainTextEdit *>(QStringLiteral("BitText")); QCOMPARE(state->value(), 0); QVERIFY(!state->isEnabled());
    QCOMPARE(text->toPlainText(), QStringLiteral("OFF")); QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("BitPicture"))->text(), QStringLiteral("switch_OFF.kvg"));
    QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("BitSize"))->minimum(), 1); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("BitSize"))->maximum(), 128);
    QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("BitOpacity"))->value(), 255); pressed->click(); QVERIFY(pressed->isChecked());
    QVERIFY(!dialog.findChild<QPushButton *>(QStringLiteral("BitBackground"))->isEnabled());
    QVERIFY(!dialog.findChild<QSpinBox *>(QStringLiteral("BitOpacity"))->isEnabled());
    QCOMPARE(state->maximum(), 0); state->setValue(1); QCOMPARE(state->value(), 0);
    QVERIFY(!dialog.findChild<QPushButton *>(QStringLiteral("BitCopyText"))->isEnabled());
    QVERIFY(!dialog.findChild<QPushButton *>(QStringLiteral("BitCopyShape"))->isEnabled());
    dialog.findChild<QCheckBox *>(QStringLiteral("BitFlash"))->click(); text->setPlainText(QStringLiteral("重启"));
    dialog.findChild<QCheckBox *>(QStringLiteral("BitSameLanguage"))->click(); dialog.findChild<QCheckBox *>(QStringLiteral("Bit3D"))->click();
    dialog.findChild<QCheckBox *>(QStringLiteral("BitTransparent"))->click();
    QVERIFY(!dialog.findChild<QPushButton *>(QStringLiteral("BitBackground"))->isEnabled());
    QVERIFY(!dialog.findChild<QSpinBox *>(QStringLiteral("BitOpacity"))->isEnabled());
    dialog.findChild<QPushButton *>(QStringLiteral("BitNoImage"))->click();
    QVERIFY(dialog.findChild<QPushButton *>(QStringLiteral("BitBackground"))->isEnabled());
    QVERIFY(dialog.findChild<QSpinBox *>(QStringLiteral("BitOpacity"))->isEnabled());
    dialog.findChild<QSpinBox *>(QStringLiteral("BitOpacity"))->setValue(128);
    draft = dialog.object(); QVERIFY(draft.functionButton.pressedEffect && draft.bitButton.states[0].flash);
    QVERIFY(draft.bitButton.states[0].sameLanguage && draft.bitButton.states[0].effect3d && draft.bitButton.states[0].transparent);
    QVERIFY(draft.bitButton.states[1].sameLanguage && draft.bitButton.states[1].effect3d && draft.bitButton.states[1].transparent);
    QCOMPARE(draft.bitButton.states[1].sharedText, QStringLiteral("ON"));
    QCOMPARE(draft.bitButton.states[0].sharedText, QStringLiteral("重启")); QCOMPARE(draft.bitButton.states[0].opacity, 128);
    QVERIFY(!draft.bitButton.states[0].useImage); QCOMPARE(draft.bitButton.states[0].picture, QStringLiteral("switch_OFF.kvg"));
    QCOMPARE(draft.bitButton.states[1].text.languageTexts.value(QStringLiteral("中文")), QStringLiteral("ON"));
    dialog.findChild<QTabWidget *>(QStringLiteral("BitButtonTabs"))->setCurrentIndex(2); saveFunctionShot(&dialog, QStringLiteral("appearance"));
}

/******************************************************************************
 * Function Name   : FunctionButtonDialogTests::permissionAndNotificationGates
 * Description     : Exercise both permission columns, symmetric notification exclusivity and hold-time visibility.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void FunctionButtonDialogTests::permissionAndNotificationGates() //20261110-0 add
{
    HmiProject project;
    BitButtonDialog dialog(functionObject(), project);
    QGroupBox *notice;
    QCheckBox *bit;
    QCheckBox *word;
    QLineEdit *bitAddress;
    QLineEdit *wordAddress;
    QCheckBox *press;
    HmiObject draft;

    notice = dialog.findChild<QGroupBox *>(QStringLiteral("BitNotice")); bit = dialog.findChild<QCheckBox *>(QStringLiteral("BitNoticeBit"));
    word = dialog.findChild<QCheckBox *>(QStringLiteral("BitNoticeWord")); bitAddress = dialog.findChild<QLineEdit *>(QStringLiteral("BitNoticeBitAddress"));
    wordAddress = dialog.findChild<QLineEdit *>(QStringLiteral("BitNoticeWordAddress")); press = dialog.findChild<QCheckBox *>(QStringLiteral("BitPressEnabled"));
    QVERIFY(!bit->isEnabled() && !word->isEnabled()); QVERIFY(bitAddress->isHidden() && wordAddress->isHidden());
    notice->setChecked(true); QVERIFY(bit->isEnabled() && word->isEnabled()); bit->click(); QVERIFY(bit->isChecked());
    QVERIFY(!bitAddress->isHidden() && wordAddress->isHidden()); word->click(); QVERIFY(!bit->isChecked() && word->isChecked());
    QVERIFY(bit->isEnabled() && word->isEnabled()); QVERIFY(bitAddress->isHidden() && !wordAddress->isHidden());
    bit->click(); QVERIFY(bit->isChecked() && !word->isChecked());
    dialog.findChild<QGroupBox *>(QStringLiteral("BitVisibility"))->setChecked(true);
    dialog.findChild<QGroupBox *>(QStringLiteral("BitVisibilityBit"))->setChecked(true);
    dialog.findChild<QGroupBox *>(QStringLiteral("BitTouch"))->setChecked(true);
    dialog.findChild<QGroupBox *>(QStringLiteral("BitTouchBit"))->setChecked(true);
    QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("BitTouchBitAddress"))->isEnabled());
    QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("BitVisibilityMinimum"))->currentText(), QStringLiteral("1"));
    QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("BitTouchMinimum"))->currentText(), QStringLiteral("2"));
    QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("BitPressTime"))->value(), 255);
    QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("BitPressTime"))->minimum(), 0); QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("BitPressTime"))->maximum(), 255);
    press->click(); QVERIFY(press->isChecked());
    QVERIFY(!dialog.findChild<QSpinBox *>(QStringLiteral("BitPressTime"))->parentWidget()->isHidden());
    draft = dialog.object(); QVERIFY(draft.bitButton.notice && draft.bitButton.noticeBit && !draft.bitButton.noticeWord);
    QVERIFY(draft.bitButton.touch.visibilityEnabled && draft.bitButton.visibility.visibilityEnabled && draft.bitButton.pressTimeEnabled);
    dialog.findChild<QTabWidget *>(QStringLiteral("BitButtonTabs"))->setCurrentIndex(3); saveFunctionShot(&dialog, QStringLiteral("permissions"));
    dialog.findChild<QSpinBox *>(QStringLiteral("BitPressTime"))->setValue(0);
    dialog.findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click(); QCOMPARE(dialog.result(), int(QDialog::Accepted));
    QCOMPARE(dialog.object().bitButton.pressTime, 0);
}

/******************************************************************************
 * Function Name   : FunctionButtonDialogTests::acceptedValuesReopenAndCancel
 * Description     : Accept a configured function and recover its fields while cancelled edits leave the original intact.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void FunctionButtonDialogTests::acceptedValuesReopenAndCancel() //20261110-0 add
{
    HmiProject project;
    HmiObject original = functionObject();
    HmiObject saved;
    BitButtonDialog dialog(original, project);
    QScopedPointer<BitButtonDialog> reopened;
    QScopedPointer<BitButtonDialog> cancelledReopened;
    QDialogButtonBox *buttons;

    dialog.findChild<QComboBox *>(QStringLiteral("FunctionSelection"))->setCurrentIndex(11);
    dialog.findChild<QComboBox *>(QStringLiteral("FunctionType"))->setCurrentIndex(3);
    dialog.findChild<QPlainTextEdit *>(QStringLiteral("BitText"))->setPlainText(QStringLiteral("后一个配方"));
    buttons = dialog.findChild<QDialogButtonBox *>(); buttons->button(QDialogButtonBox::Ok)->click();
    QCOMPARE(dialog.result(), int(QDialog::Accepted)); saved = dialog.object(); QCOMPARE(saved.functionButton.function, 11); QCOMPARE(saved.functionButton.type, 3);
    QCOMPARE(original.functionButton.function, 0); QVERIFY(saved.address.isEmpty());
    reopened.reset(new BitButtonDialog(saved, project));
    QCOMPARE(reopened->findChild<QComboBox *>(QStringLiteral("FunctionSelection"))->currentIndex(), 11);
    QCOMPARE(reopened->findChild<QComboBox *>(QStringLiteral("FunctionType"))->currentIndex(), 3);
    QCOMPARE(reopened->findChild<QPlainTextEdit *>(QStringLiteral("BitText"))->toPlainText(), QStringLiteral("后一个配方"));
    reopened->findChild<QComboBox *>(QStringLiteral("FunctionSelection"))->setCurrentIndex(12);
    reopened->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Cancel)->click(); QCOMPARE(reopened->result(), int(QDialog::Rejected));
    cancelledReopened.reset(new BitButtonDialog(saved, project));
    QCOMPARE(cancelledReopened->findChild<QComboBox *>(QStringLiteral("FunctionSelection"))->currentIndex(), 11);
    QCOMPARE(cancelledReopened->findChild<QComboBox *>(QStringLiteral("FunctionType"))->currentIndex(), 3);
}

/******************************************************************************
 * Function Name   : FunctionButtonDialogTests::otherButtonFamiliesRemainIndependent
 * Description     : Guard bit, word and lamp pages against accidental function-only controls or state restrictions.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void FunctionButtonDialogTests::otherButtonFamiliesRemainIndependent() //20261110-0 add
{
    HmiProject project;
    HmiObject object;
    QScopedPointer<BitButtonDialog> bit;
    QScopedPointer<BitButtonDialog> word;
    QScopedPointer<BitButtonDialog> lamp;

    object = functionObject(); object.hasFunctionButtonProperties = false; object.address = QStringLiteral("LB0");
    bit.reset(new BitButtonDialog(object, project)); QCOMPARE(bit->windowTitle(), QStringLiteral("位按钮"));
    QVERIFY(bit->findChild<QComboBox *>(QStringLiteral("FunctionSelection")) == nullptr);
    QCOMPARE(bit->findChild<QSpinBox *>(QStringLiteral("BitState"))->maximum(), 1);
    object.hasWordButtonProperties = true; object.address = QStringLiteral("LW0");
    word.reset(new BitButtonDialog(object, project)); QCOMPARE(word->windowTitle(), QStringLiteral("字按钮"));
    QVERIFY(word->findChild<QCheckBox *>(QStringLiteral("WordPressedEffect")) != nullptr);
    QVERIFY(word->findChild<QCheckBox *>(QStringLiteral("FunctionPressedEffect")) == nullptr);
    object.hasWordButtonProperties = false; object.hasLampProperties = true;
    object.bitButton.indicator = true; object.bitButton.dualMonitor = true; object.bitButton.monitorSame = false;
    lamp.reset(new BitButtonDialog(object, project)); QCOMPARE(lamp->windowTitle(), QStringLiteral("位指示灯"));
    QVERIFY(lamp->findChild<QGroupBox *>(QStringLiteral("LampMultiMonitor")) != nullptr);
    QVERIFY(lamp->findChild<QComboBox *>(QStringLiteral("FunctionSelection")) == nullptr);
}

QTEST_MAIN(FunctionButtonDialogTests)
#include "function_button_dialog_tests.moc"
