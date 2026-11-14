# 阶段9：PLC地址模板与通信实现说明

日期：2026-11-14。修改标记：`//20261114-8 add`。本说明对应[阶段9任务书](../design_plan/10_阶段9_PLC地址模板与通信.md)与[机器进度](stage9-evidence/progress.json)。

本批新增协议文件身份审计、地址窗口提交保护、8功能标准PC RTU核心、显式PC串口会话/窗口及原厂Modbus DLL静态证据。阶段9仍是部分交付：S09-01～05为partial，S09-06为open。测试目标和复核发现已列出；正式构建、Qt受控与portable结果由Root封存，本文不预填通过数量。没有本批原厂DLL执行、真实PLC串口读写、HMI→PLC运行或下载验证。

## 1. 实现范围和四层身份

| 层 | 当前范围 | 不能据此推定 |
|---|---|---|
| 地址模型 | 目录、连接、原生语法有限子集及地址窗口；同一parser校验草稿与提交 | 某地址可保存就能访问真实PLC |
| 设备模板 | 既有默认AMT COM1/站1/node1、单连接单页的36B槽和扫描 | 其它profile/端口/多站或完整DLL ABI已支持 |
| PC通信 | 标准FC1/2/3/4/5/6/15/16、受控I/O接口及显式QSerialPort适配器 | 自动映射工程族/route/node、原厂在线调度等价或真实PLC通过 |
| HMI原固件 | 本批没有新实物读写结果 | PC回放、USB工程下载等于HMI到PLC验证 |

设备36B槽、12B扫描记录与Windows DLL描述块是三种独立结构。PC串口端点也是独立于HMI COM1的端点，输入为明确站号和零起始PDU地址。route、node、station的来源不能合并。

## 2. 协议能力与139源审计

文件证据为 [protocol-file-audit.json](stage9-evidence/protocol-file-audit.json)，对应KinSealStudio V5.1安装原件和AMT070W124-L800X480R目录绑定。

| 项目 | 本批审计值 | 解释 |
|---|---|---|
| profile源 | 139 | 全部源INI存在且哈希与目录一致 |
| 源哈希匹配 | 139/139 | 文件身份匹配，未执行驱动 |
| 历史存在性匹配 | 139/139 | 与已有存在性记录一致 |
| AMT目录适用profile | 70 | 目录绑定，不等于70协议已通信 |
| 去重后观察文件 | 444 | 只读完整哈希，无DLL运行 |
| DLL存在的profile引用行 | 106 | 同一DLL可被多行引用，不能当唯一DLL数 |
| COM1/COM2 bin存在的引用行 | 65 / 65 | 分端口引用行计数，不是功能实现数量 |

默认profile为 `Link/serial_amz/MODBUS/Modbus_RTU.ini`。其源INI SHA256为 `78fdc0aa284d545c0a1a55bfa98fa0bb3bb49e464f670212996ee818eb01a477`；Windows DLL为 `169e6dcc0cdf5b57d002bb09d947952c157e6cf2adf52eda9f33dbe7f74906f5`；AMT COM1 bin为 `fddf11ab2cfec2b06fcdb408de1bb82fbe26707a97a9c2b2b4b0be382bc0290c`，COM2 bin为 `87a645158c53e41740a782a2a193f320b3a175977f901de57710a8fc9a475963`。bin存在不取消设备编译的COM1限制。

原INI的 `Parity=1` 等字段保留raw值，不直接解释成QSerialPort枚举。PC窗口的无/偶/奇对应Qt 0/2/3，是明确产品输入；不能把原INI的1直接赋给它。

[plcProtocolCapabilities](../src/plc_protocol_capabilities.cpp)从嵌入的目录和审计快照生成能力表，并核 `catalogSha256` 匹配。每行分开表达目录可见、连接元数据可保存、地址语法覆盖、受限设备编译、PC工程映射、完整原厂ABI、物理PLC与HMI联动验证。后四项不能由文件存在或目录数量自动变为true。

纯读取CLI为 `KinSealStudioLite.exe --plc-capabilities [model]`，省略model时使用AMT070W124-L800X480R；只向stdout输出JSON，不创建报告文件，不开端口。Root最终能力快照拟保存为 `docs/stage9-evidence/S09-protocol-capabilities.json`。文件审计的model固定为其归档AMT身份，查询其它model不能把它解释成其它型号的实物证据。

## 3. 连接窗口的输入与提交规则

实现入口为 [editPlcConnections](../src/plc_address_dialog.cpp)。输出是调用方连接向量，编辑期间只操作局部副本。

1. 在创建对话框前验证输出指针、目录加载与型号候选；已有连接只允许不重复、与端口一致的com1/com2标识。失败保留原向量。
2. 确认时验证当前型号可用的profile及十进制站号字符串，再构造staged向量；不把此字符串直接当PC RTU的1..247站号，也不由station自动生成node。
3. 更换profile会失效原nodeKnown/node；已有node证据与station保持独立。仅已核默认COM1/指定profile/站号字符串“1”可按既有oracle恢复node1。
4. 只有按钮的验证路径置 `confirmed=true`、对话框返回Accepted且owner仍存在时，才一次性替换输出。取消、验证失败、owner/子窗口被销毁或外部直接调用accept不提交。
5. 使用QPointer守护对话框和parent，避免嵌套事件循环结束后访问已删除窗口。parent销毁后不解引用调用方可能随parent失效的输出。

该事务修复未扩大设备支持范围。连接从工程中删除后的引用处理、全协议原生字段及原厂源格式对照继续按各自证据推进。

## 4. 地址窗口的UI→模型→设备规则

入口为同文件的 `editPlcAddress`。显式连续传送数量当前只接受1或2，传入统一 `PlcAddressContext.transferCount`；用于完整跨度校验，不能只检查首地址。

| UI动作/状态 | 处理规则 | 提交结果 |
|---|---|---|
| 初始地址非法 | 保留可识别的连接、最长匹配族及原文本，显示parser错误 | 不自动覆盖为合法值 |
| 输入修改 | 实时调用与确认相同的parsePlcAddress，错误文本保持可编辑 | 非法草稿禁用确认 |
| 更换存储器/族 | 按该族原生初始值规则重置，重新校验；未知语法明确不可确认 | 不因目录可见而放行 |
| 切换进制 | 按旧进制完整解析，再生成新进制文本；失败恢复旧进制 | 不做有损文本转换 |
| packed位地址 | 已核标准族A.B中的位号0..15，位号始终十进制 | numeric转换遵循已核族规则 |
| 传送数量2 | 显示连续数量并检查尾地址；内部LW本版起点上限4095 | 不允许尾部跨界 |
| 确认 | 再解析到局部committed，随后置confirmed | 验证通过后一次写回规范限定地址 |
| 取消/销毁/外部accept | 原输出文本不变 | 不发布未校验结果 |

当前内部LB/LW范围0..4096是项目上下文策略，不宣称芯片容量。`_dw`命名不在通用解析器中自动将显式N乘二；已核默认族packed numeric与特殊1x边界继续复用已有codec，非默认语法只按已恢复子集放行。

设备编译仍经 [DeviceAddressCompiler](../src/device_address_compiler.cpp) 的prepare/encode门槛，只开放单连接、默认AMT Modbus INI、COM1、station字符串1、nodeKnown且node1。36B槽已核位置是route+0、family+1、node+2、通用标志+5、LE32 numeric+8；其余由专用调用方模板负责，不能统一假定零。

本批Qt事务行为不能代替V5.1原生窗口U对照，也不能代替原厂 `.sm` 源格式或设备编译C的新增差分。

## 5. 独立标准PC RTU的输入与帧规则

核心为 [simulation_modbus_transport.h/.cpp](../src/simulation_modbus_transport.h)。它不加载原厂DLL、不提供串口本身，也不解释工程family或设备扫描表。标准帧由显式站号、零起始地址和数量构造。

| 功能码（十进制） | 操作 | 数量限制 | 值限制 |
|---|---|---|---|
| 1 / 2 | 读线圈/离散输入 | 1..2000位 | 不携带写值 |
| 3 / 4 | 读保持/输入寄存器 | 1..125 word | 不携带写值 |
| 5 | 写单线圈 | 1 | 逻辑0/1编码成0000/FF00 |
| 6 | 写单寄存器 | 1 | 0..65535 |
| 15（0F） | 写多线圈 | 1..1968位 | values数量相等，每值0/1；末字节高位清零 |
| 16（10） | 写多寄存器 | 1..123 word | values数量相等，每值0..65535 |

站号只接受单播1..247；地址0..65535且 `address+count<=65536`，拒绝广播、越界或绕回。请求id不得为空、长度最多256；批次不允许重复id。多写使用独立values，旧单值字段必须为0；读/单写不允许混入多值。

多字节PDU字段高字节先，CRC初FFFF、反射多项式A001，线上低CRC字节先。该独立标准实现与原厂静态CRC算法一致的观察，不等于已执行原厂DLL做oracle。

接收必须有明确帧结束边界，完整检查station、function、长度/byteCount、CRC；写应答前6B须匹配请求的station/function/address/value或quantity。读位末字节未使用高位必须为零。异常码只接受本子集支持的1..6、8、10、11并返回失败状态。所有检查与最终取消/期限门槛通过后才一起发布values或writeAcknowledged；失败清除部分读值。

队列有界，核心maxPendingRequests为1..1024；手动批次使用256。请求失败会丢弃尚未发送项并关闭会话，不自动重试。已经尝试写入时，失败/取消不能承诺回滚，`requestMayHaveReachedSlave`和`remoteStateUnconfirmed`必须保留。中途失败前已确认的前序回执仍属于其原事务，不能把整批描述为已撤销。

## 6. PC串口适配器、预算与释放

[plc_serial_session.cpp](../src/plc_serial_session.cpp) 的 `runPlcSerialBatch` 先校验settings、1..256条请求及所有id，再创建factory/打开端点。晚项无效也不能先执行前项。注入factory用于受控测试，默认factory才是QSerialPort；两者回执来源分开。

| 参数 | 当前允许范围 | 来源 |
|---|---|---|
| PC端口 | 非空、无首尾空白、最多256字符、无控制字符 | 用户显式输入/选择 |
| 波特率 | 50..4000000 | PC产品配置，驱动还可拒绝实际不支持值 |
| 数据位 | 固定8 | 本RTU有限子集 |
| 校验 | 无/偶/奇，Qt枚举0/2/3 | 不解释原INI的Parity枚举 |
| 停止位 | 1/2 | 显式PC设置 |
| 单事务期限 | 1..60000 ms | UI可编辑 |
| 批次总期限 | 不小于单事务，最多120000 ms | API参数；UI当前默认120000 |

QSerialPort在执行runPlcSerialBatch的worker线程中创建、使用和销毁，不跨线程调用close。默认配置9600/8N1与1000ms是Lite明确采用的PC初值，不宣称来自原厂窗口或INI枚举释义。

读写等待以最多20ms切片轮询取消与剩余预算。无数据的一次切片不能作为空进度返回给核心；接收回调持续等到字节、RTU静默、超时或取消。RTU结束间隔在低速按3.5字符向上取整毫秒，高于19200时采用2ms；它是本适配器规则，未宣称原厂调度完全相同。

取消令牌为shared atomic。总期限和取消在I/O前后及最终接纳处判定；同步系统调用必须先返回才能清理，不能把该轮询描述为可强行中断系统调用的硬实时保证。没有自动重连或重发写命令。

回执分开记 `opened`、`sessionInactive`、`closeAttempted`、`closeConfirmed`与`closed`。会话不可用不等于端口释放成功；已打开后仅close确认成功才能记closed。打开失败的callback按契约自行释放其已取得资源。close失败不会覆盖原来的事务错误，但会阻止批次ok。

## 7. PC窗口的使用流程与生命周期

入口 [MainWindow::showOnlineSimulationOptions](../src/main_window_simulation.cpp) 打开 [PlcOnlineDialog](../src/plc_online_dialog.cpp)。PC端口列表只枚举，初始不选端口。用户选择/输入PC端口、参数、站号、功能、PDU地址、数量/写值，再执行一次请求；界面不保存到工程连接，也不隐式推断HMI端口。

执行前完成UI数值解析与核心构帧校验；运行时冻结全部请求输入，只保留停止操作。单写数量固定1，其它数量随功能限制；写值按十进制逗号分隔（空项、符号拒绝；单项两侧空白可trim），不能靠整数截断接受非法字符串。请求和settings复制进worker，期间修改工程不会改变该事务。

worker只持shared job和参数副本，不捕获widget。普通关闭先置取消并等finished，再关闭对话框；owner直接销毁时析构仅置atomic，Qt自动断开以this为上下文的finished回调，工作对象继续完成清理。QThread独立于widget parent，finished后deleteLater。该生命周期仍需正式GUI受控测试封存，不能只凭静态检查标通过。

结果显示完成/错误、发送和接收hex、完整值列表、写应答或未确认写、串口释放状态。2000位读值放在同一文本块，避免把缩进JSON超过2000块后截去头部。`lastReport()`仍保存完整结构化回执。受控factory的scope为 `controlled-rtu-session`，真实PC端口分支为 `pc-serial-response`；两者都不自动声称HMI执行或原厂DLL等价。

## 8. 原厂DLL的实际静态边界

完整字段和原样证据见 [native-abi.md](stage9-evidence/native-abi/native-abi.md)、[native-abi.json](stage9-evidence/native-abi/native-abi.json)。本批新增12函数、4项xref与CRC精确指令，历史3函数不重跑。以下仅为D：

- DllFun依次传七个全局指针槽的地址；其中TX、描述块、RX/原位解码、bulk源有观察，后三槽用途未知。
- 已识别17项描述字段、operation分派、FC1/2/3/4/5/6/16、读改写和bulk分支。原厂读位 `desc.quantity<<3`，不能直接用标准bitCount替代。
- CRC初FFFF、polyA001、u16长度参数、AX返回和低字节先已由182B精确指令核实。
- 响应校验按 `desc+0x204+2*j`取长度，但生成首项在+0x206，+0x204本身是帧数；零起始j可能触发下溢条件。宿主是否先改为一索引尚未知，不能宣称真实原厂必触发，也不能直接运行探测。
- 七缓冲容量/生命周期/callback约定、响应索引、family63 bulk不对称载荷引用及两帧调度未闭合；完整ABI保持false。选定响应链只见CRC不能推定宿主其它层没有station/function等校验。

REA目标已关闭。原生bundle导出因当时父目录不存在失败，随后保存完整inline Evidence为一个gzip归档；没有冒记导出成功，也没有为补导出重开目标。归档不含DLL/SDK/沙盒副本。

## 9. 扫描与资料传输

既有扫描继续保持前200B为u8页面段数、仅第0页、每页最多255段、每记录12B的规则。族固定顺序、route/node分组、packed对齐/间隙和DWORD奇数调整见[USB协议第15节](AMT070W124-L800X480R_USB下载协议_已验证部分.md)。不替换为通用区间合并，不因纯写地址合法而自动订阅。

历史扫描37项和Common oracle31项未重跑为本批新通过。活动读源仍只认已核启用显现/触控、指示/监视、位交替和字算术等调用；采集/报警/宏变量/资料传输、多页拥有关系与PC自动扫描仍待核。

资料传输只有 [H帮助选段](stage9-evidence/native-abi/data-transfer-help-extract.json)：秒计时/位为1触发，位/字/双字，长度1..32，源/目标同类型，以及同PLC限制和COM1→内部寄存器→COM2两段中转表述。不能由“位为1”推定触发沿或重复频率，也不能把同PLC限制扩成所有同COM均禁止。

入口 `TREE.TRANSFER.NEW` 与问题 `Q-S00-ENTRY-TREE-16`仍未关闭。模型 `dataTransfer` 类别不证明配置表、扫描、编译或运行已实现；真实V5.1窗口、确认/取消/保存重开、源/设备差分与运行失败策略是下一链路。

## 10. 复核、测试目标与封存

[serial-ui-r1复核](stage9-evidence/reviews/serial-ui-r1.json)发现5项：close签名不匹配、20ms空轮询误判、inactive冒充close成功、回执scope冲突、2000值缩进JSON截断。Root已修源码，独立复看为修复候选；该复核没有代执行测试。

| 检查层 | 目标/来源 | 最终封存状态 |
|---|---|---|
| 地址UI事务 | [stage9_address_ui_tests.cpp](../tests/stage9_address_ui_tests.cpp)，KinSealStage9AddressUiTests | 26/0/0，windows-r1 |
| 标准8功能核心 | [simulation_modbus_transport_tests.cpp](../tests/simulation_modbus_transport_tests.cpp)，KinSealSimulationModbusTests | 340/0/0，windows-r1 |
| 串口会话注入 | [plc_serial_session_tests.cpp](../tests/plc_serial_session_tests.cpp)，KinSealPlcSerialSessionTests | 101/0/0，windows-r3 |
| PC窗口生命周期 | [stage9_online_ui_tests.cpp](../tests/stage9_online_ui_tests.cpp)，KinSealStage9OnlineUiTests | 31/0/0，windows-r3 |
| 能力CLI/文件身份 | protocol-file-audit与CLI stdout快照 | 139源/444唯一文件；CLI真实进程核验通过 |
| portable/回归 | 既有地址/原生目录/扫描/模拟与新模块测试 | 回归全部通过；便携7进程/36检查通过 |
| 原厂DLL oracle | 本批无执行 | 未通过/未运行 |
| 真实PC端点、HMI→PLC | 本批无新增 | T/E未关闭 |

正式归档采用test-summary.json逐目标身份、windows-r3最终UI/串口、portable-r2/checks-r1以及S09-protocol-capabilities.json。失败r1、独立RED和旧源码GREEN分别保留；最终包绑定真实代码提交，不覆盖失败历史。

## 11. 剩余工作与下一最小探针

S09-01补原生连接窗口字段与枚举释义，继续按profile/型号/端口逐行开放；S09-02补原生全协议字段、取消与源/设备差分；S09-03先静态闭合宿主索引、容量与bulk双字引用，再评估私有mock oracle；S09-04先核资料传输真实窗口和配置表；S09-05补工程变量映射、扫描/用户写/读改写调度及原生行为对照；S09-06待明确现场端点后完成AMT原固件到PLC，再选择下一个有实际需求的协议。

本批没有因增加目录、代码或受控回放而关闭完整ABI、真实PLC或HMI E。所有其它协议仍附具体支持层和限制，不能按相同厂商名或文件存在推定兼容。


## 正式封存结果

最终按目标取最新身份，704通过/0失败/0跳过，见[test-summary](stage9-evidence/test-summary.json)。最终串口/在线窗口/阶段7UI采用windows-r3，其余未变化目标采用windows-r1；windows-r1旧UI名称超时和独立非法输入RED均保留。静态最终复核四项候选已修，见[final-code-r1](stage9-evidence/reviews/final-code-r1.json)。

最终便携候选为`dist/KinSealStudioLite-Stage9-plc-r2`；r1在最终复核发现CLI刷新/临时目录及回执细节后废弃，未作为通过包发布。最终包启动、CLI与真实代码提交身份分别封存；完整阶段仍为5项partial/1项open，无PLC/HMI证据升级。UI规则24、USB协议97及统一台账已同步。


便携最终实际7进程/36独立语义检查通过，Qt6SerialPort实际从包内加载，已目视检查主界面及通信/能力截图。清理45个自有新中间文件共16,380,525B；废弃r1包122,439,021B删除被自动审批拒绝，0删除并保留，见cleanup-result.json。全部TEMP/TMP置Z，未重试历史拒绝路径。


阶段9应用程序提交`3be25a4c96deb0d37b09e57d181e314b59787fb9`，EXE `aee3c13a5b3ad36ea67c4d68e4033858b3f1d31e952dbf02520186819c962709`；6340源/资源、192包文件/164样例和主目录1891既存文件保护已核验。最终manifest `2b6e2a668fa84915378bb082f4fbc75e11a70f0d508fbd9d2fb92a202e2f10da`，详见docs/stage9-evidence/delivery-r1.json。首轮推送网络失败后已重试成功，GitHub main确认至79ccb915（含程序与验收元数据）；回执见docs/stage9-evidence/push-confirmed.json。仅阶段9证据设置Git -text以保留原哈希字节，应用输入/EXE/样例不变，无需重复已通过测试。
