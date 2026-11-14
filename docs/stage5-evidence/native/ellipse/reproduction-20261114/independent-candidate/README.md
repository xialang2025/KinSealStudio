# G05 椭圆独立 C++ 曲线细分候选（2026-11-14）

本目录是离线数学候选。未修改生产代码、主文档或 Git；未启动原厂、USB 或任何可见窗口。公共 Qt4 栅格程序只通过外部进程提供诊断覆盖图，其源码未编入候选。

## 结果

8 种几何各检验填充和描边，总计 16 张 800×480 覆盖图：与公共 Qt4.6.3 诊断 oracle **全部 0 差**，与已验证的独立 Python 曲线与面积积分实现也 **全部 0 差**。

| 标签 | X,Y,W,H | 填充输出元素 | 描边输出元素 |
| --- | --- | ---: | ---: |
| default | 100,100,200,100 | 113 | 226 |
| shift | 75,65,160,120 | 97 | 194 |
| wide | 100,100,300,150 | 129 | 258 |
| tall | 100,100,120,250 | 113 | 210 |
| odd | 100,100,201,101 | 113 | 226 |
| min100 | 100,100,100,100 | 129 | 258 |
| far | 498,278,200,100 | 129 | 258 |
| near | 2,2,200,100 | 97 | 194 |

仅默认几何另与保留原厂背景解压出的 768000 字节整帧比较，结果 0 差，SHA-256 为 `c5ad7f785c4acf613505a592212ab69a55edf00b45f39f7e9e753d01dd08ed25`。其他几何本轮只与诊断 oracle 比较；它们的原厂创建/重开与样式语料由另一 owner 单独采样，不能用上述 16 张覆盖图替代原厂验收或宣称 G05 全部完成。

结果见 `cpp-ellipse-coverage-r1.json`。8 种几何的独立 Qt6 路径、展开直边路径和细分选择保存在 `artifacts/`。

## 独立函数与算法

可复用原型位于 `ellipse_curve_candidate.h/.cpp`：

```cpp
bool flattenHistoricalEllipseCandidate(const QPainterPath &input,
                                      QPainterPath *output,
                                      QString *error,
                                      EllipseFlattenStats *stats = nullptr);
```

输入必须由独立模型生成。CLI 使用 Qt6 `addEllipse` 与宽 1、SquareCap、BevelJoin 的 `QPainterPathStroker` 生成填充/描边，没有读取原厂 path 作为算法输入。

1. 检查每个坐标有限、非负且在 800×480 范围内，量化到 26.6 整数。
2. 由控制点量化后的整数包围盒选取历史阈值：任一维 >24 为一级，>120 再一级，阈值为 `16 << adaptation` 与 `32 << adaptation`。
3. 使用历史坐标幅度选择深度，其中第二个幅度刻意保留 `a.y+d.y-3*(b.x+c.y)` 的 XY 交叉项。其选择随场景位置变化，不能擅自替换成平移不变的几何误差公式。
4. 26.6 点乘 4 进入 8 位子像素网格，使用整数 de Casteljau 中点二分；每个叶子输出权重 (1,3,3,1)/8 的中点和终点。每级整数舍入都必须保留。
5. 输出只含 MoveTo/LineTo，保留原始 fillRule。随后可交给独立有向直边面积积分实现，分别生成填充/描边覆盖率，再作 RGB32 混色和最终 RGB565 量化。本 C++ 模块只负责曲线几何，不包含面积积分或设备段编码。

所有固定坐标与算术中间量为 qint64。坐标守卫让绝对值运算远离 qint64 最小值，中点/加权中点最大中间量不到 2^30。函数还独立限制输入 512 元素、输出 4096 元素、最大深度 8，先计算整段曲线所需元素预算，超限明确拒绝。该预算是候选的资源上界，不能静默裁剪路径。

每个叶子的坐标都是非负整数，因此 C++ 向零除法与所需 floor 相同。负坐标、裁剪、任意变换、任意笔宽/样式等未开放。API 先在局部对象中完成全部生成，仅成功时替换 output/stats；失败时调用者的旧值保持。

## 实际拒绝检查

`guard-api-r1.json` 记录 6 项 API 检查均成功拒绝且保留 output/stats：空输出指针、空路径、负控制点、越界控制点、输入 513 元素、生成元素超预算。`cpp-ellipse-coverage-r1.json` 另记录 6 个非法 CLI 几何均退出 3 且没有写输出：X=0、宽99、右越界、下越界、分数坐标和 int64 极大输入。

第一次构建成功但出现候选的未初始化分析提示和隐式 qint64→qreal 提示；已通过显式初始化和已守卫的显式转换修正。最终两个构建日志均为空，退出码 0。Qt头通过 `-isystem` 引入，候选自身仍启用 `-Wall -Wextra -Wconversion`。

## 复现

```powershell
& 'C:/Users/lenovo/AppData/Local/Programs/Python/Python311/python.exe' -B ./build_candidate.py
& 'C:/Users/lenovo/AppData/Local/Programs/Python/Python311/python.exe' -B ./compare_cpp_curve.py
```

单场景 CLI：`ellipse_curve_probe.exe <输出前缀> x y width height`。宽高至少100；X/Y至少1；右端<=799、下端<=479。输出独立/展开后的 fill 与 stroke pathstream（Qt_4_8 流格式）及细分 JSON。CLI 文件写入是诊断用途，未替代产品包的事务发布协议。

`build-command-r2.json` 和 `run-commands-r1.json` 保存实际命令。所有子进程 `CREATE_NO_WINDOW`，所有 TEMP/TMP/TMPDIR 均指向本目录的 `temp/`。Qt6/MinGW 的具体路径由构建脚本和命令清单保留。

`reproduction-manifest-r1.json` 和 `ellipse-algorithm-reproduction-r1.tar.gz` 保留完整源码、命令、日志、结果、8种独立路径及CLI二进制，并回解检查每项 SHA-256；随后只清理本目录自己生成的临时 mask、oracle重复路径流和解压帧。历史 Qt4 源码/许可证仍在上一批 `native/polygon/reproduction-20261114/public-qt-sources/`，未重复引入候选构建。
