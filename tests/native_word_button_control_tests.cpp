#include "native_word_button_control.h"

#include <QtEndian>
#include <QtTest>

class NativeWordButtonControlTests : public QObject
{
    Q_OBJECT
private slots:
    void originalDefaultAssignedFields();
    void operationsAndTypes_data();
    void operationsAndTypes();
    void frameBoundsAndDormantOnFlash();
    void sharedPermissionsAndNotification();
    void externalAddressRoleAndFlag();
    void failuresPreserveOutput();
};

/******************************************************************************
 * Function Name   : wordSample
 * Description     : Construct the observed default word button with independent shared artwork.
 * Input Parameters: None
 * Return Value    : Word-button model.
 * Date            : 2026.11.09
 ******************************************************************************/
static HmiObject wordSample() //20261109-6 add
{
    HmiObject object;

    object.type = QStringLiteral("switch");
    object.hasBitButtonProperties = true;
    object.hasWordButtonProperties = true;
    object.address = QStringLiteral("LW0");
    object.geometry = QRect(244, 112, 120, 60);
    return object;
}

/******************************************************************************
 * Function Name   : NativeWordButtonControlTests::originalDefaultAssignedFields
 * Description     : Compare assigned fields with the captured 4237-byte native source and original compiler output.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeWordButtonControlTests::originalDefaultAssignedFields() //20261109-6 add
{
    HmiObject object = wordSample();
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;

    options.uncompressedPixelBytes = 14400;
    QVERIFY2(buildNativeWordButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.size(), 640);
    QCOMPARE(control.left(3).toHex(), QByteArray("0a0000"));
    QCOMPARE(control.mid(4, 16).toHex(), QByteArray("f400700078003c000000000040380000"));
    QCOMPARE(control.mid(0x106, 2).toHex(), QByteArray("0100"));
    QCOMPARE(control.mid(0x110, 0x28).toHex(), QByteArray("00000000000000000000000002000000000000000000f03f00000000000000000000000000000000"));
    QCOMPARE(control.mid(0x138, 3).toHex(), QByteArray("005700"));
    QCOMPARE(control[0x13d], char(1));
    QCOMPARE(control.mid(0x140, 4), QByteArray(4, char(0)));
    // Unexposed source bools and unused subtype0 monitor fields are intentionally not copied from native stack residue.
    QCOMPARE(control.mid(0x180, 0x3d), QByteArray(0x3d, char(0)));
}

/******************************************************************************
 * Function Name   : NativeWordButtonControlTests::operationsAndTypes_data
 * Description     : Exercise all seven operation and seven word data-type indices.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeWordButtonControlTests::operationsAndTypes_data() //20261109-6 add
{
    int operation;
    int type;

    QTest::addColumn<int>("operation"); QTest::addColumn<int>("type");
    for (type = 0; type < 7; ++type) {
        for (operation = 0; operation < 7; ++operation) {
            QTest::newRow(qPrintable(QStringLiteral("type%1-op%2").arg(type).arg(operation))) << operation << type;
        }
    }
}

/******************************************************************************
 * Function Name   : NativeWordButtonControlTests::operationsAndTypes
 * Description     : Preserve subtype1 field locations independently of the shared condition type enumeration.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeWordButtonControlTests::operationsAndTypes() //20261109-6 add
{
    HmiObject object = wordSample();
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;
    QFETCH(int, operation);
    QFETCH(int, type);

    object.wordButton.resetDefaultsForDataType(type); object.wordButton.operation = operation;
    object.wordButton.constantValue = type == 6 ? QStringLiteral("2.5") : QStringLiteral("2");
    object.wordButton.minimumValue = QStringLiteral("0"); object.wordButton.maximumValue = QStringLiteral("100");
    object.wordButton.totalDigits = 5; object.wordButton.decimalDigits = operation == 1 ? 2 : 0;
    options.uncompressedPixelBytes = 14400;
    QVERIFY2(buildNativeWordButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(qFromLittleEndian<u32>(control.constData() + 0x118), u32(operation));
    QCOMPARE(qFromLittleEndian<u32>(control.constData() + 0x11c), u32(type));
    QCOMPARE(control[0x13d], char(operation < 3 ? 1 : 0));
    QCOMPARE(qFromLittleEndian<u32>(control.constData() + 0x110), u32(operation == 1 || operation == 2 ? 5 : 0));
    QCOMPARE(qFromLittleEndian<u32>(control.constData() + 0x114), u32(operation == 1 ? 2 : 0));
    if (operation == 0 || operation >= 3) {
        QCOMPARE(control.mid(0x120, 8).toHex(), type == 6 ? QByteArray("0000000000000440") : QByteArray("0000000000000040"));
    }
    if (operation == 1 || operation == 2 || operation == 3 || operation >= 5) {
        QCOMPARE(control.mid(0x130, 8).toHex(), QByteArray("0000000000005940"));
    }
}

/******************************************************************************
 * Function Name   : NativeWordButtonControlTests::frameBoundsAndDormantOnFlash
 * Description     : Count only emitted states and every emitted state's immediate flash image.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeWordButtonControlTests::frameBoundsAndDormantOnFlash() //20261109-6 add
{
    HmiObject object = wordSample();
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;

    object.bitButton.states[1].flash = true;
    options.uncompressedPixelBytes = 14400;
    QVERIFY2(buildNativeWordButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control[0x10b], char(0));
    object.wordButton.pressedEffect = true; options.uncompressedPixelBytes = 43200;
    QVERIFY2(buildNativeWordButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control[0x103], char(1)); QCOMPARE(control[0x10b], char(1));
    object.bitButton.states[0].flash = true; options.languageCount = 2; options.uncompressedPixelBytes = 115200;
    QVERIFY2(buildNativeWordButtonControl(object, options, &control, &error), qPrintable(error));
    options.uncompressedPixelBytes = 114000;
    QVERIFY(!buildNativeWordButtonControl(object, options, &control, &error));
}

/******************************************************************************
 * Function Name   : NativeWordButtonControlTests::sharedPermissionsAndNotification
 * Description     : Retain the independently verified common permissions and single notification slot.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeWordButtonControlTests::sharedPermissionsAndNotification() //20261109-6 add
{
    HmiObject object = wordSample();
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;

    object.bitButton.touch.visibilityEnabled = true; object.bitButton.touch.bitEnabled = true;
    object.bitButton.touch.bitAddress = QStringLiteral("LB17");
    object.bitButton.visibility.visibilityEnabled = true; object.bitButton.visibility.levelEnabled = true;
    object.bitButton.visibility.minimumLevel = 3;
    object.bitButton.notice = true; object.bitButton.noticeWord = true;
    object.bitButton.noticeWordAddress = QStringLiteral("LW7"); object.bitButton.noticeWordValue = QStringLiteral("42");
    options.uncompressedPixelBytes = 14400;
    QVERIFY2(buildNativeWordButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control[0x14], char(1)); QCOMPARE(control[0x21], char(4));
    QCOMPARE(control.mid(0x54, 4).toHex(), QByteArray("11000000"));
    QCOMPARE(control[0x15], char(1)); QCOMPARE(control[0x22], char(1)); QCOMPARE(control[0x1f], char(2));
    QCOMPARE(control[0x16], char(1)); QCOMPARE(control[0x23], char(2));
    QCOMPARE(control.mid(0x44, 8).toHex(), QByteArray("0000000000004540"));
}

/******************************************************************************
 * Function Name   : NativeWordButtonControlTests::externalAddressRoleAndFlag
 * Description     : Keep word-slot rel5 independent of external scan preparation and the bit-toggle callback.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeWordButtonControlTests::externalAddressRoleAndFlag() //20261109-6 add
{
    HmiObject object = wordSample();
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;
    int calls = 0;

    object.address = QStringLiteral("@com1:4x0"); options.uncompressedPixelBytes = 14400;
    options.externalAddressEncoder = [&calls](const QString &address, bool word, bool toggle, QByteArray *bytes, QString *) {
        if (address != QStringLiteral("@com1:4x0") || !word || toggle) { return false; }
        ++calls; *bytes = QByteArray(36, char(0x2a)); return true;
    };
    QVERIFY2(buildNativeWordButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(calls, 1); QCOMPARE(control[0x13d], char(1)); QCOMPARE(control[0x138], char(0x2a));
    object.wordButton.operation = 3;
    QVERIFY2(buildNativeWordButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(calls, 2); QCOMPARE(control[0x13d], char(0));
}

/******************************************************************************
 * Function Name   : NativeWordButtonControlTests::failuresPreserveOutput
 * Description     : Preserve the previous record after invalid values, local families or external slot lengths.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeWordButtonControlTests::failuresPreserveOutput() //20261109-6 add
{
    HmiObject object = wordSample();
    NativeBitButtonControlOptions options;
    QByteArray control("preserved");
    QString error;

    options.uncompressedPixelBytes = 14400; object.wordButton.constantValue = QStringLiteral("nan");
    QVERIFY(!buildNativeWordButtonControl(object, options, &control, &error));
    object.wordButton.constantValue = QStringLiteral("1"); object.address = QStringLiteral("LB0");
    QVERIFY(!buildNativeWordButtonControl(object, options, &control, &error));
    object.address = QStringLiteral("@com1:4x0");
    options.externalAddressEncoder = [](const QString &, bool, bool, QByteArray *bytes, QString *) {
        *bytes = QByteArray(35, char(0)); return true;
    };
    QVERIFY(!buildNativeWordButtonControl(object, options, &control, &error));
    QCOMPARE(control, QByteArray("preserved"));
}

QTEST_APPLESS_MAIN(NativeWordButtonControlTests)
#include "native_word_button_control_tests.moc"
