# 阶段7 S07-05：受控Modbus RTU事务与回放切片

2026年11月14日。本批新增独立QtCore模块及QtTest测试，不开启生产在线连接，不连接串口、USB、socket或用户PLC。代码标记为`20261114-6 add`。

## 已实现边界

- `SimulationModbusTransport`要求明确站号1..247；广播和保留站号在打开回调前拒绝。
- 地址是标准PDU的零起始16位偏移；不把原厂`3x/4x`文本、通道、native node或站号推断为同一层映射。
- 功能01读线圈1..2000位，03读保持寄存器1..125字，05写单线圈（API逻辑值0/1），06写单16位寄存器。
- CRC16初值ffff、多项式a001、CRC低字节在前；PDU地址和字数据采用网络字节序。
- 有界FIFO默认32项、可配1..1024项，入队复制快照，不发送；拒绝重复未发送请求标识和队列溢出。
- 每次`processNext()`只发送一次；整体期限默认1000ms、可配1..60000ms，涵盖写入和分片接收。没有自动重试。
- 接收必须取得明确完整帧边界后，才核验精确站号、功能、字节数量、总长度、CRC及线圈填充位。单写需要精确回显地址和值。
- 只有整帧校验通过才一次性发布`values`或`writeAcknowledged`。错误、超时和从站异常会断开、清空未发送队列，并保留第一错误。
- 写回调一旦调用，即无法证明从站完全未执行；失败结果保留`remoteStateUnconfirmed=true`，没有回滚承诺。测试真实模拟了“从站已写入但应答地址错误”的状态。
- 写、读回调中的退出只清未发送队列，待当前回调返回再close；open/close转换期间拒绝重入连接、事务和入队。

## 接口和调用约束

公共头为`src/simulation_modbus_transport.h`。用`connectToStation(station, io, options, error)`打开注入会话，再`enqueue(request)`、`processNext()`逐项执行，最后`disconnectFromStation()`。析构也会关闭并丢弃未发送工作。

`SimulationModbusIo`含`open`、`write`、`read`、`close`四个回调。write返回实际字节数量；read返回`bytes/endOfFrame/timedOut/error`，支持任意分片，完整字节和帧结束事件也可以分两次返回。endOfFrame由受控回放提供；将来若接实体串口，需要串口后端实际测量RTU静默间隔并隔离旧帧，不能把“收够预期字节”直接当帧结束。

该类为同步单owner接口。回调遵守传入的期限，不抛异常；取消无法在阻塞回调结束前抢占。生产UI若将来使用，须由后续串口后端和工作线程承接。本批没有新增该后端或打开入口。

`simulationModbusResultToJson`显式写入`scope=controlled-modbus-rtu-replay`及`physicalEndpointUsed/hardwareTransferVerified/hmiBehaviorVerified=false`。协议回放通过不代表实际串口T或实屏E。

## 既有定义核查

`plc_address.cpp::plcProfileHasModbusCodec`只允许两个同源哈希的默认`Modbus_RTU.ini`。目录原始默认项为RS485、9600、8数据位、Parity原始值1、StopBits原始值1、DefaultStation=1、DefaultTimeout=1000、DefaultRetryTimes=3。这些仅是原厂配置元数据；本批没有把Parity=1翻译成PC串口奇偶校验，也没有采用自动重试3次。native地址node继续独立于站号。

在既有src文件中查询`QSerialPort/QtSerialPort`没有结果；本批不声称已有阶段9串口接口。原厂native地址族的编码边界也不能直接当PDU地址边界，例如现有1x离线编码允许的额外65536不成为合法16位PDU偏移。

## 验证

MinGW GCC13.1.0、Qt6.12.0 Core+Test、C++17，`-Wall -Wextra -Wconversion`编译0警告。首轮91项通过；自查发现open/close回调重入边界后补防护及测试，最终r2为97通过、0失败、0跳过（含Qt初始化和清理两项）。

测试包括独立CRC黄金向量、完整请求黄金帧、站号/地址/数量/逻辑写值边界、最大响应、逐字节分片、站号/功能/数量/长度/CRC/回显损坏、9种标准异常及未分配码、短写/失败/超时、首错保留、退出清队列、析构、期限、无完整帧边界不能成功、写后读从站回放及远端已写未确认。

原始日志为`configure-r1.log`、`build-r1.log`、`test-r1.txt/xml`、`build-r2.log`、`test-r2.txt/xml`。`modbus-contract-report.json`包含源码/可执行文件/报告身份、接口规则和全部测试结果。首次测试命令把`,txt`及`,xml`拆为独立参数而未执行测试，修正参数后运行通过；不是代码失败。

## 继续保留的未完成项

真实串口端点、RS485时序、原厂在线仿真协议与地址映射、机型/通道/站号路由、PC端口设置、生产在线连接窗口与工作线程、实际PLC通信T、HMI实屏E均未完成。多写、输入/离散读取、ASCII/TCP、双字字节序及其他PLC驱动也不在本切片内。后续接入需保留这些边界，不能因纯回放成功统一关闭S07-05或阶段9任务。
