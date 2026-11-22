#include "stage2_system_settings.h"
#include "device_target.h" //20261122-4 add

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFont>
#include <QFontMetrics>
#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QSet>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QUuid>
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
 * Function Name   : referenceLabel
 * Description     : Position a label using client coordinates measured at 100 percent scale.
 * Input Parameters: parent, bounds, text
 * Return Value    : The new label.
 * Date            : 2026.11.22
 ******************************************************************************/
static QLabel *referenceLabel(QWidget *parent, const QRect &bounds, const QString &text) //20261122-5 add
{
    QLabel *label = new QLabel(text, parent);

    label->setGeometry(bounds);
    return label;
}

/******************************************************************************
 * Function Name   : referenceGroup
 * Description     : Draw the one-pixel group border and inset its title into that border.
 * Input Parameters: parent, bounds, title
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
static void referenceGroup(QWidget *parent, const QRect &bounds, const QString &title = QString()) //20261122-5 add
{
    QFrame *frame = new QFrame(parent);
    QLabel *label;
    int titleWidth;

    frame->setProperty("referenceGroup", true);
    frame->setGeometry(bounds);
    if (!title.isEmpty())
    {
        titleWidth = QFontMetrics(parent->font()).horizontalAdvance(title) + 8;
        label = referenceLabel(parent, QRect(bounds.x() + 8, bounds.y() - 9, titleWidth, 18), title);
        label->setProperty("referenceLegend", true);
        label->setAlignment(Qt::AlignCenter);
    }
}

//20261122-5 add: Qt's styled combo and spin subcontrols need explicit native-sized arrow glyphs.
class SystemReferenceArrow final : public QWidget
{
public:
    enum Shape { Chevron, UpTriangle, DownTriangle };

    SystemReferenceArrow(QWidget *parent, const QRect &bounds, Shape shape)
        : QWidget(parent), m_shape(shape)
    {
        setGeometry(bounds);
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }

protected:
    /******************************************************************************
     * Function Name   : SystemReferenceArrow::paintEvent
     * Description     : Paint the measured arrow without intercepting mouse input.
     * Input Parameters: event
     * Return Value    : None.
     * Date            : 2026.11.22
     ******************************************************************************/
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        QColor color = isEnabled() ? QColor(92, 101, 112) : QColor(150, 150, 150);

        painter.setPen(color);
        if (m_shape == Chevron)
        {
            painter.drawLine(0, 0, 3, 3);
            painter.drawLine(3, 3, 6, 0);
        }
        else
        {
            painter.fillRect(QRect(0, m_shape == UpTriangle ? 2 : 0, 5, 1), color);
            painter.fillRect(QRect(1, 1, 3, 1), color);
            painter.fillRect(QRect(2, m_shape == UpTriangle ? 0 : 2, 1, 1), color);
        }
    }

private:
    Shape m_shape;
};

/******************************************************************************
 * Function Name   : referenceCombo
 * Description     : Keep combobox input selection-only while giving its text the reference palette.
 * Input Parameters: parent, bounds
 * Return Value    : Combobox with a read-only text editor.
 * Date            : 2026.11.22
 ******************************************************************************/
static QComboBox *referenceCombo(QWidget *parent, const QRect &bounds) //20261122-5 add
{
    QComboBox *combo = new QComboBox(parent);
    QFrame *inset;

    combo->setProperty("referenceChoice", true);
    combo->setGeometry(bounds);
    combo->setEditable(true);
    combo->setInsertPolicy(QComboBox::NoInsert);
    combo->setCompleter(nullptr);
    combo->lineEdit()->setReadOnly(true);
    inset = new QFrame(combo);
    inset->setAttribute(Qt::WA_TransparentForMouseEvents);
    inset->setGeometry(1, 1, bounds.width() - 18, bounds.height() - 2);
    inset->setStyleSheet(QStringLiteral("QFrame { border: 1px solid white; background: transparent; }"));
    new SystemReferenceArrow(combo, QRect(bounds.width() - 12, 9, 7, 4), SystemReferenceArrow::Chevron);
    return combo;
}

/******************************************************************************
 * Function Name   : addReferenceSpinArrows
 * Description     : Add the paired glyphs over the editable spinner subcontrols.
 * Input Parameters: spin
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
static void addReferenceSpinArrows(QSpinBox *spin) //20261122-5 add
{
    int x = spin->width() - 14;

    new SystemReferenceArrow(spin, QRect(x, 7, 5, 3), SystemReferenceArrow::UpTriangle);
    new SystemReferenceArrow(spin, QRect(x, 20, 5, 3), SystemReferenceArrow::DownTriangle);
}

//20261122-5 add: Reference-only checkboxes show the inspected default without modifying the draft.
class SystemReferenceCheckBox final : public QCheckBox
{
public:
    using QCheckBox::QCheckBox;

protected:
    void nextCheckState() override {}
};

/******************************************************************************
 * Function Name   : referenceToggle
 * Description     : Display an unavailable setting at the supplied checked and visual state.
 * Input Parameters: parent, bounds, text, checked, dimmed
 * Return Value    : A checkbox that cannot change project data.
 * Date            : 2026.11.22
 ******************************************************************************/
static QCheckBox *referenceToggle(QWidget *parent, const QRect &bounds, const QString &text,
                                  bool checked = false, bool dimmed = false) //20261122-5 add
{
    QCheckBox *check = new SystemReferenceCheckBox(text, parent);

    check->setGeometry(bounds);
    check->setChecked(checked);
    check->setFocusPolicy(Qt::NoFocus);
    check->setEnabled(!dimmed);
    check->setToolTip(QStringLiteral("此设置尚未接入。"));
    return check;
}

/******************************************************************************
 * Function Name   : referencePassword
 * Description     : Show a public reference default or a reserved empty password field.
 * Input Parameters: parent, bounds, preset
 * Return Value    : A field that does not read or store credentials.
 * Date            : 2026.11.22
 ******************************************************************************/
static QLineEdit *referencePassword(QWidget *parent, const QRect &bounds,
                                    const QString &preset = QString()) //20261122-5 add
{
    QLineEdit *field = new QLineEdit(preset, parent);

    field->setGeometry(bounds);
    field->setReadOnly(true);
    field->setFocusPolicy(Qt::NoFocus);
    field->setAlignment(Qt::AlignHCenter);
    field->setProperty("referencePassword", preset.isEmpty() ? QStringLiteral("inactive") : QStringLiteral("preset"));
    if (preset.isEmpty())
    {
        field->setEchoMode(QLineEdit::Password);
        field->setEnabled(false);
    }
    field->setToolTip(preset.isEmpty() ? QStringLiteral("此密码功能尚未接入。")
                                      : QStringLiteral("原厂默认示例，不读取工程中的密码。"));
    return field;
}

/******************************************************************************
 * Function Name   : Stage2SystemSettingsDialog::Stage2SystemSettingsDialog
 * Description     : Lay out the reference dialog while preserving the validated Lite transaction.
 * Input Parameters: project, parent
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
Stage2SystemSettingsDialog::Stage2SystemSettingsDialog(const HmiProject &project, QWidget *parent) //20261122-5 change
    : QDialog(parent), m_working(project), m_readable(false)
{
    Stage2SystemSettings settings;
    HmiPage page;
    QFont referenceFont(QStringLiteral("SimSun"));
    QComboBox *family;
    QComboBox *size;
    QComboBox *direction;
    QComboBox *screensaverPage;
    QCheckBox *upload;
    QLineEdit *field;
    QLabel *snapshotNotice;
    QString error;
    bool editable;
    int level;
    int y;
    int modelIndex;

    setObjectName(QStringLiteral("stage2SystemSettingsDialog"));
    setWindowTitle(QStringLiteral("触摸屏参数设置"));
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowContextHelpButtonHint);
    if (parent != nullptr)
    {
        setWindowIcon(parent->windowIcon());
    }
    setProperty("localHelpTopic", QStringLiteral("project"));
    referenceFont.setPixelSize(12);
    referenceFont.setStyleStrategy(QFont::NoAntialias);
    setFont(referenceFont);
    setFixedSize(625, 627);
    setStyleSheet(QStringLiteral(
        "QDialog#stage2SystemSettingsDialog { background: #f0f0f0; }"
        "QFrame[referenceGroup=true] { border: 1px solid #dcdcdc; }"
        "QLabel[referenceLegend=true], QCheckBox[referenceLegend=true] { background: #f0f0f0; }"
        "QCheckBox { spacing: 3px; }"
        "QComboBox[referenceChoice=true] { background: #caeace; border: 1px solid #abadb3; }"
        "QComboBox[referenceChoice=true] QLineEdit { background: #caeace; border: none;"
        " selection-background-color: #0078d7; selection-color: white; }"
        "QComboBox[referenceChoice=true] QLineEdit:disabled { color: #6d6d6d; }"
        "QComboBox[referenceDevice=true]:enabled { background: #0078d7; }"
        "QComboBox[referenceChoice=true]::drop-down { subcontrol-origin: border;"
        " subcontrol-position: top right; margin: 1px; width: 15px;"
        " background: #e1e1e1; border-left: 1px solid #abadb3; }"
        "QComboBox[referenceInactive=true], QComboBox[referenceInactive=true] QLineEdit"
        " { background: #f0f0f0; color: #6d6d6d; }"
        "QSpinBox { background: #caeace; border: 1px solid #abadb3; }"
        "QSpinBox::up-button, QSpinBox::down-button { width: 17px; background: #e1e1e1;"
        " border: 1px solid #abadb3; margin-right: 2px; }"
        "QSpinBox::up-button { subcontrol-origin: border; subcontrol-position: top right; margin-top: 2px; }"
        "QSpinBox::down-button { subcontrol-origin: border; subcontrol-position: bottom right; margin-bottom: 2px; }"
        "QLineEdit[referencePassword=preset] { background: #caeace; border: 1px solid #abadb3; }"
        "QLineEdit[referencePassword=inactive] { background: #b1ceed; border: 1px solid #abadb3; }"
        "QLabel#systemSnapshotReadOnlyLabel { color: #555555; }"
        "QLabel#validationErrorLabel { color: #922519; }"));

    referenceGroup(this, QRect(19, 36, 301, 188), QStringLiteral("型号"));
    referenceGroup(this, QRect(19, 246, 301, 214), QStringLiteral("其它"));
    referenceGroup(this, QRect(19, 476, 301, 94));
    referenceGroup(this, QRect(329, 36, 281, 444), QStringLiteral("用户等级密码设定"));

    referenceLabel(this, QRect(28, 50, 75, 22), QStringLiteral("HMI系列"));
    referenceLabel(this, QRect(28, 80, 75, 22), QStringLiteral("尺 寸:"));
    referenceLabel(this, QRect(28, 110, 75, 22), QStringLiteral("设备型号"));
    referenceLabel(this, QRect(28, 170, 75, 22), QStringLiteral("显示模式"));
    family = referenceCombo(this, QRect(109, 50, 205, 22));
    family->setAccessibleName(QStringLiteral("HMI系列"));
    family->addItems({QStringLiteral("AMT"), QStringLiteral("AMZ")});
    size = referenceCombo(this, QRect(109, 80, 111, 22));
    size->setAccessibleName(QStringLiteral("尺寸"));
    size->addItem(QStringLiteral("7.0 inch"));
    direction = referenceCombo(this, QRect(109, 170, 205, 22));
    direction->setAccessibleName(QStringLiteral("显示模式"));
    direction->addItem(QStringLiteral("水平"));
    m_target = referenceCombo(this, QRect(109, 110, 205, 22));
    m_target->setObjectName(QStringLiteral("systemTargetModelCombo"));
    m_target->setAccessibleName(QStringLiteral("设备型号"));
    m_target->setProperty("referenceDevice", true);
    m_target->lineEdit()->setStyleSheet(QStringLiteral(
        "QLineEdit { background: #0078d7; color: white; border: none;"
        " selection-background-color: #0078d7; selection-color: white; }"
        "QLineEdit:disabled { background: #caeace; color: #6d6d6d; }"));
    populateStage2TargetChoices(m_target, project.targetModel);
    for (modelIndex = 0; modelIndex < m_target->count(); ++modelIndex)
    {
        if (isAmz070Target(m_target->itemData(modelIndex).toString()))
        {
            m_target->setItemText(modelIndex, QStringLiteral("AMZ070W01"));
        }
    }
    family->setCurrentIndex(isAmz070Target(project.targetModel) ? 1 : 0);
    connect(family, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_target->setCurrentIndex(index);
    });
    connect(m_target, &QComboBox::currentIndexChanged, family, [family](int index) {
        QSignalBlocker blocker(family);
        family->setCurrentIndex(index);
    });

    m_hexAddress = new QCheckBox(QStringLiteral("LW使用16进制"), this);
    m_hexAddress->setObjectName(QStringLiteral("hexWordAddressCheck"));
    m_hexAddress->setGeometry(28, 200, 128, 20);
    referenceToggle(this, QRect(158, 200, 149, 20), QStringLiteral("画面设置显示模式"));

    referenceLabel(this, QRect(28, 270, 84, 22), QStringLiteral("启动画面"));
    m_startupPage = referenceCombo(this, QRect(119, 270, 161, 22));
    m_startupPage->setObjectName(QStringLiteral("systemStartupPageCombo"));
    m_startupPage->setAccessibleName(QStringLiteral("启动画面"));
    Q_FOREACH (page, project.allPages())
    {
        if (page.kind == QStringLiteral("normal"))
        {
            m_startupPage->addItem(page.name, page.id);
        }
    }
    m_startupPage->setCurrentIndex(m_startupPage->findData(project.startupPageId));

    referenceLabel(this, QRect(28, 310, 107, 30), QStringLiteral("屏幕保护时间"));
    referenceLabel(this, QRect(239, 310, 46, 30), QStringLiteral("min"));
    m_screensaver = new QSpinBox(this);
    m_screensaver->setObjectName(QStringLiteral("screensaverMinutesSpin"));
    m_screensaver->setAccessibleName(QStringLiteral("屏幕保护时间"));
    m_screensaver->setGeometry(139, 310, 91, 30);
    m_screensaver->setRange(0, 60);
    addReferenceSpinArrows(m_screensaver);
    referenceLabel(this, QRect(28, 350, 107, 30), QStringLiteral("闪烁频率"));
    referenceLabel(this, QRect(239, 350, 63, 30), QStringLiteral("x0.1s"));
    m_flicker = new QSpinBox(this);
    m_flicker->setObjectName(QStringLiteral("flickerTenthsSpin"));
    m_flicker->setAccessibleName(QStringLiteral("闪烁频率"));
    m_flicker->setGeometry(139, 350, 91, 30);
    m_flicker->setRange(1, 50);
    addReferenceSpinArrows(m_flicker);

    referenceToggle(this, QRect(28, 390, 122, 20), QStringLiteral("报警声音"));
    m_touchSound = new QCheckBox(QStringLiteral("触摸声音"), this);
    m_touchSound->setObjectName(QStringLiteral("touchSoundCheck"));
    m_touchSound->setGeometry(168, 390, 124, 20);
    referenceToggle(this, QRect(28, 430, 133, 20), QStringLiteral("是否屏保画面"));
    screensaverPage = referenceCombo(this, QRect(169, 430, 111, 22));
    screensaverPage->setProperty("referenceInactive", true);
    screensaverPage->setAccessibleName(QStringLiteral("屏保画面"));
    screensaverPage->addItem(m_startupPage->currentText());
    screensaverPage->setEnabled(false);

    upload = referenceToggle(this, QRect(27, 467, 48, 20), QStringLiteral("上载"), true);
    upload->setProperty("referenceLegend", true);
    referenceToggle(this, QRect(39, 490, 204, 20), QStringLiteral("上载是否需要密码"));
    referenceLabel(this, QRect(52, 520, 56, 20), QStringLiteral("密码"));
    referencePassword(this, QRect(109, 520, 191, 20));
    referenceLabel(this, QRect(19, 540, 297, 20), QStringLiteral("*该密码与下载对话框中的密码相同"));

    for (level = 1; level <= 8; ++level)
    {
        y = 60 + (level - 1) * 30;
        referenceLabel(this, QRect(379, y, 40, 20), QStringLiteral("等级%1").arg(level));
        field = referencePassword(this, QRect(419, y, 170, 20), QString::number(level).repeated(4));
        field->setObjectName(QStringLiteral("systemPasswordLevel%1").arg(level));
    }
    referenceLabel(this, QRect(348, 310, 116, 30), QStringLiteral("默认用户等级"));
    m_userLevel = new QSpinBox(this);
    m_userLevel->setObjectName(QStringLiteral("defaultUserLevelSpin"));
    m_userLevel->setAccessibleName(QStringLiteral("默认用户等级"));
    m_userLevel->setGeometry(469, 310, 121, 30);
    m_userLevel->setRange(1, 8);
    addReferenceSpinArrows(m_userLevel);
    referenceToggle(this, QRect(359, 360, 190, 21), QStringLiteral("最高权限密码"));
    referenceToggle(this, QRect(359, 400, 107, 21), QStringLiteral("使能上载"), false, true);
    referenceToggle(this, QRect(469, 400, 113, 21), QStringLiteral("权限操作"), false, true);
    referenceLabel(this, QRect(396, 440, 32, 20), QStringLiteral("密码"))->setEnabled(false);
    referencePassword(this, QRect(429, 440, 151, 20));

    snapshotNotice = referenceLabel(this, QRect(19, 571, 580, 19), QString());
    snapshotNotice->setObjectName(QStringLiteral("systemSnapshotReadOnlyLabel"));
    if (!project.nativeSourceSnapshot.isEmpty())
    {
        snapshotNotice->setText(QStringLiteral("此工程带有原厂快照，参数只读。显示值来自 Lite 工程或默认设置。"));
    }
    snapshotNotice->setVisible(!snapshotNotice->text().isEmpty());
    m_error = referenceLabel(this, QRect(19, 590, 370, 34), QString());
    m_error->setObjectName(QStringLiteral("validationErrorLabel"));
    m_error->setWordWrap(true);
    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this);
    m_buttons->setObjectName(QStringLiteral("buttonBox"));
    m_buttons->setGeometry(401, 594, 209, 21);
    m_buttons->layout()->setSpacing(8);
    m_buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定"));
    m_buttons->button(QDialogButtonBox::Ok)->setFixedSize(64, 21);
    m_buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    m_buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    m_buttons->button(QDialogButtonBox::Cancel)->setFixedSize(64, 21);
    m_buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助"));
    m_buttons->button(QDialogButtonBox::Help)->setFixedSize(65, 21);

    m_readable = readStage2SystemSettings(project, &settings, &error);
    m_screensaver->setValue(settings.screensaverMinutes);
    m_flicker->setValue(settings.flickerTenths);
    m_touchSound->setChecked(settings.touchSound);
    m_userLevel->setValue(settings.defaultUserLevel);
    m_hexAddress->setChecked(settings.hexWordAddress);
    editable = m_readable && project.nativeSourceSnapshot.isEmpty();
    family->setEnabled(editable);
    m_target->setEnabled(editable);
    m_startupPage->setEnabled(editable);
    m_screensaver->setEnabled(editable);
    m_flicker->setEnabled(editable);
    m_touchSound->setEnabled(editable);
    m_userLevel->setEnabled(editable);
    m_hexAddress->setEnabled(editable);
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(m_readable);
    m_error->setText(error);
    if (editable)
    {
        m_target->setFocus(Qt::OtherFocusReason);
    }
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
