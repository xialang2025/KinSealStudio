#include "stage2_project_dialogs.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTabWidget>
#include <QUuid>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : stage2Error
 * Description     : Return a localized transaction error without opening another modal dialog.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.10
 ******************************************************************************/
static bool stage2Error(QString *error, const QString &message) //20261110-7 add
{
    if (error != nullptr) { *error = message; }
    return false;
}

/******************************************************************************
 * Function Name   : addLanguageChoices
 * Description     : Keep inherited startup language distinct from an explicit language selection.
 * Input Parameters: combo, project, inherited
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
static void addLanguageChoices(QComboBox *combo, const HmiProject &project, bool inherited) //20261110-7 add
{
    QString language;

    if (inherited) { combo->addItem(QStringLiteral("沿用工程启动语言"), QString()); }
    Q_FOREACH (language, project.languages) { combo->addItem(language, language); }
}

/******************************************************************************
 * Function Name   : Stage2NewProjectDialog::Stage2NewProjectDialog
 * Description     : Build the observed three-step workflow without creating a directory or file.
 * Input Parameters: seed, suggestedPath, parent
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
Stage2NewProjectDialog::Stage2NewProjectDialog(const HmiProject &seed, const QString &suggestedPath,
                                             QWidget *parent) //20261110-7 add
    : QDialog(parent), m_working(seed)
{
    QVBoxLayout *layout;
    QHBoxLayout *navigation;
    QFormLayout *identityForm;
    QFormLayout *pageForm;
    QFormLayout *connectionForm;
    QWidget *identityStep;
    QWidget *connectionStep;
    QWidget *pageStep;
    QLineEdit *target;
    QLineEdit *manufacturer;
    QLineEdit *series;
    QLineEdit *connectionName;
    QRadioButton *logo;
    QRadioButton *noLogo;
    QLabel *logoBoundary;
    QSpinBox *logoTime;
    QPushButton *cancel;

    setObjectName(QStringLiteral("stage2NewProjectDialog"));
    setWindowTitle(QStringLiteral("新项目"));
    resize(590, 330);
    layout = new QVBoxLayout(this);
    m_steps = new QStackedWidget(this);
    m_steps->setObjectName(QStringLiteral("wizardSteps"));
    identityStep = new QWidget(m_steps);
    identityForm = new QFormLayout(identityStep);
    m_name = new QLineEdit(identityStep);
    m_name->setObjectName(QStringLiteral("projectNameEdit"));
    m_name->setMaxLength(32767);
    m_path = new QLineEdit(suggestedPath, identityStep);
    m_path->setObjectName(QStringLiteral("projectPathEdit"));
    target = new QLineEdit(QStringLiteral("AMT070W124-L800X480R"), identityStep);
    target->setObjectName(QStringLiteral("targetModelEdit"));
    target->setReadOnly(true);
    identityForm->addRow(QStringLiteral("工程名称"), m_name);
    identityForm->addRow(QStringLiteral("工程文件（.ksp）"), m_path);
    identityForm->addRow(QStringLiteral("目标型号"), target);
    identityForm->addRow(QStringLiteral("分辨率"), new QLabel(QStringLiteral("800 × 480"), identityStep));
    identityForm->addRow(QStringLiteral("显示方向"), new QLabel(QStringLiteral("水平"), identityStep));
    connectionStep = new QWidget(m_steps);
    connectionForm = new QFormLayout(connectionStep);
    manufacturer = new QLineEdit(QStringLiteral("MODBUS"), connectionStep);
    manufacturer->setObjectName(QStringLiteral("connectionManufacturerEdit"));
    manufacturer->setReadOnly(true);
    series = new QLineEdit(QStringLiteral("Modbus_RTU"), connectionStep);
    series->setObjectName(QStringLiteral("connectionSeriesEdit"));
    series->setReadOnly(true);
    connectionName = new QLineEdit(QStringLiteral("Modbus_RTU"), connectionStep);
    connectionName->setObjectName(QStringLiteral("connectionNameEdit"));
    connectionName->setReadOnly(true);
    connectionForm->addRow(QStringLiteral("制造商"), manufacturer);
    connectionForm->addRow(QStringLiteral("系列"), series);
    connectionForm->addRow(QStringLiteral("名称"), connectionName);
    connectionForm->addRow(new QLabel(QStringLiteral("此向导使用已验证的默认通信配置；其他连接可在工程连接设置中编辑。"), connectionStep));
    pageStep = new QWidget(m_steps);
    pageForm = new QFormLayout(pageStep);
    logo = new QRadioButton(QStringLiteral("需要启动画面"), pageStep);
    logo->setObjectName(QStringLiteral("startupLogoRadio"));
    logo->setEnabled(false);
    logo->setToolTip(QStringLiteral("上电Logo与启动动画的非默认设备编码尚未恢复。"));
    noLogo = new QRadioButton(QStringLiteral("不需要启动画面"), pageStep);
    noLogo->setObjectName(QStringLiteral("noStartupLogoRadio"));
    noLogo->setEnabled(false);
    // Preserve the verified baseline behavior without inferring byte identity with the native wizard sample.
    noLogo->setChecked(true);
    logoTime = new QSpinBox(pageStep);
    logoTime->setObjectName(QStringLiteral("startupLogoTimeSpin"));
    logoTime->setRange(1, 60);
    logoTime->setValue(1);
    logoTime->setSuffix(QStringLiteral(" 秒"));
    logoTime->setEnabled(false);
    pageForm->addRow(logo);
    pageForm->addRow(noLogo);
    pageForm->addRow(QStringLiteral("单个启动画面显示时间"), logoTime);
    logoBoundary = new QLabel(QStringLiteral("沿用已验证基线的不显示开机Logo或动画设置；下面的普通启动画面与其独立。"), pageStep);
    logoBoundary->setObjectName(QStringLiteral("startupLogoBoundaryLabel"));
    logoBoundary->setWordWrap(true);
    pageForm->addRow(logoBoundary);
    m_pageName = new QLineEdit(seed.pageName, pageStep);
    m_pageName->setObjectName(QStringLiteral("pageNameEdit"));
    m_pageName->setMaxLength(qMax<int>(32767, seed.pageName.size()));
    m_startupLanguage = new QComboBox(pageStep);
    m_startupLanguage->setObjectName(QStringLiteral("startupLanguageCombo"));
    addLanguageChoices(m_startupLanguage, seed, true);
    m_startupLanguage->setCurrentIndex(m_startupLanguage->findData(seed.startupLanguage));
    pageForm->addRow(QStringLiteral("初始画面名称"), m_pageName);
    pageForm->addRow(QStringLiteral("设备启动语言"), m_startupLanguage);
    pageForm->addRow(new QLabel(QStringLiteral("完成后交由主窗口保存工程；取消会放弃本次草稿。"), pageStep));
    m_steps->addWidget(identityStep);
    m_steps->addWidget(connectionStep);
    m_steps->addWidget(pageStep);
    layout->addWidget(m_steps);
    m_error = new QLabel(this);
    m_error->setObjectName(QStringLiteral("validationErrorLabel"));
    m_error->setWordWrap(true);
    layout->addWidget(m_error);
    navigation = new QHBoxLayout;
    navigation->addStretch();
    m_back = new QPushButton(QStringLiteral("上一步"), this);
    m_back->setObjectName(QStringLiteral("wizardBackButton"));
    m_next = new QPushButton(QStringLiteral("下一步"), this);
    m_next->setObjectName(QStringLiteral("wizardNextButton"));
    m_finish = new QPushButton(QStringLiteral("完成"), this);
    m_finish->setObjectName(QStringLiteral("wizardFinishButton"));
    cancel = new QPushButton(QStringLiteral("取消"), this);
    cancel->setObjectName(QStringLiteral("wizardCancelButton"));
    navigation->addWidget(m_back);
    navigation->addWidget(m_next);
    navigation->addWidget(m_finish);
    navigation->addWidget(cancel);
    layout->addLayout(navigation);
    connect(m_back, &QPushButton::clicked, this, [this]() { updateStep(m_steps->currentIndex() - 1); });
    connect(m_next, &QPushButton::clicked, this, [this]()
    {
        HmiProject candidate;
        QString error;

        if (!collectDraft(&candidate, false, &error)) { m_error->setText(error); return; }
        updateStep(m_steps->currentIndex() + 1);
    });
    connect(m_finish, &QPushButton::clicked, this, &Stage2NewProjectDialog::accept);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    updateStep(0);
}

/******************************************************************************
 * Function Name   : Stage2NewProjectDialog::draftProject
 * Description     : Return the accepted value for publication by the caller.
 * Input Parameters: None
 * Return Value    : Detached project value; caller must check Accepted.
 * Date            : 2026.11.10
 ******************************************************************************/
HmiProject Stage2NewProjectDialog::draftProject() const //20261110-7 add
{
    return m_working;
}

/******************************************************************************
 * Function Name   : Stage2NewProjectDialog::projectPath
 * Description     : Return the normalized destination without writing it.
 * Input Parameters: None
 * Return Value    : Absolute file path.
 * Date            : 2026.11.10
 ******************************************************************************/
QString Stage2NewProjectDialog::projectPath() const //20261110-7 add
{
    if (result() != QDialog::Accepted) { return QString(); }
    return QDir::cleanPath(m_path->text().trimmed());
}

/******************************************************************************
 * Function Name   : Stage2NewProjectDialog::collectDraft
 * Description     : Validate project identity and a fresh writable destination before accepting a draft.
 * Input Parameters: candidate, includePage, error
 * Return Value    : True for a valid detached model and unused absolute .ksp destination.
 * Date            : 2026.11.10
 ******************************************************************************/
bool Stage2NewProjectDialog::collectDraft(HmiProject *candidate, bool includePage, QString *error) const //20261110-7 add
{
    QFileInfo destination;
    QFileInfo directory;
    QString path;

    if (candidate == nullptr) { return stage2Error(error, QStringLiteral("工程草稿输出无效。")); }
    if (m_name->text().trimmed().isEmpty()) { return stage2Error(error, QStringLiteral("请输入工程名称。")); }
    path = QDir::cleanPath(m_path->text().trimmed());
    destination = QFileInfo(path);
    directory = QFileInfo(destination.absolutePath());
    if (m_path->text().trimmed().isEmpty() || !destination.isAbsolute() ||
        destination.suffix().compare(QStringLiteral("ksp"), Qt::CaseInsensitive) != 0 ||
        destination.fileName() == QStringLiteral(".ksp"))
    {
        return stage2Error(error, QStringLiteral("请选择带文件名的绝对 .ksp 工程路径。"));
    }
    if (destination.exists()) { return stage2Error(error, QStringLiteral("新建工程路径已存在，请选择其他文件名。")); }
    if (!directory.exists() || !directory.isDir() || !directory.isWritable())
    {
        return stage2Error(error, QStringLiteral("工程所在目录不存在或不可写。"));
    }
    *candidate = m_working;
    candidate->name = m_name->text();
    if (includePage)
    {
        candidate->pageName = m_pageName->text();
        candidate->startupLanguage = m_startupLanguage->currentData().toString();
    }
    return candidate->validateModel(error);
}

/******************************************************************************
 * Function Name   : Stage2NewProjectDialog::updateStep
 * Description     : Change wizard visibility while preserving all steps' input widgets.
 * Input Parameters: step
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2NewProjectDialog::updateStep(int step) //20261110-7 add
{
    QStringList titles;

    titles = {QStringLiteral("新项目"), QStringLiteral("通信连接"), QStringLiteral("启动画面设置")};
    m_steps->setCurrentIndex(step);
    setWindowTitle(titles.value(step));
    m_back->setEnabled(step != 0);
    m_next->setVisible(step < m_steps->count() - 1);
    m_finish->setVisible(step == m_steps->count() - 1);
    m_finish->setDefault(step == m_steps->count() - 1);
    m_next->setDefault(step < m_steps->count() - 1);
    m_error->clear();
}

/******************************************************************************
 * Function Name   : Stage2NewProjectDialog::accept
 * Description     : Publish only a completely validated draft and leave all file I/O to its caller.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2NewProjectDialog::accept() //20261110-7 add
{
    HmiProject candidate;
    QString error;

    if (m_steps->currentIndex() < m_steps->count() - 1) { m_next->click(); return; }
    if (!collectDraft(&candidate, true, &error)) { m_error->setText(error); return; }
    m_working = candidate;
    QDialog::accept();
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::Stage2ProjectDialog
 * Description     : Edit project identity, page transactions and independent startup choices on a copy.
 * Input Parameters: project, parent
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
Stage2ProjectDialog::Stage2ProjectDialog(const HmiProject &project, QWidget *parent) //20261110-7 add
    : QDialog(parent), m_working(project)
{
    QVBoxLayout *layout;
    QVBoxLayout *pageLayout;
    QHBoxLayout *actions;
    QFormLayout *projectForm;
    QFormLayout *pageForm;
    QTabWidget *tabs;
    QWidget *projectTab;
    QWidget *pagesTab;
    QLineEdit *target;
    QPushButton *add;
    QPushButton *rename;
    QPushButton *copy;
    QPushButton *remove;
    QLabel *boundary;

    setObjectName(QStringLiteral("stage2ProjectDialog"));
    setWindowTitle(QStringLiteral("工程与画面设置"));
    resize(660, 560);
    layout = new QVBoxLayout(this);
    tabs = new QTabWidget(this);
    tabs->setObjectName(QStringLiteral("projectSettingsTabs"));
    projectTab = new QWidget(tabs);
    projectForm = new QFormLayout(projectTab);
    m_projectName = new QLineEdit(project.name, projectTab);
    m_projectName->setObjectName(QStringLiteral("projectNameEdit"));
    m_projectName->setMaxLength(qMax<int>(32767, project.name.size()));
    target = new QLineEdit(project.targetModel, projectTab);
    target->setObjectName(QStringLiteral("targetModelEdit"));
    target->setReadOnly(true);
    m_editorPage = new QComboBox(projectTab);
    m_editorPage->setObjectName(QStringLiteral("editorPageCombo"));
    m_startupPage = new QComboBox(projectTab);
    m_startupPage->setObjectName(QStringLiteral("startupPageCombo"));
    m_editorLanguage = new QComboBox(projectTab);
    m_editorLanguage->setObjectName(QStringLiteral("editorLanguageCombo"));
    m_startupLanguage = new QComboBox(projectTab);
    m_startupLanguage->setObjectName(QStringLiteral("startupLanguageCombo"));
    addLanguageChoices(m_editorLanguage, project, false);
    addLanguageChoices(m_startupLanguage, project, true);
    projectForm->addRow(QStringLiteral("工程名称"), m_projectName);
    projectForm->addRow(QStringLiteral("目标型号"), target);
    projectForm->addRow(QStringLiteral("当前编辑画面"), m_editorPage);
    projectForm->addRow(QStringLiteral("设备启动画面"), m_startupPage);
    projectForm->addRow(QStringLiteral("当前编辑语言"), m_editorLanguage);
    projectForm->addRow(QStringLiteral("设备启动语言"), m_startupLanguage);
    pagesTab = new QWidget(tabs);
    pageLayout = new QVBoxLayout(pagesTab);
    m_pages = new QListWidget(pagesTab);
    m_pages->setObjectName(QStringLiteral("pageList"));
    pageLayout->addWidget(m_pages);
    pageForm = new QFormLayout;
    m_pageName = new QLineEdit(pagesTab);
    m_pageName->setObjectName(QStringLiteral("pageNameEdit"));
    m_pageName->setMaxLength(32767);
    m_pageKind = new QComboBox(pagesTab);
    m_pageKind->setObjectName(QStringLiteral("pageKindCombo"));
    m_pageKind->addItem(QStringLiteral("普通画面"), QStringLiteral("normal"));
    m_pageKind->addItem(QStringLiteral("窗口"), QStringLiteral("window"));
    m_width = new QSpinBox(pagesTab);
    m_width->setObjectName(QStringLiteral("pageWidthSpin"));
    m_width->setRange(1, 800);
    m_width->setValue(800);
    m_height = new QSpinBox(pagesTab);
    m_height->setObjectName(QStringLiteral("pageHeightSpin"));
    m_height->setRange(1, 480);
    m_height->setValue(480);
    pageForm->addRow(QStringLiteral("名称（新增／复制时填写新名称）"), m_pageName);
    pageForm->addRow(QStringLiteral("新增类型"), m_pageKind);
    pageForm->addRow(QStringLiteral("新增窗口宽度"), m_width);
    pageForm->addRow(QStringLiteral("新增窗口高度"), m_height);
    pageLayout->addLayout(pageForm);
    actions = new QHBoxLayout;
    add = new QPushButton(QStringLiteral("新增"), pagesTab);
    add->setObjectName(QStringLiteral("addPageButton"));
    rename = new QPushButton(QStringLiteral("改名"), pagesTab);
    rename->setObjectName(QStringLiteral("renamePageButton"));
    copy = new QPushButton(QStringLiteral("复制"), pagesTab);
    copy->setObjectName(QStringLiteral("copyPageButton"));
    remove = new QPushButton(QStringLiteral("删除"), pagesTab);
    remove->setObjectName(QStringLiteral("removePageButton"));
    actions->addWidget(add);
    actions->addWidget(rename);
    actions->addWidget(copy);
    actions->addWidget(remove);
    pageLayout->addLayout(actions);
    tabs->addTab(projectTab, QStringLiteral("工程设置"));
    tabs->addTab(pagesTab, QStringLiteral("画面管理"));
    layout->addWidget(tabs);
    boundary = new QLabel(QStringLiteral("多画面和窗口可以保存、重开；当前设备下载仅支持单个800×480普通画面。"), this);
    boundary->setObjectName(QStringLiteral("backendBoundaryLabel"));
    boundary->setWordWrap(true);
    layout->addWidget(boundary);
    m_error = new QLabel(this);
    m_error->setObjectName(QStringLiteral("validationErrorLabel"));
    m_error->setWordWrap(true);
    layout->addWidget(m_error);
    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_buttons->setObjectName(QStringLiteral("buttonBox"));
    layout->addWidget(m_buttons);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &Stage2ProjectDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_pages, &QListWidget::currentRowChanged, this, [this](int) { showSelectedPage(); });
    connect(add, &QPushButton::clicked, this, &Stage2ProjectDialog::addFromFields);
    connect(rename, &QPushButton::clicked, this, &Stage2ProjectDialog::renameFromFields);
    connect(copy, &QPushButton::clicked, this, &Stage2ProjectDialog::copyFromFields);
    connect(remove, &QPushButton::clicked, this, &Stage2ProjectDialog::removeSelected);
    connect(m_editorPage, &QComboBox::currentIndexChanged, this, [this](int)
    {
        QString error;

        if (!selectPageDraft(m_editorPage->currentData().toString(), &error)) { reportError(error); }
    });
    connect(m_startupPage, &QComboBox::currentIndexChanged, this, [this](int)
    {
        QString error;

        if (!setStartupDraft(m_startupPage->currentData().toString(), m_working.startupLanguage, &error)) { reportError(error); }
    });
    connect(m_editorLanguage, &QComboBox::currentIndexChanged, this, [this](int)
    {
        QString error;

        if (!setEditorLanguageDraft(m_editorLanguage->currentData().toString(), &error)) { reportError(error); }
    });
    connect(m_startupLanguage, &QComboBox::currentIndexChanged, this, [this](int)
    {
        QString error;

        if (!setStartupDraft(m_working.startupPageId, m_startupLanguage->currentData().toString(), &error)) { reportError(error); }
    });
    connect(m_pageKind, &QComboBox::currentIndexChanged, this, [this](int)
    {
        bool window;

        window = m_pageKind->currentData().toString() == QStringLiteral("window");
        if (window) { m_width->setValue(240); m_height->setValue(180); }
        m_width->setEnabled(window);
        m_height->setEnabled(window);
    });
    refresh(project.activePageId);
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::draftProject
 * Description     : Read the detached transaction without touching the caller's active model.
 * Input Parameters: None
 * Return Value    : Project draft; publish only after Accepted.
 * Date            : 2026.11.10
 ******************************************************************************/
HmiProject Stage2ProjectDialog::draftProject() const //20261110-7 add
{
    return m_working;
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::validatePageName
 * Description     : Enforce an explicit editor naming policy without changing stable identities.
 * Input Parameters: id, name, error
 * Return Value    : True for a nonempty unique display name within the editor's bounded input.
 * Date            : 2026.11.10
 ******************************************************************************/
bool Stage2ProjectDialog::validatePageName(const QString &id, const QString &name, QString *error) const //20261110-7 add
{
    HmiPage page;

    if (name.trimmed().isEmpty() || name.size() > 32767)
    {
        return stage2Error(error, QStringLiteral("画面名称不能为空或超过32767字符。"));
    }
    Q_FOREACH (page, m_working.allPages())
    {
        if (page.id != id && page.name == name) { return stage2Error(error, QStringLiteral("名字已经存在，请重新输入。")); }
    }
    return true;
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::addPageDraft
 * Description     : Append only supported editor page roles through the model's atomic transaction.
 * Input Parameters: page, error
 * Return Value    : True on valid addition; keyboard and Logo creation await native field evidence.
 * Date            : 2026.11.10
 ******************************************************************************/
bool Stage2ProjectDialog::addPageDraft(const HmiPage &page, QString *error) //20261110-7 add
{
    if (!validatePageName(QString(), page.name, error)) { return false; }
    if (page.kind != QStringLiteral("normal") && page.kind != QStringLiteral("window"))
    {
        return stage2Error(error, QStringLiteral("此入口仅创建普通画面或窗口；键盘和Logo字段尚未核定。"));
    }
    if (page.kind == QStringLiteral("normal") && (page.width != 800 || page.height != 480))
    {
        return stage2Error(error, QStringLiteral("当前AMT普通画面尺寸必须为800×480。"));
    }
    if (!m_working.addPage(page, error)) { return false; }
    refresh(page.id);
    return true;
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::renamePageDraft
 * Description     : Rename the selected entity while preserving all inbound reference targets.
 * Input Parameters: id, name, error
 * Return Value    : True on validated rename.
 * Date            : 2026.11.10
 ******************************************************************************/
bool Stage2ProjectDialog::renamePageDraft(const QString &id, const QString &name, QString *error) //20261110-7 add
{
    if (!validatePageName(id, name, error) || !m_working.renamePage(id, name, error)) { return false; }
    refresh(id);
    return true;
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::copyPageDraft
 * Description     : Delegate every cloned page, object and owned-reference identity remap to the model.
 * Input Parameters: id, name, newId, error
 * Return Value    : True after an independent clone validates.
 * Date            : 2026.11.10
 ******************************************************************************/
bool Stage2ProjectDialog::copyPageDraft(const QString &id, const QString &name, QString *newId, QString *error) //20261110-7 add
{
    QString copiedId;

    if (!validatePageName(QString(), name, error) || !m_working.copyPage(id, name, &copiedId, error)) { return false; }
    if (newId != nullptr) { *newId = copiedId; }
    refresh(copiedId);
    return true;
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::removePageDraft
 * Description     : Refuse startup and inbound-reference deletion before changing the draft list.
 * Input Parameters: id, error
 * Return Value    : True after atomic removal.
 * Date            : 2026.11.10
 ******************************************************************************/
bool Stage2ProjectDialog::removePageDraft(const QString &id, QString *error) //20261110-7 add
{
    if (!m_working.removePage(id, error)) { return false; }
    refresh(m_working.activePageId);
    return true;
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::selectPageDraft
 * Description     : Change the editor's active storage independently from device startup selection.
 * Input Parameters: id, error
 * Return Value    : True after model selection validates.
 * Date            : 2026.11.10
 ******************************************************************************/
bool Stage2ProjectDialog::selectPageDraft(const QString &id, QString *error) //20261110-7 add
{
    if (!m_working.selectPage(id, error)) { return false; }
    refresh(id);
    return true;
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::setStartupDraft
 * Description     : Validate startup page role and language atomically without changing the editor page.
 * Input Parameters: id, language, error
 * Return Value    : True after both startup fields validate.
 * Date            : 2026.11.10
 ******************************************************************************/
bool Stage2ProjectDialog::setStartupDraft(const QString &id, const QString &language, QString *error) //20261110-7 add
{
    HmiProject candidate;

    candidate = m_working;
    candidate.startupPageId = id;
    candidate.startupLanguage = language;
    if (!candidate.validateModel(error)) { return false; }
    m_working = candidate;
    refresh(selectedPageId());
    return true;
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::setEditorLanguageDraft
 * Description     : Change current editing language without silently changing the device startup choice.
 * Input Parameters: language, error
 * Return Value    : True after a registered language is selected.
 * Date            : 2026.11.10
 ******************************************************************************/
bool Stage2ProjectDialog::setEditorLanguageDraft(const QString &language, QString *error) //20261110-7 add
{
    HmiProject candidate;

    candidate = m_working;
    candidate.activeLanguage = language;
    if (!candidate.validateModel(error)) { return false; }
    m_working = candidate;
    refresh(selectedPageId());
    return true;
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::refresh
 * Description     : Rebuild labels from stable IDs and block accidental combo/list selection transactions.
 * Input Parameters: selectedId
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialog::refresh(const QString &selectedId) //20261110-7 add
{
    QSignalBlocker listBlocker(m_pages);
    QSignalBlocker editorBlocker(m_editorPage);
    QSignalBlocker startupBlocker(m_startupPage);
    QSignalBlocker editorLanguageBlocker(m_editorLanguage);
    QSignalBlocker startupLanguageBlocker(m_startupLanguage);
    HmiPage page;
    QListWidgetItem *item;
    int row;

    m_pages->clear();
    m_editorPage->clear();
    m_startupPage->clear();
    row = 0;
    Q_FOREACH (page, m_working.allPages())
    {
        item = new QListWidgetItem(QStringLiteral("%1  [%2]  %3×%4").arg(page.name, page.kind).arg(page.width).arg(page.height), m_pages);
        item->setData(Qt::UserRole, page.id);
        m_editorPage->addItem(page.name, page.id);
        if (page.kind == QStringLiteral("normal")) { m_startupPage->addItem(page.name, page.id); }
        if (page.id == selectedId) { m_pages->setCurrentRow(row); }
        ++row;
    }
    if (m_pages->currentRow() < 0 && m_pages->count() != 0) { m_pages->setCurrentRow(0); }
    m_editorPage->setCurrentIndex(m_editorPage->findData(m_working.activePageId));
    m_startupPage->setCurrentIndex(m_startupPage->findData(m_working.startupPageId));
    m_editorLanguage->setCurrentIndex(m_editorLanguage->findData(m_working.activeLanguage));
    m_startupLanguage->setCurrentIndex(m_startupLanguage->findData(m_working.startupLanguage));
    m_error->clear();
    showSelectedPage();
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::selectedPageId
 * Description     : Read stable identity instead of treating a visual row number as a device page ID.
 * Input Parameters: None
 * Return Value    : Selected stable page ID or empty.
 * Date            : 2026.11.10
 ******************************************************************************/
QString Stage2ProjectDialog::selectedPageId() const //20261110-7 add
{
    if (m_pages->currentItem() == nullptr) { return QString(); }
    return m_pages->currentItem()->data(Qt::UserRole).toString();
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::showSelectedPage
 * Description     : Populate page fields while keeping unavailable native roles preserved in the draft.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialog::showSelectedPage() //20261110-7 add
{
    HmiPage page;
    QString id;
    bool window;

    id = selectedPageId();
    Q_FOREACH (page, m_working.allPages())
    {
        if (page.id != id) { continue; }
        m_pageName->setMaxLength(qMax<int>(32767, page.name.size()));
        m_pageName->setText(page.name);
        m_pageKind->setCurrentIndex(m_pageKind->findData(page.kind));
        m_width->setValue(page.width);
        m_height->setValue(page.height);
        window = page.kind == QStringLiteral("window");
        m_width->setEnabled(window);
        m_height->setEnabled(window);
        return;
    }
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::addFromFields
 * Description     : Generate an editor identity and add the fully validated page fields to the draft.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialog::addFromFields() //20261110-7 add
{
    HmiPage page;
    QString error;

    page.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    page.kind = m_pageKind->currentData().toString();
    page.name = m_pageName->text();
    page.width = page.kind == QStringLiteral("window") ? m_width->value() : 800;
    page.height = page.kind == QStringLiteral("window") ? m_height->value() : 480;
    if (!addPageDraft(page, &error)) { reportError(error); }
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::renameFromFields
 * Description     : Apply the name field only, never reinterpret geometry or role as an unverified edit.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialog::renameFromFields() //20261110-7 add
{
    QString error;

    if (!renamePageDraft(selectedPageId(), m_pageName->text(), &error)) { reportError(error); }
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::copyFromFields
 * Description     : Clone the selected entity with the supplied fresh name through the model transaction.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialog::copyFromFields() //20261110-7 add
{
    QString error;

    if (!copyPageDraft(selectedPageId(), m_pageName->text(), nullptr, &error)) { reportError(error); }
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::removeSelected
 * Description     : Remove a page inside the outer cancelable draft after reference validation.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialog::removeSelected() //20261110-7 add
{
    QString error;

    if (!removePageDraft(selectedPageId(), &error)) { reportError(error); }
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::reportError
 * Description     : Keep the edit dialog open and display a failed transaction without partial mutation.
 * Input Parameters: error
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialog::reportError(const QString &error) //20261110-7 add
{
    m_error->setText(error);
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::accept
 * Description     : Validate identity and all editor/startup choices before publishing the final draft.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialog::accept() //20261110-7 add
{
    HmiProject candidate;
    QString error;

    candidate = m_working;
    candidate.name = m_projectName->text();
    candidate.startupPageId = m_startupPage->currentData().toString();
    candidate.startupLanguage = m_startupLanguage->currentData().toString();
    candidate.activeLanguage = m_editorLanguage->currentData().toString();
    if (!candidate.validateModel(&error) || !candidate.selectPage(m_editorPage->currentData().toString(), &error))
    {
        reportError(error);
        return;
    }
    m_working = candidate;
    QDialog::accept();
}
