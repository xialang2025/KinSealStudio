# 指示灯属性窗口实现交接

日期：2026-11-09。修改标记：`//20261109-8 add/change`。

## 文件与接口

- `KinSealStudio/src/lamp_properties.h/.cpp`：独立监视属性、版本 1 JSON、校验、2..32 状态外观与附加状态访问。
- `KinSealStudio/src/bit_button_dialog.h/.cpp`：复用位/字按钮家族属性窗口，增加指示灯分支。
- `KinSealStudio/tests/lamp_dialog_tests.cpp`：属性、交互、地址拾取与回填测试。
- 对话框修改前基线保存在本目录 `baseline/`。

`LampProperties` 字段为 `monitorMode`（0 为对话框临时全不选，1 为双态，2 为多态）、`multiDataType`（0/1）、`multiStateCount`（2..32）、`multiMonitorSame`、`multiMonitorAddress`、`extraStates`（状态 2 以上）。状态 0/1 保留在 `BitButtonProperties::states`。

接口为 `toJson`、`fromJson`、`validate`、`stateCount`、`transferWordCount`、`ensureStateCount(count, shared)`，以及可写/只读的 `state(shared, index)`，越界返回空指针。

## 原厂证据补充

UI 手册中的下列待验项已由设备分析代理的真实 Qt getter 和成对源工程补齐：

1. 真正点击多态监视后，状态数范围为 2..32，类型恰为两项；此前仅 `setChecked` 干预取得的 256 上限已经撤回。
2. 多态 5 状态时，外观选择范围为 0..4。状态 2 以上默认为空文本、微软雅黑 16、黑色居中、`switch_ON.kvg`、背景 `#a0a0a0`、透明度 255。
3. 两处“与操作地址相同”始终禁用。指示灯的整个宏组、整个通知组均禁用；这两项不再描述为合成点击局限。
4. 多态监视标题为“多态指示灯”，双态为“位指示灯”。状态 2 的两复制按钮均从状态 0 复制且可用。
5. 新建位地址模式显示字位 15，保存为 0；新建切字地址且未改字位时保存 15。
6. 双/多组都取消后，确定操作归一为多态；新建原始默认即 U16、2 状态、LW0。实现保留已有多态草稿，只归一模式，不自行清空数值。
7. 多态状态 4 写字后，将状态数 5 降至 2 再升至 5，原厂状态 4 文本变空。实现已随显式状态数变化删除被移除的附加行，再增长时使用原厂默认值；仅切换双/多态监视模式时保留原先多态草稿。
8. “使用同一段文字”是原厂指示灯全控件开关：状态 0 勾选后状态 2/4 均勾选，状态 2 取消后状态 0 亦取消。实现仅对 Lamp 同步所有状态的 `sameLanguage`，各状态 `sharedText` 按其自己的当前语言文本初始化；新增状态继承该布尔值，文本仍为空。位/字按钮现有行为保持。

## 已验证

- 独立 Release 构建：`build/LampDialogIsolatedTests.exe`。
- Offscreen：`lamp-ui-tests.txt`，13 通过、0 失败。
- 隔离 Windows 桌面：`lamp-windows-tests.txt`，13 通过、0 失败。
- 屏幕截图：`windows-screenshots/`。已检查默认监视页、单列权限页及多态状态 4 外观，中文可读，无控件裁切。
- 只读复核覆盖：空指针与状态索引、Bit/Word 分支、隐藏触控和按压字段、两地址缓存、Alpha 图片路径、附加状态保存与取消事务。未发现未解决的代码逻辑问题。

## 尚待交接方完成

- 主程序 CMake 所有链接 `bit_button_dialog.cpp` 的目标需链接 `lamp_properties.cpp`；新增测试目标接入 Build。
- 模型 schema、画布多状态、独立设备编码、原厂源工程格式与 USB 下载由主代理及其它分工负责。此 UI 测试不等于设备端运行验收。
- 状态数 5→2→5 的原生复核证据位于 `../device-analysis/lamp-retention-r1-*.json`；针对性测试已经更新为删除额外状态文本，并另验单纯双/多态切换保留草稿。
