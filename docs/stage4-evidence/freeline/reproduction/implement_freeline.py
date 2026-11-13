"""Implement only the observed two-endpoint native freeline slice, without claiming recovered sampling."""
from pathlib import Path
root=Path(__file__).resolve().parent/'worktree'
def edit(relative,pairs):
 p=root/relative;s=p.read_text(encoding='utf-8')
 for a,b in pairs:
  assert a in s,(relative,a[:80]);s=s.replace(a,b)
 p.write_text(s,encoding='utf-8',newline='\n')
edit('src/graphic_properties.cpp',[
 ('|| type == QStringLiteral("polygon");','|| type == QStringLiteral("polygon") || type == QStringLiteral("freeline");'),
 ('return type == QStringLiteral("line") || type == QStringLiteral("polyline");','return type == QStringLiteral("line") || type == QStringLiteral("polyline") || type == QStringLiteral("freeline"); //20261113-5 change: The observed native default has two path elements, not a proven sampled freehand path.'),
 ('    if (type == QStringLiteral("polygon")) { return QStringLiteral("多边形"); }','    if (type == QStringLiteral("freeline")) { return QStringLiteral("自由直线"); } //20261113-5 add\n    if (type == QStringLiteral("polygon")) { return QStringLiteral("多边形"); }'),
 ('    if (type == QStringLiteral("line") && (points.size() != 2', '    if (type == QStringLiteral("freeline") && (points.size() != 2 || points.first() == points.last())) { return graphicError(error, QStringLiteral("自由直线当前首尾切片只支持两个不同端点；采样路径尚未恢复。")); } //20261113-5 add: Reject unverified multi-point freeline input rather than silently reducing it.\n    if (type == QStringLiteral("line") && (points.size() != 2')])
edit('src/native_shell_ui.cpp',[
 ('|| id == QStringLiteral("action_polygon"))','|| id == QStringLiteral("action_polygon") || id == QStringLiteral("action_freeline"))'),
 (': QStringLiteral("polygon"); if (object.type', ': id == QStringLiteral("action_polygon") ? QStringLiteral("polygon") : QStringLiteral("freeline"); if (object.type')])
edit('src/graphic_dialog.cpp',[
 ('    buttons = new QDialogButtonBox(QDialogButtonBox::Ok', '''    if (object.type == QStringLiteral("freeline")) //20261113-5 add: Match measured native freeline groups without promoting unknown sampling behavior.
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
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok''')])
print('Implemented independent two-endpoint freeline slice with explicit unknown-sampling refusal.')
