#include "wheel_dialog.h"
#include "hmi_property_widgets.h"
#include "bit_button_dialog.h"
#include "control_rendering.h"
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
#include <QSpinBox>
#include <QTabWidget>
#include <QUuid>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : paintWheel
 * Description     : Render a static integer midpoint sample with background-only alpha, separate row fonts and optional highlight image.
 * Input Parameters: painter, rect, properties
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void paintWheel(QPainter *painter, const QRectF &rect, const WheelProperties &properties) //20261112-0 add
{
    QColor background(properties.background);
    QFont font(QStringLiteral("SimSun"));
    QImage picture;
    QRectF band;
    qreal gap;
    qreal y;
    qreal height;
    int current = properties.rowCount / 2;
    int value;
    int index;
    background.setAlpha(properties.opacity); painter->save(); painter->setClipRect(rect); painter->fillRect(rect, background);
    gap = qMax(qreal(0), (rect.height() - properties.lowFontSize * (properties.rowCount - 1) - properties.highFontSize) / (properties.rowCount + 1)); y = rect.top() + gap;
    if (!properties.highlight.picture.isEmpty()) { picture = properties.highlight.picturePng.isEmpty() ? QImage(properties.highlight.picture) : decodedControlPng(properties.highlight.picturePng); }
    for (index = 0; index < properties.rowCount; ++index)
    {
        height = index == current ? properties.highFontSize : properties.lowFontSize; band = QRectF(rect.left(), y, rect.width(), height);
        if (index == current && !picture.isNull()) { painter->drawImage(band, picture); }
        font.setPixelSize(int(height)); painter->setFont(font); painter->setPen(QColor(index == current ? properties.highColor : properties.lowColor)); value = properties.editorSampleValue() + (index - current) * properties.step.toInt(); painter->drawText(band, Qt::AlignCenter, QString::number(value)); y += height + gap;
    }
    painter->restore();
}

/******************************************************************************
 * Function Name   : WheelDialog::WheelDialog
 * Description     : Build the two observed pages and static shared preview without exposing hidden numeric/picture modes or notifications.
 * Input Parameters: object, project, parent
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
WheelDialog::WheelDialog(const HmiObject &object, const HmiProject &project, QWidget *parent) //20261112-0 add
    : QDialog(parent), m_original(object), m_project(project), m_draft(object.wheel)
{
    QGridLayout *layout = new QGridLayout(this);
    QTabWidget *tabs = new QTabWidget(this);
    QGroupBox *preview = new QGroupBox(QStringLiteral("预览"), this);
    QVBoxLayout *previewLayout = new QVBoxLayout(preview);
    QDialogButtonBox *buttons;
    QJsonArray connections;
    PlcConnection connection;
    setObjectName(QStringLiteral("WheelDialog")); setWindowTitle(QStringLiteral("滚轮控件")); resize(865, 595);
    Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); } setProperty("plcConnections", connections.toVariantList());
    tabs->setObjectName(QStringLiteral("WheelTabs")); tabs->addTab(createAppearance(), QStringLiteral("外观设置")); tabs->addTab(createPermissions(), QStringLiteral("权限设置")); layout->addWidget(tabs, 0, 0); layout->setColumnStretch(0, 1);
    m_preview = new QLabel(preview); m_preview->setObjectName(QStringLiteral("WheelPreview")); m_preview->setFixedSize(160, 160); previewLayout->addWidget(m_preview); layout->addWidget(preview, 0, 1, Qt::AlignTop);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("WheelButtons")); buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); layout->addWidget(buttons, 1, 0, 1, 2);
    connect(buttons, &QDialogButtonBox::accepted, this, &WheelDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject); connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, QStringLiteral("滚轮控件帮助"), QStringLiteral("设置写入地址、滚动绕回、灵敏度、数字外观与高亮底图。预览显示范围中点。")); }); refresh();
}

/******************************************************************************
 * Function Name   : WheelDialog::createAppearance
 * Description     : Build measured bounds, fixed font menus, independent colors and the three highlight-picture actions.
 * Input Parameters: None
 * Return Value    : Appearance page.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *WheelDialog::createAppearance() //20261112-0 add
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    QGroupBox *properties = new QGroupBox(QStringLiteral("滚轮属性"), page);
    QGroupBox *numbers = new QGroupBox(QStringLiteral("数字显示属性"), page);
    QGridLayout *grid = new QGridLayout(properties);
    QPushButton *picker;
    QPushButton *clear;
    QPushButton *system;
    QPushButton *project;
    QStringList labels = {QStringLiteral("最 小 值:"), QStringLiteral("最 大 值:"), QStringLiteral("步距:")};
    QLineEdit *field;
    int index;
    int size;
    page->setStyleSheet(QStringLiteral("QSpinBox, QComboBox, QLineEdit {background:#caeace; min-height:28px;} QSpinBox:disabled, QComboBox:disabled, QLineEdit:disabled {background:#b1ceed;}")); properties->setFixedHeight(145); properties->setMaximumWidth(510); numbers->setMaximumWidth(510); numbers->setFixedHeight(315); layout->addWidget(properties); layout->addWidget(numbers); layout->addStretch();
    m_address = new QLineEdit(m_draft.address, properties); m_address->setObjectName(QStringLiteral("WheelAddress")); m_address->setFixedWidth(130); picker = new QPushButton(QStringLiteral("…"), properties); picker->setObjectName(QStringLiteral("WheelAddressPicker")); picker->setFixedWidth(24); m_rollover = new QCheckBox(QStringLiteral("滚动绕回"), properties); m_rollover->setObjectName(QStringLiteral("WheelRollover")); m_rollover->setChecked(m_draft.rollover); m_sensitivity = new QSpinBox(properties); m_sensitivity->setObjectName(QStringLiteral("WheelSensitivity")); m_sensitivity->setRange(1, 99); m_sensitivity->setValue(m_draft.sensitivity); m_sensitivity->setFixedWidth(130); grid->setRowStretch(0, 1); grid->addWidget(new QLabel(QStringLiteral("写入地址:"), properties), 1, 0); grid->addWidget(m_address, 1, 1); grid->addWidget(picker, 1, 2); grid->addWidget(m_rollover, 1, 3); grid->addWidget(new QLabel(QStringLiteral("灵敏度:"), properties), 2, 0); grid->addWidget(m_sensitivity, 2, 1); grid->setColumnStretch(3, 1); connect(picker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_address, false); });
    grid = new QGridLayout(numbers); grid->setVerticalSpacing(10);
    for (index = 0; index < 3; ++index) { m_colors[index] = new QPushButton(numbers); m_colors[index]->setObjectName(QStringLiteral("WheelColor%1").arg(index)); m_colors[index]->setFixedSize(140, 28); m_colors[index]->setStyleSheet(QStringLiteral("background:%1").arg(*color(index))); connect(m_colors[index], &QPushButton::clicked, this, [this, index]() { chooseColor(index); }); }
    grid->addWidget(new QLabel(QStringLiteral("背景色:"), numbers), 0, 0); grid->addWidget(m_colors[0], 0, 1); grid->addWidget(new QLabel(QStringLiteral("低亮文本色:"), numbers), 0, 2); grid->addWidget(m_colors[1], 0, 3);
    m_opacity = new QSpinBox(numbers); m_opacity->setObjectName(QStringLiteral("WheelOpacity")); m_opacity->setRange(0, 255); m_opacity->setValue(m_draft.opacity); m_opacity->setFixedWidth(140); grid->addWidget(new QLabel(QStringLiteral("透明度:"), numbers), 1, 0); grid->addWidget(m_opacity, 1, 1); m_lowFont = new QComboBox(numbers); m_lowFont->setObjectName(QStringLiteral("WheelLowFont")); m_lowFont->setFixedWidth(140); m_highFont = new QComboBox(numbers); m_highFont->setObjectName(QStringLiteral("WheelHighFont")); m_highFont->setFixedWidth(140); Q_FOREACH (size, WheelProperties::fontSizes()) { m_lowFont->addItem(QString::number(size), size); m_highFont->addItem(QString::number(size), size); } m_lowFont->setCurrentIndex(m_lowFont->findData(m_draft.lowFontSize)); m_highFont->setCurrentIndex(m_highFont->findData(m_draft.highFontSize)); grid->addWidget(new QLabel(QStringLiteral("低亮大小:"), numbers), 1, 2); grid->addWidget(m_lowFont, 1, 3);
    m_minimum = new QLineEdit(m_draft.minimum, numbers); m_minimum->setObjectName(QStringLiteral("WheelMinimum")); m_maximum = new QLineEdit(m_draft.maximum, numbers); m_maximum->setObjectName(QStringLiteral("WheelMaximum")); m_step = new QLineEdit(m_draft.step, numbers); m_step->setObjectName(QStringLiteral("WheelStep"));
    for (index = 0; index < 3; ++index) { field = index == 0 ? m_minimum : index == 1 ? m_maximum : m_step; field->setMaxLength(15); field->setFixedWidth(140); grid->addWidget(new QLabel(labels.at(index), numbers), index + 2, 0); grid->addWidget(field, index + 2, 1); connect(field, &QLineEdit::textChanged, this, [this]() { if (m_preview != nullptr) { refresh(); } }); }
    grid->addWidget(new QLabel(QStringLiteral("高亮文本色:"), numbers), 2, 2); grid->addWidget(m_colors[2], 2, 3); grid->addWidget(new QLabel(QStringLiteral("高亮大小:"), numbers), 3, 2); grid->addWidget(m_highFont, 3, 3);
    clear = new QPushButton(QStringLiteral("不使用图片"), numbers); clear->setObjectName(QStringLiteral("WheelClearPicture")); system = new QPushButton(QStringLiteral("系统图库"), numbers); system->setObjectName(QStringLiteral("WheelSystemGallery")); project = new QPushButton(QStringLiteral("工程图库"), numbers); project->setObjectName(QStringLiteral("WheelProjectGallery")); grid->addWidget(new QLabel(QStringLiteral("高亮底图:"), numbers), 4, 2); grid->addWidget(clear, 4, 3); grid->addWidget(system, 5, 3); grid->addWidget(project, 6, 3);
    m_rows = new QSpinBox(numbers); m_rows->setObjectName(QStringLiteral("WheelRows")); m_rows->setRange(2, 5); m_rows->setValue(m_draft.rowCount); m_rows->setFixedWidth(140); grid->addWidget(new QLabel(QStringLiteral("当前数目:"), numbers), 6, 0); grid->addWidget(m_rows, 6, 1);
    connect(clear, &QPushButton::clicked, this, [this]() { m_draft.highlight = ImageDisplayState(); refresh(); }); connect(system, &QPushButton::clicked, this, [this]() { selectPicture(false); }); connect(project, &QPushButton::clicked, this, [this]() { selectPicture(true); }); connect(m_rows, &QSpinBox::valueChanged, this, [this]() { refresh(); }); connect(m_opacity, &QSpinBox::valueChanged, this, [this]() { refresh(); }); connect(m_lowFont, &QComboBox::currentIndexChanged, this, [this]() { refresh(); }); connect(m_highFont, &QComboBox::currentIndexChanged, this, [this]() { refresh(); });
    return page;
}

/******************************************************************************
 * Function Name   : WheelDialog::createPermissions
 * Description     : Build only visibility and touch conditions with native fresh empty radios and no notification/hold-time fields.
 * Input Parameters: None
 * Return Value    : Permissions page.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *WheelDialog::createPermissions() //20261112-0 add
{
    QWidget *page = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(page);
    m_visibility = new HmiPermissionEditor(QStringLiteral("显现"), QStringLiteral("WheelVisibility"), m_draft.visibility, false, page); m_touch = new HmiPermissionEditor(QStringLiteral("触控有效"), QStringLiteral("WheelTouch"), m_draft.touch, true, page); m_visibility->configureUnchosenBitState(m_draft.visibilityBitChosen); m_touch->configureUnchosenBitState(m_draft.touchBitChosen); layout->addWidget(m_visibility, 0, Qt::AlignTop); layout->addWidget(m_touch, 0, Qt::AlignTop); return page;
}

/******************************************************************************
 * Function Name   : WheelDialog::color
 * Description     : Resolve independent background, low and high color fields without touching live project state.
 * Input Parameters: index
 * Return Value    : Draft field pointer.
 * Date            : 2026.11.12
 ******************************************************************************/
QString *WheelDialog::color(int index) //20261112-0 add
{
    QString *fields[3] = {&m_draft.background, &m_draft.lowColor, &m_draft.highColor};
    return fields[qBound(0, index, 2)];
}

/******************************************************************************
 * Function Name   : WheelDialog::chooseColor
 * Description     : Update one swatch and shared preview only after color-picker confirmation.
 * Input Parameters: index
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void WheelDialog::chooseColor(int index) //20261112-0 add
{
    QColor selected = QColorDialog::getColor(QColor(*color(index)), this, QStringLiteral("选择颜色"), QColorDialog::DontUseNativeDialog);
    if (selected.isValid()) { *color(index) = selected.name(); m_colors[index]->setStyleSheet(QStringLiteral("background:%1").arg(selected.name())); refresh(); }
}

/******************************************************************************
 * Function Name   : WheelDialog::selectPicture
 * Description     : Reuse bounded galleries as nested transactions, retaining resource changes only when a picture is accepted.
 * Input Parameters: projectGallery
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void WheelDialog::selectPicture(bool projectGallery) //20261112-0 add
{
    HmiObject adapter;
    HmiProject context = m_project;
    QMap<QString, QByteArray> pool;
    ImageDisplayState selected = m_draft.highlight;
    BitButtonDialog *gallery;
    context.objects.append(object()); adapter.id = QStringLiteral("wheel-gallery-adapter"); adapter.type = QStringLiteral("switch"); adapter.address = QStringLiteral("LB0"); adapter.hasBitButtonProperties = true; gallery = new BitButtonDialog(adapter, context, this);
    if (gallery->choosePictureForControl(projectGallery, &selected.picture, &selected.picturePng, &pool)) { m_draft.highlight = selected; m_project.pictures = pool; refresh(); }
    delete gallery;
}

/******************************************************************************
 * Function Name   : WheelDialog::object
 * Description     : Collect detached fields, preserving chosen zero while normalizing accepted default radios to one as observed.
 * Input Parameters: None
 * Return Value    : Complete typed wheel record.
 * Date            : 2026.11.12
 ******************************************************************************/
HmiObject WheelDialog::object() const //20261112-0 add
{
    HmiObject object = m_original;
    WheelProperties *properties = &object.wheel;
    *properties = m_draft; object.type = QStringLiteral("wheel"); object.address.clear(); object.text.clear(); object.hasBitButtonProperties = false; object.hasStaticTextProperties = false; properties->address = m_address->text().trimmed(); properties->rollover = m_rollover->isChecked(); properties->sensitivity = m_sensitivity->value(); properties->opacity = m_opacity->value(); properties->rowCount = m_rows->value(); properties->minimum = m_minimum->text(); properties->maximum = m_maximum->text(); properties->step = m_step->text(); properties->lowFontSize = m_lowFont->currentData().toInt(); properties->highFontSize = m_highFont->currentData().toInt(); properties->visibility = m_visibility->properties(); properties->touch = m_touch->properties(); properties->visibilityBitChosen = m_visibility->bitStateChosen() || m_accepted; properties->touchBitChosen = m_touch->bitStateChosen() || m_accepted; return object;
}

/******************************************************************************
 * Function Name   : WheelDialog::pictures
 * Description     : Return the local resource pool for the same accepting placement/edit transaction.
 * Input Parameters: None
 * Return Value    : Detached picture registry.
 * Date            : 2026.11.12
 ******************************************************************************/
QMap<QString, QByteArray> WheelDialog::pictures() const //20261112-0 add
{
    return m_project.pictures;
}

/******************************************************************************
 * Function Name   : WheelDialog::refresh
 * Description     : Draw the same midpoint sample as the canvas without treating it as live PLC data.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void WheelDialog::refresh() //20261112-0 add
{
    WheelProperties properties = object().wheel;
    QImage image(160, 160, QImage::Format_ARGB32);
    QPainter painter;
    int height = properties.lowFontSize * (properties.rowCount - 1) + properties.highFontSize;
    image.fill(QColor(QStringLiteral("#f0f0f0"))); painter.begin(&image); paintWheel(&painter, QRectF(20, (160 - height) / 2.0, 120, height), properties); painter.end(); m_preview->setPixmap(QPixmap::fromImage(image));
}

/******************************************************************************
 * Function Name   : WheelDialog::accept
 * Description     : Validate the whole candidate before accepting model, image resources and permission defaults atomically.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void WheelDialog::accept() //20261112-0 add
{
    HmiObject draft = object();
    HmiProject candidate = m_project;
    QString error;
    bool replaced = false;
    int index;
    if (draft.id.isEmpty()) { draft.id = QUuid::createUuid().toString(QUuid::WithoutBraces); }
    for (index = 0; index < candidate.objects.size(); ++index) { if (candidate.objects.at(index).id == draft.id) { candidate.objects[index] = draft; replaced = true; break; } }
    if (!replaced) { candidate.objects.append(draft); }
    if (!candidate.validateModel(&error)) { QMessageBox::warning(this, QStringLiteral("滚轮控件属性无效"), error); return; }
    m_accepted = true; QDialog::accept();
}
