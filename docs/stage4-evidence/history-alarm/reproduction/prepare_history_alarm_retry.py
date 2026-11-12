"""Wait for a newly queued modal to become observable without retrying the action itself."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-history-alarm';p=owner/'history_alarm_probe.py';s=p.read_text(encoding='utf-8')
a=s.index('        def widget(command):');b=s.index('\n        def tree_event(command):',a);old=s[a:b];first=old.index('            ws=');last=old.index('            if len(ws)!=1:');filters=old[first:last];filters=''.join('    '+line+'\n' for line in filters.splitlines())
new='        def widget(command):\n            limit = time.time() + 4\n            while True:\n'+filters+'                if ws or time.time() >= limit:\n                    break\n                time.sleep(.15)\n'+old[last:]
s=s[:a]+new+s[b:];p.write_text(s,encoding='utf-8',newline='\n')
data=json.loads((owner/'history-alarm-custom-r3.json').read_text(encoding='utf-8'))
for c in data:
    if 'label' in c:c['label']=c['label'].replace('history-history-alarm','history-alarm').replace('-r3','-r4')
(owner/'history-alarm-custom-r4.json').write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
