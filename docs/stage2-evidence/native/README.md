# 阶段2原厂后台调查记录

日期：2026-11-10；本轮标记：`//20261110-7`。本目录只保存原厂交互、源字段及离线编译证据，不代表阶段2全部完成。生产代码、Lite K持久化和最终USB验收由主任务负责，本子任务没有改生产源码或执行设备传输。

## 1. 身份、方法与证据范围

原件：`G:/01_Professional/触摸屏/金玺/KinSealStudio_Setup_V5.1/Installed/KinSealStudio5.1.exe`，SHA256为`305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c`。实际运行副本SHA256为`38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89`；仅沿用既有启动检查`0x7c4..0x7c6: 55 89 e5 → b0 01 c3`补丁。**这不是未修改原件运行**，不能据此证明启动初始化完全无差异。

原厂进程均位于`WinSta0/KinSealStage2Native20261110`私有桌面。Qt getter/setter、action和槽调用由该进程GUI线程的DispatchMessage钩子执行；树操作使用任务私有窗口消息。六张PNG用PrintWindow捕获任务私有模态窗，未切桌面、前置窗口或使用全局键鼠。子进程TEMP/TMP均指向本轮Z盘temp。原厂下载入口用于触发离线编译，读到本次新生成SysPara和“编译成功”后取出compile目录，随后关闭下载窗；**没有调用下载/传输按钮、USB下载器或PLC工具**。

部分原厂硬编码字符串在此私有运行环境显示为latin1/GBK乱码；JSON保留raw字符串，`ui-defaults.json`的DecodedCandidate只是可逆显示辅助，不能代替raw证据或宣称原件中文环境已核。最初缺少Link相对资源造成第二步列表空，已补齐Link/PlcDll后以`wizard-step2-r2`为正例，空列表不作为原厂默认。

索引：[完整run清单](run-manifest.json)、[控件默认/范围](ui-defaults.json)、[重放脚本](stage2_native_probe.py)、[场景配方](scenarios/)、[保留物及进程清单](cleanup.json)。只读源边界检查使用已冻结0.8.1程序，SHA256为`e2e661c04741535c2f6ed5eb15884df2a0a5a39eea64c94b49e6c468c77d21e8`；该检查不等于Lite语义导入/导出成功。

`native-probe*.json`现为可浏览表示：保留结果、操作元数据和模态窗字段；大量重复Qt batch/menu与主窗快照完整保存在各文件`rawEvidenceArchive`指向的`raw/*.json.gz`。每个档案均已解压逐字节核对原SHA，见[无损归档清单](compaction-manifest.json)。源.sm、typed差分、13组产物与PNG路径保持不变。

## 2. 新建工程向导（S02-02）

原厂真实步骤为三步：

1. **新项目**：名称与位置、HMI系列、尺寸、型号、方向在同页。名称`newprojectwizard_name`初始空；此次存放位置为运行目录下Project。系列默认AMZ，7.0 inch，AMZ070W01、水平。选择AMT系列后，7英寸列表的第二项为目标`AMT070W124-L800X480R`。向导方向只有水平/垂直两项。空名下一步提示“请输入工程的名字”。
2. **通信连接**：完整资源条件下制造商默认MODBUS，系列及名称默认Modbus_RTU。该页没有已观察到的端口选择。名称控件为`newlinkwizard_linkport_lineEdit`，制造商/系列为combobox_2/3。自定义连接名S02_Connection后，上一步与再次下一步仍保留该值；先前对“回退会重置”的口头判断已撤回。
3. **启动画面设置**：默认需要启动画面；可选不需要。图片路径指向运行目录extern下目标800×480启动PNG，单个显示时间默认1秒、范围1..60秒；完成/上一步/取消。这里的启动画面是图片设置，不能直接等同普通工程启动页。

用中文及空格工程名“阶段2 中文工程”，选择目标AMT、默认Modbus_RTU、无Logo，完成后执行普通保存，得到[原厂新建源](created-wizard-noLogo-save-r1-阶段2%20中文工程.sm)，3078字节，SHA256=`feef171598c0926b629ca4eb16f8d6b62483a37d08a687501b627c7e00dc344d`，完整typed检查583字段通过。向导完成时尚未生成.sm，普通保存后才生成。本例只证明该原厂流程可保存，没有独立验证新向导工程的设备下载。

在已开未修改工程上触发新建，原厂立即把画布页签计数从1变0；中途取消后主窗标题保持KinSealStudio，旧磁盘文件SHA保持不变。未测脏工程关闭提示。Lite按A02要求用草稿并保留取消前工程，是明确的产品事务取舍，不能写成已观察到原厂也这样处理。

图：[步骤1](native-wizard-1-identity.png)、[步骤2](native-wizard-2-link.png)、[步骤3](native-wizard-3-logo.png)。原始正例见`native-probe-wizard-step2-r2.json`、`native-probe-wizard-step3-r1.json`、`native-probe-wizard-noLogo-save-r1.json`、`native-probe-wizard-cancel-r1.json`。路径权限、重名文件覆盖、型号全集、浏览取消、脏工程事务未闭合。

## 3. 普通窗口（S02-03）

原厂`tree_newWindow`真实Qt入口打开新建窗口。默认名称“窗口2”，编号1（0..999），宽240（0..800）、高180（0..480），屏幕居中、关闭按钮勾选。选择指定坐标后X/Y出现，范围均0..1000，初始0。UI暴露零宽高不等于已证实确定时接受零尺寸，未测该边界。

默认/位图背景；默认样式共12项：纯色、横向过渡、横向对称过渡、纵向过渡、纵向对称过渡、斜上过渡、斜上对称过渡、斜下过渡、斜下对称过渡、右上角辐射、左上角辐射、中心辐射。宏表显示0/25，仅调查存在性，没有创建/执行宏。

新窗口空名确定提示“窗口名字不能为空，请输入一个名字”；普通新画面使用已有“画面1”提示“名字已经存在，请重新输入”。QLineEdit实际maxLength为32767，没有原厂256字上限证据。未核跨普通页/窗口的重名策略、复制和被引用删除。

已保存居中240×180、指定坐标55/66、指定位置320×200、取消关闭按钮四个独立源。`window-center-native-ui.sm`另开原厂进程重读，工程树中`001:Stage2_Window`可激活，主窗标题切到Stage2_Window，证明该样本可重开。源完整typed检查通过；未做Lite语义源往返或多页设备编译。

[源差分](window-source-differences.json)中可观察：后页`head.2/.3`为BE double宽高，`head.13`与居中/指定相关，`head.15/.16`与55/66坐标相关，`tail.0`与关闭按钮相关，`head.17=1`出现在窗口样本，`.18`名称、`.19`为001。**除名称/尺寸外仍按源观察候选保留**：不同原厂进程保存时，原对象common.58.3、page.head.9/.10、tail.3/.4等未解释字段也变化，不能把整份跨进程差分当成单字段协议。不能把UI编号、源页列表序号或trailer.page-map直接等同设备页ID。

图：[新建窗口](native-window-default.png)。正例`native-probe-window-create-r1.json`、`native-probe-window-reopen-r1.json`；空名/重名见`native-probe-window-empty-r1.json`、`native-probe-page-duplicate-r1.json`。

## 4. 系统五字段（S02-04）

五个字段先在同一原厂进程中，以“打开参数→无操作确定→保存”为基线，逐项修改、保存、还原；各变体与基线的完整typed语义差分只有表内目标字段。[源差分](system-source-differences.json)与六个`system-*-native-ui.sm`为N级事实。

| UI对象/含义 | 原厂默认、范围/单位 | 已确认源路径/类型 | 测试值的稳定C观察 |
|---|---|---|---|
| nScreensaver 屏幕保护时间 | 1；0..60，分钟 | config.head.5，i（BE32载体） | 1→2，SysPara byte[3]由1→2 |
| nFlickFqc 闪烁频率 | 10；1..50，标签x0.1s | config.head.6，i | 10→11，SysPara byte[4]由10→11 |
| nDefautUser 默认用户等级 | 1；1..8 | config.head.14，i | 1→2，SysPara byte[22]由1→2 |
| bTouchVoice 触摸声音 | true；布尔 | config.head.8，b | true→false，SysPara byte[8]由0→1，观察到反向编码 |
| checkBox_Hex LW使用16进制 | false；布尔 | config.flags.1，b | 排除重复编译变化后，没有稳定SysPara目标字节；不据此宣布无运行影响 |

类型i/b为本项目完整typed解析器名称。源字段位置随前面变长QString变化，不能把本例start/end当通用固定文件偏移。

**C证据**：`native-probe-system-compile-variants-r4.json`记录同一私有进程13次真正重编译：基线两次、五项各两次、还原基线一次。每次通过真实原厂参数窗口无操作确定和普通保存使缓存失效，验证本次SysPara更新时间及原厂“编译成功”，保存`compiled/final-*`，不点击传输。SysPara均4998字节。[完整比较](system-compile-comparison.json)保存每段长度、字节差异及重复变化；[重算脚本](compare_stage2_compiled.py)可重算。

同一基线重编译的SysPara本身也有非确定字节，13次合并得到121个变化偏移（主要在180..911）。它们呈地址形态，但消费者未恢复，不能武断命名为无效/可忽略padding。表中C观察要求第二轮和还原基线一致、两次目标一致，并排除所有同条件重复变化。**没有恢复字段完整宽度、整段统一端序或所有边界值；没有固件E验证。**

既有源在首次编译/再次参数确定后还有三项原厂归一化：config.200.7、config.240.7由1到7，config.head.12由null到“画面1”。[13份输入的完整字段核对](compiled-source-field-check.json)记录此事；以归一化后的第二轮基线对照，五个变体只增加各自目标源字段差异。不能拿第一次与后续的所有变化归因于测试项。

其他段的原始变化同样保留：AlarmPara、SheetPara、Usbload及压缩包等不能仅凭SHA变化推导业务副作用。重复变化掩码后，屏保/闪烁/等级/触摸声在其他原始.dat/.bin没有额外稳定变化；Hex的AlarmPara仍有7个稳定候选字节（含46192处10→16），未恢复含义，**不把它们宣布为地址进制协议**。压缩load.kns、upLoad.knr的字节差异未解为语义变化。更不能把本次C观察当作Lite编译实现已支持；主任务继续保持N/C守卫是正确边界。

参数窗口还观察到：型号/方向、最高权限、用户等级密码、屏保目标、上传密码等控件；本轮没有恢复其它活动字段或写入真实密码。方向窗口有水平/垂直/水平旋转180/垂直旋转180，与新建向导两项不同。[参数窗口图](native-parameters-default.png)。

## 5. Logo、HMI状态、PLC控制、时钟与键盘

对既有CodexProbeTest原厂打开`tree_modifylogoscreen`，Logo配置默认选择“不需要开机Logo或动画”。另有“需要开机Logo”和“需要启动动画”。因此现有单页默认基线确属无Logo路径。此编辑窗时间为**毫秒**，spinBox默认50、范围50..99000，当前禁用；向导是1..60**秒**，不能直接用同单位字段互换。添加外部图片/上下移动/删除在无Logo时禁用；未启用或保存活动Logo。[Logo图](native-logo-default.png)。

`native-probe-system-other-r1.json`取得HMI状态、PLC控制、时钟三个窗口P0默认：只枚举并取消，没有改单字段保存/编译。方向、单位、地址和总开关可在`ui-defaults.json`逐控件查看；这些只属于P0，不能宣布N/C支持。

自定义键盘`tree_newuserkeyboard`的Qt方法排队成功但本次未出现配置窗、未标脏；菜单action_Keyboard未能在本次主窗口集合定位。树上下文探针也未取到菜单，QPoint排队有false结果。**这些是探针覆盖缺口，不是原厂无键盘/无右键的证据。** 不创建猜测的键盘类别、页面角色或设备代码。

## 6. P/K/N/C/E结论与残留

| 切片 | 原厂P交互证据 | Lite K持久化 | N原厂源 | C离线编译 | E实屏 |
|---|---|---|---|---|---|
| 新建向导 | 三步/空名/回退/取消/无Logo中文保存 | 本子任务未验，主任务负责 | 无Logo原厂源typed正例；不等于Lite互通 | 本例未单独C；既有无Logo基线已确认 | 未做 |
| 新窗口 | 默认/尺寸/坐标/关闭按钮/空名/原厂重开 | 本子任务未验 | 结构可解析，几何部分候选；多页语义未闭合 | 未做多页C | 未做 |
| 系统五项 | 范围默认、确定/保存/还原 | 本子任务未验 | 五个字段严格一变量确认 | 13次成功，4项测试值稳定字节；Hex另有未解释候选 | 未做 |
| Logo活动、键盘 | Logo P0；键盘调查受限 | 本子任务未验 | 活动模式未恢复 | 应保持拒绝/禁用 | 未做 |
| HMI/PLC/时钟 | P0枚举、取消 | 本子任务未验 | 未恢复 | 应保持拒绝/禁用 | 未做 |

此调查不覆盖采集、报警、配方、资料传输、全局编辑命令和全部未保存事务；不能用于声明阶段2整阶段退出。源码/设备消费者未知项继续按A02/A05保留明确边界。

失败尝试已保留：资源不全、菜单探针未覆盖、控件文字乱码匹配、编译完成前过早采样、缓存复用被fresh-mtime守卫拒绝、下载窗关闭后的首个私有双击未生效。最终r4使用有界激活重试并成功完成13次编译，失败不被改写为通过。初次只读CLI误设offscreen与便携包不兼容，任务PID4160已精确终止；后续CLI使用windows平台/CREATE_NO_WINDOW，未把该失败作为事实来源。

所有本轮owned原厂PID均已结束；当前新runtime副本、相对资源和生成物的残留清单见`runtime-retention.json`、`cleanup.json`。本代理未删除新旧副本，也未重试以前policy拒绝的旧清理目标。只移除了本轮运行路径的MRU条目，保留其它最近工程。没有恢复已取消的关机任务。
