"""Freeze actual current five-family positive CLI cases using the already shipped Stage4 executable."""
from pathlib import Path
import copy
import hashlib
import importlib.util
import json

root=Path(__file__).resolve().parent
work=root.parent/'kinseal-stage4-20261112/worktree'
package=root.parents[1]/'KinSealStudio/dist/KinSealStudioLite-Stage4-41items-0.10.1-final-r3'
out=work/'docs/stage5-evidence/baseline-cli-r1';out.mkdir(exist_ok=False)
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
write=lambda p,d:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
spec=importlib.util.spec_from_file_location('baseline_private',work/'docs/stage1-evidence/gui-portable/probe.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
desktop=module.PrivateDesktop(out);temp=out/'temp';temp.mkdir();environment=module.clean_environment(package);environment.update(TEMP=str(temp),TMP=str(temp),TMPDIR=str(temp))
cases=[('text','static-text/外观与样式验证.ksp','static-text/显现权限配置示例.ksp'),
       ('bit','bit-button/默认交替位按钮.ksp','bit-button/四种操作配置.ksp'),
       ('word','word-button/七种数据类型.ksp','word-button/七操作七类型矩阵.ksp'),
       ('lamp','lamp/双态位地址.ksp','lamp/多态U32三十二状态.ksp'),
       ('function','function-button/系统功能18项.ksp','function-button/双语按下闪烁与无图.ksp')]
record=dict(sourceCommit='c531f529cccf40e75aacda784c6bc9810ce2d5f9',executable=str(package/'KinSealStudioLite.exe'),executableSha256=hashlib.sha256((package/'KinSealStudioLite.exe').read_bytes()).hexdigest(),runs=[],usbAccessed=False,passed=False)
try:
    for family,default,boundary in cases:
        for role,relative in [('default',default),('boundary',boundary)]:
            project=read(work/'samples'/relative)
            if role=='default':project['pages'][0]['objects']=project['pages'][0]['objects'][:1]
            path=out/(family+'-'+role+'.ksp');write(path,project)
            destination=out/(family+'-'+role+'-package')
            run=desktop.run(family+'-'+role,package/'KinSealStudioLite.exe',['--compile-project',str(path),str(destination)],environment,timeout=60,expected_exit=0)
            run.update(originalSample=relative,originalSampleSha256=hashlib.sha256((work/'samples'/relative).read_bytes()).hexdigest(),inputSha256=hashlib.sha256(path.read_bytes()).hexdigest())
            record['runs'].append(run)
            print(family,role,run['exitCode'],flush=True)
    record['passed']=all(x['passed'] for x in record['runs'])
finally:
    desktop.close();record['privateDesktopClosed']=True;write(out/'run.json',record)
