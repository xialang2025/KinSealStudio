#include "bit_button_dialog.h"
#include "word_button_properties.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPixmap>
#include <QPushButton>
#include <QScopedPointer>
#include <QSpinBox>
#include <QTabWidget>
#include <QTimer>
#include <QtTest>

/******************************************************************************
 * Function Name   : wordObject
 * Description     : Build a detached word fixture without interpreting disabled swatch pixels as saved colors.
 * Input Parameters: None
 * Return Value    : Test object.
 * Date            : 2026.11.09
 ******************************************************************************/
static HmiObject wordObject() //20261109-6 add
{
    HmiObject object;

    object.type = QStringLiteral("switch"); object.address = QStringLiteral("LW0");
    object.geometry = QRect(20, 20, 120, 60); object.hasBitButtonProperties = true;
    object.hasWordButtonProperties = true; object.bitButton.indicator = false; object.bitButton.dualMonitor = false;
    return object;
}

/******************************************************************************
 * Function Name   : saveWordShot
 * Description     : Render optional evidence offscreen without opening a desktop window.
 * Input Parameters: dialog, name
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
static void saveWordShot(BitButtonDialog *dialog, const QString &name) //20261109-6 add
{
    QString directory;
    QPixmap pixels;

    directory = qEnvironmentVariable("KINSEAL_WORD_UI_SNAPSHOT_DIR");
    if (directory.isEmpty()) { return; }
    QDir().mkpath(directory); dialog->ensurePolished(); dialog->layout()->activate();
    pixels = QPixmap(dialog->size()); pixels.fill(Qt::white); dialog->render(&pixels);
    pixels.save(directory + QChar(u'/') + name + QStringLiteral(".png"));
}

class WordButtonDialogTests : public QObject
{
    Q_OBJECT
private slots:
    void propertiesRoundTripAndGuards();
    void matrix_data();
    void matrix();
    void pressedDraftAndCancellation();
    void notificationsAndPermissionGates();
    void acceptedValuesReopen();
    void wordAddressPicker();
    void bitDialogRemainsIndependent();
};

/******************************************************************************
 * Function Name   : WordButtonDialogTests::propertiesRoundTripAndGuards
 * Description     : Check persistence, transactional failures and active numeric domain boundaries.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void WordButtonDialogTests::propertiesRoundTripAndGuards() //20261109-6 add
{
    WordButtonProperties properties;
    WordButtonProperties restored;
    QJsonObject invalid;
    QString error;

    QCOMPARE(properties.dataType, 2); QCOMPARE(properties.operation, 0);
    QCOMPARE(properties.constantValue, QStringLiteral("1")); QVERIFY(properties.validate(&error));
    properties.resetDefaultsForDataType(1); properties.constantValue = QStringLiteral("4294967295");
    properties.pressedEffect = true; QVERIFY(properties.validate(&error));
    QVERIFY(WordButtonProperties::fromJson(properties.toJson(), &restored, &error));
    QCOMPARE(restored.toJson(), properties.toJson()); QCOMPARE(restored.transferWordCount(), 2);
    invalid = properties.toJson(); invalid.insert(QStringLiteral("constantValue"), QStringLiteral("4294967296"));
    QVERIFY(!WordButtonProperties::fromJson(invalid, &restored, &error)); QCOMPARE(restored.toJson(), properties.toJson());
    properties.resetDefaultsForDataType(4); properties.constantValue = QStringLiteral("10000"); QVERIFY(!properties.validate(&error));
    properties.constantValue = QStringLiteral("1.5"); QVERIFY(!properties.validate(&error));
    properties.resetDefaultsForDataType(6); properties.constantValue = QStringLiteral("NaN"); QVERIFY(!properties.validate(&error));
    properties.constantValue = QStringLiteral("1.25e3"); QVERIFY(properties.validate(&error));
    properties.resetDefaultsForOperation(1); properties.minimumValue = QStringLiteral("10"); properties.maximumValue = QStringLiteral("9");
    QVERIFY(!properties.validate(&error)); properties.maximumValue = QStringLiteral("11");
    properties.decimalDigits = properties.totalDigits; QVERIFY(!properties.validate(&error));
    properties.decimalDigits = 0; properties.constantValue = QStringLiteral("dormant draft"); QVERIFY(properties.validate(&error));
    properties.operation = 7; QVERIFY(!properties.validate(&error));
}

/******************************************************************************
 * Function Name   : WordButtonDialogTests::matrix_data
 * Description     : Enumerate every observed data-type and operation combination.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void WordButtonDialogTests::matrix_data() //20261109-6 add
{
    QByteArray name;
    int type;
    int operation;

    QTest::addColumn<int>("type"); QTest::addColumn<int>("operation");
    for (type = 0; type < 7; ++type)
    {
        for (operation = 0; operation < 7; ++operation)
        {
            name = QByteArray::number(type) + '-' + QByteArray::number(operation);
            QTest::newRow(name.constData()) << type << operation;
        }
    }
}

/******************************************************************************
 * Function Name   : WordButtonDialogTests::matrix
 * Description     : Check the manual's seven-by-seven default and visibility matrix independently of helpers.
 * Input Parameters: Qt data row
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void WordButtonDialogTests::matrix() //20261109-6 add
{
    static const QString minimum[7] = {QStringLiteral("0"), QStringLiteral("0"), QStringLiteral("-32768"), QStringLiteral("-99999"), QStringLiteral("0"), QStringLiteral("0"), QStringLiteral("-9999")};
    static const QString maximum[7] = {QStringLiteral("65535"), QStringLiteral("99999"), QStringLiteral("32767"), QStringLiteral("99999"), QStringLiteral("9999"), QStringLiteral("9999"), QStringLiteral("9999")};
    HmiProject project;
    BitButtonDialog dialog(wordObject(), project);
    QComboBox *dataType;
    QComboBox *operationCombo;
    QLineEdit *constant;
    QLineEdit *lower;
    QLineEdit *upper;
    QSpinBox *total;
    QSpinBox *decimal;
    HmiObject draft;
    QFETCH(int, type);
    QFETCH(int, operation);

    dataType = dialog.findChild<QComboBox *>(QStringLiteral("WordDataType"));
    operationCombo = dialog.findChild<QComboBox *>(QStringLiteral("WordOperation"));
    constant = dialog.findChild<QLineEdit *>(QStringLiteral("WordConstant"));
    lower = dialog.findChild<QLineEdit *>(QStringLiteral("WordMinimum"));
    upper = dialog.findChild<QLineEdit *>(QStringLiteral("WordMaximum"));
    total = dialog.findChild<QSpinBox *>(QStringLiteral("WordTotalDigits"));
    decimal = dialog.findChild<QSpinBox *>(QStringLiteral("WordDecimalDigits"));
    QVERIFY(dataType && operationCombo && constant && lower && upper && total && decimal);
    QCOMPARE(dataType->count(), 7); QCOMPARE(operationCombo->count(), 7);
    QCOMPARE(dialog.windowTitle(), QStringLiteral("字按钮"));
    dataType->setCurrentIndex((type + 1) % 7); dataType->setCurrentIndex(type);
    constant->setText(QStringLiteral("777")); operationCombo->setCurrentIndex((operation + 1) % 7); operationCombo->setCurrentIndex(operation);
    QCOMPARE(constant->text(), QStringLiteral("1")); QCOMPARE(lower->text(), minimum[type]); QCOMPARE(upper->text(), maximum[type]);
    QCOMPARE(total->value(), type >= 4 ? 4 : 5); QCOMPARE(decimal->value(), 0);
    QCOMPARE(!constant->parentWidget()->isHidden(), operation == 0 || operation >= 3);
    QCOMPARE(!lower->parentWidget()->isHidden(), operation == 1 || operation == 2 || operation == 4 || operation >= 5);
    QCOMPARE(!upper->parentWidget()->isHidden(), operation == 1 || operation == 2 || operation == 3 || operation >= 5);
    QCOMPARE(!total->parentWidget()->isHidden(), operation == 1 || operation == 2);
    QCOMPARE(!decimal->parentWidget()->isHidden(), operation == 1);
    draft = dialog.object(); QCOMPARE(draft.wordButton.dataType, type); QCOMPARE(draft.wordButton.operation, operation);
    QVERIFY(!draft.bitButton.indicator && !draft.bitButton.dualMonitor); QVERIFY(draft.wordButton.validate(nullptr));
    if (type == 2 && operation == 0) { saveWordShot(&dialog, QStringLiteral("general-default")); }
}

/******************************************************************************
 * Function Name   : WordButtonDialogTests::pressedDraftAndCancellation
 * Description     : Preserve both drafts while gating the pressed state and disabled copy controls.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void WordButtonDialogTests::pressedDraftAndCancellation() //20261109-6 add
{
    HmiProject project;
    HmiObject original = wordObject();
    BitButtonDialog dialog(original, project);
    QCheckBox *effect;
    QSpinBox *state;
    QPlainTextEdit *text;
    QPushButton *copyText;
    QPushButton *copyShape;
    QTabWidget *tabs;
    HmiObject draft;

    effect = dialog.findChild<QCheckBox *>(QStringLiteral("WordPressedEffect"));
    state = dialog.findChild<QSpinBox *>(QStringLiteral("BitState")); text = dialog.findChild<QPlainTextEdit *>(QStringLiteral("BitText"));
    copyText = dialog.findChild<QPushButton *>(QStringLiteral("BitCopyText")); copyShape = dialog.findChild<QPushButton *>(QStringLiteral("BitCopyShape"));
    tabs = dialog.findChild<QTabWidget *>(QStringLiteral("BitButtonTabs"));
    QVERIFY(effect && state && text && copyText && copyShape && tabs);
    QCOMPARE(tabs->count(), 4); QVERIFY(tabs->widget(1)->findChildren<QWidget *>().isEmpty());
    QCOMPARE(state->maximum(), 0); QVERIFY(!copyText->isEnabled() && !copyShape->isEnabled());
    tabs->setCurrentIndex(2); saveWordShot(&dialog, QStringLiteral("appearance-state0"));
    effect->setChecked(true); QCOMPARE(state->maximum(), 1); QVERIFY(copyText->isEnabled() && copyShape->isEnabled());
    state->setValue(1); QCOMPARE(text->toPlainText(), QStringLiteral("ON"));
    QVERIFY(!copyText->isEnabled() && !copyShape->isEnabled());
    QVERIFY(!dialog.findChild<QPushButton *>(QStringLiteral("BitBackground"))->isEnabled());
    saveWordShot(&dialog, QStringLiteral("appearance-state1"));
    text->setPlainText(QStringLiteral("pressed draft")); effect->setChecked(false); QCOMPARE(state->value(), 0);
    effect->setChecked(true); state->setValue(1); QCOMPARE(text->toPlainText(), QStringLiteral("pressed draft"));
    draft = dialog.object(); QCOMPARE(draft.bitButton.states[1].background, QColor(Qt::white));
    QCOMPARE(draft.bitButton.states[1].text.languageTexts.value(QStringLiteral("中文")), QStringLiteral("pressed draft"));
    QCOMPARE(original.bitButton.states[1].text.languageTexts.value(QStringLiteral("中文")), QStringLiteral("ON"));
    dialog.reject(); QCOMPARE(original.wordButton.constantValue, QStringLiteral("1"));
}

/******************************************************************************
 * Function Name   : WordButtonDialogTests::notificationsAndPermissionGates
 * Description     : Check parent gating, asymmetric notification exclusion and shared permission fields.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void WordButtonDialogTests::notificationsAndPermissionGates() //20261109-6 add
{
    HmiProject project;
    BitButtonDialog dialog(wordObject(), project);
    QGroupBox *notice;
    QCheckBox *bit;
    QCheckBox *word;
    QGroupBox *touch;
    QGroupBox *touchBit;
    QLineEdit *bitAddress;
    QLineEdit *wordAddress;
    HmiObject draft;

    notice = dialog.findChild<QGroupBox *>(QStringLiteral("BitNotice")); bit = dialog.findChild<QCheckBox *>(QStringLiteral("BitNoticeBit"));
    word = dialog.findChild<QCheckBox *>(QStringLiteral("BitNoticeWord"));
    bitAddress = dialog.findChild<QLineEdit *>(QStringLiteral("BitNoticeBitAddress")); wordAddress = dialog.findChild<QLineEdit *>(QStringLiteral("BitNoticeWordAddress"));
    touch = dialog.findChild<QGroupBox *>(QStringLiteral("BitTouch")); touchBit = dialog.findChild<QGroupBox *>(QStringLiteral("BitTouchBit"));
    QVERIFY(notice && bit && word && bitAddress && wordAddress && touch && touchBit);
    QVERIFY(notice->isEnabled()); QVERIFY(!bit->isEnabled() && !word->isEnabled());
    QVERIFY(bitAddress->isHidden() && wordAddress->isHidden()); notice->setChecked(true);
    QVERIFY(bit->isEnabled() && word->isEnabled()); bit->setChecked(true);
    QVERIFY(!bitAddress->isHidden() && wordAddress->isHidden()); QVERIFY(word->isEnabled());
    word->setChecked(true); QVERIFY(!bit->isChecked() && !bit->isEnabled()); QVERIFY(word->isChecked());
    dialog.findChild<QTabWidget *>(QStringLiteral("BitButtonTabs"))->setCurrentIndex(3);
    saveWordShot(&dialog, QStringLiteral("permission-word-notice"));
    QVERIFY(bitAddress->isHidden() && !wordAddress->isHidden()); word->setChecked(false); QVERIFY(bit->isEnabled());
    touch->setChecked(true); QVERIFY(!dialog.findChild<QLineEdit *>(QStringLiteral("BitTouchBitAddress"))->isEnabled());
    touchBit->setChecked(true); QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("BitTouchBitAddress"))->isEnabled());
    QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("BitTouchMinimum"))->currentText(), QStringLiteral("2"));
    draft = dialog.object(); QVERIFY(draft.bitButton.touch.visibilityEnabled && draft.bitButton.touch.bitEnabled);
}

/******************************************************************************
 * Function Name   : WordButtonDialogTests::acceptedValuesReopen
 * Description     : Recover committed values without construction-time default resets and discard cancelled edits.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void WordButtonDialogTests::acceptedValuesReopen() //20261109-6 add
{
    HmiProject project;
    BitButtonDialog dialog(wordObject(), project);
    HmiObject accepted;
    QScopedPointer<BitButtonDialog> reopened;

    dialog.findChild<QLineEdit *>(QStringLiteral("WordConstant"))->setText(QStringLiteral("999"));
    dialog.findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
    QCOMPARE(dialog.result(), int(QDialog::Accepted)); accepted = dialog.object();
    reopened.reset(new BitButtonDialog(accepted, project));
    QCOMPARE(reopened->findChild<QLineEdit *>(QStringLiteral("WordConstant"))->text(), QStringLiteral("999"));
    reopened->findChild<QLineEdit *>(QStringLiteral("WordConstant"))->setText(QStringLiteral("123")); reopened->reject();
    QCOMPARE(accepted.wordButton.constantValue, QStringLiteral("999"));
}

/******************************************************************************
 * Function Name   : WordButtonDialogTests::wordAddressPicker
 * Description     : Check that the shared picker receives the word role and the two-word LW boundary.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void WordButtonDialogTests::wordAddressPicker() //20261109-6 add
{
    HmiProject project;
    BitButtonDialog dialog(wordObject(), project);
    bool inspected = false;
    bool wordFamily = false;
    bool wideBoundary = false;

    dialog.findChild<QComboBox *>(QStringLiteral("WordDataType"))->setCurrentIndex(1);
    QTimer::singleShot(0, &dialog, [&inspected, &wordFamily, &wideBoundary]() {
        QDialog *picker;
        QComboBox *family;
        QComboBox *candidate;
        QLabel *label;

        picker = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (picker == nullptr) { return; }
        family = picker->findChild<QComboBox *>(QStringLiteral("PlcFamily"));
        if (family == nullptr)
        {
            Q_FOREACH (candidate, picker->findChildren<QComboBox *>())
            {
                if (candidate->currentText() == QStringLiteral("LW")) { family = candidate; break; }
            }
        }
        inspected = true; wordFamily = family != nullptr && family->currentText() == QStringLiteral("LW") && family->findText(QStringLiteral("LB")) == -1;
        Q_FOREACH (label, picker->findChildren<QLabel *>())
        {
            if (label->text().contains(QStringLiteral("4095"))) { wideBoundary = true; }
        }
        picker->reject();
    });
    dialog.findChild<QPushButton *>(QStringLiteral("WordAddressPicker"))->click();
    QVERIFY(inspected); QVERIFY(wordFamily); QVERIFY(wideBoundary);
}

/******************************************************************************
 * Function Name   : WordButtonDialogTests::bitDialogRemainsIndependent
 * Description     : Guard the existing bit operation, monitor and notification paths.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void WordButtonDialogTests::bitDialogRemainsIndependent() //20261109-6 add
{
    HmiProject project;
    HmiObject object;
    QCheckBox *bit;
    QCheckBox *word;
    QScopedPointer<BitButtonDialog> dialog;

    object.type = QStringLiteral("switch"); object.address = QStringLiteral("LB0"); object.hasBitButtonProperties = true;
    dialog.reset(new BitButtonDialog(object, project));
    QCOMPARE(dialog->windowTitle(), QStringLiteral("位按钮"));
    QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("BitOperation"))->count(), 4);
    QVERIFY(dialog->findChild<QGroupBox *>(QStringLiteral("BitDualMonitor")) != nullptr);
    QVERIFY(dialog->findChild<QCheckBox *>(QStringLiteral("WordPressedEffect")) == nullptr);
    dialog->findChild<QGroupBox *>(QStringLiteral("BitNotice"))->setChecked(true);
    bit = dialog->findChild<QCheckBox *>(QStringLiteral("BitNoticeBit")); word = dialog->findChild<QCheckBox *>(QStringLiteral("BitNoticeWord"));
    word->setChecked(true); QVERIFY(bit->isEnabled());
    QCOMPARE(dialog->findChild<QSpinBox *>(QStringLiteral("BitState"))->maximum(), 1);
}

QTEST_MAIN(WordButtonDialogTests)
#include "word_button_dialog_tests.moc"
