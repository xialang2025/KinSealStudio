"""Assert saved historical-alarm field ownership from completed property reopen snapshots."""
from pathlib import Path
import json
base=Path(__file__).resolve().parent/'worktree/docs/stage4-evidence/history-alarm/native'
d=json.loads((base/'native-probe-history-alarm-custom-r5.json').read_text(encoding='utf-8'));assert d['status']=='native-stage3-scenario-completed'
def widgets(v,out):
    if isinstance(v,dict):
        if v.get('visible') and v.get('objectName'):out[v['objectName']]=v
        for x in v.values():widgets(x,out)
    elif isinstance(v,list):
        for x in v:widgets(x,out)
records=[]
for label,prefix in [('history-alarm-custom-reopened-r5','ZH'),('history-alarm-custom-English-reopened-r5','EN')]:
    snapshot=next(s for s in d['snapshots'] if s['label']==label);out={};widgets([w for w in snapshot['windows'] if w['title']=='历史报警显示器'],out)
    captions={key:out[name]['text'] for key,name in [('message','m_msgedit'),('date','m_dateedit'),('time','m_timeedit')]}
    assert captions==dict(message=prefix+' Message',date=prefix+' Date',time=prefix+' Time')
    assert out['m_dateedit']['enabled']==0 and out['m_timeedit']['enabled']==0 and out['m_datecheck']['checked']=='false' and out['m_timecheck']['checked']=='false'
    assert out['fontSize']['currentText']=='24' and out['m_rowspin']['value']=='20'
    colors={key:out[name]['styleSheet'] for key,name in [('titleBackground','m_tbackcolorbtn'),('titleColor','m_ttextcolorbtn'),('frameColor','m_fframcolorbtn'),('backgroundColor','m_fbackcolorbtn'),('textColor','m_fontcolorbtn')]}
    assert colors==dict(titleBackground='background:#008000',titleColor='background:#ff0000',frameColor='background:#0000ff',backgroundColor='background:#c0c0c0',textColor='background:#800080')
    records.append(dict(label=label,captions=captions,commonColors=colors,fontSize=24,rows=20,dateAndTimeEnabled=False,hiddenCaptionsRetained=True))
result=dict(sourceRun='native-probe-history-alarm-custom-r5.json',records=records,observationRaceCount=len(d.get('observationRaces',[])),runtimeHistorySourceVerified=False)
(base/'observed-fields.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(json.dumps(dict(reopenSnapshotsVerified=2,sharedColors=5,localizedCaptions=3,sourceRunCompleted=True),ensure_ascii=False))
