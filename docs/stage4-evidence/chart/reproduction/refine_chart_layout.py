"""Constrain fields and restore the native two-column source range and one-row midpoint layout."""
from pathlib import Path
p=Path(__file__).resolve().parent/'worktree/src/chart_dialog.cpp'
s=p.read_text(encoding='utf-8')
s=s.replace('basicLayout->addLayout(radios); basicLayout->addSpacing(18);', 'basicLayout->setContentsMargins(12, 35, 12, 12); radios->setContentsMargins(24, 0, 24, 0); basicLayout->addLayout(radios); basicLayout->addSpacing(18);')
s=s.replace('edit->setProperty("chartWords", words); edit->setObjectName(name);', 'edit->setMaximumWidth(address ? 210 : 140); edit->setProperty("chartWords", words); edit->setObjectName(name);')
s=s.replace('combo->setObjectName(name); combo->addItems', 'combo->setMaximumWidth(230); combo->setObjectName(name); combo->addItems')
s=s.replace('    QGridLayout *arcGrid;', '    QGridLayout *arcGrid;\n    QGridLayout *minimumGrid;\n    QGridLayout *maximumGrid;')
before='m_twoPoles = toggle(grid, 0, QStringLiteral("两极柱"), QStringLiteral("ChartTwoPoles"), &m_draft.bar.twoPoles); m_midpoint = textField(grid, 1, QStringLiteral("中点:"), QStringLiteral("ChartMidpoint"), &m_draft.bar.midpoint);'
after='m_twoPoles = new QCheckBox(QStringLiteral("两极柱"), page); m_twoPoles->setObjectName(QStringLiteral("ChartTwoPoles")); m_twoPoles->setChecked(m_draft.bar.twoPoles); grid->addWidget(m_twoPoles, 0, 0); connect(m_twoPoles, &QCheckBox::toggled, this, [this](bool checked) { m_draft.bar.twoPoles = checked; }); m_midpoint = new QLineEdit(m_draft.bar.midpoint, page); m_midpoint->setObjectName(QStringLiteral("ChartMidpoint")); m_midpoint->setMaxLength(80); m_midpoint->setMaximumWidth(170); grid->addWidget(new QLabel(QStringLiteral("中点:"), page), 0, 1); grid->addWidget(m_midpoint, 0, 2); connect(m_midpoint, &QLineEdit::textChanged, this, [this](const QString &text) { m_draft.bar.midpoint = text; refresh(); });'
assert s.count(before)==1
s=s.replace(before,after)
before='''m_sourceMinimum = number(grid, 1, QStringLiteral("最小值:"), QStringLiteral("ChartSourceMinimum"), &m_draft.bar.sourceMinimum, *dataType); m_sourceMaximum = number(grid, 2, QStringLiteral("最大值:"), QStringLiteral("ChartSourceMaximum"), &m_draft.bar.sourceMaximum, *dataType);
        minimumAddress = textField(grid, 3, QStringLiteral("最小地址:"), QStringLiteral("ChartMinimumAddress"), &m_draft.bar.minimumAddress, true, ChartProperties::wordCount(*dataType)); maximumAddress = textField(grid, 4, QStringLiteral("最大地址:"), QStringLiteral("ChartMaximumAddress"), &m_draft.bar.maximumAddress, true, ChartProperties::wordCount(*dataType));'''
after='''minimumGrid = new QGridLayout; maximumGrid = new QGridLayout; grid->addLayout(minimumGrid, 1, 0); grid->addLayout(maximumGrid, 1, 1);
        m_sourceMinimum = number(minimumGrid, 0, QStringLiteral("最小值:"), QStringLiteral("ChartSourceMinimum"), &m_draft.bar.sourceMinimum, *dataType); m_sourceMaximum = number(maximumGrid, 0, QStringLiteral("最大值:"), QStringLiteral("ChartSourceMaximum"), &m_draft.bar.sourceMaximum, *dataType);
        minimumAddress = textField(minimumGrid, 1, QString(), QStringLiteral("ChartMinimumAddress"), &m_draft.bar.minimumAddress, true, ChartProperties::wordCount(*dataType)); maximumAddress = textField(maximumGrid, 1, QString(), QStringLiteral("ChartMaximumAddress"), &m_draft.bar.maximumAddress, true, ChartProperties::wordCount(*dataType));'''
assert s.count(before)==1
s=s.replace(before,after)
p.write_text(s,encoding='utf-8',newline='\n')
