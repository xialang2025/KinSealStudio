#include "alarm_bar_dialog.h"
#include "flow_block_properties.h"
#include "alarm_registration_properties.h"
#include "hmi_property_widgets.h"
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QJsonArray>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : alarmBarBrush
 * Description     : Provide the same explicit editor gradient approximation used by other display controls.
 * Input Parameters: rect, pattern, front, back
 * Return Value    : Clipped editor fill brush.
 * Date            : 2026.11.13
 ******************************************************************************/
static QBrush alarmBarBrush(const QRectF &rect, int pattern, const QColor &front, const QColor &back) //20261113-2 add
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

/******************************************************************************
 * Function Name   : paintAlarmBar
 * Description     : Draw only the native editor sample text with fill, text color, font size and a clipped border.
 * Input Parameters: painter, rect, properties
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void paintAlarmBar(QPainter *painter, const QRectF &rect, const AlarmBarProperties &properties) //20261113-2 add
{
    QFont font(QStringLiteral("SimSun"));
    font.setPixelSize(properties.fontSize); painter->save(); painter->setClipRect(rect); painter->setPen(Qt::NoPen); painter->setBrush(alarmBarBrush(rect, properties.pattern, QColor(properties.foregroundColor), QColor(properties.backgroundColor))); painter->drawRect(rect); painter->setFont(font); painter->setPen(QColor(properties.textColor)); painter->drawText(rect.adjusted(4, 0, -4, 0), Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, QStringLiteral("Sample Message")); painter->setPen(QPen(QColor(properties.frameColor), 1)); painter->setBrush(Qt::NoBrush); painter->drawRect(rect.adjusted(0.5, 0.5, -0.5, -0.5)); painter->restore();
}

/******************************************************************************
 * Function Name   : AlarmBarDialog::basicPage
 * Description     : Bind four colors, thirteen patterns, measured speed and independent direction/order radio groups.
 * Input Parameters: None
 * Return Value    : Detached basic-settings page.
 * Date            : 2026.11.13
 ******************************************************************************/
QWidget *AlarmBarDialog::basicPage() //20261113-2 add
{
    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QGroupBox *fill = new QGroupBox(QStringLiteral("填充"), page);
    QGridLayout *fillGrid = new QGridLayout(fill);
    QGroupBox *other = new QGroupBox(QStringLiteral("其他"), page);
    QVBoxLayout *otherLayout = new QVBoxLayout(other);
    QGroupBox *text = new QGroupBox(QStringLiteral("文本"), page);
    QGridLayout *textGrid = new QGridLayout(text);
    QGridLayout *grid;
    QStringList labels = {QStringLiteral("边框色:"), QStringLiteral("前景色:"), QStringLiteral("背景色:"), QStringLiteral("文本色:")};
    QStringList names = {QStringLiteral("FrameColor"), QStringLiteral("ForegroundColor"), QStringLiteral("BackgroundColor"), QStringLiteral("TextColor")};
    QList<QString *> colors = {&m_draft.frameColor, &m_draft.foregroundColor, &m_draft.backgroundColor, &m_draft.textColor};
    QPushButton *button;
    QComboBox *pattern = new QComboBox(fill);
    QComboBox *font = new QComboBox(text);
    QSpinBox *speed = new QSpinBox(other);
    QHBoxLayout *speedLine = new QHBoxLayout;
    QGroupBox *group;
    QHBoxLayout *line;
    QRadioButton *first;
    QRadioButton *second;
    int *value;
    int row;
    int index;
    layout->setContentsMargins(12, 14, 18, 20); layout->setHorizontalSpacing(20); layout->setVerticalSpacing(12); layout->setColumnStretch(0, 4); layout->setColumnStretch(1, 5); fill->setMinimumHeight(205); other->setMinimumHeight(205);
    for (index = 0; index < 4; ++index)
    {
        grid = index == 3 ? textGrid : fillGrid; row = index == 3 ? 1 : index;
        button = new QPushButton(index == 3 ? text : fill); button->setObjectName(QStringLiteral("Alarm") + names.at(index)); button->setFixedSize(120, 24); button->setStyleSheet(QStringLiteral("background:%1;").arg(*colors.at(index))); grid->addWidget(new QLabel(labels.at(index), page), row, 0); grid->addWidget(button, row, 1, Qt::AlignLeft);
        connect(button, &QPushButton::clicked, this, [this, button, colors, index]() { QColor color = QColorDialog::getColor(QColor(*colors.at(index)), this, QStringLiteral("选择颜色"), QColorDialog::DontUseNativeDialog); if (color.isValid()) { *colors.at(index) = color.name(); button->setStyleSheet(QStringLiteral("background:%1;").arg(color.name())); refresh(); } });
    }
    pattern->setObjectName(QStringLiteral("AlarmPattern")); pattern->addItems(FlowBlockProperties::patterns()); pattern->setCurrentIndex(m_draft.pattern); pattern->setFixedWidth(120); fillGrid->addWidget(new QLabel(QStringLiteral("样式:"), fill), 3, 0); fillGrid->addWidget(pattern, 3, 1, Qt::AlignLeft); connect(pattern, &QComboBox::currentIndexChanged, this, [this](int v) { m_draft.pattern = v; refresh(); }); layout->addWidget(fill, 0, 0);
    speed->setObjectName(QStringLiteral("AlarmSpeed")); speed->setRange(0, 30); speed->setValue(m_draft.speed); speed->setFixedWidth(120); speedLine->addWidget(new QLabel(QStringLiteral("移动速度:"), other)); speedLine->addWidget(speed); speedLine->addWidget(new QLabel(QStringLiteral("x100ms"), other)); speedLine->addStretch(); otherLayout->addLayout(speedLine); connect(speed, &QSpinBox::valueChanged, this, [this](int v) { m_draft.speed = v; });
    for (index = 0; index < 2; ++index)
    {
        value = index == 0 ? &m_draft.direction : &m_draft.sortOrder; group = new QGroupBox(index == 0 ? QStringLiteral("方向") : QStringLiteral("报警排序"), other); line = new QHBoxLayout(group); first = new QRadioButton(index == 0 ? QStringLiteral("向左") : QStringLiteral("按时间顺序"), group); second = new QRadioButton(index == 0 ? QStringLiteral("向右") : QStringLiteral("按时间逆序"), group); first->setObjectName(index == 0 ? QStringLiteral("AlarmLeft") : QStringLiteral("AlarmTime")); second->setObjectName(index == 0 ? QStringLiteral("AlarmRight") : QStringLiteral("AlarmReverseTime")); first->setChecked(*value == 0); second->setChecked(*value == 1); line->addWidget(first); line->addWidget(second); otherLayout->addWidget(group); connect(first, &QRadioButton::toggled, this, [value](bool checked) { if (checked) { *value = 0; } }); connect(second, &QRadioButton::toggled, this, [value](bool checked) { if (checked) { *value = 1; } });
    }
    otherLayout->addStretch(); layout->addWidget(other, 0, 1); textGrid->setColumnStretch(2, 1); font->setObjectName(QStringLiteral("AlarmFontSize")); font->addItems({QStringLiteral("12"), QStringLiteral("16"), QStringLiteral("24"), QStringLiteral("48"), QStringLiteral("64")}); font->setCurrentText(QString::number(m_draft.fontSize)); font->setFixedWidth(120); textGrid->addWidget(new QLabel(QStringLiteral("字体大小:"), text), 0, 0); textGrid->addWidget(font, 0, 1); connect(font, &QComboBox::currentTextChanged, this, [this](const QString &v) { m_draft.fontSize = v.toInt(); refresh(); }); layout->addWidget(text, 1, 0, 1, 2); layout->setRowStretch(2, 1); return page;
}

/******************************************************************************
 * Function Name   : AlarmBarDialog::AlarmBarDialog
 * Description     : Build basic and display-only permissions pages with independent transaction lifetime.
 * Input Parameters: object, project, parent
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
AlarmBarDialog::AlarmBarDialog(const HmiObject &object, const HmiProject &project, QWidget *parent)
    : QDialog(parent), m_original(object), m_project(project), m_draft(object.alarmBar) //20261113-2 add
{
    QGridLayout *layout = new QGridLayout(this);
    QTabWidget *tabs = new QTabWidget(this);
    QWidget *permission = new QWidget(this);
    QVBoxLayout *permissions = new QVBoxLayout(permission);
    QGroupBox *preview = new QGroupBox(QStringLiteral("预览"), this);
    QVBoxLayout *previewLayout = new QVBoxLayout(preview);
    QDialogButtonBox *buttons;
    QJsonArray connections;
    PlcConnection connection;
    setObjectName(QStringLiteral("AlarmBarDialog")); setWindowTitle(QStringLiteral("动态报警条")); resize(820, 560); setStyleSheet(QStringLiteral("QLineEdit, QComboBox, QSpinBox {background:#caeace;} QLineEdit:disabled, QComboBox:disabled, QSpinBox:disabled {background:#b1ceed;}")); Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); } setProperty("plcConnections", connections.toVariantList());
    tabs->setObjectName(QStringLiteral("AlarmTabs")); tabs->addTab(basicPage(), QStringLiteral("基本设置")); m_visibility = new HmiPermissionEditor(QStringLiteral("显现"), QStringLiteral("AlarmVisible"), m_draft.visibility, false, permission); m_visibility->configureUnchosenBitState(m_draft.visibilityBitChosen); permissions->addWidget(m_visibility); permissions->addStretch(); tabs->addTab(permission, QStringLiteral("权限设置")); layout->addWidget(tabs, 0, 0); layout->setColumnStretch(0, 1); m_preview = new QLabel(preview); m_preview->setObjectName(QStringLiteral("AlarmPreview")); m_preview->setFixedSize(130, 130); previewLayout->addWidget(m_preview); layout->addWidget(preview, 0, 1, Qt::AlignTop);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("AlarmButtons")); buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); layout->addWidget(buttons, 1, 0, 1, 2); connect(buttons, &QDialogButtonBox::accepted, this, &AlarmBarDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject); connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, QStringLiteral("动态报警条帮助"), QStringLiteral("先在报警登录中配置数位地址报警或条件报警，再设置显示外观、滚动方向、排序与显现条件。")); }); refresh();
}

/******************************************************************************
 * Function Name   : AlarmBarDialog::refresh
 * Description     : Render a fixed editor sample without simulating alarm generation or scan timing.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void AlarmBarDialog::refresh() //20261113-2 add
{
    QImage image(130, 130, QImage::Format_ARGB32);
    QPainter painter;
    if (m_preview == nullptr) { return; } image.fill(QColor(QStringLiteral("#f0f0f0"))); painter.begin(&image); paintAlarmBar(&painter, QRectF(15, 15, 100, 100), m_draft); painter.end(); m_preview->setPixmap(QPixmap::fromImage(image)); // Keep native pixel font size in the editor preview.
}

/******************************************************************************
 * Function Name   : AlarmBarDialog::object
 * Description     : Return a detached display object with no inherited operation or monitor carrier.
 * Input Parameters: None
 * Return Value    : Complete detached display object.
 * Date            : 2026.11.13
 ******************************************************************************/
HmiObject AlarmBarDialog::object() const //20261113-2 add
{
    HmiObject object = m_original;
    object.type = QStringLiteral("alarmbar"); object.alarmBar = m_draft; object.address.clear(); object.text.clear(); object.hasBitButtonProperties = false; object.hasStaticTextProperties = false; object.hasNumberDisplayProperties = false; object.hasImageDisplayProperties = false; object.alarmBar.visibility = m_visibility->properties(); object.alarmBar.visibilityBitChosen = m_visibility->bitStateChosen() || m_accepted; return object;
}

/******************************************************************************
 * Function Name   : AlarmBarDialog::accept
 * Description     : Check registration prerequisites and all active visibility carriers before acceptance.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void AlarmBarDialog::accept() //20261113-2 add
{
    HmiObject candidate;
    QString error;
    m_accepted = true; candidate = object();
    if (!hasAlarmRegistrations(m_project)) { m_accepted = false; QMessageBox::warning(this, QStringLiteral("动态报警条"), QStringLiteral("位报警和条件报警都不存在")); return; }
    if (!candidate.alarmBar.validate(&error) || !candidate.alarmBar.validateAddresses(m_project.connections, &error)) { m_accepted = false; QMessageBox::warning(this, QStringLiteral("动态报警条属性未提交"), error); return; } QDialog::accept();
}

