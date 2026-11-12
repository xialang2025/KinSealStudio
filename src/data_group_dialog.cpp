#include "data_group_dialog.h"
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
 * Function Name   : DataGroupDialog::channelPage
 * Description     : Keep exactly one curve and preserve the sampling-address draft while its mode is inactive.
 * Input Parameters: None
 * Return Value    : General data-group page.
 * Date            : 2026.11.13
 ******************************************************************************/
QWidget *DataGroupDialog::channelPage() //20261113-1 add
{
    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QGroupBox *addresses = new QGroupBox(QStringLiteral("地址"), page);
    QGroupBox *sampling = new QGroupBox(page);
    QGroupBox *channel = new QGroupBox(QStringLiteral("通道设置"), page);
    QGroupBox *fill = new QGroupBox(QStringLiteral("填充"), page);
    QGridLayout *grid = new QGridLayout(addresses);
    QComboBox *sampleMode;
    QLineEdit *sampleAddress;
    QPushButton *samplePicker;
    QSpinBox *count = new QSpinBox(channel);
    QComboBox *current = new QComboBox(channel);
    layout->setContentsMargins(12, 16, 16, 26); layout->addWidget(addresses, 0, 0); layout->addWidget(sampling, 1, 0); layout->addWidget(channel, 0, 1); layout->addWidget(fill, 1, 1); layout->setRowStretch(2, 1);
    textField(grid, 0, QStringLiteral("触发地址:"), QStringLiteral("GroupTriggerAddress"), &m_draft.triggerAddress, 1); textField(grid, 1, QStringLiteral("读取地址:"), QStringLiteral("GroupReadAddress"), &m_draft.readAddress, 2);
    grid = new QGridLayout(sampling); choice(grid, 0, QStringLiteral("数据类型:"), QStringLiteral("GroupDataType"), DataGroupProperties::dataTypes(), &m_draft.dataType); sampleMode = choice(grid, 1, QStringLiteral("采样方式:"), QStringLiteral("GroupSampleMode"), {QStringLiteral("常量"), QStringLiteral("地址")}, &m_draft.sampleMode); integer(grid, 2, QStringLiteral("采样点数:"), QStringLiteral("GroupSamplePoints"), &m_draft.samplePoints, 10, 800); sampleAddress = textField(grid, 3, QStringLiteral("采样地址:"), QStringLiteral("GroupSampleAddress"), &m_draft.sampleAddress, 3); samplePicker = findChild<QPushButton *>(QStringLiteral("GroupSampleAddressPicker")); sampleAddress->setEnabled(m_draft.sampleMode == 1); samplePicker->setEnabled(m_draft.sampleMode == 1); connect(sampleMode, &QComboBox::currentIndexChanged, this, [sampleAddress, samplePicker](int mode) { sampleAddress->setEnabled(mode == 1); samplePicker->setEnabled(mode == 1); });
    grid = new QGridLayout(channel); count->setObjectName(QStringLiteral("GroupChannelCount")); count->setRange(1, 1); current->setObjectName(QStringLiteral("GroupCurrentChannel")); current->addItem(QStringLiteral("1")); grid->addWidget(new QLabel(QStringLiteral("通道总数:"), channel), 0, 0); grid->addWidget(count, 0, 1); grid->addWidget(new QLabel(QStringLiteral("当前通道:"), channel), 1, 0); grid->addWidget(current, 1, 1); choice(grid, 2, QStringLiteral("线型:"), QStringLiteral("GroupLineStyle"), {QStringLiteral("—0"), QStringLiteral("--1"), QStringLiteral("--2"), QStringLiteral("--3"), QStringLiteral("--4")}, &m_draft.lineStyle); color(grid, 3, QStringLiteral("显示颜色:"), QStringLiteral("GroupChannelColor"), &m_draft.channelColor);
    grid = new QGridLayout(fill); color(grid, 0, QStringLiteral("底色:"), QStringLiteral("GroupBottomColor"), &m_draft.bottomColor); color(grid, 1, QStringLiteral("边框色:"), QStringLiteral("GroupFrameColor"), &m_draft.frameColor); color(grid, 2, QStringLiteral("背景色:"), QStringLiteral("GroupBackgroundColor"), &m_draft.backgroundColor); return page;

}

/******************************************************************************
 * Function Name   : DataGroupDialog::axisPage
 * Description     : Expose only the X font and use a constant minimum with an exclusive constant/address maximum source.
 * Input Parameters: y
 * Return Value    : Independent X or Y configuration group.
 * Date            : 2026.11.13
 ******************************************************************************/
QWidget *DataGroupDialog::axisPage(bool y) //20261113-1 add
{
    TrendAxis *axis = y ? &m_draft.yAxis : &m_draft.xAxis;
    QString prefix = y ? QStringLiteral("GroupY") : QStringLiteral("GroupX");
    QGroupBox *page = new QGroupBox(y ? QStringLiteral("Y轴") : QStringLiteral("X轴"), this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    QGridLayout *grid = new QGridLayout;
    QWidget *axisColor = new QWidget(page);
    QGridLayout *colorGrid = new QGridLayout(axisColor);
    QWidget *gridColor = new QWidget(page);
    QGridLayout *gridColorGrid = new QGridLayout(gridColor);
    QWidget *marks = new QWidget(page);
    QGridLayout *markGrid = new QGridLayout(marks);
    QCheckBox *ticks = toggle(grid, 0, QStringLiteral("显示刻度"), prefix + QStringLiteral("Ticks"), &axis->ticks);
    QCheckBox *gridlines = toggle(grid, 1, y ? QStringLiteral("显示X轴曲线") : QStringLiteral("显示Y轴曲线"), prefix + QStringLiteral("Grid"), &axis->grid);
    QCheckBox *showMarks;
    QComboBox *font = new QComboBox(marks);
    QComboBox *source;
    QWidget *constant;
    QWidget *address;
    QGridLayout *sub;
    color(colorGrid, 0, QStringLiteral("轴/刻度颜色:"), prefix + QStringLiteral("AxisColor"), &axis->axisColor); axisColor->setVisible(axis->ticks); layout->addLayout(grid); layout->addWidget(axisColor); connect(ticks, &QCheckBox::toggled, axisColor, &QWidget::setVisible);
    color(gridColorGrid, 0, QStringLiteral("格线颜色:"), prefix + QStringLiteral("GridColor"), &axis->gridColor); gridColor->setVisible(axis->grid); layout->addWidget(gridColor); connect(gridlines, &QCheckBox::toggled, gridColor, &QWidget::setVisible);
    grid = new QGridLayout; integer(grid, 0, QStringLiteral("主分隔数:"), prefix + QStringLiteral("Major"), &axis->major, 3, 10); integer(grid, 1, QStringLiteral("次分隔数:"), prefix + QStringLiteral("Minor"), &axis->minor, 1, 3); showMarks = toggle(grid, 2, QStringLiteral("显示刻度标示"), prefix + QStringLiteral("Marks"), &axis->marks); layout->addLayout(grid);
    font->setObjectName(prefix + QStringLiteral("FontSize")); font->addItems({QStringLiteral("12"), QStringLiteral("16"), QStringLiteral("24"), QStringLiteral("48")}); font->setCurrentText(QString::number(axis->fontSize)); markGrid->addWidget(new QLabel(QStringLiteral("字体大小:"), marks), 0, 0); markGrid->addWidget(font, 0, 1); connect(font, &QComboBox::currentTextChanged, this, [this, axis](const QString &value) { axis->fontSize = value.toInt(); refresh(); });
    if (y)
    {
        source = choice(markGrid, 1, QStringLiteral("范围类型:"), QStringLiteral("GroupRangeSource"), {QStringLiteral("常量"), QStringLiteral("地址")}, &m_draft.rangeSource); textField(markGrid, 2, QStringLiteral("最小值:"), QStringLiteral("GroupYMinimum"), &m_draft.yMinimum);
        constant = new QWidget(marks); sub = new QGridLayout(constant); sub->setContentsMargins(0, 0, 0, 0); textField(sub, 0, QStringLiteral("最大值:"), QStringLiteral("GroupYMaximum"), &m_draft.yMaximum); markGrid->addWidget(constant, 3, 0, 1, 3);
        address = new QWidget(marks); sub = new QGridLayout(address); sub->setContentsMargins(0, 0, 0, 0); textField(sub, 0, QStringLiteral("最大值:"), QStringLiteral("GroupMaximumAddress"), &m_draft.maximumAddress, 4); markGrid->addWidget(address, 3, 0, 1, 3); constant->setVisible(m_draft.rangeSource == 0); address->setVisible(m_draft.rangeSource == 1); connect(source, &QComboBox::currentIndexChanged, this, [constant, address](int mode) { constant->setVisible(mode == 0); address->setVisible(mode == 1); });
        integer(markGrid, 4, QStringLiteral("总位数:"), QStringLiteral("GroupTotalDigits"), &m_draft.totalDigits, 1, 10); integer(markGrid, 5, QStringLiteral("小数位数:"), QStringLiteral("GroupFractionalDigits"), &m_draft.fractionalDigits, 0, 3);
    }
    marks->setVisible(axis->marks); layout->addWidget(marks); connect(showMarks, &QCheckBox::toggled, marks, &QWidget::setVisible); layout->addStretch(); return page;

}

/******************************************************************************
 * Function Name   : DataGroupDialog::displayPage
 * Description     : Show independent X and Y axis forms without exposing native hidden X numeric fields.
 * Input Parameters: None
 * Return Value    : Display configuration page.
 * Date            : 2026.11.13
 ******************************************************************************/
QWidget *DataGroupDialog::displayPage() //20261113-1 add
{
    QWidget *page = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(page);
    layout->setContentsMargins(14, 18, 14, 20); layout->addWidget(axisPage(false)); layout->addWidget(axisPage(true)); return page;
}

/******************************************************************************
 * Function Name   : DataGroupDialog::object
 * Description     : Collect the detached trend and display-only permissions without modifying live geometry.
 * Input Parameters: None
 * Return Value    : Complete typed trend object.
 * Date            : 2026.11.13
 ******************************************************************************/
HmiObject DataGroupDialog::object() const //20261113-1 add
{
    HmiObject object = m_original;
    object.type = QStringLiteral("datagroup"); object.dataGroup = m_draft; object.address.clear(); object.text.clear(); object.hasBitButtonProperties = false; object.hasStaticTextProperties = false; object.hasNumberDisplayProperties = false; object.hasImageDisplayProperties = false; object.dataGroup.visibility = m_visibility->properties(); object.dataGroup.visibilityBitChosen = m_visibility->bitStateChosen() || m_accepted; return object;
}

/******************************************************************************
 * Function Name   : DataGroupDialog::refresh
 * Description     : Share a static curve renderer with the canvas and tolerate construction-time field signals.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void DataGroupDialog::refresh() //20261113-1 add
{
    QImage image(260, 310, QImage::Format_ARGB32);
    QPainter painter;
    if (m_preview == nullptr) { return; } image.fill(QColor(QStringLiteral("#f0f0f0"))); painter.begin(&image); paintDataGroup(&painter, QRectF(10, 10, 240, 290), m_draft); painter.end(); m_preview->setPixmap(QPixmap::fromImage(image).scaled(130, 155, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

/******************************************************************************
 * Function Name   : DataGroupDialog::accept
 * Description     : Validate all active scalar and carrier spans before accepting one editor transaction.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void DataGroupDialog::accept() //20261113-1 add
{
    HmiObject candidate;
    QString error;
    m_accepted = true; candidate = object();
    if (!candidate.dataGroup.validate(&error) || !candidate.dataGroup.validateAddresses(m_project.connections, &error)) { m_accepted = false; QMessageBox::warning(this, QStringLiteral("数据群组属性未提交"), error); return; }
    if (m_draft.rangeSource == 1) { m_draft.yMinimum = QStringLiteral("0"); m_draft.yMaximum = QStringLiteral("1000"); } //20261113-1 add: The native address-range record retains its address and restores numeric drafts on property reopen.
    QDialog::accept();
}

/******************************************************************************
 * Function Name   : DataGroupDialog::DataGroupDialog
 * Description     : Build three detached native pages and reset the editor channel selection to the first channel.
 * Input Parameters: object, project, parent
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
DataGroupDialog::DataGroupDialog(const HmiObject &object, const HmiProject &project, QWidget *parent) //20261113-1 add
    : QDialog(parent), m_original(object), m_project(project), m_draft(object.dataGroup)
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
    if (m_draft.rangeSource == 1) { m_draft.yMinimum = QStringLiteral("0"); m_draft.yMaximum = QStringLiteral("1000"); } //20261113-1 add: Match the measured native reopen state while preserving same-session mode-switch drafts.
    setObjectName(QStringLiteral("DataGroupDialog")); setWindowTitle(QStringLiteral("数据群组")); resize(795, 618);
    setStyleSheet(QStringLiteral("QLineEdit, QComboBox, QSpinBox {background:#caeace; min-height:21px;} QLineEdit:disabled, QComboBox:disabled, QSpinBox:disabled {background:#b1ceed;}"));
    Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); } setProperty("plcConnections", connections.toVariantList());
    tabs->setObjectName(QStringLiteral("GroupTabs")); tabs->addTab(channelPage(), QStringLiteral("通道设置")); tabs->addTab(displayPage(), QStringLiteral("显示设置"));
    m_visibility = new HmiPermissionEditor(QStringLiteral("显现"), QStringLiteral("GroupVisible"), m_draft.visibility, false, permission); m_visibility->configureUnchosenBitState(m_draft.visibilityBitChosen); permissionLayout->addWidget(m_visibility); permissionLayout->addStretch(); tabs->addTab(permission, QStringLiteral("权限设置")); layout->addWidget(tabs, 0, 0); layout->setColumnStretch(0, 1);
    m_preview = new QLabel(preview); m_preview->setObjectName(QStringLiteral("GroupPreview")); m_preview->setFixedSize(130, 155); previewLayout->addWidget(m_preview); layout->addWidget(preview, 0, 1, Qt::AlignTop);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("GroupButtons")); buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); layout->addWidget(buttons, 1, 0, 1, 2);
    connect(buttons, &QDialogButtonBox::accepted, this, &DataGroupDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject); connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, QStringLiteral("数据群组帮助"), QStringLiteral("配置单通道的读取缓冲区、常量/地址采样以及Y轴最大值来源。预览使用固定示意数据。")); }); refresh();
}

/******************************************************************************
 * Function Name   : DataGroupDialog::textField
 * Description     : Bind a detached string and optional full-span word-address picker.
 * Input Parameters: layout, row, label, name, value, addressRole
 * Return Value    : Bound line edit.
 * Date            : 2026.11.13
 ******************************************************************************/
QLineEdit *DataGroupDialog::textField(QGridLayout *layout, int row, const QString &label, const QString &name, QString *value, int addressRole) //20261113-1 add
{
    QLineEdit *edit = new QLineEdit(*value, this);
    QPushButton *picker;
    edit->setMaximumWidth(addressRole ? 210 : 140); edit->setObjectName(name); edit->setMaxLength(addressRole ? 1024 : 80); layout->addWidget(new QLabel(label, this), row, 0); layout->addWidget(edit, row, 1);
    connect(edit, &QLineEdit::textChanged, this, [this, value](const QString &text) { *value = text; refresh(); });
    if (addressRole != 0) { picker = new QPushButton(QStringLiteral("…"), this); picker->setFixedWidth(26); picker->setObjectName(name + QStringLiteral("Picker")); layout->addWidget(picker, row, 2); connect(picker, &QPushButton::clicked, this, [this, edit, addressRole]() { u32 count = addressRole == 2 ? m_draft.readWordSpan() : addressRole == 4 ? DataGroupProperties::wordCount(m_draft.dataType) : 1U; editHmiAddress(this, edit, addressRole == 1, int(count)); }); }
    return edit;
}


/******************************************************************************
 * Function Name   : DataGroupDialog::choice
 * Description     : Bind a measured enumeration without replacing unrelated draft fields.
 * Input Parameters: layout, row, label, name, items, value, offset
 * Return Value    : Bound combo box.
 * Date            : 2026.11.13
 ******************************************************************************/
QComboBox *DataGroupDialog::choice(QGridLayout *layout, int row, const QString &label, const QString &name, const QStringList &items, int *value, int offset) //20261113-1 add
{
    QComboBox *combo = new QComboBox(this);
    combo->setMaximumWidth(230); combo->setObjectName(name); combo->addItems(items); combo->setCurrentIndex(*value - offset); layout->addWidget(new QLabel(label, this), row, 0); layout->addWidget(combo, row, 1);
    connect(combo, &QComboBox::currentIndexChanged, this, [this, value, offset](int index) { *value = index + offset; refresh(); }); return combo;
}


/******************************************************************************
 * Function Name   : DataGroupDialog::toggle
 * Description     : Bind a persistent checkbox to its detached boolean.
 * Input Parameters: layout, row, label, name, value
 * Return Value    : Bound checkbox.
 * Date            : 2026.11.13
 ******************************************************************************/
QCheckBox *DataGroupDialog::toggle(QGridLayout *layout, int row, const QString &label, const QString &name, bool *value) //20261113-1 add
{
    QCheckBox *check = new QCheckBox(label, this);
    check->setObjectName(name); check->setChecked(*value); layout->addWidget(check, row, 0, 1, 2); connect(check, &QCheckBox::toggled, this, [this, value](bool checked) { *value = checked; refresh(); }); return check;
}


/******************************************************************************
 * Function Name   : DataGroupDialog::integer
 * Description     : Bind bounded editor integers with explicit Lite limits.
 * Input Parameters: layout, row, label, name, value, minimum, maximum
 * Return Value    : Bounded spin editor.
 * Date            : 2026.11.13
 ******************************************************************************/
QSpinBox *DataGroupDialog::integer(QGridLayout *layout, int row, const QString &label, const QString &name, int *value, int minimum, int maximum) //20261113-1 add
{
    QSpinBox *spin = new QSpinBox(this);
    spin->setObjectName(name); spin->setRange(minimum, maximum); spin->setValue(*value); layout->addWidget(new QLabel(label, this), row, 0); layout->addWidget(spin, row, 1); connect(spin, &QSpinBox::valueChanged, this, [this, value](int number) { *value = number; refresh(); });
    return spin;
}


/******************************************************************************
 * Function Name   : DataGroupDialog::color
 * Description     : Publish a shared color selection only when the nested dialog is accepted.
 * Input Parameters: layout, row, label, name, value
 * Return Value    : Detached color picker button.
 * Date            : 2026.11.13
 ******************************************************************************/
QPushButton *DataGroupDialog::color(QGridLayout *layout, int row, const QString &label, const QString &name, QString *value) //20261113-1 add
{
    QPushButton *button = new QPushButton(this);
    button->setObjectName(name); button->setMinimumSize(110, 25); button->setStyleSheet(QStringLiteral("background:%1").arg(*value)); layout->addWidget(new QLabel(label, this), row, 0); layout->addWidget(button, row, 1);
    connect(button, &QPushButton::clicked, this, [this, button, value]() { QColor selected = QColorDialog::getColor(QColor(*value), this, QStringLiteral("选择颜色"), QColorDialog::DontUseNativeDialog); if (selected.isValid()) { *value = selected.name(); button->setStyleSheet(QStringLiteral("background:%1").arg(*value)); refresh(); } });
    return button;
}

/******************************************************************************
 * Function Name   : paintDataGroup
 * Description     : Share a fixed editor illustration while preserving outer/plot fill ownership and dormant-range drafts.
 * Input Parameters: painter, rect, properties
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void paintDataGroup(QPainter *painter, const QRectF &rect, const DataGroupProperties &properties) //20261113-1 add
{
    TrendProperties display;
    bool minimumOk;
    bool maximumOk;
    double minimum = properties.yMinimum.toDouble(&minimumOk);
    double maximum = properties.yMaximum.toDouble(&maximumOk);
    display.samplePoints = properties.samplePoints; display.backgroundColor = properties.bottomColor; display.bottomColor = properties.backgroundColor; display.frameColor = properties.frameColor; display.channels[0].color = properties.channelColor; display.channels[0].lineStyle = properties.lineStyle; display.xAxis = properties.xAxis; display.yAxis = properties.yAxis; display.totalDigits = properties.totalDigits; display.fractionalDigits = properties.fractionalDigits;
    if (minimumOk && maximumOk && std::isfinite(minimum) && std::isfinite(maximum) && minimum < maximum) { display.yMinimum = properties.yMinimum; display.yMaximum = properties.yMaximum; }
    paintTrend(painter, rect, display);

}

