#include "simulation_runtime.h"

#include <QJsonDocument>
#include <QtTest>
#include <cmath>
#include <limits>

//20261114-6 add: These tests stimulate only memory and never create a window, process, timer or physical connection.
class SimulationRuntimeTests : public QObject
{
    Q_OBJECT
private slots:
    void immutableSessionAndTransactionalStart();
    void localRolesAndSpanIsolation();
    void typedMemory_data();
    void typedMemory();
    void rawWordOrderAndInvalidInterpretations();
    void bitOperations_data();
    void bitOperations();
    void momentarySafety_data();
    void momentarySafety();
    void noticesAndOperationAreAtomic();
    void wordOperations_data();
    void wordOperations();
    void inputLifecycleAndAtomicNotices();
    void floatInputAndWideNotice();
    void inputLostAccessAndPasswordLog();
    void permissionComparisons_data();
    void permissionComparisons();
    void bitLevelAndWordBitMonitoring();
    void multiLampUnknownState();
    void languageLogoutAndMacroSkip();
    void capabilityRefusals_data();
    void capabilityRefusals();
    void boundedEventsAndRestart();
    void boundedCapabilityDetails();
};

/******************************************************************************
 * Function Name   : bitObject
 * Description     : Construct a valid typed switch for memory-only tests.
 * Input Parameters: id, operation, address
 * Return Value    : Detached bit-button object.
 * Date            : 2026.11.14
 ******************************************************************************/
static HmiObject bitObject(const QString &id, int operation = 3, const QString &address = QStringLiteral("LB0")) //20261114-6 add
{
    HmiObject object;

    object.id = id;
    object.name = id;
    object.type = QStringLiteral("switch");
    object.address = address;
    object.geometry = QRect(10, 10, 120, 60);
    object.hasBitButtonProperties = true;
    object.bitButton.operation = operation;
    return object;
}

/******************************************************************************
 * Function Name   : wordObject
 * Description     : Construct an independent word operation with valid shared single-state artwork.
 * Input Parameters: id, operation, type
 * Return Value    : Detached word-button object.
 * Date            : 2026.11.14
 ******************************************************************************/
static HmiObject wordObject(const QString &id, int operation = 0, int type = 0) //20261114-6 add
{
    HmiObject object = bitObject(id);

    object.hasWordButtonProperties = true;
    object.address = QStringLiteral("LW0");
    object.bitButton.indicator = false;
    object.bitButton.dualMonitor = false;
    object.wordButton.resetDefaultsForDataType(type);
    object.wordButton.operation = operation;
    return object;
}

/******************************************************************************
 * Function Name   : functionObject
 * Description     : Construct a local language or logout button without an operation address.
 * Input Parameters: id, function
 * Return Value    : Detached system-function object.
 * Date            : 2026.11.14
 ******************************************************************************/
static HmiObject functionObject(const QString &id, int function) //20261114-6 add
{
    HmiObject object = bitObject(id);

    object.address.clear();
    object.hasFunctionButtonProperties = true;
    object.bitButton.indicator = false;
    object.bitButton.dualMonitor = false;
    object.functionButton.function = function;
    return object;
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::immutableSessionAndTransactionalStart
 * Description     : Prove runtime memory and input ownership survive a failed replacement without editing the source.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::immutableSessionAndTransactionalStart() //20261114-6 add
{
    SimulationRuntime runtime;
    HmiProject source;
    HmiProject invalid;
    QString error;
    bool bit = false;

    source.objects = {bitObject(QStringLiteral("button"))};
    source.languages.append(QStringLiteral("English"));
    source.activeLanguage = QStringLiteral("English");
    QVERIFY2(runtime.start(source, &error), qPrintable(error));
    QVERIFY(runtime.running());
    QCOMPARE(runtime.userLevel(), 0);
    QCOMPARE(runtime.language(), QStringLiteral("中文"));
    QVERIFY(runtime.writeBit(QStringLiteral("LB0"), true, &error));
    source.name = QStringLiteral("edited-after-start");
    source.objects[0].address = QStringLiteral("LB9");
    QCOMPARE(runtime.project().objects[0].address, QStringLiteral("LB0"));
    invalid = source;
    invalid.pageWidth = 640;
    QVERIFY(!runtime.start(invalid, &error));
    QVERIFY(runtime.running());
    QVERIFY(runtime.project().name != source.name);
    QVERIFY(runtime.readBit(QStringLiteral("LB0"), &bit, &error));
    QVERIFY(bit);
    QVERIFY(!runtime.snapshot().value(QStringLiteral("hardwareIo")).toBool());
    QVERIFY(!runtime.snapshot().value(QStringLiteral("macroExecution")).toBool());
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::localRolesAndSpanIsolation
 * Description     : Exercise both inclusive endpoints, role mismatches, connection rejection and cross-word bounds.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::localRolesAndSpanIsolation() //20261114-6 add
{
    SimulationRuntime runtime;
    HmiProject source;
    QString error;
    QString value = QStringLiteral("unchanged");
    bool bit = true;

    QVERIFY2(runtime.start(source, &error), qPrintable(error));
    QVERIFY(runtime.writeBit(QStringLiteral("lb4096"), true, &error));
    QVERIFY(runtime.readBit(QStringLiteral("LB4096"), &bit, &error));
    QVERIFY(bit);
    QVERIFY(!runtime.writeBit(QStringLiteral("LW0"), true, &error));
    QVERIFY(!runtime.writeBit(QStringLiteral("LB4097"), true, &error));
    QVERIFY(!runtime.writeBit(QStringLiteral("@foreign:LB0"), true, &error));
    QVERIFY(!runtime.readBit(QStringLiteral("LB-1"), &bit, &error));
    QVERIFY(bit);
    QVERIFY(runtime.writeWord(QStringLiteral("LW4096"), 0, QStringLiteral("65535"), &error));
    QVERIFY(runtime.writeWord(QStringLiteral("LW4095"), 1, QStringLiteral("4294967295"), &error));
    QVERIFY(!runtime.writeWord(QStringLiteral("LW4096"), 1, QStringLiteral("1"), &error));
    QVERIFY(!runtime.writeWord(QStringLiteral("LB0"), 0, QStringLiteral("1"), &error));
    QVERIFY(!runtime.readWord(QStringLiteral("@foreign:LW0"), 0, &value, &error));
    QCOMPARE(value, QStringLiteral("unchanged"));
    QVERIFY(runtime.readWord(QStringLiteral("LW4096"), 0, &value, &error));
    QCOMPARE(value, QStringLiteral("65535"));
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::typedMemory_data
 * Description     : Select exact boundary values and one unrepresentable value for each public word type.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::typedMemory_data() //20261114-6 add
{
    QTest::addColumn<int>("type");
    QTest::addColumn<QString>("valid");
    QTest::addColumn<QString>("invalid");
    QTest::newRow("u16") << 0 << QStringLiteral("65535") << QStringLiteral("65536");
    QTest::newRow("u16-negative") << 0 << QStringLiteral("0") << QStringLiteral("-1");
    QTest::newRow("u32") << 1 << QStringLiteral("4294967295") << QStringLiteral("4294967296");
    QTest::newRow("u32-negative") << 1 << QStringLiteral("0") << QStringLiteral("-1");
    QTest::newRow("i16") << 2 << QStringLiteral("-32768") << QStringLiteral("-32769");
    QTest::newRow("i16-upper") << 2 << QStringLiteral("32767") << QStringLiteral("32768");
    QTest::newRow("i32") << 3 << QStringLiteral("-2147483648") << QStringLiteral("-2147483649");
    QTest::newRow("i32-upper") << 3 << QStringLiteral("2147483647") << QStringLiteral("2147483648");
    QTest::newRow("bcd16") << 4 << QStringLiteral("9999") << QStringLiteral("10000");
    QTest::newRow("bcd16-negative") << 4 << QStringLiteral("0") << QStringLiteral("-1");
    QTest::newRow("bcd32") << 5 << QStringLiteral("99999999") << QStringLiteral("100000000");
    QTest::newRow("bcd32-negative") << 5 << QStringLiteral("0") << QStringLiteral("-1");
    QTest::newRow("f32") << 6 << QStringLiteral("3.4028234663852886e38") << QStringLiteral("1e39");
    QTest::newRow("f32-negative") << 6 << QStringLiteral("-3.4028234663852886e38") << QStringLiteral("-1e39");
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::typedMemory
 * Description     : Prove rejected bounds preserve both words and all successful types round-trip numerically.
 * Input Parameters: QtTest data row
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::typedMemory() //20261114-6 add
{
    QFETCH(int, type);
    QFETCH(QString, valid);
    QFETCH(QString, invalid);
    SimulationRuntime runtime;
    HmiProject source;
    QString error;
    QString value;
    QJsonObject before;

    QVERIFY2(runtime.start(source, &error), qPrintable(error));
    QVERIFY2(runtime.writeWord(QStringLiteral("LW7"), type, valid, &error), qPrintable(error));
    QVERIFY(runtime.readWord(QStringLiteral("LW7"), type, &value, &error));
    QCOMPARE(type == 6 ? double(float(value.toDouble())) : value.toDouble(), type == 6 ? double(float(valid.toDouble())) : valid.toDouble());
    before = runtime.snapshot().value(QStringLiteral("LW")).toObject();
    QVERIFY(!runtime.writeWord(QStringLiteral("LW7"), type, invalid, &error));
    QVERIFY(!runtime.writeWord(QStringLiteral("LW7"), type, QStringLiteral("nan"), &error));
    QVERIFY(!runtime.writeWord(QStringLiteral("LW7"), type, QStringLiteral("1tail"), &error));
    QCOMPARE(runtime.snapshot().value(QStringLiteral("LW")).toObject(), before);
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::rawWordOrderAndInvalidInterpretations
 * Description     : Verify signed, BCD and float representations independently through raw U16 words.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::rawWordOrderAndInvalidInterpretations() //20261114-6 add
{
    SimulationRuntime runtime;
    HmiProject source;
    QString error;
    QString value;

    QVERIFY(runtime.start(source, &error));
    QVERIFY(runtime.writeWord(QStringLiteral("LW0"), 5, QStringLiteral("12345678"), &error));
    QVERIFY(runtime.readWord(QStringLiteral("LW0"), 0, &value, &error));
    QCOMPARE(value, QStringLiteral("22136"));
    QVERIFY(runtime.readWord(QStringLiteral("LW1"), 0, &value, &error));
    QCOMPARE(value, QStringLiteral("4660"));
    QVERIFY(runtime.writeWord(QStringLiteral("LW0"), 6, QStringLiteral("1.5"), &error));
    QVERIFY(runtime.readWord(QStringLiteral("LW0"), 0, &value, &error));
    QCOMPARE(value, QStringLiteral("0"));
    QVERIFY(runtime.readWord(QStringLiteral("LW1"), 0, &value, &error));
    QCOMPARE(value, QStringLiteral("16320"));
    QVERIFY(runtime.writeWord(QStringLiteral("LW0"), 3, QStringLiteral("-1"), &error));
    QVERIFY(runtime.readWord(QStringLiteral("LW0"), 1, &value, &error));
    QCOMPARE(value, QStringLiteral("4294967295"));
    QVERIFY(runtime.writeWord(QStringLiteral("LW0"), 0, QStringLiteral("10"), &error));
    value = QStringLiteral("retained");
    QVERIFY(!runtime.readWord(QStringLiteral("LW0"), 4, &value, &error));
    QCOMPARE(value, QStringLiteral("retained"));
    QVERIFY(runtime.writeWord(QStringLiteral("LW0"), 1, QStringLiteral("2143289344"), &error));
    QVERIFY(!runtime.readWord(QStringLiteral("LW0"), 6, &value, &error));
    QVERIFY(!runtime.writeWord(QStringLiteral("LW0"), 6, QStringLiteral("1e-100"), &error));
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::bitOperations_data
 * Description     : Separate set, reset, momentary and toggle behavior rather than mirroring one implementation branch.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::bitOperations_data() //20261114-6 add
{
    QTest::addColumn<int>("operation");
    QTest::addColumn<bool>("initial");
    QTest::addColumn<bool>("pressed");
    QTest::addColumn<bool>("released");
    QTest::newRow("set") << 0 << false << true << true;
    QTest::newRow("reset") << 1 << true << false << false;
    QTest::newRow("momentary") << 2 << false << true << false;
    QTest::newRow("toggle-on") << 3 << false << true << true;
    QTest::newRow("toggle-off") << 3 << true << false << false;
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::bitOperations
 * Description     : Check operation values, monitor state, duplicate-press protection and release semantics.
 * Input Parameters: QtTest data row
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::bitOperations() //20261114-6 add
{
    QFETCH(int, operation);
    QFETCH(bool, initial);
    QFETCH(bool, pressed);
    QFETCH(bool, released);
    SimulationRuntime runtime;
    HmiProject source;
    QString error;
    bool value;

    source.objects = {bitObject(QStringLiteral("button"), operation)};
    QVERIFY2(runtime.start(source, &error), qPrintable(error));
    QVERIFY(runtime.writeBit(QStringLiteral("LB0"), initial, &error));
    QVERIFY(runtime.press(QStringLiteral("button"), &error));
    QVERIFY(runtime.isPressed(QStringLiteral("button")));
    QVERIFY(!runtime.press(QStringLiteral("button"), &error));
    QVERIFY(runtime.readBit(QStringLiteral("LB0"), &value, &error));
    QCOMPARE(value, pressed);
    QCOMPARE(runtime.visualState(source.objects.first()), int(pressed));
    QVERIFY(runtime.release(true, &error));
    QVERIFY(runtime.readBit(QStringLiteral("LB0"), &value, &error));
    QCOMPARE(value, released);
    QVERIFY(!runtime.isPressed(QStringLiteral("button")));
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::momentarySafety_data
 * Description     : Exercise independent ways that a held pointer or its permission can disappear.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::momentarySafety_data() //20261114-6 add
{
    QTest::addColumn<int>("ending");
    QTest::newRow("release") << 0;
    QTest::newRow("move-out") << 1;
    QTest::newRow("cancel") << 2;
    QTest::newRow("stop") << 3;
    QTest::newRow("permission-loss") << 4;
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::momentarySafety
 * Description     : Force the owned bit false on all endings without clearing unrelated memory.
 * Input Parameters: QtTest data row
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::momentarySafety() //20261114-6 add
{
    QFETCH(int, ending);
    SimulationRuntime runtime;
    HmiProject source;
    HmiObject button = bitObject(QStringLiteral("button"), 2);
    QString error;
    bool value;

    button.bitButton.touch.visibilityEnabled = true;
    button.bitButton.touch.levelEnabled = true;
    button.bitButton.touch.minimumLevel = 2;
    source.objects = {button};
    QVERIFY2(runtime.start(source, &error), qPrintable(error));
    QVERIFY(!runtime.press(button.id, &error));
    QVERIFY(runtime.setUserLevel(2, &error));
    QVERIFY(runtime.writeBit(QStringLiteral("LB8"), true, &error));
    QVERIFY(runtime.press(button.id, &error));
    if (ending == 0) { QVERIFY(runtime.release(true, &error)); }
    else if (ending == 1) { QVERIFY(runtime.release(false, &error)); }
    else if (ending == 2) { runtime.cancel(QStringLiteral("window-inactive")); }
    else if (ending == 3) { runtime.stop(QStringLiteral("user-stop")); }
    else { QVERIFY(runtime.setUserLevel(0, &error)); }
    QVERIFY(runtime.readBit(QStringLiteral("LB0"), &value, &error));
    QVERIFY(!value);
    QVERIFY(runtime.readBit(QStringLiteral("LB8"), &value, &error));
    QVERIFY(value);
    QVERIFY(!runtime.isPressed(button.id));
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::noticesAndOperationAreAtomic
 * Description     : A rejected arithmetic action must not fire notices or partially update memory.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::noticesAndOperationAreAtomic() //20261114-6 add
{
    SimulationRuntime runtime;
    HmiProject source;
    HmiObject button = wordObject(QStringLiteral("add"), 3);
    QString error;
    QString value;
    bool bit;
    QJsonObject words;

    button.bitButton.notice = true;
    button.bitButton.noticeBit = true;
    button.bitButton.noticeBitAddress = QStringLiteral("LB9");
    button.bitButton.noticeWord = true;
    button.bitButton.noticeWordAddress = QStringLiteral("LW9");
    button.bitButton.noticeWordValue = QStringLiteral("27");
    source.objects = {button};
    QVERIFY2(runtime.start(source, &error), qPrintable(error));
    QVERIFY(runtime.writeWord(QStringLiteral("LW0"), 0, QStringLiteral("65535"), &error));
    words = runtime.snapshot().value(QStringLiteral("LW")).toObject();
    QVERIFY(!runtime.press(button.id, &error));
    QCOMPARE(runtime.snapshot().value(QStringLiteral("LW")).toObject(), words);
    QVERIFY(runtime.readBit(QStringLiteral("LB9"), &bit, &error));
    QVERIFY(!bit);
    QVERIFY(runtime.writeWord(QStringLiteral("LW0"), 0, QStringLiteral("1"), &error));
    QVERIFY(runtime.press(button.id, &error));
    QVERIFY(runtime.readWord(QStringLiteral("LW0"), 0, &value, &error));
    QCOMPARE(value, QStringLiteral("2"));
    QVERIFY(runtime.readWord(QStringLiteral("LW9"), 0, &value, &error));
    QCOMPARE(value, QStringLiteral("27"));
    QVERIFY(runtime.readBit(QStringLiteral("LB9"), &bit, &error));
    QVERIFY(bit);
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::wordOperations_data
 * Description     : Select bounded integer arithmetic including both wrap directions and failure edges.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::wordOperations_data() //20261114-6 add
{
    QTest::addColumn<int>("operation");
    QTest::addColumn<QString>("initial");
    QTest::addColumn<QString>("expected");
    QTest::addColumn<bool>("accepted");
    QTest::newRow("assign") << 0 << QStringLiteral("7") << QStringLiteral("2") << true;
    QTest::newRow("add") << 3 << QStringLiteral("7") << QStringLiteral("9") << true;
    QTest::newRow("subtract") << 4 << QStringLiteral("7") << QStringLiteral("5") << true;
    QTest::newRow("increment-wrap") << 5 << QStringLiteral("9") << QStringLiteral("0") << true;
    QTest::newRow("decrement-wrap") << 6 << QStringLiteral("0") << QStringLiteral("10") << true;
    QTest::newRow("add-refuse") << 3 << QStringLiteral("9") << QStringLiteral("9") << false;
    QTest::newRow("subtract-refuse") << 4 << QStringLiteral("0") << QStringLiteral("0") << false;
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::wordOperations
 * Description     : Check public button actions against declared configured bounds.
 * Input Parameters: QtTest data row
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::wordOperations() //20261114-6 add
{
    QFETCH(int, operation);
    QFETCH(QString, initial);
    QFETCH(QString, expected);
    QFETCH(bool, accepted);
    SimulationRuntime runtime;
    HmiProject source;
    HmiObject button = wordObject(QStringLiteral("word"), operation);
    QString error;
    QString value;

    button.wordButton.constantValue = QStringLiteral("2");
    button.wordButton.minimumValue = QStringLiteral("0");
    button.wordButton.maximumValue = QStringLiteral("10");
    source.objects = {button};
    QVERIFY2(runtime.start(source, &error), qPrintable(error));
    QVERIFY(runtime.writeWord(QStringLiteral("LW0"), 0, initial, &error));
    QCOMPARE(runtime.press(button.id, &error), accepted);
    QVERIFY(runtime.readWord(QStringLiteral("LW0"), 0, &value, &error));
    QCOMPARE(value, expected);
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::inputLifecycleAndAtomicNotices
 * Description     : Defer input writes and notices, preserve pending invalid input, and cancel moved-out requests.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::inputLifecycleAndAtomicNotices() //20261114-6 add
{
    SimulationRuntime runtime;
    HmiProject source;
    HmiObject button = wordObject(QStringLiteral("input"), 1);
    QString error;
    QString value;
    bool notice;

    button.wordButton.maximumValue = QStringLiteral("99");
    button.wordButton.totalDigits = 2;
    button.bitButton.notice = true;
    button.bitButton.noticeBit = true;
    button.bitButton.noticeBitAddress = QStringLiteral("LB9");
    source.objects = {button};
    QVERIFY2(runtime.start(source, &error), qPrintable(error));
    QVERIFY(runtime.press(button.id, &error));
    QCOMPARE(runtime.pendingInputObjectId(), button.id);
    QVERIFY(runtime.readBit(QStringLiteral("LB9"), &notice, &error));
    QVERIFY(!notice);
    QVERIFY(runtime.release(true, &error));
    QVERIFY(!runtime.submitInput(QStringLiteral("100"), &error));
    QCOMPARE(runtime.pendingInputObjectId(), button.id);
    QVERIFY(runtime.readWord(QStringLiteral("LW0"), 0, &value, &error));
    QCOMPARE(value, QStringLiteral("0"));
    QVERIFY(runtime.readBit(QStringLiteral("LB9"), &notice, &error));
    QVERIFY(!notice);
    QVERIFY(runtime.submitInput(QStringLiteral("42"), &error));
    QVERIFY(runtime.pendingInputObjectId().isEmpty());
    QVERIFY(runtime.readWord(QStringLiteral("LW0"), 0, &value, &error));
    QCOMPARE(value, QStringLiteral("42"));
    QVERIFY(runtime.readBit(QStringLiteral("LB9"), &notice, &error));
    QVERIFY(notice);
    QVERIFY(runtime.press(button.id, &error));
    QVERIFY(runtime.release(false, &error));
    QVERIFY(runtime.pendingInputObjectId().isEmpty());
    QVERIFY(!runtime.submitInput(QStringLiteral("12"), &error));
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::floatInputAndWideNotice
 * Description     : Apply a bounded decimal keypad value and one two-word float notice as a single event.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::floatInputAndWideNotice() //20261114-6 add
{
    SimulationRuntime runtime;
    HmiProject source;
    HmiObject button = wordObject(QStringLiteral("float-input"), 1, 6);
    QString error;
    QString value;

    button.wordButton.minimumValue = QStringLiteral("0");
    button.wordButton.maximumValue = QStringLiteral("10");
    button.wordButton.totalDigits = 4;
    button.wordButton.decimalDigits = 2;
    button.bitButton.notice = true;
    button.bitButton.noticeWord = true;
    button.bitButton.noticeWordType = 4;
    button.bitButton.noticeWordAddress = QStringLiteral("LW4095");
    button.bitButton.noticeWordValue = QStringLiteral("1.25");
    source.objects = {button};
    QVERIFY2(runtime.start(source, &error), qPrintable(error));
    QVERIFY(runtime.press(button.id, &error));
    QVERIFY(!runtime.submitInput(QStringLiteral("1.234"), &error));
    QVERIFY(!runtime.submitInput(QStringLiteral("12.34"), &error));
    QVERIFY(runtime.readWord(QStringLiteral("LW4095"), 6, &value, &error));
    QCOMPARE(value, QStringLiteral("0"));
    QVERIFY(runtime.submitInput(QStringLiteral("1.25"), &error));
    QVERIFY(runtime.readWord(QStringLiteral("LW0"), 6, &value, &error));
    QCOMPARE(value, QStringLiteral("1.25"));
    QVERIFY(runtime.readWord(QStringLiteral("LW4095"), 6, &value, &error));
    QCOMPARE(value, QStringLiteral("1.25"));
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::inputLostAccessAndPasswordLog
 * Description     : Invalidate a pending keypad after permission loss and omit entered password values from events.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::inputLostAccessAndPasswordLog() //20261114-6 add
{
    SimulationRuntime runtime;
    HmiProject source;
    HmiObject button = wordObject(QStringLiteral("password"), 2);
    QString error;
    QString value;

    button.bitButton.touch.visibilityEnabled = true;
    button.bitButton.touch.levelEnabled = true;
    button.bitButton.touch.minimumLevel = 3;
    source.objects = {button};
    QVERIFY2(runtime.start(source, &error), qPrintable(error));
    QVERIFY(runtime.setUserLevel(3, &error));
    QVERIFY(runtime.press(button.id, &error));
    QVERIFY(runtime.release(true, &error));
    QVERIFY(runtime.setUserLevel(0, &error));
    QVERIFY(runtime.pendingInputObjectId().isEmpty());
    QVERIFY(!runtime.submitInput(QStringLiteral("2468"), &error));
    QVERIFY(runtime.setUserLevel(3, &error));
    QVERIFY(runtime.press(button.id, &error));
    QVERIFY(runtime.submitInput(QStringLiteral("2468"), &error));
    QVERIFY(runtime.readWord(QStringLiteral("LW0"), 0, &value, &error));
    QCOMPARE(value, QStringLiteral("2468"));
    QVERIFY(!QJsonDocument(runtime.events()).toJson().contains("2468"));
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::permissionComparisons_data
 * Description     : Exercise all six persisted comparison indices at less, equal and greater stimuli.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::permissionComparisons_data() //20261114-6 add
{
    static const bool expected[6][3] = {{true,false,false},{false,false,true},{false,true,false},
                                      {true,false,true},{true,true,false},{false,true,true}};
    int comparison;
    int stimulus;

    QTest::addColumn<int>("comparison");
    QTest::addColumn<int>("stimulus");
    QTest::addColumn<bool>("visible");
    for (comparison = 0; comparison < 6; ++comparison)
    {
        for (stimulus = 0; stimulus < 3; ++stimulus)
        {
            QTest::newRow(qPrintable(QStringLiteral("comparison-%1-value-%2").arg(comparison).arg(stimulus)))
                << comparison << stimulus << expected[comparison][stimulus];
        }
    }
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::permissionComparisons
 * Description     : Bind the visibility predicate to an actual frozen text object and memory stimulus.
 * Input Parameters: QtTest data row
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::permissionComparisons() //20261114-6 add
{
    QFETCH(int, comparison);
    QFETCH(int, stimulus);
    QFETCH(bool, visible);
    SimulationRuntime runtime;
    HmiProject source;
    HmiObject text;
    QString error;

    text.id = QStringLiteral("text");
    text.name = text.id;
    text.type = QStringLiteral("text");
    text.text = QStringLiteral("conditional");
    text.geometry = QRect(10, 10, 100, 30);
    text.hasStaticTextProperties = true;
    text.staticText.visibilityEnabled = true;
    text.staticText.wordEnabled = true;
    text.staticText.wordAddress = QStringLiteral("LW10");
    text.staticText.compareValue = QStringLiteral("1");
    text.staticText.comparison = comparison;
    source.objects = {text};
    QVERIFY2(runtime.start(source, &error), qPrintable(error));
    QVERIFY(runtime.writeWord(QStringLiteral("LW10"), 0, QString::number(stimulus), &error));
    QCOMPARE(runtime.isVisible(text), visible);
    QVERIFY(!runtime.touchAllowed(text));
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::bitLevelAndWordBitMonitoring
 * Description     : Separate monitored word bits from the operation bit and release a hidden momentary control.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::bitLevelAndWordBitMonitoring() //20261114-6 add
{
    SimulationRuntime runtime;
    HmiProject source;
    HmiObject button = bitObject(QStringLiteral("monitored"), 2);
    QString error;
    bool value;

    button.bitButton.monitorSame = false;
    button.bitButton.monitorType = 1;
    button.bitButton.monitorWordAddress = QStringLiteral("LW4096");
    button.bitButton.bitIndex = 15;
    button.bitButton.visibility.visibilityEnabled = true;
    button.bitButton.visibility.bitEnabled = true;
    button.bitButton.visibility.bitAddress = QStringLiteral("LB4");
    source.objects = {button};
    QVERIFY2(runtime.start(source, &error), qPrintable(error));
    QVERIFY(!runtime.isVisible(button));
    QVERIFY(runtime.writeBit(QStringLiteral("LB4"), true, &error));
    QVERIFY(runtime.press(button.id, &error));
    QCOMPARE(runtime.visualState(button), 0);
    QVERIFY(runtime.writeWord(QStringLiteral("LW4096"), 0, QStringLiteral("32768"), &error));
    QCOMPARE(runtime.visualState(button), 1);
    QVERIFY(runtime.writeBit(QStringLiteral("LB4"), false, &error));
    QVERIFY(!runtime.isPressed(button.id));
    QVERIFY(runtime.readBit(QStringLiteral("LB0"), &value, &error));
    QVERIFY(!value);
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::multiLampUnknownState
 * Description     : Map a multi-state lamp to its declared range and preserve an unknown state outside that range.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::multiLampUnknownState() //20261114-6 add
{
    SimulationRuntime runtime;
    HmiProject source;
    HmiObject lamp = bitObject(QStringLiteral("lamp"));
    QString error;

    lamp.address.clear();
    lamp.hasLampProperties = true;
    lamp.lamp.monitorMode = 2;
    lamp.lamp.multiDataType = 1;
    lamp.lamp.multiStateCount = 5;
    lamp.lamp.multiMonitorAddress = QStringLiteral("LW4095");
    lamp.bitButton.monitorSame = false;
    lamp.bitButton.dualMonitor = false;
    lamp.lamp.ensureStateCount(5, lamp.bitButton);
    source.objects = {lamp};
    QVERIFY2(runtime.start(source, &error), qPrintable(error));
    QCOMPARE(runtime.visualState(lamp), 0);
    QVERIFY(!runtime.touchAllowed(lamp));
    QVERIFY(runtime.writeWord(QStringLiteral("LW4095"), 1, QStringLiteral("4"), &error));
    QCOMPARE(runtime.visualState(lamp), 4);
    QVERIFY(runtime.writeWord(QStringLiteral("LW4095"), 1, QStringLiteral("5"), &error));
    QCOMPARE(runtime.visualState(lamp), -1);
    QVERIFY(!runtime.press(lamp.id, &error));
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::languageLogoutAndMacroSkip
 * Description     : Execute only local system functions and prove a bound macro body is skipped with explicit evidence.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::languageLogoutAndMacroSkip() //20261114-6 add
{
    SimulationRuntime runtime;
    HmiProject source;
    HmiObject language = functionObject(QStringLiteral("language"), 13);
    HmiObject logout = functionObject(QStringLiteral("logout"), 8);
    HmiObject macro = bitObject(QStringLiteral("macro"), 0, QStringLiteral("LB1"));
    QString error;
    bool bit;

    source.languages.append(QStringLiteral("English"));
    language.functionButton.type = 1;
    macro.bitButton.macroEnabled = true;
    macro.bitButton.macroName = QStringLiteral("bound");
    source.macros.insert(QStringLiteral("bound"), QStringLiteral("write LB100; must never execute"));
    source.objects = {language, logout, macro};
    QVERIFY2(runtime.start(source, &error), qPrintable(error));
    QVERIFY(runtime.press(language.id, &error));
    QCOMPARE(runtime.language(), QStringLiteral("English"));
    QCOMPARE(source.activeLanguage, QStringLiteral("中文"));
    QVERIFY(runtime.release(true, &error));
    QVERIFY(runtime.setUserLevel(8, &error));
    QVERIFY(runtime.press(logout.id, &error));
    QCOMPARE(runtime.userLevel(), 0);
    QVERIFY(runtime.release(true, &error));
    QVERIFY(runtime.press(macro.id, &error));
    QVERIFY(runtime.readBit(QStringLiteral("LB1"), &bit, &error));
    QVERIFY(bit);
    QVERIFY(runtime.readBit(QStringLiteral("LB100"), &bit, &error));
    QVERIFY(!bit);
    QVERIFY(QJsonDocument(runtime.events()).toJson().contains("macro-skipped"));
    QVERIFY(!runtime.snapshot().value(QStringLiteral("warnings")).toArray().isEmpty());
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::capabilityRefusals_data
 * Description     : Enumerate independent unsupported behaviors rather than silently dropping them from a session.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::capabilityRefusals_data() //20261114-6 add
{
    int scenario;

    QTest::addColumn<int>("scenario");
    for (scenario = 0; scenario < 16; ++scenario)
    { QTest::newRow(qPrintable(QStringLiteral("refusal-%1").arg(scenario))) << scenario; }
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::capabilityRefusals
 * Description     : Keep a live prior session intact when any declared unsupported behavior is introduced.
 * Input Parameters: QtTest data row
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::capabilityRefusals() //20261114-6 add
{
    QFETCH(int, scenario);
    SimulationRuntime runtime;
    HmiProject valid;
    HmiProject invalid;
    HmiObject button = bitObject(QStringLiteral("button"));
    HmiConfigurationNode node;
    HmiPage page;
    QString error;
    QJsonObject report;
    bool bit;

    valid.objects = {button};
    QVERIFY2(runtime.start(valid, &error), qPrintable(error));
    QVERIFY(runtime.writeBit(QStringLiteral("LB0"), true, &error));
    invalid = valid;
    if (scenario == 0) { invalid.objects[0].bitButton.pressTimeEnabled = true; }
    else if (scenario == 1) { invalid.objects[0].bitButton.states[0].flash = true; }
    else if (scenario == 2) { invalid.objects = {functionObject(QStringLiteral("unimplemented"), 0)}; }
    else if (scenario == 3) { invalid.nativeSourceSnapshot = QByteArray("opaque"); }
    else if (scenario == 4)
    {
        page.id = QStringLiteral("second"); page.name = page.id;
        invalid.inactivePages.append(page); invalid.pageOrder.append(page.id);
    }
    else if (scenario == 5)
    {
        invalid.objects[0].bitButton.visibility.visibilityEnabled = true;
        invalid.objects[0].bitButton.visibility.bitEnabled = true;
        invalid.objects[0].bitButton.visibility.levelEnabled = true;
    }
    else if (scenario == 6)
    {
        invalid.objects[0].bitButton.touch.visibilityEnabled = true;
        invalid.objects[0].bitButton.touch.wordEnabled = true;
        invalid.objects[0].bitButton.touch.tolerance = QStringLiteral("1");
    }
    else if (scenario == 7)
    {
        invalid.objects[0].bitButton.notice = true;
        invalid.objects[0].bitButton.noticeBit = true;
    }
    else if (scenario == 8) { invalid.objects[0].address = QStringLiteral("LB4097"); }
    else if (scenario == 9) { invalid.objects[0].type = QStringLiteral("number"); }
    else if (scenario == 10) { invalid.objects[0].address = QStringLiteral("@foreign:LB0"); }
    else if (scenario == 11)
    {
        node.id = QStringLiteral("active-config");
        node.kind = QStringLiteral("system");
        node.name = node.id;
        node.implemented = true;
        node.active = true;
        node.properties = {{QStringLiteral("screensaverMinutes"), 1}, {QStringLiteral("flickerTenths"), 10},
                           {QStringLiteral("touchSound"), true}, {QStringLiteral("defaultUserLevel"), 1},
                           {QStringLiteral("hexWordAddress"), false}};
        invalid.configurationNodes.append(node);
        QVERIFY2(invalid.validateModel(&error), qPrintable(error));
    }
    else if (scenario == 12)
    {
        invalid.objects[0].bitButton.notice = true;
        invalid.objects[0].bitButton.noticeWord = true;
        invalid.objects[0].bitButton.noticeWordType = 1;
        invalid.objects[0].bitButton.noticeWordAddress = QStringLiteral("LW4096");
    }
    else if (scenario == 13)
    {
        invalid.objects = {wordObject(QStringLiteral("scaled-input"), 1)};
        invalid.objects[0].wordButton.decimalDigits = 1;
    }
    else if (scenario == 14) { invalid.objects[0].bitButton.touch.visibilityEnabled = true; }
    else { invalid.objects[0].bitButton.states[0].text.visibilityEnabled = true; }
    report = SimulationRuntime::capabilities(invalid);
    QVERIFY(!report.value(QStringLiteral("supported")).toBool());
    QVERIFY(!report.value(QStringLiteral("errors")).toArray().isEmpty());
    QVERIFY(!runtime.start(invalid, &error));
    QVERIFY(runtime.running());
    QVERIFY(runtime.readBit(QStringLiteral("LB0"), &bit, &error));
    QVERIFY(bit);
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::boundedEventsAndRestart
 * Description     : Bound memory evidence while preserving lifecycle history and resetting banks on a successful new start.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::boundedEventsAndRestart() //20261114-6 add
{
    SimulationRuntime runtime;
    HmiProject source;
    QString error;
    bool bit;
    int index;

    QVERIFY(runtime.start(source, &error));
    for (index = 0; index < 600; ++index)
    { QVERIFY(runtime.writeBit(QStringLiteral("LB0"), true, &error)); }
    QCOMPARE(runtime.events().size(), 512);
    runtime.stop(QStringLiteral("test-stop"));
    QVERIFY(!runtime.running());
    QVERIFY(!runtime.writeBit(QStringLiteral("LB0"), false, &error));
    QVERIFY(runtime.start(source, &error));
    QVERIFY(runtime.readBit(QStringLiteral("LB0"), &bit, &error));
    QVERIFY(!bit);
    QCOMPARE(runtime.userLevel(), 0);
    QVERIFY(QJsonDocument(runtime.events()).toJson().contains("test-stop"));
    QCOMPARE(runtime.events().size(), 512);
}

/******************************************************************************
 * Function Name   : SimulationRuntimeTests::boundedCapabilityDetails
 * Description     : Bound diagnostic output even when legacy model macro names or invalid identities are very large.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntimeTests::boundedCapabilityDetails() //20261114-6 add
{
    SimulationRuntime runtime;
    HmiProject source;
    HmiObject button = bitObject(QStringLiteral("macro"));
    QString name(100000, QChar(u'M'));
    QString error;
    QJsonObject report;

    button.bitButton.macroEnabled = true;
    button.bitButton.macroName = name;
    source.macros.insert(name, QStringLiteral("not executed"));
    source.objects = {button};
    QVERIFY2(source.validateModel(&error), qPrintable(error));
    report = SimulationRuntime::capabilities(source);
    QVERIFY(report.value(QStringLiteral("supported")).toBool());
    QVERIFY(QJsonDocument(report).toJson().size() < 16384);
    QVERIFY(runtime.start(source, &error));
    QVERIFY(QJsonDocument(runtime.snapshot()).toJson().size() < 16384);
    source.objects[0].id = name;
    report = SimulationRuntime::capabilities(source);
    QVERIFY(!report.value(QStringLiteral("supported")).toBool());
    QVERIFY(QJsonDocument(report).toJson().size() < 16384);
}

QTEST_APPLESS_MAIN(SimulationRuntimeTests)
#include "simulation_runtime_tests.moc"
