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
    QGroupBox *radius; //20261113-2 add
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

    if (object.type == QStringLiteral("table")) { initializeTableUi(); return; } //20261113-8 add: Pure-image tables open appearance before placement.
    setWindowTitle(graphicName(object.type)); setObjectName(QStringLiteral("GraphicDialog")); resize(600, 480);
    if (object.type == QStringLiteral("polygon")) { resize(600,500); } //20261113-4 add: Retain the observed native polygon client dimensions.
    if (graphicStrokeOnly(object.type) || object.type == QStringLiteral("roundrect")) { setStyleSheet(QStringLiteral("QComboBox, QSpinBox { background-color:#cce8cf; }")); } //20261113-2 change: Native rounded-rectangle inputs retain the observed green style.
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
    if (!graphicStrokeOnly(object.type)) //20261113-2 add: Filled graphics have no endpoint or arrow fields.
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
        if (object.type == QStringLiteral("roundrect")) //20261113-2 add: Recreate the observed two independent 0..99 radius fields.
        {
            resize(600,500); grid->setHorizontalSpacing(28); leftPane->setFixedWidth(312); rightPane->setFixedWidth(190);
            grid->setColumnMinimumWidth(0,312); grid->setColumnMinimumWidth(1,190); leftLayout->setSpacing(32); rightLayout->setSpacing(7);
            stroke->setFixedHeight(105); fill->setFixedHeight(205); preview->setFixedHeight(153); m_preview->setFixedSize(150,120); m_alpha->setFixedWidth(100);
            qobject_cast<QFormLayout *>(stroke->layout())->setVerticalSpacing(20); qobject_cast<QFormLayout *>(fill->layout())->setVerticalSpacing(18); opacity->layout()->setContentsMargins(20,9,9,9); //20261113-2 change: Preserve native field row spacing inside the observed groups.
            Q_FOREACH (combo, page->findChildren<QComboBox *>()) { combo->setFixedWidth(200); }
            Q_FOREACH (label, leftPane->findChildren<QLabel *>()) { label->setMinimumWidth(74); }
            radius = new QGroupBox(QStringLiteral("半径"), page); radius->setFixedHeight(104); form = new QFormLayout(radius);
            m_radiusX = new QSpinBox(radius); m_radiusX->setObjectName(QStringLiteral("m_XR")); m_radiusX->setRange(0,99); m_radiusX->setValue(m_draft.radiusX);
            m_radiusY = new QSpinBox(radius); m_radiusY->setObjectName(QStringLiteral("m_YR")); m_radiusY->setRange(0,99); m_radiusY->setValue(m_draft.radiusY);
            m_radiusX->setFixedWidth(80); m_radiusY->setFixedWidth(80);
            form->addRow(QStringLiteral("x管道半径："),m_radiusX); form->addRow(QStringLiteral("y管道半径："),m_radiusY); rightLayout->insertWidget(2,radius);
            form->setVerticalSpacing(18); Q_FOREACH (label, radius->findChildren<QLabel *>()) { label->setMinimumWidth(80); }
            m_style->setObjectName(QStringLiteral("m_LineType")); m_width->setObjectName(QStringLiteral("m_LineWidth")); m_color->setObjectName(QStringLiteral("m_LineColorBtn"));
            m_foreground->setObjectName(QStringLiteral("m_FrontColorBtn")); m_background->setObjectName(QStringLiteral("m_BackColorBtn")); m_pattern->setObjectName(QStringLiteral("m_Pattern")); m_alpha->setObjectName(QStringLiteral("m_Alpha"));
            connect(m_radiusX,&QSpinBox::valueChanged,this,&GraphicDialog::refreshPreview); connect(m_radiusY,&QSpinBox::valueChanged,this,&GraphicDialog::refreshPreview);
        }
        if (object.type == QStringLiteral("polygon")) //20261113-4 add: Reproduce measured native group and field geometry without changing other families.
        {
            setFont(QFont(QStringLiteral("SimSun"),9)); setStyleSheet(QStringLiteral("QComboBox, QSpinBox { background-color:white; }"));
            grid->setContentsMargins(10,20,9,9); grid->setHorizontalSpacing(19); leftPane->setFixedWidth(281); rightPane->setFixedWidth(181);
            grid->setColumnMinimumWidth(0,281); grid->setColumnMinimumWidth(1,181); leftLayout->setSpacing(19); rightLayout->setSpacing(19);
            stroke->setFixedHeight(91); fill->setFixedHeight(151); preview->setFixedHeight(171); opacity->setFixedHeight(71);
            qobject_cast<QFormLayout *>(stroke->layout())->setContentsMargins(10,2,30,19); qobject_cast<QFormLayout *>(stroke->layout())->setVerticalSpacing(8);
            qobject_cast<QFormLayout *>(fill->layout())->setContentsMargins(10,2,30,19); qobject_cast<QFormLayout *>(fill->layout())->setVerticalSpacing(8);
            Q_FOREACH (combo, page->findChildren<QComboBox *>()) { combo->setFixedSize(161,22); }
            Q_FOREACH (label, leftPane->findChildren<QLabel *>()) { label->setMinimumWidth(74); }
            m_color->setFixedSize(161,22); m_foreground->setFixedSize(161,22); m_background->setFixedSize(161,22);
            m_alpha->setFixedSize(141,30); opacity->layout()->setContentsMargins(10,13,10,10);
            m_preview->setFixedSize(161,141); m_preview->setStyleSheet(QStringLiteral("border:1px solid #808a99;")); preview->layout()->setContentsMargins(10,2,10,10);
        }
        if (object.type == QStringLiteral("sector")) //20261113-7 add: Native ArcDlg exposes appearance only; no speculative angle spin boxes.
        {
            resize(600,500); setFont(QFont(QStringLiteral("SimSun"),9)); setStyleSheet(QStringLiteral("QComboBox, QSpinBox { background-color:#cce8cf; }"));
            grid->setContentsMargins(20,10,9,9); grid->setHorizontalSpacing(19); leftPane->setFixedWidth(291); rightPane->setFixedWidth(161);
            grid->setColumnMinimumWidth(0,291); grid->setColumnMinimumWidth(1,161); leftLayout->setSpacing(19); rightLayout->setSpacing(39);
            stroke->setFixedHeight(111); fill->setFixedHeight(171); preview->setFixedHeight(191); opacity->setFixedHeight(71);
            qobject_cast<QFormLayout *>(stroke->layout())->setContentsMargins(10,7,10,23); qobject_cast<QFormLayout *>(stroke->layout())->setVerticalSpacing(17);
            qobject_cast<QFormLayout *>(fill->layout())->setContentsMargins(10,7,10,18); qobject_cast<QFormLayout *>(fill->layout())->setVerticalSpacing(12);
            Q_FOREACH (combo, page->findChildren<QComboBox *>()) { combo->setFixedSize(181,23); }
            Q_FOREACH (label, leftPane->findChildren<QLabel *>()) { label->setMinimumWidth(84); }
            m_color->setFixedSize(181,23); m_foreground->setFixedSize(181,23); m_background->setFixedSize(181,23);
            m_alpha->setFixedSize(121,30); opacity->layout()->setContentsMargins(20,2,20,21);
            m_preview->setFixedSize(141,161); m_preview->setStyleSheet(QStringLiteral("border:1px solid #808a99;")); preview->layout()->setContentsMargins(10,2,10,10);
            m_style->setObjectName(QStringLiteral("m_LineType")); m_width->setObjectName(QStringLiteral("m_LineWidth")); m_color->setObjectName(QStringLiteral("m_LineColorBtn"));
            m_foreground->setObjectName(QStringLiteral("m_FrontColorBtn")); m_background->setObjectName(QStringLiteral("m_BackColorBtn")); m_pattern->setObjectName(QStringLiteral("m_Pattern")); m_alpha->setObjectName(QStringLiteral("m_Alpha"));
        }
    }
    if (object.type == QStringLiteral("freeline")) //20261113-5 add: Match measured native freeline groups without promoting unknown sampling behavior.
    {
        resize(605,471); setFont(QFont(QStringLiteral("SimSun"),9));
        while (grid->count() > 0) { delete grid->takeAt(0); }
        stroke->setGeometry(20,10,281,91); shape->setGeometry(20,110,281,101); fill->setGeometry(20,220,281,61);
        preview->setGeometry(320,10,151,201); opacity->setGeometry(320,220,151,61);
        qobject_cast<QFormLayout *>(stroke->layout())->setContentsMargins(10,2,30,19); qobject_cast<QFormLayout *>(stroke->layout())->setVerticalSpacing(8);
        qobject_cast<QFormLayout *>(shape->layout())->setContentsMargins(10,2,30,29); qobject_cast<QFormLayout *>(shape->layout())->setVerticalSpacing(8);
        qobject_cast<QFormLayout *>(fill->layout())->setContentsMargins(10,2,30,19);
        Q_FOREACH (combo, page->findChildren<QComboBox *>()) { combo->setFixedSize(161,22); }
        Q_FOREACH (label, page->findChildren<QLabel *>()) { if (label != m_preview) { label->setMinimumWidth(74); } }
        m_color->setFixedSize(161,22); m_alpha->setFixedSize(121,30); opacity->layout()->setContentsMargins(20,2,10,11);
        m_preview->setFixedSize(131,171); m_preview->setStyleSheet(QStringLiteral("border:1px solid #808a99;")); preview->layout()->setContentsMargins(10,2,10,10);
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

    if (m_original.type == QStringLiteral("table")) { refreshTablePreview(); return; } //20261113-8 add
    m_draft.lineStyle = m_style->currentIndex(); m_draft.lineWidth = m_width->currentIndex() + 1;
    m_draft.endArrow = m_arrow->currentIndex(); m_draft.endCap = m_cap->currentIndex(); m_draft.alpha = m_alpha->value();
    m_color->setStyleSheet(QStringLiteral("background:%1").arg(m_draft.lineColor.name()));
    if (m_pattern != nullptr) { m_draft.fillStyle = m_pattern->currentIndex(); m_foreground->setStyleSheet(QStringLiteral("background:%1").arg(m_draft.foreground.name())); m_background->setStyleSheet(QStringLiteral("background:%1").arg(m_draft.background.name())); }
    if (m_radiusX != nullptr) { m_draft.radiusX = m_radiusX->value(); m_draft.radiusY = m_radiusY->value(); } //20261113-2 add: Preview and committed object share the same bounded radius draft.
    preview = m_draft; preview.points = {QPointF(0, 0), QPointF(1, 1)};
    if (m_original.type == QStringLiteral("polygon")) { preview.points = {QPointF(0,0), QPointF(1,0), QPointF(.75,1)}; } //20261113-4 add: Canonical triangle preview does not overwrite the object vertices.
    pixels = QPixmap(m_preview->size()); pixels.fill(graphicStrokeOnly(m_original.type) || m_original.type == QStringLiteral("roundrect") || m_original.type == QStringLiteral("sector") ? QColor(QStringLiteral("#cce8cf")) : QColor(Qt::white)); painter.begin(&pixels); paintGraphic(&painter, m_original.type, graphicStrokeOnly(m_original.type) ? QRectF(15,35,100,100) : m_original.type == QStringLiteral("roundrect") ? QRectF(25,10,100,100) : m_original.type == QStringLiteral("sector") ? QRectF(20,30,100,100) : QRectF(30,20,100,100), preview); painter.end(); //20261113-2 change: Match the rounded-rectangle native preview inset and background.
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
