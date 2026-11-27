# V122 离线介质 load.bin 文件与独立生成规则

原厂 V5.1 写出的 load.bin 描述目标面板、画面尺寸、工程宏、驱动引用和编译时资源。Lite 可以独立生成这份元数据。比较基准是原厂编译生成的完整文件，包含全部字符串的长度前缀、空值编码、列表顺序和最后一个计数。

## 从原生源工程生成

在编译上下文 JSON 中填写原厂同源编译散件目录和编译选项。下例为本次 440 个归档、实际 7 英寸 AMZ 安装案例及安装器六页 AMZ101 工程经过验证的编译配置。目录应包含同源的 SheetPara.dat，含动态控件的工程还需 ControlPara.dat。目录中 Modbus_RTU_com1.bin 的有无决定此配置下的驱动名称。输出目录应已存在。

```json
{
  "compiledDirectory": "D:/Projects/MyPanel/compile",
  "assetPairs": [],
  "downloadCharacterLibrary": false
}
```

```powershell
.\KinSealStudioLite.exe --build-native-load-metadata 工程.sm 输出目录/load.bin 编译上下文.json
```

引擎从源解析工程宏的名称和顺序，为每项追加 `.bin`。COM1 为 Modbus_RTU、第二端口无设备的已验证配置中，若同源目录有实际驱动则写 `Link/serial_amz/MODBUS/Modbus_RTU_com1.bin`，否则写 `Link/serial_amz/MODBUS/_com1.bin`。在 440 个归档中，编译状态计数 +0x688 与 ControlPara.dat 中类型 35 消息对象数量逐项相同。散件推导使用固定 640 字节控件记录，且按普通画面数量逐页读取 72 字节 SheetPara.dat 记录。每页 +48 和 +50 的小端 16 位宽、高应与 SM 对应画面尺寸一致。各页 +2 的小端 16 位动态控件数之和乘以 640 必须等于 ControlPara.dat 长度。未验证的现场驱动解析、资源对表或其他编译状态应使用对应编译器实际提供的参数。

具备原厂编译上下文时，也可直接提供 `driverPaths`、`compilerCounter688`、`assetPairs`、`compilerCounter838` 四项，省去 `compiledDirectory`。独立写端可处理原厂成品没有附带散件名称的情况，还支持没有源工程、由调用方提供完整元数据的 `--build-native-load-metadata 元数据.json 输出目录/load.bin` 入口。下面表格列出完整 JSON 所需的字段。运行和回归均不发送 USB 数据。

## 字段顺序与来源

写端入口为官方 KinSealStudio5.1.exe 的 0x00635ABC，由 0x006422DB 传入 compile\\load.bin。原厂文件 SHA-256 为 `305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c`。主体自 0x00636C9E 写标识至 0x0063704D 写末尾计数。

整数和浮点值采用大端字节序。QString 的四字节前缀记录 UTF-16BE 正文字节数。JSON 的 null 写为 `FF FF FF FF`，空串写为 `00 00 00 00`。每张 QSizeF 画面宽、高为两个大端八字节浮点，布尔值各一字节。所有列表先写大端 32 位有符号项数。

| 顺序 | JSON 字段 | 类型及来源 |
|---|---|---|
| 1 | 写端常量 | QString `KinSealStudio-V1.0.2`、整数 122 |
| 2 | model、pageCount | 系统面板型号 identity.1、普通画面数 |
| 3 | startupType、startupImage | 单字节启动选项及 QString，对应 system.tail.0、system.tail.1 |
| 4 | flags | 正好四个布尔值。依次来自 config.flags.4、config.head.18、config.head.16、config.head.15 |
| 5 | macroCount、macroFiles | 工程宏数，再写带独立项数的宏文件名列表，与 history.1e4 的原有顺序一致 |
| 6 | driverPaths | 驱动路径 QString 列表，驱动解析发生在编译时 |
| 7 | assetPairs | 原厂资源对表。每项顺序写两个 QString，原件由 0x00636F27 和 0x00636F40 写出 |
| 8 | panelType、optionText | 32 位面板参数、可空 QString，对应 system.identity.3 和 config.head.19 |
| 9 | compilerCounter838、connectionCount、compilerCounter688 | 三个 32 位整数。首项是下载中文字库开关 +0x838，第三项为编译器 +0x688 消息计数，中央一项为 system.16c 的通讯端口数量 |
| 10 | pageSizes | QSizeF 列表，每项 [宽, 高]，取自原生页头 head.2、head.3 |
| 11 | historyCount | 32 位历史数据配置数量，取自 config.cc |

`assetPairs` 的非空条目业务规则及 `downloadCharacterLibrary=true` 尚缺实际勾选保存的独立原厂文件。完整 JSON 可以编码调用方给出的相应值，写端不会根据旧 load.bin、Usbload.bin 或最终 KNS 反推它们。对二维码等控件的原厂源支持、ControlPara.dat 设备编码和这份介质元数据分别验收。

## 下载中文字库选项

两个原厂下载对话框构造分支分别在 0x005D98ED 和 0x005DEA99 将 +0x838 清零。下载对话框元对象的第 5 项从 0 计数，对应文本 `on_isDownLoadWord_clicked(bool)`。其分派于 0x00B812C8 调用 0x005D5DE8，后者将收到的一个字节扩为 32 位整数并写回 +0x838。运行分支 0x009EDBE9 和 0x00A8A9B8 在此值为零时立即返回，其余路径出现 “Download Chinese character library” 及 “Abnormal Chinese character library file”。因此有源工程的入口可以省略该选项，显式开关用 `downloadCharacterLibrary`。兼容参数 `compilerCounter838` 仍可提供，两个参数同时出现会报错。生成此元数据没有发送字库或固件。

## 完整字节对照

| 来源 | 同源输入 | 结果 |
|---|---|---|
| Git e3b72657 的 440 个原厂目录 | 431 个直接保留原厂 input-native.sm，另 9 个从同目录 upLoad.knr 还原 SM；使用独立的画面描述、控件记录及驱动文件，外部编译状态 0 和空资源对表 | 319 个不同输入组合，440 份 load.bin 全文件相同。412 份 233 字节，28 份 253 字节 |
| 本机原厂安装 AMZ070W01 | 从 upLoad.knr 还原原厂 SM，SHA-256 为 `898343a29a64ad801d2f723eca9f21fd8838b1436fca26b28448f49f904fd73e`，散件含 COM1 驱动。SM 的宏名为 up_test | 新生成 257 字节，与安装原件逐字节相同 |
| 原厂安装器内 AMZ101W01 六页 KNS | 仅取 upLoad.knr 还原 1,438,059 字节 SM（SHA-256 `c2669be583def49d08c4a6f56a76421f4b86a43320faca979e3393567b755665`）、432 字节 SheetPara.dat、157,440 字节 ControlPara.dat 和 COM1 驱动。六页各为 1024×600，动态控件数依次 33、33、49、49、49、33 | 全文件 343 字节相同，SHA-256 `8266b014d50b107ccc54060155ac2937a16df119b6011efe07ad3db922145d3c` |
| 手工构造两页完整 JSON | 传入两种尺寸、非整数尺寸、中文、多项 QString、空串和空值，另以 Python struct 独立编码期望文件 | 全部字节吻合；5 种非法输入均返回错误且保留原输出 |

可用下列命令复验 440 个原厂目录。原厂 load.bin 仅在新文件生成后比较。临时夹在 build 下创建，退出时清理。查看每份目录输出可加 `--report 结果.json`。

```powershell
py -3 tests/verify_native_load_metadata.py build/amz070w01r/KinSealStudioLite.exe
```


指定 `--native-kns 原厂介质.kns` 或 `--native-compile 原厂散件目录` 可随上述归档复验额外原件。有源工程的测试从编译散件推导消息计数和驱动名称，源目录不含任何旧 load.bin。安装案例的额外检查包括修改末页宽度、改动末页控件数均被拒绝，目标文件保留，以及勾选字库仅改变大端开关字段的最后一字节。该勾选对照的期望值来自原厂函数和已确认的字段位置，独立实测原件均使用未勾选值。
