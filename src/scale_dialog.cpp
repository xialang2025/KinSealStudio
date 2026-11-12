#include "scale_dialog.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <limits>

/******************************************************************************
 * Function Name   : paintScale
 * Description     : Render clipped axis, major/minor lines and labels using observed orientation and count semantics.
 * Input Parameters: painter, rect, properties
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void paintScale(QPainter *painter, const QRectF &rect, const ScaleProperties &properties) //20261112-0 add
{
    QFont font(QStringLiteral("宋体"));
    QRectF bounds;
    QRectF label;
    QPointF axisStart;
    QPointF axisEnd;
    QPointF point;
    QPointF inward;
    double fraction;
    double value;
    double axis;
    int index;
    int minor;
    bool horizontal;
    QString error;
    if (!properties.validate(&error) || rect.width() <= 0 || rect.height() <= 0) { return; }
    horizontal = properties.position < 2; font.setPixelSize(properties.fontSize); font.setStyleStrategy(QFont::NoAntialias);
    painter->save(); painter->setClipRect(rect, Qt::IntersectClip); painter->setRenderHint(QPainter::Antialiasing, false); painter->setRenderHint(QPainter::TextAntialiasing, false); painter->setFont(font);
    bounds = rect.adjusted(2, properties.fontSize * 0.5 + 1, -2, -properties.fontSize * 0.5 - 1);
    if (horizontal)
    {
        bounds = rect.adjusted(properties.fontSize, 2, -properties.fontSize, -2);
        axis = properties.position == 0 ? bounds.top() + properties.fontSize + 8 : bounds.bottom() - properties.fontSize - 8;
        axisStart = QPointF(bounds.left(), axis); axisEnd = QPointF(bounds.right(), axis);
        inward = QPointF(0, properties.position == 0 ? -1 : 1);
    }
    else
    {
        axis = properties.position == 3 ? bounds.left() : bounds.right();
        axisStart = QPointF(axis, bounds.top()); axisEnd = QPointF(axis, bounds.bottom());
        inward = QPointF(properties.position == 3 ? 1 : -1, 0);
    }
    painter->setPen(QPen(properties.lineColor, 1)); painter->drawLine(axisStart, axisEnd);
    for (index = 0; index < properties.majorCount; ++index)
    {
        fraction = double(index) / (properties.majorCount - 1);
        point = axisStart + (axisEnd - axisStart) * fraction;
        painter->setPen(QPen(properties.lineColor, 1)); painter->drawLine(point, point + inward * 24);
        value = properties.direction == 0 ? properties.maximum - (double(properties.maximum) - properties.minimum) * fraction :
                                           properties.minimum + (double(properties.maximum) - properties.minimum) * fraction;
        if (properties.showValues)
        {
            if (horizontal) { label = QRectF(point.x() - 80, properties.position == 0 ? point.y() - properties.fontSize - 8 : point.y() + 8, 160, properties.fontSize + 2); }
            else { label = QRectF(properties.position == 3 ? point.x() + 32 : point.x() - 112, point.y() - properties.fontSize * 0.5 - 1, 80, properties.fontSize + 2); }
            painter->setPen(properties.textColor); painter->drawText(label, horizontal ? Qt::AlignCenter : (properties.position == 3 ? Qt::AlignLeft | Qt::AlignVCenter : Qt::AlignRight | Qt::AlignVCenter), QString::number(value, 'f', properties.decimals));
        }
        if (index == properties.majorCount - 1) { continue; }
        for (minor = 1; minor <= properties.minorCount; ++minor)
        {
            fraction = (index + double(minor) / (properties.minorCount + 1)) / (properties.majorCount - 1);
            point = axisStart + (axisEnd - axisStart) * fraction;
            painter->setPen(QPen(properties.lineColor, 1)); painter->drawLine(point, point + inward * 6);
        }
    }
    painter->restore();
}

/******************************************************************************
 * Function Name   : ScaleDialog::ScaleDialog
 * Description     : Restore the native scale's sole appearance page without adding addresses or permission controls.
 * Input Parameters: object, parent
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
ScaleDialog::ScaleDialog(const HmiObject &object, QWidget *parent) //20261112-0 add
    : QDialog(parent), m_original(object), m_draft(object.scale)
{
    QVBoxLayout *root;
    QHBoxLayout *content;
    QTabWidget *tabs;
    QWidget *page;
    QGridLayout *grid;
    QGroupBox *attributes;
    QFormLayout *attributesLayout;
    QGroupBox *positions;
    QGridLayout *positionsLayout;
    QGroupBox *range;
    QFormLayout *rangeLayout;
    QGroupBox *colors;
    QFormLayout *colorLayout;
    QGroupBox *ticks;
    QFormLayout *tickLayout;
    QGroupBox *preview;
    QVBoxLayout *previewLayout;
    QRadioButton *radio;
    QPushButton *textColor;
    QPushButton *lineColor;
    QDialogButtonBox *buttons;
    QStringList positionNames;
    int index;

    setObjectName(QStringLiteral("ScaleDialog")); setWindowTitle(QStringLiteral("刻度")); resize(728, 507); setFont(QFont(QStringLiteral("SimSun"), 9));
    setStyleSheet(QStringLiteral("QComboBox,QSpinBox,QLineEdit { background:#caeace; } QGroupBox { background:white; }"));
    root = new QVBoxLayout(this); content = new QHBoxLayout(); tabs = new QTabWidget(this); page = new QWidget(tabs); grid = new QGridLayout(page);
    attributes = new QGroupBox(QStringLiteral("属性"), page); attributesLayout = new QFormLayout(attributes);
    m_fontSize = new QComboBox(attributes); m_fontSize->setObjectName(QStringLiteral("ScaleFontSize"));
    for (index = 5; index <= 20; ++index) { m_fontSize->addItem(QString::number(index)); }
    m_fontSize->setCurrentText(QString::number(m_draft.fontSize));
    m_direction = new QComboBox(attributes); m_direction->setObjectName(QStringLiteral("ScaleDirection"));
    attributesLayout->addRow(QStringLiteral("字体大小:"), m_fontSize); attributesLayout->addRow(QStringLiteral("文本方向:"), m_direction);
    positions = new QGroupBox(QStringLiteral("位置"), page); positionsLayout = new QGridLayout(positions); m_position = new QButtonGroup(this);
    positionNames = {QStringLiteral("上"), QStringLiteral("下"), QStringLiteral("左"), QStringLiteral("右")};
    for (index = 0; index < positionNames.size(); ++index)
    {
        radio = new QRadioButton(positionNames.at(index), positions); radio->setObjectName(QStringLiteral("ScalePosition%1").arg(index)); m_position->addButton(radio, index);
        radio->setChecked(index == m_draft.position); positionsLayout->addWidget(radio, index % 2, index / 2);
    }
    range = new QGroupBox(QStringLiteral("显示范围"), page); rangeLayout = new QFormLayout(range);
    m_minimum = new QLineEdit(QString::number(m_draft.minimum), range); m_minimum->setObjectName(QStringLiteral("ScaleMinimum")); m_minimum->setMaxLength(15);
    m_maximum = new QLineEdit(QString::number(m_draft.maximum), range); m_maximum->setObjectName(QStringLiteral("ScaleMaximum")); m_maximum->setMaxLength(15);
    m_minimum->setValidator(new QIntValidator(std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), m_minimum));
    m_maximum->setValidator(new QIntValidator(std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), m_maximum));
    m_decimals = new QComboBox(range); m_decimals->setObjectName(QStringLiteral("ScaleDecimals")); m_decimals->addItems({QStringLiteral("0"), QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3")}); m_decimals->setCurrentIndex(m_draft.decimals);
    rangeLayout->addRow(QStringLiteral("最小值:"), m_minimum); rangeLayout->addRow(QStringLiteral("最大值:"), m_maximum); rangeLayout->addRow(QStringLiteral("小数位:"), m_decimals);
    colors = new QGroupBox(page); colorLayout = new QFormLayout(colors); textColor = new QPushButton(colors); lineColor = new QPushButton(colors);
    textColor->setObjectName(QStringLiteral("ScaleTextColor")); lineColor->setObjectName(QStringLiteral("ScaleLineColor"));
    textColor->setStyleSheet(QStringLiteral("background:%1").arg(m_draft.textColor.name())); lineColor->setStyleSheet(QStringLiteral("background:%1").arg(m_draft.lineColor.name()));
    colorLayout->addRow(QStringLiteral("文本色:"), textColor); colorLayout->addRow(QStringLiteral("线条色:"), lineColor);
    ticks = new QGroupBox(page); tickLayout = new QFormLayout(ticks);
    m_showValues = new QCheckBox(QStringLiteral("显示刻度值"), ticks); m_showValues->setObjectName(QStringLiteral("ScaleShowValues")); m_showValues->setChecked(m_draft.showValues);
    m_major = new QSpinBox(ticks); m_major->setObjectName(QStringLiteral("ScaleMajorCount")); m_major->setRange(2, 20); m_major->setValue(m_draft.majorCount);
    m_minor = new QSpinBox(ticks); m_minor->setObjectName(QStringLiteral("ScaleMinorCount")); m_minor->setRange(0, 5); m_minor->setValue(m_draft.minorCount);
    tickLayout->addRow(m_showValues); tickLayout->addRow(QStringLiteral("主刻度分割数:"), m_major); tickLayout->addRow(QStringLiteral("副刻度分割数:"), m_minor);
    grid->addWidget(attributes, 0, 0); grid->addWidget(positions, 0, 1); grid->addWidget(range, 1, 0); grid->addWidget(colors, 1, 1); grid->addWidget(ticks, 2, 0, 1, 2); grid->setRowStretch(3, 1);
    tabs->addTab(page, QStringLiteral("外观设置")); content->addWidget(tabs, 1);
    preview = new QGroupBox(QStringLiteral("预览"), this); previewLayout = new QVBoxLayout(preview);
    m_preview = new QLabel(preview); m_preview->setObjectName(QStringLiteral("ScalePreview")); m_preview->setFixedSize(150, 171); previewLayout->addWidget(m_preview); content->addWidget(preview, 0, Qt::AlignTop); root->addLayout(content, 1);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("ScaleButtons"));
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助"));
    buttons->button(QDialogButtonBox::Ok)->setMinimumSize(72, 28); buttons->button(QDialogButtonBox::Cancel)->setMinimumSize(72, 28); buttons->button(QDialogButtonBox::Help)->setMinimumSize(72, 28); root->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &ScaleDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, QStringLiteral("刻度帮助"), QStringLiteral("参见design_plan/刻度功能与交互复刻手册.md。位置决定横向或竖向，方向决定数值排列；主刻度为带值刻度数量，副刻度为相邻主刻度间短线数量。")); });
    connect(m_position, &QButtonGroup::idClicked, this, [this]() { updateDirections(); refreshPreview(); });
    connect(m_fontSize, &QComboBox::currentIndexChanged, this, [this]() { refreshPreview(); }); connect(m_direction, &QComboBox::currentIndexChanged, this, [this]() { refreshPreview(); });
    connect(m_minimum, &QLineEdit::textChanged, this, [this]() { refreshPreview(); }); connect(m_maximum, &QLineEdit::textChanged, this, [this]() { refreshPreview(); });
    connect(m_decimals, &QComboBox::currentIndexChanged, this, [this]() { refreshPreview(); }); connect(m_showValues, &QCheckBox::toggled, this, [this]() { refreshPreview(); });
    connect(m_major, &QSpinBox::valueChanged, this, [this]() { refreshPreview(); }); connect(m_minor, &QSpinBox::valueChanged, this, [this]() { refreshPreview(); });
    connect(textColor, &QPushButton::clicked, this, [this, textColor]() {
        QColor color = QColorDialog::getColor(m_draft.textColor, this, QStringLiteral("选择颜色"));
        if (color.isValid()) { m_draft.textColor = color; textColor->setStyleSheet(QStringLiteral("background:%1").arg(color.name())); refreshPreview(); }
    });
    connect(lineColor, &QPushButton::clicked, this, [this, lineColor]() {
        QColor color = QColorDialog::getColor(m_draft.lineColor, this, QStringLiteral("选择颜色"));
        if (color.isValid()) { m_draft.lineColor = color; lineColor->setStyleSheet(QStringLiteral("background:%1").arg(color.name())); refreshPreview(); }
    });
    updateDirections(); m_direction->setCurrentIndex(m_draft.direction); refreshPreview();
}

/******************************************************************************
 * Function Name   : ScaleDialog::updateDirections
 * Description     : Switch vertical up/down labels to horizontal left/right labels while preserving their independent index.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void ScaleDialog::updateDirections() //20261112-0 add
{
    QSignalBlocker blocker(m_direction);
    int direction = qMax(0, m_direction->currentIndex());
    m_direction->clear(); m_direction->addItems(m_position->checkedId() < 2 ? QStringList{QStringLiteral("向左"), QStringLiteral("向右")} : QStringList{QStringLiteral("向上"), QStringLiteral("向下")}); m_direction->setCurrentIndex(direction);
}

/******************************************************************************
 * Function Name   : ScaleDialog::readProperties
 * Description     : Read all detached inputs without converting invalid or overflowing text to zero.
 * Input Parameters: properties, error
 * Return Value    : True with valid observed scale fields.
 * Date            : 2026.11.12
 ******************************************************************************/
bool ScaleDialog::readProperties(ScaleProperties *properties, QString *error) const //20261112-0 add
{
    ScaleProperties candidate = m_draft;
    bool minimumValid;
    bool maximumValid;
    candidate.minimum = m_minimum->text().toInt(&minimumValid); candidate.maximum = m_maximum->text().toInt(&maximumValid);
    if (!minimumValid || !maximumValid)
    { if (error != nullptr) { *error = QStringLiteral("显示范围必须为有效32位整数。"); } return false; }
    candidate.fontSize = m_fontSize->currentText().toInt(); candidate.position = m_position->checkedId(); candidate.direction = m_direction->currentIndex(); candidate.decimals = m_decimals->currentIndex();
    candidate.showValues = m_showValues->isChecked(); candidate.majorCount = m_major->value(); candidate.minorCount = m_minor->value();
    if (!candidate.validate(error)) { return false; }
    *properties = candidate; return true;
}

/******************************************************************************
 * Function Name   : ScaleDialog::refreshPreview
 * Description     : Render a static 100-pixel scale using the same code as the canvas and preserve the last valid preview during typing.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void ScaleDialog::refreshPreview() //20261112-0 add
{
    ScaleProperties properties;
    QPixmap image(m_preview->size());
    QPainter painter;
    if (!readProperties(&properties)) { return; }
    image.fill(QColor(240, 240, 240)); painter.begin(&image); paintScale(&painter, QRectF(40, 30, 100, 116), properties); painter.end(); m_preview->setPixmap(image);
}

/******************************************************************************
 * Function Name   : ScaleDialog::object
 * Description     : Return an independent typed scale object after the complete property validation.
 * Input Parameters: None
 * Return Value    : Scale draft without publishing live state.
 * Date            : 2026.11.12
 ******************************************************************************/
HmiObject ScaleDialog::object() const //20261112-0 add
{
    HmiObject result = m_original;
    readProperties(&result.scale); return result;
}

/******************************************************************************
 * Function Name   : ScaleDialog::accept
 * Description     : Keep invalid input open and reproduce the native minimum/maximum message.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void ScaleDialog::accept() //20261112-0 add
{
    ScaleProperties properties;
    QString error;
    if (!readProperties(&properties, &error)) { QMessageBox::warning(this, QStringLiteral("KinSealStudio"), error); return; }
    QDialog::accept();
}
