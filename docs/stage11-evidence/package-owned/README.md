# 阶段11白名单打包工具

本目录记录 `tools/package_stage11.py` 的测试、只读复核和工具边界。生产程序的实际构建、最终包创建与私有桌面启动由主任务单独记录；单元测试使用微型合成文件，不冒充真实 EXE 或触摸屏验证。

## 调用方式

```powershell
python -B tools/package_stage11.py create `
  --worktree <当前安全工作树> `
  --executable <工作树>/build/KinSealStudioLite.exe `
  --expected-exe-sha <本次实际新EXE的SHA256> `
  --runtime <已冻结Stage9-plc-r2目录> `
  --runtime-manifest-sha 2b6e2a668fa84915378bb082f4fbc75e11a70f0d508fbd9d2fb92a202e2f10da `
  --help-dir <工作树>/help `
  --licenses <工作树>/docs/stage11-evidence/dependencies-owned/licenses `
  --dependencies <工作树>/release/stage11-dependencies.json `
  --guide <工作树>/release/使用说明.md `
  --package <全新Z盘包目录> `
  --evidence <全新Z盘证据目录>

python -B tools/package_stage11.py verify --package <包目录> `
  --expected-manifest-sha <从独立交付证据取得的SHA256> --report <全新Z盘报告.json>

python -B tools/package_stage11.py compare --left <第一次包> --right <第二次包>

python -B tools/package_stage11.py bind-commit --package <包目录> `
  --worktree <原构建工作树> --commit <真实40位Git提交ID> --evidence <全新Z盘绑定证据目录>
```

`--help-dir` 是帮助目录参数；`--help` 保留给命令行自身帮助。工具依赖 Python 标准库。`bind-commit` 另外需要 Git，只读取既有提交，不暂存、提交、切换或推送仓库。

## 用户包白名单

1. 本次工作树 `build/` 内的新 `KinSealStudioLite.exe`；调用者必须显式传入期望 SHA256。
2. 固定22个 DLL 与2个继承 MIT 许可文件，逐条比对旧包已冻结 manifest 的长度及 SHA256。工具不会遍历复制旧包其余内容，更不会使用旧 EXE。
3. 新写 `help/` 中限定扩展名的文件、完整64份许可及依赖清单、用户说明、打包工具自身。
4. 默认7份代表 Lite 工程：阶段10位、字、功能三页；阶段1基础单页；阶段5椭圆；阶段7内部五类页；阶段8普通双宏页。双宏编译仍需外部 SDK，不能因进入包内而认定设备执行已验证。

支持重复 `--sample samples/<路径>.ksp` 显式选定其他 Lite JSON 样例；拒绝路径穿越、非 Lite 格式、含 opaque/snapshot/nativeSource/nativePayload 元数据的项目。默认7份实际不存在这些字段。

生产源码、资源 `.sm`、qrc 与图标等只作为 sourceFiles 哈希清单绑定，不复制到包。该清单还包含 `Build.ps1`、`tools/BuildStage11.ps1`、版本 RC、帮助正文/命令映射与本工具自身。调查图片、完整 docs、原厂 EXE、外部 SDK、固件、编译中间物均不进入复制白名单。应用 EXE 本身仍含既有内嵌资源，其分发依据未知必须保留。

## 安全与身份门禁

- 包和证据目录必须为 Z 盘全新路径；不覆盖旧包、不删除输入；失败保留本次部分输出和 `failure.json`。
- 检查每一级已存在父目录的 symlink/reparse 属性，拒绝任何输入文件/包文件 `st_nlink != 1`。复制使用新文件字节流，输出逐项重新计算哈希。
- 采用 Windows 大小写无关的包含比较，拒绝输入输出树交叠；manifest 路径禁止绝对路径、反斜线、`..`、ADS、尾点/尾空格、设备保留名、大小写重名。
- JSON 限16MiB、身份清单限25000项；拒绝重复 JSON 键、NaN、Infinity、`1e999` 等非有限数。有限小数可用于样例坐标。
- 依赖清单必须明确 `restricted_internal_validation`、`publicRedistribution=false`，且绑定本次旧 runtime manifest；清单 `licenseFiles` 与真实许可目录必须逐项完全一致。
- source 与包清单顺序确定；清单 hash 包含 path、bytes、SHA256。实际执行 UTC/绝对输入路径只进入包外 evidence，不污染确定性包内容。
- manifest 无法包含自身 SHA256；包外 `run.json` 保存其实际 SHA256。`verify --expected-manifest-sha` 可同时发现 manifest 与 payload 被协调替换。没有独立期望哈希时，`verify` 验证的是当前 manifest 与当前内容的自洽性，不是数字签名或可信发布者证明。
- `verify` 核对全文件集合、缺失、额外、篡改、角色白名单和完整 runtime/帮助/许可/样例/工具/说明/依赖必备项。正常但内容不符合返回3，非法输入或安全门禁拒绝返回2，全部通过返回0。
- `bind-commit` 按冻结文件逐条比对当前源和真实 Git blob，文本只允许 Git CRLF/LF 转换；保留旧 manifest 后仅原子替换 manifest 中的提交绑定，包 payload hash 不改变。

## 测试与复核说明

第一轮28项测试复现了 `argparse --help` 参数冲突，随后改为 `--help-dir` 并通过。Windows 创建 symlink 缺少权限，测试回退到实际目录 junction，确认同一 reparse 属性门禁拒绝其父级路径，不将该项跳过。后续只读复核增加许可清单的语义绑定，并新增对应反例。最终计数、脚本身份和实际日志以本目录机器报告为准。

测试临时文件全部位于独占 `Z:/Project/vscode_prj_temp/REA/.stage11-run-20261114/package-temp/`；`TemporaryDirectory` 只移除每次自建且范围固定的合成测试根。真实 runtime、主工作区用户文件和历史拒绝清理路径均未删除。

本工具不制作安装器、不操作 USB/PLC、不执行 SDK 编译、不声称公开发行授权、不证明最终 EXE 功能或实屏兼容；这些验收应使用主任务的独立证据。
