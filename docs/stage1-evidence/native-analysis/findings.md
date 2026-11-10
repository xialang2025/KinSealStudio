# 阶段1原厂源、普通页与AMT型号调查记录

日期：2026-11-10。修改标记：`//20261110-5`。本目录归 `/root/stage0_entries` 本次调查所有；本文件记录已采集事实和待验边界，不表示整个阶段1已经完成。

## 1. 证据来源与运行边界

采用普通 Python、Frida 和 Win32/Qt4 操作，不是新生成的 REA Evidence。原安装程序位于 `G:\01_Professional\触摸屏\金玺\KinSealStudio_Setup_V5.1\Installed\KinSealStudio5.1.exe`，SHA256 为 `305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c`。每次运行只复制原 EXE、两个型号文件、语言配置和所需源文件；私有 EXE 仅在 `0x7c4..0x7c6` 把 `5589e5` 改为 `b001c3`，其 SHA256 为 `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89`。应称“原厂程序的已记录私有补丁副本”，不能称未修改原件运行。

所有进程位于私有桌面 `KinSealStage1Native20261110`，没有切换桌面、前置窗口、全局键鼠输入、USB/PLC访问或原厂编译。Qt getter、setter 与实际动作调用均在目标 GUI 线程执行；通过目标进程的消息派发钩子排队。控件表选择使用仅发往所拥有 viewport 的后台窗口消息，不调用 `QGraphicsItem::setSelected` 冒充用户选择。

完整身份、输入/输出哈希、每次 run/PID、成功/失败状态和重开关系见 [manifest.json](manifest.json)。最终可复核采集脚本为 [native_probe.py](native_probe.py)；manifest 中脚本哈希对应最终版本，不能倒称早期所有 run 使用完全相同脚本。滚动 `native-probe.json`、checkpoint 只用于观察最新进度，正式引用使用带 run 名的 JSON。

## 2. 原厂普通页的最小源样本

| 样本 | 实际原厂操作 | 完整 typed 遍历 | 独立原厂重开 |
|---|---|---|---|
| `A-empty-one-page.sm` | 打开原厂 Probe，正常全选、删除、保存 | 1页，完整EOF | `empty-a-reopen-r1`，PID9344 |
| `B-two-empty-pages.sm` | 在A所在进程，新建普通页，默认确认、保存 | 2页，完整EOF | r6/PID20468和r7/PID10888作为输入打开，之后分别生成C/D |
| `C-third-page-number9.sm` | B重开后新建第3页，编号 spinBox 实际填9，默认页名 | 3页，完整EOF | r8/PID17440，逐次下一页到第3页，再下一页停留 |
| `D-third-page-name.sm` | B重开后新建第3页，实际页名 `Stage1_Third`，默认编号2 | 3页，完整EOF | `named-d-reopen-r1`，PID18800，下一页两次看到 `Stage1_Third` |

字段值依据 source worker 的 C++ 完整 typed 结构遍历再按 span 解码，汇总在 [source-multipage-boundaries.json](../source-multipage-boundaries.json)，本目录保留各样本的 `*-fields.json` 和工具结果。不是扫描偶然字符串，也不是把旧单页结构固定扩成3份。

- `page.head.18` 随实际页名改变。
- `page.head.19` 是 QString 页编号：A的第一页 `000`，B第二页 `001`，C第三页 `009`，D第三页 `002`。
- `trailer.page-map` 在B为 `(1,1),(0,0)`，在C/D为 `(2,2),(1,1),(0,0)`。C的界面编号9没有把对应pair变成9。因此仅确认这组序列及编号字段彼此不同，不把pair解释成原生页编号，也不推断稳定引用的完整含义。
- 新页对话框为 `QTreeCtrlNewScreenDlg`，实际页名框 `newscreen_name_lineedit`，编号 `spinBox` 最小0、最大999、第二页默认1，正常确定为 `pushButton_sure`。
- `comboBox_DisModel` 包含横向、竖向、横向180、竖向180四项，但在本次AMT窗口隐藏。该观察不能作为AMT已支持四方向编译的依据。

多页源已经有结构边界和完整EOF正例；多页的语义可编辑导入/导出与设备编译仍须分别验收。页面改名/删除/复制后的引用变化、普通页与窗口/键盘/Logo角色、启动页映射，本次没有足够原厂差分，不填写推测协议。

## 3. 缺失源回归夹具的原厂补证

从已有真实原厂最小源出发，实际属性界面保存了9个新样本，已交 source worker 完整 typed/EOF 复核并冻结。它们是新采集夹具，不宣称恢复了旧 `.scratch` 中已缺失的历史文件。

| 新样本 | GUI字段/路径 | run |
|---|---|---|
| `bit-operation-{set,reset,jog,toggle}-native-ui.sm` | 单bit正常全选→属性，`comboBox_operateType` 分别0/1/2/3→正常确定→保存 | `bit-operations-r2`，PID492 |
| `bit-notice-word7-native-ui.sm` | 权限页启用 `groupBox_Notice` 并调用正常勾选联动，`checkBox_NoteW` 点击，`LW0`，值7→确定保存 | `bit-notice-r2`，PID304 |
| `bit-external-{U32,I32,F32}-native-ui.sm` | Touch/TouchByW/touchlevel 启用，用户等级1；dataT分别1/3/4，地址 `[Modbus_RTU]4x0`，比较值7 | `bit-external-r1`，PID17444 |
| `static-visible-level3-native-ui.sm` | 静态文本最后权限页，GroupV及showlevel启用，ShowUserLevel索引2 | `static-level-r1`，PID17312 |

monitor-disabled 的原厂组操作失败，`bit-monitor-r1` 原始日志保留，没有生成或声明正例。source worker 采用已知 typed span 注入休眠脏值的测试属于人工负向样本，应单独记录来源，不能改称原厂 UI 样本。

## 4. 产品源经过原厂writer再保存与重开

输入位于 `../source-inputs/`，由产品导出，不是原厂空白创建的源。成功路径都打开任务工作副本，在原厂真实单bit属性把操作改一档再恢复，正常确定后正常保存；仅把实际写出的工作源复制到本目录。这个来源分类必须随夹具保留。

| 输出 | writer / 独立重开 | 可采用的结论 |
|---|---|---|
| `two-language-distinct-native-resaved.sm` | `bilingual-resave-r1`/19392；`bilingual-reopen-r1`/21800 | 原厂可打开和保存该产品双语单bit源；typed/EOF及语义解码通过，双语内容由source worker核对 |
| `mixed-two-language-distinct-native-resaved.sm` | `mixed-list-resave-r3`/23656；`mixed-bilingual-reopen-r1`/15300 | 控件一览表实际8行，选择第4行并按表确定后打开属性，确认是bit才改回保存；typed/EOF及语义解码通过 |
| `two-language-startup-english-native-resaved.sm` | `english-startup-resave-r1`/9356；`english-startup-reopen-r1`/19728 | 原厂能重开，但保存后两处语言字段不一致，是保留的配置冲突负例 |
| `two-language-startup-english-ui-native-resaved.sm` | `english-startup-resave-r2`/20956；`english-startup-reopen-r2`/2008 | 先实际把编辑器语言combo切到English1，再属性改回保存；typed/EOF及语义解码通过，区别于r1负例 |
| `macro-bound-native-resaved.sm` | `macro-bound-resave-r1`/21824；`macro-bound-reopen-r1`/1764 | 原厂能读写此源；完整typed通过，语义导入因未核实宏入口/扩展/云端信息而拒绝；不是宏功能支持正例 |

英文r1保存后，`system.identity.16`（`s`，字节[333,335)）为 `0000`，`config.290.0`（`i`，字节[2538,2542)）为 `00000001`。实际编辑器语言切English再保存的r2中，两者分别为 `0001` 与 `00000001`。本次只直接证明这组字段变化和原厂保存结果，不能由此把编辑态与设备启动态的全部语义合并。现有导入一致性守卫保持，r1不通过不得靠修字节或放宽整块校验绕开。

宏属性初始 `groupBox_macro.checked=true`，但 `enabled=0`；`comboBox_macroIndex` 同样禁用、`currentIndex=-1`、`count=0`、`items=[]`。原厂没有在该实际列表中识别 `SetLB1`，没有宏编译、可执行缓存或下载证据。仅凭原厂能保存源文件，不能判断MACRO_MAIN绑定可用。

控件表的早期 queued `cellDoubleClicked` 调用返回false，直接背景双击也没有打开属性，均按失败保留。成功r3是表正常选择后按“确定”，回主窗口再开属性；没有对8对象全选后批量改值。

原厂“另存为”的6次有界调查均未得到目标输出。常见文件对话框真实出现，但字段设置/确认的路径未闭合；最终预填钩子也未得到实际SetFileName确认。没有接受覆盖确认，没有把它们计入成功writer样本，详见 `native-probe-bilingual-saveas-r*.json`。

## 5. AMT型号文件的静态事实

精确原件SHA、目标XML元素字节范围、摘录SHA和全部10+51原始字段见 [model-metadata.md](model-metadata.md)、[model-metadata.json](model-metadata.json)。`model.dat`（25936B）和`modelInfo.dat`（161214B）都是带UTF-8 BOM的XML，不是未知二进制容器。

针对 `AMT070W124-L800X480R`，静态声明包含 Seris=AMT、Pixels=800*480、COM1/COM2/USB=true；modelInfo包含 Device=76、USB=2、SerialPortDown=false、maxScreen=200、maxControl=100、MfbStates/MonitorStates=32。容量/地址原文如 `20000`、`300000`、`1E0000`、`FD84` 均保留原文；消费者的转换进制、单位和实际边界尚未确认。Device76不解释成USB VID/PID。磁盘 `fbottomFile/cbottomFile` 不等于连接屏的固件身份。

页编号0..999与配置的页面数量上限200是两个不同事实。型号XML的帮助/配置字段、UI实际可用性、编译后端能力和设备固件事实仍分层记录，不通过静态值开放未测能力。

## 6. 本次未闭合项与清理状态

尚需原厂证据：页面稳定引用及删除/复制/改名事务、完整启动配置UI、语言列表编辑与删除、各配置消费者、窗口/键盘/Logo角色、型号容量进制与单位、连接固件身份、宏名称列表和绑定、各新多页/配置对应设备编译。与这些项有关的产品功能继续保留显式拒绝或待验状态。

本次子任务没有原厂离线编译或USB/PLC操作；父任务的设备事实应引用其独立证据，不归入本目录的运行结论。

36次保留run的任务PID均已结束，当前所拥有runtime路径没有活动进程。最早r1/r2/r3日志没有内嵌run键，manifest明确标注其run名由保留文件名取得；不把早期失败/未采用结果合并进后续正例。清理只记账：181个文件、679499152B留在 `.scratch/kinseal-stage1-20261110/native/`。此前删除被自动审批拒绝，未重试或改用其他删除通道，未清理历史阶段0/S02目录。完整文件身份和剩余字节见 [cleanup.json](cleanup.json)。这些残留不影响“所拥有进程已结束”的结论，但不能报告磁盘清理完成。

按主代理追加要求，已生成 [只读清理计划](cleanup-plan.md) 和 [完整绝对路径/哈希清单](cleanup-plan.json)。仅盘点本阶段runtime-*直接子文件中的4个指定运行文件名称，共144份，其中140份是相同名称+SHA重复候选，假设可释放660440970B；保留4份唯一运行内容及全部正式源/日志证据。因为先前policy拒绝的完整目标不明，主代理明确决定本轮也不尝试删除，计划状态为 `not_executed_due_prior_policy_target_ambiguity`，实际删除0文件、0B，181文件/679499152B保持。
