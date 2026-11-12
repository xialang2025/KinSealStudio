#include "flow_block_dialog.h"
#include "hmi_property_widgets.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTabWidget>
#include <QUuid>
#include <QVBoxLayout>

namespace
{
/******************************************************************************
 * Function Name   : flowBrush
 * Description     : Provide explicit PC approximations of named patterns without treating gradient coordinates as recovered native pixels.
 * Input Parameters: rect, pattern, front, back
 * Return Value    : Bounded editor brush.
 * Date            : 2026.11.12
 ******************************************************************************/
QBrush flowBrush(const QRectF &rect, int pattern, const QColor &front, const QColor &back) //20261112-0 add
{
    QLinearGradient linear;
    QRadialGradient radial;
    QPointF start;
    QPointF end;
    QPointF center;
    if (pattern == 0) { return QBrush(Qt::NoBrush); }
    if (pattern == 1) { return QBrush(back); }
    if (pattern >= 10)
    {
        center = pattern == 10 ? rect.topRight() : pattern == 11 ? rect.topLeft() : rect.center();
        radial = QRadialGradient(center, qMax(qreal(1), qMax(rect.width(), rect.height()))); radial.setColorAt(0, back); radial.setColorAt(1, front); return QBrush(radial);
    }
    start = rect.topLeft(); end = rect.bottomLeft();
    if (pattern == 2 || pattern == 3) { end = rect.topRight(); }
    if (pattern == 6 || pattern == 7) { start = rect.bottomLeft(); end = rect.topRight(); }
    if (pattern == 8 || pattern == 9) { end = rect.bottomRight(); }
    linear = QLinearGradient(start, end); linear.setColorAt(0, front);
    if (pattern == 3 || pattern == 5 || pattern == 7 || pattern == 9) { linear.setColorAt(0.5, back); linear.setColorAt(1, front); }
    else { linear.setColorAt(1, back); }
    return QBrush(linear);
}
}

/******************************************************************************
 * Function Name   : paintFlowBlock
 * Description     : Draw a clipped static pipe and exact block count with editor pattern brushes and optional one-pixel border.
 * Input Parameters: painter, rect, properties
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void paintFlowBlock(QPainter *painter, const QRectF &rect, const FlowBlockProperties &properties) //20261112-0 add
{
    QRectF block;
    qreal length = properties.vertical ? rect.height() : rect.width();
    qreal step = length / qMax(1, properties.count);
    int index;
    painter->save(); painter->setClipRect(rect); painter->setPen(Qt::NoPen); painter->setBrush(flowBrush(rect, properties.pattern, QColor(properties.pipeFront), QColor(properties.pipeBack))); painter->drawRect(rect);
    for (index = 0; index < properties.count; ++index)
    {
        block = properties.vertical ? QRectF(rect.left(), rect.top() + index * step, rect.width(), step * 2 / 3) : QRectF(rect.left() + index * step, rect.top(), step * 2 / 3, rect.height());
        painter->setBrush(flowBrush(block, properties.pattern, QColor(properties.flowFront), QColor(properties.flowBack))); painter->drawRect(block);
    }
    if (!properties.noBorder) { painter->setBrush(Qt::NoBrush); painter->setPen(QPen(QColor(properties.borderColor), 1)); painter->drawRect(rect.adjusted(0.5, 0.5, -0.5, -0.5)); }
    painter->restore();
}

/******************************************************************************
 * Function Name   : FlowBlockDialog::FlowBlockDialog
 * Description     : Build two native pages and a shared static preview, keeping every change local until full-model acceptance.
 * Input Parameters: object, project, parent
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
FlowBlockDialog::FlowBlockDialog(const HmiObject &object, const HmiProject &project, QWidget *parent) //20261112-0 add
    : QDialog(parent), m_original(object), m_project(project), m_draft(object.flowBlock)
{
    QGridLayout *layout = new QGridLayout(this);
    QTabWidget *tabs = new QTabWidget(this);
    QWidget *permissions = new QWidget(this);
    QHBoxLayout *permissionLayout = new QHBoxLayout(permissions);
    QGroupBox *preview = new QGroupBox(QStringLiteral("预览"), this);
    QVBoxLayout *previewLayout = new QVBoxLayout(preview);
    QDialogButtonBox *buttons;
    QJsonArray connections;
    PlcConnection connection;
    setObjectName(QStringLiteral("FlowBlockDialog")); setWindowTitle(QStringLiteral("流动块")); resize(842, 571);
    Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); } setProperty("plcConnections", connections.toVariantList());
    tabs->setObjectName(QStringLiteral("FlowTabs")); tabs->addTab(createAppearance(), QStringLiteral("外观设置"));
    m_visibility = new HmiPermissionEditor(QStringLiteral("显现"), QStringLiteral("FlowVisibility"), m_draft.visibility, false, permissions); m_visibility->configureUnchosenBitState(m_draft.visibilityBitChosen); m_visibility->setMaximumWidth(300); permissionLayout->addWidget(m_visibility, 0, Qt::AlignTop); permissionLayout->addStretch(); tabs->addTab(permissions, QStringLiteral("权限设置")); layout->addWidget(tabs, 0, 0); layout->setColumnStretch(0, 1); //20261112-0 change: The native display-only permission column keeps its natural height.
    m_preview = new QLabel(preview); m_preview->setObjectName(QStringLiteral("FlowPreview")); m_preview->setFixedSize(142, 161); previewLayout->addWidget(m_preview); layout->addWidget(preview, 0, 1, Qt::AlignTop);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("FlowButtons")); buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); layout->addWidget(buttons, 1, 0, 1, 2);
    connect(buttons, &QDialogButtonBox::accepted, this, &FlowBlockDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject); connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, QStringLiteral("流动块帮助"), QStringLiteral("设置触发位、方向、块数、颜色和显现条件。预览显示静态外观。\n设备端流动、速度帧频、触发扫描和原厂编码仍需验证。")); });
    connect(m_horizontal, &QRadioButton::toggled, this, [this](bool checked) { if (checked) { changeOrientation(false); } }); connect(m_vertical, &QRadioButton::toggled, this, [this](bool checked) { if (checked) { changeOrientation(true); } }); connect(m_pattern, &QComboBox::currentIndexChanged, this, [this]() { refresh(); }); connect(m_direction, &QComboBox::currentIndexChanged, this, [this]() { refresh(); }); connect(m_count, &QSpinBox::valueChanged, this, [this]() { refresh(); }); connect(m_noBorder, &QCheckBox::toggled, this, [this]() { refresh(); }); refresh();
}

/******************************************************************************
 * Function Name   : FlowBlockDialog::createAppearance
 * Description     : Build the observed five colors, thirteen patterns, bounded block count and independent orientation/trigger groups.
 * Input Parameters: None
 * Return Value    : Flow appearance page.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *FlowBlockDialog::createAppearance() //20261112-0 add
{
    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QGroupBox *colors = new QGroupBox(QStringLiteral("颜色"), page);
    QGridLayout *grid = new QGridLayout(colors);
    QGroupBox *properties;
    QGroupBox *trigger;
    QGroupBox *orientation;
    QVBoxLayout *column;
    QRadioButton *radio;
    QPushButton *picker;
    QStringList names = {QStringLiteral("流动块前景色:"), QStringLiteral("流动块背景色:"), QStringLiteral("管道前景色:"), QStringLiteral("管道背景色:"), QStringLiteral("边框色:")};
    int index;
    layout->setColumnStretch(0, 1); layout->setColumnStretch(1, 1); layout->setRowStretch(2, 1); colors->setFixedHeight(215); //20261112-0 add: Preserve equal lower columns and the native color-group height.
    page->setStyleSheet(QStringLiteral("QSpinBox, QComboBox, QLineEdit {background:#caeace;} QSpinBox:disabled, QComboBox:disabled, QLineEdit:disabled {background:#b1ceed;}"));
    for (index = 0; index < 5; ++index)
    { m_colors[index] = new QPushButton(colors); m_colors[index]->setObjectName(QStringLiteral("FlowColor%1").arg(index)); m_colors[index]->setMinimumSize(120, 24); grid->addWidget(new QLabel(names.at(index), colors), index / 2, (index % 2) * 2); grid->addWidget(m_colors[index], index / 2, (index % 2) * 2 + 1); connect(m_colors[index], &QPushButton::clicked, this, [this, index]() { chooseColor(index); }); }
    m_pattern = new QComboBox(colors); m_pattern->setObjectName(QStringLiteral("FlowPattern")); m_pattern->addItems(FlowBlockProperties::patterns()); m_pattern->setCurrentIndex(m_draft.pattern); grid->addWidget(new QLabel(QStringLiteral("样式:"), colors), 3, 0); grid->addWidget(m_pattern, 3, 1, 1, 3); layout->addWidget(colors, 0, 0, 1, 2);
    properties = new QGroupBox(QStringLiteral("属性"), page); grid = new QGridLayout(properties); m_direction = new QComboBox(properties); m_direction->setObjectName(QStringLiteral("FlowDirection")); m_direction->addItems(m_draft.directions()); m_direction->setCurrentIndex(m_draft.direction); m_count = new QSpinBox(properties); m_count->setObjectName(QStringLiteral("FlowCount")); m_count->setRange(1, 30); m_count->setValue(m_draft.count); grid->addWidget(new QLabel(QStringLiteral("流动方向:"), properties), 0, 0); grid->addWidget(m_direction, 0, 1); grid->addWidget(new QLabel(QStringLiteral("流动块数:"), properties), 1, 0); grid->addWidget(m_count, 1, 1); grid->addWidget(new QLabel(QStringLiteral("流动速度:"), properties), 2, 0); m_speed = new QButtonGroup(this); names = {QStringLiteral("低"), QStringLiteral("中"), QStringLiteral("高")};
    for (index = 0; index < 3; ++index) { radio = new QRadioButton(names.at(index), properties); radio->setObjectName(QStringLiteral("FlowSpeed%1").arg(index)); m_speed->addButton(radio, index); radio->setChecked(index == m_draft.speed); grid->addWidget(radio, index + 2, 1); } layout->addWidget(properties, 1, 0, 2, 1);
    trigger = new QGroupBox(QStringLiteral("触发地址"), page); trigger->setFixedHeight(70); grid = new QGridLayout(trigger); m_trigger = new QLineEdit(m_draft.triggerAddress, trigger); m_trigger->setObjectName(QStringLiteral("FlowTrigger")); picker = new QPushButton(QStringLiteral("…"), trigger); picker->setObjectName(QStringLiteral("FlowTriggerPicker")); picker->setFixedWidth(28); grid->addWidget(new QLabel(QStringLiteral("触发地址:"), trigger), 0, 0); grid->addWidget(m_trigger, 0, 1); grid->addWidget(picker, 0, 2); layout->addWidget(trigger, 1, 1); connect(picker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_trigger, true); });
    orientation = new QGroupBox(QStringLiteral("显示方式"), page); column = new QVBoxLayout(orientation); m_horizontal = new QRadioButton(QStringLiteral("水平"), orientation); m_horizontal->setObjectName(QStringLiteral("FlowHorizontal")); m_vertical = new QRadioButton(QStringLiteral("竖直"), orientation); m_vertical->setObjectName(QStringLiteral("FlowVertical")); m_horizontal->setChecked(!m_draft.vertical); m_vertical->setChecked(m_draft.vertical); m_noBorder = new QCheckBox(QStringLiteral("无边线"), orientation); m_noBorder->setObjectName(QStringLiteral("FlowNoBorder")); m_noBorder->setChecked(m_draft.noBorder); column->addWidget(m_horizontal); column->addWidget(m_vertical); column->addWidget(m_noBorder); layout->addWidget(orientation, 2, 1);
    return page;
}

/******************************************************************************
 * Function Name   : FlowBlockDialog::color
 * Description     : Resolve the five independently stored color fields for a local color-picker transaction.
 * Input Parameters: index
 * Return Value    : Color draft pointer.
 * Date            : 2026.11.12
 ******************************************************************************/
QString *FlowBlockDialog::color(int index) //20261112-0 add
{
    QString *fields[5] = {&m_draft.flowFront, &m_draft.flowBack, &m_draft.pipeFront, &m_draft.pipeBack, &m_draft.borderColor};
    return fields[qBound(0, index, 4)];
}

/******************************************************************************
 * Function Name   : FlowBlockDialog::chooseColor
 * Description     : Publish one color into the detached draft only after explicit color-dialog acceptance.
 * Input Parameters: index
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void FlowBlockDialog::chooseColor(int index) //20261112-0 add
{
    QColor selected = QColorDialog::getColor(QColor(*color(index)), this, QStringLiteral("选择颜色"), QColorDialog::DontUseNativeDialog);
    if (!selected.isValid()) { return; } *color(index) = selected.name(); refresh();
}

/******************************************************************************
 * Function Name   : FlowBlockDialog::changeOrientation
 * Description     : Apply measured reset behavior without copying old direction indices or pattern drafts into the new orientation.
 * Input Parameters: vertical
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void FlowBlockDialog::changeOrientation(bool vertical) //20261112-0 add
{
    QSignalBlocker directionBlocker(m_direction);
    QSignalBlocker patternBlocker(m_pattern);
    bool previous = m_draft.vertical;
    m_draft = object().flowBlock; m_draft.vertical = previous; m_draft.setVertical(vertical); m_direction->clear(); m_direction->addItems(m_draft.directions()); m_direction->setCurrentIndex(m_draft.direction); m_pattern->setCurrentIndex(m_draft.pattern); refresh();
}

/******************************************************************************
 * Function Name   : FlowBlockDialog::refresh
 * Description     : Refresh the five swatches and a static thin-pipe preview; retain frame-color editing even with no border checked.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void FlowBlockDialog::refresh() //20261112-0 add
{
    FlowBlockProperties properties = object().flowBlock;
    QImage sample(142, 161, QImage::Format_ARGB32);
    QPainter painter;
    int index;
    sample.fill(QColor(QStringLiteral("#f0f0f0"))); painter.begin(&sample); paintFlowBlock(&painter, properties.vertical ? QRectF(61, 14, 20, 132) : QRectF(5, 71, 132, 20), properties); painter.end(); m_preview->setPixmap(QPixmap::fromImage(sample));
    for (index = 0; index < 5; ++index) { m_colors[index]->setStyleSheet(QStringLiteral("background:%1").arg(*color(index))); }
}

/******************************************************************************
 * Function Name   : FlowBlockDialog::object
 * Description     : Collect a standalone display draft; accepted fresh empty bit radios normalize to one as observed on native save/reopen.
 * Input Parameters: None
 * Return Value    : Complete typed flow object.
 * Date            : 2026.11.12
 ******************************************************************************/
HmiObject FlowBlockDialog::object() const //20261112-0 add
{
    HmiObject object = m_original;
    FlowBlockProperties *properties = &object.flowBlock;
    *properties = m_draft; object.type = QStringLiteral("flow"); object.address.clear(); object.text.clear(); object.hasBitButtonProperties = false; object.hasStaticTextProperties = false; properties->pattern = m_pattern->currentIndex(); properties->direction = m_direction->currentIndex(); properties->count = m_count->value(); properties->speed = m_speed->checkedId(); properties->vertical = m_vertical->isChecked(); properties->noBorder = m_noBorder->isChecked(); properties->triggerAddress = m_trigger->text().trimmed(); properties->visibility = m_visibility->properties(); properties->visibilityBitChosen = m_visibility->bitStateChosen() || m_accepted; return object;
}

/******************************************************************************
 * Function Name   : FlowBlockDialog::accept
 * Description     : Validate trigger, appearance and visibility against the whole project before committing the editor result.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void FlowBlockDialog::accept() //20261112-0 add
{
    HmiObject draft = object();
    HmiProject candidate = m_project;
    QString error;
    bool replaced = false;
    int index;
    if (draft.id.isEmpty()) { draft.id = QUuid::createUuid().toString(QUuid::WithoutBraces); }
    for (index = 0; index < candidate.objects.size(); ++index) { if (candidate.objects.at(index).id == draft.id) { candidate.objects[index] = draft; replaced = true; break; } }
    if (!replaced) { candidate.objects.append(draft); }
    if (!candidate.validateModel(&error)) { QMessageBox::warning(this, QStringLiteral("流动块属性无效"), error); return; }
    m_accepted = true; QDialog::accept();
}
