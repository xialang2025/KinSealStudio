#include "native_bit_button_control.h"

#include <QtTest>

//20261109-0 add: Compare fields assigned by native functions, excluding recorded stack residue.
class NativeBitButtonControlTests : public QObject
{
    Q_OBJECT
private slots:
    void recordedOperations_data();
    void recordedOperations();
    void monitorModes();
    void offsetsAddressesAndMacro();
    void paddingIsClean();
    void unsupportedConditionsAreTransactional();
    void inconsistentPixelsAreRejected();
    void nativeFieldBounds();
    void recordedPermissionsAndPressTime();
    void permissionWordTypesAndComparisons();
    void recordedFlashAndAppearance();
    void flashStateCounts();
    void singleModesIgnoreInactiveOnFlash();
    void recordedBitNotification();
    void recordedWordNotification();
    void notificationValuesAreBounded();
    void externalAddressesReachEveryActiveField();
    void externalFailuresAreTransactional();
    void multilingualPixelLengths();
    void languageCountsAreBounded();
};

/******************************************************************************
 * Function Name   : sampleButton
 * Description     : Reproduce the geometry and properties of the four recorded native projects.
 * Input Parameters: None
 * Return Value    : A complete default LB0 bit button.
 * Date            : 2026.11.09
 ******************************************************************************/
static HmiObject sampleButton() //20261109-0 add
{
    HmiObject object;

    object.type = QStringLiteral("switch");
    object.address = QStringLiteral("LB0");
    object.geometry = QRect(244, 112, 120, 60);
    object.hasBitButtonProperties = true;
    return object;
}

/******************************************************************************
 * Function Name   : assignedNativeFields
 * Description     : Project only assigned fields from independent native compiler recordings.
 * Input Parameters: control
 * Return Value    : The concatenated semantic field bytes.
 * Date            : 2026.11.09
 ******************************************************************************/
static QByteArray assignedNativeFields(const QByteArray &control) //20261109-0 add
{
    const int ranges[][2] = {{0, 3}, {4, 72}, {0x100, 4}, {0x106, 6},
                             {0x114, 7}, {0x11d, 1}, {0x120, 4}, {0x180, 2},
                             {0x184, 8}, {0x198, 3}, {0x19d, 1}, {0x1a0, 4}, {0x1bd, 3}};
    QByteArray projected;
    int index;

    for (index = 0; index < int(sizeof(ranges) / sizeof(ranges[0])); ++index)
    {
        projected.append(control.mid(ranges[index][0], ranges[index][1]));
    }
    return projected;
}

/******************************************************************************
 * Function Name   : NativeBitButtonControlTests::recordedOperations_data
 * Description     : Retain projected oracle bytes from the four original UI-compiled operations.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeBitButtonControlTests::recordedOperations_data() //20261109-0 add
{
    QTest::addColumn<int>("operation");
    QTest::addColumn<QByteArray>("nativeFields");
    QTest::addColumn<QByteArray>("nativeWriteFlags");

    //20261109-0 add: Sources are operation-{set,reset,jog,toggle}-ui-compiled/ControlPara.dat.
    QTest::newRow("native-set") << 0 << QByteArray::fromHex(
        "0a0000f400700078003c0000000000807000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000420000000000000001010000000f0000000042000000000000009c63") << QByteArray::fromHex("0101");
    QTest::newRow("native-reset") << 1 << QByteArray::fromHex(
        "0a0000f400700078003c0000000000807000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000100000000420000000000000001010000000f0000000042000000000000009c63") << QByteArray::fromHex("0000");
    QTest::newRow("native-jog") << 2 << QByteArray::fromHex(
        "0a0000f400700078003c0000000000807000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000200000000420000000000000001010000000f0000000042000000000000009c63") << QByteArray::fromHex("0000");
    QTest::newRow("native-toggle") << 3 << QByteArray::fromHex(
        "0a0000f400700078003c0000000000807000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000300000000420001000000000001010000000f0000000042000000000000009c63") << QByteArray::fromHex("0000");
}

/******************************************************************************
 * Function Name   : NativeBitButtonControlTests::recordedOperations
 * Description     : Match every recovered active field against the native operation oracle.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeBitButtonControlTests::recordedOperations() //20261109-0 add
{
    HmiObject object;
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;
    QFETCH(int, operation);
    QFETCH(QByteArray, nativeFields);
    QFETCH(QByteArray, nativeWriteFlags);

    object = sampleButton();
    object.bitButton.operation = operation;
    options.uncompressedPixelBytes = 28800;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.size(), 640);
    QCOMPARE(assignedNativeFields(control), nativeFields);
    QCOMPARE(control.mid(0x10c, 2), nativeWriteFlags);
    QCOMPARE(control.mid(0x190, 4).toHex(), QByteArray("02000000"));
}

/******************************************************************************
 * Function Name   : NativeBitButtonControlTests::monitorModes
 * Description     : Preserve the three recorded monitor modes and the single-state word source.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeBitButtonControlTests::monitorModes() //20261109-0 add
{
    HmiObject object;
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;

    object = sampleButton();
    object.bitButton.indicator = false;
    options.uncompressedPixelBytes = 14400;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x184, 4).toHex(), QByteArray("00000000"));
    QCOMPARE(control.mid(0x198, 36), QByteArray(36, char(0)));
    QCOMPARE(control.mid(0x18c, 12), QByteArray(12, char(0)));

    object.bitButton.indicator = true;
    object.bitButton.dualMonitor = false;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x180, 2).toHex(), QByteArray("0100"));
    QCOMPARE(control.mid(0x184, 4).toHex(), QByteArray("02000000"));
    QCOMPARE(control.mid(0x198, 3).toHex(), QByteArray("005700"));
    QCOMPARE(control.mid(0x190, 4).toHex(), QByteArray("02000000"));
    QCOMPARE(control.mid(0x18c, 4), QByteArray(4, char(0)));
    QCOMPARE(control.mid(0x194, 4), QByteArray(4, char(0)));

    object.bitButton.dualMonitor = true;
    object.bitButton.monitorSame = false;
    object.bitButton.monitorType = 1;
    object.bitButton.monitorWordAddress = QStringLiteral("LW17");
    options.uncompressedPixelBytes = 28800;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x180, 2).toHex(), QByteArray("0100"));
    QCOMPARE(control.mid(0x184, 4).toHex(), QByteArray("01000000"));
    QCOMPARE(control.mid(0x1a0, 4).toHex(), QByteArray("11000000"));
}

/******************************************************************************
 * Function Name   : NativeBitButtonControlTests::offsetsAddressesAndMacro
 * Description     : Encode nonzero offsets, local picker metadata and the resolved macro ordinal.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeBitButtonControlTests::offsetsAddressesAndMacro() //20261109-0 add
{
    HmiObject object;
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;

    object = sampleButton();
    object.address = QStringLiteral("LB17");
    object.bitButton.macroEnabled = true;
    object.bitButton.macroName = QStringLiteral("MacroA");
    object.bitButton.pressTimeEnabled = true;
    object.bitButton.pressTime = 10;
    options.uncompressedPixelOffset = 0x123456;
    options.uncompressedPixelBytes = 28800;
    options.addressByte2 = 100;
    options.macroIndex = 4;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.left(3).toHex(), QByteArray("0a0104"));
    QCOMPARE(control.mid(0x0c, 8).toHex(), QByteArray("5634120080700000"));
    QCOMPARE(control.mid(0x118, 3).toHex(), QByteArray("004264"));
    QCOMPARE(control.mid(0x120, 4).toHex(), QByteArray("11000000"));
    QCOMPARE(control.mid(0x198, 3).toHex(), QByteArray("004264"));
    QCOMPARE(control.mid(0x1bd, 3).toHex(), QByteArray("01e803"));
}

/******************************************************************************
 * Function Name   : NativeBitButtonControlTests::paddingIsClean
 * Description     : Prevent stack addresses and inactive union members from leaking into device records.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeBitButtonControlTests::paddingIsClean() //20261109-0 add
{
    HmiObject object;
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;

    object = sampleButton();
    options.uncompressedPixelBytes = 28800;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.at(3), char(0));
    QCOMPARE(control.mid(0x4c, 0xb4), QByteArray(0xb4, char(0)));
    QCOMPARE(control.mid(0x104, 2), QByteArray(2, char(0)));
    QCOMPARE(control.mid(0x138, 0x48), QByteArray(0x48, char(0)));
    QCOMPARE(control.mid(0x1c0), QByteArray(192, char(0)));
}

/******************************************************************************
 * Function Name   : NativeBitButtonControlTests::unsupportedConditionsAreTransactional
 * Description     : Preserve the previous result when an enabled unverified condition is rejected.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeBitButtonControlTests::unsupportedConditionsAreTransactional() //20261109-0 add
{
    HmiObject object;
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;

    object = sampleButton();
    options.uncompressedPixelBytes = 28800;
    control = QByteArrayLiteral("preserved");
    object.bitButton.notice = true;
    object.bitButton.noticeBit = true;
    object.bitButton.noticeWord = true;
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
    QVERIFY(!error.isEmpty());
    QCOMPARE(control, QByteArrayLiteral("preserved"));
    object.bitButton.notice = false;
    object.address = QStringLiteral("M0");
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
    QCOMPARE(control, QByteArrayLiteral("preserved"));
}

/******************************************************************************
 * Function Name   : NativeBitButtonControlTests::recordedPermissionsAndPressTime
 * Description     : Match pure UI native samples for both bit gates and the one-unit press timer.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeBitButtonControlTests::recordedPermissionsAndPressTime() //20261109-0 add
{
    HmiObject object;
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;

    object = sampleButton();
    options.uncompressedPixelBytes = 28800;
    object.bitButton.visibility.visibilityEnabled = true;
    object.bitButton.visibility.bitEnabled = true;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x14, 16).toHex(), QByteArray("00010000000000000000010000000400"));
    QCOMPARE(control.mid(0x94, 3).toHex(), QByteArray("004200"));
    QCOMPARE(control.mid(0x99, 1).toHex(), QByteArray("00"));
    QCOMPARE(control.mid(0x9c, 4).toHex(), QByteArray("00000000"));

    object = sampleButton();
    object.bitButton.touch.visibilityEnabled = true;
    object.bitButton.touch.bitEnabled = true;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x14, 16).toHex(), QByteArray("01000000000000000100000000040000"));
    QCOMPARE(control.mid(0x4c, 3).toHex(), QByteArray("004200"));

    object = sampleButton();
    object.bitButton.pressTimeEnabled = true;
    object.bitButton.pressTime = 1;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x1bd, 3).toHex(), QByteArray("016400"));
}

/******************************************************************************
 * Function Name   : NativeBitButtonControlTests::permissionWordTypesAndComparisons
 * Description     : Cover the five recovered numeric types, all six comparisons and combined level gates.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeBitButtonControlTests::permissionWordTypesAndComparisons() //20261109-0 add
{
    const u8 nativeTypes[5] = {4, 5, 2, 3, 8};
    HmiObject object;
    NativeBitButtonControlOptions options;
    StaticTextProperties *condition;
    QByteArray control;
    QString error;
    int kind;
    int type;
    int comparison;

    options.uncompressedPixelBytes = 28800;
    for (kind = 0; kind < 2; ++kind)
    {
        for (type = 0; type < 5; ++type)
        {
            for (comparison = 0; comparison < 6; ++comparison)
            {
                object = sampleButton();
                condition = kind == 0 ? &object.bitButton.touch : &object.bitButton.visibility;
                condition->visibilityEnabled = true;
                condition->bitEnabled = true;
                condition->bitAddress = QStringLiteral("LB17");
                condition->bitState = false;
                condition->levelEnabled = true;
                condition->minimumLevel = 3;
                condition->wordEnabled = true;
                condition->wordAddress = QStringLiteral("LW7");
                condition->dataType = type;
                condition->comparison = comparison;
                condition->compareValue = QStringLiteral("42");
                condition->tolerance = QStringLiteral("0.25");
                QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
                QCOMPARE(u8(control.at(kind == 0 ? 0x17 : 0x18)), nativeTypes[type]);
                QCOMPARE(u8(control.at(kind == 0 ? 0x1a : 0x1b)), u8(comparison));
                QCOMPARE(u8(control.at(kind == 0 ? 0x21 : 0x22)), u8(7));
                QCOMPARE(u8(control.at(kind == 0 ? 0x1d : 0x1f)), u8(2));
                QCOMPARE(control.mid(kind == 0 ? 0x24 : 0x34, 16).toHex(),
                         QByteArray("0000000000004540000000000000d03f"));
                QCOMPARE(control.mid(kind == 0 ? 0x54 : 0x9c, 4).toHex(), QByteArray("11000000"));
                QCOMPARE(control.mid(kind == 0 ? 0xc0 : 0xe4, 4).toHex(), QByteArray("07000000"));
            }
        }
    }
}

/******************************************************************************
 * Function Name   : NativeBitButtonControlTests::inconsistentPixelsAreRejected
 * Description     : Reject compressed lengths and overflowing decoded offsets before any output changes.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeBitButtonControlTests::inconsistentPixelsAreRejected() //20261109-0 add
{
    HmiObject object;
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;

    object = sampleButton();
    options.uncompressedPixelBytes = 7200;
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
    options.uncompressedPixelBytes = 28800;
    options.uncompressedPixelOffset = 0xfffffff0U;
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
    options.uncompressedPixelOffset = 0;
    object.geometry.setWidth(0);
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
}

/******************************************************************************
 * Function Name   : NativeBitButtonControlTests::nativeFieldBounds
 * Description     : Prevent active timer wraparound, unresolved macro ordinals and native signed parse overflow.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeBitButtonControlTests::nativeFieldBounds() //20261109-0 add
{
    HmiObject object;
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;

    object = sampleButton();
    options.uncompressedPixelBytes = 28800;
    control = QByteArrayLiteral("preserved");
    object.bitButton.pressTimeEnabled = true;
    object.bitButton.pressTime = 656;
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
    QCOMPARE(control, QByteArrayLiteral("preserved"));
    object.bitButton.pressTimeEnabled = false;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x1bd, 3).toHex(), QByteArray("009c63"));
    object.bitButton.macroEnabled = true;
    object.bitButton.macroName = QStringLiteral("MacroA");
    options.macroIndex = 100;
    control = QByteArrayLiteral("preserved");
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
    QCOMPARE(control, QByteArrayLiteral("preserved"));
    object.bitButton.macroEnabled = false;
    object.address = QStringLiteral("LB2147483648");
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
    QCOMPARE(control, QByteArrayLiteral("preserved"));
}

/******************************************************************************
 * Function Name   : NativeBitButtonControlTests::recordedFlashAndAppearance
 * Description     : Match the four original UI flag samples and reject unrepresentable state-specific appearance.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeBitButtonControlTests::recordedFlashAndAppearance() //20261109-0 add
{
    HmiObject object;
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;

    object = sampleButton();
    object.bitButton.states[0].flash = true;
    options.uncompressedPixelBytes = 43200;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x10, 4).toHex(), QByteArray("c0a80000"));
    QCOMPARE(control.mid(0x106, 6).toHex(), QByteArray("000000000100"));

    object = sampleButton();
    object.bitButton.states[1].flash = true;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x106, 6).toHex(), QByteArray("000000000001"));

    object = sampleButton();
    object.bitButton.states[0].effect3d = true;
    object.bitButton.states[1].effect3d = true;
    options.uncompressedPixelBytes = 28800;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x106, 6).toHex(), QByteArray("000001000000"));

    object = sampleButton();
    object.bitButton.states[0].transparent = true;
    object.bitButton.states[1].transparent = true;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x106, 6).toHex(), QByteArray("000000010000"));
    object.bitButton.states[1].transparent = false;
    control = QByteArrayLiteral("preserved");
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
    QCOMPARE(control, QByteArrayLiteral("preserved"));
    object = sampleButton();
    object.bitButton.states[1].effect3d = true;
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
    QCOMPARE(control, QByteArrayLiteral("preserved"));
}

/******************************************************************************
 * Function Name   : NativeBitButtonControlTests::flashStateCounts
 * Description     : Count OFF-only mode sprites and both independent dual-mode blink frames.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeBitButtonControlTests::flashStateCounts() //20261109-0 add
{
    HmiObject object;
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;

    object = sampleButton();
    object.bitButton.states[0].flash = true;
    object.bitButton.states[1].flash = true;
    options.uncompressedPixelBytes = 57600;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x10, 4).toHex(), QByteArray("00e10000"));
    QCOMPARE(control.mid(0x10a, 2).toHex(), QByteArray("0101"));

    object.bitButton.dualMonitor = false;
    options.uncompressedPixelBytes = 28800;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    object.bitButton.states[0].flash = false;
    options.uncompressedPixelBytes = 14400;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    object.bitButton.indicator = false;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    options.uncompressedPixelBytes = 28800;
    control = QByteArrayLiteral("preserved");
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
    QCOMPARE(control, QByteArrayLiteral("preserved"));
}

/******************************************************************************
 * Function Name   : NativeBitButtonControlTests::singleModesIgnoreInactiveOnFlash
 * Description     : Preserve native OFF-only selection for disabled and single monitoring in every language.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeBitButtonControlTests::singleModesIgnoreInactiveOnFlash() //20261109-0 add
{
    HmiObject object;
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;
    int mode;
    int languages;

    // Native disabled/single streams are both3502B and match the original dual-mode OFF stream byte for byte.
    for (mode = 0; mode < 2; ++mode)
    {
        for (languages = 1; languages <= 2; ++languages)
        {
            object = sampleButton();
            object.bitButton.indicator = mode != 0;
            object.bitButton.dualMonitor = false;
            object.bitButton.states[0].flash = false;
            object.bitButton.states[1].flash = true;
            options.languageCount = u32(languages);
            options.uncompressedPixelBytes = 14400U * u32(languages);
            QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));

            control = QByteArrayLiteral("preserved");
            options.uncompressedPixelBytes = 28800U * u32(languages);
            QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
            QCOMPARE(control, QByteArrayLiteral("preserved"));

            object.bitButton.states[0].flash = true;
            QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
            object.bitButton.states[1].flash = false;
            QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
        }
    }
}

/******************************************************************************
 * Function Name   : NativeBitButtonControlTests::recordedBitNotification
 * Description     : Match the original active LB0 notification sample and preserve bit0 writes.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeBitButtonControlTests::recordedBitNotification() //20261109-0 add
{
    HmiObject object;
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;

    object = sampleButton();
    object.bitButton.notice = true;
    object.bitButton.noticeBit = true;
    options.uncompressedPixelBytes = 28800;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x14, 16).toHex(), QByteArray("00000100000000000000000001000001"));
    QCOMPARE(control.mid(0x70, 3).toHex(), QByteArray("004200"));
    QCOMPARE(control.mid(0x75, 2).toHex(), QByteArray("0001"));
    QCOMPARE(control.mid(0x78, 4).toHex(), QByteArray("00000000"));
    object.bitButton.noticeBitValue = false;
    //20261109-0 add: The original notice-bit-zero-ui sample changes only its assigned state byte.
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x14, 16).toHex(), QByteArray("00000100000000000000000000000001"));
    object.bitButton.noticeBitAddress = QStringLiteral("LB17");
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(u8(control.at(0x20)), u8(0));
    QCOMPARE(control.mid(0x78, 4).toHex(), QByteArray("11000000"));
    object.bitButton.notice = false;
    object.bitButton.noticeBitAddress = QStringLiteral("M0");
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x70, 36), QByteArray(36, char(0)));
}

/******************************************************************************
 * Function Name   : NativeBitButtonControlTests::recordedWordNotification
 * Description     : Match the original LW0 u16 notification sample and the common native numeric type mapping.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeBitButtonControlTests::recordedWordNotification() //20261109-0 add
{
    const u8 nativeTypes[5] = {4, 5, 2, 3, 8};
    HmiObject object;
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;
    int type;

    object = sampleButton();
    object.bitButton.notice = true;
    object.bitButton.noticeWord = true;
    options.uncompressedPixelBytes = 28800;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x14, 16).toHex(), QByteArray("00000100000400000000000000000002"));
    QCOMPARE(control.mid(0x44, 8), QByteArray(8, char(0)));
    QCOMPARE(control.mid(0x70, 3).toHex(), QByteArray("005700"));
    QCOMPARE(control.mid(0x75, 2).toHex(), QByteArray("0004"));
    QCOMPARE(control.mid(0x78, 4).toHex(), QByteArray("00000000"));
    //20261109-0 add: The separate original notice-word7-ui sample stores 7.0 as this f64 byte sequence.
    object.bitButton.noticeWordValue = QStringLiteral("7");
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x44, 8).toHex(), QByteArray("0000000000001c40"));
    object.bitButton.noticeWordAddress = QStringLiteral("LW17");
    for (type = 0; type < 5; ++type)
    {
        object.bitButton.noticeWordType = type;
        object.bitButton.noticeWordValue = type == 4 ? QStringLiteral("-7.25") : QStringLiteral("7");
        QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
        QCOMPARE(u8(control.at(0x19)), nativeTypes[type]);
        QCOMPARE(u8(control.at(0x76)), nativeTypes[type]);
        QCOMPARE(control.mid(0x78, 4).toHex(), QByteArray("11000000"));
        QCOMPARE(control.mid(0x44, 8).toHex(),
                 type == 4 ? QByteArray("0000000000001dc0") : QByteArray("0000000000001c40"));
    }
}

/******************************************************************************
 * Function Name   : NativeBitButtonControlTests::notificationValuesAreBounded
 * Description     : Reject malformed, nonfinite, fractional integer and overflowing notification values transactionally.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeBitButtonControlTests::notificationValuesAreBounded() //20261109-0 add
{
    const QString values[] = {QStringLiteral("text"), QStringLiteral("nan"), QStringLiteral("inf"),
                              QStringLiteral("-1"), QStringLiteral("65536"), QStringLiteral("1.5")};
    HmiObject object;
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;
    int index;

    object = sampleButton();
    object.bitButton.notice = true;
    object.bitButton.noticeWord = true;
    options.uncompressedPixelBytes = 28800;
    control = QByteArrayLiteral("preserved");
    for (index = 0; index < int(sizeof(values) / sizeof(values[0])); ++index)
    {
        object.bitButton.noticeWordValue = values[index];
        QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
        QCOMPARE(control, QByteArrayLiteral("preserved"));
    }
    object.bitButton.noticeWordType = 4;
    object.bitButton.noticeWordValue = QStringLiteral("3.5e38");
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
    QCOMPARE(control, QByteArrayLiteral("preserved"));
    object.bitButton.notice = false;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x44, 8), QByteArray(8, char(0)));
}

/******************************************************************************
 * Function Name   : externalTestRecord
 * Description     : Return a distinguishable complete callback record to detect field loss and incorrect overrides.
 * Input Parameters: word, toggle, tag
 * Return Value    : A 36-byte external address record.
 * Date            : 2026.11.09
 ******************************************************************************/
static QByteArray externalTestRecord(bool word, bool toggle, u8 tag) //20261109-0 add
{
    QByteArray record;

    record = QByteArray(36, char(0x5a));
    record[0] = 1;
    record[1] = char(word ? 0x14 : 0x10);
    record[2] = 1;
    record[5] = char(toggle ? 1 : 0);
    record[6] = char(0x33);
    record[8] = char(tag);
    return record;
}

/******************************************************************************
 * Function Name   : NativeBitButtonControlTests::externalAddressesReachEveryActiveField
 * Description     : Verify role and toggle forwarding for all seven slots and retain native notification type overrides.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeBitButtonControlTests::externalAddressesReachEveryActiveField() //20261109-0 add
{
    HmiObject object;
    NativeBitButtonControlOptions options;
    QMap<QString, QByteArray> returned;
    QStringList calls;
    QByteArray control;
    QByteArray noticeRecord;
    QString error;

    object = sampleButton();
    object.address = QStringLiteral("@write");
    object.bitButton.monitorSame = false;
    object.bitButton.monitorType = 1;
    object.bitButton.monitorWordAddress = QStringLiteral("@monitor-word");
    object.bitButton.touch.visibilityEnabled = true;
    object.bitButton.touch.bitEnabled = true;
    object.bitButton.touch.wordEnabled = true;
    object.bitButton.touch.bitAddress = QStringLiteral("@touch-bit");
    object.bitButton.touch.wordAddress = QStringLiteral("@touch-word");
    object.bitButton.visibility.visibilityEnabled = true;
    object.bitButton.visibility.bitEnabled = true;
    object.bitButton.visibility.wordEnabled = true;
    object.bitButton.visibility.bitAddress = QStringLiteral("@visible-bit");
    object.bitButton.visibility.wordAddress = QStringLiteral("@visible-word");
    object.bitButton.notice = true;
    object.bitButton.noticeWord = true;
    object.bitButton.noticeWordType = 4;
    object.bitButton.noticeWordValue = QStringLiteral("7.25");
    object.bitButton.noticeWordAddress = QStringLiteral("@notice-word");
    options.uncompressedPixelBytes = 28800;
    options.externalAddressEncoder = [&returned, &calls](const QString &address, bool word, bool toggle,
                                                        QByteArray *record, QString *) {
        *record = externalTestRecord(word, toggle, u8(returned.size() + 1));
        returned.insert(address, *record);
        calls.append(address + (word ? QStringLiteral("|word") : QStringLiteral("|bit")) +
                     (toggle ? QStringLiteral("|toggle") : QStringLiteral("|plain")));
        return true;
    };
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(calls.size(), 7);
    QVERIFY(calls.contains(QStringLiteral("@write|bit|toggle")));
    QVERIFY(calls.contains(QStringLiteral("@monitor-word|word|plain")));
    QVERIFY(calls.contains(QStringLiteral("@touch-bit|bit|plain")));
    QVERIFY(calls.contains(QStringLiteral("@touch-word|word|plain")));
    QVERIFY(calls.contains(QStringLiteral("@visible-bit|bit|plain")));
    QVERIFY(calls.contains(QStringLiteral("@visible-word|word|plain")));
    QVERIFY(calls.contains(QStringLiteral("@notice-word|word|plain")));
    QCOMPARE(control.mid(0x118, 36), returned.value(QStringLiteral("@write")));
    QCOMPARE(control.mid(0x198, 36), returned.value(QStringLiteral("@monitor-word")));
    QCOMPARE(control.mid(0x4c, 36), returned.value(QStringLiteral("@touch-bit")));
    QCOMPARE(control.mid(0xb8, 36), returned.value(QStringLiteral("@touch-word")));
    QCOMPARE(control.mid(0x94, 36), returned.value(QStringLiteral("@visible-bit")));
    QCOMPARE(control.mid(0xdc, 36), returned.value(QStringLiteral("@visible-word")));
    noticeRecord = returned.value(QStringLiteral("@notice-word"));
    noticeRecord[6] = 8;
    QCOMPARE(control.mid(0x70, 36), noticeRecord);

    object.bitButton.noticeWord = false;
    object.bitButton.noticeBit = true;
    object.bitButton.noticeBitAddress = QStringLiteral("@notice-bit");
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    noticeRecord = returned.value(QStringLiteral("@notice-bit"));
    noticeRecord[6] = 1;
    QCOMPARE(control.mid(0x70, 36), noticeRecord);
}

/******************************************************************************
 * Function Name   : NativeBitButtonControlTests::externalFailuresAreTransactional
 * Description     : Reject missing, failed and malformed providers without changing a previously compiled record.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeBitButtonControlTests::externalFailuresAreTransactional() //20261109-0 add
{
    HmiObject object;
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;
    int calls;

    object = sampleButton();
    object.address = QStringLiteral("@unverified");
    options.uncompressedPixelBytes = 28800;
    control = QByteArrayLiteral("preserved");
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
    QCOMPARE(control, QByteArrayLiteral("preserved"));
    options.externalAddressEncoder = [](const QString &, bool, bool, QByteArray *record, QString *message) {
        *record = QByteArray(36, char(1));
        *message = QStringLiteral("Unverified route");
        return false;
    };
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
    QCOMPARE(error, QStringLiteral("Unverified route"));
    QCOMPARE(control, QByteArrayLiteral("preserved"));
    options.externalAddressEncoder = [](const QString &, bool, bool, QByteArray *record, QString *) {
        *record = QByteArray(35, char(0));
        return true;
    };
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
    QCOMPARE(control, QByteArrayLiteral("preserved"));

    calls = 0;
    options.externalAddressEncoder = [&calls](const QString &, bool, bool, QByteArray *, QString *) {
        ++calls;
        return false;
    };
    object = sampleButton();
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(calls, 0);
    control = QByteArrayLiteral("preserved");
    object.bitButton.monitorSame = false;
    object.bitButton.monitorBitAddress = QStringLiteral("@late-failure");
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
    QCOMPARE(calls, 1);
    QCOMPARE(control, QByteArrayLiteral("preserved"));
}

/******************************************************************************
 * Function Name   : NativeBitButtonControlTests::multilingualPixelLengths
 * Description     : Include every language's state images while preserving the two logical monitor states.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeBitButtonControlTests::multilingualPixelLengths() //20261109-0 add
{
    HmiObject object;
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;

    object = sampleButton();
    options.languageCount = 2;
    options.uncompressedPixelBytes = 57600;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x10, 4).toHex(), QByteArray("00e10000"));
    QCOMPARE(control.mid(0x190, 4).toHex(), QByteArray("02000000"));
    options.uncompressedPixelOffset = 57600;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x0c, 8).toHex(), QByteArray("00e1000000e10000"));
    object.bitButton.states[0].flash = true;
    object.bitButton.states[1].flash = true;
    options.uncompressedPixelBytes = 115200;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x10, 4).toHex(), QByteArray("00c20100"));
    object = sampleButton();
    options.languageCount = 8;
    options.uncompressedPixelBytes = 230400;
    QVERIFY2(buildNativeBitButtonControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0x10, 4).toHex(), QByteArray("00840300"));
}

/******************************************************************************
 * Function Name   : NativeBitButtonControlTests::languageCountsAreBounded
 * Description     : Reject unsupported language counts and overflowing multilingual offsets transactionally.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeBitButtonControlTests::languageCountsAreBounded() //20261109-0 add
{
    HmiObject object;
    NativeBitButtonControlOptions options;
    QByteArray control;
    QString error;

    object = sampleButton();
    options.uncompressedPixelBytes = 28800;
    control = QByteArrayLiteral("preserved");
    options.languageCount = 0;
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
    QCOMPARE(control, QByteArrayLiteral("preserved"));
    options.languageCount = 9;
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
    QCOMPARE(control, QByteArrayLiteral("preserved"));
    options.languageCount = 2;
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
    QCOMPARE(control, QByteArrayLiteral("preserved"));
    options.uncompressedPixelBytes = 57600;
    options.uncompressedPixelOffset = 0xfffffff0U;
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
    QCOMPARE(control, QByteArrayLiteral("preserved"));
    options.uncompressedPixelOffset = 0;
    object.geometry.setSize(QSize(65535, 65535));
    QVERIFY(!buildNativeBitButtonControl(object, options, &control, &error));
    QCOMPARE(control, QByteArrayLiteral("preserved"));
}

QTEST_APPLESS_MAIN(NativeBitButtonControlTests)
#include "native_bit_button_control_tests.moc"
