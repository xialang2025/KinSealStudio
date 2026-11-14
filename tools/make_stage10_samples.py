"""Generate independent internal-address acceptance lamps from already typed sample records."""
#20261114-9 add: These fixtures reuse existing encoders and do not claim new firmware support.
import copy
import hashlib
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]


def load(path):
    """Input: existing relative sample. Return: parsed source without modifying it."""
    return json.loads((ROOT/path).read_text(encoding='utf-8-sig'))


def appearance(state,text,colour):
    """Input: supported state, label and colour. Return: None after bounded appearance edits."""
    state.update(sharedText=text,sameLanguage=True,useImage=False,picturePng='',flash=False,
                 transparent=False,effect3d=False,background=colour,opacity=255)
    state['text'].update(fontSize=16,texts={'中文':text})


def position(obj,identity,x,y,width,height):
    """Input: existing typed record and new identity/geometry. Return: independent object copy."""
    result=copy.deepcopy(obj)
    result.update(id=identity,name=identity,x=x,y=y,width=width,height=height,layer=1)
    return result


def title(seed,text):
    """Input: supported text seed and exact page identifier. Return: noninteractive marker."""
    result=position(seed,'page-marker',20,15,760,36)
    result['text']=text
    result['staticText'].update(texts={'中文':text},fontSize=20)
    return result


def save(name,project):
    """Input: new sample name and model. Return: exact written source digest."""
    path=ROOT/'samples/stage10'/name
    path.parent.mkdir(parents=True,exist_ok=True)
    with path.open('x',encoding='utf-8',newline='\n') as stream:
        json.dump(project,stream,ensure_ascii=False,indent=2)
        stream.write('\n')
    return dict(path=path.relative_to(ROOT).as_posix(),sha256=hashlib.sha256(path.read_bytes()).hexdigest())


def main():
    """Input: None. Return: zero after creating two new internal-only acceptance fixtures."""
    bit=load('samples/bit-button/四操作实机验收.ksp')
    lamp=load('samples/lamp/指示灯实机联调.ksp')
    original=bit['pages'][0]['objects']
    lamps=lamp['pages'][0]['objects']
    seed=next(obj for obj in original if obj['type']=='text')
    first=copy.deepcopy(bit)
    first['schemaVersion']=max(bit['schemaVersion'],lamp['schemaVersion'])
    first['projectName']='S10-BIT-LAMP-1114'
    objects=[title(seed,first['projectName'])]
    for index,obj in enumerate(obj for obj in original if obj['type']=='switch'):
        result=position(obj,f'bit-{index}',25+index*195,100,165,85)
        for state in result['bitButton']['states']:
            appearance(state,['TOGGLE LB0','SET LB1','RESET LB1','HOLD LB2'][index],'#ffffffff')
        objects.append(result)
    for index in range(3):
        result=position(lamps[0],f'lamp-LB{index}',55+index*250,285,185,85)
        result['bitButton']['monitorBitAddress']=f'LB{index}'
        for number,state in enumerate(result['bitButton']['states']):
            appearance(state,f'LB{index} = {number}',['#ffffffff','#ff80ff80'][number])
        objects.append(result)
    first['pages'][0]['objects']=objects
    second=copy.deepcopy(lamp)
    second['projectName']='S10-WORD-LAMP-1114'
    objects=[title(seed,second['projectName'])]
    for index,(address,value) in enumerate([('LW20','0'),('LW20','1'),('LW20','4'),('LW30','0'),('LW30','3')]):
        obj=next(o for o in lamps if 'wordButton' in o and o['wordButton']['dataType']==(0 if address=='LW20' else 1))
        result=position(obj,f'word-{index}',20+index*155,100,140,75)
        result['address']=address
        result['wordButton'].update(operation=0,constantValue=value)
        for state in result['bitButton']['states']:
            appearance(state,f'{address} = {value}','#ffffffff')
        objects.append(result)
    for index in range(2):
        result=position(lamps[index+1],f'lamp-word-{index}',85+index*365,270,270,90)
        for number,state in enumerate(result['bitButton']['states']+result['lamp']['extraStates']):
            appearance(state,f'{["LW20","LW30/31"][index]} : {number}','#ffb0ffb0')
        objects.append(result)
    second['pages'][0]['objects']=objects
    print(json.dumps([save('bit-lamp-local.ksp',first),save('word-lamp-local.ksp',second)],ensure_ascii=False))
    return 0


if __name__=='__main__':
    raise SystemExit(main())
