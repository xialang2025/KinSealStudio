#include "native_lamp_control.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QtEndian>
#include <QtTest>

#include <limits>

class NativeLampControlTests : public QObject
{
    Q_OBJECT
private slots:
    void originalAssignedFields_data();
    void originalAssignedFields();
    void originalSharedLanguageFixture();
    void multiStateAndLanguages_data();
    void multiStateAndLanguages();
    void visibilityAndReadOnlyExternalAddress();
    void unsupportedFieldsAndBoundsPreserveOutput();
};

/******************************************************************************
 * Function Name   : lampSample
 * Description     : Construct a lamp with the geometry and persisted defaults observed in the native UI seed.
 * Input Parameters: None
 * Return Value    : Read-only lamp object.
 * Date            : 2026.11.09
 ******************************************************************************/
static HmiObject lampSample() //20261109-8 add
{
    HmiObject object;

    object.type = QStringLiteral("switch");
    object.hasBitButtonProperties = true;
    object.hasLampProperties = true;
    object.bitButton.monitorSame = false;
    object.bitButton.bitIndex = 0;
    object.geometry = QRect(244, 112, 120, 60);
    return object;
}

/******************************************************************************
 * Function Name   : NativeLampControlTests::originalAssignedFields_data
 * Description     : Select independently captured original UI exports instead of generated expected records.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeLampControlTests::originalAssignedFields_data() //20261109-8 add
{
    QTest::addColumn<QString>("fixture");
    QTest::addColumn<int>("mode");
    QTest::addColumn<int>("dataType");
    QTest::addColumn<int>("states");
    QTest::addColumn<int>("wordBit");
    QTest::addColumn<int>("flashState");
    QTest::newRow("default-bit") << QStringLiteral("lamp-default-r1") << 1 << 0 << 2 << -1 << -1;
    QTest::newRow("word-bit7") << QStringLiteral("lamp-wordbit7-r1") << 1 << 0 << 2 << 7 << -1;
    QTest::newRow("word-bit15") << QStringLiteral("lamp-wordbit15-r1") << 1 << 0 << 2 << 15 << -1;
    QTest::newRow("multi-u16") << QStringLiteral("lamp-multi5-u16-r4") << 2 << 0 << 5 << -1 << -1;
    QTest::newRow("multi-u32") << QStringLiteral("lamp-multi5-u32-r1") << 2 << 1 << 5 << -1 << -1;
    QTest::newRow("unchecked-becomes-multi") << QStringLiteral("lamp-none-r1") << 2 << 0 << 2 << -1 << -1;
    QTest::newRow("multi-flash0") << QStringLiteral("lamp-multi5-flash0-r1") << 2 << 0 << 5 << -1 << 0;
    QTest::newRow("multi-flash2") << QStringLiteral("lamp-multi5-flash2-r1") << 2 << 0 << 5 << -1 << 2;
    QTest::newRow("dual-flash0") << QStringLiteral("lamp-dual-flash0-r1") << 1 << 0 << 2 << -1 << 0;
    QTest::newRow("multi2-flash0") << QStringLiteral("lamp-multi2-flash0-r1") << 2 << 0 << 2 << -1 << 0;
}

/******************************************************************************
 * Function Name   : NativeLampControlTests::originalAssignedFields
 * Description     : Compare every active monitor field while excluding documented native stack residues.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeLampControlTests::originalAssignedFields() //20261110-7 change: Durable fixtures are fresh native recompiled outputs with retained source identity.
{
    HmiObject object = lampSample();
    NativeBitButtonControlOptions options;
    QByteArray encoded;
    QByteArray native;
    QString error;
    QFile file;
    int offset;
    QFETCH(QString, fixture);
    QFETCH(int, mode);
    QFETCH(int, dataType);
    QFETCH(int, states);
    QFETCH(int, wordBit);
    QFETCH(int, flashState);

    object.lamp.monitorMode = mode;
    object.lamp.multiDataType = dataType;
    object.lamp.multiStateCount = states;
    object.lamp.ensureStateCount(states, object.bitButton);
    object.bitButton.monitorType = wordBit < 0 ? 0 : 1;
    object.bitButton.bitIndex = qMax(0, wordBit);
    if (flashState >= 0) { object.lamp.state(object.bitButton, flashState)->flash = true; }
    options.uncompressedPixelBytes = u32(14400 * (states + (mode == 1 && flashState >= 0 ? 1 : 0)));
    file.setFileName(QFileInfo(QString::fromUtf8(__FILE__)).dir().absoluteFilePath(
                         QStringLiteral("fixtures/lamp-control/%1.dat").arg(fixture)));
    QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(file.fileName()));
    native = file.readAll();
    QCOMPARE(native.size(), 640);
    QVERIFY2(buildNativeLampControl(object, options, &encoded, &error), qPrintable(error));
    QCOMPARE(encoded.left(3), native.left(3));
    QCOMPARE(encoded.mid(4, 16), native.mid(4, 16));
    QCOMPARE(encoded.mid(0x106, 6), native.mid(0x106, 6));
    QCOMPARE(encoded.mid(0x180, 2), native.mid(0x180, 2));
    QCOMPARE(encoded.mid(0x184, 4), native.mid(0x184, 4));
    QCOMPARE(encoded.mid(0x190, 4), native.mid(0x190, 4));
    // Bit-index drafts vary after native appearance edits and affect only word-address dual monitoring.
    if (mode == 1 && wordBit >= 0) { QCOMPARE(encoded.mid(0x188, 4), native.mid(0x188, 4)); }
    if (mode == 2) { QCOMPARE(encoded.mid(0x18c, 4), native.mid(0x18c, 4)); }
    for (offset = 0; offset < 36; ++offset)
    {
        if (offset == 0 || offset == 1 || offset == 2 || offset == 5 || (offset >= 8 && offset <= 11))
        {
            QCOMPARE(encoded[0x198 + offset], native[0x198 + offset]);
        }
    }
    QCOMPARE(encoded.mid(0x10c, 0x74), QByteArray(0x74, char(0)));
    QCOMPARE(encoded[0x14], char(0));
    QCOMPARE(encoded[0x16], char(0));
}

/******************************************************************************
 * Function Name   : NativeLampControlTests::originalSharedLanguageFixture
 * Description     : Match the original five-frame export with global shared language text and a rectangular state2.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeLampControlTests::originalSharedLanguageFixture() //20261110-7 change: This independent native output uses an explicitly labelled new product input.
{
    HmiObject object = lampSample();
    NativeBitButtonControlOptions options;
    QByteArray encoded;
    QByteArray native;
    QString error;
    QFile file;
    BitButtonState *appearance;
    int state;

    object.geometry = QRect(40, 40, 180, 60);
    object.lamp.monitorMode = 2;
    object.lamp.multiStateCount = 5;
    object.lamp.ensureStateCount(5, object.bitButton);
    for (state = 0; state < 5; ++state)
    {
        appearance = object.lamp.state(object.bitButton, state);
        appearance->sameLanguage = true;
        appearance->sharedText = QStringLiteral("STATE %1").arg(state);
    }
    object.lamp.state(object.bitButton, 2)->useImage = false;
    options.languageCount = 2;
    options.uncompressedPixelBytes = 108000;
    file.setFileName(QFileInfo(QString::fromUtf8(__FILE__)).dir().absoluteFilePath(
                         QStringLiteral("fixtures/lamp-control/product-shared-lamp-new-native.dat")));
    QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(file.fileName()));
    native = file.readAll();
    QCOMPARE(native.size(), 640);
    QVERIFY2(buildNativeLampControl(object, options, &encoded, &error), qPrintable(error));
    QCOMPARE(encoded.mid(4, 16), native.mid(4, 16));
    QCOMPARE(encoded.mid(0x106, 6), native.mid(0x106, 6));
    QCOMPARE(encoded.mid(0x180, 2), native.mid(0x180, 2));
    QCOMPARE(encoded.mid(0x184, 4), native.mid(0x184, 4));
    QCOMPARE(encoded.mid(0x18c, 8), native.mid(0x18c, 8));
    QCOMPARE(encoded[0x1bc], native[0x1bc]);
    QCOMPARE(encoded[0x1bc], char(1));
}

/******************************************************************************
 * Function Name   : NativeLampControlTests::multiStateAndLanguages_data
 * Description     : Exercise the full original UI state-count and language bounds.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeLampControlTests::multiStateAndLanguages_data() //20261109-8 add
{
    int states;
    int languages;

    QTest::addColumn<int>("states");
    QTest::addColumn<int>("languages");
    for (states = 2; states <= 32; ++states)
    {
        for (languages = 1; languages <= 8; languages += 7)
        {
            QTest::newRow(qPrintable(QStringLiteral("states%1-languages%2").arg(states).arg(languages))) << states << languages;
        }
    }
}

/******************************************************************************
 * Function Name   : NativeLampControlTests::multiStateAndLanguages
 * Description     : Keep multi-state flash drafts from creating unobserved device image streams.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeLampControlTests::multiStateAndLanguages() //20261109-8 add
{
    HmiObject object = lampSample();
    NativeBitButtonControlOptions options;
    QByteArray encoded;
    QString error;
    QFETCH(int, states);
    QFETCH(int, languages);

    object.lamp.monitorMode = 2;
    object.lamp.multiStateCount = states;
    object.lamp.ensureStateCount(states, object.bitButton);
    object.lamp.state(object.bitButton, states - 1)->flash = true;
    options.languageCount = u32(languages);
    options.uncompressedPixelOffset = 100;
    options.uncompressedPixelBytes = u32(14400 * states * languages);
    QVERIFY2(buildNativeLampControl(object, options, &encoded, &error), qPrintable(error));
    QCOMPARE(qFromLittleEndian<u32>(encoded.constData() + 0x10), options.uncompressedPixelBytes);
    QCOMPARE(qFromLittleEndian<u32>(encoded.constData() + 0x190), u32(states));
    QCOMPARE(qFromLittleEndian<u32>(encoded.constData() + 0x0c), u32(100));
}

/******************************************************************************
 * Function Name   : NativeLampControlTests::visibilityAndReadOnlyExternalAddress
 * Description     : Retain shared visibility while requesting only read-role external monitoring records.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeLampControlTests::visibilityAndReadOnlyExternalAddress() //20261109-8 add
{
    HmiObject object = lampSample();
    NativeBitButtonControlOptions options;
    QByteArray encoded;
    QString error;
    int calls = 0;
    bool observedWord = false;
    bool observedToggle = true;

    object.bitButton.visibility.visibilityEnabled = true;
    object.bitButton.visibility.bitEnabled = true;
    object.bitButton.visibility.bitAddress = QStringLiteral("LB12");
    object.lamp.monitorMode = 2;
    object.lamp.multiDataType = 1;
    object.lamp.multiMonitorAddress = QStringLiteral("@PLC|D12");
    options.uncompressedPixelBytes = 28800;
    options.externalAddressEncoder = [&](const QString &, bool word, bool toggle, QByteArray *record, QString *)
    {
        ++calls; observedWord = word; observedToggle = toggle;
        *record = QByteArray(36, char(0)); (*record)[0] = 5; (*record)[5] = 1;
        qToLittleEndian<u32>(12U, record->data() + 8);
        return true;
    };
    QVERIFY2(buildNativeLampControl(object, options, &encoded, &error), qPrintable(error));
    QCOMPARE(calls, 1);
    QVERIFY(observedWord);
    QVERIFY(!observedToggle);
    QCOMPARE(encoded[0x19d], char(0));
    QCOMPARE(encoded[0x15], char(1));
    QCOMPARE(qFromLittleEndian<u32>(encoded.constData() + 0x9c), 12U);
}

/******************************************************************************
 * Function Name   : NativeLampControlTests::unsupportedFieldsAndBoundsPreserveOutput
 * Description     : Reject latent write features and inconsistent image bounds transactionally.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeLampControlTests::unsupportedFieldsAndBoundsPreserveOutput() //20261109-8 add
{
    HmiObject object;
    NativeBitButtonControlOptions options;
    QByteArray encoded("unchanged");
    QString error;
    int scenario;

    for (scenario = 0; scenario < 14; ++scenario)
    {
        object = lampSample(); options = NativeBitButtonControlOptions();
        options.uncompressedPixelBytes = 28800;
        if (scenario == 0) { object.lamp.monitorMode = 0; }
        if (scenario == 1) { object.bitButton.monitorSame = true; }
        if (scenario == 2) { object.bitButton.macroEnabled = true; }
        if (scenario == 3) { object.bitButton.notice = true; }
        if (scenario == 4) { object.bitButton.touch.visibilityEnabled = true; }
        if (scenario == 5) { object.bitButton.pressTimeEnabled = true; }
        if (scenario == 6) { options.uncompressedPixelBytes = 14400; }
        if (scenario == 7) { options.uncompressedPixelOffset = std::numeric_limits<u32>::max() - 10; }
        if (scenario == 8) { object.bitButton.monitorBitAddress = QStringLiteral("LW0"); }
        if (scenario == 9) { object.lamp.monitorMode = 2; object.lamp.multiStateCount = 33; }
        if (scenario == 10) { object.bitButton.monitorBitAddress = QStringLiteral("LB4097"); }
        if (scenario == 11) { object.lamp.monitorMode = 2; object.lamp.multiDataType = 1; object.lamp.multiMonitorAddress = QStringLiteral("LW4096"); }
        if (scenario == 12) { object.bitButton.monitorBitAddress = QStringLiteral("LB18446744073709551615"); }
        if (scenario == 13) { object.bitButton.states[1].sameLanguage = true; }
        QVERIFY2(!buildNativeLampControl(object, options, &encoded, &error), qPrintable(QString::number(scenario)));
        QCOMPARE(encoded, QByteArray("unchanged"));
        QVERIFY(!error.isEmpty());
    }
}

QTEST_MAIN(NativeLampControlTests)
#include "native_lamp_control_tests.moc"
