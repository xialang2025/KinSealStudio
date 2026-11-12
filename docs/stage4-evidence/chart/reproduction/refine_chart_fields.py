"""Match fresh native observations: scalar line edits, two arc radios, dynamic picker spans."""
from pathlib import Path
root=Path(__file__).resolve().parent/'worktree/src'
p=root/'chart_dialog.h'; s=p.read_text(encoding='utf-8')
s=s.replace('QDoubleSpinBox *m_', 'QLineEdit *m_').replace('QDoubleSpinBox *number(', 'QLineEdit *number(')
p.write_text(s,encoding='utf-8',newline='\n')
p=root/'chart_dialog.cpp'; s=p.read_text(encoding='utf-8')
s=s.replace('    QCheckBox *arc;', '    QRadioButton *arc;\n    QRadioButton *ringRadio;\n    QButtonGroup *arcGroup;')
s=s.replace('arc = toggle(grid, 0, QStringLiteral("圆圆弧（取消为圆圆环）"), QStringLiteral("ChartArc"), &m_draft.donut.arc);', 'arcGroup = new QButtonGroup(attributes); ringRadio = new QRadioButton(QStringLiteral("圆圆环"), attributes); arc = new QRadioButton(QStringLiteral("圆圆弧"), attributes); ringRadio->setObjectName(QStringLiteral("ChartRing")); arc->setObjectName(QStringLiteral("ChartArc")); arcGroup->addButton(ringRadio); arcGroup->addButton(arc); ringRadio->setChecked(!m_draft.donut.arc); arc->setChecked(m_draft.donut.arc); grid->addWidget(ringRadio, 0, 0); grid->addWidget(arc, 0, 1);')
s=s.replace('connect(arc, &QCheckBox::toggled, this, [this](bool checked) { m_donutAttributes->setCurrentIndex(checked ? 1 : 0); });', 'connect(arc, &QRadioButton::toggled, this, [this](bool checked) { m_draft.donut.arc = checked; m_donutAttributes->setCurrentIndex(checked ? 1 : 0); refresh(); });')
start=s.index('QDoubleSpinBox *ChartDialog::number(')
end=s.index('/******************************************************************************',start)
s=s[:start]+'''QLineEdit *ChartDialog::number(QGridLayout *layout, int row, const QString &label, const QString &name, QString *value, int type) //20261112-1 add
{
    QLineEdit *edit = textField(layout, row, label, name, value);
    Q_UNUSED(type);
    return edit;
}

'''+s[end:]
s=s.replace('    QList<QDoubleSpinBox *> spins;', '    QList<QLineEdit *> spins;').replace('    QDoubleSpinBox *spin;', '    QLineEdit *spin;')
s=s.replace("spin->setDecimals(type == 6 ? 6 : 0); spin->setRange(range.first.toDouble(), range.second.toDouble()); spin->setValue(index++ == 0 ? range.first.toDouble() : range.second.toDouble());", 'spin->setText(index++ == 0 ? range.first : range.second);')
s=s.replace('m_twoPoles->setChecked(false); spins', 'm_twoPoles->setChecked(false); m_midpoint->setText(QString::number(std::trunc((range.first.toDouble() + range.second.toDouble()) / 2), \'g\', 16)); spins')
s=s.replace('edit->setObjectName(name); edit->setMaxLength', 'edit->setProperty("chartWords", words); edit->setObjectName(name); edit->setMaxLength')
s=s.replace('[this, edit, words]() { editHmiAddress(this, edit, false, words); }','[this, edit]() { editHmiAddress(this, edit, false, edit->property("chartWords").toInt()); }')
s=s.replace('    refresh();\n}\n\n/******************************************************************************\n * Function Name   : ChartDialog::object', '''    findChild<QLineEdit *>(QStringLiteral("Chart%1Address").arg(shape == 0 ? QStringLiteral("Bar") : shape == 1 ? QStringLiteral("Meter") : QStringLiteral("Donut")))->setProperty("chartWords", int(ChartProperties::wordCount(type)));
    if (shape == 0) { findChild<QLineEdit *>(QStringLiteral("ChartMinimumAddress"))->setProperty("chartWords", int(ChartProperties::wordCount(type))); findChild<QLineEdit *>(QStringLiteral("ChartMaximumAddress"))->setProperty("chartWords", int(ChartProperties::wordCount(type))); }
    refresh();
}

/******************************************************************************
 * Function Name   : ChartDialog::object''')
p.write_text(s,encoding='utf-8',newline='\n')
