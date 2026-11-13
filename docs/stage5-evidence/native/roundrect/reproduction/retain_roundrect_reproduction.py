"""Retain this bounded compiler investigation before the reviewed runtime cleanup."""
from pathlib import Path
import hashlib
import json
import shutil

root=Path(__file__).resolve().parent
work=root.parent/'kinseal-stage4-20261112/worktree'
base=work/'docs/stage5-evidence/native/roundrect/reproduction'
base.mkdir(exist_ok=False)
names=['native_probe.py','native_paint_probe.py','run_native_paint.py','run_roundrect_canonical.py','analyze_roundrect_corpus.py','scanline_candidate.py','roundrect_path_probe.cpp','compare_independent_roundrect.py','graphic_candidate_probe.cpp','path_trace_replay.cpp','compare_path_trace.py','compare_native_stroke.py','compare_gdi.py','compare_cosmetic.py','investigate_aa.py','aa_mask_probe.cpp','run_roundrect_red.py','archive_family.py','execute_family_cleanup.ps1','retain_roundrect_reproduction.py']
for name in names: shutil.copy2(root/name,base/name)
sources=base/'public-qt-sources';sources.mkdir()
manifest=[]
for local,remote in [('qpaintengine_raster-qt463.cpp','qpaintengine_raster.cpp'),('qstroker-qt463.cpp','qstroker.cpp'),('qstroker-qt463.h','qstroker_p.h'),('qrasterizer-qt463.cpp','qrasterizer.cpp')]:
    # The local header's original filename is checked from its content, not inferred from the renamed suffix.
    if local=='qstroker-qt463.h':remote='qstroker_p.h'
    p=root/local;shutil.copy2(p,sources/local)
    manifest.append(dict(file=local,url='https://code.qt.io/cgit/qt/qt.git/plain/src/gui/painting/'+remote+'?h=v4.6.3',bytes=p.stat().st_size,sha256=hashlib.sha256(p.read_bytes()).hexdigest(),copyrightHeaderRetained=True,role='public_library_investigation_only_not_copied_into_production'))
(sources/'sources.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
audit=work/'docs/stage5-evidence/native/audit'
for name in ('cleanup.json','cleanup-plan.json','raw-archives.json'):
    destination=audit/(name.replace('.json','-before-roundrect.json'))
    assert not destination.exists();shutil.copy2(audit/name,destination)
print(json.dumps(dict(retainedScripts=len(names),publicSources=len(manifest),historicalAuditReceiptsPreserved=True)))
