#include "stage2_system_settings.h"
#include "device_target.h" //20261122-4 add

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSet>
#include <QSpinBox>
#include <QTabWidget>
#include <QUuid>
#include <QVBoxLayout>
#include <cmath>

/******************************************************************************
 * Function Name   : systemSettingsError
 * Description     : Report a system-configuration failure without modifying any caller-owned value.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.10
 ******************************************************************************/
static bool systemSettingsError(QString *error, const QString &message) //20261110-7 add
{
    if (error != nullptr) { *error = message; }
    return false;
}

/******************************************************************************
 * Function Name   : boundedSystemInteger
 * Description     : Require a finite integral JSON number within an observed native UI range.
 * Input Parameters: value, minimum, maximum
 * Return Value    : True for an exact bounded integer; booleans, strings and fractions are rejected.
 * Date            : 2026.11.10
 ******************************************************************************/
static bool boundedSystemInteger(const QJsonValue &value, int minimum, int maximum) //20261110-7 add
{
    double number;

    if (!value.isDouble()) { return false; }
    number = value.toDouble();
    return std::isfinite(number) && std::floor(number) == number && number >= minimum && number <= maximum;
}

/******************************************************************************
 * Function Name   : Stage2SystemSettings::toJson
 * Description     : Serialize only the five independently observed settings without adding native byte claims.
 * Input Parameters: None
 * Return Value    : Strict configuration properties object.
 * Date            : 2026.11.10
 ******************************************************************************/
QJsonObject Stage2SystemSettings::toJson() const //20261110-7 add
{
    return {{QStringLiteral("screensaverMinutes"), screensaverMinutes},
            {QStringLiteral("flickerTenths"), flickerTenths},
            {QStringLiteral("touchSound"), touchSound},
            {QStringLiteral("defaultUserLevel"), defaultUserLevel},
            {QStringLiteral("hexWordAddress"), hexWordAddress}};
}

/******************************************************************************
 * Function Name   : validateStage2SystemSettingsProperties
 * Description     : Validate an exact five-key active system record before the project parser accepts it.
 * Input Parameters: properties, error
 * Return Value    : True only when all known fields have their exact JSON types and observed ranges.
 * Date            : 2026.11.10
 ******************************************************************************/
bool validateStage2SystemSettingsProperties(const QJsonObject &properties, QString *error) //20261110-7 add
{
    QSet<QString> keys;
    QString key;

    keys = {QStringLiteral("screensaverMinutes"), QStringLiteral("flickerTenths"),
            QStringLiteral("touchSound"), QStringLiteral("defaultUserLevel"), QStringLiteral("hexWordAddress")};
    if (properties.size() != keys.size())
    {
        return systemSettingsError(error, QStringLiteral("系统参数必须完整包含五个已恢复字段，不能缺项或增加字段。"));
    }
    Q_FOREACH (key, properties.keys())
    {
        if (!keys.contains(key)) { return systemSettingsError(error, QStringLiteral("未知系统参数字段：%1。").arg(key)); }
    }
    if (!boundedSystemInteger(properties.value(QStringLiteral("screensaverMinutes")), 0, 60) ||
        !boundedSystemInteger(properties.value(QStringLiteral("flickerTenths")), 1, 50) ||
        !boundedSystemInteger(properties.value(QStringLiteral("defaultUserLevel")), 1, 8) ||
        !properties.value(QStringLiteral("touchSound")).isBool() ||
        !properties.value(QStringLiteral("hexWordAddress")).isBool())
    {
        return systemSettingsError(error, QStringLiteral("系统参数类型或范围无效：屏保0..60分钟、闪烁1..50×0.1秒、用户等级1..8，声音和LW进制必须为布尔值。"));
    }
    return true;
}

/******************************************************************************
 * Function Name   : readStage2SystemSettings
 * Description     : Read one project-owned implemented active system node or return the observed defaults.
 * Input Parameters: project, settings, error
 * Return Value    : True on an unambiguous record; placeholders and multiple system nodes are never overwritten.
 * Date            : 2026.11.10
 ******************************************************************************/
bool readStage2SystemSettings(const HmiProject &project, Stage2SystemSettings *settings, QString *error) //20261110-7 add
{
    Stage2SystemSettings candidate;
    HmiConfigurationNode node;
    bool found;

    if (settings == nullptr) { return systemSettingsError(error, QStringLiteral("系统参数输出无效。")); }
    found = false;
    Q_FOREACH (node, project.configurationNodes)
    {
        if (node.kind != QStringLiteral("system")) { continue; }
        if (found) { return systemSettingsError(error, QStringLiteral("工程含多个system节点，无法确定系统参数归属。")); }
        found = true;
        if (!node.ownerPageId.isEmpty() || !node.implemented || !node.active)
        {
            return systemSettingsError(error, QStringLiteral("既有system节点%1不是工程级已实施活动配置，不能覆盖此占位或其他归属的数据。").arg(node.id));
        }
        if (!validateStage2SystemSettingsProperties(node.properties, error)) { return false; }
        candidate.screensaverMinutes = node.properties.value(QStringLiteral("screensaverMinutes")).toInt();
        candidate.flickerTenths = node.properties.value(QStringLiteral("flickerTenths")).toInt();
        candidate.touchSound = node.properties.value(QStringLiteral("touchSound")).toBool();
        candidate.defaultUserLevel = node.properties.value(QStringLiteral("defaultUserLevel")).toInt();
        candidate.hexWordAddress = node.properties.value(QStringLiteral("hexWordAddress")).toBool();
    }
    *settings = candidate;
    return true;
}

/******************************************************************************
 * Function Name   : applyStage2SystemSettings
 * Description     : Apply settings to an independently validated copy while preserving every unrelated entity.
 * Input Parameters: source, settings, draft, error
 * Return Value    : True on atomic publication; native snapshots reject value changes and defaults add no node.
 * Date            : 2026.11.10
 ******************************************************************************/
bool applyStage2SystemSettings(const HmiProject &source, const Stage2SystemSettings &settings,
                               HmiProject *draft, QString *error) //20261110-7 add
{
    Stage2SystemSettings previous;
    Stage2SystemSettings defaults;
    HmiProject candidate;
    HmiConfigurationNode node;
    int index;
    int systemIndex;

    if (draft == nullptr) { return systemSettingsError(error, QStringLiteral("工程草稿输出无效。")); }
    if (!validateStage2SystemSettingsProperties(settings.toJson(), error) ||
        !readStage2SystemSettings(source, &previous, error) || !source.validateModel(error))
    {
        return false;
    }
    if (!source.nativeSourceSnapshot.isEmpty() && settings.toJson() != previous.toJson())
    {
        return systemSettingsError(error, QStringLiteral("工程包含原生源快照，系统参数当前只读；为保留原始配置，本次修改已取消。"));
    }
    candidate = source;
    systemIndex = -1;
    for (index = 0; index < candidate.configurationNodes.size(); ++index)
    {
        if (candidate.configurationNodes.at(index).kind == QStringLiteral("system")) { systemIndex = index; break; }
    }
    if (systemIndex >= 0)
    {
        // Update only the known properties; stable identity and all versioned extension metadata survive.
        candidate.configurationNodes[systemIndex].properties = settings.toJson();
    }
    else if (settings.toJson() != defaults.toJson())
    {
        node.id = QStringLiteral("system-") + QUuid::createUuid().toString(QUuid::WithoutBraces);
        node.kind = QStringLiteral("system");
        node.name = QStringLiteral("系统参数");
        node.implemented = true;
        node.active = true;
        node.properties = settings.toJson();
        candidate.configurationNodes.append(node);
    }
    if (!candidate.validateModel(error)) { return false; }
    *draft = candidate;
    return true;
}

/******************************************************************************
 * Function Name   : populateStage2TargetChoices
 * Description     : Share target choices without changing the spelling of an existing project key.
 * Input Parameters: combo, model
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
void populateStage2TargetChoices(QComboBox *combo, const QString &model) //20261122-4 add
{
    QString amt = QStringLiteral("AMT070W124-L800X480R");
    QString amz = isAmz070Target(model) ? model : QStringLiteral("AMZ070W01");

    combo->addItem(amt, amt);
    combo->addItem(QStringLiteral("AMZ070W01R"), amz);
    combo->setCurrentIndex(combo->findData(model));
}

/******************************************************************************
 * Function Name   : systemSection
 * Description     : Create a consistently spaced form inside a named settings group.
 * Input Parameters: layout, title
 * Return Value    : The populated group's form layout.
 * Date            : 2026.11.22
 ******************************************************************************/
static QFormLayout *systemSection(QBoxLayout *layout, const QString &title) //20261122-4 add
{
    QGroupBox *group = new QGroupBox(title, layout->parentWidget());
    QFormLayout *form = new QFormLayout(group);

    form->setContentsMargins(14, 14, 14, 12);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(9);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    layout->addWidget(group, 1, Qt::AlignTop);
    return form;
}

/******************************************************************************
 * Function Name   : unavailableToggle
 * Description     : Show a reserved option without creating project data for it.
 * Input Parameters: parent, label
 * Return Value    : Disabled check box.
 * Date            : 2026.11.22
 ******************************************************************************/
static QCheckBox *unavailableToggle(QWidget *parent, const QString &label) //20261122-4 add
{
    QCheckBox *check = new QCheckBox(label, parent);

    check->setEnabled(false);
    return check;
}

/******************************************************************************
 * Function Name   : unavailablePassword
 * Description     : Reserve the location of a credential without loading or storing a password.
 * Input Parameters: parent
 * Return Value    : Empty disabled password field.
 * Date            : 2026.11.22
 ******************************************************************************/
static QLineEdit *unavailablePassword(QWidget *parent) //20261122-4 add
{
    QLineEdit *field = new QLineEdit(parent);

    field->setEchoMode(QLineEdit::Password);
    field->setEnabled(false);
    return field;
}

/******************************************************************************
 * Function Name   : Stage2SystemSettingsDialog::Stage2SystemSettingsDialog
 * Description     : Organize the validated editor settings by device, runtime, and access.
 * Input Parameters: project, parent
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
Stage2SystemSettingsDialog::Stage2SystemSettingsDialog(const HmiProject &project, QWidget *parent) //20261122-4 change
    : QDialog(parent), m_working(project), m_readable(false)
{
    Stage2SystemSettings settings;
    HmiPage page;
    QVBoxLayout *layout;
    QHBoxLayout *profileLayout;
    QHBoxLayout *displayLayout;
    QHBoxLayout *accessLayout;
    QFormLayout *displayForm;
    QFormLayout *runtimeForm;
    QFormLayout *levelsForm;
    QFormLayout *uploadForm;
    QGridLayout *passwords;
    QTabWidget *tabs;
    QWidget *displayPage;
    QWidget *accessPage;
    QLabel *specification;
    QLabel *boundary;
    QLabel *snapshotNotice;
    QString error;
    QString levelLabel;
    bool editable;
    int level;

    setObjectName(QStringLiteral("stage2SystemSettingsDialog"));
    setWindowTitle(QStringLiteral("触摸屏参数设置"));
    setProperty("localHelpTopic", QStringLiteral("project"));
    resize(700, 430);
    setStyleSheet(QStringLiteral(
        "QDialog#stage2SystemSettingsDialog { background: #f1f5f9; }"
        "QGroupBox { background: #ffffff; border: 1px solid #cbd5e1;"
        " border-radius: 6px; margin-top: 12px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 5px;"
        " color: #24476a; font-weight: bold; }"
        "QLabel#systemBackendBoundaryLabel, QLabel#systemSnapshotReadOnlyLabel { color: #475569; }"
        "QLabel#validationErrorLabel { color: #922519; }"));
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(18, 16, 18, 14);
    layout->setSpacing(10);

    profileLayout = new QHBoxLayout;
    profileLayout->setSpacing(12);
    m_target = new QComboBox(this);
    m_target->setObjectName(QStringLiteral("systemTargetModelCombo"));
    m_target->setAccessibleName(QStringLiteral("目标型号"));
    populateStage2TargetChoices(m_target, project.targetModel);
    specification = new QLabel(this);
    connect(m_target, &QComboBox::currentIndexChanged, specification, [this, specification]() {
        specification->setText(isAmz070Target(m_target->currentData().toString())
            ? QStringLiteral("AMZ 系列 · 7.0 英寸 · 800 × 480")
            : QStringLiteral("AMT 系列 · 7.0 英寸 · 800 × 480"));
    });
    specification->setText(isAmz070Target(project.targetModel)
        ? QStringLiteral("AMZ 系列 · 7.0 英寸 · 800 × 480")
        : QStringLiteral("AMT 系列 · 7.0 英寸 · 800 × 480"));
    profileLayout->addWidget(new QLabel(QStringLiteral("目标型号"), this));
    profileLayout->addWidget(m_target);
    profileLayout->addWidget(specification);
    profileLayout->addStretch();
    layout->addLayout(profileLayout);

    tabs = new QTabWidget(this);
    tabs->setObjectName(QStringLiteral("systemSettingsTabs"));
    displayPage = new QWidget(tabs);
    displayLayout = new QHBoxLayout(displayPage);
    displayLayout->setContentsMargins(12, 10, 12, 12);
    displayForm = systemSection(displayLayout, QStringLiteral("显示与启动"));
    runtimeForm = systemSection(displayLayout, QStringLiteral("屏保与响应"));
    tabs->addTab(displayPage, QStringLiteral("画面与运行"));
    accessPage = new QWidget(tabs);
    accessLayout = new QHBoxLayout(accessPage);
    accessLayout->setContentsMargins(12, 10, 12, 12);
    levelsForm = systemSection(accessLayout, QStringLiteral("用户等级"));
    uploadForm = systemSection(accessLayout, QStringLiteral("上载与最高权限"));
    tabs->addTab(accessPage, QStringLiteral("权限与上载"));
    layout->addWidget(tabs, 1);

    m_startupPage = new QComboBox(displayPage);
    m_startupPage->setObjectName(QStringLiteral("systemStartupPageCombo"));
    Q_FOREACH (page, project.allPages())
    {
        if (page.kind == QStringLiteral("normal"))
        {
            m_startupPage->addItem(page.name, page.id);
        }
    }
    m_startupPage->setCurrentIndex(m_startupPage->findData(project.startupPageId));
    displayForm->addRow(QStringLiteral("启动画面"), m_startupPage);
    displayForm->addRow(QStringLiteral("显示方向"), new QLabel(QStringLiteral("水平"), displayPage));
    m_hexAddress = new QCheckBox(QStringLiteral("使用 16 进制"), displayPage);
    m_hexAddress->setObjectName(QStringLiteral("hexWordAddressCheck"));
    displayForm->addRow(QStringLiteral("LW 地址显示"), m_hexAddress);
    displayForm->addRow(unavailableToggle(displayPage, QStringLiteral("各画面单独设置显示模式")));

    m_screensaver = new QSpinBox(displayPage);
    m_screensaver->setObjectName(QStringLiteral("screensaverMinutesSpin"));
    m_screensaver->setRange(0, 60);
    m_screensaver->setSuffix(QStringLiteral(" 分钟"));
    m_flicker = new QSpinBox(displayPage);
    m_flicker->setObjectName(QStringLiteral("flickerTenthsSpin"));
    m_flicker->setRange(1, 50);
    m_flicker->setSuffix(QStringLiteral(" × 0.1 秒"));
    m_touchSound = new QCheckBox(QStringLiteral("触摸声音"), displayPage);
    m_touchSound->setObjectName(QStringLiteral("touchSoundCheck"));
    runtimeForm->addRow(QStringLiteral("屏保时间"), m_screensaver);
    runtimeForm->addRow(QStringLiteral("闪烁频率"), m_flicker);
    runtimeForm->addRow(m_touchSound);
    runtimeForm->addRow(unavailableToggle(displayPage, QStringLiteral("报警声音")));
    runtimeForm->addRow(unavailableToggle(displayPage, QStringLiteral("启用屏保画面")));
    runtimeForm->addRow(QStringLiteral("屏保画面"), new QLabel(QStringLiteral("暂未接入"), displayPage));

    m_userLevel = new QSpinBox(accessPage);
    m_userLevel->setObjectName(QStringLiteral("defaultUserLevelSpin"));
    m_userLevel->setRange(1, 8);
    levelsForm->addRow(QStringLiteral("默认用户等级"), m_userLevel);
    passwords = new QGridLayout;
    passwords->setHorizontalSpacing(10);
    for (level = 1; level <= 8; ++level)
    {
        levelLabel = QStringLiteral("等级 %1 密码").arg(level);
        passwords->addWidget(new QLabel(levelLabel, accessPage), (level - 1) % 4, (level - 1) / 4 * 2);
        passwords->addWidget(unavailablePassword(accessPage), (level - 1) % 4, (level - 1) / 4 * 2 + 1);
    }
    levelsForm->addRow(passwords);
    uploadForm->addRow(unavailableToggle(accessPage, QStringLiteral("启用上载")));
    uploadForm->addRow(unavailableToggle(accessPage, QStringLiteral("上载需密码")));
    uploadForm->addRow(QStringLiteral("上载密码"), unavailablePassword(accessPage));
    uploadForm->addRow(unavailableToggle(accessPage, QStringLiteral("启用最高权限密码")));
    uploadForm->addRow(unavailableToggle(accessPage, QStringLiteral("允许上载")));
    uploadForm->addRow(unavailableToggle(accessPage, QStringLiteral("允许权限操作")));
    uploadForm->addRow(QStringLiteral("最高权限密码"), unavailablePassword(accessPage));

    boundary = new QLabel(QStringLiteral("灰色选项尚未接入。屏保、闪烁、触摸声、默认等级和 LW 进制可保存在 KSP 工程中；含显式系统参数的工程暂不能下载。"), this);
    boundary->setObjectName(QStringLiteral("systemBackendBoundaryLabel"));
    boundary->setWordWrap(true);
    layout->addWidget(boundary);
    snapshotNotice = new QLabel(this);
    snapshotNotice->setObjectName(QStringLiteral("systemSnapshotReadOnlyLabel"));
    snapshotNotice->setWordWrap(true);
    if (!project.nativeSourceSnapshot.isEmpty())
    {
        snapshotNotice->setText(QStringLiteral("此工程带有原厂快照，参数只读。显示值来自 Lite 工程或默认设置。"));
    }
    snapshotNotice->setVisible(!snapshotNotice->text().isEmpty());
    layout->addWidget(snapshotNotice);
    m_error = new QLabel(this);
    m_error->setObjectName(QStringLiteral("validationErrorLabel"));
    m_error->setWordWrap(true);
    layout->addWidget(m_error);
    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this);
    m_buttons->setObjectName(QStringLiteral("buttonBox"));
    m_buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定"));
    m_buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    m_buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助"));
    layout->addWidget(m_buttons);
    m_readable = readStage2SystemSettings(project, &settings, &error);
    m_screensaver->setValue(settings.screensaverMinutes);
    m_flicker->setValue(settings.flickerTenths);
    m_touchSound->setChecked(settings.touchSound);
    m_userLevel->setValue(settings.defaultUserLevel);
    m_hexAddress->setChecked(settings.hexWordAddress);
    editable = m_readable && project.nativeSourceSnapshot.isEmpty();
    m_target->setEnabled(editable);
    m_startupPage->setEnabled(editable);
    m_screensaver->setEnabled(editable);
    m_flicker->setEnabled(editable);
    m_touchSound->setEnabled(editable);
    m_userLevel->setEnabled(editable);
    m_hexAddress->setEnabled(editable);
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(m_readable);
    m_error->setText(error);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &Stage2SystemSettingsDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_buttons, &QDialogButtonBox::helpRequested, this, &Stage2SystemSettingsDialog::helpRequested);
}

/******************************************************************************
 * Function Name   : Stage2SystemSettingsDialog::draftProject
 * Description     : Return the detached project for publication only after the caller observes Accepted.
 * Input Parameters: None
 * Return Value    : Validated accepted draft or the unchanged input before acceptance.
 * Date            : 2026.11.10
 ******************************************************************************/
HmiProject Stage2SystemSettingsDialog::draftProject() const //20261110-7 add
{
    return m_working;
}

/******************************************************************************
 * Function Name   : Stage2SystemSettingsDialog::accept
 * Description     : Commit model, startup page and system settings as one validated project draft.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2SystemSettingsDialog::accept() //20261122-4 change
{
    Stage2SystemSettings settings;
    HmiProject source = m_working;
    HmiProject candidate;
    QString error;

    if (!m_readable) { return; }
    source.targetModel = m_target->currentData().toString();
    source.startupPageId = m_startupPage->currentData().toString();
    if (!m_working.nativeSourceSnapshot.isEmpty() &&
        (source.targetModel != m_working.targetModel || source.startupPageId != m_working.startupPageId))
    {
        m_error->setText(QStringLiteral("此工程带有原厂快照，型号和启动画面只读。"));
        return;
    }
    settings.screensaverMinutes = m_screensaver->value();
    settings.flickerTenths = m_flicker->value();
    settings.touchSound = m_touchSound->isChecked();
    settings.defaultUserLevel = m_userLevel->value();
    settings.hexWordAddress = m_hexAddress->isChecked();
    if (!applyStage2SystemSettings(source, settings, &candidate, &error))
    {
        m_error->setText(error);
        return;
    }
    m_working = candidate;
    QDialog::accept();
}
