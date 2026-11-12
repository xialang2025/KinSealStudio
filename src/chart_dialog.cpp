#include "chart_dialog.h"
#include "hmi_property_widgets.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
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
#include <QStackedWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <cmath>

/******************************************************************************
 * Function Name   : paintChart
 * Description     : Render a clipped static editor sample; no PLC value or device behavior is simulated.
 * Input Parameters: painter, rect, properties
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void paintChart(QPainter *painter, const QRectF &rect, const ChartProperties &properties) //20261112-1 add
{
    ChartBarProperties bar = properties.bar;
    ChartMeterProperties meter = properties.meter;
    ChartDonutProperties donut = properties.donut;
    QRectF fill = rect;
    QRectF circle;
    QLinearGradient gradient;
    QRadialGradient radial;
    QBrush brush;
    QPainterPath pointer;
    QFont font;
    QColor line;
    int index;
    int divisions;
    double angle;
    double radius;
    double start;
    double span;
    painter->save(); painter->setClipRect(rect); painter->setRenderHint(QPainter::Antialiasing);
    if (properties.shape == 0)
    {
        painter->fillRect(rect, QColor(bar.background));
        if (bar.direction == 0) { fill.setTop(rect.center().y()); } else if (bar.direction == 1) { fill.setBottom(rect.center().y()); } else if (bar.direction == 2) { fill.setRight(rect.center().x()); } else { fill.setLeft(rect.center().x()); }
        brush = QColor(bar.foreground);
        if (bar.pattern == 0) { brush = Qt::NoBrush; }
        else if (bar.pattern >= 10) { radial = QRadialGradient(bar.pattern == 10 ? fill.topRight() : bar.pattern == 11 ? fill.topLeft() : fill.center(), qMax(fill.width(), fill.height())); radial.setColorAt(0, QColor(bar.foreground)); radial.setColorAt(1, QColor(bar.patternColor)); brush = radial; }
        else if (bar.pattern >= 2)
        {
            gradient = QLinearGradient(fill.topLeft(), bar.pattern < 4 ? fill.topRight() : bar.pattern < 6 ? fill.bottomLeft() : bar.pattern < 8 ? fill.bottomRight() : fill.topRight());
            if (bar.pattern >= 8) { gradient.setStart(fill.bottomLeft()); }
            gradient.setColorAt(0, QColor(bar.foreground)); gradient.setColorAt(1, QColor(bar.pattern % 2 ? bar.foreground : bar.patternColor));
            if (bar.pattern % 2) { gradient.setColorAt(.5, QColor(bar.patternColor)); } brush = gradient;
        }
        painter->fillRect(fill, brush);
    }
    else if (properties.shape == 1)
    {
        radius = qMin(rect.width(), rect.height()) * .30; circle = QRectF(rect.center().x() - radius, rect.center().y() - radius, radius * 2, radius * 2);
        if (!meter.transparent) { painter->setPen(Qt::NoPen); painter->setBrush(QColor(meter.background)); painter->drawEllipse(circle); }
        painter->setBrush(Qt::NoBrush); painter->setPen(QColor(meter.scaleColor)); start = meter.shape == 2 ? -90 : meter.shape == 0 ? 180 : meter.shape == 3 ? 225 : 90; span = meter.shape == 0 ? 180 : meter.shape == 3 ? 270 : 360;
        if (meter.showAxis) { painter->drawArc(circle, int(start * 16), int(-span * 16)); }
        font = painter->font(); font.setPixelSize(meter.fontSize); painter->setFont(font); divisions = meter.mainDivisions * (meter.showSubScale ? meter.subDivisions : 1);
        for (index = 0; index <= divisions; ++index)
        {
            if (index == divisions && span == 360) { continue; } // Skip the duplicate full-circle endpoint label.
            angle = (start - span * index / divisions) * 3.141592653589793 / 180; painter->save(); painter->translate(rect.center()); painter->rotate(-angle * 180 / 3.141592653589793); painter->drawLine(QPointF(radius * .88, 0), QPointF(radius, 0)); painter->restore();
            if (meter.showValues && index % (meter.showSubScale ? meter.subDivisions : 1) == 0) { painter->drawText(QRectF(rect.center().x() + std::cos(angle) * radius * 1.4 - 23, rect.center().y() - std::sin(angle) * radius * 1.4 - meter.fontSize, 46, meter.fontSize * 2), Qt::AlignCenter, QString::number(meter.minimum.toDouble() + (meter.maximum.toDouble() - meter.minimum.toDouble()) * index / divisions, 'g', 5)); }
        }
        painter->translate(rect.center()); painter->setPen(QPen(QColor(meter.foreground), meter.pointer == 0 ? 1 : 3)); painter->setBrush(QColor(meter.foreground));
        if (meter.pointer < 2) { painter->drawLine(QPointF(0, 0), QPointF(0, -radius * .85)); }
        else { pointer.moveTo(0, -radius * .85); pointer.lineTo(radius * .08, 0); if (meter.pointer == 2) { pointer.lineTo(0, radius * .17); } pointer.lineTo(-radius * .08, 0); pointer.closeSubpath(); painter->drawPath(pointer); }
    }
    else if (donut.lineType != 0)
    {
        line = QColor(donut.lineColor); line.setAlpha(donut.opacity); radius = qMax(0.0, qMin(rect.width(), rect.height()) * .30 - qMax(donut.lineWidth, donut.fillWidth) / 2.0); circle = QRectF(rect.center().x() - radius, rect.center().y() - radius, radius * 2, radius * 2);
        painter->setBrush(Qt::NoBrush); painter->setPen(QPen(line, donut.lineWidth, donut.lineType == 1 ? Qt::SolidLine : donut.lineType == 2 ? Qt::DashLine : donut.lineType == 3 ? Qt::DotLine : donut.lineType == 4 ? Qt::DashDotLine : Qt::DashDotDotLine));
        if (donut.arc) { painter->drawArc(circle, int(std::fmod(donut.startAngle.toDouble(), 360) * 16), int(qBound(-360.0, donut.spanAngle.toDouble(), 360.0) * 16)); } else { painter->drawEllipse(circle); }
    }
    painter->restore();
}

/******************************************************************************
 * Function Name   : ChartDialog::ChartDialog
 * Description     : Construct the measured basic, appearance and display-only permission pages.
 * Input Parameters: object, project, parent
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
ChartDialog::ChartDialog(const HmiObject &object, const HmiProject &project, QWidget *parent) //20261112-1 add
    : QDialog(parent), m_original(object), m_project(project), m_draft(object.chart)
{
    QGridLayout *layout = new QGridLayout(this);
    QWidget *basic = new QWidget(this);
    QVBoxLayout *basicLayout = new QVBoxLayout(basic);
    QHBoxLayout *radios = new QHBoxLayout;
    QButtonGroup *group = new QButtonGroup(this);
    QGroupBox *preview = new QGroupBox(QStringLiteral("预览"), this);
    QVBoxLayout *previewLayout = new QVBoxLayout(preview);
    QWidget *permission = new QWidget(this);
    QVBoxLayout *permissionLayout = new QVBoxLayout(permission);
    QDialogButtonBox *buttons;
    QJsonArray connections;
    PlcConnection connection;
    QStringList names = {QStringLiteral("柱状图"), QStringLiteral("仪表"), QStringLiteral("圆环")};
    int index;
    setObjectName(QStringLiteral("ChartDialog")); setWindowTitle(QStringLiteral("图表")); resize(825, 617);
    setStyleSheet(QStringLiteral("QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {background:#caeace; min-height:20px;} QLineEdit:disabled, QComboBox:disabled, QSpinBox:disabled, QDoubleSpinBox:disabled {background:#b1ceed;}"));
    Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); } setProperty("plcConnections", connections.toVariantList());
    m_tabs = new QTabWidget(this); m_tabs->setObjectName(QStringLiteral("ChartTabs")); m_basic = new QStackedWidget(this); m_appearance = new QStackedWidget(this);
    for (index = 0; index < 3; ++index) { m_shapes[index] = new QRadioButton(names.at(index), basic); m_shapes[index]->setObjectName(QStringLiteral("ChartShape%1").arg(index)); group->addButton(m_shapes[index], index); radios->addWidget(m_shapes[index]); m_basic->addWidget(basicPage(index)); }
    basicLayout->setContentsMargins(12, 35, 12, 12); radios->setContentsMargins(24, 0, 24, 0); basicLayout->addLayout(radios); basicLayout->addSpacing(18); basicLayout->addWidget(m_basic); m_appearance->addWidget(appearancePage(0)); m_appearance->addWidget(appearancePage(1));
    m_visibility = new HmiPermissionEditor(QStringLiteral("显现"), QStringLiteral("ChartVisible"), m_draft.visibility, false, permission); m_visibility->configureUnchosenBitState(m_draft.visibilityBitChosen); permissionLayout->addWidget(m_visibility); permissionLayout->addStretch();
    m_tabs->addTab(basic, QStringLiteral("基本")); m_tabs->addTab(m_appearance, QStringLiteral("外形")); m_tabs->addTab(permission, QStringLiteral("权限设置")); layout->addWidget(m_tabs, 0, 0); layout->setColumnStretch(0, 1);
    m_preview = new QLabel(preview); m_preview->setObjectName(QStringLiteral("ChartPreview")); m_preview->setFixedSize(130, 120); previewLayout->addWidget(m_preview); layout->addWidget(preview, 0, 1, Qt::AlignTop);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("ChartButtons")); buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); layout->addWidget(buttons, 1, 0, 1, 2);
    connect(group, &QButtonGroup::idClicked, this, &ChartDialog::selectShape); connect(buttons, &QDialogButtonBox::accepted, this, &ChartDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, QStringLiteral("图表帮助"), QStringLiteral("选择柱状图、仪表或圆环，配置监控地址、范围和显现条件。当前预览是静态编辑样例；各形态草稿独立保存。")); }); selectShape(m_draft.shape);
}

/******************************************************************************
 * Function Name   : ChartDialog::textField
 * Description     : Bind a detached string and optional full-span word-address picker.
 * Input Parameters: layout, row, label, name, value, address, words
 * Return Value    : Bound line edit.
 * Date            : 2026.11.12
 ******************************************************************************/
QLineEdit *ChartDialog::textField(QGridLayout *layout, int row, const QString &label, const QString &name, QString *value, bool address, int words) //20261112-1 add
{
    QLineEdit *edit = new QLineEdit(*value, this);
    QPushButton *picker;
    edit->setMaximumWidth(address ? 210 : 140); edit->setProperty("chartWords", words); edit->setObjectName(name); edit->setMaxLength(address ? 1024 : 80); layout->addWidget(new QLabel(label, this), row, 0); layout->addWidget(edit, row, 1);
    connect(edit, &QLineEdit::textChanged, this, [this, value](const QString &text) { *value = text; refresh(); });
    if (address) { picker = new QPushButton(QStringLiteral("…"), this); picker->setFixedWidth(26); picker->setObjectName(name + QStringLiteral("Picker")); layout->addWidget(picker, row, 2); connect(picker, &QPushButton::clicked, this, [this, edit]() { editHmiAddress(this, edit, false, edit->property("chartWords").toInt()); }); }
    return edit;
}

/******************************************************************************
 * Function Name   : ChartDialog::choice
 * Description     : Bind a measured enumeration without replacing unrelated draft fields.
 * Input Parameters: layout, row, label, name, items, value, offset
 * Return Value    : Bound combo box.
 * Date            : 2026.11.12
 ******************************************************************************/
QComboBox *ChartDialog::choice(QGridLayout *layout, int row, const QString &label, const QString &name, const QStringList &items, int *value, int offset) //20261112-1 add
{
    QComboBox *combo = new QComboBox(this);
    combo->setMaximumWidth(230); combo->setObjectName(name); combo->addItems(items); combo->setCurrentIndex(*value - offset); layout->addWidget(new QLabel(label, this), row, 0); layout->addWidget(combo, row, 1);
    connect(combo, &QComboBox::currentIndexChanged, this, [this, value, offset](int index) { *value = index + offset; refresh(); }); return combo;
}

/******************************************************************************
 * Function Name   : ChartDialog::toggle
 * Description     : Bind a persistent checkbox to its detached boolean.
 * Input Parameters: layout, row, label, name, value
 * Return Value    : Bound checkbox.
 * Date            : 2026.11.12
 ******************************************************************************/
QCheckBox *ChartDialog::toggle(QGridLayout *layout, int row, const QString &label, const QString &name, bool *value) //20261112-1 add
{
    QCheckBox *check = new QCheckBox(label, this);
    check->setObjectName(name); check->setChecked(*value); layout->addWidget(check, row, 0, 1, 2); connect(check, &QCheckBox::toggled, this, [this, value](bool checked) { *value = checked; refresh(); }); return check;
}

/******************************************************************************
 * Function Name   : ChartDialog::number
 * Description     : Preserve native text entry, including precise unsigned 32-bit scalar strings.
 * Input Parameters: layout, row, label, name, value, type
 * Return Value    : Bound scalar editor.
 * Date            : 2026.11.12
 ******************************************************************************/
QLineEdit *ChartDialog::number(QGridLayout *layout, int row, const QString &label, const QString &name, QString *value, int type) //20261112-1 add
{
    QLineEdit *edit = textField(layout, row, label, name, value);
    Q_UNUSED(type);
    return edit;
}

/******************************************************************************
 * Function Name   : ChartDialog::integer
 * Description     : Bind bounded editor integers with explicit Lite limits.
 * Input Parameters: layout, row, label, name, value, minimum, maximum
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void ChartDialog::integer(QGridLayout *layout, int row, const QString &label, const QString &name, int *value, int minimum, int maximum) //20261112-1 add
{
    QSpinBox *spin = new QSpinBox(this);
    spin->setObjectName(name); spin->setRange(minimum, maximum); spin->setValue(*value); layout->addWidget(new QLabel(label, this), row, 0); layout->addWidget(spin, row, 1); connect(spin, &QSpinBox::valueChanged, this, [this, value](int number) { *value = number; refresh(); });
}

/******************************************************************************
 * Function Name   : ChartDialog::color
 * Description     : Publish a shared color selection only when the nested dialog is accepted.
 * Input Parameters: layout, row, label, name, value
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void ChartDialog::color(QGridLayout *layout, int row, const QString &label, const QString &name, QString *value) //20261112-1 add
{
    QPushButton *button = new QPushButton(this);
    button->setObjectName(name); button->setMinimumSize(110, 25); button->setStyleSheet(QStringLiteral("background:%1").arg(*value)); layout->addWidget(new QLabel(label, this), row, 0); layout->addWidget(button, row, 1);
    connect(button, &QPushButton::clicked, this, [this, button, value]() { QColor selected = QColorDialog::getColor(QColor(*value), this, QStringLiteral("选择颜色"), QColorDialog::DontUseNativeDialog); if (selected.isValid()) { *value = selected.name(); button->setStyleSheet(QStringLiteral("background:%1").arg(*value)); refresh(); } });
}

/******************************************************************************
 * Function Name   : ChartDialog::basicPage
 * Description     : Construct independent native form controls and their measured enable/visibility links.
 * Input Parameters: shape
 * Return Value    : Basic form page.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *ChartDialog::basicPage(int shape) //20261112-1 add
{
    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QGroupBox *data = new QGroupBox(QStringLiteral("数据属性"), page);
    QGroupBox *range = new QGroupBox(QStringLiteral("显示范围"), page);
    QGroupBox *source;
    QGroupBox *attributes;
    QGroupBox *fill;
    QGridLayout *grid = new QGridLayout(data);
    QGridLayout *rangeGrid = new QGridLayout(range);
    QComboBox *type;
    QComboBox *rangeType;
    QRadioButton *arc;
    QRadioButton *ringRadio;
    QButtonGroup *arcGroup;
    QLineEdit *minimumAddress;
    QLineEdit *maximumAddress;
    QWidget *ring;
    QWidget *arcPage;
    QGridLayout *ringGrid;
    QGridLayout *arcGrid;
    QGridLayout *minimumGrid;
    QGridLayout *maximumGrid;
    QStringList widths;
    int index;
    int *dataType = shape == 0 ? &m_draft.bar.dataType : shape == 1 ? &m_draft.meter.dataType : &m_draft.donut.dataType;
    QString *address = shape == 0 ? &m_draft.bar.address : shape == 1 ? &m_draft.meter.address : &m_draft.donut.address;
    QString prefix = shape == 0 ? QStringLiteral("ChartBar") : shape == 1 ? QStringLiteral("ChartMeter") : QStringLiteral("ChartDonut");
    layout->setContentsMargins(12, 18, 20, 18); layout->setHorizontalSpacing(20); layout->addWidget(data, 0, 0); layout->addWidget(range, 0, 1);
    type = choice(grid, 0, QStringLiteral("数据类型:"), prefix + QStringLiteral("DataType"), ChartProperties::dataTypes(), dataType);
    textField(grid, 1, QStringLiteral("监控字地址:"), prefix + QStringLiteral("Address"), address, true, ChartProperties::wordCount(*dataType));
    connect(type, &QComboBox::currentIndexChanged, this, [this, shape](int type) { changeType(shape, type); });
    if (shape == 0)
    {
        m_barMinimum = textField(rangeGrid, 0, QStringLiteral("最小值:"), QStringLiteral("ChartBarMinimum"), &m_draft.bar.minimum); m_barMaximum = textField(rangeGrid, 1, QStringLiteral("最大值:"), QStringLiteral("ChartBarMaximum"), &m_draft.bar.maximum);
        grid = new QGridLayout; layout->addLayout(grid, 1, 0, 1, 2); m_twoPoles = new QCheckBox(QStringLiteral("两极柱"), page); m_twoPoles->setObjectName(QStringLiteral("ChartTwoPoles")); m_twoPoles->setChecked(m_draft.bar.twoPoles); grid->addWidget(m_twoPoles, 0, 0); connect(m_twoPoles, &QCheckBox::toggled, this, [this](bool checked) { m_draft.bar.twoPoles = checked; }); m_midpoint = new QLineEdit(m_draft.bar.midpoint, page); m_midpoint->setObjectName(QStringLiteral("ChartMidpoint")); m_midpoint->setMaxLength(80); m_midpoint->setMaximumWidth(170); grid->addWidget(new QLabel(QStringLiteral("中点:"), page), 0, 1); grid->addWidget(m_midpoint, 0, 2); connect(m_midpoint, &QLineEdit::textChanged, this, [this](const QString &text) { m_draft.bar.midpoint = text; refresh(); }); m_midpoint->setEnabled(m_draft.bar.twoPoles);
        connect(m_twoPoles, &QCheckBox::toggled, this, [this](bool checked) { m_midpoint->setEnabled(checked); if (checked) { m_midpoint->setText(QString::number(std::trunc((m_draft.bar.minimum.toDouble() + m_draft.bar.maximum.toDouble()) / 2), 'g', 16)); } });
        source = new QGroupBox(QStringLiteral("源范围"), page); grid = new QGridLayout(source); rangeType = new QComboBox(source); rangeType->setObjectName(QStringLiteral("ChartRangeType")); rangeType->addItems({QStringLiteral("常量"), QStringLiteral("地址")}); rangeType->setCurrentIndex(m_draft.bar.dynamicRange ? 1 : 0); grid->addWidget(new QLabel(QStringLiteral("范围类型:"), source), 0, 0); grid->addWidget(rangeType, 0, 1);
        minimumGrid = new QGridLayout; maximumGrid = new QGridLayout; grid->addLayout(minimumGrid, 1, 0); grid->addLayout(maximumGrid, 1, 1);
        m_sourceMinimum = number(minimumGrid, 0, QStringLiteral("最小值:"), QStringLiteral("ChartSourceMinimum"), &m_draft.bar.sourceMinimum, *dataType); m_sourceMaximum = number(maximumGrid, 0, QStringLiteral("最大值:"), QStringLiteral("ChartSourceMaximum"), &m_draft.bar.sourceMaximum, *dataType);
        minimumAddress = textField(minimumGrid, 1, QString(), QStringLiteral("ChartMinimumAddress"), &m_draft.bar.minimumAddress, true, ChartProperties::wordCount(*dataType)); maximumAddress = textField(maximumGrid, 1, QString(), QStringLiteral("ChartMaximumAddress"), &m_draft.bar.maximumAddress, true, ChartProperties::wordCount(*dataType));
        minimumAddress->setEnabled(m_draft.bar.dynamicRange); maximumAddress->setEnabled(m_draft.bar.dynamicRange); findChild<QPushButton *>(QStringLiteral("ChartMinimumAddressPicker"))->setEnabled(m_draft.bar.dynamicRange); findChild<QPushButton *>(QStringLiteral("ChartMaximumAddressPicker"))->setEnabled(m_draft.bar.dynamicRange); m_sourceMinimum->setEnabled(!m_draft.bar.dynamicRange); m_sourceMaximum->setEnabled(!m_draft.bar.dynamicRange);
        connect(rangeType, &QComboBox::currentIndexChanged, this, [this, minimumAddress, maximumAddress](int index) { m_draft.bar.dynamicRange = index == 1; minimumAddress->setEnabled(index == 1); maximumAddress->setEnabled(index == 1); findChild<QPushButton *>(QStringLiteral("ChartMinimumAddressPicker"))->setEnabled(index == 1); findChild<QPushButton *>(QStringLiteral("ChartMaximumAddressPicker"))->setEnabled(index == 1); m_sourceMinimum->setEnabled(index == 0); m_sourceMaximum->setEnabled(index == 0); }); layout->addWidget(source, 2, 0, 1, 2);
    }
    else if (shape == 1)
    {
        m_meterMinimum = number(rangeGrid, 0, QStringLiteral("最小值:"), QStringLiteral("ChartMeterMinimum"), &m_draft.meter.minimum, *dataType); m_meterMaximum = number(rangeGrid, 1, QStringLiteral("最大值:"), QStringLiteral("ChartMeterMaximum"), &m_draft.meter.maximum, *dataType);
        source = new QGroupBox(QStringLiteral("其他"), page); grid = new QGridLayout(source); choice(grid, 0, QStringLiteral("显示方向:"), QStringLiteral("ChartMeterDirection"), {QStringLiteral("顺时针"), QStringLiteral("逆时针")}, &m_draft.meter.direction); toggle(grid, 1, QStringLiteral("背景透明"), QStringLiteral("ChartMeterTransparent"), &m_draft.meter.transparent); layout->addWidget(source, 1, 0, 1, 2); layout->setRowStretch(2, 1);
    }
    else
    {
        m_donutMinimum = number(rangeGrid, 0, QStringLiteral("最小值:"), QStringLiteral("ChartDonutMinimum"), &m_draft.donut.minimum, *dataType); m_donutMaximum = number(rangeGrid, 1, QStringLiteral("最大值:"), QStringLiteral("ChartDonutMaximum"), &m_draft.donut.maximum, *dataType);
        attributes = new QGroupBox(QStringLiteral("属性"), page); grid = new QGridLayout(attributes); arcGroup = new QButtonGroup(attributes); ringRadio = new QRadioButton(QStringLiteral("圆圆环"), attributes); arc = new QRadioButton(QStringLiteral("圆圆弧"), attributes); ringRadio->setObjectName(QStringLiteral("ChartRing")); arc->setObjectName(QStringLiteral("ChartArc")); arcGroup->addButton(ringRadio); arcGroup->addButton(arc); ringRadio->setChecked(!m_draft.donut.arc); arc->setChecked(m_draft.donut.arc); grid->addWidget(ringRadio, 0, 0); grid->addWidget(arc, 0, 1); m_donutAttributes = new QStackedWidget(attributes); ring = new QWidget(attributes); arcPage = new QWidget(attributes); ringGrid = new QGridLayout(ring); arcGrid = new QGridLayout(arcPage);
        for (index = 1; index <= 32; ++index) { widths.append(QString::number(index)); }
        choice(ringGrid, 0, QStringLiteral("线型:"), QStringLiteral("ChartLineType"), ChartProperties::lineTypes(), &m_draft.donut.lineType); choice(ringGrid, 1, QStringLiteral("线宽:"), QStringLiteral("ChartLineWidth"), widths, &m_draft.donut.lineWidth, 1); textField(ringGrid, 2, QStringLiteral("圆环半径:"), QStringLiteral("ChartRadius"), &m_draft.donut.radius); textField(arcGrid, 0, QStringLiteral("圆弧起始角:"), QStringLiteral("ChartStartAngle"), &m_draft.donut.startAngle); textField(arcGrid, 1, QStringLiteral("圆弧跨度:"), QStringLiteral("ChartSpanAngle"), &m_draft.donut.spanAngle);
        m_donutAttributes->addWidget(ring); m_donutAttributes->addWidget(arcPage); m_donutAttributes->setCurrentIndex(m_draft.donut.arc ? 1 : 0); grid->addWidget(m_donutAttributes, 1, 0, 1, 2); connect(arc, &QRadioButton::toggled, this, [this](bool checked) { m_draft.donut.arc = checked; m_donutAttributes->setCurrentIndex(checked ? 1 : 0); refresh(); }); layout->addWidget(attributes, 1, 0, 2, 1);
        source = new QGroupBox(QStringLiteral("透明度"), page); grid = new QGridLayout(source); integer(grid, 0, QStringLiteral("透明度:"), QStringLiteral("ChartOpacity"), &m_draft.donut.opacity, 0, 255); layout->addWidget(source, 1, 1);
        fill = new QGroupBox(QStringLiteral("填充"), page); grid = new QGridLayout(fill); color(grid, 0, QStringLiteral("线颜色:"), QStringLiteral("ChartDonutColor"), &m_draft.donut.lineColor); choice(grid, 1, QStringLiteral("线宽:"), QStringLiteral("ChartFillWidth"), widths, &m_draft.donut.fillWidth, 1); layout->addWidget(fill, 2, 1);
    }
    return page;
}

/******************************************************************************
 * Function Name   : ChartDialog::appearancePage
 * Description     : Keep bar pattern/direction separate from meter pointer, scale and background controls.
 * Input Parameters: shape
 * Return Value    : Appearance page.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *ChartDialog::appearancePage(int shape) //20261112-1 add
{
    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QGroupBox *colors = new QGroupBox(shape == 0 ? QStringLiteral("填充") : QStringLiteral("颜色"), page);
    QGroupBox *other = new QGroupBox(QStringLiteral("其他"), page);
    QGroupBox *scale;
    QGridLayout *grid = new QGridLayout(colors);
    QCheckBox *sub;
    layout->setContentsMargins(20, 24, 20, 20); layout->addWidget(colors, 0, 0); layout->addWidget(other, 0, 1);
    if (shape == 0)
    {
        color(grid, 0, QStringLiteral("柱状色:"), QStringLiteral("ChartBarColor"), &m_draft.bar.foreground); color(grid, 1, QStringLiteral("背景色:"), QStringLiteral("ChartBarBackground"), &m_draft.bar.background);
        grid = new QGridLayout(other); choice(grid, 0, QStringLiteral("图案:"), QStringLiteral("ChartPattern"), ChartProperties::patterns(), &m_draft.bar.pattern); color(grid, 1, QStringLiteral("图案色:"), QStringLiteral("ChartPatternColor"), &m_draft.bar.patternColor); choice(grid, 2, QStringLiteral("方向:"), QStringLiteral("ChartBarDirection"), ChartProperties::directions(), &m_draft.bar.direction); layout->setRowStretch(1, 1);
    }
    else
    {
        color(grid, 0, QStringLiteral("显示颜色:"), QStringLiteral("ChartMeterColor"), &m_draft.meter.foreground); color(grid, 1, QStringLiteral("背景颜色:"), QStringLiteral("ChartMeterBackground"), &m_draft.meter.background);
        grid = new QGridLayout(other); choice(grid, 0, QStringLiteral("指针类型:"), QStringLiteral("ChartPointer"), ChartProperties::pointers(), &m_draft.meter.pointer); choice(grid, 1, QStringLiteral("形状:"), QStringLiteral("ChartMeterShape"), ChartProperties::meterShapes(), &m_draft.meter.shape);
        scale = new QGroupBox(QStringLiteral("显示主刻度"), page); grid = new QGridLayout(scale); integer(grid, 0, QStringLiteral("主刻度分割数:"), QStringLiteral("ChartMainDivisions"), &m_draft.meter.mainDivisions, 2, 20); toggle(grid, 1, QStringLiteral("显示刻度值"), QStringLiteral("ChartShowValues"), &m_draft.meter.showValues); toggle(grid, 2, QStringLiteral("显示刻度轴"), QStringLiteral("ChartShowAxis"), &m_draft.meter.showAxis); integer(grid, 3, QStringLiteral("刻度字体大小:"), QStringLiteral("ChartFontSize"), &m_draft.meter.fontSize, 5, 100); color(grid, 4, QStringLiteral("标尺颜色:"), QStringLiteral("ChartScaleColor"), &m_draft.meter.scaleColor); layout->addWidget(scale, 1, 0);
        scale = new QGroupBox(QStringLiteral("显示次刻度"), page); grid = new QGridLayout(scale); sub = toggle(grid, 0, QStringLiteral("显示次刻度"), QStringLiteral("ChartShowSubScale"), &m_draft.meter.showSubScale); integer(grid, 1, QStringLiteral("次刻度分割数:"), QStringLiteral("ChartSubDivisions"), &m_draft.meter.subDivisions, 2, 5); findChild<QSpinBox *>(QStringLiteral("ChartSubDivisions"))->setEnabled(m_draft.meter.showSubScale); connect(sub, &QCheckBox::toggled, findChild<QSpinBox *>(QStringLiteral("ChartSubDivisions")), &QSpinBox::setEnabled); layout->addWidget(scale, 1, 1); layout->setRowStretch(2, 1);
    }
    return page;
}

/******************************************************************************
 * Function Name   : ChartDialog::selectShape
 * Description     : Switch entire form drafts and hide the appearance tab for rings and arcs.
 * Input Parameters: shape
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void ChartDialog::selectShape(int shape) //20261112-1 add
{
    int index;
    m_draft.shape = shape; m_basic->setCurrentIndex(shape); m_appearance->setCurrentIndex(qMin(shape, 1)); m_tabs->setTabVisible(1, shape != 2);
    for (index = 0; index < 3; ++index) { m_shapes[index]->setChecked(index == shape); m_shapes[index]->setEnabled(index != shape); } refresh();
}

/******************************************************************************
 * Function Name   : ChartDialog::changeType
 * Description     : Reset only this form ranges and bipolar state after explicit native-type changes.
 * Input Parameters: shape, type
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void ChartDialog::changeType(int shape, int type) //20261112-1 add
{
    QPair<QString, QString> range = ChartProperties::typeRange(type);
    QList<QLineEdit *> spins;
    QLineEdit *spin;
    int index = 0;
    if (shape == 0) { m_draft.bar.minimum = range.first; m_draft.bar.maximum = range.second; m_barMinimum->setText(range.first); m_barMaximum->setText(range.second); m_twoPoles->setChecked(false); m_midpoint->setText(QString::number(std::trunc((range.first.toDouble() + range.second.toDouble()) / 2), 'g', 16)); spins = {m_sourceMinimum, m_sourceMaximum}; }
    else if (shape == 1) { spins = {m_meterMinimum, m_meterMaximum}; } else { spins = {m_donutMinimum, m_donutMaximum}; }
    Q_FOREACH (spin, spins) { spin->setText(index++ == 0 ? range.first : range.second); }
    if (shape == 0) { m_draft.bar.sourceMinimum = range.first; m_draft.bar.sourceMaximum = range.second; } else if (shape == 1) { m_draft.meter.minimum = range.first; m_draft.meter.maximum = range.second; } else { m_draft.donut.minimum = range.first; m_draft.donut.maximum = range.second; }
    findChild<QLineEdit *>(QStringLiteral("Chart%1Address").arg(shape == 0 ? QStringLiteral("Bar") : shape == 1 ? QStringLiteral("Meter") : QStringLiteral("Donut")))->setProperty("chartWords", int(ChartProperties::wordCount(type)));
    if (shape == 0) { findChild<QLineEdit *>(QStringLiteral("ChartMinimumAddress"))->setProperty("chartWords", int(ChartProperties::wordCount(type))); findChild<QLineEdit *>(QStringLiteral("ChartMaximumAddress"))->setProperty("chartWords", int(ChartProperties::wordCount(type))); }
    refresh();
}

/******************************************************************************
 * Function Name   : ChartDialog::object
 * Description     : Return the complete detached editor state without changing geometry or other objects.
 * Input Parameters: None
 * Return Value    : Typed chart object.
 * Date            : 2026.11.12
 ******************************************************************************/
HmiObject ChartDialog::object() const //20261112-1 add
{
    HmiObject object = m_original;
    object.type = QStringLiteral("chart"); object.chart = m_draft; object.address.clear(); object.text.clear(); object.hasBitButtonProperties = false; object.hasStaticTextProperties = false; object.hasNumberDisplayProperties = false; object.hasImageDisplayProperties = false; object.chart.visibility = m_visibility->properties(); object.chart.visibilityBitChosen = m_visibility->bitStateChosen() || m_accepted; return object;
}

/******************************************************************************
 * Function Name   : ChartDialog::refresh
 * Description     : Render the same static preview as the canvas without emitting document mutations.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void ChartDialog::refresh() //20261112-1 add
{
    QImage image(130, 120, QImage::Format_ARGB32);
    QPainter painter;
    image.fill(QColor(QStringLiteral("#f0f0f0"))); painter.begin(&image); paintChart(&painter, QRectF(15, 10, 100, 100), m_draft); painter.end(); m_preview->setPixmap(QPixmap::fromImage(image));
}

/******************************************************************************
 * Function Name   : ChartDialog::accept
 * Description     : Validate active ranges and register spans before publishing one document transaction.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void ChartDialog::accept() //20261112-1 add
{
    HmiObject candidate;
    QString error;
    m_accepted = true; candidate = object();
    if (!candidate.chart.validate(&error) || !candidate.chart.validateAddresses(m_project.connections, &error)) { m_accepted = false; QMessageBox::warning(this, QStringLiteral("图表属性未提交"), error); return; } QDialog::accept();
}
