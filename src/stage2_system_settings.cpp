#include "stage2_system_settings.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QSet>
#include <QSpinBox>
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
 * Function Name   : Stage2SystemSettingsDialog::Stage2SystemSettingsDialog
 * Description     : Edit the observed five-field persistence slice without claiming native compilation support.
 * Input Parameters: project, parent
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
Stage2SystemSettingsDialog::Stage2SystemSettingsDialog(const HmiProject &project, QWidget *parent) //20261110-7 add
    : QDialog(parent), m_working(project), m_readable(false)
{
    Stage2SystemSettings settings;
    QVBoxLayout *layout;
    QFormLayout *form;
    QLabel *boundary;
    QLabel *snapshotNotice;
    QString error;
    bool editable;

    setObjectName(QStringLiteral("stage2SystemSettingsDialog"));
    setWindowTitle(QStringLiteral("系统参数设置"));
    resize(530, 350);
    layout = new QVBoxLayout(this);
    form = new QFormLayout;
    m_screensaver = new QSpinBox(this);
    m_screensaver->setObjectName(QStringLiteral("screensaverMinutesSpin"));
    m_screensaver->setRange(0, 60);
    m_screensaver->setSuffix(QStringLiteral(" 分钟"));
    m_flicker = new QSpinBox(this);
    m_flicker->setObjectName(QStringLiteral("flickerTenthsSpin"));
    m_flicker->setRange(1, 50);
    m_flicker->setSuffix(QStringLiteral(" ×0.1秒"));
    m_touchSound = new QCheckBox(QStringLiteral("启用"), this);
    m_touchSound->setObjectName(QStringLiteral("touchSoundCheck"));
    m_userLevel = new QSpinBox(this);
    m_userLevel->setObjectName(QStringLiteral("defaultUserLevelSpin"));
    m_userLevel->setRange(1, 8);
    m_hexAddress = new QCheckBox(QStringLiteral("使用16进制"), this);
    m_hexAddress->setObjectName(QStringLiteral("hexWordAddressCheck"));
    form->addRow(QStringLiteral("屏保时间"), m_screensaver);
    form->addRow(QStringLiteral("闪烁频率"), m_flicker);
    form->addRow(QStringLiteral("触摸声音"), m_touchSound);
    form->addRow(QStringLiteral("默认用户等级"), m_userLevel);
    form->addRow(QStringLiteral("LW地址显示"), m_hexAddress);
    layout->addLayout(form);
    boundary = new QLabel(QStringLiteral("这五项可以保存到工程并在重开后恢复；含这些显式设置的工程暂不能下载，设备编译前会明确拒绝。"), this);
    boundary->setObjectName(QStringLiteral("systemBackendBoundaryLabel"));
    boundary->setWordWrap(true);
    layout->addWidget(boundary);
    snapshotNotice = new QLabel(this);
    snapshotNotice->setObjectName(QStringLiteral("systemSnapshotReadOnlyLabel"));
    snapshotNotice->setWordWrap(true);
    if (!project.nativeSourceSnapshot.isEmpty())
    {
        snapshotNotice->setText(QStringLiteral("工程包含原生源数据，当前仅显示Lite默认或已保存的五项配置；为保留原始配置，本窗口只读。"));
    }
    layout->addWidget(snapshotNotice);
    m_error = new QLabel(this);
    m_error->setObjectName(QStringLiteral("validationErrorLabel"));
    m_error->setWordWrap(true);
    layout->addWidget(m_error);
    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_buttons->setObjectName(QStringLiteral("buttonBox"));
    layout->addWidget(m_buttons);
    m_readable = readStage2SystemSettings(project, &settings, &error);
    m_screensaver->setValue(settings.screensaverMinutes);
    m_flicker->setValue(settings.flickerTenths);
    m_touchSound->setChecked(settings.touchSound);
    m_userLevel->setValue(settings.defaultUserLevel);
    m_hexAddress->setChecked(settings.hexWordAddress);
    editable = m_readable && project.nativeSourceSnapshot.isEmpty();
    m_screensaver->setEnabled(editable);
    m_flicker->setEnabled(editable);
    m_touchSound->setEnabled(editable);
    m_userLevel->setEnabled(editable);
    m_hexAddress->setEnabled(editable);
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(m_readable);
    m_error->setText(error);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &Stage2SystemSettingsDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
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
 * Description     : Commit all five widgets as one persistence transaction after full model validation.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2SystemSettingsDialog::accept() //20261110-7 add
{
    Stage2SystemSettings settings;
    HmiProject candidate;
    QString error;

    if (!m_readable) { return; }
    settings.screensaverMinutes = m_screensaver->value();
    settings.flickerTenths = m_flicker->value();
    settings.touchSound = m_touchSound->isChecked();
    settings.defaultUserLevel = m_userLevel->value();
    settings.hexWordAddress = m_hexAddress->isChecked();
    if (!applyStage2SystemSettings(m_working, settings, &candidate, &error))
    {
        m_error->setText(error);
        return;
    }
    m_working = candidate;
    QDialog::accept();
}
