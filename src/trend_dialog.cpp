#include "trend_dialog.h"
#include "hmi_property_widgets.h"
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <cmath>

/******************************************************************************
 * Function Name   : TrendDialog::channelPage
 * Description     : Edit common sampling carriers and the currently selected independent channel.
 * Input Parameters: None
 * Return Value    : Channel configuration page.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *TrendDialog::channelPage() //20261112-1 add
{
    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QGroupBox *addresses = new QGroupBox(QStringLiteral("地址"), page);
    QGroupBox *sampling = new QGroupBox(page);
    QGroupBox *fill = new QGroupBox(QStringLiteral("填充"), page);
    QGroupBox *channels = new QGroupBox(QStringLiteral("通道设置"), page);
    QGroupBox *direction = new QGroupBox(page);
    QGridLayout *grid = new QGridLayout(addresses);
    QRadioButton *left = new QRadioButton(QStringLiteral("从左到右"), direction);
    QRadioButton *right = new QRadioButton(QStringLiteral("从右到左"), direction);
    QComboBox *type;
    layout->setContentsMargins(12, 16, 16, 26); layout->addWidget(addresses, 0, 0); layout->addWidget(sampling, 1, 0); layout->addWidget(fill, 2, 0); layout->addWidget(channels, 0, 1, 2, 1); layout->addWidget(direction, 2, 1);
    textField(grid, 0, QStringLiteral("触发地址:"), QStringLiteral("TrendTriggerAddress"), &m_draft.triggerAddress, 1); textField(grid, 1, QStringLiteral("读取地址:"), QStringLiteral("TrendReadAddress"), &m_draft.readAddress, 2); textField(grid, 2, QStringLiteral("清除地址:"), QStringLiteral("TrendClearAddress"), &m_draft.clearAddress, 1);
    grid = new QGridLayout(sampling); type = choice(grid, 0, QStringLiteral("数据类型:"), QStringLiteral("TrendDataType"), TrendProperties::dataTypes(), &m_draft.dataType); integer(grid, 1, QStringLiteral("采样点数:"), QStringLiteral("TrendSamplePoints"), &m_draft.samplePoints, 1, 100); integer(grid, 2, QStringLiteral("采样时间（秒）:"), QStringLiteral("TrendSampleTime"), &m_draft.sampleTime, 1, 255); Q_UNUSED(type);
    grid = new QGridLayout(fill); color(grid, 0, QStringLiteral("底色:"), QStringLiteral("TrendBottomColor"), &m_draft.bottomColor); color(grid, 1, QStringLiteral("边框色:"), QStringLiteral("TrendFrameColor"), &m_draft.frameColor); color(grid, 2, QStringLiteral("背景色:"), QStringLiteral("TrendBackgroundColor"), &m_draft.backgroundColor);
    grid = new QGridLayout(channels); m_count = new QSpinBox(channels); m_count->setObjectName(QStringLiteral("TrendChannelCount")); m_count->setRange(1, 8); m_count->setValue(m_draft.channelCount); grid->addWidget(new QLabel(QStringLiteral("通道总数:"), channels), 0, 0); grid->addWidget(m_count, 0, 1);
    m_channel = new QComboBox(channels); m_channel->setObjectName(QStringLiteral("TrendCurrentChannel")); grid->addWidget(new QLabel(QStringLiteral("当前通道:"), channels), 1, 0); grid->addWidget(m_channel, 1, 1);
    m_minimum = new QLineEdit(channels); m_minimum->setObjectName(QStringLiteral("TrendChannelMinimum")); m_minimum->setMaxLength(80); m_maximum = new QLineEdit(channels); m_maximum->setObjectName(QStringLiteral("TrendChannelMaximum")); m_maximum->setMaxLength(80); grid->addWidget(new QLabel(QStringLiteral("最小值:"), channels), 2, 0); grid->addWidget(m_minimum, 2, 1); grid->addWidget(new QLabel(QStringLiteral("最大值:"), channels), 3, 0); grid->addWidget(m_maximum, 3, 1);
    m_lineStyle = new QComboBox(channels); m_lineStyle->setObjectName(QStringLiteral("TrendLineStyle")); m_lineStyle->addItems({QStringLiteral("—0"), QStringLiteral("--1"), QStringLiteral("--2"), QStringLiteral("--3"), QStringLiteral("--4")}); grid->addWidget(new QLabel(QStringLiteral("线型:"), channels), 4, 0); grid->addWidget(m_lineStyle, 4, 1); m_channelColor = new QPushButton(channels); m_channelColor->setObjectName(QStringLiteral("TrendChannelColor")); m_channelColor->setMinimumHeight(25); grid->addWidget(new QLabel(QStringLiteral("显示颜色:"), channels), 5, 0); grid->addWidget(m_channelColor, 5, 1);
    connect(m_minimum, &QLineEdit::textChanged, this, [this](const QString &text) { m_draft.channels[m_draft.currentChannel].minimum = text; refresh(); }); connect(m_maximum, &QLineEdit::textChanged, this, [this](const QString &text) { m_draft.channels[m_draft.currentChannel].maximum = text; refresh(); }); connect(m_lineStyle, &QComboBox::currentIndexChanged, this, [this](int style) { m_draft.channels[m_draft.currentChannel].lineStyle = style; refresh(); });
    connect(m_channelColor, &QPushButton::clicked, this, [this]() { QColor selected = QColorDialog::getColor(QColor(m_draft.channels[m_draft.currentChannel].color), this, QStringLiteral("选择颜色"), QColorDialog::DontUseNativeDialog); if (selected.isValid()) { m_draft.channels[m_draft.currentChannel].color = selected.name(); showChannel(m_draft.currentChannel); } });
    connect(m_count, &QSpinBox::valueChanged, this, &TrendDialog::changeCount); connect(m_channel, &QComboBox::currentIndexChanged, this, &TrendDialog::showChannel); changeCount(m_draft.channelCount);
    grid = new QGridLayout(direction); left->setObjectName(QStringLiteral("TrendLeftToRight")); right->setObjectName(QStringLiteral("TrendRightToLeft")); left->setChecked(!m_draft.rightToLeft); right->setChecked(m_draft.rightToLeft); grid->addWidget(left, 0, 0); grid->addWidget(right, 1, 0); connect(right, &QRadioButton::toggled, this, [this](bool checked) { m_draft.rightToLeft = checked; refresh(); }); return page;
}

/******************************************************************************
 * Function Name   : TrendDialog::axisPage
 * Description     : Apply independent axis tick/mark visibility and retain all disabled or hidden drafts.
 * Input Parameters: y
 * Return Value    : One axis configuration group.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *TrendDialog::axisPage(bool y) //20261112-1 add
{
    TrendAxis *axis = y ? &m_draft.yAxis : &m_draft.xAxis;
    QString prefix = y ? QStringLiteral("TrendY") : QStringLiteral("TrendX");
    QGroupBox *page = new QGroupBox(y ? QStringLiteral("Y轴") : QStringLiteral("X轴"), this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    QGridLayout *base = new QGridLayout;
    QWidget *axisColor = new QWidget(page);
    QGridLayout *colorGrid = new QGridLayout(axisColor);
    QWidget *marks = new QWidget(page);
    QGridLayout *markGrid = new QGridLayout(marks);
    QCheckBox *ticks = toggle(base, 0, QStringLiteral("显示刻度"), prefix + QStringLiteral("Ticks"), &axis->ticks);
    QCheckBox *showMarks;
    QComboBox *font;
    toggle(base, 1, y ? QStringLiteral("显示X轴曲线") : QStringLiteral("显示Y轴曲线"), prefix + QStringLiteral("Grid"), &axis->grid); layout->addLayout(base);
    color(colorGrid, 0, QStringLiteral("轴/刻度颜色:"), prefix + QStringLiteral("AxisColor"), &axis->axisColor); axisColor->setVisible(axis->ticks); layout->addWidget(axisColor); connect(ticks, &QCheckBox::toggled, axisColor, &QWidget::setVisible);
    base = new QGridLayout; color(base, 0, QStringLiteral("格线颜色:"), prefix + QStringLiteral("GridColor"), &axis->gridColor); integer(base, 1, QStringLiteral("主分隔数:"), prefix + QStringLiteral("Major"), &axis->major, 3, 10); integer(base, 2, QStringLiteral("次分隔数:"), prefix + QStringLiteral("Minor"), &axis->minor, 1, 3); showMarks = toggle(base, 3, QStringLiteral("显示刻度标识"), prefix + QStringLiteral("Marks"), &axis->marks); layout->addLayout(base);
    font = new QComboBox(marks); font->setObjectName(prefix + QStringLiteral("FontSize")); font->addItems({QStringLiteral("12"), QStringLiteral("16"), QStringLiteral("24"), QStringLiteral("48")}); font->setCurrentText(QString::number(axis->fontSize)); markGrid->addWidget(new QLabel(QStringLiteral("字体大小:"), marks), 0, 0); markGrid->addWidget(font, 0, 1); connect(font, &QComboBox::currentTextChanged, this, [this, axis](const QString &text) { axis->fontSize = text.toInt(); refresh(); });
    if (y) { textField(markGrid, 1, QStringLiteral("最小值:"), QStringLiteral("TrendYMinimum"), &m_draft.yMinimum); textField(markGrid, 2, QStringLiteral("最大值:"), QStringLiteral("TrendYMaximum"), &m_draft.yMaximum); integer(markGrid, 3, QStringLiteral("总位数:"), QStringLiteral("TrendTotalDigits"), &m_draft.totalDigits, 1, 10); integer(markGrid, 4, QStringLiteral("小数位数:"), QStringLiteral("TrendFractionalDigits"), &m_draft.fractionalDigits, 0, 3); }
    layout->addWidget(marks); marks->setVisible(axis->marks); connect(showMarks, &QCheckBox::toggled, marks, &QWidget::setVisible); layout->addStretch(); return page;
}

/******************************************************************************
 * Function Name   : TrendDialog::displayPage
 * Description     : Show independent X and Y axis forms without exposing native hidden X numeric fields.
 * Input Parameters: None
 * Return Value    : Display configuration page.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *TrendDialog::displayPage() //20261112-1 add
{
    QWidget *page = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(page);
    layout->setContentsMargins(14, 18, 14, 20); layout->addWidget(axisPage(false)); layout->addWidget(axisPage(true)); return page;
}

/******************************************************************************
 * Function Name   : TrendDialog::changeCount
 * Description     : Reset removed channels as observed natively and clamp the current editor selection.
 * Input Parameters: count
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void TrendDialog::changeCount(int count) //20261112-1 add
{
    QSignalBlocker blocker(m_channel);
    int index;
    if (count < m_draft.channelCount) { for (index = count; index < 8; ++index) { m_draft.channels[index] = TrendChannel(); } }
    m_draft.channelCount = count; m_draft.currentChannel = qMin(m_draft.currentChannel, count - 1); m_channel->clear(); for (index = 0; index < count; ++index) { m_channel->addItem(QString::number(index + 1)); } m_channel->setCurrentIndex(m_draft.currentChannel); showChannel(m_draft.currentChannel);
}

/******************************************************************************
 * Function Name   : TrendDialog::showChannel
 * Description     : Refill only the selected channel while blocking cross-channel write-back signals.
 * Input Parameters: channel
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void TrendDialog::showChannel(int channel) //20261112-1 add
{
    QSignalBlocker minimum(m_minimum);
    QSignalBlocker maximum(m_maximum);
    QSignalBlocker style(m_lineStyle);
    if (channel < 0 || channel >= m_draft.channelCount) { return; }
    m_draft.currentChannel = channel; m_minimum->setText(m_draft.channels[channel].minimum); m_maximum->setText(m_draft.channels[channel].maximum); m_lineStyle->setCurrentIndex(m_draft.channels[channel].lineStyle); m_channelColor->setStyleSheet(QStringLiteral("background:%1").arg(m_draft.channels[channel].color)); refresh();
}

/******************************************************************************
 * Function Name   : TrendDialog::object
 * Description     : Collect the detached trend and display-only permissions without modifying live geometry.
 * Input Parameters: None
 * Return Value    : Complete typed trend object.
 * Date            : 2026.11.12
 ******************************************************************************/
HmiObject TrendDialog::object() const //20261112-1 add
{
    HmiObject object = m_original;
    object.type = QStringLiteral("trend"); object.trend = m_draft; object.address.clear(); object.text.clear(); object.hasBitButtonProperties = false; object.hasStaticTextProperties = false; object.hasNumberDisplayProperties = false; object.hasImageDisplayProperties = false; object.trend.visibility = m_visibility->properties(); object.trend.visibilityBitChosen = m_visibility->bitStateChosen() || m_accepted; return object;
}

/******************************************************************************
 * Function Name   : TrendDialog::refresh
 * Description     : Share a static curve renderer with the canvas and tolerate construction-time field signals.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void TrendDialog::refresh() //20261112-1 add
{
    QImage image(260, 310, QImage::Format_ARGB32);
    QPainter painter;
    if (m_preview == nullptr) { return; } image.fill(QColor(QStringLiteral("#f0f0f0"))); painter.begin(&image); paintTrend(&painter, QRectF(10, 10, 240, 290), m_draft); painter.end(); m_preview->setPixmap(QPixmap::fromImage(image).scaled(130, 155, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

/******************************************************************************
 * Function Name   : TrendDialog::accept
 * Description     : Validate all active scalar and carrier spans before accepting one editor transaction.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void TrendDialog::accept() //20261112-1 add
{
    HmiObject candidate;
    QString error;
    m_accepted = true; candidate = object();
    if (!candidate.trend.validate(&error) || !candidate.trend.validateAddresses(m_project.connections, &error)) { m_accepted = false; QMessageBox::warning(this, QStringLiteral("趋势图属性未提交"), error); return; } QDialog::accept();
}

/******************************************************************************
 * Function Name   : paintTrend
 * Description     : Render a fixed editor curve with configured axes; no acquired PLC samples are implied.
 * Input Parameters: painter, rect, properties
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void paintTrend(QPainter *painter, const QRectF &rect, const TrendProperties &properties) //20261112-1 add
{
    QRectF plot = rect.adjusted(qMin(40.0, rect.width() * .2), 8, -8, -qMin(24.0, rect.height() * .2));
    QPainterPath curve;
    QFont font;
    int index;
    int channel;
    double x;
    double y;
    double value;
    painter->save(); painter->setClipRect(rect); painter->fillRect(rect, QColor(properties.backgroundColor)); painter->fillRect(plot, QColor(properties.bottomColor)); painter->setRenderHint(QPainter::Antialiasing);
    if (plot.width() <= 0 || plot.height() <= 0) { painter->restore(); return; } // Keep tiny object bounds clipped without a negative plot area.
    painter->setPen(QColor(properties.frameColor)); painter->setBrush(Qt::NoBrush); painter->drawRect(plot);
    for (index = 0; index <= properties.xAxis.major; ++index) { x = plot.left() + plot.width() * index / properties.xAxis.major; if (properties.xAxis.grid) { painter->setPen(QColor(properties.xAxis.gridColor)); painter->drawLine(QPointF(x, plot.top()), QPointF(x, plot.bottom())); } if (properties.xAxis.marks) { font = painter->font(); font.setPixelSize(properties.xAxis.fontSize); painter->setFont(font); painter->setPen(QColor(properties.xAxis.axisColor)); painter->drawText(QRectF(x - 20, plot.bottom() + 2, 40, rect.bottom() - plot.bottom()), Qt::AlignHCenter, QString::number(properties.samplePoints * index / properties.xAxis.major)); } }
    for (index = 0; index <= properties.yAxis.major; ++index) { y = plot.bottom() - plot.height() * index / properties.yAxis.major; if (properties.yAxis.grid) { painter->setPen(QColor(properties.yAxis.gridColor)); painter->drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y)); } if (properties.yAxis.marks) { value = (properties.yMinimum.toDouble() + (properties.yMaximum.toDouble() - properties.yMinimum.toDouble()) * index / properties.yAxis.major) / std::pow(10.0, properties.fractionalDigits); font = painter->font(); font.setPixelSize(properties.yAxis.fontSize); painter->setFont(font); painter->setPen(QColor(properties.yAxis.axisColor)); painter->drawText(QRectF(rect.left(), y - 12, plot.left() - rect.left() - 2, 24), Qt::AlignRight | Qt::AlignVCenter, QString::number(value, 'f', properties.fractionalDigits)); } }
    if (properties.xAxis.ticks) { painter->setPen(QColor(properties.xAxis.axisColor)); painter->drawLine(plot.bottomLeft(), plot.bottomRight()); }
    if (properties.yAxis.ticks) { painter->setPen(QColor(properties.yAxis.axisColor)); painter->drawLine(plot.bottomLeft(), plot.topLeft()); }
    painter->setClipRect(plot);
    for (channel = 0; channel < properties.channelCount; ++channel)
    {
        curve = QPainterPath();
        for (index = 0; index < 12; ++index) { x = plot.left() + plot.width() * (properties.rightToLeft ? 11 - index : index) / 11; y = plot.bottom() - plot.height() * (.15 + .7 * ((index * 3 + channel * 2) % 11) / 10); if (index == 0) { curve.moveTo(x, y); } else { curve.lineTo(x, y); } }
        painter->setPen(QPen(QColor(properties.channels[channel].color), 1, properties.channels[channel].lineStyle == 0 ? Qt::SolidLine : properties.channels[channel].lineStyle == 1 ? Qt::DashLine : properties.channels[channel].lineStyle == 2 ? Qt::DotLine : properties.channels[channel].lineStyle == 3 ? Qt::DashDotLine : Qt::DashDotDotLine)); painter->drawPath(curve);
    }
    painter->restore();
}

/******************************************************************************
 * Function Name   : TrendDialog::TrendDialog
 * Description     : Build three detached native pages and reset the editor channel selection to the first channel.
 * Input Parameters: object, project, parent
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
TrendDialog::TrendDialog(const HmiObject &object, const HmiProject &project, QWidget *parent) //20261112-1 add
    : QDialog(parent), m_original(object), m_project(project), m_draft(object.trend)
{
    QGridLayout *layout = new QGridLayout(this);
    QTabWidget *tabs = new QTabWidget(this);
    QGroupBox *preview = new QGroupBox(QStringLiteral("预览"), this);
    QVBoxLayout *previewLayout = new QVBoxLayout(preview);
    QWidget *permission = new QWidget(this);
    QVBoxLayout *permissionLayout = new QVBoxLayout(permission);
    QDialogButtonBox *buttons;
    QJsonArray connections;
    PlcConnection connection;
    m_draft.currentChannel = 0; setObjectName(QStringLiteral("TrendDialog")); setWindowTitle(QStringLiteral("趋势图")); resize(795, 584);
    setStyleSheet(QStringLiteral("QLineEdit, QComboBox, QSpinBox {background:#caeace; min-height:21px;} QLineEdit:disabled, QComboBox:disabled, QSpinBox:disabled {background:#b1ceed;}"));
    Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); } setProperty("plcConnections", connections.toVariantList());
    tabs->setObjectName(QStringLiteral("TrendTabs")); tabs->addTab(channelPage(), QStringLiteral("通道设置")); tabs->addTab(displayPage(), QStringLiteral("显示设置"));
    m_visibility = new HmiPermissionEditor(QStringLiteral("显现"), QStringLiteral("TrendVisible"), m_draft.visibility, false, permission); m_visibility->configureUnchosenBitState(m_draft.visibilityBitChosen); permissionLayout->addWidget(m_visibility); permissionLayout->addStretch(); tabs->addTab(permission, QStringLiteral("权限设置")); layout->addWidget(tabs, 0, 0); layout->setColumnStretch(0, 1);
    m_preview = new QLabel(preview); m_preview->setObjectName(QStringLiteral("TrendPreview")); m_preview->setFixedSize(130, 155); previewLayout->addWidget(m_preview); layout->addWidget(preview, 0, 1, Qt::AlignTop);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("TrendButtons")); buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); layout->addWidget(buttons, 1, 0, 1, 2);
    connect(buttons, &QDialogButtonBox::accepted, this, &TrendDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject); connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, QStringLiteral("趋势图帮助"), QStringLiteral("设置触发、读取和清除地址，逐通道配置范围、线型、颜色及坐标轴。预览使用固定示意数据。")); }); refresh();
}

/******************************************************************************
 * Function Name   : TrendDialog::textField
 * Description     : Bind a detached string and optional full-span word-address picker.
 * Input Parameters: layout, row, label, name, value, addressRole
 * Return Value    : Bound line edit.
 * Date            : 2026.11.12
 ******************************************************************************/
QLineEdit *TrendDialog::textField(QGridLayout *layout, int row, const QString &label, const QString &name, QString *value, int addressRole) //20261112-1 add
{
    QLineEdit *edit = new QLineEdit(*value, this);
    QPushButton *picker;
    edit->setMaximumWidth(addressRole ? 210 : 140); edit->setObjectName(name); edit->setMaxLength(addressRole ? 1024 : 80); layout->addWidget(new QLabel(label, this), row, 0); layout->addWidget(edit, row, 1);
    connect(edit, &QLineEdit::textChanged, this, [this, value](const QString &text) { *value = text; refresh(); });
    if (addressRole != 0) { picker = new QPushButton(QStringLiteral("…"), this); picker->setFixedWidth(26); picker->setObjectName(name + QStringLiteral("Picker")); layout->addWidget(picker, row, 2); connect(picker, &QPushButton::clicked, this, [this, edit, addressRole]() { editHmiAddress(this, edit, addressRole == 1, addressRole == 1 ? 1 : int(TrendProperties::wordCount(m_draft.dataType)) * m_draft.channelCount); }); }
    return edit;
}


/******************************************************************************
 * Function Name   : TrendDialog::choice
 * Description     : Bind a measured enumeration without replacing unrelated draft fields.
 * Input Parameters: layout, row, label, name, items, value, offset
 * Return Value    : Bound combo box.
 * Date            : 2026.11.12
 ******************************************************************************/
QComboBox *TrendDialog::choice(QGridLayout *layout, int row, const QString &label, const QString &name, const QStringList &items, int *value, int offset) //20261112-1 add
{
    QComboBox *combo = new QComboBox(this);
    combo->setMaximumWidth(230); combo->setObjectName(name); combo->addItems(items); combo->setCurrentIndex(*value - offset); layout->addWidget(new QLabel(label, this), row, 0); layout->addWidget(combo, row, 1);
    connect(combo, &QComboBox::currentIndexChanged, this, [this, value, offset](int index) { *value = index + offset; refresh(); }); return combo;
}


/******************************************************************************
 * Function Name   : TrendDialog::toggle
 * Description     : Bind a persistent checkbox to its detached boolean.
 * Input Parameters: layout, row, label, name, value
 * Return Value    : Bound checkbox.
 * Date            : 2026.11.12
 ******************************************************************************/
QCheckBox *TrendDialog::toggle(QGridLayout *layout, int row, const QString &label, const QString &name, bool *value) //20261112-1 add
{
    QCheckBox *check = new QCheckBox(label, this);
    check->setObjectName(name); check->setChecked(*value); layout->addWidget(check, row, 0, 1, 2); connect(check, &QCheckBox::toggled, this, [this, value](bool checked) { *value = checked; refresh(); }); return check;
}


/******************************************************************************
 * Function Name   : TrendDialog::integer
 * Description     : Bind bounded editor integers with explicit Lite limits.
 * Input Parameters: layout, row, label, name, value, minimum, maximum
 * Return Value    : Bounded spin editor.
 * Date            : 2026.11.12
 ******************************************************************************/
QSpinBox *TrendDialog::integer(QGridLayout *layout, int row, const QString &label, const QString &name, int *value, int minimum, int maximum) //20261112-1 add
{
    QSpinBox *spin = new QSpinBox(this);
    spin->setObjectName(name); spin->setRange(minimum, maximum); spin->setValue(*value); layout->addWidget(new QLabel(label, this), row, 0); layout->addWidget(spin, row, 1); connect(spin, &QSpinBox::valueChanged, this, [this, value](int number) { *value = number; refresh(); });
    return spin;
}


/******************************************************************************
 * Function Name   : TrendDialog::color
 * Description     : Publish a shared color selection only when the nested dialog is accepted.
 * Input Parameters: layout, row, label, name, value
 * Return Value    : Detached color picker button.
 * Date            : 2026.11.12
 ******************************************************************************/
QPushButton *TrendDialog::color(QGridLayout *layout, int row, const QString &label, const QString &name, QString *value) //20261112-1 add
{
    QPushButton *button = new QPushButton(this);
    button->setObjectName(name); button->setMinimumSize(110, 25); button->setStyleSheet(QStringLiteral("background:%1").arg(*value)); layout->addWidget(new QLabel(label, this), row, 0); layout->addWidget(button, row, 1);
    connect(button, &QPushButton::clicked, this, [this, button, value]() { QColor selected = QColorDialog::getColor(QColor(*value), this, QStringLiteral("选择颜色"), QColorDialog::DontUseNativeDialog); if (selected.isValid()) { *value = selected.name(); button->setStyleSheet(QStringLiteral("background:%1").arg(*value)); refresh(); } });
    return button;
}

