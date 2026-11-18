# Lite 侧编译确定性差分报告（清单 S05-02）

> 复放：使用本轮已识别的阶段11 r2 EXE，通过 `tools/run_stage10_performance.py --matrix mixed` 重建 100×8 KSP 输入，再用本报告的编译差分工具检查。以下原始路径为运行时临时位置，输入与各产物以哈希标识。
- 生成时间：2026-11-19 06:44:50
- 样例输入：`Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-followup-20261119\performance-mixed\inputs\mixed-o100-l8.ksp`（SHA256 `7549635c98729ffd7543a6dca78fa5362e904ce77879efbd4acc9f7d153e153c`）
- 编译器：`Z:\Project\vscode_prj_temp\REA\KinSealStudio\dist\KinSealStudioLite-Stage11-release-r2\KinSealStudioLite.exe`
- 重复次数：3 次独立 CLI 进程，输出目录：
  - run1：`Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-followup-20261119\determinism-mixed\run1`
  - run2：`Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-followup-20261119\determinism-mixed\run2`
  - run3：`Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-followup-20261119\determinism-mixed\run3`
- 每次运行超时阈值：60 秒；超时按隐藏模态框挂死处理，杀进程后带 `QT_PLUGIN_PATH` 重试一次。
- 运行环境：`QT_QPA_PLATFORM=offscreen`，`QT_PLUGIN_PATH=G:\01_Professional\QT6\Installed\6.12.0\mingw_64\plugins`。
- 进程替代说明：CLI 为单命令进程，同进程内二次编译不可行；本工具以「3 次独立 CLI 进程」替代同进程重复（等价于"进程A跑2次+进程B跑1次"的进程间隔离方案），特此如实说明。
- 工具命令行：`tools/verify_compile_determinism.py Z:/Project/vscode_prj_temp/REA/.scratch/kinseal-followup-20261119/performance-mixed/inputs/mixed-o100-l8.ksp --exe Z:\Project\vscode_prj_temp\REA\KinSealStudio\dist\KinSealStudioLite-Stage11-release-r2\KinSealStudioLite.exe --runs 3 --out-root Z:/Project/vscode_prj_temp/REA/.scratch/kinseal-followup-20261119/determinism-mixed --report Z:/Project/vscode_prj_temp/REA/.scratch/kinseal-followup-20261119/determinism-mixed.md --timeout 60`

## 编译命令与运行结果

| 次数 | 命令行 | 退出码 | 编译返回消息 | 重试 |
| --- | --- | --- | --- | --- |
| run1 | `Z:\Project\vscode_prj_temp\REA\KinSealStudio\dist\KinSealStudioLite-Stage11-release-r2\KinSealStudioLite.exe --compile-project Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-followup-20261119\performance-mixed\inputs\mixed-o100-l8.ksp Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-followup-20261119\determinism-mixed\run1` | 0 | 工程设备数据编译完成，尚未访问USB。 | 否 |
| run2 | `Z:\Project\vscode_prj_temp\REA\KinSealStudio\dist\KinSealStudioLite-Stage11-release-r2\KinSealStudioLite.exe --compile-project Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-followup-20261119\performance-mixed\inputs\mixed-o100-l8.ksp Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-followup-20261119\determinism-mixed\run2` | 0 | 工程设备数据编译完成，尚未访问USB。 | 否 |
| run3 | `Z:\Project\vscode_prj_temp\REA\KinSealStudio\dist\KinSealStudioLite-Stage11-release-r2\KinSealStudioLite.exe --compile-project Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-followup-20261119\performance-mixed\inputs\mixed-o100-l8.ksp Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-followup-20261119\determinism-mixed\run3` | 0 | 工程设备数据编译完成，尚未访问USB。 | 否 |

## 各次产物 SHA256 清单

### run1

| 文件 | 长度（字节） | SHA256 |
| --- | --- | --- |
| AddrPara.dat | 200 | `6d9c54dee5660c46886f32d80e57e9dd0ffa57ee0cd2a762b036d9c8e0c3a33a` |
| AlarmPara.dat | 47840 | `12d43ec809fb64507247b520958c99813143fb4c12e63a872d50a7a0aa5c7b10` |
| Collect.dat | 1424 | `5e796fbf009f80334fea0839be003bfbcc320f4dcacdfa2f53c6ca00c7d99e8c` |
| ControlPara.dat | 64000 | `43594335a62573f84b200cf6904edddc03026151ee5f0c26679a34ba0b541c0f` |
| ControlPixel.dat | 51200 | `ce7d46296513f6405c10deeceb446ec1e28ed43062c9d2fdc52dd720409ecb66` |
| Curve.dat | 501 | `01064fb25c62c76b1f0b7b12de7a44b021b320a735eb9361639500029cb11586` |
| Modbus_RTU_com1.bin | 12496 | `fddf11ab2cfec2b06fcdb408de1bb82fbe26707a97a9c2b2b4b0be382bc0290c` |
| PicLenPara.dat | 12000 | `8c9d54c02239e2030aa57347f95b198a09524a62e0ce06d75e2e8fd0d681671a` |
| SheetPara.dat | 72 | `6de6b79c65322a64c32f3a36fc351ac9777e25a15c44be0721e660434cff41b8` |
| SheetPixel.dat | 4137 | `6cb9f072f61b05e024da878709c2631500a138d827450a8381960621ff4e0cd6` |
| SysPara.dat | 4998 | `688d549d966d564edef037e7b68ddd545f498d16912bd8813404444c7b44e807` |
| compiler-manifest.json | 5696 | `c89d17736495ed7398d3054366fe5ee0ea9ba90adac20a07b10c8059ca5c7cff` |
| upLoad.knr | 3615 | `c0e8f79313a1272221dfc5868ca13efc4f1966445fe0efe2f35116828a7a4382` |

### run2

| 文件 | 长度（字节） | SHA256 |
| --- | --- | --- |
| AddrPara.dat | 200 | `6d9c54dee5660c46886f32d80e57e9dd0ffa57ee0cd2a762b036d9c8e0c3a33a` |
| AlarmPara.dat | 47840 | `12d43ec809fb64507247b520958c99813143fb4c12e63a872d50a7a0aa5c7b10` |
| Collect.dat | 1424 | `5e796fbf009f80334fea0839be003bfbcc320f4dcacdfa2f53c6ca00c7d99e8c` |
| ControlPara.dat | 64000 | `43594335a62573f84b200cf6904edddc03026151ee5f0c26679a34ba0b541c0f` |
| ControlPixel.dat | 51200 | `ce7d46296513f6405c10deeceb446ec1e28ed43062c9d2fdc52dd720409ecb66` |
| Curve.dat | 501 | `01064fb25c62c76b1f0b7b12de7a44b021b320a735eb9361639500029cb11586` |
| Modbus_RTU_com1.bin | 12496 | `fddf11ab2cfec2b06fcdb408de1bb82fbe26707a97a9c2b2b4b0be382bc0290c` |
| PicLenPara.dat | 12000 | `8c9d54c02239e2030aa57347f95b198a09524a62e0ce06d75e2e8fd0d681671a` |
| SheetPara.dat | 72 | `6de6b79c65322a64c32f3a36fc351ac9777e25a15c44be0721e660434cff41b8` |
| SheetPixel.dat | 4137 | `6cb9f072f61b05e024da878709c2631500a138d827450a8381960621ff4e0cd6` |
| SysPara.dat | 4998 | `688d549d966d564edef037e7b68ddd545f498d16912bd8813404444c7b44e807` |
| compiler-manifest.json | 5696 | `c89d17736495ed7398d3054366fe5ee0ea9ba90adac20a07b10c8059ca5c7cff` |
| upLoad.knr | 3615 | `c0e8f79313a1272221dfc5868ca13efc4f1966445fe0efe2f35116828a7a4382` |

### run3

| 文件 | 长度（字节） | SHA256 |
| --- | --- | --- |
| AddrPara.dat | 200 | `6d9c54dee5660c46886f32d80e57e9dd0ffa57ee0cd2a762b036d9c8e0c3a33a` |
| AlarmPara.dat | 47840 | `12d43ec809fb64507247b520958c99813143fb4c12e63a872d50a7a0aa5c7b10` |
| Collect.dat | 1424 | `5e796fbf009f80334fea0839be003bfbcc320f4dcacdfa2f53c6ca00c7d99e8c` |
| ControlPara.dat | 64000 | `43594335a62573f84b200cf6904edddc03026151ee5f0c26679a34ba0b541c0f` |
| ControlPixel.dat | 51200 | `ce7d46296513f6405c10deeceb446ec1e28ed43062c9d2fdc52dd720409ecb66` |
| Curve.dat | 501 | `01064fb25c62c76b1f0b7b12de7a44b021b320a735eb9361639500029cb11586` |
| Modbus_RTU_com1.bin | 12496 | `fddf11ab2cfec2b06fcdb408de1bb82fbe26707a97a9c2b2b4b0be382bc0290c` |
| PicLenPara.dat | 12000 | `8c9d54c02239e2030aa57347f95b198a09524a62e0ce06d75e2e8fd0d681671a` |
| SheetPara.dat | 72 | `6de6b79c65322a64c32f3a36fc351ac9777e25a15c44be0721e660434cff41b8` |
| SheetPixel.dat | 4137 | `6cb9f072f61b05e024da878709c2631500a138d827450a8381960621ff4e0cd6` |
| SysPara.dat | 4998 | `688d549d966d564edef037e7b68ddd545f498d16912bd8813404444c7b44e807` |
| compiler-manifest.json | 5696 | `c89d17736495ed7398d3054366fe5ee0ea9ba90adac20a07b10c8059ca5c7cff` |
| upLoad.knr | 3615 | `c0e8f79313a1272221dfc5868ca13efc4f1966445fe0efe2f35116828a7a4382` |

## 差分结论：通过：全部产物逐字节一致

- 对比文件总数：13；逐字节一致：13；存在差异：0。

## 已知确定性文件汇总（静态段/字体帧等）

| 文件 | 本次结果 |
| --- | --- |
| AddrPara.dat | 一致 |
| Collect.dat | 一致 |
| ControlPara.dat | 一致 |
| ControlPixel.dat | 一致 |
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
