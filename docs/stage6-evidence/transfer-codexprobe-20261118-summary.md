# CodexProbeTest.ksp 实物传输回执摘要（2026-11-18）

> 性质：主线对真实USB传输stdout的转录摘要（未tee完整原始stdout，未为取回执而重复下载）。
> 命令：`build/KinSealStudioLite.exe --download-project .scratch/kinseal-20261118/CodexProbeTest.ksp`（样例复制自 `samples/native-device-test/CodexProbeTest.ksp`，ASCII路径）。
> 前置授权：用户本轮明确指示"需要下载的就编译下载到实物中验证，USB口已连接到屏"。
> 代码状态：HEAD=87dcb649（修改6）+工作树（Q_OBJECT修复未提交、用户阶段3续作WIP原样保留）。

## 传输结果（设备真实ACK）

- `ok: true`，`outcome: "transport_complete"`，`planValidated: true`，`payloadBlocks: 10`，`completedSteps: 12/12`
- `interfaceClaimed: true`、`interfaceReleased: true`、`opened: true`
- `commitAccepted: true`、`commitCommandWritten: true`、`resetWritten: true`、`postResetReadCompleted: true`（返回-5，设备重启后读取结束，属预期）
- `firstError: present=false`（无任何错误）
- 设备回执：识别/参数阶段各返回24字节（modelReply=4c0103000102030432303135303731330000004100000000），各段命令均回 `'O'(0x4f)`

## 各段写入与SHA256对照（对照=src/independent_downloader.cpp固化参考，即此前与原厂产物对比验证的基线）

| 段 | 长度(实测) | 传输SHA256 | 与参考一致 |
|---|---|---|---|
| SysPara.dat | 4998 | b22f00a160e3dec23d54fdf9e595f6e0b842dee494f95cc9d43ca104c687a01f | ✓ |
| SheetPara.dat | 72 | 9ddd74c4c47d461ab48cb089cf1037b36769c35f8e6e5a830eab23b1381e1c90 | ✓ |
| AlarmPara.dat | 47840 | 12d43ec809fb64507247b520958c99813143fb4c12e63a872d50a7a0aa5c7b10 | ✓ |
| 0x16（空段命令） | 0 | — | 命令ACK ✓ |
| Collect.dat | 1424 | 5e796fbf009f80334fea0839be003bfbcc320f4dcacdfa2f53c6ca00c7d99e8c | ✓ |
| 0x19（空段命令） | 0 | — | 命令ACK ✓ |
| AddrPara.dat | 200 | 6d9c54dee5660c46886f32d80e57e9dd0ffa57ee0cd2a762b036d9c8e0c3a33a | ✓ |
| PicLenPara.dat | 2400 | fbcd494595c1c67c51efaa7e98b003d30591875be305874d8619cf0f41eab27d | ✓ |
| Curve.dat | 501 | 01064fb25c62c76b1f0b7b12de7a44b021b320a735eb9361639500029cb11586 | ✓ |
| upLoad.knr | **1097** | **f869063784ff77cb5669d649e59ea9c944b2f16f4395716f62ee17913619a494** | **✗ 见下** |
| SheetPixel.dat | 3919 | ea3aeed6d83fe39111cbd3add758b643e3472894fae355aabab3c64953dd3eca | ✓ |
| Modbus_RTU_com1.bin | 12496 | fddf11ab2cfec2b06fcdb408de1bb82fbe26707a97a9c2b2b4b0be382bc0290c | ✓ |

## 新发现差异：upLoad.knr 传输路径与离线编译路径不一致

- 同一exe（含Q_OBJECT修复后的build8）同一输入：
  - `--compile-project`（离线路径，复测）→ upLoad.knr = 1093字节，SHA `9324e2c3…`（=固化参考）；
  - 传输内部编译路径 → upLoad.knr = 1097字节，SHA `f8690637…`（多4字节）。
- 该分歧在今日改动前即存在于工作树（今日改动均为PC侧帮助/渲染/选择，不触及upLoad.knr生成；其生成代码位于用户未提交WIP文件 independent_downloader.cpp，本轮不改）。
- 处置：登记为待归因差异项；后续会话在用户WIP入库后单变量定位（传输路径相对离线路径多的4字节内容）。
- 当次传输的其余10个数据段与参考逐字节一致；设备对全部段、提交与重启命令均ACK。

## E（实屏观察）状态

- `screenDisplayAndTouchConfirmed: false`——回执明确"数据段及提交已获确认…实屏显示/触控仍需独立观察"。
- 待用户目视确认：屏幕重启后应显示 CodexProbeTest 工程的既定画面内容。
