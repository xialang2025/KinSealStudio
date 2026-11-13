"""Exercise only the affected table UI, persistence and legacy preview behavior on an owned desktop."""
from pathlib import Path
import datetime
import hashlib
import importlib.util
import json
import os
import re
import argparse

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree'
parser=argparse.ArgumentParser();parser.add_argument('--family',choices=['table','rect','line','freeline'],default='table');args=parser.parse_args()
out=work/('docs/stage5-evidence/'+args.family+'-editor-windows-r1');out.mkdir(exist_ok=False)
spec=importlib.util.spec_from_file_location('table_editor_private',work/'docs/stage1-evidence/gui-portable/probe.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
desktop=module.PrivateDesktop(out);environment=module.clean_environment();environment['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;'+environment['PATH'];environment.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'),KINSEAL_STAGE4_CAPTURE_DIR=str(out))
names=['tableNativeFieldsCountsAndEnabledDrafts','tableStrictFamilyJsonAndAtomicBounds','tableEightNativeSwitchRasterMasks','tableBackgroundOnlyAlphaAndTransparentGridFrame','tableNativeThreeRowsTwoColumnsAndOneCell','tableKspMixedCopyAndNativeRefusal','tablePropertyFirstMenuCancelPlacementAndUndo','tableEightResizeHandlesAndDraftCancel']
if args.family=='rect':names=['rectNativeFieldsAndTypedDrafts','rectStrictPersistenceMixedCopyAndNativeRefusal','rectFourQuadrantsEightHandleResizeAndCancellation','rectDefaultNativePixelsAndIndependentFillPatterns','rectNativeMenuDoubleClickDraftAndUndo']
if args.family=='line':names=['lineStrictModelPersistenceAndBackendRefusal','lineDirectedPlacementAndAxisAlignedGeometry','lineEndpointRoleAndCancellation','lineDraftCancelAndClipboardPreserveDirection','lineRendererHitAndOpacity','lineNativeMenuAndDoubleClickStyleTransaction']
if args.family=='freeline':names=['freelineObservedFieldsAndSamplingRefusal','freelineObservedDragEndpointsOneTransaction','freelineEndpointResizeCancelAndZeroLength','freelineMixedKspCopyAndNativeRefusal','freelineDefaultRasterAndTransparentStroke','freelineNativeMenuDialogDraftAndUndo']
exe=work/'build/KinSealStage4ControlTests.exe';log=out/'tests.txt';record=dict(startedAtUtc=datetime.datetime.now(datetime.timezone.utc).isoformat(),usbAccessed=False,systemInput=False,desktopSwitched=False,runs=[],passed=False)
try:
    result=desktop.run('table-editor',exe,[*names,'-o',str(log)+',txt'],environment,timeout=180)
    result['sha256']=hashlib.sha256(exe.read_bytes()).hexdigest();text=log.read_text(encoding='utf-8-sig',errors='replace')
    totals=re.search(r'Totals: (\d+) passed, (\d+) failed, (\d+) skipped',text);result['totals']=dict(zip(('passed','failed','skipped'),map(int,totals.groups()))) if totals else None
    result['passed']=result['passed'] and totals is not None and totals[2]=='0' and totals[3]=='0';record['runs']=[result];record['passed']=result['passed']
finally:
    desktop.close();record['privateDesktopHandleClosed']=True;(out/'run.json').write_text(json.dumps(record,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps(dict(passed=record['passed'],totals=record['runs'][0]['totals']),ensure_ascii=False))
raise SystemExit(0 if record['passed'] else 1)
