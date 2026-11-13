"""Record a real geometry blocker and native transport evidence without claiming a curve implementation."""
from pathlib import Path
import datetime,hashlib,json,re,shutil,subprocess,sys
from commit_physical_backfill import git,ROOT,WORK
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
def write(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
def append(relative,text):
 with (WORK/relative).open('a',encoding='utf-8',newline='\n') as f:f.write('\n\n'+text.strip()+'\n')
base=WORK/'docs/stage4-evidence/curve';physical='docs/stage4-evidence/curve/physical/curve-baseline-r1/run.json';h=read(WORK/physical)
assert h['completed'] and h['inputUnchanged'] and h['ownedProcessTerminated'] and h['sourceAtTransferMatchesInput']
assert h['nativeHardware']['usbCompletion']['ready'] and h['nativeHardware']['usbCompletion']['resetReadCompleted']
assert h['sourceProvenance']=='retained_native_reference_source' and h['liteIndependentEncoderVerified'] is False
assert hashlib.sha256((base/'native/curve-threeclick-default.sm').read_bytes()).hexdigest()==h['sourceSha256']=='870b51a7d507b7e3e32ac5a9ed8350edd4851f3bd3d708da861f89cee1bf0c2e'
shutil.copy2(ROOT/'native-physical/physical-cleanup-curve-baseline-r1.json.result.json',base/'physical/curve-baseline-r1/cleanup.json')
shutil.copy2(ROOT/'native-graphics/native-cleanup-curve-selection-r1.json.result.json',base/'native/selection-investigation/cleanup.json')
findings=['原厂三点、三阶段与锚点拖动样例目前均未证明真实曲率；三点源前缀为move/line、move/line四元素，解码背景和属性预览均为倒V折段。','新增选中探针确认三个顶点柄，未发现切换曲率的上下文窗口；场景和截图均已保留。该轮最后尝试保存因工程未改动、action_save禁用而失败，不把完整场景报告标通过。','只交付调查/未决记录；不创建curve类型、折线替代、伪曲线样例或SM/设备编码器。','本轮没有新增软件回归测试，不借用上一控件289项作为曲线用例通过数；T使用原厂基线，和几何恢复/独立输出/完整E分开。']
unresolved=['Q-S04-G03-001','Q-S04-G03-002','Q-S04-G03-003']
write(base/'review.json',dict(mode='read_only_investigation_review',findings=findings,geometryRecovered=False,editorImplementationDelivered=False,softwareTestsExecuted=False,nativeHardwareEvidence=physical,screenConfirmed=False,unresolved=unresolved))
report='docs/曲线圆弧调查与阻塞记录.md'
(WORK/report).write_text('''# 曲线/圆弧：阶段4调查与阻塞记录

第39/41项已处理，编号G03。本项保留为几何阻塞，没有交付曲线绘制器，不能称整控件完成。

## 已取得的原厂证据

菜单action_blight_circle直接进入绘制；三点100,150→200,50→300,150、右键结束后，原厂源的已观测前缀为move/line、move/line四个元素。原厂编译背景和属性预览显示两段倒V直线；第二锚点拖动样例也没有取得曲率。已见类型前缀0x1002a只用于识别拒绝边界，不等于完整SM body恢复。

原厂外观字段独立观测：Line_Type六项、Line_Width 1..9、End_Arrow六项、End_Cap两项、Line_Color黑色、Alpha 0..255，默认实线/1/无箭头/矩形端点/255。字段证据不替代真实曲线几何和控制点角色。

[原始清单](stage4-evidence/curve/native/retained-manifest.json)保留61份已核验文件。本轮[选中探针完整原始记录](stage4-evidence/curve/native/selection-investigation/native-probe-curve-selection-r1.json.gz)确认三个顶点柄；右击未出现曲率切换窗口。最后保存因原工程未发生修改、action_save禁用而被脚本拒绝，因此该场景整体保持partial-or-error，没有隐藏失败。

## 本轮处理和未决

不以折线实现冒充曲线/圆弧；不增加未经证实的贝塞尔、圆心、角度、弧方向或平滑参数。当前继续由现有未支持类型/原厂源门槛拒绝正式输出，没有生成伪SM、设备包或本地curve.ksp。

- Q-S04-G03-001：真实曲率的创建/切换手势、控制柄角色、曲线与圆弧几何尚未恢复。
- Q-S04-G03-002：完整SM body和独立设备背景合成尚未恢复，源/设备输出保持拒绝。
- Q-S04-G03-003：完整现场显示/触控E未验收。原厂基线的传输成功不等于曲线功能通过。

## 实物与清理

[实际下载](stage4-evidence/curve/physical/curve-baseline-r1/run.json)使用保留原厂三点SM，SHA `870b51a7d507b7e3e32ac5a9ed8350edd4851f3bd3d708da861f89cee1bf0c2e`。原厂编译后开始/提交ACK、重启write和后续read均完整退出，清除/固件更新四项false，源输入未变。本次只证明该原厂诊断基线T，不证明独立编码或真实曲率。

选中调查清理30文件，下载清理44文件；源、编译段、原始gzip与截图保留，全部宿主TEMP/TMP/TMPDIR位于独占Z目录。清理逻辑尺寸含硬链接，不冒称实际释放量；证据分别见[调查清理](stage4-evidence/curve/native/selection-investigation/cleanup.json)与[下载清理](stage4-evidence/curve/physical/curve-baseline-r1/cleanup.json)。

本轮没有新增软件回归用例，不借用上一控件的289项作为曲线用例通过数。只读复核见[记录](stage4-evidence/curve/review.json)。按用户允许记录条件限制的要求，进入G07扇形。
''',encoding='utf-8',newline='\n')
progress_path=WORK/'docs/stage4-evidence/progress.json';d=read(progress_path);item=next(x for x in d['controls'] if x['id']=='G03');assert item['status']=='in_progress'
item.update(status='investigated_geometry_blocked',scope='原厂外观/源前缀/编译与控制柄调查；真实曲率未恢复，不提供折线替代实现',evidence='curve/review.json',report=report,hardwareEvidence=physical,unresolved=unresolved,layers=dict(D='bounded_investigation',U='geometry_blocked',P_KSP='not_implemented',P_SM='blocked',R_PC='geometry_blocked',C='blocked',T='native_reference_transport_passed_independent_encoder_blocked',E='unobserved'))
next(x for x in d['controls'] if x['id']=='G07')['status']='in_progress';d['active']='G07';d['investigationSlicesRecorded']=1;d['processedIncludingExisting']=39;d['screenWidthDiagnosis'].update(activeControl='G07',processedIncludingExisting=39);d['blockedLayers']=sum(len(x.get('unresolved',[])) for x in d['controls']);write(progress_path,d)
p=WORK/'docs/阶段4逐控件进度.md';text=p.read_text(encoding='utf-8-sig');text=re.sub(r'^\| 当前步骤 \|.*$','| 当前步骤 | 已处理39/41项：38项编辑/已有切片，1项曲率阻塞调查；下一项G07；新下载均核查提交/重启后的USB调用 |',text,flags=re.M);p.write_text(text,encoding='utf-8',newline='\n');append('docs/阶段4逐控件进度.md','## G03：第39/41项调查完成，曲率阻塞\n\n原厂样例及新选中探针仍只有已证折段与三个顶点柄；真实曲线/圆弧未恢复，不创建折线替代实现。已实际下载原厂三点基线并核查完整提交/重启/read，T和几何/E分别记录。无新增软件用例通过声明。[专项记录](曲线圆弧调查与阻塞记录.md)。下一项G07。')
protocol='docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md';n=max(map(int,re.findall(r'^## (\d+)\.',(WORK/protocol).read_text(encoding='utf-8-sig'),re.M)))+1
append(protocol,f'## {n}. 阶段4曲线/圆弧调查阻塞及原厂基线T（2026-11-13）\n\nG03真实曲率未恢复，原厂三点和锚点拖动样例的已观测path前缀及背景仅为倒V折段；新增选中探针只证明三个顶点柄，保存因未改动而禁用，场景失败边界保留。不将折线作为曲线交付，不新增几何/SM/设备编码。\n\n保留原厂源SHA `{h["sourceSha256"]}` 的[实际下载](stage4-evidence/curve/physical/curve-baseline-r1/run.json)按§73完整提交ACK、重启及其后读取通过，四清除/更新false。T只证明该基线传输，独立编码和真实曲率/完整E仍阻塞。见[记录](曲线圆弧调查与阻塞记录.md)。')
manual='design_plan/A03_基础图形与表格规范.md';p=WORK/manual;text=p.read_text(encoding='utf-8-sig');text=re.sub(r'^\| G03 \|.*$', '| G03 | 曲线/圆弧 | 调查完成；曲率未恢复 | [记录](../'+report+') | [原厂基线提交/重启通过](../'+physical+') |',text,flags=re.M);p.write_text(text,encoding='utf-8',newline='\n');append(manual,'## 2026-11-13 G03调查阻塞\n\n原厂三点与锚点拖动样例没有取得曲率；本轮选中补证确认三个顶点柄。没有交付折线替代或推测贝塞尔/圆弧参数。原厂基线T已按完整提交/重启/read流程下载，详见[记录](../'+report+')。')
matrix='design_plan/控件文档与实施覆盖矩阵.md';p=WORK/matrix;text=p.read_text(encoding='utf-8-sig');row='| 曲线/圆弧 | [A03](A03_基础图形与表格规范.md) | 未计 | [阻塞记录](../'+report+') | 调查补证 | 曲率阻塞 | 未实施；SM阻塞 | 曲率阻塞 | 阻塞 | 原厂基线下载通过；独立编码阻塞 | 待现场 | 恢复真实曲率 |'
if re.search(r'^\| 曲线/圆弧 \|',text,re.M):text=re.sub(r'^\| 曲线/圆弧 \|.*$',row,text,flags=re.M)
else:text+='\n'+row+'\n'
p.write_text(text,encoding='utf-8',newline='\n')
index='design_plan/A04_31控件实施索引.md';p=WORK/index;lines=p.read_text(encoding='utf-8-sig').splitlines()
for i,line in enumerate(lines):
 fields=line.split('|')
 if len(fields)==8 and 'G03' in fields[1]:fields[5]=' [曲率阻塞记录](../'+report+') ';lines[i]='|'.join(fields)
p.write_text('\n'.join(lines)+'\n',encoding='utf-8')
ledger='design_plan/分阶段文档证据/阶段交付与清理记录.json';p=WORK/ledger;d=read(p);task=next(x for x in d['taskStates'] if x['taskId']=='S04-11');task['resumePoint']=dict(document='05_阶段4_控件逐项复刻.md',nextAction='G07扇形；G03曲率保持未决');d.setdefault('stage4Executions',[]).append(dict(id='stage4-curve-investigation-20261113',geometryRecovered=False,editorImplemented=False,softwareTestsExecuted=False,usbAccessed=True,completeControl=False,hardwareEvidence=physical,unresolved=unresolved));write(p,d)
log='程序修改记录.md';nlog=max(map(int,re.findall(r'^### 修改(\d+)：',(WORK/log).read_text(encoding='utf-8-sig'),re.M)))+1;append(log,f'### 修改{nlog}：阶段4曲线圆弧调查与曲率阻塞记录\n- 修改内容：\n  1. 保留原厂61份基础证据及本轮选中/控制柄补证和场景失败边界，未恢复真实曲率，不提供折线替代。\n  2. 实际下载三点原厂诊断基线，完整提交/重启/read通过；清理调查30、下载44个文件。\n  3. 更新协议§{n}、覆盖矩阵/索引、进度和恢复台账；调查完成计入处理进度，不计编辑器交付或用例通过。\n- 修改文件：docs/曲线圆弧调查与阻塞记录.md、curve证据、阶段4进度、USB协议、A03/A04/覆盖与恢复台账、本记录。\n- 影响范围：文档/原厂基线传输证据；未修改程序业务逻辑。\n- 原因：按用户要求记录暂时无法恢复的功能，再进入下一项。\n- 修改标记：//20261113-6 add（文档）')
repro=base/'reproduction';repro.mkdir(exist_ok=True)
for name in ['deliver_curve_block.py','archive_graphic_probe.py','run_stage4_native_physical.py','archive_physical_run.py','execute_native_cleanup.ps1']:shutil.copy2(ROOT/name,repro/name)
paths=[report,'docs/stage4-evidence/curve/','docs/stage4-evidence/progress.json','docs/阶段4逐控件进度.md',protocol,manual,matrix,index,ledger,log]
entries=[x[3:] for x in git('status','--porcelain=v1','-z','--untracked-files=all').stdout.decode('utf-8').split('\0') if x]
assert all(any(x==y or y.endswith('/') and x.startswith(y) for y in paths) for x in entries),entries
assert not git('diff','--cached','--name-only').stdout.strip();git('diff','--check');previous=git('rev-parse','HEAD').stdout.decode().strip();now=datetime.datetime.now(datetime.timezone(datetime.timedelta(hours=8)));day=now.strftime('%Y年%m月%d日');subjects=git('log','--all','--format=%s','--since='+now.strftime('%Y-%m-%dT00:00:00+08:00')).stdout.decode('utf-8');number=max([int(m.group(1)) for m in re.finditer(re.escape(day)+r' \d\d:\d\d:\d\d，修改(\d+)：',subjects)],default=0)+1;title=now.strftime('%Y年%m月%d日 %H:%M:%S')+f'，修改{number}：记录曲线圆弧曲率阻塞并完成原厂基线下载';git('add','--',*paths);git('commit','-m',title);print(json.dumps(dict(commit=git('rev-parse','HEAD').stdout.decode().strip(),title=title,protocolSection=n),ensure_ascii=False));subprocess.run([sys.executable,'-B',str(ROOT/'publish_stage4.py'),'--previous',previous],check=True)
