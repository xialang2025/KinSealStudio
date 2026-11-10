# 阶段3原生编辑及静态像素证据

日期：2026-11-10；修改标记：`//20261110-8 add`。

本目录由独立原厂进程执行 QAction、GUI 线程场景查询、原厂保存及原厂编译，未操作 USB/PLC，未点击下载传输按钮。所有窗口位于私有桌面 `WinSta0/KinSealStage3Native20261110`，未切换桌面、前置用户窗口或发送系统级键鼠输入。

## 身份和输入来源

- 原厂文件：`G:/01_Professional/触摸屏/金玺/KinSealStudio_Setup_V5.1/Installed/KinSealStudio5.1.exe`，SHA256 `305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c`。
- 运行的是私有副本，沿用启动探针唯一补丁 `0x7c4..0x7c6: 55 89 e5 → b0 01 c3`，运行体 SHA256 `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89`。不能称为未修改原厂运行体。
- 所有 Qt 操作通过自有进程 GUI 线程 DispatchMessage 钩子执行；场景选择使用 `QGraphicsItem::setSelected`，不是原厂鼠标点击。因此原生 `selectedItems()` 顺序不能等同于“最后点击对象”或“主选择”。
- 非对称输入来自上级 `../native-input/product-five-asymmetric.sm`。该输入实际12项，仅前5项调整了几何，本目录未删除其余7项。对齐只选择明确的A/B/C三项。
- 五族紧凑输入位于 `inputs/product-five-compact-all-address-roles.*`，为明确的产品自建 KSP 经冻结0.9.0导出 SM，**并非原厂从空白UI创建的样本**。导出器 SHA256 `dd61fe6bdf1a51e02ed20794743ccf9e9979699fd3fcd085d2a10b437c351a94`；仅使用其源导出和完整 typed 边界检查，未以其设备 C 编译结果作为原生基准。
- 场景脚本和运行目录位于 `.scratch/kinseal-stage3-20261110/native/`；exe 使用单个私有种子及 NTFS hardlink，未每次复制大 DLL/资源。TEMP/TMP均指向Z盘自有目录。

## 多重复制确证

原厂默认：X数量5、Y数量5，范围1..99；X间隔10（0..800）、Y间隔10（0..480）；“是否地址递增”默认关闭，关闭时递增方向/间隔隐藏。右增默认选中，下增未选；地址间隔默认1，范围0..256。唯一实际看到的排列类型为“间隔型”。截图 `native-multiple-copy-default.png`。

数量包括原对象。单灯3列×2行、120×60、X间隔17/Y间隔23，X为244/381/518，Y为112/195，确认间隔指边缘空隙。右增5的LB地址上排0/5/10，下排15/20/25；下增5上排0/10/20，下排5/15/25。原对象地址不变。

五种单体均在明确选中一个原对象后完成原厂复制及正常保存；源文件 `*-single-copy-2x2-right-increment5-native.sm`，同名 `-fields/-values/-objects.json` 保存完整 typed 边界、精确值和便读清单。

| 单体输入 | 原厂实际递增字段 | 保持不变的已激活字段 |
| --- | --- | --- |
| 位按钮，操作LB7、独立字监视LW40 | `switch.12c.0` → LB12/17/22；`switch.254.0` → LW45/50/55 | 通知LB21/LW44、显现LB31/LW52、触控LB41/LW60 |
| 字按钮，U32操作LW70 | `word.150.0` → LW75/80/85，字面步长5，未乘双字宽度 | common字段原样 |
| 多态指示灯，`lamp.monitorMode=2`、`multiDataType=1`、独立LW80 | `switch.254.0` → LW85/90/95，字面步长5 | common字段原样 |
| 静态文本，显现LB101/LW102 | 无递增字段 | 已激活显现LB101/LW102原样 |
| 默认系统功能按钮 | 无操作地址，未发现地址递增 | 包括通知LB40在内的common字段原样 |

位按钮的六个common地址对应：触控bit `common.40.0`、通知bit `common.e4.0`、显现bit `common.90.0`、触控word `common.60.0`、显现word `common.b0.0`、通知word `common.fc.0`。不能将这些“不变”的字段自动纳入原生递增规则。

多选边界：紧凑五族bbox303×168做2×2 gap17/23，理论最外界654×406；另选A/B bbox327×133做2×1 gap17，理论最右702。两个均在画面内的案例，原厂确认后没有新增可选择根项；不能将单体规则推断为多选整体包围框复制。产品若支持多选，应明确为扩展规则。本轮不再扩大矩阵。

## 对齐、尺寸、间距、组合和锁定

对象A `(31,47,81,39)`，B `(233,117,125,63)`，C `(487,289,57,91)`。原厂选择顺序来自Qt集合，跨进程可变：r2的 `selectedItems()` 为B/C/A，r3倒序API选择后为C/B/A。

- r2左/右/水平居中/垂直居中以列表首项B为参照：left x233；right右界358；`action_vcenter`（中文水平居中）中心X295.5；`action_hcenter`（中文垂直居中）中心Y148.5。现有样本宽高同奇偶，尚未确证半像素整数舍入。
- r3 `action_top`上对齐与`action_under`下对齐以列表首项C为参照，分别y289、底界380。
- `action_widthsame`等宽81、`action_heightsame`等高39、`action_widthheightsame`等宽高81×39，均符合列表尾项A。把A升到z12、B/C降到z1/2后仍取A，排除“最低z必为等尺寸参照”。未证明它等于鼠标先选项。
- `action_horizontal`水平等间距保留左右端，B.x233→237；`action_vertcal`垂直等间距保留上下端，B.y117→156。由非等尺寸样本区分了边缘空隙均分与中心等距。r3的七个命令各一次原厂Undo均恢复三项原几何。
- A/B `action_group`组合产生一个可选根项bbox `(31,47,327,133)`；`action_split`取消组合恢复两个根项及几何。原始子项保留在组合父项下。
- A `action_lock`后flags3→2：仍可选、移除movable；`action_delock`解锁后2→3。本轮未用原厂实际鼠标拖动确证锁定手势。

浏览精简结果见 `native-evidence-summary.json`；原始场景记录保存在同名 `.json.gz`。

## 奇数宽静态像素原厂编译

输入 `inputs/static-odd2-{base-only,transparent-top,opacity128-top}.{ksp,sm}`；来源见 `inputs/static-pixel-input-provenance.json`。冻结0.9.0仅导SM。第一版Arial输入被源导出保护拒绝；第二版保留seed微软雅黑，均成功导出。所有实际C结果来自私有原厂运行体的 `action_USBdownload` 编译阶段，以新鲜SysPara时间戳及“编译成功”日志双重确认，没有调用实际传输。运行日志 `native-probe-pixel-*-r1.json.gz`。

每版完整原厂输出位于 `compiled/static-odd2-*`，包括 ControlPara/ControlPixel/PicLenPara、其他原厂编译段、输入源。底层静态渐变文本为空、几何 `(41,53,101,61)`；上层文本T、几何 `(54,64,55,29)`；两语言相同。第三版上层style1、opacity128。没有动态控制项。

- ControlPara每对象640B。底图原始101×61×2=12322B，每语言LZO249B、补齐256B。顶层55×29×2=3190B，每语言LZO66B、补齐96B。奇数宽行紧密排列，没有把QImage的行填充写入原始像素。
- 三版底图sprite解压后逐字节相同。上层透明style0的sprite有1579像素RGB565 `0xc6ff`（该输入的页底色）及16个T文字像素`0xfe84`，**不是预合成下层渐变**。
- opacity128/style1的顶层底色为`0xda6a`，T文字仍`0xfe84`；与透明style0的顶层sprite不同。
- 底图ControlPara前4字节：单底图和opacity128上层为`1c00000e`；透明style0上层存在时为`1c000006`。这只是当前输入的原厂编译字段相关性；尚未定义flag的全部位语义。
- `native-static-pixels.json`记录精确偏移、长度、record hex、像素色计数、哈希；同目录frame `.lzo/.rgb565/.png`经独立MIT LZO解码器生成，未链接产品renderer或device compiler。

**这些C缓冲区不能证明屏端最终静态合成结果，也不支持动态重叠开放。** 本轮未运行Lite设备编译与这三个源的对照；该项由主任务以最终构建执行，不能把原厂编译成功表述为产品一致或实机通过。

## 失败、未知与退出

23次原厂运行中8次保留为 `partial-or-error`：场景flags调用最初遗漏QFlags隐藏返回缓冲导致自有进程退出；一次bool参数使用JS true被Frida拒绝；r2在等高后普通保存未置脏而停止；混合复制r1未置脏；其后四次Save As未得到有效新文件。Save As诊断中自有另存为对话框出现覆盖确认，但没有接受覆盖确认，未将这些尝试作为保存源证据。失败状态、错误、PID及输入哈希保留在原始日志和摘要，不混入成功基准。

未确证：原生鼠标主选择/拖动/撤销中断/缩放交互、等尺寸的一般选择锚点、半像素舍入、全部多选复制排列、地址边界溢出、全部族的所有未激活地址角色、组合保存格式/组合实机运行、动态透明叠加、屏端最终像素。本轮不自行填补。

所有23个本轮自有原厂PID均在finally终止并记录；未触碰用户PID300/18328。运行目录及旧调查目录保留，不作旧目标清理；原始大JSON仅无损gzip归档，逐字节往返验证，原始与压缩SHA256保存在摘要。MRU只移除当前自有运行目录或确切SaveAs目标条目，保留其他条目。关机要求保持取消。

独立LZO解码工具已持久化为 `tools/lzo_decode.exe` 及源码、MIT许可证、构建与文件身份 `tools/decoder-identity.json`。分析脚本默认使用此相对路径，也可通过 `--decoder` 显式指定；原始RGB565保留，重放不依赖scratch。
