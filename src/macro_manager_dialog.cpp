#include "macro_manager_dialog.h"
#include "macro_workspace.h"
#include "dialog_button_text.h" //20261127-1 add
#include <QCloseEvent>
#include <QCheckBox> //20261127-1 add
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMenu>
#include <QMessageBox> //20261127-1 add
#include <QTreeWidget>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression> //20261127-1 add
#include <QSaveFile>
#include <QSignalBlocker>
#include <QSplitter>
#include <QTextCursor>
#include <QTextDocument> //20261127-1 add
#include <QThread>
#include <QTabWidget>
#include <QVBoxLayout>
#include <atomic>

struct MacroCompileJob //20261114-7 add: Worker-owned values outlive a destroyed dialog without retaining QWidget pointers.
{
    std::atomic_bool cancelled{false};
    bool ok = false;
    DeviceMacroPackage package;
    QJsonObject report;
    QString error;
};

/******************************************************************************
 * Function Name   : macroLibraryExamples
 * Description     : Read bounded embedded source fragments observed in the native editor, without executing them.
 * Input Parameters: None
 * Return Value    : Ordered function and graphics library entries.
 * Date            : 2026.11.14
 ******************************************************************************/
static QJsonArray macroLibraryExamples() //20261114-7 add
{
    QFile file(QStringLiteral(":/macro-library/examples.json"));

    if (!file.open(QIODevice::ReadOnly) || file.size() > 256 * 1024) { return {}; }
    return QJsonDocument::fromJson(file.readAll()).object().value(QStringLiteral("examples")).toArray();
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::MacroManagerDialog
 * Description     : Build a detached ordinary-source workspace with local editing and asynchronous SDK compilation.
 * Input Parameters: project, parent
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
MacroManagerDialog::MacroManagerDialog(const HmiProject &project, QWidget *parent) //20261114-7 add
    : QDialog(parent), m_draft(project), m_originalMacros(project.macros), m_originalIds(project.macroIds) //20261127-1 change
{
    QVBoxLayout *layout;
    QHBoxLayout *toolbar;
    QSplitter *splitter;
    QWidget *editor;
    QVBoxLayout *editorLayout;
    QDialogButtonBox *buttons;
    QTabWidget *leftTabs;
    QTabWidget *outputTabs;
    QTreeWidget *library;
    QTreeWidgetItem *group = nullptr;
    QTreeWidgetItem *entry;
    QString groupName;
    QLabel *variableScope;
    QPushButton *button;
    QStringList labels;
    QStringList names;
    int index;

    setObjectName(QStringLiteral("MacroManagerDialog"));
    setWindowTitle(QStringLiteral("宏指令编辑器"));
    resize(1050, 680);
    layout = new QVBoxLayout(this);
    toolbar = new QHBoxLayout();
    labels = {QStringLiteral("新建"), QStringLiteral("复制宏"), QStringLiteral("重命名"), QStringLiteral("删除"),
              QStringLiteral("导入源码"), QStringLiteral("导出源码"), QStringLiteral("撤销"), QStringLiteral("恢复"), QStringLiteral("查找")};
    names = {QStringLiteral("MacroNew"), QStringLiteral("MacroCopy"), QStringLiteral("MacroRename"), QStringLiteral("MacroDelete"),
             QStringLiteral("MacroImport"), QStringLiteral("MacroExport"), QStringLiteral("MacroUndo"), QStringLiteral("MacroRedo"), QStringLiteral("MacroFind")};
    for (index = 0; index < labels.size(); ++index)
    {
        button = new QPushButton(labels.at(index), this);
        button->setObjectName(names.at(index));
        toolbar->addWidget(button);
        m_editingWidgets.append(button);
        connect(button, &QPushButton::clicked, this, [this, index]() {
            if (index == 0) { promptNew(); }
            else if (index == 1) { promptCopy(); }
            else if (index == 2) { promptRename(); }
            else if (index == 3) { promptDelete(); }
            else if (index == 4) { promptImport(); }
            else if (index == 5) { promptExport(); }
            else if (index == 6) { m_source->undo(); }
            else if (index == 7) { m_source->redo(); }
            else { findSource(); }
        });
    }
    layout->addLayout(toolbar);
    splitter = new QSplitter(this);
    leftTabs = new QTabWidget(splitter);
    leftTabs->setObjectName(QStringLiteral("MacroLibraryTabs"));
    m_list = new QListWidget(leftTabs);
    m_list->setObjectName(QStringLiteral("MacroList"));
    leftTabs->addTab(m_list, QStringLiteral("宏"));
    library = new QTreeWidget(leftTabs);
    library->setObjectName(QStringLiteral("MacroLibraryTree"));
    library->setHeaderHidden(true);
    for (const QJsonValue &value : macroLibraryExamples())
    {
        QJsonObject example = value.toObject();

        if (groupName != example.value(QStringLiteral("group")).toString())
        {
            groupName = example.value(QStringLiteral("group")).toString();
            group = new QTreeWidgetItem(library, {groupName});
        }
        entry = new QTreeWidgetItem(group, {example.value(QStringLiteral("name")).toString()});
        entry->setData(0, Qt::UserRole, example.value(QStringLiteral("name")).toString());
        entry->setToolTip(0, QStringLiteral("右键插入例子；请放入适当位置后编译。"));
    }
    library->expandAll();
    library->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(library, &QTreeWidget::customContextMenuRequested, this, [this, library](const QPoint &position) {
        QTreeWidgetItem *item = library->itemAt(position);
        QMenu menu; //20261114-7 change: A nested menu must not be stack-deleted by a destroyed parent widget.
        QString name;

        if (item == nullptr) { return; }
        name = item->data(0, Qt::UserRole).toString();
        if (name.isEmpty()) { return; }
        menu.addAction(QStringLiteral("插入例子"), this, [this, name]() {
            QString error;

            if (!insertLibraryExample(name, &error)) { showError(error); }
        });
        menu.exec(library->viewport()->mapToGlobal(position));
    });
    leftTabs->addTab(library, QStringLiteral("函数库"));
    editor = new QWidget(splitter);
    editorLayout = new QVBoxLayout(editor);
    editorLayout->setContentsMargins(0, 0, 0, 0);
    m_identity = new QLabel(editor);
    m_identity->setObjectName(QStringLiteral("MacroIdentity"));
    editorLayout->addWidget(m_identity);
    m_source = new QPlainTextEdit(editor);
    m_source->setObjectName(QStringLiteral("MacroSource"));
    m_source->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_source->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_source->setTabStopDistance(m_source->fontMetrics().horizontalAdvance(QLatin1Char(' ')) * 4);
    editorLayout->addWidget(m_source, 1);
    outputTabs = new QTabWidget(editor);
    outputTabs->setObjectName(QStringLiteral("MacroOutputTabs"));
    m_output = new QPlainTextEdit(outputTabs);
    m_output->setObjectName(QStringLiteral("MacroCompileOutput"));
    m_output->setReadOnly(true);
    m_output->setMaximumBlockCount(4000);
    outputTabs->addTab(m_output, QStringLiteral("信息输出"));
    variableScope = new QLabel(QStringLiteral("当前仅编译零地址变量的普通宏。地址关联变量尚未开放。"), outputTabs);
    variableScope->setWordWrap(true);
    outputTabs->addTab(variableScope, QStringLiteral("变量设定(0/25)"));
    outputTabs->setMaximumHeight(180);
    editorLayout->addWidget(outputTabs);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({180, 800});
    layout->addWidget(splitter, 1);
    m_editingWidgets.append(m_list);
    m_editingWidgets.append(m_source);
    m_editingWidgets.append(library);
    m_error = new QLabel(this);
    m_error->setObjectName(QStringLiteral("MacroError"));
    m_error->setWordWrap(true);
    layout->addWidget(m_error);
    toolbar = new QHBoxLayout();
    m_compile = new QPushButton(QStringLiteral("全部编译"), this);
    m_compile->setObjectName(QStringLiteral("MacroCompile"));
    m_cancelCompile = new QPushButton(QStringLiteral("停止编译"), this);
    m_cancelCompile->setObjectName(QStringLiteral("MacroCancelCompile"));
    m_cancelCompile->setEnabled(false);
    toolbar->addWidget(m_compile);
    toolbar->addWidget(m_cancelCompile);
    toolbar->addStretch();
    buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    buttons->setObjectName(QStringLiteral("MacroDialogButtons"));
    toolbar->addWidget(buttons);
    layout->addLayout(toolbar);
    connect(buttons, &QDialogButtonBox::accepted, this, &MacroManagerDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &MacroManagerDialog::reject);
    connect(m_compile, &QPushButton::clicked, this, [this]() { startCompilation(); });
    connect(m_cancelCompile, &QPushButton::clicked, this, &MacroManagerDialog::cancelCompilation);
    connect(m_list, &QListWidget::currentTextChanged, this, [this](const QString &name) {
        QString error;

        if (!m_rebuilding && !selectMacro(name, &error))
        {
            QSignalBlocker blocker(m_list);

            showError(error);
            m_list->setCurrentRow(m_draft.macros.keys().indexOf(m_currentName)); //20261114-7 change: Retain invalid text for correction instead of reloading the saved draft.
        }
    });
    connect(m_source, &QPlainTextEdit::textChanged, this, [this]() {
        if (!m_rebuilding) { m_report = QJsonObject(); }
    });
    rebuild(m_draft.macros.isEmpty() ? QString() : m_draft.macros.firstKey());
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::~MacroManagerDialog
 * Description     : Request cancellation without destroying a live worker thread or accessing it from another thread.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
MacroManagerDialog::~MacroManagerDialog() //20261114-7 add
{
    cancelCompilation();
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::draftProject
 * Description     : Expose only source drafts already validated through workspace transactions.
 * Input Parameters: None
 * Return Value    : Detached project reference.
 * Date            : 2026.11.14
 ******************************************************************************/
const HmiProject &MacroManagerDialog::draftProject() const //20261114-7 add
{
    return m_draft;
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::rebuild
 * Description     : Rebuild the name list without flushing the previous source into another macro.
 * Input Parameters: selected
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroManagerDialog::rebuild(const QString &selected) //20261114-7 add
{
    QSignalBlocker blocker(m_list);

    m_rebuilding = true;
    m_list->clear();
    m_list->addItems(m_draft.macros.keys());
    m_currentName = m_draft.macros.contains(selected) ? selected : (m_draft.macros.isEmpty() ? QString() : m_draft.macros.firstKey());
    m_list->setCurrentRow(m_draft.macros.keys().indexOf(m_currentName));
    m_source->setPlainText(m_draft.macros.value(m_currentName));
    m_source->setReadOnly(m_currentName.isEmpty());
    m_identity->setText(m_currentName.isEmpty() ? QStringLiteral("尚无宏，请新建或导入源码。") : m_currentName + QStringLiteral(" · 普通宏"));
    m_rebuilding = false;
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::showError
 * Description     : Keep validation feedback in the workspace so the source remains available for correction.
 * Input Parameters: error
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroManagerDialog::showError(const QString &error) //20261114-7 add
{
    m_error->setText(error);
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::applySource
 * Description     : Validate current text before leaving a macro or publishing an accepted project draft.
 * Input Parameters: error
 * Return Value    : True after an atomic source update.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroManagerDialog::applySource(QString *error) //20261114-7 add
{
    if (compiling()) { if (error != nullptr) { *error = QStringLiteral("请先停止编译。"); } return false; }
    if (m_currentName.isEmpty()) { return true; }
    return MacroWorkspace::update(&m_draft, m_currentName, m_source->toPlainText(), error);
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::selectMacro
 * Description     : Flush the previous macro and restore a different immutable source by its name.
 * Input Parameters: name, error
 * Return Value    : True after selection; failed selection retains text for correction.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroManagerDialog::selectMacro(const QString &name, QString *error) //20261114-7 add
{
    if (!m_draft.macros.contains(name) || !applySource(error)) { return false; }
    if (name != m_currentName) { rebuild(name); }
    return true;
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::createSource
 * Description     : Add a validated ordinary macro without editing the caller's project.
 * Input Parameters: name, source, error
 * Return Value    : True after a successful source transaction.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroManagerDialog::createSource(const QString &name, const QString &source, QString *error) //20261114-7 add
{
    if (!applySource(error) || !MacroWorkspace::create(&m_draft, name, source, error)) { return false; }
    m_report = QJsonObject(); rebuild(name); return true;
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::renameSource
 * Description     : Rename the source and all supported reference carriers through one workspace transaction.
 * Input Parameters: name, newName, error
 * Return Value    : True after references and stable identities migrate together.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroManagerDialog::renameSource(const QString &name, const QString &newName, QString *error) //20261114-7 add
{
    if (!applySource(error) || !MacroWorkspace::rename(&m_draft, name, newName, error)) { return false; }
    m_report = QJsonObject(); rebuild(newName); return true;
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::removeSource
 * Description     : Reject referenced macros while allowing unreferenced source removal from a detached project.
 * Input Parameters: name, error
 * Return Value    : True after a safe atomic removal.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroManagerDialog::removeSource(const QString &name, QString *error) //20261114-7 add
{
    if (!applySource(error) || !MacroWorkspace::remove(&m_draft, name, error)) { return false; }
    m_report = QJsonObject(); rebuild(QString()); return true;
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::importSourceFile
 * Description     : Import a bounded strict UTF-8 .c source after reading it completely.
 * Input Parameters: path, name, replaceExisting, error
 * Return Value    : True without publishing partial reads or invalid bytes.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroManagerDialog::importSourceFile(const QString &path, const QString &name, bool replaceExisting, QString *error) //20261114-7 add
{
    QFile file(path);
    QByteArray bytes;

    if (!applySource(error)) { return false; }
    if (QFileInfo(path).suffix().compare(QStringLiteral("c"), Qt::CaseInsensitive) != 0 ||
        !file.open(QIODevice::ReadOnly) || file.size() > MacroWorkspace::kMaximumEncodedSourceBytes)
    { if (error != nullptr) { *error = QStringLiteral("请选择不超过1MiB的UTF-8 .c源码文件。"); } return false; }
    bytes = file.read(MacroWorkspace::kMaximumEncodedSourceBytes + 1);
    if (file.error() != QFileDevice::NoError || !file.atEnd() || !MacroWorkspace::importSource(&m_draft, name, bytes, MacroWorkspace::BomPolicy::Allow, replaceExisting, error)) { return false; }
    m_report = QJsonObject(); rebuild(name); return true;
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::exportSourceFile
 * Description     : Export strict UTF-8 source with an atomic file replacement, never disguising it as native SM.
 * Input Parameters: path, name, error
 * Return Value    : True after complete independent .c publication.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroManagerDialog::exportSourceFile(const QString &path, const QString &name, QString *error) //20261114-7 add
{
    QSaveFile file(path);
    QByteArray bytes;

    if (!applySource(error)) { return false; }
    if (QFileInfo(path).suffix().compare(QStringLiteral("c"), Qt::CaseInsensitive) != 0)
    { if (error != nullptr) { *error = QStringLiteral("宏源码仅导出为UTF-8 .c文件。"); } return false; }
    if (!MacroWorkspace::exportSource(m_draft, name, false, &bytes, error)) { return false; }
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
    { if (error != nullptr) { *error = file.errorString(); } return false; }
    return true;
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::insertLibraryExample
 * Description     : Insert a verbatim native fragment at the current cursor as one local undo operation.
 * Input Parameters: name, error
 * Return Value    : True after a bounded insertion; no compiler or hardware execution occurs.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroManagerDialog::insertLibraryExample(const QString &name, QString *error) //20261114-7 add
{
    QString fragment;
    QString candidate;
    QByteArray encoded;
    QTextCursor cursor = m_source->textCursor();

    if (compiling() || m_currentName.isEmpty())
    { if (error != nullptr) { *error = QStringLiteral("请先选择可编辑的普通宏。"); } return false; }
    for (const QJsonValue &value : macroLibraryExamples())
    {
        if (value.toObject().value(QStringLiteral("name")).toString() == name)
        { fragment = value.toObject().value(QStringLiteral("text")).toString(); break; }
    }
    if (fragment.isEmpty()) { if (error != nullptr) { *error = QStringLiteral("函数库例子不存在。"); } return false; }
    candidate = m_source->toPlainText();
    candidate.replace(cursor.selectionStart(), cursor.selectionEnd() - cursor.selectionStart(), fragment);
    if (!MacroWorkspace::encodeUtf8Source(candidate, false, &encoded, error)) { return false; }
    cursor.beginEditBlock();
    cursor.insertText(fragment); // Preserve native newlines and spacing; do not repair or run the example.
    cursor.endEditBlock();
    m_source->setTextCursor(cursor);
    return true;
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::compiling
 * Description     : Read whether an owned SDK operation is still in progress.
 * Input Parameters: None
 * Return Value    : True while the worker result has not been delivered.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroManagerDialog::compiling() const //20261114-7 add
{
    return m_thread != nullptr;
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::compilationReport
 * Description     : Return the latest source-bound compile or failure evidence.
 * Input Parameters: None
 * Return Value    : Structured report, empty after source edits.
 * Date            : 2026.11.14
 ******************************************************************************/
QJsonObject MacroManagerDialog::compilationReport() const //20261114-7 add
{
    return m_report;
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::setCompiling
 * Description     : Freeze editable inputs until the worker publishes its result or completes cancellation.
 * Input Parameters: running
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroManagerDialog::setCompiling(bool running) //20261114-7 add
{
    for (QWidget *widget : m_editingWidgets) { widget->setEnabled(!running); }
    m_compile->setEnabled(!running);
    m_cancelCompile->setEnabled(running);
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::startCompilation
 * Description     : Run a frozen source set on a value-owned thread while preserving a responsive cancel action.
 * Input Parameters: sdk, options
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroManagerDialog::startCompilation(const QString &sdk, const DeviceMacroCompileOptions &options) //20261114-7 add
{
    QString error;
    QString selectedSdk;
    HmiProject snapshot;
    DeviceMacroCompileOptions selectedOptions;
    std::shared_ptr<MacroCompileJob> job;
    std::function<bool()> callerCancellation;

    if (compiling() || !applySource(&error)) { showError(error); return; }
    snapshot = m_draft;
    selectedSdk = sdk.isEmpty() ? deviceMacroSdkDirectory() : sdk;
    selectedOptions = options;
    if (selectedOptions.temporaryRoot.isEmpty())
    { selectedOptions.temporaryRoot = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("macro-temp")); }
    if (!QDir().mkpath(selectedOptions.temporaryRoot)) { showError(QStringLiteral("无法创建宏临时目录，请检查程序目录写入权限。")); return; }
    job = std::make_shared<MacroCompileJob>();
    callerCancellation = selectedOptions.isCancellationRequested;
    selectedOptions.isCancellationRequested = [job, callerCancellation]() {
        return job->cancelled.load() || (callerCancellation && callerCancellation());
    };
    m_report = QJsonObject();
    m_job = job;
    m_thread = QThread::create([snapshot, selectedSdk, selectedOptions, job]() {
        job->ok = compileDeviceMacros(snapshot, selectedSdk, &job->package, &job->error, selectedOptions, &job->report);
    });
    connect(m_thread, &QThread::finished, this, [this, job]() {
        m_thread = nullptr;
        m_job.reset();
        m_report = job->report;
        setCompiling(false);
        m_output->setPlainText(job->ok ? QStringLiteral("编译成功；尚未下载或运行。\n") + job->package.diagnostics.join(QLatin1Char('\n')) : job->error);
        showError(QString());
        if (m_closeAfterCompile == CloseAfterCompile::Discard) { QDialog::reject(); }
        else if (m_closeAfterCompile == CloseAfterCompile::AskToSave) //20261127-1 add
        {
            m_closeAfterCompile = CloseAfterCompile::None;
            close();
        }
    });
    connect(m_thread, &QThread::finished, m_thread, &QObject::deleteLater);
    setCompiling(true);
    m_output->setPlainText(QStringLiteral("正在编译普通宏..."));
    m_thread->start();
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::cancelCompilation
 * Description     : Set a shared cancellation flag observed only by the owned compiler job.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroManagerDialog::cancelCompilation() //20261114-7 add
{
    if (m_job) { m_job->cancelled.store(true); }
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::accept
 * Description     : Publish a validated source draft only after compilation has stopped.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroManagerDialog::accept() //20261114-7 add
{
    QString error;

    if (!applySource(&error)) { showError(error); return; }
    QDialog::accept();
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::reject
 * Description     : Discard the workspace after any live owned SDK process completes cancellation.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroManagerDialog::reject() //20261114-7 add
{
    if (compiling()) { m_closeAfterCompile = CloseAfterCompile::Discard; cancelCompilation(); showError(QStringLiteral("正在停止编译，请稍候...")); return; } //20261127-1 change
    QDialog::reject();
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::closeEvent
 * Description     : Protect all edited sources on title-bar close or Alt+F4 and keep rejected saves editable.
 * Input Parameters: event
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroManagerDialog::closeEvent(QCloseEvent *event) //20261127-1 change
{
    QPointer<MacroManagerDialog> lifetime(this);
    QPointer<QMessageBox> prompt;
    QStringList names = m_originalMacros.keys();
    QStringList changed;
    QString currentText = m_source->toPlainText();
    QString error;
    int choice;

    event->ignore();
    if (compiling())
    {
        m_closeAfterCompile = CloseAfterCompile::AskToSave;
        cancelCompilation();
        showError(QStringLiteral("正在停止编译，请稍候..."));
        return;
    }
    names.append(m_draft.macros.keys());
    names.removeDuplicates();
    for (const QString &name : names)
    {
        // Ignore IDs allocated solely to normalize legacy projects. Count actual rename, removal or replacement.
        if (!m_originalMacros.contains(name) || !m_draft.macros.contains(name) ||
            (name == m_currentName ? currentText : m_draft.macros.value(name)) != m_originalMacros.value(name) ||
            (m_originalIds.contains(name) && m_originalIds.value(name) != m_draft.macroIds.value(name)))
        {
            changed.append(name);
        }
    }
    if (!changed.isEmpty())
    {
        prompt = new QMessageBox(QMessageBox::Information, QStringLiteral("未保存"),
            changed.size() == 1 ? QStringLiteral("是否保存代码到宏：%1").arg(changed.first()) :
                QStringLiteral("是否保存以下宏指令的更改？\n%1").arg(changed.join(QStringLiteral("、"))),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this);
        prompt->setTextFormat(Qt::PlainText);
        prompt->button(QMessageBox::Save)->setText(dialogButtonText(QStringLiteral("保存"), "Save"));
        prompt->button(QMessageBox::Discard)->setText(dialogButtonText(QStringLiteral("不保存"), "Don't Save"));
        prompt->button(QMessageBox::Cancel)->setText(dialogButtonText(QStringLiteral("取消"), "Cancel"));
        prompt->setDefaultButton(QMessageBox::Save);
        prompt->setEscapeButton(QMessageBox::Cancel);
        choice = prompt->exec();
        delete prompt.data();
        if (lifetime.isNull() || (choice != QMessageBox::Save && choice != QMessageBox::Discard)) { return; }
        if (choice == QMessageBox::Save)
        {
            if (!applySource(&error)) { showError(error); return; }
            event->accept();
            QDialog::accept();
            return;
        }
    }
    event->accept();
    QDialog::reject();
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::promptNew
 * Description     : Collect a nonconflicting ordinary macro name before adding the default source.
 * Input Parameters: promptParent, the tree window when starting a new editor.
 * Return Value    : True when the name and source are present in the detached workspace.
 * Date            : 2026.11.27
 ******************************************************************************/
bool MacroManagerDialog::promptNew(QWidget *promptParent) //20261127-1 change
{
    QPointer<MacroManagerDialog> lifetime(this);
    QPointer<QDialog> input = new QDialog(promptParent != nullptr ? promptParent : this);
    QLabel *caption = new QLabel(QStringLiteral("请输入宏指令名称:"), input);
    QLineEdit *name = new QLineEdit(input);
    QLabel *validation = new QLabel(input);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, input);
    int result;

    input->setObjectName(QStringLiteral("MacroNameDialog"));
    input->setWindowTitle(QStringLiteral("添加宏"));
    input->setFixedSize(379, 72);
    caption->setGeometry(10, 10, 108, 20);
    name->setGeometry(120, 10, 168, 20);
    name->setFocus();
    buttons->setGeometry(207, 36, 161, 26);
    buttons->button(QDialogButtonBox::Ok)->setText(dialogButtonText(QStringLiteral("确定"), "OK"));
    buttons->button(QDialogButtonBox::Cancel)->setText(dialogButtonText(QStringLiteral("取消"), "Cancel"));
    validation->setGeometry(10, 33, 198, 37);
    validation->setWordWrap(true);
    connect(buttons, &QDialogButtonBox::rejected, input, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, [this, input, name, validation]() {
        QString error;

        if (createSource(name->text(), MacroWorkspace::defaultSource(), &error))
        {
            input->accept();
            return;
        }
        validation->setText(error);
        name->setFocus();
        name->selectAll();
    });
    result = input->exec();
    delete input.data();
    return !lifetime.isNull() && result == QDialog::Accepted;
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::promptCopy
 * Description     : Copy a complete macro with a new stable identity rather than sharing its source record.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroManagerDialog::promptCopy() //20261114-7 add
{
    QString name;
    QString error;
    bool ok;
    QPointer<MacroManagerDialog> lifetime(this);

    if (m_currentName.isEmpty() || !applySource(&error)) { showError(error); return; }
    name = QInputDialog::getText(this, QStringLiteral("复制宏"), QStringLiteral("新宏名称"), QLineEdit::Normal, MacroWorkspace::nextCopyName(m_draft, m_currentName), &ok);
    if (lifetime.isNull()) { return; }
    if (ok && MacroWorkspace::copy(&m_draft, m_currentName, name, &error)) { m_report = QJsonObject(); rebuild(name); }
    else if (ok) { showError(error); }
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::promptRename
 * Description     : Request one new name while keeping conflicting and referenced source drafts intact.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroManagerDialog::promptRename() //20261114-7 add
{
    QString name;
    QString error;
    bool ok;
    QPointer<MacroManagerDialog> lifetime(this);

    if (m_currentName.isEmpty()) { return; }
    name = QInputDialog::getText(this, QStringLiteral("重命名宏"), QStringLiteral("新宏名称"), QLineEdit::Normal, m_currentName, &ok);
    if (lifetime.isNull()) { return; }
    if (ok && !renameSource(m_currentName, name, &error)) { showError(error); }
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::promptDelete
 * Description     : Remove only an unreferenced source through the shared transaction guard.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroManagerDialog::promptDelete() //20261114-7 add
{
    QString error;

    if (!m_currentName.isEmpty() && !removeSource(m_currentName, &error)) { showError(error); }
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::promptImport
 * Description     : Import a named UTF-8 file as a new macro without silently replacing a same-named source.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroManagerDialog::promptImport() //20261114-7 add
{
    QString path;
    QString name;
    QString error;
    bool ok;
    QPointer<MacroManagerDialog> lifetime(this);

    path = QFileDialog::getOpenFileName(this, QStringLiteral("导入UTF-8宏源码"), QString(), QStringLiteral("C源码 (*.c)"));
    if (lifetime.isNull()) { return; }
    if (path.isEmpty()) { return; }
    name = QInputDialog::getText(this, QStringLiteral("导入宏"), QStringLiteral("宏名称"), QLineEdit::Normal, QFileInfo(path).completeBaseName(), &ok);
    if (lifetime.isNull()) { return; }
    if (ok && !importSourceFile(path, name, false, &error)) { showError(error); }
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::promptExport
 * Description     : Save the selected source through an explicit C-source file filter.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroManagerDialog::promptExport() //20261114-7 add
{
    QString path;
    QString error;
    QPointer<MacroManagerDialog> lifetime(this);

    if (m_currentName.isEmpty()) { return; }
    path = QFileDialog::getSaveFileName(this, QStringLiteral("导出UTF-8宏源码"), m_currentName + QStringLiteral(".c"), QStringLiteral("C源码 (*.c)"));
    if (lifetime.isNull()) { return; }
    if (!path.isEmpty() && !exportSourceFile(path, m_currentName, &error)) { showError(error); }
}

/******************************************************************************
 * Function Name   : MacroManagerDialog::findSource
 * Description     : Find or replace a match in the selected macro with the native case, word, direction and regex options.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroManagerDialog::findSource() //20261127-1 change
{
    QPointer<QDialog> dialog = new QDialog(this);
    QLabel *findLabel = new QLabel(QStringLiteral("查找内容:"), dialog);
    QLabel *replaceLabel = new QLabel(QStringLiteral("替换为:"), dialog);
    QLineEdit *needle = new QLineEdit(dialog);
    QLineEdit *replacement = new QLineEdit(dialog);
    QPushButton *find = new QPushButton(QStringLiteral("查找(&F)"), dialog);
    QPushButton *replace = new QPushButton(QStringLiteral("替换(&R)"), dialog);
    QPushButton *close = new QPushButton(QStringLiteral("关闭(&C)"), dialog);
    QCheckBox *matchCase = new QCheckBox(QStringLiteral("匹配大小写(&M)"), dialog);
    QCheckBox *whole = new QCheckBox(QStringLiteral("整字匹配(&O)"), dialog);
    QCheckBox *back = new QCheckBox(QStringLiteral("向前查找(&S)"), dialog);
    QCheckBox *regex = new QCheckBox(QStringLiteral("正则表达式(&E)"), dialog);
    int previousEmpty = -1;

    dialog->setObjectName(QStringLiteral("MacroFindDialog"));
    dialog->setWindowTitle(QStringLiteral("查找(&F)"));
    dialog->setFixedSize(600, 105);
    findLabel->setGeometry(9, 10, 59, 20);
    replaceLabel->setGeometry(9, 37, 49, 20);
    needle->setGeometry(69, 10, 441, 20);
    replacement->setGeometry(57, 37, 453, 20);
    find->setGeometry(517, 10, 73, 22);
    replace->setGeometry(517, 38, 73, 22);
    close->setGeometry(517, 66, 73, 22);
    matchCase->setGeometry(10, 61, 135, 20);
    back->setGeometry(264, 61, 135, 20);
    whole->setGeometry(10, 83, 135, 20);
    regex->setGeometry(264, 83, 135, 20);
    needle->setFocus();
    find->setDefault(true);
    find->setEnabled(false);
    connect(needle, &QLineEdit::textChanged, dialog, [&](const QString &text) {
        previousEmpty = -1;
        find->setEnabled(!text.isEmpty());
    });
    connect(close, &QPushButton::clicked, dialog, &QDialog::accept);
    // Both commands use the same document matcher, so Replace cannot change a stale selection.
    const auto search = [&](bool replaceMatch) {
        QTextDocument::FindFlags flags;
        QRegularExpression pattern;
        QTextCursor current = m_source->textCursor();
        QTextCursor found;
        int from;

        if (needle->text().isEmpty() || m_currentName.isEmpty() || compiling()) { return; }
        pattern.setPattern(regex->isChecked() ? needle->text() : QRegularExpression::escape(needle->text()));
        if (!pattern.isValid())
        {
            showError(pattern.errorString());
            return;
        }
        if (matchCase->isChecked()) { flags |= QTextDocument::FindCaseSensitively; }
        if (whole->isChecked()) { flags |= QTextDocument::FindWholeWords; }
        if (replaceMatch && current.hasSelection())
        {
            found = m_source->document()->find(pattern, current.selectionStart(), flags);
            if (found.selectionStart() != current.selectionStart() || found.selectionEnd() != current.selectionEnd())
            {
                found = QTextCursor();
            }
        }
        if (found.isNull())
        {
            from = back->isChecked() ? current.selectionStart() : current.selectionEnd();
            if (back->isChecked()) { flags |= QTextDocument::FindBackward; }
            if (from == previousEmpty) { from += back->isChecked() ? -1 : 1; }
            if (from >= 0 && from < m_source->document()->characterCount())
            {
                found = m_source->document()->find(pattern, from, flags);
            }
            if (found.isNull())
            {
                // Preserve the editor's wrap-around behavior at either end of a script.
                from = back->isChecked() ? m_source->document()->characterCount() - 1 : 0;
                found = m_source->document()->find(pattern, from, flags);
            }
        }
        if (found.isNull())
        {
            showError(QStringLiteral("未找到该内容。"));
            return;
        }
        previousEmpty = found.hasSelection() ? -1 : found.position();
        if (replaceMatch)
        {
            from = found.selectionStart();
            found.insertText(replacement->text());
            if (back->isChecked()) { found.setPosition(from); }
        }
        m_source->setTextCursor(found);
        m_source->ensureCursorVisible();
        showError(QString());
    };
    connect(find, &QPushButton::clicked, dialog, [&]() { search(false); });
    connect(replace, &QPushButton::clicked, dialog, [&]() { search(true); });
    dialog->exec();
    delete dialog.data();
}
