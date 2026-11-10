# 阶段1源语料索引

日期：2026-11-10。修改标记：`//20261110-5`。

[机器索引](../../tests/fixtures/native-source/manifest.json) 是42个固定文件的路径、SHA-256、字节数、来源分类与证据入口。测试读取 `tests/fixtures/native-source/`，不再依赖旧 scratch。原厂运行身份、私有副本差异、正常UI动作、失败采样和独立重开见[原厂证据索引](native-analysis/manifest.json)与[调查结论](native-analysis/findings.md)。

| 子目录 | 文件数 | 来源/用途 |
|---|---:|---|
| bit | 10 | 历史 bit seed，加本轮四操作/通知/外部触控及双语原厂回存；当前单控件原厂语义往返 |
| word | 1 | 现有真实原厂 word seed；保留七类型/七操作和按下态产品回归 |
| lamp | 16 | 历史原厂真实支持源；双/多态、同文、无图、状态保留 |
| function | 5 | 原厂默认/高级/完整矩阵、真实无效UTF16源及对应specs |
| static | 1 | 本轮原厂显现等级3；静态文本默认seed仍使用原有嵌入样本 |
| multipage | 4 | 本轮原厂A/B/C/D；完整typed/EOF及多页语义/设备拒绝 |
| bit-analysis | 3 | 原厂混合双语、原厂实际English编辑态回存、明确产品宏源码正向输入 |
| rejected | 2 | 原厂真实0/1语言组合、活动宏名清空；结构识别及整源归档、语义拒绝 |

`function-tail-r2.sm` 是历史原厂无效代理项反例，不是语义正例。原厂旧失败空画布源没有搬进支持集；不存在的旧源码不伪造旧SHA。旧编译宏源未找回，正向宏源码/绑定来自明确产品输入，缓存MD5注入是测试人工构造；原厂宏组禁用与清名事实继续保留。

产品输入 `.ksp/.sm`、来源说明和实际导出结果位于[source-inputs](source-inputs/input-provenance.json)。这些输入在原厂另存/保存前不称原厂输出。最终三份原厂回存的活动语义比较见[source-resaved-semantic-check.json](source-resaved-semantic-check.json)。

静态文本、位按钮、字按钮、指示灯和功能按钮的原有 `.ksp` GUI样本继续就地读取，不复制整套样本目录。source-readonly-review.json 登记修改前/后源码身份与本次使用的真实GUI混合样本SHA；本索引不替代实际构建、原厂编译或USB回执。
