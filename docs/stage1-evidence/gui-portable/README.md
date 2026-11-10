# 阶段1冻结便携与Windows后台GUI验收

日期：2026-11-10。修改标记：`//20261110-5`。

最终r2通过：29项进程检查、13项语义检查全部通过；完整Windows `KinSealStaticTextTests` 为 **43通过、0失败、0跳过**，`nativeFontAndFillEvidence` 明确PASS。实际MainWindow页面/事务桥接为4通过、0失败、0跳过（含初始化与清理）。本代理未改生产代码、未构建、未执行USB/PLC传输。

- [完整结果](run-r10-windows-2/result.json)、[Windows完整静态/控件测试](run-r10-windows-2/static-text-windows.txt)、[MainWindow桥接](run-r10-windows-2/private-gui-regressions.txt)。
- [结构化验收摘要](final-summary.json)、[探针源](probe.py)、[准备与执行状态](prepared-plan.json)。
- 冻结便携EXE：`dist/KinSealStudioLite-Stage1/KinSealStudioLite.exe`，运行前后SHA256均为 `a5859fbb97238f23873240256f5e09427de2da37009d1120276d6f46cb44615f`。
- BackendTests SHA256：`6396d8a8bfc7f1f5edbc28adce0de9bb625ed0a0b1def6e11aa3fc64e8f06de8`；StaticTextTests SHA256：`2b2db8e9fe896e52286db4aba15fef0fc80e66ee34c49073923b203592e004f6`。

全部目标进程通过`CreateDesktopW`和`STARTUPINFO.lpDesktop`置于专用Windows桌面，未切换桌面、未调用前置窗口、未使用系统键鼠。主程序PATH只有Windows/System32和Windows；测试程序PATH额外含冻结便携目录。真实`qwindows.dll`、Qt6Core/Gui/Svg/Widgets均观测到从冻结包加载，没有观测到安装版Qt依赖。模块采样不是全时追踪，快速进程可能已退出而未枚举全部模块。测试另用现有Qt6Test.dll，其身份单独记录且不是便携产品依赖。

[主窗体截图](run-r10-windows-2/portable-main-window.png)已实际打开检查：1376×736，中文树、工具箱、菜单和800×480蓝色空画布清晰，没有明显裁切或异常。这是便携EXE空工程截图，不宣称GUI已从磁盘打开两份样例；真实MainWindow桥接验证了双页切换、隐藏页对象保存及失败资源草稿不修改编辑状态。

r2同时验证四份打包阶段1样例与源文件SHA一致；真实阶段0十二控件/macros=[]样例迁移为规范空对象；单页和语言重排编译后启动语言索引分别为1与0；单页源往返后的ControlPara/ControlPixel/PicLenPara/AddrPara四段文件SHA完全一致。多页、未实现配置及旧入口明确拒绝且没有生成编译输出目录；向.sm迁移返回21，原有哨兵文件SHA不变。

## 保留的r1失败和修正依据

[r1原结果](run-r10-windows-1/result.json)原样保留。完整静态测试当时41通过、2失败、0跳过，两个失败来自测试harness缺失`executableDir/../artifacts`，导致图库PNG保存失败。r2仅创建该证据目录，未改产品、未改测试EXE、未关闭或跳过测试，最终43项全部通过。

r1原先把双页迁移前后raw JSON完全相等作为判定，遇到24处已声明默认字段展开。[逐项差异清单](run-r10-windows-1/two-page-normalization.json)证明没有删除或改变源字段：两个文本对象分别补充`group=""`及11个休眠权限默认字段。r2保留每个已提供源字段和有序列表内容，严格比较projectId/pageOrder/activePageId/startupPageId/startupLanguage/references，并要求第二次完整CLI迁移JSON完全幂等；全部通过。没有以未经检查的“可选字段”理由放过差异。

17:02:10+08:00的CIM检查未发现本任务三种目标进程。两轮专用桌面句柄均关闭，所有目标退出；已向主代理报告可开始其独立USB工作。本代理随后不再启动GUI程序。主程序可能只读枚举USB存在性，不能把此描述为USB下载验收。

仅删除两轮harness中六个已核SHA的可再生EXE/Qt6Test副本，释放181097928字节；[清理回执](harness-cleanup.json)保留完整路径、大小和SHA。所有日志、项目、设备段、PNG、原build及交付便携均保留。

当前证据不覆盖本代理未执行的实机显示、触控、USB/PLC传输，亦不将多页和新激活配置记为可下载。连接删除整条对话框路径与逐个图库选择事务未在本轮单独刺激。
