# S00-03 字段追踪生成与结构审计配方

日期：2026-11-10。修改标记：`#20261110-4`。

本目录保留 [字段追踪 JSON](../field-traceability.json) 与 [导航报告](../field-traceability.md) 的可重跑配方，不依赖 `.scratch` 文件。脚本只读取项目内文档和实现源码，写入这两份产物及本目录审计结果，不启动原厂软件、不构建、不操作设备。

## 输入与保留方式

- 完整读取 `design_plan` 的总入口、阶段0、A01/A04/A05 和31份控件手册。
- 读取五类已有控件实施报告、位按钮设备编译台账，以及明确映射到的模型声明、源格式和设备消费者源码。
- `source_registry` 保存每个真实输入的相对路径、精确字节 SHA-256、大小和行数；原文保留在源文件，引用使用一基行号。
- `header_registry`、`mapping_registry`、`gap_registry`、`procedure_registry` 在 JSON 顶层共享，避免给每条记录复制模板。未知映射保持 `null` 并说明理由。

## 运行顺序

在 `REA` 根目录运行。以下 Python 路径是本机已验证的随应用运行时，可替换为兼容的 Python 3.10 及以上解释器。

```powershell
$taskPython = 'C:/Users/lenovo/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe'
$taskRecipe = 'KinSealStudio/docs/stage0-evidence/field-review-source'
$env:PYTHONIOENCODING = 'utf-8'

& $taskPython "$taskRecipe/build_traceability.py"
if ($LASTEXITCODE -ne 0) { throw '字段提取失败' }
& $taskPython "$taskRecipe/audit_traceability.py"
if ($LASTEXITCODE -ne 0) { throw '字段结构/引用审计失败' }
& $taskPython "$taskRecipe/write_report.py"
if ($LASTEXITCODE -ne 0) { throw '字段报告生成失败' }
```

`build-audit.json` 记录提取分类数量；`audit-result.json` 记录引用结构审计结果和所审 JSON 的 SHA。报告生成器只接受当前 JSON 的无错误审计，避免使用旧结果生成“审计无错误”的表述。`repeatability-result.json` 是本次相同输入两次生成、ID 集合和 JSON 字节一致性核对记录。

需要重跑一致性核对时执行 `& $taskPython "$taskRecipe/verify_repeatability.py"`。该脚本会依次执行上述三个配方两遍，比较来源登记、ID 集合、JSON 与报告字节，并检查报告和本说明中的本地链接是否存在，再保存结果；它会重生成两份正式产物，应在输入差异已审阅后使用。

只核查既有基线时应先单独运行 `audit_traceability.py`。如果任何来源 SHA 变化，应停止复用旧引用或旧结论，检查差异后再有意识地重建基线；直接重生成不能证明新内容语义正确。脚本从自身位置向上定位项目，不依赖启动时的当前目录。

## 稳定 ID

C01..C31 沿 A04。F、EV、AC 和 `Q-S00F` 使用控件身份、章节路径及原标签/原句组成锚点，把连续空白规范化后取 SHA-256 前12位。ID 不包含行号、发现顺序或来源文件 SHA。增加无关空行不会整体重排身份；同一段正文发生语义改写或被拆为原子字段时，需建立新 ID 与显式去向，不能承诺整句候选具有最终原子字段身份。算法以 `build_traceability.py` 中 `sid()` 和各调用处为准。

## 检查的含义

结构审计检查31控件身份、来源 SHA/行数、SourceSpan 界限、F/EV/AC/Q 唯一性和跨记录链接、真实 Markdown 表行，以及每控件入口样例不是标题。当前来源和记录数量以产物为准，不把文档行数或候选数当已通过规格数。

字段表行、枚举/条件行、正文树候选分别计数；复合字段仍需原子化。EV 是手册原句候选；AC 是待执行设计或手册原文；Q 是字段缺口占位，不能当独立问题总数或原308候选数。V/H/B/I 是原手册历史声明，实施源码映射另列。结构通过不会关闭原U候选、不会更新 D/U/P/R/C/T/E 阶段，也不会证明设备行为。主代理本轮 USB 和屏端验证的范围由其独立记录负责。
