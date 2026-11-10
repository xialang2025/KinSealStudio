# 阶段0入口与后台证据台账

日期：2026-11-10；任务：S00-02、S00-05。机器可读完整字段见[entry-inventory.json](entry-inventory.json)，原始快照见[entry-probe-readonly.json](entry-probe-readonly.json)，可复现配方见[entry-probe-readonly.py](entry-probe-readonly.py)。

本轮补齐已返回入口的身份、菜单/工具栏归属、无工程状态，并通过正常QAction全选确认12对象多选。单选/脏工程补采仅取得部分状态，未闭合。没有原厂编译、USB或PLC操作，不宣称阶段0已使全部功能可下载。

## 身份和补采范围

- 原件SHA256：`305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c`。
- 私有运行副本SHA256：`38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89`；仅0x7c4..0x7c6启动检查三字节`5589e5→b001c3`，不称未修改原件运行。
- 桌面`KinSealStage0Entries20261110`；无工程PID 284、已打开未改Probe PID 17696，均已终止。没有切换桌面或调用系统键鼠/前置窗口。
- 输入`KinSealStudio/samples/native-device-test/CodexProbeTest.sm`，SHA256 `aadec3da23d8bb5cf473504f57a79026e9a6e7707a2c498b76a46ef9244f9422`。本轮没有确认该样本对象选择数与配置全集，因此不把“已打开Probe”改称空工程或有配置状态。

## 旧采集器缺项的纠正

旧采集器按Qt QList数组偏移16读取；本轮r1重现每列表首项垃圾、末项漏采。纠正为本构建的20后，两状态全部已枚举菜单/工具栏0读取异常。r2每行保留20字节头供复核；[失败r1](entry-probe-readonly-r1.json)与[当时配方](entry-probe-readonly-r1.py)均保留。该ABI事实只适用已记录的Qt4构建，不能当.sm或设备协议字段。旧JSON保持原样。

根菜单实际为8个：`文件(&F)` → `编辑(&E)` → `视图(V)` → `控件(K)` → `添加(A)` → `工具(&T)` → `°ïÖú(&H)` → `语言(&R)`。旧快照漏掉最后“语言(&R)”及多个子菜单最后项；补得关于、安装usb驱动、辅助功能、工程图库、宏管理器、下移/取消组合/解锁/垂直翻转/缩小/同宽高及报警/数据群组入口。

旧10个图形探针只有`queue_status=queued`而`activation_result=null`；动作未证实激活，不能把无对话框判断为图形没有属性。

## 计数与8状态边界

| 类别 | 入口记录数 | 说明 |
|---|---:|---|
| 实际命名QAction | 121 | 菜单与工具栏按对象名去重，保留每处路径 |
| 工具箱 | 31 | 行序来自既有手册；btn原文多数空text，实际中文标签不写回按钮text |
| 基础图形 | 9 | 独立于31 |
| 表格 | 1 | 控件工具栏第32项，独立于31 |
| 工程树/配置与其他候选 | 32 | 来源与实际观察状态逐条明确 |

这些是入口层记录而不是独立功能数量。`toolBar_Control`有31控件+表格32项；`toolBar_Graphs`有9图形。

| 要求状态 | 现有证据 |
|---|---|
| 无工程 | 本轮真实Qt QAction/QWidget enabled；没有触发行为 |
| 空工程 | 未由本轮样本确认，不外推 |
| 单选 | Win32/Qt事件补采均未确认selectedItems=1；失败保留，不算通过 |
| 多选 | 已真实激活action_checkall，selectedItems=12；同一121动作已读属性 |
| 无前置配置 | 旧手册独立观察动态报警条/历史趋势图/历史数据显示器门槛；配方显示器无配方状态仍待验 |
| 有配置 | 旧手册独立观察最小报警/采集/配方具体样例；不外推所有配置组合 |
| 脏工程 | 仅部分证据：12对象x+1、Save/Undo enabled；windowModified=false，未确认内部dirty/保存重开转换 |
| 只读或运行中 | 两类均未观察，不写禁用 |

补充状态“已打开未改Probe”的全部Qt属性保存在JSON。该状态保存、撤销、恢复、取消组合disabled；无工程上载数据enabled，控件/下载/大多数编辑动作disabled。属性是当时状态，未验证点击结果。

## 31个工具箱入口

| ID | 名称 | btn对象 | 实际菜单/工具栏action | 属性窗口（既有手册） | 格式/消费者 |
|---|---|---|---|---|---|
| TBX.btn_switch_bit | 位按钮 | `btn_switch_bit` | `action_bitBtn` | 位按钮 | A05既有受限映射 |
| TBX.btn_switch_word | 字按钮 | `btn_switch_word` | `action_wordbutton` | 字按钮 | A05既有受限映射 |
| TBX.btn_switch_Lamp | 指示灯 | `btn_switch_Lamp` | `action_lamp` | 位指示灯（落指示灯设置页） | A05既有受限映射 |
| TBX.btn_switch_Fun | 功能按钮 | `btn_switch_Fun` | `action_funbutton` | 功能按钮 | A05既有受限映射 |
| TBX.btn_switch_Screen | 画面按钮 | `btn_switch_Screen` | `action_screenbutton` | 画面按钮 | Q-S00-ENTRY-MAP-05 |
| TBX.btn_switch_MultiState | 多态按钮 | `btn_switch_MultiState` | `action_polyBtn` | 多态按钮 | Q-S00-ENTRY-MAP-06 |
| TBX.btn_dataDisplay | 数值显示 | `btn_dataDisplay` | `action_dataDisplay` | 数值输入↔数值显示（随"允许输入"动态） | Q-S00-ENTRY-MAP-07 |
| TBX.btn_asciiDisplay | ASCII显示 | `btn_asciiDisplay` | `action_asciiDisplay` | ASCII输入↔ASCII显示 | Q-S00-ENTRY-MAP-08 |
| TBX.btn_timeDisplay | 时间显示 | `btn_timeDisplay` | `action_timeDisplay` | 时间显示 | Q-S00-ENTRY-MAP-09 |
| TBX.btn_MF | 多功能按钮 | `btn_MF` | `action_MFbtn` | 多功能按钮（MFBtnDlg） | Q-S00-ENTRY-MAP-10 |
| TBX.btn_gif | GIF显示 | `btn_gif` | `action_GIFoutput` | GIF显示器 | Q-S00-ENTRY-MAP-11 |
| TBX.btn_picture | 图片显示 | `btn_picture` | `pictruedisplay` | 图片显示器 | Q-S00-ENTRY-MAP-12 |
| TBX.btn_alarmbar | 动态报警条 | `btn_alarmbar` | `action_dynamicarlarm` | 动态报警条 | Q-S00-ENTRY-MAP-13 |
| TBX.btn_hisalarmDisplay | 历史报警显示 | `btn_hisalarmDisplay` | `action_histotyarlarm` | 历史报警显示器 | Q-S00-ENTRY-MAP-14 |
| TBX.btn_alarmDisplay | 报警显示 | `btn_alarmDisplay` | `action_arlamcontrol` | 报警显示器 | Q-S00-ENTRY-MAP-15 |
| TBX.btn_text | 静态文本 | `btn_text` | `action_text` | 静态文本 | A05既有受限映射 |
| TBX.btn_messageDisplay | 消息显示 | `btn_messageDisplay` | `action_messagedisplay` | 消息显示器 | Q-S00-ENTRY-MAP-17 |
| TBX.btn_XYtrend | X-Y曲线 | `btn_XYtrend` | `action_X_Y` | X-Y曲线 | Q-S00-ENTRY-MAP-18 |
| TBX.btn_trend | 趋势图 | `btn_trend` | `action_trendchart` | 趋势图 | Q-S00-ENTRY-MAP-19 |
| TBX.btn_HisTrend | 历史趋势图 | `btn_HisTrend` | `action_DatagroupDisplay` | 历史趋势图 | Q-S00-ENTRY-MAP-20 |
| TBX.btn_dataGroup | 数据群组 | `btn_dataGroup` | `action_dataGroup` | 数据群组 | Q-S00-ENTRY-MAP-21 |
| TBX.btn_hisdataDisplay | 历史数据显示器 | `btn_hisdataDisplay` | `action_historyrecord` | 历史数据显示器 | Q-S00-ENTRY-MAP-22 |
| TBX.btn__recipetable | 配方显示器 | `btn__recipetable` | `action_recipetable` | 配方显示器 | Q-S00-ENTRY-MAP-23 |
| TBX.btn_metre | 图表 | `btn_metre` | `graphicsitem` | 图表（btn_metre，柱状图/仪表/圆环三形态） | Q-S00-ENTRY-MAP-24 |
| TBX.btn_droplist | 下拉框 | `btn_droplist` | `action_droplist` | 下拉框 | Q-S00-ENTRY-MAP-25 |
| TBX.btn_flow | 流动块 | `btn_flow` | `action_flow_block` | 流动块 | Q-S00-ENTRY-MAP-26 |
| TBX.btn_slider | 滑动开关 | `btn_slider` | `action_Slider` | 滑动开关（SlideSwitchDlg） | Q-S00-ENTRY-MAP-27 |
| TBX.btn_moveTrack | 移动轨迹 | `btn_moveTrack` | `action_moveTrack` | 移动轨迹 | Q-S00-ENTRY-MAP-28 |
| TBX.btn_wheelview | 滚轮 | `btn_wheelview` | `action_wheelView` | 滚轮控件 | Q-S00-ENTRY-MAP-29 |
| TBX.btn_qrcode | 二维码 | `btn_qrcode` | `action_qrcode` | 二维码 | Q-S00-ENTRY-MAP-30 |
| TBX.btn_scale | 刻度 | `btn_scale` | `action_mark` | 刻度 | Q-S00-ENTRY-MAP-31 |

历史趋势图action实际为`action_DatagroupDisplay`；数据群组为新补`action_dataGroup`。不能按英文片段交换。报警三项分别为`action_dynamicarlarm`、`action_histotyarlarm`、`action_arlamcontrol`（原文拼写保留）。

## 9图形与表格

| ID | 名称 | QAction | 源/设备映射待验 |
|---|---|---|---|
| GEO.LINE | 直线 | `action_line` | Q-S00-ENTRY-MAP-32 |
| GEO.POLYLINE | 折线 | `action_foldline` | Q-S00-ENTRY-MAP-33 |
| GEO.FREE | 自由直线 | `action_freeline` | Q-S00-ENTRY-MAP-34 |
| GEO.ARC | 曲线/圆弧 | `action_blight_circle` | Q-S00-ENTRY-MAP-35 |
| GEO.RECT | 矩形 | `action_rect` | Q-S00-ENTRY-MAP-36 |
| GEO.ROUND | 圆角矩形 | `action_RoundedRect` | Q-S00-ENTRY-MAP-37 |
| GEO.ELLIPSE | 圆/椭圆 | `action_ellipse` | Q-S00-ENTRY-MAP-38 |
| GEO.POLYGON | 多边形 | `action_polygon` | Q-S00-ENTRY-MAP-39 |
| GEO.SECTOR | 扇形 | `action_ArcItem` | Q-S00-ENTRY-MAP-40 |
| GEO.TABLE | 表格 | `action_table` | Q-S00-ENTRY-MAP-41 |

## 主菜单和工具栏命令

精确标签保留原始换行和乱码；JSON原文未改。快捷键通过QAction::shortcut和QKeySequence::toString(PortableText)直接读取；空序列不证明不存在自定义按键事件。`Ctrl+P`为属性设置；多重复制本轮标准QAction快捷键为空，画布/文本/模态焦点的实际事件仍待Q-A02-CMD-001。锁定与解锁均返回Ctrl+L；同时存在不代表两个动作在每个上下文均触发。

| 稳定ID | 精确标签 | 菜单/工具栏归属 | 无工程enabled | 已开未改Probe enabled | QAction快捷键 |
|---|---|---|---|---|---|
| CMD.action_bitBtn | 位按钮 | toolBar_Control; 控件(K)/位按钮 | False | True |  |
| CMD.action_wordbutton | 字按钮 | toolBar_Control; 控件(K)/字按钮 | False | True |  |
| CMD.action_lamp | 指示灯 | toolBar_Control; 控件(K)/指示灯 | False | True |  |
| CMD.action_funbutton | 功能按钮 | toolBar_Control; 控件(K)/功能按钮 | False | True |  |
| CMD.action_screenbutton | 画面按钮 | toolBar_Control; 控件(K)/画面按钮 | False | True |  |
| CMD.action_polyBtn | 多态按钮 | toolBar_Control; 控件(K)/多态按钮 | False | True |  |
| CMD.action_dataDisplay | 数值显示 | toolBar_Control; 控件(K)/数值显示 | False | True |  |
| CMD.action_asciiDisplay | ASCII显示 | toolBar_Control; 控件(K)/ASCII显示 | False | True |  |
| CMD.action_timeDisplay | 时间显示 | toolBar_Control; 控件(K)/时间显示 | False | True |  |
| CMD.action_messagedisplay | 消息显示 | toolBar_Control; 控件(K)/消息显示 | False | True |  |
| CMD.action_GIFoutput | GIF显示器 | toolBar_Control; 控件(K)/GIF显示器 | False | True |  |
| CMD.pictruedisplay | 图片显示 | toolBar_Control; 控件(K)/图片显示 | False | True |  |
| CMD.action_MFbtn | 多功能按钮开关 | toolBar_Control; 控件(K)/多功能按钮开关 | False | True |  |
| CMD.action_flow_block | 流动块 | toolBar_Control; 控件(K)/流动块 | False | True |  |
| CMD.action_historyrecord | 历史数据显示器 | toolBar_Control; 控件(K)/历史数据显示器 | False | True |  |
| CMD.graphicsitem | 图表 | toolBar_Control; 控件(K)/图表 | False | True |  |
| CMD.action_trendchart | 趋势图 | toolBar_Control; 控件(K)/曲线/趋势图 | False | True |  |
| CMD.action_histotyarlarm | 历史报警显示器 | toolBar_Control; 控件(K)/报警/历史报警显示器 | False | True |  |
| CMD.action_DatagroupDisplay | 历史趋势图 | toolBar_Control; 控件(K)/曲线/历史趋势图 | False | True |  |
| CMD.action_dynamicarlarm | 动态报警条 | toolBar_Control; 控件(K)/报警/动态报警条 | False | True |  |
| CMD.action_arlamcontrol | 报警控件 | toolBar_Control; 控件(K)/报警/报警控件 | False | True |  |
| CMD.action_X_Y | XY曲线 | toolBar_Control; 控件(K)/曲线/XY曲线 | False | True |  |
| CMD.action_droplist | 下拉框 | toolBar_Control; 控件(K)/下拉框 | False | True |  |
| CMD.action_moveTrack | 移动轨迹 | toolBar_Control; 控件(K)/移动轨迹 | False | True |  |
| CMD.action_dataGroup | 数据群组 | toolBar_Control; 控件(K)/曲线/数据群组 | False | True |  |
| CMD.action_mark | 刻度 | toolBar_Control; 控件(K)/刻度 | False | True |  |
| CMD.action_text | 文字 | toolBar_Control; 控件(K)/文字 | False | True |  |
| CMD.action_qrcode | 二维码 | toolBar_Control; 控件(K)/二维码 | False | True |  |
| CMD.action_table | 表格 | toolBar_Control; 控件(K)/表格 | False | True |  |
| CMD.action_Slider | 滑动开关 | toolBar_Control; 控件(K)/滑动开关 | False | True |  |
| CMD.action_wheelView | 滚轮 | toolBar_Control; 控件(K)/滚轮 | False | True |  |
| CMD.action_recipetable | 配方显示器 | toolBar_Control; 控件(K)/配方显示器 | False | True |  |
| CMD.action_line | 直线(&L) | toolBar_Graphs; 控件(K)/直线(&L) | False | True |  |
| CMD.action_foldline | 折线(&B) | toolBar_Graphs; 控件(K)/折线(&B) | False | True |  |
| CMD.action_blight_circle | 曲线/圆弧(&A) | toolBar_Graphs; 控件(K)/曲线/圆弧(&A) | False | True |  |
| CMD.action_freeline | 自由直线 | toolBar_Graphs; 控件(K)/自由直线 | False | True |  |
| CMD.action_rect | 矩形(&R) | toolBar_Graphs; 控件(K)/矩形(&R) | False | True |  |
| CMD.action_RoundedRect | 圆角矩形 | toolBar_Graphs; 控件(K)/圆角矩形 | False | True |  |
| CMD.action_polygon | 多边形(&Y) | toolBar_Graphs; 控件(K)/多边形(&Y) | False | True |  |
| CMD.action_ellipse | 椭圆(&E) | toolBar_Graphs; 控件(K)/椭圆(&E) | False | True |  |
| CMD.action_ArcItem | 扇形 | toolBar_Graphs; 控件(K)/扇形 | False | True |  |
| CMD.action_new_pro | 新建工程(&N)... | 文件(&F)/新建工程(&N)...; standardToolBar | True | True | Ctrl+N |
| CMD.action_open_pro | 打开工程(&O)... | 文件(&F)/打开工程(&O)...; standardToolBar | True | True | Ctrl+O |
| CMD.action_close | 关闭 | 文件(&F)/关闭 | False | True |  |
| CMD.action_save | 保存(&S) | 文件(&F)/保存(&S); standardToolBar | False | False | Ctrl+S |
| CMD.action_save_as | 另存为 | 文件(&F)/另存为 | False | True |  |
| CMD.actionCompress | 解压V31标准组态 | 文件(&F)/解压V31标准组态 | True | True |  |
| CMD.action_quit | 退出(&Q) | 文件(&F)/退出(&Q) | True | True |  |
| CMD.action_Auxi | 辅助功能 | 文件(&F)/辅助功能 | True | True |  |
| CMD.action_undo | ³·Ïú | 编辑(&E)/³·Ïú; standardToolBar | False | False | Ctrl+Z |
| CMD.action_redo | »Ö¸´ | 编辑(&E)/»Ö¸´; standardToolBar | False | False | Ctrl+Y |
| CMD.action_cut | 剪切(&T) | 编辑(&E)/剪切(&T); mapToolBar | False | True | Ctrl+X |
| CMD.action_paste | 粘贴(&P) | 编辑(&E)/粘贴(&P); mapToolBar | False | True | Ctrl+V |
| CMD.action_copy | 复制(&C) | 编辑(&E)/复制(&C); mapToolBar | False | True | Ctrl+C |
| CMD.action_copym | 多重复制(&D) | 编辑(&E)/多重复制(&D) | False | True |  |
| CMD.action_delete | 删除(&D) | 编辑(&E)/删除(&D); mapToolBar | False | True | Del |
| CMD.action_checkall | 全选(&A) | 编辑(&E)/全选(&A) | False | True | Ctrl+A |
| CMD.action_bringtop | 移到最上层(&F) | 编辑(&E)/层次/移到最上层(&F); toolBar_Aligning | False | True |  |
| CMD.action_bringforward | 移到上一层 | 编辑(&E)/层次/移到上一层; toolBar_Aligning | False | True |  |
| CMD.action_sengtoback | 移到最下层 | 编辑(&E)/层次/移到最下层; toolBar_Aligning | False | True |  |
| CMD.action_bringback | 移到下一层(&B) | 编辑(&E)/层次/移到下一层(&B); toolBar_Aligning | False | True |  |
| CMD.action_top | 上对齐 | 编辑(&E)/对齐/上对齐; toolBar_Aligning | False | True | Ctrl+Up |
| CMD.action_under | 下对齐 | 编辑(&E)/对齐/下对齐; toolBar_Aligning | False | True | Ctrl+Down |
| CMD.action_vcenter | 水平居中 | 编辑(&E)/对齐/水平居中; toolBar_Aligning | False | True |  |
| CMD.action_left | 左对齐 | 编辑(&E)/对齐/左对齐; toolBar_Aligning | False | True | Ctrl+Left |
| CMD.action_right | 右对齐 | 编辑(&E)/对齐/右对齐; toolBar_Aligning | False | True | Ctrl+Right |
| CMD.action_hcenter | 垂直居中 | 编辑(&E)/对齐/垂直居中; toolBar_Aligning | False | True |  |
| CMD.action_horizontal | 水平等间距 | 编辑(&E)/对齐/水平等间距; toolBar_Aligning | False | True |  |
| CMD.action_vertcal | 垂直等间距 | 编辑(&E)/对齐/垂直等间距; toolBar_Aligning | False | True |  |
| CMD.action_widthsame | 宽度相同 | 编辑(&E)/对齐/宽度相同; toolBar_Aligning | False | True | Ctrl+W |
| CMD.action_heightsame | 高度相同 | 编辑(&E)/对齐/高度相同; toolBar_Aligning | False | True | Ctrl+H |
| CMD.action_widthheightsame | 宽度高度都相同 | 编辑(&E)/对齐/宽度高度都相同; toolBar_Aligning | False | True | Ctrl+R |
| CMD.action_group | 组合 | 编辑(&E)/组合/组合; mapToolBar | False | True |  |
| CMD.action_split | 取消组合 | 编辑(&E)/组合/取消组合; mapToolBar | False | False |  |
| CMD.action_lock | 锁定 | 编辑(&E)/锁定/锁定; mapToolBar | False | True | Ctrl+L |
| CMD.action_delock | 解锁 | 编辑(&E)/锁定/解锁; mapToolBar | False | True | Ctrl+L |
| CMD.action_h | 水平翻转 | 编辑(&E)/镜像/水平翻转; mapToolBar | False | True |  |
| CMD.action_v | 垂直翻转 | 编辑(&E)/镜像/垂直翻转; mapToolBar | False | True |  |
| CMD.action_zoonbig | 放大 | 编辑(&E)/图像比例/放大; mapToolBar | False | True |  |
| CMD.action_zoonfit | 图像复原 | 编辑(&E)/图像比例/图像复原; mapToolBar | False | True |  |
| CMD.action_zoonsmall | 缩小 | 编辑(&E)/图像比例/缩小; mapToolBar | False | True |  |
| CMD.action_grid | 网格 | 编辑(&E)/网格; mapToolBar | False | True |  |
| CMD.action_panview | 平移视图 | 编辑(&E)/平移视图 | False | True |  |
| CMD.action_upscreen | 上一幅画面 | 编辑(&E)/上一幅画面 | False | True |  |
| CMD.action_nextscreen | 下一幅画面 | 编辑(&E)/下一幅画面 | False | True |  |
| CMD.action_reverse_selection | 反向选择 | 编辑(&E)/反向选择 | False | True |  |
| CMD.action_address_find | 查找替换 | 编辑(&E)/查找替换; mapToolBar | False | True | Ctrl+F |
| CMD.action_items_table | 控件一览表 | 编辑(&E)/控件一览表; toolBar_Tool | False | True |  |
| CMD.action_TextView | 文本<br>一览表 | 编辑(&E)/文本<br>一览表; toolBar_Tool | False | True | Ctrl+T |
| CMD.action_address_table | 地址一览表 | 编辑(&E)/地址一览表; toolBar_Tool | False | True | Ctrl+U |
| CMD.action_openpropertiset | 属性设置 | 编辑(&E)/属性设置; toolBar_Tool | False | True | Ctrl+P |
| CMD.action_Size | 优化控件尺寸坐标 | 编辑(&E)/优化控件尺寸坐标 | False | True |  |
| CMD.action_macro_find | 宏指令<br>查找 | 编辑(&E)/宏指令<br>查找; toolBar_Tool | False | True |  |
| CMD.action_macro_admin | 宏指令<br>管理器 | 编辑(&E)/宏指令<br>管理器; toolBar_Tool | False | True |  |
| CMD.action_40 | 标准状态栏 | 视图(V)/标准状态栏 | True | True |  |
| CMD.action_36 | 标准工具栏 | 视图(V)/标准工具栏 | True | True |  |
| CMD.action_tool | 组态工具箱 | 视图(V)/组态工具箱 | False | True | Ctrl+B |
| CMD.action_37 | 扩展工具栏 | 视图(V)/扩展工具栏 | True | True |  |
| CMD.action_41 | 信息输出窗口 | 视图(V)/信息输出窗口 | True | True |  |
| CMD.action_42 | 工程管理器 | 视图(V)/工程管理器 | True | True |  |
| CMD.action_rep | 添加配方 | 添加(A)/添加配方 | False | True |  |
| CMD.action_datasamp | 添加数据采集 | 添加(A)/添加数据采集 | False | True |  |
| CMD.action_waring | 添加数位地址报警 | 添加(A)/添加数位地址报警 | False | True |  |
| CMD.action_conalarm | 添加条件报警 | 添加(A)/添加条件报警 | False | True |  |
| CMD.action_script | 打开宏指令编辑器 | 添加(A)/打开宏指令编辑器; toolBar_Tool | False | True |  |
| CMD.action_InitMacro | 初始化宏 | 添加(A)/初始化宏; toolBar_Tool | False | True |  |
| CMD.action_GlobalMacro | 全局宏 | 添加(A)/全局宏; toolBar_Tool | False | True |  |
| CMD.action_ProLib | 工程图库 | 添加(A)/工程图库; toolBar_Tool | False | True |  |
| CMD.action_upload | 上载数据 | 工具(&T)/上载数据; toolBar_Tool | True | True |  |
| CMD.action_USBdownload | 下载工程(&D) | 工具(&T)/下载工程(&D); toolBar_Tool | False | True |  |
| CMD.action_offline_simulation | 离线<br>模拟 | 工具(&T)/离线<br>模拟; toolBar_Tool | False | True |  |
| CMD.action_online_simulation | 在线模拟 | 工具(&T)/在线模拟; toolBar_Tool | False | True |  |
| CMD.action_upgrade | 升级 | 工具(&T)/升级 | False | True |  |
| CMD.action_version | 底层版本号 | 工具(&T)/底层版本号 | False | True |  |
| CMD.action_usbdriver | 安装usb驱动 | 工具(&T)/安装usb驱动 | True | True |  |
| CMD.action_H | 帮助(&H) | °ïÖú(&H)/帮助(&H) | True | True | F1 |
| CMD.action_KinSeal | 关于 | °ïÖú(&H)/关于 | True | True |  |
| CMD.action_chinese | 中文 | 语言(&R)/中文 | True | True |  |
| CMD.action_english | English | 语言(&R)/English | True | True |  |
| CMD.action_upgrade2 | 升级底层 | toolBar_Tool | False | True |  |
| CMD.action_new_screen | 新建画面 | standardToolBar | False | True |  |

## 工程树、右键、双击及候选

本轮读取主窗口和工具栏，不向树发上下文动作。帮助/旧手册已给路径的条目继续保留；QObject未知写null，不伪造对象名。每行的问题与下一动作已在JSON的questions关联。

| 稳定ID | 来源路径/标签 | 入口状态 | 窗口/结果 |
|---|---|---|---|
| TREE.COM.PROPERTIES | 连接→COM1/COM2→双击串口 | help-candidate-not-currently-triggered | 连接——串口 |
| TREE.COM.ADD_PROTOCOL | 连接→COM1/COM2→右键→增加协议 | help-candidate-not-currently-triggered | 连接——协议 |
| TREE.PAGE.NEW | 画面→右键→新建画面 | help-candidate-not-currently-triggered | 新建画面 |
| TREE.PAGE.OPEN | 画面名称→右键→打开 | help-candidate-not-currently-triggered | 画面编辑 |
| TREE.PAGE.DOUBLE_CLICK | 工程管理器→画面名称→双击 | help-candidate-not-currently-triggered | 画面编辑 |
| TREE.PAGE.PROPERTIES | 画面名称→右键→属性 | help-candidate-not-currently-triggered | 画面属性 |
| TREE.PAGE.COPY | 画面名称→右键→复制 | help-candidate-not-currently-triggered | 画面复制 |
| TREE.PAGE.DELETE | 画面名称→右键→删除 | help-candidate-not-currently-triggered | 删除确认 |
| TREE.WINDOW.NEW | 窗口→右键→新建窗口（实际标签待验） | help-candidate-not-currently-triggered | 新建窗口 |
| TREE.WINDOW.OPEN | 窗口名称→右键→打开 | help-candidate-not-currently-triggered | 窗口编辑 |
| TREE.WINDOW.DOUBLE_CLICK | 窗口名称→双击 | help-candidate-not-currently-triggered | 窗口编辑 |
| TREE.WINDOW.PROPERTIES | 窗口名称→右键→属性 | help-candidate-not-currently-triggered | 窗口属性 |
| TREE.WINDOW.COPY | 窗口名称→右键→复制 | help-candidate-not-currently-triggered | 窗口复制 |
| TREE.WINDOW.DELETE | 窗口名称→右键→删除 | help-candidate-not-currently-triggered | 删除确认 |
| TREE.DATA.NEW | 系统参数→数据采集→右键→新建数据采集 | historically-observed-menu-and-dialog | 数据采集 |
| TREE.TRANSFER.NEW | 系统参数→资料传输→右键→添加资料传输 | help-candidate-not-currently-triggered | 资料传输 |
| TREE.RECIPE.NEW | 系统参数→配方→右键→新建配方/添加配方（帮助用词不同） | help-candidate-not-currently-triggered | 配方 |
| TREE.LANGUAGE | 系统参数→语言设置→右键（具体项待验） | help-candidate-not-currently-triggered | 语言设置 |
| TREE.FILE_ACCESS | 系统参数→访问加密→右键（具体项待验） | help-candidate-not-currently-triggered | 访问加密 |
| TREE.SYS.PARAMETERS | 系统设置→参数设置（真实入口手势待验） | help-candidate-not-currently-triggered | 参数设置 |
| TREE.SYS.HMI_STATUS | 系统设置→HMI状态（真实入口手势待验） | help-candidate-not-currently-triggered | HMI状态 |
| TREE.SYS.PLC_CONTROL | 系统设置→PLC控制（真实入口手势待验） | help-candidate-not-currently-triggered | PLC控制 |
| TREE.SYS.FILE_PROTECTION | 系统设置→文件保护（真实入口手势待验） | help-candidate-not-currently-triggered | 文件保护 |
| TREE.SYS.HMI_PROTECTION | 系统设置→HMI保护（真实入口手势待验） | help-candidate-not-currently-triggered | HMI保护 |
| TREE.SYS.CLOCK | 系统设置→时钟设置（真实入口手势待验） | help-candidate-not-currently-triggered | 时钟设置 |
| TREE.ALARM.BIT.DOUBLE_CLICK | 系统参数→报警登录→数位地址报警→双击 | historically-observed-bounded | 数位地址报警 |
| TREE.ALARM.CONDITION | 系统参数→报警登录→条件报警（手势待验） | candidate-not-observed | 条件报警 |
| TREE.KEYBOARD.NEW | 自定义键盘→右键→添加（实际标签待验） | candidate-not-observed | 自定义键盘 |
| TREE.LOGO | Logo候选节点/向导启动页关联待核 | candidate-not-observed | 待验 |
| GEO.CONNECTION_LINE.CANDIDATE | 连接线候选（静态线索，不计入9图形） | candidate-not-observed | 待验 |
| CTX.OBJECT.PROPERTIES | 画布对象→右键→属性 | historically-observed-bounded | 当前选中对象属性 |
| DBL.OBJECT.PROPERTIES | 画布对象→双击 | historically-observed-bounded | 当前对象属性 |

## 未闭环与下一步

- 每个命名动作的业务与焦点行为用`Q-S00-ENTRY-CMD-*`追踪；工具箱/图形/表格未知源与设备消费者用`Q-S00-ENTRY-MAP-*`逐项追踪。已有5类只引用A05受限范围；此次未重跑其格式/设备测试。
- 工程树/配置项用`Q-S00-ENTRY-TREE-*`，其余路线/连接线候选用`Q-S00-ENTRY-ROUTE-*`；每个Q在JSON写出指定窗口/动作、源A/A2/B和原厂重开/离线差分的下一探针。
- Qt读取失败、事件未激活、真实disabled分别记录。r1读取错误已保留；旧图形未激活仍待行为探针；新属性disabled只用于对应状态。
- 本轮没有借入口台账开放未知编译/下载，也没有把USB ACK升级为屏端行为通过。

恢复点：先按`Q-A02-CMD-001`建立空工程/单选/多选/脏/只读/运行的状态矩阵，再在各阶段执行对应单个命令的业务探针；未知源/消费者维持拒绝。

## 有界状态补采与冻结

最终配方与记录：[entry-probe-states.py](entry-probe-states.py)、[entry-probe-states.json](entry-probe-states.json)。采用的多选证据在[状态r2](entry-probe-states-r2.json)：GUI线程正常QAction::activate(action_checkall)实际执行，selectedItems从0变12；此后采同一121个QAction。单选点击与正常Qt鼠标事件最终均未达到1，不写成功。

r2私有键事件曾使12对象x各+1、Save/Undo由0变1，而QWidget windowModified=false；这只证明场景变化与原生保存指示，尚未读到真实dirty字段/完成持久化转换。r3异常打开趋势图窗口已取消并留Win32窗口记录，不能把它改称保存提示；r4缺QString默认构造导出、r1返回ABI错误均保留。最终r5 PID 3244已终止，Qt事件分发但单选/保存转换仍未成立。没有使用Windows共享剪贴板。

空工程、配置、只读与运行的具体阻塞、单选及dirty下一探针分别为Q-S00-ENTRY-STATE-EMPTY/CONFIG/READONLY/RUNNING/SINGLE/DIRTY，完整问题和下一动作在JSON。此次探针范围已冻结，未继续试配置或模拟。

## 清理记录

[entry-cleanup.json](entry-cleanup.json)列出本任务8个runtime目录的40个可再生文件，共151,066,487字节；本任务进程均已结束。主代理对本任务生成pycache的删除被自动审批以`blocked by policy`拒绝，按指示未再次删除或改用其他通道。该分工删除0文件/0字节，原有S02目录未触碰；残留不冒称已清理。
