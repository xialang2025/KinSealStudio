# Lite 侧编译确定性差分报告（清单 S05-02）

- 生成时间：2026-11-18 13:45:14
- 样例输入：`samples\native-device-test\CodexProbeTest.ksp`（SHA256 `9d08bdab87e171f03b0c7a2b83b2f0642d176e7581eaa23117f528acea3c8752`）
- 编译器：`Z:\Project\vscode_prj_temp\REA\KinSealStudio\build\KinSealStudioLite.exe`
- 重复次数：3 次独立 CLI 进程，输出目录：
  - run1：`Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-20261118\determinism\run1`
  - run2：`Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-20261118\determinism\run2`
  - run3：`Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-20261118\determinism\run3`
- 每次运行超时阈值：30 秒；超时按隐藏模态框挂死处理，杀进程后带 `QT_PLUGIN_PATH` 重试一次。
- 运行环境：`QT_QPA_PLATFORM=offscreen`，`QT_PLUGIN_PATH=G:\01_Professional\QT6\Installed\6.12.0\mingw_64\plugins`。
- 进程替代说明：CLI 为单命令进程，同进程内二次编译不可行；本工具以「3 次独立 CLI 进程」替代同进程重复（等价于"进程A跑2次+进程B跑1次"的进程间隔离方案），特此如实说明。
- 工具命令行：`tools/verify_compile_determinism.py samples/native-device-test/CodexProbeTest.ksp --out-root Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-20261118\determinism --report Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-20261118\determinism-report.md`

## 编译命令与运行结果

| 次数 | 命令行 | 退出码 | 编译返回消息 | 重试 |
| --- | --- | --- | --- | --- |
| run1 | `Z:\Project\vscode_prj_temp\REA\KinSealStudio\build\KinSealStudioLite.exe --compile-project Z:\Project\vscode_prj_temp\REA\KinSealStudio\samples\native-device-test\CodexProbeTest.ksp Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-20261118\determinism\run1` | 0 | 工程设备数据编译完成，尚未访问USB。 | 否 |
| run2 | `Z:\Project\vscode_prj_temp\REA\KinSealStudio\build\KinSealStudioLite.exe --compile-project Z:\Project\vscode_prj_temp\REA\KinSealStudio\samples\native-device-test\CodexProbeTest.ksp Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-20261118\determinism\run2` | 0 | 工程设备数据编译完成，尚未访问USB。 | 否 |
| run3 | `Z:\Project\vscode_prj_temp\REA\KinSealStudio\build\KinSealStudioLite.exe --compile-project Z:\Project\vscode_prj_temp\REA\KinSealStudio\samples\native-device-test\CodexProbeTest.ksp Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-20261118\determinism\run3` | 0 | 工程设备数据编译完成，尚未访问USB。 | 否 |

## 各次产物 SHA256 清单

### run1

| 文件 | 长度（字节） | SHA256 |
| --- | --- | --- |
| AddrPara.dat | 200 | `6d9c54dee5660c46886f32d80e57e9dd0ffa57ee0cd2a762b036d9c8e0c3a33a` |
| AlarmPara.dat | 47840 | `12d43ec809fb64507247b520958c99813143fb4c12e63a872d50a7a0aa5c7b10` |
| Collect.dat | 1424 | `5e796fbf009f80334fea0839be003bfbcc320f4dcacdfa2f53c6ca00c7d99e8c` |
| Curve.dat | 501 | `01064fb25c62c76b1f0b7b12de7a44b021b320a735eb9361639500029cb11586` |
| Modbus_RTU_com1.bin | 12496 | `fddf11ab2cfec2b06fcdb408de1bb82fbe26707a97a9c2b2b4b0be382bc0290c` |
| PicLenPara.dat | 2400 | `fbcd494595c1c67c51efaa7e98b003d30591875be305874d8619cf0f41eab27d` |
| SheetPara.dat | 72 | `9ddd74c4c47d461ab48cb089cf1037b36769c35f8e6e5a830eab23b1381e1c90` |
| SheetPixel.dat | 3919 | `ea3aeed6d83fe39111cbd3add758b643e3472894fae355aabab3c64953dd3eca` |
| SysPara.dat | 4998 | `b22f00a160e3dec23d54fdf9e595f6e0b842dee494f95cc9d43ca104c687a01f` |
| compiler-manifest.json | 4884 | `8ef74d41580d560adea80b19730de0ab3297bb6b5c21ac74b75ab02153e16d1c` |
| upLoad.knr | 1093 | `9324e2c37f9684fa0a26d33710a61d4c16a641cc19b210533bbd8d3437c9db5c` |

### run2

| 文件 | 长度（字节） | SHA256 |
| --- | --- | --- |
| AddrPara.dat | 200 | `6d9c54dee5660c46886f32d80e57e9dd0ffa57ee0cd2a762b036d9c8e0c3a33a` |
| AlarmPara.dat | 47840 | `12d43ec809fb64507247b520958c99813143fb4c12e63a872d50a7a0aa5c7b10` |
| Collect.dat | 1424 | `5e796fbf009f80334fea0839be003bfbcc320f4dcacdfa2f53c6ca00c7d99e8c` |
| Curve.dat | 501 | `01064fb25c62c76b1f0b7b12de7a44b021b320a735eb9361639500029cb11586` |
| Modbus_RTU_com1.bin | 12496 | `fddf11ab2cfec2b06fcdb408de1bb82fbe26707a97a9c2b2b4b0be382bc0290c` |
| PicLenPara.dat | 2400 | `fbcd494595c1c67c51efaa7e98b003d30591875be305874d8619cf0f41eab27d` |
| SheetPara.dat | 72 | `9ddd74c4c47d461ab48cb089cf1037b36769c35f8e6e5a830eab23b1381e1c90` |
| SheetPixel.dat | 3919 | `ea3aeed6d83fe39111cbd3add758b643e3472894fae355aabab3c64953dd3eca` |
| SysPara.dat | 4998 | `b22f00a160e3dec23d54fdf9e595f6e0b842dee494f95cc9d43ca104c687a01f` |
| compiler-manifest.json | 4884 | `8ef74d41580d560adea80b19730de0ab3297bb6b5c21ac74b75ab02153e16d1c` |
| upLoad.knr | 1093 | `9324e2c37f9684fa0a26d33710a61d4c16a641cc19b210533bbd8d3437c9db5c` |

### run3

| 文件 | 长度（字节） | SHA256 |
| --- | --- | --- |
| AddrPara.dat | 200 | `6d9c54dee5660c46886f32d80e57e9dd0ffa57ee0cd2a762b036d9c8e0c3a33a` |
| AlarmPara.dat | 47840 | `12d43ec809fb64507247b520958c99813143fb4c12e63a872d50a7a0aa5c7b10` |
| Collect.dat | 1424 | `5e796fbf009f80334fea0839be003bfbcc320f4dcacdfa2f53c6ca00c7d99e8c` |
| Curve.dat | 501 | `01064fb25c62c76b1f0b7b12de7a44b021b320a735eb9361639500029cb11586` |
| Modbus_RTU_com1.bin | 12496 | `fddf11ab2cfec2b06fcdb408de1bb82fbe26707a97a9c2b2b4b0be382bc0290c` |
| PicLenPara.dat | 2400 | `fbcd494595c1c67c51efaa7e98b003d30591875be305874d8619cf0f41eab27d` |
| SheetPara.dat | 72 | `9ddd74c4c47d461ab48cb089cf1037b36769c35f8e6e5a830eab23b1381e1c90` |
| SheetPixel.dat | 3919 | `ea3aeed6d83fe39111cbd3add758b643e3472894fae355aabab3c64953dd3eca` |
| SysPara.dat | 4998 | `b22f00a160e3dec23d54fdf9e595f6e0b842dee494f95cc9d43ca104c687a01f` |
| compiler-manifest.json | 4884 | `8ef74d41580d560adea80b19730de0ab3297bb6b5c21ac74b75ab02153e16d1c` |
| upLoad.knr | 1093 | `9324e2c37f9684fa0a26d33710a61d4c16a641cc19b210533bbd8d3437c9db5c` |

## 差分结论：通过：全部产物逐字节一致

- 对比文件总数：11；逐字节一致：11；存在差异：0。

## 已知确定性文件汇总（静态段/字体帧等）

| 文件 | 本次结果 |
| --- | --- |
| AddrPara.dat | 一致 |
| Collect.dat | 一致 |
| ControlPara.dat | 本次产物缺失 |
| ControlPixel.dat | 本次产物缺失 |
| Curve.dat | 一致 |
| MarcoRun.dat | 本次产物缺失 |
| MarcoVar.dat | 本次产物缺失 |
| Modbus_RTU_com1.bin | 一致 |
| PicLenPara.dat | 一致 |
| SheetPara.dat | 一致 |
| SheetPixel.dat | 一致 |

## 历史已知非确定容器汇总（SysPara/AlarmPara/upLoad.knr 等）

| 文件 | 本次结果 | 差异段数 | 差异字节总数 | 首段偏移（十进制） | 首段偏移（十六进制） | 状态 |
| --- | --- | --- | --- | --- | --- | --- |
| AlarmPara.dat | 一致（本次样例下复现稳定） | 0 | 0 | - | - | 归因可收敛 |
| SysPara.dat | 一致（本次样例下复现稳定） | 0 | 0 | - | - | 归因可收敛 |
| upLoad.knr | 一致（本次样例下复现稳定） | 0 | 0 | - | - | 归因可收敛 |

## 非确定区明细表

本次三次编译产物逐字节一致，无非确定区。

## 归因提示

历史验收（docs/阶段5_扇形默认角度背景编译与差分验收.md 等）记录了 27 次同源编译对比，
其中 SysPara/AlarmPara/Usbload.bin/load.kns/upLoad.knr 等容器存在非确定原始字节或长度，原因未知。
本工具不做忽略掩码、不删改容器，只按文件名分组呈现差异偏移段；差异容器统一标注"非确定区待归因"，
供后续在 buildProjectPayload/publishDevicePackage 链路上定位随机源（时间戳、未初始化字段、随机序等）。
