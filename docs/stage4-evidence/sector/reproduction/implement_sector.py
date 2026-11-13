"""Implement the native default ninety-degree pie with measured creation and appearance rules."""
from pathlib import Path
r=Path(__file__).resolve().parent/'worktree'
def edit(relative,pairs):
 p=r/relative;s=p.read_text(encoding='utf-8')
 for a,b in pairs:
  assert a in s,(relative,a[:70]);s=s.replace(a,b)
 p.write_text(s,encoding='utf-8',newline='\n')
p=r/'src/graphic_properties.cpp';s=p.read_text(encoding='utf-8');old='|| type == QStringLiteral("freeline"); //20261113-2 change';assert old in s;s=s.replace(old,'|| type == QStringLiteral("freeline") || type == QStringLiteral("sector"); //20261113-7 change',1);s=s.replace('    if (type == QStringLiteral("freeline")) { return QStringLiteral("自由直线"); }','    if (type == QStringLiteral("sector")) { return QStringLiteral("扇形"); } //20261113-7 add\n    if (type == QStringLiteral("freeline")) { return QStringLiteral("自由直线"); }',1);old='    if (type == QStringLiteral("rect")) { path.addRect(rect); return path; }';new='''    if (type == QStringLiteral("sector")) //20261113-7 add: Native default prefix describes a center plus the 45-to-135 degree circular arc.
    {
        path.moveTo(rect.center()); path.arcTo(rect,45,90); return path; // Native cached prefix has eight elements; the fill closes implicitly and no unobserved ninth stroke element is added.
    }
'''+old;assert old in s;s=s.replace(old,new,1);s=s.replace('type == QStringLiteral("ellipse")); painter->setPen','(type == QStringLiteral("ellipse") || type == QStringLiteral("sector"))); painter->setPen',1);p.write_text(s,encoding='utf-8',newline='\n')
edit('src/native_shell_ui.cpp',[
 ('|| id == QStringLiteral("action_freeline"))','|| id == QStringLiteral("action_freeline") || id == QStringLiteral("action_ArcItem"))'),
 (': QStringLiteral("freeline"); if (object.type',': id == QStringLiteral("action_freeline") ? QStringLiteral("freeline") : QStringLiteral("sector"); if (object.type'),
 ('                    object.text.clear(); object.address.clear(); m_canvas->beginControlPlacement(object);','                    if (object.type == QStringLiteral("sector")) { object.graphic.lineStyle = 0; } //20261113-7 add: Native default sector has transparent stroke and white solid fill.\n                    object.text.clear(); object.address.clear(); m_canvas->beginControlPlacement(object);')])
p=r/'src/hmi_canvas.cpp';s=p.read_text(encoding='utf-8');marker='/******************************************************************************\n * Function Name   : ScreenCanvas::mouseMoveEvent';helper='''/******************************************************************************
 * Function Name   : sectorCreationRect
 * Description     : Use the observed larger drag dimension for default square sectors and retain a bounded local reverse-drag policy.
 * Input Parameters: origin, end, pageWidth, pageHeight
 * Return Value    : Page-bounded square creation box.
 * Date            : 2026.11.13
 ******************************************************************************/
static QRectF sectorCreationRect(const QPointF &origin, const QPointF &end, int pageWidth, int pageHeight) //20261113-7 add
{
    double xSign = end.x() < origin.x() ? -1 : 1;
    double ySign = end.y() < origin.y() ? -1 : 1;
    double side = qMax(qAbs(end.x()-origin.x()),qAbs(end.y()-origin.y()));

    side = qMin(side,xSign < 0 ? origin.x() : pageWidth-origin.x());
    side = qMin(side,ySign < 0 ? origin.y() : pageHeight-origin.y());
    return QRectF(origin,origin+QPointF(xSign*qMax(0.0,side),ySign*qMax(0.0,side))).normalized();
}

''';assert marker in s;s=s.replace(marker,helper+marker,1);s=s.replace('    QPoint point;\n\n    point = mapToScene(event->position().toPoint()).toPoint(); emit pageCoordinatesChanged(point);','    QPoint point;\n    QRectF draft; //20261113-7 add\n\n    point = mapToScene(event->position().toPoint()).toPoint(); emit pageCoordinatesChanged(point);',1);old='        m_draftRect->setRect(QRectF(m_dragOrigin, QPointF(point)).normalized()); event->accept(); return;';new='        draft = m_pendingObject.type == QStringLiteral("sector") ? sectorCreationRect(m_dragOrigin,point,m_pageWidth,m_pageHeight) : QRectF(m_dragOrigin,QPointF(point)).normalized(); //20261113-7 change\n        m_draftRect->setRect(draft); event->accept(); return;';assert old in s;s=s.replace(old,new,1);old='        geometry = m_draftRect->rect().toRect();';new=old+'\n        if (m_pendingObject.type == QStringLiteral("sector")) { geometry = sectorCreationRect(m_dragOrigin,mapToScene(event->position().toPoint()),m_pageWidth,m_pageHeight).toRect(); } //20261113-7 add: Include the final release coordinate in the same creation transaction.';assert old in s;s=s.replace(old,new,1);p.write_text(s,encoding='utf-8',newline='\n')
# Keep the measured sector layout inside the filled-shape branch, after its panel creation.
p=r/'src/graphic_dialog.cpp';s=p.read_text(encoding='utf-8');needle='    }\n    if (object.type == QStringLiteral("freeline")) //20261113-5 add:';block='''        if (object.type == QStringLiteral("sector")) //20261113-7 add: Native ArcDlg exposes appearance only; no speculative angle spin boxes.
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
''';assert needle in s;s=s.replace(needle,block+needle,1);s=s.replace('graphicStrokeOnly(m_original.type) || m_original.type == QStringLiteral("roundrect") ?', 'graphicStrokeOnly(m_original.type) || m_original.type == QStringLiteral("roundrect") || m_original.type == QStringLiteral("sector") ?',1);s=s.replace(': QRectF(30,20,100,100), preview);', ': m_original.type == QStringLiteral("sector") ? QRectF(20,30,100,100) : QRectF(30,20,100,100), preview);',1);p.write_text(s,encoding='utf-8',newline='\n')
print('Implemented default 90-degree sector, observed square creation and native appearance fields; angle editing remains unrecovered.')
