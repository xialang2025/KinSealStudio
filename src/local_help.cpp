#include "local_help.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QTextBrowser>
#include <QTextDocument>
#include <QVBoxLayout>

namespace {
constexpr qint64 HelpPageLimit = 1024 * 1024;
constexpr qint64 HelpMapLimit = 128 * 1024;
constexpr qint64 HelpResourceLimit = 2 * 1024 * 1024;

/******************************************************************************
 * Function Name   : boundedRead
 * Description     : Reject missing, nonregular, empty, or oversized local help files before allocation.
 * Input Parameters: path, limit, bytes
 * Return Value    : True when the complete bounded file was read.
 * Date            : 2026.11.14
 ******************************************************************************/
bool boundedRead(const QString &path, qint64 limit, QByteArray *bytes) //20261114-10 add
{
    QFileInfo info(path);
    QFile file(path);

    if (!info.isFile() || info.size() <= 0 || info.size() > limit || !file.open(QIODevice::ReadOnly)) { return false; }
    *bytes = file.read(limit + 1);
    return bytes->size() == info.size() && bytes->size() <= limit && file.atEnd();
}

class HelpBrowser : public QTextBrowser
{
public:
    const LocalHelpStore *store;
    QUrl page;

    /******************************************************************************
     * Function Name   : HelpBrowser::HelpBrowser
     * Description     : Disable automatic navigation and external launches in the rich-text viewer.
     * Input Parameters: storeValue, parent
     * Return Value    : None
     * Date            : 2026.11.14
     ******************************************************************************/
    HelpBrowser(const LocalHelpStore *storeValue, QWidget *parent) : QTextBrowser(parent), store(storeValue) //20261114-10 add
    {
        setOpenLinks(false);
        setOpenExternalLinks(false);
        setSearchPaths(QStringList());
    }

protected:
    /******************************************************************************
     * Function Name   : HelpBrowser::loadResource
     * Description     : Resolve image and stylesheet requests through the same local-file boundary as navigation.
     * Input Parameters: type, name
     * Return Value    : Bounded local resource bytes, or an invalid value without fallback I/O.
     * Date            : 2026.11.14
     ******************************************************************************/
    QVariant loadResource(int type, const QUrl &name) override //20261114-10 add
    {
        QByteArray bytes;

        if (type != QTextDocument::ImageResource && type != QTextDocument::StyleSheetResource) { return QVariant(); }
        bytes = store->readResource(name, page);
        return bytes.isEmpty() ? QVariant() : QVariant(bytes);
    }
};
}

/******************************************************************************
 * Function Name   : LocalHelpStore::LocalHelpStore
 * Description     : Load the packaged topic map from the application-relative help directory with a fixed size bound.
 * Input Parameters: root (optional explicit root for controlled tests)
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
LocalHelpStore::LocalHelpStore(const QString &root) //20261114-10 add
{
    QString requested;
    QByteArray bytes;
    QJsonParseError parseError;
    QJsonDocument document;
    QFileInfo mapInfo;
    QFileInfo rootInfo;

    requested = root.isEmpty() ? QCoreApplication::applicationDirPath() + QStringLiteral("/help") : root;
    rootInfo.setFile(requested);
    m_root = rootInfo.isSymLink() || rootInfo.isJunction() ? QString() : rootInfo.canonicalFilePath();
    mapInfo.setFile(QDir(m_root).filePath(QStringLiteral("command-map.json")));
    if (m_root.isEmpty() || !QFileInfo(m_root).isDir() || mapInfo.isSymLink() || mapInfo.isJunction() || mapInfo.canonicalPath() != m_root ||
        !boundedRead(mapInfo.filePath(), HelpMapLimit, &bytes))
    {
        m_mapError = QStringLiteral("本地帮助缺失或无法读取。请恢复程序目录旁的 help 文件夹，再按 F1。");
        return;
    }
    document = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject() ||
        !document.object().value(QStringLiteral("schemaVersion")).isDouble() ||
        document.object().value(QStringLiteral("schemaVersion")).toDouble() != 1.0 ||
        !document.object().value(QStringLiteral("topics")).isObject() ||
        document.object().value(QStringLiteral("topics")).toObject().size() > 128 ||
        !document.object().value(QStringLiteral("commands")).isObject() ||
        document.object().value(QStringLiteral("commands")).toObject().size() > 1024 ||
        document.object().value(QStringLiteral("contexts")).toArray().size() > 256)
    {
        m_mapError = QStringLiteral("本地帮助索引损坏。请从同一版本便携包恢复 help 文件夹。");
        return;
    }
    m_map = document.object();
}

/******************************************************************************
 * Function Name   : LocalHelpStore::rootPath
 * Description     : Expose the canonical help root for diagnostics without changing application settings.
 * Input Parameters: None
 * Return Value    : Canonical help directory, or empty when absent.
 * Date            : 2026.11.14
 ******************************************************************************/
QString LocalHelpStore::rootPath() const //20261114-10 add
{
    return m_root;
}

/******************************************************************************
 * Function Name   : LocalHelpStore::topicForCommand
 * Description     : Resolve a declared command without treating unknown commands as supported features.
 * Input Parameters: commandId
 * Return Value    : Declared topic identifier, or empty for unknown commands.
 * Date            : 2026.11.14
 ******************************************************************************/
QString LocalHelpStore::topicForCommand(const QString &commandId) const //20261114-10 add
{
    if (commandId.size() > 256) { return QString(); }
    return m_map.value(QStringLiteral("commands")).toObject().value(commandId).toObject().value(QStringLiteral("topic")).toString();
}

/******************************************************************************
 * Function Name   : LocalHelpStore::topicForContext
 * Description     : Resolve explicit widget topics and limited dialog-title mappings from the nearest ancestor.
 * Input Parameters: context
 * Return Value    : A mapped topic, or the start page for unlisted contexts.
 * Date            : 2026.11.14
 ******************************************************************************/
QString LocalHelpStore::topicForContext(const QWidget *context) const //20261114-10 add
{
    const QWidget *widget;
    QString explicitTopic;
    QJsonArray contexts;
    QJsonValue value;
    QJsonObject entry;

    contexts = m_map.value(QStringLiteral("contexts")).toArray();
    for (widget = context; widget != nullptr; widget = widget->parentWidget())
    {
        explicitTopic = widget->property("localHelpTopic").toString();
        if (!explicitTopic.isEmpty()) { return explicitTopic; }
        Q_FOREACH (value, contexts)
        {
            entry = value.toObject();
            if ((!entry.value(QStringLiteral("objectName")).toString().isEmpty() && widget->objectName() == entry.value(QStringLiteral("objectName")).toString()) ||
                (!entry.value(QStringLiteral("titleContains")).toString().isEmpty() && widget->windowTitle().contains(entry.value(QStringLiteral("titleContains")).toString())))
            {
                return entry.value(QStringLiteral("topic")).toString();
            }
        }
    }
    return QStringLiteral("start");
}

/******************************************************************************
 * Function Name   : LocalHelpStore::resolveFile
 * Description     : Allow only regular files directly inside the canonical help root; reject remote, traversal, query, and alias escapes.
 * Input Parameters: target, current, page, path, resolved, error
 * Return Value    : True for a permitted existing file with the requested media extension.
 * Date            : 2026.11.14
 ******************************************************************************/
bool LocalHelpStore::resolveFile(const QUrl &target, const QUrl &current, bool page, QString *path,
                                QUrl *resolved, QString *error) const //20261114-10 add
{
    QUrl url;
    QString targetPath;
    QFileInfo info;
    QStringList suffixes;
    QString segment;

    if (m_root.isEmpty()) { *error = QStringLiteral("本地帮助目录不存在，请恢复随包的 help 文件夹。"); return false; }
    if (target.toString().size() > 2048 || !target.isValid() || target.hasQuery() || !target.authority().isEmpty() ||
        (!target.scheme().isEmpty() && target.scheme() != QStringLiteral("file")))
    {
        *error = QStringLiteral("帮助仅打开随包本地页面；此链接不是允许的本地帮助文件。"); return false;
    }
    targetPath = target.path(QUrl::FullyDecoded);
    targetPath.replace(QLatin1Char('\\'), QLatin1Char('/'));
    Q_FOREACH (segment, targetPath.split(QLatin1Char('/')))
    {
        if (segment == QStringLiteral("..") || segment == QStringLiteral("."))
        { *error = QStringLiteral("帮助链接包含不允许的路径跳转。"); return false; }
    }
    url = current.isEmpty() ? QUrl::fromLocalFile(QDir(m_root).filePath(QStringLiteral("index.html"))).resolved(target) : current.resolved(target);
    if (!url.isLocalFile()) { *error = QStringLiteral("无法定位本地帮助页面。"); return false; }
    info.setFile(url.toLocalFile());
    suffixes = page ? QStringList{QStringLiteral("html"), QStringLiteral("htm")} :
        QStringList{QStringLiteral("png"), QStringLiteral("jpg"), QStringLiteral("jpeg"), QStringLiteral("bmp"), QStringLiteral("css")};
    if (!info.isFile() || info.isSymLink() || info.isJunction() || info.fileName().contains(QLatin1Char(':')) ||
        info.canonicalPath() != m_root || !suffixes.contains(info.suffix().toLower()))
    {
        *error = QStringLiteral("本地帮助文件缺失或链接超出 help 目录；请恢复同版本帮助文件。"); return false;
    }
    *path = info.canonicalFilePath();
    *resolved = QUrl::fromLocalFile(*path);
    resolved->setFragment(url.fragment());
    return true;
}

/******************************************************************************
 * Function Name   : LocalHelpStore::readTopic
 * Description     : Resolve a known topic through the packaged map and then apply normal bounded page loading.
 * Input Parameters: topic, html, url, error
 * Return Value    : True when the mapped HTML page is available.
 * Date            : 2026.11.14
 ******************************************************************************/
bool LocalHelpStore::readTopic(const QString &topic, QString *html, QUrl *url, QString *error) const //20261114-10 add
{
    QString file;

    if (!m_mapError.isEmpty()) { *error = m_mapError; return false; }
    if (topic.size() > 128) { *error = QStringLiteral("帮助主题标识过长，无法打开。"); return false; }
    file = m_map.value(QStringLiteral("topics")).toObject().value(topic).toObject().value(QStringLiteral("file")).toString();
    if (file.isEmpty()) { *error = QStringLiteral("此入口尚无独立帮助章节。请按“目录”查看已支持流程与限制。"); return false; }
    return readPage(QUrl(file), QUrl(), html, url, error);
}

/******************************************************************************
 * Function Name   : LocalHelpStore::readPage
 * Description     : Load a complete local HTML page with a hard size bound and a canonical document URL.
 * Input Parameters: target, current, html, url, error
 * Return Value    : True when HTML was read without fallback access.
 * Date            : 2026.11.14
 ******************************************************************************/
bool LocalHelpStore::readPage(const QUrl &target, const QUrl &current, QString *html, QUrl *url, QString *error) const //20261114-10 add
{
    QString path;
    QByteArray bytes;
    QRegularExpression anchor;

    if (!resolveFile(target, current, true, &path, url, error)) { return false; }
    if (!boundedRead(path, HelpPageLimit, &bytes))
    { *error = QStringLiteral("帮助页面为空、过大或读取失败；请恢复同版本 help 文件夹。"); return false; }
    *html = QString::fromUtf8(bytes);
    if (!url->fragment().isEmpty())
    {
        anchor.setPattern(QStringLiteral("\\b(?:id|name)\\s*=\\s*[\"']") + QRegularExpression::escape(url->fragment()) + QStringLiteral("[\"']"));
        if (!anchor.match(*html).hasMatch())
        { *error = QStringLiteral("帮助章节内的定位标记不存在。请返回目录或恢复同版本帮助文件。"); return false; }
    }
    return true;
}

/******************************************************************************
 * Function Name   : LocalHelpStore::readResource
 * Description     : Block all nonlocal resource loads, including data URLs, Qt resources, and external image references.
 * Input Parameters: target, current
 * Return Value    : Bounded image or stylesheet bytes, or empty on rejection.
 * Date            : 2026.11.14
 ******************************************************************************/
QByteArray LocalHelpStore::readResource(const QUrl &target, const QUrl &current) const //20261114-10 add
{
    QString path;
    QString error;
    QUrl url;
    QByteArray bytes;

    if (!resolveFile(target, current, false, &path, &url, &error) || !boundedRead(path, HelpResourceLimit, &bytes)) { return QByteArray(); }
    return bytes;
}

/******************************************************************************
 * Function Name   : helpCatalogReport
 * Description     : Audit packaged topic, command, and quoted HTML link targets without opening windows or endpoints.
 * Input Parameters: root (empty uses the executable-relative help folder)
 * Return Value    : JSON report whose valid flag covers this catalog only, not complete application entry coverage.
 * Date            : 2026.11.14
 ******************************************************************************/
QJsonObject helpCatalogReport(const QString &root) //20261114-10 add
{
    LocalHelpStore store(root);
    QJsonObject report;
    QJsonObject map;
    QJsonObject topics;
    QJsonObject commands;
    QJsonArray failures;
    QJsonArray checked;
    QByteArray bytes;
    QString html;
    QString linkedHtml;
    QString error;
    QString topic;
    QString attribute;
    QUrl page;
    QUrl destination;
    QJsonObject::const_iterator it;
    QRegularExpression links(QStringLiteral("\\b(href|src)\\s*=\\s*[\"']([^\"']*)[\"']"), QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator matches;
    QRegularExpressionMatch match;
    int linkCount;

    linkCount = 0;
    if (!boundedRead(QDir(store.rootPath()).filePath(QStringLiteral("command-map.json")), HelpMapLimit, &bytes))
    { failures.append(QStringLiteral("帮助索引不存在、为空或超出大小限制。")); }
    else { map = QJsonDocument::fromJson(bytes).object(); }
    topics = map.value(QStringLiteral("topics")).toObject();
    commands = map.value(QStringLiteral("commands")).toObject();
    if (topics.isEmpty() || topics.size() > 128 || commands.isEmpty() || commands.size() > 1024)
    { failures.append(QStringLiteral("帮助索引的主题或命令数量无效。")); }
    for (it = topics.constBegin(); it != topics.constEnd(); ++it)
    {
        if (!store.readTopic(it.key(), &html, &page, &error)) { failures.append(it.key() + QStringLiteral(": ") + error); continue; }
        checked.append(it.key());
        matches = links.globalMatch(html);
        while (matches.hasNext())
        {
            match = matches.next();
            attribute = match.captured(1).toLower();
            ++linkCount;
            if (attribute == QStringLiteral("href"))
            {
                if (!store.readPage(QUrl(match.captured(2)), page, &linkedHtml, &destination, &error))
                { failures.append(it.key() + QStringLiteral(" 链接: ") + error); }
            }
            else if (store.readResource(QUrl(match.captured(2)), page).isEmpty())
            { failures.append(it.key() + QStringLiteral(" 包含不可读取的本地资源。")); }
        }
    }
    for (it = commands.constBegin(); it != commands.constEnd(); ++it)
    {
        topic = store.topicForCommand(it.key());
        if (it.key().size() > 256 || !topics.contains(topic) || it.value().toObject().value(QStringLiteral("capability")).toString().isEmpty())
        { failures.append(QStringLiteral("命令缺少有效主题或能力说明：") + it.key().left(256)); }
    }
    report.insert(QStringLiteral("schemaVersion"), 1);
    report.insert(QStringLiteral("valid"), failures.isEmpty());
    report.insert(QStringLiteral("topicsChecked"), checked);
    report.insert(QStringLiteral("commandCount"), commands.size());
    report.insert(QStringLiteral("linkCount"), linkCount);
    report.insert(QStringLiteral("failures"), failures);
    report.insert(QStringLiteral("globalEntryCoverageComplete"), false);
    report.insert(QStringLiteral("scope"), QStringLiteral("随包本地帮助目录、显式命令映射与有限上下文；不据此声明全入口D/U或设备T/E通过。"));
    return report;
}

/******************************************************************************
 * Function Name   : LocalHelpController::LocalHelpController
 * Description     : Keep help lifetime within the editor without opening external processes.
 * Input Parameters: owner, root
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
LocalHelpController::LocalHelpController(QWidget *owner, const QString &root) : QObject(owner), m_owner(owner), m_store(root) //20261114-10 add
{
}

/******************************************************************************
 * Function Name   : LocalHelpController::~LocalHelpController
 * Description     : Destroy the viewer before its resource store, even when the owner closes before the help window.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
LocalHelpController::~LocalHelpController() //20261114-10 add
{
    delete m_dialog.data();
}

/******************************************************************************
 * Function Name   : LocalHelpController::installContextHelp
 * Description     : Install one application filter that handles help events only within the owning editor hierarchy.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpController::installContextHelp() //20261114-10 add
{
    if (!m_installed && qApp != nullptr) { qApp->installEventFilter(this); m_installed = true; }
}

/******************************************************************************
 * Function Name   : LocalHelpController::ensureDialog
 * Description     : Create a reusable in-application viewer parented to the current editor dialog when needed.
 * Input Parameters: context
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpController::ensureDialog(QWidget *context) //20261114-10 add
{
    QWidget *parent;
    QVBoxLayout *layout;
    QHBoxLayout *navigation;
    QPushButton *contents;
    QDialogButtonBox *buttons;

    if (!m_dialog.isNull())
    {
        if (context != nullptr && context->window() != m_dialog->parentWidget())
        {
            m_dialog->hide();
            m_dialog->setParent(context->window(), Qt::Dialog);
        }
        return;
    }
    parent = context != nullptr ? context->window() : m_owner.data();
    m_dialog = new QDialog(parent);
    m_dialog->setObjectName(QStringLiteral("LocalHelpDialog"));
    m_dialog->setWindowTitle(QStringLiteral("KinSealStudio Lite 本地帮助"));
    m_dialog->resize(820, 620);
    layout = new QVBoxLayout(m_dialog);
    navigation = new QHBoxLayout();
    contents = new QPushButton(QStringLiteral("目录"), m_dialog);
    contents->setObjectName(QStringLiteral("LocalHelpContents"));
    m_status = new QLabel(m_dialog);
    m_status->setObjectName(QStringLiteral("LocalHelpStatus"));
    m_status->setWordWrap(true);
    navigation->addWidget(contents);
    navigation->addWidget(m_status, 1);
    layout->addLayout(navigation);
    m_browser = new HelpBrowser(&m_store, m_dialog);
    m_browser->setObjectName(QStringLiteral("LocalHelpBrowser"));
    layout->addWidget(m_browser, 1);
    buttons = new QDialogButtonBox(QDialogButtonBox::Close, m_dialog);
    buttons->button(QDialogButtonBox::Close)->setText(QStringLiteral("关闭"));
    layout->addWidget(buttons);
    connect(contents, &QPushButton::clicked, this, [this]() { showTopic(QStringLiteral("start")); });
    connect(buttons, &QDialogButtonBox::rejected, m_dialog, &QDialog::reject);
    connect(m_browser, &QTextBrowser::anchorClicked, this, [this](const QUrl &target) { navigate(target); });
}

/******************************************************************************
 * Function Name   : LocalHelpController::showPage
 * Description     : Set only already-validated HTML and a local base URL; preserve local anchors.
 * Input Parameters: html, url
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpController::showPage(const QString &html, const QUrl &url) //20261114-10 add
{
    m_current = url;
    static_cast<HelpBrowser *>(m_browser)->page = url;
    m_browser->document()->setBaseUrl(url);
    m_browser->setHtml(html);
    if (!url.fragment().isEmpty()) { m_browser->scrollToAnchor(url.fragment()); }
    m_status->setText(QStringLiteral("离线帮助 · %1").arg(QFileInfo(url.toLocalFile()).fileName()));
}

/******************************************************************************
 * Function Name   : LocalHelpController::showError
 * Description     : Present a recoverable local-help failure without launching another application.
 * Input Parameters: error
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpController::showError(const QString &error) //20261114-10 add
{
    m_current = QUrl();
    static_cast<HelpBrowser *>(m_browser)->page = QUrl();
    m_browser->setPlainText(error);
    m_status->setText(QStringLiteral("帮助未加载 · 可关闭窗口继续编辑"));
}

/******************************************************************************
 * Function Name   : LocalHelpController::navigate
 * Description     : Apply local navigation rules to every clicked link and show failures inside the viewer.
 * Input Parameters: target
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpController::navigate(const QUrl &target) //20261114-10 add
{
    QString html;
    QString error;
    QUrl url;

    if (m_store.readPage(target, m_current, &html, &url, &error)) { showPage(html, url); }
    else { showError(error); }
}

/******************************************************************************
 * Function Name   : LocalHelpController::showTopic
 * Description     : Open a packaged help topic within the application, including clear missing-file feedback.
 * Input Parameters: topic, context
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpController::showTopic(const QString &topic, QWidget *context) //20261114-10 add
{
    QString html;
    QString error;
    QUrl url;

    ensureDialog(context);
    if (m_store.readTopic(topic, &html, &url, &error)) { showPage(html, url); }
    else { showError(error); }
    m_dialog->show();
}

/******************************************************************************
 * Function Name   : LocalHelpController::showCommand
 * Description     : Route a command identifier through the frozen user-help map.
 * Input Parameters: commandId, context
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void LocalHelpController::showCommand(const QString &commandId, QWidget *context) //20261114-10 add
{
    showTopic(m_store.topicForCommand(commandId), context);
}

/******************************************************************************
 * Function Name   : LocalHelpController::contextTopic
 * Description     : Expose the finite context resolver so integrations and tests use identical topic rules.
 * Input Parameters: context
 * Return Value    : Context topic or start page.
 * Date            : 2026.11.14
 ******************************************************************************/
QString LocalHelpController::contextTopic(const QWidget *context) const //20261114-10 add
{
    return m_store.topicForContext(context);
}

/******************************************************************************
 * Function Name   : LocalHelpController::eventFilter
 * Description     : Consume explicit F1 and What's This requests only for widgets owned by this editor.
 * Input Parameters: watched, event
 * Return Value    : True only when an owned help request was handled.
 * Date            : 2026.11.14
 ******************************************************************************/
bool LocalHelpController::eventFilter(QObject *watched, QEvent *event) //20261114-10 add
{
    QWidget *widget;
    QWidget *ancestor;
    QKeyEvent *key;
    bool request;
    bool owned;

    request = event->type() == QEvent::WhatsThis;
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::ShortcutOverride)
    {
        key = static_cast<QKeyEvent *>(event);
        request = key->key() == Qt::Key_F1 && key->modifiers() == Qt::NoModifier && !key->isAutoRepeat();
    }
    if (!request || m_owner.isNull()) { return QObject::eventFilter(watched, event); }
    widget = qobject_cast<QWidget *>(watched);
    owned = false;
    for (ancestor = widget; ancestor != nullptr; ancestor = ancestor->parentWidget())
    {
        if (ancestor == m_dialog.data()) { return QObject::eventFilter(watched, event); }
        if (ancestor == m_owner.data()) { owned = true; break; }
    }
    if (!owned) { return QObject::eventFilter(watched, event); }
    // Reserve F1 before the main-window QAction shortcut can discard the nested dialog context.
    if (event->type() == QEvent::ShortcutOverride) { event->accept(); return true; }
    showTopic(contextTopic(widget), widget);
    event->accept();
    return true;
}
