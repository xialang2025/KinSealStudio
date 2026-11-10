#include "native_function_button_control.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QtEndian>
#include <QtTest>

class NativeFunctionButtonControlTests : public QObject
{
    Q_OBJECT
private slots:
    void originalMatrix_data();
    void originalMatrix();
    void allLanguageAndFrameCounts();
    void permissionsNotificationsAndZeroPress();
    void rejectedFieldsPreserveOutput();
};

/******************************************************************************
 * Function Name   : functionSample
 * Description     : Create the geometry used by the true native UI matrix.
 * Input Parameters: None
 * Return Value    : Function-button model with inactive operation address.
 * Date            : 2026.11.10
 ******************************************************************************/
static HmiObject functionSample() //20261110-0 add
{
    HmiObject object;

    object.type = QStringLiteral("switch");
    object.hasBitButtonProperties = true;
    object.hasFunctionButtonProperties = true;
    object.geometry = QRect(244, 112, 120, 60);
    object.bitButton.indicator = false;
    object.bitButton.dualMonitor = false;
    return object;
}

/******************************************************************************
 * Function Name   : functionFixture
 * Description     : Read immutable native export evidence relative to this test file.
 * Input Parameters: name
 * Return Value    : Complete saved bytes, or empty on failure.
 * Date            : 2026.11.10
 ******************************************************************************/
static QByteArray functionFixture(const QString &name) //20261110-0 add
{
    QFile file(QFileInfo(QString::fromUtf8(__FILE__)).dir().absoluteFilePath(
                   QStringLiteral("fixtures/function-button/") + name));

    if (!file.open(QIODevice::ReadOnly)) { return QByteArray(); }
    return file.readAll();
}

/******************************************************************************
 * Function Name   : NativeFunctionButtonControlTests::originalMatrix_data
 * Description     : Select every original function and subtype, retaining unsupported Unicode as a rejection case.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeFunctionButtonControlTests::originalMatrix_data() //20261110-0 add
{
    QJsonArray specs;
    QJsonObject spec;
    int index;

    QTest::addColumn<int>("recordIndex");
    QTest::addColumn<QJsonObject>("specification");
    specs = QJsonDocument::fromJson(functionFixture(QStringLiteral("matrix-specs.json"))).array();
    QVERIFY(!specs.isEmpty());
    for (index = 0; index < specs.size(); ++index)
    {
        spec = specs.at(index).toObject();
        if (spec.value(QStringLiteral("text")).toString().size() > 1) { continue; }
        QTest::newRow(qPrintable(QStringLiteral("original-%1-f%2-t%3").arg(index)
                               .arg(spec.value(QStringLiteral("function")).toInt())
                               .arg(spec.value(QStringLiteral("type")).toInt()))) << index << spec;
    }
}

/******************************************************************************
 * Function Name   : NativeFunctionButtonControlTests::originalMatrix
 * Description     : Compare assigned original fields while excluding documented native stack residues.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeFunctionButtonControlTests::originalMatrix() //20261110-0 add
{
    HmiObject object = functionSample();
    NativeBitButtonControlOptions options;
    QByteArray native;
    QByteArray encoded;
    QString error;
    QString text;
    QFETCH(int, recordIndex);
    QFETCH(QJsonObject, specification);

    native = functionFixture(QStringLiteral("matrix-control.dat")).mid(recordIndex * 640, 640);
    QCOMPARE(native.size(), 640);
    object.functionButton.function = specification.value(QStringLiteral("function")).toInt();
    object.functionButton.type = specification.value(QStringLiteral("type")).toInt();
    object.functionButton.keyboardKey = specification.value(QStringLiteral("key")).toInt();
    // Native Space clicks are persisted as Enter; key8 itself remains an explicit separately validated field.
    if (object.functionButton.keyboardKey == 8) { object.functionButton.keyboardKey = 0; }
    text = specification.value(QStringLiteral("text")).toString();
    object.functionButton.keyboardText = text.left(1);
    object.functionButton.pressedEffect = specification.value(QStringLiteral("pressed")).toBool();
    object.bitButton.states[0].flash = specification.value(QStringLiteral("flash")).toBool();
    object.bitButton.states[0].sameLanguage = specification.value(QStringLiteral("same")).toBool();
    options.uncompressedPixelOffset = qFromLittleEndian<u32>(native.constData() + 12);
    options.uncompressedPixelBytes = qFromLittleEndian<u32>(native.constData() + 16);
    QVERIFY2(buildNativeFunctionButtonControl(object, options, &encoded, &error), qPrintable(error));
    QCOMPARE(encoded.left(3), native.left(3));
    QCOMPARE(encoded.mid(4, 16), native.mid(4, 16));
    QCOMPARE(encoded.mid(0x103, 1), native.mid(0x103, 1));
    QCOMPARE(encoded.mid(0x106, 5), native.mid(0x106, 5));
    QCOMPARE(encoded.mid(0x118, 8), native.mid(0x118, 8));
    if (object.functionButton.function == 14)
    {
        QCOMPARE(encoded.mid(0x120, 1), native.mid(0x120, 1));
        if (object.functionButton.keyboardKey == 9 && !text.isEmpty())
        {
            QCOMPARE(encoded.mid(0x121, 1), native.mid(0x121, 1));
        }
    }
    QCOMPARE(encoded.mid(0x1bc, 4), native.mid(0x1bc, 4));
}

/******************************************************************************
 * Function Name   : NativeFunctionButtonControlTests::allLanguageAndFrameCounts
 * Description     : Exercise every pressed/flash/shared-language combination and byte-bound failure.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeFunctionButtonControlTests::allLanguageAndFrameCounts() //20261110-0 add
{
    HmiObject object;
    NativeBitButtonControlOptions options;
    QByteArray bytes;
    QString error;
    int mask;

    for (mask = 0; mask < 8; ++mask)
    {
        object = functionSample();
        object.functionButton.pressedEffect = (mask & 1) != 0;
        object.bitButton.states[0].flash = (mask & 2) != 0;
        object.bitButton.states[0].sameLanguage = (mask & 4) != 0;
        options.languageCount = 3;
        options.uncompressedPixelBytes = 14400U * u32(1 + ((mask & 1) ? 1 : 0) + ((mask & 2) ? 1 : 0)) *
                                         u32((mask & 4) ? 1 : 3);
        QVERIFY2(buildNativeFunctionButtonControl(object, options, &bytes, &error), qPrintable(error));
        QCOMPARE(bytes.size(), 640);
        QCOMPARE(qFromLittleEndian<u32>(bytes.constData() + 16), options.uncompressedPixelBytes);
        ++options.uncompressedPixelBytes;
        QVERIFY(!buildNativeFunctionButtonControl(object, options, &bytes, &error));
    }
}

/******************************************************************************
 * Function Name   : NativeFunctionButtonControlTests::permissionsNotificationsAndZeroPress
 * Description     : Verify shared writable notifications and valid zero hold-time through the subtype3 wrapper.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeFunctionButtonControlTests::permissionsNotificationsAndZeroPress() //20261110-0 add
{
    HmiObject object = functionSample();
    NativeBitButtonControlOptions options;
    QByteArray bytes;
    QString error;

    options.uncompressedPixelBytes = 14400;
    object.bitButton.pressTimeEnabled = true;
    object.bitButton.pressTime = 0;
    object.bitButton.notice = true;
    object.bitButton.noticeBit = true;
    object.bitButton.noticeWord = false;
    object.bitButton.noticeBitAddress = QStringLiteral("LB17");
    QVERIFY2(buildNativeFunctionButtonControl(object, options, &bytes, &error), qPrintable(error));
    QCOMPARE(u8(bytes[0x1bd]), u8(1));
    QCOMPARE(qFromLittleEndian<u16>(bytes.constData() + 0x1be), u16(0));
    QCOMPARE(u8(bytes[0x16]), u8(1));
    QCOMPARE(u8(bytes[0x23]), u8(1));
    QCOMPARE(u8(bytes[0x76]), u8(1));
    QCOMPARE(qFromLittleEndian<u32>(bytes.constData() + 0x78), u32(17));
}

/******************************************************************************
 * Function Name   : NativeFunctionButtonControlTests::rejectedFieldsPreserveOutput
 * Description     : Reject unsupported macro, malformed keyboard, invalid language and subtype conflicts atomically.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeFunctionButtonControlTests::rejectedFieldsPreserveOutput() //20261110-0 add
{
    HmiObject object;
    NativeBitButtonControlOptions options;
    QByteArray bytes("preserve-me");
    QString error;
    int failure;

    options.uncompressedPixelBytes = 14400;
    for (failure = 0; failure < 7; ++failure)
    {
        object = functionSample();
        if (failure == 0) { object.bitButton.macroEnabled = true; }
        if (failure == 1) { object.bitButton.pressTime = 256; }
        if (failure == 2) { object.hasWordButtonProperties = true; }
        if (failure == 3) { object.functionButton.function = 18; }
        if (failure == 4) { object.functionButton.function = 13; object.functionButton.type = 1; }
        if (failure == 5) { object.functionButton.function = 14; object.functionButton.keyboardKey = 9; object.functionButton.keyboardText = QString(QChar(0xd83d)); }
        if (failure == 6) { object.geometry = QRect(-1, 0, 120, 60); }
        QVERIFY(!buildNativeFunctionButtonControl(object, options, &bytes, &error));
        QCOMPARE(bytes, QByteArray("preserve-me"));
        QVERIFY(!error.isEmpty());
    }
}

QTEST_APPLESS_MAIN(NativeFunctionButtonControlTests)
#include "native_function_button_control_tests.moc"
