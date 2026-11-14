"""Record the verified r2 package, preserving the rejected r1 sample and cleanup receipt."""
from pathlib import Path
import datetime,hashlib,json,re,shutil,subprocess
root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree'
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
write=lambda p,d:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def append(relative,text):
 p=work/relative;p.write_text(p.read_text(encoding='utf-8-sig').rstrip()+'\n\n'+text.strip()+'\n',encoding='utf-8')
def git(*a):return subprocess.run(['git','-C',str(work),*a],check=True,capture_output=True).stdout
head=git('rev-parse','HEAD').decode().strip();assert head=='4607ba7f76294475360aaaf43cf1b028005a9997'
out=work/'docs/stage5-evidence/delivery-slice7-r2';d=read(out/'package-manifest.json');run=read(out/'run.json');assert run['passed'] and d['checksPassed']==102 and d['sourceCommit']==head and run['onlyBundledQtDllsObserved']
failed=read(work/'docs/stage5-evidence/delivery-slice7-r1/failed-package-cleanup.json');assert failed['deletedFiles']==42 and failed['deletedLogicalBytes']==114061208
note=f"第七批便携包`{d['package']}`已通过102项真实检查，源提交`{head}`已推送GitHub。新增多边形默认/线框/凹五边形及十二对象混合；实际10/12/14段编译并复读，Qt DLL均来自包内，十二对象截图已实际查看。完整SM/旧单文本/自交等负向入口正确拒绝且无残留输出，没有USB通信。"
append('docs/阶段5_多边形单元格背景编译与差分验收.md','## 第七批便携交付\n\n'+note+'\n\n[实际交付记录](stage5-evidence/delivery-slice7-r2/package-manifest.json)。r1生成样例时误把结构体休眠endArrow/endCap加入polygon JSON，严格模型解析正确拒绝；修正采样配方后用同一EXE重跑r2。失败输入/原始日志及清理清单保留，已删除42个失败r1包文件/114061208B；没有修改生产代码，也没有把失败包报告为可用。')
append('docs/UI编译规则与阶段交付规范.md','## 14. 第七批便携包与工程字段检查（2026-11-14）\n\n'+note+'\n\npolygon的模型结构体保留endArrow/endCap为0，但严格JSON只序列化闭合家族定义的线/填充/颜色/Alpha/有序points；不能把线条专用休眠键写进polygon源文件。r1负例实际触发“图形字段缺失或未知”，r2修正样例键后全部102项通过，生产EXE保持同源SHA。复验先看project-capabilities，再compile-project与verify-compiled-package；旧目标拒绝覆盖，设备ACK始终false。[证据](stage5-evidence/delivery-slice7-r2/run.json)。')
append('docs/阶段5逐项进度.md','### 第七批便携交付r2\n\n'+note+'\n\n后续G05椭圆正在建立原厂变体，候选算法的诊断零差不等于已开放C；阶段5仍未整体完成。')
append('design_plan/06_阶段5_设备编译与格式差分.md','### 13.1 第七批便携实测\n\n'+note+'\n\n[实际证据](../docs/stage5-evidence/delivery-slice7-r2/package-manifest.json)，新产物T/E继续按阶段6/10执行。')
append('docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md','### 90.1 多边形便携包验证\n\n'+note+'\n\n源/设备门槛分别检查，包内compile/verify不触发USB。[交付证据](stage5-evidence/delivery-slice7-r2/run.json)。')
p=work/'docs/stage5-evidence/progress.json';progress=read(p);progress.update(active='G09多边形与第七批便携交付完成；继续G05椭圆正式语料及独立曲线候选');progress['latestDelivery']=dict(package=d['package'],sourceCommit=head,evidence='docs/stage5-evidence/delivery-slice7-r2/package-manifest.json',checksPassed=102,usbAccessed=False);progress['polygonSlice']['delivery']=progress['latestDelivery'];write(p,progress)
append('程序修改记录.md',f'''### 修改85：阶段5多边形便携包与十二对象离线样例
- 修改内容：第七批便携包通过102项实际检查，源提交`{head}`已推送；新增多边形默认/线框/凹五边形及十二对象混合，10/12/14段实际编译复读，截图已核且只加载包内Qt。r1因多余JSON键被严格解析拒绝，修正样例配方后用同EXE通过r2；保留失败输入与日志并删除42个失败包文件/114061208B。另5份默认解码中间帧1536444B已核gzip并清理。
- 修改文件：samples/stage5新增4样例、docs/stage5-evidence/delivery-slice7-r1和r2、UI编译规则/专项报告/阶段5进度/协议90及本记录。
- 影响范围：可运行离线交付与证据，无新生产逻辑；没有USB通信，阶段5整体仍未完成。
- 原因：形成第七个受限背景C切片的可复验便携交付，并同步本批详细UI与JSON编译规则。
- 修改标记：//20261114-1 add（交付）''')
repro=out/'reproduction'
for name in ('prepare_polygon_delivery.py','finalize_polygon_delivery.py','cleanup_polygon_decoded.ps1','cleanup_failed_slice7.ps1'):shutil.copy2(root/name,repro/name)
paths=['samples/stage5/polygon-default.ksp','samples/stage5/polygon-outline.ksp','samples/stage5/polygon-concave.ksp','samples/stage5/mixed-twelve.ksp','docs/stage5-evidence/delivery-slice7-r1','docs/stage5-evidence/delivery-slice7-r2','docs/UI编译规则与阶段交付规范.md','docs/阶段5_多边形单元格背景编译与差分验收.md','docs/阶段5逐项进度.md','design_plan/06_阶段5_设备编译与格式差分.md','docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md','docs/stage5-evidence/progress.json','程序修改记录.md']
assert not git('diff','--cached','--name-only').strip();git('status','--short');git('add','--',*paths)
staged=list(filter(None,git('diff','--cached','--name-only','-z').decode().split('\0')));assert all(any(n==p or n.startswith(p+'/') for p in paths) for n in staged)
git('diff','--cached','--check')
now=datetime.datetime.now(datetime.timezone(datetime.timedelta(hours=8)));day=now.strftime('%Y年%m月%d日');subjects=git('log','--all','-400','--format=%s').decode();sequence=max([int(m[1]) for m in re.finditer(re.escape(day)+r' \d\d:\d\d:\d\d，修改(\d+)：',subjects)],default=0)+1
message=now.strftime('%Y年%m月%d日 %H:%M:%S')+f'，修改{sequence}：交付多边形便携包并验证十二对象离线编译规则'
git('commit','-m',message);result=dict(commit=git('rev-parse','HEAD').decode().strip(),message=message,files=len(staged),sourceCommit=head,checksPassed=102)
write(root/'polygon-delivery-commit.json',result);print(json.dumps(result,ensure_ascii=False))
