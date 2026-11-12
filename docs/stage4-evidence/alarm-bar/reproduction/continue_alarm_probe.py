"""Keep the large-size observation separate from a small valid alarm baseline."""
from pathlib import Path
import json
root = Path(__file__).resolve().parent
owner = root / 'native-alarm-bar'
p = owner / 'alarm_probe.py'
s = p.read_text(encoding='utf-8').replace('limit = time.time() + 5\n', 'limit = time.time() + 30\n')
p.write_text(s, encoding='utf-8', newline='\n')
prior = json.loads((owner / 'alarm-default-r3.json').read_text(encoding='utf-8'))
scenario = prior[:3] + [dict(op='tree-double', entry='数位地址报警'), dict(op='combo', kind='QComboBox', name='nReadLen', value=2)]
start = next(i for i, c in enumerate(prior) if c.get('op') == 'table-click' and c.get('name') == 'BitAlarmTable')
scenario += prior[start:]
for c in scenario:
    if 'label' in c:
        c['label'] = c['label'].replace('-r3', '-r4')
(owner / 'alarm-default-r4.json').write_text(json.dumps(scenario, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
