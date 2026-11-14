"""Record the actually verified eighth portable delivery without inventing remote push success."""
from pathlib import Path
import datetime,json,re,shutil,subprocess
root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree'
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
write=lambda p,d:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def append(relative,text):
 p=work/relative;p.write_text(p.read_text(encoding='utf-8-sig').rstrip()+'\n\n'+text.strip()+'\n',encoding='utf-8')
def git(*a):return subprocess.run(['git','-C',str(work),*a],check=True,capture_output=True).stdout
head=git('rev-parse','HEAD').decode().strip();assert head=='304151a2e59859f46e29e5345099fcfab8b21b14'
out=work/'docs/stage5-evidence/delivery-slice8-r1';d=read(out/'package-manifest.json');run=read(out/'run.json');assert d['sourceCommit']==head and run['passed'] and d['checksPassed']==117 and run['onlyBundledQtDllsObserved']
note=f"第八批便携包`{d['package']}`已通过117项实际检查，源提交`{head}`。新增圆/椭圆默认、圆形、宽椭圆和无描边白填充回归样例，以及十三对象混合。10/12/14段均实际编译并复读；Qt DLL全部来自包内，十三对象截图已实际查看。源/legacy/Alpha等负向正确拒绝且无输出残留，没有USB通信，deviceAcknowledged=false。"
append('docs/阶段5_椭圆定点细分背景编译与差分验收.md','## 第八批便携交付\n\n'+note+'\n\n[实际交付证据](stage5-evidence/delivery-slice8-r1/package-manifest.json)。默认/宽椭圆/白填充样例分别覆盖已修正舍入组合，模型严格解析/设备编译/包复读逐个执行。Git同步与远端状态以发布回执为准；推送未确认时保留本地提交，不能写成已推送。阶段5整体仍未完成。')
append('docs/UI编译规则与阶段交付规范.md','## 16. 第八批椭圆便携规则（2026-11-14）\n\n'+note+'\n\nellipse JSON使用闭合图形线型/宽度/线色、三色/填充/Alpha字段，不增加line专用endArrow/endCap或polygon的points。几何由整数x/y/width/height生成路径；白填充与黑边独立D舍入后合成，不把raw565背景展开再混色。宽300×150和无线白填样例必须与默认一起回归，防止只默认相等漏掉加128顺序。模型/源/设备能力分别报告，完整SM仍拒绝。[证据](stage5-evidence/delivery-slice8-r1/run.json)。')
append('docs/阶段5逐项进度.md','### 第八批便携交付\n\n'+note+'\n\n扇形仅继续离线候选调查，未直接开放；26新动态类型、多页及活动配置继续按台账推进。')
append('design_plan/06_阶段5_设备编译与格式差分.md','### 14.1 第八批便携实测\n\n'+note+'\n\n[原始证据](../docs/stage5-evidence/delivery-slice8-r1/run.json)。新产物T/E分别留给阶段6/10。')
append('docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md','### 91.1 圆/椭圆便携验证\n\n'+note+'\n\n[交付回执](stage5-evidence/delivery-slice8-r1/package-manifest.json)。静态图形未改变既有控制记录/扫描及纯传输计划。')
p=work/'docs/stage5-evidence/progress.json';progress=read(p);progress['latestDelivery']=dict(package=d['package'],sourceCommit=head,evidence='docs/stage5-evidence/delivery-slice8-r1/package-manifest.json',checksPassed=117,usbAccessed=False);progress['ellipseSlice']['delivery']=progress['latestDelivery'];progress['active']='G06椭圆及第八批便携已交付；继续扇形等剩余编译任务';write(p,progress)
append('程序修改记录.md',f'''### 修改87：阶段5圆/椭圆便携包与十三对象样例
- 修改内容：第八批便携包通过117项真实检查，源提交`{head}`；圆/椭圆默认、圆形、宽300×150、无线白填及十三对象混合均实际生成10/12/14段并复读，截图已核且只加载包内Qt；源/legacy/Alpha负向继续拒绝，未访问USB。
- 修改文件：samples/stage5新增5样例、docs/stage5-evidence/delivery-slice8-r1、UI编译规则/椭圆专项/阶段5进度/协议91及本记录。
- 影响范围：可运行离线交付与证据，无新增生产逻辑；完整阶段5、新产物T/E仍未完成。同步/远端状态以实际发布回执为准，未确认推送时本地提交保留。
- 原因：交付第八个受限背景C切片，提供覆盖舍入错误的可复验样例，并同步详细UI/JSON/设备编译规则。
- 修改标记：//20261114-2 add（交付）''')
for name in ('prepare_ellipse_delivery.py','finalize_ellipse_delivery.py'):shutil.copy2(root/name,out/'reproduction'/name)
paths=['samples/stage5/ellipse-default.ksp','samples/stage5/ellipse-circle.ksp','samples/stage5/ellipse-wide.ksp','samples/stage5/ellipse-fill-only.ksp','samples/stage5/mixed-thirteen.ksp','docs/stage5-evidence/delivery-slice8-r1','docs/UI编译规则与阶段交付规范.md','docs/阶段5_椭圆定点细分背景编译与差分验收.md','docs/阶段5逐项进度.md','design_plan/06_阶段5_设备编译与格式差分.md','docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md','docs/stage5-evidence/progress.json','程序修改记录.md']
assert not git('diff','--cached','--name-only').strip();git('status','--short');git('add','--',*paths)
staged=list(filter(None,git('diff','--cached','--name-only','-z').decode().split('\0')));assert all(any(n==p or n.startswith(p+'/') for p in paths) for n in staged);git('diff','--cached','--check')
now=datetime.datetime.now(datetime.timezone(datetime.timedelta(hours=8)));day=now.strftime('%Y年%m月%d日');subjects=git('log','--all','-400','--format=%s').decode();sequence=max([int(m[1]) for m in re.finditer(re.escape(day)+r' \d\d:\d\d:\d\d，修改(\d+)：',subjects)],default=0)+1
message=now.strftime('%Y年%m月%d日 %H:%M:%S')+f'，修改{sequence}：交付圆与椭圆便携包及十三对象编译回归样例'
git('commit','-m',message);result=dict(commit=git('rev-parse','HEAD').decode().strip(),message=message,files=len(staged),sourceCommit=head,checksPassed=117);write(root/'ellipse-delivery-commit.json',result);print(json.dumps(result,ensure_ascii=False))
