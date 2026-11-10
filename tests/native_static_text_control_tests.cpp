#include "native_static_text_control.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtEndian>
#include <QtTest>

//20261109-0 add: These masks contain original compiler-assigned fields, excluding inactive stack residue.
static const char originalTextOracleJson[] = R"json([
{"name":"mixed-two-language-ui-vendor-compile-title-0","geometry":[570,105,160,40],"rawOffset":0,"rawBytes":25600,"languageCount":2,"visibilityEnabled":false,"flags":0,"fields":[{"offset":0,"hex":"1c0000"},{"offset":4,"hex":"3a026900a00028000000000000640000"},{"offset":256,"hex":"00000200"},{"offset":21,"hex":"00"}]},
{"name":"mixed-two-language-ui-vendor-compile-title-1","geometry":[390,105,160,40],"rawOffset":25600,"rawBytes":25600,"languageCount":2,"visibilityEnabled":false,"flags":0,"fields":[{"offset":0,"hex":"1c0000"},{"offset":4,"hex":"86016900a00028000064000000640000"},{"offset":256,"hex":"00000200"},{"offset":21,"hex":"00"}]},
{"name":"mixed-two-language-ui-vendor-compile-title-2","geometry":[210,105,160,40],"rawOffset":51200,"rawBytes":25600,"languageCount":2,"visibilityEnabled":false,"flags":0,"fields":[{"offset":0,"hex":"1c0000"},{"offset":4,"hex":"d2006900a000280000c8000000640000"},{"offset":256,"hex":"00000200"},{"offset":21,"hex":"00"}]},
{"name":"mixed-two-language-ui-vendor-compile-title-3","geometry":[30,105,160,40],"rawOffset":76800,"rawBytes":25600,"languageCount":2,"visibilityEnabled":false,"flags":0,"fields":[{"offset":0,"hex":"1c0000"},{"offset":4,"hex":"1e006900a0002800002c010000640000"},{"offset":256,"hex":"00000200"},{"offset":21,"hex":"00"}]},
{"name":"mixed-two-language-distinct-vendor-compile-title-0","geometry":[570,105,160,40],"rawOffset":0,"rawBytes":25600,"languageCount":2,"visibilityEnabled":false,"flags":0,"fields":[{"offset":0,"hex":"1c0000"},{"offset":4,"hex":"3a026900a00028000000000000640000"},{"offset":256,"hex":"00000200"},{"offset":21,"hex":"00"}]},
{"name":"mixed-two-language-distinct-vendor-compile-title-1","geometry":[390,105,160,40],"rawOffset":25600,"rawBytes":25600,"languageCount":2,"visibilityEnabled":false,"flags":0,"fields":[{"offset":0,"hex":"1c0000"},{"offset":4,"hex":"86016900a00028000064000000640000"},{"offset":256,"hex":"00000200"},{"offset":21,"hex":"00"}]},
{"name":"mixed-two-language-distinct-vendor-compile-title-2","geometry":[210,105,160,40],"rawOffset":51200,"rawBytes":25600,"languageCount":2,"visibilityEnabled":false,"flags":0,"fields":[{"offset":0,"hex":"1c0000"},{"offset":4,"hex":"d2006900a000280000c8000000640000"},{"offset":256,"hex":"00000200"},{"offset":21,"hex":"00"}]},
{"name":"mixed-two-language-distinct-vendor-compile-title-3","geometry":[30,105,160,40],"rawOffset":76800,"rawBytes":25600,"languageCount":2,"visibilityEnabled":false,"flags":0,"fields":[{"offset":0,"hex":"1c0000"},{"offset":4,"hex":"1e006900a0002800002c010000640000"},{"offset":256,"hex":"00000200"},{"offset":21,"hex":"00"}]},
{"name":"bit-off0","geometry":[244,112,150,50],"rawOffset":0,"rawBytes":15000,"languageCount":1,"visibilityEnabled":true,"flags":4,"fields":[{"offset":0,"hex":"1c0000"},{"offset":4,"hex":"f40070009600320000000000983a0000"},{"offset":256,"hex":"00000100"},{"offset":21,"hex":"01"},{"offset":34,"hex":"04"},{"offset":30,"hex":"00"},{"offset":148,"hex":"004200"},{"offset":153,"hex":"00"},{"offset":156,"hex":"00000000"}],"bitState":false,"bitNumber":0,"bitByte2":0},
{"name":"bit-on0","geometry":[244,112,150,50],"rawOffset":0,"rawBytes":15000,"languageCount":1,"visibilityEnabled":true,"flags":4,"fields":[{"offset":0,"hex":"1c0000"},{"offset":4,"hex":"f40070009600320000000000983a0000"},{"offset":256,"hex":"00000100"},{"offset":21,"hex":"01"},{"offset":34,"hex":"04"},{"offset":30,"hex":"01"},{"offset":148,"hex":"004200"},{"offset":153,"hex":"00"},{"offset":156,"hex":"00000000"}],"bitState":true,"bitNumber":0,"bitByte2":0},
{"name":"level3","geometry":[244,112,150,50],"rawOffset":0,"rawBytes":15000,"languageCount":1,"visibilityEnabled":true,"flags":1,"fields":[{"offset":0,"hex":"1c0000"},{"offset":4,"hex":"f40070009600320000000000983a0000"},{"offset":256,"hex":"00000100"},{"offset":21,"hex":"01"},{"offset":34,"hex":"01"},{"offset":31,"hex":"02"}],"minimumLevel":3},
{"name":"word-i16","geometry":[244,112,150,50],"rawOffset":0,"rawBytes":15000,"languageCount":1,"visibilityEnabled":true,"flags":2,"fields":[{"offset":0,"hex":"1c0000"},{"offset":4,"hex":"f40070009600320000000000983a0000"},{"offset":256,"hex":"00000100"},{"offset":21,"hex":"01"},{"offset":34,"hex":"02"},{"offset":24,"hex":"02"},{"offset":27,"hex":"00"},{"offset":52,"hex":"0000000000c05ec00000000000000000"},{"offset":220,"hex":"005700"},{"offset":225,"hex":"00"},{"offset":228,"hex":"07000000"}],"dataType":2,"comparison":0,"value":"-123","tolerance":"0","wordNumber":7,"wordByte2":0},
{"name":"word-i32","geometry":[244,112,150,50],"rawOffset":0,"rawBytes":15000,"languageCount":1,"visibilityEnabled":true,"flags":2,"fields":[{"offset":0,"hex":"1c0000"},{"offset":4,"hex":"f40070009600320000000000983a0000"},{"offset":256,"hex":"00000100"},{"offset":21,"hex":"01"},{"offset":34,"hex":"02"},{"offset":24,"hex":"03"},{"offset":27,"hex":"00"},{"offset":52,"hex":"000000c0298c67c10000000000000000"},{"offset":220,"hex":"005700"},{"offset":225,"hex":"00"},{"offset":228,"hex":"07000000"}],"dataType":3,"comparison":0,"value":"-12345678","tolerance":"0","wordNumber":7,"wordByte2":0},
{"name":"word-u16","geometry":[244,112,150,50],"rawOffset":0,"rawBytes":15000,"languageCount":1,"visibilityEnabled":true,"flags":2,"fields":[{"offset":0,"hex":"1c0000"},{"offset":4,"hex":"f40070009600320000000000983a0000"},{"offset":256,"hex":"00000100"},{"offset":21,"hex":"01"},{"offset":34,"hex":"02"},{"offset":24,"hex":"04"},{"offset":27,"hex":"00"},{"offset":52,"hex":"000000000034b2400000000000000000"},{"offset":220,"hex":"005700"},{"offset":225,"hex":"00"},{"offset":228,"hex":"07000000"}],"dataType":0,"comparison":0,"value":"4660","tolerance":"0","wordNumber":7,"wordByte2":0},
{"name":"word-u32","geometry":[244,112,150,50],"rawOffset":0,"rawBytes":15000,"languageCount":1,"visibilityEnabled":true,"flags":2,"fields":[{"offset":0,"hex":"1c0000"},{"offset":4,"hex":"f40070009600320000000000983a0000"},{"offset":256,"hex":"00000100"},{"offset":21,"hex":"01"},{"offset":34,"hex":"02"},{"offset":24,"hex":"05"},{"offset":27,"hex":"00"},{"offset":52,"hex":"000000785634b2410000000000000000"},{"offset":220,"hex":"005700"},{"offset":225,"hex":"00"},{"offset":228,"hex":"07000000"}],"dataType":1,"comparison":0,"value":"305419896","tolerance":"0","wordNumber":7,"wordByte2":0},
{"name":"v3-bit-on1","geometry":[244,112,150,50],"rawOffset":0,"rawBytes":15000,"languageCount":1,"visibilityEnabled":true,"flags":4,"fields":[{"offset":0,"hex":"1c0000"},{"offset":4,"hex":"f40070009600320000000000983a0000"},{"offset":256,"hex":"00000100"},{"offset":21,"hex":"01"},{"offset":34,"hex":"04"},{"offset":30,"hex":"01"},{"offset":148,"hex":"004264"},{"offset":153,"hex":"00"},{"offset":156,"hex":"01000000"}],"bitState":true,"bitNumber":1,"bitByte2":100},
{"name":"v3-word-f32-eq","geometry":[244,112,150,50],"rawOffset":0,"rawBytes":15000,"languageCount":1,"visibilityEnabled":true,"flags":2,"fields":[{"offset":0,"hex":"1c0000"},{"offset":4,"hex":"f40070009600320000000000983a0000"},{"offset":256,"hex":"00000100"},{"offset":21,"hex":"01"},{"offset":34,"hex":"02"},{"offset":24,"hex":"08"},{"offset":27,"hex":"02"},{"offset":52,"hex":"0000000000002940000000000000d03f"},{"offset":220,"hex":"005764"},{"offset":225,"hex":"00"},{"offset":228,"hex":"07000000"}],"dataType":4,"comparison":2,"value":"12.5","tolerance":"0.25","wordNumber":7,"wordByte2":100},
{"name":"v3-all","geometry":[244,112,150,50],"rawOffset":0,"rawBytes":15000,"languageCount":1,"visibilityEnabled":true,"flags":7,"fields":[{"offset":0,"hex":"1c0000"},{"offset":4,"hex":"f40070009600320000000000983a0000"},{"offset":256,"hex":"00000100"},{"offset":21,"hex":"01"},{"offset":34,"hex":"07"},{"offset":30,"hex":"00"},{"offset":148,"hex":"004264"},{"offset":153,"hex":"00"},{"offset":156,"hex":"01000000"},{"offset":31,"hex":"02"},{"offset":24,"hex":"08"},{"offset":27,"hex":"05"},{"offset":52,"hex":"00000000000029400000000000000000"},{"offset":220,"hex":"005764"},{"offset":225,"hex":"00"},{"offset":228,"hex":"07000000"}],"bitState":false,"bitNumber":1,"bitByte2":100,"minimumLevel":3,"dataType":4,"comparison":5,"value":"12.5","tolerance":"0","wordNumber":7,"wordByte2":100}
])json";

class NativeStaticTextControlTests : public QObject
{
    Q_OBJECT
private slots:
    void originalControlRecords_data();
    void originalControlRecords();
    void legacyAndPadding();
    void externalVisibilityAddresses();
    void externalFailuresPreserveOutput();
    void fieldBoundsPreserveOutput();
    void numericConditionsPreserveOutput();
};

/******************************************************************************
 * Function Name   : NativeStaticTextControlTests::originalControlRecords_data
 * Description     : Register two multilingual title recordings and the original visibility fixtures.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeStaticTextControlTests::originalControlRecords_data() //20261109-0 add
{
    QJsonArray cases;
    QJsonObject test;
    int index;

    QTest::addColumn<QByteArray>("testJson");
    cases = QJsonDocument::fromJson(QByteArray(originalTextOracleJson)).array();
    QVERIFY(!cases.isEmpty());
    for (index = 0; index < cases.size(); ++index)
    {
        test = cases[index].toObject();
        QTest::newRow(test[QStringLiteral("name")].toString().toUtf8().constData())
            << QJsonDocument(test).toJson(QJsonDocument::Compact);
    }
}

/******************************************************************************
 * Function Name   : NativeStaticTextControlTests::originalControlRecords
 * Description     : Match verified type28 geometry, language, visibility and local address bytes.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeStaticTextControlTests::originalControlRecords() //20261109-0 add
{
    HmiObject object;
    NativeStaticTextControlOptions options;
    QJsonObject test;
    QJsonObject field;
    QJsonArray geometry;
    QJsonArray masks;
    QByteArray control;
    QByteArray expected;
    QString error;
    int flags;
    int index;

    QFETCH(QByteArray, testJson);
    test = QJsonDocument::fromJson(testJson).object();
    geometry = test[QStringLiteral("geometry")].toArray();
    object.type = QStringLiteral("text");
    object.hasStaticTextProperties = true;
    object.geometry = QRect(geometry[0].toInt(), geometry[1].toInt(), geometry[2].toInt(), geometry[3].toInt());
    options.languageCount = u32(test[QStringLiteral("languageCount")].toInt());
    options.uncompressedPixelOffset = u32(test[QStringLiteral("rawOffset")].toDouble());
    options.uncompressedPixelBytes = u32(test[QStringLiteral("rawBytes")].toDouble());
    flags = test[QStringLiteral("flags")].toInt();
    object.staticText.visibilityEnabled = test[QStringLiteral("visibilityEnabled")].toBool();
    object.staticText.bitEnabled = (flags & 4) != 0;
    object.staticText.levelEnabled = (flags & 1) != 0;
    object.staticText.wordEnabled = (flags & 2) != 0;
    object.staticText.bitState = test[QStringLiteral("bitState")].toBool();
    object.staticText.bitAddress = QStringLiteral("LB%1").arg(test[QStringLiteral("bitNumber")].toInt());
    object.staticText.minimumLevel = test[QStringLiteral("minimumLevel")].toInt(1);
    object.staticText.wordAddress = QStringLiteral("LW%1").arg(test[QStringLiteral("wordNumber")].toInt());
    object.staticText.dataType = test[QStringLiteral("dataType")].toInt();
    object.staticText.comparison = test[QStringLiteral("comparison")].toInt();
    object.staticText.compareValue = test[QStringLiteral("value")].toString(QStringLiteral("0"));
    object.staticText.tolerance = test[QStringLiteral("tolerance")].toString(QStringLiteral("0"));
    options.bitAddressByte2 = u8(test[QStringLiteral("bitByte2")].toInt());
    options.wordAddressByte2 = u8(test[QStringLiteral("wordByte2")].toInt());
    QVERIFY2(buildNativeStaticTextControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.size(), 640);
    masks = test[QStringLiteral("fields")].toArray();
    for (index = 0; index < masks.size(); ++index)
    {
        field = masks[index].toObject();
        expected = QByteArray::fromHex(field[QStringLiteral("hex")].toString().toLatin1());
        QCOMPARE(control.mid(field[QStringLiteral("offset")].toInt(), expected.size()), expected);
    }
    QCOMPARE(control[3], char(0));
    QCOMPARE(control.mid(0x104), QByteArray(640 - 0x104, char(0)));
}

/******************************************************************************
 * Function Name   : NativeStaticTextControlTests::legacyAndPadding
 * Description     : Encode legacy text with default dormant fields and the verified multilingual raw size.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeStaticTextControlTests::legacyAndPadding() //20261109-0 add
{
    HmiObject object;
    NativeStaticTextControlOptions options;
    QByteArray control;
    QString error;

    object.type = QStringLiteral("text");
    object.geometry = QRect(1, 2, 160, 40);
    object.staticText.visibilityEnabled = true;
    object.staticText.wordEnabled = true;
    object.staticText.wordAddress = QStringLiteral("@dormant");
    options.languageCount = 2;
    options.uncompressedPixelOffset = 51200;
    options.uncompressedPixelBytes = 25600;
    QVERIFY2(buildNativeStaticTextControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(control.mid(0, 4).toHex(), QByteArray("1c000000"));
    QCOMPARE(control.mid(0x14, 0xec), QByteArray(0xec, char(0)));
    QCOMPARE(control.mid(0x100, 4).toHex(), QByteArray("00000200"));
    QCOMPARE(control.mid(0x104), QByteArray(380, char(0)));
    options.languageCount = 8;
    options.uncompressedPixelBytes = 102400;
    QVERIFY(buildNativeStaticTextControl(object, options, &control, &error));
    QCOMPARE(control.mid(0x102, 2).toHex(), QByteArray("0800"));
}

/******************************************************************************
 * Function Name   : NativeStaticTextControlTests::externalVisibilityAddresses
 * Description     : Forward both read roles and preserve every byte of verified external visibility slots.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeStaticTextControlTests::externalVisibilityAddresses() //20261109-0 add
{
    HmiObject object;
    NativeStaticTextControlOptions options;
    QByteArray control;
    QByteArray bitRecord;
    QByteArray wordRecord;
    QString error;
    QStringList calls;

    object.type = QStringLiteral("text");
    object.geometry = QRect(0, 0, 3, 2);
    object.hasStaticTextProperties = true;
    object.staticText.visibilityEnabled = true;
    object.staticText.bitEnabled = true;
    object.staticText.wordEnabled = true;
    object.staticText.bitAddress = QStringLiteral("@bit");
    object.staticText.wordAddress = QStringLiteral("@word");
    options.uncompressedPixelBytes = 12;
    bitRecord = QByteArray(36, char(0x31));
    wordRecord = QByteArray(36, char(0x42));
    options.externalAddressEncoder = [&calls, &bitRecord, &wordRecord](const QString &address, bool word,
                                                                    bool toggle, QByteArray *record, QString *) {
        calls.append(address + (word ? QStringLiteral("/word") : QStringLiteral("/bit")) +
                     (toggle ? QStringLiteral("/toggle") : QStringLiteral("/read")));
        *record = word ? wordRecord : bitRecord;
        return true;
    };
    QVERIFY2(buildNativeStaticTextControl(object, options, &control, &error), qPrintable(error));
    QCOMPARE(calls, QStringList({QStringLiteral("@bit/bit/read"), QStringLiteral("@word/word/read")}));
    QCOMPARE(control.mid(0x94, 36), bitRecord);
    QCOMPARE(control.mid(0xdc, 36), wordRecord);
    QCOMPARE(control.mid(0x100, 4).toHex(), QByteArray("00000100"));
}

/******************************************************************************
 * Function Name   : NativeStaticTextControlTests::externalFailuresPreserveOutput
 * Description     : Preserve an existing record after missing, malformed or late failing external encoders.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeStaticTextControlTests::externalFailuresPreserveOutput() //20261109-0 add
{
    HmiObject object;
    NativeStaticTextControlOptions options;
    QByteArray control;
    QString error;

    object.type = QStringLiteral("text");
    object.geometry = QRect(0, 0, 3, 2);
    object.hasStaticTextProperties = true;
    object.staticText.visibilityEnabled = true;
    object.staticText.bitEnabled = true;
    object.staticText.bitAddress = QStringLiteral("@bit");
    options.uncompressedPixelBytes = 12;
    control = QByteArrayLiteral("preserved");
    QVERIFY(!buildNativeStaticTextControl(object, options, &control, &error));
    QCOMPARE(control, QByteArrayLiteral("preserved"));
    options.externalAddressEncoder = [](const QString &, bool, bool, QByteArray *record, QString *) {
        *record = QByteArray(35, char(0));
        return true;
    };
    QVERIFY(!buildNativeStaticTextControl(object, options, &control, &error));
    QCOMPARE(control, QByteArrayLiteral("preserved"));
    options.externalAddressEncoder = [](const QString &, bool word, bool, QByteArray *record, QString *message) {
        *record = QByteArray(36, char(0));
        if (word)
        {
            *message = QStringLiteral("Unverified word route");
            return false;
        }
        return true;
    };
    object.staticText.wordEnabled = true;
    object.staticText.wordAddress = QStringLiteral("@word");
    QVERIFY(!buildNativeStaticTextControl(object, options, &control, &error));
    QCOMPARE(error, QStringLiteral("Unverified word route"));
    QCOMPARE(control, QByteArrayLiteral("preserved"));
}

/******************************************************************************
 * Function Name   : NativeStaticTextControlTests::fieldBoundsPreserveOutput
 * Description     : Reject language, geometry, raw length and offset overflows without partial output.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeStaticTextControlTests::fieldBoundsPreserveOutput() //20261109-0 add
{
    HmiObject object;
    NativeStaticTextControlOptions options;
    QByteArray control;
    QString error;

    object.type = QStringLiteral("text");
    object.geometry = QRect(0, 0, 160, 40);
    options.uncompressedPixelBytes = 12800;
    control = QByteArrayLiteral("preserved");
    options.languageCount = 0;
    QVERIFY(!buildNativeStaticTextControl(object, options, &control, &error));
    options.languageCount = 9;
    QVERIFY(!buildNativeStaticTextControl(object, options, &control, &error));
    options.languageCount = 2;
    QVERIFY(!buildNativeStaticTextControl(object, options, &control, &error));
    options.uncompressedPixelBytes = 25600;
    options.uncompressedPixelOffset = 0xfffffff0U;
    QVERIFY(!buildNativeStaticTextControl(object, options, &control, &error));
    options.uncompressedPixelOffset = 0;
    object.geometry = QRect(0, 0, 65535, 65535);
    QVERIFY(!buildNativeStaticTextControl(object, options, &control, &error));
    object.geometry = QRect(-1, 0, 160, 40);
    QVERIFY(!buildNativeStaticTextControl(object, options, &control, &error));
    object.type = QStringLiteral("switch");
    QVERIFY(!buildNativeStaticTextControl(object, options, &control, &error));
    QCOMPARE(control, QByteArrayLiteral("preserved"));
    QVERIFY(!buildNativeStaticTextControl(object, options, nullptr, &error));
}

/******************************************************************************
 * Function Name   : NativeStaticTextControlTests::numericConditionsPreserveOutput
 * Description     : Validate active word values and addresses while ignoring inactive permission drafts.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeStaticTextControlTests::numericConditionsPreserveOutput() //20261109-0 add
{
    HmiObject object;
    NativeStaticTextControlOptions options;
    QByteArray control;
    QString error;

    object.type = QStringLiteral("text");
    object.geometry = QRect(0, 0, 3, 2);
    object.hasStaticTextProperties = true;
    object.staticText.visibilityEnabled = true;
    object.staticText.wordEnabled = true;
    object.staticText.wordAddress = QStringLiteral("LW0");
    object.staticText.compareValue = QStringLiteral("nan");
    options.uncompressedPixelBytes = 12;
    control = QByteArrayLiteral("preserved");
    QVERIFY(!buildNativeStaticTextControl(object, options, &control, &error));
    object.staticText.compareValue = QStringLiteral("65536");
    QVERIFY(!buildNativeStaticTextControl(object, options, &control, &error));
    object.staticText.compareValue = QStringLiteral("1.5");
    QVERIFY(!buildNativeStaticTextControl(object, options, &control, &error));
    object.staticText.compareValue = QStringLiteral("7");
    object.staticText.wordAddress = QStringLiteral("LW2147483648");
    QVERIFY(!buildNativeStaticTextControl(object, options, &control, &error));
    QCOMPARE(control, QByteArrayLiteral("preserved"));
    object.staticText.visibilityEnabled = false;
    QVERIFY(buildNativeStaticTextControl(object, options, &control, &error));
    QCOMPARE(control.mid(0x14, 0xec), QByteArray(0xec, char(0)));
}

QTEST_APPLESS_MAIN(NativeStaticTextControlTests)
#include "native_static_text_control_tests.moc"
