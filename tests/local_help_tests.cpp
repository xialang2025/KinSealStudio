#include "local_help.h"

#include <QApplication>
#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QHelpEvent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QProcess>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTextBrowser>
#include <QTextDocument>
#include <QtTest>

class LocalHelpTests : public QObject //20261114-10 add
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void catalogAndTopicIdentity();
    void missingAndMalformedMap();
    void unknownAndOversizedIdentifiers();
    void unsafeNavigation_data();
    void unsafeNavigation();
    void localAnchorAndResource();
    void oversizedAndMissingFiles();
    void tamperedMapCannotEscape();
    void viewerLinkAndResourceBoundary();
    void scopedF1AndWhatsThis();
    void contextF1PrecedesMainAction();
    void closeReopenAndOwnerDestruction();
    void nestedDialogOwnsHelp();
    void junctionEscapeIsRejected();
    void packagedCatalog();

private:
    QTemporaryDir *m_temp = nullptr;
    QString m_root;
    bool write(const QString &name, const QByteArray &bytes);
    bool writeMap(const QString &page = QStringLiteral("index.html"));
};

/******************************************************************************
 * Function Name   : LocalHelpTests::write
 * Description     : Write a controlled test file under the Z-drive temporary fixture.
 * Input Parameters: name, bytes
 * Return Value    : True for a complete write.
 * Date            : 2026.11.14
 ******************************************************************************/
bool LocalHelpTests::write(const QString &name, const QByteArray &bytes) //20261114-10 add
{
    QFile file(QDir(m_root).filePath(name));

    return file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size();
}

/******************************************************************************
 * Function Name   : LocalHelpTests::writeMap
 * Description     : Create an independent minimal map with explicit command and nested-dialog context.
 * Input Parameters: page
 * Return Value    : True when the fixture map was saved.
 * Date            : 2026.11.14
 ******************************************************************************/
bool LocalHelpTests::writeMap(const QString &page) //20261114-10 add
{
    QJsonObject map;

    map.insert(QStringLiteral("schemaVersion"), 1);
    map.insert(QStringLiteral("topics"), QJsonObject{{QStringLiteral("start"), QJsonObject{{QStringLiteral("file"), page}}},
        {QStringLiteral("controls"), QJsonObject{{QStringLiteral("file"), QStringLiteral("controls.html#edit")}}}});
    map.insert(QStringLiteral("commands"), QJsonObject{{QStringLiteral("action_H"), QJsonObject{{QStringLiteral("topic"), QStringLiteral("start")},
        {QStringLiteral("capability"), QStringLiteral("本地目录")}}}});
    map.insert(QStringLiteral("contexts"), QJsonArray{QJsonObject{{QStringLiteral("objectName"), QStringLiteral("FixtureDialog")},
        {QStringLiteral("topic"), QStringLiteral("controls")}}});
    return write(QStringLiteral("command-map.json"), QJsonDocument(map).toJson());
}

/******************************************************************************
 * Function Name   : LocalHelpTests::init
 * Description     : Create actual local HTML and resources for every case without touching a device or user window.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpTests::init() //20261114-10 add
{
    m_temp = new QTemporaryDir();
    QVERIFY(m_temp->isValid());
    QVERIFY2(!m_temp->path().startsWith(QStringLiteral("C:"), Qt::CaseInsensitive), "Tests require TEMP/TMP/TMPDIR on Z.");
    m_root = QDir(m_temp->path()).filePath(QStringLiteral("help"));
    QVERIFY(QDir().mkpath(m_root));
    QVERIFY(writeMap());
    QVERIFY(write(QStringLiteral("index.html"), QByteArrayLiteral("<html><body><h1>Start fixture</h1><a href='controls.html#edit'>Edit</a></body></html>")));
    QVERIFY(write(QStringLiteral("controls.html"), QByteArrayLiteral("<html><body><a name='edit'></a><h1>Edit fixture</h1></body></html>")));
    QVERIFY(write(QStringLiteral("style.css"), QByteArrayLiteral("body {color:#123456;}")));
}

/******************************************************************************
 * Function Name   : LocalHelpTests::cleanup
 * Description     : Delete only this test's self-owned temporary fixture.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpTests::cleanup() //20261114-10 add
{
    delete m_temp;
    m_temp = nullptr;
}

/******************************************************************************
 * Function Name   : LocalHelpTests::catalogAndTopicIdentity
 * Description     : Exercise real topic, command, and cross-page anchor resolution from a minimal on-disk map.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpTests::catalogAndTopicIdentity() //20261114-10 add
{
    LocalHelpStore store(m_root);
    QString html;
    QString error;
    QUrl url;
    QJsonObject report;

    QCOMPARE(store.topicForCommand(QStringLiteral("action_H")), QStringLiteral("start"));
    QVERIFY(store.readTopic(QStringLiteral("controls"), &html, &url, &error));
    QVERIFY(html.contains(QStringLiteral("Edit fixture")));
    QCOMPARE(url.fragment(), QStringLiteral("edit"));
    QCOMPARE(QFileInfo(url.toLocalFile()).fileName(), QStringLiteral("controls.html"));
    report = helpCatalogReport(m_root);
    QVERIFY2(report.value(QStringLiteral("valid")).toBool(), QJsonDocument(report).toJson().constData());
    QCOMPARE(report.value(QStringLiteral("commandCount")).toInt(), 1);
    QVERIFY(!report.value(QStringLiteral("globalEntryCoverageComplete")).toBool());
}

/******************************************************************************
 * Function Name   : LocalHelpTests::missingAndMalformedMap
 * Description     : Confirm missing or malformed local files produce user-facing failures without development paths.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpTests::missingAndMalformedMap() //20261114-10 add
{
    QString html;
    QString error;
    QUrl url;

    QVERIFY(QFile::remove(QDir(m_root).filePath(QStringLiteral("command-map.json"))));
    QVERIFY(!LocalHelpStore(m_root).readTopic(QStringLiteral("start"), &html, &url, &error));
    QVERIFY(error.contains(QStringLiteral("help")));
    QVERIFY(!error.contains(m_root));
    QVERIFY(write(QStringLiteral("command-map.json"), QByteArrayLiteral("{broken")));
    QVERIFY(!LocalHelpStore(m_root).readTopic(QStringLiteral("start"), &html, &url, &error));
    QVERIFY(error.contains(QStringLiteral("索引损坏")));
    QVERIFY(!helpCatalogReport(m_root).value(QStringLiteral("valid")).toBool());
}

/******************************************************************************
 * Function Name   : LocalHelpTests::unknownAndOversizedIdentifiers
 * Description     : Verify unknown and oversized help selectors are rejected rather than silently advertising support.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpTests::unknownAndOversizedIdentifiers() //20261114-10 add
{
    LocalHelpStore store(m_root);
    QString html;
    QString error;
    QUrl url;

    QVERIFY(store.topicForCommand(QStringLiteral("missing")).isEmpty());
    QVERIFY(store.topicForCommand(QString(257, QLatin1Char('a'))).isEmpty());
    QVERIFY(!store.readTopic(QStringLiteral("missing"), &html, &url, &error));
    QVERIFY(error.contains(QStringLiteral("尚无独立帮助")));
    QVERIFY(!store.readTopic(QString(129, QLatin1Char('a')), &html, &url, &error));
    QVERIFY(error.contains(QStringLiteral("过长")));
}

/******************************************************************************
 * Function Name   : LocalHelpTests::unsafeNavigation_data
 * Description     : Enumerate independent remote, traversal, query, directory, and unsupported resource schemes.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpTests::unsafeNavigation_data() //20261114-10 add
{
    QTest::addColumn<QString>("target");
    QTest::newRow("https") << QStringLiteral("https://example.invalid/help.html");
    QTest::newRow("http-image") << QStringLiteral("http://example.invalid/image.png");
    QTest::newRow("mailto") << QStringLiteral("mailto:test@example.invalid");
    QTest::newRow("data") << QStringLiteral("data:text/html,secret");
    QTest::newRow("qrc") << QStringLiteral("qrc:/outside.html");
    QTest::newRow("parent") << QStringLiteral("../secret.html");
    QTest::newRow("encoded-parent") << QStringLiteral("%2e%2e/secret.html");
    QTest::newRow("backslash-parent") << QStringLiteral("..\\secret.html");
    QTest::newRow("directory") << QStringLiteral("/");
    QTest::newRow("query") << QStringLiteral("index.html?open=1");
    QTest::newRow("unc") << QStringLiteral("file://example.invalid/share/index.html");
    QTest::newRow("extension") << QStringLiteral("command-map.json");
    QTest::newRow("oversized") << QString(2049, QLatin1Char('a'));
    QTest::newRow("missing-anchor") << QStringLiteral("index.html#missing");
}

/******************************************************************************
 * Function Name   : LocalHelpTests::unsafeNavigation
 * Description     : Reject each unsafe page and subresource target through actual file resolution.
 * Input Parameters: Qt data row target
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpTests::unsafeNavigation() //20261114-10 add
{
    QFETCH(QString, target);
    LocalHelpStore store(m_root);
    QString html;
    QString error;
    QUrl url;
    QUrl current = QUrl::fromLocalFile(QDir(m_root).filePath(QStringLiteral("index.html")));

    QVERIFY(!store.readPage(QUrl(target), current, &html, &url, &error));
    QVERIFY(!error.isEmpty());
    QVERIFY(store.readResource(QUrl(target), current).isEmpty());
}

/******************************************************************************
 * Function Name   : LocalHelpTests::localAnchorAndResource
 * Description     : Keep valid same-root pages, anchors, and bounded CSS usable while rejecting sibling and subdirectory files.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpTests::localAnchorAndResource() //20261114-10 add
{
    LocalHelpStore store(m_root);
    QString html;
    QString error;
    QUrl url;
    QUrl current = QUrl::fromLocalFile(QDir(m_root).filePath(QStringLiteral("controls.html")));
    QFile outside(QDir(m_temp->path()).filePath(QStringLiteral("outside.html")));

    QVERIFY(store.readPage(QUrl(QStringLiteral("#edit")), current, &html, &url, &error));
    QCOMPARE(url.fragment(), QStringLiteral("edit"));
    QVERIFY(!store.readResource(QUrl(QStringLiteral("style.css")), current).isEmpty());
    QVERIFY(outside.open(QIODevice::WriteOnly)); outside.write("private sentinel"); outside.close();
    QVERIFY(!store.readPage(QUrl::fromLocalFile(outside.fileName()), current, &html, &url, &error));
    QVERIFY(QDir().mkpath(QDir(m_root).filePath(QStringLiteral("sub"))));
    QVERIFY(write(QStringLiteral("sub/other.html"), QByteArrayLiteral("subdirectory sentinel")));
    QVERIFY(!store.readPage(QUrl(QStringLiteral("sub/other.html")), current, &html, &url, &error));
}

/******************************************************************************
 * Function Name   : LocalHelpTests::oversizedAndMissingFiles
 * Description     : Confirm bounded loading fails for oversized pages, resources, maps, and missing chapter files.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpTests::oversizedAndMissingFiles() //20261114-10 add
{
    LocalHelpStore store(m_root);
    QString html;
    QString error;
    QUrl url;
    QUrl current = QUrl::fromLocalFile(QDir(m_root).filePath(QStringLiteral("index.html")));

    QVERIFY(write(QStringLiteral("index.html"), QByteArray(1024 * 1024 + 1, 'x')));
    QVERIFY(!store.readTopic(QStringLiteral("start"), &html, &url, &error));
    QVERIFY(write(QStringLiteral("style.css"), QByteArray(2 * 1024 * 1024 + 1, 'x')));
    QVERIFY(store.readResource(QUrl(QStringLiteral("style.css")), current).isEmpty());
    QVERIFY(QFile::remove(QDir(m_root).filePath(QStringLiteral("controls.html"))));
    QVERIFY(!store.readTopic(QStringLiteral("controls"), &html, &url, &error));
    QVERIFY(write(QStringLiteral("command-map.json"), QByteArray(128 * 1024 + 1, ' ')));
    QVERIFY(!LocalHelpStore(m_root).readTopic(QStringLiteral("start"), &html, &url, &error));
}

/******************************************************************************
 * Function Name   : LocalHelpTests::tamperedMapCannotEscape
 * Description     : A modified topic map cannot grant a remote or outside file access capability.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpTests::tamperedMapCannotEscape() //20261114-10 add
{
    QString html;
    QString error;
    QUrl url;

    QVERIFY(writeMap(QStringLiteral("https://example.invalid/remote.html")));
    QVERIFY(!LocalHelpStore(m_root).readTopic(QStringLiteral("start"), &html, &url, &error));
    QVERIFY(!helpCatalogReport(m_root).value(QStringLiteral("valid")).toBool());
    QVERIFY(writeMap(QStringLiteral("../private.html")));
    QVERIFY(!LocalHelpStore(m_root).readTopic(QStringLiteral("start"), &html, &url, &error));
}

/******************************************************************************
 * Function Name   : LocalHelpTests::viewerLinkAndResourceBoundary
 * Description     : Exercise the actual browser's click signal and document resource loader, including a foreign local image.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpTests::viewerLinkAndResourceBoundary() //20261114-10 add
{
    QWidget owner;
    LocalHelpController controller(&owner, m_root);
    QTextBrowser *browser;
    QDialog *dialog;
    QVariant resource;

    controller.showTopic();
    dialog = owner.findChild<QDialog *>(QStringLiteral("LocalHelpDialog"));
    QVERIFY(dialog != nullptr);
    browser = dialog->findChild<QTextBrowser *>(QStringLiteral("LocalHelpBrowser"));
    QVERIFY(browser != nullptr);
    QVERIFY(!browser->openExternalLinks());
    QVERIFY(!browser->openLinks());
    QVERIFY(browser->toPlainText().contains(QStringLiteral("Start fixture")));
    resource = browser->document()->resource(QTextDocument::ImageResource, QUrl(QStringLiteral("https://example.invalid/private.png")));
    QVERIFY(!resource.isValid() || resource.toByteArray().isEmpty());
    resource = browser->document()->resource(QTextDocument::StyleSheetResource, QUrl(QStringLiteral("style.css")));
    QVERIFY(!resource.toByteArray().isEmpty());
    QVERIFY(QMetaObject::invokeMethod(browser, "anchorClicked", Q_ARG(QUrl, QUrl(QStringLiteral("controls.html#edit")))));
    QVERIFY(browser->toPlainText().contains(QStringLiteral("Edit fixture")));
    QVERIFY(QMetaObject::invokeMethod(browser, "anchorClicked", Q_ARG(QUrl, QUrl(QStringLiteral("file:///C:/Windows/win.ini")))));
    QVERIFY(browser->toPlainText().contains(QStringLiteral("help")));
    QVERIFY(!browser->toPlainText().contains(QStringLiteral("Start fixture")));
}

/******************************************************************************
 * Function Name   : LocalHelpTests::scopedF1AndWhatsThis
 * Description     : Handle help events from editor descendants without claiming unrelated application windows.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpTests::scopedF1AndWhatsThis() //20261114-10 add
{
    QWidget owner;
    QWidget unrelated;
    QLineEdit field(&owner);
    LocalHelpController controller(&owner, m_root);
    QKeyEvent key(QEvent::KeyPress, Qt::Key_F1, Qt::NoModifier);
    QKeyEvent shortcut(QEvent::ShortcutOverride, Qt::Key_F1, Qt::NoModifier);
    QKeyEvent foreignKey(QEvent::KeyPress, Qt::Key_F1, Qt::NoModifier);
    QHelpEvent whatsThis(QEvent::WhatsThis, QPoint(), QPoint());
    QDialog *dialog;
    QTextBrowser *browser;

    controller.installContextHelp(); controller.installContextHelp();
    QApplication::sendEvent(&unrelated, &foreignKey);
    QVERIFY(owner.findChild<QDialog *>(QStringLiteral("LocalHelpDialog")) == nullptr);
    field.setProperty("localHelpTopic", QStringLiteral("controls"));
    shortcut.setAccepted(false);
    QApplication::sendEvent(&field, &shortcut);
    QVERIFY(shortcut.isAccepted());
    QVERIFY(owner.findChild<QDialog *>(QStringLiteral("LocalHelpDialog")) == nullptr);
    QApplication::sendEvent(&field, &key);
    dialog = owner.findChild<QDialog *>(QStringLiteral("LocalHelpDialog"));
    QVERIFY(dialog != nullptr);
    browser = dialog->findChild<QTextBrowser *>(QStringLiteral("LocalHelpBrowser"));
    QVERIFY(browser->toPlainText().contains(QStringLiteral("Edit fixture")));
    field.setProperty("localHelpTopic", QStringLiteral("start"));
    QApplication::sendEvent(&field, &whatsThis);
    QVERIFY(browser->toPlainText().contains(QStringLiteral("Start fixture")));
}

/******************************************************************************
 * Function Name   : LocalHelpTests::contextF1PrecedesMainAction
 * Description     : Send real F1 key events with a competing QAction and preserve the focused widget's help topic.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpTests::contextF1PrecedesMainAction() //20261114-10 add
{
    QWidget owner;
    QLineEdit field(&owner);
    QAction action(&owner);
    QSignalSpy triggered(&action, &QAction::triggered);
    LocalHelpController controller(&owner, m_root);
    QDialog *dialog;
    QTextBrowser *browser;

    action.setShortcut(QKeySequence(Qt::Key_F1));
    action.setShortcutContext(Qt::WidgetWithChildrenShortcut);
    owner.addAction(&action);
    field.setProperty("localHelpTopic", QStringLiteral("controls"));
    controller.installContextHelp();
    owner.show(); field.setFocus();
    QTest::keyClick(&field, Qt::Key_F1);
    dialog = owner.findChild<QDialog *>(QStringLiteral("LocalHelpDialog"));
    QVERIFY(dialog != nullptr);
    browser = dialog->findChild<QTextBrowser *>(QStringLiteral("LocalHelpBrowser"));
    QVERIFY(browser->toPlainText().contains(QStringLiteral("Edit fixture")));
    QCOMPARE(triggered.count(), 0);
}

/******************************************************************************
 * Function Name   : LocalHelpTests::closeReopenAndOwnerDestruction
 * Description     : Close and reopen help without leaked dialogs; owner destruction removes both controller and viewer.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpTests::closeReopenAndOwnerDestruction() //20261114-10 add
{
    QWidget *owner;
    LocalHelpController *controller;
    QPointer<QDialog> dialog;
    QPointer<LocalHelpController> guardedController;
    QDialogButtonBox *buttons;

    owner = new QWidget(); controller = new LocalHelpController(owner, m_root); guardedController = controller;
    controller->installContextHelp(); controller->showTopic();
    dialog = owner->findChild<QDialog *>(QStringLiteral("LocalHelpDialog"));
    QVERIFY(!dialog.isNull());
    buttons = dialog->findChild<QDialogButtonBox *>();
    QVERIFY(buttons != nullptr);
    buttons->button(QDialogButtonBox::Close)->click();
    QVERIFY(!dialog->isVisible());
    controller->showCommand(QStringLiteral("action_H"));
    QCOMPARE(owner->findChildren<QDialog *>(QStringLiteral("LocalHelpDialog")).size(), 1);
    QVERIFY(dialog->isVisible());
    delete owner;
    QVERIFY(dialog.isNull()); QVERIFY(guardedController.isNull());
}

/******************************************************************************
 * Function Name   : LocalHelpTests::nestedDialogOwnsHelp
 * Description     : Keep F1 help usable inside a property dialog and recreate it after that dialog is deleted.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpTests::nestedDialogOwnsHelp() //20261114-10 add
{
    QWidget owner;
    LocalHelpController controller(&owner, m_root);
    QDialog *nested;
    QPointer<QDialog> help;

    controller.showTopic();
    nested = new QDialog(&owner); nested->setObjectName(QStringLiteral("FixtureDialog"));
    QCOMPARE(controller.contextTopic(nested), QStringLiteral("controls"));
    controller.showTopic(controller.contextTopic(nested), nested);
    help = nested->findChild<QDialog *>(QStringLiteral("LocalHelpDialog"));
    QVERIFY(!help.isNull());
    delete nested;
    QVERIFY(help.isNull());
    controller.showTopic();
    QVERIFY(owner.findChild<QDialog *>(QStringLiteral("LocalHelpDialog")) != nullptr);
}

/******************************************************************************
 * Function Name   : LocalHelpTests::junctionEscapeIsRejected
 * Description     : Create a real Windows junction and reject it both as help root and as an escaping child resource directory.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpTests::junctionEscapeIsRejected() //20261114-10 add
{
#ifdef Q_OS_WIN
    QString target;
    QString junction;
    QProcess process;
    QString html;
    QString error;
    QUrl url;
    QFile sentinel;

    target = QDir(m_temp->path()).filePath(QStringLiteral("outside"));
    junction = QDir(m_root).filePath(QStringLiteral("alias"));
    QVERIFY(QDir().mkpath(target));
    sentinel.setFileName(QDir(target).filePath(QStringLiteral("index.html")));
    QVERIFY(sentinel.open(QIODevice::WriteOnly)); sentinel.write("outside sentinel"); sentinel.close();
    process.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *arguments) { arguments->flags |= 0x08000000; });
    process.start(QStringLiteral("cmd.exe"), {QStringLiteral("/c"), QStringLiteral("mklink"), QStringLiteral("/J"),
        QDir::toNativeSeparators(junction), QDir::toNativeSeparators(target)});
    QVERIFY(process.waitForFinished(5000));
    QCOMPARE(process.exitCode(), 0);
    QVERIFY(QFileInfo(junction).isJunction());
    QVERIFY(!LocalHelpStore(junction).readTopic(QStringLiteral("start"), &html, &url, &error));
    QVERIFY(!LocalHelpStore(m_root).readPage(QUrl(QStringLiteral("alias/index.html")), QUrl(), &html, &url, &error));
    QVERIFY(QDir().rmdir(junction));
    QVERIFY(QFileInfo::exists(sentinel.fileName()));
#else
    QSKIP("This package's Windows junction contract is exercised on Windows only.");
#endif
}

/******************************************************************************
 * Function Name   : LocalHelpTests::packagedCatalog
 * Description     : Audit the real packaged manual separately from fixture-only behavior tests.
 * Input Parameters: KINSEAL_HELP_SOURCE_ROOT environment variable, or executable-relative help.
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpTests::packagedCatalog() //20261114-10 add
{
    QString root = qEnvironmentVariable("KINSEAL_HELP_SOURCE_ROOT");
    QJsonObject report = helpCatalogReport(root);

    QVERIFY2(report.value(QStringLiteral("valid")).toBool(), QJsonDocument(report).toJson().constData());
    QCOMPARE(report.value(QStringLiteral("topicsChecked")).toArray().size(), 8);
    QVERIFY(report.value(QStringLiteral("commandCount")).toInt() >= 100);
}

QTEST_MAIN(LocalHelpTests)
#include "local_help_tests.moc"
