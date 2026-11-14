# 阶段11最终r2便携验证

本批实际运行程序：`KinSealStudio/dist/KinSealStudioLite-Stage11-release-r2/KinSealStudioLite.exe`。EXE SHA256为 `1bcd29cf1b799096def3d3a820e836b5d683de27ef5a96b23ace4b7ab8783b97`；本轮开始和结束的manifest SHA256均为 `1a726d8a917640e7b7b0423db71382042a34025f94f47f246f4a3af66f811df9`。后续正式提交绑定会只改变manifest身份；本验证对应绑定前完整包快照，不能默认为任意后续程序通过。

使用 `tools/run_stage11_portable.py`，复用阶段1的 `PrivateDesktop` 和 `clean_environment`；主程序全部在新建私有桌面运行，没有切换桌面、前置窗口或全局输入。PATH只保留包和Windows系统目录，清理Qt安装的插件搜索环境，全部TEMP/TMP/TMPDIR及宏临时根位于Z盘。环境中的SDK路径被显式设为不存在的自有路径。模块采样没有发现已安装Qt、MinGW或原厂软件路径；该结果属于开发机上的清洁进程隔离，不冒充已卸载Qt的全新系统。

没有下载、上载、串口或PLC命令。实际MainWindow构造允许已有的只读USB枚举，单独披露，不把它写成设备传输。没有调用外部SDK、安装驱动、更改系统字体或关机。

## 最终轮结果

`run-r2/run.json`记录80次私有桌面进程：79次观察到实际退出码，1次是缺qwindows后的明确Qt诊断及阻塞，10秒后终止该私有进程。最终25项语义检查满足，包内109个文件（包括manifest）前后完整哈希一致。报告状态是“有明确限制的检查完成”，不称80次正常成功，更不是80项设备通过。

- `--release-info`：0，版本0.11.0，安装器未提供、未认证完整复刻；实际字体可用/替代结果保留在JSON。
- `--help-catalog`：0，8章、128条命令，明确`globalEntryCoverageComplete=false`。
- `--capture-help`：从真正MainWindow的`action_H`触发帮助，输出实际文本及820×620 PNG；没有直接用独立伪窗口代替菜单接线。
- 7份随包声明样例全部实际校验、MainWindow加载截图、加载保存、保存后复验及二次保存。7例二次保存均幂等。
- 独立离线模拟真实执行6事件：LB0反转、LW10写5、切换English，并读到独立状态`LB0=true`、`LW10=5`、`language=English`。保存800×480最终帧；`hardwareIo=false`、`hmiBehaviorConfirmed=false`保持不变。
- 在“中文 空格路径/便携程序 r2”真实副本启动版本/帮助并保存到“中文 工程保存.ksp”；副本与源包完整身份先核对一致。

| 随包样例 | 源导出实际退出码 | 设备编译实际退出码 | 加载保存结果 |
|---|---:|---:|---|
| stage10/bit-lamp-local | 20，明确拒绝 | 0，独立verify成功 | 原供字段保留，二次幂等 |
| stage10/word-lamp-local | 20，明确拒绝 | 0，独立verify成功 | 原供字段保留，二次幂等 |
| stage10/function-local | 20，明确拒绝 | 0，独立verify成功 | 原供字段保留，二次幂等 |
| stage1/stage1-single-page | 0，输出.sm | 0，独立verify成功 | 原供字段保留，二次幂等 |
| stage5/ellipse-default | 20，明确拒绝 | 0，独立verify成功 | 原供字段保留，二次幂等 |
| stage7/internal-five-family | 20，明确拒绝 | 0，独立verify成功 | 仅下述精确归一化，二次幂等 |
| stage8/ordinary-two-macros | 0，输出.sm | 19，缺外部SDK | 仅下述精确归一化，二次幂等 |

五项源导出拒绝没有生成半份.sm，宏样例整工程编译拒绝没有发布半包。不能把“样例随包且可编辑”写成“7例全部可原生导出/独立设备编译”。成功的6份设备包均实际执行`--verify-compiled-package`，保留原始wire/文件哈希及程序身份后清理可再生产物；没有因此推定本版本实物T/E。

两份历史样例使用schema12声明但仅含schema9实际字段。重新保存采用最小schema9，并省略空`animations:{}`与空`connections:[]`，恰好3项差分；每例的`*-save-differences.json`完整列出。脚本仅允许这组精确变化，其他丢失/值变更仍失败；没有把原始`suppliedFieldsPreserved=false`改成true。全部控件、宏源码、语言及实际数据通过第二次保存幂等进一步核对。

## 依赖缺失的真实行为

| 场景 | 实际结果 | 解释范围 |
|---|---|---|
| 缺SDK，`--compile-macros` | 26，明确诊断且无输出报告 | 无外部SDK调用；普通编辑继续可用 |
| 缺SDK，`--compile-project`宏样例 | 19，明确拒绝且无半包 | 与宏专用命令的26分别记录 |
| 自有副本删除help | `--help-catalog`为28，`valid=false`与具体缺失提示 | 恢复同版本help；源包未改 |
| 自有副本删除Qt6Widgets.dll | 原始退出码-1073741515，按无符号等于0xC0000135 | Windows加载器失败，应用未进入主逻辑；保留原始码 |
| 自有副本删除qwindows.dll | 明确“Could not find the Qt platform plugin windows”后阻塞，10秒终止自有进程 | 未观察到正常退出；不能称应用正常拒绝返回或完整异常恢复通过 |

`PrivateDesktop.run`的原始`passed`与退出码不改写；runner另外记录无符号退出码、允许的明确拒绝/受控阻塞分类。缺插件那次保留`timeout/error/ownedProcessTerminatedAfterFailure`，单独`guardedBlockedStartup=true`，不虚构退出码。它是已界定的部署限制。

## 证据与只读复核

首轮`run-r1`保留完整真实日志、失败结论和归一化差分，没有覆盖。首轮失败包括runner未归一化有符号NTSTATUS、未区分最小schema空容器变化，以及把Qt阻塞当作正常退出的过严预期；后续只修验收脚本分类和临时文件清理，冻结App/help/build/packager未变。

最终runner SHA256：`92487ddc2a4b04f5750a978e13a2fefc1dd29c8d706cb9809496c1f7c52f7359`。`readonly-review-r2.json`只读复核12项通过，无阻断发现。`visual-review-r2.json`记录实际查看的9张原始PNG：7样例、帮助、模拟；主窗口截图1353×708，右侧页面处于水平滚动视口外，不作为HMI裁剪结论。宏样例有0个画布控件，其空白画布与状态栏0一致，不误判加载失败。

复验命令使用完整SHA与全新输出目录；Python加`-B`避免导入helper时产生字节码：

```text
python -B tools/run_stage11_portable.py --run --package <冻结包目录> --expected-manifest-sha <完整manifest SHA256> --expected-exe-sha <完整EXE SHA256> --output <新的Z盘证据目录> --temp-root <新的Z盘专用临时根>
```

未执行：独立干净系统/VM、普通用户权限、只读安装目录、长路径上限、多DPI/多显示器、系统缺字体、卸载或安装USB驱动、真实设备/PLC、安装升级卸载器和长期性能。原结果不能填补这些验收项。

## 自动清理

最终轮10项自有临时对象实际累计删除125713995字节；TEMP末尾文件为0。首轮同样累计删除125713995字节（含单独`cleanup-after-classification.json`所列690198字节）。两轮合计251427990字节，均为本批新建编译输出和负向副本；中途为独立缺失场景恢复过help/Qt文件，所以这是实际删除量累计，不是净磁盘空间增长。

全部清理只限本批fresh temporary root内核验过的普通子路径；原包、历史包、用户工程和其他代理被拒清理目标不在本脚本范围。保留全部原日志、清单、保存文件及首轮失败，最终证据目录约5.5MB；没有保留大体积负向包副本。
