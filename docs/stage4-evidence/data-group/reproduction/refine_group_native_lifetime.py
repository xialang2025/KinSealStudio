"""Extend the existing C26 mode-lifetime test and generation recipe with observed native confirmation/reopen normalization."""
from pathlib import Path
root=Path(__file__).resolve().parent;work=root/'worktree'
name='groupSampleAndMaximumDraftLifetime'
p=work/'tests/stage4_controls_tests.cpp';s=p.read_text(encoding='utf-8');i=s.index('void Stage4ControlTests::'+name+'()');j=s.index('/******************************************************************************',i);piece=s[i:j]
anchor='    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR"))'
assert piece.count(anchor)==1
test='''    dialog.findChild<QDialogButtonBox *>(QStringLiteral("GroupButtons"))->button(QDialogButtonBox::Ok)->click(); QCOMPARE(dialog.result(), int(QDialog::Accepted)); QCOMPARE(dialog.object().dataGroup.yMinimum, QStringLiteral("0")); QCOMPARE(dialog.object().dataGroup.yMaximum, QStringLiteral("1000")); QCOMPARE(dialog.object().dataGroup.maximumAddress, QStringLiteral("LW20"));
    { HmiObject raw = groupObject(); raw.dataGroup.rangeSource = 1; raw.dataGroup.yMinimum = QStringLiteral("-20"); raw.dataGroup.yMaximum = QStringLiteral("5000"); DataGroupDialog reopened(raw, HmiProject()); QCOMPARE(reopened.object().dataGroup.yMinimum, QStringLiteral("0")); QCOMPARE(reopened.object().dataGroup.yMaximum, QStringLiteral("1000")); }
'''
# Capture the visible source-mode draft before confirming the dialog.
end=piece.index('\n}',piece.index(anchor));piece=piece[:end]+'\n'+test+piece[end:];s=s[:i]+piece+s[j:];p.write_text(s,encoding='utf-8',newline='\n')
p=root/'make_group_dialog.py';s=p.read_text(encoding='utf-8');needle="s+=tail\n";assert needle in s
s=s.replace(needle,'''tail=tail.replace('return; } QDialog::accept();', 'return; }\\n    if (m_draft.rangeSource == 1) { m_draft.yMinimum = QStringLiteral("0"); m_draft.yMaximum = QStringLiteral("1000"); } //20261113-1 add: Match native address-range confirmation.\\n    QDialog::accept();')
tail=tail.replace('    setObjectName(QStringLiteral("DataGroupDialog"));', '    if (m_draft.rangeSource == 1) { m_draft.yMinimum = QStringLiteral("0"); m_draft.yMaximum = QStringLiteral("1000"); } //20261113-1 add: Match the native property reopen state.\\n    setObjectName(QStringLiteral("DataGroupDialog"));')
s+=tail
''')
p.write_text(s,encoding='utf-8',newline='\n')
print('Added measured address-range confirmation and reopen checks without introducing new test slots.')
