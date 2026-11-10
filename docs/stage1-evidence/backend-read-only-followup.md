# 阶段1六项修复的独立只读复核

日期：2026-11-10。修改标记：`//20261110-5`。这是[初次报告](backend-read-only-review.md)的后续记录；保留原发现，不覆盖初检结果。

六项修复的源码控制流复核通过。初次复核阶段仅检查源码并引用主代理日志；本代理没有修改业务代码或构建。冻结后追加的独立Windows运行见末节，历史阶段的未运行记录予以保留。源码SHA及分层运行事实见[JSON](backend-read-only-followup.json)。

| 原发现 | 修复复核 | 定位 | 已有/待补运行范围 |
|---|---|---|---|
| R-S01-BACKEND-001 | 旧单文本入口先检查snapshot默认指纹及地址编译白名单，并拒绝模板不能承载的连接/宏/图片。 | `src/independent_downloader.cpp:585` | legacyEntriesRejectUnrepresentedConfiguration |
| R-S01-BACKEND-002 | 固定源导出先拒绝多语言/连接/资源；写文件前将非空snapshot与完成文字patch的模板逐字节比较，非一致即拒绝。 | `src/native_test_project.cpp:119` | 新增非默认snapshot反例等待主代理实际日志 |
| R-S01-BACKEND-003 | 连接UI使用整个工程候选验证，显式稳定引用与隐藏页地址都参与；失败前没有提交connections或动态属性。 | `src/static_text_actions.cpp:337` | 完整连接UI删除引用刺激尚未在r3日志独立执行 |
| R-S01-BACKEND-004 | 对象和宏/图片进入同一候选；编辑和四类创建都调用helper，验证失败不修改模型或画布。 | `src/static_text_actions.cpp:20` | resourceDraftFailurePreservesEditorState |
| R-S01-BACKEND-005 | 迁移CLI在load/save前强制.ksp目的格式，.sm原位覆盖被拒绝。 | `src/main.cpp:305` | 冻结便携后CLI哨兵文件测试 |
| R-S01-BACKEND-006 | 能力查询加入纯离线prepare，阻塞连接明确返回原因，数量与对象类型原因单独传到设备分支。 | `src/project_capabilities.cpp:72` | capabilitiesExplainKnownBlockedRoutes |

主代理r3记录：
- [backend-tests-r3.txt](backend-tests-r3.txt)：`Totals: 11 passed, 0 failed, 0 skipped, 0 blacklisted, 417ms`。
- [model-tests-r3.txt](model-tests-r3.txt)：`Totals: 53 passed, 0 failed, 0 skipped, 0 blacklisted, 237ms`。

002的snapshot检查位于输出文件open之前，比对的是已完成四字符替换的整个模板；精确TEST原件可保持往返，任何其他snapshot不再被该旧入口静默替换。此前补充消息未截取到这段代码，本轮已纠正该疑点。

准备好的[后台便携探针](gui-portable/probe.py)需要显式`--run`、冻结EXE的SHA和新输出目录；当前仅语法检查，没有启动程序。主代理冻结后再检查四份stage1样例、启动语言、源往返设备数据、拒绝输出保护及Windows私有桌面截图。QtTest桥接作为独立测试程序记录身份，不冒称交付EXE。

GUI/运行限制继续保留：r3资源测试通过helper验证编辑/创建失败模型不变；没有把它描述为逐个操作图库对话框。连接UI整条删除引用路径及新snapshot反例，按后续实际日志追加。没有USB/PLC通信或现场显示、触控结论。

## r5集成回归修复的追加复核

主代理报告完整r5检查发现语言重建丢选择及旧单文本English注册回归，并进行了修复。本代理随后只读核对：`configureLanguages`在重建画布前保存所选ID，重建后按ID恢复选择；旧单文本编译对非默认单语言或已有原生snapshot使用typed源导出保全源内容，固定原厂测试源仍只接受默认中文及已核模板范围。没有撤销多页、连接或配置的明确拒绝。

后续必须使用最终`KinSealStaticTextTests.exe`在私有Windows桌面跑完整测试；offscreen的字体证据跳过不能算验收通过。准备脚本要求`nativeFontAndFillEvidence`明确PASS、失败数和跳过数均为0。最终便携EXE与独立测试EXE/Qt6Test依赖分别记录SHA；当前仍等待主代理冻结通知。

## r10旧空宏表迁移的追加复核

真实阶段0基线曾把空宏映射写为`[]`。主代理新增的兼容分支只接受旧schema、`macros`键及完全空数组；先前的schema3能力门槛仍生效，实际可迁移区间为schema3..8。非空数组、null、其他资源数组与schema9空数组继续拒绝。空数组经局部解析得到空映射，保存为规范`{}`，没有宏源码可被丢弃。

新增`actualStage0EmptyMacroArrayMigrates`使用现存12控件阶段0样例作正例，并验证非空旧数组及schema9空数组失败时输出工程不变。本代理只检查了上述实现与测试，未把尚在运行的r10构建记为通过。

## 最终冻结Windows运行的追加复核

最终便携EXE `a5859fbb97238f23873240256f5e09427de2da37009d1120276d6f46cb44615f` 经本代理独立私有桌面、干净PATH验证，运行前后SHA不变。r2的29项进程检查、13项语义检查全部通过；完整Windows StaticTextTests **43通过、0失败、0跳过**，`nativeFontAndFillEvidence` 明确PASS；MainWindow双页/资源草稿桥接4通过、0失败、0跳过。详见[最终GUI报告](gui-portable/README.md)及[完整运行结果](gui-portable/run-r10-windows-2/result.json)。

这次独立执行补充004资源失败事务helper、005迁移.sm保留哨兵、四份样例能力路由的运行证据；未把静态复核等同于逐个GUI路径运行。最终阶段0样例空宏表迁移保留十二控件，双页提供字段保全且二次迁移幂等。r1两处图库失败来自证据目录缺失，及raw JSON默认展开误报，均保留原记录和逐项诊断；r2只修独立harness后重跑同一冻结位，没有更改生产或测试EXE。

专用桌面所有目标进程已退出、句柄已关闭；本代理无USB/PLC传输或现场显示触控结论。六项原发现的完整主代理后端回归日志与本代理上述独立验证分别记载，不互相替代。
