"""Add typed pure-image table properties and preserve each family's exact JSON key set."""
from pathlib import Path
r=Path(__file__).resolve().parent/'worktree'
def edit(relative,pairs):
 p=r/relative;s=p.read_text(encoding='utf-8')
 for a,b in pairs:
  assert a in s,(relative,a[:80]);s=s.replace(a,b)
 p.write_text(s,encoding='utf-8',newline='\n')
edit('src/graphic_properties.h', [('    int radiusX = 10;', '''    int rows = 4; //20261113-8 add: Pure-image table counts are independent of any data-grid control.
    int columns = 4;
    bool showHorizontal = true;
    bool showVertical = true;
    bool showOutline = true;
    QColor frameColor = Qt::black;
    int radiusX = 10;''')])
p=r/'src/graphic_properties.cpp';s=p.read_text(encoding='utf-8');a='|| type == QStringLiteral("sector"); //20261113-7 change';assert a in s;s=s.replace(a,'|| type == QStringLiteral("sector") || type == QStringLiteral("table"); //20261113-8 change',1);s=s.replace('    if (type == QStringLiteral("sector")) { return QStringLiteral("扇形"); }','    if (type == QStringLiteral("table")) { return QStringLiteral("表格"); } //20261113-8 add\n    if (type == QStringLiteral("sector")) { return QStringLiteral("扇形"); }',1)
a='    json = {{QStringLiteral("lineStyle"), lineStyle},';b='''    if (type == QStringLiteral("table")) //20261113-8 add: No cell data, width, arrow, fill-pattern or angle keys belong to this family.
    {
        return {{QStringLiteral("rows"),rows},{QStringLiteral("columns"),columns},
                {QStringLiteral("horizontal"),showHorizontal},{QStringLiteral("vertical"),showVertical},{QStringLiteral("outline"),showOutline},
                {QStringLiteral("lineStyle"),lineStyle},{QStringLiteral("lineColor"),lineColor.name(QColor::HexArgb)},
                {QStringLiteral("frameColor"),frameColor.name(QColor::HexArgb)},{QStringLiteral("background"),background.name(QColor::HexArgb)},{QStringLiteral("alpha"),alpha}};
    }
'''+a;assert a in s;s=s.replace(a,b,1)
a='    candidate.lineStyle = json.value(QStringLiteral("lineStyle")).toInt();';b='''    if (type == QStringLiteral("table")) //20261113-8 add: Parse the bounded table draft atomically before ordinary stroke/fill fields.
    {
        candidate.rows = json.value(QStringLiteral("rows")).toInt(); candidate.columns = json.value(QStringLiteral("columns")).toInt();
        candidate.showHorizontal = json.value(QStringLiteral("horizontal")).toBool(); candidate.showVertical = json.value(QStringLiteral("vertical")).toBool(); candidate.showOutline = json.value(QStringLiteral("outline")).toBool();
        candidate.lineStyle = json.value(QStringLiteral("lineStyle")).toInt(); candidate.alpha = json.value(QStringLiteral("alpha")).toInt();
        candidate.lineColor = QColor(json.value(QStringLiteral("lineColor")).toString()); candidate.frameColor = QColor(json.value(QStringLiteral("frameColor")).toString()); candidate.background = QColor(json.value(QStringLiteral("background")).toString());
        if (!candidate.validate(type,error)) { return false; }
        *output = candidate; return true;
    }
'''+a;assert a in s;s=s.replace(a,b,1)
a='    if (!isSupportedGraphic(type) || lineStyle < 0';b='''    if (type == QStringLiteral("table")) //20261113-8 add: Pens stay opaque; the separate alpha affects background only.
    {
        if (rows < 1 || rows > 30 || columns < 1 || columns > 30 || lineStyle < 0 || lineStyle > 5 || alpha < 0 || alpha > 255 || lineWidth != 1 || endArrow != 0 || endCap != 0 || !lineColor.isValid() || lineColor.alpha() != 255 || !frameColor.isValid() || frameColor.alpha() != 255 || !background.isValid() || background.alpha() != 255)
        { return graphicError(error,QStringLiteral("表格行列必须为1到30，线型/透明度/独立颜色必须有效，且不支持线宽或箭头。")); }
        return true;
    }
'''+a;assert a in s;s=s.replace(a,b,1);s=s.replace('if (type == QStringLiteral("rect")) { path.addRect(rect); return path; }','if (type == QStringLiteral("rect") || type == QStringLiteral("table")) { path.addRect(rect); return path; } //20261113-8 change',1)
marker='/******************************************************************************\n * Function Name   : paintGraphic';helper='''/******************************************************************************
 * Function Name   : paintTableGraphic
 * Description     : Render background alpha independently from opaque internal grid and solid outer frame.
 * Input Parameters: painter, rect, properties
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
static void paintTableGraphic(QPainter *painter, const QRectF &rect, const GraphicProperties &properties) //20261113-8 add
{
    QColor background = properties.background;
    QPainterPath grid;
    int index;
    const Qt::PenStyle styles[] = {Qt::NoPen,Qt::SolidLine,Qt::DashLine,Qt::DotLine,Qt::DashDotLine,Qt::DashDotDotLine};

    background.setAlpha(properties.alpha); painter->save(); painter->setRenderHint(QPainter::Antialiasing,false); painter->fillRect(rect,background);
    if (properties.showHorizontal) { for (index=1;index<properties.rows;++index) { grid.moveTo(rect.left(),rect.top()+rect.height()*index/properties.rows); grid.lineTo(rect.right(),rect.top()+rect.height()*index/properties.rows); } }
    if (properties.showVertical) { for (index=1;index<properties.columns;++index) { grid.moveTo(rect.left()+rect.width()*index/properties.columns,rect.top()); grid.lineTo(rect.left()+rect.width()*index/properties.columns,rect.bottom()); } }
    painter->setBrush(Qt::NoBrush); painter->setPen(QPen(properties.lineColor,1,styles[properties.lineStyle])); painter->drawPath(grid);
    if (properties.showOutline) { painter->setPen(QPen(properties.frameColor,1,Qt::SolidLine)); painter->drawRect(rect); } // Native line-style zero suppresses the grid while the checked solid frame stays visible.
    painter->restore();
}

''';assert marker in s;s=s.replace(marker,helper+marker,1);a='    if (painter == nullptr || !properties.validate(type)) { return; }';b=a+'\n    if (type == QStringLiteral("table")) { paintTableGraphic(painter,rect,properties); return; } //20261113-8 add';assert a in s;s=s.replace(a,b,1);p.write_text(s,encoding='utf-8',newline='\n')
edit('CMakeLists.txt',[('    src/graphic_dialog.cpp','    src/graphic_dialog.cpp\n    src/graphic_table_dialog.cpp')])
edit('src/graphic_dialog.h',[('class QComboBox;','class QComboBox;\nclass QCheckBox;'),('    QComboBox *m_style;','    QComboBox *m_style = nullptr;'),('    QComboBox *m_width;','    QComboBox *m_width = nullptr;'),('    QComboBox *m_arrow;','    QComboBox *m_arrow = nullptr;'),('    QComboBox *m_cap;','    QComboBox *m_cap = nullptr;'),('    QSpinBox *m_alpha;','    QSpinBox *m_alpha = nullptr;'),('    QLabel *m_preview;','    QLabel *m_preview = nullptr;'),('    QPushButton *m_color;','    QPushButton *m_color = nullptr;'),('    void refreshPreview();','''    QSpinBox *m_rows = nullptr; //20261113-8 add
    QSpinBox *m_columns = nullptr;
    QCheckBox *m_horizontal = nullptr;
    QCheckBox *m_vertical = nullptr;
    QCheckBox *m_outline = nullptr;
    QPushButton *m_frame = nullptr;
    void initializeTableUi();
    void refreshTablePreview();
    void refreshPreview();''')])
edit('src/graphic_dialog.cpp',[('    setWindowTitle(graphicName(object.type));','    if (object.type == QStringLiteral("table")) { initializeTableUi(); return; } //20261113-8 add: Pure-image tables open appearance before placement.\n    setWindowTitle(graphicName(object.type));'),('    m_draft.lineStyle = m_style->currentIndex();','    if (m_original.type == QStringLiteral("table")) { refreshTablePreview(); return; } //20261113-8 add\n    m_draft.lineStyle = m_style->currentIndex();')])
edit('src/native_shell_ui.cpp',[('#include "main_window.h"','#include "main_window.h"\n#include "graphic_dialog.h" //20261113-8 add'),('            else if (id == QStringLiteral("action_qrcode"))','''            else if (id == QStringLiteral("action_table")) //20261113-8 add: The native table workflow requires accepted appearance before drawing.
            {
                connect(action,&QAction::triggered,this,[this]() {
                    HmiObject object;
                    GraphicDialog *dialog;

                    m_canvas->cancelInteraction(); object.type = QStringLiteral("table"); object.text.clear(); object.address.clear(); object.graphic.background = QColor(QStringLiteral("#e0e0e0"));
                    dialog = new GraphicDialog(object,this);
                    if (dialog->exec() == QDialog::Accepted) { object = dialog->object(); delete dialog; m_canvas->beginControlPlacement(object); }
                    else { delete dialog; }
                });
            }
            else if (id == QStringLiteral("action_qrcode"))''')])
print('Added pure-image table model, renderer and property-first command routing.')
