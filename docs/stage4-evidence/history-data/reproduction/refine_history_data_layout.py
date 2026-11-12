"""Correct the C30 data-page geometry against the retained native screenshot."""
from pathlib import Path
p=Path(__file__).resolve().parent/'worktree/src/history_data_dialog.cpp';s=p.read_text(encoding='utf-8')
s=s.replace('#include <QHeaderView>', '#include <QHeaderView>\n#include <QHBoxLayout>')
start=s.index('QWidget *HistoryDataDialog::dataPage()');end=s.index('\n/******************************************************************************',start)
part=s[start:end].replace('    QTableWidgetItem *name;','    QHBoxLayout *fields = new QHBoxLayout();\n    QHBoxLayout *synchronization = new QHBoxLayout();\n    QTableWidgetItem *name;')
part=part.replace('m_table->setMinimumHeight(300);','m_table->setFixedSize(470, 300);')
before='layout->addWidget(new QLabel(QStringLiteral("语言:"), page), 2, 0); layout->addWidget(m_dataLanguages, 2, 1); layout->addWidget(new QLabel(QStringLiteral("名称:"), page), 2, 2); layout->addWidget(m_name, 2, 3);'
after='m_dataLanguages->setFixedWidth(120); m_name->setFixedWidth(112); fields->addWidget(new QLabel(QStringLiteral("语言:"), page)); fields->addWidget(m_dataLanguages); fields->addSpacing(20); fields->addWidget(new QLabel(QStringLiteral("名称:"), page)); fields->addWidget(m_name); fields->addStretch(); layout->addLayout(fields, 2, 0, 1, 4);'
assert part.count(before)==1;part=part.replace(before,after)
before='current->setMinimumHeight(48); all->setMinimumHeight(48); layout->addWidget(current, 3, 0, 1, 2); layout->addWidget(all, 3, 2, 1, 2);'
after='current->setFixedSize(178, 48); all->setFixedSize(170, 48); synchronization->addWidget(current); synchronization->addSpacing(36); synchronization->addWidget(all); synchronization->addStretch(); layout->addLayout(synchronization, 3, 0, 1, 4);'
assert part.count(before)==1;part=part.replace(before,after);s=s[:start]+part+s[end:];p.write_text(s,encoding='utf-8',newline='\n')
print('Matched native table and synchronization-button widths without changing any draft logic.')
