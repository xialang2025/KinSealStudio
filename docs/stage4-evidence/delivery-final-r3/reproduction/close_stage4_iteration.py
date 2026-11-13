"""Audit actual per-control proof and close the bounded Stage4 iteration without upgrading blocked layers."""
from pathlib import Path
import collections
import copy
import datetime
import hashlib
import json
import re
import shutil
import subprocess

root = Path(__file__).resolve().parent
work = root / 'worktree'
main = root.parents[1] / 'KinSealStudio'
out = work / 'docs/stage4-evidence/delivery-final-r3'
read = lambda p: json.loads(p.read_text(encoding='utf-8-sig'))
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
def write(p, d):
    p.write_text(json.dumps(d, ensure_ascii=False, indent=2) + '\n', encoding='utf-8', newline='\n')
def append(relative, text):
    with (work / relative).open('a', encoding='utf-8', newline='\n') as stream:
        stream.write('\n\n' + text.strip() + '\n')

assert not (out / 'completion-audit.json').exists(), 'Do not duplicate final ledgers.'
manifest = read(out / 'package-manifest.json')
smoke = read(out / 'portable-r1/run.json')
tests = read(work / 'docs/stage4-evidence/table/windows-r1/run.json')
assert smoke['passed'] and len(smoke['runs']) == 40 and all(x['passed'] for x in smoke['runs'])
assert smoke['roundtripFortyObjectsAndConfigurationsEqual'] and smoke['onlyBundledQtDllsObserved'] and smoke['privateDesktopClosed']
assert not smoke['temporaryFilesRemaining'] and tests['passed']
assert sum(x['totals']['passed'] for x in tests['runs']) == 303
assert all(x['totals']['failed'] == x['totals']['skipped'] == 0 for x in tests['runs'])
package = Path(manifest['package'])
for entry in manifest['files']:
    path = package / entry['path']
    assert path.stat().st_size == entry['bytes'] and sha(path) == entry['sha256']
for n in range(1,6):
    assert sha(out / f'portable-r1/page-{n}.png') == sha(work / f'docs/stage4-evidence/delivery-final-r2/portable-r1/page-{n}.png')
sample = read(package / 'samples/stage4/stage4-editor-40-slices.ksp')
assert len(sample['pages']) == 6 and sum(len(p['objects']) for p in sample['pages']) == 40
assert all(o['type'] != 'curve' for p in sample['pages'] for o in p['objects'])
progress_path = work / 'docs/stage4-evidence/progress.json'
progress = read(progress_path)
assert len(progress['controls']) == 41 and progress['processedIncludingExisting'] == 41
assert progress['editorSlicesDelivered'] + progress['skippedExisting'] == 40
assert progress['investigationSlicesRecorded'] == 1 and progress['complete'] is False
retained_old = read(work / 'docs/stage4-evidence/physical-backfill/completion-audit.json')
records = []
for item in progress['controls']:
    if item['id'] in ('C01','C02','C03','C04','C05'):
        old = next(x for x in retained_old['records'] if x['id'] == item['id'])
        assert old['provenance'] == 'existing_independent_transport' and sha(work / old['evidence']) == old['evidenceSha256']
        records.append(dict(id=item['id'], name=item['name'], provenance='retained_earlier_independent_transport',
            evidence=old['evidence'], evidenceSha256=old['evidenceSha256'], newDownloadExecutedInFinalAudit=False))
        continue
    evidence = item['hardwareEvidence']
    path = work / evidence
    run = read(path)
    assert all(run[x] for x in ('completed','inputUnchanged','sourceAtTransferMatchesInput','ownedProcessTerminated'))
    usb = run['nativeHardware']['usbCompletion']
    assert usb['ready'] and usb['resetReadCompleted'] and usb['inFlightCalls'] == 0
    assert run['sourceProvenance'] == 'retained_native_reference_source' and run['liteIndependentEncoderVerified'] is False
    assert all(x['afterChecked'] is False for x in run['nativeHardware']['transferOptions'])
    native = path.parents[2] / 'native'
    candidates = [p for p in native.rglob('*.sm') if sha(p) == run['sourceSha256']]
    assert candidates, 'Missing retained source for ' + item['id']
    cleanup = read(path.with_name('cleanup.json'))
    assert cleanup.get('deletedFiles', 0) > 0
    record = dict(id=item['id'], name=item['name'], provenance='retained_native_reference_source',
        evidence=evidence, evidenceSha256=sha(path), sourceSha256=run['sourceSha256'],
        retainedSource=str(candidates[0].relative_to(work)).replace('\\','/'),
        commitResetAndSubsequentReadVerified=True, independentEncoderVerified=False, fullEVerified=False)
    screen_path = path.with_name('screen-result.json')
    if screen_path.exists():
        screen = read(screen_path)
        assert screen['source'] == 'explicit_user_reply' and screen['screenTouchConfirmed'] is False
        relative = str(screen_path.relative_to(work)).replace('\\','/')
        item.update(screenObservation=relative, screenObservationScope='native_reference_default_diagnostic_only')
        item.setdefault('layers',{})['E'] = 'native_default_diagnostic_display_confirmed_full_E_unverified'
        record.update(defaultNativeDiagnosticDisplayConfirmed=True, userAnswer=screen['answer'], screenObservation=relative)
    records.append(record)
assert sum(x['provenance'] == 'retained_native_reference_source' for x in records) == 36
assert sum(x.get('defaultNativeDiagnosticDisplayConfirmed',False) for x in records) == 6
cleanup = read(out / 'cleanup.json')
plan = read(out / 'cleanup-plan.json')
assert cleanup['deletedFiles'] == len(plan['files']) and all(not Path(x['path']).exists() for x in plan['files'])
assert cleanup['allRetainedPeersVerified'] and cleanup['mainUserBuildPreserved']
c_temp = read(root / 'c-temp-older-frida-plan.json')
assert len(c_temp['files']) == 0
main_status = subprocess.check_output(['git','-C',str(main),'status','--porcelain=v1','-z']).decode('utf-8').split('\0')
tracked = [x[3:] for x in main_status if x and not x.startswith('??')]
untracked = [x[3:] for x in main_status if x.startswith('??')]
assert len(tracked) == 18 and len(untracked) == 7
source_status = subprocess.check_output(['git','-C',str(work),'status','--porcelain','--','src','tests','CMakeLists.txt']).strip()
assert not source_status, 'Final close must not include untested source changes.'
audit = dict(recordedAtUtc=datetime.datetime.now(datetime.timezone.utc).isoformat(), processed=41,
    existingOrEditorSlices=40, geometryBlocked=['G03'], completeStage4=False,
    softwareTests=dict(passed=303,failed=0,skipped=0), portableChecks=40,
    mixedPages=6,mixedObjects=40,mixedRoundtripEqual=True,
    hardwareEvidence=dict(retainedEarlierIndependent=5,nativeReferenceWithFullLifecycle=36,defaultDiagnosticHumanObservations=6,
        completeIndependentRequirementPassed=False,completeEPassed=False,records=records),
    readOnlyReview=dict(businessLogicModified=False,sourceAndTestsClean=True,
        findings=['40对象及配置保存重开严格相等；页顺序和三语言保留。',
        '最终包六页真实截图已检查；前五页与已检查r2哈希一致，第六页已按默认10/10圆角修正并重新检查。',
        '800×480画布在固定窗口内使用滚动条显示，右侧视口外对象不表示源几何被裁掉；不据截图宣称全部像素一致。',
        '36项原厂基线完整提交/重启/后续read和源哈希已复核；已有5项只复核历史证据身份。',
        '完整SM/独立设备编码、全部变体与现场E未通过；G03曲率、G02采样、G07角柄仍有缺口。']),
    cleanupEvidence='docs/stage4-evidence/delivery-final-r3/cleanup.json',
    cTempVerifiedOwnFilesRemaining=0, mainPreExistingChanges=dict(tracked=tracked,untracked=untracked),
    originalInstallationAndUserBuildPreserved=True)
write(out / 'completion-audit.json', audit)
write(out / 'c-temp-verification.json', dict(knownFridaFilesRemaining=0, scanEvidence='reproduction/c-temp-older-frida-plan.json',
    exclusiveZTemporaryRoots=True, unrelatedUserTemporaryFilesNotDeleted=True))
repro = out / 'reproduction'
for name in ('close_stage4_iteration.py','prepare_stage4_final_cleanup.py','execute_stage4_final_cleanup.ps1',
             'record_graphic_screen_replies.py','cleanup_older_c_frida.ps1','c-temp-older-frida-plan.json'):
    shutil.copy2(root / name, repro / name)
progress.update(iterationState='bounded_editor_iteration_closed_with_recorded_gaps', processedIncludingExisting=41,
    active=None, complete=False, completionAudit='docs/stage4-evidence/delivery-final-r3/completion-audit.json',
    hardwareEvidenceSummary=dict(retainedEarlierIndependent=5,nativeReferenceWithFullLifecycle=36,defaultDiagnosticHumanObservations=6),
    nextWork='阶段5独立源/设备格式恢复；按各控件Q-ID接续；G03真实曲率与G02/G07缺口保持登记')
write(progress_path, progress)
doc_path = work / 'docs/阶段4逐控件进度.md'
doc = doc_path.read_text(encoding='utf-8-sig')
doc = re.sub(r'^\| 当前步骤 \|.*$', '| 当前步骤 | 本轮41/41项已处理：40项已有/编辑切片，1项几何阻塞；统一混合回归及便携包检查完成，阶段4完整各层仍有未决 |', doc, flags=re.M)
doc = re.sub(r'^\| C21～C31、G01/G08 \|.*$', '| 实物证据 | 36项原厂基线已核查完整提交/重启/后续read；5类保留早期独立下载；6份默认诊断页人工显示确认，完整E仍未验 |', doc, flags=re.M)
doc_path.write_text(doc, encoding='utf-8', newline='\n')
summary = '本轮41项逐项处理结束：40项为已有或编辑/KSP切片，G03曲线/圆弧真实曲率阻塞。最终控件234、源59、SDK宏10，共303/0/0；六页40对象混合工程保存重开及配置严格相等，便携包40项检查通过。5类保留此前独立下载证据；36项原厂基线的提交ACK、重启write和其后read完整退出，源身份及四清除/更新项复核通过。用户已确认椭圆、圆角矩形、自由直线、多边形、曲线诊断折段、纯图像表格这6份各自默认页显示且没有宽度提示；这不代表完整变体、触控或Lite独立编码通过。'
append('docs/阶段4逐控件进度.md', '## S04-12：本轮混合回归与收尾\n\n' + summary + '\n\n[最终交付](阶段4_41项本轮交付.md) · [逐项实物证据核对](stage4-evidence/delivery-final-r3/completion-audit.json)。下一轮按专项Q-ID恢复完整源/设备与几何缺口，不重复已交付编辑工作。')
report = work / 'docs/阶段4_41项本轮交付.md'
append(str(report.relative_to(work)), '## 复核、实物及清理\n\n' + summary + f'\n\n首轮混合工程因目标工程缺少报警列名语言English被现有模型拒绝；修正打包样例的语言集合，保留三语言及消息/列名后通过，没有放宽生产校验。最终包使用原厂默认10/10圆角展示；失败r1输入、错误及成功r2记录仍保留。\n\n最终清理{cleanup["deletedFiles"]}份可再生构建对象/依赖与重复包文件，逻辑尺寸{cleanup["deletedLogicalBytes"]}字节，保留r3最终包、唯一输入、源、日志和原始证据。C盘本任务已知Frida临时文件剩余0；主目录18个既存修改和7项未跟踪内容保持。\n\n[收尾核对](stage4-evidence/delivery-final-r3/completion-audit.json) · [清理记录](stage4-evidence/delivery-final-r3/cleanup.json)。\n\n调序/删除/复制/撤销、资源及语言字段的逐类型测试保留在303项内；本次40对象混合附加检查限定为加载、保存重开、页顺序、配置、实际六页启动和输出拒绝。全部40对象的混合PLC读写跨度、完整运行和原厂SM往返未升级为通过。')
protocol_relative = 'docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md'
protocol = (work / protocol_relative).read_text(encoding='utf-8-sig')
number = max(map(int,re.findall(r'^## (\d+)\.',protocol,re.M))) + 1
append(protocol_relative, f'## {number}. 阶段4本轮逐项实物证据收尾（2026-11-13）\n\n' + summary + '\n\n[逐项源身份、原始下载和人工确认索引](stage4-evidence/delivery-final-r3/completion-audit.json)。原厂UI“下载完成”不代表生命周期终点；仍按§73等提交确认、完整重启write和后续read退出后才结束自有进程。本机型完整上下文内重启后read=-5的既有边界保持，未扩大为任意失败可忽略。')
append('design_plan/05_阶段4_控件逐项复刻.md', '## 6. 2026-11-13本轮实施与收尾状态\n\n' + summary + '\n\n本轮顺序清单处理完毕，阶段4全控件D/U/P/R/C/T/E整层未全部通过。完整源/独立设备编码按阶段5和各专项Q-ID继续；G03真实曲率、G02采样、G07角度柄保持明确缺口。S04-12附加验证限定为混合KSP重开、顺序/配置、六页启动、包内依赖和未支持输出拒绝；混合PLC读写跨度与完整现场E仍待后续。\n\n[交付报告](../docs/阶段4_41项本轮交付.md) · [进度](../docs/阶段4逐控件进度.md) · [逐项核对](../docs/stage4-evidence/delivery-final-r3/completion-audit.json)。')
matrix_path = work / 'design_plan/控件文档与实施覆盖矩阵.md'
lines = matrix_path.read_text(encoding='utf-8-sig').splitlines()
observed = {x['name'] for x in records if x.get('defaultNativeDiagnosticDisplayConfirmed')}
for index,line in enumerate(lines):
    fields = line.split('|')
    if len(fields) >= 13 and fields[1].strip() in observed:
        fields[11] = ' 原厂默认诊断页显示确认；完整E未验 '
        lines[index] = '|'.join(fields)
matrix_path.write_text('\n'.join(lines)+'\n',encoding='utf-8',newline='\n')
ledger_path = work / 'design_plan/分阶段文档证据/阶段交付与清理记录.json'
ledger = read(ledger_path)
for task in ledger['taskStates']:
    if task['taskId'].startswith('S04-'):
        task.setdefault('executionHistory',[]).append(dict(executionId='stage4-final-20261113',previousState=copy.deepcopy(task.get('resumePoint')),recordedAtUtc=audit['recordedAtUtc']))
        task['resumePoint'] = dict(document='05_阶段4_控件逐项复刻.md',nextAction='本轮顺序切片已处理；按专项Q-ID恢复独立源/设备、几何缺口及完整E，不重复已交付编辑工作。')
        if task['taskId'] == 'S04-12':
            task.update(documentation='updated_final_iteration', investigation=dict(state='actual_evidence_audited',evidence='../docs/stage4-evidence/delivery-final-r3/completion-audit.json'),
                implementation=dict(state='bounded_mixed_editor_regression_passed',scope='6页40对象KSP加载/严格重开、页顺序/配置、40项实际包检查；完整SM/设备/PLC/E未通过'))
ledger['stage4IterationClose'] = dict(processed=41,editorSlicesIncludingExisting=40,geometryBlocked=['G03'],completeStage4=False,
    finalReport='../docs/阶段4_41项本轮交付.md',audit='../docs/stage4-evidence/delivery-final-r3/completion-audit.json',
    cleanupFiles=cleanup['deletedFiles'],deletedLogicalBytes=cleanup['deletedLogicalBytes'])
write(ledger_path,ledger)
log_path = work / '程序修改记录.md'
modification = max(map(int,re.findall(r'^### 修改(\d+)：',log_path.read_text(encoding='utf-8-sig'),re.M)))+1
append('程序修改记录.md', f'### 修改{modification}：阶段4本轮混合工程、便携包和逐项证据收尾\n- 修改内容：\n  1. {summary}\n  2. 修正混合样例的工程语言集合及默认圆角，保留首轮失败和中间成功记录；模型校验未放宽。\n  3. 清理自有构建中间物和重复包，保留最终包和唯一输入；同步阶段文档、协议、进度、覆盖和台账。\n- 修改文件：samples/stage4/stage4-editor-40-slices.ksp、docs/阶段4_41项本轮交付.md、docs/stage4-evidence/delivery-final-r1~r3、progress.json、阶段4逐控件进度、协议、design_plan对应文档与本记录。\n- 影响范围：本轮交付样例、文档及可再生中间物；本次没有新增生产逻辑。\n- 原因：用户要求逐项交付、实物验证、自动清理及继续完成阶段4后续内容。\n- 修改标记：//20261113-10 add（样例/文档）')
print(json.dumps(dict(processed=41,editorSlices=40,geometryBlocked=['G03'],nativeReferenceLifecycleVerified=36,retainedEarlierIndependent=5,defaultDiagnosticHumanObservations=6,protocolSection=number,cleanupFiles=cleanup['deletedFiles']),ensure_ascii=False))
