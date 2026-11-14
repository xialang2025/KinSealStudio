#include "device_macro_compiler.h"
#include "project_model.h"

#include <QCryptographicHash>
#include <QTemporaryDir>
#include <QDir> //20261114-7 add
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QElapsedTimer>
#include <QtTest>

//20261109-0 add: Real ARM compiler cases run only with an explicitly configured SDK.
class DeviceMacroTests : public QObject
{
    Q_OBJECT
private slots:
    void emptyProjectNeedsNoSdk();
    void invalidSdkIsTransactional();
    void projectLimitsFailBeforeCompilation();
    void realCompilerBuildsNativePackage();
    void realCompilerRejectsSyntaxError();
    void realCompilerRequiresEntry();
    void realCompilerRejectsOverlappingRw();
    void realCompilerKeepsUnicodeNamesOutOfPaths();
    void optionsAndIndependentReport(); //20261114-7 add
    void syntheticProcessFailures_data();
    void syntheticProcessFailures();
    void realManifestRepeatabilityAndConstantChange();
};

/******************************************************************************
 * Function Name   : setBitSource
 * Description     : Return the independent 24-byte native code oracle source.
 * Input Parameters: None
 * Return Value    : ANSI C source that sets local bit one.
 * Date            : 2026.11.09
 ******************************************************************************/
static QString setBitSource() //20261109-0 add
{
    return QStringLiteral("#include \"macro.h\"\n#include \"include\\macrolib.h\"\n"
                          "#include \"include\\glib.h\"\nvoid Macro_Entry( )\n"
                          "{\n    LocalBit[1] = 1;\n}\n");
}

/******************************************************************************
 * Function Name   : DeviceMacroTests::emptyProjectNeedsNoSdk
 * Description     : Clear a stale package for a project without any macros.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void DeviceMacroTests::emptyProjectNeedsNoSdk() //20261109-0 add
{
    HmiProject project;
    DeviceMacroPackage package;
    QString error;

    package.marcoRun = QByteArray("old");
    package.marcoVar = QByteArray("old");
    package.macroIndices.insert(QStringLiteral("old"), 3);
    error = QStringLiteral("old");
    QVERIFY2(compileDeviceMacros(project, QString(), &package, &error), qPrintable(error));
    QVERIFY(package.marcoRun.isEmpty());
    QVERIFY(package.marcoVar.isEmpty());
    QVERIFY(package.macroIndices.isEmpty());
    QVERIFY(package.diagnostics.isEmpty());
    QVERIFY(error.isEmpty());
    QVERIFY(!compileDeviceMacros(project, QString(), nullptr, &error));
}

/******************************************************************************
 * Function Name   : DeviceMacroTests::invalidSdkIsTransactional
 * Description     : Fail clearly for missing or incomplete SDKs and retain previous output.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void DeviceMacroTests::invalidSdkIsTransactional() //20261109-0 add
{
    HmiProject project;
    DeviceMacroPackage package;
    QTemporaryDir directory;
    QString error;

    project.macros.insert(QStringLiteral("SetLB1"), setBitSource());
    package.marcoRun = QByteArray("existing-run");
    package.marcoVar = QByteArray("existing-vars");
    QVERIFY(!compileDeviceMacros(project, QString(), &package, &error));
    QVERIFY(error.contains(QStringLiteral("KINSEAL_ARM_SDK")));
    QVERIFY(directory.isValid());
    QVERIFY(!compileDeviceMacros(project, directory.filePath(QStringLiteral("absent")), &package, &error));
    QVERIFY(!compileDeviceMacros(project, directory.path(), &package, &error));
    QVERIFY(error.contains(QStringLiteral("armcc.exe")));
    QCOMPARE(package.marcoRun, QByteArray("existing-run"));
    QCOMPARE(package.marcoVar, QByteArray("existing-vars"));
    QVERIFY(package.macroIndices.isEmpty());
}

/******************************************************************************
 * Function Name   : DeviceMacroTests::projectLimitsFailBeforeCompilation
 * Description     : Reject unsupported models, excess source counts and oversized sources.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void DeviceMacroTests::projectLimitsFailBeforeCompilation() //20261109-0 add
{
    HmiProject project;
    DeviceMacroPackage package;
    QString error;
    int index;

    project.macros.insert(QStringLiteral("SetLB1"), setBitSource());
    project.targetModel = QStringLiteral("unknown");
    QVERIFY(!compileDeviceMacros(project, QString(), &package, &error));
    QVERIFY(error.contains(QStringLiteral("内存配置")));
    project.targetModel = QStringLiteral("AMT070W124-L800X480R");
    project.macros.clear();
    for (index = 0; index < 101; ++index)
    {
        project.macros.insert(QString::number(index), setBitSource());
    }
    QVERIFY(!compileDeviceMacros(project, QString(), &package, &error));
    QVERIFY(error.contains(QStringLiteral("100")));
    project.macros.clear();
    project.macros.insert(QStringLiteral("large"), QString(1024 * 1024 + 1, QLatin1Char(' ')));
    QVERIFY(!compileDeviceMacros(project, QString(), &package, &error));
    QVERIFY(error.contains(QStringLiteral("1 MiB")));
    project.macros.clear();
    project.macros.insert(QStringLiteral(" "), setBitSource());
    QVERIFY(!compileDeviceMacros(project, QString(), &package, &error));
    QVERIFY(error.contains(QStringLiteral("名称为空")));
    QVERIFY(package.marcoRun.isEmpty());
}

/******************************************************************************
 * Function Name   : DeviceMacroTests::realCompilerBuildsNativePackage
 * Description     : Compare real compiled ARM bytes with the independently recovered oracle.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void DeviceMacroTests::realCompilerBuildsNativePackage() //20261109-0 add
{
    HmiProject project;
    DeviceMacroPackage package;
    QString sdk;
    QString error;
    QByteArray code;

    sdk = qEnvironmentVariable("KINSEAL_ARM_SDK");
    if (sdk.isEmpty())
    {
        QSKIP("KINSEAL_ARM_SDK is not configured; the proprietary SDK is not bundled.");
    }
    project.macros.insert(QStringLiteral("置位一"), setBitSource());
    QVERIFY2(compileDeviceMacros(project, sdk, &package, &error), qPrintable(error));
    QCOMPARE(package.marcoRun.size(), 1048);
    QCOMPARE(package.marcoRun.left(2).toHex(), QByteArray("01ff"));
    QCOMPARE(package.marcoRun.mid(2, 100), QByteArray(100, char(0)));
    QCOMPARE(package.marcoRun.mid(102, 2).toHex(), QByteArray("1800"));
    QCOMPARE(package.marcoRun.mid(104, 920), QByteArray(920, char(0)));
    QCOMPARE(package.marcoVar, QByteArray(904, char(0)));
    QCOMPARE(package.macroIndices.value(QStringLiteral("置位一")), u8(0));
    code = package.marcoRun.mid(1024);
    QCOMPARE(QCryptographicHash::hash(code, QCryptographicHash::Sha256).toHex(),
             QByteArray("d6e4f44119b03071a0dea0e8066f084054da6736f8ba5aeb74d443fff914e5da"));
    QVERIFY(!package.diagnostics.isEmpty());
}

/******************************************************************************
 * Function Name   : DeviceMacroTests::realCompilerRejectsSyntaxError
 * Description     : Expose the compiler stage and source error without emitting a new package.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void DeviceMacroTests::realCompilerRejectsSyntaxError() //20261109-0 add
{
    HmiProject project;
    DeviceMacroPackage package;
    QString sdk;
    QString error;

    sdk = qEnvironmentVariable("KINSEAL_ARM_SDK");
    if (sdk.isEmpty())
    {
        QSKIP("KINSEAL_ARM_SDK is not configured.");
    }
    project.macros.insert(QStringLiteral("错误宏"), QStringLiteral("void Macro_Entry(void) { broken = ; }"));
    package.marcoRun = QByteArray("previous-good-package");
    QVERIFY(!compileDeviceMacros(project, sdk, &package, &error));
    QVERIFY(error.contains(QStringLiteral("错误宏")));
    QVERIFY(error.contains(QStringLiteral("armcc.exe")));
    QVERIFY(error.contains(QStringLiteral("macro_000.c")));
    QCOMPARE(package.marcoRun, QByteArray("previous-good-package"));
    QVERIFY(package.marcoVar.isEmpty());
}

/******************************************************************************
 * Function Name   : DeviceMacroTests::realCompilerRequiresEntry
 * Description     : Reject an otherwise linkable function that cannot be called as a macro.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void DeviceMacroTests::realCompilerRequiresEntry() //20261109-0 add
{
    HmiProject project;
    DeviceMacroPackage package;
    QString sdk;
    QString error;

    sdk = qEnvironmentVariable("KINSEAL_ARM_SDK");
    if (sdk.isEmpty())
    {
        QSKIP("KINSEAL_ARM_SDK is not configured.");
    }
    project.macros.insert(QStringLiteral("missing-entry"), QStringLiteral("void OtherFunction(void) { }"));
    QVERIFY(!compileDeviceMacros(project, sdk, &package, &error));
    QVERIFY(error.contains(QStringLiteral("Macro_Entry")));
    QVERIFY(package.marcoRun.isEmpty());
}

/******************************************************************************
 * Function Name   : DeviceMacroTests::realCompilerRejectsOverlappingRw
 * Description     : Reject writable ELF data that would overlap the next macro's memory slot.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void DeviceMacroTests::realCompilerRejectsOverlappingRw() //20261109-0 add
{
    HmiProject project;
    DeviceMacroPackage package;
    QString sdk;
    QString error;

    sdk = qEnvironmentVariable("KINSEAL_ARM_SDK");
    if (sdk.isEmpty())
    {
        QSKIP("KINSEAL_ARM_SDK is not configured.");
    }
    project.macros.insert(QStringLiteral("too-much-rw"),
                          QStringLiteral("#include \"macro.h\"\nchar block[2048];\n"
                                         "void Macro_Entry(void) { block[0] = LocalBit[1]; }"));
    QVERIFY(!compileDeviceMacros(project, sdk, &package, &error));
    QVERIFY2(error.contains(QStringLiteral("RW/ZI")), qPrintable(error));
    QVERIFY(package.marcoRun.isEmpty());
}

/******************************************************************************
 * Function Name   : DeviceMacroTests::realCompilerKeepsUnicodeNamesOutOfPaths
 * Description     : Preserve QMap indices while special display names never become file paths.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void DeviceMacroTests::realCompilerKeepsUnicodeNamesOutOfPaths() //20261109-0 add
{
    HmiProject project;
    DeviceMacroPackage package;
    QString sdk;
    QString error;

    sdk = qEnvironmentVariable("KINSEAL_ARM_SDK");
    if (sdk.isEmpty())
    {
        QSKIP("KINSEAL_ARM_SDK is not configured.");
    }
    project.macros.insert(QStringLiteral("../中文名;\"|SetLB1"), setBitSource());
    project.macros.insert(QStringLiteral("第二条"), setBitSource());
    QVERIFY2(compileDeviceMacros(project, sdk, &package, &error), qPrintable(error));
    QCOMPARE(package.macroIndices.value(QStringLiteral("../中文名;\"|SetLB1")), u8(0));
    QCOMPARE(package.macroIndices.value(QStringLiteral("第二条")), u8(1));
    QCOMPARE(package.marcoRun.size(), 1072);
    QCOMPARE(package.marcoRun.mid(102, 4).toHex(), QByteArray("18001800"));
    QCOMPARE(package.marcoVar, QByteArray(1808, char(0)));
    QVERIFY(package.diagnostics.join(QLatin1Char('\n')).contains(QStringLiteral("RO=0x80520800")));
    QVERIFY(package.diagnostics.join(QLatin1Char('\n')).contains(QStringLiteral("RW=0x80440400")));
}


/******************************************************************************
 * Function Name   : macroFixtureSdk
 * Description     : Create only a new synthetic SDK tree for owned-process failure tests, never an ADS oracle.
 * Input Parameters: path, fixture
 * Return Value    : True for all fixture files copied or written.
 * Date            : 2026.11.14
 ******************************************************************************/
static bool macroFixtureSdk(const QString &path, const QString &fixture) //20261114-7 add
{
    QString name;
    QFile file;
    QStringList tools{QStringLiteral("armcc.exe"), QStringLiteral("armcc.dll"), QStringLiteral("armlink.exe"),
        QStringLiteral("armlink.dll"), QStringLiteral("fromelf.exe"), QStringLiteral("fromelf.dll")};
    QStringList data{QStringLiteral("Include/macrolib.h"), QStringLiteral("Include/glib.h"), QStringLiteral("Include/math.h"),
        QStringLiteral("Include/stdio.h"), QStringLiteral("Include/stdlib.h"), QStringLiteral("script_amzlib.a"),
        QStringLiteral("guilib70_amz.a"), QStringLiteral("licenses/LICENSE.DAT"), QStringLiteral("Lib/armlib/c_a__un.l")};

    if (!QDir().mkpath(path)) { return false; }
    Q_FOREACH (name, tools) { if (!QFile::copy(fixture, QDir(path).filePath(name))) { return false; } }
    Q_FOREACH (name, data)
    {
        if (!QDir().mkpath(QFileInfo(QDir(path).filePath(name)).absolutePath())) { return false; }
        file.setFileName(QDir(path).filePath(name));
        if (!file.open(QIODevice::WriteOnly | QIODevice::NewOnly) || file.write("SYNTHETIC; NOT ADS", 18) != 18) { return false; }
        file.close();
    }
    return true;
}

/******************************************************************************
 * Function Name   : macroSaveEvidence
 * Description     : Preserve optional invocation evidence only under the caller-selected task directory.
 * Input Parameters: name, report
 * Return Value    : True when no destination is configured or the bounded report was written.
 * Date            : 2026.11.14
 ******************************************************************************/
static bool macroSaveEvidence(const QString &name, const QJsonObject &report) //20261114-7 add
{
    QString path = qEnvironmentVariable("KINSEAL_STAGE8_COMPILER_EVIDENCE");
    QFile file;
    QByteArray bytes;

    if (path.isEmpty()) { return true; }
    if (!QDir().mkpath(path)) { return false; }
    file.setFileName(QDir(path).filePath(name + QStringLiteral(".json")));
    bytes = QJsonDocument(report).toJson(QJsonDocument::Indented);
    return file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size();
}

/******************************************************************************
 * Function Name   : DeviceMacroTests::optionsAndIndependentReport
 * Description     : Keep every old package field intact on invalid options, cancellation and report aliases.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceMacroTests::optionsAndIndependentReport() //20261114-7 add
{
    HmiProject project;
    DeviceMacroPackage package;
    DeviceMacroCompileOptions options;
    QJsonObject report;
    QString error;

    project.macros.insert(QStringLiteral("set"), setBitSource());
    package.marcoRun = QByteArray("existing");
    package.compileManifest.insert(QStringLiteral("sentinel"), true);
    options.processTimeoutMs = 0;
    QVERIFY(!compileDeviceMacros(project, QString(), &package, &error, options, &report));
    QCOMPARE(report.value(QStringLiteral("firstError")).toObject().value(QStringLiteral("code")).toString(), QStringLiteral("invalid-options"));
    QCOMPARE(package.marcoRun, QByteArray("existing"));
    QVERIFY(package.compileManifest.value(QStringLiteral("sentinel")).toBool());
    options = DeviceMacroCompileOptions();
    options.isCancellationRequested = []() { return true; };
    QVERIFY(!compileDeviceMacros(project, QString(), &package, &error, options, &report));
    QVERIFY(report.value(QStringLiteral("cancelled")).toBool());
    QCOMPARE(report.value(QStringLiteral("stages")).toArray().size(), 0);
    QCOMPARE(package.marcoRun, QByteArray("existing"));
    QVERIFY(!compileDeviceMacros(project, QString(), &package, &error, options, &package.compileManifest));
    QVERIFY(package.compileManifest.value(QStringLiteral("sentinel")).toBool());
}

/******************************************************************************
 * Function Name   : DeviceMacroTests::syntheticProcessFailures_data
 * Description     : Specify short deterministic injected-process failures without waiting for production deadlines.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceMacroTests::syntheticProcessFailures_data() //20261114-7 add
{
    QTest::addColumn<QString>("mode");
    QTest::addColumn<QString>("expected");
    QTest::newRow("synthetic-nonzero") << QStringLiteral("fail") << QStringLiteral("sdk-stage-failed");
    QTest::newRow("synthetic-output-flood") << QStringLiteral("flood") << QStringLiteral("diagnostic-output-limit");
    QTest::newRow("synthetic-stage-timeout") << QStringLiteral("sleep") << QStringLiteral("stage-timeout");
    QTest::newRow("synthetic-total-timeout") << QStringLiteral("total") << QStringLiteral("total-timeout");
    QTest::newRow("synthetic-cancel") << QStringLiteral("cancel") << QStringLiteral("cancelled");
    QTest::newRow("synthetic-invalid-elf") << QStringLiteral("invalid-elf") << QStringLiteral("missing-entry");
    QTest::newRow("synthetic-raw-overflow") << QStringLiteral("raw-overflow") << QStringLiteral("raw-image-read-failed");
}

/******************************************************************************
 * Function Name   : DeviceMacroTests::syntheticProcessFailures
 * Description     : Observe bounded diagnostics, owned child cleanup and old-package preservation using a clearly synthetic SDK.
 * Input Parameters: Data row.
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceMacroTests::syntheticProcessFailures() //20261114-7 add
{
    QFETCH(QString, mode);
    QFETCH(QString, expected);
    QString fixture = qEnvironmentVariable("KINSEAL_MACRO_PROCESS_FIXTURE");
    QTemporaryDir directory;
    HmiProject project;
    DeviceMacroPackage package;
    DeviceMacroCompileOptions options;
    QJsonObject report;
    QJsonObject stage;
    QElapsedTimer timer;
    QString error;
    QByteArray oldMode = qgetenv("KINSEAL_MACRO_FIXTURE_MODE");
    QByteArray oldStage = qgetenv("KINSEAL_MACRO_FIXTURE_STAGE");
    bool result;

    if (fixture.isEmpty()) { QSKIP("Explicit synthetic process fixture is not configured; not an ADS oracle."); }
    QVERIFY(directory.isValid());
    QVERIFY(macroFixtureSdk(directory.filePath(QStringLiteral("sdk")), fixture));
    project.macros.insert(QStringLiteral("synthetic"), setBitSource());
    package.marcoRun = QByteArray("old-run");
    package.marcoVar = QByteArray("old-var");
    package.compileManifest.insert(QStringLiteral("old"), true);
    options.temporaryRoot = directory.path();
    options.processTimeoutMs = mode == QStringLiteral("sleep") ? 60 : 3000;
    options.totalTimeoutMs = mode == QStringLiteral("total") ? 100 : 120000;
    options.maxProcessOutputBytes = 1024;
    timer.start();
    if (mode == QStringLiteral("cancel")) { options.isCancellationRequested = [&timer]() { return timer.elapsed() > 120; }; }
    qputenv("KINSEAL_MACRO_FIXTURE_MODE", (mode == QStringLiteral("total") || mode == QStringLiteral("cancel")) ? QByteArray("sleep") : mode.toLatin1());
    qputenv("KINSEAL_MACRO_FIXTURE_STAGE", QByteArray("armcc"));
    if (mode == QStringLiteral("invalid-elf") || mode == QStringLiteral("raw-overflow")) { qunsetenv("KINSEAL_MACRO_FIXTURE_STAGE"); }
    result = compileDeviceMacros(project, directory.filePath(QStringLiteral("sdk")), &package, &error, options, &report);
    if (oldMode.isNull()) { qunsetenv("KINSEAL_MACRO_FIXTURE_MODE"); } else { qputenv("KINSEAL_MACRO_FIXTURE_MODE", oldMode); }
    if (oldStage.isNull()) { qunsetenv("KINSEAL_MACRO_FIXTURE_STAGE"); } else { qputenv("KINSEAL_MACRO_FIXTURE_STAGE", oldStage); }
    QVERIFY2(!result, qPrintable(error));
    QCOMPARE(report.value(QStringLiteral("firstError")).toObject().value(QStringLiteral("code")).toString(), expected);
    QCOMPARE(package.marcoRun, QByteArray("old-run"));
    QCOMPARE(package.marcoVar, QByteArray("old-var"));
    QVERIFY(package.compileManifest.value(QStringLiteral("old")).toBool());
    QVERIFY(report.value(QStringLiteral("temporaryCleanupSucceeded")).toBool());
    QVERIFY(report.value(QStringLiteral("capturedOutputBytes")).toDouble() <= 1024 * 1024);
    QVERIFY(timer.elapsed() < 5000);
    if (!report.value(QStringLiteral("stages")).toArray().isEmpty())
    {
        stage = report.value(QStringLiteral("stages")).toArray().first().toObject();
        QVERIFY(stage.value(QStringLiteral("capturedBytes")).toInt() <= 1024);
        if (mode == QStringLiteral("flood")) { QVERIFY(stage.value(QStringLiteral("outputTruncated")).toBool()); }
        if (mode == QStringLiteral("sleep")) { QVERIFY(stage.value(QStringLiteral("ownedProcessKilled")).toBool()); }
    }
    report.insert(QStringLiteral("testEvidenceScope"), QStringLiteral("synthetic-process-fixture-not-ADS-oracle"));
    QVERIFY(macroSaveEvidence(QStringLiteral("synthetic-") + mode, report));
}

/******************************************************************************
 * Function Name   : DeviceMacroTests::realManifestRepeatabilityAndConstantChange
 * Description     : Compare two true SDK runs with the 24-byte oracle and distinguish a changed constant using exact source/SDK evidence.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceMacroTests::realManifestRepeatabilityAndConstantChange() //20261114-7 add
{
    QString sdk = qEnvironmentVariable("KINSEAL_ARM_SDK");
    QTemporaryDir directory;
    HmiProject project;
    DeviceMacroPackage first;
    DeviceMacroPackage second;
    DeviceMacroPackage changed;
    DeviceMacroCompileOptions options;
    QJsonObject report;
    QJsonObject failed;
    QString error;
    QString key;
    QByteArray expected("d6e4f44119b03071a0dea0e8066f084054da6736f8ba5aeb74d443fff914e5da");

    if (sdk.isEmpty()) { QSKIP("Explicit proprietary SDK is not configured; no toolchain is bundled."); }
    QVERIFY(directory.isValid());
    options.temporaryRoot = directory.path();
    project.macros.insert(QStringLiteral("SetLB1"), setBitSource());
    QVERIFY2(compileDeviceMacros(project, sdk, &first, &error, options, &report), qPrintable(error));
    QVERIFY(macroSaveEvidence(QStringLiteral("real-oracle-first"), report));
    QCOMPARE(report.value(QStringLiteral("sdk")).toObject().value(QStringLiteral("requiredFiles")).toArray().size(), 15);
    QCOMPARE(report.value(QStringLiteral("stages")).toArray().size(), 3);
    QVERIFY(!report.value(QStringLiteral("sdkAuthenticated")).toBool());
    QCOMPARE(report.value(QStringLiteral("macros")).toArray().first().toObject().value(QStringLiteral("rawSha256")).toString().toLatin1(), expected);
    key = report.value(QStringLiteral("reproducibilityKeySha256")).toString();
    QVERIFY2(compileDeviceMacros(project, sdk, &second, &error, options, &report), qPrintable(error));
    QVERIFY(macroSaveEvidence(QStringLiteral("real-oracle-repeat"), report));
    QCOMPARE(first.marcoRun, second.marcoRun);
    QCOMPARE(first.marcoVar, second.marcoVar);
    QCOMPARE(key, report.value(QStringLiteral("reproducibilityKeySha256")).toString());
    project.macros[QStringLiteral("SetLB1")].replace(QStringLiteral("LocalBit[1] = 1"), QStringLiteral("LocalBit[1] = 0"));
    QVERIFY2(compileDeviceMacros(project, sdk, &changed, &error, options, &report), qPrintable(error));
    QVERIFY(macroSaveEvidence(QStringLiteral("real-constant-changed"), report));
    QCOMPARE(changed.marcoRun.size(), 1048);
    QVERIFY(changed.marcoRun != first.marcoRun);
    QVERIFY(key != report.value(QStringLiteral("reproducibilityKeySha256")).toString());
    project.macros[QStringLiteral("SetLB1")] = QStringLiteral("void Macro_Entry(void) { invalid = ; }");
    QVERIFY(!compileDeviceMacros(project, sdk, &changed, &error, options, &failed));
    QVERIFY(macroSaveEvidence(QStringLiteral("real-syntax-rejected"), failed));
    QVERIFY(!failed.value(QStringLiteral("ok")).toBool());
    QVERIFY(!failed.value(QStringLiteral("firstError")).toObject().isEmpty());
    QVERIFY(changed.compileManifest.value(QStringLiteral("ok")).toBool());
    QCOMPARE(changed.marcoRun.size(), 1048);
}

QTEST_GUILESS_MAIN(DeviceMacroTests)
#include "device_macro_tests.moc"
