#include "history_trend_dialog.h"
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
 * Function Name   : HistoryTrendDialog::channelPage
 * Description     : Bind a named collection and independent curve source, range, line and color drafts.
 * Input Parameters: None
 * Return Value    : General history page.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *HistoryTrendDialog::channelPage() //20261112-1 add
{
    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QGroupBox *collection = new QGroupBox(page);
    QGroupBox *channels = new QGroupBox(QStringLiteral("通道设置"), page);
    QGroupBox *fill = new QGroupBox(QStringLiteral("填充"), page);
    QGroupBox *range = new QGroupBox(QStringLiteral("显示范围"), page);
    QGridLayout *grid = new QGridLayout(collection);
    QComboBox *name = new QComboBox(collection);
    DataCollectionItem item;
    layout->setContentsMargins(12, 16, 16, 26); layout->addWidget(collection, 0, 0); layout->addWidget(channels, 0, 1); layout->addWidget(fill, 1, 0); layout->addWidget(range, 1, 1); layout->setRowStretch(2, 1);
    name->setObjectName(QStringLiteral("HistoryCollection")); name->addItem(m_collectionName, m_draft.collectionId); grid->addWidget(new QLabel(QStringLiteral("数据收集:"), collection), 0, 0); grid->addWidget(name, 0, 1);
    m_count = new QSpinBox(collection); m_count->setObjectName(QStringLiteral("HistoryCurveCount")); m_count->setRange(1, qMin(8, int(m_collection.items.size()))); m_count->setValue(m_draft.curveCount); grid->addWidget(new QLabel(QStringLiteral("曲线总数:"), collection), 1, 0); grid->addWidget(m_count, 1, 1);
    grid = new QGridLayout(fill); color(grid, 0, QStringLiteral("底色:"), QStringLiteral("HistoryBottomColor"), &m_draft.bottomColor); color(grid, 1, QStringLiteral("背景色:"), QStringLiteral("HistoryBackgroundColor"), &m_draft.backgroundColor);
    grid = new QGridLayout(channels); m_channel = new QComboBox(channels); m_channel->setObjectName(QStringLiteral("HistoryCurrentCurve")); grid->addWidget(new QLabel(QStringLiteral("当前通道:"), channels), 0, 0); grid->addWidget(m_channel, 0, 1);
    m_source = new QComboBox(channels); m_source->setObjectName(QStringLiteral("HistorySource")); Q_FOREACH (item, m_collection.items) { m_source->addItem(item.name, item.id); } grid->addWidget(new QLabel(QStringLiteral("数据来源:"), channels), 1, 0); grid->addWidget(m_source, 1, 1);
    m_lineStyle = new QComboBox(channels); m_lineStyle->setObjectName(QStringLiteral("HistoryLineStyle")); m_lineStyle->addItems({QStringLiteral("—0"), QStringLiteral("--1"), QStringLiteral("--2"), QStringLiteral("--3"), QStringLiteral("--4")}); grid->addWidget(new QLabel(QStringLiteral("线型:"), channels), 2, 0); grid->addWidget(m_lineStyle, 2, 1);
    m_channelColor = new QPushButton(channels); m_channelColor->setObjectName(QStringLiteral("HistoryCurveColor")); m_channelColor->setMinimumHeight(25); grid->addWidget(new QLabel(QStringLiteral("颜色:"), channels), 3, 0); grid->addWidget(m_channelColor, 3, 1);
    grid = new QGridLayout(range); m_minimum = new QLineEdit(range); m_minimum->setObjectName(QStringLiteral("HistoryCurveMinimum")); m_minimum->setMaxLength(80); m_maximum = new QLineEdit(range); m_maximum->setObjectName(QStringLiteral("HistoryCurveMaximum")); m_maximum->setMaxLength(80); grid->addWidget(new QLabel(QStringLiteral("最小值:"), range), 0, 0); grid->addWidget(m_minimum, 0, 1); grid->addWidget(new QLabel(QStringLiteral("最大值:"), range), 1, 0); grid->addWidget(m_maximum, 1, 1);
    connect(m_minimum, &QLineEdit::textChanged, this, [this](const QString &text) { m_draft.curves[m_draft.currentCurve].minimum = text; refresh(); }); connect(m_maximum, &QLineEdit::textChanged, this, [this](const QString &text) { m_draft.curves[m_draft.currentCurve].maximum = text; refresh(); }); connect(m_lineStyle, &QComboBox::currentIndexChanged, this, [this](int style) { m_draft.curves[m_draft.currentCurve].lineStyle = style; refresh(); });
    connect(m_source, &QComboBox::currentIndexChanged, this, [this](int index) { if (index >= 0) { m_draft.curves[m_draft.currentCurve].sourceItemId = m_source->itemData(index).toString(); refresh(); } });
    connect(m_channelColor, &QPushButton::clicked, this, [this]() { QColor selected = QColorDialog::getColor(QColor(m_draft.curves[m_draft.currentCurve].color), this, QStringLiteral("选择颜色"), QColorDialog::DontUseNativeDialog); if (selected.isValid()) { m_draft.curves[m_draft.currentCurve].color = selected.name(); showChannel(m_draft.currentCurve); } });
    connect(m_count, &QSpinBox::valueChanged, this, &HistoryTrendDialog::changeCount); connect(m_channel, &QComboBox::currentIndexChanged, this, &HistoryTrendDialog::showChannel); changeCount(m_draft.curveCount); return page;
}

/******************************************************************************
 * Function Name   : HistoryTrendDialog::axisPage
 * Description     : Keep axis limits visible while the native checkable mark/time groups hide only their own fields.
 * Input Parameters: y
 * Return Value    : Independent X or Y axis group.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *HistoryTrendDialog::axisPage(bool y) //20261112-1 add
{
    TrendAxis *axis = y ? &m_draft.yAxis : &m_draft.xAxis;
    QString prefix = y ? QStringLiteral("HistoryY") : QStringLiteral("HistoryX");
    QGroupBox *page = new QGroupBox(y ? QStringLiteral("Y轴") : QStringLiteral("X轴"), this);
    QGridLayout *layout = new QGridLayout(page);
    QWidget *flags = new QWidget(page);
    QHBoxLayout *flagLayout = new QHBoxLayout(flags);
    QCheckBox *ticks = new QCheckBox(QStringLiteral("显示刻度"), flags);
    QCheckBox *gridlines = new QCheckBox(y ? QStringLiteral("显示X轴曲线") : QStringLiteral("显示Y轴曲线"), flags);
    QWidget *numbers = new QWidget(page);
    QGridLayout *numberGrid = new QGridLayout(numbers);
    QWidget *palette = new QWidget(page);
    QGridLayout *paletteGrid = new QGridLayout(palette);
    QGroupBox *marks = new QGroupBox(y ? QStringLiteral("刻度") : QStringLiteral("显示时间"), page);
    QWidget *fields = new QWidget(marks);
    QGridLayout *markGrid = new QGridLayout(fields);
    QVBoxLayout *markLayout = new QVBoxLayout(marks);
    QComboBox *font = new QComboBox(fields);
    QPushButton *axisColor;
    QGroupBox *time;
    QHBoxLayout *timeLayout;
    QWidget *part;
    QGridLayout *partGrid;
    int index;
    int *values[3] = {&m_draft.hours, &m_draft.minutes, &m_draft.seconds};
    QStringList names = {QStringLiteral("Hours"), QStringLiteral("Minutes"), QStringLiteral("Seconds")};
    QStringList labels = {QStringLiteral("小时:"), QStringLiteral("分钟:"), QStringLiteral("秒:")};
    ticks->setObjectName(prefix + QStringLiteral("Ticks")); ticks->setChecked(axis->ticks); gridlines->setObjectName(prefix + QStringLiteral("Grid")); gridlines->setChecked(axis->grid); flagLayout->addWidget(ticks); flagLayout->addWidget(gridlines); flagLayout->setContentsMargins(0, 0, 0, 0);
    connect(ticks, &QCheckBox::toggled, this, [this, axis](bool checked) { axis->ticks = checked; refresh(); }); connect(gridlines, &QCheckBox::toggled, this, [this, axis](bool checked) { axis->grid = checked; refresh(); });
    numberGrid->setContentsMargins(0, 0, 0, 0); paletteGrid->setContentsMargins(0, 0, 0, 0);
    integer(numberGrid, 0, QStringLiteral("主分隔数:"), prefix + QStringLiteral("Major"), &axis->major, 3, 10); integer(numberGrid, 1, QStringLiteral("次分隔数:"), prefix + QStringLiteral("Minor"), &axis->minor, 1, 3);
    axisColor = color(paletteGrid, 0, QStringLiteral("轴/刻度颜色:"), prefix + QStringLiteral("AxisColor"), &axis->axisColor); color(paletteGrid, 1, QStringLiteral("格线颜色:"), prefix + QStringLiteral("GridColor"), &axis->gridColor); axisColor->setVisible(axis->ticks); connect(ticks, &QCheckBox::toggled, axisColor, &QWidget::setVisible);
    if (y) { textField(numberGrid, 2, QStringLiteral("最大值:"), QStringLiteral("HistoryYMaximum"), &m_draft.yMaximum); textField(paletteGrid, 2, QStringLiteral("最小值:"), QStringLiteral("HistoryYMinimum"), &m_draft.yMinimum); }
    marks->setObjectName(prefix + QStringLiteral("Marks")); marks->setCheckable(true); marks->setChecked(axis->marks); markLayout->addWidget(fields); fields->setVisible(axis->marks); connect(marks, &QGroupBox::toggled, this, [this, axis, fields](bool checked) { axis->marks = checked; fields->setVisible(checked); refresh(); });
    font->setObjectName(prefix + QStringLiteral("FontSize")); font->addItems({QStringLiteral("12"), QStringLiteral("16"), QStringLiteral("24"), QStringLiteral("48")}); font->setCurrentText(QString::number(axis->fontSize)); markGrid->addWidget(new QLabel(QStringLiteral("字体大小:"), fields), 0, 0); markGrid->addWidget(font, 0, 1); connect(font, &QComboBox::currentTextChanged, this, [this, axis](const QString &value) { axis->fontSize = value.toInt(); refresh(); });
    if (y) { integer(markGrid, 1, QStringLiteral("总位数:"), QStringLiteral("HistoryTotalDigits"), &m_draft.totalDigits, 1, 10); integer(markGrid, 2, QStringLiteral("小数位数:"), QStringLiteral("HistoryFractionalDigits"), &m_draft.fractionalDigits, 0, 3); }
    else { choice(markGrid, 1, QStringLiteral("格式:"), QStringLiteral("HistoryTimeFormat"), {QStringLiteral("HH:MM"), QStringLiteral("HH:MM:SS")}, &m_draft.timeFormat); }
    layout->addWidget(flags, 0, 0, 1, 2); layout->addWidget(numbers, 1, 0); layout->addWidget(palette, 1, 1); layout->addWidget(marks, 0, 2, 2, 1, Qt::AlignTop); layout->setColumnStretch(0, 1); layout->setColumnStretch(1, 1); layout->setColumnStretch(2, 1);
    if (!y)
    {
        time = new QGroupBox(QStringLiteral("时间范围"), page); timeLayout = new QHBoxLayout(time);
        for (index = 0; index < 3; ++index) { part = new QWidget(time); partGrid = new QGridLayout(part); partGrid->setContentsMargins(0, 0, 0, 0); integer(partGrid, 0, labels[index], QStringLiteral("History") + names[index], values[index], 0, index == 0 ? 48 : 59); timeLayout->addWidget(part); }
        layout->addWidget(time, 2, 0, 1, 3);
    }
    return page;
}

/******************************************************************************
 * Function Name   : HistoryTrendDialog::displayPage
 * Description     : Show independent X and Y axis forms without exposing native hidden X numeric fields.
 * Input Parameters: None
 * Return Value    : Display configuration page.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *HistoryTrendDialog::displayPage() //20261112-1 add
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(14, 18, 14, 20); layout->addWidget(axisPage(false)); layout->addWidget(axisPage(true)); return page;
}

/******************************************************************************
 * Function Name   : HistoryTrendDialog::changeCount
 * Description     : Reset removed curve drafts by Lite policy and clamp the current editor selection.
 * Input Parameters: count
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void HistoryTrendDialog::changeCount(int count) //20261112-1 add
{
    QSignalBlocker blocker(m_channel);
    int index;
    if (count < m_draft.curveCount) { for (index = count; index < 8; ++index) { m_draft.curves[index] = HistoryTrendCurve(); m_draft.curves[index].sourceItemId = m_collection.items.first().id; } }
    m_draft.curveCount = count; m_draft.currentCurve = qMin(m_draft.currentCurve, count - 1); m_channel->clear(); for (index = 0; index < count; ++index) { m_channel->addItem(QString::number(index + 1)); } m_channel->setCurrentIndex(m_draft.currentCurve); showChannel(m_draft.currentCurve);
}

/******************************************************************************
 * Function Name   : HistoryTrendDialog::showChannel
 * Description     : Refill only the selected channel while blocking cross-channel write-back signals.
 * Input Parameters: channel
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void HistoryTrendDialog::showChannel(int channel) //20261112-1 add
{
    QSignalBlocker source(m_source);
    QSignalBlocker minimum(m_minimum);
    QSignalBlocker maximum(m_maximum);
    QSignalBlocker style(m_lineStyle);
    if (channel < 0 || channel >= m_draft.curveCount) { return; }
    m_draft.currentCurve = channel; m_source->setCurrentIndex(m_source->findData(m_draft.curves[channel].sourceItemId)); m_minimum->setText(m_draft.curves[channel].minimum); m_maximum->setText(m_draft.curves[channel].maximum); m_lineStyle->setCurrentIndex(m_draft.curves[channel].lineStyle); m_channelColor->setStyleSheet(QStringLiteral("background:%1").arg(m_draft.curves[channel].color)); refresh();
}

/******************************************************************************
 * Function Name   : HistoryTrendDialog::object
 * Description     : Collect the detached trend and display-only permissions without modifying live geometry.
 * Input Parameters: None
 * Return Value    : Complete typed trend object.
 * Date            : 2026.11.12
 ******************************************************************************/
HmiObject HistoryTrendDialog::object() const //20261112-1 add
{
    HmiObject object = m_original;
    object.type = QStringLiteral("historytrend"); object.historyTrend = m_draft; object.address.clear(); object.text.clear(); object.hasBitButtonProperties = false; object.hasStaticTextProperties = false; object.hasNumberDisplayProperties = false; object.hasImageDisplayProperties = false; object.historyTrend.visibility = m_visibility->properties(); object.historyTrend.visibilityBitChosen = m_visibility->bitStateChosen() || m_accepted; return object;
}

/******************************************************************************
 * Function Name   : HistoryTrendDialog::refresh
 * Description     : Share a static curve renderer with the canvas and tolerate construction-time field signals.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void HistoryTrendDialog::refresh() //20261112-1 add
{
    QImage image(260, 310, QImage::Format_ARGB32);
    QPainter painter;
    if (m_preview == nullptr) { return; } image.fill(QColor(QStringLiteral("#f0f0f0"))); painter.begin(&image); paintHistoryTrend(&painter, QRectF(10, 10, 240, 290), m_draft); painter.end(); m_preview->setPixmap(QPixmap::fromImage(image).scaled(130, 155, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

/******************************************************************************
 * Function Name   : HistoryTrendDialog::accept
 * Description     : Validate all active scalar and carrier spans before accepting one editor transaction.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void HistoryTrendDialog::accept() //20261112-1 add
{
    HmiObject candidate;
    QString error;
    m_accepted = true; candidate = object();
    if (!candidate.historyTrend.validate(&error) || !candidate.historyTrend.validateCollection(m_collection, &error) || !candidate.historyTrend.validateAddresses(m_project.connections, &error)) { m_accepted = false; QMessageBox::warning(this, QStringLiteral("历史趋势图属性未提交"), error); return; } QDialog::accept();
}

/******************************************************************************
 * Function Name   : paintHistoryTrend
 * Description     : Render a fixed editor curve with configured axes; no acquired PLC samples are implied.
 * Input Parameters: painter, rect, properties
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void paintHistoryTrend(QPainter *painter, const QRectF &rect, const HistoryTrendProperties &properties) //20261112-1 add
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
    painter->setPen(QColor(properties.yAxis.axisColor)); painter->setBrush(Qt::NoBrush); painter->drawRect(plot);
    for (index = 0; index <= properties.xAxis.major; ++index) { x = plot.left() + plot.width() * index / properties.xAxis.major; if (properties.xAxis.grid) { painter->setPen(QColor(properties.xAxis.gridColor)); painter->drawLine(QPointF(x, plot.top()), QPointF(x, plot.bottom())); } if (properties.xAxis.marks) { font = painter->font(); font.setPixelSize(properties.xAxis.fontSize); painter->setFont(font); painter->setPen(QColor(properties.xAxis.axisColor)); painter->drawText(QRectF(x - 20, plot.bottom() + 2, 40, rect.bottom() - plot.bottom()), Qt::AlignHCenter, QStringLiteral("%1:%2%3").arg((properties.hours * 3600 + properties.minutes * 60 + properties.seconds) * index / properties.xAxis.major / 3600, 2, 10, QLatin1Char('0')).arg(((properties.hours * 3600 + properties.minutes * 60 + properties.seconds) * index / properties.xAxis.major / 60) % 60, 2, 10, QLatin1Char('0')).arg(properties.timeFormat == 0 ? QString() : QStringLiteral(":%1").arg(((properties.hours * 3600 + properties.minutes * 60 + properties.seconds) * index / properties.xAxis.major) % 60, 2, 10, QLatin1Char('0')))); } }
    for (index = 0; index <= properties.yAxis.major; ++index) { y = plot.bottom() - plot.height() * index / properties.yAxis.major; if (properties.yAxis.grid) { painter->setPen(QColor(properties.yAxis.gridColor)); painter->drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y)); } if (properties.yAxis.marks) { value = (properties.yMinimum.toDouble() + (properties.yMaximum.toDouble() - properties.yMinimum.toDouble()) * index / properties.yAxis.major) / std::pow(10.0, properties.fractionalDigits); font = painter->font(); font.setPixelSize(properties.yAxis.fontSize); painter->setFont(font); painter->setPen(QColor(properties.yAxis.axisColor)); painter->drawText(QRectF(rect.left(), y - 12, plot.left() - rect.left() - 2, 24), Qt::AlignRight | Qt::AlignVCenter, QString::number(value, 'f', properties.fractionalDigits)); } }
    if (properties.xAxis.ticks) { painter->setPen(QColor(properties.xAxis.axisColor)); painter->drawLine(plot.bottomLeft(), plot.bottomRight()); }
    if (properties.yAxis.ticks) { painter->setPen(QColor(properties.yAxis.axisColor)); painter->drawLine(plot.bottomLeft(), plot.topLeft()); }
    painter->setClipRect(plot);
    for (channel = 0; channel < properties.curveCount; ++channel)
    {
        curve = QPainterPath();
        for (index = 0; index < 12; ++index) { x = plot.left() + plot.width() * (index) / 11; y = plot.bottom() - plot.height() * (.15 + .7 * ((index * 3 + channel * 2) % 11) / 10); if (index == 0) { curve.moveTo(x, y); } else { curve.lineTo(x, y); } }
        painter->setPen(QPen(QColor(properties.curves[channel].color), 1, properties.curves[channel].lineStyle == 0 ? Qt::SolidLine : properties.curves[channel].lineStyle == 1 ? Qt::DashLine : properties.curves[channel].lineStyle == 2 ? Qt::DotLine : properties.curves[channel].lineStyle == 3 ? Qt::DashDotLine : Qt::DashDotDotLine)); painter->drawPath(curve);
    }
    painter->restore();
}

/******************************************************************************
 * Function Name   : HistoryTrendDialog::HistoryTrendDialog
 * Description     : Build three detached native pages and reset the editor channel selection to the first channel.
 * Input Parameters: object, project, parent
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
HistoryTrendDialog::HistoryTrendDialog(const HmiObject &object, const HmiProject &project, QWidget *parent) //20261112-1 add
    : QDialog(parent), m_original(object), m_project(project), m_draft(object.historyTrend)
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
    HmiConfigurationNode node;
    QString error;
    int index;
    Q_FOREACH (node, project.configurationNodes)
    {
        if (node.kind == QStringLiteral("dataCollection") && node.implemented && node.active && (m_draft.collectionId.isEmpty() || node.id == m_draft.collectionId) && DataCollectionProperties::fromJson(node.properties, &m_collection, &error))
        {
            m_collectionName = node.name;
            if (m_draft.collectionId.isEmpty()) { m_draft.collectionId = node.id; m_draft.curveCount = qMin(8, int(m_collection.items.size())); for (index = 0; index < 8; ++index) { m_draft.curves[index].sourceItemId = m_collection.items.first().id; } }
            break;
        }
    }
    m_draft.currentCurve = 0; setObjectName(QStringLiteral("HistoryTrendDialog")); setWindowTitle(QStringLiteral("历史趋势图")); resize(805, 584);
    setStyleSheet(QStringLiteral("QLineEdit, QComboBox, QSpinBox {background:#caeace; min-height:21px;} QLineEdit:disabled, QComboBox:disabled, QSpinBox:disabled {background:#b1ceed;}"));
    Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); } setProperty("plcConnections", connections.toVariantList());
    tabs->setObjectName(QStringLiteral("HistoryTabs")); tabs->addTab(channelPage(), QStringLiteral("一般")); tabs->addTab(displayPage(), QStringLiteral("XY轴"));
    m_visibility = new HmiPermissionEditor(QStringLiteral("显现"), QStringLiteral("HistoryVisible"), m_draft.visibility, false, permission); m_visibility->configureUnchosenBitState(m_draft.visibilityBitChosen); permissionLayout->addWidget(m_visibility); permissionLayout->addStretch(); tabs->addTab(permission, QStringLiteral("权限设置")); layout->addWidget(tabs, 0, 0); layout->setColumnStretch(0, 1);
    m_preview = new QLabel(preview); m_preview->setObjectName(QStringLiteral("HistoryPreview")); m_preview->setFixedSize(130, 155); previewLayout->addWidget(m_preview); layout->addWidget(preview, 0, 1, Qt::AlignTop);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("HistoryButtons")); buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); layout->addWidget(buttons, 1, 0, 1, 2);
    connect(buttons, &QDialogButtonBox::accepted, this, &HistoryTrendDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject); connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, QStringLiteral("历史趋势图帮助"), QStringLiteral("选择数据采集和逐曲线数据来源，配置独立范围、线型、颜色与时间轴。预览使用固定示意数据。")); }); refresh();
}

/******************************************************************************
 * Function Name   : HistoryTrendDialog::textField
 * Description     : Bind a detached string and optional full-span word-address picker.
 * Input Parameters: layout, row, label, name, value, addressRole
 * Return Value    : Bound line edit.
 * Date            : 2026.11.12
 ******************************************************************************/
QLineEdit *HistoryTrendDialog::textField(QGridLayout *layout, int row, const QString &label, const QString &name, QString *value, int addressRole) //20261112-1 add
{
    QLineEdit *edit = new QLineEdit(*value, this);
    Q_UNUSED(addressRole);
    edit->setMaximumWidth(addressRole ? 210 : 140); edit->setObjectName(name); edit->setMaxLength(addressRole ? 1024 : 80); layout->addWidget(new QLabel(label, this), row, 0); layout->addWidget(edit, row, 1);
    connect(edit, &QLineEdit::textChanged, this, [this, value](const QString &text) { *value = text; refresh(); });
    return edit;
}


/******************************************************************************
 * Function Name   : HistoryTrendDialog::choice
 * Description     : Bind a measured enumeration without replacing unrelated draft fields.
 * Input Parameters: layout, row, label, name, items, value, offset
 * Return Value    : Bound combo box.
 * Date            : 2026.11.12
 ******************************************************************************/
QComboBox *HistoryTrendDialog::choice(QGridLayout *layout, int row, const QString &label, const QString &name, const QStringList &items, int *value, int offset) //20261112-1 add
{
    QComboBox *combo = new QComboBox(this);
    combo->setMaximumWidth(230); combo->setObjectName(name); combo->addItems(items); combo->setCurrentIndex(*value - offset); layout->addWidget(new QLabel(label, this), row, 0); layout->addWidget(combo, row, 1);
    connect(combo, &QComboBox::currentIndexChanged, this, [this, value, offset](int index) { *value = index + offset; refresh(); }); return combo;
}


/******************************************************************************
 * Function Name   : HistoryTrendDialog::toggle
 * Description     : Bind a persistent checkbox to its detached boolean.
 * Input Parameters: layout, row, label, name, value
 * Return Value    : Bound checkbox.
 * Date            : 2026.11.12
 ******************************************************************************/
QCheckBox *HistoryTrendDialog::toggle(QGridLayout *layout, int row, const QString &label, const QString &name, bool *value) //20261112-1 add
{
    QCheckBox *check = new QCheckBox(label, this);
    check->setObjectName(name); check->setChecked(*value); layout->addWidget(check, row, 0, 1, 2); connect(check, &QCheckBox::toggled, this, [this, value](bool checked) { *value = checked; refresh(); }); return check;
}


/******************************************************************************
 * Function Name   : HistoryTrendDialog::integer
 * Description     : Bind bounded editor integers with explicit Lite limits.
 * Input Parameters: layout, row, label, name, value, minimum, maximum
 * Return Value    : Bounded spin editor.
 * Date            : 2026.11.12
 ******************************************************************************/
QSpinBox *HistoryTrendDialog::integer(QGridLayout *layout, int row, const QString &label, const QString &name, int *value, int minimum, int maximum) //20261112-1 add
{
    QSpinBox *spin = new QSpinBox(this);
    spin->setObjectName(name); spin->setRange(minimum, maximum); spin->setValue(*value); layout->addWidget(new QLabel(label, this), row, 0); layout->addWidget(spin, row, 1); connect(spin, &QSpinBox::valueChanged, this, [this, value](int number) { *value = number; refresh(); });
    return spin;
}


/******************************************************************************
 * Function Name   : HistoryTrendDialog::color
 * Description     : Publish a shared color selection only when the nested dialog is accepted.
 * Input Parameters: layout, row, label, name, value
 * Return Value    : Detached color picker button.
 * Date            : 2026.11.12
 ******************************************************************************/
QPushButton *HistoryTrendDialog::color(QGridLayout *layout, int row, const QString &label, const QString &name, QString *value) //20261112-1 add
{
    QPushButton *button = new QPushButton(this);
    button->setObjectName(name); button->setMinimumSize(110, 25); button->setStyleSheet(QStringLiteral("background:%1").arg(*value)); layout->addWidget(new QLabel(label, this), row, 0); layout->addWidget(button, row, 1);
    connect(button, &QPushButton::clicked, this, [this, button, value]() { QColor selected = QColorDialog::getColor(QColor(*value), this, QStringLiteral("选择颜色"), QColorDialog::DontUseNativeDialog); if (selected.isValid()) { *value = selected.name(); button->setStyleSheet(QStringLiteral("background:%1").arg(*value)); refresh(); } });
    return button;
}

