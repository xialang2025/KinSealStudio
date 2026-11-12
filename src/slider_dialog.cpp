#include "slider_dialog.h"
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
#include <QPlainTextEdit>
#include <QPolygonF>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTabWidget>
#include <QUuid>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : paintSlider
 * Description     : Draw the measured centered editor thumb and half rail fill; directional polygons are bounded PC approximations.
 * Input Parameters: painter, rect, properties
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void paintSlider(QPainter *painter, const QRectF &rect, const SliderProperties &properties) //20261112-0 add
{
    QRectF rail;
    QRectF fill;
    QRectF thumb;
    QPolygonF shape;
    QTransform transform;
    qreal width;
    qreal height;
    painter->save(); painter->setClipRect(rect); painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setPen(properties.borderTransparent ? QPen(Qt::NoPen) : QPen(QColor(properties.borderColor), 1)); painter->setBrush(properties.backgroundTransparent ? QBrush(Qt::NoBrush) : QBrush(QColor(properties.background))); painter->drawRect(rect.adjusted(0.5, 0.5, -0.5, -0.5));
    if (properties.vertical())
    {
        rail = QRectF(rect.center().x() - properties.railHeight / 2.0, rect.top() + 5, properties.railHeight, qMax(qreal(0), rect.height() - 10));
        thumb = QRectF(rect.left() + 2, rect.center().y() - properties.thumbWidth / 2.0, properties.thumbHeight, properties.thumbWidth);
        fill = QRectF(rail.left(), properties.direction == 0 ? rail.center().y() : rail.top(), rail.width(), rail.height() / 2);
    }
    else
    {
        rail = QRectF(rect.left() + 5, rect.center().y() - properties.railHeight / 2.0, qMax(qreal(0), rect.width() - 10), properties.railHeight);
        thumb = QRectF(rect.center().x() - properties.thumbWidth / 2.0, rect.top() + 2, properties.thumbWidth, properties.thumbHeight);
        fill = QRectF(properties.direction == 1 ? rail.left() : rail.center().x(), rail.top(), rail.width() / 2, rail.height());
    }
    painter->setPen(Qt::NoPen); painter->setBrush(properties.railBackgroundTransparent ? QBrush(Qt::NoBrush) : QBrush(QColor(properties.railBackground))); painter->drawRect(rail);
    painter->setBrush(QColor(properties.fillColor)); painter->drawRect(fill);
    if (!properties.railBorderTransparent) { painter->setPen(QPen(QColor(properties.railBorderColor), 1)); painter->setBrush(Qt::NoBrush); painter->drawRect(rail.adjusted(0.5, 0.5, -0.5, -0.5)); }
    painter->setPen(QPen(QColor(properties.thumbBorderColor), 1)); painter->setBrush(QColor(properties.thumbBackground));
    if (properties.style == 0) { painter->drawEllipse(thumb.adjusted(0.5, 0.5, -0.5, -0.5)); }
    else if (properties.style == 1) { painter->drawRect(thumb.adjusted(0.5, 0.5, -0.5, -0.5)); }
    else
    {
        width = properties.thumbWidth; height = properties.thumbHeight;
        if (properties.style == 2) { shape << QPointF(0, height) << QPointF(0, height / 4) << QPointF(width / 2, 0) << QPointF(width, height / 4) << QPointF(width, height); }
        else { shape << QPointF(0, height / 4) << QPointF(width / 2, 0) << QPointF(width, height / 4) << QPointF(width, height * 3 / 4) << QPointF(width / 2, height) << QPointF(0, height * 3 / 4); }
        transform.translate(thumb.center().x(), thumb.center().y()); transform.rotate(properties.pointing == 1 ? 180 : properties.pointing == 2 ? -90 : properties.pointing == 3 ? 90 : 0); transform.translate(-width / 2, -height / 2); painter->drawPolygon(transform.map(shape));
    }
    painter->restore();
}

/******************************************************************************
 * Function Name   : SliderDialog::SliderDialog
 * Description     : Build the two observed pages without adding a preview or hidden alarm/use-rail controls.
 * Input Parameters: object, project, parent
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
SliderDialog::SliderDialog(const HmiObject &object, const HmiProject &project, QWidget *parent) //20261112-0 add
    : QDialog(parent), m_original(object), m_project(project), m_draft(object.slider)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QTabWidget *tabs = new QTabWidget(this);
    QDialogButtonBox *buttons;
    QJsonArray connections;
    PlcConnection connection;
    setObjectName(QStringLiteral("SliderDialog")); setWindowTitle(QStringLiteral("滑动开关")); resize(630, 605);
    Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); } setProperty("plcConnections", connections.toVariantList());
    tabs->setObjectName(QStringLiteral("SliderTabs")); tabs->addTab(createGeneral(), QStringLiteral("一般属性")); tabs->addTab(createPermissions(), QStringLiteral("权限控制")); layout->addWidget(tabs);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("SliderButtons")); buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &SliderDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject); connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, QStringLiteral("滑动开关帮助"), QStringLiteral("配置写入地址、滑块外观、范围、权限和宏指令。动态范围每次重新开启时恢复LW2/LW3。")); });
}

/******************************************************************************
 * Function Name   : SliderDialog::createGeneral
 * Description     : Build native field groups and keep constants, range carriers and macro edits in detached drafts.
 * Input Parameters: None
 * Return Value    : General properties page.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *SliderDialog::createGeneral() //20261112-0 add
{
    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QGroupBox *properties = new QGroupBox(QStringLiteral("属性"), page);
    QGroupBox *pointing = new QGroupBox(QStringLiteral("滑块属性"), page);
    QGroupBox *appearance = new QGroupBox(QStringLiteral("外观"), page);
    QGroupBox *thumb = new QGroupBox(QStringLiteral("滑块"), appearance);
    QGroupBox *range = new QGroupBox(page);
    QGridLayout *grid = new QGridLayout(properties);
    QGridLayout *outer;
    QPushButton *picker;
    QPushButton *write;
    QRadioButton *radio;
    QStringList labels = {QStringLiteral("上"), QStringLiteral("下"), QStringLiteral("左"), QStringLiteral("右")};
    QStringList colors = {QStringLiteral("背景色:"), QStringLiteral("边框色:"), QStringLiteral("背景色:"), QStringLiteral("填充色:"), QStringLiteral("边框色:"), QStringLiteral("背景色:"), QStringLiteral("边框色:")};
    QSpinBox *spin;
    int index;
    layout->setColumnStretch(0, 1); layout->setColumnStretch(1, 1); range->setFixedWidth(270); //20261112-0 change: Preserve native equal top columns and narrow range group.
    page->setStyleSheet(QStringLiteral("QSpinBox, QComboBox, QLineEdit {background:#caeace;} QSpinBox:disabled, QComboBox:disabled, QLineEdit:disabled {background:#b1ceed;}"));
    m_type = new QComboBox(properties); m_type->setObjectName(QStringLiteral("SliderDataType")); m_type->addItems(SliderProperties::dataTypes()); m_type->setCurrentIndex(m_draft.dataType); m_address = new QLineEdit(m_draft.address, properties); m_address->setObjectName(QStringLiteral("SliderAddress")); picker = new QPushButton(QStringLiteral("…"), properties); picker->setObjectName(QStringLiteral("SliderAddressPicker")); picker->setFixedWidth(24); grid->addWidget(new QLabel(QStringLiteral("数据类型"), properties), 0, 0); grid->addWidget(m_type, 0, 1, 1, 2); grid->addWidget(new QLabel(QStringLiteral("写入地址:"), properties), 1, 0); grid->addWidget(m_address, 1, 1); grid->addWidget(picker, 1, 2); layout->addWidget(properties, 0, 0);
    connect(picker, &QPushButton::clicked, this, [this]() { SliderProperties properties; properties.dataType = m_type->currentIndex(); editHmiAddress(this, m_address, false, properties.transferWordCount()); });
    grid = new QGridLayout(pointing); grid->addWidget(new QLabel(QStringLiteral("滑块指向:"), pointing), 0, 0); m_pointing = new QButtonGroup(this);
    for (index = 0; index < 4; ++index) { radio = new QRadioButton(labels.at(index), pointing); radio->setObjectName(QStringLiteral("SliderPoint%1").arg(index)); m_pointing->addButton(radio, index); radio->setChecked(index == m_draft.pointing); grid->addWidget(radio, 0, index + 1); }
    m_direction = new QComboBox(pointing); m_direction->setObjectName(QStringLiteral("SliderDirection")); m_direction->addItems(m_draft.directions()); m_direction->setCurrentIndex(m_draft.direction); grid->addWidget(new QLabel(QStringLiteral("滑动方向:"), pointing), 1, 0); grid->addWidget(m_direction, 1, 1, 1, 4); layout->addWidget(pointing, 0, 1); connect(m_pointing, &QButtonGroup::idClicked, this, &SliderDialog::changePointing);
    outer = new QGridLayout(appearance);
    for (index = 0; index < 7; ++index) { m_colors[index] = new QPushButton(index < 5 ? appearance : thumb); m_colors[index]->setObjectName(QStringLiteral("SliderColor%1").arg(index)); m_colors[index]->setFixedSize(85, 22); m_colors[index]->setStyleSheet(QStringLiteral("background:%1").arg(*color(index))); connect(m_colors[index], &QPushButton::clicked, this, [this, index]() { chooseColor(index); }); }
    for (index = 0; index < 4; ++index) { m_transparent[index] = new QCheckBox(QStringLiteral("透明"), appearance); m_transparent[index]->setObjectName(QStringLiteral("SliderTransparent%1").arg(index)); }
    m_transparent[0]->setChecked(m_draft.backgroundTransparent); m_transparent[1]->setChecked(m_draft.borderTransparent); m_transparent[2]->setChecked(m_draft.railBackgroundTransparent); m_transparent[3]->setChecked(m_draft.railBorderTransparent);
    outer->addWidget(new QLabel(colors.at(0), appearance), 0, 0); outer->addWidget(m_colors[0], 0, 1); outer->addWidget(m_transparent[0], 0, 2); outer->addWidget(new QLabel(colors.at(1), appearance), 0, 3); outer->addWidget(m_colors[1], 0, 4); outer->addWidget(m_transparent[1], 0, 5);
    outer->addWidget(new QLabel(colors.at(2), appearance), 1, 0); outer->addWidget(m_colors[2], 1, 1); outer->addWidget(m_transparent[2], 1, 2); outer->addWidget(new QLabel(colors.at(3), appearance), 1, 3); outer->addWidget(m_colors[3], 1, 4, 1, 2);
    outer->addWidget(new QLabel(colors.at(4), appearance), 2, 0); outer->addWidget(m_colors[4], 2, 1); outer->addWidget(m_transparent[3], 2, 2);
    m_railHeight = new QSpinBox(appearance); m_railHeight->setObjectName(QStringLiteral("SliderRailHeight")); m_railHeight->setRange(1, 800); m_railHeight->setValue(m_draft.railHeight); outer->addWidget(new QLabel(QStringLiteral("滑轨高度:"), appearance), 2, 3); outer->addWidget(m_railHeight, 2, 4, 1, 2);
    grid = new QGridLayout(thumb); grid->addWidget(new QLabel(colors.at(5), thumb), 0, 0); grid->addWidget(m_colors[5], 0, 1); grid->addWidget(new QLabel(colors.at(6), thumb), 0, 2); grid->addWidget(m_colors[6], 0, 3);
    m_style = new QComboBox(thumb); m_style->setObjectName(QStringLiteral("SliderStyle")); m_style->addItems(SliderProperties::styles()); m_style->setCurrentIndex(m_draft.style); grid->addWidget(new QLabel(QStringLiteral("滑块样式:"), thumb), 1, 0); grid->addWidget(m_style, 1, 1, 1, 2);
    m_thumbWidth = new QSpinBox(thumb); m_thumbWidth->setObjectName(QStringLiteral("SliderThumbWidth")); m_thumbHeight = new QSpinBox(thumb); m_thumbHeight->setObjectName(QStringLiteral("SliderThumbHeight"));
    for (index = 0; index < 2; ++index) { spin = index == 0 ? m_thumbWidth : m_thumbHeight; spin->setRange(1, 800); spin->setValue(index == 0 ? m_draft.thumbWidth : m_draft.thumbHeight); grid->addWidget(new QLabel(index == 0 ? QStringLiteral("滑块宽度:") : QStringLiteral("滑块高度:"), thumb), 2, index * 2); grid->addWidget(spin, 2, index * 2 + 1); }
    outer->addWidget(thumb, 3, 0, 1, 6); layout->addWidget(appearance, 1, 0, 1, 2);
    grid = new QGridLayout(range); m_dynamic = new QCheckBox(QStringLiteral("动态范围"), range); m_dynamic->setObjectName(QStringLiteral("SliderDynamic")); m_dynamic->setChecked(m_draft.dynamicRange); grid->addWidget(m_dynamic, 0, 0, 1, 3); m_minimum = new QLineEdit(m_draft.dynamicRange ? m_draft.minimumAddress : m_draft.minimum, range); m_minimum->setObjectName(QStringLiteral("SliderMinimum")); m_maximum = new QLineEdit(m_draft.dynamicRange ? m_draft.maximumAddress : m_draft.maximum, range); m_maximum->setObjectName(QStringLiteral("SliderMaximum")); m_minimumPicker = new QPushButton(QStringLiteral("…"), range); m_minimumPicker->setObjectName(QStringLiteral("SliderMinimumPicker")); m_maximumPicker = new QPushButton(QStringLiteral("…"), range); m_maximumPicker->setObjectName(QStringLiteral("SliderMaximumPicker")); m_minimum->setFixedWidth(130); m_maximum->setFixedWidth(130); m_minimumPicker->setFixedWidth(24); m_maximumPicker->setFixedWidth(24); m_minimumPicker->setVisible(m_draft.dynamicRange); m_maximumPicker->setVisible(m_draft.dynamicRange); grid->addWidget(new QLabel(QStringLiteral("最小值:"), range), 1, 0); grid->addWidget(m_minimum, 1, 1); grid->addWidget(m_minimumPicker, 1, 2); grid->addWidget(new QLabel(QStringLiteral("最大值:"), range), 2, 0); grid->addWidget(m_maximum, 2, 1); grid->addWidget(m_maximumPicker, 2, 2); layout->addWidget(range, 2, 0); connect(m_dynamic, &QCheckBox::toggled, this, &SliderDialog::changeDynamic);
    connect(m_minimumPicker, &QPushButton::clicked, this, [this]() { SliderProperties properties; properties.dataType = m_type->currentIndex(); editHmiAddress(this, m_minimum, false, properties.transferWordCount()); }); connect(m_maximumPicker, &QPushButton::clicked, this, [this]() { SliderProperties properties; properties.dataType = m_type->currentIndex(); editHmiAddress(this, m_maximum, false, properties.transferWordCount()); });
    m_macro = new QGroupBox(QStringLiteral("宏指令"), page); m_macro->setObjectName(QStringLiteral("SliderMacro")); m_macro->setCheckable(true); m_macro->setChecked(m_draft.macroEnabled); grid = new QGridLayout(m_macro); m_macroList = new QComboBox(m_macro); m_macroList->setObjectName(QStringLiteral("SliderMacroList")); m_macroList->addItems(m_project.macros.keys()); if (!m_draft.macroName.isEmpty() && !m_project.macros.contains(m_draft.macroName)) { m_macroList->addItem(m_draft.macroName); } m_macroList->setCurrentIndex(m_macroList->findText(m_draft.macroName)); write = new QPushButton(QStringLiteral("写宏"), m_macro); write->setObjectName(QStringLiteral("SliderWriteMacro")); write->setFixedWidth(75); grid->setColumnStretch(1, 1); grid->addWidget(new QLabel(QStringLiteral("使用宏:"), m_macro), 0, 0); grid->addWidget(m_macroList, 0, 1); grid->addWidget(write, 0, 2); layout->addWidget(m_macro, 3, 0, 1, 2); connect(write, &QPushButton::clicked, this, &SliderDialog::editMacro);
    return page;
}

/******************************************************************************
 * Function Name   : SliderDialog::changePointing
 * Description     : Rebuild the direction pair only on an axis change while keeping dimensions editable for every style.
 * Input Parameters: value
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void SliderDialog::changePointing(int value) //20261112-0 add
{
    QSignalBlocker blocker(m_direction);
    m_draft.direction = m_direction->currentIndex(); m_draft.setPointing(value); m_direction->clear(); m_direction->addItems(m_draft.directions()); m_direction->setCurrentIndex(m_draft.direction);
}

/******************************************************************************
 * Function Name   : SliderDialog::changeDynamic
 * Description     : Save the outgoing range draft before applying native constant-cache and dynamic-reset behavior.
 * Input Parameters: enabled
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void SliderDialog::changeDynamic(bool enabled) //20261112-0 add
{
    if (m_draft.dynamicRange) { m_draft.minimumAddress = m_minimum->text(); m_draft.maximumAddress = m_maximum->text(); }
    else { m_draft.minimum = m_minimum->text(); m_draft.maximum = m_maximum->text(); }
    m_draft.setDynamicRange(enabled); m_minimum->setText(enabled ? m_draft.minimumAddress : m_draft.minimum); m_maximum->setText(enabled ? m_draft.maximumAddress : m_draft.maximum); m_minimumPicker->setVisible(enabled); m_maximumPicker->setVisible(enabled);
}

/******************************************************************************
 * Function Name   : SliderDialog::color
 * Description     : Resolve one of seven independent appearance colors for a detached picker transaction.
 * Input Parameters: index
 * Return Value    : Draft field pointer.
 * Date            : 2026.11.12
 ******************************************************************************/
QString *SliderDialog::color(int index) //20261112-0 add
{
    QString *fields[7] = {&m_draft.background, &m_draft.borderColor, &m_draft.railBackground, &m_draft.fillColor, &m_draft.railBorderColor, &m_draft.thumbBackground, &m_draft.thumbBorderColor};
    return fields[qBound(0, index, 6)];
}

/******************************************************************************
 * Function Name   : SliderDialog::chooseColor
 * Description     : Preserve editable swatches even while their independently stored transparency flag is enabled.
 * Input Parameters: index
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void SliderDialog::chooseColor(int index) //20261112-0 add
{
    QColor selected = QColorDialog::getColor(QColor(*color(index)), this, QStringLiteral("选择颜色"), QColorDialog::DontUseNativeDialog);
    if (selected.isValid()) { *color(index) = selected.name(); m_colors[index]->setStyleSheet(QStringLiteral("background:%1").arg(selected.name())); }
}

/******************************************************************************
 * Function Name   : SliderDialog::object
 * Description     : Collect complete independent slider state and constrain geometry when its axis or thumb height changes.
 * Input Parameters: None
 * Return Value    : Detached typed slider object.
 * Date            : 2026.11.12
 ******************************************************************************/
HmiObject SliderDialog::object() const //20261112-0 add
{
    HmiObject object = m_original;
    SliderProperties *properties = &object.slider;
    *properties = m_draft; object.type = QStringLiteral("slider"); object.address.clear(); object.text.clear(); object.hasBitButtonProperties = false; object.hasStaticTextProperties = false; properties->dataType = m_type->currentIndex(); properties->address = m_address->text().trimmed(); properties->pointing = m_pointing->checkedId(); properties->direction = m_direction->currentIndex(); properties->style = m_style->currentIndex(); properties->railHeight = m_railHeight->value(); properties->thumbWidth = m_thumbWidth->value(); properties->thumbHeight = m_thumbHeight->value(); properties->backgroundTransparent = m_transparent[0]->isChecked(); properties->borderTransparent = m_transparent[1]->isChecked(); properties->railBackgroundTransparent = m_transparent[2]->isChecked(); properties->railBorderTransparent = m_transparent[3]->isChecked();
    if (properties->dynamicRange) { properties->minimumAddress = m_minimum->text().trimmed(); properties->maximumAddress = m_maximum->text().trimmed(); }
    else { properties->minimum = m_minimum->text(); properties->maximum = m_maximum->text(); }
    properties->macroEnabled = m_macro->isChecked(); properties->macroName = m_macroList->currentText(); properties->visibility = m_visibility->properties(); properties->touch = m_touch->properties(); properties->notice = m_notice->isChecked(); properties->noticeBit = m_noticeBit->isChecked(); properties->noticeWord = m_noticeWord->isChecked(); properties->noticeBitValue = m_noticeBitOn->isChecked(); properties->noticeBitAddress = m_noticeBitAddress->text().trimmed(); properties->noticeWordAddress = m_noticeWordAddress->text().trimmed(); properties->noticeWordType = m_noticeWordType->currentIndex(); properties->noticeWordValue = m_noticeWordValue->text();
    if (properties->vertical() != m_original.slider.vertical() || properties->thumbHeight != m_original.slider.thumbHeight) { object.geometry = properties->constrainedGeometry(object.geometry); }
    return object;
}


/******************************************************************************
 * Function Name   : SliderDialog::createPermissions
 * Description     : Preserve the native selected-one bit conditions, expose notification drafts and omit the absent hold-time row.
 * Input Parameters: None
 * Return Value    : Permission page.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *SliderDialog::createPermissions() //20261112-0 add
{
    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QGridLayout *grid;
    QWidget *statePanel;
    QHBoxLayout *row;
    QRadioButton *off;
    QPushButton *bitPicker;
    QPushButton *wordPicker;
    m_visibility = new HmiPermissionEditor(QStringLiteral("显现"), QStringLiteral("SliderVisibility"), m_draft.visibility, false, page); m_touch = new HmiPermissionEditor(QStringLiteral("触控有效"), QStringLiteral("SliderTouch"), m_draft.touch, true, page); layout->addWidget(m_visibility, 0, 0); layout->addWidget(m_touch, 0, 1);
    m_notice = new QGroupBox(QStringLiteral("通知"), page); m_notice->setObjectName(QStringLiteral("SliderNotice")); m_notice->setCheckable(true); m_notice->setChecked(m_draft.notice); grid = new QGridLayout(m_notice);
    m_noticeBit = new QCheckBox(QStringLiteral("通知位"), m_notice); m_noticeBit->setObjectName(QStringLiteral("SliderNoticeBit")); m_noticeBit->setChecked(m_draft.noticeBit); m_noticeWord = new QCheckBox(QStringLiteral("通知字"), m_notice); m_noticeWord->setObjectName(QStringLiteral("SliderNoticeWord")); m_noticeWord->setChecked(m_draft.noticeWord);
    m_noticeBitAddress = new QLineEdit(m_draft.noticeBitAddress, m_notice); m_noticeBitAddress->setObjectName(QStringLiteral("SliderNoticeBitAddress")); m_noticeWordAddress = new QLineEdit(m_draft.noticeWordAddress, m_notice); m_noticeWordAddress->setObjectName(QStringLiteral("SliderNoticeWordAddress")); bitPicker = new QPushButton(QStringLiteral("…"), m_notice); wordPicker = new QPushButton(QStringLiteral("…"), m_notice);
    statePanel = new QWidget(m_notice); row = new QHBoxLayout(statePanel); row->setContentsMargins(0, 0, 0, 0); m_noticeBitOn = new QRadioButton(QStringLiteral("1"), statePanel); m_noticeBitOn->setObjectName(QStringLiteral("SliderNoticeBitOn")); off = new QRadioButton(QStringLiteral("0"), statePanel); off->setObjectName(QStringLiteral("SliderNoticeBitOff")); m_noticeBitOn->setChecked(m_draft.noticeBitValue); off->setChecked(!m_draft.noticeBitValue); row->addWidget(m_noticeBitOn); row->addWidget(off);
    m_noticeWordType = new QComboBox(m_notice); m_noticeWordType->setObjectName(QStringLiteral("SliderNoticeWordType")); m_noticeWordType->addItems(StaticTextProperties::dataTypes()); m_noticeWordType->setCurrentIndex(m_draft.noticeWordType); m_noticeWordValue = new QLineEdit(m_draft.noticeWordValue, m_notice); m_noticeWordValue->setObjectName(QStringLiteral("SliderNoticeWordValue"));
    grid->addWidget(m_noticeBit, 0, 0); grid->addWidget(m_noticeBitAddress, 0, 1); grid->addWidget(bitPicker, 0, 2); grid->addWidget(m_noticeWord, 0, 3); grid->addWidget(m_noticeWordAddress, 0, 4); grid->addWidget(wordPicker, 0, 5); grid->addWidget(statePanel, 1, 0, 1, 3); grid->addWidget(m_noticeWordType, 1, 3, 1, 3); grid->addWidget(m_noticeWordValue, 2, 3, 1, 3); layout->addWidget(m_notice, 1, 0, 1, 2); layout->setRowStretch(2, 1);
    connect(m_noticeBit, &QCheckBox::toggled, this, [this, statePanel, bitPicker](bool enabled) { if (enabled) { m_noticeWord->setChecked(false); } m_noticeBitAddress->setEnabled(enabled); statePanel->setEnabled(enabled); bitPicker->setEnabled(enabled); });
    connect(m_noticeWord, &QCheckBox::toggled, this, [this, wordPicker](bool enabled) { if (enabled) { m_noticeBit->setChecked(false); } m_noticeWordAddress->setEnabled(enabled); m_noticeWordType->setEnabled(enabled); m_noticeWordValue->setEnabled(enabled); wordPicker->setEnabled(enabled); });
    m_noticeBitAddress->setEnabled(m_draft.noticeBit); statePanel->setEnabled(m_draft.noticeBit); bitPicker->setEnabled(m_draft.noticeBit); m_noticeWordAddress->setEnabled(m_draft.noticeWord); m_noticeWordType->setEnabled(m_draft.noticeWord); m_noticeWordValue->setEnabled(m_draft.noticeWord); wordPicker->setEnabled(m_draft.noticeWord);
    connect(bitPicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_noticeBitAddress, true); }); connect(wordPicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_noticeWordAddress, false, m_noticeWordType->currentIndex() == 0 || m_noticeWordType->currentIndex() == 2 ? 1 : 2); });
    return page;
}


/******************************************************************************
 * Function Name   : SliderDialog::macros
 * Description     : Return the local macro pool for the same accepting whole-document transaction.
 * Input Parameters: None
 * Return Value    : Detached macro registry.
 * Date            : 2026.11.12
 ******************************************************************************/
QMap<QString, QString> SliderDialog::macros() const //20261112-0 add
{
    return m_project.macros;
}


/******************************************************************************
 * Function Name   : SliderDialog::accept
 * Description     : Check range, geometry, active carriers and macro dependencies against the complete project before closing.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void SliderDialog::accept() //20261112-0 add
{
    HmiObject draft = object();
    HmiProject candidate = m_project;
    QString error;
    bool replaced = false;
    int index;
    if (draft.id.isEmpty()) { draft.id = QUuid::createUuid().toString(QUuid::WithoutBraces); }
    for (index = 0; index < candidate.objects.size(); ++index) { if (candidate.objects.at(index).id == draft.id) { candidate.objects[index] = draft; replaced = true; break; } }
    if (!replaced) { candidate.objects.append(draft); }
    if (!candidate.validateModel(&error)) { QMessageBox::warning(this, QStringLiteral("滑动开关属性无效"), error); return; }
    QDialog::accept();
}


/******************************************************************************
 * Function Name   : SliderDialog::editMacro
 * Description     : Reuse the existing Lite source-editor transaction, keeping its template distinct from verified native editor behavior.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void SliderDialog::editMacro() //20261112-0 add
{
    QDialog dialog(this);
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QLineEdit *name = new QLineEdit(&dialog);
    QPlainTextEdit *source = new QPlainTextEdit(&dialog);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    QString initial = m_macroList->currentText();
    dialog.setObjectName(QStringLiteral("SliderMacroEditor")); dialog.setWindowTitle(QStringLiteral("宏指令编辑器")); dialog.resize(760, 540); name->setObjectName(QStringLiteral("MacroName")); source->setObjectName(QStringLiteral("MacroSource")); name->setText(initial.isEmpty() ? QStringLiteral("Macro%1").arg(m_project.macros.size() + 1) : initial); source->setPlainText(m_project.macros.value(initial, QStringLiteral("#include \"macro.h\"\n\nvoid Macro_Entry( )\n{\n\n}\n"))); layout->addWidget(name); layout->addWidget(source); layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject); connect(buttons, &QDialogButtonBox::accepted, &dialog, [&dialog, name, source]() { if (name->text().trimmed().isEmpty() || name->text().size() > 256 || source->toPlainText().size() > 1024 * 1024) { QMessageBox::warning(&dialog, QStringLiteral("宏无效"), QStringLiteral("宏名称须为1..256字符，源代码不能超过本版1 MiB保护范围。")); return; } dialog.accept(); });
    if (dialog.exec() == QDialog::Accepted) { m_project.macros.insert(name->text().trimmed(), source->toPlainText()); m_macroList->clear(); m_macroList->addItems(m_project.macros.keys()); m_macroList->setCurrentText(name->text().trimmed()); }
}
