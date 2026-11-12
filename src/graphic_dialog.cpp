#include "graphic_dialog.h"
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : GraphicDialog::GraphicDialog
 * Description     : Recreate observed line fields with a private draft and a shared local renderer.
 * Input Parameters: object, parent
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
GraphicDialog::GraphicDialog(const HmiObject &object, QWidget *parent) //20261113-2 add
    : QDialog(parent), m_original(object), m_draft(object.graphic)
{
    QVBoxLayout *outer;
    QTabWidget *tabs;
    QWidget *page;
    QGridLayout *grid;
    QGroupBox *stroke;
    QGroupBox *shape;
    QGroupBox *fill;
    QGroupBox *preview;
    QGroupBox *opacity;
    QFormLayout *form;
    QVBoxLayout *column;
    QDialogButtonBox *buttons;
    QWidget *leftPane;
    QWidget *rightPane;
    QVBoxLayout *leftLayout;
    QVBoxLayout *rightLayout;
    QLabel *label;
    QComboBox *combo;
    int index;

    setWindowTitle(graphicName(object.type)); setObjectName(QStringLiteral("GraphicDialog")); resize(600, 480);
    if (graphicUsesVertices(object.type)) { setStyleSheet(QStringLiteral("QComboBox, QSpinBox { background-color:#cce8cf; }")); } //20261113-2 change: Closed native dialogs use white inputs.
    outer = new QVBoxLayout(this); tabs = new QTabWidget(this); page = new QWidget(tabs); grid = new QGridLayout(page);
    tabs->addTab(page, graphicName(object.type)); outer->addWidget(tabs, 1);
    stroke = new QGroupBox(QStringLiteral("属性"), page); form = new QFormLayout(stroke);
    m_style = new QComboBox(stroke); m_style->setObjectName(QStringLiteral("Line_Type"));
    m_style->addItems({QStringLiteral("透明线"), QStringLiteral("实线"), QStringLiteral("虚线"), QStringLiteral("短划线"), QStringLiteral("点划线"), QStringLiteral("两点划线")});
    m_style->setCurrentIndex(m_draft.lineStyle); form->addRow(QStringLiteral("线 型："), m_style);
    m_width = new QComboBox(stroke); m_width->setObjectName(QStringLiteral("Line_Width"));
    for (index = 1; index <= 9; ++index) { m_width->addItem(QString::number(index)); }
    m_width->setCurrentIndex(m_draft.lineWidth - 1); form->addRow(QStringLiteral("线 宽："), m_width); grid->addWidget(stroke, 0, 0);
    shape = new QGroupBox(QStringLiteral("形状"), page); form = new QFormLayout(shape);
    m_arrow = new QComboBox(shape); m_arrow->setObjectName(QStringLiteral("End_Arrow"));
    m_arrow->addItems({QStringLiteral("无形状"), QStringLiteral("实心三角形"), QStringLiteral("向右箭头"), QStringLiteral("折三角形"), QStringLiteral("菱形"), QStringLiteral("圆形")});
    m_arrow->setCurrentIndex(m_draft.endArrow); form->addRow(QStringLiteral("末端箭头："), m_arrow);
    m_cap = new QComboBox(shape); m_cap->setObjectName(QStringLiteral("End_Cap")); m_cap->addItems({QStringLiteral("矩形"), QStringLiteral("半圆")});
    m_cap->setCurrentIndex(m_draft.endCap); form->addRow(QStringLiteral("端点形状："), m_cap); grid->addWidget(shape, 1, 0);
    fill = new QGroupBox(QStringLiteral("填充"), page); form = new QFormLayout(fill); m_color = new QPushButton(fill); m_color->setObjectName(QStringLiteral("Line_Color"));
    form->addRow(QStringLiteral("边框色："), m_color); grid->addWidget(fill, 2, 0);
    preview = new QGroupBox(QStringLiteral("预览"), page); column = new QVBoxLayout(preview); m_preview = new QLabel(preview); m_preview->setObjectName(QStringLiteral("GraphicPreview"));
    m_preview->setFixedSize(130, 170); column->addWidget(m_preview, 0, Qt::AlignCenter); grid->addWidget(preview, 0, 1, 2, 1);
    opacity = new QGroupBox(QStringLiteral("透明度"), page); column = new QVBoxLayout(opacity); m_alpha = new QSpinBox(opacity); m_alpha->setObjectName(QStringLiteral("Alpha"));
    m_alpha->setRange(0, 255); m_alpha->setValue(m_draft.alpha); column->addWidget(m_alpha); grid->addWidget(opacity, 2, 1);
    grid->setContentsMargins(21, 9, 9, 9); grid->setHorizontalSpacing(20);
    grid->setColumnMinimumWidth(0, 280); grid->setColumnStretch(2, 1); grid->setRowStretch(3, 1);
    Q_FOREACH (combo, page->findChildren<QComboBox *>()) { combo->setFixedWidth(160); }
    Q_FOREACH (label, page->findChildren<QLabel *>()) { if (label != m_preview) { label->setMinimumWidth(60); } }
    if (!graphicUsesVertices(object.type)) //20261113-2 add: Filled graphics have no endpoint or arrow fields.
    {
        form = qobject_cast<QFormLayout *>(fill->layout());
        m_foreground = new QPushButton(fill); m_foreground->setObjectName(QStringLiteral("Fore_Color")); form->addRow(QStringLiteral("前景色："), m_foreground);
        m_background = new QPushButton(fill); m_background->setObjectName(QStringLiteral("Back_Color")); form->addRow(QStringLiteral("背景色："), m_background);
        m_pattern = new QComboBox(fill); m_pattern->setObjectName(QStringLiteral("Pattern")); m_pattern->setFixedWidth(160);
        m_pattern->addItems({QStringLiteral("透明"), QStringLiteral("纯色"), QStringLiteral("横向过渡"), QStringLiteral("横向对称过渡"), QStringLiteral("纵向过渡"), QStringLiteral("纵向对称过渡"), QStringLiteral("斜上过渡"), QStringLiteral("斜上对称过渡"), QStringLiteral("斜下过渡"), QStringLiteral("斜下对称过渡"), QStringLiteral("右上角辐射"), QStringLiteral("左上角辐射"), QStringLiteral("中心辐射")});
        m_pattern->setCurrentIndex(m_draft.fillStyle); form->addRow(QStringLiteral("样 式："), m_pattern);
        while (grid->count() > 0) { delete grid->takeAt(0); }
        shape->hide(); leftPane = new QWidget(page); rightPane = new QWidget(page); leftLayout = new QVBoxLayout(leftPane); rightLayout = new QVBoxLayout(rightPane);
        leftLayout->setContentsMargins(0,0,0,0); rightLayout->setContentsMargins(0,0,0,0); leftLayout->setSpacing(26); rightLayout->setSpacing(26);
        stroke->setFixedHeight(85); fill->setFixedHeight(153); preview->setFixedHeight(173); opacity->setFixedHeight(64); m_preview->setFixedSize(160,140);
        leftLayout->addWidget(stroke); leftLayout->addWidget(fill); leftLayout->addStretch(1); rightLayout->addWidget(preview); rightLayout->addWidget(opacity); rightLayout->addStretch(1);
        grid->addWidget(leftPane,0,0); grid->addWidget(rightPane,0,1); grid->setColumnMinimumWidth(1,180); grid->setRowStretch(0,1);
        connect(m_pattern, &QComboBox::currentIndexChanged, this, &GraphicDialog::refreshPreview);
        connect(m_foreground, &QPushButton::clicked, this, [this]() { QColor color = QColorDialog::getColor(m_draft.foreground, this, QStringLiteral("前景色")); if (color.isValid()) { m_draft.foreground = color; refreshPreview(); } });
        connect(m_background, &QPushButton::clicked, this, [this]() { QColor color = QColorDialog::getColor(m_draft.background, this, QStringLiteral("背景色")); if (color.isValid()) { m_draft.background = color; refreshPreview(); } });
    }
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("GraphicButtons")); outer->addWidget(buttons);
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助"));
    connect(buttons, &QDialogButtonBox::accepted, this, &GraphicDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, QStringLiteral("帮助"), QStringLiteral("线型、线宽、末端箭头、端点形状、边框色与透明度按工程保存。绘制后双击修改。")); });
    connect(m_style, &QComboBox::currentIndexChanged, this, &GraphicDialog::refreshPreview); connect(m_width, &QComboBox::currentIndexChanged, this, &GraphicDialog::refreshPreview);
    connect(m_arrow, &QComboBox::currentIndexChanged, this, &GraphicDialog::refreshPreview); connect(m_cap, &QComboBox::currentIndexChanged, this, &GraphicDialog::refreshPreview);
    connect(m_alpha, &QSpinBox::valueChanged, this, &GraphicDialog::refreshPreview);
    connect(m_color, &QPushButton::clicked, this, [this]() { QColor color = QColorDialog::getColor(m_draft.lineColor, this, QStringLiteral("边框色")); if (color.isValid()) { m_draft.lineColor = color; refreshPreview(); } });
    refreshPreview();
}

/******************************************************************************
 * Function Name   : GraphicDialog::refreshPreview
 * Description     : Update stroke-only fields and render an independent canonical preview line.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void GraphicDialog::refreshPreview() //20261113-2 add
{
    GraphicProperties preview;
    QPixmap pixels;
    QPainter painter;

    m_draft.lineStyle = m_style->currentIndex(); m_draft.lineWidth = m_width->currentIndex() + 1;
    m_draft.endArrow = m_arrow->currentIndex(); m_draft.endCap = m_cap->currentIndex(); m_draft.alpha = m_alpha->value();
    m_color->setStyleSheet(QStringLiteral("background:%1").arg(m_draft.lineColor.name()));
    if (m_pattern != nullptr) { m_draft.fillStyle = m_pattern->currentIndex(); m_foreground->setStyleSheet(QStringLiteral("background:%1").arg(m_draft.foreground.name())); m_background->setStyleSheet(QStringLiteral("background:%1").arg(m_draft.background.name())); }
    preview = m_draft; preview.points = {QPointF(0, 0), QPointF(1, 1)};
    pixels = QPixmap(m_preview->size()); pixels.fill(graphicUsesVertices(m_original.type) ? QColor(QStringLiteral("#cce8cf")) : QColor(Qt::white)); painter.begin(&pixels); paintGraphic(&painter, m_original.type, graphicUsesVertices(m_original.type) ? QRectF(15,35,100,100) : QRectF(30,20,100,100), preview); painter.end();
    m_preview->setPixmap(pixels);
}

/******************************************************************************
 * Function Name   : GraphicDialog::object
 * Description     : Return accepted style fields while preserving directed geometry and identity.
 * Input Parameters: None
 * Return Value    : Candidate object.
 * Date            : 2026.11.13
 ******************************************************************************/
HmiObject GraphicDialog::object() const //20261113-2 add
{
    HmiObject object = m_original;

    object.graphic = m_draft;
    return object;
}

/******************************************************************************
 * Function Name   : GraphicDialog::accept
 * Description     : Publish no object changes until all independent graphic fields validate.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void GraphicDialog::accept() //20261113-2 add
{
    QString error;

    refreshPreview();
    if (!m_draft.validate(m_original.type, &error)) { QMessageBox::warning(this, QStringLiteral("提示"), error); return; }
    QDialog::accept();
}
