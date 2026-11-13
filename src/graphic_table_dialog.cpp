#include "graphic_dialog.h"
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : GraphicDialog::initializeTableUi
 * Description     : Recreate the native pure-image table appearance fields using an isolated draft and measured group geometry.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void GraphicDialog::initializeTableUi() //20261113-8 add
{
    QVBoxLayout *outer;
    QTabWidget *tabs;
    QWidget *page;
    QGroupBox *parameters;
    QGroupBox *fill;
    QGroupBox *preview;
    QGroupBox *opacity;
    QGroupBox *flags;
    QLabel *label;
    QDialogButtonBox *buttons;

    setWindowTitle(QStringLiteral("表格")); setObjectName(QStringLiteral("GraphicDialog")); resize(600,500); setFont(QFont(QStringLiteral("SimSun"),9));
    setStyleSheet(QStringLiteral("QComboBox, QSpinBox { background-color:#cce8cf; }"));
    outer = new QVBoxLayout(this); tabs = new QTabWidget(this); tabs->setObjectName(QStringLiteral("TableAppearanceTabs")); page = new QWidget(tabs); page->setObjectName(QStringLiteral("TableDlg"));
    tabs->addTab(page,QStringLiteral("外观设置")); outer->addWidget(tabs,1);
    parameters = new QGroupBox(page); parameters->setObjectName(QStringLiteral("groupBox")); parameters->setGeometry(10,20,251,131);
    label = new QLabel(QStringLiteral("横向个数："),parameters); label->setGeometry(10,10,80,30);
    m_rows = new QSpinBox(parameters); m_rows->setObjectName(QStringLiteral("m_hnumberspin")); m_rows->setRange(1,30); m_rows->setValue(m_draft.rows); m_rows->setGeometry(90,10,151,30);
    label = new QLabel(QStringLiteral("纵向个数："),parameters); label->setGeometry(10,50,80,30);
    m_columns = new QSpinBox(parameters); m_columns->setObjectName(QStringLiteral("m_Vnumberspin")); m_columns->setRange(1,30); m_columns->setValue(m_draft.columns); m_columns->setGeometry(90,50,151,30);
    label = new QLabel(QStringLiteral("线条类型："),parameters); label->setGeometry(10,90,80,22);
    m_style = new QComboBox(parameters); m_style->setObjectName(QStringLiteral("m_linetypecmb")); m_style->setGeometry(90,90,151,22);
    m_style->addItems({QStringLiteral("透明线"),QStringLiteral("实线"),QStringLiteral("虚线"),QStringLiteral("短划线"),QStringLiteral("点划线"),QStringLiteral("两点划线")}); m_style->setCurrentIndex(m_draft.lineStyle);
    fill = new QGroupBox(QStringLiteral("填充"),page); fill->setObjectName(QStringLiteral("groupBox_3")); fill->setGeometry(10,160,251,121);
    label = new QLabel(QStringLiteral("线条颜色："),fill); label->setGeometry(10,20,80,23);
    m_color = new QPushButton(fill); m_color->setObjectName(QStringLiteral("m_linecolorbtn")); m_color->setGeometry(90,20,151,23);
    label = new QLabel(QStringLiteral("边 框 色："),fill); label->setGeometry(10,50,80,23);
    m_frame = new QPushButton(fill); m_frame->setObjectName(QStringLiteral("m_bordercolorbtn")); m_frame->setGeometry(90,50,151,23);
    label = new QLabel(QStringLiteral("背 景 色："),fill); label->setGeometry(10,80,80,23);
    m_background = new QPushButton(fill); m_background->setObjectName(QStringLiteral("m_backcolorbtn")); m_background->setGeometry(90,80,151,23);
    preview = new QGroupBox(QStringLiteral("预览"),page); preview->setObjectName(QStringLiteral("groupBox_2")); preview->setGeometry(280,20,201,221);
    m_preview = new QLabel(preview); m_preview->setObjectName(QStringLiteral("GraphicPreview")); m_preview->setGeometry(10,20,181,181); m_preview->setStyleSheet(QStringLiteral("border:1px solid #808a99;"));
    opacity = new QGroupBox(QStringLiteral("透明度"),page); opacity->setObjectName(QStringLiteral("groupBox_5")); opacity->setGeometry(280,250,201,61);
    m_alpha = new QSpinBox(opacity); m_alpha->setObjectName(QStringLiteral("m_TransparentspBox")); m_alpha->setRange(0,255); m_alpha->setValue(m_draft.alpha); m_alpha->setGeometry(20,20,121,30);
    flags = new QGroupBox(page); flags->setObjectName(QStringLiteral("groupBox_4")); flags->setGeometry(10,330,471,61);
    m_vertical = new QCheckBox(QStringLiteral("垂直"),flags); m_vertical->setObjectName(QStringLiteral("m_vcheck")); m_vertical->setGeometry(10,20,70,20); m_vertical->setChecked(m_draft.showVertical);
    m_horizontal = new QCheckBox(QStringLiteral("水平"),flags); m_horizontal->setObjectName(QStringLiteral("m_hcheck")); m_horizontal->setGeometry(160,20,70,20); m_horizontal->setChecked(m_draft.showHorizontal);
    m_outline = new QCheckBox(QStringLiteral("描边"),flags); m_outline->setObjectName(QStringLiteral("m_bordercheck")); m_outline->setGeometry(310,20,70,20); m_outline->setChecked(m_draft.showOutline);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Help,this); buttons->setObjectName(QStringLiteral("GraphicButtons")); outer->addWidget(buttons);
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助"));
    connect(buttons,&QDialogButtonBox::accepted,this,&GraphicDialog::accept); connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);
    connect(buttons,&QDialogButtonBox::helpRequested,this,[this]() { QMessageBox::information(this,QStringLiteral("帮助"),QStringLiteral("表格是纯图像底盘。文字或数值控件需叠放在表格上方。横向个数设置行数，纵向个数设置列数。")); });
    connect(m_rows,&QSpinBox::valueChanged,this,&GraphicDialog::refreshTablePreview); connect(m_columns,&QSpinBox::valueChanged,this,&GraphicDialog::refreshTablePreview);
    connect(m_alpha,&QSpinBox::valueChanged,this,&GraphicDialog::refreshTablePreview); connect(m_style,&QComboBox::currentIndexChanged,this,&GraphicDialog::refreshTablePreview);
    connect(m_horizontal,&QCheckBox::toggled,this,&GraphicDialog::refreshTablePreview); connect(m_vertical,&QCheckBox::toggled,this,&GraphicDialog::refreshTablePreview); connect(m_outline,&QCheckBox::toggled,this,&GraphicDialog::refreshTablePreview);
    connect(m_color,&QPushButton::clicked,this,[this]() { QColor color = QColorDialog::getColor(m_draft.lineColor,this,QStringLiteral("线条颜色")); if (color.isValid()) { m_draft.lineColor = color; refreshTablePreview(); } });
    connect(m_frame,&QPushButton::clicked,this,[this]() { QColor color = QColorDialog::getColor(m_draft.frameColor,this,QStringLiteral("边框色")); if (color.isValid()) { m_draft.frameColor = color; refreshTablePreview(); } });
    connect(m_background,&QPushButton::clicked,this,[this]() { QColor color = QColorDialog::getColor(m_draft.background,this,QStringLiteral("背景色")); if (color.isValid()) { m_draft.background = color; refreshTablePreview(); } });
    refreshTablePreview();
}

/******************************************************************************
 * Function Name   : GraphicDialog::refreshTablePreview
 * Description     : Preserve all enabled hidden drafts and paint the same typed pure-image grid used by the canvas.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void GraphicDialog::refreshTablePreview() //20261113-8 add
{
    QPixmap pixels;
    QPainter painter;

    m_draft.rows = m_rows->value(); m_draft.columns = m_columns->value(); m_draft.lineStyle = m_style->currentIndex(); m_draft.alpha = m_alpha->value();
    m_draft.showHorizontal = m_horizontal->isChecked(); m_draft.showVertical = m_vertical->isChecked(); m_draft.showOutline = m_outline->isChecked();
    m_color->setStyleSheet(QStringLiteral("background:%1").arg(m_draft.lineColor.name())); m_frame->setStyleSheet(QStringLiteral("background:%1").arg(m_draft.frameColor.name())); m_background->setStyleSheet(QStringLiteral("background:%1").arg(m_draft.background.name()));
    pixels = QPixmap(m_preview->size()); pixels.fill(QColor(QStringLiteral("#cce8cf"))); painter.begin(&pixels); paintGraphic(&painter,QStringLiteral("table"),QRectF(10,10,160,160),m_draft); painter.end(); m_preview->setPixmap(pixels);
}
