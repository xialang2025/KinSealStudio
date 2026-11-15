# 固件窄范围分析复现记录

本轮工具为已经安装的 Windows Ghidra 12.1.4 PUBLIC（build 20260921）、Temurin OpenJDK 21.0.12.1+1、Python 3.11.2；Capstone 版本见 `raw-evidence.json`。没有安装软件，没有连接设备，没有执行固件。

源文件绝对路径及完整SHA在 `memory-map.json`。**原始固件和可再生内存影像不入Git**；复现须由现有授权安装路径读取原件。以下输出目录与项目目录都是本次专用空间。项目父目录不能有以点开头的路径段，初试失败已记录于 `ghidra-r1.log`。

## 所有headless运行的环境

```powershell
$env:TEMP='Z:/Project/vscode_prj_temp/REA/.bitbutton-rea-20261115/firmware/temp'
$env:TMP=$env:TEMP
$env:JAVA_HOME='G:/01_Professional/Java/jdk-21.0.12.1+1'
$env:GHIDRA_HEADLESS_MAXMEM='2G'
$env:GHIDRA_HEADLESS_JAVA_OPTIONS='-Djava.io.tmpdir=Z:/Project/vscode_prj_temp/REA/.bitbutton-rea-20261115/firmware/temp -Dapplication.settingsdir=Z:/Project/vscode_prj_temp/REA/.bitbutton-rea-20261115/firmware/temp/settings -Dapplication.cachedir=Z:/Project/vscode_prj_temp/REA/.bitbutton-rea-20261115/firmware/temp/cache -Dapplication.tempdir=Z:/Project/vscode_prj_temp/REA/.bitbutton-rea-20261115/firmware/temp'
```

初次失败运行读了现有C盘preferences，日志缓存清扫报告0字节。成功导入与后续运行均显式指向Z盘专属temp/settings/cache。

## 数据重建与首次导入

以下为从实际日志和保留脚本恢复的可重放必要参数；完整执行结果见 `ghidra-r2.log`。重放前自行确认两个专用目录不存在或为空；不得覆盖他人的同名项目。

```powershell
& 'C:/Users/lenovo/AppData/Local/Programs/Python/Python311/python.exe' -B 'Z:/Project/vscode_prj_temp/REA/.bitbutton-rea-20261115/firmware/scripts/reconstruct_firmware.py'
& 'G:/01_Professional/Ghidra/support/analyzeHeadless.bat' 'Z:/Project/vscode_prj_temp/REA/bitbutton-arm-ghidra-20261115' 'bitbutton-arm' -import 'Z:/Project/vscode_prj_temp/REA/.bitbutton-rea-20261115/firmware/temp/firmware-memory.bin' -loader BinaryLoader -loader-baseAddr 0x8001ffe0 -processor ARM:LE:32:v5t -cspec default -scriptPath 'Z:/Project/vscode_prj_temp/REA/.bitbutton-rea-20261115/firmware/scripts' -preScript FirmwareSetup.java -postScript FirmwareSurvey.java 'Z:/Project/vscode_prj_temp/REA/.bitbutton-rea-20261115/firmware/survey.json' -max-cpu 2 -log 'Z:/Project/vscode_prj_temp/REA/.bitbutton-rea-20261115/firmware/ghidra-r2.log' -scriptlog 'Z:/Project/vscode_prj_temp/REA/.bitbutton-rea-20261115/firmware/script-r2.log'
```

选择v5t是分析配置，不是CPU硬件型号鉴定。Ghidra自动分析58秒，函数边界和flags伪代码有错误；必须核原始ARM控制流。

## 后续导出实际调用记录

下面是最后一次实际执行的完整headless命令；此前各轮使用相同 `-process/-noanalysis/-scriptPath/-max-cpu` 参数，替换输出文件、地址选择器和日志序号。各日志同时保留实际 `Execute script` 参数。

```powershell
& 'G:/01_Professional/Ghidra/support/analyzeHeadless.bat' 'Z:/Project/vscode_prj_temp/REA/bitbutton-arm-ghidra-20261115' 'bitbutton-arm' -process firmware-memory.bin -noanalysis -scriptPath 'Z:/Project/vscode_prj_temp/REA/.bitbutton-rea-20261115/firmware/scripts' -postScript FirmwareDetails.java 'Z:/Project/vscode_prj_temp/REA/.bitbutton-rea-20261115/firmware/details-r5.json' 8003A670 '@80100285' -max-cpu 2 -log 'Z:/Project/vscode_prj_temp/REA/.bitbutton-rea-20261115/firmware/ghidra-r7.log' -scriptlog 'Z:/Project/vscode_prj_temp/REA/.bitbutton-rea-20261115/firmware/script-r7.log'
& 'C:/Users/lenovo/AppData/Local/Programs/Python/Python311/python.exe' -B 'Z:/Project/vscode_prj_temp/REA/.bitbutton-rea-20261115/firmware/scripts/export_raw_evidence.py'
```

| 产物 | 执行日志 | 选择器/附加脚本 |
|---|---|---|
| details-r1.json | ghidra-r3.log | 8007f13c 80054ed0 8005434c 8002b038 8002af44 8002ae44 8002ad18 80074830 80060e5c 80040820 8007c1e4 8005fe94 8008c0d0 |
| details-r2.json | ghidra-r4.log | 80057258 800567b0 80056808 80080da8 8004b014 800a36fc @80100270 @80100271 @80100272 @80100276 @8010043c @801004c0 |
| details-r3.json | ghidra-r5.log | 先运行 FirmwareSeedExtra.java；800a3984 800a3c58 800a3efc 800a32ac 80056840 80082248 80057074 8008c294 8007c0dc 80061f3c @80082248 |
| details-r4.json | ghidra-r6.log | 80079B14 8009A224 8009F594 8008A100 8002A034 @801882C0 @800FFFA0 @8007F5F4 @8007F5F8 |
| details-r5.json | ghidra-r7.log | 8003A670 @80100285 |

## 归档白名单与清理

归档白名单仅包含本目录顶层 `memory-map.json`、`survey.json`、`details-r1.json` 至 `details-r5.json`、`raw-evidence.json`、`findings.md`、本记录、`cleanup.json`、`archive-manifest.json`，以及 `scripts/` 下2个Python、4个Java脚本、`ghidra-r1.log` 至 `ghidra-r7.log` 和 `script-r1.log` 至 `script-r7.log`。`archive-manifest.json` 记录除其自身之外每个文件的哈希与大小。

清理目标仅为已核所有者的 `Z:/Project/vscode_prj_temp/REA/bitbutton-arm-ghidra-20261115/` 和本目录 `temp/`。结束时只读核验了精确绝对目标、所有者标记、无reparse项、无运行中的本轮headless进程。自动批准审查先拒绝了复合清理命令，又拒绝了只删除精确项目字面路径的单个PowerShell命令；两次均只返回 `blocked by policy`，没有具体原因。**0字节实际删除，两个目录保持原处，不再重试；共35,908,325字节不入归档。** `cleanup.json` 保留拒绝及原件最终SHA核验记录。
