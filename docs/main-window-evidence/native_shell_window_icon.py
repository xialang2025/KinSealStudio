"""Rebuild the unique PE RT_GROUP_ICON from its original RT_ICON bytes."""
from pathlib import Path
import hashlib
import io
import json
import struct
import pefile
from PIL import Image

ROOT=Path(r'Z:\Project\vscode_prj_temp\REA')
EXE=Path(r'G:\01_Professional\触摸屏\金玺\KinSealStudio_Setup_V5.1\Installed\KinSealStudio5.1.exe')
OUT=ROOT/'KinSealStudio/resources/native-shell/icons'
META=ROOT/'KinSealStudio/docs/main-window-evidence/native-shell.json'
original=EXE.read_bytes()
pe=pefile.PE(data=original)
group_type=next(e for e in pe.DIRECTORY_ENTRY_RESOURCE.entries if e.id==14)
icon_type=next(e for e in pe.DIRECTORY_ENTRY_RESOURCE.entries if e.id==3)
assert len(group_type.directory.entries)==1
group=group_type.directory.entries[0]
language=group.directory.entries[0]
group_bytes=pe.get_data(language.data.struct.OffsetToData,language.data.struct.Size)
reserved,kind,count=struct.unpack_from('<3H',group_bytes)
offset=6+count*16
entries=[]
data=[]
for index in range(count):
    raw=group_bytes[6+14*index:20+14*index]
    width,height,colors,reserved,planes,bits,size,resource_id=struct.unpack('<4B2HIH',raw)
    selected=next(e for e in icon_type.directory.entries if e.id==resource_id)
    icon_language=next((e for e in selected.directory.entries if e.id==language.id),selected.directory.entries[0])
    pixels=pe.get_data(icon_language.data.struct.OffsetToData,icon_language.data.struct.Size)
    assert len(pixels)==size
    entries.append(struct.pack('<4B2H2I',width,height,colors,reserved,planes,bits,size,offset))
    data.append(pixels)
    offset+=size
ico=struct.pack('<3H',0,1,count)+b''.join(entries)+b''.join(data)
(OUT/'windowIcon.ico').write_bytes(ico)
image=Image.open(io.BytesIO(ico)).convert('RGBA')
image.save(OUT/'windowIcon-native.png')
for size,suffix in [(24,''),(32,'-32')]:
    image.resize((size,size),Image.Resampling.LANCZOS).save(OUT/('windowIcon'+suffix+'.png'))
record=json.loads(META.read_text(encoding='utf8'))
fallback={'source':'PE RT_GROUP_ICON and corresponding original RT_ICON bytes','exeSha256':hashlib.sha256(original).hexdigest(),'groupName':str(group.name or group.id),'language':language.id,'entryCount':count,'originalSize':image.size,'originalIcoSha256':hashlib.sha256(ico).hexdigest(),'path':'resources/native-shell/icons/windowIcon.png','nativePng':'resources/native-shell/icons/windowIcon-native.png','ico':'resources/native-shell/icons/windowIcon.ico','note':'QWidget.windowIcon QIcon is nonnull but returns0x0. This independent PE fallback is retained separately; 24/32 PNG variants resample the original PE image with Lanczos.'}
record['windowIcon']['fallback']=fallback
record['iconFiles']=[{'path':str(p.relative_to(ROOT/'KinSealStudio')).replace('\\','/'),'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in sorted(OUT.glob('*')) if p.is_file()]
META.write_text(json.dumps(record,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
print(json.dumps(fallback))
