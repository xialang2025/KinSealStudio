# AMT型号元数据的静态身份与原始字段

日期：2026-11-10。来源为原安装model.dat/modelInfo.dat；两者都是带UTF-8 BOM的XML。此记录采用普通Python文件解析，不是REA运行证据，也没有读回固件。

| 文件 | 字节数 | SHA256 |
|---|---:|---|
| model.dat | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| modelInfo.dat | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |

## model.dat

目标节点精确字节区间：[18195,18510)，摘录哈希`0d4585d5af378594a7f2c506e52e2bcd43e06f3b10b19a105e6efb55ca93f92f`。

| 原始标签 | 原始值 |
|---|---|
| `Seris` | `AMT` |
| `Inch` | `7.0` |
| `Pixels` | `800*480` |
| `Colors` | `1667M` |
| `Memory` | `8M` |
| `NFlash` | `16M(Nor Flash)` |
| `Voltage` | `DC24V(+/-15%)` |
| `COM1` | `true` |
| `COM2` | `true` |
| `USB` | `true` |

## modelInfo.dat

目标节点精确字节区间：[114539,116559)，摘录哈希`2d145a867229a9730aa84e5b58dd9240288964fee3c2590d333568421e27511a`。

| 原始标签 | 原始值 |
|---|---|
| `Device` | `76` |
| `USB` | `2` |
| `SerialPortDown` | `false` |
| `AsciiSize` | `410,410` |
| `AsciiSSize` | `320,288` |
| `NumberSize` | `320,288` |
| `HexSize` | `320,288` |
| `MessageFile` | `20000` |
| `TransBuf` | `40000` |
| `UploadSize` | `f0000` |
| `Pagenocolect` | `144000` |
| `Pagecolect` | `144000` |
| `SigleScreen` | `300000` |
| `SigleScreenBK` | `0` |
| `macrofileSize` | `20000` |
| `SiglecontrolSize` | `300000` |
| `maxScreen` | `200` |
| `maxControl` | `100` |
| `controlFileSize` | `350000` |
| `controlParaFileSize` | `1E0000` |
| `screenFileSize` | `310000` |
| `LogoSize` | `40000` |
| `addrRange` | `4096` |
| `gifFrames` | `0` |
| `MfbStates` | `32` |
| `MonitorStates` | `32` |
| `collectMaxsize` | `65535` |
| `collectMaxlength` | `32` |
| `collectMen` | `FD84` |
| `buseSPI` | `true` |
| `bDataHL` | `true` |
| `bgb2312` | `true` |
| `macroRWEntry` | `80440000` |
| `macrogetData` | `804c5070` |
| `macroEntry` | `80520400` |
| `macroEntryStr` | `80520400` |
| `guilib` | `guilib70.a` |
| `guilibT` | `guilib70_amt.a` |
| `script` | `script_amzlib.a` |
| `macroLB` | `0x804c5010` |
| `macroLW` | `0x804c5020` |
| `macroRWI` | `0x804c5030` |
| `HzFile` | `HZ_ASmall.bin` |
| `fbottomFile` | `AMT070W124-L800X480R.bin` |
| `cbottomFile` | `AMT070W124-L800X480R.bin` |
| `offlineExe` | `V51_800X480.exe` |
| `onlineExe` | `onlineV51_800X480.exe` |
| `com1Suffix` | `_com1.bin` |
| `com2Suffix` | `_com2.bin` |
| `drivePath` | `Link/serial_amz/` |
| `startScreen` | `HMIStudiostart_800x480.png` |

## 可采用的范围

- 型号节点名与原厂源头型号均为AMT070W124-L800X480R；model.dat声明Seris=AMT、Pixels=800*480、COM1/COM2/USB=true。它们是静态配置声明，不代替端口/方向/通信运行验证。
- modelInfo.dat声明Device=76、USB=2、SerialPortDown=false，maxScreen=200、maxControl=100、MonitorStates/MfbStates=32等。其他容量、地址、缓冲区均保留原文，尚未统一进制或单位。
- 新建普通页UI实际编号微调0..999，和maxScreen=200的配置字段分别记录；编号空间与页面数量上限不可混同。默认新页方向下拉有4项但当前AMT窗口中隐藏，不能由此开放旋转编译。
- 已有AMT设备编译/USB事实仍引用A05及既有协议。XML的guilib、驱动目录、宏地址及离线/在线EXE字段只说明所配置的文件/地址，不证明所有消费者与固件ABI已恢复。
- Q-S01-NATIVE-MODEL-001：定位原厂对这些字段的具体toInt/toUInt进制、单位及限制检查；在指定AMT UI按字段取边界，再做原厂离线差分。Q-S01-NATIVE-FIRMWARE-001：后续读取实际连接设备版本/固件身份，不以磁盘同名bin代替。
