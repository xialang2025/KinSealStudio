"""Capture the original export with its unchanged default Save filename."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-recipe';p=owner/'recipe_probe.py';s=p.read_text(encoding='utf-8')
before="            elif name=='export-csv':"
after="""            elif name=='native-export-default':
                w=widget({'kind':'QPushButton','name':'ExportButton'})
                op({'op':'invoke-queued','hwnd':w['hwnd'],'slot':'click'})
                deadline=time.time()+7
                dialogs=[]
                while time.time()<deadline:
                    dialogs=[h for h in windows(desk,pid) if win32gui.IsWindowVisible(h) and win32gui.GetClassName(h)=='#32770']
                    if dialogs:break
                    time.sleep(.1)
                assert len(dialogs)==1
                controls=[]
                win32gui.EnumChildWindows(dialogs[0],lambda h,_:controls.append(h),None)
                button=next(h for h in controls if win32gui.GetClassName(h)=='Button' and win32gui.GetDlgCtrlID(h)==1)
                win32gui.PostMessage(button,win32con.BM_CLICK,0,0)
                deadline=time.time()+7
                while time.time()<deadline and not (WORK/command['filename']).is_file():time.sleep(.1)
                generated=WORK/command['filename'];assert generated.is_file(),'Default native export not in owned working directory'
                destination=OUT/command['output'];assert destination.parent==OUT and not destination.exists();shutil.copy2(generated,destination)
                result.setdefault('outputs',[]).append({'path':destination.name,'bytes':destination.stat().st_size,'sha256':hashlib.sha256(destination.read_bytes()).hexdigest(),'origin':'Original ExportButton and unchanged default owned native Save dialog filename','runtimePath':str(generated)})
"""+before
assert s.count(before)==1;s=s.replace(before,after);p.write_text(s,encoding='utf-8',newline='\n')
steps=[dict(op='action',name='action_rep'),dict(op='text',kind='QLineEdit',name='sRecipeName',value='C31Golden'),dict(op='native-export-default',filename='C31Golden.csv',output='C31Golden-native-r14.csv'),dict(op='dump',label='recipe-golden-export-result-r14')]
(owner/'recipe-golden-export-r14.json').write_text(json.dumps(steps,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
