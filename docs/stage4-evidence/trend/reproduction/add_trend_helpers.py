"""Reuse measured shared field widgets without importing chart form semantics."""
from pathlib import Path
root=Path(__file__).resolve().parent/'worktree/src'
source=(root/'chart_dialog.cpp').read_text(encoding='utf-8')
p=root/'trend_dialog.cpp'; target=p.read_text(encoding='utf-8')
for name in ['textField','choice','toggle','integer','color']:
 a=source.index(' * Function Name   : ChartDialog::'+name)
 a=source.rfind('/******************************************************************************',0,a)
 b=source.index('/******************************************************************************',a+90)
 part=source[a:b].replace('ChartDialog','TrendDialog')
 if name=='textField':
  part=part.replace('bool address, int words','int addressRole').replace('address ? 210 : 140','addressRole ? 210 : 140').replace('edit->setProperty("chartWords", words); ','').replace('address ? 1024 : 80','addressRole ? 1024 : 80').replace('if (address)','if (addressRole != 0)').replace('[this, edit]() { editHmiAddress(this, edit, false, edit->property("chartWords").toInt()); }','[this, edit, addressRole]() { editHmiAddress(this, edit, addressRole == 1, addressRole == 1 ? 1 : int(TrendProperties::wordCount(m_draft.dataType)) * m_draft.channelCount); }')
 if name=='integer': part=part.replace('void TrendDialog::integer','QSpinBox *TrendDialog::integer').replace('\n}\n','\n    return spin;\n}\n')
 if name=='color': part=part.replace('void TrendDialog::color','QPushButton *TrendDialog::color').replace('\n}\n','\n    return button;\n}\n')
 target+='\n'+part
p.write_text(target,encoding='utf-8',newline='\n')
p=root/'trend_dialog.h'; s=p.read_text(encoding='utf-8').replace('const QStringList &items, int *value);','const QStringList &items, int *value, int offset = 0);'); p.write_text(s,encoding='utf-8',newline='\n')
