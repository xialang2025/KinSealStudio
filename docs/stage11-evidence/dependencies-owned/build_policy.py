#!/usr/bin/env python3
"""Generate the small release-facing policy from the retained static inventory."""
#20261114-10 add: Preserve a machine-readable split between identity and permission.
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]


def main():
    """Bind observed components and future missing-dependency cases without granting rights."""
    inventory_path = HERE / 'inventory.json'
    inventory_bytes = inventory_path.read_bytes()
    inv = json.loads(inventory_bytes)
    records = inv['peFiles']
    qt_files = [x for x in records if 'qtSbom' in x]
    def binaries(names):
        return [{k: x[k] for k in ('relativePath', 'bytes', 'sha256', 'architecture')}
                for x in records if x['relativePath'] in names]
    def component(cid, name, delivery, permission, scenarios, **extra):
        return {'id': cid, 'component': name, 'deliveryPolicy': delivery,
                'permissionState': permission, 'scenarios': scenarios, **extra}
    def sources(names):
        return [{'path': n, 'bytes': (ROOT / n).stat().st_size,
                 'sha256': hashlib.sha256((ROOT / n).read_bytes()).hexdigest()} for n in names]
    components = [
        component('APP-OWN', 'KinSealStudioLite 自主开发 C++/工具部分', '内部验证包交付；发布包整体仍含下列受限资源',
                  '仓库未提供整体公开发行许可证；不为用户推定新的开源授权', ['编辑', '保存', '导出', '编译', '模拟', '下载'],
                  publicLicenseChosen=False),
        component('QT-6.12', 'Qt6.12.0 动态库及插件', '随受限便携包复制经哈希冻结的 DLL 和许可文本',
                  '本机文件版本及来源已观察；LGPL/替换/对应源码提供安排待正式发行核实', ['编辑', '保存', '渲染', '编译', '串口', '模拟'],
                  files=binaries([x['relativePath'] for x in qt_files]),
                  observedVersion='6.12.0.0', linkMode='dynamic',
                  localSpdxDeclaredLicense='LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only',
                  sbomHashMatchedCount=sum(x['matchesSbomSha1'] for x in qt_files), sbomCandidateFileCount=len(qt_files),
                  sbomMismatchExplanation='原因未知；有效数字签名不自动解释 SBOM SHA1 差异',
                  obligations=['保留版权、LGPLv3及其引用GPLv3文本和适用第三方声明',
                               '正式发行前为实际Qt二进制提供相应源码/有效提供方式并核实来源',
                               '允许用户替换/调试修改Qt库；动态链接只说明方式，不等于全部义务已满足']),
        component('MINGW-RUNTIME', 'MinGW GCC C++/SEH/winpthreads 运行库', '只复制运行DLL及适用声明，不带g++/构建工具',
                  '本机候选文件SHA256一致；GCC GPLv3+Runtime Exception及MinGW/winpthreads声明已保留', ['所有启动场景'],
                  files=binaries(['libgcc_s_seh-1.dll', 'libstdc++-6.dll', 'libwinpthread-1.dll']),
                  toolchainDirectory='G:/01_Professional/QT6/Installed/Tools/mingw1310_64',
                  runtimeFileVersionNotComplete=True),
        component('D3D-COMPILER', 'Microsoft D3Dcompiler_47.dll', '现有受限包保留已部署文件，不单独另行散发',
                  '微软有效签名/本机Qt候选hash已观察；该具体binary的可再分发依据未定位', ['Qt图形后端按需'],
                  files=binaries(['D3Dcompiler_47.dll']), publicRedistributionCleared=False),
        component('MESA-SW', 'opengl32sw.dll / Mesa llvmpipe及LLVM归属', '受限包保留已部署文件及本机Qt归属文本',
                  '本机Qt候选hash一致；MIT/Boost/LLVM/NCSA等归属文档已找到，具体构建成分仍待核', ['Qt软件OpenGL按需'],
                  files=binaries(['opengl32sw.dll']), publicRedistributionCleared=False),
        component('LZOKAY-MIT', 'lzokay', '编译入应用，随包保留原MIT文本', '源码LICENSE明确MIT', ['原生源格式编解码', '编译'],
                  source='third_party/lzokay/LICENSE', notice='licenses/LZO-MIT-LICENSE.txt',
                  sourceFiles=sources(['third_party/lzokay/lzokay.cpp', 'third_party/lzokay/lzokay.hpp', 'third_party/lzokay/LICENSE'])),
        component('QRCODE-MIT', 'Project Nayuki QR Code generator', '编译入应用，保留源码版权和QR-MIT文本',
                  '源码文件头明确MIT；包已有同源说明', ['二维码渲染'],
                  source='third_party/qrcodegen/qrcodegen.cpp', notice='licenses/QR-MIT-LICENSE.txt',
                  sourceFiles=sources(['third_party/qrcodegen/qrcodegen.cpp', 'third_party/qrcodegen/qrcodegen.hpp', 'third_party/qrcodegen/provenance.json'])),
    ]
    for group in inv['resources']:
        name = Path(group['qrc']['path']).name
        components.append(component('EMBEDDED-' + name.removesuffix('.qrc').upper(), name,
              '当前程序已内嵌，限内部验证；不把无外部文件等同于自主拥有或已获公开分发权',
              '技术来源可追溯；原厂图库/图标/宏例/seed payload或派生元数据分发依据未核',
              ['按该qrc定义加载'], qrcSha256=group['qrc']['sha256'], resourceInputSetSha256=group['inputSetSha256'],
              fileCount=group['fileCount'], sourceBytes=group['totalBytes'], publicRedistributionCleared=False))
    components += [
        component('USB-SYSTEM', 'Windows系统 libusb0.dll / libusb-win32驱动', '外部提供，不复制DLL/驱动，不自动安装',
                  '现有系统端点及库可观察；未核分发条件，外置', ['USB下载', 'USB上载'],
                  systemCandidates=inv['systemUsbCandidates'], loader='GetSystemDirectoryW + /libusb0.dll'),
        component('ARM-SDK', '外部ARM宏工具链/头文件/库/许可文件', '用户在工具中选择自有完整SDK或设置KINSEAL_ARM_SDK，不入便携包',
                  '安装不等于再分发许可；许可证真实性/范围未由程序认证', ['设备宏编译'],
                  sdkRedistributed=False, selectionOrder=['KINSEAL_ARM_SDK', 'QSettings armSdkDirectory', 'applicationDir/arm-sdk'],
                  requiredFiles=['armcc.exe', 'armcc.dll', 'armlink.exe', 'armlink.dll', 'fromelf.exe', 'fromelf.dll',
                                 'Include/macrolib.h', 'Include/glib.h', 'Include/math.h', 'Include/stdio.h', 'Include/stdlib.h',
                                 'script_amzlib.a', 'guilib70_amz.a', 'licenses/LICENSE.DAT', 'Lib/armlib/c_a__un.l']),
        component('HOST-FONTS', '系统字体和用户工程指定字体', '不分发系统/原厂字体；缺字族由系统解析，需人工核实际字形',
                  '字体文件授权未核，不复制', ['文字渲染', '设备字模编译'],
                  fontFilesInBaselinePackage=inv['fontFilesInPackage'],
                  observedRequestedFamilies=['宋体/SimSun', 'Microsoft YaHei', 'Microsoft YaHei UI', 'Arial', 'Times New Roman', 'DigifaceWide', '方正中等线简体'],
                  rendererDiagnostic='requestedFamily/resolvedFamily/exactMatch；不能承诺缺字族像素一致'),
        component('VENDOR-RUNTIME-EXTERNAL', '原厂EXE/PLC DLL/固件/安装包', '不作为应用运行依赖复制；独立设备SDK按上项显式配置',
                  '未核再分发依据，禁止将本盘存在解释为授权', ['原厂对照独立环境'],
                  vendorExecutableRedistributed=False, nativePlcDllRedistributed=False, firmwareRedistributed=False),
        component('WINDOWS-OS', 'Windows系统API/API-set及系统图形/串口组件', '由目标Windows提供，不从开发机复制系统DLL',
                  '系统条件；PE imports只列名称不证明所有目标OS版本均可启动', ['所有场景'],
                  architecture='x86_64', targetOsVersionMustBeVerified=True),
    ]
    missing_cases = [
      ('DEP-01','无Qt安装/无MinGW编译器','清除进程PATH中开发环境路径；包内保留Qt/运行DLL','普通编辑、保存、允许范围渲染/编译/模拟应可用；不要求开发环境'),
      ('DEP-02','缺核心Qt DLL','只在自有测试副本去除Qt6Core.dll','启动失败属于Windows加载器阶段；不能预期App自身友好弹窗；旧工程/包不变'),
      ('DEP-03','缺qwindows平台插件','只在自有测试副本去除platforms/qwindows.dll，隔离QT_PLUGIN_PATH','Windows GUI初始化应失败；不能假称可回退offscreen，Stage9包未带qoffscreen'),
      ('DEP-04','无原厂安装目录','清除进程PATH/SDK环境并使用完整便携包；不改外部安装','普通编辑/内嵌图库/无宏离线编译应不依赖原厂EXE；宏仍按SDK条件失败'),
      ('DEP-05','无USB库/驱动/设备','本机已安装且设备在线；禁止重命名系统DLL/卸载驱动/拔线模拟','从源码确认缺库中文错误；真实缺失分支留外部环境待验，不向在线设备发起试探下载'),
      ('DEP-06','无宏SDK','进程环境KINSEAL_ARM_SDK指向自有不存在目录','宏编译明确拒绝并指出完整SDK；无宏工程不受影响；不修改用户QSettings/真实SDK'),
      ('DEP-07','缺指定字体','使用虚构字族并读取requested/resolved/exactMatch诊断，不卸载字体','保留工程请求字族，可能fallback；不得将可渲染等同于原生像素一致'),
      ('DEP-08','缺外部图库目录','完整程序启动于无原厂图库/工作目录的自有路径','系统图库已内嵌，普通加载无需外部目录；资源本体损坏需另做拒绝案例'),
      ('DEP-09','工程图片/资源损坏','对自有工程副本写入无效PNG，不改内嵌EXE','16MiB/4096边界与无效PNG明确拒绝；结果以实际validate/compile命令为准'),
      ('DEP-10','只读安装目录','只在专用副本用ACL/普通账户或只读介质测试','用户工程/编译输出需指向可写目录；文件只读属性不等同普通账户权限场景'),
    ]
    policy = {
      'format':'KinSealLite-stage11-dependency-policy-v1', 'scope':'阶段11受限内部验证包，非公开完整发行授权',
      'releaseChannel':'restricted_internal_validation', 'publicRedistribution':False,
      'appliesTo':'Stage9 runtime baseline inventory plus unchanged resource inputs; Stage11 final EXE identity is frozen by its own release manifest',
      'baselineExecutableSha256': inv['executable']['sha256'],
      'baselineManifestSha256': inv['stage9Manifest']['sha256'],
      'inventory': {'path':'docs/stage11-evidence/dependencies-owned/inventory.json','sha256':hashlib.sha256(inventory_bytes).hexdigest()},
      'components': components,
      'licenseFiles':[{'path':'licenses/' + Path(x['retained']['path']).name,'bytes':x['retained']['bytes'],
                       'sha256':x['retained']['sha256'],'localSourcePath':x['source']['path']}
                      for x in inv['localLicenseTexts'] if x.get('sameBytes')],
      'missingDependencyCases':[{'id':cid,'scenario':name,'safeMethod':method,'expected':expected,
                                 'executionStatus':'not_run_by_static_audit','actualEvidence':'由本轮部署测试回执单独记录'}
                                for cid,name,method,expected in missing_cases],
      'remainingBeforePublicDistribution':[
        '取得或替换内嵌原厂图库/图标/宏例/模板payload等资源的可再分发依据；重编并复验受影响功能',
        '核实Qt对应来源及SBOM签名前后hash差异，完成实际二进制对应源码/许可及替换使用安排',
        '核实D3Dcompiler_47和opengl32sw具体版本构建的适用再分发条件',
        '明确本项目自有代码整体公开许可证/用户分发政策，不能由MIT依赖反推整个程序MIT',
        '在真实目标Windows/普通用户/缺驱动/缺字体条件完成尚未执行用例'],
      'notClaims':['本清单不是法律授权文书','本机有效签名不代表再分发授权','静态imports不等于完整运行时模块跟踪',
                   '便携验证不能升级为设备E通过','内部验证标识不消除第三方许可义务'],
    }
    target = ROOT / 'release/stage11-dependencies.json'
    target.parent.mkdir(exist_ok=True)
    target.write_text(json.dumps(policy,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'path':target.as_posix(),'components':len(components),'licenseFiles':len(policy['licenseFiles']),
                      'sha256':hashlib.sha256(target.read_bytes()).hexdigest()},ensure_ascii=False))


if __name__ == '__main__':
    main()
