#include "native_project_codec.h"
#include "project_model.h"

#include <QDataStream>
#include <QCryptographicHash> //20261110-5 add: Construct explicitly synthetic compiled-cache format tests.
#include <QDirIterator>
#include <QFile>
#include <QFileInfo> //20261110-0 add
#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>
#include <memory> //20261110-5 add: Construct corpus iterators only after fixture directories are checked.
#include <algorithm> //20261110-6 add: Exercise name-based remapping with a reversed language registration order.
#include <QtTest>

//20261110-5 change: Frozen fixture provenance is in fixtures/native-source/manifest.json; no tests launch the vendor program.
class NativeProjectCodecTests : public QObject
{
    Q_OBJECT
private slots:
    void nativeCorpusKeepsExactBytes();
    void malformedHeadersAreTransactional();
    void changedArchivesAreRejected();
    void opaqueBodyIsNeverReportedAsValidated();
    void nativeAddressOracleRoundTrip();
    void sourceAddressMatchesQt4Primitives();
    void malformedAddressesAreTransactional();
    void semanticOperationsFailExplicitly();
    void supportedEditsKeepNativeStructure();
    void savedBitUiVariantsAndProductMacroSemantics();
    void inactiveDirtyConditionsBecomeCanonicalWhenEnabled();
    void createAddCopyDeleteNativeButtons();
    void mixedNativeButtonsAndStaticTitles();
    void nativeExternalWordTypeOracles();
    void unsupportedMetadataEditsAreTransactional();
    void nativeMultilingualSourcesAndLanguageDefaults();
    void nativeStartupLanguageRemainsIndependentOfEditorSelection();
    void nativeIndicatorStateListsCanShrinkAndGrow();
    void productMacroSourcesAndBindingsRemainEditable();
    void nativeWordButtonCorpusKeepsExactBytes(); //20261109-6 add
    void nativeWordButtonOperationsAndNewMixedProjects();
    void nativeWordButtonPressedStatesAndGuards();
    void nativeLampCorpusKeepsExactBytes(); //20261109-8 add
    void nativeLampMonitoringEditsAndMixedCreation();
    void nativeLampStateExpansionAndGuards();
    void nativeLampNoImageAndSharedText(); //20261109-8 add
    void nativeLampActualSamplesRoundTrip_data(); //20261109-8 add
    void nativeLampActualSamplesRoundTrip();
    void nativeFunctionCorpusKeepsExactBytes(); //20261110-0 add
    void nativeFunctionUnsupportedUnicodeIsExplicit();
    void nativeFunctionOpaqueMetadataAndKeyWidth();
    void nativeFunctionAllSelectionsAndFreshProjects();
    void nativeFunctionAppearancePermissionsAndGuards();
    void nativeFunctionActualSamplesRoundTrip_data();
    void nativeFunctionActualSamplesRoundTrip();
    void typedFieldBoundariesAndMalformedCollections(); //20261110-5 add
    void everyTruncatedPrefixPreservesSemanticOutputs();
    void multiplePagesAreReadOnlyArchives();
    void sourceRejectsUnmappedModelEntities();
    void explicitStartupLanguagePreservesDialogSelection(); //20261110-6 change
    void nativeIndependentLanguageSelections_data(); //20261110-6 add
    void nativeIndependentLanguageSelections();
    void unknownLanguageDialogSlotsArePreserved(); //20261110-6 add
    void typedMetadataBudgetIsBounded();
    void largeNativeDiagnosticsKeepCompleteSnapshot();
    void nativeUnrepresentableActiveMetadataIsRejected();
    void productMacroCachePreservationAndInvalidation();
};

/******************************************************************************
 * Function Name   : sourceWordBytes
 * Description     : Encode test mutations using the native big-endian integer wire width.
 * Input Parameters: value
 * Return Value    : Exactly four bytes.
 * Date            : 2026.11.10
 ******************************************************************************/
static QByteArray sourceWordBytes(u32 value) //20261110-5 add
{
    QByteArray bytes;
    int shift;

    for (shift = 24; shift >= 0; shift -= 8) {
        bytes.append(static_cast<char>((value >> shift) & 0xffU));
    }
    return bytes;
}

/******************************************************************************
 * Function Name   : sourceFieldSpan
 * Description     : Locate a mutation boundary through complete typed inspection rather than a file offset.
 * Input Parameters: bytes, path
 * Return Value    : Verified span, or a zero-length span on failure.
 * Date            : 2026.11.10
 ******************************************************************************/
static NativeSourceFieldSpan sourceFieldSpan(const QByteArray &bytes, const QString &path) //20261110-5 add
{
    QList<NativeSourceFieldSpan> spans;
    NativeSourceFieldSpan field;

    if (!inspectNativeProjectSourceFields(bytes, &spans)) {
        return field;
    }
    Q_FOREACH (field, spans) {
        if (field.path == path) {
            return field;
        }
    }
    return NativeSourceFieldSpan();
}

/******************************************************************************
 * Function Name   : corpusPath
 * Description     : Locate read-only native source fixtures without a vendor installation.
 * Input Parameters: None
 * Return Value    : Existing native corpus directory or an empty path.
 * Date            : 2026.11.09
 ******************************************************************************/
static QString corpusPath() //20261109-0 add
{
    QString path;

    path = qEnvironmentVariable("KINSEAL_NATIVE_SOURCE_CORPUS");
    if (!path.isEmpty()) {
        return path;
    }
    return QFINDTESTDATA("fixtures/native-source/bit");
}

/******************************************************************************
 * Function Name   : nativeOracle
 * Description     : Load the saved default native button source fixture.
 * Input Parameters: None
 * Return Value    : Native source bytes, empty when unavailable.
 * Date            : 2026.11.09
 ******************************************************************************/
static QByteArray nativeOracle() //20261109-0 add
{
    QFile file(QDir(corpusPath()).filePath(QStringLiteral("native-bit-seed.sm")));

    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    return file.readAll();
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeCorpusKeepsExactBytes
 * Description     : Recognize every saved native fixture and restore its bytes exactly.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::nativeCorpusKeepsExactBytes() //20261109-0 add
{
    QString root = corpusPath();
    QString path;
    QString error;
    QByteArray bytes;
    QByteArray restored;
    NativeProjectEnvelope envelope;
    HmiProject project;
    int count;
    std::unique_ptr<QDirIterator> iterator; //20261110-5 change
    QFile file;

    QVERIFY2(!root.isEmpty() && QDir(root).exists(), "Frozen native source corpus is required; do not scan the current directory."); //20261110-5 change
    iterator = std::make_unique<QDirIterator>(root, QStringList{QStringLiteral("*.sm")}, QDir::Files, QDirIterator::Subdirectories);
    count = 0;
    while (iterator->hasNext()) {
        path = iterator->next();
        file.setFileName(path);
        QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(path));
        bytes = file.readAll();
        file.close();
        error = QStringLiteral("stale");
        QVERIFY2(inspectNativeProjectEnvelope(bytes, &envelope, &error), qPrintable(error));
        QVERIFY(error.isEmpty());
        QCOMPARE(envelope.version, 122U);
        QCOMPARE(envelope.targetModel, QStringLiteral("AMT070W124-L800X480R"));
        QCOMPARE(envelope.payloadOffset, 92);
        QVERIFY(!envelope.limitations.isEmpty());
        QVERIFY2(validateNativeProjectSourceStructure(bytes, &error), qPrintable(error));
        QVERIFY2(nativeProjectUsesSupportedDeviceDefaults(bytes, &error), qPrintable(error));
        QVERIFY2(importNativeProjectSource(bytes, &project, &error), qPrintable(path + QStringLiteral(": ") + error));
        QCOMPARE(project.objects.size(), 1);
        QCOMPARE(project.pageWidth, 800);
        QCOMPARE(project.pageHeight, 480);
        QVERIFY2(exportNativeProjectSource(project, &restored, &error), qPrintable(error));
        QCOMPARE(restored, bytes);
        QVERIFY2(restoreNativeProjectArchive(envelope, &restored, &error), qPrintable(error));
        QCOMPARE(restored, bytes);
        ++count;
    }
    QVERIFY(count >= 10);
    qInfo() << "Native source snapshots checked:" << count;
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::malformedHeadersAreTransactional
 * Description     : Reject bad signatures, versions and unbounded or truncated QString lengths.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::malformedHeadersAreTransactional() //20261109-0 add
{
    QByteArray oracle;
    QByteArray bad;
    NativeProjectEnvelope envelope;
    QString error;
    int size;

    oracle = nativeOracle();
    QVERIFY(!oracle.isEmpty());
    envelope.version = 999;
    for (size = 0; size <= 92; ++size) {
        QVERIFY(!inspectNativeProjectEnvelope(oracle.left(size), &envelope, &error));
        QCOMPARE(envelope.version, 999U);
        QVERIFY(!error.isEmpty());
    }
    bad = oracle;
    bad[47] = static_cast<char>(123);
    QVERIFY(!inspectNativeProjectEnvelope(bad, &envelope, &error));
    QVERIFY(error.contains(QStringLiteral("123")));
    bad = oracle;
    bad[7] = 'X';
    QVERIFY(!inspectNativeProjectEnvelope(bad, &envelope, &error));
    bad = oracle;
    bad[0] = static_cast<char>(0x7f);
    QVERIFY(!inspectNativeProjectEnvelope(bad, &envelope, &error));
    bad = oracle;
    bad[3] = 41;
    QVERIFY(!inspectNativeProjectEnvelope(bad, &envelope, &error));
    bad = oracle;
    bad[91] = 'Z';
    QVERIFY(!inspectNativeProjectEnvelope(bad, &envelope, &error));
    QVERIFY(!inspectNativeProjectEnvelope(oracle, nullptr, nullptr));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::changedArchivesAreRejected
 * Description     : Prevent accidental snapshot edits from being called lossless restoration.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::changedArchivesAreRejected() //20261109-0 add
{
    NativeProjectEnvelope envelope;
    QByteArray output;
    QByteArray oracle;
    QString error;

    oracle = nativeOracle();
    QVERIFY(inspectNativeProjectEnvelope(oracle, &envelope, &error));
    output = QByteArray("retained");
    envelope.originalBytes[100] = static_cast<char>(envelope.originalBytes.at(100) ^ 1);
    QVERIFY(!restoreNativeProjectArchive(envelope, &output, &error));
    QCOMPARE(output, QByteArray("retained"));
    QVERIFY(inspectNativeProjectEnvelope(oracle, &envelope, &error));
    envelope.targetModel = QStringLiteral("changed");
    QVERIFY(!restoreNativeProjectArchive(envelope, &output, &error));
    QCOMPARE(output, QByteArray("retained"));
    QVERIFY(!restoreNativeProjectArchive(envelope, nullptr, nullptr));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::opaqueBodyIsNeverReportedAsValidated
 * Description     : Keep envelope success distinct from validation of unknown control records.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::opaqueBodyIsNeverReportedAsValidated() //20261109-0 add
{
    NativeProjectEnvelope envelope;
    QByteArray headerWithOpaqueBody;
    QString error;

    headerWithOpaqueBody = nativeOracle().left(92) + QByteArray("?");
    QVERIFY(inspectNativeProjectEnvelope(headerWithOpaqueBody, &envelope, &error));
    QVERIFY(envelope.limitations.join(QString()).contains(QStringLiteral("主体完整性未知")));
    QCOMPARE(envelope.originalBytes, headerWithOpaqueBody);
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeAddressOracleRoundTrip
 * Description     : Reproduce a native LB0 source address from its independently mapped boundary.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::nativeAddressOracleRoundTrip() //20261109-0 add
{
    QByteArray oracle;
    QByteArray encoded;
    NativeSourceAddress address;
    qsizetype next;
    QString error;

    oracle = nativeOracle();
    QVERIFY2(readNativeSourceAddress(oracle, 3200, &address, &next, &error), qPrintable(error));
    QCOMPARE(next, 3236);
    QCOMPARE(address.text, QStringLiteral("LB0"));
    QCOMPARE(address.numericText, QStringLiteral("0"));
    QCOMPARE(address.registerText, QStringLiteral("LB"));
    QCOMPARE(address.rawLength, 1U);
    QCOMPARE(address.field0, 0);
    QCOMPARE(address.field1, 0);
    QCOMPARE(address.field2, 0);
    QCOMPARE(address.field3, 0);
    QVERIFY(writeNativeSourceAddress(address, &encoded, &error));
    QCOMPARE(encoded, oracle.mid(3200, next - 3200));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::sourceAddressMatchesQt4Primitives
 * Description     : Match Qt4 primitive output including null strings, Unicode and opaque bytes.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::sourceAddressMatchesQt4Primitives() //20261109-0 add
{
    NativeSourceAddress original;
    NativeSourceAddress decoded;
    QByteArray encoded;
    QByteArray qtEncoded;
    QByteArray framed;
    qsizetype next;
    QString error;
    QDataStream stream(&qtEncoded, QIODevice::WriteOnly);

    original.text = QStringLiteral("中文-LW4096");
    original.field0 = 255;
    original.field1 = 128;
    original.field2 = 100;
    original.numericText = QStringLiteral("");
    original.registerText = QString();
    original.field3 = 7;
    original.rawLength = 0x80000001U;
    original.auxiliary = QStringLiteral("辅助");
    stream.setVersion(QDataStream::Qt_4_0);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << original.text << original.field0 << original.field1 << original.field2
           << original.numericText << original.registerText << original.field3
           << original.rawLength << original.auxiliary;
    QVERIFY(writeNativeSourceAddress(original, &encoded, &error));
    QCOMPARE(encoded, qtEncoded);
    framed = QByteArray("prefix") + encoded + QByteArray("suffix");
    QVERIFY(readNativeSourceAddress(framed, 6, &decoded, &next, &error));
    QCOMPARE(next, 6 + encoded.size());
    QCOMPARE(decoded.text, original.text);
    QCOMPARE(decoded.field0, original.field0);
    QCOMPARE(decoded.field1, original.field1);
    QCOMPARE(decoded.field2, 100);
    QCOMPARE(decoded.field3, original.field3);
    QCOMPARE(decoded.rawLength, original.rawLength);
    QVERIFY(!decoded.numericText.isNull());
    QVERIFY(decoded.registerText.isNull());
    QCOMPARE(decoded.auxiliary, original.auxiliary);
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::malformedAddressesAreTransactional
 * Description     : Reject every truncated address prefix and preserve prior outputs.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::malformedAddressesAreTransactional() //20261109-0 add
{
    NativeSourceAddress address;
    QByteArray record;
    QByteArray bad;
    QByteArray output;
    qsizetype next;
    qsizetype i;
    QString error;

    record = nativeOracle().mid(3200, 36);
    address.text = QStringLiteral("retained");
    next = 123;
    for (i = 0; i < record.size(); ++i) {
        QVERIFY(!readNativeSourceAddress(record.left(i), 0, &address, &next, &error));
        QCOMPARE(address.text, QStringLiteral("retained"));
        QCOMPARE(next, 123);
    }
    bad = record;
    bad[3] = 7;
    QVERIFY(!readNativeSourceAddress(bad, 0, &address, &next, &error));
    bad = record;
    bad[0] = 0x7f;
    QVERIFY(!readNativeSourceAddress(bad, 0, &address, &next, &error));
    QVERIFY(!readNativeSourceAddress(record, -1, &address, &next, &error));
    QVERIFY(!readNativeSourceAddress(record, 0, nullptr, &next, &error));
    QVERIFY(!readNativeSourceAddress(record, 0, &address, nullptr, &error));
    QVERIFY(!writeNativeSourceAddress(address, nullptr, nullptr));
    output = QByteArray("retained");
    address.text = QString(524289, QChar(0x41));
    QVERIFY(!writeNativeSourceAddress(address, &output, &error));
    QCOMPARE(output, QByteArray("retained"));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::semanticOperationsFailExplicitly
 * Description     : Preserve existing editable state and bytes when native semantic support is absent.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::semanticOperationsFailExplicitly() //20261109-0 add
{
    HmiProject project;
    QByteArray output;
    QString error;

    project.name = QStringLiteral("retained");
    project.macros.insert(QStringLiteral("unverified-name"), QStringLiteral("void Macro_Entry(void) {}"));
    output = QByteArray("retained");
    QVERIFY(!importNativeProjectSource(nativeOracle().left(3300), &project, &error));
    QCOMPARE(project.name, QStringLiteral("retained"));
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QVERIFY(error.contains(QStringLiteral("ASCII")));
    QCOMPARE(output, QByteArray("retained"));
    QVERIFY(!importNativeProjectSource(nativeOracle(), nullptr, nullptr));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::supportedEditsKeepNativeStructure
 * Description     : Reparse scalar, longer UTF-16 text, address and geometry edits with native metadata retained.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::supportedEditsKeepNativeStructure() //20261109-0 add
{
    HmiProject project;
    HmiProject restored;
    HmiProject rejected;
    QByteArray output;
    QByteArray kept;
    QByteArray bad;
    QString error;
    QFile exported;
    QString exportDirectory;

    QVERIFY2(importNativeProjectSource(nativeOracle(), &project, &error), qPrintable(error));
    project.objects[0].address = QStringLiteral("LB4096");
    project.objects[0].geometry = QRect(300, 150, 200, 90);
    project.objects[0].bitButton.operation = 0;
    project.objects[0].bitButton.pressTimeEnabled = true;
    project.objects[0].bitButton.pressTime = 12;
    project.objects[0].bitButton.states[0].flash = true;
    project.objects[0].bitButton.states[0].sharedText = QStringLiteral("启动设备");
    project.objects[0].bitButton.states[0].text.languageTexts[QStringLiteral("中文")] = QStringLiteral("启动设备");
    project.objects[0].text = QStringLiteral("启动设备");
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.objects[0].address, QStringLiteral("LB4096"));
    QCOMPARE(restored.objects[0].bitButton.monitorBitAddress, QStringLiteral("LB4096"));
    QCOMPARE(restored.objects[0].geometry, QRect(300, 150, 200, 90));
    QCOMPARE(restored.objects[0].text, QStringLiteral("启动设备"));
    QCOMPARE(restored.objects[0].bitButton.operation, 0);
    QCOMPARE(restored.objects[0].bitButton.pressTime, 12);
    QVERIFY(restored.objects[0].bitButton.pressTimeEnabled);
    QVERIFY(restored.objects[0].bitButton.states[0].flash);
    QVERIFY(nativeProjectUsesSupportedDeviceDefaults(output, &error));
    exportDirectory = qEnvironmentVariable("KINSEAL_SOURCE_EXPORT_TEST_DIR");
    if (!exportDirectory.isEmpty()) {
        exported.setFileName(QDir(exportDirectory).filePath(QStringLiteral("product-export-edited.sm")));
        QVERIFY(exported.open(QIODevice::WriteOnly));
        QCOMPARE(exported.write(output), output.size());
        exported.close();
    }
    kept = output;
    QVERIFY(exportNativeProjectSource(restored, &output, &error));
    QCOMPARE(output, kept);
    project.objects[0].bitButton.states[0].text.bold = true;
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, kept);
    bad = nativeOracle();
    bad[2752] = 11; // The oracle's typed top-level control tag is independently mapped to 2749..2753.
    QVERIFY(!importNativeProjectSource(bad, &rejected, &error));
    QVERIFY(error.contains(QStringLiteral("控件类型")));
    bad = nativeOracle();
    bad[3243] = 9; // Invalid enabled native level index at common+84 in this oracle.
    bad[3195] = 1;
    bad[3199] = 1;
    QVERIFY(!importNativeProjectSource(bad, &rejected, &error));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::savedBitUiVariantsAndProductMacroSemantics
 * Description     : Match new original bit UI variants, then separately check the explicitly product-generated macro source.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::savedBitUiVariantsAndProductMacroSemantics() //20261109-0 add
{
    HmiProject project;
    QString error;
    QStringList operations;
    QFile file;
    int i;

    operations = QStringList{QStringLiteral("set"), QStringLiteral("reset"), QStringLiteral("jog"), QStringLiteral("toggle")};
    for (i = 0; i < operations.size(); ++i) {
        file.setFileName(QDir(corpusPath()).filePath(QStringLiteral("ui-variants/operation-%1-ui.sm").arg(operations.at(i))));
        QVERIFY(file.open(QIODevice::ReadOnly));
        QVERIFY2(importNativeProjectSource(file.readAll(), &project, &error), qPrintable(error));
        file.close();
        QCOMPARE(project.objects[0].bitButton.operation, i);
    }
    file.setFileName(QDir(corpusPath()).filePath(QStringLiteral("ui-variants/notice-word7-ui.sm")));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QVERIFY2(importNativeProjectSource(file.readAll(), &project, &error), qPrintable(error));
    file.close();
    QVERIFY(project.objects[0].bitButton.notice);
    QVERIFY(project.objects[0].bitButton.noticeWord);
    QCOMPARE(project.objects[0].bitButton.noticeWordValue, QStringLiteral("7"));
    QCOMPARE(project.objects[0].bitButton.noticeWordAddress, QStringLiteral("LW0"));
    file.setFileName(QDir(QFINDTESTDATA("fixtures/native-source/bit-analysis")).filePath(QStringLiteral("macro-bound-product-seed.sm")));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QVERIFY2(importNativeProjectSource(file.readAll(), &project, &error), qPrintable(error));
    QVERIFY(project.macros.value(QStringLiteral("SetLB1")).contains(QStringLiteral("LocalBit[1] = 1")));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::inactiveDirtyConditionsBecomeCanonicalWhenEnabled
 * Description     : Preserve known inactive garbage with warnings and rewrite all fields on activation.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::inactiveDirtyConditionsBecomeCanonicalWhenEnabled() //20261109-0 add
{
    HmiProject project;
    HmiProject restored;
    QByteArray input;
    QByteArray output;
    QString error;
    NativeSourceFieldSpan level;
    NativeSourceFieldSpan comparison;

    input = nativeOracle(); //20261110-5 change: The historical dirty UI fixture is missing; inject explicit dormant garbage into a real retained seed instead.
    level = sourceFieldSpan(input, QStringLiteral("object[0].common.58.1"));
    comparison = sourceFieldSpan(input, QStringLiteral("object[0].common.78.0"));
    QVERIFY(level.end > level.start && comparison.end > comparison.start);
    input.replace(level.start, 4, sourceWordBytes(99));
    input[comparison.start] = 99;
    QVERIFY2(importNativeProjectSource(input, &project, &error), qPrintable(error));
    QVERIFY(!project.nativeSourceWarnings.isEmpty());
    QVERIFY(exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, input);
    project.objects[0].bitButton.touch.visibilityEnabled = true;
    project.objects[0].bitButton.touch.levelEnabled = true;
    project.objects[0].bitButton.touch.minimumLevel = 1;
    project.objects[0].bitButton.touch.wordEnabled = true;
    project.objects[0].bitButton.touch.wordAddress = QStringLiteral("LW0");
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QVERIFY(restored.objects[0].bitButton.touch.visibilityEnabled);
    QVERIFY(restored.objects[0].bitButton.touch.levelEnabled);
    QCOMPARE(restored.objects[0].bitButton.touch.minimumLevel, 1);
    QVERIFY(restored.objects[0].bitButton.touch.wordEnabled);
    QCOMPARE(restored.objects[0].bitButton.touch.dataType, 0);
    QCOMPARE(restored.objects[0].bitButton.touch.comparison, 0);
    QVERIFY(!restored.nativeSourceWarnings.join(QString()).contains(QStringLiteral("common.touch 的未启用权限等级")));
    QVERIFY(!restored.nativeSourceWarnings.join(QString()).contains(QStringLiteral("common.touch 的未启用比较符")));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::createAddCopyDeleteNativeButtons
 * Description     : Build a new native project from the typed seed, then add, copy and delete controls.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::createAddCopyDeleteNativeButtons() //20261109-0 add
{
    HmiProject project;
    HmiProject restored;
    HmiProject blank;
    HmiObject object;
    QByteArray output;
    QByteArray kept;
    QString error;
    QString directory;
    QFile exported;
    int i;

    project.name = QStringLiteral("新建四操作");
    for (i = 0; i < 4; ++i) {
        object = HmiObject();
        object.id = QStringLiteral("new-%1").arg(i);
        object.type = QStringLiteral("switch");
        object.name = QStringLiteral("按钮%1").arg(i + 1);
        object.address = QStringLiteral("LB%1").arg(i);
        object.text = QStringLiteral("OFF");
        object.geometry = QRect(40 + 180 * i, 130, 120, 60);
        object.hasBitButtonProperties = true;
        object.bitButton.operation = i;
        QVERIFY(object.bitButton.validate(&error));
        project.objects.append(object);
    }
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.objects.size(), 4);
    for (i = 0; i < 4; ++i) {
        QCOMPARE(restored.objects[i].bitButton.operation, i);
        QCOMPARE(restored.objects[i].geometry, project.objects[i].geometry);
        QCOMPARE(restored.objects[i].address, project.objects[i].address);
    }
    directory = qEnvironmentVariable("KINSEAL_SOURCE_EXPORT_TEST_DIR");
    if (!directory.isEmpty()) {
        exported.setFileName(QDir(directory).filePath(QStringLiteral("product-new-multiple.sm")));
        QVERIFY(exported.open(QIODevice::WriteOnly));
        QCOMPARE(exported.write(output), output.size());
        exported.close();
    }
    kept = output;
    QVERIFY(exportNativeProjectSource(restored, &output, &error));
    QCOMPARE(output, kept);
    object = restored.objects[0];
    object.id = QStringLiteral("copied-control");
    object.name = QStringLiteral("复制按钮");
    object.geometry.moveTo(70, 260);
    restored.objects.append(object);
    QVERIFY2(exportNativeProjectSource(restored, &output, &error), qPrintable(error));
    QVERIFY(importNativeProjectSource(output, &restored, &error));
    QCOMPARE(restored.objects.size(), 5);
    QCOMPARE(restored.objects[4].geometry.topLeft(), QPoint(70, 260));
    restored.objects.removeAt(2);
    QVERIFY2(exportNativeProjectSource(restored, &output, &error), qPrintable(error));
    QVERIFY(importNativeProjectSource(output, &restored, &error));
    QCOMPARE(restored.objects.size(), 4);
    restored.objects.clear();
    QVERIFY2(exportNativeProjectSource(restored, &output, &error), qPrintable(error));
    QVERIFY(importNativeProjectSource(output, &restored, &error));
    QVERIFY(restored.objects.isEmpty());
    QVERIFY2(exportNativeProjectSource(blank, &output, &error), qPrintable(error));
    QVERIFY(importNativeProjectSource(output, &restored, &error));
    QVERIFY(restored.objects.isEmpty());
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::mixedNativeButtonsAndStaticTitles
 * Description     : Preserve every control and title from the real four-operation device project.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::mixedNativeButtonsAndStaticTitles() //20261109-0 add
{
    HmiProject project;
    HmiProject restored;
    HmiProject seedProject;
    QByteArray output;
    QByteArray original;
    QByteArray kept;
    QString error;
    QString path;
    QString directory;
    QFile file(QStringLiteral(":/samples/native_text.sm"));
    int i;
    int textCount;
    int buttonCount;

    QVERIFY(file.open(QIODevice::ReadOnly));
    original = file.readAll();
    file.close();
    QVERIFY2(importNativeProjectSource(original, &seedProject, &error), qPrintable(error));
    QVERIFY2(exportNativeProjectSource(seedProject, &output, &error), qPrintable(error));
    QCOMPARE(output, original);
    path = QDir(QFINDTESTDATA("../samples/bit-button")).filePath(QStringLiteral("四操作实机验收.ksp")); //20261110-5 change: QFINDTESTDATA receives only ASCII; QStringLiteral preserves the real Unicode sample name.
    QVERIFY2(HmiProject::load(path, &project, &error), qPrintable(error));
    QCOMPARE(project.objects.size(), 8);
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.objects.size(), project.objects.size());
    textCount = 0;
    buttonCount = 0;
    for (i = 0; i < project.objects.size(); ++i) {
        QCOMPARE(restored.objects[i].type, project.objects[i].type);
        QCOMPARE(restored.objects[i].geometry, project.objects[i].geometry);
        QCOMPARE(restored.objects[i].name, project.objects[i].name);
        if (project.objects[i].type == QStringLiteral("text")) {
            ++textCount;
            QCOMPARE(restored.objects[i].text, project.objects[i].text);
            QCOMPARE(restored.objects[i].staticText.toJson(), project.objects[i].staticText.toJson());
        } else {
            ++buttonCount;
            QCOMPARE(restored.objects[i].address, project.objects[i].address);
            QCOMPARE(restored.objects[i].bitButton.operation, project.objects[i].bitButton.operation);
        }
    }
    QCOMPARE(textCount, 4);
    QCOMPARE(buttonCount, 4);
    directory = qEnvironmentVariable("KINSEAL_SOURCE_EXPORT_TEST_DIR");
    if (!directory.isEmpty()) {
        file.setFileName(QDir(directory).filePath(QStringLiteral("product-mixed-four-operations.sm")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(output), output.size());
        file.close();
    }
    kept = output;
    QVERIFY2(exportNativeProjectSource(restored, &output, &error), qPrintable(error));
    QCOMPARE(output, kept);
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeExternalWordTypeOracles
 * Description     : Distinguish native numeric type integers from the adjacent level byte using original UI samples.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::nativeExternalWordTypeOracles() //20261109-0 add
{
    HmiProject project;
    QByteArray input;
    QByteArray output;
    QString error;
    QString directory;
    QStringList types;
    QString path;
    QFile file;
    const int modelTypes[3] = {1, 3, 4};
    int i;

    directory = QFINDTESTDATA("fixtures/native-source/bit/touch-word32");
    types = QStringList{QStringLiteral("u32"), QStringLiteral("i32"), QStringLiteral("f32")};
    for (i = 0; i < types.size(); ++i) {
        file.setFileName(QDir(directory).filePath(QStringLiteral("external-touch-%1-4x0-ui.sm").arg(types.at(i))));
        QVERIFY(file.open(QIODevice::ReadOnly));
        input = file.readAll();
        file.close();
        QVERIFY2(importNativeProjectSource(input, &project, &error), qPrintable(error));
        QCOMPARE(project.objects[0].bitButton.touch.dataType, modelTypes[i]);
        QCOMPARE(project.objects[0].bitButton.touch.minimumLevel, 1);
        QCOMPARE(project.objects[0].bitButton.touch.wordAddress, QStringLiteral("@com1:4x0"));
        QVERIFY(project.objects[0].bitButton.touch.visibilityEnabled);
        QVERIFY(project.objects[0].bitButton.touch.wordEnabled);
        QCOMPARE(project.objects[0].bitButton.touch.compareValue, QStringLiteral("7"));
        QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
        QCOMPARE(output, input);
    }
    path = QFINDTESTDATA("fixtures/native-source/static/level3.sm");
    file.setFileName(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    input = file.readAll();
    file.close();
    QVERIFY2(importNativeProjectSource(input, &project, &error), qPrintable(error));
    QVERIFY(project.objects[0].staticText.visibilityEnabled);
    QVERIFY(project.objects[0].staticText.levelEnabled);
    QCOMPARE(project.objects[0].staticText.minimumLevel, 3);
    QCOMPARE(project.objects[0].staticText.comparison, 0);
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QCOMPARE(output, input);
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::unsupportedMetadataEditsAreTransactional
 * Description     : Reject changed native connection, language and macro metadata without replacing the caller output.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::unsupportedMetadataEditsAreTransactional() //20261109-0 add
{
    HmiProject original;
    HmiProject project;
    PlcConnection connection;
    QByteArray output("retained");
    QString error;

    QVERIFY2(importNativeProjectSource(nativeOracle(), &original, &error), qPrintable(error));
    project = original;
    connection.id = QStringLiteral("changed-profile");
    connection.profileId = QStringLiteral("unverified-profile.ini");
    project.connections.append(connection);
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, QByteArray("retained"));
    project = original;
    project.languages.append(QStringLiteral("中文"));
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, QByteArray("retained"));
    project = original;
    project.macros.insert(QStringLiteral("Changed-Macro"), QStringLiteral("void Macro_Entry(void) {}"));
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, QByteArray("retained"));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeMultilingualSourcesAndLanguageDefaults
 * Description     : Round-trip original multilingual controls and generate complete new language rows without widening metadata support.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::nativeMultilingualSourcesAndLanguageDefaults() //20261109-0 add
{
    HmiProject project;
    HmiProject restored;
    HmiProject rejected;
    QString directory;
    QString error;
    QStringList fixtures;
    QString path;
    QByteArray input;
    QByteArray output;
    QByteArray kept;
    QByteArray bad;
    QFile file;
    int i;
    NativeSourceFieldSpan font; //20261110-5 add

    directory = QFINDTESTDATA("fixtures/native-source/bit-analysis");
    fixtures = QStringList{QStringLiteral("../bit/ui-variants/bilingual-native-ui.sm"), QStringLiteral("mixed-two-language-distinct-native-resaved.sm")};
    for (i = 0; i < fixtures.size(); ++i) {
        file.setFileName(QDir(directory).filePath(fixtures.at(i)));
        QVERIFY(file.open(QIODevice::ReadOnly));
        input = file.readAll();
        file.close();
        QVERIFY2(importNativeProjectSource(input, &project, &error), qPrintable(error));
        QCOMPARE(project.languages, (QStringList{QStringLiteral("中文"), QStringLiteral("English")}));
        QVERIFY2(nativeProjectUsesSupportedDeviceDefaults(input, &error), qPrintable(error));
        QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
        QCOMPARE(output, input);
    }
    path = QDir(QFINDTESTDATA("../samples/bit-button")).filePath(QStringLiteral("四操作实机验收.ksp")); //20261110-5 change
    QVERIFY2(HmiProject::load(path, &project, &error), qPrintable(error));
    project.languages.append(QStringLiteral("English"));
    for (i = 0; i < project.objects.size(); ++i) {
        if (project.objects[i].type == QStringLiteral("text")) {
            project.objects[i].staticText.languageTexts.insert(QStringLiteral("English"), QStringLiteral("English title %1").arg(i));
        } else {
            project.objects[i].bitButton.states[0].text.languageTexts.insert(QStringLiteral("English"), QStringLiteral("STOP %1").arg(i));
            project.objects[i].bitButton.states[1].text.languageTexts.insert(QStringLiteral("English"), QStringLiteral("RUN %1").arg(i));
        }
    }
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.languages, project.languages);
    QCOMPARE(restored.objects.size(), project.objects.size());
    QVERIFY2(nativeProjectUsesSupportedDeviceDefaults(output, &error), qPrintable(error));
    for (i = 0; i < project.objects.size(); ++i) {
        if (project.objects[i].type == QStringLiteral("text")) {
            QCOMPARE(restored.objects[i].staticText.languageTexts, project.objects[i].staticText.languageTexts);
        } else {
            QCOMPARE(restored.objects[i].bitButton.states[0].text.languageTexts, project.objects[i].bitButton.states[0].text.languageTexts);
            QCOMPARE(restored.objects[i].bitButton.states[1].text.languageTexts, project.objects[i].bitButton.states[1].text.languageTexts);
        }
    }
    kept = output;
    QVERIFY2(exportNativeProjectSource(restored, &output, &error), qPrintable(error));
    QCOMPARE(output, kept);
    path = qEnvironmentVariable("KINSEAL_SOURCE_EXPORT_TEST_DIR");
    if (!path.isEmpty()) {
        file.setFileName(QDir(path).filePath(QStringLiteral("product-multilingual-mixed.sm")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(output), output.size());
        file.close();
    }
    file.setFileName(QDir(directory).filePath(QStringLiteral("../bit/ui-variants/bilingual-native-ui.sm")));
    QVERIFY(file.open(QIODevice::ReadOnly));
    bad = file.readAll();
    font = sourceFieldSpan(bad, QStringLiteral("object[0].switch.290[0][1].2")); //20261110-5 change: New original re-save fixtures have different metadata lengths.
    QVERIFY(font.end > font.start);
    bad.replace(font.start, 4, sourceWordBytes(22));
    rejected.name = QStringLiteral("retained");
    QVERIFY(!importNativeProjectSource(bad, &rejected, &error));
    QCOMPARE(rejected.name, QStringLiteral("retained"));
    QVERIFY(error.contains(QStringLiteral("各语言")));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeStartupLanguageRemainsIndependentOfEditorSelection
 * Description     : Keep the firmware startup language by name and reject invalid language or unrelated system bytes.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::nativeStartupLanguageRemainsIndependentOfEditorSelection() //20261109-0 add
{
    HmiProject project;
    HmiProject blank;
    QString path;
    QString error;
    QFile file;
    QByteArray input;
    QByteArray bad;
    u16 initial;

    path = QFINDTESTDATA("fixtures/native-source/bit-analysis/two-language-startup-english-ui-native-resaved.sm");
    file.setFileName(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    input = file.readAll();
    QVERIFY2(importNativeProjectSource(input, &project, &error), qPrintable(error));
    QVERIFY2(nativeProjectUsesSupportedDeviceDefaults(input, &error), qPrintable(error));
    initial = 99;
    QVERIFY(nativeProjectInitialLanguageIndex(blank, &initial, &error));
    QCOMPARE(initial, 0);
    QVERIFY(nativeProjectInitialLanguageIndex(project, &initial, &error));
    QCOMPARE(initial, 1);
    project.activeLanguage = QStringLiteral("中文");
    QVERIFY(nativeProjectInitialLanguageIndex(project, &initial, &error));
    QCOMPARE(initial, 1);
    project.languages = QStringList{QStringLiteral("English"), QStringLiteral("中文")};
    QVERIFY(nativeProjectInitialLanguageIndex(project, &initial, &error));
    QCOMPARE(initial, 0);
    project.languages = QStringList{QStringLiteral("中文")};
    QVERIFY(nativeProjectInitialLanguageIndex(project, &initial, &error));
    QCOMPARE(initial, 0);
    project.nativeSourceSnapshot = input.left(100);
    initial = 99;
    QVERIFY(!nativeProjectInitialLanguageIndex(project, &initial, &error));
    QCOMPARE(initial, 99);
    project.nativeSourceSnapshot = input;
    project.nativeSourceSnapshot[334] = 7; // Independent typed trace: startup short at 333..335 cannot reference a seventh language.
    QVERIFY(!nativeProjectInitialLanguageIndex(project, &initial, &error));
    QCOMPARE(initial, 99);
    QVERIFY(!nativeProjectUsesSupportedDeviceDefaults(project.nativeSourceSnapshot, &error));
    bad = nativeOracle();
    bad[347] = 1; // Independent baseline trace: system.flags.0 is unrelated to the language whitelist.
    QVERIFY(validateNativeProjectSourceStructure(bad, &error));
    QVERIFY(!nativeProjectUsesSupportedDeviceDefaults(bad, &error));
    QVERIFY(!nativeProjectInitialLanguageIndex(project, nullptr, nullptr));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeIndicatorStateListsCanShrinkAndGrow
 * Description     : Resize parsed native visual lists and restore an independently edited second state.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::nativeIndicatorStateListsCanShrinkAndGrow() //20261109-0 add
{
    HmiProject project;
    HmiProject restored;
    BitButtonProperties defaults;
    QString error;
    QString directory;
    QByteArray output;
    QByteArray kept;
    QFile file;

    QVERIFY2(importNativeProjectSource(nativeOracle(), &project, &error), qPrintable(error));
    project.objects[0].bitButton.indicator = false;
    project.objects[0].bitButton.dualMonitor = false;
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QVERIFY(!restored.objects[0].bitButton.indicator);
    QCOMPARE(restored.objects[0].bitButton.states[0].sharedText, QStringLiteral("OFF"));
    QVERIFY(restored.nativeSourceWarnings.join(QString()).contains(QStringLiteral("仅保存一个显示状态")));
    directory = qEnvironmentVariable("KINSEAL_SOURCE_EXPORT_TEST_DIR");
    if (!directory.isEmpty()) {
        file.setFileName(QDir(directory).filePath(QStringLiteral("product-indicator-disabled.sm")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(output), output.size());
        file.close();
    }
    project = restored;
    project.objects[0].bitButton.indicator = true;
    project.objects[0].bitButton.dualMonitor = true;
    project.objects[0].bitButton.states[1] = defaults.states[1];
    project.objects[0].bitButton.states[1].sharedText = QStringLiteral("ACTIVE");
    project.objects[0].bitButton.states[1].text.languageTexts[QStringLiteral("中文")] = QStringLiteral("ACTIVE");
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QVERIFY(restored.objects[0].bitButton.indicator);
    QVERIFY(restored.objects[0].bitButton.dualMonitor);
    QCOMPARE(restored.objects[0].bitButton.states[0].sharedText, QStringLiteral("OFF"));
    QCOMPARE(restored.objects[0].bitButton.states[1].sharedText, QStringLiteral("ACTIVE"));
    QCOMPARE(restored.objects[0].bitButton.states[1].picture, QStringLiteral("switch_ON.kvg"));
    kept = output;
    QVERIFY2(exportNativeProjectSource(restored, &output, &error), qPrintable(error));
    QCOMPARE(output, kept);
    if (!directory.isEmpty()) {
        file.setFileName(QDir(directory).filePath(QStringLiteral("product-indicator-restored.sm")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(output), output.size());
    }
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::productMacroSourcesAndBindingsRemainEditable
 * Description     : Save product-generated macro sources and bindings; the native UI rejection counterexample is tested separately.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::productMacroSourcesAndBindingsRemainEditable() //20261109-0 add
{
    HmiProject project;
    HmiProject restored;
    QString directory;
    QString error;
    QString source;
    QByteArray input;
    QByteArray output;
    QByteArray kept;
    QByteArray bad;
    QFile file;
    NativeSourceFieldSpan entry; //20261110-5 add
    NativeSourceFieldSpan flag;

    directory = QFINDTESTDATA("fixtures/native-source/bit-analysis");
    file.setFileName(QDir(directory).filePath(QStringLiteral("macro-bound-product-seed.sm")));
    QVERIFY(file.open(QIODevice::ReadOnly));
    input = file.readAll();
    file.close();
    QVERIFY2(importNativeProjectSource(input, &project, &error), qPrintable(error));
    QVERIFY(project.objects[0].bitButton.macroEnabled);
    QCOMPARE(project.objects[0].bitButton.macroName, QStringLiteral("SetLB1"));
    QCOMPARE(project.objects[0].bitButton.states[0].text.fontFamily, QStringLiteral("宋体"));
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QCOMPARE(output, input);
    source = QStringLiteral("#include \"macro.h\"\nvoid Macro_Entry(void)\n{\n    LocalBit[2] = 1;\n}\n");
    project.macros[QStringLiteral("SetLB1")] = source;
    project.macros.insert(QStringLiteral("SetLB3"), source.replace(QStringLiteral("[2]"), QStringLiteral("[3]")));
    project.objects[0].bitButton.macroName = QStringLiteral("SetLB3");
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(nativeProjectUsesSupportedDeviceDefaults(output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.macros, project.macros);
    QVERIFY(restored.objects[0].bitButton.macroEnabled);
    QCOMPARE(restored.objects[0].bitButton.macroName, QStringLiteral("SetLB3"));
    QVERIFY(restored.nativeSourceWarnings.join(QString()).contains(QStringLiteral("未编译源码")));
    kept = output;
    QVERIFY2(exportNativeProjectSource(restored, &output, &error), qPrintable(error));
    QCOMPARE(output, kept);
    directory = qEnvironmentVariable("KINSEAL_SOURCE_EXPORT_TEST_DIR");
    if (!directory.isEmpty()) {
        file.setFileName(QDir(directory).filePath(QStringLiteral("product-macro-edited.sm")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(output), output.size());
        file.close();
    }
    project = restored;
    project.macros.remove(QStringLiteral("SetLB1"));
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    kept = output;
    project.macros.remove(QStringLiteral("SetLB3"));
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, kept);
    bad = input;
    entry = sourceFieldSpan(input, QStringLiteral("object[0].switch.22c.3")); //20261110-5 change: Use the new retained native re-save boundary.
    QVERIFY(entry.end > entry.start);
    bad[entry.end - 1] = 'X';
    QVERIFY(!importNativeProjectSource(bad, &project, &error));
    bad = input;
    flag = sourceFieldSpan(input, QStringLiteral("history.1e4[0].head.2"));
    QVERIFY(flag.end > flag.start);
    bad[flag.start] = 0; //20261110-5 change: Reject the recovered required source-entry flag independently of compiled/uncompiled cache state.
    QVERIFY(!importNativeProjectSource(bad, &project, &error));
    QCOMPARE(output, kept);
}

/******************************************************************************
 * Function Name   : wordSourceOracle
 * Description     : Read the verified original subtype-one source independently of the product seed resource.
 * Input Parameters: None
 * Return Value    : Original word-button source bytes.
 * Date            : 2026.11.09
 ******************************************************************************/
static QByteArray wordSourceOracle() //20261109-6 add
{
    QFile file(QFINDTESTDATA("fixtures/native-source/word/word-native-seed.sm"));

    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    return file.readAll();
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeWordButtonCorpusKeepsExactBytes
 * Description     : Traverse and semantically round-trip original saved subtype-one sources, excluding failed empty-canvas probes.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::nativeWordButtonCorpusKeepsExactBytes() //20261109-6 add
{
    QString directory;
    QString error;
    QByteArray source;
    QByteArray restored;
    HmiProject project;
    QFile file;
    int count;
    int index;

    std::unique_ptr<QDirIterator> iterator; //20261110-5 add

    directory = QFINDTESTDATA("fixtures/native-source/word");
    QVERIFY2(!directory.isEmpty() && QDir(directory).exists(), "Frozen source fixtures are required before creating an iterator."); //20261110-5 add
    iterator = std::make_unique<QDirIterator>(directory, QStringList{QStringLiteral("word-*.sm")}, QDir::Files);
    count = 0;
    while (iterator->hasNext()) {
        file.setFileName(iterator->next());
        QVERIFY(file.open(QIODevice::ReadOnly));
        source = file.readAll();
        file.close();
        if (source.size() < 4000) {
            continue;
        }
        QVERIFY2(validateNativeProjectSourceStructure(source, &error), qPrintable(file.fileName() + QStringLiteral(": ") + error));
        QVERIFY2(importNativeProjectSource(source, &project, &error), qPrintable(file.fileName() + QStringLiteral(": ") + error));
        QVERIFY(!project.objects.isEmpty());
        for (index = 0; index < project.objects.size(); ++index) {
            QVERIFY(project.objects[index].hasWordButtonProperties);
            QVERIFY(!project.objects[index].bitButton.indicator);
            QVERIFY(!project.objects[index].bitButton.dualMonitor);
        }
        QVERIFY2(exportNativeProjectSource(project, &restored, &error), qPrintable(error));
        QCOMPARE(restored, source);
        ++count;
    }
    QVERIFY(count >= 1);
    qInfo() << "Original word-button source snapshots checked:" << count;
    QVERIFY2(importNativeProjectSource(wordSourceOracle(), &project, &error), qPrintable(error));
    QCOMPARE(project.objects[0].address, QStringLiteral("LW0"));
    QCOMPARE(project.objects[0].wordButton.dataType, 2);
    QCOMPARE(project.objects[0].wordButton.operation, 0);
    QCOMPARE(project.objects[0].wordButton.constantValue, QStringLiteral("1"));
    QVERIFY(!project.objects[0].wordButton.pressedEffect);
    QCOMPARE(project.objects[0].bitButton.states[0].background, QColor(QStringLiteral("#f0f0f0")));
    QVERIFY2(nativeProjectUsesSupportedDeviceDefaults(wordSourceOracle(), &error), qPrintable(error));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeWordButtonOperationsAndNewMixedProjects
 * Description     : Verify all seven editor types and operations, then construct a multilingual mixed source with shared macros.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::nativeWordButtonOperationsAndNewMixedProjects() //20261109-6 add
{
    HmiProject project;
    HmiProject restored;
    HmiProject matrix;
    HmiProject bitProject;
    HmiProject textProject;
    HmiObject object;
    QByteArray output;
    QByteArray kept;
    QString error;
    QString directory;
    QFile file;
    int type;
    int operation;

    QVERIFY2(importNativeProjectSource(wordSourceOracle(), &matrix, &error), qPrintable(error));
    matrix.nativeSourceSnapshot.clear();
    matrix.nativeSourceWarnings.clear(); //20261110-5 change: A newly constructed project must discard both native provenance fields.
    matrix.name = QStringLiteral("WordMatrix49");
    matrix.objects.clear();
    for (type = 0; type < 7; ++type) {
        for (operation = 0; operation < 7; ++operation) {
            QVERIFY2(importNativeProjectSource(wordSourceOracle(), &project, &error), qPrintable(error));
            project.objects[0].wordButton.resetDefaultsForDataType(type);
            project.objects[0].wordButton.operation = operation;
            project.objects[0].wordButton.constantValue = type == 6 ? QStringLiteral("2.5") : QStringLiteral("2");
            project.objects[0].wordButton.minimumValue = QStringLiteral("0");
            project.objects[0].wordButton.maximumValue = QStringLiteral("100");
            project.objects[0].wordButton.totalDigits = 5;
            project.objects[0].wordButton.decimalDigits = operation == 1 ? 2 : 0;
            QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
            QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
            QCOMPARE(restored.objects[0].wordButton.dataType, type);
            QCOMPARE(restored.objects[0].wordButton.operation, operation);
            if (project.objects[0].wordButton.usesConstant()) {
                QCOMPARE(restored.objects[0].wordButton.constantValue, project.objects[0].wordButton.constantValue);
            }
            if (project.objects[0].wordButton.usesMinimum()) {
                QCOMPARE(restored.objects[0].wordButton.minimumValue, QStringLiteral("0"));
            }
            if (project.objects[0].wordButton.usesMaximum()) {
                QCOMPARE(restored.objects[0].wordButton.maximumValue, QStringLiteral("100"));
            }
            if (project.objects[0].wordButton.usesTotalDigits()) {
                QCOMPARE(restored.objects[0].wordButton.totalDigits, 5);
            }
            if (project.objects[0].wordButton.usesDecimalDigits()) {
                QCOMPARE(restored.objects[0].wordButton.decimalDigits, 2);
            }
            kept = output;
            QVERIFY2(exportNativeProjectSource(restored, &output, &error), qPrintable(error));
            QCOMPARE(output, kept);
            object = project.objects[0];
            object.id = QStringLiteral("word-matrix-%1-%2").arg(type).arg(operation);
            object.name = QStringLiteral("W_%1_%2").arg(type).arg(operation);
            object.address = QStringLiteral("LW%1").arg((type * 7 + operation) * 2);
            object.geometry = QRect(10 + operation * 110, 10 + type * 65, 100, 55);
            matrix.objects.append(object);
        }
    }
    QVERIFY2(exportNativeProjectSource(matrix, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.objects.size(), 49);
    directory = qEnvironmentVariable("KINSEAL_WORD_SOURCE_EXPORT_TEST_DIR");
    if (!directory.isEmpty()) {
        file.setFileName(QDir(directory).filePath(QStringLiteral("product-word-type-operation-matrix-49.sm")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(output), output.size());
        file.close();
    }
    QVERIFY2(importNativeProjectSource(wordSourceOracle(), &project, &error), qPrintable(error));
    project.nativeSourceSnapshot.clear();
    project.nativeSourceWarnings.clear(); //20261110-5 change: Imported diagnostics cannot outlive the intentionally discarded source snapshot.
    project.name = QStringLiteral("WordMixedSource");
    project.languages.append(QStringLiteral("English"));
    project.activeLanguage = QStringLiteral("English");
    project.objects[0].id = QStringLiteral("word-created-1");
    project.objects[0].name = QStringLiteral("Word Input");
    project.objects[0].address = QStringLiteral("LW20");
    project.objects[0].geometry = QRect(30, 40, 140, 70);
    project.objects[0].wordButton.resetDefaultsForDataType(6);
    project.objects[0].wordButton.operation = 1;
    project.objects[0].wordButton.pressedEffect = true;
    project.objects[0].wordButton.decimalDigits = 2;
    project.objects[0].bitButton.states[0].text.languageTexts[QStringLiteral("English")] = QStringLiteral("INPUT");
    project.objects[0].bitButton.states[1].text.languageTexts[QStringLiteral("English")] = QStringLiteral("PRESSED");
    project.macros.insert(QStringLiteral("WordMacro"), QStringLiteral("#include \"macro.h\"\nvoid Macro_Entry(void) { LocalWord[20] = 7; }\n"));
    project.objects[0].bitButton.macroEnabled = true;
    project.objects[0].bitButton.macroName = QStringLiteral("WordMacro");
    QVERIFY2(importNativeProjectSource(nativeOracle(), &bitProject, &error), qPrintable(error));
    object = bitProject.objects.first();
    object.id = QStringLiteral("bit-created-2");
    project.objects.append(object);
    file.setFileName(QStringLiteral(":/samples/native_text.sm"));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QVERIFY2(importNativeProjectSource(file.readAll(), &textProject, &error), qPrintable(error));
    file.close();
    object = textProject.objects.first();
    object.id = QStringLiteral("text-created-3");
    project.objects.append(object);
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(nativeProjectUsesSupportedDeviceDefaults(output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.objects.size(), 3);
    QCOMPARE(restored.languages, project.languages);
    QCOMPARE(restored.objects[0].address, QStringLiteral("LW20"));
    QVERIFY(restored.objects[0].wordButton.pressedEffect);
    QCOMPARE(restored.objects[0].bitButton.states[1].text.languageTexts.value(QStringLiteral("English")), QStringLiteral("PRESSED"));
    QCOMPARE(restored.macros, project.macros);
    QCOMPARE(restored.objects[0].bitButton.macroName, QStringLiteral("WordMacro"));
    QVERIFY(!restored.objects[1].hasWordButtonProperties);
    QCOMPARE(restored.objects[2].type, QStringLiteral("text"));
    kept = output;
    QVERIFY2(exportNativeProjectSource(restored, &output, &error), qPrintable(error));
    QCOMPARE(output, kept);
    directory = qEnvironmentVariable("KINSEAL_WORD_SOURCE_EXPORT_TEST_DIR");
    if (!directory.isEmpty()) {
        file.setFileName(QDir(directory).filePath(QStringLiteral("product-new-word-mixed-multilingual.sm")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(output), output.size());
    }
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeWordButtonPressedStatesAndGuards
 * Description     : Grow and shrink pressed states while rejecting unknown types, inconsistent flags and hidden bit edits transactionally.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::nativeWordButtonPressedStatesAndGuards() //20261109-6 add
{
    HmiProject project;
    HmiProject restored;
    QByteArray output;
    QByteArray kept;
    QByteArray bad;
    QString error;

    QVERIFY2(importNativeProjectSource(wordSourceOracle(), &project, &error), qPrintable(error));
    project.objects[0].wordButton.pressedEffect = true;
    project.objects[0].bitButton.states[1].sharedText = QStringLiteral("DOWN");
    project.objects[0].bitButton.states[1].text.languageTexts[QStringLiteral("中文")] = QStringLiteral("DOWN");
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QVERIFY(restored.objects[0].wordButton.pressedEffect);
    QCOMPARE(restored.objects[0].bitButton.states[1].sharedText, QStringLiteral("DOWN"));
    project = restored;
    project.objects[0].wordButton.pressedEffect = false;
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QVERIFY(!restored.objects[0].wordButton.pressedEffect);
    QCOMPARE(restored.objects[0].bitButton.states[1].sharedText, restored.objects[0].bitButton.states[0].sharedText);
    kept = output;
    project = restored;
    project.objects[0].bitButton.operation = 0;
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, kept);
    project = restored;
    project.objects[0].wordButton.operation = 1;
    project.objects[0].wordButton.minimumValue = QStringLiteral("10");
    project.objects[0].wordButton.maximumValue = QStringLiteral("1");
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, kept);
    project.name = QStringLiteral("KeepOnFailure");
    bad = wordSourceOracle();
    bad[3528] = 99; // Original typed trace: word.16c.0 is the native data-type field at 3525..3528.
    QVERIFY(!importNativeProjectSource(bad, &project, &error));
    QCOMPARE(project.name, QStringLiteral("KeepOnFailure"));
    bad = wordSourceOracle();
    bad[3828] = 1; // The default source has one state, so an active pressed-effect tail is inconsistent.
    QVERIFY(!importNativeProjectSource(bad, &project, &error));
    QCOMPARE(project.name, QStringLiteral("KeepOnFailure"));
}

/******************************************************************************
 * Function Name   : lampSourceOracle
 * Description     : Load one original UI-created lamp source without running the vendor program.
 * Input Parameters: name
 * Return Value    : Original source bytes, or empty when the fixture is unavailable.
 * Date            : 2026.11.09
 ******************************************************************************/
static QByteArray lampSourceOracle(const QString &name = QStringLiteral("lamp-default-r1.sm")) //20261109-8 add
{
    QFile file(QDir(QFINDTESTDATA("fixtures/native-source/lamp")).filePath(name));

    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    return file.readAll();
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeLampCorpusKeepsExactBytes
 * Description     : Preserve every original supported lamp source and verify the active monitor contract.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::nativeLampCorpusKeepsExactBytes() //20261109-8 add
{
    QString directory;
    QString error;
    QByteArray source;
    QByteArray output;
    HmiProject project;
    QFile file;
    int count;
    int index;

    std::unique_ptr<QDirIterator> iterator; //20261110-5 add

    directory = QFINDTESTDATA("fixtures/native-source/lamp");
    QVERIFY2(!directory.isEmpty() && QDir(directory).exists(), "Frozen source fixtures are required before creating an iterator."); //20261110-5 add
    iterator = std::make_unique<QDirIterator>(directory, QStringList{QStringLiteral("lamp-*.sm")}, QDir::Files);
    count = 0;
    while (iterator->hasNext()) {
        file.setFileName(iterator->next());
        QVERIFY(file.open(QIODevice::ReadOnly));
        source = file.readAll();
        file.close();
        if (source.size() < 4000) {
            continue;
        }
        QVERIFY2(validateNativeProjectSourceStructure(source, &error), qPrintable(file.fileName() + QStringLiteral(": ") + error));
        QVERIFY2(importNativeProjectSource(source, &project, &error), qPrintable(file.fileName() + QStringLiteral(": ") + error));
        QVERIFY(!project.objects.isEmpty());
        for (index = 0; index < project.objects.size(); ++index) {
            QVERIFY(project.objects[index].hasLampProperties);
            QVERIFY(!project.objects[index].hasWordButtonProperties);
            QVERIFY(project.objects[index].address.isEmpty());
        }
        QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
        QCOMPARE(output, source);
        ++count;
    }
    QVERIFY(count >= 5);
    qInfo() << "Original lamp source snapshots checked:" << count;
    QVERIFY2(importNativeProjectSource(lampSourceOracle(), &project, &error), qPrintable(error));
    QCOMPARE(project.objects[0].lamp.monitorMode, 1);
    QCOMPARE(project.objects[0].bitButton.bitIndex, 0);
    QCOMPARE(project.objects[0].bitButton.monitorBitAddress, QStringLiteral("LB0"));
    QVERIFY(!project.objects[0].bitButton.monitorSame);
    QCOMPARE(project.objects[0].bitButton.states[0].sharedText, QStringLiteral("OFF"));
    QCOMPARE(project.objects[0].bitButton.states[1].sharedText, QStringLiteral("ON"));
    QCOMPARE(project.objects[0].bitButton.states[1].background, QColor(QStringLiteral("#f0f0f0")));
    QVERIFY2(nativeProjectUsesSupportedDeviceDefaults(lampSourceOracle(), &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(lampSourceOracle(QStringLiteral("lamp-wordbit7-r1.sm")), &project, &error), qPrintable(error));
    QCOMPARE(project.objects[0].bitButton.monitorType, 1);
    QCOMPARE(project.objects[0].bitButton.bitIndex, 7);
    QVERIFY2(importNativeProjectSource(lampSourceOracle(QStringLiteral("lamp-multi5-u32-r1.sm")), &project, &error), qPrintable(error));
    QCOMPARE(project.objects[0].lamp.monitorMode, 2);
    QCOMPARE(project.objects[0].lamp.multiDataType, 1);
    QCOMPARE(project.objects[0].lamp.multiStateCount, 5);
    QCOMPARE(project.objects[0].lamp.extraStates.size(), 3);
    QCOMPARE(project.objects[0].lamp.extraStates[0].text.fontFamily, QStringLiteral("微软雅黑"));
    QCOMPARE(project.objects[0].lamp.extraStates[0].picture, QStringLiteral("switch_ON.kvg"));
    QCOMPARE(project.objects[0].lamp.extraStates[0].background, QColor(QStringLiteral("#a0a0a0")));
    QVERIFY2(importNativeProjectSource(lampSourceOracle(QStringLiteral("lamp-none-r1.sm")), &project, &error), qPrintable(error));
    QCOMPARE(project.objects[0].lamp.monitorMode, 2); // Both unchecked native groups still serialize and compile as multi-state.
    QCOMPARE(project.objects[0].lamp.multiStateCount, 2);
    QVERIFY(project.nativeSourceWarnings.join(QString()).contains(QStringLiteral("状态值表为空")));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeLampMonitoringEditsAndMixedCreation
 * Description     : Save dual word-bit monitoring, then create a multilingual multi-state lamp beside old supported controls.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::nativeLampMonitoringEditsAndMixedCreation() //20261109-8 add
{
    HmiProject project;
    HmiProject restored;
    HmiProject fixture;
    HmiObject object;
    QByteArray output;
    QByteArray kept;
    QString error;
    QString directory;
    QFile file;
    int state;

    QVERIFY2(importNativeProjectSource(lampSourceOracle(), &project, &error), qPrintable(error));
    project.objects[0].bitButton.monitorType = 1;
    project.objects[0].bitButton.monitorWordAddress = QStringLiteral("LW12");
    project.objects[0].bitButton.bitIndex = 7;
    project.objects[0].bitButton.visibility.visibilityEnabled = true;
    project.objects[0].bitButton.visibility.bitEnabled = true;
    project.objects[0].bitButton.visibility.bitAddress = QStringLiteral("LB3");
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.objects[0].bitButton.monitorType, 1);
    QCOMPARE(restored.objects[0].bitButton.monitorWordAddress, QStringLiteral("LW12"));
    QCOMPARE(restored.objects[0].bitButton.bitIndex, 7);
    QVERIFY(restored.objects[0].bitButton.visibility.bitEnabled);
    QCOMPARE(restored.objects[0].bitButton.visibility.bitAddress, QStringLiteral("LB3"));
    QVERIFY2(importNativeProjectSource(lampSourceOracle(QStringLiteral("lamp-multi5-u32-r1.sm")), &project, &error), qPrintable(error));
    project.objects[0].lamp.multiDataType = 0;
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.objects[0].lamp.multiDataType, 0);
    project = restored;
    project.objects[0].lamp.multiDataType = 1;
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.objects[0].lamp.multiDataType, 1);
    project = restored;
    project.nativeSourceSnapshot.clear();
    project.nativeSourceWarnings.clear(); //20261110-5 change: This fixture becomes a new mixed project without retained native provenance.
    project.name = QStringLiteral("LampMixedSource");
    project.languages.append(QStringLiteral("English"));
    project.objects[0].id = QStringLiteral("lamp-created");
    project.objects[0].name = QStringLiteral("Multi Lamp");
    project.objects[0].lamp.multiMonitorAddress = QStringLiteral("LW20");
    project.objects[0].geometry = QRect(20, 30, 140, 70);
    for (state = 0; state < 5; ++state) {
        project.objects[0].lamp.state(project.objects[0].bitButton, state)->text.languageTexts[QStringLiteral("English")] = QStringLiteral("STATE %1").arg(state);
    }
    QVERIFY2(importNativeProjectSource(nativeOracle(), &fixture, &error), qPrintable(error));
    object = fixture.objects.first();
    object.id = QStringLiteral("lamp-mixed-bit");
    project.objects.append(object);
    QVERIFY2(importNativeProjectSource(wordSourceOracle(), &fixture, &error), qPrintable(error));
    object = fixture.objects.first();
    object.id = QStringLiteral("lamp-mixed-word");
    project.objects.append(object);
    file.setFileName(QStringLiteral(":/samples/native_text.sm"));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QVERIFY2(importNativeProjectSource(file.readAll(), &fixture, &error), qPrintable(error));
    file.close();
    object = fixture.objects.first();
    object.id = QStringLiteral("lamp-mixed-text");
    project.objects.append(object);
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(nativeProjectUsesSupportedDeviceDefaults(output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.objects.size(), 4);
    QCOMPARE(restored.languages, project.languages);
    QCOMPARE(restored.objects[0].lamp.multiDataType, 1);
    QCOMPARE(restored.objects[0].lamp.multiMonitorAddress, QStringLiteral("LW20"));
    QCOMPARE(restored.objects[0].lamp.extraStates[2].text.languageTexts.value(QStringLiteral("English")), QStringLiteral("STATE 4"));
    QVERIFY(!restored.objects[1].hasLampProperties);
    QVERIFY(restored.objects[2].hasWordButtonProperties);
    QCOMPARE(restored.objects[3].type, QStringLiteral("text"));
    kept = output;
    QVERIFY2(exportNativeProjectSource(restored, &output, &error), qPrintable(error));
    QCOMPARE(output, kept);
    directory = qEnvironmentVariable("KINSEAL_LAMP_SOURCE_EXPORT_TEST_DIR");
    if (!directory.isEmpty()) {
        file.setFileName(QDir(directory).filePath(QStringLiteral("product-new-lamp-mixed-multilingual.sm")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(output), output.size());
    }
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeLampStateExpansionAndGuards
 * Description     : Preserve the verified 32-state boundary and reject unsupported active monitor or touch metadata transactionally.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::nativeLampStateExpansionAndGuards() //20261109-8 add
{
    HmiProject project;
    HmiProject restored;
    QJsonObject draft; //20261109-8 add
    QByteArray output;
    QByteArray kept;
    QByteArray bad;
    QByteArray value;
    QDataStream stream(&value, QIODevice::WriteOnly);
    QString error;

    QVERIFY2(importNativeProjectSource(lampSourceOracle(), &project, &error), qPrintable(error));
    project.objects[0].lamp.monitorMode = 2;
    project.objects[0].lamp.multiStateCount = 32;
    project.objects[0].lamp.ensureStateCount(32, project.objects[0].bitButton);
    project.objects[0].bitButton.dualMonitor = false;
    project.objects[0].lamp.extraStates[29].sharedText = QStringLiteral("LAST");
    project.objects[0].lamp.extraStates[29].text.languageTexts[QStringLiteral("中文")] = QStringLiteral("LAST");
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.objects[0].lamp.stateCount(), 32);
    QCOMPARE(restored.objects[0].lamp.extraStates[29].sharedText, QStringLiteral("LAST"));
    project = restored;
    project.objects[0].lamp.monitorMode = 1;
    project.objects[0].bitButton.dualMonitor = true;
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.objects[0].lamp.monitorMode, 1);
    QCOMPARE(restored.objects[0].lamp.stateCount(), 2);
    kept = output;
    project = restored;
    project.objects[0].bitButton.touch.minimumLevel = 2; //20261109-8 add: Match the GUI factory's disabled touch draft rather than the native seed's defaults.
    project.objects[0].bitButton.touch.wordAddress = QStringLiteral("LW0");
    draft = project.toJson();
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QCOMPARE(output, kept); // Inactive touch drafts must preserve the original native bytes.
    QCOMPARE(project.toJson(), draft);
    project = restored;
    project.objects[0].lamp.monitorMode = 0;
    project.objects[0].bitButton.indicator = false;
    project.objects[0].bitButton.dualMonitor = false;
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, kept);
    project = restored;
    project.objects[0].bitButton.notice = true;
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, kept);
    project = restored;
    project.objects[0].bitButton.touch.visibilityEnabled = true;
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, kept);
    project = restored;
    project.objects[0].bitButton.pressTimeEnabled = true; //20261109-8 add: Disabled touch tolerance does not enable active press or macro behavior.
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, kept);
    project = restored;
    project.objects[0].bitButton.macroEnabled = true;
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, kept);
    project = restored;
    project.objects[0].address = QStringLiteral("LW2");
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, kept);
    project = restored;
    project.objects[0].lamp.monitorMode = 2;
    project.objects[0].lamp.multiStateCount = 33;
    project.objects[0].bitButton.dualMonitor = false;
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, kept);
    bad = lampSourceOracle();
    bad[3557] = 0; // Original Lamp source trace: switch.248.5 stores the monitor mode at 3554..3557.
    project.name = QStringLiteral("KeepLampOnFailure");
    QVERIFY(!importNativeProjectSource(bad, &project, &error));
    QCOMPARE(project.name, QStringLiteral("KeepLampOnFailure"));
    bad = lampSourceOracle(QStringLiteral("lamp-multi5-u16-r4.sm"));
    stream.setVersion(QDataStream::Qt_4_8);
    stream << 99.0;
    bad.replace(3562, 8, value); // Original typed oracle maps the first active state value to this span.
    project.name = QStringLiteral("KeepLampOnFailure");
    QVERIFY(!importNativeProjectSource(bad, &project, &error));
    QCOMPARE(project.name, QStringLiteral("KeepLampOnFailure"));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeLampNoImageAndSharedText
 * Description     : Verify the original rectangle-child option, global shared-language flag and both preview-child conversions.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::nativeLampNoImageAndSharedText() //20261109-8 add
{
    HmiProject project;
    HmiProject restored;
    QJsonObject draft;
    QByteArray output;
    QByteArray kept;
    QString error;
    QString directory;
    QFile file;
    BitButtonState *appearance;
    int state;

    QVERIFY2(importNativeProjectSource(lampSourceOracle(QStringLiteral("lamp-multi5-state2-noimage-r1.sm")), &project, &error), qPrintable(error));
    QVERIFY(!project.objects[0].lamp.extraStates[0].useImage);
    QCOMPARE(project.objects[0].lamp.extraStates[0].picture, QStringLiteral("switch_ON.kvg"));
    project.languages.append(QStringLiteral("English"));
    project.objects[0].geometry = QRect(40, 40, 180, 60);
    for (state = 0; state < 5; ++state) {
        appearance = project.objects[0].lamp.state(project.objects[0].bitButton, state);
        appearance->sameLanguage = true;
        appearance->sharedText = QStringLiteral("STATE %1").arg(state);
        appearance->text.languageTexts[QStringLiteral("中文")] = QStringLiteral("unused Chinese draft");
        appearance->text.languageTexts[QStringLiteral("English")] = QStringLiteral("unused English draft");
    }
    draft = project.toJson();
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QCOMPARE(project.toJson(), draft); // Source fan-out must not erase retained KSP language drafts.
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    for (state = 0; state < 5; ++state) {
        appearance = restored.objects[0].lamp.state(restored.objects[0].bitButton, state);
        QVERIFY(appearance->sameLanguage);
        QCOMPARE(appearance->text.languageTexts.value(QStringLiteral("中文")), QStringLiteral("STATE %1").arg(state));
        QCOMPARE(appearance->text.languageTexts.value(QStringLiteral("English")), QStringLiteral("STATE %1").arg(state));
    }
    QVERIFY(!restored.objects[0].lamp.extraStates[0].useImage);
    project = restored;
    project.objects[0].lamp.extraStates[0].useImage = true;
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QVERIFY(restored.objects[0].lamp.extraStates[0].useImage);
    project = restored;
    project.objects[0].lamp.extraStates[0].useImage = false;
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QVERIFY(!restored.objects[0].lamp.extraStates[0].useImage);
    QCOMPARE(restored.objects[0].lamp.extraStates[0].picture, QStringLiteral("switch_ON.kvg"));
    QCOMPARE(restored.objects[0].lamp.extraStates[0].background, QColor(QStringLiteral("#a0a0a0")));
    kept = output;
    QVERIFY2(exportNativeProjectSource(restored, &output, &error), qPrintable(error));
    QCOMPARE(output, kept);
    directory = qEnvironmentVariable("KINSEAL_LAMP_SOURCE_EXPORT_TEST_DIR");
    if (!directory.isEmpty()) {
        file.setFileName(QDir(directory).filePath(QStringLiteral("product-lamp-noimage-samelanguage.sm")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(output), output.size());
        file.close();
    }
    project = restored;
    project.objects[0].bitButton.states[1].sameLanguage = false;
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, kept);
    project = restored;
    project.objects[0].lamp.extraStates[0].text.languageTexts[QStringLiteral("Deutsch")] = QStringLiteral("unregistered");
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, kept);
    QVERIFY2(importNativeProjectSource(nativeOracle(), &project, &error), qPrintable(error));
    project.objects[0].bitButton.states[0].sameLanguage = true;
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, kept);
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeLampActualSamplesRoundTrip_data
 * Description     : Exercise the six frozen GUI-created KSP fixtures independently of native source seeds.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::nativeLampActualSamplesRoundTrip_data() //20261109-8 add
{
    QTest::addColumn<QString>("sampleName");
    QTest::newRow("u16-five") << QStringLiteral("多态U16五状态.ksp");
    QTest::newRow("u32-thirty-two") << QStringLiteral("多态U32三十二状态.ksp");
    QTest::newRow("dual-bit") << QStringLiteral("双态位地址.ksp");
    QTest::newRow("dual-word-bit") << QStringLiteral("双态字位.ksp");
    QTest::newRow("bilingual-visibility") << QStringLiteral("双语多态与显现条件.ksp");
    QTest::newRow("device-mixed") << QStringLiteral("指示灯实机联调.ksp");
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeLampActualSamplesRoundTrip
 * Description     : Save real editor projects as native source, verify active lamp semantics and preserve canonical source bytes.
 * Input Parameters: sampleName test column
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void NativeProjectCodecTests::nativeLampActualSamplesRoundTrip() //20261109-8 add
{
    QFETCH(QString, sampleName);
    HmiProject project;
    HmiProject restored;
    QJsonObject draft;
    QJsonObject beforeVisibility;
    QJsonObject afterVisibility;
    QByteArray output;
    QByteArray kept;
    QString directory;
    QString error;
    QFile file;
    const HmiObject *before;
    const HmiObject *after;
    const BitButtonState *beforeState;
    const BitButtonState *afterState;
    int index;
    int state;
    int languageIndex;

    directory = QFINDTESTDATA("../samples/lamp");
    QVERIFY2(!directory.isEmpty(), "Frozen GUI lamp KSP fixtures are required.");
    QVERIFY2(HmiProject::load(QDir(directory).filePath(sampleName), &project, &error), qPrintable(error));
    draft = project.toJson();
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(sampleName + QStringLiteral(": ") + error));
    QCOMPARE(project.toJson(), draft); // Export must preserve inactive editor drafts in the KSP model.
    QVERIFY2(validateNativeProjectSourceStructure(output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.objects.size(), project.objects.size());
    QCOMPARE(restored.languages, project.languages);
    for (index = 0; index < project.objects.size(); ++index) {
        before = &project.objects.at(index);
        after = &restored.objects.at(index);
        QCOMPARE(after->geometry, before->geometry);
        QCOMPARE(after->hasLampProperties, before->hasLampProperties);
        if (!before->hasLampProperties) {
            continue;
        }
        QCOMPARE(after->lamp.monitorMode, before->lamp.monitorMode);
        QCOMPARE(after->lamp.stateCount(), before->lamp.stateCount());
        beforeVisibility = before->bitButton.visibility.toJson();
        afterVisibility = after->bitButton.visibility.toJson();
        beforeVisibility.remove(QStringLiteral("texts")); // Visibility conditions do not serialize the shared text-style class's unused language drafts.
        afterVisibility.remove(QStringLiteral("texts"));
        QCOMPARE(afterVisibility, beforeVisibility);
        if (before->lamp.monitorMode == 1) {
            QCOMPARE(after->bitButton.monitorType, before->bitButton.monitorType);
            if (before->bitButton.monitorType == 0) {
                QCOMPARE(after->bitButton.monitorBitAddress, before->bitButton.monitorBitAddress);
            } else {
                QCOMPARE(after->bitButton.monitorWordAddress, before->bitButton.monitorWordAddress);
                QCOMPARE(after->bitButton.bitIndex, before->bitButton.bitIndex);
            }
        } else {
            QCOMPARE(after->lamp.multiDataType, before->lamp.multiDataType);
            QCOMPARE(after->lamp.multiMonitorAddress, before->lamp.multiMonitorAddress);
        }
        for (state = 0; state < before->lamp.stateCount(); ++state) {
            beforeState = before->lamp.state(before->bitButton, state);
            afterState = after->lamp.state(after->bitButton, state);
            QVERIFY(beforeState != nullptr && afterState != nullptr);
            QCOMPARE(afterState->sameLanguage, beforeState->sameLanguage);
            QCOMPARE(afterState->useImage, beforeState->useImage);
            QCOMPARE(afterState->picture, beforeState->picture);
            QCOMPARE(afterState->background, beforeState->background);
            if (beforeState->sameLanguage) {
                QCOMPARE(afterState->sharedText, beforeState->sharedText);
                for (languageIndex = 0; languageIndex < project.languages.size(); ++languageIndex) {
                    QCOMPARE(afterState->text.languageTexts.value(project.languages.at(languageIndex)), beforeState->sharedText);
                }
            } else {
                QCOMPARE(afterState->text.languageTexts, beforeState->text.languageTexts); // A disabled shared-text draft is retained only in KSP.
            }
        }
    }
    kept = output;
    QVERIFY2(exportNativeProjectSource(restored, &output, &error), qPrintable(error));
    QCOMPARE(output, kept);
    directory = qEnvironmentVariable("KINSEAL_LAMP_SOURCE_EXPORT_TEST_DIR");
    if (!directory.isEmpty()) {
        file.setFileName(QDir(directory).filePath(sampleName.chopped(4) + QStringLiteral(".sm")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(output), output.size());
    }
}

/******************************************************************************
 * Function Name   : newFunctionSourceObject
 * Description     : Build a genuine new editor object without inheriting dormant values from a native source fixture.
 * Input Parameters: index
 * Return Value    : Single-state system-function object.
 * Date            : 2026.11.10
 ******************************************************************************/
static HmiObject newFunctionSourceObject(int index) //20261110-0 add
{
    HmiObject object;

    object.id = QStringLiteral("new-function-%1").arg(index);
    object.type = QStringLiteral("switch");
    object.name = QStringLiteral("Function %1").arg(index);
    object.text = object.bitButton.states[0].sharedText;
    object.geometry = QRect(20 + (index % 5) * 145, 20 + (index % 6) * 70, 120, 60);
    object.hasBitButtonProperties = true;
    object.bitButton.indicator = false;
    object.bitButton.dualMonitor = false;
    object.hasFunctionButtonProperties = true;
    return object;
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeFunctionCorpusKeepsExactBytes
 * Description     : Parse original function-button sources and preserve every opaque source byte on an unchanged export.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeProjectCodecTests::nativeFunctionCorpusKeepsExactBytes() //20261110-0 add
{
    QString directory;
    QString error;
    QFile file;
    QByteArray source;
    QByteArray output;
    HmiProject project;
    QSet<int> functions;
    QSet<int> keys;
    QJsonArray specifications;
    QJsonObject specification;
    int count;
    int index;
    int sourceIndex;
    bool foundSpace;

    std::unique_ptr<QDirIterator> iterator; //20261110-5 add

    directory = qEnvironmentVariable("KINSEAL_FUNCTION_SOURCE_CORPUS");
    if (directory.isEmpty()) {
        directory = QFINDTESTDATA("fixtures/native-source/function");
    }
    QVERIFY2(!directory.isEmpty() && QDir(directory).exists(), "Frozen source fixtures are required before creating an iterator."); //20261110-5 add
    iterator = std::make_unique<QDirIterator>(directory, QStringList{QStringLiteral("function-*.sm")}, QDir::Files);
    count = 0;
    while (iterator->hasNext()) {
        file.setFileName(iterator->next());
        // Early capture attempts and intentionally invalid Unicode probes are not a validated semantic oracle.
        if (!QFileInfo(file.fileName()).fileName().startsWith(QStringLiteral("function-default-")) &&
            !QFileInfo(file.fileName()).fileName().startsWith(QStringLiteral("function-valid-")) &&
            !QFileInfo(file.fileName()).fileName().startsWith(QStringLiteral("function-advanced-"))) {
            continue;
        }
        QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(file.fileName()));
        source = file.readAll();
        file.close();
        if (source.size() < 4000) {
            continue;
        }
        QVERIFY2(validateNativeProjectSourceStructure(source, &error), qPrintable(file.fileName() + QStringLiteral(": ") + error));
        QVERIFY2(importNativeProjectSource(source, &project, &error), qPrintable(file.fileName() + QStringLiteral(": ") + error));
        QVERIFY(!project.objects.isEmpty());
        for (index = 0; index < project.objects.size(); ++index) {
            QVERIFY(project.objects.at(index).hasFunctionButtonProperties);
            QVERIFY(!project.objects.at(index).hasWordButtonProperties);
            QVERIFY(!project.objects.at(index).hasLampProperties);
            QVERIFY(project.objects.at(index).address.isEmpty());
            QVERIFY(!project.objects.at(index).bitButton.indicator);
            QVERIFY(!project.objects.at(index).bitButton.dualMonitor);
            functions.insert(project.objects.at(index).functionButton.function);
            if (project.objects.at(index).functionButton.function == 14) {
                keys.insert(project.objects.at(index).functionButton.keyboardKey);
            }
        }
        QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(file.fileName() + QStringLiteral(": ") + error));
        QCOMPARE(output, source);
        ++count;
    }
    QVERIFY(count >= 1);
    QCOMPARE(functions.size(), 18);
    // The original Space radio is outside the exclusive group; normal UI capture keeps Enter checked and serializes key zero.
    QCOMPARE(keys, QSet<int>({0, 1, 2, 3, 4, 5, 6, 7, 9}));
    file.setFileName(QDir(directory).filePath(QStringLiteral("function-valid-r4.sm")));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QVERIFY2(importNativeProjectSource(file.readAll(), &project, &error), qPrintable(error));
    file.close();
    file.setFileName(QDir(directory).filePath(QStringLiteral("function-valid-r4-specs.json")));
    QVERIFY(file.open(QIODevice::ReadOnly));
    specifications = QJsonDocument::fromJson(file.readAll()).array();
    QCOMPARE(specifications.size(), project.objects.size());
    foundSpace = false;
    for (index = 0; index < specifications.size(); ++index) {
        specification = specifications.at(index).toObject();
        if (specification.value(QStringLiteral("function")).toInt() == 14 && specification.value(QStringLiteral("key")).toInt(-1) == 8) {
            sourceIndex = specifications.size() - 1 - specification.value(QStringLiteral("index")).toInt();
            QCOMPARE(project.objects.at(sourceIndex).functionButton.function, 14);
            QCOMPARE(project.objects.at(sourceIndex).functionButton.keyboardKey, 0);
            foundSpace = true;
        }
    }
    QVERIFY(foundSpace);
    qInfo() << "Original function sources:" << count << "functions:" << functions.size() << "keyboard keys:" << keys.size();
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeFunctionUnsupportedUnicodeIsExplicit
 * Description     : Retain the original malformed-surrogate probe as an opaque archive while refusing semantic conversion.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeProjectCodecTests::nativeFunctionUnsupportedUnicodeIsExplicit() //20261110-0 add
{
    QFile file(QFINDTESTDATA("fixtures/native-source/function/function-tail-r2.sm"));
    QByteArray source;
    QByteArray restored;
    NativeProjectEnvelope envelope;
    HmiProject project;
    QString error;

    QVERIFY(file.open(QIODevice::ReadOnly));
    source = file.readAll();
    QVERIFY2(validateNativeProjectSourceStructure(source, &error), qPrintable(error));
    QVERIFY2(inspectNativeProjectEnvelope(source, &envelope, &error), qPrintable(error));
    QVERIFY2(restoreNativeProjectArchive(envelope, &restored, &error), qPrintable(error));
    QCOMPARE(restored, source);
    QVERIFY(!importNativeProjectSource(source, &project, &error));
    QVERIFY(error.contains(QStringLiteral("功能按钮")));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeFunctionOpaqueMetadataAndKeyWidth
 * Description     : Preserve a nonempty opaque QString and reject nonzero high keyboard-key bits using the original writer's typed union.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeProjectCodecTests::nativeFunctionOpaqueMetadataAndKeyWidth() //20261110-0 add
{
    QFile file(QFINDTESTDATA("fixtures/native-source/function/function-default-r1.sm"));
    QByteArray source;
    QByteArray changed;
    QByteArray metadata;
    QByteArray output;
    QDataStream stream(&metadata, QIODevice::WriteOnly);
    HmiProject project;
    QString error;

    QVERIFY(file.open(QIODevice::ReadOnly));
    source = file.readAll();
    QCOMPARE(source.size(), 4137);
    // The saved original writer trace places function/type at 3489/3493, opaque QString at 3497, flags at 3501/3502 and i32 key at 3503.
    stream.setVersion(QDataStream::Qt_4_8);
    stream << QStringLiteral("opaque metadata remains intact");
    changed = source;
    changed.replace(3497, 4, metadata);
    QVERIFY2(importNativeProjectSource(changed, &project, &error), qPrintable(error));
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QCOMPARE(output, changed);
    changed = source;
    changed[3492] = 14;
    changed[3504] = 1; // This creates native keyboardKey 65536; interpreting only its low ushort would incorrectly accept Enter.
    QVERIFY2(validateNativeProjectSourceStructure(changed, &error), qPrintable(error));
    QVERIFY(!importNativeProjectSource(changed, &project, &error));
    QVERIFY(error.contains(QStringLiteral("功能按钮")));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeFunctionAllSelectionsAndFreshProjects
 * Description     : Create every function/type and keyboard selection from fresh KSP defaults and canonically round-trip native source.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeProjectCodecTests::nativeFunctionAllSelectionsAndFreshProjects() //20261110-0 add
{
    HmiProject project;
    HmiProject restored;
    HmiProject ksp;
    HmiObject object;
    QStringList choices;
    QString directory;
    QString error;
    QByteArray output;
    QByteArray kept;
    QJsonObject originalJson;
    QFile file;
    int function;
    int type;
    int key;
    int index;

    project.name = QStringLiteral("FunctionSourceMatrix");
    project.languages.append(QStringLiteral("English"));
    for (function = 0; function < 18; ++function) {
        choices = FunctionButtonProperties::types(function, project.languages);
        for (type = 0; type < qMax(1, static_cast<int>(choices.size())); ++type) {
            object = newFunctionSourceObject(project.objects.size());
            object.functionButton.function = function;
            object.functionButton.type = type;
            project.objects.append(object);
        }
    }
    for (key = 1; key < 10; ++key) {
        object = newFunctionSourceObject(project.objects.size());
        object.functionButton.function = 14;
        object.functionButton.keyboardKey = key;
        if (key == 9) {
            object.functionButton.keyboardText = QStringLiteral("中");
        }
        project.objects.append(object);
    }
    originalJson = project.toJson();
    QVERIFY2(HmiProject::fromJson(originalJson, &ksp, &error), qPrintable(error));
    QVERIFY(ksp.nativeSourceSnapshot.isEmpty());
    QVERIFY2(exportNativeProjectSource(ksp, &output, &error), qPrintable(error));
    QCOMPARE(ksp.toJson(), originalJson);
    QVERIFY2(nativeProjectUsesSupportedDeviceDefaults(output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.objects.size(), project.objects.size());
    QCOMPARE(restored.languages, project.languages);
    for (index = 0; index < project.objects.size(); ++index) {
        QVERIFY(restored.objects.at(index).hasFunctionButtonProperties);
        QCOMPARE(restored.objects.at(index).functionButton.function, project.objects.at(index).functionButton.function);
        QCOMPARE(restored.objects.at(index).functionButton.type, project.objects.at(index).functionButton.type);
        if (project.objects.at(index).functionButton.function == 14) {
            QCOMPARE(restored.objects.at(index).functionButton.keyboardKey, project.objects.at(index).functionButton.keyboardKey);
            if (project.objects.at(index).functionButton.keyboardKey == 9) {
                QCOMPARE(restored.objects.at(index).functionButton.keyboardText, project.objects.at(index).functionButton.keyboardText);
            }
        }
    }
    kept = output;
    QVERIFY2(exportNativeProjectSource(restored, &output, &error), qPrintable(error));
    QCOMPARE(output, kept);
    directory = qEnvironmentVariable("KINSEAL_FUNCTION_SOURCE_EXPORT_TEST_DIR");
    if (!directory.isEmpty()) {
        file.setFileName(QDir(directory).filePath(QStringLiteral("product-new-function-matrix.sm")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(output), output.size());
    }
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeFunctionAppearancePermissionsAndGuards
 * Description     : Exercise no-image/shared text, pressed effect, access gates, notifications and unsupported source edits transactionally.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeProjectCodecTests::nativeFunctionAppearancePermissionsAndGuards() //20261110-0 add
{
    HmiProject project;
    HmiProject restored;
    HmiProject bad;
    HmiObject object;
    QByteArray output;
    QByteArray kept;
    QJsonObject original;
    QString error;
    QString directory;
    QFile file;

    object = newFunctionSourceObject(0);
    object.functionButton.function = 12;
    object.functionButton.type = 13;
    object.functionButton.pressedEffect = true;
    object.bitButton.states[0].useImage = false;
    object.bitButton.states[0].sameLanguage = true;
    object.bitButton.states[0].sharedText = QStringLiteral("共享按钮");
    object.bitButton.states[0].background = QColor(QStringLiteral("#226688"));
    object.bitButton.states[0].transparent = true;
    object.bitButton.states[0].effect3d = true;
    object.bitButton.states[0].flash = true;
    object.bitButton.states[0].opacity = 177;
    object.bitButton.states[1].sameLanguage = true;
    object.bitButton.states[1].effect3d = true;
    object.bitButton.states[1].transparent = true;
    object.bitButton.touch.visibilityEnabled = true;
    object.bitButton.touch.levelEnabled = true;
    object.bitButton.touch.minimumLevel = 3;
    object.bitButton.visibility.visibilityEnabled = true;
    object.bitButton.visibility.bitEnabled = true;
    object.bitButton.visibility.bitAddress = QStringLiteral("LB7");
    object.bitButton.notice = true;
    object.bitButton.noticeBit = true;
    object.bitButton.noticeBitAddress = QStringLiteral("LB8");
    object.bitButton.pressTimeEnabled = true;
    object.bitButton.pressTime = 4;
    object.text = object.bitButton.states[0].sharedText;
    project.languages.append(QStringLiteral("English"));
    object.bitButton.states[0].text.languageTexts.insert(QStringLiteral("English"), QStringLiteral("dormant draft"));
    project.objects.append(object);
    original = project.toJson();
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QCOMPARE(project.toJson(), original);
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QVERIFY(restored.objects[0].functionButton.pressedEffect);
    QVERIFY(!restored.objects[0].bitButton.states[0].useImage);
    QVERIFY(restored.objects[0].bitButton.states[0].sameLanguage);
    QCOMPARE(restored.objects[0].bitButton.states[0].text.languageTexts.value(QStringLiteral("English")), QStringLiteral("共享按钮"));
    QCOMPARE(restored.objects[0].bitButton.states[0].background, QColor(QStringLiteral("#226688")));
    QVERIFY(restored.objects[0].bitButton.touch.levelEnabled);
    QCOMPARE(restored.objects[0].bitButton.touch.minimumLevel, 3);
    QCOMPARE(restored.objects[0].bitButton.visibility.bitAddress, QStringLiteral("LB7"));
    QVERIFY(restored.objects[0].bitButton.noticeBit);
    QCOMPARE(restored.objects[0].bitButton.noticeBitAddress, QStringLiteral("LB8"));
    QCOMPARE(restored.objects[0].bitButton.pressTime, 4);
    kept = output;
    QVERIFY2(exportNativeProjectSource(restored, &output, &error), qPrintable(error));
    QCOMPARE(output, kept);
    project = restored;
    project.objects[0].bitButton.states[0].useImage = true;
    project.objects[0].bitButton.states[0].picture = QStringLiteral("switch_ON.kvg");
    project.objects[0].bitButton.noticeBit = false;
    project.objects[0].bitButton.noticeWord = true;
    project.objects[0].bitButton.noticeWordAddress = QStringLiteral("LW21");
    project.objects[0].bitButton.noticeWordType = 1;
    project.objects[0].bitButton.noticeWordValue = QStringLiteral("123456");
    project.objects[0].geometry = QRect(60, 80, 135, 75);
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QVERIFY(restored.objects[0].bitButton.states[0].useImage);
    QCOMPARE(restored.objects[0].geometry, project.objects[0].geometry);
    QVERIFY(restored.objects[0].bitButton.noticeWord);
    QCOMPARE(restored.objects[0].bitButton.noticeWordValue, QStringLiteral("123456"));
    kept = output;
    QVERIFY2(exportNativeProjectSource(restored, &output, &error), qPrintable(error));
    QCOMPARE(output, kept);
    bad = restored;
    bad.objects[0].bitButton.macroEnabled = true;
    bad.objects[0].bitButton.macroName = QStringLiteral("ForbiddenFunctionMacro");
    bad.macros.insert(QStringLiteral("ForbiddenFunctionMacro"), QStringLiteral("void MACRO_MAIN(){}"));
    output = QByteArrayLiteral("unchanged");
    QVERIFY(!exportNativeProjectSource(bad, &output, &error));
    QCOMPARE(output, QByteArrayLiteral("unchanged"));
    bad = restored;
    bad.objects[0].bitButton.indicator = true;
    QVERIFY(!exportNativeProjectSource(bad, &output, &error));
    bad = restored;
    bad.objects[0].address = QStringLiteral("LB5");
    QVERIFY(!exportNativeProjectSource(bad, &output, &error));
    bad = restored;
    bad.objects[0].bitButton.pressTime = 256;
    QVERIFY(!exportNativeProjectSource(bad, &output, &error));
    bad = restored;
    bad.objects[0].bitButton.states[0].text.languageTexts.insert(QStringLiteral("unregistered"), QStringLiteral("extra"));
    QVERIFY(!exportNativeProjectSource(bad, &output, &error));
    project = restored;
    project.objects[0].bitButton.pressTime = 0;
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.objects[0].bitButton.pressTime, 0);
    directory = qEnvironmentVariable("KINSEAL_FUNCTION_SOURCE_EXPORT_TEST_DIR");
    if (!directory.isEmpty()) {
        file.setFileName(QDir(directory).filePath(QStringLiteral("product-function-appearance-permissions.sm")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(kept), kept.size());
    }
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeFunctionActualSamplesRoundTrip_data
 * Description     : Discover frozen final editor projects so integration tests exercise factory-created objects as well as native seeds.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeProjectCodecTests::nativeFunctionActualSamplesRoundTrip_data() //20261110-0 add
{
    QString directory;
    QStringList names;
    int index;

    QTest::addColumn<QString>("samplePath");
    directory = qEnvironmentVariable("KINSEAL_FUNCTION_SAMPLE_DIR");
    if (directory.isEmpty()) {
        directory = QFINDTESTDATA("../samples/function-button");
    }
    names = QDir(directory).entryList(QStringList{QStringLiteral("*.ksp")}, QDir::Files, QDir::Name);
    for (index = 0; index < names.size(); ++index) {
        QTest::newRow(names.at(index).toUtf8().constData()) << QDir(directory).filePath(names.at(index));
    }
    if (names.isEmpty()) {
        QTest::newRow("final-samples-not-created") << QString();
    }
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeFunctionActualSamplesRoundTrip
 * Description     : Save final KSP samples as native source and preserve complete canonical source bytes on reimport/reexport.
 * Input Parameters: Qt data row samplePath.
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeProjectCodecTests::nativeFunctionActualSamplesRoundTrip() //20261110-0 add
{
    QFETCH(QString, samplePath);
    HmiProject project;
    HmiProject restored;
    QJsonObject original;
    QByteArray output;
    QByteArray kept;
    QString directory;
    QString error;
    QFile file;
    int index;
    int functions;

    if (samplePath.isEmpty()) {
        QFAIL("Frozen GUI-created function-button samples are required; missing fixtures cannot count as passing."); //20261110-5 change
    }
    QVERIFY2(HmiProject::load(samplePath, &project, &error), qPrintable(error));
    original = project.toJson();
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(samplePath + QStringLiteral(": ") + error));
    QCOMPARE(project.toJson(), original);
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.objects.size(), project.objects.size());
    functions = 0;
    for (index = 0; index < project.objects.size(); ++index) {
        QCOMPARE(restored.objects.at(index).hasFunctionButtonProperties, project.objects.at(index).hasFunctionButtonProperties);
        if (!project.objects.at(index).hasFunctionButtonProperties) {
            continue;
        }
        ++functions;
        QCOMPARE(restored.objects.at(index).functionButton.function, project.objects.at(index).functionButton.function);
        QCOMPARE(restored.objects.at(index).functionButton.type, project.objects.at(index).functionButton.type);
        QCOMPARE(restored.objects.at(index).functionButton.pressedEffect, project.objects.at(index).functionButton.pressedEffect);
        if (project.objects.at(index).functionButton.function == 14) {
            QCOMPARE(restored.objects.at(index).functionButton.keyboardKey, project.objects.at(index).functionButton.keyboardKey);
        }
    }
    QVERIFY(functions > 0);
    kept = output;
    QVERIFY2(exportNativeProjectSource(restored, &output, &error), qPrintable(error));
    QCOMPARE(output, kept);
    directory = qEnvironmentVariable("KINSEAL_FUNCTION_SOURCE_EXPORT_TEST_DIR");
    if (!directory.isEmpty()) {
        file.setFileName(QDir(directory).filePath(QFileInfo(samplePath).completeBaseName() + QStringLiteral(".sm")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(output), output.size());
    }
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::typedFieldBoundariesAndMalformedCollections
 * Description     : Check every typed span and reject malformed lengths, counts and unknown graphical tags.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeProjectCodecTests::typedFieldBoundariesAndMalformedCollections() //20261110-5 add
{
    QStringList seeds = {QStringLiteral(":/native/bit-button-default.sm"), QStringLiteral(":/native/word-button-default.sm"),
                         QStringLiteral(":/native/lamp-default.sm"), QStringLiteral(":/native/function-button-default.sm"),
                         QStringLiteral(":/samples/native_text.sm")};
    QString seed;
    QString error;
    QByteArray source;
    QByteArray bad;
    QByteArray output;
    QList<NativeSourceFieldSpan> spans;
    QList<NativeSourceFieldSpan> preserved;
    NativeSourceFieldSpan span;
    NativeSourceFieldSpan marker;
    QSet<QString> paths;
    HmiProject retained;
    QJsonObject modelBefore;
    QFile file;
    qsizetype position;
    int malformed;

    retained.name = QStringLiteral("KeepTypedFailure");
    modelBefore = retained.toJson();
    marker.path = QStringLiteral("retained");
    malformed = 0;
    Q_FOREACH (seed, seeds) {
        file.setFileName(seed);
        QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(seed));
        source = file.readAll();
        file.close();
        QVERIFY2(inspectNativeProjectSourceFields(source, &spans, &error), qPrintable(error));
        position = 0;
        paths.clear();
        Q_FOREACH (span, spans) {
            QCOMPARE(span.start, position);
            QVERIFY(span.end > span.start && span.end <= source.size());
            QVERIFY(!paths.contains(span.path));
            paths.insert(span.path);
            position = span.end;
            if (span.kind != 'q' && span.kind != 'x' && !span.path.endsWith(QStringLiteral(".count")) &&
                !span.path.endsWith(QStringLiteral(".type")) && !span.path.endsWith(QStringLiteral(".mode"))) {
                continue;
            }
            bad = source;
            bad.replace(span.start, 4, sourceWordBytes(span.kind == 'q' ? 0x7ffffffeU : 0x7fffffffU));
            preserved = {marker};
            QVERIFY2(!inspectNativeProjectSourceFields(bad, &preserved, &error), qPrintable(span.path));
            QCOMPARE(preserved.size(), 1);
            QCOMPARE(preserved.first().path, marker.path);
            QVERIFY(!error.isEmpty());
            QVERIFY(!importNativeProjectSource(bad, &retained, &error));
            QCOMPARE(retained.toJson(), modelBefore);
            if (span.kind == 'q') { //20261110-5 add: A small odd length must fail independently of allocation-limit checks.
                bad = source;
                bad.replace(span.start, 4, sourceWordBytes(3));
                QVERIFY(!importNativeProjectSource(bad, &retained, &error));
                QCOMPARE(retained.toJson(), modelBefore);
            } else if (span.path.endsWith(QStringLiteral(".count"))) {
                bad = source;
                bad.replace(span.start, 4, sourceWordBytes(0xffffffffU));
                QVERIFY(!importNativeProjectSource(bad, &retained, &error));
                QCOMPARE(retained.toJson(), modelBefore);
            }
            ++malformed;
        }
        QCOMPARE(position, source.size());
        bad = source + QByteArray("tail");
        QVERIFY(!validateNativeProjectSourceStructure(bad, &error));
        QVERIFY(error.contains(QStringLiteral("file-end")));
        QVERIFY(!importNativeProjectSource(bad, &retained, &error));
        QCOMPARE(retained.toJson(), modelBefore);
        retained.nativeSourceSnapshot = bad;
        output = QByteArray("retained-output");
        QVERIFY(!exportNativeProjectSource(retained, &output, &error));
        QCOMPARE(output, QByteArray("retained-output"));
        retained.nativeSourceSnapshot.clear();
    }
    QVERIFY(malformed > 100);
    QVERIFY(!inspectNativeProjectSourceFields(source, nullptr, &error));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::everyTruncatedPrefixPreservesSemanticOutputs
 * Description     : Reject every incomplete prefix of the original seed without replacing caller model or span output.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeProjectCodecTests::everyTruncatedPrefixPreservesSemanticOutputs() //20261110-5 add
{
    QByteArray source = nativeOracle();
    HmiProject retained;
    QJsonObject before;
    QList<NativeSourceFieldSpan> spans;
    NativeSourceFieldSpan marker;
    QString error;
    qsizetype size;

    QVERIFY(!source.isEmpty());
    retained.name = QStringLiteral("KeepEveryPrefix");
    before = retained.toJson();
    marker.path = QStringLiteral("retained-span");
    for (size = 0; size < source.size(); ++size) {
        spans = {marker};
        QVERIFY(!inspectNativeProjectSourceFields(source.left(size), &spans, &error));
        QCOMPARE(spans.size(), 1);
        QCOMPARE(spans.first().path, marker.path);
        QVERIFY(!error.isEmpty());
        QVERIFY(!importNativeProjectSource(source.left(size), &retained, &error));
        QCOMPARE(retained.toJson(), before);
    }
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::multiplePagesAreReadOnlyArchives
 * Description     : Recognize complete newly captured native page lists while refusing semantic flattening.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeProjectCodecTests::multiplePagesAreReadOnlyArchives() //20261110-5 add
{
    QString directory = QFINDTESTDATA("fixtures/native-source/multipage");
    QStringList names = {QStringLiteral("B-two-empty-pages.sm"), QStringLiteral("C-third-page-number9.sm"), QStringLiteral("D-third-page-name.sm")};
    QString name;
    QString error;
    QFile file;
    u16 initial; //20261110-5 add
    QByteArray source;
    QByteArray output;
    QList<NativeSourceFieldSpan> fields;
    NativeProjectEnvelope archive;
    NativeSourceFieldSpan page;
    HmiProject retained;
    QJsonObject before;

    QVERIFY(!directory.isEmpty());
    retained.name = QStringLiteral("KeepMultiPageFailure");
    before = retained.toJson();
    Q_FOREACH (name, names) {
        file.setFileName(QDir(directory).filePath(name));
        QVERIFY(file.open(QIODevice::ReadOnly));
        source = file.readAll();
        file.close();
        QVERIFY2(inspectNativeProjectSourceFields(source, &fields, &error), qPrintable(error));
        QCOMPARE(fields.last().end, source.size());
        page = sourceFieldSpan(source, QStringLiteral("page[1].head.18"));
        QVERIFY(page.end > page.start);
        QVERIFY(validateNativeProjectSourceStructure(source, &error));
        QVERIFY(!importNativeProjectSource(source, &retained, &error));
        QVERIFY(error.contains(QStringLiteral("多个页面")));
        QCOMPARE(retained.toJson(), before);
        QVERIFY(!nativeProjectUsesSupportedDeviceDefaults(source, &error)); //20261110-5 add: Configuration fingerprints cannot bypass page topology.
        QVERIFY(inspectNativeProjectEnvelope(source, &archive, &error));
        QVERIFY(restoreNativeProjectArchive(archive, &output, &error));
        QCOMPARE(output, source);
        retained.nativeSourceSnapshot = source;
        initial = 99;
        QVERIFY(!nativeProjectInitialLanguageIndex(retained, &initial, &error));
        QCOMPARE(initial, 99);
        output = QByteArray("keep-output");
        QVERIFY(!exportNativeProjectSource(retained, &output, &error));
        QCOMPARE(output, QByteArray("keep-output"));
        retained.nativeSourceSnapshot.clear();
    }
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::sourceRejectsUnmappedModelEntities
 * Description     : Keep multi-page, configuration and unmapped page extensions outside native semantic export.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeProjectCodecTests::sourceRejectsUnmappedModelEntities() //20261110-5 add
{
    HmiProject original;
    HmiProject project;
    HmiPage page;
    HmiConfigurationNode configuration;
    QByteArray output("keep-export");
    QJsonObject before;
    QString error;

    QVERIFY2(importNativeProjectSource(nativeOracle(), &original, &error), qPrintable(error));
    project = original;
    page.id = QStringLiteral("second-page");
    page.name = QStringLiteral("Second page");
    QVERIFY2(project.addPage(page, &error), qPrintable(error));
    before = project.toJson();
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, QByteArray("keep-export"));
    QCOMPARE(project.toJson(), before);
    project = original;
    configuration.id = QStringLiteral("alarm-node");
    configuration.kind = QStringLiteral("alarm");
    configuration.name = QStringLiteral("Unimplemented alarm");
    project.configurationNodes.append(configuration);
    QVERIFY2(project.validateModel(&error), qPrintable(error));
    before = project.toJson();
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, QByteArray("keep-export"));
    QCOMPARE(project.toJson(), before);
    project = original;
    project.activePageExtensions.insert(QStringLiteral("test.metadata"), QJsonObject{{QStringLiteral("version"), 1}, {QStringLiteral("inactive"), true},
        {QStringLiteral("data"), QJsonObject{{QStringLiteral("note"), QStringLiteral("must not disappear")}}}});
    QVERIFY2(project.validateModel(&error), qPrintable(error));
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QVERIFY(error.contains(QStringLiteral("扩展元数据")));
    QCOMPARE(output, QByteArray("keep-export"));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::explicitStartupLanguagePreservesDialogSelection
 * Description     : Change the compiled system choice while preserving the independent native dialog choice.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeProjectCodecTests::explicitStartupLanguagePreservesDialogSelection() //20261110-6 change
{
    HmiProject project;
    HmiProject restored;
    HmiProject retained;
    QByteArray output;
    QByteArray kept;
    QByteArray bad;
    NativeSourceFieldSpan mirror;
    QString error;
    QJsonObject before;
    u16 index;

    QVERIFY2(importNativeProjectSource(nativeOracle(), &project, &error), qPrintable(error));
    project.languages.append(QStringLiteral("English"));
    project.startupLanguage = QStringLiteral("English");
    project.activeLanguage = QStringLiteral("中文");
    index = 99;
    QVERIFY2(nativeProjectInitialLanguageIndex(project, &index, &error), qPrintable(error));
    QCOMPARE(index, 1);
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(nativeProjectUsesSupportedDeviceDefaults(output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.activeLanguage, QStringLiteral("English"));
    QVERIFY(restored.startupLanguage.isEmpty());
    restored.activeLanguage = QStringLiteral("中文");
    kept = output;
    QVERIFY2(exportNativeProjectSource(restored, &output, &error), qPrintable(error));
    QCOMPARE(output, kept);
    restored.startupLanguage = QStringLiteral("中文");
    QVERIFY2(exportNativeProjectSource(restored, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.activeLanguage, QStringLiteral("中文"));
    mirror = sourceFieldSpan(kept, QStringLiteral("config.290.0"));
    QVERIFY(mirror.end > mirror.start);
    bad = kept;
    QCOMPARE(kept.mid(mirror.start, 4), sourceWordBytes(0)); //20261110-6 add: Explicit English startup must not change the dialog's Chinese setting.
    bad.replace(mirror.start, 4, sourceWordBytes(2)); //20261110-6 change: Only an out-of-range independent index is invalid.
    QVERIFY(validateNativeProjectSourceStructure(bad, &error));
    retained.name = QStringLiteral("KeepStartupConflict");
    before = retained.toJson();
    QVERIFY(!importNativeProjectSource(bad, &retained, &error));
    QCOMPARE(retained.toJson(), before);
    project.nativeSourceSnapshot = bad;
    index = 99;
    QVERIFY(!nativeProjectInitialLanguageIndex(project, &index, &error));
    QCOMPARE(index, 99);
    output = QByteArray("keep-output");
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, QByteArray("keep-output"));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeIndependentLanguageSelections_data
 * Description     : Register native GUI samples whose editor and language-dialog choices differ.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeProjectCodecTests::nativeIndependentLanguageSelections_data() //20261110-6 add
{
    QTest::addColumn<QString>("fixture");
    QTest::addColumn<int>("editorIndex");
    QTest::addColumn<int>("dialogIndex");
    QTest::newRow("settings-English-editor-Chinese") << QStringLiteral("language/settings1-editor0-native.sm") << 0 << 1;
    QTest::newRow("settings-Chinese-editor-English") << QStringLiteral("language/settings0-editor1-native.sm") << 1 << 0;
    QTest::newRow("three-languages") << QStringLiteral("language/settings2-editor1-three-native.sm") << 1 << 2;
    QTest::newRow("previous-native-counterexample") << QStringLiteral("rejected/startup-editor-divergence-native.sm") << 0 << 1;
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeIndependentLanguageSelections
 * Description     : Preserve both choices exactly, remap them separately, and reject malformed indices atomically.
 * Input Parameters: Qt fixture data.
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeProjectCodecTests::nativeIndependentLanguageSelections() //20261110-6 add
{
    QFETCH(QString, fixture);
    QFETCH(int, editorIndex);
    QFETCH(int, dialogIndex);
    QFile file;
    HmiProject project;
    HmiProject restored;
    HmiProject retained;
    QString error;
    QString directory;
    QString editorName;
    QString dialogName;
    QStringList reordered;
    QByteArray input;
    QByteArray output;
    QByteArray bad;
    NativeSourceFieldSpan field;
    QJsonObject before;
    u16 index;

    directory = QFINDTESTDATA("fixtures/native-source");
    QVERIFY(!directory.isEmpty());
    file.setFileName(QDir(directory).filePath(fixture));
    QVERIFY(file.open(QIODevice::ReadOnly));
    input = file.readAll();
    QVERIFY2(importNativeProjectSource(input, &project, &error), qPrintable(error));
    QCOMPARE(project.activeLanguage, project.languages.at(editorIndex));
    QVERIFY2(nativeProjectUsesSupportedDeviceDefaults(input, &error), qPrintable(error));
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QCOMPARE(output, input);
    QVERIFY2(HmiProject::fromJson(project.toJson(), &restored, &error), qPrintable(error));
    QVERIFY2(exportNativeProjectSource(restored, &output, &error), qPrintable(error));
    QCOMPARE(output, input);
    editorName = project.languages.at(editorIndex);
    dialogName = project.languages.at(dialogIndex);
    project.activeLanguage = project.languages.at((editorIndex + 1) % project.languages.size());
    index = 99;
    QVERIFY2(nativeProjectInitialLanguageIndex(project, &index, &error), qPrintable(error));
    QCOMPARE(index, static_cast<u16>(editorIndex));
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QCOMPARE(output, input);
    reordered = project.languages;
    std::reverse(reordered.begin(), reordered.end());
    QVERIFY2(project.setLanguages(reordered, &error), qPrintable(error));
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.activeLanguage, editorName);
    QVERIFY2(nativeProjectUsesSupportedDeviceDefaults(output, &error), qPrintable(error));
    field = sourceFieldSpan(output, QStringLiteral("config.290.0"));
    QCOMPARE(output.mid(field.start, field.end - field.start), sourceWordBytes(static_cast<u32>(reordered.indexOf(dialogName))));
    before = restored.toJson();
    retained = restored;
    bad = output;
    bad.replace(field.start, 4, sourceWordBytes(static_cast<u32>(reordered.size())));
    QVERIFY(validateNativeProjectSourceStructure(bad, &error));
    QVERIFY(!importNativeProjectSource(bad, &retained, &error));
    QCOMPARE(retained.toJson(), before);
    QVERIFY(!nativeProjectUsesSupportedDeviceDefaults(bad, &error));
    retained.nativeSourceSnapshot = bad;
    index = 99;
    QVERIFY(!nativeProjectInitialLanguageIndex(retained, &index, &error));
    QCOMPARE(index, u16(99));
    output = QByteArray("keep-output");
    QVERIFY(!exportNativeProjectSource(retained, &output, &error));
    QCOMPARE(output, QByteArray("keep-output"));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::unknownLanguageDialogSlotsArePreserved
 * Description     : Keep unknown dialog labels unchanged and refuse registry edits that would reinterpret them.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeProjectCodecTests::unknownLanguageDialogSlotsArePreserved() //20261110-6 add
{
    QFile file(QFINDTESTDATA("fixtures/native-source/language/settings1-editor0-native.sm"));
    HmiProject project;
    HmiProject restored;
    QString error;
    QByteArray input;
    QByteArray output;
    QByteArray label;
    QDataStream stream(&label, QIODevice::WriteOnly);
    NativeSourceFieldSpan field;

    QVERIFY(file.open(QIODevice::ReadOnly));
    input = file.readAll();
    field = sourceFieldSpan(input, QStringLiteral("config.298[0]"));
    QVERIFY(field.end > field.start);
    stream.setVersion(QDataStream::Qt_4_8);
    stream << QStringLiteral("Unknown setting label");
    input.replace(field.start, field.end - field.start, label); // Explicit synthetic negative input, not a native UI sample.
    QVERIFY2(importNativeProjectSource(input, &project, &error), qPrintable(error));
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QCOMPARE(output, input);
    QVERIFY(!nativeProjectUsesSupportedDeviceDefaults(input, &error));
    QVERIFY2(project.setLanguages({QStringLiteral("English"), QStringLiteral("中文")}, &error), qPrintable(error));
    output = QByteArray("keep-unknown-dialog");
    QVERIFY(!exportNativeProjectSource(project, &output, &error));
    QCOMPARE(output, QByteArray("keep-unknown-dialog"));
    QVERIFY(error.contains(QStringLiteral("设置槽")));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::typedMetadataBudgetIsBounded
 * Description     : Bound parser metadata even when each malicious repeated collection fits its local count limit.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeProjectCodecTests::typedMetadataBudgetIsBounded() //20261110-5 add
{
    QFile file(QFINDTESTDATA("fixtures/native-source/function/function-valid-r4.sm"));
    QByteArray source;
    QByteArray oversized;
    QByteArray pageBytes;
    NativeSourceFieldSpan count;
    NativeSourceFieldSpan page;
    NativeSourceFieldSpan history;
    QString error;

    QVERIFY(file.open(QIODevice::ReadOnly));
    source = file.readAll();
    count = sourceFieldSpan(source, QStringLiteral("project.pages.count"));
    page = sourceFieldSpan(source, QStringLiteral("page.head.0"));
    history = sourceFieldSpan(source, QStringLiteral("history.1e4.count"));
    QVERIFY(count.end == page.start && history.start > page.start);
    pageBytes = source.mid(page.start, history.start - page.start);
    oversized = source.left(count.start) + sourceWordBytes(20) + pageBytes.repeated(20) + source.mid(history.start);
    QVERIFY(oversized.size() < 64 * 1024 * 1024);
    QVERIFY(!validateNativeProjectSourceStructure(oversized, &error));
    QVERIFY2(error.contains(QStringLiteral("字段总数")), qPrintable(error));
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::largeNativeDiagnosticsKeepCompleteSnapshot
 * Description     : Bound generated warnings for the independently specified real 67-control source while retaining its exact opaque stream.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeProjectCodecTests::largeNativeDiagnosticsKeepCompleteSnapshot() //20261110-5 add
{
    QFile file(QFINDTESTDATA("fixtures/native-source/function/function-valid-r4.sm"));
    QByteArray source;
    QByteArray output;
    HmiProject project;
    HmiProject restored;
    QJsonArray specifications;
    QJsonObject specification;
    QJsonParseError parseError;
    QString error;
    int index;
    int sourceIndex;

    QVERIFY(file.open(QIODevice::ReadOnly));
    source = file.readAll();
    file.close();
    file.setFileName(QFINDTESTDATA("fixtures/native-source/function/function-valid-r4-specs.json"));
    QVERIFY(file.open(QIODevice::ReadOnly));
    specifications = QJsonDocument::fromJson(file.readAll(), &parseError).array();
    QCOMPARE(parseError.error, QJsonParseError::NoError);
    QCOMPARE(specifications.size(), 67);
    QVERIFY2(importNativeProjectSource(source, &project, &error), qPrintable(error));
    QCOMPARE(project.objects.size(), specifications.size());
    for (index = 0; index < specifications.size(); ++index) {
        specification = specifications.at(index).toObject();
        sourceIndex = specifications.size() - 1 - specification.value(QStringLiteral("index")).toInt(-1);
        QVERIFY(sourceIndex >= 0 && sourceIndex < project.objects.size());
        QVERIFY(project.objects.at(sourceIndex).hasFunctionButtonProperties);
        QCOMPARE(project.objects.at(sourceIndex).functionButton.function, specification.value(QStringLiteral("function")).toInt(-1));
    }
    QCOMPARE(project.nativeSourceWarnings.size(), 128);
    QVERIFY(project.nativeSourceWarnings.last().contains(QStringLiteral("未逐条展示")));
    QCOMPARE(project.nativeSourceSnapshot, source);
    QVERIFY2(HmiProject::fromJson(project.toJson(), &restored, &error), qPrintable(error));
    QCOMPARE(restored.nativeSourceWarnings, project.nativeSourceWarnings);
    QCOMPARE(restored.nativeSourceSnapshot, source);
    QVERIFY2(exportNativeProjectSource(restored, &output, &error), qPrintable(error));
    QCOMPARE(output, source);
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::nativeUnrepresentableActiveMetadataIsRejected
 * Description     : Archive real native writer counterexamples without treating unsupported active metadata as editable.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeProjectCodecTests::nativeUnrepresentableActiveMetadataIsRejected() //20261110-5 add
{
    QString directory = QFINDTESTDATA("fixtures/native-source/rejected");
    QStringList names = {QStringLiteral("active-macro-name-cleared-native.sm")}; //20261110-6 change: The real language-divergence sample is now covered by positive lossless tests.
    QString name;
    QString error;
    QByteArray bytes;
    QByteArray output;
    HmiProject retained;
    QJsonObject before;
    NativeProjectEnvelope archive;
    QFile file;

    QVERIFY(!directory.isEmpty());
    retained.name = QStringLiteral("KeepUnrepresentableSource");
    before = retained.toJson();
    Q_FOREACH (name, names) {
        file.setFileName(QDir(directory).filePath(name));
        QVERIFY(file.open(QIODevice::ReadOnly));
        bytes = file.readAll();
        file.close();
        QVERIFY2(validateNativeProjectSourceStructure(bytes, &error), qPrintable(error));
        QVERIFY(!importNativeProjectSource(bytes, &retained, &error));
        QVERIFY(!error.isEmpty());
        QCOMPARE(retained.toJson(), before);
        QVERIFY(inspectNativeProjectEnvelope(bytes, &archive, &error));
        QVERIFY(restoreNativeProjectArchive(archive, &output, &error));
        QCOMPARE(output, bytes);
        retained.nativeSourceSnapshot = bytes;
        output = QByteArray("keep-metadata-output");
        QVERIFY(!exportNativeProjectSource(retained, &output, &error));
        QCOMPARE(output, QByteArray("keep-metadata-output"));
        retained.nativeSourceSnapshot.clear();
    }
}

/******************************************************************************
 * Function Name   : NativeProjectCodecTests::productMacroCachePreservationAndInvalidation
 * Description     : Preserve a synthetically injected MD5 cache and invalidate it after changing actual macro source.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void NativeProjectCodecTests::productMacroCachePreservationAndInvalidation() //20261110-5 add
{
    QFile file(QFINDTESTDATA("fixtures/native-source/bit-analysis/macro-bound-product-seed.sm"));
    QByteArray input;
    QByteArray cached;
    QByteArray cache;
    QByteArray output;
    NativeSourceFieldSpan code;
    NativeSourceFieldSpan compiled;
    HmiProject project;
    HmiProject restored;
    QString error;

    QVERIFY(file.open(QIODevice::ReadOnly));
    input = file.readAll();
    QVERIFY2(importNativeProjectSource(input, &project, &error), qPrintable(error));
    code = sourceFieldSpan(input, QStringLiteral("history.1e4[0].code"));
    compiled = sourceFieldSpan(input, QStringLiteral("history.1e4[0].head.0"));
    QVERIFY(code.end > code.start && compiled.end > compiled.start);
    cache = QCryptographicHash::hash(project.macros.value(QStringLiteral("SetLB1")).toUtf8(), QCryptographicHash::Md5);
    cached = input;
    cached[compiled.start] = 1;
    cached.replace(code.start, code.end - code.start, sourceWordBytes(16) + cache); // This is a format/transaction fixture, not evidence that the native compiler ran.
    QVERIFY2(importNativeProjectSource(cached, &project, &error), qPrintable(error));
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QCOMPARE(output, cached);
    project.macros[QStringLiteral("SetLB1")] += QStringLiteral("\n// changed source\n");
    QVERIFY2(exportNativeProjectSource(project, &output, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(output, &restored, &error), qPrintable(error));
    QCOMPARE(restored.macros, project.macros);
    code = sourceFieldSpan(output, QStringLiteral("history.1e4[0].code"));
    compiled = sourceFieldSpan(output, QStringLiteral("history.1e4[0].head.0"));
    QCOMPARE(code.end - code.start, 4);
    QCOMPARE(static_cast<u8>(output.at(compiled.start)), static_cast<u8>(0));
    QVERIFY(restored.nativeSourceWarnings.join(QString()).contains(QStringLiteral("未编译源码")));
}

QTEST_GUILESS_MAIN(NativeProjectCodecTests)
#include "native_project_codec_tests.moc"
