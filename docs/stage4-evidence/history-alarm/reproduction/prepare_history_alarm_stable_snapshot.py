"""Ignore only destroyed native child handles during an owned dialog snapshot."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-history-alarm';p=owner/'history_alarm_probe.py';s=p.read_text(encoding='utf-8');a=s.index('        def dump(label):');b=s.index('\n        def action(name):',a)
new='''        def dump(label):
            rows = []
            for top in windows(desk, pid):
                try:
                    if not win32gui.IsWindow(top) or not win32gui.IsWindowVisible(top) or win32gui.GetClassName(top) not in ('QWidget','#32770'):
                        continue
                    cs = [top]
                    win32gui.EnumChildWindows(top, lambda h, _: cs.append(h), None)
                    native_children = []
                    for h in cs:
                        try:
                            if not win32gui.IsWindow(h) or win32process.GetWindowThreadProcessId(h)[1] != pid:
                                continue
                            native_children.append({'hwnd':h,'text':win32gui.GetWindowText(h),'class':win32gui.GetClassName(h),'controlId':win32gui.GetDlgCtrlID(h)})
                        except pywintypes.error as exc:
                            if exc.winerror != 1400:
                                raise
                            result.setdefault('observationRaces',[]).append({'label':label,'destroyedChild':h})
                    rows.append({'hwnd':top,'title':win32gui.GetWindowText(top),'winClass':win32gui.GetClassName(top),'widgets':op({'op':'batch','hwnds':[c['hwnd'] for c in native_children]}),'nativeChildren':native_children})
                except pywintypes.error as exc:
                    if exc.winerror != 1400:
                        raise
                    result.setdefault('observationRaces',[]).append({'label':label,'destroyedTop':top})
            result['snapshots'].append({'label':label,'windows':[r for r in rows if r['hwnd']!=editor]})
            (OUT/'native-probe-checkpoint.json').write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf-8')
            print(json.dumps({'snapshot':label,'windows':[r['title'] for r in rows]},ensure_ascii=False),flush=True)
            return rows
'''
s=s[:a]+new+s[b:];p.write_text(s,encoding='utf-8',newline='\n')
data=json.loads((owner/'history-alarm-custom-r4.json').read_text(encoding='utf-8'))
for c in data:
    if 'label' in c:c['label']=c['label'].replace('-r4','-r5')
(owner/'history-alarm-custom-r5.json').write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
