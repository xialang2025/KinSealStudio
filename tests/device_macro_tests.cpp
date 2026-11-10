#include "device_macro_compiler.h"
#include "project_model.h"

#include <QCryptographicHash>
#include <QTemporaryDir>
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

QTEST_GUILESS_MAIN(DeviceMacroTests)
#include "device_macro_tests.moc"
