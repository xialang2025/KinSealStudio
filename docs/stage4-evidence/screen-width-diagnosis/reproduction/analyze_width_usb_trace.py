"""Verify immutable raw trace bytes and pair libusb calls before interpreting the transfer lifecycle."""
from pathlib import Path
import argparse
import hashlib
import json
import sys
parser=argparse.ArgumentParser();parser.add_argument('run');args=parser.parse_args()
root=Path(__file__).resolve().parent;work=root/'worktree';base=work/'docs/stage4-evidence/screen-width-diagnosis/physical'/args.run
sys.path.insert(0,str(work/'tools'))
from usb_transfer_completion import classify_native_usb_completion
probe=json.loads((base/'usb-events.json').read_text(encoding='utf-8'));hardware=json.loads((base/'run.json').read_text(encoding='utf-8'))
events=probe['events'];calls={};commands=[];errors=[];blocks=[]
for event in events:
    if event.get('rawPath'):
        data=(base/event['rawPath']).read_bytes();assert len(data)==event['bytes'] and hashlib.sha256(data).hexdigest()==event['sha256']
        if len(data)<=64:event['dataHex']=data.hex()
    if event.get('kind')=='usb-call':calls.setdefault(event['id'],{})[event['phase']]=event
    if event.get('captureError') or event.get('instrumentationError'):errors.append(event)
for identity,pair in sorted(calls.items()):
    enter=pair.get('enter');leave=pair.get('leave');assert enter and leave,(identity,pair)
    if enter['direction']=='out' and leave['returned'] != enter['requested']:errors.append(pair)
    if enter['direction']=='out' and enter.get('rawPath'):
        data=(base/enter['rawPath']).read_bytes()
        if len(data)==32 and data.startswith(b'KinSealStudioKiss'):
            commands.append(dict(callId=identity,code=f'{data[17]:02X}',parameters=data[18:].hex(),epochMs=enter['epochMs'],writeReturned=leave['returned'],rawPath=enter['rawPath']))
        else:blocks.append(dict(callId=identity,bytes=len(data),sha256=hashlib.sha256(data).hexdigest(),rawPath=enter['rawPath']))
    if enter['direction']=='in' and leave.get('rawPath'):
        data=(base/leave['rawPath']).read_bytes();pair['decodedReplyHex']=data.hex()
commit=[c for c in commands if c['code']=='1B'];reset=[c for c in commands if c['code']=='1A']
result=dict(run=args.run,sourceSha256=hardware['sourceSha256'],hooks=[e for e in events if e.get('kind')=='usb-hook-ready'],calls=len(calls),commands=commands,dataBlocks=blocks,errors=errors,commitObserved=bool(commit),resetObserved=bool(reset),uiCompletedEpochMs=hardware['nativeHardware']['completedEpoch']*1000,resetMinusUiCompletedMs=(reset[-1]['epochMs']-hardware['nativeHardware']['completedEpoch']*1000) if reset else None,readReplies=[dict(callId=k,hex=v['decodedReplyHex'],returned=v['leave']['returned']) for k,v in sorted(calls.items()) if 'decodedReplyHex' in v],rawBytesShaVerified=True,screenResult='awaiting_user')
result['completion']=classify_native_usb_completion(events)
result['expectedResetReadFailures']=result['completion']['expectedResetReadFailures']
result['errors']=result['completion']['errors']
(base/'usb-analysis.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps({k:v for k,v in result.items() if k not in ['hooks','dataBlocks','readReplies']},ensure_ascii=False,indent=2))
print('REPLIES',json.dumps(result['readReplies'][:4]))
