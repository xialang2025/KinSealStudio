"""Apply the bounded tiny-plot guard found in read-only review and correct copied helper contracts."""
from pathlib import Path
p=Path(__file__).resolve().parent/'worktree/src/trend_dialog.cpp'
s=p.read_text(encoding='utf-8')
before='painter->setPen(QColor(properties.frameColor)); painter->setBrush(Qt::NoBrush); painter->drawRect(plot);'
assert s.count(before)==1
s=s.replace(before,'if (plot.width() <= 0 || plot.height() <= 0) { painter->restore(); return; } // Keep tiny object bounds clipped without a negative plot area.\n    '+before)
for name,result in [('integer','Bounded spin editor.'),('color','Detached color picker button.')]:
 a=s.index(' * Function Name   : TrendDialog::'+name)
 b=s.index(' ******************************************************************************/',a)
 s=s[:a]+s[a:b].replace(' * Return Value    : None.',' * Return Value    : '+result)+s[b:]
s=s.replace(' * Input Parameters: layout, row, label, name, value, address, words',' * Input Parameters: layout, row, label, name, value, addressRole')
p.write_text(s,encoding='utf-8',newline='\n')
