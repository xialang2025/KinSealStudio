"""Replay the actual generated hardware block with a native trace whose reset follows the UI completion."""
from pathlib import Path
import argparse
import ast
import hashlib
import importlib.util
import json
import textwrap
from types import SimpleNamespace

root=Path(__file__).resolve().parent;work=root/'worktree'
parser=argparse.ArgumentParser();parser.add_argument('--probe-script',type=Path,required=True);parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
tree=ast.parse(args.probe_script.read_text(encoding='utf-8'))
nodes=[node for node in ast.walk(tree) if isinstance(node,ast.If) and ast.unparse(node.test)=="command.get('download')"]
assert len(nodes)==1
fixture=work/'docs/stage4-evidence/screen-width-diagnosis/physical/marker-a-r1'
probe=json.loads((fixture/'usb-events.json').read_text(encoding='utf-8'));events=[]
for original in probe['events']:
    event=dict(original)
    if event.get('rawPath'):
        data=(fixture/event['rawPath']).read_bytes();assert hashlib.sha256(data).hexdigest()==event['sha256']
        if len(data)<=64:event['dataHex']=data.hex()
    events.append(event)
cutoff=probe['uiCompletedEpochMs']
pending=[e for e in events if e.get('epochMs',0)<=cutoff]
state={'usbTrace':{'events':[e for e in events if e.get('kind')=='usb-hook-ready']}}

class FakeTime:
    def __init__(self):self.now=1000.0
    def time(self):return self.now
    def sleep(self,seconds):
        self.now+=seconds
        state['usbTrace']['events'][:]=events if self.now>=1001.5 else pending

clock=FakeTime()
def widget(request):return {'hwnd':request['name'],'qt':{'checked':'false'}}
def operation(request):
    if request['op']=='batch':return [{'qt':{'checked':'false'}}]
    if request['op']=='table-read':return [['下载完成']]
    return True

context=dict(command={'download':True},args=SimpleNamespace(download=True),result=state,time=clock,widget=widget,op=operation,hashlib=hashlib,source=fixture/'scenario.json',tables=[{'hwnd':'m_table'}],dump=lambda label:None)
helper=work/'tools/usb_transfer_completion.py'
if helper.exists():
    module_spec=importlib.util.spec_from_file_location('usb_transfer_completion',helper);module=importlib.util.module_from_spec(module_spec);module_spec.loader.exec_module(module)
    context['classify_native_usb_completion']=module.classify_native_usb_completion
exec(compile(ast.fix_missing_locations(ast.Module(body=nodes,type_ignores=[])),str(args.probe_script),'exec'),context)
record=dict(test='actual_generated_native_download_block_waits_for_reset',sourceScript=str(args.probe_script),sourceSha256=hashlib.sha256(args.probe_script.read_bytes()).hexdigest(),elapsedFakeSeconds=clock.now-1000,resetAvailableAtFakeSeconds=1.5,completed=state['nativeHardware']['completed'],passed=state['nativeHardware']['completed'] and clock.now>=1001.5,realUsbAccessed=False)
args.output.write_text(json.dumps(record,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(json.dumps(record,ensure_ascii=False));raise SystemExit(0 if record['passed'] else 1)
