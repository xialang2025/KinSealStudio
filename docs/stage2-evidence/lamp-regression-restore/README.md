# 指示灯原厂回归产物重新编译

日期：2026-11-10；`//20261110-7`。这是阶段2完整回归时发现旧scratch缺失后的独立恢复证据，**没有恢复或冒充2026-11-09旧产物SHA**。

`tests/fixtures/native-source/lamp/`保留的10个精确原厂UI源，已分别由原厂私有桌面新进程重新离线编译。每个`compiled/<fixture>/ControlPara.dat`均为640字节；源输入在本次过程中未修改，`input-native.sm`与现存原源逐字节一致。

| 原测试fixture | 原厂新产物目录 |
|---|---|
| lamp-default-r1 | compiled/lamp-default-r1 |
| lamp-wordbit7-r1 | compiled/lamp-wordbit7-r1 |
| lamp-wordbit15-r1 | compiled/lamp-wordbit15-r1 |
| lamp-multi5-u16-r4 | compiled/lamp-multi5-u16-r4 |
| lamp-multi5-u32-r1 | compiled/lamp-multi5-u32-r1 |
| lamp-none-r1 | compiled/lamp-none-r1 |
| lamp-multi5-flash0-r1 | compiled/lamp-multi5-flash0-r1 |
| lamp-multi5-flash2-r1 | compiled/lamp-multi5-flash2-r1 |
| lamp-dual-flash0-r1 | compiled/lamp-dual-flash0-r1 |
| lamp-multi2-flash0-r1 | compiled/lamp-multi2-flash0-r1 |

历史特殊源`product-lamp-noimage-samelanguage.sm`（4953字节、SHA256 `DE269318A9B6D30232648967552CB4144133485DD92A5405AC73DC2A50319E92`）只有旧文档记载，精确文件未找到，不能恢复旧oracle。主任务另提供显式产品输入`../product-lamp-input/shared-lamp-product.sm`（5056字节、SHA256 `f0ca66db052af78f6e79be20cbd67c88572f8bb59d17fe90eaa0b1798b1bd464`），本任务把它作为**新的original-native-compiled-product-input**交给原厂独立编译，输出`compiled/product-shared-lamp-new-native/ControlPara.dat`。它不是原厂UI创建源，也不是旧精确样本；设备oracle来自原厂编译器，未调用Lite设备编译器生成期望记录。

新组合产物同为640字节：type10/subtype4、geometry40,40,180,60、mode2/U16、5状态、sameLanguage=1、rawPixelBytes=108000，与显式新输入条件对应。10个历史源重编的活动字段（默认/位7/位15、U16/U32、多态/双态闪烁）也已读回记录；未把不活动union中的原厂地址形态残留当成常量。完整回归是否通过由主任务运行测试确认，本目录未改旧测试或生产代码。

[restoration-manifest.json](restoration-manifest.json)逐份记录输入SHA、原件/运行副本SHA、启动三字节补丁、新运行时间、原厂产物写入时间、完整输出SHA及活动字段。原件SHA为`305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c`；运行副本SHA为`38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89`，沿用启动检查0x7c4三字节补丁，不能称未修改原件运行。

全部原厂GUI位于`WinSta0/KinSealStage2LampRestore20261110`私有桌面，Qt操作仅在任务GUI线程执行。真实原厂编译成功并生成本轮fresh SysPara后取出compile目录，随后关闭下载窗，**没有调用下载/传输按钮、USB下载器或PLC工具**。没有全局键鼠、窗口前置、共享build操作。11个owned原厂进程已结束，当前列表为`active-owned-processes.json=[]`。新runtime残留只登记于cleanup，不删除旧scratch或任何先前被policy拒绝的目标。

原始Qt快照按`rawEvidenceArchive`保存无损gzip和原始SHA；可浏览JSON保留每个run状态、操作元数据及初始/模态窗。压缩/恢复清单和所有文件校验见`compaction-manifest.json`、`artifact-manifest.json`。
