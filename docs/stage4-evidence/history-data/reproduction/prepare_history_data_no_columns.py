"""Separate the all-hidden data case from the case with no date/time or data columns."""
from pathlib import Path
import json
owner=Path(__file__).resolve().parent/'native-history-data'
s=[dict(op='action',name='action_historyrecord'),dict(op='invoke',kind='QCheckBox',name='m_datecheck'),dict(op='invoke',kind='QCheckBox',name='m_timecheck'),dict(op='tab',kind='QTabWidget',name='tabWidget',value=1),dict(op='table-click',kind='QTableWidget',name='HisDataTable',row=0,column=1,checkbox=True),dict(op='table-state',kind='QTableWidget',name='HisDataTable',label='history-data-no-columns-r9'),dict(op='invoke',kind='QPushButton',name='accept'),dict(op='dump',label='history-data-no-columns-accept-result-r9'),dict(op='assert-main-only'),dict(op='scene-drag',**{'from':[150,100],'to':[470,280]}),dict(op='scene-assert',minSelectableRoots=1),dict(op='save',output='history-data-no-columns.sm'),dict(op='native-compile',label='history-data-no-columns-r9')]
(owner/'history-data-no-columns-r9.json').write_text(json.dumps(s,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
