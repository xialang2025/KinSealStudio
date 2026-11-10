# 阶段1：主窗口与后端接口只读复核

日期：2026-11-10。复核者：`/root/function_ui`。修改标记：`//20261110-5`。

本轮只检查源码与已有日志，未修改业务逻辑、未构建、未启动窗口、未访问USB或PLC。配套[JSON记录](backend-read-only-review.json)保存审阅输入SHA、定位和建议回归；以下行号对应审阅时文件身份，后续修复可能改变行号。

本轮发现6项需要处理的接口问题。主代理已收到具体风险；本报告保留修复前事实，不把通知或修改计划当作问题已经关闭。问题均为可定位的静态控制流推导，未声称本轮实机或自动化复现。

## 发现

### R-S01-BACKEND-001：旧单文本编译绕过已知设备配置守卫（P1）

定位：[independent_downloader.cpp:577](../../src/independent_downloader.cpp#L577)、[同文件:641](../../src/independent_downloader.cpp#L641)、[同文件:1400](../../src/independent_downloader.cpp#L1400)。

`buildSingleTextPayload`新增了`validateSinglePageBackend`，但该模型接口只证明单页拓扑、有效实体与引用，不证明默认PLC配置或原生配置指纹。旧编译函数没有调用`DeviceAddressCompiler::prepare`或`nativeProjectUsesSupportedDeviceDefaults`，随后直接读取默认十段模板。`--compile-single-text`、`--download-single-text`和`--download-native-test`均能走这条路径。

最小输入为一个合法单页文本工程，添加合法但未被设备编译恢复的COM2连接或非默认站号。模型可以保存，普通`buildProjectPayload`会拒绝，旧单文本入口则没有相应拒绝条件；合法但非默认配置的原生快照同样被模板忽略。固定TEST快速返回也不补配置校验。结果是用户配置未被编码，仍可能获得成功编译及继续传输的结果。

建议回归：分别用COM2、非默认站号、未确认node、合法非默认系统快照，比较通用入口与全部旧入口；均须在打开USB前拒绝，`payloads`哨兵及既有输出目录保持不变。原始默认TEST必须继续逐字节一致。允许有界旧入口直接拒绝无法表达的配置，不要求临时恢复新协议。

### R-S01-BACKEND-002：固定原厂源导出接受并丢弃更丰富模型（P2）

定位：[native_test_project.cpp:34](../../src/native_test_project.cpp#L34)、[main_window.cpp:832](../../src/main_window.cpp#L832)、[main.cpp:407](../../src/main.cpp#L407)。

`exportNativeTestProject`的新增拓扑守卫通过后，仍只检查目标、对象数量、旧文本属性、固定几何及四字符文本。它没有拒绝额外语言、显式启动语言、连接、宏/图库或现有原生快照，而输出始终来自固定单文本源模板。这些实体在当前模型中可以合法存在。

最小输入：保留默认固定位置旧文本，文字`TEST`，设置两种语言并将启动语言设为第二种；旧源导出条件均满足，但生成源不包含这组语言或启动选择。CLI`--export-native-test`直接可达；GUI无snapshot、无按钮时也优先尝试旧导出，成功后不会再走typed源codec。

建议回归：对额外语言/启动选择、连接和活动配置各做旧导出拒绝测试；目标已有文件字节不变。只有完整语义被旧模板表达时才允许成功，其余应转typed导出或明确拒绝。K层纯编辑元数据是否允许丢失须单独声明，不能据此忽略活动语言/配置。

### R-S01-BACKEND-003：连接UI删除未检查显式引用（P2）

定位：[static_text_actions.cpp:263](../../src/static_text_actions.cpp#L263)。

`configureConnections`仍只调用`candidate.validatePlcConfiguration`。该旧检查会验证全部页面的`@connection:address`，但不解析新增`HmiReference`图。若某连接只被一个显式稳定引用使用，控件没有限定外部地址，删除连接后旧检查可以通过，随后`m_project.connections`被直接替换，显式引用悬空；全模型保存才拒绝。

建议回归：建立合法连接及`ownerKind=project,targetKind=connection`引用，通过真实连接窗口尝试删除。操作应拒绝且模型、动态`plcConnections`属性、脏标记及画布保持原值；取消也不提交。再覆盖隐藏页限定地址仍被使用的路径。

### R-S01-BACKEND-004：按钮资源先提交，后续拒绝无法回滚（P1）

定位：[static_text_actions.cpp:194](../../src/static_text_actions.cpp#L194)，同类创建路径在87、115、142、169行；[main_window.cpp:860](../../src/main_window.cpp#L860)。图库删除实现参见`bit_button_dialog.cpp:1473`。

按钮对话框确认后，调用方先把`dialog->macros()/pictures()`直接赋给`m_project`，再更新画布并等待`documentChanged`进行`validateModel`。若资源变化破坏新增引用，`addSelectedControl`虽然拒绝对象候选，却从已被资源赋值污染的`m_project`恢复画布，无法恢复原资源。

实际可达刺激：一个项目图片被显式`HmiReference`引用；在任一按钮的工程图库中右键删除该图片，确认图库与属性窗口。图库确认允许图片列表为空，对话框接受仅检查控件局部字段。编辑路径随后发现全模型引用悬空，但原项目图片已经丢失；创建路径则可能尚未进入画布`documentChanged`就保留无效项目。原.ksp文件尚未被覆盖，但当前工作模型不再满足可保存契约。

建议回归：按钮编辑和四种创建入口各验证“删除被引用图片→确认”事务。图片、宏、对象、显式引用和选择须一起保留或一起提交；未提交绘制/取消拖框时也不能把拒绝的资源改动留在工程里。使用完整工程候选验证后提交，不能只恢复画布对象。

### R-S01-BACKEND-005：迁移CLI可把JSON写为.sm并覆盖原厂源（P1）

定位：[main.cpp:303](../../src/main.cpp#L303)。

新`--migrate-project input output`直接将用户给定输出路径传给`HmiProject::save`，没有限制输出为`.ksp`。`save`按JSON编辑格式写入，而`load`按`.sm`后缀选择二进制原厂源导入。因而输出`.sm`时命令可以报告迁移成功，结果下一次无法按同一路径打开；若输出等于输入原厂`.sm`，还会原子替换掉原生源。

建议回归：`input.ksp→output.sm`、`input.sm→原路径.sm`必须拒绝，原文件字节不变；`.sm→新.ksp`必须保留原生源且生成可重开的K文件。允许明确提供`.ksp→同一路径.ksp`原子迁移，但不要用模糊后缀写入可误认格式。

### R-S01-BACKEND-006：能力报告遗漏确定的编译阻塞及原因（P2）

定位：[project_capabilities.cpp:36](../../src/project_capabilities.cpp#L36)、[同文件:72](../../src/project_capabilities.cpp#L72)。

`deviceCompile`候选只检查模型、单页拓扑、目标、对象类型、snapshot默认指纹及数量。COM2、非默认站号或node未确认等确定被`DeviceAddressCompiler::prepare`拒绝的连接状态未纳入查询，故会报告`candidate`，而该状态并不存在可成功执行的设备路径。`candidate`声明仍需实际编译是合理边界，但不能替代已经明确掌握的阻塞条件。

另一个确定诊断遗漏：合法`number/image`对象或空工程会得到`blocked`，但`deviceCompile.reason`只取`backendError/defaultsError`，两者此时都可能为空。`nativeSource`的对象类型原因没有正确传入设备分支。

建议回归：默认单页五类继续为候选；COM2、站号/node不支持应阻塞并返回具体原因；空对象、未恢复对象类型及非默认快照均要独立定位原因。查询继续保持纯离线，不因查询访问USB或运行宏工具链。

## 未发现同类缺口的路径

- `selectTreeObject`在重建树前已复制对象ID和页面ID；重建后没有继续解引用旧`current`指针。`rebuildProjectTree`期间关闭树信号，选择变化不会直接递归进入切页函数。本轮未把推测的Qt内部行为写成UAF发现。
- 切页先把当前画布对象合入工程候选，再经`selectPage`完整验证并提交；活动页与启动页分离。现有真实MainWindow测试覆盖双页切换及隐藏对象保存，见既有日志。
- 语言设置使用`setLanguages`工程候选，功能13按旧语言名称在全部页面重映射。删除被显式启动语言或按钮目标引用的语言会拒绝；翻译草稿不被清理。编辑语言选择不会覆盖启动语言。
- 通用`buildProjectPayload`、地址扫描准备、栅格请求及固定下载入口均有新的多页/角色守卫。`downloadProject`先完成编译才调用设备传输；未发现绕过这些守卫而只下载所选页面的通用入口。
- CLI能力查询、typed源结构检查与迁移均无直接USB调用；`.ksp`保存使用模型验证与`QSaveFile`。整目录编译输出仍是既有逐文件提交，已被A05列为阶段5边界，本报告不重复当作本次新增缺陷。

## 既有验证与限制

主代理已保留的[模型日志](model-tests-first.txt)记录53通过、0失败；[后端日志](backend-tests-first.txt)记录8通过、0失败，含隐藏页保存检查。它们没有覆盖上述六项完整刺激，不因这两份日志通过而关闭发现。本轮未重跑测试，也未检查仍由源codec代理负责的typed内部实现。

`configureLanguages`和页面切换使用`ScreenCanvas::setProject`，会清空当前画布撤销历史；本轮把它作为当前UI行为记录，未凭原生缺证宣布其已等价，也未在缺少契约时将其单列为必修逻辑缺陷。后续若开放工程级撤销，应把语言列表、重映射后的按钮及资源一起记录，不能恢复旧语言索引而保留新列表。

恢复点：由主代理逐项修复并补建议回归，追加处理结果与新SHA；本报告的静态发现保留为修复前记录，不覆盖成虚假的初检无问题。
