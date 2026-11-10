# S00-04 冻结候选逐条语义初审台账

日期：2026-11-10。由代理逐条读取并显式拆问；不代表人类签字或本轮新测试通过。完整机器可读内容见[candidate-ledger.json](candidate-ledger.json)。

原始快照 `CAND-20261110` 保持308行，CSV SHA256 `27f423c4b35d387b243d0e4ca69485ff2a892b41e8b3cd81e19cfc81b532b87c`。本文件不修改原CSV、旧核销、程序日志或阶段状态；旧U号只作候选身份，正式问题使用独立分配并锁定的Q-S00-CAND编号。

## 数量与方法

| 口径 | 数量 |
|---|---:|
| 原始候选 / 已逐条初审候选 | 308 / 308 |
| 正式Q记录（包含父项） | 799 |
| 具体原子问题 | 613 |
| 复合父项Q | 186 |
| 沿用关闭事实 / 本轮新关闭 | 3 / 0 |
| 尚未闭合的原子问题 | 610 |
| 本轮实际执行测试 | 0 |

候选分类：复合父项167；单一问题119；说明文字9；已有证据12；重复1。候选状态与问题状态分别统计，不据此计算软件完成率。

显式审阅表与生成器归档于[candidate-review-source/review.tsv](candidate-review-source/review.tsv)及[生成器](candidate-review-source/build_candidate_ledger.py)，工作副本在 `.scratch/kinseal-stage0-20261110/candidates/`。每一行问题标题和分类均显式编写，探针模板只规范记录前置、刺激和观察。生成器校验308个原ID/原SHA、引用完整性及3个旧事实关闭；不运行历史覆盖CSV的survey脚本。正式JSON保存stable_assignment_key与原题，scratch锁丢失仍可恢复Q号；禁止静默移除既有Q。现存手册与5类实施报告另存当前SHA，冻结CSV旧来源SHA始终保留。

## 关键边界与复用

- U071→Q-FUNC-MACRO-001、U178→Q-LAMP-MACRO-001的宏禁用事实沿用；U181→Q-LAMP-COUNT-001的2..32和0..N−1事实沿用。这是3个事实、仅2个候选整体事实关闭，U181还有渲染分支。
- U181继续保留Q-LAMP-RENDER-001及PC外观、五态显示、闪烁、语言子项；既有源/图帧解释不重新当成未知，但不能替代E。
- U079继续保留Q-FUNC-RUN-001开放父项，按18功能分组形成59个具体动作问项；历史7、配方13、报警14和键盘10分别拆出。功能17的两动作与U078精确复用，不重复当测试。
- U085原文中的两个未测报警门槛，分别映射U091历史报警和U168报警显示的同一前置/刺激/观察。没有拿动态报警条的已知门槛关闭另外两个入口。
- U308仅排除为说明句，无Q且仍保留原始行和308计数。其他说明/归属/异常上下文保留可判断问题，不要求实现旧界面字段或复刻崩溃。
- 未按宏/权限/通知等同名关键词跨控件合并；调用点或设备结果相同缺证时保持独立，并保留相关候选链接。

## 逐候选去向

以下308行保持冻结CSV顺序。每个Q的前置、刺激、预期观察、现有证据和下一探针在后面的逐问题表及JSON中。

| 原候选 | 来源模块 / 实际归属 | 分类 / 候选状态 | 正式问题去向 |
|---|---|---|---|
| U-001 | ASCII显示 / ASCII显示 | 复合父项 / 初审开放 | [Q-S00-CAND-001-ASCII-PARENT](#q-s00-cand-001-ascii-parent)、[Q-S00-CAND-002-ASCII-UI](#q-s00-cand-002-ascii-ui)、[Q-S00-CAND-003-ASCII-UI](#q-s00-cand-003-ascii-ui) |
| U-002 | ASCII显示 / ASCII显示 | 单一问题 / 初审开放 | [Q-S00-CAND-004-ASCII-RENDER](#q-s00-cand-004-ascii-render) |
| U-003 | ASCII显示 / ASCII显示 | 复合父项 / 初审开放 | [Q-S00-CAND-005-ASCII-PARENT](#q-s00-cand-005-ascii-parent)、[Q-S00-CAND-006-ASCII-RENDER](#q-s00-cand-006-ascii-render)、[Q-S00-CAND-007-ASCII-RUNTIME](#q-s00-cand-007-ascii-runtime)、[Q-S00-CAND-008-ASCII-RENDER](#q-s00-cand-008-ascii-render)、[Q-S00-CAND-009-ASCII-RUNTIME](#q-s00-cand-009-ascii-runtime) |
| U-004 | ASCII显示 / ASCII显示 | 复合父项 / 初审开放 | [Q-S00-CAND-010-ASCII-PARENT](#q-s00-cand-010-ascii-parent)、[Q-S00-CAND-011-ASCII-UI](#q-s00-cand-011-ascii-ui)、[Q-S00-CAND-012-ASCII-BOUND](#q-s00-cand-012-ascii-bound) |
| U-005 | ASCII显示 / ASCII显示 | 复合父项 / 初审开放 | [Q-S00-CAND-013-ASCII-PARENT](#q-s00-cand-013-ascii-parent)、[Q-S00-CAND-014-ASCII-RUNTIME](#q-s00-cand-014-ascii-runtime)、[Q-S00-CAND-015-ASCII-VISIBILITY](#q-s00-cand-015-ascii-visibility) |
| U-006 | ASCII显示 / ASCII显示 | 单一问题 / 初审开放 | [Q-S00-CAND-016-ASCII-RENDER](#q-s00-cand-016-ascii-render) |
| U-007 | ASCII显示 / ASCII显示 | 复合父项 / 初审开放 | [Q-S00-CAND-017-ASCII-PARENT](#q-s00-cand-017-ascii-parent)、[Q-S00-CAND-018-ASCII-GALLERY](#q-s00-cand-018-ascii-gallery)、[Q-S00-CAND-019-ASCII-GALLERY](#q-s00-cand-019-ascii-gallery) |
| U-008 | ASCII显示 / ASCII显示 | 复合父项 / 初审开放 | [Q-S00-CAND-020-ASCII-PARENT](#q-s00-cand-020-ascii-parent)、[Q-S00-CAND-021-ASCII-RENDER](#q-s00-cand-021-ascii-render)、[Q-S00-CAND-022-ASCII-RENDER](#q-s00-cand-022-ascii-render)、[Q-S00-CAND-023-ASCII-RENDER](#q-s00-cand-023-ascii-render) |
| U-009 | ASCII显示 / ASCII显示 | 单一问题 / 初审开放 | [Q-S00-CAND-024-ASCII-UI](#q-s00-cand-024-ascii-ui) |
| U-010 | ASCII显示 / ASCII显示 | 单一问题 / 初审开放 | [Q-S00-CAND-025-ASCII-PERSIST](#q-s00-cand-025-ascii-persist) |
| U-011 | ASCII显示 / ASCII显示 | 复合父项 / 初审开放 | [Q-S00-CAND-026-ASCII-PARENT](#q-s00-cand-026-ascii-parent)、[Q-S00-CAND-027-ASCII-SOURCE](#q-s00-cand-027-ascii-source)、[Q-S00-CAND-028-ASCII-COMPILE](#q-s00-cand-028-ascii-compile)、[Q-S00-CAND-029-ASCII-RUNTIME](#q-s00-cand-029-ascii-runtime) |
| U-012 | ASCII显示 / ASCII显示 | 单一问题 / 初审开放 | [Q-S00-CAND-030-ASCII-HELP](#q-s00-cand-030-ascii-help) |
| U-013 | ASCII显示 / ASCII显示 | 单一问题 / 初审开放 | [Q-S00-CAND-031-ASCII-UI](#q-s00-cand-031-ascii-ui) |
| U-014 | GIF显示 / GIF显示 | 单一问题 / 初审开放 | [Q-S00-CAND-032-GIF-HELP](#q-s00-cand-032-gif-help) |
| U-015 | GIF显示 / GIF显示 | 复合父项 / 初审开放 | [Q-S00-CAND-033-GIF-PARENT](#q-s00-cand-033-gif-parent)、[Q-S00-CAND-034-GIF-RUNTIME](#q-s00-cand-034-gif-runtime)、[Q-S00-CAND-035-GIF-RUNTIME](#q-s00-cand-035-gif-runtime) |
| U-016 | GIF显示 / GIF显示 | 复合父项 / 初审开放 | [Q-S00-CAND-036-GIF-PARENT](#q-s00-cand-036-gif-parent)、[Q-S00-CAND-037-GIF-BOUND](#q-s00-cand-037-gif-bound)、[Q-S00-CAND-038-GIF-BOUND](#q-s00-cand-038-gif-bound)、[Q-S00-CAND-039-GIF-RUNTIME](#q-s00-cand-039-gif-runtime) |
| U-017 | GIF显示 / GIF显示 | 单一问题 / 初审开放 | [Q-S00-CAND-040-GIF-UI](#q-s00-cand-040-gif-ui) |
| U-018 | GIF显示 / GIF显示 | 复合父项 / 初审开放 | [Q-S00-CAND-041-GIF-PARENT](#q-s00-cand-041-gif-parent)、[Q-S00-CAND-042-GIF-PERMISSION](#q-s00-cand-042-gif-permission)、[Q-S00-CAND-043-GIF-THRESHOLD](#q-s00-cand-043-gif-threshold) |
| U-019 | GIF显示 / GIF显示 | 复合父项 / 初审开放 | [Q-S00-CAND-044-GIF-PARENT](#q-s00-cand-044-gif-parent)、[Q-S00-CAND-045-GIF-GALLERY](#q-s00-cand-045-gif-gallery)、[Q-S00-CAND-046-GIF-GALLERY](#q-s00-cand-046-gif-gallery)、[Q-S00-CAND-047-GIF-PERSIST](#q-s00-cand-047-gif-persist) |
| U-020 | GIF显示 / GIF显示 | 复合父项 / 初审开放 | [Q-S00-CAND-048-GIF-PARENT](#q-s00-cand-048-gif-parent)、[Q-S00-CAND-049-GIF-RENDER](#q-s00-cand-049-gif-render)、[Q-S00-CAND-050-GIF-RUNTIME](#q-s00-cand-050-gif-runtime) |
| U-021 | GIF显示 / GIF显示 | 复合父项 / 初审开放 | [Q-S00-CAND-051-GIF-PARENT](#q-s00-cand-051-gif-parent)、[Q-S00-CAND-052-GIF-SOURCE](#q-s00-cand-052-gif-source)、[Q-S00-CAND-053-GIF-COMPILE](#q-s00-cand-053-gif-compile) |
| U-022 | GIF显示 / GIF显示 | 说明文字 / 说明上下文保留，关联问题开放 | [Q-S00-CAND-054-GIF-SCOPE](#q-s00-cand-054-gif-scope) |
| U-023 | X-Y曲线 / X-Y曲线 | 复合父项 / 初审开放 | [Q-S00-CAND-055-XY-PARENT](#q-s00-cand-055-xy-parent)、[Q-S00-CAND-056-XY-BOUND](#q-s00-cand-056-xy-bound)、[Q-S00-CAND-057-XY-BOUND](#q-s00-cand-057-xy-bound)、[Q-S00-CAND-058-XY-BOUND](#q-s00-cand-058-xy-bound) |
| U-024 | X-Y曲线 / X-Y曲线 | 单一问题 / 初审开放 | [Q-S00-CAND-059-XY-PERSIST](#q-s00-cand-059-xy-persist) |
| U-025 | X-Y曲线 / X-Y曲线 | 单一问题 / 初审开放 | [Q-S00-CAND-060-XY-BOUND](#q-s00-cand-060-xy-bound) |
| U-026 | X-Y曲线 / X-Y曲线 | 复合父项 / 初审开放 | [Q-S00-CAND-061-XY-PARENT](#q-s00-cand-061-xy-parent)、[Q-S00-CAND-062-XY-RENDER](#q-s00-cand-062-xy-render)、[Q-S00-CAND-063-XY-BOUND](#q-s00-cand-063-xy-bound) |
| U-027 | X-Y曲线 / X-Y曲线 | 单一问题 / 初审开放 | [Q-S00-CAND-064-XY-OPTIONS](#q-s00-cand-064-xy-options) |
| U-028 | X-Y曲线 / X-Y曲线 | 单一问题 / 初审开放 | [Q-S00-CAND-065-XY-UI](#q-s00-cand-065-xy-ui) |
| U-029 | X-Y曲线 / X-Y曲线 | 复合父项 / 初审开放 | [Q-S00-CAND-066-XY-PARENT](#q-s00-cand-066-xy-parent)、[Q-S00-CAND-067-XY-UI](#q-s00-cand-067-xy-ui)、[Q-S00-CAND-068-XY-COMPILE](#q-s00-cand-068-xy-compile) |
| U-030 | X-Y曲线 / X-Y曲线 | 复合父项 / 初审开放 | [Q-S00-CAND-069-XY-PARENT](#q-s00-cand-069-xy-parent)、[Q-S00-CAND-070-XY-BOUND](#q-s00-cand-070-xy-bound)、[Q-S00-CAND-071-XY-BOUND](#q-s00-cand-071-xy-bound) |
| U-031 | X-Y曲线 / X-Y曲线 | 单一问题 / 初审开放 | [Q-S00-CAND-072-XY-HELP](#q-s00-cand-072-xy-help) |
| U-032 | X-Y曲线 / X-Y曲线 | 单一问题 / 初审开放 | [Q-S00-CAND-073-XY-VISIBILITY](#q-s00-cand-073-xy-visibility) |
| U-033 | X-Y曲线 / X-Y曲线 | 复合父项 / 初审开放 | [Q-S00-CAND-074-XY-PARENT](#q-s00-cand-074-xy-parent)、[Q-S00-CAND-075-XY-UI](#q-s00-cand-075-xy-ui)、[Q-S00-CAND-076-XY-RUNTIME](#q-s00-cand-076-xy-runtime) |
| U-034 | X-Y曲线 / X-Y曲线 | 复合父项 / 初审开放 | [Q-S00-CAND-077-XY-PARENT](#q-s00-cand-077-xy-parent)、[Q-S00-CAND-078-XY-SOURCE](#q-s00-cand-078-xy-source)、[Q-S00-CAND-079-XY-COMPILE](#q-s00-cand-079-xy-compile)、[Q-S00-CAND-080-XY-RUNTIME](#q-s00-cand-080-xy-runtime) |
| U-035 | 下拉框 / 下拉框 | 单一问题 / 初审开放 | [Q-S00-CAND-081-DROPLIST-RUNTIME](#q-s00-cand-081-droplist-runtime) |
| U-036 | 下拉框 / 下拉框 | 单一问题 / 初审开放 | [Q-S00-CAND-082-DROPLIST-BOUND](#q-s00-cand-082-droplist-bound) |
| U-037 | 下拉框 / 下拉框 | 单一问题 / 初审开放 | [Q-S00-CAND-083-DROPLIST-ADDRESS](#q-s00-cand-083-droplist-address) |
| U-038 | 下拉框 / 下拉框 | 单一问题 / 初审开放 | [Q-S00-CAND-084-DROPLIST-UI](#q-s00-cand-084-droplist-ui) |
| U-039 | 下拉框 / 下拉框 | 单一问题 / 初审开放 | [Q-S00-CAND-085-DROPLIST-BOUND](#q-s00-cand-085-droplist-bound) |
| U-040 | 下拉框 / 下拉框 | 单一问题 / 初审开放 | [Q-S00-CAND-086-DROPLIST-UI](#q-s00-cand-086-droplist-ui) |
| U-041 | 下拉框 / 下拉框 | 单一问题 / 初审开放 | [Q-S00-CAND-087-DROPLIST-PERSIST](#q-s00-cand-087-droplist-persist) |
| U-042 | 下拉框 / 下拉框 | 复合父项 / 初审开放 | [Q-S00-CAND-088-DROPLIST-PARENT](#q-s00-cand-088-droplist-parent)、[Q-S00-CAND-089-DROPLIST-RUNTIME](#q-s00-cand-089-droplist-runtime)、[Q-S00-CAND-090-DROPLIST-RUNTIME](#q-s00-cand-090-droplist-runtime) |
| U-043 | 下拉框 / 下拉框 | 复合父项 / 初审开放 | [Q-S00-CAND-091-DROPLIST-PARENT](#q-s00-cand-091-droplist-parent)、[Q-S00-CAND-092-DROPLIST-SOURCE](#q-s00-cand-092-droplist-source)、[Q-S00-CAND-093-DROPLIST-COMPILE](#q-s00-cand-093-droplist-compile) |
| U-044 | 二维码 / 二维码 | 单一问题 / 初审开放 | [Q-S00-CAND-094-QR-RUNTIME](#q-s00-cand-094-qr-runtime) |
| U-045 | 二维码 / 二维码 | 单一问题 / 初审开放 | [Q-S00-CAND-095-QR-COMPILE](#q-s00-cand-095-qr-compile) |
| U-046 | 二维码 / 二维码 | 复合父项 / 初审开放 | [Q-S00-CAND-096-QR-PARENT](#q-s00-cand-096-qr-parent)、[Q-S00-CAND-097-QR-RENDER](#q-s00-cand-097-qr-render)、[Q-S00-CAND-098-QR-RENDER](#q-s00-cand-098-qr-render) |
| U-047 | 二维码 / 二维码 | 单一问题 / 初审开放 | [Q-S00-CAND-099-QR-UI](#q-s00-cand-099-qr-ui) |
| U-048 | 二维码 / 二维码 | 单一问题 / 初审开放 | [Q-S00-CAND-100-QR-RUNTIME](#q-s00-cand-100-qr-runtime) |
| U-049 | 二维码 / 二维码 | 复合父项 / 初审开放 | [Q-S00-CAND-101-QR-PARENT](#q-s00-cand-101-qr-parent)、[Q-S00-CAND-102-QR-PERMISSION](#q-s00-cand-102-qr-permission)、[Q-S00-CAND-103-QR-THRESHOLD](#q-s00-cand-103-qr-threshold) |
| U-050 | 二维码 / 二维码 | 单一问题 / 初审开放 | [Q-S00-CAND-104-QR-BOUND](#q-s00-cand-104-qr-bound) |
| U-051 | 二维码 / 二维码 | 复合父项 / 初审开放 | [Q-S00-CAND-105-QR-PARENT](#q-s00-cand-105-qr-parent)、[Q-S00-CAND-106-QR-SOURCE](#q-s00-cand-106-qr-source)、[Q-S00-CAND-107-QR-COMPILE](#q-s00-cand-107-qr-compile)、[Q-S00-CAND-108-QR-RUNTIME](#q-s00-cand-108-qr-runtime) |
| U-052 | 二维码 / 二维码 | 单一问题 / 初审开放 | [Q-S00-CAND-109-QR-VISIBILITY](#q-s00-cand-109-qr-visibility) |
| U-053 | 位按钮 / 位按钮 | 已有证据 / 已有历史补证，整体未关闭 | [Q-S00-CAND-110-BIT-UI](#q-s00-cand-110-bit-ui) |
| U-054 | 位按钮 / 位按钮 | 已有证据 / 已有历史补证，整体未关闭 | [Q-S00-CAND-111-BIT-UI](#q-s00-cand-111-bit-ui) |
| U-055 | 位按钮 / 位按钮 | 已有证据 / 已有历史补证，整体未关闭 | [Q-S00-CAND-112-BIT-UI](#q-s00-cand-112-bit-ui) |
| U-056 | 位按钮 / 位按钮 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-113-BIT-PARENT](#q-s00-cand-113-bit-parent)、[Q-S00-CAND-114-BIT-LANGUAGE](#q-s00-cand-114-bit-language)、[Q-S00-CAND-115-BIT-RENDER](#q-s00-cand-115-bit-render)、[Q-S00-CAND-116-BIT-RUNTIME](#q-s00-cand-116-bit-runtime) |
| U-057 | 位按钮 / 位按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-117-BIT-PARENT](#q-s00-cand-117-bit-parent)、[Q-S00-CAND-118-BIT-OPTIONS](#q-s00-cand-118-bit-options)、[Q-S00-CAND-119-BIT-BOUND](#q-s00-cand-119-bit-bound) |
| U-058 | 位按钮 / 位按钮 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-120-BIT-PARENT](#q-s00-cand-120-bit-parent)、[Q-S00-CAND-121-BIT-RUNTIME](#q-s00-cand-121-bit-runtime)、[Q-S00-CAND-122-BIT-BOUND](#q-s00-cand-122-bit-bound)、[Q-S00-CAND-123-BIT-RENDER](#q-s00-cand-123-bit-render) |
| U-059 | 位按钮 / 位按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-124-BIT-PARENT](#q-s00-cand-124-bit-parent)、[Q-S00-CAND-125-BIT-PERMISSION](#q-s00-cand-125-bit-permission)、[Q-S00-CAND-126-BIT-THRESHOLD](#q-s00-cand-126-bit-threshold)、[Q-S00-CAND-127-BIT-THRESHOLD](#q-s00-cand-127-bit-threshold) |
| U-060 | 位按钮 / 位按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-128-BIT-PARENT](#q-s00-cand-128-bit-parent)、[Q-S00-CAND-129-BIT-RUNTIME](#q-s00-cand-129-bit-runtime)、[Q-S00-CAND-130-BIT-RUNTIME](#q-s00-cand-130-bit-runtime)、[Q-S00-CAND-131-BIT-RUNTIME](#q-s00-cand-131-bit-runtime)、[Q-S00-CAND-132-BIT-RUNTIME](#q-s00-cand-132-bit-runtime) |
| U-061 | 位按钮 / 位按钮 | 单一问题 / 初审开放 | [Q-S00-CAND-133-BIT-VISIBILITY](#q-s00-cand-133-bit-visibility) |
| U-062 | 位按钮 / 位按钮 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-134-BIT-PARENT](#q-s00-cand-134-bit-parent)、[Q-S00-CAND-135-BIT-SOURCE](#q-s00-cand-135-bit-source)、[Q-S00-CAND-136-BIT-COMPILE](#q-s00-cand-136-bit-compile)、[Q-S00-CAND-137-BIT-RUNTIME](#q-s00-cand-137-bit-runtime) |
| U-063 | 刻度 / 刻度 | 单一问题 / 初审开放 | [Q-S00-CAND-138-SCALE-UI](#q-s00-cand-138-scale-ui) |
| U-064 | 刻度 / 刻度 | 单一问题 / 初审开放 | [Q-S00-CAND-139-SCALE-BOUND](#q-s00-cand-139-scale-bound) |
| U-065 | 刻度 / 刻度 | 单一问题 / 初审开放 | [Q-S00-CAND-140-SCALE-BOUND](#q-s00-cand-140-scale-bound) |
| U-066 | 刻度 / 刻度 | 复合父项 / 初审开放 | [Q-S00-CAND-141-SCALE-PARENT](#q-s00-cand-141-scale-parent)、[Q-S00-CAND-142-SCALE-BOUND](#q-s00-cand-142-scale-bound)、[Q-S00-CAND-143-SCALE-BOUND](#q-s00-cand-143-scale-bound) |
| U-067 | 刻度 / 刻度 | 单一问题 / 初审开放 | [Q-S00-CAND-144-SCALE-RENDER](#q-s00-cand-144-scale-render) |
| U-068 | 刻度 / 刻度 | 单一问题 / 初审开放 | [Q-S00-CAND-145-SCALE-RUNTIME](#q-s00-cand-145-scale-runtime) |
| U-069 | 刻度 / A02全局入口/仪表归属待核 | 复合父项 / 初审开放 | [Q-S00-CAND-146-SCALE-PARENT](#q-s00-cand-146-scale-parent)、[Q-S00-CAND-147-SCALE-VISIBILITY](#q-s00-cand-147-scale-visibility)、[Q-S00-CAND-148-SCALE-UI](#q-s00-cand-148-scale-ui) |
| U-070 | 刻度 / 刻度 | 复合父项 / 初审开放 | [Q-S00-CAND-149-SCALE-PARENT](#q-s00-cand-149-scale-parent)、[Q-S00-CAND-150-SCALE-SOURCE](#q-s00-cand-150-scale-source)、[Q-S00-CAND-151-SCALE-COMPILE](#q-s00-cand-151-scale-compile)、[Q-S00-CAND-152-SCALE-RUNTIME](#q-s00-cand-152-scale-runtime) |
| U-071 | 功能按钮 / 功能按钮 | 已有证据 / 沿用事实关闭 | [Q-S00-CAND-153-FUNC-FACT](#q-s00-cand-153-func-fact) |
| U-072 | 功能按钮 / 功能按钮 | 单一问题 / 初审开放 | [Q-S00-CAND-154-FUNC-HELP](#q-s00-cand-154-func-help) |
| U-073 | 功能按钮 / 功能按钮 | 单一问题 / 初审开放 | [Q-S00-CAND-155-FUNC-UI](#q-s00-cand-155-func-ui) |
| U-074 | 功能按钮 / 功能按钮 | 单一问题 / 初审开放 | [Q-S00-CAND-156-FUNC-VISIBILITY](#q-s00-cand-156-func-visibility) |
| U-075 | 功能按钮 / 功能按钮 | 已有证据 / 已有历史补证，整体未关闭 | [Q-S00-CAND-157-FUNC-LANGUAGE](#q-s00-cand-157-func-language) |
| U-076 | 功能按钮 / 功能按钮 | 已有证据 / 已有历史补证，整体未关闭 | [Q-S00-CAND-158-FUNC-UI](#q-s00-cand-158-func-ui) |
| U-077 | 功能按钮 / 功能按钮 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-159-FUNC-PARENT](#q-s00-cand-159-func-parent)、[Q-S00-CAND-160-FUNC-RENDER](#q-s00-cand-160-func-render)、[Q-S00-CAND-161-FUNC-COMPILE](#q-s00-cand-161-func-compile)、[Q-S00-CAND-162-FUNC-RUNTIME](#q-s00-cand-162-func-runtime) |
| U-078 | 功能按钮 / 功能按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-163-FUNC-PARENT](#q-s00-cand-163-func-parent)、[Q-S00-CAND-164-FUNC-RUNTIME](#q-s00-cand-164-func-runtime)、[Q-S00-CAND-165-FUNC-RUNTIME](#q-s00-cand-165-func-runtime) |
| U-079 | 功能按钮 / 功能按钮 | 复合父项 / 复合父项开放，18功能分别接续 | [Q-S00-CAND-724-FUNC-PARENT](#q-s00-cand-724-func-parent)、[Q-S00-CAND-725-FUNC-PARENT](#q-s00-cand-725-func-parent)、[Q-S00-CAND-726-FUNC-RUNTIME](#q-s00-cand-726-func-runtime)、[Q-S00-CAND-727-FUNC-PARENT](#q-s00-cand-727-func-parent)、[Q-S00-CAND-728-FUNC-RUNTIME](#q-s00-cand-728-func-runtime)、[Q-S00-CAND-729-FUNC-PARENT](#q-s00-cand-729-func-parent)、[Q-S00-CAND-730-FUNC-RUNTIME](#q-s00-cand-730-func-runtime)、[Q-S00-CAND-731-FUNC-PARENT](#q-s00-cand-731-func-parent)、[Q-S00-CAND-732-FUNC-RUNTIME](#q-s00-cand-732-func-runtime)、[Q-S00-CAND-733-FUNC-PARENT](#q-s00-cand-733-func-parent)、[Q-S00-CAND-734-FUNC-RUNTIME](#q-s00-cand-734-func-runtime)、[Q-S00-CAND-735-FUNC-PARENT](#q-s00-cand-735-func-parent)、[Q-S00-CAND-736-FUNC-RUNTIME](#q-s00-cand-736-func-runtime)、[Q-S00-CAND-737-FUNC-PARENT](#q-s00-cand-737-func-parent)、[Q-S00-CAND-738-FUNC-RUNTIME](#q-s00-cand-738-func-runtime)、[Q-S00-CAND-739-FUNC-PARENT](#q-s00-cand-739-func-parent)、[Q-S00-CAND-740-FUNC-RUNTIME](#q-s00-cand-740-func-runtime)、[Q-S00-CAND-741-FUNC-PARENT](#q-s00-cand-741-func-parent)、[Q-S00-CAND-742-FUNC-RUNTIME](#q-s00-cand-742-func-runtime)、[Q-S00-CAND-743-FUNC-PARENT](#q-s00-cand-743-func-parent)、[Q-S00-CAND-744-FUNC-RUNTIME](#q-s00-cand-744-func-runtime)、[Q-S00-CAND-745-FUNC-PARENT](#q-s00-cand-745-func-parent)、[Q-S00-CAND-746-FUNC-RUNTIME](#q-s00-cand-746-func-runtime)、[Q-S00-CAND-747-FUNC-RUNTIME](#q-s00-cand-747-func-runtime)、[Q-S00-CAND-748-FUNC-RUNTIME](#q-s00-cand-748-func-runtime)、[Q-S00-CAND-749-FUNC-RUNTIME](#q-s00-cand-749-func-runtime)、[Q-S00-CAND-750-FUNC-RUNTIME](#q-s00-cand-750-func-runtime)、[Q-S00-CAND-751-FUNC-RUNTIME](#q-s00-cand-751-func-runtime)、[Q-S00-CAND-752-FUNC-RUNTIME](#q-s00-cand-752-func-runtime)、[Q-S00-CAND-753-FUNC-PARENT](#q-s00-cand-753-func-parent)、[Q-S00-CAND-754-FUNC-RUNTIME](#q-s00-cand-754-func-runtime)、[Q-S00-CAND-755-FUNC-RUNTIME](#q-s00-cand-755-func-runtime)、[Q-S00-CAND-756-FUNC-RUNTIME](#q-s00-cand-756-func-runtime)、[Q-S00-CAND-757-FUNC-RUNTIME](#q-s00-cand-757-func-runtime)、[Q-S00-CAND-758-FUNC-RUNTIME](#q-s00-cand-758-func-runtime)、[Q-S00-CAND-759-FUNC-RUNTIME](#q-s00-cand-759-func-runtime)、[Q-S00-CAND-760-FUNC-RUNTIME](#q-s00-cand-760-func-runtime)、[Q-S00-CAND-761-FUNC-RUNTIME](#q-s00-cand-761-func-runtime)、[Q-S00-CAND-762-FUNC-RUNTIME](#q-s00-cand-762-func-runtime)、[Q-S00-CAND-763-FUNC-RUNTIME](#q-s00-cand-763-func-runtime)、[Q-S00-CAND-764-FUNC-RUNTIME](#q-s00-cand-764-func-runtime)、[Q-S00-CAND-765-FUNC-RUNTIME](#q-s00-cand-765-func-runtime)、[Q-S00-CAND-766-FUNC-RUNTIME](#q-s00-cand-766-func-runtime)、[Q-S00-CAND-767-FUNC-PARENT](#q-s00-cand-767-func-parent)、[Q-S00-CAND-768-FUNC-RUNTIME](#q-s00-cand-768-func-runtime)、[Q-S00-CAND-769-FUNC-RUNTIME](#q-s00-cand-769-func-runtime)、[Q-S00-CAND-770-FUNC-RUNTIME](#q-s00-cand-770-func-runtime)、[Q-S00-CAND-771-FUNC-RUNTIME](#q-s00-cand-771-func-runtime)、[Q-S00-CAND-772-FUNC-RUNTIME](#q-s00-cand-772-func-runtime)、[Q-S00-CAND-773-FUNC-RUNTIME](#q-s00-cand-773-func-runtime)、[Q-S00-CAND-774-FUNC-RUNTIME](#q-s00-cand-774-func-runtime)、[Q-S00-CAND-775-FUNC-RUNTIME](#q-s00-cand-775-func-runtime)、[Q-S00-CAND-776-FUNC-RUNTIME](#q-s00-cand-776-func-runtime)、[Q-S00-CAND-777-FUNC-RUNTIME](#q-s00-cand-777-func-runtime)、[Q-S00-CAND-778-FUNC-RUNTIME](#q-s00-cand-778-func-runtime)、[Q-S00-CAND-779-FUNC-RUNTIME](#q-s00-cand-779-func-runtime)、[Q-S00-CAND-780-FUNC-RUNTIME](#q-s00-cand-780-func-runtime)、[Q-S00-CAND-781-FUNC-RUNTIME](#q-s00-cand-781-func-runtime)、[Q-S00-CAND-782-FUNC-PARENT](#q-s00-cand-782-func-parent)、[Q-S00-CAND-783-FUNC-RUNTIME](#q-s00-cand-783-func-runtime)、[Q-S00-CAND-784-FUNC-PARENT](#q-s00-cand-784-func-parent)、[Q-S00-CAND-785-FUNC-RUNTIME](#q-s00-cand-785-func-runtime)、[Q-S00-CAND-786-FUNC-RUNTIME](#q-s00-cand-786-func-runtime)、[Q-S00-CAND-787-FUNC-RUNTIME](#q-s00-cand-787-func-runtime)、[Q-S00-CAND-788-FUNC-RUNTIME](#q-s00-cand-788-func-runtime)、[Q-S00-CAND-789-FUNC-RUNTIME](#q-s00-cand-789-func-runtime)、[Q-S00-CAND-790-FUNC-RUNTIME](#q-s00-cand-790-func-runtime)、[Q-S00-CAND-791-FUNC-RUNTIME](#q-s00-cand-791-func-runtime)、[Q-S00-CAND-792-FUNC-RUNTIME](#q-s00-cand-792-func-runtime)、[Q-S00-CAND-793-FUNC-RUNTIME](#q-s00-cand-793-func-runtime)、[Q-S00-CAND-794-FUNC-RUNTIME](#q-s00-cand-794-func-runtime)、[Q-S00-CAND-795-FUNC-PARENT](#q-s00-cand-795-func-parent)、[Q-S00-CAND-796-FUNC-RUNTIME](#q-s00-cand-796-func-runtime)、[Q-S00-CAND-797-FUNC-PARENT](#q-s00-cand-797-func-parent)、[Q-S00-CAND-798-FUNC-RUNTIME](#q-s00-cand-798-func-runtime)、[Q-S00-CAND-799-FUNC-PARENT](#q-s00-cand-799-func-parent)、[Q-S00-CAND-164-FUNC-RUNTIME](#q-s00-cand-164-func-runtime)、[Q-S00-CAND-165-FUNC-RUNTIME](#q-s00-cand-165-func-runtime) |
| U-080 | 功能按钮 / 功能按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-166-FUNC-PARENT](#q-s00-cand-166-func-parent)、[Q-S00-CAND-167-FUNC-PERMISSION](#q-s00-cand-167-func-permission)、[Q-S00-CAND-168-FUNC-THRESHOLD](#q-s00-cand-168-func-threshold)、[Q-S00-CAND-169-FUNC-THRESHOLD](#q-s00-cand-169-func-threshold) |
| U-081 | 功能按钮 / 功能按钮 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-170-FUNC-PARENT](#q-s00-cand-170-func-parent)、[Q-S00-CAND-171-FUNC-SOURCE](#q-s00-cand-171-func-source)、[Q-S00-CAND-172-FUNC-COMPILE](#q-s00-cand-172-func-compile)、[Q-S00-CAND-173-FUNC-RUNTIME](#q-s00-cand-173-func-runtime) |
| U-082 | 功能按钮 / 功能按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-174-FUNC-PARENT](#q-s00-cand-174-func-parent)、[Q-S00-CAND-175-FUNC-SCOPE](#q-s00-cand-175-func-scope)、[Q-S00-CAND-176-FUNC-SCOPE](#q-s00-cand-176-func-scope) |
| U-083 | 动态报警条 / 动态报警条 | 单一问题 / 初审开放 | [Q-S00-CAND-177-ALARMBAR-BOUND](#q-s00-cand-177-alarmbar-bound) |
| U-084 | 动态报警条 / 动态报警条 | 单一问题 / 初审开放 | [Q-S00-CAND-178-ALARMBAR-OPTIONS](#q-s00-cand-178-alarmbar-options) |
| U-085 | 动态报警条 / 动态报警条 | 重复 / 重复映射，目标问题仍开放 | [Q-S00-CAND-189-HISTORYALARM-UI](#q-s00-cand-189-historyalarm-ui)、[Q-S00-CAND-395-ALARM-UI](#q-s00-cand-395-alarm-ui) |
| U-086 | 动态报警条 / A02报警登录配置 | 单一问题 / 初审开放 | [Q-S00-CAND-179-ALARMBAR-BOUND](#q-s00-cand-179-alarmbar-bound) |
| U-087 | 动态报警条 / A02报警登录配置 | 单一问题 / 初审开放 | [Q-S00-CAND-180-ALARMBAR-OPTIONS](#q-s00-cand-180-alarmbar-options) |
| U-088 | 动态报警条 / 动态报警条 | 复合父项 / 初审开放 | [Q-S00-CAND-181-ALARMBAR-PARENT](#q-s00-cand-181-alarmbar-parent)、[Q-S00-CAND-182-ALARMBAR-RUNTIME](#q-s00-cand-182-alarmbar-runtime)、[Q-S00-CAND-183-ALARMBAR-RUNTIME](#q-s00-cand-183-alarmbar-runtime) |
| U-089 | 动态报警条 / A02条件报警配置 | 单一问题 / 初审开放 | [Q-S00-CAND-184-ALARMBAR-UI](#q-s00-cand-184-alarmbar-ui) |
| U-090 | 动态报警条 / 动态报警条 | 复合父项 / 初审开放 | [Q-S00-CAND-185-ALARMBAR-PARENT](#q-s00-cand-185-alarmbar-parent)、[Q-S00-CAND-186-ALARMBAR-SOURCE](#q-s00-cand-186-alarmbar-source)、[Q-S00-CAND-187-ALARMBAR-COMPILE](#q-s00-cand-187-alarmbar-compile)、[Q-S00-CAND-188-ALARMBAR-RUNTIME](#q-s00-cand-188-alarmbar-runtime) |
| U-091 | 历史报警显示 / 历史报警显示 | 单一问题 / 初审开放 | [Q-S00-CAND-189-HISTORYALARM-UI](#q-s00-cand-189-historyalarm-ui) |
| U-092 | 历史报警显示 / 历史报警显示 | 单一问题 / 初审开放 | [Q-S00-CAND-190-HISTORYALARM-OPTIONS](#q-s00-cand-190-historyalarm-options) |
| U-093 | 历史报警显示 / 历史报警显示 | 复合父项 / 初审开放 | [Q-S00-CAND-191-HISTORYALARM-PARENT](#q-s00-cand-191-historyalarm-parent)、[Q-S00-CAND-192-HISTORYALARM-RUNTIME](#q-s00-cand-192-historyalarm-runtime)、[Q-S00-CAND-193-HISTORYALARM-RUNTIME](#q-s00-cand-193-historyalarm-runtime)、[Q-S00-CAND-194-HISTORYALARM-RUNTIME](#q-s00-cand-194-historyalarm-runtime)、[Q-S00-CAND-195-HISTORYALARM-RUNTIME](#q-s00-cand-195-historyalarm-runtime) |
| U-094 | 历史报警显示 / A02历史报警清除入口调查 | 单一问题 / 初审开放 | [Q-S00-CAND-196-HISTORYALARM-VISIBILITY](#q-s00-cand-196-historyalarm-visibility) |
| U-095 | 历史报警显示 / 历史报警显示 | 复合父项 / 初审开放 | [Q-S00-CAND-197-HISTORYALARM-PARENT](#q-s00-cand-197-historyalarm-parent)、[Q-S00-CAND-198-HISTORYALARM-LANGUAGE](#q-s00-cand-198-historyalarm-language)、[Q-S00-CAND-199-HISTORYALARM-RUNTIME](#q-s00-cand-199-historyalarm-runtime) |
| U-096 | 历史报警显示 / 历史报警显示 | 复合父项 / 初审开放 | [Q-S00-CAND-200-HISTORYALARM-PARENT](#q-s00-cand-200-historyalarm-parent)、[Q-S00-CAND-201-HISTORYALARM-SOURCE](#q-s00-cand-201-historyalarm-source)、[Q-S00-CAND-202-HISTORYALARM-COMPILE](#q-s00-cand-202-historyalarm-compile)、[Q-S00-CAND-203-HISTORYALARM-RUNTIME](#q-s00-cand-203-historyalarm-runtime) |
| U-097 | 历史数据显示器 / 历史数据显示器 | 复合父项 / 初审开放 | [Q-S00-CAND-204-HISTORYDATA-PARENT](#q-s00-cand-204-historydata-parent)、[Q-S00-CAND-205-HISTORYDATA-BOUND](#q-s00-cand-205-historydata-bound)、[Q-S00-CAND-206-HISTORYDATA-OPTIONS](#q-s00-cand-206-historydata-options)、[Q-S00-CAND-207-HISTORYDATA-RUNTIME](#q-s00-cand-207-historydata-runtime) |
| U-098 | 历史数据显示器 / 历史数据显示器 | 单一问题 / 初审开放 | [Q-S00-CAND-208-HISTORYDATA-LANGUAGE](#q-s00-cand-208-historydata-language) |
| U-099 | 历史数据显示器 / 历史数据显示器 | 单一问题 / 初审开放 | [Q-S00-CAND-209-HISTORYDATA-UI](#q-s00-cand-209-historydata-ui) |
| U-100 | 历史数据显示器 / 历史数据显示器 | 复合父项 / 初审开放 | [Q-S00-CAND-210-HISTORYDATA-PARENT](#q-s00-cand-210-historydata-parent)、[Q-S00-CAND-211-HISTORYDATA-RENDER](#q-s00-cand-211-historydata-render)、[Q-S00-CAND-212-HISTORYDATA-RUNTIME](#q-s00-cand-212-historydata-runtime) |
| U-101 | 历史数据显示器 / 历史数据显示器 | 单一问题 / 初审开放 | [Q-S00-CAND-213-HISTORYDATA-PERSIST](#q-s00-cand-213-historydata-persist) |
| U-102 | 历史数据显示器 / 历史数据显示器 | 单一问题 / 初审开放 | [Q-S00-CAND-214-HISTORYDATA-UI](#q-s00-cand-214-historydata-ui) |
| U-103 | 历史数据显示器 / 历史数据显示器 | 单一问题 / 初审开放 | [Q-S00-CAND-215-HISTORYDATA-HELP](#q-s00-cand-215-historydata-help) |
| U-104 | 历史数据显示器 / A02数据采集配置 | 复合父项 / 初审开放 | [Q-S00-CAND-216-HISTORYDATA-PARENT](#q-s00-cand-216-historydata-parent)、[Q-S00-CAND-217-HISTORYDATA-UI](#q-s00-cand-217-historydata-ui)、[Q-S00-CAND-218-HISTORYDATA-RUNTIME](#q-s00-cand-218-historydata-runtime)、[Q-S00-CAND-219-HISTORYDATA-RUNTIME](#q-s00-cand-219-historydata-runtime) |
| U-105 | 历史数据显示器 / 历史数据显示器 | 说明文字 / 说明上下文保留，关联问题开放 | [Q-S00-CAND-220-HISTORYDATA-DIAGNOSTIC](#q-s00-cand-220-historydata-diagnostic) |
| U-106 | 历史趋势图 / A02采集数据项/C24历史趋势 | 复合父项 / 初审开放 | [Q-S00-CAND-221-HISTORYTREND-PARENT](#q-s00-cand-221-historytrend-parent)、[Q-S00-CAND-222-HISTORYTREND-UI](#q-s00-cand-222-historytrend-ui)、[Q-S00-CAND-223-HISTORYTREND-UI](#q-s00-cand-223-historytrend-ui) |
| U-107 | 历史趋势图 / A02数据采集配置 | 复合父项 / 初审开放 | [Q-S00-CAND-224-HISTORYTREND-PARENT](#q-s00-cand-224-historytrend-parent)、[Q-S00-CAND-225-HISTORYTREND-BOUND](#q-s00-cand-225-historytrend-bound)、[Q-S00-CAND-226-HISTORYTREND-BOUND](#q-s00-cand-226-historytrend-bound) |
| U-108 | 历史趋势图 / 历史趋势图 | 复合父项 / 初审开放 | [Q-S00-CAND-227-HISTORYTREND-PARENT](#q-s00-cand-227-historytrend-parent)、[Q-S00-CAND-228-HISTORYTREND-BOUND](#q-s00-cand-228-historytrend-bound)、[Q-S00-CAND-229-HISTORYTREND-BOUND](#q-s00-cand-229-historytrend-bound) |
| U-109 | 历史趋势图 / 历史趋势图 | 复合父项 / 初审开放 | [Q-S00-CAND-230-HISTORYTREND-PARENT](#q-s00-cand-230-historytrend-parent)、[Q-S00-CAND-231-HISTORYTREND-BOUND](#q-s00-cand-231-historytrend-bound)、[Q-S00-CAND-232-HISTORYTREND-BOUND](#q-s00-cand-232-historytrend-bound) |
| U-110 | 历史趋势图 / 历史趋势图 | 复合父项 / 初审开放 | [Q-S00-CAND-233-HISTORYTREND-PARENT](#q-s00-cand-233-historytrend-parent)、[Q-S00-CAND-234-HISTORYTREND-UI](#q-s00-cand-234-historytrend-ui)、[Q-S00-CAND-235-HISTORYTREND-BOUND](#q-s00-cand-235-historytrend-bound) |
| U-111 | 历史趋势图 / 历史趋势图 | 单一问题 / 初审开放 | [Q-S00-CAND-236-HISTORYTREND-UI](#q-s00-cand-236-historytrend-ui) |
| U-112 | 历史趋势图 / 历史趋势图 | 复合父项 / 初审开放 | [Q-S00-CAND-237-HISTORYTREND-PARENT](#q-s00-cand-237-historytrend-parent)、[Q-S00-CAND-238-HISTORYTREND-RENDER](#q-s00-cand-238-historytrend-render)、[Q-S00-CAND-239-HISTORYTREND-RUNTIME](#q-s00-cand-239-historytrend-runtime) |
| U-113 | 历史趋势图 / 历史趋势图 | 单一问题 / 初审开放 | [Q-S00-CAND-240-HISTORYTREND-HELP](#q-s00-cand-240-historytrend-help) |
| U-114 | 历史趋势图 / 历史趋势图 | 复合父项 / 初审开放 | [Q-S00-CAND-241-HISTORYTREND-PARENT](#q-s00-cand-241-historytrend-parent)、[Q-S00-CAND-242-HISTORYTREND-SOURCE](#q-s00-cand-242-historytrend-source)、[Q-S00-CAND-243-HISTORYTREND-COMPILE](#q-s00-cand-243-historytrend-compile)、[Q-S00-CAND-244-HISTORYTREND-RUNTIME](#q-s00-cand-244-historytrend-runtime) |
| U-115 | 图片显示 / 图片显示 | 复合父项 / 初审开放 | [Q-S00-CAND-245-PICTURE-PARENT](#q-s00-cand-245-picture-parent)、[Q-S00-CAND-246-PICTURE-UI](#q-s00-cand-246-picture-ui)、[Q-S00-CAND-247-PICTURE-PERSIST](#q-s00-cand-247-picture-persist)、[Q-S00-CAND-248-PICTURE-DIAGNOSTIC](#q-s00-cand-248-picture-diagnostic) |
| U-116 | 图片显示 / 图片显示 | 复合父项 / 初审开放 | [Q-S00-CAND-249-PICTURE-PARENT](#q-s00-cand-249-picture-parent)、[Q-S00-CAND-250-PICTURE-ADDRESS](#q-s00-cand-250-picture-address)、[Q-S00-CAND-251-PICTURE-UI](#q-s00-cand-251-picture-ui) |
| U-117 | 图片显示 / 图片显示 | 复合父项 / 初审开放 | [Q-S00-CAND-252-PICTURE-PARENT](#q-s00-cand-252-picture-parent)、[Q-S00-CAND-253-PICTURE-GALLERY](#q-s00-cand-253-picture-gallery)、[Q-S00-CAND-254-PICTURE-GALLERY](#q-s00-cand-254-picture-gallery) |
| U-118 | 图片显示 / 图片显示 | 复合父项 / 初审开放 | [Q-S00-CAND-255-PICTURE-PARENT](#q-s00-cand-255-picture-parent)、[Q-S00-CAND-256-PICTURE-GALLERY](#q-s00-cand-256-picture-gallery)、[Q-S00-CAND-257-PICTURE-UI](#q-s00-cand-257-picture-ui) |
| U-119 | 图片显示 / 图片显示 | 复合父项 / 初审开放 | [Q-S00-CAND-258-PICTURE-PARENT](#q-s00-cand-258-picture-parent)、[Q-S00-CAND-259-PICTURE-BOUND](#q-s00-cand-259-picture-bound)、[Q-S00-CAND-260-PICTURE-BOUND](#q-s00-cand-260-picture-bound) |
| U-120 | 图片显示 / 图片显示 | 单一问题 / 初审开放 | [Q-S00-CAND-261-PICTURE-UI](#q-s00-cand-261-picture-ui) |
| U-121 | 图片显示 / 图片显示 | 复合父项 / 初审开放 | [Q-S00-CAND-262-PICTURE-PARENT](#q-s00-cand-262-picture-parent)、[Q-S00-CAND-263-PICTURE-BOUND](#q-s00-cand-263-picture-bound)、[Q-S00-CAND-264-PICTURE-BOUND](#q-s00-cand-264-picture-bound)、[Q-S00-CAND-265-PICTURE-RUNTIME](#q-s00-cand-265-picture-runtime) |
| U-122 | 图片显示 / 图片显示 | 单一问题 / 初审开放 | [Q-S00-CAND-266-PICTURE-HELP](#q-s00-cand-266-picture-help) |
| U-123 | 图片显示 / 图片显示 | 复合父项 / 初审开放 | [Q-S00-CAND-267-PICTURE-PARENT](#q-s00-cand-267-picture-parent)、[Q-S00-CAND-268-PICTURE-SOURCE](#q-s00-cand-268-picture-source)、[Q-S00-CAND-269-PICTURE-COMPILE](#q-s00-cand-269-picture-compile) |
| U-124 | 图片显示 / 图片显示 | 说明文字 / 说明上下文保留，关联问题开放 | [Q-S00-CAND-270-PICTURE-SCOPE](#q-s00-cand-270-picture-scope) |
| U-125 | 图表 / 图表 | 复合父项 / 初审开放 | [Q-S00-CAND-271-GRAPH-PARENT](#q-s00-cand-271-graph-parent)、[Q-S00-CAND-272-GRAPH-OPTIONS](#q-s00-cand-272-graph-options)、[Q-S00-CAND-273-GRAPH-UI](#q-s00-cand-273-graph-ui) |
| U-126 | 图表 / 图表 | 单一问题 / 初审开放 | [Q-S00-CAND-274-GRAPH-OPTIONS](#q-s00-cand-274-graph-options) |
| U-127 | 图表 / 图表 | 复合父项 / 初审开放 | [Q-S00-CAND-275-GRAPH-PARENT](#q-s00-cand-275-graph-parent)、[Q-S00-CAND-276-GRAPH-BOUND](#q-s00-cand-276-graph-bound)、[Q-S00-CAND-277-GRAPH-BOUND](#q-s00-cand-277-graph-bound)、[Q-S00-CAND-278-GRAPH-BOUND](#q-s00-cand-278-graph-bound) |
| U-128 | 图表 / 图表 | 复合父项 / 初审开放 | [Q-S00-CAND-279-GRAPH-PARENT](#q-s00-cand-279-graph-parent)、[Q-S00-CAND-280-GRAPH-RENDER](#q-s00-cand-280-graph-render)、[Q-S00-CAND-281-GRAPH-RUNTIME](#q-s00-cand-281-graph-runtime) |
| U-129 | 图表 / 图表 | 单一问题 / 初审开放 | [Q-S00-CAND-282-GRAPH-PERSIST](#q-s00-cand-282-graph-persist) |
| U-130 | 图表 / 图表 | 复合父项 / 初审开放 | [Q-S00-CAND-283-GRAPH-PARENT](#q-s00-cand-283-graph-parent)、[Q-S00-CAND-284-GRAPH-RENDER](#q-s00-cand-284-graph-render)、[Q-S00-CAND-285-GRAPH-RENDER](#q-s00-cand-285-graph-render) |
| U-131 | 图表 / 图表 | 单一问题 / 初审开放 | [Q-S00-CAND-286-GRAPH-PERMISSION](#q-s00-cand-286-graph-permission) |
| U-132 | 图表 / 图表 | 复合父项 / 初审开放 | [Q-S00-CAND-287-GRAPH-PARENT](#q-s00-cand-287-graph-parent)、[Q-S00-CAND-288-GRAPH-RENDER](#q-s00-cand-288-graph-render)、[Q-S00-CAND-289-GRAPH-RUNTIME](#q-s00-cand-289-graph-runtime) |
| U-133 | 图表 / 图表 | 复合父项 / 初审开放 | [Q-S00-CAND-290-GRAPH-PARENT](#q-s00-cand-290-graph-parent)、[Q-S00-CAND-291-GRAPH-SOURCE](#q-s00-cand-291-graph-source)、[Q-S00-CAND-292-GRAPH-COMPILE](#q-s00-cand-292-graph-compile)、[Q-S00-CAND-293-GRAPH-RUNTIME](#q-s00-cand-293-graph-runtime) |
| U-134 | 图表 / 图表 | 单一问题 / 初审开放 | [Q-S00-CAND-294-GRAPH-HELP](#q-s00-cand-294-graph-help) |
| U-135 | 多功能按钮 / 多功能按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-295-MULTIFUNC-PARENT](#q-s00-cand-295-multifunc-parent)、[Q-S00-CAND-296-MULTIFUNC-UI](#q-s00-cand-296-multifunc-ui)、[Q-S00-CAND-297-MULTIFUNC-UI](#q-s00-cand-297-multifunc-ui) |
| U-136 | 多功能按钮 / 多功能按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-298-MULTIFUNC-PARENT](#q-s00-cand-298-multifunc-parent)、[Q-S00-CAND-299-MULTIFUNC-LANGUAGE](#q-s00-cand-299-multifunc-language)、[Q-S00-CAND-300-MULTIFUNC-RENDER](#q-s00-cand-300-multifunc-render)、[Q-S00-CAND-301-MULTIFUNC-RUNTIME](#q-s00-cand-301-multifunc-runtime) |
| U-137 | 多功能按钮 / 多功能按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-302-MULTIFUNC-PARENT](#q-s00-cand-302-multifunc-parent)、[Q-S00-CAND-303-MULTIFUNC-GALLERY](#q-s00-cand-303-multifunc-gallery)、[Q-S00-CAND-304-MULTIFUNC-UI](#q-s00-cand-304-multifunc-ui)、[Q-S00-CAND-305-MULTIFUNC-RENDER](#q-s00-cand-305-multifunc-render) |
| U-138 | 多功能按钮 / 多功能按钮 | 单一问题 / 初审开放 | [Q-S00-CAND-306-MULTIFUNC-ADDRESS](#q-s00-cand-306-multifunc-address) |
| U-139 | 多功能按钮 / 多功能按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-307-MULTIFUNC-PARENT](#q-s00-cand-307-multifunc-parent)、[Q-S00-CAND-308-MULTIFUNC-BOUND](#q-s00-cand-308-multifunc-bound)、[Q-S00-CAND-309-MULTIFUNC-UI](#q-s00-cand-309-multifunc-ui) |
| U-140 | 多功能按钮 / 多功能按钮 | 单一问题 / 初审开放 | [Q-S00-CAND-310-MULTIFUNC-PERSIST](#q-s00-cand-310-multifunc-persist) |
| U-141 | 多功能按钮 / 多功能按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-311-MULTIFUNC-PARENT](#q-s00-cand-311-multifunc-parent)、[Q-S00-CAND-312-MULTIFUNC-RUNTIME](#q-s00-cand-312-multifunc-runtime)、[Q-S00-CAND-313-MULTIFUNC-RUNTIME](#q-s00-cand-313-multifunc-runtime)、[Q-S00-CAND-314-MULTIFUNC-RUNTIME](#q-s00-cand-314-multifunc-runtime) |
| U-142 | 多功能按钮 / 多功能按钮 | 单一问题 / 初审开放 | [Q-S00-CAND-315-MULTIFUNC-RUNTIME](#q-s00-cand-315-multifunc-runtime) |
| U-143 | 多功能按钮 / 多功能按钮 | 单一问题 / 初审开放 | [Q-S00-CAND-316-MULTIFUNC-VISIBILITY](#q-s00-cand-316-multifunc-visibility) |
| U-144 | 多功能按钮 / 多功能按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-317-MULTIFUNC-PARENT](#q-s00-cand-317-multifunc-parent)、[Q-S00-CAND-318-MULTIFUNC-SOURCE](#q-s00-cand-318-multifunc-source)、[Q-S00-CAND-319-MULTIFUNC-COMPILE](#q-s00-cand-319-multifunc-compile)、[Q-S00-CAND-320-MULTIFUNC-RUNTIME](#q-s00-cand-320-multifunc-runtime) |
| U-145 | 多态按钮 / 多态按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-321-MULTISTATE-PARENT](#q-s00-cand-321-multistate-parent)、[Q-S00-CAND-322-MULTISTATE-RUNTIME](#q-s00-cand-322-multistate-runtime)、[Q-S00-CAND-323-MULTISTATE-RUNTIME](#q-s00-cand-323-multistate-runtime) |
| U-146 | 多态按钮 / 多态按钮 | 单一问题 / 初审开放 | [Q-S00-CAND-324-MULTISTATE-SCOPE](#q-s00-cand-324-multistate-scope) |
| U-147 | 多态按钮 / 多态按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-325-MULTISTATE-PARENT](#q-s00-cand-325-multistate-parent)、[Q-S00-CAND-326-MULTISTATE-UI](#q-s00-cand-326-multistate-ui)、[Q-S00-CAND-327-MULTISTATE-UI](#q-s00-cand-327-multistate-ui) |
| U-148 | 多态按钮 / 多态按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-328-MULTISTATE-PARENT](#q-s00-cand-328-multistate-parent)、[Q-S00-CAND-329-MULTISTATE-LANGUAGE](#q-s00-cand-329-multistate-language)、[Q-S00-CAND-330-MULTISTATE-RENDER](#q-s00-cand-330-multistate-render)、[Q-S00-CAND-331-MULTISTATE-RUNTIME](#q-s00-cand-331-multistate-runtime) |
| U-149 | 多态按钮 / 多态按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-332-MULTISTATE-PARENT](#q-s00-cand-332-multistate-parent)、[Q-S00-CAND-333-MULTISTATE-OPTIONS](#q-s00-cand-333-multistate-options)、[Q-S00-CAND-334-MULTISTATE-BOUND](#q-s00-cand-334-multistate-bound) |
| U-150 | 多态按钮 / 多态按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-335-MULTISTATE-PARENT](#q-s00-cand-335-multistate-parent)、[Q-S00-CAND-336-MULTISTATE-BOUND](#q-s00-cand-336-multistate-bound)、[Q-S00-CAND-337-MULTISTATE-RENDER](#q-s00-cand-337-multistate-render)、[Q-S00-CAND-338-MULTISTATE-RUNTIME](#q-s00-cand-338-multistate-runtime) |
| U-151 | 多态按钮 / 多态按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-339-MULTISTATE-PARENT](#q-s00-cand-339-multistate-parent)、[Q-S00-CAND-340-MULTISTATE-PERMISSION](#q-s00-cand-340-multistate-permission)、[Q-S00-CAND-341-MULTISTATE-THRESHOLD](#q-s00-cand-341-multistate-threshold)、[Q-S00-CAND-342-MULTISTATE-THRESHOLD](#q-s00-cand-342-multistate-threshold) |
| U-152 | 多态按钮 / 多态按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-343-MULTISTATE-PARENT](#q-s00-cand-343-multistate-parent)、[Q-S00-CAND-344-MULTISTATE-SCOPE](#q-s00-cand-344-multistate-scope)、[Q-S00-CAND-345-MULTISTATE-RUNTIME](#q-s00-cand-345-multistate-runtime)、[Q-S00-CAND-346-MULTISTATE-SOURCE](#q-s00-cand-346-multistate-source) |
| U-153 | 多态按钮 / 多态按钮 | 单一问题 / 初审开放 | [Q-S00-CAND-347-MULTISTATE-VISIBILITY](#q-s00-cand-347-multistate-visibility) |
| U-154 | 多态按钮 / 多态按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-348-MULTISTATE-PARENT](#q-s00-cand-348-multistate-parent)、[Q-S00-CAND-349-MULTISTATE-SOURCE](#q-s00-cand-349-multistate-source)、[Q-S00-CAND-350-MULTISTATE-COMPILE](#q-s00-cand-350-multistate-compile)、[Q-S00-CAND-351-MULTISTATE-RUNTIME](#q-s00-cand-351-multistate-runtime) |
| U-155 | 多态按钮 / C04指示灯，原多态按钮手册范围转交 | 说明文字 / 说明上下文保留，关联问题开放 | [Q-S00-CAND-352-MULTISTATE-VISIBILITY](#q-s00-cand-352-multistate-visibility) |
| U-156 | 字按钮 / 字按钮 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-353-WORD-PARENT](#q-s00-cand-353-word-parent)、[Q-S00-CAND-354-WORD-UI](#q-s00-cand-354-word-ui)、[Q-S00-CAND-355-WORD-UI](#q-s00-cand-355-word-ui) |
| U-157 | 字按钮 / 字按钮 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-356-WORD-PARENT](#q-s00-cand-356-word-parent)、[Q-S00-CAND-357-WORD-UI](#q-s00-cand-357-word-ui)、[Q-S00-CAND-358-WORD-UI](#q-s00-cand-358-word-ui)、[Q-S00-CAND-359-WORD-UI](#q-s00-cand-359-word-ui) |
| U-158 | 字按钮 / 字按钮 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-360-WORD-PARENT](#q-s00-cand-360-word-parent)、[Q-S00-CAND-361-WORD-OPTIONS](#q-s00-cand-361-word-options)、[Q-S00-CAND-362-WORD-PERSIST](#q-s00-cand-362-word-persist) |
| U-159 | 字按钮 / 字按钮 | 已有证据 / 已有历史补证，整体未关闭 | [Q-S00-CAND-363-WORD-PERSIST](#q-s00-cand-363-word-persist) |
| U-160 | 字按钮 / 字按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-364-WORD-PARENT](#q-s00-cand-364-word-parent)、[Q-S00-CAND-365-WORD-RUNTIME](#q-s00-cand-365-word-runtime)、[Q-S00-CAND-366-WORD-RUNTIME](#q-s00-cand-366-word-runtime)、[Q-S00-CAND-367-WORD-RUNTIME](#q-s00-cand-367-word-runtime)、[Q-S00-CAND-368-WORD-RUNTIME](#q-s00-cand-368-word-runtime)、[Q-S00-CAND-369-WORD-RUNTIME](#q-s00-cand-369-word-runtime)、[Q-S00-CAND-370-WORD-RUNTIME](#q-s00-cand-370-word-runtime)、[Q-S00-CAND-371-WORD-RUNTIME](#q-s00-cand-371-word-runtime) |
| U-161 | 字按钮 / 字按钮 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-372-WORD-PARENT](#q-s00-cand-372-word-parent)、[Q-S00-CAND-373-WORD-BOUND](#q-s00-cand-373-word-bound)、[Q-S00-CAND-374-WORD-BOUND](#q-s00-cand-374-word-bound)、[Q-S00-CAND-375-WORD-RENDER](#q-s00-cand-375-word-render) |
| U-162 | 字按钮 / 字按钮 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-376-WORD-PARENT](#q-s00-cand-376-word-parent)、[Q-S00-CAND-377-WORD-LANGUAGE](#q-s00-cand-377-word-language)、[Q-S00-CAND-378-WORD-RENDER](#q-s00-cand-378-word-render)、[Q-S00-CAND-379-WORD-RUNTIME](#q-s00-cand-379-word-runtime) |
| U-163 | 字按钮 / 字按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-380-WORD-PARENT](#q-s00-cand-380-word-parent)、[Q-S00-CAND-381-WORD-OPTIONS](#q-s00-cand-381-word-options)、[Q-S00-CAND-382-WORD-BOUND](#q-s00-cand-382-word-bound) |
| U-164 | 字按钮 / 字按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-383-WORD-PARENT](#q-s00-cand-383-word-parent)、[Q-S00-CAND-384-WORD-PERMISSION](#q-s00-cand-384-word-permission)、[Q-S00-CAND-385-WORD-THRESHOLD](#q-s00-cand-385-word-threshold)、[Q-S00-CAND-386-WORD-THRESHOLD](#q-s00-cand-386-word-threshold) |
| U-165 | 字按钮 / 字按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-387-WORD-PARENT](#q-s00-cand-387-word-parent)、[Q-S00-CAND-388-WORD-VISIBILITY](#q-s00-cand-388-word-visibility)、[Q-S00-CAND-389-WORD-SCOPE](#q-s00-cand-389-word-scope) |
| U-166 | 字按钮 / 字按钮 | 单一问题 / 初审开放 | [Q-S00-CAND-390-WORD-VISIBILITY](#q-s00-cand-390-word-visibility) |
| U-167 | 字按钮 / 字按钮 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-391-WORD-PARENT](#q-s00-cand-391-word-parent)、[Q-S00-CAND-392-WORD-SOURCE](#q-s00-cand-392-word-source)、[Q-S00-CAND-393-WORD-COMPILE](#q-s00-cand-393-word-compile)、[Q-S00-CAND-394-WORD-RUNTIME](#q-s00-cand-394-word-runtime) |
| U-168 | 报警显示 / 报警显示 | 单一问题 / 初审开放 | [Q-S00-CAND-395-ALARM-UI](#q-s00-cand-395-alarm-ui) |
| U-169 | 报警显示 / 报警显示 | 单一问题 / 初审开放 | [Q-S00-CAND-396-ALARM-OPTIONS](#q-s00-cand-396-alarm-options) |
| U-170 | 报警显示 / 报警显示 | 单一问题 / 初审开放 | [Q-S00-CAND-397-ALARM-DIAGNOSTIC](#q-s00-cand-397-alarm-diagnostic) |
| U-171 | 报警显示 / 报警显示 | 单一问题 / 初审开放 | [Q-S00-CAND-398-ALARM-RUNTIME](#q-s00-cand-398-alarm-runtime) |
| U-172 | 报警显示 / 报警显示 | 说明文字 / 说明上下文保留，关联问题开放 | [Q-S00-CAND-399-ALARM-RUNTIME](#q-s00-cand-399-alarm-runtime) |
| U-173 | 报警显示 / 报警显示 | 复合父项 / 初审开放 | [Q-S00-CAND-400-ALARM-PARENT](#q-s00-cand-400-alarm-parent)、[Q-S00-CAND-401-ALARM-SCOPE](#q-s00-cand-401-alarm-scope)、[Q-S00-CAND-402-ALARM-RUNTIME](#q-s00-cand-402-alarm-runtime) |
| U-174 | 报警显示 / 报警显示 | 复合父项 / 初审开放 | [Q-S00-CAND-403-ALARM-PARENT](#q-s00-cand-403-alarm-parent)、[Q-S00-CAND-404-ALARM-LANGUAGE](#q-s00-cand-404-alarm-language)、[Q-S00-CAND-405-ALARM-RUNTIME](#q-s00-cand-405-alarm-runtime) |
| U-175 | 报警显示 / 报警显示 | 复合父项 / 初审开放 | [Q-S00-CAND-406-ALARM-PARENT](#q-s00-cand-406-alarm-parent)、[Q-S00-CAND-407-ALARM-SOURCE](#q-s00-cand-407-alarm-source)、[Q-S00-CAND-408-ALARM-COMPILE](#q-s00-cand-408-alarm-compile)、[Q-S00-CAND-409-ALARM-RUNTIME](#q-s00-cand-409-alarm-runtime) |
| U-176 | 指示灯 / 指示灯 | 已有证据 / 已有历史补证，整体未关闭 | [Q-S00-CAND-410-LAMP-UI](#q-s00-cand-410-lamp-ui) |
| U-177 | 指示灯 / 指示灯 | 已有证据 / 已有历史补证，整体未关闭 | [Q-S00-CAND-411-LAMP-UI](#q-s00-cand-411-lamp-ui) |
| U-178 | 指示灯 / 指示灯 | 已有证据 / 沿用事实关闭 | [Q-S00-CAND-412-LAMP-FACT](#q-s00-cand-412-lamp-fact) |
| U-179 | 指示灯 / 指示灯 | 单一问题 / 初审开放 | [Q-S00-CAND-413-LAMP-HELP](#q-s00-cand-413-lamp-help) |
| U-180 | 指示灯 / 指示灯 | 已有证据 / 已有历史补证，整体未关闭 | [Q-S00-CAND-414-LAMP-OPTIONS](#q-s00-cand-414-lamp-options) |
| U-181 | 指示灯 / 指示灯 | 复合父项 / 部分关闭：范围事实已闭，渲染分支开放 | [Q-S00-CAND-717-LAMP-PARENT](#q-s00-cand-717-lamp-parent)、[Q-S00-CAND-718-LAMP-FACT](#q-s00-cand-718-lamp-fact)、[Q-S00-CAND-719-LAMP-PARENT](#q-s00-cand-719-lamp-parent)、[Q-S00-CAND-720-LAMP-RENDER](#q-s00-cand-720-lamp-render)、[Q-S00-CAND-721-LAMP-RUNTIME](#q-s00-cand-721-lamp-runtime)、[Q-S00-CAND-722-LAMP-RUNTIME](#q-s00-cand-722-lamp-runtime)、[Q-S00-CAND-723-LAMP-RUNTIME](#q-s00-cand-723-lamp-runtime) |
| U-182 | 指示灯 / 指示灯 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-415-LAMP-PARENT](#q-s00-cand-415-lamp-parent)、[Q-S00-CAND-416-LAMP-PERSIST](#q-s00-cand-416-lamp-persist)、[Q-S00-CAND-417-LAMP-RUNTIME](#q-s00-cand-417-lamp-runtime) |
| U-183 | 指示灯 / 指示灯 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-418-LAMP-PARENT](#q-s00-cand-418-lamp-parent)、[Q-S00-CAND-419-LAMP-RUNTIME](#q-s00-cand-419-lamp-runtime)、[Q-S00-CAND-420-LAMP-RUNTIME](#q-s00-cand-420-lamp-runtime)、[Q-S00-CAND-421-LAMP-PERMISSION](#q-s00-cand-421-lamp-permission) |
| U-184 | 指示灯 / 指示灯 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-422-LAMP-PARENT](#q-s00-cand-422-lamp-parent)、[Q-S00-CAND-423-LAMP-PERSIST](#q-s00-cand-423-lamp-persist)、[Q-S00-CAND-424-LAMP-RENDER](#q-s00-cand-424-lamp-render) |
| U-185 | 指示灯 / 指示灯 | 复合父项 / 初审开放 | [Q-S00-CAND-425-LAMP-PARENT](#q-s00-cand-425-lamp-parent)、[Q-S00-CAND-426-LAMP-OPTIONS](#q-s00-cand-426-lamp-options)、[Q-S00-CAND-427-LAMP-OPTIONS](#q-s00-cand-427-lamp-options)、[Q-S00-CAND-428-LAMP-UI](#q-s00-cand-428-lamp-ui) |
| U-186 | 指示灯 / 指示灯 | 已有证据 / 已有历史补证，整体未关闭 | [Q-S00-CAND-429-LAMP-LANGUAGE](#q-s00-cand-429-lamp-language) |
| U-187 | 指示灯 / 指示灯 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-430-LAMP-PARENT](#q-s00-cand-430-lamp-parent)、[Q-S00-CAND-431-LAMP-SOURCE](#q-s00-cand-431-lamp-source)、[Q-S00-CAND-432-LAMP-COMPILE](#q-s00-cand-432-lamp-compile)、[Q-S00-CAND-433-LAMP-RUNTIME](#q-s00-cand-433-lamp-runtime) |
| U-188 | 数值显示 / 数值显示 | 复合父项 / 初审开放 | [Q-S00-CAND-434-NUMBER-PARENT](#q-s00-cand-434-number-parent)、[Q-S00-CAND-435-NUMBER-UI](#q-s00-cand-435-number-ui)、[Q-S00-CAND-436-NUMBER-UI](#q-s00-cand-436-number-ui) |
| U-189 | 数值显示 / 数值显示 | 复合父项 / 初审开放 | [Q-S00-CAND-437-NUMBER-PARENT](#q-s00-cand-437-number-parent)、[Q-S00-CAND-438-NUMBER-BOUND](#q-s00-cand-438-number-bound)、[Q-S00-CAND-439-NUMBER-BOUND](#q-s00-cand-439-number-bound)、[Q-S00-CAND-440-NUMBER-RENDER](#q-s00-cand-440-number-render) |
| U-190 | 数值显示 / 数值显示 | 单一问题 / 初审开放 | [Q-S00-CAND-441-NUMBER-RENDER](#q-s00-cand-441-number-render) |
| U-191 | 数值显示 / 数值显示 | 复合父项 / 初审开放 | [Q-S00-CAND-442-NUMBER-PARENT](#q-s00-cand-442-number-parent)、[Q-S00-CAND-443-NUMBER-RENDER](#q-s00-cand-443-number-render)、[Q-S00-CAND-444-NUMBER-RUNTIME](#q-s00-cand-444-number-runtime) |
| U-192 | 数值显示 / 数值显示 | 复合父项 / 初审开放 | [Q-S00-CAND-445-NUMBER-PARENT](#q-s00-cand-445-number-parent)、[Q-S00-CAND-446-NUMBER-UI](#q-s00-cand-446-number-ui)、[Q-S00-CAND-447-NUMBER-UI](#q-s00-cand-447-number-ui)、[Q-S00-CAND-448-NUMBER-OPTIONS](#q-s00-cand-448-number-options) |
| U-193 | 数值显示 / 数值显示 | 复合父项 / 初审开放 | [Q-S00-CAND-449-NUMBER-PARENT](#q-s00-cand-449-number-parent)、[Q-S00-CAND-450-NUMBER-GALLERY](#q-s00-cand-450-number-gallery)、[Q-S00-CAND-451-NUMBER-GALLERY](#q-s00-cand-451-number-gallery) |
| U-194 | 数值显示 / 数值显示 | 单一问题 / 初审开放 | [Q-S00-CAND-452-NUMBER-UI](#q-s00-cand-452-number-ui) |
| U-195 | 数值显示 / 数值显示 | 复合父项 / 初审开放 | [Q-S00-CAND-453-NUMBER-PARENT](#q-s00-cand-453-number-parent)、[Q-S00-CAND-454-NUMBER-BOUND](#q-s00-cand-454-number-bound)、[Q-S00-CAND-455-NUMBER-BOUND](#q-s00-cand-455-number-bound) |
| U-196 | 数值显示 / 数值显示 | 复合父项 / 初审开放 | [Q-S00-CAND-456-NUMBER-PARENT](#q-s00-cand-456-number-parent)、[Q-S00-CAND-457-NUMBER-RENDER](#q-s00-cand-457-number-render)、[Q-S00-CAND-458-NUMBER-VISIBILITY](#q-s00-cand-458-number-visibility) |
| U-197 | 数值显示 / 数值显示 | 单一问题 / 初审开放 | [Q-S00-CAND-459-NUMBER-UI](#q-s00-cand-459-number-ui) |
| U-198 | 数值显示 / 数值显示 | 复合父项 / 初审开放 | [Q-S00-CAND-460-NUMBER-PARENT](#q-s00-cand-460-number-parent)、[Q-S00-CAND-461-NUMBER-SOURCE](#q-s00-cand-461-number-source)、[Q-S00-CAND-462-NUMBER-COMPILE](#q-s00-cand-462-number-compile)、[Q-S00-CAND-463-NUMBER-RUNTIME](#q-s00-cand-463-number-runtime) |
| U-199 | 数值显示 / 数值显示 | 单一问题 / 初审开放 | [Q-S00-CAND-464-NUMBER-HELP](#q-s00-cand-464-number-help) |
| U-200 | 数据群组 / 数据群组 | 复合父项 / 初审开放 | [Q-S00-CAND-465-DATAGROUP-PARENT](#q-s00-cand-465-datagroup-parent)、[Q-S00-CAND-466-DATAGROUP-BOUND](#q-s00-cand-466-datagroup-bound)、[Q-S00-CAND-467-DATAGROUP-BOUND](#q-s00-cand-467-datagroup-bound) |
| U-201 | 数据群组 / 数据群组 | 单一问题 / 初审开放 | [Q-S00-CAND-468-DATAGROUP-BOUND](#q-s00-cand-468-datagroup-bound) |
| U-202 | 数据群组 / 数据群组 | 单一问题 / 初审开放 | [Q-S00-CAND-469-DATAGROUP-OPTIONS](#q-s00-cand-469-datagroup-options) |
| U-203 | 数据群组 / 数据群组 | 复合父项 / 初审开放 | [Q-S00-CAND-470-DATAGROUP-PARENT](#q-s00-cand-470-datagroup-parent)、[Q-S00-CAND-471-DATAGROUP-BOUND](#q-s00-cand-471-datagroup-bound)、[Q-S00-CAND-472-DATAGROUP-UI](#q-s00-cand-472-datagroup-ui) |
| U-204 | 数据群组 / 数据群组 | 单一问题 / 初审开放 | [Q-S00-CAND-473-DATAGROUP-RENDER](#q-s00-cand-473-datagroup-render) |
| U-205 | 数据群组 / 数据群组 | 单一问题 / 初审开放 | [Q-S00-CAND-474-DATAGROUP-UI](#q-s00-cand-474-datagroup-ui) |
| U-206 | 数据群组 / 数据群组 | 单一问题 / 初审开放 | [Q-S00-CAND-475-DATAGROUP-PERSIST](#q-s00-cand-475-datagroup-persist) |
| U-207 | 数据群组 / 数据群组 | 单一问题 / 初审开放 | [Q-S00-CAND-476-DATAGROUP-HELP](#q-s00-cand-476-datagroup-help) |
| U-208 | 数据群组 / 数据群组 | 复合父项 / 初审开放 | [Q-S00-CAND-477-DATAGROUP-PARENT](#q-s00-cand-477-datagroup-parent)、[Q-S00-CAND-478-DATAGROUP-RUNTIME](#q-s00-cand-478-datagroup-runtime)、[Q-S00-CAND-479-DATAGROUP-RUNTIME](#q-s00-cand-479-datagroup-runtime)、[Q-S00-CAND-480-DATAGROUP-RUNTIME](#q-s00-cand-480-datagroup-runtime) |
| U-209 | 数据群组 / 数据群组 | 说明文字 / 说明上下文保留，关联问题开放 | [Q-S00-CAND-481-DATAGROUP-DIAGNOSTIC](#q-s00-cand-481-datagroup-diagnostic) |
| U-210 | 时间显示 / 时间显示 | 单一问题 / 初审开放 | [Q-S00-CAND-482-TIME-UI](#q-s00-cand-482-time-ui) |
| U-211 | 时间显示 / 时间显示 | 复合父项 / 初审开放 | [Q-S00-CAND-483-TIME-PARENT](#q-s00-cand-483-time-parent)、[Q-S00-CAND-484-TIME-OPTIONS](#q-s00-cand-484-time-options)、[Q-S00-CAND-485-TIME-OPTIONS](#q-s00-cand-485-time-options) |
| U-212 | 时间显示 / 时间显示 | 单一问题 / 初审开放 | [Q-S00-CAND-486-TIME-RUNTIME](#q-s00-cand-486-time-runtime) |
| U-213 | 时间显示 / 时间显示 | 单一问题 / 初审开放 | [Q-S00-CAND-487-TIME-RUNTIME](#q-s00-cand-487-time-runtime) |
| U-214 | 时间显示 / 时间显示 | 复合父项 / 初审开放 | [Q-S00-CAND-488-TIME-PARENT](#q-s00-cand-488-time-parent)、[Q-S00-CAND-489-TIME-GALLERY](#q-s00-cand-489-time-gallery)、[Q-S00-CAND-490-TIME-GALLERY](#q-s00-cand-490-time-gallery) |
| U-215 | 时间显示 / 时间显示 | 复合父项 / 初审开放 | [Q-S00-CAND-491-TIME-PARENT](#q-s00-cand-491-time-parent)、[Q-S00-CAND-492-TIME-RENDER](#q-s00-cand-492-time-render)、[Q-S00-CAND-493-TIME-RENDER](#q-s00-cand-493-time-render) |
| U-216 | 时间显示 / 时间显示 | 复合父项 / 初审开放 | [Q-S00-CAND-494-TIME-PARENT](#q-s00-cand-494-time-parent)、[Q-S00-CAND-495-TIME-UI](#q-s00-cand-495-time-ui)、[Q-S00-CAND-496-TIME-UI](#q-s00-cand-496-time-ui) |
| U-217 | 时间显示 / 时间显示 | 单一问题 / 初审开放 | [Q-S00-CAND-497-TIME-PERSIST](#q-s00-cand-497-time-persist) |
| U-218 | 时间显示 / 时间显示 | 复合父项 / 初审开放 | [Q-S00-CAND-498-TIME-PARENT](#q-s00-cand-498-time-parent)、[Q-S00-CAND-499-TIME-SOURCE](#q-s00-cand-499-time-source)、[Q-S00-CAND-500-TIME-COMPILE](#q-s00-cand-500-time-compile)、[Q-S00-CAND-501-TIME-RUNTIME](#q-s00-cand-501-time-runtime) |
| U-219 | 时间显示 / 时间显示 | 单一问题 / 初审开放 | [Q-S00-CAND-502-TIME-HELP](#q-s00-cand-502-time-help) |
| U-220 | 时间显示 / 时间显示 | 单一问题 / 初审开放 | [Q-S00-CAND-503-TIME-UI](#q-s00-cand-503-time-ui) |
| U-221 | 时间显示 / 时间显示 | 单一问题 / 初审开放 | [Q-S00-CAND-504-TIME-PERSIST](#q-s00-cand-504-time-persist) |
| U-222 | 流动块 / 流动块 | 单一问题 / 初审开放 | [Q-S00-CAND-505-FLOW-BOUND](#q-s00-cand-505-flow-bound) |
| U-223 | 流动块 / 流动块 | 单一问题 / 初审开放 | [Q-S00-CAND-506-FLOW-RUNTIME](#q-s00-cand-506-flow-runtime) |
| U-224 | 流动块 / 流动块 | 复合父项 / 初审开放 | [Q-S00-CAND-507-FLOW-PARENT](#q-s00-cand-507-flow-parent)、[Q-S00-CAND-508-FLOW-OPTIONS](#q-s00-cand-508-flow-options)、[Q-S00-CAND-509-FLOW-OPTIONS](#q-s00-cand-509-flow-options)、[Q-S00-CAND-510-FLOW-UI](#q-s00-cand-510-flow-ui) |
| U-225 | 流动块 / 流动块 | 复合父项 / 初审开放 | [Q-S00-CAND-511-FLOW-PARENT](#q-s00-cand-511-flow-parent)、[Q-S00-CAND-512-FLOW-RENDER](#q-s00-cand-512-flow-render)、[Q-S00-CAND-513-FLOW-RUNTIME](#q-s00-cand-513-flow-runtime) |
| U-226 | 流动块 / 流动块 | 单一问题 / 初审开放 | [Q-S00-CAND-514-FLOW-UI](#q-s00-cand-514-flow-ui) |
| U-227 | 流动块 / 流动块 | 复合父项 / 初审开放 | [Q-S00-CAND-515-FLOW-PARENT](#q-s00-cand-515-flow-parent)、[Q-S00-CAND-516-FLOW-SCOPE](#q-s00-cand-516-flow-scope)、[Q-S00-CAND-517-FLOW-ADDRESS](#q-s00-cand-517-flow-address) |
| U-228 | 流动块 / 流动块 | 复合父项 / 初审开放 | [Q-S00-CAND-518-FLOW-PARENT](#q-s00-cand-518-flow-parent)、[Q-S00-CAND-519-FLOW-SOURCE](#q-s00-cand-519-flow-source)、[Q-S00-CAND-520-FLOW-COMPILE](#q-s00-cand-520-flow-compile)、[Q-S00-CAND-521-FLOW-RUNTIME](#q-s00-cand-521-flow-runtime) |
| U-229 | 消息显示 / 消息显示 | 单一问题 / 初审开放 | [Q-S00-CAND-522-MESSAGE-BOUND](#q-s00-cand-522-message-bound) |
| U-230 | 消息显示 / 消息显示 | 单一问题 / 初审开放 | [Q-S00-CAND-523-MESSAGE-ADDRESS](#q-s00-cand-523-message-address) |
| U-231 | 消息显示 / 消息显示 | 复合父项 / 初审开放 | [Q-S00-CAND-524-MESSAGE-PARENT](#q-s00-cand-524-message-parent)、[Q-S00-CAND-525-MESSAGE-BOUND](#q-s00-cand-525-message-bound)、[Q-S00-CAND-526-MESSAGE-RUNTIME](#q-s00-cand-526-message-runtime) |
| U-232 | 消息显示 / 消息显示 | 单一问题 / 初审开放 | [Q-S00-CAND-527-MESSAGE-LANGUAGE](#q-s00-cand-527-message-language) |
| U-233 | 消息显示 / 消息显示 | 单一问题 / 初审开放 | [Q-S00-CAND-528-MESSAGE-OPTIONS](#q-s00-cand-528-message-options) |
| U-234 | 消息显示 / 消息显示 | 单一问题 / 初审开放 | [Q-S00-CAND-529-MESSAGE-OPTIONS](#q-s00-cand-529-message-options) |
| U-235 | 消息显示 / 消息显示 | 单一问题 / 初审开放 | [Q-S00-CAND-530-MESSAGE-BOUND](#q-s00-cand-530-message-bound) |
| U-236 | 消息显示 / 消息显示 | 复合父项 / 初审开放 | [Q-S00-CAND-531-MESSAGE-PARENT](#q-s00-cand-531-message-parent)、[Q-S00-CAND-532-MESSAGE-PERMISSION](#q-s00-cand-532-message-permission)、[Q-S00-CAND-533-MESSAGE-THRESHOLD](#q-s00-cand-533-message-threshold)、[Q-S00-CAND-534-MESSAGE-THRESHOLD](#q-s00-cand-534-message-threshold) |
| U-237 | 消息显示 / 消息显示 | 单一问题 / 初审开放 | [Q-S00-CAND-535-MESSAGE-HELP](#q-s00-cand-535-message-help) |
| U-238 | 消息显示 / 消息显示 | 单一问题 / 初审开放 | [Q-S00-CAND-536-MESSAGE-VISIBILITY](#q-s00-cand-536-message-visibility) |
| U-239 | 消息显示 / 消息显示 | 复合父项 / 初审开放 | [Q-S00-CAND-537-MESSAGE-PARENT](#q-s00-cand-537-message-parent)、[Q-S00-CAND-538-MESSAGE-SOURCE](#q-s00-cand-538-message-source)、[Q-S00-CAND-539-MESSAGE-COMPILE](#q-s00-cand-539-message-compile)、[Q-S00-CAND-540-MESSAGE-RUNTIME](#q-s00-cand-540-message-runtime) |
| U-240 | 消息显示 / 消息显示 | 复合父项 / 初审开放 | [Q-S00-CAND-541-MESSAGE-PARENT](#q-s00-cand-541-message-parent)、[Q-S00-CAND-542-MESSAGE-SCOPE](#q-s00-cand-542-message-scope)、[Q-S00-CAND-543-MESSAGE-RUNTIME](#q-s00-cand-543-message-runtime) |
| U-241 | 滑动开关 / 滑动开关 | 复合父项 / 初审开放 | [Q-S00-CAND-544-SLIDE-PARENT](#q-s00-cand-544-slide-parent)、[Q-S00-CAND-545-SLIDE-UI](#q-s00-cand-545-slide-ui)、[Q-S00-CAND-546-SLIDE-RENDER](#q-s00-cand-546-slide-render)、[Q-S00-CAND-547-SLIDE-PERSIST](#q-s00-cand-547-slide-persist) |
| U-242 | 滑动开关 / 滑动开关 | 单一问题 / 初审开放 | [Q-S00-CAND-548-SLIDE-SCOPE](#q-s00-cand-548-slide-scope) |
| U-243 | 滑动开关 / 滑动开关 | 单一问题 / 初审开放 | [Q-S00-CAND-549-SLIDE-RUNTIME](#q-s00-cand-549-slide-runtime) |
| U-244 | 滑动开关 / 滑动开关 | 复合父项 / 初审开放 | [Q-S00-CAND-550-SLIDE-PARENT](#q-s00-cand-550-slide-parent)、[Q-S00-CAND-551-SLIDE-UI](#q-s00-cand-551-slide-ui)、[Q-S00-CAND-552-SLIDE-RENDER](#q-s00-cand-552-slide-render) |
| U-245 | 滑动开关 / 滑动开关 | 复合父项 / 初审开放 | [Q-S00-CAND-553-SLIDE-PARENT](#q-s00-cand-553-slide-parent)、[Q-S00-CAND-554-SLIDE-UI](#q-s00-cand-554-slide-ui)、[Q-S00-CAND-555-SLIDE-ADDRESS](#q-s00-cand-555-slide-address) |
| U-246 | 滑动开关 / 滑动开关 | 复合父项 / 初审开放 | [Q-S00-CAND-556-SLIDE-PARENT](#q-s00-cand-556-slide-parent)、[Q-S00-CAND-557-SLIDE-BOUND](#q-s00-cand-557-slide-bound)、[Q-S00-CAND-558-SLIDE-BOUND](#q-s00-cand-558-slide-bound) |
| U-247 | 滑动开关 / 滑动开关 | 复合父项 / 初审开放 | [Q-S00-CAND-559-SLIDE-PARENT](#q-s00-cand-559-slide-parent)、[Q-S00-CAND-560-SLIDE-PERSIST](#q-s00-cand-560-slide-persist)、[Q-S00-CAND-561-SLIDE-UI](#q-s00-cand-561-slide-ui)、[Q-S00-CAND-562-SLIDE-UI](#q-s00-cand-562-slide-ui) |
| U-248 | 滑动开关 / 滑动开关 | 单一问题 / 初审开放 | [Q-S00-CAND-563-SLIDE-UI](#q-s00-cand-563-slide-ui) |
| U-249 | 滑动开关 / 滑动开关 | 复合父项 / 初审开放 | [Q-S00-CAND-564-SLIDE-PARENT](#q-s00-cand-564-slide-parent)、[Q-S00-CAND-565-SLIDE-RUNTIME](#q-s00-cand-565-slide-runtime)、[Q-S00-CAND-566-SLIDE-RUNTIME](#q-s00-cand-566-slide-runtime) |
| U-250 | 滑动开关 / 滑动开关 | 复合父项 / 初审开放 | [Q-S00-CAND-567-SLIDE-PARENT](#q-s00-cand-567-slide-parent)、[Q-S00-CAND-568-SLIDE-SOURCE](#q-s00-cand-568-slide-source)、[Q-S00-CAND-569-SLIDE-COMPILE](#q-s00-cand-569-slide-compile)、[Q-S00-CAND-570-SLIDE-RUNTIME](#q-s00-cand-570-slide-runtime) |
| U-251 | 滑动开关 / 滑动开关 | 说明文字 / 说明上下文保留，关联问题开放 | [Q-S00-CAND-571-SLIDE-SCOPE](#q-s00-cand-571-slide-scope) |
| U-252 | 滚轮 / 滚轮 | 复合父项 / 初审开放 | [Q-S00-CAND-572-WHEEL-PARENT](#q-s00-cand-572-wheel-parent)、[Q-S00-CAND-573-WHEEL-BOUND](#q-s00-cand-573-wheel-bound)、[Q-S00-CAND-574-WHEEL-RUNTIME](#q-s00-cand-574-wheel-runtime) |
| U-253 | 滚轮 / 滚轮 | 复合父项 / 初审开放 | [Q-S00-CAND-575-WHEEL-PARENT](#q-s00-cand-575-wheel-parent)、[Q-S00-CAND-576-WHEEL-BOUND](#q-s00-cand-576-wheel-bound)、[Q-S00-CAND-577-WHEEL-RUNTIME](#q-s00-cand-577-wheel-runtime) |
| U-254 | 滚轮 / 滚轮 | 复合父项 / 初审开放 | [Q-S00-CAND-578-WHEEL-PARENT](#q-s00-cand-578-wheel-parent)、[Q-S00-CAND-579-WHEEL-OPTIONS](#q-s00-cand-579-wheel-options)、[Q-S00-CAND-580-WHEEL-OPTIONS](#q-s00-cand-580-wheel-options) |
| U-255 | 滚轮 / 滚轮 | 单一问题 / 初审开放 | [Q-S00-CAND-581-WHEEL-RENDER](#q-s00-cand-581-wheel-render) |
| U-256 | 滚轮 / 滚轮 | 复合父项 / 初审开放 | [Q-S00-CAND-582-WHEEL-PARENT](#q-s00-cand-582-wheel-parent)、[Q-S00-CAND-583-WHEEL-GALLERY](#q-s00-cand-583-wheel-gallery)、[Q-S00-CAND-584-WHEEL-GALLERY](#q-s00-cand-584-wheel-gallery)、[Q-S00-CAND-585-WHEEL-UI](#q-s00-cand-585-wheel-ui) |
| U-257 | 滚轮 / 滚轮 | 单一问题 / 初审开放 | [Q-S00-CAND-586-WHEEL-RUNTIME](#q-s00-cand-586-wheel-runtime) |
| U-258 | 滚轮 / 滚轮 | 复合父项 / 初审开放 | [Q-S00-CAND-587-WHEEL-PARENT](#q-s00-cand-587-wheel-parent)、[Q-S00-CAND-588-WHEEL-OPTIONS](#q-s00-cand-588-wheel-options)、[Q-S00-CAND-589-WHEEL-OPTIONS](#q-s00-cand-589-wheel-options) |
| U-259 | 滚轮 / 滚轮 | 单一问题 / 初审开放 | [Q-S00-CAND-590-WHEEL-VISIBILITY](#q-s00-cand-590-wheel-visibility) |
| U-260 | 滚轮 / 滚轮 | 复合父项 / 初审开放 | [Q-S00-CAND-591-WHEEL-PARENT](#q-s00-cand-591-wheel-parent)、[Q-S00-CAND-592-WHEEL-SOURCE](#q-s00-cand-592-wheel-source)、[Q-S00-CAND-593-WHEEL-COMPILE](#q-s00-cand-593-wheel-compile)、[Q-S00-CAND-594-WHEEL-RUNTIME](#q-s00-cand-594-wheel-runtime) |
| U-261 | 画面按钮 / 画面按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-595-SCREEN-PARENT](#q-s00-cand-595-screen-parent)、[Q-S00-CAND-596-SCREEN-RUNTIME](#q-s00-cand-596-screen-runtime)、[Q-S00-CAND-597-SCREEN-RUNTIME](#q-s00-cand-597-screen-runtime)、[Q-S00-CAND-598-SCREEN-RUNTIME](#q-s00-cand-598-screen-runtime) |
| U-262 | 画面按钮 / 画面按钮 | 单一问题 / 初审开放 | [Q-S00-CAND-599-SCREEN-UI](#q-s00-cand-599-screen-ui) |
| U-263 | 画面按钮 / 画面按钮 | 单一问题 / 初审开放 | [Q-S00-CAND-600-SCREEN-UI](#q-s00-cand-600-screen-ui) |
| U-264 | 画面按钮 / 画面按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-601-SCREEN-PARENT](#q-s00-cand-601-screen-parent)、[Q-S00-CAND-602-SCREEN-UI](#q-s00-cand-602-screen-ui)、[Q-S00-CAND-603-SCREEN-UI](#q-s00-cand-603-screen-ui) |
| U-265 | 画面按钮 / 画面按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-604-SCREEN-PARENT](#q-s00-cand-604-screen-parent)、[Q-S00-CAND-605-SCREEN-UI](#q-s00-cand-605-screen-ui)、[Q-S00-CAND-606-SCREEN-UI](#q-s00-cand-606-screen-ui) |
| U-266 | 画面按钮 / 画面按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-607-SCREEN-PARENT](#q-s00-cand-607-screen-parent)、[Q-S00-CAND-608-SCREEN-UI](#q-s00-cand-608-screen-ui)、[Q-S00-CAND-609-SCREEN-LANGUAGE](#q-s00-cand-609-screen-language) |
| U-267 | 画面按钮 / 画面按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-610-SCREEN-PARENT](#q-s00-cand-610-screen-parent)、[Q-S00-CAND-611-SCREEN-GALLERY](#q-s00-cand-611-screen-gallery)、[Q-S00-CAND-612-SCREEN-GALLERY](#q-s00-cand-612-screen-gallery) |
| U-268 | 画面按钮 / 画面按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-613-SCREEN-PARENT](#q-s00-cand-613-screen-parent)、[Q-S00-CAND-614-SCREEN-ADDRESS](#q-s00-cand-614-screen-address)、[Q-S00-CAND-615-SCREEN-ADDRESS](#q-s00-cand-615-screen-address) |
| U-269 | 画面按钮 / 画面按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-616-SCREEN-PARENT](#q-s00-cand-616-screen-parent)、[Q-S00-CAND-617-SCREEN-PERMISSION](#q-s00-cand-617-screen-permission)、[Q-S00-CAND-618-SCREEN-THRESHOLD](#q-s00-cand-618-screen-threshold)、[Q-S00-CAND-619-SCREEN-THRESHOLD](#q-s00-cand-619-screen-threshold) |
| U-270 | 画面按钮 / 画面按钮 | 单一问题 / 初审开放 | [Q-S00-CAND-620-SCREEN-RUNTIME](#q-s00-cand-620-screen-runtime) |
| U-271 | 画面按钮 / 画面按钮 | 单一问题 / 初审开放 | [Q-S00-CAND-621-SCREEN-VISIBILITY](#q-s00-cand-621-screen-visibility) |
| U-272 | 画面按钮 / 画面按钮 | 复合父项 / 初审开放 | [Q-S00-CAND-622-SCREEN-PARENT](#q-s00-cand-622-screen-parent)、[Q-S00-CAND-623-SCREEN-SOURCE](#q-s00-cand-623-screen-source)、[Q-S00-CAND-624-SCREEN-COMPILE](#q-s00-cand-624-screen-compile)、[Q-S00-CAND-625-SCREEN-RUNTIME](#q-s00-cand-625-screen-runtime) |
| U-273 | 画面按钮 / 画面按钮 | 说明文字 / 说明上下文保留，关联问题开放 | [Q-S00-CAND-626-SCREEN-DECISION](#q-s00-cand-626-screen-decision) |
| U-274 | 移动轨迹 / 移动轨迹 | 复合父项 / 初审开放 | [Q-S00-CAND-627-TRACK-PARENT](#q-s00-cand-627-track-parent)、[Q-S00-CAND-628-TRACK-UI](#q-s00-cand-628-track-ui)、[Q-S00-CAND-629-TRACK-RUNTIME](#q-s00-cand-629-track-runtime) |
| U-275 | 移动轨迹 / 移动轨迹 | 复合父项 / 初审开放 | [Q-S00-CAND-630-TRACK-PARENT](#q-s00-cand-630-track-parent)、[Q-S00-CAND-631-TRACK-BOUND](#q-s00-cand-631-track-bound)、[Q-S00-CAND-632-TRACK-BOUND](#q-s00-cand-632-track-bound) |
| U-276 | 移动轨迹 / 移动轨迹 | 复合父项 / 初审开放 | [Q-S00-CAND-633-TRACK-PARENT](#q-s00-cand-633-track-parent)、[Q-S00-CAND-634-TRACK-GALLERY](#q-s00-cand-634-track-gallery)、[Q-S00-CAND-635-TRACK-GALLERY](#q-s00-cand-635-track-gallery) |
| U-277 | 移动轨迹 / 移动轨迹 | 单一问题 / 初审开放 | [Q-S00-CAND-636-TRACK-VISIBILITY](#q-s00-cand-636-track-visibility) |
| U-278 | 移动轨迹 / 移动轨迹 | 单一问题 / 初审开放 | [Q-S00-CAND-637-TRACK-RUNTIME](#q-s00-cand-637-track-runtime) |
| U-279 | 移动轨迹 / 移动轨迹 | 复合父项 / 初审开放 | [Q-S00-CAND-638-TRACK-PARENT](#q-s00-cand-638-track-parent)、[Q-S00-CAND-639-TRACK-OPTIONS](#q-s00-cand-639-track-options)、[Q-S00-CAND-640-TRACK-OPTIONS](#q-s00-cand-640-track-options)、[Q-S00-CAND-641-TRACK-UI](#q-s00-cand-641-track-ui) |
| U-280 | 移动轨迹 / 移动轨迹 | 复合父项 / 初审开放 | [Q-S00-CAND-642-TRACK-PARENT](#q-s00-cand-642-track-parent)、[Q-S00-CAND-643-TRACK-SOURCE](#q-s00-cand-643-track-source)、[Q-S00-CAND-644-TRACK-COMPILE](#q-s00-cand-644-track-compile)、[Q-S00-CAND-645-TRACK-RUNTIME](#q-s00-cand-645-track-runtime) |
| U-281 | 趋势图 / 趋势图 | 复合父项 / 初审开放 | [Q-S00-CAND-646-TREND-PARENT](#q-s00-cand-646-trend-parent)、[Q-S00-CAND-647-TREND-BOUND](#q-s00-cand-647-trend-bound)、[Q-S00-CAND-648-TREND-BOUND](#q-s00-cand-648-trend-bound) |
| U-282 | 趋势图 / 趋势图 | 单一问题 / 初审开放 | [Q-S00-CAND-649-TREND-BOUND](#q-s00-cand-649-trend-bound) |
| U-283 | 趋势图 / 趋势图 | 复合父项 / 初审开放 | [Q-S00-CAND-650-TREND-PARENT](#q-s00-cand-650-trend-parent)、[Q-S00-CAND-651-TREND-BOUND](#q-s00-cand-651-trend-bound)、[Q-S00-CAND-652-TREND-BOUND](#q-s00-cand-652-trend-bound) |
| U-284 | 趋势图 / 趋势图 | 单一问题 / 初审开放 | [Q-S00-CAND-653-TREND-RUNTIME](#q-s00-cand-653-trend-runtime) |
| U-285 | 趋势图 / 趋势图 | 复合父项 / 初审开放 | [Q-S00-CAND-654-TREND-PARENT](#q-s00-cand-654-trend-parent)、[Q-S00-CAND-655-TREND-RUNTIME](#q-s00-cand-655-trend-runtime)、[Q-S00-CAND-656-TREND-RUNTIME](#q-s00-cand-656-trend-runtime) |
| U-286 | 趋势图 / 趋势图 | 单一问题 / 初审开放 | [Q-S00-CAND-657-TREND-UI](#q-s00-cand-657-trend-ui) |
| U-287 | 趋势图 / 趋势图 | 单一问题 / 初审开放 | [Q-S00-CAND-658-TREND-UI](#q-s00-cand-658-trend-ui) |
| U-288 | 趋势图 / 趋势图 | 复合父项 / 初审开放 | [Q-S00-CAND-659-TREND-PARENT](#q-s00-cand-659-trend-parent)、[Q-S00-CAND-660-TREND-BOUND](#q-s00-cand-660-trend-bound)、[Q-S00-CAND-661-TREND-BOUND](#q-s00-cand-661-trend-bound) |
| U-289 | 趋势图 / 趋势图 | 单一问题 / 初审开放 | [Q-S00-CAND-662-TREND-VISIBILITY](#q-s00-cand-662-trend-visibility) |
| U-290 | 趋势图 / 趋势图 | 复合父项 / 初审开放 | [Q-S00-CAND-663-TREND-PARENT](#q-s00-cand-663-trend-parent)、[Q-S00-CAND-664-TREND-SOURCE](#q-s00-cand-664-trend-source)、[Q-S00-CAND-665-TREND-COMPILE](#q-s00-cand-665-trend-compile)、[Q-S00-CAND-666-TREND-RUNTIME](#q-s00-cand-666-trend-runtime) |
| U-291 | 趋势图 / 趋势图 | 单一问题 / 初审开放 | [Q-S00-CAND-667-TREND-HELP](#q-s00-cand-667-trend-help) |
| U-292 | 趋势图 / 趋势图 | 复合父项 / 初审开放 | [Q-S00-CAND-668-TREND-PARENT](#q-s00-cand-668-trend-parent)、[Q-S00-CAND-669-TREND-RUNTIME](#q-s00-cand-669-trend-runtime)、[Q-S00-CAND-670-TREND-RUNTIME](#q-s00-cand-670-trend-runtime) |
| U-293 | 配方显示器 / 配方显示器 | 复合父项 / 初审开放 | [Q-S00-CAND-671-RECIPE-PARENT](#q-s00-cand-671-recipe-parent)、[Q-S00-CAND-672-RECIPE-UI](#q-s00-cand-672-recipe-ui)、[Q-S00-CAND-673-RECIPE-DIAGNOSTIC](#q-s00-cand-673-recipe-diagnostic) |
| U-294 | 配方显示器 / 配方显示器 | 单一问题 / 初审开放 | [Q-S00-CAND-674-RECIPE-RENDER](#q-s00-cand-674-recipe-render) |
| U-295 | 配方显示器 / 配方显示器 | 单一问题 / 初审开放 | [Q-S00-CAND-675-RECIPE-UI](#q-s00-cand-675-recipe-ui) |
| U-296 | 配方显示器 / 配方显示器 | 单一问题 / 初审开放 | [Q-S00-CAND-676-RECIPE-UI](#q-s00-cand-676-recipe-ui) |
| U-297 | 配方显示器 / 配方显示器 | 复合父项 / 初审开放 | [Q-S00-CAND-677-RECIPE-PARENT](#q-s00-cand-677-recipe-parent)、[Q-S00-CAND-678-RECIPE-PERSIST](#q-s00-cand-678-recipe-persist)、[Q-S00-CAND-679-RECIPE-UI](#q-s00-cand-679-recipe-ui)、[Q-S00-CAND-680-RECIPE-HELP](#q-s00-cand-680-recipe-help) |
| U-298 | 配方显示器 / A02配方配置/C31配方显示 | 复合父项 / 初审开放 | [Q-S00-CAND-681-RECIPE-PARENT](#q-s00-cand-681-recipe-parent)、[Q-S00-CAND-682-RECIPE-UI](#q-s00-cand-682-recipe-ui)、[Q-S00-CAND-683-RECIPE-BOUND](#q-s00-cand-683-recipe-bound)、[Q-S00-CAND-684-RECIPE-PERSIST](#q-s00-cand-684-recipe-persist) |
| U-299 | 配方显示器 / 配方显示器 | 单一问题 / 初审开放 | [Q-S00-CAND-685-RECIPE-UI](#q-s00-cand-685-recipe-ui) |
| U-300 | 配方显示器 / 配方显示器 | 复合父项 / 初审开放 | [Q-S00-CAND-686-RECIPE-PARENT](#q-s00-cand-686-recipe-parent)、[Q-S00-CAND-687-RECIPE-DIAGNOSTIC](#q-s00-cand-687-recipe-diagnostic)、[Q-S00-CAND-688-RECIPE-UI](#q-s00-cand-688-recipe-ui) |
| U-301 | 静态文本 / 静态文本 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-689-TEXT-PARENT](#q-s00-cand-689-text-parent)、[Q-S00-CAND-690-TEXT-BOUND](#q-s00-cand-690-text-bound)、[Q-S00-CAND-691-TEXT-BOUND](#q-s00-cand-691-text-bound)、[Q-S00-CAND-692-TEXT-BOUND](#q-s00-cand-692-text-bound)、[Q-S00-CAND-693-TEXT-BOUND](#q-s00-cand-693-text-bound) |
| U-302 | 静态文本 / 静态文本 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-694-TEXT-PARENT](#q-s00-cand-694-text-parent)、[Q-S00-CAND-695-TEXT-RENDER](#q-s00-cand-695-text-render)、[Q-S00-CAND-696-TEXT-RENDER](#q-s00-cand-696-text-render)、[Q-S00-CAND-697-TEXT-VISIBILITY](#q-s00-cand-697-text-visibility) |
| U-303 | 静态文本 / 静态文本 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-698-TEXT-PARENT](#q-s00-cand-698-text-parent)、[Q-S00-CAND-699-TEXT-SOURCE](#q-s00-cand-699-text-source)、[Q-S00-CAND-700-TEXT-RUNTIME](#q-s00-cand-700-text-runtime) |
| U-304 | 静态文本 / 静态文本 | 复合父项 / 初审开放 | [Q-S00-CAND-701-TEXT-PARENT](#q-s00-cand-701-text-parent)、[Q-S00-CAND-702-TEXT-PERMISSION](#q-s00-cand-702-text-permission)、[Q-S00-CAND-703-TEXT-THRESHOLD](#q-s00-cand-703-text-threshold)、[Q-S00-CAND-704-TEXT-THRESHOLD](#q-s00-cand-704-text-threshold) |
| U-305 | 静态文本 / 静态文本 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-705-TEXT-PARENT](#q-s00-cand-705-text-parent)、[Q-S00-CAND-706-TEXT-UI](#q-s00-cand-706-text-ui)、[Q-S00-CAND-707-TEXT-HELP](#q-s00-cand-707-text-help)、[Q-S00-CAND-708-TEXT-UI](#q-s00-cand-708-text-ui) |
| U-306 | 静态文本 / 静态文本 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-709-TEXT-PARENT](#q-s00-cand-709-text-parent)、[Q-S00-CAND-710-TEXT-ADDRESS](#q-s00-cand-710-text-address)、[Q-S00-CAND-711-TEXT-RUNTIME](#q-s00-cand-711-text-runtime)、[Q-S00-CAND-712-TEXT-RUNTIME](#q-s00-cand-712-text-runtime) |
| U-307 | 静态文本 / 静态文本 | 复合父项 / 已有历史补证，整体未关闭 | [Q-S00-CAND-713-TEXT-PARENT](#q-s00-cand-713-text-parent)、[Q-S00-CAND-714-TEXT-SOURCE](#q-s00-cand-714-text-source)、[Q-S00-CAND-715-TEXT-COMPILE](#q-s00-cand-715-text-compile)、[Q-S00-CAND-716-TEXT-RUNTIME](#q-s00-cand-716-text-runtime) |
| U-308 | 静态文本 / 静态文本 | 说明文字 / 说明排除，保留原行 | 说明排除；不建Q |

## 逐问题记录

本节的刺激均是下一探针设计，实际观察字段全部为空。已有报告的范围声明与其引用来源保留，未在本轮重新执行。

<a id="q-s00-cand-001-ascii-parent"></a>

### Q-S00-CAND-001-ASCII-PARENT

- 问题：ASCII显示候选U-001的分层问题？
- 来源/归属：CAND-20261110/U-001；ASCII显示；C12
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：ASCII显示候选U-001的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-002-ASCII-UI、Q-S00-CAND-003-ASCII-UI

<a id="q-s00-cand-002-ascii-ui"></a>

### Q-S00-CAND-002-ASCII-UI

- 问题：宏组真实enabled与勾选后使用宏列表的联动？
- 来源/归属：CAND-20261110/U-001；ASCII显示；C12
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：宏组真实enabled与勾选后使用宏列表的联动。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-001-ASCII-PARENT

<a id="q-s00-cand-003-ascii-ui"></a>

### Q-S00-CAND-003-ASCII-UI

- 问题：写宏窗口的模板、编辑、保存及回到宏选择流程？
- 来源/归属：CAND-20261110/U-001；ASCII显示；C12
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：写宏窗口的模板、编辑、保存及回到宏选择流程。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-001-ASCII-PARENT

<a id="q-s00-cand-004-ascii-render"></a>

### Q-S00-CAND-004-ASCII-RENDER

- 问题：密码开关是否在预览与画布将实际字符替换为掩码？
- 来源/归属：CAND-20261110/U-002；ASCII显示；C12
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：密码开关是否在预览与画布将实际字符替换为掩码。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-005-ascii-parent"></a>

### Q-S00-CAND-005-ASCII-PARENT

- 问题：ASCII显示候选U-003的分层问题？
- 来源/归属：CAND-20261110/U-003；ASCII显示；C12
- 层/状态：E/R；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：ASCII显示候选U-003的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-006-ASCII-RENDER、Q-S00-CAND-007-ASCII-RUNTIME、Q-S00-CAND-008-ASCII-RENDER、Q-S00-CAND-009-ASCII-RUNTIME

<a id="q-s00-cand-006-ascii-render"></a>

### Q-S00-CAND-006-ASCII-RENDER

- 问题：自动换行在非占位长文本中的预览及画布效果？
- 来源/归属：CAND-20261110/U-003；ASCII显示；C12
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：自动换行在非占位长文本中的预览及画布效果。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-005-ASCII-PARENT

<a id="q-s00-cand-007-ascii-runtime"></a>

### Q-S00-CAND-007-ASCII-RUNTIME

- 问题：自动换行在屏端的折行规则？
- 来源/归属：CAND-20261110/U-003；ASCII显示；C12
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：自动换行在屏端的折行规则。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-005-ASCII-PARENT

<a id="q-s00-cand-008-ascii-render"></a>

### Q-S00-CAND-008-ASCII-RENDER

- 问题：高低字节互换在预览及画布中的字符变换？
- 来源/归属：CAND-20261110/U-003；ASCII显示；C12
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：高低字节互换在预览及画布中的字符变换。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-005-ASCII-PARENT

<a id="q-s00-cand-009-ascii-runtime"></a>

### Q-S00-CAND-009-ASCII-RUNTIME

- 问题：高低字节互换的屏端字节与字符顺序？
- 来源/归属：CAND-20261110/U-003；ASCII显示；C12
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：高低字节互换的屏端字节与字符顺序。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-005-ASCII-PARENT

<a id="q-s00-cand-010-ascii-parent"></a>

### Q-S00-CAND-010-ASCII-PARENT

- 问题：ASCII显示候选U-004的分层问题？
- 来源/归属：CAND-20261110/U-004；ASCII显示；C12
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：ASCII显示候选U-004的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-011-ASCII-UI、Q-S00-CAND-012-ASCII-BOUND

<a id="q-s00-cand-011-ascii-ui"></a>

### Q-S00-CAND-011-ASCII-UI

- 问题：修改输入选项控制哪些字段及输入路径？
- 来源/归属：CAND-20261110/U-004；ASCII显示；C12
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：修改输入选项控制哪些字段及输入路径。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-010-ASCII-PARENT

<a id="q-s00-cand-012-ascii-bound"></a>

### Q-S00-CAND-012-ASCII-BOUND

- 问题：自定义键盘为空错误的准确触发条件与确定时机？
- 来源/归属：CAND-20261110/U-004；ASCII显示；C12
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：自定义键盘为空错误的准确触发条件与确定时机。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-010-ASCII-PARENT

<a id="q-s00-cand-013-ascii-parent"></a>

### Q-S00-CAND-013-ASCII-PARENT

- 问题：ASCII显示候选U-005的分层问题？
- 来源/归属：CAND-20261110/U-005；ASCII显示；C12
- 层/状态：D/E/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：ASCII显示候选U-005的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-014-ASCII-RUNTIME、Q-S00-CAND-015-ASCII-VISIBILITY

<a id="q-s00-cand-014-ascii-runtime"></a>

### Q-S00-CAND-014-ASCII-RUNTIME

- 问题：GB2312键盘的设备端键位和可输入字符集？
- 来源/归属：CAND-20261110/U-005；ASCII显示；C12
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：GB2312键盘的设备端键位和可输入字符集。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-013-ASCII-PARENT

<a id="q-s00-cand-015-ascii-visibility"></a>

### Q-S00-CAND-015-ASCII-VISIBILITY

- 问题：V5.1当前编码下拉之外的UNICIDE、BIG5、GBK是否有可达入口？
- 来源/归属：CAND-20261110/U-005；ASCII显示；C12
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定当前AMT和V5.1；B/H隐藏字段只作线索，另列型号或版本扩展范围。
- 刺激：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 对象：V5.1当前编码下拉之外的UNICIDE、BIG5、GBK是否有可达入口。
- 预期观察：只对搜索边界报告可达、隐藏或未找到；不因字符串存在增加字段，也不推断所有型号不存在。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-013-ASCII-PARENT

<a id="q-s00-cand-016-ascii-render"></a>

### Q-S00-CAND-016-ASCII-RENDER

- 问题：显示字符数变化是否实时改变预览占位文本长度？
- 来源/归属：CAND-20261110/U-006；ASCII显示；C12
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：显示字符数变化是否实时改变预览占位文本长度。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-017-ascii-parent"></a>

### Q-S00-CAND-017-ASCII-PARENT

- 问题：ASCII显示候选U-007的分层问题？
- 来源/归属：CAND-20261110/U-007；ASCII显示；C12
- 层/状态：P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：ASCII显示候选U-007的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-018-ASCII-GALLERY、Q-S00-CAND-019-ASCII-GALLERY

<a id="q-s00-cand-018-ascii-gallery"></a>

### Q-S00-CAND-018-ASCII-GALLERY

- 问题：工程图库的可见内容及选择后外形回填？
- 来源/归属：CAND-20261110/U-007；ASCII显示；C12
- 层/状态：U/P/R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立独立工程图库，加入有唯一内容的两张图并设置一个对象引用，保留资源及工程SHA。
- 刺激：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 对象：工程图库的可见内容及选择后外形回填。
- 预期观察：确定列表、路径、引用、删除恢复及落盘时机；共用图库实现不证明每个控件调用点相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-017-ASCII-PARENT

<a id="q-s00-cand-019-ascii-gallery"></a>

### Q-S00-CAND-019-ASCII-GALLERY

- 问题：删除被引用工程图片后的回退与保存重开结果？
- 来源/归属：CAND-20261110/U-007；ASCII显示；C12
- 层/状态：U/P/R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立独立工程图库，加入有唯一内容的两张图并设置一个对象引用，保留资源及工程SHA。
- 刺激：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 对象：删除被引用工程图片后的回退与保存重开结果。
- 预期观察：确定列表、路径、引用、删除恢复及落盘时机；共用图库实现不证明每个控件调用点相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-017-ASCII-PARENT

<a id="q-s00-cand-020-ascii-parent"></a>

### Q-S00-CAND-020-ASCII-PARENT

- 问题：ASCII显示候选U-008的分层问题？
- 来源/归属：CAND-20261110/U-008；ASCII显示；C12
- 层/状态：R；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：ASCII显示候选U-008的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-021-ASCII-RENDER、Q-S00-CAND-022-ASCII-RENDER、Q-S00-CAND-023-ASCII-RENDER

<a id="q-s00-cand-021-ascii-render"></a>

### Q-S00-CAND-021-ASCII-RENDER

- 问题：透明度数值对应的Alpha合成结果？
- 来源/归属：CAND-20261110/U-008；ASCII显示；C12
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：透明度数值对应的Alpha合成结果。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-020-ASCII-PARENT

<a id="q-s00-cand-022-ascii-render"></a>

### Q-S00-CAND-022-ASCII-RENDER

- 问题：使用透明效果开关对应的背景处理？
- 来源/归属：CAND-20261110/U-008；ASCII显示；C12
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：使用透明效果开关对应的背景处理。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-020-ASCII-PARENT

<a id="q-s00-cand-023-ascii-render"></a>

### Q-S00-CAND-023-ASCII-RENDER

- 问题：终端3D标志在编辑器像素及设备像素中的作用？
- 来源/归属：CAND-20261110/U-008；ASCII显示；C12
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：终端3D标志在编辑器像素及设备像素中的作用。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-020-ASCII-PARENT

<a id="q-s00-cand-024-ascii-ui"></a>

### Q-S00-CAND-024-ASCII-UI

- 问题：ASCII对象通知组与位字子项的真实enabled及解除禁用条件？
- 来源/归属：CAND-20261110/U-009；ASCII显示；C12
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：ASCII对象通知组与位字子项的真实enabled及解除禁用条件。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-025-ascii-persist"></a>

### Q-S00-CAND-025-ASCII-PERSIST

- 问题：ASCII属性逐字段修改、确定、保存、重开后的回填？
- 来源/归属：CAND-20261110/U-010；ASCII显示；C12
- 层/状态：U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留可辨识原值和工程副本，固定当前语言、状态、类型及对象ID。
- 刺激：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 对象：ASCII属性逐字段修改、确定、保存、重开后的回填。
- 预期观察：取得提交、取消和休眠草稿的保存寿命；区分ksp草稿与原厂sm实际保存字段。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-026-ascii-parent"></a>

### Q-S00-CAND-026-ASCII-PARENT

- 问题：ASCII显示候选U-011的分层问题？
- 来源/归属：CAND-20261110/U-011；ASCII显示；C12
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：ASCII显示候选U-011的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-027-ASCII-SOURCE、Q-S00-CAND-028-ASCII-COMPILE、Q-S00-CAND-029-ASCII-RUNTIME

<a id="q-s00-cand-027-ascii-source"></a>

### Q-S00-CAND-027-ASCII-SOURCE

- 问题：ASCII对象的sm字段、字符串与地址保存布局？
- 来源/归属：CAND-20261110/U-011；ASCII显示；C12
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：ASCII对象的sm字段、字符串与地址保存布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-026-ASCII-PARENT

<a id="q-s00-cand-028-ascii-compile"></a>

### Q-S00-CAND-028-ASCII-COMPILE

- 问题：ASCII对象下载控制及字符数据编码？
- 来源/归属：CAND-20261110/U-011；ASCII显示；C12
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：ASCII对象下载控制及字符数据编码。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-026-ASCII-PARENT

<a id="q-s00-cand-029-ascii-runtime"></a>

### Q-S00-CAND-029-ASCII-RUNTIME

- 问题：固件如何按所编译ASCII记录读取和显示字符串？
- 来源/归属：CAND-20261110/U-011；ASCII显示；C12
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件如何按所编译ASCII记录读取和显示字符串。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-026-ASCII-PARENT

<a id="q-s00-cand-030-ascii-help"></a>

### Q-S00-CAND-030-ASCII-HELP

- 问题：ASCII属性帮助按钮是否打开对应CHM成员？
- 来源/归属：CAND-20261110/U-012；ASCII显示；C12
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：确认原CHM路径/哈希及成员存在，记录完整安装依赖和私有副本环境差异。
- 刺激：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 对象：ASCII属性帮助按钮是否打开对应CHM成员。
- 预期观察：确认目标成员或有界失败；CHM成员存在不能替代按钮直达事实。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-031-ascii-ui"></a>

### Q-S00-CAND-031-ASCII-UI

- 问题：ASCII对象右键菜单实际项集及可用状态？
- 来源/归属：CAND-20261110/U-013；ASCII显示；C12
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：ASCII对象右键菜单实际项集及可用状态。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ASCII；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-032-gif-help"></a>

### Q-S00-CAND-032-GIF-HELP

- 问题：完整安装环境GIF属性帮助按钮调用的页面及无反应原因？
- 来源/归属：CAND-20261110/U-014；GIF显示；C10
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：确认原CHM路径/哈希及成员存在，记录完整安装依赖和私有副本环境差异。
- 刺激：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 对象：完整安装环境GIF属性帮助按钮调用的页面及无反应原因。
- 预期观察：确认目标成员或有界失败；CHM成员存在不能替代按钮直达事实。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-033-gif-parent"></a>

### Q-S00-CAND-033-GIF-PARENT

- 问题：GIF显示候选U-015的分层问题？
- 来源/归属：CAND-20261110/U-015；GIF显示；C10
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：GIF显示候选U-015的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-034-GIF-RUNTIME、Q-S00-CAND-035-GIF-RUNTIME

<a id="q-s00-cand-034-gif-runtime"></a>

### Q-S00-CAND-034-GIF-RUNTIME

- 问题：GIF控制位为有效状态时是播放还是静止？
- 来源/归属：CAND-20261110/U-015；GIF显示；C10
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：GIF控制位为有效状态时是播放还是静止。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-033-GIF-PARENT

<a id="q-s00-cand-035-gif-runtime"></a>

### Q-S00-CAND-035-GIF-RUNTIME

- 问题：GIF控制位切到非有效状态和边界瞬间的播放位置？
- 来源/归属：CAND-20261110/U-015；GIF显示；C10
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：GIF控制位切到非有效状态和边界瞬间的播放位置。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-033-GIF-PARENT

<a id="q-s00-cand-036-gif-parent"></a>

### Q-S00-CAND-036-GIF-PARENT

- 问题：GIF显示候选U-016的分层问题？
- 来源/归属：CAND-20261110/U-016；GIF显示；C10
- 层/状态：D/E/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：GIF显示候选U-016的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-037-GIF-BOUND、Q-S00-CAND-038-GIF-BOUND、Q-S00-CAND-039-GIF-RUNTIME

<a id="q-s00-cand-037-gif-bound"></a>

### Q-S00-CAND-037-GIF-BOUND

- 问题：大于1MiB的GIF导入拒绝路径及提示？
- 来源/归属：CAND-20261110/U-016；GIF显示；C10
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：大于1MiB的GIF导入拒绝路径及提示。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-036-GIF-PARENT

<a id="q-s00-cand-038-gif-bound"></a>

### Q-S00-CAND-038-GIF-BOUND

- 问题：非GIF和损坏GIF的拒绝路径及提示？
- 来源/归属：CAND-20261110/U-016；GIF显示；C10
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：非GIF和损坏GIF的拒绝路径及提示。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-036-GIF-PARENT

<a id="q-s00-cand-039-gif-runtime"></a>

### Q-S00-CAND-039-GIF-RUNTIME

- 问题：GIF文件帧延时及循环计数在设备端是否原样执行？
- 来源/归属：CAND-20261110/U-016；GIF显示；C10
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：GIF文件帧延时及循环计数在设备端是否原样执行。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-036-GIF-PARENT

<a id="q-s00-cand-040-gif-ui"></a>

### Q-S00-CAND-040-GIF-UI

- 问题：GIF字控制比较符等于与不等于时误差行显隐？
- 来源/归属：CAND-20261110/U-017；GIF显示；C10
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：GIF字控制比较符等于与不等于时误差行显隐。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-041-gif-parent"></a>

### Q-S00-CAND-041-GIF-PARENT

- 问题：GIF显示候选U-018的分层问题？
- 来源/归属：CAND-20261110/U-018；GIF显示；C10
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：GIF显示候选U-018的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-042-GIF-PERMISSION、Q-S00-CAND-043-GIF-THRESHOLD

<a id="q-s00-cand-042-gif-permission"></a>

### Q-S00-CAND-042-GIF-PERMISSION

- 问题：GIF显现多条件满足与冲突时的组合逻辑？
- 来源/归属：CAND-20261110/U-018；GIF显示；C10
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：选择该控件的显现或触控列，单独建立位、等级、字条件，且对象无重叠遮挡。
- 刺激：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 对象：GIF显现多条件满足与冲突时的组合逻辑。
- 预期观察：取得显现与触控各自的真值表、组合方式及优先级；不由共用容器推断所有控件相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-041-GIF-PARENT

<a id="q-s00-cand-043-gif-threshold"></a>

### Q-S00-CAND-043-GIF-THRESHOLD

- 问题：GIF最低等级等于阈值时是否显示？
- 来源/归属：CAND-20261110/U-018；GIF显示；C10
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：固定该控件的最低等级阈值，位和字条件关闭或恒定满足。
- 刺激：分别以阈值-1、阈值、阈值+1等级观察显现或触控；显现和触控列分开记录。 对象：GIF最低等级等于阈值时是否显示。
- 预期观察：取得最低等级的等值边界与实际权限结果，不由字段名称预设闭开关系。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：分别以阈值-1、阈值、阈值+1等级观察显现或触控；显现和触控列分开记录。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-041-GIF-PARENT

<a id="q-s00-cand-044-gif-parent"></a>

### Q-S00-CAND-044-GIF-PARENT

- 问题：GIF显示候选U-019的分层问题？
- 来源/归属：CAND-20261110/U-019；GIF显示；C10
- 层/状态：P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：GIF显示候选U-019的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-045-GIF-GALLERY、Q-S00-CAND-046-GIF-GALLERY、Q-S00-CAND-047-GIF-PERSIST

<a id="q-s00-cand-045-gif-gallery"></a>

### Q-S00-CAND-045-GIF-GALLERY

- 问题：GIF工程图库批量添加图片的列表和引用结果？
- 来源/归属：CAND-20261110/U-019；GIF显示；C10
- 层/状态：U/P/R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立独立工程图库，加入有唯一内容的两张图并设置一个对象引用，保留资源及工程SHA。
- 刺激：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 对象：GIF工程图库批量添加图片的列表和引用结果。
- 预期观察：确定列表、路径、引用、删除恢复及落盘时机；共用图库实现不证明每个控件调用点相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-044-GIF-PARENT

<a id="q-s00-cand-046-gif-gallery"></a>

### Q-S00-CAND-046-GIF-GALLERY

- 问题：GIF工程图库删除及导出流程？
- 来源/归属：CAND-20261110/U-019；GIF显示；C10
- 层/状态：U/P/R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立独立工程图库，加入有唯一内容的两张图并设置一个对象引用，保留资源及工程SHA。
- 刺激：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 对象：GIF工程图库删除及导出流程。
- 预期观察：确定列表、路径、引用、删除恢复及落盘时机；共用图库实现不证明每个控件调用点相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-044-GIF-PARENT

<a id="q-s00-cand-047-gif-persist"></a>

### Q-S00-CAND-047-GIF-PERSIST

- 问题：ProPicLib.klib在添加、确认、保存和关闭时的落盘时机？
- 来源/归属：CAND-20261110/U-019；GIF显示；C10
- 层/状态：U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留可辨识原值和工程副本，固定当前语言、状态、类型及对象ID。
- 刺激：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 对象：ProPicLib.klib在添加、确认、保存和关闭时的落盘时机。
- 预期观察：取得提交、取消和休眠草稿的保存寿命；区分ksp草稿与原厂sm实际保存字段。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-044-GIF-PARENT

<a id="q-s00-cand-048-gif-parent"></a>

### Q-S00-CAND-048-GIF-PARENT

- 问题：GIF显示候选U-020的分层问题？
- 来源/归属：CAND-20261110/U-020；GIF显示；C10
- 层/状态：E/R；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：GIF显示候选U-020的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-049-GIF-RENDER、Q-S00-CAND-050-GIF-RUNTIME

<a id="q-s00-cand-049-gif-render"></a>

### Q-S00-CAND-049-GIF-RENDER

- 问题：GIF预览的帧延时及循环与文件元数据的关系？
- 来源/归属：CAND-20261110/U-020；GIF显示；C10
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：GIF预览的帧延时及循环与文件元数据的关系。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-048-GIF-PARENT

<a id="q-s00-cand-050-gif-runtime"></a>

### Q-S00-CAND-050-GIF-RUNTIME

- 问题：同一GIF预览与屏端播放进度及循环的一致性？
- 来源/归属：CAND-20261110/U-020；GIF显示；C10
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：同一GIF预览与屏端播放进度及循环的一致性。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-048-GIF-PARENT

<a id="q-s00-cand-051-gif-parent"></a>

### Q-S00-CAND-051-GIF-PARENT

- 问题：GIF显示候选U-021的分层问题？
- 来源/归属：CAND-20261110/U-021；GIF显示；C10
- 层/状态：C/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：GIF显示候选U-021的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-052-GIF-SOURCE、Q-S00-CAND-053-GIF-COMPILE

<a id="q-s00-cand-052-gif-source"></a>

### Q-S00-CAND-052-GIF-SOURCE

- 问题：GIF源工程采用内嵌数据还是资源引用？
- 来源/归属：CAND-20261110/U-021；GIF显示；C10
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：GIF源工程采用内嵌数据还是资源引用。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-051-GIF-PARENT

<a id="q-s00-cand-053-gif-compile"></a>

### Q-S00-CAND-053-GIF-COMPILE

- 问题：GIF下载数据的帧、调色板及索引布局？
- 来源/归属：CAND-20261110/U-021；GIF显示；C10
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：GIF下载数据的帧、调色板及索引布局。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-051-GIF-PARENT

<a id="q-s00-cand-054-gif-scope"></a>

### Q-S00-CAND-054-GIF-SCOPE

- 问题：带10.1及11.1痕迹的GIF帮助图片对V5.1的适用边界？
- 来源/归属：CAND-20261110/U-022；GIF显示；C10
- 层/状态：D；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：并列原始候选、原帮助及当前V5.1直接证据，保留版本/型号和文档章节。
- 刺激：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 对象：带10.1及11.1痕迹的GIF帮助图片对V5.1的适用边界。
- 预期观察：形成可追溯的来源与采用边界；说明句或旧图不自动成为待实现功能或运行算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GIF；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-055-xy-parent"></a>

### Q-S00-CAND-055-XY-PARENT

- 问题：X-Y曲线候选U-023的分层问题？
- 来源/归属：CAND-20261110/U-023；X-Y曲线；C25
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：X-Y曲线候选U-023的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-056-XY-BOUND、Q-S00-CAND-057-XY-BOUND、Q-S00-CAND-058-XY-BOUND

<a id="q-s00-cand-056-xy-bound"></a>

### Q-S00-CAND-056-XY-BOUND

- 问题：X-Y通道总数上下限与非法输入处理？
- 来源/归属：CAND-20261110/U-023；X-Y曲线；C25
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：X-Y通道总数上下限与非法输入处理。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-055-XY-PARENT

<a id="q-s00-cand-057-xy-bound"></a>

### Q-S00-CAND-057-XY-BOUND

- 问题：X-Y采样点数上下限与非法输入处理？
- 来源/归属：CAND-20261110/U-023；X-Y曲线；C25
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：X-Y采样点数上下限与非法输入处理。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-055-XY-PARENT

<a id="q-s00-cand-058-xy-bound"></a>

### Q-S00-CAND-058-XY-BOUND

- 问题：X-Y采样时间上下限及下调到1的钳制？
- 来源/归属：CAND-20261110/U-023；X-Y曲线；C25
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：X-Y采样时间上下限及下调到1的钳制。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-055-XY-PARENT

<a id="q-s00-cand-059-xy-persist"></a>

### Q-S00-CAND-059-XY-PERSIST

- 问题：X-Y各通道的字段草稿是否独立并能保存往返？
- 来源/归属：CAND-20261110/U-024；X-Y曲线；C25
- 层/状态：U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留可辨识原值和工程副本，固定当前语言、状态、类型及对象ID。
- 刺激：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 对象：X-Y各通道的字段草稿是否独立并能保存往返。
- 预期观察：取得提交、取消和休眠草稿的保存寿命；区分ksp草稿与原厂sm实际保存字段。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-060-xy-bound"></a>

### Q-S00-CAND-060-XY-BOUND

- 问题：X-Y显示设置最小值大于最大值时的弹窗文案和触发时机？
- 来源/归属：CAND-20261110/U-025；X-Y曲线；C25
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：X-Y显示设置最小值大于最大值时的弹窗文案和触发时机。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-061-xy-parent"></a>

### Q-S00-CAND-061-XY-PARENT

- 问题：X-Y曲线候选U-026的分层问题？
- 来源/归属：CAND-20261110/U-026；X-Y曲线；C25
- 层/状态：D/P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：X-Y曲线候选U-026的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-062-XY-RENDER、Q-S00-CAND-063-XY-BOUND

<a id="q-s00-cand-062-xy-render"></a>

### Q-S00-CAND-062-XY-RENDER

- 问题：X-Y线型索引0至4对应的点划图案？
- 来源/归属：CAND-20261110/U-026；X-Y曲线；C25
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：X-Y线型索引0至4对应的点划图案。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-061-XY-PARENT

<a id="q-s00-cand-063-xy-bound"></a>

### Q-S00-CAND-063-XY-BOUND

- 问题：X-Y线宽的实际可选范围？
- 来源/归属：CAND-20261110/U-026；X-Y曲线；C25
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：X-Y线宽的实际可选范围。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-061-XY-PARENT

<a id="q-s00-cand-064-xy-options"></a>

### Q-S00-CAND-064-XY-OPTIONS

- 问题：X-Y的Y轴字体大小完整项集是否为12、16、24、48？
- 来源/归属：CAND-20261110/U-027；X-Y曲线；C25
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：X-Y的Y轴字体大小完整项集是否为12、16、24、48。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-065-xy-ui"></a>

### Q-S00-CAND-065-XY-UI

- 问题：X-Y显示设置最小值及最大值键入后的提交方式和回填？
- 来源/归属：CAND-20261110/U-028；X-Y曲线；C25
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：X-Y显示设置最小值及最大值键入后的提交方式和回填。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-066-xy-parent"></a>

### Q-S00-CAND-066-XY-PARENT

- 问题：X-Y曲线候选U-029的分层问题？
- 来源/归属：CAND-20261110/U-029；X-Y曲线；C25
- 层/状态：C/D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：X-Y曲线候选U-029的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-067-XY-UI、Q-S00-CAND-068-XY-COMPILE

<a id="q-s00-cand-067-xy-ui"></a>

### Q-S00-CAND-067-XY-UI

- 问题：X-Y切到32位时地址文本和字段启用状态是否变化？
- 来源/归属：CAND-20261110/U-029；X-Y曲线；C25
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：X-Y切到32位时地址文本和字段启用状态是否变化。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-066-XY-PARENT

<a id="q-s00-cand-068-xy-compile"></a>

### Q-S00-CAND-068-XY-COMPILE

- 问题：X-Y的32位间隔读取对应地址跨度与编译布局？
- 来源/归属：CAND-20261110/U-029；X-Y曲线；C25
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：X-Y的32位间隔读取对应地址跨度与编译布局。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-066-XY-PARENT

<a id="q-s00-cand-069-xy-parent"></a>

### Q-S00-CAND-069-XY-PARENT

- 问题：X-Y曲线候选U-030的分层问题？
- 来源/归属：CAND-20261110/U-030；X-Y曲线；C25
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：X-Y曲线候选U-030的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-070-XY-BOUND、Q-S00-CAND-071-XY-BOUND

<a id="q-s00-cand-070-xy-bound"></a>

### Q-S00-CAND-070-XY-BOUND

- 问题：X-Y的BCD16输入如何拒绝非十进制有效数字？
- 来源/归属：CAND-20261110/U-030；X-Y曲线；C25
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：X-Y的BCD16输入如何拒绝非十进制有效数字。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-069-XY-PARENT

<a id="q-s00-cand-071-xy-bound"></a>

### Q-S00-CAND-071-XY-BOUND

- 问题：X-Y的BCD32输入长度与越界处理？
- 来源/归属：CAND-20261110/U-030；X-Y曲线；C25
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：X-Y的BCD32输入长度与越界处理。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-069-XY-PARENT

<a id="q-s00-cand-072-xy-help"></a>

### Q-S00-CAND-072-XY-HELP

- 问题：X-Y帮助及问号按钮的实际目标页面？
- 来源/归属：CAND-20261110/U-031；X-Y曲线；C25
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：确认原CHM路径/哈希及成员存在，记录完整安装依赖和私有副本环境差异。
- 刺激：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 对象：X-Y帮助及问号按钮的实际目标页面。
- 预期观察：确认目标成员或有界失败；CHM成员存在不能替代按钮直达事实。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-073-xy-visibility"></a>

### Q-S00-CAND-073-XY-VISIBILITY

- 问题：X-Y方向、画点标记、连接线字段在当前V5.1是否可达？
- 来源/归属：CAND-20261110/U-032；X-Y曲线；C25
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定当前AMT和V5.1；B/H隐藏字段只作线索，另列型号或版本扩展范围。
- 刺激：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 对象：X-Y方向、画点标记、连接线字段在当前V5.1是否可达。
- 预期观察：只对搜索边界报告可达、隐藏或未找到；不因字符串存在增加字段，也不推断所有型号不存在。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-074-xy-parent"></a>

### Q-S00-CAND-074-XY-PARENT

- 问题：X-Y曲线候选U-033的分层问题？
- 来源/归属：CAND-20261110/U-033；X-Y曲线；C25
- 层/状态：D/E/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：X-Y曲线候选U-033的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-075-XY-UI、Q-S00-CAND-076-XY-RUNTIME

<a id="q-s00-cand-075-xy-ui"></a>

### Q-S00-CAND-075-XY-UI

- 问题：X-Y运行采样是否依赖工程树数据采集配置？
- 来源/归属：CAND-20261110/U-033；X-Y曲线；C25
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：X-Y运行采样是否依赖工程树数据采集配置。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-074-XY-PARENT

<a id="q-s00-cand-076-xy-runtime"></a>

### Q-S00-CAND-076-XY-RUNTIME

- 问题：X-Y触发、清除和采样的设备端时序？
- 来源/归属：CAND-20261110/U-033；X-Y曲线；C25
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：X-Y触发、清除和采样的设备端时序。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-074-XY-PARENT

<a id="q-s00-cand-077-xy-parent"></a>

### Q-S00-CAND-077-XY-PARENT

- 问题：X-Y曲线候选U-034的分层问题？
- 来源/归属：CAND-20261110/U-034；X-Y曲线；C25
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：X-Y曲线候选U-034的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-078-XY-SOURCE、Q-S00-CAND-079-XY-COMPILE、Q-S00-CAND-080-XY-RUNTIME

<a id="q-s00-cand-078-xy-source"></a>

### Q-S00-CAND-078-XY-SOURCE

- 问题：X-Y源对象与数据采集引用的保存布局？
- 来源/归属：CAND-20261110/U-034；X-Y曲线；C25
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：X-Y源对象与数据采集引用的保存布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-077-XY-PARENT

<a id="q-s00-cand-079-xy-compile"></a>

### Q-S00-CAND-079-XY-COMPILE

- 问题：Curve.dat与Collect.dat的X-Y字段布局？
- 来源/归属：CAND-20261110/U-034；X-Y曲线；C25
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：Curve.dat与Collect.dat的X-Y字段布局。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-077-XY-PARENT

<a id="q-s00-cand-080-xy-runtime"></a>

### Q-S00-CAND-080-XY-RUNTIME

- 问题：固件按X-Y曲线记录取样及绘制的消费规则？
- 来源/归属：CAND-20261110/U-034；X-Y曲线；C25
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件按X-Y曲线记录取样及绘制的消费规则。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-XY；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-077-XY-PARENT

<a id="q-s00-cand-081-droplist-runtime"></a>

### Q-S00-CAND-081-DROPLIST-RUNTIME

- 问题：下拉框向上和向下方向在屏端的展开位置？
- 来源/归属：CAND-20261110/U-035；下拉框；C17
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：下拉框向上和向下方向在屏端的展开位置。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DROPLIST；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-082-droplist-bound"></a>

### Q-S00-CAND-082-DROPLIST-BOUND

- 问题：下拉框字号上下限及键入越界处理？
- 来源/归属：CAND-20261110/U-036；下拉框；C17
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：下拉框字号上下限及键入越界处理。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DROPLIST；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-083-droplist-address"></a>

### Q-S00-CAND-083-DROPLIST-ADDRESS

- 问题：下拉框地址按钮是否打开正确字地址拾取窗口并回填？
- 来源/归属：CAND-20261110/U-037；下拉框；C17
- 层/状态：D/U/C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定所用地址角色、位宽、连接/型号和通道；保留无连接及已配置工程两份。
- 刺激：从指定控件字段打开拾取器，枚举相应家族并选首末/宽地址边界，确认与取消后核对回填和跨度。 对象：下拉框地址按钮是否打开正确字地址拾取窗口并回填。
- 预期观察：取得此调用点的地址角色与可达族；别的控件同款窗口仅作线索，不能代替本入口。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DROPLIST；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：从指定控件字段打开拾取器，枚举相应家族并选首末/宽地址边界，确认与取消后核对回填和跨度。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-084-droplist-ui"></a>

### Q-S00-CAND-084-DROPLIST-UI

- 问题：下拉框颜色确认及取消是否分别提交与保留原色？
- 来源/归属：CAND-20261110/U-038；下拉框；C17
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：下拉框颜色确认及取消是否分别提交与保留原色。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DROPLIST；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-085-droplist-bound"></a>

### Q-S00-CAND-085-DROPLIST-BOUND

- 问题：下拉框数据单元格超过所选数值类型范围时的校验？
- 来源/归属：CAND-20261110/U-039；下拉框；C17
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：下拉框数据单元格超过所选数值类型范围时的校验。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DROPLIST；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-086-droplist-ui"></a>

### Q-S00-CAND-086-DROPLIST-UI

- 问题：下拉框通知组真实enabled、启用条件和子项展开？
- 来源/归属：CAND-20261110/U-040；下拉框；C17
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：下拉框通知组真实enabled、启用条件和子项展开。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DROPLIST；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-087-droplist-persist"></a>

### Q-S00-CAND-087-DROPLIST-PERSIST

- 问题：取消下拉框编辑后再次双击是否保留提交前字段？
- 来源/归属：CAND-20261110/U-041；下拉框；C17
- 层/状态：U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留可辨识原值和工程副本，固定当前语言、状态、类型及对象ID。
- 刺激：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 对象：取消下拉框编辑后再次双击是否保留提交前字段。
- 预期观察：取得提交、取消和休眠草稿的保存寿命；区分ksp草稿与原厂sm实际保存字段。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DROPLIST；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-088-droplist-parent"></a>

### Q-S00-CAND-088-DROPLIST-PARENT

- 问题：下拉框候选U-042的分层问题？
- 来源/归属：CAND-20261110/U-042；下拉框；C17
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：下拉框候选U-042的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DROPLIST；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-089-DROPLIST-RUNTIME、Q-S00-CAND-090-DROPLIST-RUNTIME

<a id="q-s00-cand-089-droplist-runtime"></a>

### Q-S00-CAND-089-DROPLIST-RUNTIME

- 问题：下拉框屏端弹出动画与触控区域？
- 来源/归属：CAND-20261110/U-042；下拉框；C17
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：下拉框屏端弹出动画与触控区域。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DROPLIST；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-088-DROPLIST-PARENT

<a id="q-s00-cand-090-droplist-runtime"></a>

### Q-S00-CAND-090-DROPLIST-RUNTIME

- 问题：下拉框选择项目后向地址写值的触发时机？
- 来源/归属：CAND-20261110/U-042；下拉框；C17
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：下拉框选择项目后向地址写值的触发时机。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DROPLIST；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-088-DROPLIST-PARENT

<a id="q-s00-cand-091-droplist-parent"></a>

### Q-S00-CAND-091-DROPLIST-PARENT

- 问题：下拉框候选U-043的分层问题？
- 来源/归属：CAND-20261110/U-043；下拉框；C17
- 层/状态：C/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：下拉框候选U-043的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DROPLIST；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-092-DROPLIST-SOURCE、Q-S00-CAND-093-DROPLIST-COMPILE

<a id="q-s00-cand-092-droplist-source"></a>

### Q-S00-CAND-092-DROPLIST-SOURCE

- 问题：下拉框项目表与数值的sm保存布局？
- 来源/归属：CAND-20261110/U-043；下拉框；C17
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：下拉框项目表与数值的sm保存布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DROPLIST；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-091-DROPLIST-PARENT

<a id="q-s00-cand-093-droplist-compile"></a>

### Q-S00-CAND-093-DROPLIST-COMPILE

- 问题：DropListText.dat及控制记录的编码？
- 来源/归属：CAND-20261110/U-043；下拉框；C17
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：DropListText.dat及控制记录的编码。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DROPLIST；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-091-DROPLIST-PARENT

<a id="q-s00-cand-094-qr-runtime"></a>

### Q-S00-CAND-094-QR-RUNTIME

- 问题：二维码写入监视地址发生在编译初始化还是运行期？
- 来源/归属：CAND-20261110/U-044；二维码；C08
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：二维码写入监视地址发生在编译初始化还是运行期。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-QR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-095-qr-compile"></a>

### Q-S00-CAND-095-QR-COMPILE

- 问题：二维码高低字节互换如何变换输入字节并影响点阵？
- 来源/归属：CAND-20261110/U-045；二维码；C08
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：二维码高低字节互换如何变换输入字节并影响点阵。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-QR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-096-qr-parent"></a>

### Q-S00-CAND-096-QR-PARENT

- 问题：二维码候选U-046的分层问题？
- 来源/归属：CAND-20261110/U-046；二维码；C08
- 层/状态：R；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：二维码候选U-046的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-QR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-097-QR-RENDER、Q-S00-CAND-098-QR-RENDER

<a id="q-s00-cand-097-qr-render"></a>

### Q-S00-CAND-097-QR-RENDER

- 问题：二维码的纠错等级、版本及留白如何从输出反推？
- 来源/归属：CAND-20261110/U-046；二维码；C08
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：二维码的纠错等级、版本及留白如何从输出反推。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-QR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-096-QR-PARENT

<a id="q-s00-cand-098-qr-render"></a>

### Q-S00-CAND-098-QR-RENDER

- 问题：二维码生成点阵能否被标准解码器按原文还原？
- 来源/归属：CAND-20261110/U-046；二维码；C08
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：二维码生成点阵能否被标准解码器按原文还原。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-QR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-096-QR-PARENT

<a id="q-s00-cand-099-qr-ui"></a>

### Q-S00-CAND-099-QR-UI

- 问题：二维码权限页等于与不等于时误差行是否可见？
- 来源/归属：CAND-20261110/U-047；二维码；C08
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：二维码权限页等于与不等于时误差行是否可见。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-QR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-100-qr-runtime"></a>

### Q-S00-CAND-100-QR-RUNTIME

- 问题：二维码GB2312字符串的地址来源及多字节读取规则？
- 来源/归属：CAND-20261110/U-048；二维码；C08
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：二维码GB2312字符串的地址来源及多字节读取规则。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-QR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-101-qr-parent"></a>

### Q-S00-CAND-101-QR-PARENT

- 问题：二维码候选U-049的分层问题？
- 来源/归属：CAND-20261110/U-049；二维码；C08
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：二维码候选U-049的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-QR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-102-QR-PERMISSION、Q-S00-CAND-103-QR-THRESHOLD

<a id="q-s00-cand-102-qr-permission"></a>

### Q-S00-CAND-102-QR-PERMISSION

- 问题：二维码显现多条件满足或冲突时的组合逻辑？
- 来源/归属：CAND-20261110/U-049；二维码；C08
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：选择该控件的显现或触控列，单独建立位、等级、字条件，且对象无重叠遮挡。
- 刺激：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 对象：二维码显现多条件满足或冲突时的组合逻辑。
- 预期观察：取得显现与触控各自的真值表、组合方式及优先级；不由共用容器推断所有控件相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-QR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-101-QR-PARENT

<a id="q-s00-cand-103-qr-threshold"></a>

### Q-S00-CAND-103-QR-THRESHOLD

- 问题：二维码最低等级恰等于阈值时的显现？
- 来源/归属：CAND-20261110/U-049；二维码；C08
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：固定该控件的最低等级阈值，位和字条件关闭或恒定满足。
- 刺激：分别以阈值-1、阈值、阈值+1等级观察显现或触控；显现和触控列分开记录。 对象：二维码最低等级恰等于阈值时的显现。
- 预期观察：取得最低等级的等值边界与实际权限结果，不由字段名称预设闭开关系。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-QR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：分别以阈值-1、阈值、阈值+1等级观察显现或触控；显现和触控列分开记录。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-101-QR-PARENT

<a id="q-s00-cand-104-qr-bound"></a>

### Q-S00-CAND-104-QR-BOUND

- 问题：二维码字符长度字段在98、99、100及越界输入时的限制？
- 来源/归属：CAND-20261110/U-050；二维码；C08
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：二维码字符长度字段在98、99、100及越界输入时的限制。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-QR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-105-qr-parent"></a>

### Q-S00-CAND-105-QR-PARENT

- 问题：二维码候选U-051的分层问题？
- 来源/归属：CAND-20261110/U-051；二维码；C08
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：二维码候选U-051的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-QR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-106-QR-SOURCE、Q-S00-CAND-107-QR-COMPILE、Q-S00-CAND-108-QR-RUNTIME

<a id="q-s00-cand-106-qr-source"></a>

### Q-S00-CAND-106-QR-SOURCE

- 问题：二维码对象的sm结构与字符串保存？
- 来源/归属：CAND-20261110/U-051；二维码；C08
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：二维码对象的sm结构与字符串保存。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-QR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-105-QR-PARENT

<a id="q-s00-cand-107-qr-compile"></a>

### Q-S00-CAND-107-QR-COMPILE

- 问题：二维码设备控制和点阵数据结构？
- 来源/归属：CAND-20261110/U-051；二维码；C08
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：二维码设备控制和点阵数据结构。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-QR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-105-QR-PARENT

<a id="q-s00-cand-108-qr-runtime"></a>

### Q-S00-CAND-108-QR-RUNTIME

- 问题：固件如何按二维码记录读取并显示字符串？
- 来源/归属：CAND-20261110/U-051；二维码；C08
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件如何按二维码记录读取并显示字符串。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-QR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-105-QR-PARENT

<a id="q-s00-cand-109-qr-visibility"></a>

### Q-S00-CAND-109-QR-VISIBILITY

- 问题：二维码不可见UNICIDE与BIG5字符串是否属于其他可达编码入口？
- 来源/归属：CAND-20261110/U-052；二维码；C08
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定当前AMT和V5.1；B/H隐藏字段只作线索，另列型号或版本扩展范围。
- 刺激：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 对象：二维码不可见UNICIDE与BIG5字符串是否属于其他可达编码入口。
- 预期观察：只对搜索边界报告可达、隐藏或未找到；不因字符串存在增加字段，也不推断所有型号不存在。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-QR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-110-bit-ui"></a>

### Q-S00-CAND-110-BIT-UI

- 问题：位按钮原生宏编辑器模板展开与保存回选流程？
- 来源/归属：CAND-20261110/U-053；位按钮；C02
- 层/状态：D/U；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：位按钮原生宏编辑器模板展开与保存回选流程。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；真实SDK编译与宏源保存已有P/C证据；原厂宏窗口全部交互不能仅凭Lite测试关闭。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-111-bit-ui"></a>

### Q-S00-CAND-111-BIT-UI

- 问题：位按钮宏组真实enabled及绑定宏列表的联动？
- 来源/归属：CAND-20261110/U-054；位按钮；C02
- 层/状态：D/U；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：位按钮宏组真实enabled及绑定宏列表的联动。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；位按钮报告已支持宏编辑绑定，旧合成点击失败不再当作宏不可用；精确原生启用链仍按引用复核。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-112-bit-ui"></a>

### Q-S00-CAND-112-BIT-UI

- 问题：位按钮通知组和互斥位字子项的真实可用状态与展开？
- 来源/归属：CAND-20261110/U-055；位按钮；C02
- 层/状态：D/U；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：位按钮通知组和互斥位字子项的真实可用状态与展开。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；位按钮报告已有互斥通知UI和设备记录补证；本控件旧禁用说法不能直接作为产品禁用条件。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-113-bit-parent"></a>

### Q-S00-CAND-113-BIT-PARENT

- 问题：位按钮候选U-056的分层问题？
- 来源/归属：CAND-20261110/U-056；位按钮；C02
- 层/状态：D/E/P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：位按钮候选U-056的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；原厂两语言样本、图序和闪烁无字帧已有P/C证据；闪烁频率和运行切换保持E未验。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-114-BIT-LANGUAGE、Q-S00-CAND-115-BIT-RENDER、Q-S00-CAND-116-BIT-RUNTIME

<a id="q-s00-cand-114-bit-language"></a>

### Q-S00-CAND-114-BIT-LANGUAGE

- 问题：位按钮多语言列表及各语言独立文本保存？
- 来源/归属：CAND-20261110/U-056；位按钮；C02
- 层/状态：D/U/P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立中文单语及中文/English/第三种语言工程，给各状态各语言写不同文本。
- 刺激：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 对象：位按钮多语言列表及各语言独立文本保存。
- 预期观察：确认语言列表来源、索引及草稿规则；运行切换是否正确仍需独立E观察。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；原厂两语言样本、图序和闪烁无字帧已有P/C证据；闪烁频率和运行切换保持E未验。
- 下一探针：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-113-BIT-PARENT

<a id="q-s00-cand-115-bit-render"></a>

### Q-S00-CAND-115-BIT-RENDER

- 问题：位按钮同文和闪烁的源图像语义？
- 来源/归属：CAND-20261110/U-056；位按钮；C02
- 层/状态：R；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：位按钮同文和闪烁的源图像语义。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；原厂两语言样本、图序和闪烁无字帧已有P/C证据；闪烁频率和运行切换保持E未验。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-113-BIT-PARENT

<a id="q-s00-cand-116-bit-runtime"></a>

### Q-S00-CAND-116-BIT-RUNTIME

- 问题：位按钮闪烁周期及运行语言切换？
- 来源/归属：CAND-20261110/U-056；位按钮；C02
- 层/状态：E；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：位按钮闪烁周期及运行语言切换。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；原厂两语言样本、图序和闪烁无字帧已有P/C证据；闪烁频率和运行切换保持E未验。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-113-BIT-PARENT

<a id="q-s00-cand-117-bit-parent"></a>

### Q-S00-CAND-117-BIT-PARENT

- 问题：位按钮候选U-057的分层问题？
- 来源/归属：CAND-20261110/U-057；位按钮；C02
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：位按钮候选U-057的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-118-BIT-OPTIONS、Q-S00-CAND-119-BIT-BOUND

<a id="q-s00-cand-118-bit-options"></a>

### Q-S00-CAND-118-BIT-OPTIONS

- 问题：位按钮对齐选项实际完整顺序？
- 来源/归属：CAND-20261110/U-057；位按钮；C02
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：位按钮对齐选项实际完整顺序。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-117-BIT-PARENT

<a id="q-s00-cand-119-bit-bound"></a>

### Q-S00-CAND-119-BIT-BOUND

- 问题：位按钮字体大小的原生输入范围？
- 来源/归属：CAND-20261110/U-057；位按钮；C02
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：位按钮字体大小的原生输入范围。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-117-BIT-PARENT

<a id="q-s00-cand-120-bit-parent"></a>

### Q-S00-CAND-120-BIT-PARENT

- 问题：位按钮候选U-058的分层问题？
- 来源/归属：CAND-20261110/U-058；位按钮；C02
- 层/状态：D/E/P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：位按钮候选U-058的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；位号及图像编码已有C证据；位值运行映射和完整合成公式保留开放。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-121-BIT-RUNTIME、Q-S00-CAND-122-BIT-BOUND、Q-S00-CAND-123-BIT-RENDER

<a id="q-s00-cand-121-bit-runtime"></a>

### Q-S00-CAND-121-BIT-RUNTIME

- 问题：位按钮字的位索引0至15与固件位值的映射？
- 来源/归属：CAND-20261110/U-058；位按钮；C02
- 层/状态：E；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：位按钮字的位索引0至15与固件位值的映射。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；位号及图像编码已有C证据；位值运行映射和完整合成公式保留开放。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-120-BIT-PARENT

<a id="q-s00-cand-122-bit-bound"></a>

### Q-S00-CAND-122-BIT-BOUND

- 问题：位按钮透明度原生输入上下限？
- 来源/归属：CAND-20261110/U-058；位按钮；C02
- 层/状态：D/U/P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：位按钮透明度原生输入上下限。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；位号及图像编码已有C证据；位值运行映射和完整合成公式保留开放。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-120-BIT-PARENT

<a id="q-s00-cand-123-bit-render"></a>

### Q-S00-CAND-123-BIT-RENDER

- 问题：位按钮双态图片的透明合成公式？
- 来源/归属：CAND-20261110/U-058；位按钮；C02
- 层/状态：R；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：位按钮双态图片的透明合成公式。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；位号及图像编码已有C证据；位值运行映射和完整合成公式保留开放。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-120-BIT-PARENT

<a id="q-s00-cand-124-bit-parent"></a>

### Q-S00-CAND-124-BIT-PARENT

- 问题：位按钮候选U-059的分层问题？
- 来源/归属：CAND-20261110/U-059；位按钮；C02
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：位按钮候选U-059的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-125-BIT-PERMISSION、Q-S00-CAND-126-BIT-THRESHOLD、Q-S00-CAND-127-BIT-THRESHOLD

<a id="q-s00-cand-125-bit-permission"></a>

### Q-S00-CAND-125-BIT-PERMISSION

- 问题：位按钮显现和触控多条件组合逻辑？
- 来源/归属：CAND-20261110/U-059；位按钮；C02
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：选择该控件的显现或触控列，单独建立位、等级、字条件，且对象无重叠遮挡。
- 刺激：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 对象：位按钮显现和触控多条件组合逻辑。
- 预期观察：取得显现与触控各自的真值表、组合方式及优先级；不由共用容器推断所有控件相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-124-BIT-PARENT

<a id="q-s00-cand-126-bit-threshold"></a>

### Q-S00-CAND-126-BIT-THRESHOLD

- 问题：位按钮等级等于阈值边界？
- 来源/归属：CAND-20261110/U-059；位按钮；C02
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：固定该控件的最低等级阈值，位和字条件关闭或恒定满足。
- 刺激：分别以阈值-1、阈值、阈值+1等级观察显现或触控；显现和触控列分开记录。 对象：位按钮等级等于阈值边界。
- 预期观察：取得最低等级的等值边界与实际权限结果，不由字段名称预设闭开关系。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：分别以阈值-1、阈值、阈值+1等级观察显现或触控；显现和触控列分开记录。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-124-BIT-PARENT

<a id="q-s00-cand-127-bit-threshold"></a>

### Q-S00-CAND-127-BIT-THRESHOLD

- 问题：位按钮等于与不等于比较的误差算式？
- 来源/归属：CAND-20261110/U-059；位按钮；C02
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：固定字比较值和误差，位和等级条件恒定满足；整数和浮点分别留样本。
- 刺激：测比较值±容差及邻界，分别测试等于、不等于及0误差，保持其他字段不变。 对象：位按钮等于与不等于比较的误差算式。
- 预期观察：取得容差闭开区间、绝对/相对误差及浮点边界，不提前假设具体公式。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：测比较值±容差及邻界，分别测试等于、不等于及0误差，保持其他字段不变。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-124-BIT-PARENT

<a id="q-s00-cand-128-bit-parent"></a>

### Q-S00-CAND-128-BIT-PARENT

- 问题：位按钮候选U-060的分层问题？
- 来源/归属：CAND-20261110/U-060；位按钮；C02
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：位按钮候选U-060的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-129-BIT-RUNTIME、Q-S00-CAND-130-BIT-RUNTIME、Q-S00-CAND-131-BIT-RUNTIME、Q-S00-CAND-132-BIT-RUNTIME

<a id="q-s00-cand-129-bit-runtime"></a>

### Q-S00-CAND-129-BIT-RUNTIME

- 问题：位置位操作按下与松开写值时序？
- 来源/归属：CAND-20261110/U-060；位按钮；C02
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：位置位操作按下与松开写值时序。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-128-BIT-PARENT

<a id="q-s00-cand-130-bit-runtime"></a>

### Q-S00-CAND-130-BIT-RUNTIME

- 问题：位复位操作按下与松开写值时序？
- 来源/归属：CAND-20261110/U-060；位按钮；C02
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：位复位操作按下与松开写值时序。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-128-BIT-PARENT

<a id="q-s00-cand-131-bit-runtime"></a>

### Q-S00-CAND-131-BIT-RUNTIME

- 问题：位点动操作按住、松开及失去触控时的写值？
- 来源/归属：CAND-20261110/U-060；位按钮；C02
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：位点动操作按住、松开及失去触控时的写值。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-128-BIT-PARENT

<a id="q-s00-cand-132-bit-runtime"></a>

### Q-S00-CAND-132-BIT-RUNTIME

- 问题：位交替操作一次触碰如何读取并翻转当前值？
- 来源/归属：CAND-20261110/U-060；位按钮；C02
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：位交替操作一次触碰如何读取并翻转当前值。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-128-BIT-PARENT

<a id="q-s00-cand-133-bit-visibility"></a>

### Q-S00-CAND-133-BIT-VISIBILITY

- 问题：位按钮自动上传或云端上传字段在当前型号是否可达？
- 来源/归属：CAND-20261110/U-061；位按钮；C02
- 层/状态：D/U；未闭合；atomic
- 适用范围：当前V5.1/AMT已观察的隐藏或缺席状态保留；新增其他机型/版本出现条件转能力矩阵扩展，不阻塞当前已核AMT切片。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定当前AMT和V5.1；B/H隐藏字段只作线索，另列型号或版本扩展范围。
- 刺激：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 对象：位按钮自动上传或云端上传字段在当前型号是否可达。
- 预期观察：只对搜索边界报告可达、隐藏或未找到；不因字符串存在增加字段，也不推断所有型号不存在。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-134-bit-parent"></a>

### Q-S00-CAND-134-BIT-PARENT

- 问题：位按钮候选U-062的分层问题？
- 来源/归属：CAND-20261110/U-062；位按钮；C02
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：位按钮候选U-062的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；受支持位按钮sm和640B设备记录已有P/C；固件时序与全部组合仍未验。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-135-BIT-SOURCE、Q-S00-CAND-136-BIT-COMPILE、Q-S00-CAND-137-BIT-RUNTIME

<a id="q-s00-cand-135-bit-source"></a>

### Q-S00-CAND-135-BIT-SOURCE

- 问题：位按钮受支持属性的原厂源字段布局？
- 来源/归属：CAND-20261110/U-062；位按钮；C02
- 层/状态：P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：位按钮受支持属性的原厂源字段布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；受支持位按钮sm和640B设备记录已有P/C；固件时序与全部组合仍未验。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-134-BIT-PARENT

<a id="q-s00-cand-136-bit-compile"></a>

### Q-S00-CAND-136-BIT-COMPILE

- 问题：位按钮设备记录、地址扫描及图帧布局？
- 来源/归属：CAND-20261110/U-062；位按钮；C02
- 层/状态：C；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：位按钮设备记录、地址扫描及图帧布局。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；受支持位按钮sm和640B设备记录已有P/C；固件时序与全部组合仍未验。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-134-BIT-PARENT

<a id="q-s00-cand-137-bit-runtime"></a>

### Q-S00-CAND-137-BIT-RUNTIME

- 问题：固件消费位按钮记录后的触控及监视兼容性？
- 来源/归属：CAND-20261110/U-062；位按钮；C02
- 层/状态：E；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件消费位按钮记录后的触控及监视兼容性。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-BIT、EV-S00-CAND-BIT；受支持位按钮sm和640B设备记录已有P/C；固件时序与全部组合仍未验。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-134-BIT-PARENT

<a id="q-s00-cand-138-scale-ui"></a>

### Q-S00-CAND-138-SCALE-UI

- 问题：刻度文本色及线色的颜色窗口与确认取消回写？
- 来源/归属：CAND-20261110/U-063；刻度；C07
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：刻度文本色及线色的颜色窗口与确认取消回写。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCALE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-139-scale-bound"></a>

### Q-S00-CAND-139-SCALE-BOUND

- 问题：刻度最小值大于最大值时真实等价点击的提示和拦截？
- 来源/归属：CAND-20261110/U-064；刻度；C07
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：刻度最小值大于最大值时真实等价点击的提示和拦截。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCALE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-140-scale-bound"></a>

### Q-S00-CAND-140-SCALE-BOUND

- 问题：刻度字号是否连续6至20及上下限输入限制？
- 来源/归属：CAND-20261110/U-065；刻度；C07
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：刻度字号是否连续6至20及上下限输入限制。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCALE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-141-scale-parent"></a>

### Q-S00-CAND-141-SCALE-PARENT

- 问题：刻度候选U-066的分层问题？
- 来源/归属：CAND-20261110/U-066；刻度；C07
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：刻度候选U-066的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCALE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-142-SCALE-BOUND、Q-S00-CAND-143-SCALE-BOUND

<a id="q-s00-cand-142-scale-bound"></a>

### Q-S00-CAND-142-SCALE-BOUND

- 问题：刻度主分割数0、1及帮助上限20的原生边界？
- 来源/归属：CAND-20261110/U-066；刻度；C07
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：刻度主分割数0、1及帮助上限20的原生边界。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCALE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-141-SCALE-PARENT

<a id="q-s00-cand-143-scale-bound"></a>

### Q-S00-CAND-143-SCALE-BOUND

- 问题：刻度副分割数0、1及帮助上限5的原生边界？
- 来源/归属：CAND-20261110/U-066；刻度；C07
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：刻度副分割数0、1及帮助上限5的原生边界。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCALE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-141-SCALE-PARENT

<a id="q-s00-cand-144-scale-render"></a>

### Q-S00-CAND-144-SCALE-RENDER

- 问题：刻度副分割数与实际绘制副线数量及位置的对应？
- 来源/归属：CAND-20261110/U-067；刻度；C07
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：刻度副分割数与实际绘制副线数量及位置的对应。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCALE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-145-scale-runtime"></a>

### Q-S00-CAND-145-SCALE-RUNTIME

- 问题：独立刻度与趋势图或仪表组合时是否存在数据绑定？
- 来源/归属：CAND-20261110/U-068；刻度；C07
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：独立刻度与趋势图或仪表组合时是否存在数据绑定。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCALE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-146-scale-parent"></a>

### Q-S00-CAND-146-SCALE-PARENT

- 问题：刻度候选U-069的分层问题？
- 来源/归属：CAND-20261110/U-069；A02全局入口/仪表归属待核；C07
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：刻度候选U-069的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCALE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-147-SCALE-VISIBILITY、Q-S00-CAND-148-SCALE-UI

<a id="q-s00-cand-147-scale-visibility"></a>

### Q-S00-CAND-147-SCALE-VISIBILITY

- 问题：工具箱btn_metre是否可达并打开哪一类仪表窗口？
- 来源/归属：CAND-20261110/U-069；A02全局入口/仪表归属待核；C07
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定当前AMT和V5.1；B/H隐藏字段只作线索，另列型号或版本扩展范围。
- 刺激：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 对象：工具箱btn_metre是否可达并打开哪一类仪表窗口。
- 预期观察：只对搜索边界报告可达、隐藏或未找到；不因字符串存在增加字段，也不推断所有型号不存在。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCALE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-146-SCALE-PARENT

<a id="q-s00-cand-148-scale-ui"></a>

### Q-S00-CAND-148-SCALE-UI

- 问题：btn_metre窗口的页签、刻度及方向字段？
- 来源/归属：CAND-20261110/U-069；A02全局入口/仪表归属待核；C07
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：btn_metre窗口的页签、刻度及方向字段。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCALE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-146-SCALE-PARENT

<a id="q-s00-cand-149-scale-parent"></a>

### Q-S00-CAND-149-SCALE-PARENT

- 问题：刻度候选U-070的分层问题？
- 来源/归属：CAND-20261110/U-070；刻度；C07
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：刻度候选U-070的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCALE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-150-SCALE-SOURCE、Q-S00-CAND-151-SCALE-COMPILE、Q-S00-CAND-152-SCALE-RUNTIME

<a id="q-s00-cand-150-scale-source"></a>

### Q-S00-CAND-150-SCALE-SOURCE

- 问题：刻度对象源字段布局？
- 来源/归属：CAND-20261110/U-070；刻度；C07
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：刻度对象源字段布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCALE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-149-SCALE-PARENT

<a id="q-s00-cand-151-scale-compile"></a>

### Q-S00-CAND-151-SCALE-COMPILE

- 问题：刻度下载记录或背景烘焙布局？
- 来源/归属：CAND-20261110/U-070；刻度；C07
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：刻度下载记录或背景烘焙布局。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCALE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-149-SCALE-PARENT

<a id="q-s00-cand-152-scale-runtime"></a>

### Q-S00-CAND-152-SCALE-RUNTIME

- 问题：固件显示刻度对象的兼容性？
- 来源/归属：CAND-20261110/U-070；刻度；C07
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件显示刻度对象的兼容性。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCALE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-149-SCALE-PARENT

<a id="q-s00-cand-153-func-fact"></a>

### Q-S00-CAND-153-FUNC-FACT

- 问题：所核V5.1功能按钮宏组能否启用？
- 来源/归属：CAND-20261110/U-071；功能按钮；C05
- 层/状态：D/U；沿用事实关闭；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：复用旧核销中相同V5.1身份及AMT范围。
- 刺激：核对原核销引用与现存证据身份；本任务不重跑原生UI或硬件。 对象：所核V5.1功能按钮宏组能否启用。
- 预期观察：只沿用已关闭事实及其版本/型号/层，不扩大为整个控件或E通过。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB、EV-S00-CAND-PRIOR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：核对原核销引用与现存证据身份；本任务不重跑原生UI或硬件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 沿用问题身份：Q-FUNC-MACRO-001
- 关闭范围：{"basis": "旧核销原有关闭，不是本轮重新测试", "version": "V5.1，同哈希私有运行副本的所核AMT范围", "layers": ["D", "U"], "conclusion": "宏组真实Qt enabled=false；使用宏/写宏没有可用交互路径。", "not_covered": "其他控件宏、其它机型、整控件通过及E。"}

<a id="q-s00-cand-154-func-help"></a>

### Q-S00-CAND-154-FUNC-HELP

- 问题：功能按钮帮助点击后的图标窗口与CHM页面路径？
- 来源/归属：CAND-20261110/U-072；功能按钮；C05
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：确认原CHM路径/哈希及成员存在，记录完整安装依赖和私有副本环境差异。
- 刺激：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 对象：功能按钮帮助点击后的图标窗口与CHM页面路径。
- 预期观察：确认目标成员或有界失败；CHM成员存在不能替代按钮直达事实。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-155-func-ui"></a>

### Q-S00-CAND-155-FUNC-UI

- 问题：功能按钮权限比较等于或不等于时误差行的直接可见性？
- 来源/归属：CAND-20261110/U-073；功能按钮；C05
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：功能按钮权限比较等于或不等于时误差行的直接可见性。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-156-func-visibility"></a>

### Q-S00-CAND-156-FUNC-VISIBILITY

- 问题：功能按钮无类型功能在当前AMT配置之外是否会产生类型项？
- 来源/归属：CAND-20261110/U-074；功能按钮；C05
- 层/状态：D/U；未闭合；atomic
- 适用范围：当前V5.1/AMT已观察的隐藏或缺席状态保留；新增其他机型/版本出现条件转能力矩阵扩展，不阻塞当前已核AMT切片。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定当前AMT和V5.1；B/H隐藏字段只作线索，另列型号或版本扩展范围。
- 刺激：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 对象：功能按钮无类型功能在当前AMT配置之外是否会产生类型项。
- 预期观察：只对搜索边界报告可达、隐藏或未找到；不因字符串存在增加字段，也不推断所有型号不存在。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-157-func-language"></a>

### Q-S00-CAND-157-FUNC-LANGUAGE

- 问题：功能按钮语言类型列表如何对应项目语言数量和顺序？
- 来源/归属：CAND-20261110/U-075；功能按钮；C05
- 层/状态：D/U/P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立中文单语及中文/English/第三种语言工程，给各状态各语言写不同文本。
- 刺激：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 对象：功能按钮语言类型列表如何对应项目语言数量和顺序。
- 预期观察：确认语言列表来源、索引及草稿规则；运行切换是否正确仍需独立E观察。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原厂双语单控件语言count=2及对应编译已补证；完整语言顺序/配置变更矩阵不由此全关。
- 下一探针：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-158-func-ui"></a>

### Q-S00-CAND-158-FUNC-UI

- 问题：功能按钮从状态1复制文字与外形按钮的真实enabled？
- 来源/归属：CAND-20261110/U-076；功能按钮；C05
- 层/状态：D/U；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：功能按钮从状态1复制文字与外形按钮的真实enabled。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原厂Qt getter已确认两项复制按钮disabled；作为已有事实复用，不为不存在的可用复制路径再造窗口。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-159-func-parent"></a>

### Q-S00-CAND-159-FUNC-PARENT

- 问题：功能按钮候选U-077的分层问题？
- 来源/归属：CAND-20261110/U-077；功能按钮；C05
- 层/状态：C/E/R；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能按钮候选U-077的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；隐藏state1橙ON、编辑state0不自动改ON及OFF/flash/ON图序已解码；E按下显示未确认。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-160-FUNC-RENDER、Q-S00-CAND-161-FUNC-COMPILE、Q-S00-CAND-162-FUNC-RUNTIME

<a id="q-s00-cand-160-func-render"></a>

### Q-S00-CAND-160-FUNC-RENDER

- 问题：功能按钮按下效果在编辑器中是否仍固定状态0？
- 来源/归属：CAND-20261110/U-077；功能按钮；C05
- 层/状态：R；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：功能按钮按下效果在编辑器中是否仍固定状态0。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；隐藏state1橙ON、编辑state0不自动改ON及OFF/flash/ON图序已解码；E按下显示未确认。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-159-FUNC-PARENT

<a id="q-s00-cand-161-func-compile"></a>

### Q-S00-CAND-161-FUNC-COMPILE

- 问题：功能按钮按下帧是否采用隐藏状态1及其图序？
- 来源/归属：CAND-20261110/U-077；功能按钮；C05
- 层/状态：C；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：功能按钮按下帧是否采用隐藏状态1及其图序。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；隐藏state1橙ON、编辑state0不自动改ON及OFF/flash/ON图序已解码；E按下显示未确认。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-159-FUNC-PARENT

<a id="q-s00-cand-162-func-runtime"></a>

### Q-S00-CAND-162-FUNC-RUNTIME

- 问题：功能按钮按下后的实际屏端外观切换？
- 来源/归属：CAND-20261110/U-077；功能按钮；C05
- 层/状态：E；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：功能按钮按下后的实际屏端外观切换。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；隐藏state1橙ON、编辑state0不自动改ON及OFF/flash/ON图序已解码；E按下显示未确认。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-159-FUNC-PARENT

<a id="q-s00-cand-163-func-parent"></a>

### Q-S00-CAND-163-FUNC-PARENT

- 问题：功能按钮候选U-078的分层问题？
- 来源/归属：CAND-20261110/U-078；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能按钮候选U-078的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-164-FUNC-RUNTIME、Q-S00-CAND-165-FUNC-RUNTIME

<a id="q-s00-cand-164-func-runtime"></a>

### Q-S00-CAND-164-FUNC-RUNTIME

- 问题：功能17选择USB下载后设备进入何种下载状态？
- 来源/归属：CAND-20261110/U-078、CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：功能17选择USB下载后设备进入何种下载状态。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-163-FUNC-PARENT、Q-S00-CAND-799-FUNC-PARENT

<a id="q-s00-cand-165-func-runtime"></a>

### Q-S00-CAND-165-FUNC-RUNTIME

- 问题：功能17选择U盘下载后设备查找何种介质及文件？
- 来源/归属：CAND-20261110/U-078、CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：功能17选择U盘下载后设备查找何种介质及文件。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-163-FUNC-PARENT、Q-S00-CAND-799-FUNC-PARENT

<a id="q-s00-cand-166-func-parent"></a>

### Q-S00-CAND-166-FUNC-PARENT

- 问题：功能按钮候选U-080的分层问题？
- 来源/归属：CAND-20261110/U-080；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能按钮候选U-080的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-167-FUNC-PERMISSION、Q-S00-CAND-168-FUNC-THRESHOLD、Q-S00-CAND-169-FUNC-THRESHOLD

<a id="q-s00-cand-167-func-permission"></a>

### Q-S00-CAND-167-FUNC-PERMISSION

- 问题：功能按钮显现与触控多条件组合逻辑？
- 来源/归属：CAND-20261110/U-080；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：选择该控件的显现或触控列，单独建立位、等级、字条件，且对象无重叠遮挡。
- 刺激：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 对象：功能按钮显现与触控多条件组合逻辑。
- 预期观察：取得显现与触控各自的真值表、组合方式及优先级；不由共用容器推断所有控件相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-166-FUNC-PARENT

<a id="q-s00-cand-168-func-threshold"></a>

### Q-S00-CAND-168-FUNC-THRESHOLD

- 问题：功能按钮等级等于阈值边界？
- 来源/归属：CAND-20261110/U-080；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：固定该控件的最低等级阈值，位和字条件关闭或恒定满足。
- 刺激：分别以阈值-1、阈值、阈值+1等级观察显现或触控；显现和触控列分开记录。 对象：功能按钮等级等于阈值边界。
- 预期观察：取得最低等级的等值边界与实际权限结果，不由字段名称预设闭开关系。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：分别以阈值-1、阈值、阈值+1等级观察显现或触控；显现和触控列分开记录。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-166-FUNC-PARENT

<a id="q-s00-cand-169-func-threshold"></a>

### Q-S00-CAND-169-FUNC-THRESHOLD

- 问题：功能按钮比较误差的计算方式？
- 来源/归属：CAND-20261110/U-080；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：固定字比较值和误差，位和等级条件恒定满足；整数和浮点分别留样本。
- 刺激：测比较值±容差及邻界，分别测试等于、不等于及0误差，保持其他字段不变。 对象：功能按钮比较误差的计算方式。
- 预期观察：取得容差闭开区间、绝对/相对误差及浮点边界，不提前假设具体公式。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：测比较值±容差及邻界，分别测试等于、不等于及0误差，保持其他字段不变。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-166-FUNC-PARENT

<a id="q-s00-cand-170-func-parent"></a>

### Q-S00-CAND-170-FUNC-PARENT

- 问题：功能按钮候选U-081的分层问题？
- 来源/归属：CAND-20261110/U-081；功能按钮；C05
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能按钮候选U-081的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；subtype3 typed源union与640B设备字段及全新样例往返已补证；E解析及动作不由ACK关闭。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-171-FUNC-SOURCE、Q-S00-CAND-172-FUNC-COMPILE、Q-S00-CAND-173-FUNC-RUNTIME

<a id="q-s00-cand-171-func-source"></a>

### Q-S00-CAND-171-FUNC-SOURCE

- 问题：功能按钮原厂源union与状态列表布局？
- 来源/归属：CAND-20261110/U-081；功能按钮；C05
- 层/状态：P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：功能按钮原厂源union与状态列表布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；subtype3 typed源union与640B设备字段及全新样例往返已补证；E解析及动作不由ACK关闭。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-170-FUNC-PARENT

<a id="q-s00-cand-172-func-compile"></a>

### Q-S00-CAND-172-FUNC-COMPILE

- 问题：功能按钮设备记录、字符字段及图帧布局？
- 来源/归属：CAND-20261110/U-081；功能按钮；C05
- 层/状态：C；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：功能按钮设备记录、字符字段及图帧布局。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；subtype3 typed源union与640B设备字段及全新样例往返已补证；E解析及动作不由ACK关闭。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-170-FUNC-PARENT

<a id="q-s00-cand-173-func-runtime"></a>

### Q-S00-CAND-173-FUNC-RUNTIME

- 问题：固件消费所编译功能按钮记录的运行兼容性？
- 来源/归属：CAND-20261110/U-081；功能按钮；C05
- 层/状态：E；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件消费所编译功能按钮记录的运行兼容性。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；subtype3 typed源union与640B设备字段及全新样例往返已补证；E解析及动作不由ACK关闭。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-170-FUNC-PARENT

<a id="q-s00-cand-174-func-parent"></a>

### Q-S00-CAND-174-FUNC-PARENT

- 问题：功能按钮候选U-082的分层问题？
- 来源/归属：CAND-20261110/U-082；功能按钮；C05
- 层/状态：D；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能按钮候选U-082的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-175-FUNC-SCOPE、Q-S00-CAND-176-FUNC-SCOPE

<a id="q-s00-cand-175-func-scope"></a>

### Q-S00-CAND-175-FUNC-SCOPE

- 问题：无线设置暂不支持的帮助声明与当前AMT适用边界？
- 来源/归属：CAND-20261110/U-082；功能按钮；C05
- 层/状态：D；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：并列原始候选、原帮助及当前V5.1直接证据，保留版本/型号和文档章节。
- 刺激：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 对象：无线设置暂不支持的帮助声明与当前AMT适用边界。
- 预期观察：形成可追溯的来源与采用边界；说明句或旧图不自动成为待实现功能或运行算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-174-FUNC-PARENT

<a id="q-s00-cand-176-func-scope"></a>

### Q-S00-CAND-176-FUNC-SCOPE

- 问题：背光调节两篇帮助冲突应采用的产品能力边界？
- 来源/归属：CAND-20261110/U-082；功能按钮；C05
- 层/状态：D；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：并列原始候选、原帮助及当前V5.1直接证据，保留版本/型号和文档章节。
- 刺激：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 对象：背光调节两篇帮助冲突应采用的产品能力边界。
- 预期观察：形成可追溯的来源与采用边界；说明句或旧图不自动成为待实现功能或运行算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-174-FUNC-PARENT

<a id="q-s00-cand-177-alarmbar-bound"></a>

### Q-S00-CAND-177-ALARMBAR-BOUND

- 问题：动态报警条移动速度是否接受0或负数以及键入提交方式？
- 来源/归属：CAND-20261110/U-083；动态报警条；C27
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：动态报警条移动速度是否接受0或负数以及键入提交方式。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARMBAR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-178-alarmbar-options"></a>

### Q-S00-CAND-178-ALARMBAR-OPTIONS

- 问题：动态报警条样式列表在右上辐射之后的完整项集？
- 来源/归属：CAND-20261110/U-084；动态报警条；C27
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：动态报警条样式列表在右上辐射之后的完整项集。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARMBAR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-179-alarmbar-bound"></a>

### Q-S00-CAND-179-ALARMBAR-BOUND

- 问题：报警登录扫描时间在1、3600及越界输入时的实际限制？
- 来源/归属：CAND-20261110/U-086；A02报警登录配置；C27
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：报警登录扫描时间在1、3600及越界输入时的实际限制。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARMBAR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-180-alarmbar-options"></a>

### Q-S00-CAND-180-ALARMBAR-OPTIONS

- 问题：报警条数完整下拉范围与512条报警区索引的关系？
- 来源/归属：CAND-20261110/U-087；A02报警登录配置；C27
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：报警条数完整下拉范围与512条报警区索引的关系。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARMBAR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-181-alarmbar-parent"></a>

### Q-S00-CAND-181-ALARMBAR-PARENT

- 问题：动态报警条候选U-088的分层问题？
- 来源/归属：CAND-20261110/U-088；动态报警条；C27
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：动态报警条候选U-088的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARMBAR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-182-ALARMBAR-RUNTIME、Q-S00-CAND-183-ALARMBAR-RUNTIME

<a id="q-s00-cand-182-alarmbar-runtime"></a>

### Q-S00-CAND-182-ALARMBAR-RUNTIME

- 问题：动态报警条移动速度值与实际滚动周期的换算？
- 来源/归属：CAND-20261110/U-088；动态报警条；C27
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：动态报警条移动速度值与实际滚动周期的换算。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARMBAR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-181-ALARMBAR-PARENT

<a id="q-s00-cand-183-alarmbar-runtime"></a>

### Q-S00-CAND-183-ALARMBAR-RUNTIME

- 问题：多条活动报警在报警条中的轮播顺序？
- 来源/归属：CAND-20261110/U-088；动态报警条；C27
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：多条活动报警在报警条中的轮播顺序。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARMBAR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-181-ALARMBAR-PARENT

<a id="q-s00-cand-184-alarmbar-ui"></a>

### Q-S00-CAND-184-ALARMBAR-UI

- 问题：条件报警编辑器V5.1页签、字段及状态联动？
- 来源/归属：CAND-20261110/U-089；A02条件报警配置；C27
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：条件报警编辑器V5.1页签、字段及状态联动。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARMBAR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-185-alarmbar-parent"></a>

### Q-S00-CAND-185-ALARMBAR-PARENT

- 问题：动态报警条候选U-090的分层问题？
- 来源/归属：CAND-20261110/U-090；动态报警条；C27
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：动态报警条候选U-090的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARMBAR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-186-ALARMBAR-SOURCE、Q-S00-CAND-187-ALARMBAR-COMPILE、Q-S00-CAND-188-ALARMBAR-RUNTIME

<a id="q-s00-cand-186-alarmbar-source"></a>

### Q-S00-CAND-186-ALARMBAR-SOURCE

- 问题：动态报警条和报警登录引用的sm布局？
- 来源/归属：CAND-20261110/U-090；动态报警条；C27
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：动态报警条和报警登录引用的sm布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARMBAR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-185-ALARMBAR-PARENT

<a id="q-s00-cand-187-alarmbar-compile"></a>

### Q-S00-CAND-187-ALARMBAR-COMPILE

- 问题：动态报警条编译记录及报警数据布局？
- 来源/归属：CAND-20261110/U-090；动态报警条；C27
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：动态报警条编译记录及报警数据布局。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARMBAR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-185-ALARMBAR-PARENT

<a id="q-s00-cand-188-alarmbar-runtime"></a>

### Q-S00-CAND-188-ALARMBAR-RUNTIME

- 问题：固件消费动态报警条记录并滚动消息的规则？
- 来源/归属：CAND-20261110/U-090；动态报警条；C27
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件消费动态报警条记录并滚动消息的规则。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARMBAR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-185-ALARMBAR-PARENT

<a id="q-s00-cand-189-historyalarm-ui"></a>

### Q-S00-CAND-189-HISTORYALARM-UI

- 问题：历史报警显示在无报警配置工程中的入口拦截提示？
- 来源/归属：CAND-20261110/U-091、CAND-20261110/U-085；历史报警显示；C29
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：历史报警显示在无报警配置工程中的入口拦截提示。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-190-historyalarm-options"></a>

### Q-S00-CAND-190-HISTORYALARM-OPTIONS

- 问题：历史报警显示字号大小下拉的完整项集？
- 来源/归属：CAND-20261110/U-092；历史报警显示；C29
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：历史报警显示字号大小下拉的完整项集。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-191-historyalarm-parent"></a>

### Q-S00-CAND-191-HISTORYALARM-PARENT

- 问题：历史报警显示候选U-093的分层问题？
- 来源/归属：CAND-20261110/U-093；历史报警显示；C29
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：历史报警显示候选U-093的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-192-HISTORYALARM-RUNTIME、Q-S00-CAND-193-HISTORYALARM-RUNTIME、Q-S00-CAND-194-HISTORYALARM-RUNTIME、Q-S00-CAND-195-HISTORYALARM-RUNTIME

<a id="q-s00-cand-192-historyalarm-runtime"></a>

### Q-S00-CAND-192-HISTORYALARM-RUNTIME

- 问题：历史报警记录上限及满记录处理？
- 来源/归属：CAND-20261110/U-093；历史报警显示；C29
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：历史报警记录上限及满记录处理。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-191-HISTORYALARM-PARENT

<a id="q-s00-cand-193-historyalarm-runtime"></a>

### Q-S00-CAND-193-HISTORYALARM-RUNTIME

- 问题：历史报警存储介质与重启后保留？
- 来源/归属：CAND-20261110/U-093；历史报警显示；C29
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：历史报警存储介质与重启后保留。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-191-HISTORYALARM-PARENT

<a id="q-s00-cand-194-historyalarm-runtime"></a>

### Q-S00-CAND-194-HISTORYALARM-RUNTIME

- 问题：历史报警时间范围和类别过滤的实际入口？
- 来源/归属：CAND-20261110/U-093；历史报警显示；C29
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：历史报警时间范围和类别过滤的实际入口。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-191-HISTORYALARM-PARENT

<a id="q-s00-cand-195-historyalarm-runtime"></a>

### Q-S00-CAND-195-HISTORYALARM-RUNTIME

- 问题：历史报警确认、清除、导出及翻页动作？
- 来源/归属：CAND-20261110/U-093；历史报警显示；C29
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：历史报警确认、清除、导出及翻页动作。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-191-HISTORYALARM-PARENT

<a id="q-s00-cand-196-historyalarm-visibility"></a>

### Q-S00-CAND-196-HISTORYALARM-VISIBILITY

- 问题：clear_hisAlarm_CB实际归属窗口及在V5.1的可达条件？
- 来源/归属：CAND-20261110/U-094；A02历史报警清除入口调查；C29
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定当前AMT和V5.1；B/H隐藏字段只作线索，另列型号或版本扩展范围。
- 刺激：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 对象：clear_hisAlarm_CB实际归属窗口及在V5.1的可达条件。
- 预期观察：只对搜索边界报告可达、隐藏或未找到；不因字符串存在增加字段，也不推断所有型号不存在。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-197-historyalarm-parent"></a>

### Q-S00-CAND-197-HISTORYALARM-PARENT

- 问题：历史报警显示候选U-095的分层问题？
- 来源/归属：CAND-20261110/U-095；历史报警显示；C29
- 层/状态：D/E/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：历史报警显示候选U-095的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-198-HISTORYALARM-LANGUAGE、Q-S00-CAND-199-HISTORYALARM-RUNTIME

<a id="q-s00-cand-198-historyalarm-language"></a>

### Q-S00-CAND-198-HISTORYALARM-LANGUAGE

- 问题：历史报警显示语言列表与工程语言对应？
- 来源/归属：CAND-20261110/U-095；历史报警显示；C29
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立中文单语及中文/English/第三种语言工程，给各状态各语言写不同文本。
- 刺激：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 对象：历史报警显示语言列表与工程语言对应。
- 预期观察：确认语言列表来源、索引及草稿规则；运行切换是否正确仍需独立E观察。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-197-HISTORYALARM-PARENT

<a id="q-s00-cand-199-historyalarm-runtime"></a>

### Q-S00-CAND-199-HISTORYALARM-RUNTIME

- 问题：历史报警消息语言切换后的实际文本？
- 来源/归属：CAND-20261110/U-095；历史报警显示；C29
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：历史报警消息语言切换后的实际文本。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-197-HISTORYALARM-PARENT

<a id="q-s00-cand-200-historyalarm-parent"></a>

### Q-S00-CAND-200-HISTORYALARM-PARENT

- 问题：历史报警显示候选U-096的分层问题？
- 来源/归属：CAND-20261110/U-096；历史报警显示；C29
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：历史报警显示候选U-096的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-201-HISTORYALARM-SOURCE、Q-S00-CAND-202-HISTORYALARM-COMPILE、Q-S00-CAND-203-HISTORYALARM-RUNTIME

<a id="q-s00-cand-201-historyalarm-source"></a>

### Q-S00-CAND-201-HISTORYALARM-SOURCE

- 问题：历史报警显示的sm及配置引用布局？
- 来源/归属：CAND-20261110/U-096；历史报警显示；C29
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：历史报警显示的sm及配置引用布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-200-HISTORYALARM-PARENT

<a id="q-s00-cand-202-historyalarm-compile"></a>

### Q-S00-CAND-202-HISTORYALARM-COMPILE

- 问题：历史报警设备数据编码？
- 来源/归属：CAND-20261110/U-096；历史报警显示；C29
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：历史报警设备数据编码。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-200-HISTORYALARM-PARENT

<a id="q-s00-cand-203-historyalarm-runtime"></a>

### Q-S00-CAND-203-HISTORYALARM-RUNTIME

- 问题：历史报警固件解析与记录消费？
- 来源/归属：CAND-20261110/U-096；历史报警显示；C29
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：历史报警固件解析与记录消费。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-200-HISTORYALARM-PARENT

<a id="q-s00-cand-204-historydata-parent"></a>

### Q-S00-CAND-204-HISTORYDATA-PARENT

- 问题：历史数据显示器候选U-097的分层问题？
- 来源/归属：CAND-20261110/U-097；历史数据显示器；C30
- 层/状态：D/E/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：历史数据显示器候选U-097的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYDATA；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-205-HISTORYDATA-BOUND、Q-S00-CAND-206-HISTORYDATA-OPTIONS、Q-S00-CAND-207-HISTORYDATA-RUNTIME

<a id="q-s00-cand-205-historydata-bound"></a>

### Q-S00-CAND-205-HISTORYDATA-BOUND

- 问题：历史数据显示行数上限是否为20？
- 来源/归属：CAND-20261110/U-097；历史数据显示器；C30
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：历史数据显示行数上限是否为20。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYDATA；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-204-HISTORYDATA-PARENT

<a id="q-s00-cand-206-historydata-options"></a>

### Q-S00-CAND-206-HISTORYDATA-OPTIONS

- 问题：历史数据显示字体大小的完整选项？
- 来源/归属：CAND-20261110/U-097；历史数据显示器；C30
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：历史数据显示字体大小的完整选项。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYDATA；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-204-HISTORYDATA-PARENT

<a id="q-s00-cand-207-historydata-runtime"></a>

### Q-S00-CAND-207-HISTORYDATA-RUNTIME

- 问题：历史数据显示采样与查询的运行交互？
- 来源/归属：CAND-20261110/U-097；历史数据显示器；C30
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：历史数据显示采样与查询的运行交互。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYDATA；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-204-HISTORYDATA-PARENT

<a id="q-s00-cand-208-historydata-language"></a>

### Q-S00-CAND-208-HISTORYDATA-LANGUAGE

- 问题：历史数据显示数据页语言同步按钮复制哪些列和文本？
- 来源/归属：CAND-20261110/U-098；历史数据显示器；C30
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立中文单语及中文/English/第三种语言工程，给各状态各语言写不同文本。
- 刺激：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 对象：历史数据显示数据页语言同步按钮复制哪些列和文本。
- 预期观察：确认语言列表来源、索引及草稿规则；运行切换是否正确仍需独立E观察。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYDATA；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-209-historydata-ui"></a>

### Q-S00-CAND-209-HISTORYDATA-UI

- 问题：多条多通道采集如何展开历史数据显示数据页及画布列？
- 来源/归属：CAND-20261110/U-099；历史数据显示器；C30
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：多条多通道采集如何展开历史数据显示数据页及画布列。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYDATA；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-210-historydata-parent"></a>

### Q-S00-CAND-210-HISTORYDATA-PARENT

- 问题：历史数据显示器候选U-100的分层问题？
- 来源/归属：CAND-20261110/U-100；历史数据显示器；C30
- 层/状态：E/R；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：历史数据显示器候选U-100的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYDATA；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-211-HISTORYDATA-RENDER、Q-S00-CAND-212-HISTORYDATA-RUNTIME

<a id="q-s00-cand-211-historydata-render"></a>

### Q-S00-CAND-211-HISTORYDATA-RENDER

- 问题：修改历史数据显示日期及时间列名后的画布回写？
- 来源/归属：CAND-20261110/U-100；历史数据显示器；C30
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：修改历史数据显示日期及时间列名后的画布回写。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYDATA；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-210-HISTORYDATA-PARENT

<a id="q-s00-cand-212-historydata-runtime"></a>

### Q-S00-CAND-212-HISTORYDATA-RUNTIME

- 问题：历史数据显示设备列头是否采用修改后的名称？
- 来源/归属：CAND-20261110/U-100；历史数据显示器；C30
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：历史数据显示设备列头是否采用修改后的名称。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYDATA；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-210-HISTORYDATA-PARENT

<a id="q-s00-cand-213-historydata-persist"></a>

### Q-S00-CAND-213-HISTORYDATA-PERSIST

- 问题：历史数据显示取消编辑后重开是否丢弃未提交字段？
- 来源/归属：CAND-20261110/U-101；历史数据显示器；C30
- 层/状态：U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留可辨识原值和工程副本，固定当前语言、状态、类型及对象ID。
- 刺激：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 对象：历史数据显示取消编辑后重开是否丢弃未提交字段。
- 预期观察：取得提交、取消和休眠草稿的保存寿命；区分ksp草稿与原厂sm实际保存字段。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYDATA；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-214-historydata-ui"></a>

### Q-S00-CAND-214-HISTORYDATA-UI

- 问题：历史数据显示对象右键菜单的实际项集？
- 来源/归属：CAND-20261110/U-102；历史数据显示器；C30
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：历史数据显示对象右键菜单的实际项集。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYDATA；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-215-historydata-help"></a>

### Q-S00-CAND-215-HISTORYDATA-HELP

- 问题：历史数据显示帮助按钮的实际CHM目标？
- 来源/归属：CAND-20261110/U-103；历史数据显示器；C30
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：确认原CHM路径/哈希及成员存在，记录完整安装依赖和私有副本环境差异。
- 刺激：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 对象：历史数据显示帮助按钮的实际CHM目标。
- 预期观察：确认目标成员或有界失败；CHM成员存在不能替代按钮直达事实。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYDATA；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-216-historydata-parent"></a>

### Q-S00-CAND-216-HISTORYDATA-PARENT

- 问题：历史数据显示器候选U-104的分层问题？
- 来源/归属：CAND-20261110/U-104；A02数据采集配置；C30
- 层/状态：D/E/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：历史数据显示器候选U-104的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYDATA；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-217-HISTORYDATA-UI、Q-S00-CAND-218-HISTORYDATA-RUNTIME、Q-S00-CAND-219-HISTORYDATA-RUNTIME

<a id="q-s00-cand-217-historydata-ui"></a>

### Q-S00-CAND-217-HISTORYDATA-UI

- 问题：数据采集数据项页的逐通道字段与启用规则？
- 来源/归属：CAND-20261110/U-104；A02数据采集配置；C30
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：数据采集数据项页的逐通道字段与启用规则。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYDATA；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-216-HISTORYDATA-PARENT

<a id="q-s00-cand-218-historydata-runtime"></a>

### Q-S00-CAND-218-HISTORYDATA-RUNTIME

- 问题：触发采样与整点采样的实际调度？
- 来源/归属：CAND-20261110/U-104；A02数据采集配置；C30
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：触发采样与整点采样的实际调度。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYDATA；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-216-HISTORYDATA-PARENT

<a id="q-s00-cand-219-historydata-runtime"></a>

### Q-S00-CAND-219-HISTORYDATA-RUNTIME

- 问题：采集满界限及满通知的写入时机？
- 来源/归属：CAND-20261110/U-104；A02数据采集配置；C30
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：采集满界限及满通知的写入时机。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYDATA；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-216-HISTORYDATA-PARENT

<a id="q-s00-cand-220-historydata-diagnostic"></a>

### Q-S00-CAND-220-HISTORYDATA-DIAGNOSTIC

- 问题：历史数据显示调查自退是否影响已引用截图的可复现性？
- 来源/归属：CAND-20261110/U-105；历史数据显示器；C30
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留发生问题的源工程、原生身份、私有桌面PID、事件类型和退出码。
- 刺激：仅在自有实例串行最小复现，区分直接点击、Qt等价事件及重试；核对已引用截图来自哪次成功会话。 对象：历史数据显示调查自退是否影响已引用截图的可复现性。
- 预期观察：确定采集可复现性或留下有界异常；未证实原因不得写成试用状态或其他程序冲突，更不要求复刻崩溃。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYDATA；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有实例串行最小复现，区分直接点击、Qt等价事件及重试；核对已引用截图来自哪次成功会话。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-221-historytrend-parent"></a>

### Q-S00-CAND-221-HISTORYTREND-PARENT

- 问题：历史趋势图候选U-106的分层问题？
- 来源/归属：CAND-20261110/U-106；A02采集数据项/C24历史趋势；C24
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：历史趋势图候选U-106的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-222-HISTORYTREND-UI、Q-S00-CAND-223-HISTORYTREND-UI

<a id="q-s00-cand-222-historytrend-ui"></a>

### Q-S00-CAND-222-HISTORYTREND-UI

- 问题：8数据项采集是否生成历史趋势1至8曲线选项？
- 来源/归属：CAND-20261110/U-106；A02采集数据项/C24历史趋势；C24
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：8数据项采集是否生成历史趋势1至8曲线选项。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-221-HISTORYTREND-PARENT

<a id="q-s00-cand-223-historytrend-ui"></a>

### Q-S00-CAND-223-HISTORYTREND-UI

- 问题：采集数据项添加删除的真实入口与引用联动？
- 来源/归属：CAND-20261110/U-106；A02采集数据项/C24历史趋势；C24
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：采集数据项添加删除的真实入口与引用联动。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-221-HISTORYTREND-PARENT

<a id="q-s00-cand-224-historytrend-parent"></a>

### Q-S00-CAND-224-HISTORYTREND-PARENT

- 问题：历史趋势图候选U-107的分层问题？
- 来源/归属：CAND-20261110/U-107；A02数据采集配置；C24
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：历史趋势图候选U-107的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-225-HISTORYTREND-BOUND、Q-S00-CAND-226-HISTORYTREND-BOUND

<a id="q-s00-cand-225-historytrend-bound"></a>

### Q-S00-CAND-225-HISTORYTREND-BOUND

- 问题：采集取样长度、总数、时距及满界限的实际边界？
- 来源/归属：CAND-20261110/U-107；A02数据采集配置；C24
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：采集取样长度、总数、时距及满界限的实际边界。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-224-HISTORYTREND-PARENT

<a id="q-s00-cand-226-historytrend-bound"></a>

### Q-S00-CAND-226-HISTORYTREND-BOUND

- 问题：采集63kbyte限制在何种提交时机校验？
- 来源/归属：CAND-20261110/U-107；A02数据采集配置；C24
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：采集63kbyte限制在何种提交时机校验。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-224-HISTORYTREND-PARENT

<a id="q-s00-cand-227-historytrend-parent"></a>

### Q-S00-CAND-227-HISTORYTREND-PARENT

- 问题：历史趋势图候选U-108的分层问题？
- 来源/归属：CAND-20261110/U-108；历史趋势图；C24
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：历史趋势图候选U-108的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-228-HISTORYTREND-BOUND、Q-S00-CAND-229-HISTORYTREND-BOUND

<a id="q-s00-cand-228-historytrend-bound"></a>

### Q-S00-CAND-228-HISTORYTREND-BOUND

- 问题：历史趋势显示范围LE_Min与LE_Max的数值输入边界？
- 来源/归属：CAND-20261110/U-108；历史趋势图；C24
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：历史趋势显示范围LE_Min与LE_Max的数值输入边界。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-227-HISTORYTREND-PARENT

<a id="q-s00-cand-229-historytrend-bound"></a>

### Q-S00-CAND-229-HISTORYTREND-BOUND

- 问题：历史趋势Y轴最小最大值的越界提示？
- 来源/归属：CAND-20261110/U-108；历史趋势图；C24
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：历史趋势Y轴最小最大值的越界提示。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-227-HISTORYTREND-PARENT

<a id="q-s00-cand-230-historytrend-parent"></a>

### Q-S00-CAND-230-HISTORYTREND-PARENT

- 问题：历史趋势图候选U-109的分层问题？
- 来源/归属：CAND-20261110/U-109；历史趋势图；C24
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：历史趋势图候选U-109的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-231-HISTORYTREND-BOUND、Q-S00-CAND-232-HISTORYTREND-BOUND

<a id="q-s00-cand-231-historytrend-bound"></a>

### Q-S00-CAND-231-HISTORYTREND-BOUND

- 问题：历史趋势次分隔数上下限？
- 来源/归属：CAND-20261110/U-109；历史趋势图；C24
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：历史趋势次分隔数上下限。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-230-HISTORYTREND-PARENT

<a id="q-s00-cand-232-historytrend-bound"></a>

### Q-S00-CAND-232-HISTORYTREND-BOUND

- 问题：历史趋势小时及秒字段的上下限？
- 来源/归属：CAND-20261110/U-109；历史趋势图；C24
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：历史趋势小时及秒字段的上下限。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-230-HISTORYTREND-PARENT

<a id="q-s00-cand-233-historytrend-parent"></a>

### Q-S00-CAND-233-HISTORYTREND-PARENT

- 问题：历史趋势图候选U-110的分层问题？
- 来源/归属：CAND-20261110/U-110；历史趋势图；C24
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：历史趋势图候选U-110的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-234-HISTORYTREND-UI、Q-S00-CAND-235-HISTORYTREND-BOUND

<a id="q-s00-cand-234-historytrend-ui"></a>

### Q-S00-CAND-234-HISTORYTREND-UI

- 问题：历史趋势X显示刻度关闭后隐藏的精确行集合？
- 来源/归属：CAND-20261110/U-110；历史趋势图；C24
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：历史趋势X显示刻度关闭后隐藏的精确行集合。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-233-HISTORYTREND-PARENT

<a id="q-s00-cand-235-historytrend-bound"></a>

### Q-S00-CAND-235-HISTORYTREND-BOUND

- 问题：历史趋势Y主分隔数的钳制范围？
- 来源/归属：CAND-20261110/U-110；历史趋势图；C24
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：历史趋势Y主分隔数的钳制范围。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-233-HISTORYTREND-PARENT

<a id="q-s00-cand-236-historytrend-ui"></a>

### Q-S00-CAND-236-HISTORYTREND-UI

- 问题：历史趋势轴色、刻度色和格线色的颜色窗口及回写？
- 来源/归属：CAND-20261110/U-111；历史趋势图；C24
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：历史趋势轴色、刻度色和格线色的颜色窗口及回写。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-237-historytrend-parent"></a>

### Q-S00-CAND-237-HISTORYTREND-PARENT

- 问题：历史趋势图候选U-112的分层问题？
- 来源/归属：CAND-20261110/U-112；历史趋势图；C24
- 层/状态：E/R；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：历史趋势图候选U-112的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-238-HISTORYTREND-RENDER、Q-S00-CAND-239-HISTORYTREND-RUNTIME

<a id="q-s00-cand-238-historytrend-render"></a>

### Q-S00-CAND-238-HISTORYTREND-RENDER

- 问题：历史趋势多数据项的分色和图例对应？
- 来源/归属：CAND-20261110/U-112；历史趋势图；C24
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：历史趋势多数据项的分色和图例对应。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-237-HISTORYTREND-PARENT

<a id="q-s00-cand-239-historytrend-runtime"></a>

### Q-S00-CAND-239-HISTORYTREND-RUNTIME

- 问题：历史趋势时间轴起点是上电还是采集启动时刻？
- 来源/归属：CAND-20261110/U-112；历史趋势图；C24
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：历史趋势时间轴起点是上电还是采集启动时刻。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-237-HISTORYTREND-PARENT

<a id="q-s00-cand-240-historytrend-help"></a>

### Q-S00-CAND-240-HISTORYTREND-HELP

- 问题：历史趋势帮助按钮的实际CHM页面？
- 来源/归属：CAND-20261110/U-113；历史趋势图；C24
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：确认原CHM路径/哈希及成员存在，记录完整安装依赖和私有副本环境差异。
- 刺激：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 对象：历史趋势帮助按钮的实际CHM页面。
- 预期观察：确认目标成员或有界失败；CHM成员存在不能替代按钮直达事实。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-241-historytrend-parent"></a>

### Q-S00-CAND-241-HISTORYTREND-PARENT

- 问题：历史趋势图候选U-114的分层问题？
- 来源/归属：CAND-20261110/U-114；历史趋势图；C24
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：历史趋势图候选U-114的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-242-HISTORYTREND-SOURCE、Q-S00-CAND-243-HISTORYTREND-COMPILE、Q-S00-CAND-244-HISTORYTREND-RUNTIME

<a id="q-s00-cand-242-historytrend-source"></a>

### Q-S00-CAND-242-HISTORYTREND-SOURCE

- 问题：数据采集与历史趋势引用的sm布局？
- 来源/归属：CAND-20261110/U-114；历史趋势图；C24
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：数据采集与历史趋势引用的sm布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-241-HISTORYTREND-PARENT

<a id="q-s00-cand-243-historytrend-compile"></a>

### Q-S00-CAND-243-HISTORYTREND-COMPILE

- 问题：历史趋势及采集的设备数据结构？
- 来源/归属：CAND-20261110/U-114；历史趋势图；C24
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：历史趋势及采集的设备数据结构。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-241-HISTORYTREND-PARENT

<a id="q-s00-cand-244-historytrend-runtime"></a>

### Q-S00-CAND-244-HISTORYTREND-RUNTIME

- 问题：固件消费采集及历史趋势记录的规则？
- 来源/归属：CAND-20261110/U-114；历史趋势图；C24
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件消费采集及历史趋势记录的规则。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-HISTORYTREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-241-HISTORYTREND-PARENT

<a id="q-s00-cand-245-picture-parent"></a>

### Q-S00-CAND-245-PICTURE-PARENT

- 问题：图片显示候选U-115的分层问题？
- 来源/归属：CAND-20261110/U-115；图片显示；C09
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：图片显示候选U-115的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-246-PICTURE-UI、Q-S00-CAND-247-PICTURE-PERSIST、Q-S00-CAND-248-PICTURE-DIAGNOSTIC

<a id="q-s00-cand-246-picture-ui"></a>

### Q-S00-CAND-246-PICTURE-UI

- 问题：图片显示静态模式状态切换条件的真实enabled与点击联动？
- 来源/归属：CAND-20261110/U-115；图片显示；C09
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：图片显示静态模式状态切换条件的真实enabled与点击联动。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-245-PICTURE-PARENT

<a id="q-s00-cand-247-picture-persist"></a>

### Q-S00-CAND-247-PICTURE-PERSIST

- 问题：多幅切换再回静态后的闪烁标题及回填寿命？
- 来源/归属：CAND-20261110/U-115；图片显示；C09
- 层/状态：U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留可辨识原值和工程副本，固定当前语言、状态、类型及对象ID。
- 刺激：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 对象：多幅切换再回静态后的闪烁标题及回填寿命。
- 预期观察：取得提交、取消和休眠草稿的保存寿命；区分ksp草稿与原厂sm实际保存字段。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-245-PICTURE-PARENT

<a id="q-s00-cand-248-picture-diagnostic"></a>

### Q-S00-CAND-248-PICTURE-DIAGNOSTIC

- 问题：图片显示合成输入导致窗口消失能否由等价事件复现？
- 来源/归属：CAND-20261110/U-115；图片显示；C09
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留发生问题的源工程、原生身份、私有桌面PID、事件类型和退出码。
- 刺激：仅在自有实例串行最小复现，区分直接点击、Qt等价事件及重试；核对已引用截图来自哪次成功会话。 对象：图片显示合成输入导致窗口消失能否由等价事件复现。
- 预期观察：确定采集可复现性或留下有界异常；未证实原因不得写成试用状态或其他程序冲突，更不要求复刻崩溃。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有实例串行最小复现，区分直接点击、Qt等价事件及重试；核对已引用截图来自哪次成功会话。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-245-PICTURE-PARENT

<a id="q-s00-cand-249-picture-parent"></a>

### Q-S00-CAND-249-PICTURE-PARENT

- 问题：图片显示候选U-116的分层问题？
- 来源/归属：CAND-20261110/U-116；图片显示；C09
- 层/状态：C/D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：图片显示候选U-116的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-250-PICTURE-ADDRESS、Q-S00-CAND-251-PICTURE-UI

<a id="q-s00-cand-250-picture-address"></a>

### Q-S00-CAND-250-PICTURE-ADDRESS

- 问题：图片显示数值条件选择后监视地址是否限定字地址？
- 来源/归属：CAND-20261110/U-116；图片显示；C09
- 层/状态：D/U/C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定所用地址角色、位宽、连接/型号和通道；保留无连接及已配置工程两份。
- 刺激：从指定控件字段打开拾取器，枚举相应家族并选首末/宽地址边界，确认与取消后核对回填和跨度。 对象：图片显示数值条件选择后监视地址是否限定字地址。
- 预期观察：取得此调用点的地址角色与可达族；别的控件同款窗口仅作线索，不能代替本入口。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：从指定控件字段打开拾取器，枚举相应家族并选首末/宽地址边界，确认与取消后核对回填和跨度。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-249-PICTURE-PARENT

<a id="q-s00-cand-251-picture-ui"></a>

### Q-S00-CAND-251-PICTURE-UI

- 问题：数值条件与总状态数的联动？
- 来源/归属：CAND-20261110/U-116；图片显示；C09
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：数值条件与总状态数的联动。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-249-PICTURE-PARENT

<a id="q-s00-cand-252-picture-parent"></a>

### Q-S00-CAND-252-PICTURE-PARENT

- 问题：图片显示候选U-117的分层问题？
- 来源/归属：CAND-20261110/U-117；图片显示；C09
- 层/状态：P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：图片显示候选U-117的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-253-PICTURE-GALLERY、Q-S00-CAND-254-PICTURE-GALLERY

<a id="q-s00-cand-253-picture-gallery"></a>

### Q-S00-CAND-253-PICTURE-GALLERY

- 问题：系统图库选择后图片显示列表如何回填？
- 来源/归属：CAND-20261110/U-117；图片显示；C09
- 层/状态：U/P/R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立独立工程图库，加入有唯一内容的两张图并设置一个对象引用，保留资源及工程SHA。
- 刺激：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 对象：系统图库选择后图片显示列表如何回填。
- 预期观察：确定列表、路径、引用、删除恢复及落盘时机；共用图库实现不证明每个控件调用点相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-252-PICTURE-PARENT

<a id="q-s00-cand-254-picture-gallery"></a>

### Q-S00-CAND-254-PICTURE-GALLERY

- 问题：多状态图片逐状态选择及保存后的图片对应？
- 来源/归属：CAND-20261110/U-117；图片显示；C09
- 层/状态：U/P/R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立独立工程图库，加入有唯一内容的两张图并设置一个对象引用，保留资源及工程SHA。
- 刺激：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 对象：多状态图片逐状态选择及保存后的图片对应。
- 预期观察：确定列表、路径、引用、删除恢复及落盘时机；共用图库实现不证明每个控件调用点相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-252-PICTURE-PARENT

<a id="q-s00-cand-255-picture-parent"></a>

### Q-S00-CAND-255-PICTURE-PARENT

- 问题：图片显示候选U-118的分层问题？
- 来源/归属：CAND-20261110/U-118；图片显示；C09
- 层/状态：D/P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：图片显示候选U-118的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-256-PICTURE-GALLERY、Q-S00-CAND-257-PICTURE-UI

<a id="q-s00-cand-256-picture-gallery"></a>

### Q-S00-CAND-256-PICTURE-GALLERY

- 问题：图片显示批量添加和删除的起止位置含义？
- 来源/归属：CAND-20261110/U-118；图片显示；C09
- 层/状态：U/P/R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立独立工程图库，加入有唯一内容的两张图并设置一个对象引用，保留资源及工程SHA。
- 刺激：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 对象：图片显示批量添加和删除的起止位置含义。
- 预期观察：确定列表、路径、引用、删除恢复及落盘时机；共用图库实现不证明每个控件调用点相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-255-PICTURE-PARENT

<a id="q-s00-cand-257-picture-ui"></a>

### Q-S00-CAND-257-PICTURE-UI

- 问题：图片显示上下行调整后的顺序与实际轮播顺序？
- 来源/归属：CAND-20261110/U-118；图片显示；C09
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：图片显示上下行调整后的顺序与实际轮播顺序。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-255-PICTURE-PARENT

<a id="q-s00-cand-258-picture-parent"></a>

### Q-S00-CAND-258-PICTURE-PARENT

- 问题：图片显示候选U-119的分层问题？
- 来源/归属：CAND-20261110/U-119；图片显示；C09
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：图片显示候选U-119的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-259-PICTURE-BOUND、Q-S00-CAND-260-PICTURE-BOUND

<a id="q-s00-cand-259-picture-bound"></a>

### Q-S00-CAND-259-PICTURE-BOUND

- 问题：图片显示导入GIF的拒绝提示？
- 来源/归属：CAND-20261110/U-119；图片显示；C09
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：图片显示导入GIF的拒绝提示。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-258-PICTURE-PARENT

<a id="q-s00-cand-260-picture-bound"></a>

### Q-S00-CAND-260-PICTURE-BOUND

- 问题：图片显示大于1MiB或损坏图片的校验？
- 来源/归属：CAND-20261110/U-119；图片显示；C09
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：图片显示大于1MiB或损坏图片的校验。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-258-PICTURE-PARENT

<a id="q-s00-cand-261-picture-ui"></a>

### Q-S00-CAND-261-PICTURE-UI

- 问题：图片显示权限等于与不等于时误差行显隐？
- 来源/归属：CAND-20261110/U-120；图片显示；C09
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：图片显示权限等于与不等于时误差行显隐。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-262-picture-parent"></a>

### Q-S00-CAND-262-PICTURE-PARENT

- 问题：图片显示候选U-121的分层问题？
- 来源/归属：CAND-20261110/U-121；图片显示；C09
- 层/状态：D/E/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：图片显示候选U-121的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-263-PICTURE-BOUND、Q-S00-CAND-264-PICTURE-BOUND、Q-S00-CAND-265-PICTURE-RUNTIME

<a id="q-s00-cand-263-picture-bound"></a>

### Q-S00-CAND-263-PICTURE-BOUND

- 问题：图片显示间隔时间的合法范围？
- 来源/归属：CAND-20261110/U-121；图片显示；C09
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：图片显示间隔时间的合法范围。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-262-PICTURE-PARENT

<a id="q-s00-cand-264-picture-bound"></a>

### Q-S00-CAND-264-PICTURE-BOUND

- 问题：图片显示总状态数上限与列表行数的关系？
- 来源/归属：CAND-20261110/U-121；图片显示；C09
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：图片显示总状态数上限与列表行数的关系。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-262-PICTURE-PARENT

<a id="q-s00-cand-265-picture-runtime"></a>

### Q-S00-CAND-265-PICTURE-RUNTIME

- 问题：图片显示多幅切换的设备轮播周期？
- 来源/归属：CAND-20261110/U-121；图片显示；C09
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：图片显示多幅切换的设备轮播周期。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-262-PICTURE-PARENT

<a id="q-s00-cand-266-picture-help"></a>

### Q-S00-CAND-266-PICTURE-HELP

- 问题：图片显示帮助按钮在完整安装环境的目标和行为？
- 来源/归属：CAND-20261110/U-122；图片显示；C09
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：确认原CHM路径/哈希及成员存在，记录完整安装依赖和私有副本环境差异。
- 刺激：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 对象：图片显示帮助按钮在完整安装环境的目标和行为。
- 预期观察：确认目标成员或有界失败；CHM成员存在不能替代按钮直达事实。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-267-picture-parent"></a>

### Q-S00-CAND-267-PICTURE-PARENT

- 问题：图片显示候选U-123的分层问题？
- 来源/归属：CAND-20261110/U-123；图片显示；C09
- 层/状态：C/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：图片显示候选U-123的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-268-PICTURE-SOURCE、Q-S00-CAND-269-PICTURE-COMPILE

<a id="q-s00-cand-268-picture-source"></a>

### Q-S00-CAND-268-PICTURE-SOURCE

- 问题：图片显示多状态资源的内嵌或引用保存结构？
- 来源/归属：CAND-20261110/U-123；图片显示；C09
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：图片显示多状态资源的内嵌或引用保存结构。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-267-PICTURE-PARENT

<a id="q-s00-cand-269-picture-compile"></a>

### Q-S00-CAND-269-PICTURE-COMPILE

- 问题：图片显示调色板及多状态下载打包布局？
- 来源/归属：CAND-20261110/U-123；图片显示；C09
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：图片显示调色板及多状态下载打包布局。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-267-PICTURE-PARENT

<a id="q-s00-cand-270-picture-scope"></a>

### Q-S00-CAND-270-PICTURE-SCOPE

- 问题：帮助11.1外部图片直选流程与V5.1差异的版本适用性？
- 来源/归属：CAND-20261110/U-124；图片显示；C09
- 层/状态：D；未闭合；atomic
- 适用范围：当前V5.1/AMT已观察的隐藏或缺席状态保留；新增其他机型/版本出现条件转能力矩阵扩展，不阻塞当前已核AMT切片。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：并列原始候选、原帮助及当前V5.1直接证据，保留版本/型号和文档章节。
- 刺激：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 对象：帮助11.1外部图片直选流程与V5.1差异的版本适用性。
- 预期观察：形成可追溯的来源与采用边界；说明句或旧图不自动成为待实现功能或运行算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-PICTURE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-271-graph-parent"></a>

### Q-S00-CAND-271-GRAPH-PARENT

- 问题：图表候选U-125的分层问题？
- 来源/归属：CAND-20261110/U-125；图表；C22
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：图表候选U-125的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-272-GRAPH-OPTIONS、Q-S00-CAND-273-GRAPH-UI

<a id="q-s00-cand-272-graph-options"></a>

### Q-S00-CAND-272-GRAPH-OPTIONS

- 问题：图表图案列表右上辐射之后的完整选项？
- 来源/归属：CAND-20261110/U-125；图表；C22
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：图表图案列表右上辐射之后的完整选项。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-271-GRAPH-PARENT

<a id="q-s00-cand-273-graph-ui"></a>

### Q-S00-CAND-273-GRAPH-UI

- 问题：图案色按钮是否依赖非纯色图案并打开颜色窗？
- 来源/归属：CAND-20261110/U-125；图表；C22
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：图案色按钮是否依赖非纯色图案并打开颜色窗。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-271-GRAPH-PARENT

<a id="q-s00-cand-274-graph-options"></a>

### Q-S00-CAND-274-GRAPH-OPTIONS

- 问题：图表线宽和填充线宽的实际项集是否超过11到32？
- 来源/归属：CAND-20261110/U-126；图表；C22
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：图表线宽和填充线宽的实际项集是否超过11到32。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-275-graph-parent"></a>

### Q-S00-CAND-275-GRAPH-PARENT

- 问题：图表候选U-127的分层问题？
- 来源/归属：CAND-20261110/U-127；图表；C22
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：图表候选U-127的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-276-GRAPH-BOUND、Q-S00-CAND-277-GRAPH-BOUND、Q-S00-CAND-278-GRAPH-BOUND

<a id="q-s00-cand-276-graph-bound"></a>

### Q-S00-CAND-276-GRAPH-BOUND

- 问题：图表仪表显示范围最小值大于最大值的提示？
- 来源/归属：CAND-20261110/U-127；图表；C22
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：图表仪表显示范围最小值大于最大值的提示。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-275-GRAPH-PARENT

<a id="q-s00-cand-277-graph-bound"></a>

### Q-S00-CAND-277-GRAPH-BOUND

- 问题：图表源范围极值格式及颠倒校验？
- 来源/归属：CAND-20261110/U-127；图表；C22
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：图表源范围极值格式及颠倒校验。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-275-GRAPH-PARENT

<a id="q-s00-cand-278-graph-bound"></a>

### Q-S00-CAND-278-GRAPH-BOUND

- 问题：两极柱中点超过边界的提示？
- 来源/归属：CAND-20261110/U-127；图表；C22
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：两极柱中点超过边界的提示。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-275-GRAPH-PARENT

<a id="q-s00-cand-279-graph-parent"></a>

### Q-S00-CAND-279-GRAPH-PARENT

- 问题：图表候选U-128的分层问题？
- 来源/归属：CAND-20261110/U-128；图表；C22
- 层/状态：E/R；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：图表候选U-128的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-280-GRAPH-RENDER、Q-S00-CAND-281-GRAPH-RUNTIME

<a id="q-s00-cand-280-graph-render"></a>

### Q-S00-CAND-280-GRAPH-RENDER

- 问题：图表刻度值、刻度轴和次刻度开关的画布变化？
- 来源/归属：CAND-20261110/U-128；图表；C22
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：图表刻度值、刻度轴和次刻度开关的画布变化。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-279-GRAPH-PARENT

<a id="q-s00-cand-281-graph-runtime"></a>

### Q-S00-CAND-281-GRAPH-RUNTIME

- 问题：图表刻度颜色及字号在屏端的生效范围？
- 来源/归属：CAND-20261110/U-128；图表；C22
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：图表刻度颜色及字号在屏端的生效范围。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-279-GRAPH-PARENT

<a id="q-s00-cand-282-graph-persist"></a>

### Q-S00-CAND-282-GRAPH-PERSIST

- 问题：图表各形态逐字段修改后保存重开的回填？
- 来源/归属：CAND-20261110/U-129；图表；C22
- 层/状态：U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留可辨识原值和工程副本，固定当前语言、状态、类型及对象ID。
- 刺激：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 对象：图表各形态逐字段修改后保存重开的回填。
- 预期观察：取得提交、取消和休眠草稿的保存寿命；区分ksp草稿与原厂sm实际保存字段。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-283-graph-parent"></a>

### Q-S00-CAND-283-GRAPH-PARENT

- 问题：图表候选U-130的分层问题？
- 来源/归属：CAND-20261110/U-130；图表；C22
- 层/状态：R；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：图表候选U-130的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-284-GRAPH-RENDER、Q-S00-CAND-285-GRAPH-RENDER

<a id="q-s00-cand-284-graph-render"></a>

### Q-S00-CAND-284-GRAPH-RENDER

- 问题：图表顺逆时针与帮助左右旋转的方向对应？
- 来源/归属：CAND-20261110/U-130；图表；C22
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：图表顺逆时针与帮助左右旋转的方向对应。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-283-GRAPH-PARENT

<a id="q-s00-cand-285-graph-render"></a>

### Q-S00-CAND-285-GRAPH-RENDER

- 问题：图表圆弧起始角和跨度的零点及正方向？
- 来源/归属：CAND-20261110/U-130；图表；C22
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：图表圆弧起始角和跨度的零点及正方向。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-283-GRAPH-PARENT

<a id="q-s00-cand-286-graph-permission"></a>

### Q-S00-CAND-286-GRAPH-PERMISSION

- 问题：图表多个显现条件的组合逻辑与优先级？
- 来源/归属：CAND-20261110/U-131；图表；C22
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：选择该控件的显现或触控列，单独建立位、等级、字条件，且对象无重叠遮挡。
- 刺激：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 对象：图表多个显现条件的组合逻辑与优先级。
- 预期观察：取得显现与触控各自的真值表、组合方式及优先级；不由共用容器推断所有控件相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-287-graph-parent"></a>

### Q-S00-CAND-287-GRAPH-PARENT

- 问题：图表候选U-132的分层问题？
- 来源/归属：CAND-20261110/U-132；图表；C22
- 层/状态：E/R；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：图表候选U-132的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-288-GRAPH-RENDER、Q-S00-CAND-289-GRAPH-RUNTIME

<a id="q-s00-cand-288-graph-render"></a>

### Q-S00-CAND-288-GRAPH-RENDER

- 问题：图表圆环透明度与仪表透明背景的PC合成公式？
- 来源/归属：CAND-20261110/U-132；图表；C22
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：图表圆环透明度与仪表透明背景的PC合成公式。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-287-GRAPH-PARENT

<a id="q-s00-cand-289-graph-runtime"></a>

### Q-S00-CAND-289-GRAPH-RUNTIME

- 问题：图表透明度及背景在设备端的实际绘制？
- 来源/归属：CAND-20261110/U-132；图表；C22
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：图表透明度及背景在设备端的实际绘制。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-287-GRAPH-PARENT

<a id="q-s00-cand-290-graph-parent"></a>

### Q-S00-CAND-290-GRAPH-PARENT

- 问题：图表候选U-133的分层问题？
- 来源/归属：CAND-20261110/U-133；图表；C22
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：图表候选U-133的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-291-GRAPH-SOURCE、Q-S00-CAND-292-GRAPH-COMPILE、Q-S00-CAND-293-GRAPH-RUNTIME

<a id="q-s00-cand-291-graph-source"></a>

### Q-S00-CAND-291-GRAPH-SOURCE

- 问题：图表各形态的sm对象布局？
- 来源/归属：CAND-20261110/U-133；图表；C22
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：图表各形态的sm对象布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-290-GRAPH-PARENT

<a id="q-s00-cand-292-graph-compile"></a>

### Q-S00-CAND-292-GRAPH-COMPILE

- 问题：图表各形态的设备记录与图像布局？
- 来源/归属：CAND-20261110/U-133；图表；C22
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：图表各形态的设备记录与图像布局。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-290-GRAPH-PARENT

<a id="q-s00-cand-293-graph-runtime"></a>

### Q-S00-CAND-293-GRAPH-RUNTIME

- 问题：固件消费图表形态记录的规则？
- 来源/归属：CAND-20261110/U-133；图表；C22
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件消费图表形态记录的规则。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-290-GRAPH-PARENT

<a id="q-s00-cand-294-graph-help"></a>

### Q-S00-CAND-294-GRAPH-HELP

- 问题：完整安装环境图表帮助是否直达Graph/Graph.htm？
- 来源/归属：CAND-20261110/U-134；图表；C22
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：确认原CHM路径/哈希及成员存在，记录完整安装依赖和私有副本环境差异。
- 刺激：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 对象：完整安装环境图表帮助是否直达Graph/Graph.htm。
- 预期观察：确认目标成员或有界失败；CHM成员存在不能替代按钮直达事实。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-GRAPH；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-295-multifunc-parent"></a>

### Q-S00-CAND-295-MULTIFUNC-PARENT

- 问题：多功能按钮候选U-135的分层问题？
- 来源/归属：CAND-20261110/U-135；多功能按钮；C16
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：多功能按钮候选U-135的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-296-MULTIFUNC-UI、Q-S00-CAND-297-MULTIFUNC-UI

<a id="q-s00-cand-296-multifunc-ui"></a>

### Q-S00-CAND-296-MULTIFUNC-UI

- 问题：多功能按钮宏组真实enabled与列表联动？
- 来源/归属：CAND-20261110/U-135；多功能按钮；C16
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：多功能按钮宏组真实enabled与列表联动。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-295-MULTIFUNC-PARENT

<a id="q-s00-cand-297-multifunc-ui"></a>

### Q-S00-CAND-297-MULTIFUNC-UI

- 问题：多功能按钮写宏编辑保存流程？
- 来源/归属：CAND-20261110/U-135；多功能按钮；C16
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：多功能按钮写宏编辑保存流程。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-295-MULTIFUNC-PARENT

<a id="q-s00-cand-298-multifunc-parent"></a>

### Q-S00-CAND-298-MULTIFUNC-PARENT

- 问题：多功能按钮候选U-136的分层问题？
- 来源/归属：CAND-20261110/U-136；多功能按钮；C16
- 层/状态：D/E/P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：多功能按钮候选U-136的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-299-MULTIFUNC-LANGUAGE、Q-S00-CAND-300-MULTIFUNC-RENDER、Q-S00-CAND-301-MULTIFUNC-RUNTIME

<a id="q-s00-cand-299-multifunc-language"></a>

### Q-S00-CAND-299-MULTIFUNC-LANGUAGE

- 问题：多功能按钮多语言列表与各语言文本保存？
- 来源/归属：CAND-20261110/U-136；多功能按钮；C16
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立中文单语及中文/English/第三种语言工程，给各状态各语言写不同文本。
- 刺激：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 对象：多功能按钮多语言列表与各语言文本保存。
- 预期观察：确认语言列表来源、索引及草稿规则；运行切换是否正确仍需独立E观察。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-298-MULTIFUNC-PARENT

<a id="q-s00-cand-300-multifunc-render"></a>

### Q-S00-CAND-300-MULTIFUNC-RENDER

- 问题：多功能按钮同文和文本闪烁的图像含义？
- 来源/归属：CAND-20261110/U-136；多功能按钮；C16
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：多功能按钮同文和文本闪烁的图像含义。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-298-MULTIFUNC-PARENT

<a id="q-s00-cand-301-multifunc-runtime"></a>

### Q-S00-CAND-301-MULTIFUNC-RUNTIME

- 问题：多功能按钮同文与闪烁的运行表现？
- 来源/归属：CAND-20261110/U-136；多功能按钮；C16
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：多功能按钮同文与闪烁的运行表现。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-298-MULTIFUNC-PARENT

<a id="q-s00-cand-302-multifunc-parent"></a>

### Q-S00-CAND-302-MULTIFUNC-PARENT

- 问题：多功能按钮候选U-137的分层问题？
- 来源/归属：CAND-20261110/U-137；多功能按钮；C16
- 层/状态：D/P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：多功能按钮候选U-137的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-303-MULTIFUNC-GALLERY、Q-S00-CAND-304-MULTIFUNC-UI、Q-S00-CAND-305-MULTIFUNC-RENDER

<a id="q-s00-cand-303-multifunc-gallery"></a>

### Q-S00-CAND-303-MULTIFUNC-GALLERY

- 问题：多功能按钮系统及工程图库选择回填？
- 来源/归属：CAND-20261110/U-137；多功能按钮；C16
- 层/状态：U/P/R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立独立工程图库，加入有唯一内容的两张图并设置一个对象引用，保留资源及工程SHA。
- 刺激：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 对象：多功能按钮系统及工程图库选择回填。
- 预期观察：确定列表、路径、引用、删除恢复及落盘时机；共用图库实现不证明每个控件调用点相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-302-MULTIFUNC-PARENT

<a id="q-s00-cand-304-multifunc-ui"></a>

### Q-S00-CAND-304-MULTIFUNC-UI

- 问题：多功能按钮不使用图片的路径保留及字段启用？
- 来源/归属：CAND-20261110/U-137；多功能按钮；C16
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：多功能按钮不使用图片的路径保留及字段启用。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-302-MULTIFUNC-PARENT

<a id="q-s00-cand-305-multifunc-render"></a>

### Q-S00-CAND-305-MULTIFUNC-RENDER

- 问题：多功能按钮3D、透明及透明度的合成？
- 来源/归属：CAND-20261110/U-137；多功能按钮；C16
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：多功能按钮3D、透明及透明度的合成。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-302-MULTIFUNC-PARENT

<a id="q-s00-cand-306-multifunc-address"></a>

### Q-S00-CAND-306-MULTIFUNC-ADDRESS

- 问题：多功能按钮地址拾取窗口的地址角色和回填？
- 来源/归属：CAND-20261110/U-138；多功能按钮；C16
- 层/状态：D/U/C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定所用地址角色、位宽、连接/型号和通道；保留无连接及已配置工程两份。
- 刺激：从指定控件字段打开拾取器，枚举相应家族并选首末/宽地址边界，确认与取消后核对回填和跨度。 对象：多功能按钮地址拾取窗口的地址角色和回填。
- 预期观察：取得此调用点的地址角色与可达族；别的控件同款窗口仅作线索，不能代替本入口。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：从指定控件字段打开拾取器，枚举相应家族并选首末/宽地址边界，确认与取消后核对回填和跨度。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-307-multifunc-parent"></a>

### Q-S00-CAND-307-MULTIFUNC-PARENT

- 问题：多功能按钮候选U-139的分层问题？
- 来源/归属：CAND-20261110/U-139；多功能按钮；C16
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：多功能按钮候选U-139的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-308-MULTIFUNC-BOUND、Q-S00-CAND-309-MULTIFUNC-UI

<a id="q-s00-cand-308-multifunc-bound"></a>

### Q-S00-CAND-308-MULTIFUNC-BOUND

- 问题：多功能按钮功能列表清空到0条能否确定？
- 来源/归属：CAND-20261110/U-139；多功能按钮；C16
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：多功能按钮功能列表清空到0条能否确定。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-307-MULTIFUNC-PARENT

<a id="q-s00-cand-309-multifunc-ui"></a>

### Q-S00-CAND-309-MULTIFUNC-UI

- 问题：空功能列表确定后的画布对象及可编辑性？
- 来源/归属：CAND-20261110/U-139；多功能按钮；C16
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：空功能列表确定后的画布对象及可编辑性。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-307-MULTIFUNC-PARENT

<a id="q-s00-cand-310-multifunc-persist"></a>

### Q-S00-CAND-310-MULTIFUNC-PERSIST

- 问题：多功能按钮取消后再次双击是否保留原列表及字段？
- 来源/归属：CAND-20261110/U-140；多功能按钮；C16
- 层/状态：U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留可辨识原值和工程副本，固定当前语言、状态、类型及对象ID。
- 刺激：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 对象：多功能按钮取消后再次双击是否保留原列表及字段。
- 预期观察：取得提交、取消和休眠草稿的保存寿命；区分ksp草稿与原厂sm实际保存字段。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-311-multifunc-parent"></a>

### Q-S00-CAND-311-MULTIFUNC-PARENT

- 问题：多功能按钮候选U-141的分层问题？
- 来源/归属：CAND-20261110/U-141；多功能按钮；C16
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：多功能按钮候选U-141的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-312-MULTIFUNC-RUNTIME、Q-S00-CAND-313-MULTIFUNC-RUNTIME、Q-S00-CAND-314-MULTIFUNC-RUNTIME

<a id="q-s00-cand-312-multifunc-runtime"></a>

### Q-S00-CAND-312-MULTIFUNC-RUNTIME

- 问题：多功能按钮置位、复位、点动、交替的触控时序？
- 来源/归属：CAND-20261110/U-141；多功能按钮；C16
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：多功能按钮置位、复位、点动、交替的触控时序。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-311-MULTIFUNC-PARENT

<a id="q-s00-cand-313-multifunc-runtime"></a>

### Q-S00-CAND-313-MULTIFUNC-RUNTIME

- 问题：多功能按钮多条命令依次执行的间隔？
- 来源/归属：CAND-20261110/U-141；多功能按钮；C16
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：多功能按钮多条命令依次执行的间隔。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-311-MULTIFUNC-PARENT

<a id="q-s00-cand-314-multifunc-runtime"></a>

### Q-S00-CAND-314-MULTIFUNC-RUNTIME

- 问题：多功能按钮某条命令失败后的后续执行策略？
- 来源/归属：CAND-20261110/U-141；多功能按钮；C16
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：多功能按钮某条命令失败后的后续执行策略。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-311-MULTIFUNC-PARENT

<a id="q-s00-cand-315-multifunc-runtime"></a>

### Q-S00-CAND-315-MULTIFUNC-RUNTIME

- 问题：多功能按钮默认0的按压时间是否表示不限时及实际触发单位？
- 来源/归属：CAND-20261110/U-142；多功能按钮；C16
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：多功能按钮默认0的按压时间是否表示不限时及实际触发单位。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-316-multifunc-visibility"></a>

### Q-S00-CAND-316-MULTIFUNC-VISIBILITY

- 问题：多功能按钮隐藏autoLoadB及autoLoadW的可达条件？
- 来源/归属：CAND-20261110/U-143；多功能按钮；C16
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定当前AMT和V5.1；B/H隐藏字段只作线索，另列型号或版本扩展范围。
- 刺激：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 对象：多功能按钮隐藏autoLoadB及autoLoadW的可达条件。
- 预期观察：只对搜索边界报告可达、隐藏或未找到；不因字符串存在增加字段，也不推断所有型号不存在。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-317-multifunc-parent"></a>

### Q-S00-CAND-317-MULTIFUNC-PARENT

- 问题：多功能按钮候选U-144的分层问题？
- 来源/归属：CAND-20261110/U-144；多功能按钮；C16
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：多功能按钮候选U-144的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-318-MULTIFUNC-SOURCE、Q-S00-CAND-319-MULTIFUNC-COMPILE、Q-S00-CAND-320-MULTIFUNC-RUNTIME

<a id="q-s00-cand-318-multifunc-source"></a>

### Q-S00-CAND-318-MULTIFUNC-SOURCE

- 问题：多功能按钮命令列表与公共属性的sm布局？
- 来源/归属：CAND-20261110/U-144；多功能按钮；C16
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：多功能按钮命令列表与公共属性的sm布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-317-MULTIFUNC-PARENT

<a id="q-s00-cand-319-multifunc-compile"></a>

### Q-S00-CAND-319-MULTIFUNC-COMPILE

- 问题：多功能按钮设备命令序列编码？
- 来源/归属：CAND-20261110/U-144；多功能按钮；C16
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：多功能按钮设备命令序列编码。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-317-MULTIFUNC-PARENT

<a id="q-s00-cand-320-multifunc-runtime"></a>

### Q-S00-CAND-320-MULTIFUNC-RUNTIME

- 问题：固件执行多功能列表的兼容性？
- 来源/归属：CAND-20261110/U-144；多功能按钮；C16
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件执行多功能列表的兼容性。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTIFUNC；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-317-MULTIFUNC-PARENT

<a id="q-s00-cand-321-multistate-parent"></a>

### Q-S00-CAND-321-MULTISTATE-PARENT

- 问题：多态按钮候选U-145的分层问题？
- 来源/归属：CAND-20261110/U-145；多态按钮；C15
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：多态按钮候选U-145的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-322-MULTISTATE-RUNTIME、Q-S00-CAND-323-MULTISTATE-RUNTIME

<a id="q-s00-cand-322-multistate-runtime"></a>

### Q-S00-CAND-322-MULTISTATE-RUNTIME

- 问题：多态按钮六操作分别如何改变当前状态？
- 来源/归属：CAND-20261110/U-145；多态按钮；C15
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：多态按钮六操作分别如何改变当前状态。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-321-MULTISTATE-PARENT

<a id="q-s00-cand-323-multistate-runtime"></a>

### Q-S00-CAND-323-MULTISTATE-RUNTIME

- 问题：多态键盘输入的弹出内容与提交结果？
- 来源/归属：CAND-20261110/U-145；多态按钮；C15
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：多态键盘输入的弹出内容与提交结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-321-MULTISTATE-PARENT

<a id="q-s00-cand-324-multistate-scope"></a>

### Q-S00-CAND-324-MULTISTATE-SCOPE

- 问题：多态按钮下限2与状态编号0至31的约束依据？
- 来源/归属：CAND-20261110/U-146；多态按钮；C15
- 层/状态：D；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：并列原始候选、原帮助及当前V5.1直接证据，保留版本/型号和文档章节。
- 刺激：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 对象：多态按钮下限2与状态编号0至31的约束依据。
- 预期观察：形成可追溯的来源与采用边界；说明句或旧图不自动成为待实现功能或运行算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-325-multistate-parent"></a>

### Q-S00-CAND-325-MULTISTATE-PARENT

- 问题：多态按钮候选U-147的分层问题？
- 来源/归属：CAND-20261110/U-147；多态按钮；C15
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：多态按钮候选U-147的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-326-MULTISTATE-UI、Q-S00-CAND-327-MULTISTATE-UI

<a id="q-s00-cand-326-multistate-ui"></a>

### Q-S00-CAND-326-MULTISTATE-UI

- 问题：多态按钮宏组真实enabled与绑定联动？
- 来源/归属：CAND-20261110/U-147；多态按钮；C15
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：多态按钮宏组真实enabled与绑定联动。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-325-MULTISTATE-PARENT

<a id="q-s00-cand-327-multistate-ui"></a>

### Q-S00-CAND-327-MULTISTATE-UI

- 问题：多态按钮宏编辑器内容和保存流程？
- 来源/归属：CAND-20261110/U-147；多态按钮；C15
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：多态按钮宏编辑器内容和保存流程。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-325-MULTISTATE-PARENT

<a id="q-s00-cand-328-multistate-parent"></a>

### Q-S00-CAND-328-MULTISTATE-PARENT

- 问题：多态按钮候选U-148的分层问题？
- 来源/归属：CAND-20261110/U-148；多态按钮；C15
- 层/状态：D/E/P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：多态按钮候选U-148的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-329-MULTISTATE-LANGUAGE、Q-S00-CAND-330-MULTISTATE-RENDER、Q-S00-CAND-331-MULTISTATE-RUNTIME

<a id="q-s00-cand-329-multistate-language"></a>

### Q-S00-CAND-329-MULTISTATE-LANGUAGE

- 问题：多态按钮多语言列表及文本独立保存？
- 来源/归属：CAND-20261110/U-148；多态按钮；C15
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立中文单语及中文/English/第三种语言工程，给各状态各语言写不同文本。
- 刺激：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 对象：多态按钮多语言列表及文本独立保存。
- 预期观察：确认语言列表来源、索引及草稿规则；运行切换是否正确仍需独立E观察。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-328-MULTISTATE-PARENT

<a id="q-s00-cand-330-multistate-render"></a>

### Q-S00-CAND-330-MULTISTATE-RENDER

- 问题：多态按钮同文及闪烁图帧规则？
- 来源/归属：CAND-20261110/U-148；多态按钮；C15
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：多态按钮同文及闪烁图帧规则。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-328-MULTISTATE-PARENT

<a id="q-s00-cand-331-multistate-runtime"></a>

### Q-S00-CAND-331-MULTISTATE-RUNTIME

- 问题：多态按钮同文和闪烁的运行规则？
- 来源/归属：CAND-20261110/U-148；多态按钮；C15
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：多态按钮同文和闪烁的运行规则。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-328-MULTISTATE-PARENT

<a id="q-s00-cand-332-multistate-parent"></a>

### Q-S00-CAND-332-MULTISTATE-PARENT

- 问题：多态按钮候选U-149的分层问题？
- 来源/归属：CAND-20261110/U-149；多态按钮；C15
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：多态按钮候选U-149的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-333-MULTISTATE-OPTIONS、Q-S00-CAND-334-MULTISTATE-BOUND

<a id="q-s00-cand-333-multistate-options"></a>

### Q-S00-CAND-333-MULTISTATE-OPTIONS

- 问题：多态按钮对齐下拉的完整顺序？
- 来源/归属：CAND-20261110/U-149；多态按钮；C15
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：多态按钮对齐下拉的完整顺序。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-332-MULTISTATE-PARENT

<a id="q-s00-cand-334-multistate-bound"></a>

### Q-S00-CAND-334-MULTISTATE-BOUND

- 问题：多态按钮字号范围？
- 来源/归属：CAND-20261110/U-149；多态按钮；C15
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：多态按钮字号范围。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-332-MULTISTATE-PARENT

<a id="q-s00-cand-335-multistate-parent"></a>

### Q-S00-CAND-335-MULTISTATE-PARENT

- 问题：多态按钮候选U-150的分层问题？
- 来源/归属：CAND-20261110/U-150；多态按钮；C15
- 层/状态：D/E/P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：多态按钮候选U-150的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-336-MULTISTATE-BOUND、Q-S00-CAND-337-MULTISTATE-RENDER、Q-S00-CAND-338-MULTISTATE-RUNTIME

<a id="q-s00-cand-336-multistate-bound"></a>

### Q-S00-CAND-336-MULTISTATE-BOUND

- 问题：多态按钮透明度输入范围？
- 来源/归属：CAND-20261110/U-150；多态按钮；C15
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：多态按钮透明度输入范围。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-335-MULTISTATE-PARENT

<a id="q-s00-cand-337-multistate-render"></a>

### Q-S00-CAND-337-MULTISTATE-RENDER

- 问题：多态按钮各状态外形与文本色背景色的合成？
- 来源/归属：CAND-20261110/U-150；多态按钮；C15
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：多态按钮各状态外形与文本色背景色的合成。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-335-MULTISTATE-PARENT

<a id="q-s00-cand-338-multistate-runtime"></a>

### Q-S00-CAND-338-MULTISTATE-RUNTIME

- 问题：多态按钮各状态文本色与背景色的设备显示？
- 来源/归属：CAND-20261110/U-150；多态按钮；C15
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：多态按钮各状态文本色与背景色的设备显示。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-335-MULTISTATE-PARENT

<a id="q-s00-cand-339-multistate-parent"></a>

### Q-S00-CAND-339-MULTISTATE-PARENT

- 问题：多态按钮候选U-151的分层问题？
- 来源/归属：CAND-20261110/U-151；多态按钮；C15
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：多态按钮候选U-151的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-340-MULTISTATE-PERMISSION、Q-S00-CAND-341-MULTISTATE-THRESHOLD、Q-S00-CAND-342-MULTISTATE-THRESHOLD

<a id="q-s00-cand-340-multistate-permission"></a>

### Q-S00-CAND-340-MULTISTATE-PERMISSION

- 问题：多态按钮显现与触控多条件组合？
- 来源/归属：CAND-20261110/U-151；多态按钮；C15
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：选择该控件的显现或触控列，单独建立位、等级、字条件，且对象无重叠遮挡。
- 刺激：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 对象：多态按钮显现与触控多条件组合。
- 预期观察：取得显现与触控各自的真值表、组合方式及优先级；不由共用容器推断所有控件相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-339-MULTISTATE-PARENT

<a id="q-s00-cand-341-multistate-threshold"></a>

### Q-S00-CAND-341-MULTISTATE-THRESHOLD

- 问题：多态按钮最低等级等于阈值边界？
- 来源/归属：CAND-20261110/U-151；多态按钮；C15
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：固定该控件的最低等级阈值，位和字条件关闭或恒定满足。
- 刺激：分别以阈值-1、阈值、阈值+1等级观察显现或触控；显现和触控列分开记录。 对象：多态按钮最低等级等于阈值边界。
- 预期观察：取得最低等级的等值边界与实际权限结果，不由字段名称预设闭开关系。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：分别以阈值-1、阈值、阈值+1等级观察显现或触控；显现和触控列分开记录。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-339-MULTISTATE-PARENT

<a id="q-s00-cand-342-multistate-threshold"></a>

### Q-S00-CAND-342-MULTISTATE-THRESHOLD

- 问题：多态按钮比较误差算式？
- 来源/归属：CAND-20261110/U-151；多态按钮；C15
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：固定字比较值和误差，位和等级条件恒定满足；整数和浮点分别留样本。
- 刺激：测比较值±容差及邻界，分别测试等于、不等于及0误差，保持其他字段不变。 对象：多态按钮比较误差算式。
- 预期观察：取得容差闭开区间、绝对/相对误差及浮点边界，不提前假设具体公式。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：测比较值±容差及邻界，分别测试等于、不等于及0误差，保持其他字段不变。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-339-MULTISTATE-PARENT

<a id="q-s00-cand-343-multistate-parent"></a>

### Q-S00-CAND-343-MULTISTATE-PARENT

- 问题：多态按钮候选U-152的分层问题？
- 来源/归属：CAND-20261110/U-152；多态按钮；C15
- 层/状态：D/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：多态按钮候选U-152的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-344-MULTISTATE-SCOPE、Q-S00-CAND-345-MULTISTATE-RUNTIME、Q-S00-CAND-346-MULTISTATE-SOURCE

<a id="q-s00-cand-344-multistate-scope"></a>

### Q-S00-CAND-344-MULTISTATE-SCOPE

- 问题：多态按钮位字通知互斥的能力适用边界？
- 来源/归属：CAND-20261110/U-152；多态按钮；C15
- 层/状态：D；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：并列原始候选、原帮助及当前V5.1直接证据，保留版本/型号和文档章节。
- 刺激：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 对象：多态按钮位字通知互斥的能力适用边界。
- 预期观察：形成可追溯的来源与采用边界；说明句或旧图不自动成为待实现功能或运行算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-343-MULTISTATE-PARENT

<a id="q-s00-cand-345-multistate-runtime"></a>

### Q-S00-CAND-345-MULTISTATE-RUNTIME

- 问题：多态按钮通知写值的按下松开时序？
- 来源/归属：CAND-20261110/U-152；多态按钮；C15
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：多态按钮通知写值的按下松开时序。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-343-MULTISTATE-PARENT

<a id="q-s00-cand-346-multistate-source"></a>

### Q-S00-CAND-346-MULTISTATE-SOURCE

- 问题：多态按钮位字通知二选一的源保存布局？
- 来源/归属：CAND-20261110/U-152；多态按钮；C15
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：多态按钮位字通知二选一的源保存布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-343-MULTISTATE-PARENT

<a id="q-s00-cand-347-multistate-visibility"></a>

### Q-S00-CAND-347-MULTISTATE-VISIBILITY

- 问题：多态按钮云端上传在当前型号是否可达？
- 来源/归属：CAND-20261110/U-153；多态按钮；C15
- 层/状态：D/U；未闭合；atomic
- 适用范围：当前V5.1/AMT已观察的隐藏或缺席状态保留；新增其他机型/版本出现条件转能力矩阵扩展，不阻塞当前已核AMT切片。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定当前AMT和V5.1；B/H隐藏字段只作线索，另列型号或版本扩展范围。
- 刺激：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 对象：多态按钮云端上传在当前型号是否可达。
- 预期观察：只对搜索边界报告可达、隐藏或未找到；不因字符串存在增加字段，也不推断所有型号不存在。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-348-multistate-parent"></a>

### Q-S00-CAND-348-MULTISTATE-PARENT

- 问题：多态按钮候选U-154的分层问题？
- 来源/归属：CAND-20261110/U-154；多态按钮；C15
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：多态按钮候选U-154的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-349-MULTISTATE-SOURCE、Q-S00-CAND-350-MULTISTATE-COMPILE、Q-S00-CAND-351-MULTISTATE-RUNTIME

<a id="q-s00-cand-349-multistate-source"></a>

### Q-S00-CAND-349-MULTISTATE-SOURCE

- 问题：多态按钮各操作与状态的sm布局？
- 来源/归属：CAND-20261110/U-154；多态按钮；C15
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：多态按钮各操作与状态的sm布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-348-MULTISTATE-PARENT

<a id="q-s00-cand-350-multistate-compile"></a>

### Q-S00-CAND-350-MULTISTATE-COMPILE

- 问题：多态按钮设备记录及图像布局？
- 来源/归属：CAND-20261110/U-154；多态按钮；C15
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：多态按钮设备记录及图像布局。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-348-MULTISTATE-PARENT

<a id="q-s00-cand-351-multistate-runtime"></a>

### Q-S00-CAND-351-MULTISTATE-RUNTIME

- 问题：固件消费多态按钮记录的兼容性？
- 来源/归属：CAND-20261110/U-154；多态按钮；C15
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件消费多态按钮记录的兼容性。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-348-MULTISTATE-PARENT

<a id="q-s00-cand-352-multistate-visibility"></a>

### Q-S00-CAND-352-MULTISTATE-VISIBILITY

- 问题：转交指示灯模块的条件及自定义值字段在当前V5.1是否可达？
- 来源/归属：CAND-20261110/U-155；C04指示灯，原多态按钮手册范围转交；C15
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定当前AMT和V5.1；B/H隐藏字段只作线索，另列型号或版本扩展范围。
- 刺激：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 对象：转交指示灯模块的条件及自定义值字段在当前V5.1是否可达。
- 预期观察：只对搜索边界报告可达、隐藏或未找到；不因字符串存在增加字段，也不推断所有型号不存在。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MULTISTATE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-353-word-parent"></a>

### Q-S00-CAND-353-WORD-PARENT

- 问题：字按钮候选U-156的分层问题？
- 来源/归属：CAND-20261110/U-156；字按钮；C03
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：字按钮候选U-156的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；字按钮报告已有宏编辑绑定和真实SDK编译；旧合成点击失败不能推定宏禁用。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-354-WORD-UI、Q-S00-CAND-355-WORD-UI

<a id="q-s00-cand-354-word-ui"></a>

### Q-S00-CAND-354-WORD-UI

- 问题：字按钮宏组真实enabled与宏绑定列表？
- 来源/归属：CAND-20261110/U-156；字按钮；C03
- 层/状态：D/U；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：字按钮宏组真实enabled与宏绑定列表。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；字按钮报告已有宏编辑绑定和真实SDK编译；旧合成点击失败不能推定宏禁用。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-353-WORD-PARENT

<a id="q-s00-cand-355-word-ui"></a>

### Q-S00-CAND-355-WORD-UI

- 问题：字按钮宏编辑器模板和保存流程？
- 来源/归属：CAND-20261110/U-156；字按钮；C03
- 层/状态：D/U；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：字按钮宏编辑器模板和保存流程。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；字按钮报告已有宏编辑绑定和真实SDK编译；旧合成点击失败不能推定宏禁用。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-353-WORD-PARENT

<a id="q-s00-cand-356-word-parent"></a>

### Q-S00-CAND-356-WORD-PARENT

- 问题：字按钮候选U-157的分层问题？
- 来源/归属：CAND-20261110/U-157；字按钮；C03
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：字按钮候选U-157的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；字报告及Qt测试覆盖复制与state1禁用；原厂逐字段复制采用范围需核对原证据，不能把测试实现当原生事实。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-357-WORD-UI、Q-S00-CAND-358-WORD-UI、Q-S00-CAND-359-WORD-UI

<a id="q-s00-cand-357-word-ui"></a>

### Q-S00-CAND-357-WORD-UI

- 问题：字按钮从另一状态复制文字逐字段结果？
- 来源/归属：CAND-20261110/U-157；字按钮；C03
- 层/状态：D/U；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：字按钮从另一状态复制文字逐字段结果。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；字报告及Qt测试覆盖复制与state1禁用；原厂逐字段复制采用范围需核对原证据，不能把测试实现当原生事实。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-356-WORD-PARENT

<a id="q-s00-cand-358-word-ui"></a>

### Q-S00-CAND-358-WORD-UI

- 问题：字按钮从另一状态复制外形逐字段结果？
- 来源/归属：CAND-20261110/U-157；字按钮；C03
- 层/状态：D/U；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：字按钮从另一状态复制外形逐字段结果。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；字报告及Qt测试覆盖复制与state1禁用；原厂逐字段复制采用范围需核对原证据，不能把测试实现当原生事实。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-356-WORD-PARENT

<a id="q-s00-cand-359-word-ui"></a>

### Q-S00-CAND-359-WORD-UI

- 问题：字按钮状态1复制按钮灰显的真实enabled？
- 来源/归属：CAND-20261110/U-157；字按钮；C03
- 层/状态：D/U；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：字按钮状态1复制按钮灰显的真实enabled。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；字报告及Qt测试覆盖复制与state1禁用；原厂逐字段复制采用范围需核对原证据，不能把测试实现当原生事实。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-356-WORD-PARENT

<a id="q-s00-cand-360-word-parent"></a>

### Q-S00-CAND-360-WORD-PARENT

- 问题：字按钮候选U-158的分层问题？
- 来源/归属：CAND-20261110/U-158；字按钮；C03
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：字按钮候选U-158的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；字报告已有通知字五类型和位项禁用规则；取消再开启的原生草稿恢复仍应逐路径核验。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-361-WORD-OPTIONS、Q-S00-CAND-362-WORD-PERSIST

<a id="q-s00-cand-361-word-options"></a>

### Q-S00-CAND-361-WORD-OPTIONS

- 问题：字按钮通知字数据类型完整项集？
- 来源/归属：CAND-20261110/U-158；字按钮；C03
- 层/状态：D/U；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：字按钮通知字数据类型完整项集。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；字报告已有通知字五类型和位项禁用规则；取消再开启的原生草稿恢复仍应逐路径核验。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-360-WORD-PARENT

<a id="q-s00-cand-362-word-persist"></a>

### Q-S00-CAND-362-WORD-PERSIST

- 问题：字按钮取消再启用通知后的互斥选择及字段草稿？
- 来源/归属：CAND-20261110/U-158；字按钮；C03
- 层/状态：U/P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留可辨识原值和工程副本，固定当前语言、状态、类型及对象ID。
- 刺激：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 对象：字按钮取消再启用通知后的互斥选择及字段草稿。
- 预期观察：取得提交、取消和休眠草稿的保存寿命；区分ksp草稿与原厂sm实际保存字段。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；字报告已有通知字五类型和位项禁用规则；取消再开启的原生草稿恢复仍应逐路径核验。
- 下一探针：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-360-WORD-PARENT

<a id="q-s00-cand-363-word-persist"></a>

### Q-S00-CAND-363-WORD-PERSIST

- 问题：字按钮关闭按下效果再开启是否保留隐藏状态1草稿？
- 来源/归属：CAND-20261110/U-159；字按钮；C03
- 层/状态：U/P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留可辨识原值和工程副本，固定当前语言、状态、类型及对象ID。
- 刺激：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 对象：字按钮关闭按下效果再开启是否保留隐藏状态1草稿。
- 预期观察：取得提交、取消和休眠草稿的保存寿命；区分ksp草稿与原厂sm实际保存字段。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；ksp保留两态草稿且sm单态列表不能保存休眠另一态已解释；原厂交互路径与Lite保存声明分开。
- 下一探针：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-364-word-parent"></a>

### Q-S00-CAND-364-WORD-PARENT

- 问题：字按钮候选U-160的分层问题？
- 来源/归属：CAND-20261110/U-160；字按钮；C03
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：字按钮候选U-160的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-365-WORD-RUNTIME、Q-S00-CAND-366-WORD-RUNTIME、Q-S00-CAND-367-WORD-RUNTIME、Q-S00-CAND-368-WORD-RUNTIME、Q-S00-CAND-369-WORD-RUNTIME、Q-S00-CAND-370-WORD-RUNTIME、Q-S00-CAND-371-WORD-RUNTIME

<a id="q-s00-cand-365-word-runtime"></a>

### Q-S00-CAND-365-WORD-RUNTIME

- 问题：字设置常数的触控写入时机？
- 来源/归属：CAND-20261110/U-160；字按钮；C03
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：字设置常数的触控写入时机。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-364-WORD-PARENT

<a id="q-s00-cand-366-word-runtime"></a>

### Q-S00-CAND-366-WORD-RUNTIME

- 问题：字数值输入的键盘、提交及取消？
- 来源/归属：CAND-20261110/U-160；字按钮；C03
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：字数值输入的键盘、提交及取消。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-364-WORD-PARENT

<a id="q-s00-cand-367-word-runtime"></a>

### Q-S00-CAND-367-WORD-RUNTIME

- 问题：字密码输入的遮蔽与提交？
- 来源/归属：CAND-20261110/U-160；字按钮；C03
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：字密码输入的遮蔽与提交。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-364-WORD-PARENT

<a id="q-s00-cand-368-word-runtime"></a>

### Q-S00-CAND-368-WORD-RUNTIME

- 问题：字数值加的数值边界及触发次数？
- 来源/归属：CAND-20261110/U-160；字按钮；C03
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：字数值加的数值边界及触发次数。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-364-WORD-PARENT

<a id="q-s00-cand-369-word-runtime"></a>

### Q-S00-CAND-369-WORD-RUNTIME

- 问题：字数值减的数值边界及触发次数？
- 来源/归属：CAND-20261110/U-160；字按钮；C03
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：字数值减的数值边界及触发次数。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-364-WORD-PARENT

<a id="q-s00-cand-370-word-runtime"></a>

### Q-S00-CAND-370-WORD-RUNTIME

- 问题：字递增绕回在上限后的结果？
- 来源/归属：CAND-20261110/U-160；字按钮；C03
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：字递增绕回在上限后的结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-364-WORD-PARENT

<a id="q-s00-cand-371-word-runtime"></a>

### Q-S00-CAND-371-WORD-RUNTIME

- 问题：字递减绕回在下限后的结果？
- 来源/归属：CAND-20261110/U-160；字按钮；C03
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：字递减绕回在下限后的结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-364-WORD-PARENT

<a id="q-s00-cand-372-word-parent"></a>

### Q-S00-CAND-372-WORD-PARENT

- 问题：字按钮候选U-161的分层问题？
- 来源/归属：CAND-20261110/U-161；字按钮；C03
- 层/状态：D/P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：字按钮候选U-161的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；实现采用可表示数值域和保护范围不等于原厂全域输入上限；透明和全部渐变仍有边界。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-373-WORD-BOUND、Q-S00-CAND-374-WORD-BOUND、Q-S00-CAND-375-WORD-RENDER

<a id="q-s00-cand-373-word-bound"></a>

### Q-S00-CAND-373-WORD-BOUND

- 问题：字按钮32位最大值能否编辑到数据类型全域？
- 来源/归属：CAND-20261110/U-161；字按钮；C03
- 层/状态：D/U/P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：字按钮32位最大值能否编辑到数据类型全域。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；实现采用可表示数值域和保护范围不等于原厂全域输入上限；透明和全部渐变仍有边界。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-372-WORD-PARENT

<a id="q-s00-cand-374-word-bound"></a>

### Q-S00-CAND-374-WORD-BOUND

- 问题：字按钮透明度原生范围？
- 来源/归属：CAND-20261110/U-161；字按钮；C03
- 层/状态：D/U/P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：字按钮透明度原生范围。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；实现采用可表示数值域和保护范围不等于原厂全域输入上限；透明和全部渐变仍有边界。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-372-WORD-PARENT

<a id="q-s00-cand-375-word-render"></a>

### Q-S00-CAND-375-WORD-RENDER

- 问题：字按钮OFF与ON双态透明合成？
- 来源/归属：CAND-20261110/U-161；字按钮；C03
- 层/状态：R；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：字按钮OFF与ON双态透明合成。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；实现采用可表示数值域和保护范围不等于原厂全域输入上限；透明和全部渐变仍有边界。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-372-WORD-PARENT

<a id="q-s00-cand-376-word-parent"></a>

### Q-S00-CAND-376-WORD-PARENT

- 问题：字按钮候选U-162的分层问题？
- 来源/归属：CAND-20261110/U-162；字按钮；C03
- 层/状态：D/E/P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：字按钮候选U-162的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；字按钮语言和闪烁编译有历史局部证据，运行切换和周期不由C推出。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-377-WORD-LANGUAGE、Q-S00-CAND-378-WORD-RENDER、Q-S00-CAND-379-WORD-RUNTIME

<a id="q-s00-cand-377-word-language"></a>

### Q-S00-CAND-377-WORD-LANGUAGE

- 问题：字按钮多语言下拉和独立文本保存？
- 来源/归属：CAND-20261110/U-162；字按钮；C03
- 层/状态：D/U/P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立中文单语及中文/English/第三种语言工程，给各状态各语言写不同文本。
- 刺激：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 对象：字按钮多语言下拉和独立文本保存。
- 预期观察：确认语言列表来源、索引及草稿规则；运行切换是否正确仍需独立E观察。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；字按钮语言和闪烁编译有历史局部证据，运行切换和周期不由C推出。
- 下一探针：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-376-WORD-PARENT

<a id="q-s00-cand-378-word-render"></a>

### Q-S00-CAND-378-WORD-RENDER

- 问题：字按钮同文和闪烁图帧含义？
- 来源/归属：CAND-20261110/U-162；字按钮；C03
- 层/状态：R；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：字按钮同文和闪烁图帧含义。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；字按钮语言和闪烁编译有历史局部证据，运行切换和周期不由C推出。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-376-WORD-PARENT

<a id="q-s00-cand-379-word-runtime"></a>

### Q-S00-CAND-379-WORD-RUNTIME

- 问题：字按钮运行语言切换及闪烁周期？
- 来源/归属：CAND-20261110/U-162；字按钮；C03
- 层/状态：E；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：字按钮运行语言切换及闪烁周期。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；字按钮语言和闪烁编译有历史局部证据，运行切换和周期不由C推出。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-376-WORD-PARENT

<a id="q-s00-cand-380-word-parent"></a>

### Q-S00-CAND-380-WORD-PARENT

- 问题：字按钮候选U-163的分层问题？
- 来源/归属：CAND-20261110/U-163；字按钮；C03
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：字按钮候选U-163的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-381-WORD-OPTIONS、Q-S00-CAND-382-WORD-BOUND

<a id="q-s00-cand-381-word-options"></a>

### Q-S00-CAND-381-WORD-OPTIONS

- 问题：字按钮对齐完整选项顺序？
- 来源/归属：CAND-20261110/U-163；字按钮；C03
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：字按钮对齐完整选项顺序。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-380-WORD-PARENT

<a id="q-s00-cand-382-word-bound"></a>

### Q-S00-CAND-382-WORD-BOUND

- 问题：字按钮原生字体大小范围？
- 来源/归属：CAND-20261110/U-163；字按钮；C03
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：字按钮原生字体大小范围。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-380-WORD-PARENT

<a id="q-s00-cand-383-word-parent"></a>

### Q-S00-CAND-383-WORD-PARENT

- 问题：字按钮候选U-164的分层问题？
- 来源/归属：CAND-20261110/U-164；字按钮；C03
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：字按钮候选U-164的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-384-WORD-PERMISSION、Q-S00-CAND-385-WORD-THRESHOLD、Q-S00-CAND-386-WORD-THRESHOLD

<a id="q-s00-cand-384-word-permission"></a>

### Q-S00-CAND-384-WORD-PERMISSION

- 问题：字按钮显现与触控多条件组合？
- 来源/归属：CAND-20261110/U-164；字按钮；C03
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：选择该控件的显现或触控列，单独建立位、等级、字条件，且对象无重叠遮挡。
- 刺激：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 对象：字按钮显现与触控多条件组合。
- 预期观察：取得显现与触控各自的真值表、组合方式及优先级；不由共用容器推断所有控件相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-383-WORD-PARENT

<a id="q-s00-cand-385-word-threshold"></a>

### Q-S00-CAND-385-WORD-THRESHOLD

- 问题：字按钮最低等级等于阈值边界？
- 来源/归属：CAND-20261110/U-164；字按钮；C03
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：固定该控件的最低等级阈值，位和字条件关闭或恒定满足。
- 刺激：分别以阈值-1、阈值、阈值+1等级观察显现或触控；显现和触控列分开记录。 对象：字按钮最低等级等于阈值边界。
- 预期观察：取得最低等级的等值边界与实际权限结果，不由字段名称预设闭开关系。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：分别以阈值-1、阈值、阈值+1等级观察显现或触控；显现和触控列分开记录。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-383-WORD-PARENT

<a id="q-s00-cand-386-word-threshold"></a>

### Q-S00-CAND-386-WORD-THRESHOLD

- 问题：字按钮比较误差算式？
- 来源/归属：CAND-20261110/U-164；字按钮；C03
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：固定字比较值和误差，位和等级条件恒定满足；整数和浮点分别留样本。
- 刺激：测比较值±容差及邻界，分别测试等于、不等于及0误差，保持其他字段不变。 对象：字按钮比较误差算式。
- 预期观察：取得容差闭开区间、绝对/相对误差及浮点边界，不提前假设具体公式。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：测比较值±容差及邻界，分别测试等于、不等于及0误差，保持其他字段不变。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-383-WORD-PARENT

<a id="q-s00-cand-387-word-parent"></a>

### Q-S00-CAND-387-WORD-PARENT

- 问题：字按钮候选U-165的分层问题？
- 来源/归属：CAND-20261110/U-165；字按钮；C03
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：字按钮候选U-165的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-388-WORD-VISIBILITY、Q-S00-CAND-389-WORD-SCOPE

<a id="q-s00-cand-388-word-visibility"></a>

### Q-S00-CAND-388-WORD-VISIBILITY

- 问题：字按钮旧标题与云端上传在当前型号是否有入口？
- 来源/归属：CAND-20261110/U-165；字按钮；C03
- 层/状态：D/U；未闭合；atomic
- 适用范围：当前V5.1/AMT已观察的隐藏或缺席状态保留；新增其他机型/版本出现条件转能力矩阵扩展，不阻塞当前已核AMT切片。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定当前AMT和V5.1；B/H隐藏字段只作线索，另列型号或版本扩展范围。
- 刺激：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 对象：字按钮旧标题与云端上传在当前型号是否有入口。
- 预期观察：只对搜索边界报告可达、隐藏或未找到；不因字符串存在增加字段，也不推断所有型号不存在。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-387-WORD-PARENT

<a id="q-s00-cand-389-word-scope"></a>

### Q-S00-CAND-389-WORD-SCOPE

- 问题：密码总位数帮助4与V5.1默认5的采用边界？
- 来源/归属：CAND-20261110/U-165；字按钮；C03
- 层/状态：D；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：并列原始候选、原帮助及当前V5.1直接证据，保留版本/型号和文档章节。
- 刺激：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 对象：密码总位数帮助4与V5.1默认5的采用边界。
- 预期观察：形成可追溯的来源与采用边界；说明句或旧图不自动成为待实现功能或运行算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-387-WORD-PARENT

<a id="q-s00-cand-390-word-visibility"></a>

### Q-S00-CAND-390-WORD-VISIBILITY

- 问题：字按钮空白指示灯页在其他配置或型号是否会出现内容？
- 来源/归属：CAND-20261110/U-166；字按钮；C03
- 层/状态：D/U；未闭合；atomic
- 适用范围：当前V5.1/AMT已观察的隐藏或缺席状态保留；新增其他机型/版本出现条件转能力矩阵扩展，不阻塞当前已核AMT切片。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定当前AMT和V5.1；B/H隐藏字段只作线索，另列型号或版本扩展范围。
- 刺激：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 对象：字按钮空白指示灯页在其他配置或型号是否会出现内容。
- 预期观察：只对搜索边界报告可达、隐藏或未找到；不因字符串存在增加字段，也不推断所有型号不存在。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-391-word-parent"></a>

### Q-S00-CAND-391-WORD-PARENT

- 问题：字按钮候选U-167的分层问题？
- 来源/归属：CAND-20261110/U-167；字按钮；C03
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：字按钮候选U-167的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；49种原厂字记录活动字段及源往返已有P/C；休眠字段、残留及E动作未等同。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-392-WORD-SOURCE、Q-S00-CAND-393-WORD-COMPILE、Q-S00-CAND-394-WORD-RUNTIME

<a id="q-s00-cand-392-word-source"></a>

### Q-S00-CAND-392-WORD-SOURCE

- 问题：字按钮原厂源操作union及状态列表？
- 来源/归属：CAND-20261110/U-167；字按钮；C03
- 层/状态：P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：字按钮原厂源操作union及状态列表。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；49种原厂字记录活动字段及源往返已有P/C；休眠字段、残留及E动作未等同。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-391-WORD-PARENT

<a id="q-s00-cand-393-word-compile"></a>

### Q-S00-CAND-393-WORD-COMPILE

- 问题：字按钮七类型七操作设备记录及地址跨度？
- 来源/归属：CAND-20261110/U-167；字按钮；C03
- 层/状态：C；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：字按钮七类型七操作设备记录及地址跨度。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；49种原厂字记录活动字段及源往返已有P/C；休眠字段、残留及E动作未等同。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-391-WORD-PARENT

<a id="q-s00-cand-394-word-runtime"></a>

### Q-S00-CAND-394-WORD-RUNTIME

- 问题：固件消费字按钮记录后的键盘与数值兼容性？
- 来源/归属：CAND-20261110/U-167；字按钮；C03
- 层/状态：E；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件消费字按钮记录后的键盘与数值兼容性。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WORD、EV-S00-CAND-WORD；49种原厂字记录活动字段及源往返已有P/C；休眠字段、残留及E动作未等同。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-391-WORD-PARENT

<a id="q-s00-cand-395-alarm-ui"></a>

### Q-S00-CAND-395-ALARM-UI

- 问题：报警显示在无报警配置工程中的入口拦截提示？
- 来源/归属：CAND-20261110/U-168、CAND-20261110/U-085；报警显示；C28
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：报警显示在无报警配置工程中的入口拦截提示。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-396-alarm-options"></a>

### Q-S00-CAND-396-ALARM-OPTIONS

- 问题：报警显示大小下拉超过12、16后的完整选项？
- 来源/归属：CAND-20261110/U-169；报警显示；C28
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：报警显示大小下拉超过12、16后的完整选项。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-397-alarm-diagnostic"></a>

### Q-S00-CAND-397-ALARM-DIAGNOSTIC

- 问题：报警显示行数输入0后窗口消失是否为可复现崩溃？
- 来源/归属：CAND-20261110/U-170；报警显示；C28
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留发生问题的源工程、原生身份、私有桌面PID、事件类型和退出码。
- 刺激：仅在自有实例串行最小复现，区分直接点击、Qt等价事件及重试；核对已引用截图来自哪次成功会话。 对象：报警显示行数输入0后窗口消失是否为可复现崩溃。
- 预期观察：确定采集可复现性或留下有界异常；未证实原因不得写成试用状态或其他程序冲突，更不要求复刻崩溃。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有实例串行最小复现，区分直接点击、Qt等价事件及重试；核对已引用截图来自哪次成功会话。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-398-alarm-runtime"></a>

### Q-S00-CAND-398-ALARM-RUNTIME

- 问题：报警显示的确认按钮或确认位在设备端如何执行？
- 来源/归属：CAND-20261110/U-171；报警显示；C28
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：报警显示的确认按钮或确认位在设备端如何执行。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-399-alarm-runtime"></a>

### Q-S00-CAND-399-ALARM-RUNTIME

- 问题：V5.1无编辑字段的报警历史时间及类别查询应由何处触发？
- 来源/归属：CAND-20261110/U-172；报警显示；C28
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：V5.1无编辑字段的报警历史时间及类别查询应由何处触发。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-400-alarm-parent"></a>

### Q-S00-CAND-400-ALARM-PARENT

- 问题：报警显示候选U-173的分层问题？
- 来源/归属：CAND-20261110/U-173；报警显示；C28
- 层/状态：D/E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：报警显示候选U-173的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-401-ALARM-SCOPE、Q-S00-CAND-402-ALARM-RUNTIME

<a id="q-s00-cand-401-alarm-scope"></a>

### Q-S00-CAND-401-ALARM-SCOPE

- 问题：报警类别过滤不在显示器字段的配置归属？
- 来源/归属：CAND-20261110/U-173；报警显示；C28
- 层/状态：D；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：并列原始候选、原帮助及当前V5.1直接证据，保留版本/型号和文档章节。
- 刺激：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 对象：报警类别过滤不在显示器字段的配置归属。
- 预期观察：形成可追溯的来源与采用边界；说明句或旧图不自动成为待实现功能或运行算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-400-ALARM-PARENT

<a id="q-s00-cand-402-alarm-runtime"></a>

### Q-S00-CAND-402-ALARM-RUNTIME

- 问题：报警显示表格列顺序的设备渲染？
- 来源/归属：CAND-20261110/U-173；报警显示；C28
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：报警显示表格列顺序的设备渲染。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-400-ALARM-PARENT

<a id="q-s00-cand-403-alarm-parent"></a>

### Q-S00-CAND-403-ALARM-PARENT

- 问题：报警显示候选U-174的分层问题？
- 来源/归属：CAND-20261110/U-174；报警显示；C28
- 层/状态：D/E/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：报警显示候选U-174的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-404-ALARM-LANGUAGE、Q-S00-CAND-405-ALARM-RUNTIME

<a id="q-s00-cand-404-alarm-language"></a>

### Q-S00-CAND-404-ALARM-LANGUAGE

- 问题：报警显示多语言条目和列名编辑？
- 来源/归属：CAND-20261110/U-174；报警显示；C28
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立中文单语及中文/English/第三种语言工程，给各状态各语言写不同文本。
- 刺激：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 对象：报警显示多语言条目和列名编辑。
- 预期观察：确认语言列表来源、索引及草稿规则；运行切换是否正确仍需独立E观察。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-403-ALARM-PARENT

<a id="q-s00-cand-405-alarm-runtime"></a>

### Q-S00-CAND-405-ALARM-RUNTIME

- 问题：报警显示消息多语言在运行切换时的显示？
- 来源/归属：CAND-20261110/U-174；报警显示；C28
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：报警显示消息多语言在运行切换时的显示。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-403-ALARM-PARENT

<a id="q-s00-cand-406-alarm-parent"></a>

### Q-S00-CAND-406-ALARM-PARENT

- 问题：报警显示候选U-175的分层问题？
- 来源/归属：CAND-20261110/U-175；报警显示；C28
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：报警显示候选U-175的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-407-ALARM-SOURCE、Q-S00-CAND-408-ALARM-COMPILE、Q-S00-CAND-409-ALARM-RUNTIME

<a id="q-s00-cand-407-alarm-source"></a>

### Q-S00-CAND-407-ALARM-SOURCE

- 问题：报警显示源对象及报警配置引用？
- 来源/归属：CAND-20261110/U-175；报警显示；C28
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：报警显示源对象及报警配置引用。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-406-ALARM-PARENT

<a id="q-s00-cand-408-alarm-compile"></a>

### Q-S00-CAND-408-ALARM-COMPILE

- 问题：报警显示下载记录布局？
- 来源/归属：CAND-20261110/U-175；报警显示；C28
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：报警显示下载记录布局。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-406-ALARM-PARENT

<a id="q-s00-cand-409-alarm-runtime"></a>

### Q-S00-CAND-409-ALARM-RUNTIME

- 问题：固件消费报警显示记录的兼容性？
- 来源/归属：CAND-20261110/U-175；报警显示；C28
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件消费报警显示记录的兼容性。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-ALARM；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-406-ALARM-PARENT

<a id="q-s00-cand-410-lamp-ui"></a>

### Q-S00-CAND-410-LAMP-UI

- 问题：指示灯两处与操作地址相同复选框的真实enabled？
- 来源/归属：CAND-20261110/U-176；指示灯；C04
- 层/状态：D/U；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：指示灯两处与操作地址相同复选框的真实enabled。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；指示灯报告原厂补证第1项：两处同地址复选框均disabled，不能沿旧手册要求开放同步。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-411-lamp-ui"></a>

### Q-S00-CAND-411-LAMP-UI

- 问题：指示灯通知整个组及子项的真实enabled？
- 来源/归属：CAND-20261110/U-177；指示灯；C04
- 层/状态：D/U；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：指示灯通知整个组及子项的真实enabled。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；指示灯报告原厂补证第1项：通知整个组disabled；不继续实现不可用通知展开。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-412-lamp-fact"></a>

### Q-S00-CAND-412-LAMP-FACT

- 问题：所核V5.1指示灯宏组能否启用？
- 来源/归属：CAND-20261110/U-178；指示灯；C04
- 层/状态：D/U；沿用事实关闭；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：复用旧核销中相同V5.1身份及AMT范围。
- 刺激：核对原核销引用与现存证据身份；本任务不重跑原生UI或硬件。 对象：所核V5.1指示灯宏组能否启用。
- 预期观察：只沿用已关闭事实及其版本/型号/层，不扩大为整个控件或E通过。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP、EV-S00-CAND-PRIOR；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：核对原核销引用与现存证据身份；本任务不重跑原生UI或硬件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 沿用问题身份：Q-LAMP-MACRO-001
- 关闭范围：{"basis": "旧核销原有关闭，不是本轮重新测试", "version": "V5.1，同哈希私有运行副本的所核AMT范围", "layers": ["D", "U"], "conclusion": "宏组真实Qt enabled=false；使用宏/写宏没有可用交互路径。", "not_covered": "其他控件宏、其它机型、整控件通过及E。"}

<a id="q-s00-cand-413-lamp-help"></a>

### Q-S00-CAND-413-LAMP-HELP

- 问题：指示灯帮助按钮无窗口时是否调用CHM及哪一成员？
- 来源/归属：CAND-20261110/U-179；指示灯；C04
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：确认原CHM路径/哈希及成员存在，记录完整安装依赖和私有副本环境差异。
- 刺激：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 对象：指示灯帮助按钮无窗口时是否调用CHM及哪一成员。
- 预期观察：确认目标成员或有界失败；CHM成员存在不能替代按钮直达事实。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-414-lamp-options"></a>

### Q-S00-CAND-414-LAMP-OPTIONS

- 问题：指示灯多态数据类型在当前AMT是否只有U16与U32？
- 来源/归属：CAND-20261110/U-180；指示灯；C04
- 层/状态：D/U；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：指示灯多态数据类型在当前AMT是否只有U16与U32。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；当前AMT实际多态监视已恢复U16/U32；同SHA当前范围直接复用，不能据此说所有型号只有两项。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-415-lamp-parent"></a>

### Q-S00-CAND-415-LAMP-PARENT

- 问题：指示灯候选U-182的分层问题？
- 来源/归属：CAND-20261110/U-182；指示灯；C04
- 层/状态：E/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：指示灯候选U-182的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；报告第3项解释新建15/存储0及切字监视可存15；真实位值取值另验。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-416-LAMP-PERSIST、Q-S00-CAND-417-LAMP-RUNTIME

<a id="q-s00-cand-416-lamp-persist"></a>

### Q-S00-CAND-416-LAMP-PERSIST

- 问题：指示灯新建显示15但存储回显0的字位草稿规则？
- 来源/归属：CAND-20261110/U-182；指示灯；C04
- 层/状态：U/P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留可辨识原值和工程副本，固定当前语言、状态、类型及对象ID。
- 刺激：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 对象：指示灯新建显示15但存储回显0的字位草稿规则。
- 预期观察：取得提交、取消和休眠草稿的保存寿命；区分ksp草稿与原厂sm实际保存字段。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；报告第3项解释新建15/存储0及切字监视可存15；真实位值取值另验。
- 下一探针：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-415-LAMP-PARENT

<a id="q-s00-cand-417-lamp-runtime"></a>

### Q-S00-CAND-417-LAMP-RUNTIME

- 问题：指示灯字位0至15与监视字实际位值映射？
- 来源/归属：CAND-20261110/U-182；指示灯；C04
- 层/状态：E；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：指示灯字位0至15与监视字实际位值映射。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；报告第3项解释新建15/存储0及切字监视可存15；真实位值取值另验。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-415-LAMP-PARENT

<a id="q-s00-cand-418-lamp-parent"></a>

### Q-S00-CAND-418-LAMP-PARENT

- 问题：指示灯候选U-183的分层问题？
- 来源/归属：CAND-20261110/U-183；指示灯；C04
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：指示灯候选U-183的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；报告第4及9项已有图序、同文图数；屏端显示与权限不以其代替。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-419-LAMP-RUNTIME、Q-S00-CAND-420-LAMP-RUNTIME、Q-S00-CAND-421-LAMP-PERMISSION

<a id="q-s00-cand-419-lamp-runtime"></a>

### Q-S00-CAND-419-LAMP-RUNTIME

- 问题：指示灯双态ON与OFF监视值的屏端显示？
- 来源/归属：CAND-20261110/U-183；指示灯；C04
- 层/状态：E；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：指示灯双态ON与OFF监视值的屏端显示。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；报告第4及9项已有图序、同文图数；屏端显示与权限不以其代替。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-418-LAMP-PARENT

<a id="q-s00-cand-420-lamp-runtime"></a>

### Q-S00-CAND-420-LAMP-RUNTIME

- 问题：指示灯多态监视值越界和各状态的屏端显示？
- 来源/归属：CAND-20261110/U-183；指示灯；C04
- 层/状态：E；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：指示灯多态监视值越界和各状态的屏端显示。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；报告第4及9项已有图序、同文图数；屏端显示与权限不以其代替。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-418-LAMP-PARENT

<a id="q-s00-cand-421-lamp-permission"></a>

### Q-S00-CAND-421-LAMP-PERMISSION

- 问题：指示灯显现权限的多条件运行逻辑？
- 来源/归属：CAND-20261110/U-183；指示灯；C04
- 层/状态：E；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：选择该控件的显现或触控列，单独建立位、等级、字条件，且对象无重叠遮挡。
- 刺激：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 对象：指示灯显现权限的多条件运行逻辑。
- 预期观察：取得显现与触控各自的真值表、组合方式及优先级；不由共用容器推断所有控件相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；报告第4及9项已有图序、同文图数；屏端显示与权限不以其代替。
- 下一探针：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-418-LAMP-PARENT

<a id="q-s00-cand-422-lamp-parent"></a>

### Q-S00-CAND-422-LAMP-PARENT

- 问题：指示灯候选U-184的分层问题？
- 来源/归属：CAND-20261110/U-184；指示灯；C04
- 层/状态：P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：指示灯候选U-184的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；报告第6、7项解释新增默认和显式删除的草稿寿命；3D实屏语义不扩展。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-423-LAMP-PERSIST、Q-S00-CAND-424-LAMP-RENDER

<a id="q-s00-cand-423-lamp-persist"></a>

### Q-S00-CAND-423-LAMP-PERSIST

- 问题：指示灯扩展和缩减状态时的外观草稿寿命？
- 来源/归属：CAND-20261110/U-184；指示灯；C04
- 层/状态：U/P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留可辨识原值和工程副本，固定当前语言、状态、类型及对象ID。
- 刺激：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 对象：指示灯扩展和缩减状态时的外观草稿寿命。
- 预期观察：取得提交、取消和休眠草稿的保存寿命；区分ksp草稿与原厂sm实际保存字段。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；报告第6、7项解释新增默认和显式删除的草稿寿命；3D实屏语义不扩展。
- 下一探针：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-422-LAMP-PARENT

<a id="q-s00-cand-424-lamp-render"></a>

### Q-S00-CAND-424-LAMP-RENDER

- 问题：指示灯3D勾选后背景色禁用与实际图像语义？
- 来源/归属：CAND-20261110/U-184；指示灯；C04
- 层/状态：R；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：指示灯3D勾选后背景色禁用与实际图像语义。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；报告第6、7项解释新增默认和显式删除的草稿寿命；3D实屏语义不扩展。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-422-LAMP-PARENT

<a id="q-s00-cand-425-lamp-parent"></a>

### Q-S00-CAND-425-LAMP-PARENT

- 问题：指示灯候选U-185的分层问题？
- 来源/归属：CAND-20261110/U-185；指示灯；C04
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：指示灯候选U-185的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-426-LAMP-OPTIONS、Q-S00-CAND-427-LAMP-OPTIONS、Q-S00-CAND-428-LAMP-UI

<a id="q-s00-cand-426-lamp-options"></a>

### Q-S00-CAND-426-LAMP-OPTIONS

- 问题：指示灯显现最低等级的完整序列？
- 来源/归属：CAND-20261110/U-185；指示灯；C04
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：指示灯显现最低等级的完整序列。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-425-LAMP-PARENT

<a id="q-s00-cand-427-lamp-options"></a>

### Q-S00-CAND-427-LAMP-OPTIONS

- 问题：指示灯字条件五种数据类型的完整序列？
- 来源/归属：CAND-20261110/U-185；指示灯；C04
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：指示灯字条件五种数据类型的完整序列。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-425-LAMP-PARENT

<a id="q-s00-cand-428-lamp-ui"></a>

### Q-S00-CAND-428-LAMP-UI

- 问题：指示灯等于和不等于的误差行显隐？
- 来源/归属：CAND-20261110/U-185；指示灯；C04
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：指示灯等于和不等于的误差行显隐。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-425-LAMP-PARENT

<a id="q-s00-cand-429-lamp-language"></a>

### Q-S00-CAND-429-LAMP-LANGUAGE

- 问题：指示灯语言下拉与工程语言数量及状态文本对应？
- 来源/归属：CAND-20261110/U-186；指示灯；C04
- 层/状态：D/U/P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立中文单语及中文/English/第三种语言工程，给各状态各语言写不同文本。
- 刺激：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 对象：指示灯语言下拉与工程语言数量及状态文本对应。
- 预期观察：确认语言列表来源、索引及草稿规则；运行切换是否正确仍需独立E观察。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；双语多态原厂复开编译和状态独立文字已有补证；完整语言配置变更矩阵仍保留。
- 下一探针：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-430-lamp-parent"></a>

### Q-S00-CAND-430-LAMP-PARENT

- 问题：指示灯候选U-187的分层问题？
- 来源/归属：CAND-20261110/U-187；指示灯；C04
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：指示灯候选U-187的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；Lamp typed源、kind1/3及subtype4图帧已补证；固件最终显示和权限仍未验。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-431-LAMP-SOURCE、Q-S00-CAND-432-LAMP-COMPILE、Q-S00-CAND-433-LAMP-RUNTIME

<a id="q-s00-cand-431-lamp-source"></a>

### Q-S00-CAND-431-LAMP-SOURCE

- 问题：指示灯原厂源监视、状态及无图结构？
- 来源/归属：CAND-20261110/U-187；指示灯；C04
- 层/状态：P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：指示灯原厂源监视、状态及无图结构。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；Lamp typed源、kind1/3及subtype4图帧已补证；固件最终显示和权限仍未验。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-430-LAMP-PARENT

<a id="q-s00-cand-432-lamp-compile"></a>

### Q-S00-CAND-432-LAMP-COMPILE

- 问题：指示灯设备记录、语言图帧和扫描布局？
- 来源/归属：CAND-20261110/U-187；指示灯；C04
- 层/状态：C；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：指示灯设备记录、语言图帧和扫描布局。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；Lamp typed源、kind1/3及subtype4图帧已补证；固件最终显示和权限仍未验。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-430-LAMP-PARENT

<a id="q-s00-cand-433-lamp-runtime"></a>

### Q-S00-CAND-433-LAMP-RUNTIME

- 问题：固件消费指示灯记录的兼容性？
- 来源/归属：CAND-20261110/U-187；指示灯；C04
- 层/状态：E；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件消费指示灯记录的兼容性。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；Lamp typed源、kind1/3及subtype4图帧已补证；固件最终显示和权限仍未验。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-430-LAMP-PARENT

<a id="q-s00-cand-434-number-parent"></a>

### Q-S00-CAND-434-NUMBER-PARENT

- 问题：数值显示候选U-188的分层问题？
- 来源/归属：CAND-20261110/U-188；数值显示；C11
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：数值显示候选U-188的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-435-NUMBER-UI、Q-S00-CAND-436-NUMBER-UI

<a id="q-s00-cand-435-number-ui"></a>

### Q-S00-CAND-435-NUMBER-UI

- 问题：数值显示宏组真实enabled与使用宏列表？
- 来源/归属：CAND-20261110/U-188；数值显示；C11
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：数值显示宏组真实enabled与使用宏列表。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-434-NUMBER-PARENT

<a id="q-s00-cand-436-number-ui"></a>

### Q-S00-CAND-436-NUMBER-UI

- 问题：数值显示写宏窗口内容及保存？
- 来源/归属：CAND-20261110/U-188；数值显示；C11
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：数值显示写宏窗口内容及保存。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-434-NUMBER-PARENT

<a id="q-s00-cand-437-number-parent"></a>

### Q-S00-CAND-437-NUMBER-PARENT

- 问题：数值显示候选U-189的分层问题？
- 来源/归属：CAND-20261110/U-189；数值显示；C11
- 层/状态：D/P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：数值显示候选U-189的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-438-NUMBER-BOUND、Q-S00-CAND-439-NUMBER-BOUND、Q-S00-CAND-440-NUMBER-RENDER

<a id="q-s00-cand-438-number-bound"></a>

### Q-S00-CAND-438-NUMBER-BOUND

- 问题：数值显示总位数与小数位数的键入提交及边界？
- 来源/归属：CAND-20261110/U-189；数值显示；C11
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：数值显示总位数与小数位数的键入提交及边界。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-437-NUMBER-PARENT

<a id="q-s00-cand-439-number-bound"></a>

### Q-S00-CAND-439-NUMBER-BOUND

- 问题：数值显示透明度和最小最大值的键入提交及边界？
- 来源/归属：CAND-20261110/U-189；数值显示；C11
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：数值显示透明度和最小最大值的键入提交及边界。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-437-NUMBER-PARENT

<a id="q-s00-cand-440-number-render"></a>

### Q-S00-CAND-440-NUMBER-RENDER

- 问题：数值显示总位数及小数位数修改后的预览变化？
- 来源/归属：CAND-20261110/U-189；数值显示；C11
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：数值显示总位数及小数位数修改后的预览变化。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-437-NUMBER-PARENT

<a id="q-s00-cand-441-number-render"></a>

### Q-S00-CAND-441-NUMBER-RENDER

- 问题：数值显示自定义显示文本在预览和画布如何显示？
- 来源/归属：CAND-20261110/U-190；数值显示；C11
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：数值显示自定义显示文本在预览和画布如何显示。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-442-number-parent"></a>

### Q-S00-CAND-442-NUMBER-PARENT

- 问题：数值显示候选U-191的分层问题？
- 来源/归属：CAND-20261110/U-191；数值显示；C11
- 层/状态：E/R；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：数值显示候选U-191的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-443-NUMBER-RENDER、Q-S00-CAND-444-NUMBER-RUNTIME

<a id="q-s00-cand-443-number-render"></a>

### Q-S00-CAND-443-NUMBER-RENDER

- 问题：数值显示监控地址开关在预览及画布的真实作用？
- 来源/归属：CAND-20261110/U-191；数值显示；C11
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：数值显示监控地址开关在预览及画布的真实作用。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-442-NUMBER-PARENT

<a id="q-s00-cand-444-number-runtime"></a>

### Q-S00-CAND-444-NUMBER-RUNTIME

- 问题：数值显示监控地址开关的屏端作用？
- 来源/归属：CAND-20261110/U-191；数值显示；C11
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：数值显示监控地址开关的屏端作用。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-442-NUMBER-PARENT

<a id="q-s00-cand-445-number-parent"></a>

### Q-S00-CAND-445-NUMBER-PARENT

- 问题：数值显示候选U-192的分层问题？
- 来源/归属：CAND-20261110/U-192；数值显示；C11
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：数值显示候选U-192的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-446-NUMBER-UI、Q-S00-CAND-447-NUMBER-UI、Q-S00-CAND-448-NUMBER-OPTIONS

<a id="q-s00-cand-446-number-ui"></a>

### Q-S00-CAND-446-NUMBER-UI

- 问题：数值显示的ASCII类型完整字段清单？
- 来源/归属：CAND-20261110/U-192；数值显示；C11
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：数值显示的ASCII类型完整字段清单。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-445-NUMBER-PARENT

<a id="q-s00-cand-447-number-ui"></a>

### Q-S00-CAND-447-NUMBER-UI

- 问题：数值显示的日期时间类型完整字段清单？
- 来源/归属：CAND-20261110/U-192；数值显示；C11
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：数值显示的日期时间类型完整字段清单。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-445-NUMBER-PARENT

<a id="q-s00-cand-448-number-options"></a>

### Q-S00-CAND-448-NUMBER-OPTIONS

- 问题：日期时间类型格式下拉的完整选项？
- 来源/归属：CAND-20261110/U-192；数值显示；C11
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：日期时间类型格式下拉的完整选项。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-445-NUMBER-PARENT

<a id="q-s00-cand-449-number-parent"></a>

### Q-S00-CAND-449-NUMBER-PARENT

- 问题：数值显示候选U-193的分层问题？
- 来源/归属：CAND-20261110/U-193；数值显示；C11
- 层/状态：P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：数值显示候选U-193的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-450-NUMBER-GALLERY、Q-S00-CAND-451-NUMBER-GALLERY

<a id="q-s00-cand-450-number-gallery"></a>

### Q-S00-CAND-450-NUMBER-GALLERY

- 问题：数值显示工程图库内容及选图回填？
- 来源/归属：CAND-20261110/U-193；数值显示；C11
- 层/状态：U/P/R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立独立工程图库，加入有唯一内容的两张图并设置一个对象引用，保留资源及工程SHA。
- 刺激：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 对象：数值显示工程图库内容及选图回填。
- 预期观察：确定列表、路径、引用、删除恢复及落盘时机；共用图库实现不证明每个控件调用点相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-449-NUMBER-PARENT

<a id="q-s00-cand-451-number-gallery"></a>

### Q-S00-CAND-451-NUMBER-GALLERY

- 问题：数值显示引用图片删除后的恢复？
- 来源/归属：CAND-20261110/U-193；数值显示；C11
- 层/状态：U/P/R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立独立工程图库，加入有唯一内容的两张图并设置一个对象引用，保留资源及工程SHA。
- 刺激：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 对象：数值显示引用图片删除后的恢复。
- 预期观察：确定列表、路径、引用、删除恢复及落盘时机；共用图库实现不证明每个控件调用点相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-449-NUMBER-PARENT

<a id="q-s00-cand-452-number-ui"></a>

### Q-S00-CAND-452-NUMBER-UI

- 问题：数值显示切换数据类型后输入范围上下限是否重置？
- 来源/归属：CAND-20261110/U-194；数值显示；C11
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：数值显示切换数据类型后输入范围上下限是否重置。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-453-number-parent"></a>

### Q-S00-CAND-453-NUMBER-PARENT

- 问题：数值显示候选U-195的分层问题？
- 来源/归属：CAND-20261110/U-195；数值显示；C11
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：数值显示候选U-195的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-454-NUMBER-BOUND、Q-S00-CAND-455-NUMBER-BOUND

<a id="q-s00-cand-454-number-bound"></a>

### Q-S00-CAND-454-NUMBER-BOUND

- 问题：数值显示空最小值或最大值的触发提示？
- 来源/归属：CAND-20261110/U-195；数值显示；C11
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：数值显示空最小值或最大值的触发提示。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-453-NUMBER-PARENT

<a id="q-s00-cand-455-number-bound"></a>

### Q-S00-CAND-455-NUMBER-BOUND

- 问题：数值显示低高限值越界的触发提示？
- 来源/归属：CAND-20261110/U-195；数值显示；C11
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：数值显示低高限值越界的触发提示。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-453-NUMBER-PARENT

<a id="q-s00-cand-456-number-parent"></a>

### Q-S00-CAND-456-NUMBER-PARENT

- 问题：数值显示候选U-196的分层问题？
- 来源/归属：CAND-20261110/U-196；数值显示；C11
- 层/状态：D/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：数值显示候选U-196的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-457-NUMBER-RENDER、Q-S00-CAND-458-NUMBER-VISIBILITY

<a id="q-s00-cand-457-number-render"></a>

### Q-S00-CAND-457-NUMBER-RENDER

- 问题：数值显示透明度、透明及3D的合成？
- 来源/归属：CAND-20261110/U-196；数值显示；C11
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：数值显示透明度、透明及3D的合成。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-456-NUMBER-PARENT

<a id="q-s00-cand-458-number-visibility"></a>

### Q-S00-CAND-458-NUMBER-VISIBILITY

- 问题：7寸型号仅一个背景状态的能力边界？
- 来源/归属：CAND-20261110/U-196；数值显示；C11
- 层/状态：D/U；未闭合；atomic
- 适用范围：当前V5.1/AMT已观察的隐藏或缺席状态保留；新增其他机型/版本出现条件转能力矩阵扩展，不阻塞当前已核AMT切片。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定当前AMT和V5.1；B/H隐藏字段只作线索，另列型号或版本扩展范围。
- 刺激：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 对象：7寸型号仅一个背景状态的能力边界。
- 预期观察：只对搜索边界报告可达、隐藏或未找到；不因字符串存在增加字段，也不推断所有型号不存在。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-456-NUMBER-PARENT

<a id="q-s00-cand-459-number-ui"></a>

### Q-S00-CAND-459-NUMBER-UI

- 问题：数值显示通知组及位字通知的真实enabled和启用条件？
- 来源/归属：CAND-20261110/U-197；数值显示；C11
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：数值显示通知组及位字通知的真实enabled和启用条件。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-460-number-parent"></a>

### Q-S00-CAND-460-NUMBER-PARENT

- 问题：数值显示候选U-198的分层问题？
- 来源/归属：CAND-20261110/U-198；数值显示；C11
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：数值显示候选U-198的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-461-NUMBER-SOURCE、Q-S00-CAND-462-NUMBER-COMPILE、Q-S00-CAND-463-NUMBER-RUNTIME

<a id="q-s00-cand-461-number-source"></a>

### Q-S00-CAND-461-NUMBER-SOURCE

- 问题：数值显示源对象字段与资源布局？
- 来源/归属：CAND-20261110/U-198；数值显示；C11
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：数值显示源对象字段与资源布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-460-NUMBER-PARENT

<a id="q-s00-cand-462-number-compile"></a>

### Q-S00-CAND-462-NUMBER-COMPILE

- 问题：数值显示设备记录及图像结构？
- 来源/归属：CAND-20261110/U-198；数值显示；C11
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：数值显示设备记录及图像结构。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-460-NUMBER-PARENT

<a id="q-s00-cand-463-number-runtime"></a>

### Q-S00-CAND-463-NUMBER-RUNTIME

- 问题：固件消费数值显示记录的兼容性？
- 来源/归属：CAND-20261110/U-198；数值显示；C11
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件消费数值显示记录的兼容性。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-460-NUMBER-PARENT

<a id="q-s00-cand-464-number-help"></a>

### Q-S00-CAND-464-NUMBER-HELP

- 问题：数值显示帮助按钮是否直达对应CHM页面？
- 来源/归属：CAND-20261110/U-199；数值显示；C11
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：确认原CHM路径/哈希及成员存在，记录完整安装依赖和私有副本环境差异。
- 刺激：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 对象：数值显示帮助按钮是否直达对应CHM页面。
- 预期观察：确认目标成员或有界失败；CHM成员存在不能替代按钮直达事实。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-NUMBER；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-465-datagroup-parent"></a>

### Q-S00-CAND-465-DATAGROUP-PARENT

- 问题：数据群组候选U-200的分层问题？
- 来源/归属：CAND-20261110/U-200；数据群组；C26
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：数据群组候选U-200的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DATAGROUP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-466-DATAGROUP-BOUND、Q-S00-CAND-467-DATAGROUP-BOUND

<a id="q-s00-cand-466-datagroup-bound"></a>

### Q-S00-CAND-466-DATAGROUP-BOUND

- 问题：数据群组采样点数是否以800为上限？
- 来源/归属：CAND-20261110/U-200；数据群组；C26
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：数据群组采样点数是否以800为上限。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DATAGROUP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-465-DATAGROUP-PARENT

<a id="q-s00-cand-467-datagroup-bound"></a>

### Q-S00-CAND-467-DATAGROUP-BOUND

- 问题：数据群组主次分隔数是否分别以10和3为上限？
- 来源/归属：CAND-20261110/U-200；数据群组；C26
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：数据群组主次分隔数是否分别以10和3为上限。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DATAGROUP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-465-DATAGROUP-PARENT

<a id="q-s00-cand-468-datagroup-bound"></a>

### Q-S00-CAND-468-DATAGROUP-BOUND

- 问题：数据群组显示设置最小值大于最大值时的提示？
- 来源/归属：CAND-20261110/U-201；数据群组；C26
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：数据群组显示设置最小值大于最大值时的提示。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DATAGROUP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-469-datagroup-options"></a>

### Q-S00-CAND-469-DATAGROUP-OPTIONS

- 问题：数据群组范围类型第二选项的名称及字段联动？
- 来源/归属：CAND-20261110/U-202；数据群组；C26
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：数据群组范围类型第二选项的名称及字段联动。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DATAGROUP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-470-datagroup-parent"></a>

### Q-S00-CAND-470-DATAGROUP-PARENT

- 问题：数据群组候选U-203的分层问题？
- 来源/归属：CAND-20261110/U-203；数据群组；C26
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：数据群组候选U-203的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DATAGROUP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-471-DATAGROUP-BOUND、Q-S00-CAND-472-DATAGROUP-UI

<a id="q-s00-cand-471-datagroup-bound"></a>

### Q-S00-CAND-471-DATAGROUP-BOUND

- 问题：数据群组通道总数的合法范围？
- 来源/归属：CAND-20261110/U-203；数据群组；C26
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：数据群组通道总数的合法范围。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DATAGROUP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-470-DATAGROUP-PARENT

<a id="q-s00-cand-472-datagroup-ui"></a>

### Q-S00-CAND-472-DATAGROUP-UI

- 问题：数据群组多通道对当前通道下拉和通道草稿的联动？
- 来源/归属：CAND-20261110/U-203；数据群组；C26
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：数据群组多通道对当前通道下拉和通道草稿的联动。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DATAGROUP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-470-DATAGROUP-PARENT

<a id="q-s00-cand-473-datagroup-render"></a>

### Q-S00-CAND-473-DATAGROUP-RENDER

- 问题：数据群组画布Y轴100与对话框1000如何映射？
- 来源/归属：CAND-20261110/U-204；数据群组；C26
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：数据群组画布Y轴100与对话框1000如何映射。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DATAGROUP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-474-datagroup-ui"></a>

### Q-S00-CAND-474-DATAGROUP-UI

- 问题：数据群组右键菜单实际项集及可用状态？
- 来源/归属：CAND-20261110/U-205；数据群组；C26
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：数据群组右键菜单实际项集及可用状态。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DATAGROUP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-475-datagroup-persist"></a>

### Q-S00-CAND-475-DATAGROUP-PERSIST

- 问题：数据群组取消编辑后重新打开是否保留原字段？
- 来源/归属：CAND-20261110/U-206；数据群组；C26
- 层/状态：U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留可辨识原值和工程副本，固定当前语言、状态、类型及对象ID。
- 刺激：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 对象：数据群组取消编辑后重新打开是否保留原字段。
- 预期观察：取得提交、取消和休眠草稿的保存寿命；区分ksp草稿与原厂sm实际保存字段。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DATAGROUP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-476-datagroup-help"></a>

### Q-S00-CAND-476-DATAGROUP-HELP

- 问题：数据群组帮助按钮的实际CHM目标？
- 来源/归属：CAND-20261110/U-207；数据群组；C26
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：确认原CHM路径/哈希及成员存在，记录完整安装依赖和私有副本环境差异。
- 刺激：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 对象：数据群组帮助按钮的实际CHM目标。
- 预期观察：确认目标成员或有界失败；CHM成员存在不能替代按钮直达事实。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DATAGROUP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-477-datagroup-parent"></a>

### Q-S00-CAND-477-DATAGROUP-PARENT

- 问题：数据群组候选U-208的分层问题？
- 来源/归属：CAND-20261110/U-208；数据群组；C26
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：数据群组候选U-208的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DATAGROUP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-478-DATAGROUP-RUNTIME、Q-S00-CAND-479-DATAGROUP-RUNTIME、Q-S00-CAND-480-DATAGROUP-RUNTIME

<a id="q-s00-cand-478-datagroup-runtime"></a>

### Q-S00-CAND-478-DATAGROUP-RUNTIME

- 问题：数据群组触发读取的上升沿或电平时机？
- 来源/归属：CAND-20261110/U-208；数据群组；C26
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：数据群组触发读取的上升沿或电平时机。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DATAGROUP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-477-DATAGROUP-PARENT

<a id="q-s00-cand-479-datagroup-runtime"></a>

### Q-S00-CAND-479-DATAGROUP-RUNTIME

- 问题：数据群组多个监控地址的采样顺序及同步？
- 来源/归属：CAND-20261110/U-208；数据群组；C26
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：数据群组多个监控地址的采样顺序及同步。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DATAGROUP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-477-DATAGROUP-PARENT

<a id="q-s00-cand-480-datagroup-runtime"></a>

### Q-S00-CAND-480-DATAGROUP-RUNTIME

- 问题：数据群组曲线滚动的时序与范围？
- 来源/归属：CAND-20261110/U-208；数据群组；C26
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：数据群组曲线滚动的时序与范围。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DATAGROUP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-477-DATAGROUP-PARENT

<a id="q-s00-cand-481-datagroup-diagnostic"></a>

### Q-S00-CAND-481-DATAGROUP-DIAGNOSTIC

- 问题：数据群组退出码0x1与重试截图的证据上下文能否复现？
- 来源/归属：CAND-20261110/U-209；数据群组；C26
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留发生问题的源工程、原生身份、私有桌面PID、事件类型和退出码。
- 刺激：仅在自有实例串行最小复现，区分直接点击、Qt等价事件及重试；核对已引用截图来自哪次成功会话。 对象：数据群组退出码0x1与重试截图的证据上下文能否复现。
- 预期观察：确定采集可复现性或留下有界异常；未证实原因不得写成试用状态或其他程序冲突，更不要求复刻崩溃。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-DATAGROUP；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有实例串行最小复现，区分直接点击、Qt等价事件及重试；核对已引用截图来自哪次成功会话。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-482-time-ui"></a>

### Q-S00-CAND-482-TIME-UI

- 问题：时间显示预览行m_ShowlineEdit的只读状态及键入行为？
- 来源/归属：CAND-20261110/U-210；时间显示；C13
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：时间显示预览行m_ShowlineEdit的只读状态及键入行为。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-483-time-parent"></a>

### Q-S00-CAND-483-TIME-PARENT

- 问题：时间显示候选U-211的分层问题？
- 来源/归属：CAND-20261110/U-211；时间显示；C13
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：时间显示候选U-211的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-484-TIME-OPTIONS、Q-S00-CAND-485-TIME-OPTIONS

<a id="q-s00-cand-484-time-options"></a>

### Q-S00-CAND-484-TIME-OPTIONS

- 问题：时间显示字号完整下拉序列？
- 来源/归属：CAND-20261110/U-211；时间显示；C13
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：时间显示字号完整下拉序列。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-483-TIME-PARENT

<a id="q-s00-cand-485-time-options"></a>

### Q-S00-CAND-485-TIME-OPTIONS

- 问题：时间显示样式完整下拉序列？
- 来源/归属：CAND-20261110/U-211；时间显示；C13
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：时间显示样式完整下拉序列。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-483-TIME-PARENT

<a id="q-s00-cand-486-time-runtime"></a>

### Q-S00-CAND-486-TIME-RUNTIME

- 问题：时间显示星期最终采用中文还是英文缩写？
- 来源/归属：CAND-20261110/U-212；时间显示；C13
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：时间显示星期最终采用中文还是英文缩写。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-487-time-runtime"></a>

### Q-S00-CAND-487-TIME-RUNTIME

- 问题：时间显示屏端日期分隔符及两位四位年的实际渲染？
- 来源/归属：CAND-20261110/U-213；时间显示；C13
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：时间显示屏端日期分隔符及两位四位年的实际渲染。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-488-time-parent"></a>

### Q-S00-CAND-488-TIME-PARENT

- 问题：时间显示候选U-214的分层问题？
- 来源/归属：CAND-20261110/U-214；时间显示；C13
- 层/状态：P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：时间显示候选U-214的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-489-TIME-GALLERY、Q-S00-CAND-490-TIME-GALLERY

<a id="q-s00-cand-489-time-gallery"></a>

### Q-S00-CAND-489-TIME-GALLERY

- 问题：时间显示系统及工程图库选择回填？
- 来源/归属：CAND-20261110/U-214；时间显示；C13
- 层/状态：U/P/R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立独立工程图库，加入有唯一内容的两张图并设置一个对象引用，保留资源及工程SHA。
- 刺激：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 对象：时间显示系统及工程图库选择回填。
- 预期观察：确定列表、路径、引用、删除恢复及落盘时机；共用图库实现不证明每个控件调用点相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-488-TIME-PARENT

<a id="q-s00-cand-490-time-gallery"></a>

### Q-S00-CAND-490-TIME-GALLERY

- 问题：时间显示删除引用图片后的恢复？
- 来源/归属：CAND-20261110/U-214；时间显示；C13
- 层/状态：U/P/R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立独立工程图库，加入有唯一内容的两张图并设置一个对象引用，保留资源及工程SHA。
- 刺激：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 对象：时间显示删除引用图片后的恢复。
- 预期观察：确定列表、路径、引用、删除恢复及落盘时机；共用图库实现不证明每个控件调用点相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-488-TIME-PARENT

<a id="q-s00-cand-491-time-parent"></a>

### Q-S00-CAND-491-TIME-PARENT

- 问题：时间显示候选U-215的分层问题？
- 来源/归属：CAND-20261110/U-215；时间显示；C13
- 层/状态：R；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：时间显示候选U-215的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-492-TIME-RENDER、Q-S00-CAND-493-TIME-RENDER

<a id="q-s00-cand-492-time-render"></a>

### Q-S00-CAND-492-TIME-RENDER

- 问题：时间显示透明度的像素合成？
- 来源/归属：CAND-20261110/U-215；时间显示；C13
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：时间显示透明度的像素合成。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-491-TIME-PARENT

<a id="q-s00-cand-493-time-render"></a>

### Q-S00-CAND-493-TIME-RENDER

- 问题：时间显示透明与3D标志的图像作用？
- 来源/归属：CAND-20261110/U-215；时间显示；C13
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：时间显示透明与3D标志的图像作用。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-491-TIME-PARENT

<a id="q-s00-cand-494-time-parent"></a>

### Q-S00-CAND-494-TIME-PARENT

- 问题：时间显示候选U-216的分层问题？
- 来源/归属：CAND-20261110/U-216；时间显示；C13
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：时间显示候选U-216的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-495-TIME-UI、Q-S00-CAND-496-TIME-UI

<a id="q-s00-cand-495-time-ui"></a>

### Q-S00-CAND-495-TIME-UI

- 问题：时间显示显现位及等级条件的逐级启用？
- 来源/归属：CAND-20261110/U-216；时间显示；C13
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：时间显示显现位及等级条件的逐级启用。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-494-TIME-PARENT

<a id="q-s00-cand-496-time-ui"></a>

### Q-S00-CAND-496-TIME-UI

- 问题：时间显示字条件比较符及误差行联动？
- 来源/归属：CAND-20261110/U-216；时间显示；C13
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：时间显示字条件比较符及误差行联动。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-494-TIME-PARENT

<a id="q-s00-cand-497-time-persist"></a>

### Q-S00-CAND-497-TIME-PERSIST

- 问题：时间显示逐字段改值后保存重开的回填？
- 来源/归属：CAND-20261110/U-217；时间显示；C13
- 层/状态：U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留可辨识原值和工程副本，固定当前语言、状态、类型及对象ID。
- 刺激：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 对象：时间显示逐字段改值后保存重开的回填。
- 预期观察：取得提交、取消和休眠草稿的保存寿命；区分ksp草稿与原厂sm实际保存字段。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-498-time-parent"></a>

### Q-S00-CAND-498-TIME-PARENT

- 问题：时间显示候选U-218的分层问题？
- 来源/归属：CAND-20261110/U-218；时间显示；C13
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：时间显示候选U-218的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-499-TIME-SOURCE、Q-S00-CAND-500-TIME-COMPILE、Q-S00-CAND-501-TIME-RUNTIME

<a id="q-s00-cand-499-time-source"></a>

### Q-S00-CAND-499-TIME-SOURCE

- 问题：时间显示源对象与格式字段布局？
- 来源/归属：CAND-20261110/U-218；时间显示；C13
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：时间显示源对象与格式字段布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-498-TIME-PARENT

<a id="q-s00-cand-500-time-compile"></a>

### Q-S00-CAND-500-TIME-COMPILE

- 问题：时间显示设备记录及文字图像编码？
- 来源/归属：CAND-20261110/U-218；时间显示；C13
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：时间显示设备记录及文字图像编码。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-498-TIME-PARENT

<a id="q-s00-cand-501-time-runtime"></a>

### Q-S00-CAND-501-TIME-RUNTIME

- 问题：固件消费时间显示记录的格式规则？
- 来源/归属：CAND-20261110/U-218；时间显示；C13
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件消费时间显示记录的格式规则。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-498-TIME-PARENT

<a id="q-s00-cand-502-time-help"></a>

### Q-S00-CAND-502-TIME-HELP

- 问题：时间显示帮助按钮的实际CHM页面？
- 来源/归属：CAND-20261110/U-219；时间显示；C13
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：确认原CHM路径/哈希及成员存在，记录完整安装依赖和私有副本环境差异。
- 刺激：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 对象：时间显示帮助按钮的实际CHM页面。
- 预期观察：确认目标成员或有界失败；CHM成员存在不能替代按钮直达事实。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-503-time-ui"></a>

### Q-S00-CAND-503-TIME-UI

- 问题：时间显示对象右键菜单的实际项集？
- 来源/归属：CAND-20261110/U-220；时间显示；C13
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：时间显示对象右键菜单的实际项集。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-504-time-persist"></a>

### Q-S00-CAND-504-TIME-PERSIST

- 问题：时间切到数值及ASCII再切回后的格式草稿寿命？
- 来源/归属：CAND-20261110/U-221；时间显示；C13
- 层/状态：U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留可辨识原值和工程副本，固定当前语言、状态、类型及对象ID。
- 刺激：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 对象：时间切到数值及ASCII再切回后的格式草稿寿命。
- 预期观察：取得提交、取消和休眠草稿的保存寿命；区分ksp草稿与原厂sm实际保存字段。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TIME；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-505-flow-bound"></a>

### Q-S00-CAND-505-FLOW-BOUND

- 问题：流动块数量在30及邻界输入时的原生限制？
- 来源/归属：CAND-20261110/U-222；流动块；C18
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：流动块数量在30及邻界输入时的原生限制。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FLOW；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-506-flow-runtime"></a>

### Q-S00-CAND-506-FLOW-RUNTIME

- 问题：流动块三档速度分别对应的屏端帧频？
- 来源/归属：CAND-20261110/U-223；流动块；C18
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：流动块三档速度分别对应的屏端帧频。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FLOW；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-507-flow-parent"></a>

### Q-S00-CAND-507-FLOW-PARENT

- 问题：流动块候选U-224的分层问题？
- 来源/归属：CAND-20261110/U-224；流动块；C18
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：流动块候选U-224的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FLOW；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-508-FLOW-OPTIONS、Q-S00-CAND-509-FLOW-OPTIONS、Q-S00-CAND-510-FLOW-UI

<a id="q-s00-cand-508-flow-options"></a>

### Q-S00-CAND-508-FLOW-OPTIONS

- 问题：流动块最低等级完整项集？
- 来源/归属：CAND-20261110/U-224；流动块；C18
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：流动块最低等级完整项集。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FLOW；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-507-FLOW-PARENT

<a id="q-s00-cand-509-flow-options"></a>

### Q-S00-CAND-509-FLOW-OPTIONS

- 问题：流动块字比较符完整项集？
- 来源/归属：CAND-20261110/U-224；流动块；C18
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：流动块字比较符完整项集。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FLOW；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-507-FLOW-PARENT

<a id="q-s00-cand-510-flow-ui"></a>

### Q-S00-CAND-510-FLOW-UI

- 问题：流动块等于及不等于时的误差行？
- 来源/归属：CAND-20261110/U-224；流动块；C18
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：流动块等于及不等于时的误差行。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FLOW；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-507-FLOW-PARENT

<a id="q-s00-cand-511-flow-parent"></a>

### Q-S00-CAND-511-FLOW-PARENT

- 问题：流动块候选U-225的分层问题？
- 来源/归属：CAND-20261110/U-225；流动块；C18
- 层/状态：E/R；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：流动块候选U-225的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FLOW；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-512-FLOW-RENDER、Q-S00-CAND-513-FLOW-RUNTIME

<a id="q-s00-cand-512-flow-render"></a>

### Q-S00-CAND-512-FLOW-RENDER

- 问题：流动块13样式在PC预览及画布的绘制差异？
- 来源/归属：CAND-20261110/U-225；流动块；C18
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：流动块13样式在PC预览及画布的绘制差异。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FLOW；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-511-FLOW-PARENT

<a id="q-s00-cand-513-flow-runtime"></a>

### Q-S00-CAND-513-FLOW-RUNTIME

- 问题：流动块13样式的屏端绘制差异？
- 来源/归属：CAND-20261110/U-225；流动块；C18
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：流动块13样式的屏端绘制差异。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FLOW；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-511-FLOW-PARENT

<a id="q-s00-cand-514-flow-ui"></a>

### Q-S00-CAND-514-FLOW-UI

- 问题：流动块显示方式切换时四方向的映射及手动覆盖？
- 来源/归属：CAND-20261110/U-226；流动块；C18
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：流动块显示方式切换时四方向的映射及手动覆盖。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FLOW；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-515-flow-parent"></a>

### Q-S00-CAND-515-FLOW-PARENT

- 问题：流动块候选U-227的分层问题？
- 来源/归属：CAND-20261110/U-227；流动块；C18
- 层/状态：C/D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：流动块候选U-227的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FLOW；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-516-FLOW-SCOPE、Q-S00-CAND-517-FLOW-ADDRESS

<a id="q-s00-cand-516-flow-scope"></a>

### Q-S00-CAND-516-FLOW-SCOPE

- 问题：流动块范围触发缺席的当前版本能力范围？
- 来源/归属：CAND-20261110/U-227；流动块；C18
- 层/状态：D；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：并列原始候选、原帮助及当前V5.1直接证据，保留版本/型号和文档章节。
- 刺激：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 对象：流动块范围触发缺席的当前版本能力范围。
- 预期观察：形成可追溯的来源与采用边界；说明句或旧图不自动成为待实现功能或运行算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FLOW；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-515-FLOW-PARENT

<a id="q-s00-cand-517-flow-address"></a>

### Q-S00-CAND-517-FLOW-ADDRESS

- 问题：流动块触发地址拾取家族与工程设备的关联？
- 来源/归属：CAND-20261110/U-227；流动块；C18
- 层/状态：D/U/C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定所用地址角色、位宽、连接/型号和通道；保留无连接及已配置工程两份。
- 刺激：从指定控件字段打开拾取器，枚举相应家族并选首末/宽地址边界，确认与取消后核对回填和跨度。 对象：流动块触发地址拾取家族与工程设备的关联。
- 预期观察：取得此调用点的地址角色与可达族；别的控件同款窗口仅作线索，不能代替本入口。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FLOW；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：从指定控件字段打开拾取器，枚举相应家族并选首末/宽地址边界，确认与取消后核对回填和跨度。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-515-FLOW-PARENT

<a id="q-s00-cand-518-flow-parent"></a>

### Q-S00-CAND-518-FLOW-PARENT

- 问题：流动块候选U-228的分层问题？
- 来源/归属：CAND-20261110/U-228；流动块；C18
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：流动块候选U-228的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FLOW；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-519-FLOW-SOURCE、Q-S00-CAND-520-FLOW-COMPILE、Q-S00-CAND-521-FLOW-RUNTIME

<a id="q-s00-cand-519-flow-source"></a>

### Q-S00-CAND-519-FLOW-SOURCE

- 问题：流动块源对象字段布局？
- 来源/归属：CAND-20261110/U-228；流动块；C18
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：流动块源对象字段布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FLOW；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-518-FLOW-PARENT

<a id="q-s00-cand-520-flow-compile"></a>

### Q-S00-CAND-520-FLOW-COMPILE

- 问题：流动块下载记录与资源编码？
- 来源/归属：CAND-20261110/U-228；流动块；C18
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：流动块下载记录与资源编码。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FLOW；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-518-FLOW-PARENT

<a id="q-s00-cand-521-flow-runtime"></a>

### Q-S00-CAND-521-FLOW-RUNTIME

- 问题：固件消费流动块记录的运动规则？
- 来源/归属：CAND-20261110/U-228；流动块；C18
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件消费流动块记录的运动规则。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FLOW；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-518-FLOW-PARENT

<a id="q-s00-cand-522-message-bound"></a>

### Q-S00-CAND-522-MESSAGE-BOUND

- 问题：消息总状态数在2、128及邻界输入时的限制？
- 来源/归属：CAND-20261110/U-229；消息显示；C06
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：消息总状态数在2、128及邻界输入时的限制。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-523-message-address"></a>

### Q-S00-CAND-523-MESSAGE-ADDRESS

- 问题：消息切到32位正整数后监视地址文本及跨度是否改写？
- 来源/归属：CAND-20261110/U-230；消息显示；C06
- 层/状态：D/U/C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定所用地址角色、位宽、连接/型号和通道；保留无连接及已配置工程两份。
- 刺激：从指定控件字段打开拾取器，枚举相应家族并选首末/宽地址边界，确认与取消后核对回填和跨度。 对象：消息切到32位正整数后监视地址文本及跨度是否改写。
- 预期观察：取得此调用点的地址角色与可达族；别的控件同款窗口仅作线索，不能代替本入口。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：从指定控件字段打开拾取器，枚举相应家族并选首末/宽地址边界，确认与取消后核对回填和跨度。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-524-message-parent"></a>

### Q-S00-CAND-524-MESSAGE-PARENT

- 问题：消息显示候选U-231的分层问题？
- 来源/归属：CAND-20261110/U-231；消息显示；C06
- 层/状态：D/E/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：消息显示候选U-231的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-525-MESSAGE-BOUND、Q-S00-CAND-526-MESSAGE-RUNTIME

<a id="q-s00-cand-525-message-bound"></a>

### Q-S00-CAND-525-MESSAGE-BOUND

- 问题：消息移动速度的合法输入范围？
- 来源/归属：CAND-20261110/U-231；消息显示；C06
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：消息移动速度的合法输入范围。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-524-MESSAGE-PARENT

<a id="q-s00-cand-526-message-runtime"></a>

### Q-S00-CAND-526-MESSAGE-RUNTIME

- 问题：消息移动速度单位与屏端滚动速度换算？
- 来源/归属：CAND-20261110/U-231；消息显示；C06
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：消息移动速度单位与屏端滚动速度换算。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-524-MESSAGE-PARENT

<a id="q-s00-cand-527-message-language"></a>

### Q-S00-CAND-527-MESSAGE-LANGUAGE

- 问题：消息多语言列表与每状态多语言文本表的形态？
- 来源/归属：CAND-20261110/U-232；消息显示；C06
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立中文单语及中文/English/第三种语言工程，给各状态各语言写不同文本。
- 刺激：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 对象：消息多语言列表与每状态多语言文本表的形态。
- 预期观察：确认语言列表来源、索引及草稿规则；运行切换是否正确仍需独立E观察。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-528-message-options"></a>

### Q-S00-CAND-528-MESSAGE-OPTIONS

- 问题：消息字体完整列表与选择后的回填？
- 来源/归属：CAND-20261110/U-233；消息显示；C06
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：消息字体完整列表与选择后的回填。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-529-message-options"></a>

### Q-S00-CAND-529-MESSAGE-OPTIONS

- 问题：消息13样式中三种辐射项的完整索引顺序？
- 来源/归属：CAND-20261110/U-234；消息显示；C06
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：消息13样式中三种辐射项的完整索引顺序。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-530-message-bound"></a>

### Q-S00-CAND-530-MESSAGE-BOUND

- 问题：消息单状态文本的字节上限及超限提示？
- 来源/归属：CAND-20261110/U-235；消息显示；C06
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：消息单状态文本的字节上限及超限提示。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-531-message-parent"></a>

### Q-S00-CAND-531-MESSAGE-PARENT

- 问题：消息显示候选U-236的分层问题？
- 来源/归属：CAND-20261110/U-236；消息显示；C06
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：消息显示候选U-236的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-532-MESSAGE-PERMISSION、Q-S00-CAND-533-MESSAGE-THRESHOLD、Q-S00-CAND-534-MESSAGE-THRESHOLD

<a id="q-s00-cand-532-message-permission"></a>

### Q-S00-CAND-532-MESSAGE-PERMISSION

- 问题：消息显现多条件的组合逻辑？
- 来源/归属：CAND-20261110/U-236；消息显示；C06
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：选择该控件的显现或触控列，单独建立位、等级、字条件，且对象无重叠遮挡。
- 刺激：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 对象：消息显现多条件的组合逻辑。
- 预期观察：取得显现与触控各自的真值表、组合方式及优先级；不由共用容器推断所有控件相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-531-MESSAGE-PARENT

<a id="q-s00-cand-533-message-threshold"></a>

### Q-S00-CAND-533-MESSAGE-THRESHOLD

- 问题：消息最低等级等于阈值边界？
- 来源/归属：CAND-20261110/U-236；消息显示；C06
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：固定该控件的最低等级阈值，位和字条件关闭或恒定满足。
- 刺激：分别以阈值-1、阈值、阈值+1等级观察显现或触控；显现和触控列分开记录。 对象：消息最低等级等于阈值边界。
- 预期观察：取得最低等级的等值边界与实际权限结果，不由字段名称预设闭开关系。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：分别以阈值-1、阈值、阈值+1等级观察显现或触控；显现和触控列分开记录。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-531-MESSAGE-PARENT

<a id="q-s00-cand-534-message-threshold"></a>

### Q-S00-CAND-534-MESSAGE-THRESHOLD

- 问题：消息比较误差的计算方式？
- 来源/归属：CAND-20261110/U-236；消息显示；C06
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：固定字比较值和误差，位和等级条件恒定满足；整数和浮点分别留样本。
- 刺激：测比较值±容差及邻界，分别测试等于、不等于及0误差，保持其他字段不变。 对象：消息比较误差的计算方式。
- 预期观察：取得容差闭开区间、绝对/相对误差及浮点边界，不提前假设具体公式。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：测比较值±容差及邻界，分别测试等于、不等于及0误差，保持其他字段不变。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-531-MESSAGE-PARENT

<a id="q-s00-cand-535-message-help"></a>

### Q-S00-CAND-535-MESSAGE-HELP

- 问题：消息帮助按钮是否直达messageshowIndex.htm？
- 来源/归属：CAND-20261110/U-237；消息显示；C06
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：确认原CHM路径/哈希及成员存在，记录完整安装依赖和私有副本环境差异。
- 刺激：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 对象：消息帮助按钮是否直达messageshowIndex.htm。
- 预期观察：确认目标成员或有界失败；CHM成员存在不能替代按钮直达事实。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-536-message-visibility"></a>

### Q-S00-CAND-536-MESSAGE-VISIBILITY

- 问题：消息外形及自动上传隐藏字段在当前V5.1是否可达？
- 来源/归属：CAND-20261110/U-238；消息显示；C06
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定当前AMT和V5.1；B/H隐藏字段只作线索，另列型号或版本扩展范围。
- 刺激：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 对象：消息外形及自动上传隐藏字段在当前V5.1是否可达。
- 预期观察：只对搜索边界报告可达、隐藏或未找到；不因字符串存在增加字段，也不推断所有型号不存在。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-537-message-parent"></a>

### Q-S00-CAND-537-MESSAGE-PARENT

- 问题：消息显示候选U-239的分层问题？
- 来源/归属：CAND-20261110/U-239；消息显示；C06
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：消息显示候选U-239的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-538-MESSAGE-SOURCE、Q-S00-CAND-539-MESSAGE-COMPILE、Q-S00-CAND-540-MESSAGE-RUNTIME

<a id="q-s00-cand-538-message-source"></a>

### Q-S00-CAND-538-MESSAGE-SOURCE

- 问题：消息显示状态及语言的sm结构？
- 来源/归属：CAND-20261110/U-239；消息显示；C06
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：消息显示状态及语言的sm结构。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-537-MESSAGE-PARENT

<a id="q-s00-cand-539-message-compile"></a>

### Q-S00-CAND-539-MESSAGE-COMPILE

- 问题：Message.dat和SimMessage.dat的编码？
- 来源/归属：CAND-20261110/U-239；消息显示；C06
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：Message.dat和SimMessage.dat的编码。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-537-MESSAGE-PARENT

<a id="q-s00-cand-540-message-runtime"></a>

### Q-S00-CAND-540-MESSAGE-RUNTIME

- 问题：固件按监视值选择消息记录的兼容性？
- 来源/归属：CAND-20261110/U-239；消息显示；C06
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件按监视值选择消息记录的兼容性。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-537-MESSAGE-PARENT

<a id="q-s00-cand-541-message-parent"></a>

### Q-S00-CAND-541-MESSAGE-PARENT

- 问题：消息显示候选U-240的分层问题？
- 来源/归属：CAND-20261110/U-240；消息显示；C06
- 层/状态：D/E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：消息显示候选U-240的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-542-MESSAGE-SCOPE、Q-S00-CAND-543-MESSAGE-RUNTIME

<a id="q-s00-cand-542-message-scope"></a>

### Q-S00-CAND-542-MESSAGE-SCOPE

- 问题：消息是否与采集或报警配置存在运行依赖？
- 来源/归属：CAND-20261110/U-240；消息显示；C06
- 层/状态：D；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：并列原始候选、原帮助及当前V5.1直接证据，保留版本/型号和文档章节。
- 刺激：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 对象：消息是否与采集或报警配置存在运行依赖。
- 预期观察：形成可追溯的来源与采用边界；说明句或旧图不自动成为待实现功能或运行算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-541-MESSAGE-PARENT

<a id="q-s00-cand-543-message-runtime"></a>

### Q-S00-CAND-543-MESSAGE-RUNTIME

- 问题：消息监视地址数值与状态文本的屏端映射？
- 来源/归属：CAND-20261110/U-240；消息显示；C06
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：消息监视地址数值与状态文本的屏端映射。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-MESSAGE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-541-MESSAGE-PARENT

<a id="q-s00-cand-544-slide-parent"></a>

### Q-S00-CAND-544-SLIDE-PARENT

- 问题：滑动开关候选U-241的分层问题？
- 来源/归属：CAND-20261110/U-241；滑动开关；C19
- 层/状态：D/P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：滑动开关候选U-241的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-545-SLIDE-UI、Q-S00-CAND-546-SLIDE-RENDER、Q-S00-CAND-547-SLIDE-PERSIST

<a id="q-s00-cand-545-slide-ui"></a>

### Q-S00-CAND-545-SLIDE-UI

- 问题：滑动开关确定后等价拖框是否生成对象？
- 来源/归属：CAND-20261110/U-241；滑动开关；C19
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：滑动开关确定后等价拖框是否生成对象。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-544-SLIDE-PARENT

<a id="q-s00-cand-546-slide-render"></a>

### Q-S00-CAND-546-SLIDE-RENDER

- 问题：滑动开关新建后的画布外观及状态栏类型？
- 来源/归属：CAND-20261110/U-241；滑动开关；C19
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：滑动开关新建后的画布外观及状态栏类型。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-544-SLIDE-PARENT

<a id="q-s00-cand-547-slide-persist"></a>

### Q-S00-CAND-547-SLIDE-PERSIST

- 问题：滑动开关成功创建后双击回填？
- 来源/归属：CAND-20261110/U-241；滑动开关；C19
- 层/状态：U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留可辨识原值和工程副本，固定当前语言、状态、类型及对象ID。
- 刺激：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 对象：滑动开关成功创建后双击回填。
- 预期观察：取得提交、取消和休眠草稿的保存寿命；区分ksp草稿与原厂sm实际保存字段。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-544-SLIDE-PARENT

<a id="q-s00-cand-548-slide-scope"></a>

### Q-S00-CAND-548-SLIDE-SCOPE

- 问题：滑动开关帮助Bin及BCD与V5.1七数据类型的对应？
- 来源/归属：CAND-20261110/U-242；滑动开关；C19
- 层/状态：D；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：并列原始候选、原帮助及当前V5.1直接证据，保留版本/型号和文档章节。
- 刺激：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 对象：滑动开关帮助Bin及BCD与V5.1七数据类型的对应。
- 预期观察：形成可追溯的来源与采用边界；说明句或旧图不自动成为待实现功能或运行算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-549-slide-runtime"></a>

### Q-S00-CAND-549-SLIDE-RUNTIME

- 问题：滑块指向与滑动方向组合的设备运行矩阵？
- 来源/归属：CAND-20261110/U-243；滑动开关；C19
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：滑块指向与滑动方向组合的设备运行矩阵。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-550-slide-parent"></a>

### Q-S00-CAND-550-SLIDE-PARENT

- 问题：滑动开关候选U-244的分层问题？
- 来源/归属：CAND-20261110/U-244；滑动开关；C19
- 层/状态：D/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：滑动开关候选U-244的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-551-SLIDE-UI、Q-S00-CAND-552-SLIDE-RENDER

<a id="q-s00-cand-551-slide-ui"></a>

### Q-S00-CAND-551-SLIDE-UI

- 问题：滑动开关各透明框对应的色块启用状态？
- 来源/归属：CAND-20261110/U-244；滑动开关；C19
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：滑动开关各透明框对应的色块启用状态。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-550-SLIDE-PARENT

<a id="q-s00-cand-552-slide-render"></a>

### Q-S00-CAND-552-SLIDE-RENDER

- 问题：滑动开关各透明框对PC渲染的影响？
- 来源/归属：CAND-20261110/U-244；滑动开关；C19
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：滑动开关各透明框对PC渲染的影响。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-550-SLIDE-PARENT

<a id="q-s00-cand-553-slide-parent"></a>

### Q-S00-CAND-553-SLIDE-PARENT

- 问题：滑动开关候选U-245的分层问题？
- 来源/归属：CAND-20261110/U-245；滑动开关；C19
- 层/状态：C/D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：滑动开关候选U-245的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-554-SLIDE-UI、Q-S00-CAND-555-SLIDE-ADDRESS

<a id="q-s00-cand-554-slide-ui"></a>

### Q-S00-CAND-554-SLIDE-UI

- 问题：滑动开关颜色窗口的确认取消回写？
- 来源/归属：CAND-20261110/U-245；滑动开关；C19
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：滑动开关颜色窗口的确认取消回写。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-553-SLIDE-PARENT

<a id="q-s00-cand-555-slide-address"></a>

### Q-S00-CAND-555-SLIDE-ADDRESS

- 问题：滑动开关地址拾取窗口的角色和回填？
- 来源/归属：CAND-20261110/U-245；滑动开关；C19
- 层/状态：D/U/C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定所用地址角色、位宽、连接/型号和通道；保留无连接及已配置工程两份。
- 刺激：从指定控件字段打开拾取器，枚举相应家族并选首末/宽地址边界，确认与取消后核对回填和跨度。 对象：滑动开关地址拾取窗口的角色和回填。
- 预期观察：取得此调用点的地址角色与可达族；别的控件同款窗口仅作线索，不能代替本入口。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：从指定控件字段打开拾取器，枚举相应家族并选首末/宽地址边界，确认与取消后核对回填和跨度。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-553-SLIDE-PARENT

<a id="q-s00-cand-556-slide-parent"></a>

### Q-S00-CAND-556-SLIDE-PARENT

- 问题：滑动开关候选U-246的分层问题？
- 来源/归属：CAND-20261110/U-246；滑动开关；C19
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：滑动开关候选U-246的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-557-SLIDE-BOUND、Q-S00-CAND-558-SLIDE-BOUND

<a id="q-s00-cand-557-slide-bound"></a>

### Q-S00-CAND-557-SLIDE-BOUND

- 问题：滑轨高度与滑块宽高的合法范围？
- 来源/归属：CAND-20261110/U-246；滑动开关；C19
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：滑轨高度与滑块宽高的合法范围。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-556-SLIDE-PARENT

<a id="q-s00-cand-558-slide-bound"></a>

### Q-S00-CAND-558-SLIDE-BOUND

- 问题：滑动开关最小最大值的边界及越界提示？
- 来源/归属：CAND-20261110/U-246；滑动开关；C19
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：滑动开关最小最大值的边界及越界提示。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-556-SLIDE-PARENT

<a id="q-s00-cand-559-slide-parent"></a>

### Q-S00-CAND-559-SLIDE-PARENT

- 问题：滑动开关候选U-247的分层问题？
- 来源/归属：CAND-20261110/U-247；滑动开关；C19
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：滑动开关候选U-247的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-560-SLIDE-PERSIST、Q-S00-CAND-561-SLIDE-UI、Q-S00-CAND-562-SLIDE-UI

<a id="q-s00-cand-560-slide-persist"></a>

### Q-S00-CAND-560-SLIDE-PERSIST

- 问题：滑动开关取消编辑后重新打开的字段？
- 来源/归属：CAND-20261110/U-247；滑动开关；C19
- 层/状态：U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留可辨识原值和工程副本，固定当前语言、状态、类型及对象ID。
- 刺激：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 对象：滑动开关取消编辑后重新打开的字段。
- 预期观察：取得提交、取消和休眠草稿的保存寿命；区分ksp草稿与原厂sm实际保存字段。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-559-SLIDE-PARENT

<a id="q-s00-cand-561-slide-ui"></a>

### Q-S00-CAND-561-SLIDE-UI

- 问题：滑动开关右键菜单的实际项集？
- 来源/归属：CAND-20261110/U-247；滑动开关；C19
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：滑动开关右键菜单的实际项集。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-559-SLIDE-PARENT

<a id="q-s00-cand-562-slide-ui"></a>

### Q-S00-CAND-562-SLIDE-UI

- 问题：滑动开关状态栏的对象类型文字？
- 来源/归属：CAND-20261110/U-247；滑动开关；C19
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：滑动开关状态栏的对象类型文字。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-559-SLIDE-PARENT

<a id="q-s00-cand-563-slide-ui"></a>

### Q-S00-CAND-563-SLIDE-UI

- 问题：滑动开关通知组及位字通知的真实enabled与展开？
- 来源/归属：CAND-20261110/U-248；滑动开关；C19
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：滑动开关通知组及位字通知的真实enabled与展开。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-564-slide-parent"></a>

### Q-S00-CAND-564-SLIDE-PARENT

- 问题：滑动开关候选U-249的分层问题？
- 来源/归属：CAND-20261110/U-249；滑动开关；C19
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：滑动开关候选U-249的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-565-SLIDE-RUNTIME、Q-S00-CAND-566-SLIDE-RUNTIME

<a id="q-s00-cand-565-slide-runtime"></a>

### Q-S00-CAND-565-SLIDE-RUNTIME

- 问题：滑动开关拖动期间还是松手时写值？
- 来源/归属：CAND-20261110/U-249；滑动开关；C19
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：滑动开关拖动期间还是松手时写值。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-564-SLIDE-PARENT

<a id="q-s00-cand-566-slide-runtime"></a>

### Q-S00-CAND-566-SLIDE-RUNTIME

- 问题：滑动开关单向与双向样式的实际触控行为？
- 来源/归属：CAND-20261110/U-249；滑动开关；C19
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：滑动开关单向与双向样式的实际触控行为。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-564-SLIDE-PARENT

<a id="q-s00-cand-567-slide-parent"></a>

### Q-S00-CAND-567-SLIDE-PARENT

- 问题：滑动开关候选U-250的分层问题？
- 来源/归属：CAND-20261110/U-250；滑动开关；C19
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：滑动开关候选U-250的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-568-SLIDE-SOURCE、Q-S00-CAND-569-SLIDE-COMPILE、Q-S00-CAND-570-SLIDE-RUNTIME

<a id="q-s00-cand-568-slide-source"></a>

### Q-S00-CAND-568-SLIDE-SOURCE

- 问题：滑动开关源字段及资源保存布局？
- 来源/归属：CAND-20261110/U-250；滑动开关；C19
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：滑动开关源字段及资源保存布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-567-SLIDE-PARENT

<a id="q-s00-cand-569-slide-compile"></a>

### Q-S00-CAND-569-SLIDE-COMPILE

- 问题：滑动开关设备控制及图像布局？
- 来源/归属：CAND-20261110/U-250；滑动开关；C19
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：滑动开关设备控制及图像布局。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-567-SLIDE-PARENT

<a id="q-s00-cand-570-slide-runtime"></a>

### Q-S00-CAND-570-SLIDE-RUNTIME

- 问题：固件消费滑动开关拖动记录的规则？
- 来源/归属：CAND-20261110/U-250；滑动开关；C19
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件消费滑动开关拖动记录的规则。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-567-SLIDE-PARENT

<a id="q-s00-cand-571-slide-scope"></a>

### Q-S00-CAND-571-SLIDE-SCOPE

- 问题：滑动开关V5.1两页与静态三页字符串的适用边界？
- 来源/归属：CAND-20261110/U-251；滑动开关；C19
- 层/状态：D；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：并列原始候选、原帮助及当前V5.1直接证据，保留版本/型号和文档章节。
- 刺激：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 对象：滑动开关V5.1两页与静态三页字符串的适用边界。
- 预期观察：形成可追溯的来源与采用边界；说明句或旧图不自动成为待实现功能或运行算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SLIDE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-572-wheel-parent"></a>

### Q-S00-CAND-572-WHEEL-PARENT

- 问题：滚轮候选U-252的分层问题？
- 来源/归属：CAND-20261110/U-252；滚轮；C20
- 层/状态：D/E/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：滚轮候选U-252的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-573-WHEEL-BOUND、Q-S00-CAND-574-WHEEL-RUNTIME

<a id="q-s00-cand-573-wheel-bound"></a>

### Q-S00-CAND-573-WHEEL-BOUND

- 问题：滚轮灵敏度的合法输入范围？
- 来源/归属：CAND-20261110/U-252；滚轮；C20
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：滚轮灵敏度的合法输入范围。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-572-WHEEL-PARENT

<a id="q-s00-cand-574-wheel-runtime"></a>

### Q-S00-CAND-574-WHEEL-RUNTIME

- 问题：滚轮灵敏度20与滚动步数及写值的换算？
- 来源/归属：CAND-20261110/U-252；滚轮；C20
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：滚轮灵敏度20与滚动步数及写值的换算。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-572-WHEEL-PARENT

<a id="q-s00-cand-575-wheel-parent"></a>

### Q-S00-CAND-575-WHEEL-PARENT

- 问题：滚轮候选U-253的分层问题？
- 来源/归属：CAND-20261110/U-253；滚轮；C20
- 层/状态：D/E/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：滚轮候选U-253的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-576-WHEEL-BOUND、Q-S00-CAND-577-WHEEL-RUNTIME

<a id="q-s00-cand-576-wheel-bound"></a>

### Q-S00-CAND-576-WHEEL-BOUND

- 问题：滚轮当前数目在5及邻界输入时的限制？
- 来源/归属：CAND-20261110/U-253；滚轮；C20
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：滚轮当前数目在5及邻界输入时的限制。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-575-WHEEL-PARENT

<a id="q-s00-cand-577-wheel-runtime"></a>

### Q-S00-CAND-577-WHEEL-RUNTIME

- 问题：滚轮端点的绕回方向和步距？
- 来源/归属：CAND-20261110/U-253；滚轮；C20
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：滚轮端点的绕回方向和步距。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-575-WHEEL-PARENT

<a id="q-s00-cand-578-wheel-parent"></a>

### Q-S00-CAND-578-WHEEL-PARENT

- 问题：滚轮候选U-254的分层问题？
- 来源/归属：CAND-20261110/U-254；滚轮；C20
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：滚轮候选U-254的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-579-WHEEL-OPTIONS、Q-S00-CAND-580-WHEEL-OPTIONS

<a id="q-s00-cand-579-wheel-options"></a>

### Q-S00-CAND-579-WHEEL-OPTIONS

- 问题：滚轮低亮字号完整项集？
- 来源/归属：CAND-20261110/U-254；滚轮；C20
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：滚轮低亮字号完整项集。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-578-WHEEL-PARENT

<a id="q-s00-cand-580-wheel-options"></a>

### Q-S00-CAND-580-WHEEL-OPTIONS

- 问题：滚轮高亮字号完整项集？
- 来源/归属：CAND-20261110/U-254；滚轮；C20
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：滚轮高亮字号完整项集。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-578-WHEEL-PARENT

<a id="q-s00-cand-581-wheel-render"></a>

### Q-S00-CAND-581-WHEEL-RENDER

- 问题：滚轮默认空白高亮文本色是白色绘制还是不绘制？
- 来源/归属：CAND-20261110/U-255；滚轮；C20
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：滚轮默认空白高亮文本色是白色绘制还是不绘制。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-582-wheel-parent"></a>

### Q-S00-CAND-582-WHEEL-PARENT

- 问题：滚轮候选U-256的分层问题？
- 来源/归属：CAND-20261110/U-256；滚轮；C20
- 层/状态：D/P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：滚轮候选U-256的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-583-WHEEL-GALLERY、Q-S00-CAND-584-WHEEL-GALLERY、Q-S00-CAND-585-WHEEL-UI

<a id="q-s00-cand-583-wheel-gallery"></a>

### Q-S00-CAND-583-WHEEL-GALLERY

- 问题：滚轮高亮底图系统图库选择回填？
- 来源/归属：CAND-20261110/U-256；滚轮；C20
- 层/状态：U/P/R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立独立工程图库，加入有唯一内容的两张图并设置一个对象引用，保留资源及工程SHA。
- 刺激：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 对象：滚轮高亮底图系统图库选择回填。
- 预期观察：确定列表、路径、引用、删除恢复及落盘时机；共用图库实现不证明每个控件调用点相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-582-WHEEL-PARENT

<a id="q-s00-cand-584-wheel-gallery"></a>

### Q-S00-CAND-584-WHEEL-GALLERY

- 问题：滚轮高亮底图工程图库选择回填？
- 来源/归属：CAND-20261110/U-256；滚轮；C20
- 层/状态：U/P/R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立独立工程图库，加入有唯一内容的两张图并设置一个对象引用，保留资源及工程SHA。
- 刺激：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 对象：滚轮高亮底图工程图库选择回填。
- 预期观察：确定列表、路径、引用、删除恢复及落盘时机；共用图库实现不证明每个控件调用点相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-582-WHEEL-PARENT

<a id="q-s00-cand-585-wheel-ui"></a>

### Q-S00-CAND-585-WHEEL-UI

- 问题：滚轮不使用高亮底图后的状态？
- 来源/归属：CAND-20261110/U-256；滚轮；C20
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：滚轮不使用高亮底图后的状态。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-582-WHEEL-PARENT

<a id="q-s00-cand-586-wheel-runtime"></a>

### Q-S00-CAND-586-WHEEL-RUNTIME

- 问题：滚轮写入地址与显示读取地址是否相同及实际写值规则？
- 来源/归属：CAND-20261110/U-257；滚轮；C20
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：滚轮写入地址与显示读取地址是否相同及实际写值规则。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-587-wheel-parent"></a>

### Q-S00-CAND-587-WHEEL-PARENT

- 问题：滚轮候选U-258的分层问题？
- 来源/归属：CAND-20261110/U-258；滚轮；C20
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：滚轮候选U-258的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-588-WHEEL-OPTIONS、Q-S00-CAND-589-WHEEL-OPTIONS

<a id="q-s00-cand-588-wheel-options"></a>

### Q-S00-CAND-588-WHEEL-OPTIONS

- 问题：滚轮权限最低等级完整项集？
- 来源/归属：CAND-20261110/U-258；滚轮；C20
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：滚轮权限最低等级完整项集。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-587-WHEEL-PARENT

<a id="q-s00-cand-589-wheel-options"></a>

### Q-S00-CAND-589-WHEEL-OPTIONS

- 问题：滚轮权限比较符完整项集？
- 来源/归属：CAND-20261110/U-258；滚轮；C20
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：滚轮权限比较符完整项集。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-587-WHEEL-PARENT

<a id="q-s00-cand-590-wheel-visibility"></a>

### Q-S00-CAND-590-WHEEL-VISIBILITY

- 问题：滚轮数值图形组及m_currentedit的V5.1可达条件？
- 来源/归属：CAND-20261110/U-259；滚轮；C20
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定当前AMT和V5.1；B/H隐藏字段只作线索，另列型号或版本扩展范围。
- 刺激：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 对象：滚轮数值图形组及m_currentedit的V5.1可达条件。
- 预期观察：只对搜索边界报告可达、隐藏或未找到；不因字符串存在增加字段，也不推断所有型号不存在。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-591-wheel-parent"></a>

### Q-S00-CAND-591-WHEEL-PARENT

- 问题：滚轮候选U-260的分层问题？
- 来源/归属：CAND-20261110/U-260；滚轮；C20
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：滚轮候选U-260的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-592-WHEEL-SOURCE、Q-S00-CAND-593-WHEEL-COMPILE、Q-S00-CAND-594-WHEEL-RUNTIME

<a id="q-s00-cand-592-wheel-source"></a>

### Q-S00-CAND-592-WHEEL-SOURCE

- 问题：滚轮源对象字段布局？
- 来源/归属：CAND-20261110/U-260；滚轮；C20
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：滚轮源对象字段布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-591-WHEEL-PARENT

<a id="q-s00-cand-593-wheel-compile"></a>

### Q-S00-CAND-593-WHEEL-COMPILE

- 问题：滚轮设备记录及高低亮资源布局？
- 来源/归属：CAND-20261110/U-260；滚轮；C20
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：滚轮设备记录及高低亮资源布局。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-591-WHEEL-PARENT

<a id="q-s00-cand-594-wheel-runtime"></a>

### Q-S00-CAND-594-WHEEL-RUNTIME

- 问题：固件消费滚轮记录的兼容性？
- 来源/归属：CAND-20261110/U-260；滚轮；C20
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件消费滚轮记录的兼容性。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-WHEEL；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-591-WHEEL-PARENT

<a id="q-s00-cand-595-screen-parent"></a>

### Q-S00-CAND-595-SCREEN-PARENT

- 问题：画面按钮候选U-261的分层问题？
- 来源/归属：CAND-20261110/U-261；画面按钮；C14
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：画面按钮候选U-261的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-596-SCREEN-RUNTIME、Q-S00-CAND-597-SCREEN-RUNTIME、Q-S00-CAND-598-SCREEN-RUNTIME

<a id="q-s00-cand-596-screen-runtime"></a>

### Q-S00-CAND-596-SCREEN-RUNTIME

- 问题：画面按钮四种操作的页面或窗口切换规则？
- 来源/归属：CAND-20261110/U-261；画面按钮；C14
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：画面按钮四种操作的页面或窗口切换规则。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-595-SCREEN-PARENT

<a id="q-s00-cand-597-screen-runtime"></a>

### Q-S00-CAND-597-SCREEN-RUNTIME

- 问题：画面按钮等级登录的密码键盘和权限变化？
- 来源/归属：CAND-20261110/U-261；画面按钮；C14
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：画面按钮等级登录的密码键盘和权限变化。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-595-SCREEN-PARENT

<a id="q-s00-cand-598-screen-runtime"></a>

### Q-S00-CAND-598-SCREEN-RUNTIME

- 问题：画面按钮跳转返回栈深度及窗口页边界？
- 来源/归属：CAND-20261110/U-261；画面按钮；C14
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：画面按钮跳转返回栈深度及窗口页边界。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-595-SCREEN-PARENT

<a id="q-s00-cand-599-screen-ui"></a>

### Q-S00-CAND-599-SCREEN-UI

- 问题：画面按钮打开窗口的列表如何由窗口、键盘及Logo节点填充？
- 来源/归属：CAND-20261110/U-262；画面按钮；C14
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：画面按钮打开窗口的列表如何由窗口、键盘及Logo节点填充。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-600-screen-ui"></a>

### Q-S00-CAND-600-SCREEN-UI

- 问题：画面按钮监视与操作地址相同的同步及禁用规则？
- 来源/归属：CAND-20261110/U-263；画面按钮；C14
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：画面按钮监视与操作地址相同的同步及禁用规则。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-601-screen-parent"></a>

### Q-S00-CAND-601-SCREEN-PARENT

- 问题：画面按钮候选U-264的分层问题？
- 来源/归属：CAND-20261110/U-264；画面按钮；C14
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：画面按钮候选U-264的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-602-SCREEN-UI、Q-S00-CAND-603-SCREEN-UI

<a id="q-s00-cand-602-screen-ui"></a>

### Q-S00-CAND-602-SCREEN-UI

- 问题：画面按钮显现位控制展开后的字段与启用？
- 来源/归属：CAND-20261110/U-264；画面按钮；C14
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：画面按钮显现位控制展开后的字段与启用。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-601-SCREEN-PARENT

<a id="q-s00-cand-603-screen-ui"></a>

### Q-S00-CAND-603-SCREEN-UI

- 问题：画面按钮触控位控制展开后的字段与启用？
- 来源/归属：CAND-20261110/U-264；画面按钮；C14
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：画面按钮触控位控制展开后的字段与启用。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-601-SCREEN-PARENT

<a id="q-s00-cand-604-screen-parent"></a>

### Q-S00-CAND-604-SCREEN-PARENT

- 问题：画面按钮候选U-265的分层问题？
- 来源/归属：CAND-20261110/U-265；画面按钮；C14
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：画面按钮候选U-265的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-605-SCREEN-UI、Q-S00-CAND-606-SCREEN-UI

<a id="q-s00-cand-605-screen-ui"></a>

### Q-S00-CAND-605-SCREEN-UI

- 问题：画面按钮宏组真实enabled与绑定列表？
- 来源/归属：CAND-20261110/U-265；画面按钮；C14
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：画面按钮宏组真实enabled与绑定列表。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-604-SCREEN-PARENT

<a id="q-s00-cand-606-screen-ui"></a>

### Q-S00-CAND-606-SCREEN-UI

- 问题：画面按钮宏模板编辑及保存流程？
- 来源/归属：CAND-20261110/U-265；画面按钮；C14
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：画面按钮宏模板编辑及保存流程。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-604-SCREEN-PARENT

<a id="q-s00-cand-607-screen-parent"></a>

### Q-S00-CAND-607-SCREEN-PARENT

- 问题：画面按钮候选U-266的分层问题？
- 来源/归属：CAND-20261110/U-266；画面按钮；C14
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：画面按钮候选U-266的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-608-SCREEN-UI、Q-S00-CAND-609-SCREEN-LANGUAGE

<a id="q-s00-cand-608-screen-ui"></a>

### Q-S00-CAND-608-SCREEN-UI

- 问题：画面按钮文本闪烁真实enabled和启用条件？
- 来源/归属：CAND-20261110/U-266；画面按钮；C14
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：画面按钮文本闪烁真实enabled和启用条件。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-607-SCREEN-PARENT

<a id="q-s00-cand-609-screen-language"></a>

### Q-S00-CAND-609-SCREEN-LANGUAGE

- 问题：画面按钮同文及多语言条目和文本保存？
- 来源/归属：CAND-20261110/U-266；画面按钮；C14
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立中文单语及中文/English/第三种语言工程，给各状态各语言写不同文本。
- 刺激：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 对象：画面按钮同文及多语言条目和文本保存。
- 预期观察：确认语言列表来源、索引及草稿规则；运行切换是否正确仍需独立E观察。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-607-SCREEN-PARENT

<a id="q-s00-cand-610-screen-parent"></a>

### Q-S00-CAND-610-SCREEN-PARENT

- 问题：画面按钮候选U-267的分层问题？
- 来源/归属：CAND-20261110/U-267；画面按钮；C14
- 层/状态：P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：画面按钮候选U-267的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-611-SCREEN-GALLERY、Q-S00-CAND-612-SCREEN-GALLERY

<a id="q-s00-cand-611-screen-gallery"></a>

### Q-S00-CAND-611-SCREEN-GALLERY

- 问题：画面按钮系统图库缩略图及选择回填？
- 来源/归属：CAND-20261110/U-267；画面按钮；C14
- 层/状态：U/P/R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立独立工程图库，加入有唯一内容的两张图并设置一个对象引用，保留资源及工程SHA。
- 刺激：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 对象：画面按钮系统图库缩略图及选择回填。
- 预期观察：确定列表、路径、引用、删除恢复及落盘时机；共用图库实现不证明每个控件调用点相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-610-SCREEN-PARENT

<a id="q-s00-cand-612-screen-gallery"></a>

### Q-S00-CAND-612-SCREEN-GALLERY

- 问题：画面按钮工程图库的添加、删除及引用恢复？
- 来源/归属：CAND-20261110/U-267；画面按钮；C14
- 层/状态：U/P/R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立独立工程图库，加入有唯一内容的两张图并设置一个对象引用，保留资源及工程SHA。
- 刺激：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 对象：画面按钮工程图库的添加、删除及引用恢复。
- 预期观察：确定列表、路径、引用、删除恢复及落盘时机；共用图库实现不证明每个控件调用点相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-610-SCREEN-PARENT

<a id="q-s00-cand-613-screen-parent"></a>

### Q-S00-CAND-613-SCREEN-PARENT

- 问题：画面按钮候选U-268的分层问题？
- 来源/归属：CAND-20261110/U-268；画面按钮；C14
- 层/状态：C/D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：画面按钮候选U-268的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-614-SCREEN-ADDRESS、Q-S00-CAND-615-SCREEN-ADDRESS

<a id="q-s00-cand-614-screen-address"></a>

### Q-S00-CAND-614-SCREEN-ADDRESS

- 问题：画面按钮监视地址拾取角色及回填？
- 来源/归属：CAND-20261110/U-268；画面按钮；C14
- 层/状态：D/U/C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定所用地址角色、位宽、连接/型号和通道；保留无连接及已配置工程两份。
- 刺激：从指定控件字段打开拾取器，枚举相应家族并选首末/宽地址边界，确认与取消后核对回填和跨度。 对象：画面按钮监视地址拾取角色及回填。
- 预期观察：取得此调用点的地址角色与可达族；别的控件同款窗口仅作线索，不能代替本入口。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：从指定控件字段打开拾取器，枚举相应家族并选首末/宽地址边界，确认与取消后核对回填和跨度。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-613-SCREEN-PARENT

<a id="q-s00-cand-615-screen-address"></a>

### Q-S00-CAND-615-SCREEN-ADDRESS

- 问题：画面按钮显现触控和通知拾取角色及回填？
- 来源/归属：CAND-20261110/U-268；画面按钮；C14
- 层/状态：D/U/C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定所用地址角色、位宽、连接/型号和通道；保留无连接及已配置工程两份。
- 刺激：从指定控件字段打开拾取器，枚举相应家族并选首末/宽地址边界，确认与取消后核对回填和跨度。 对象：画面按钮显现触控和通知拾取角色及回填。
- 预期观察：取得此调用点的地址角色与可达族；别的控件同款窗口仅作线索，不能代替本入口。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：从指定控件字段打开拾取器，枚举相应家族并选首末/宽地址边界，确认与取消后核对回填和跨度。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-613-SCREEN-PARENT

<a id="q-s00-cand-616-screen-parent"></a>

### Q-S00-CAND-616-SCREEN-PARENT

- 问题：画面按钮候选U-269的分层问题？
- 来源/归属：CAND-20261110/U-269；画面按钮；C14
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：画面按钮候选U-269的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-617-SCREEN-PERMISSION、Q-S00-CAND-618-SCREEN-THRESHOLD、Q-S00-CAND-619-SCREEN-THRESHOLD

<a id="q-s00-cand-617-screen-permission"></a>

### Q-S00-CAND-617-SCREEN-PERMISSION

- 问题：画面按钮显现触控多条件组合？
- 来源/归属：CAND-20261110/U-269；画面按钮；C14
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：选择该控件的显现或触控列，单独建立位、等级、字条件，且对象无重叠遮挡。
- 刺激：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 对象：画面按钮显现触控多条件组合。
- 预期观察：取得显现与触控各自的真值表、组合方式及优先级；不由共用容器推断所有控件相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-616-SCREEN-PARENT

<a id="q-s00-cand-618-screen-threshold"></a>

### Q-S00-CAND-618-SCREEN-THRESHOLD

- 问题：画面按钮最低等级等于阈值边界？
- 来源/归属：CAND-20261110/U-269；画面按钮；C14
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：固定该控件的最低等级阈值，位和字条件关闭或恒定满足。
- 刺激：分别以阈值-1、阈值、阈值+1等级观察显现或触控；显现和触控列分开记录。 对象：画面按钮最低等级等于阈值边界。
- 预期观察：取得最低等级的等值边界与实际权限结果，不由字段名称预设闭开关系。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：分别以阈值-1、阈值、阈值+1等级观察显现或触控；显现和触控列分开记录。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-616-SCREEN-PARENT

<a id="q-s00-cand-619-screen-threshold"></a>

### Q-S00-CAND-619-SCREEN-THRESHOLD

- 问题：画面按钮比较误差算式？
- 来源/归属：CAND-20261110/U-269；画面按钮；C14
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：固定字比较值和误差，位和等级条件恒定满足；整数和浮点分别留样本。
- 刺激：测比较值±容差及邻界，分别测试等于、不等于及0误差，保持其他字段不变。 对象：画面按钮比较误差算式。
- 预期观察：取得容差闭开区间、绝对/相对误差及浮点边界，不提前假设具体公式。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：测比较值±容差及邻界，分别测试等于、不等于及0误差，保持其他字段不变。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-616-SCREEN-PARENT

<a id="q-s00-cand-620-screen-runtime"></a>

### Q-S00-CAND-620-SCREEN-RUNTIME

- 问题：画面按钮通知写值相对按下松开及页面跳转的顺序？
- 来源/归属：CAND-20261110/U-270；画面按钮；C14
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：画面按钮通知写值相对按下松开及页面跳转的顺序。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-621-screen-visibility"></a>

### Q-S00-CAND-621-SCREEN-VISIBILITY

- 问题：画面按钮隐藏云端上传在当前型号是否可达？
- 来源/归属：CAND-20261110/U-271；画面按钮；C14
- 层/状态：D/U；未闭合；atomic
- 适用范围：当前V5.1/AMT已观察的隐藏或缺席状态保留；新增其他机型/版本出现条件转能力矩阵扩展，不阻塞当前已核AMT切片。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定当前AMT和V5.1；B/H隐藏字段只作线索，另列型号或版本扩展范围。
- 刺激：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 对象：画面按钮隐藏云端上传在当前型号是否可达。
- 预期观察：只对搜索边界报告可达、隐藏或未找到；不因字符串存在增加字段，也不推断所有型号不存在。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-622-screen-parent"></a>

### Q-S00-CAND-622-SCREEN-PARENT

- 问题：画面按钮候选U-272的分层问题？
- 来源/归属：CAND-20261110/U-272；画面按钮；C14
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：画面按钮候选U-272的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-623-SCREEN-SOURCE、Q-S00-CAND-624-SCREEN-COMPILE、Q-S00-CAND-625-SCREEN-RUNTIME

<a id="q-s00-cand-623-screen-source"></a>

### Q-S00-CAND-623-SCREEN-SOURCE

- 问题：画面按钮页面窗口及权限的sm布局？
- 来源/归属：CAND-20261110/U-272；画面按钮；C14
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：画面按钮页面窗口及权限的sm布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-622-SCREEN-PARENT

<a id="q-s00-cand-624-screen-compile"></a>

### Q-S00-CAND-624-SCREEN-COMPILE

- 问题：画面按钮设备跳转记录与资源结构？
- 来源/归属：CAND-20261110/U-272；画面按钮；C14
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：画面按钮设备跳转记录与资源结构。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-622-SCREEN-PARENT

<a id="q-s00-cand-625-screen-runtime"></a>

### Q-S00-CAND-625-SCREEN-RUNTIME

- 问题：固件消费画面按钮记录的兼容性？
- 来源/归属：CAND-20261110/U-272；画面按钮；C14
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件消费画面按钮记录的兼容性。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-622-SCREEN-PARENT

<a id="q-s00-cand-626-screen-decision"></a>

### Q-S00-CAND-626-SCREEN-DECISION

- 问题：画面按钮残留窗口名称标签应保留原生瑕疵还是采用显式差异？
- 来源/归属：CAND-20261110/U-273；画面按钮；C14
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留已观察到的原生瑕疵及受影响字段，不改变其事实记录。
- 刺激：比较保留原生表现与采用明确差异的用户影响，登记采用方案及相关验收例。 对象：画面按钮残留窗口名称标签应保留原生瑕疵还是采用显式差异。
- 预期观察：产生显式产品差异记录；不把原生瑕疵改述为正确业务行为。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-SCREEN；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：比较保留原生表现与采用明确差异的用户影响，登记采用方案及相关验收例。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-627-track-parent"></a>

### Q-S00-CAND-627-TRACK-PARENT

- 问题：移动轨迹候选U-274的分层问题？
- 来源/归属：CAND-20261110/U-274；移动轨迹；C21
- 层/状态：D/E/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：移动轨迹候选U-274的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TRACK；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-628-TRACK-UI、Q-S00-CAND-629-TRACK-RUNTIME

<a id="q-s00-cand-628-track-ui"></a>

### Q-S00-CAND-628-TRACK-UI

- 问题：移动轨迹范围类型为地址时最小最大地址行的启用？
- 来源/归属：CAND-20261110/U-274；移动轨迹；C21
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：移动轨迹范围类型为地址时最小最大地址行的启用。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TRACK；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-627-TRACK-PARENT

<a id="q-s00-cand-629-track-runtime"></a>

### Q-S00-CAND-629-TRACK-RUNTIME

- 问题：移动轨迹地址范围边界对运行位置的含义？
- 来源/归属：CAND-20261110/U-274；移动轨迹；C21
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：移动轨迹地址范围边界对运行位置的含义。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TRACK；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-627-TRACK-PARENT

<a id="q-s00-cand-630-track-parent"></a>

### Q-S00-CAND-630-TRACK-PARENT

- 问题：移动轨迹候选U-275的分层问题？
- 来源/归属：CAND-20261110/U-275；移动轨迹；C21
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：移动轨迹候选U-275的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TRACK；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-631-TRACK-BOUND、Q-S00-CAND-632-TRACK-BOUND

<a id="q-s00-cand-631-track-bound"></a>

### Q-S00-CAND-631-TRACK-BOUND

- 问题：移动轨迹移动距离的合法范围？
- 来源/归属：CAND-20261110/U-275；移动轨迹；C21
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：移动轨迹移动距离的合法范围。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TRACK；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-630-TRACK-PARENT

<a id="q-s00-cand-632-track-bound"></a>

### Q-S00-CAND-632-TRACK-BOUND

- 问题：移动轨迹最小最大值的输入和非法值处理？
- 来源/归属：CAND-20261110/U-275；移动轨迹；C21
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：移动轨迹最小最大值的输入和非法值处理。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TRACK；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-630-TRACK-PARENT

<a id="q-s00-cand-633-track-parent"></a>

### Q-S00-CAND-633-TRACK-PARENT

- 问题：移动轨迹候选U-276的分层问题？
- 来源/归属：CAND-20261110/U-276；移动轨迹；C21
- 层/状态：P/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：移动轨迹候选U-276的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TRACK；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-634-TRACK-GALLERY、Q-S00-CAND-635-TRACK-GALLERY

<a id="q-s00-cand-634-track-gallery"></a>

### Q-S00-CAND-634-TRACK-GALLERY

- 问题：移动轨迹系统图库图片选择后的回填？
- 来源/归属：CAND-20261110/U-276；移动轨迹；C21
- 层/状态：U/P/R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立独立工程图库，加入有唯一内容的两张图并设置一个对象引用，保留资源及工程SHA。
- 刺激：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 对象：移动轨迹系统图库图片选择后的回填。
- 预期观察：确定列表、路径、引用、删除恢复及落盘时机；共用图库实现不证明每个控件调用点相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TRACK；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-633-TRACK-PARENT

<a id="q-s00-cand-635-track-gallery"></a>

### Q-S00-CAND-635-TRACK-GALLERY

- 问题：移动轨迹工程图库导入、删除及引用恢复？
- 来源/归属：CAND-20261110/U-276；移动轨迹；C21
- 层/状态：U/P/R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：建立独立工程图库，加入有唯一内容的两张图并设置一个对象引用，保留资源及工程SHA。
- 刺激：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 对象：移动轨迹工程图库导入、删除及引用恢复。
- 预期观察：确定列表、路径、引用、删除恢复及落盘时机；共用图库实现不证明每个控件调用点相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TRACK；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-633-TRACK-PARENT

<a id="q-s00-cand-636-track-visibility"></a>

### Q-S00-CAND-636-TRACK-VISIBILITY

- 问题：移动轨迹轨迹点坐标字段在V5.1是否有可达入口？
- 来源/归属：CAND-20261110/U-277；移动轨迹；C21
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定当前AMT和V5.1；B/H隐藏字段只作线索，另列型号或版本扩展范围。
- 刺激：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 对象：移动轨迹轨迹点坐标字段在V5.1是否有可达入口。
- 预期观察：只对搜索边界报告可达、隐藏或未找到；不因字符串存在增加字段，也不推断所有型号不存在。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TRACK；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-637-track-runtime"></a>

### Q-S00-CAND-637-TRACK-RUNTIME

- 问题：移动轨迹方向字段控制位置运动还是数值增减？
- 来源/归属：CAND-20261110/U-278；移动轨迹；C21
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：移动轨迹方向字段控制位置运动还是数值增减。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TRACK；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-638-track-parent"></a>

### Q-S00-CAND-638-TRACK-PARENT

- 问题：移动轨迹候选U-279的分层问题？
- 来源/归属：CAND-20261110/U-279；移动轨迹；C21
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：移动轨迹候选U-279的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TRACK；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-639-TRACK-OPTIONS、Q-S00-CAND-640-TRACK-OPTIONS、Q-S00-CAND-641-TRACK-UI

<a id="q-s00-cand-639-track-options"></a>

### Q-S00-CAND-639-TRACK-OPTIONS

- 问题：移动轨迹权限等级完整下拉？
- 来源/归属：CAND-20261110/U-279；移动轨迹；C21
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：移动轨迹权限等级完整下拉。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TRACK；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-638-TRACK-PARENT

<a id="q-s00-cand-640-track-options"></a>

### Q-S00-CAND-640-TRACK-OPTIONS

- 问题：移动轨迹权限比较符完整下拉？
- 来源/归属：CAND-20261110/U-279；移动轨迹；C21
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：打开指定下拉或列表，记录当前工程配置及真实Qt count。
- 刺激：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 对象：移动轨迹权限比较符完整下拉。
- 预期观察：形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TRACK；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-638-TRACK-PARENT

<a id="q-s00-cand-641-track-ui"></a>

### Q-S00-CAND-641-TRACK-UI

- 问题：移动轨迹等于与不等于的误差行联动？
- 来源/归属：CAND-20261110/U-279；移动轨迹；C21
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：移动轨迹等于与不等于的误差行联动。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TRACK；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-638-TRACK-PARENT

<a id="q-s00-cand-642-track-parent"></a>

### Q-S00-CAND-642-TRACK-PARENT

- 问题：移动轨迹候选U-280的分层问题？
- 来源/归属：CAND-20261110/U-280；移动轨迹；C21
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：移动轨迹候选U-280的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TRACK；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-643-TRACK-SOURCE、Q-S00-CAND-644-TRACK-COMPILE、Q-S00-CAND-645-TRACK-RUNTIME

<a id="q-s00-cand-643-track-source"></a>

### Q-S00-CAND-643-TRACK-SOURCE

- 问题：移动轨迹源对象字段及图片引用？
- 来源/归属：CAND-20261110/U-280；移动轨迹；C21
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：移动轨迹源对象字段及图片引用。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TRACK；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-642-TRACK-PARENT

<a id="q-s00-cand-644-track-compile"></a>

### Q-S00-CAND-644-TRACK-COMPILE

- 问题：移动轨迹设备运动记录及资源编码？
- 来源/归属：CAND-20261110/U-280；移动轨迹；C21
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：移动轨迹设备运动记录及资源编码。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TRACK；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-642-TRACK-PARENT

<a id="q-s00-cand-645-track-runtime"></a>

### Q-S00-CAND-645-TRACK-RUNTIME

- 问题：固件消费移动轨迹记录的兼容性？
- 来源/归属：CAND-20261110/U-280；移动轨迹；C21
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件消费移动轨迹记录的兼容性。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TRACK；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-642-TRACK-PARENT

<a id="q-s00-cand-646-trend-parent"></a>

### Q-S00-CAND-646-TREND-PARENT

- 问题：趋势图候选U-281的分层问题？
- 来源/归属：CAND-20261110/U-281；趋势图；C23
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：趋势图候选U-281的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-647-TREND-BOUND、Q-S00-CAND-648-TREND-BOUND

<a id="q-s00-cand-647-trend-bound"></a>

### Q-S00-CAND-647-TREND-BOUND

- 问题：趋势图通道最小最大值的键入范围？
- 来源/归属：CAND-20261110/U-281；趋势图；C23
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：趋势图通道最小最大值的键入范围。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-646-TREND-PARENT

<a id="q-s00-cand-648-trend-bound"></a>

### Q-S00-CAND-648-TREND-BOUND

- 问题：趋势图X及Y次分隔数的范围和钳制？
- 来源/归属：CAND-20261110/U-281；趋势图；C23
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：趋势图X及Y次分隔数的范围和钳制。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-646-TREND-PARENT

<a id="q-s00-cand-649-trend-bound"></a>

### Q-S00-CAND-649-TREND-BOUND

- 问题：趋势图采样时间上限及越界回退？
- 来源/归属：CAND-20261110/U-282；趋势图；C23
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：趋势图采样时间上限及越界回退。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-650-trend-parent"></a>

### Q-S00-CAND-650-TREND-PARENT

- 问题：趋势图候选U-283的分层问题？
- 来源/归属：CAND-20261110/U-283；趋势图；C23
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：趋势图候选U-283的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-651-TREND-BOUND、Q-S00-CAND-652-TREND-BOUND

<a id="q-s00-cand-651-trend-bound"></a>

### Q-S00-CAND-651-TREND-BOUND

- 问题：趋势图采样点99999提交后的确切回退值？
- 来源/归属：CAND-20261110/U-283；趋势图；C23
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：趋势图采样点99999提交后的确切回退值。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-650-TREND-PARENT

<a id="q-s00-cand-652-trend-bound"></a>

### Q-S00-CAND-652-TREND-BOUND

- 问题：趋势图通道99及0提交后的确切回退值？
- 来源/归属：CAND-20261110/U-283；趋势图；C23
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：趋势图通道99及0提交后的确切回退值。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-650-TREND-PARENT

<a id="q-s00-cand-653-trend-runtime"></a>

### Q-S00-CAND-653-TREND-RUNTIME

- 问题：趋势图五种线型在设备端的实际点划画法？
- 来源/归属：CAND-20261110/U-284；趋势图；C23
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：趋势图五种线型在设备端的实际点划画法。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-654-trend-parent"></a>

### Q-S00-CAND-654-TREND-PARENT

- 问题：趋势图候选U-285的分层问题？
- 来源/归属：CAND-20261110/U-285；趋势图；C23
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：趋势图候选U-285的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-655-TREND-RUNTIME、Q-S00-CAND-656-TREND-RUNTIME

<a id="q-s00-cand-655-trend-runtime"></a>

### Q-S00-CAND-655-TREND-RUNTIME

- 问题：趋势图从右到左的屏端滚动及绘制方向？
- 来源/归属：CAND-20261110/U-285；趋势图；C23
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：趋势图从右到左的屏端滚动及绘制方向。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-654-TREND-PARENT

<a id="q-s00-cand-656-trend-runtime"></a>

### Q-S00-CAND-656-TREND-RUNTIME

- 问题：趋势图多通道采样的同步方式？
- 来源/归属：CAND-20261110/U-285；趋势图；C23
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：趋势图多通道采样的同步方式。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-654-TREND-PARENT

<a id="q-s00-cand-657-trend-ui"></a>

### Q-S00-CAND-657-TREND-UI

- 问题：趋势图Y显示刻度标识取消后隐藏的精确字段集合？
- 来源/归属：CAND-20261110/U-286；趋势图；C23
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：趋势图Y显示刻度标识取消后隐藏的精确字段集合。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-658-trend-ui"></a>

### Q-S00-CAND-658-TREND-UI

- 问题：趋势图不等于比较符是否同样显示误差行？
- 来源/归属：CAND-20261110/U-287；趋势图；C23
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：趋势图不等于比较符是否同样显示误差行。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-659-trend-parent"></a>

### Q-S00-CAND-659-TREND-PARENT

- 问题：趋势图候选U-288的分层问题？
- 来源/归属：CAND-20261110/U-288；趋势图；C23
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：趋势图候选U-288的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-660-TREND-BOUND、Q-S00-CAND-661-TREND-BOUND

<a id="q-s00-cand-660-trend-bound"></a>

### Q-S00-CAND-660-TREND-BOUND

- 问题：趋势图Y最大值能否键入大于1000？
- 来源/归属：CAND-20261110/U-288；趋势图；C23
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：趋势图Y最大值能否键入大于1000。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-659-TREND-PARENT

<a id="q-s00-cand-661-trend-bound"></a>

### Q-S00-CAND-661-TREND-BOUND

- 问题：趋势图Y最小值是否接受负数？
- 来源/归属：CAND-20261110/U-288；趋势图；C23
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：趋势图Y最小值是否接受负数。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-659-TREND-PARENT

<a id="q-s00-cand-662-trend-visibility"></a>

### Q-S00-CAND-662-TREND-VISIBILITY

- 问题：趋势图旧画点标记及连接线在当前型号是否可达？
- 来源/归属：CAND-20261110/U-289；趋势图；C23
- 层/状态：D/U；未闭合；atomic
- 适用范围：当前V5.1/AMT已观察的隐藏或缺席状态保留；新增其他机型/版本出现条件转能力矩阵扩展，不阻塞当前已核AMT切片。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定当前AMT和V5.1；B/H隐藏字段只作线索，另列型号或版本扩展范围。
- 刺激：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 对象：趋势图旧画点标记及连接线在当前型号是否可达。
- 预期观察：只对搜索边界报告可达、隐藏或未找到；不因字符串存在增加字段，也不推断所有型号不存在。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-663-trend-parent"></a>

### Q-S00-CAND-663-TREND-PARENT

- 问题：趋势图候选U-290的分层问题？
- 来源/归属：CAND-20261110/U-290；趋势图；C23
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：趋势图候选U-290的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-664-TREND-SOURCE、Q-S00-CAND-665-TREND-COMPILE、Q-S00-CAND-666-TREND-RUNTIME

<a id="q-s00-cand-664-trend-source"></a>

### Q-S00-CAND-664-TREND-SOURCE

- 问题：趋势图源通道、触发及显示字段布局？
- 来源/归属：CAND-20261110/U-290；趋势图；C23
- 层/状态：P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：趋势图源通道、触发及显示字段布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-663-TREND-PARENT

<a id="q-s00-cand-665-trend-compile"></a>

### Q-S00-CAND-665-TREND-COMPILE

- 问题：趋势图设备记录与采样资源结构？
- 来源/归属：CAND-20261110/U-290；趋势图；C23
- 层/状态：C；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：趋势图设备记录与采样资源结构。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-663-TREND-PARENT

<a id="q-s00-cand-666-trend-runtime"></a>

### Q-S00-CAND-666-TREND-RUNTIME

- 问题：固件消费趋势图记录的兼容性？
- 来源/归属：CAND-20261110/U-290；趋势图；C23
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：固件消费趋势图记录的兼容性。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-663-TREND-PARENT

<a id="q-s00-cand-667-trend-help"></a>

### Q-S00-CAND-667-TREND-HELP

- 问题：趋势图帮助按钮是否直达对应CHM成员？
- 来源/归属：CAND-20261110/U-291；趋势图；C23
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：确认原CHM路径/哈希及成员存在，记录完整安装依赖和私有副本环境差异。
- 刺激：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 对象：趋势图帮助按钮是否直达对应CHM成员。
- 预期观察：确认目标成员或有界失败；CHM成员存在不能替代按钮直达事实。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-668-trend-parent"></a>

### Q-S00-CAND-668-TREND-PARENT

- 问题：趋势图候选U-292的分层问题？
- 来源/归属：CAND-20261110/U-292；趋势图；C23
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：趋势图候选U-292的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-669-TREND-RUNTIME、Q-S00-CAND-670-TREND-RUNTIME

<a id="q-s00-cand-669-trend-runtime"></a>

### Q-S00-CAND-669-TREND-RUNTIME

- 问题：趋势图触发位由位按钮置位后的采样时序？
- 来源/归属：CAND-20261110/U-292；趋势图；C23
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：趋势图触发位由位按钮置位后的采样时序。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-668-TREND-PARENT

<a id="q-s00-cand-670-trend-runtime"></a>

### Q-S00-CAND-670-TREND-RUNTIME

- 问题：趋势图清除位由位按钮置位后的曲线及采样状态？
- 来源/归属：CAND-20261110/U-292；趋势图；C23
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：趋势图清除位由位按钮置位后的曲线及采样状态。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TREND；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-668-TREND-PARENT

<a id="q-s00-cand-671-recipe-parent"></a>

### Q-S00-CAND-671-RECIPE-PARENT

- 问题：配方显示器候选U-293的分层问题？
- 来源/归属：CAND-20261110/U-293；配方显示器；C31
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：配方显示器候选U-293的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-RECIPE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-672-RECIPE-UI、Q-S00-CAND-673-RECIPE-DIAGNOSTIC

<a id="q-s00-cand-672-recipe-ui"></a>

### Q-S00-CAND-672-RECIPE-UI

- 问题：配方显示编辑配方开关的真实enabled与展开字段？
- 来源/归属：CAND-20261110/U-293；配方显示器；C31
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：配方显示编辑配方开关的真实enabled与展开字段。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-RECIPE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-671-RECIPE-PARENT

<a id="q-s00-cand-673-recipe-diagnostic"></a>

### Q-S00-CAND-673-RECIPE-DIAGNOSTIC

- 问题：配方编辑开启伴随窗口自退是否可复现？
- 来源/归属：CAND-20261110/U-293；配方显示器；C31
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留发生问题的源工程、原生身份、私有桌面PID、事件类型和退出码。
- 刺激：仅在自有实例串行最小复现，区分直接点击、Qt等价事件及重试；核对已引用截图来自哪次成功会话。 对象：配方编辑开启伴随窗口自退是否可复现。
- 预期观察：确定采集可复现性或留下有界异常；未证实原因不得写成试用状态或其他程序冲突，更不要求复刻崩溃。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-RECIPE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有实例串行最小复现，区分直接点击、Qt等价事件及重试；核对已引用截图来自哪次成功会话。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-671-RECIPE-PARENT

<a id="q-s00-cand-674-recipe-render"></a>

### Q-S00-CAND-674-RECIPE-RENDER

- 问题：配方表格绘制阈值按宽度还是面积及最小阈值？
- 来源/归属：CAND-20261110/U-294；配方显示器；C31
- 层/状态：R；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：配方表格绘制阈值按宽度还是面积及最小阈值。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-RECIPE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-675-recipe-ui"></a>

### Q-S00-CAND-675-RECIPE-UI

- 问题：无配方配置时配方显示器入口是否拦截及提示？
- 来源/归属：CAND-20261110/U-295；配方显示器；C31
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：无配方配置时配方显示器入口是否拦截及提示。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-RECIPE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-676-recipe-ui"></a>

### Q-S00-CAND-676-RECIPE-UI

- 问题：配方标题色块与pbtn_TtextColor的对应及回写？
- 来源/归属：CAND-20261110/U-296；配方显示器；C31
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：配方标题色块与pbtn_TtextColor的对应及回写。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-RECIPE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-677-recipe-parent"></a>

### Q-S00-CAND-677-RECIPE-PARENT

- 问题：配方显示器候选U-297的分层问题？
- 来源/归属：CAND-20261110/U-297；配方显示器；C31
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：配方显示器候选U-297的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-RECIPE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-678-RECIPE-PERSIST、Q-S00-CAND-679-RECIPE-UI、Q-S00-CAND-680-RECIPE-HELP

<a id="q-s00-cand-678-recipe-persist"></a>

### Q-S00-CAND-678-RECIPE-PERSIST

- 问题：配方显示取消编辑后重新打开的字段？
- 来源/归属：CAND-20261110/U-297；配方显示器；C31
- 层/状态：U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留可辨识原值和工程副本，固定当前语言、状态、类型及对象ID。
- 刺激：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 对象：配方显示取消编辑后重新打开的字段。
- 预期观察：取得提交、取消和休眠草稿的保存寿命；区分ksp草稿与原厂sm实际保存字段。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-RECIPE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-677-RECIPE-PARENT

<a id="q-s00-cand-679-recipe-ui"></a>

### Q-S00-CAND-679-RECIPE-UI

- 问题：配方显示对象右键菜单实际项集？
- 来源/归属：CAND-20261110/U-297；配方显示器；C31
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：配方显示对象右键菜单实际项集。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-RECIPE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-677-RECIPE-PARENT

<a id="q-s00-cand-680-recipe-help"></a>

### Q-S00-CAND-680-RECIPE-HELP

- 问题：配方显示帮助按钮实际CHM目标？
- 来源/归属：CAND-20261110/U-297；配方显示器；C31
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：确认原CHM路径/哈希及成员存在，记录完整安装依赖和私有副本环境差异。
- 刺激：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 对象：配方显示帮助按钮实际CHM目标。
- 预期观察：确认目标成员或有界失败；CHM成员存在不能替代按钮直达事实。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-RECIPE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-677-RECIPE-PARENT

<a id="q-s00-cand-681-recipe-parent"></a>

### Q-S00-CAND-681-RECIPE-PARENT

- 问题：配方显示器候选U-298的分层问题？
- 来源/归属：CAND-20261110/U-298；A02配方配置/C31配方显示；C31
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：配方显示器候选U-298的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-RECIPE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-682-RECIPE-UI、Q-S00-CAND-683-RECIPE-BOUND、Q-S00-CAND-684-RECIPE-PERSIST

<a id="q-s00-cand-682-recipe-ui"></a>

### Q-S00-CAND-682-RECIPE-UI

- 问题：多配方配置时配方显示表与选择的对应？
- 来源/归属：CAND-20261110/U-298；A02配方配置/C31配方显示；C31
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：多配方配置时配方显示表与选择的对应。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-RECIPE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-681-RECIPE-PARENT

<a id="q-s00-cand-683-recipe-bound"></a>

### Q-S00-CAND-683-RECIPE-BOUND

- 问题：配方显示32位数据的编辑及范围？
- 来源/归属：CAND-20261110/U-298；A02配方配置/C31配方显示；C31
- 层/状态：D/U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：配方显示32位数据的编辑及范围。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-RECIPE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-681-RECIPE-PARENT

<a id="q-s00-cand-684-recipe-persist"></a>

### Q-S00-CAND-684-RECIPE-PERSIST

- 问题：配方CSV导入导出的维度、编码及字段回写？
- 来源/归属：CAND-20261110/U-298；A02配方配置/C31配方显示；C31
- 层/状态：U/P；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留可辨识原值和工程副本，固定当前语言、状态、类型及对象ID。
- 刺激：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 对象：配方CSV导入导出的维度、编码及字段回写。
- 预期观察：取得提交、取消和休眠草稿的保存寿命；区分ksp草稿与原厂sm实际保存字段。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-RECIPE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-681-RECIPE-PARENT

<a id="q-s00-cand-685-recipe-ui"></a>

### Q-S00-CAND-685-RECIPE-UI

- 问题：配方显示位字通知的启用、展开及互斥？
- 来源/归属：CAND-20261110/U-299；配方显示器；C31
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：配方显示位字通知的启用、展开及互斥。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-RECIPE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。

<a id="q-s00-cand-686-recipe-parent"></a>

### Q-S00-CAND-686-RECIPE-PARENT

- 问题：配方显示器候选U-300的分层问题？
- 来源/归属：CAND-20261110/U-300；配方显示器；C31
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：配方显示器候选U-300的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-RECIPE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-687-RECIPE-DIAGNOSTIC、Q-S00-CAND-688-RECIPE-UI

<a id="q-s00-cand-687-recipe-diagnostic"></a>

### Q-S00-CAND-687-RECIPE-DIAGNOSTIC

- 问题：配方显示自退与重试截图的证据上下文？
- 来源/归属：CAND-20261110/U-300；配方显示器；C31
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：保留发生问题的源工程、原生身份、私有桌面PID、事件类型和退出码。
- 刺激：仅在自有实例串行最小复现，区分直接点击、Qt等价事件及重试；核对已引用截图来自哪次成功会话。 对象：配方显示自退与重试截图的证据上下文。
- 预期观察：确定采集可复现性或留下有界异常；未证实原因不得写成试用状态或其他程序冲突，更不要求复刻崩溃。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-RECIPE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅在自有实例串行最小复现，区分直接点击、Qt等价事件及重试；核对已引用截图来自哪次成功会话。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-686-RECIPE-PARENT

<a id="q-s00-cand-688-recipe-ui"></a>

### Q-S00-CAND-688-RECIPE-UI

- 问题：配方按钮或复选框使用等价焦点事件后的稳定联动？
- 来源/归属：CAND-20261110/U-300；配方显示器；C31
- 层/状态：D/U；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：配方按钮或复选框使用等价焦点事件后的稳定联动。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-RECIPE；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-686-RECIPE-PARENT

<a id="q-s00-cand-689-text-parent"></a>

### Q-S00-CAND-689-TEXT-PARENT

- 问题：静态文本候选U-301的分层问题？
- 来源/归属：CAND-20261110/U-301；静态文本；C01
- 层/状态：D/P/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：静态文本候选U-301的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT；静态文本报告明确字号/字距等实现保护范围不是已测原生极限。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-690-TEXT-BOUND、Q-S00-CAND-691-TEXT-BOUND、Q-S00-CAND-692-TEXT-BOUND、Q-S00-CAND-693-TEXT-BOUND

<a id="q-s00-cand-690-text-bound"></a>

### Q-S00-CAND-690-TEXT-BOUND

- 问题：静态文本透明度原生输入上下限？
- 来源/归属：CAND-20261110/U-301；静态文本；C01
- 层/状态：D/U/P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：静态文本透明度原生输入上下限。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT；静态文本报告明确字号/字距等实现保护范围不是已测原生极限。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-689-TEXT-PARENT

<a id="q-s00-cand-691-text-bound"></a>

### Q-S00-CAND-691-TEXT-BOUND

- 问题：静态文本字距负数及上下限？
- 来源/归属：CAND-20261110/U-301；静态文本；C01
- 层/状态：D/U/P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：静态文本字距负数及上下限。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT；静态文本报告明确字号/字距等实现保护范围不是已测原生极限。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-689-TEXT-PARENT

<a id="q-s00-cand-692-text-bound"></a>

### Q-S00-CAND-692-TEXT-BOUND

- 问题：静态文本字号小数及上下限？
- 来源/归属：CAND-20261110/U-301；静态文本；C01
- 层/状态：D/U/P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：静态文本字号小数及上下限。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT；静态文本报告明确字号/字距等实现保护范围不是已测原生极限。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-689-TEXT-PARENT

<a id="q-s00-cand-693-text-bound"></a>

### Q-S00-CAND-693-TEXT-BOUND

- 问题：静态文本比较值空值、负数、小数及数值边界？
- 来源/归属：CAND-20261110/U-301；静态文本；C01
- 层/状态：D/U/P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。
- 刺激：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 对象：静态文本比较值空值、负数、小数及数值边界。
- 预期观察：记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT；静态文本报告明确字号/字距等实现保护范围不是已测原生极限。
- 下一探针：清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-689-TEXT-PARENT

<a id="q-s00-cand-694-text-parent"></a>

### Q-S00-CAND-694-TEXT-PARENT

- 问题：静态文本候选U-302的分层问题？
- 来源/归属：CAND-20261110/U-302；静态文本；C01
- 层/状态：D/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：静态文本候选U-302的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT；13种默认100×100填充容差及指定粗体帧有R证据；非方形、透明全域和跨分辨率未穷尽。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-695-TEXT-RENDER、Q-S00-CAND-696-TEXT-RENDER、Q-S00-CAND-697-TEXT-VISIBILITY

<a id="q-s00-cand-695-text-render"></a>

### Q-S00-CAND-695-TEXT-RENDER

- 问题：静态文本13填充在非方形中的精确插值？
- 来源/归属：CAND-20261110/U-302；静态文本；C01
- 层/状态：R；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：静态文本13填充在非方形中的精确插值。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT；13种默认100×100填充容差及指定粗体帧有R证据；非方形、透明全域和跨分辨率未穷尽。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-694-TEXT-PARENT

<a id="q-s00-cand-696-text-render"></a>

### Q-S00-CAND-696-TEXT-RENDER

- 问题：静态文本透明叠加的像素公式？
- 来源/归属：CAND-20261110/U-302；静态文本；C01
- 层/状态：R；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：静态文本透明叠加的像素公式。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT；13种默认100×100填充容差及指定粗体帧有R证据；非方形、透明全域和跨分辨率未穷尽。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-694-TEXT-PARENT

<a id="q-s00-cand-697-text-visibility"></a>

### Q-S00-CAND-697-TEXT-VISIBILITY

- 问题：静态文本跨分辨率外观的适用范围？
- 来源/归属：CAND-20261110/U-302；静态文本；C01
- 层/状态：D/U；部分历史补证，余项未闭合；atomic
- 适用范围：当前V5.1/AMT已观察的隐藏或缺席状态保留；新增其他机型/版本出现条件转能力矩阵扩展，不阻塞当前已核AMT切片。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定当前AMT和V5.1；B/H隐藏字段只作线索，另列型号或版本扩展范围。
- 刺激：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 对象：静态文本跨分辨率外观的适用范围。
- 预期观察：只对搜索边界报告可达、隐藏或未找到；不因字符串存在增加字段，也不推断所有型号不存在。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT；13种默认100×100填充容差及指定粗体帧有R证据；非方形、透明全域和跨分辨率未穷尽。
- 下一探针：在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-694-TEXT-PARENT

<a id="q-s00-cand-698-text-parent"></a>

### Q-S00-CAND-698-TEXT-PARENT

- 问题：静态文本候选U-303的分层问题？
- 来源/归属：CAND-20261110/U-303；静态文本；C01
- 层/状态：E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：静态文本候选U-303的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT、EV-S00-CAND-BIT-DEVICE；需采用0.4后type28多语言/显现编译补证；旧报告设备多语言拒绝文字仅为历史，E切换仍未验。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-699-TEXT-SOURCE、Q-S00-CAND-700-TEXT-RUNTIME

<a id="q-s00-cand-699-text-source"></a>

### Q-S00-CAND-699-TEXT-SOURCE

- 问题：静态文本多语言文本在原厂工程中的保存结构？
- 来源/归属：CAND-20261110/U-303；静态文本；C01
- 层/状态：P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：静态文本多语言文本在原厂工程中的保存结构。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT、EV-S00-CAND-BIT-DEVICE；需采用0.4后type28多语言/显现编译补证；旧报告设备多语言拒绝文字仅为历史，E切换仍未验。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-698-TEXT-PARENT

<a id="q-s00-cand-700-text-runtime"></a>

### Q-S00-CAND-700-TEXT-RUNTIME

- 问题：静态文本运行语言切换和文本选择规则？
- 来源/归属：CAND-20261110/U-303；静态文本；C01
- 层/状态：E；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：静态文本运行语言切换和文本选择规则。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT、EV-S00-CAND-BIT-DEVICE；需采用0.4后type28多语言/显现编译补证；旧报告设备多语言拒绝文字仅为历史，E切换仍未验。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-698-TEXT-PARENT

<a id="q-s00-cand-701-text-parent"></a>

### Q-S00-CAND-701-TEXT-PARENT

- 问题：静态文本候选U-304的分层问题？
- 来源/归属：CAND-20261110/U-304；静态文本；C01
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：静态文本候选U-304的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-702-TEXT-PERMISSION、Q-S00-CAND-703-TEXT-THRESHOLD、Q-S00-CAND-704-TEXT-THRESHOLD

<a id="q-s00-cand-702-text-permission"></a>

### Q-S00-CAND-702-TEXT-PERMISSION

- 问题：静态文本多个显现条件的组合？
- 来源/归属：CAND-20261110/U-304；静态文本；C01
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：选择该控件的显现或触控列，单独建立位、等级、字条件，且对象无重叠遮挡。
- 刺激：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 对象：静态文本多个显现条件的组合。
- 预期观察：取得显现与触控各自的真值表、组合方式及优先级；不由共用容器推断所有控件相同。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-701-TEXT-PARENT

<a id="q-s00-cand-703-text-threshold"></a>

### Q-S00-CAND-703-TEXT-THRESHOLD

- 问题：静态文本等级等于阈值边界？
- 来源/归属：CAND-20261110/U-304；静态文本；C01
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：固定该控件的最低等级阈值，位和字条件关闭或恒定满足。
- 刺激：分别以阈值-1、阈值、阈值+1等级观察显现或触控；显现和触控列分开记录。 对象：静态文本等级等于阈值边界。
- 预期观察：取得最低等级的等值边界与实际权限结果，不由字段名称预设闭开关系。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：分别以阈值-1、阈值、阈值+1等级观察显现或触控；显现和触控列分开记录。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-701-TEXT-PARENT

<a id="q-s00-cand-704-text-threshold"></a>

### Q-S00-CAND-704-TEXT-THRESHOLD

- 问题：静态文本误差比较公式？
- 来源/归属：CAND-20261110/U-304；静态文本；C01
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：固定字比较值和误差，位和等级条件恒定满足；整数和浮点分别留样本。
- 刺激：测比较值±容差及邻界，分别测试等于、不等于及0误差，保持其他字段不变。 对象：静态文本误差比较公式。
- 预期观察：取得容差闭开区间、绝对/相对误差及浮点边界，不提前假设具体公式。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：测比较值±容差及邻界，分别测试等于、不等于及0误差，保持其他字段不变。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-701-TEXT-PARENT

<a id="q-s00-cand-705-text-parent"></a>

### Q-S00-CAND-705-TEXT-PARENT

- 问题：静态文本候选U-305的分层问题？
- 来源/归属：CAND-20261110/U-305；静态文本；C01
- 层/状态：D/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：静态文本候选U-305的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT；Lite入口取消、属性与双击测试已覆盖；原生同链及帮助直达必须按本条分别核实。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-706-TEXT-UI、Q-S00-CAND-707-TEXT-HELP、Q-S00-CAND-708-TEXT-UI

<a id="q-s00-cand-706-text-ui"></a>

### Q-S00-CAND-706-TEXT-UI

- 问题：静态文本新建取消后是否退出绘制模式？
- 来源/归属：CAND-20261110/U-305；静态文本；C01
- 层/状态：D/U；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：静态文本新建取消后是否退出绘制模式。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT；Lite入口取消、属性与双击测试已覆盖；原生同链及帮助直达必须按本条分别核实。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-705-TEXT-PARENT

<a id="q-s00-cand-707-text-help"></a>

### Q-S00-CAND-707-TEXT-HELP

- 问题：静态文本帮助按钮是否直达目标页？
- 来源/归属：CAND-20261110/U-305；静态文本；C01
- 层/状态：D/U；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：确认原CHM路径/哈希及成员存在，记录完整安装依赖和私有副本环境差异。
- 刺激：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 对象：静态文本帮助按钮是否直达目标页。
- 预期观察：确认目标成员或有界失败；CHM成员存在不能替代按钮直达事实。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT；Lite入口取消、属性与双击测试已覆盖；原生同链及帮助直达必须按本条分别核实。
- 下一探针：仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-705-TEXT-PARENT

<a id="q-s00-cand-708-text-ui"></a>

### Q-S00-CAND-708-TEXT-UI

- 问题：静态文本属性菜单和双击是否走同一窗口与事务？
- 来源/归属：CAND-20261110/U-305；静态文本；C01
- 层/状态：D/U；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。
- 刺激：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 对象：静态文本属性菜单和双击是否走同一窗口与事务。
- 预期观察：取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT；Lite入口取消、属性与双击测试已覆盖；原生同链及帮助直达必须按本条分别核实。
- 下一探针：对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-705-TEXT-PARENT

<a id="q-s00-cand-709-text-parent"></a>

### Q-S00-CAND-709-TEXT-PARENT

- 问题：静态文本候选U-306的分层问题？
- 来源/归属：CAND-20261110/U-306；静态文本；C01
- 层/状态：C/D/E/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：静态文本候选U-306的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT、EV-S00-CAND-BIT-DEVICE；位按钮后续报告已有139配置与1689/1724族纯C++地址编码；PC/屏端通信、字序及浮点字节序未因此关闭。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-710-TEXT-ADDRESS、Q-S00-CAND-711-TEXT-RUNTIME、Q-S00-CAND-712-TEXT-RUNTIME

<a id="q-s00-cand-710-text-address"></a>

### Q-S00-CAND-710-TEXT-ADDRESS

- 问题：完整地址家族与PLC或芯片型号的联动？
- 来源/归属：CAND-20261110/U-306；静态文本；C01
- 层/状态：D/U/C；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定所用地址角色、位宽、连接/型号和通道；保留无连接及已配置工程两份。
- 刺激：从指定控件字段打开拾取器，枚举相应家族并选首末/宽地址边界，确认与取消后核对回填和跨度。 对象：完整地址家族与PLC或芯片型号的联动。
- 预期观察：取得此调用点的地址角色与可达族；别的控件同款窗口仅作线索，不能代替本入口。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT、EV-S00-CAND-BIT-DEVICE；位按钮后续报告已有139配置与1689/1724族纯C++地址编码；PC/屏端通信、字序及浮点字节序未因此关闭。
- 下一探针：从指定控件字段打开拾取器，枚举相应家族并选首末/宽地址边界，确认与取消后核对回填和跨度。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-709-TEXT-PARENT

<a id="q-s00-cand-711-text-runtime"></a>

### Q-S00-CAND-711-TEXT-RUNTIME

- 问题：32位字值的字序？
- 来源/归属：CAND-20261110/U-306；静态文本；C01
- 层/状态：E；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：32位字值的字序。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT、EV-S00-CAND-BIT-DEVICE；位按钮后续报告已有139配置与1689/1724族纯C++地址编码；PC/屏端通信、字序及浮点字节序未因此关闭。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-709-TEXT-PARENT

<a id="q-s00-cand-712-text-runtime"></a>

### Q-S00-CAND-712-TEXT-RUNTIME

- 问题：32位浮点数的字节序？
- 来源/归属：CAND-20261110/U-306；静态文本；C01
- 层/状态：E；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：32位浮点数的字节序。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT、EV-S00-CAND-BIT-DEVICE；位按钮后续报告已有139配置与1689/1724族纯C++地址编码；PC/屏端通信、字序及浮点字节序未因此关闭。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-709-TEXT-PARENT

<a id="q-s00-cand-713-text-parent"></a>

### Q-S00-CAND-713-TEXT-PARENT

- 问题：静态文本候选U-307的分层问题？
- 来源/归属：CAND-20261110/U-307；静态文本；C01
- 层/状态：C/E/P；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：静态文本候选U-307的分层问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT、EV-S00-CAND-BIT-DEVICE；type28及静态背景编译、typed源已有后续P/C；特定样例现场显示不推出全固件字段兼容。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-714-TEXT-SOURCE、Q-S00-CAND-715-TEXT-COMPILE、Q-S00-CAND-716-TEXT-RUNTIME

<a id="q-s00-cand-714-text-source"></a>

### Q-S00-CAND-714-TEXT-SOURCE

- 问题：静态文本原生源外观与权限字段布局？
- 来源/归属：CAND-20261110/U-307；静态文本；C01
- 层/状态：P；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。
- 刺激：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 对象：静态文本原生源外观与权限字段布局。
- 预期观察：定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT、EV-S00-CAND-BIT-DEVICE；type28及静态背景编译、typed源已有后续P/C；特定样例现场显示不推出全固件字段兼容。
- 下一探针：逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-713-TEXT-PARENT

<a id="q-s00-cand-715-text-compile"></a>

### Q-S00-CAND-715-TEXT-COMPILE

- 问题：静态文本type28及静态背景的下载结构？
- 来源/归属：CAND-20261110/U-307；静态文本；C01
- 层/状态：C；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。
- 刺激：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 对象：静态文本type28及静态背景的下载结构。
- 预期观察：形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT、EV-S00-CAND-BIT-DEVICE；type28及静态背景编译、typed源已有后续P/C；特定样例现场显示不推出全固件字段兼容。
- 下一探针：单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-713-TEXT-PARENT

<a id="q-s00-cand-716-text-runtime"></a>

### Q-S00-CAND-716-TEXT-RUNTIME

- 问题：目标固件消费静态文本记录的兼容性？
- 来源/归属：CAND-20261110/U-307；静态文本；C01
- 层/状态：E；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：目标固件消费静态文本记录的兼容性。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-TEXT、EV-S00-CAND-TEXT、EV-S00-CAND-BIT-DEVICE；type28及静态背景编译、typed源已有后续P/C；特定样例现场显示不推出全固件字段兼容。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-713-TEXT-PARENT

<a id="q-s00-cand-717-lamp-parent"></a>

### Q-S00-CAND-717-LAMP-PARENT

- 问题：指示灯状态范围与超过两态的显示问题？
- 来源/归属：CAND-20261110/U-181；指示灯；C04
- 层/状态：D/E/R/U；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：指示灯状态范围与超过两态的显示问题。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；旧核销的2..32/0..N-1事实关闭继续有效；报告还解释状态图序、新增默认及5→2→5删除，但E渲染分支不闭合。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 子项：Q-S00-CAND-718-LAMP-FACT、Q-S00-CAND-719-LAMP-PARENT

<a id="q-s00-cand-718-lamp-fact"></a>

### Q-S00-CAND-718-LAMP-FACT

- 问题：真实切入多态后状态范围2至32及外观索引0至N减1？
- 来源/归属：CAND-20261110/U-181；指示灯；C04
- 层/状态：D/U；沿用事实关闭；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：复用旧核销中相同V5.1身份及AMT范围。
- 刺激：核对原核销引用与现存证据身份；本任务不重跑原生UI或硬件。 对象：真实切入多态后状态范围2至32及外观索引0至N减1。
- 预期观察：只沿用已关闭事实及其版本/型号/层，不扩大为整个控件或E通过。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP、EV-S00-CAND-PRIOR；旧核销的2..32/0..N-1事实关闭继续有效；报告还解释状态图序、新增默认及5→2→5删除，但E渲染分支不闭合。
- 下一探针：核对原核销引用与现存证据身份；本任务不重跑原生UI或硬件。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 沿用问题身份：Q-LAMP-COUNT-001
- 父项：Q-S00-CAND-717-LAMP-PARENT
- 关闭范围：{"basis": "旧核销已有事实关闭", "version": "V5.1所核AMT", "layers": ["D", "U"], "conclusion": "真实进入多态后2..32；外观索引0..N-1。setChecked残留256及步进到27均非上限依据。", "not_covered": "多态屏端图像、闪烁、语言和E。"}

<a id="q-s00-cand-719-lamp-parent"></a>

### Q-S00-CAND-719-LAMP-PARENT

- 问题：指示灯大于两态的外观及屏端显示分支？
- 来源/归属：CAND-20261110/U-181；指示灯；C04
- 层/状态：E/R；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：指示灯大于两态的外观及屏端显示分支。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；旧核销的2..32/0..N-1事实关闭继续有效；报告还解释状态图序、新增默认及5→2→5删除，但E渲染分支不闭合。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 沿用问题身份：Q-LAMP-RENDER-001
- 父项：Q-S00-CAND-717-LAMP-PARENT
- 子项：Q-S00-CAND-720-LAMP-RENDER、Q-S00-CAND-721-LAMP-RUNTIME、Q-S00-CAND-722-LAMP-RUNTIME、Q-S00-CAND-723-LAMP-RUNTIME

<a id="q-s00-cand-720-lamp-render"></a>

### Q-S00-CAND-720-LAMP-RENDER

- 问题：指示灯大于两态时外观页逐态选择与预览？
- 来源/归属：CAND-20261110/U-181；指示灯；C04
- 层/状态：R；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。
- 刺激：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 对象：指示灯大于两态时外观页逐态选择与预览。
- 预期观察：给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；旧核销的2..32/0..N-1事实关闭继续有效；报告还解释状态图序、新增默认及5→2→5删除，但E渲染分支不闭合。
- 下一探针：对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-719-LAMP-PARENT

<a id="q-s00-cand-721-lamp-runtime"></a>

### Q-S00-CAND-721-LAMP-RUNTIME

- 问题：指示灯五状态LW20依次0至4及归零的实际显示？
- 来源/归属：CAND-20261110/U-181；指示灯；C04
- 层/状态：E；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：指示灯五状态LW20依次0至4及归零的实际显示。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；旧核销的2..32/0..N-1事实关闭继续有效；报告还解释状态图序、新增默认及5→2→5删除，但E渲染分支不闭合。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-719-LAMP-PARENT

<a id="q-s00-cand-722-lamp-runtime"></a>

### Q-S00-CAND-722-LAMP-RUNTIME

- 问题：指示灯多态闪烁的实际可用范围？
- 来源/归属：CAND-20261110/U-181；指示灯；C04
- 层/状态：E；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：指示灯多态闪烁的实际可用范围。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；旧核销的2..32/0..N-1事实关闭继续有效；报告还解释状态图序、新增默认及5→2→5删除，但E渲染分支不闭合。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-719-LAMP-PARENT

<a id="q-s00-cand-723-lamp-runtime"></a>

### Q-S00-CAND-723-LAMP-RUNTIME

- 问题：指示灯多语言状态图切换的实际显示？
- 来源/归属：CAND-20261110/U-181；指示灯；C04
- 层/状态：E；部分历史补证，余项未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 对象：指示灯多语言状态图切换的实际显示。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-LAMP、EV-S00-CAND-LAMP；旧核销的2..32/0..N-1事实关闭继续有效；报告还解释状态图序、新增默认及5→2→5删除，但E渲染分支不闭合。
- 下一探针：针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-719-LAMP-PARENT

<a id="q-s00-cand-724-func-parent"></a>

### Q-S00-CAND-724-FUNC-PARENT

- 问题：18功能及全部子类型的设备动作与前置配置？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：18功能及全部子类型的设备动作与前置配置。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 沿用问题身份：Q-FUNC-RUN-001
- 子项：Q-S00-CAND-725-FUNC-PARENT、Q-S00-CAND-727-FUNC-PARENT、Q-S00-CAND-729-FUNC-PARENT、Q-S00-CAND-731-FUNC-PARENT、Q-S00-CAND-733-FUNC-PARENT、Q-S00-CAND-735-FUNC-PARENT、Q-S00-CAND-737-FUNC-PARENT、Q-S00-CAND-739-FUNC-PARENT、Q-S00-CAND-741-FUNC-PARENT、Q-S00-CAND-743-FUNC-PARENT、Q-S00-CAND-745-FUNC-PARENT、Q-S00-CAND-753-FUNC-PARENT、Q-S00-CAND-767-FUNC-PARENT、Q-S00-CAND-782-FUNC-PARENT、Q-S00-CAND-784-FUNC-PARENT、Q-S00-CAND-795-FUNC-PARENT、Q-S00-CAND-797-FUNC-PARENT、Q-S00-CAND-799-FUNC-PARENT

<a id="q-s00-cand-725-func-parent"></a>

### Q-S00-CAND-725-FUNC-PARENT

- 问题：功能0 背景灯开关的运行观察？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能0 背景灯开关的运行观察。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-724-FUNC-PARENT
- 子项：Q-S00-CAND-726-FUNC-RUNTIME

<a id="q-s00-cand-726-func-runtime"></a>

### Q-S00-CAND-726-FUNC-RUNTIME

- 问题：功能0 背景灯开关的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：记录初始背光，触碰该按钮后再触碰屏幕空白区，记录亮度和恢复。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：记录初始背光，触碰该按钮后再触碰屏幕空白区，记录亮度和恢复。
- 父项：Q-S00-CAND-725-FUNC-PARENT

<a id="q-s00-cand-727-func-parent"></a>

### Q-S00-CAND-727-FUNC-PARENT

- 问题：功能1 系统设置的运行观察？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能1 系统设置的运行观察。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-724-FUNC-PARENT
- 子项：Q-S00-CAND-728-FUNC-RUNTIME

<a id="q-s00-cand-728-func-runtime"></a>

### Q-S00-CAND-728-FUNC-RUNTIME

- 问题：功能1 系统设置的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：触碰按钮，记录系统设置弹窗、可返回路径及原画面状态。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：触碰按钮，记录系统设置弹窗、可返回路径及原画面状态。
- 父项：Q-S00-CAND-727-FUNC-PARENT

<a id="q-s00-cand-729-func-parent"></a>

### Q-S00-CAND-729-FUNC-PARENT

- 问题：功能2 系统重启的运行观察？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能2 系统重启的运行观察。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-724-FUNC-PARENT
- 子项：Q-S00-CAND-730-FUNC-RUNTIME

<a id="q-s00-cand-730-func-runtime"></a>

### Q-S00-CAND-730-FUNC-RUNTIME

- 问题：功能2 系统重启的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：在已保存且可恢复的独立工程中触碰重启，记录停机、启动画面及保留值；先取得原生期望。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在已保存且可恢复的独立工程中触碰重启，记录停机、启动画面及保留值；先取得原生期望。
- 父项：Q-S00-CAND-729-FUNC-PARENT

<a id="q-s00-cand-731-func-parent"></a>

### Q-S00-CAND-731-FUNC-PARENT

- 问题：功能3 设置屏保时间的运行观察？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能3 设置屏保时间的运行观察。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-724-FUNC-PARENT
- 子项：Q-S00-CAND-732-FUNC-RUNTIME

<a id="q-s00-cand-732-func-runtime"></a>

### Q-S00-CAND-732-FUNC-RUNTIME

- 问题：功能3 设置屏保时间的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：触碰后输入两种可辨识屏保时长，观察确认、取消和实际进入屏保时间。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：触碰后输入两种可辨识屏保时长，观察确认、取消和实际进入屏保时间。
- 父项：Q-S00-CAND-731-FUNC-PARENT

<a id="q-s00-cand-733-func-parent"></a>

### Q-S00-CAND-733-FUNC-PARENT

- 问题：功能4 触摸校正的运行观察？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能4 触摸校正的运行观察。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-724-FUNC-PARENT
- 子项：Q-S00-CAND-734-FUNC-RUNTIME

<a id="q-s00-cand-734-func-runtime"></a>

### Q-S00-CAND-734-FUNC-RUNTIME

- 问题：功能4 触摸校正的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：记录按钮触发的校正界面及退出路径；是否执行校正作为后续独立现场授权范围。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：记录按钮触发的校正界面及退出路径；是否执行校正作为后续独立现场授权范围。
- 父项：Q-S00-CAND-733-FUNC-PARENT

<a id="q-s00-cand-735-func-parent"></a>

### Q-S00-CAND-735-FUNC-PARENT

- 问题：功能5 设置系统时间的运行观察？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能5 设置系统时间的运行观察。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-724-FUNC-PARENT
- 子项：Q-S00-CAND-736-FUNC-RUNTIME

<a id="q-s00-cand-736-func-runtime"></a>

### Q-S00-CAND-736-FUNC-RUNTIME

- 问题：功能5 设置系统时间的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：触碰后输入可辨识日期时间，分别确认与取消并记录系统时钟变化。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：触碰后输入可辨识日期时间，分别确认与取消并记录系统时钟变化。
- 父项：Q-S00-CAND-735-FUNC-PARENT

<a id="q-s00-cand-737-func-parent"></a>

### Q-S00-CAND-737-FUNC-PARENT

- 问题：功能6 面板保护解锁的运行观察？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能6 面板保护解锁的运行观察。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-724-FUNC-PARENT
- 子项：Q-S00-CAND-738-FUNC-RUNTIME

<a id="q-s00-cand-738-func-runtime"></a>

### Q-S00-CAND-738-FUNC-RUNTIME

- 问题：功能6 面板保护解锁的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：配置保护锁定，在锁定前后触碰，分别输入错误及正确密码并观察保护状态。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：配置保护锁定，在锁定前后触碰，分别输入错误及正确密码并观察保护状态。
- 父项：Q-S00-CAND-737-FUNC-PARENT

<a id="q-s00-cand-739-func-parent"></a>

### Q-S00-CAND-739-FUNC-PARENT

- 问题：功能7 更改用户等级的运行观察？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能7 更改用户等级的运行观察。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-724-FUNC-PARENT
- 子项：Q-S00-CAND-740-FUNC-RUNTIME

<a id="q-s00-cand-740-func-runtime"></a>

### Q-S00-CAND-740-FUNC-RUNTIME

- 问题：功能7 更改用户等级的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：配置已知等级及受限对象，执行更改等级，记录登录键盘、结果等级及权限变化。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：配置已知等级及受限对象，执行更改等级，记录登录键盘、结果等级及权限变化。
- 父项：Q-S00-CAND-739-FUNC-PARENT

<a id="q-s00-cand-741-func-parent"></a>

### Q-S00-CAND-741-FUNC-PARENT

- 问题：功能8 注销用户等级的运行观察？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能8 注销用户等级的运行观察。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-724-FUNC-PARENT
- 子项：Q-S00-CAND-742-FUNC-RUNTIME

<a id="q-s00-cand-742-func-runtime"></a>

### Q-S00-CAND-742-FUNC-RUNTIME

- 问题：功能8 注销用户等级的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：先登录非默认等级再注销，记录实际返回等级及受限对象可用性；不得预设注销后回哪个等级。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：先登录非默认等级再注销，记录实际返回等级及受限对象可用性；不得预设注销后回哪个等级。
- 父项：Q-S00-CAND-741-FUNC-PARENT

<a id="q-s00-cand-743-func-parent"></a>

### Q-S00-CAND-743-FUNC-PARENT

- 问题：功能9 触摸声音开关的运行观察？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能9 触摸声音开关的运行观察。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-724-FUNC-PARENT
- 子项：Q-S00-CAND-744-FUNC-RUNTIME

<a id="q-s00-cand-744-func-runtime"></a>

### Q-S00-CAND-744-FUNC-RUNTIME

- 问题：功能9 触摸声音开关的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：先记录触摸声音，交替触碰此按钮和普通按钮，记录声音开关状态。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：先记录触摸声音，交替触碰此按钮和普通按钮，记录声音开关状态。
- 父项：Q-S00-CAND-743-FUNC-PARENT

<a id="q-s00-cand-745-func-parent"></a>

### Q-S00-CAND-745-FUNC-PARENT

- 问题：功能10 历史数据操作的运行观察？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能10 历史数据操作的运行观察。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-724-FUNC-PARENT
- 子项：Q-S00-CAND-746-FUNC-RUNTIME、Q-S00-CAND-747-FUNC-RUNTIME、Q-S00-CAND-748-FUNC-RUNTIME、Q-S00-CAND-749-FUNC-RUNTIME、Q-S00-CAND-750-FUNC-RUNTIME、Q-S00-CAND-751-FUNC-RUNTIME、Q-S00-CAND-752-FUNC-RUNTIME

<a id="q-s00-cand-746-func-runtime"></a>

### Q-S00-CAND-746-FUNC-RUNTIME

- 问题：功能10 历史数据操作/类型0 清除全部历史数据的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整数据采集与历史数据显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“清除全部历史数据”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“清除全部历史数据”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-745-FUNC-PARENT

<a id="q-s00-cand-747-func-runtime"></a>

### Q-S00-CAND-747-FUNC-RUNTIME

- 问题：功能10 历史数据操作/类型1 历史数据上移的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整数据采集与历史数据显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“历史数据上移”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“历史数据上移”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-745-FUNC-PARENT

<a id="q-s00-cand-748-func-runtime"></a>

### Q-S00-CAND-748-FUNC-RUNTIME

- 问题：功能10 历史数据操作/类型2 历史数据下移的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整数据采集与历史数据显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“历史数据下移”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“历史数据下移”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-745-FUNC-PARENT

<a id="q-s00-cand-749-func-runtime"></a>

### Q-S00-CAND-749-FUNC-RUNTIME

- 问题：功能10 历史数据操作/类型3 历史数据上翻页的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整数据采集与历史数据显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“历史数据上翻页”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“历史数据上翻页”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-745-FUNC-PARENT

<a id="q-s00-cand-750-func-runtime"></a>

### Q-S00-CAND-750-FUNC-RUNTIME

- 问题：功能10 历史数据操作/类型4 历史数据下翻页的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整数据采集与历史数据显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“历史数据下翻页”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“历史数据下翻页”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-745-FUNC-PARENT

<a id="q-s00-cand-751-func-runtime"></a>

### Q-S00-CAND-751-FUNC-RUNTIME

- 问题：功能10 历史数据操作/类型5 U盘历史数据导出的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整数据采集与历史数据显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“U盘历史数据导出”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“U盘历史数据导出”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-745-FUNC-PARENT

<a id="q-s00-cand-752-func-runtime"></a>

### Q-S00-CAND-752-FUNC-RUNTIME

- 问题：功能10 历史数据操作/类型6 SD卡历史数据导出的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整数据采集与历史数据显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“SD卡历史数据导出”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“SD卡历史数据导出”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-745-FUNC-PARENT

<a id="q-s00-cand-753-func-parent"></a>

### Q-S00-CAND-753-FUNC-PARENT

- 问题：功能11 配方操作的运行观察？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能11 配方操作的运行观察。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-724-FUNC-PARENT
- 子项：Q-S00-CAND-754-FUNC-RUNTIME、Q-S00-CAND-755-FUNC-RUNTIME、Q-S00-CAND-756-FUNC-RUNTIME、Q-S00-CAND-757-FUNC-RUNTIME、Q-S00-CAND-758-FUNC-RUNTIME、Q-S00-CAND-759-FUNC-RUNTIME、Q-S00-CAND-760-FUNC-RUNTIME、Q-S00-CAND-761-FUNC-RUNTIME、Q-S00-CAND-762-FUNC-RUNTIME、Q-S00-CAND-763-FUNC-RUNTIME、Q-S00-CAND-764-FUNC-RUNTIME、Q-S00-CAND-765-FUNC-RUNTIME、Q-S00-CAND-766-FUNC-RUNTIME

<a id="q-s00-cand-754-func-runtime"></a>

### Q-S00-CAND-754-FUNC-RUNTIME

- 问题：功能11 配方操作/类型0 当前配方写入PLC的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整配方及PLC可观测区，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“当前配方写入PLC”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“当前配方写入PLC”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-753-FUNC-PARENT

<a id="q-s00-cand-755-func-runtime"></a>

### Q-S00-CAND-755-FUNC-RUNTIME

- 问题：功能11 配方操作/类型1 从PLC读当前配方的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整配方及PLC可观测区，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“从PLC读当前配方”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“从PLC读当前配方”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-753-FUNC-PARENT

<a id="q-s00-cand-756-func-runtime"></a>

### Q-S00-CAND-756-FUNC-RUNTIME

- 问题：功能11 配方操作/类型2 前一个配方的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整配方及PLC可观测区，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“前一个配方”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“前一个配方”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-753-FUNC-PARENT

<a id="q-s00-cand-757-func-runtime"></a>

### Q-S00-CAND-757-FUNC-RUNTIME

- 问题：功能11 配方操作/类型3 后一个配方的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整配方及PLC可观测区，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“后一个配方”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“后一个配方”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-753-FUNC-PARENT

<a id="q-s00-cand-758-func-runtime"></a>

### Q-S00-CAND-758-FUNC-RUNTIME

- 问题：功能11 配方操作/类型4 当前配方保存的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整配方及PLC可观测区，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“当前配方保存”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“当前配方保存”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-753-FUNC-PARENT

<a id="q-s00-cand-759-func-runtime"></a>

### Q-S00-CAND-759-FUNC-RUNTIME

- 问题：功能11 配方操作/类型5 配方读取及保存的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整配方及PLC可观测区，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“配方读取及保存”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“配方读取及保存”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-753-FUNC-PARENT

<a id="q-s00-cand-760-func-runtime"></a>

### Q-S00-CAND-760-FUNC-RUNTIME

- 问题：功能11 配方操作/类型6 重载配方的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整配方及PLC可观测区，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“重载配方”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“重载配方”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-753-FUNC-PARENT

<a id="q-s00-cand-761-func-runtime"></a>

### Q-S00-CAND-761-FUNC-RUNTIME

- 问题：功能11 配方操作/类型7 当前配方删除的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整配方及PLC可观测区，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“当前配方删除”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“当前配方删除”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-753-FUNC-PARENT

<a id="q-s00-cand-762-func-runtime"></a>

### Q-S00-CAND-762-FUNC-RUNTIME

- 问题：功能11 配方操作/类型8 U盘配方导出的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整配方及PLC可观测区，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“U盘配方导出”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“U盘配方导出”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-753-FUNC-PARENT

<a id="q-s00-cand-763-func-runtime"></a>

### Q-S00-CAND-763-FUNC-RUNTIME

- 问题：功能11 配方操作/类型9 U盘配方导入的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整配方及PLC可观测区，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“U盘配方导入”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“U盘配方导入”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-753-FUNC-PARENT

<a id="q-s00-cand-764-func-runtime"></a>

### Q-S00-CAND-764-FUNC-RUNTIME

- 问题：功能11 配方操作/类型10 配方添加的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整配方及PLC可观测区，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“配方添加”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“配方添加”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-753-FUNC-PARENT

<a id="q-s00-cand-765-func-runtime"></a>

### Q-S00-CAND-765-FUNC-RUNTIME

- 问题：功能11 配方操作/类型11 SD卡配方导出的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整配方及PLC可观测区，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“SD卡配方导出”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“SD卡配方导出”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-753-FUNC-PARENT

<a id="q-s00-cand-766-func-runtime"></a>

### Q-S00-CAND-766-FUNC-RUNTIME

- 问题：功能11 配方操作/类型12 SD卡配方导入的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整配方及PLC可观测区，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“SD卡配方导入”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“SD卡配方导入”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-753-FUNC-PARENT

<a id="q-s00-cand-767-func-parent"></a>

### Q-S00-CAND-767-FUNC-PARENT

- 问题：功能12 报警操作的运行观察？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能12 报警操作的运行观察。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-724-FUNC-PARENT
- 子项：Q-S00-CAND-768-FUNC-RUNTIME、Q-S00-CAND-769-FUNC-RUNTIME、Q-S00-CAND-770-FUNC-RUNTIME、Q-S00-CAND-771-FUNC-RUNTIME、Q-S00-CAND-772-FUNC-RUNTIME、Q-S00-CAND-773-FUNC-RUNTIME、Q-S00-CAND-774-FUNC-RUNTIME、Q-S00-CAND-775-FUNC-RUNTIME、Q-S00-CAND-776-FUNC-RUNTIME、Q-S00-CAND-777-FUNC-RUNTIME、Q-S00-CAND-778-FUNC-RUNTIME、Q-S00-CAND-779-FUNC-RUNTIME、Q-S00-CAND-780-FUNC-RUNTIME、Q-S00-CAND-781-FUNC-RUNTIME

<a id="q-s00-cand-768-func-runtime"></a>

### Q-S00-CAND-768-FUNC-RUNTIME

- 问题：功能12 报警操作/类型0 确认报警的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整报警登录、活动及历史报警显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“确认报警”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“确认报警”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-767-FUNC-PARENT

<a id="q-s00-cand-769-func-runtime"></a>

### Q-S00-CAND-769-FUNC-RUNTIME

- 问题：功能12 报警操作/类型1 清除报警的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整报警登录、活动及历史报警显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“清除报警”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“清除报警”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-767-FUNC-PARENT

<a id="q-s00-cand-770-func-runtime"></a>

### Q-S00-CAND-770-FUNC-RUNTIME

- 问题：功能12 报警操作/类型2 报警声音开关的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整报警登录、活动及历史报警显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“报警声音开关”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“报警声音开关”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-767-FUNC-PARENT

<a id="q-s00-cand-771-func-runtime"></a>

### Q-S00-CAND-771-FUNC-RUNTIME

- 问题：功能12 报警操作/类型3 清除历史报警数据的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整报警登录、活动及历史报警显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“清除历史报警数据”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“清除历史报警数据”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-767-FUNC-PARENT

<a id="q-s00-cand-772-func-runtime"></a>

### Q-S00-CAND-772-FUNC-RUNTIME

- 问题：功能12 报警操作/类型4 报警控件报警上移的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整报警登录、活动及历史报警显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“报警控件报警上移”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“报警控件报警上移”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-767-FUNC-PARENT

<a id="q-s00-cand-773-func-runtime"></a>

### Q-S00-CAND-773-FUNC-RUNTIME

- 问题：功能12 报警操作/类型5 报警控件报警下移的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整报警登录、活动及历史报警显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“报警控件报警下移”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“报警控件报警下移”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-767-FUNC-PARENT

<a id="q-s00-cand-774-func-runtime"></a>

### Q-S00-CAND-774-FUNC-RUNTIME

- 问题：功能12 报警操作/类型6 报警控件报警上翻页的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整报警登录、活动及历史报警显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“报警控件报警上翻页”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“报警控件报警上翻页”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-767-FUNC-PARENT

<a id="q-s00-cand-775-func-runtime"></a>

### Q-S00-CAND-775-FUNC-RUNTIME

- 问题：功能12 报警操作/类型7 报警控件报警下翻页的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整报警登录、活动及历史报警显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“报警控件报警下翻页”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“报警控件报警下翻页”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-767-FUNC-PARENT

<a id="q-s00-cand-776-func-runtime"></a>

### Q-S00-CAND-776-FUNC-RUNTIME

- 问题：功能12 报警操作/类型8 历史报警控件报警上移的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整报警登录、活动及历史报警显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“历史报警控件报警上移”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“历史报警控件报警上移”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-767-FUNC-PARENT

<a id="q-s00-cand-777-func-runtime"></a>

### Q-S00-CAND-777-FUNC-RUNTIME

- 问题：功能12 报警操作/类型9 历史报警控件报警下移的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整报警登录、活动及历史报警显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“历史报警控件报警下移”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“历史报警控件报警下移”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-767-FUNC-PARENT

<a id="q-s00-cand-778-func-runtime"></a>

### Q-S00-CAND-778-FUNC-RUNTIME

- 问题：功能12 报警操作/类型10 历史报警控件报警上翻页的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整报警登录、活动及历史报警显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“历史报警控件报警上翻页”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“历史报警控件报警上翻页”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-767-FUNC-PARENT

<a id="q-s00-cand-779-func-runtime"></a>

### Q-S00-CAND-779-FUNC-RUNTIME

- 问题：功能12 报警操作/类型11 历史报警控件报警下翻页的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整报警登录、活动及历史报警显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“历史报警控件报警下翻页”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“历史报警控件报警下翻页”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-767-FUNC-PARENT

<a id="q-s00-cand-780-func-runtime"></a>

### Q-S00-CAND-780-FUNC-RUNTIME

- 问题：功能12 报警操作/类型12 U盘历史报警导出的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整报警登录、活动及历史报警显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“U盘历史报警导出”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“U盘历史报警导出”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-767-FUNC-PARENT

<a id="q-s00-cand-781-func-runtime"></a>

### Q-S00-CAND-781-FUNC-RUNTIME

- 问题：功能12 报警操作/类型13 SD卡历史报警导出的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片；阶段10固定固件及现场记录
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需完整报警登录、活动及历史报警显示，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。
- 刺激：在上述可观测数据中只触碰一次“SD卡历史报警导出”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在上述可观测数据中只触碰一次“SD卡历史报警导出”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。
- 父项：Q-S00-CAND-767-FUNC-PARENT

<a id="q-s00-cand-782-func-parent"></a>

### Q-S00-CAND-782-FUNC-PARENT

- 问题：功能13 语言切换的运行观察？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能13 语言切换的运行观察。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-724-FUNC-PARENT
- 子项：Q-S00-CAND-783-FUNC-RUNTIME

<a id="q-s00-cand-783-func-runtime"></a>

### Q-S00-CAND-783-FUNC-RUNTIME

- 问题：功能13 语言切换的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：给中英文状态设置明显不同文本，逐目标语言触碰，记录当前语言及所有相关对象文本。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：给中英文状态设置明显不同文本，逐目标语言触碰，记录当前语言及所有相关对象文本。
- 父项：Q-S00-CAND-782-FUNC-PARENT

<a id="q-s00-cand-784-func-parent"></a>

### Q-S00-CAND-784-FUNC-PARENT

- 问题：功能14 键盘组件的运行观察？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能14 键盘组件的运行观察。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-724-FUNC-PARENT
- 子项：Q-S00-CAND-785-FUNC-RUNTIME、Q-S00-CAND-786-FUNC-RUNTIME、Q-S00-CAND-787-FUNC-RUNTIME、Q-S00-CAND-788-FUNC-RUNTIME、Q-S00-CAND-789-FUNC-RUNTIME、Q-S00-CAND-790-FUNC-RUNTIME、Q-S00-CAND-791-FUNC-RUNTIME、Q-S00-CAND-792-FUNC-RUNTIME、Q-S00-CAND-793-FUNC-RUNTIME、Q-S00-CAND-794-FUNC-RUNTIME

<a id="q-s00-cand-785-func-runtime"></a>

### Q-S00-CAND-785-FUNC-RUNTIME

- 问题：功能14 键盘组件/类型0 Enter的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需原生数值或ASCII输入框及可观察的键盘焦点、文本和光标位置。
- 刺激：在文本Ab12和中间光标处触发“Enter”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在文本Ab12和中间光标处触发“Enter”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。
- 父项：Q-S00-CAND-784-FUNC-PARENT

<a id="q-s00-cand-786-func-runtime"></a>

### Q-S00-CAND-786-FUNC-RUNTIME

- 问题：功能14 键盘组件/类型1 BackSpace的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需原生数值或ASCII输入框及可观察的键盘焦点、文本和光标位置。
- 刺激：在文本Ab12和中间光标处触发“BackSpace”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在文本Ab12和中间光标处触发“BackSpace”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。
- 父项：Q-S00-CAND-784-FUNC-PARENT

<a id="q-s00-cand-787-func-runtime"></a>

### Q-S00-CAND-787-FUNC-RUNTIME

- 问题：功能14 键盘组件/类型2 Clear的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需原生数值或ASCII输入框及可观察的键盘焦点、文本和光标位置。
- 刺激：在文本Ab12和中间光标处触发“Clear”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在文本Ab12和中间光标处触发“Clear”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。
- 父项：Q-S00-CAND-784-FUNC-PARENT

<a id="q-s00-cand-788-func-runtime"></a>

### Q-S00-CAND-788-FUNC-RUNTIME

- 问题：功能14 键盘组件/类型3 ESC的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需原生数值或ASCII输入框及可观察的键盘焦点、文本和光标位置。
- 刺激：在文本Ab12和中间光标处触发“ESC”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在文本Ab12和中间光标处触发“ESC”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。
- 父项：Q-S00-CAND-784-FUNC-PARENT

<a id="q-s00-cand-789-func-runtime"></a>

### Q-S00-CAND-789-FUNC-RUNTIME

- 问题：功能14 键盘组件/类型4 Delete的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需原生数值或ASCII输入框及可观察的键盘焦点、文本和光标位置。
- 刺激：在文本Ab12和中间光标处触发“Delete”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在文本Ab12和中间光标处触发“Delete”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。
- 父项：Q-S00-CAND-784-FUNC-PARENT

<a id="q-s00-cand-790-func-runtime"></a>

### Q-S00-CAND-790-FUNC-RUNTIME

- 问题：功能14 键盘组件/类型5 Caps的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需原生数值或ASCII输入框及可观察的键盘焦点、文本和光标位置。
- 刺激：在文本Ab12和中间光标处触发“Caps”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在文本Ab12和中间光标处触发“Caps”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。
- 父项：Q-S00-CAND-784-FUNC-PARENT

<a id="q-s00-cand-791-func-runtime"></a>

### Q-S00-CAND-791-FUNC-RUNTIME

- 问题：功能14 键盘组件/类型6 Left的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需原生数值或ASCII输入框及可观察的键盘焦点、文本和光标位置。
- 刺激：在文本Ab12和中间光标处触发“Left”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在文本Ab12和中间光标处触发“Left”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。
- 父项：Q-S00-CAND-784-FUNC-PARENT

<a id="q-s00-cand-792-func-runtime"></a>

### Q-S00-CAND-792-FUNC-RUNTIME

- 问题：功能14 键盘组件/类型7 Right的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需原生数值或ASCII输入框及可观察的键盘焦点、文本和光标位置。
- 刺激：在文本Ab12和中间光标处触发“Right”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在文本Ab12和中间光标处触发“Right”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。
- 父项：Q-S00-CAND-784-FUNC-PARENT

<a id="q-s00-cand-793-func-runtime"></a>

### Q-S00-CAND-793-FUNC-RUNTIME

- 问题：功能14 键盘组件/类型8 Space的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需原生数值或ASCII输入框及可观察的键盘焦点、文本和光标位置。
- 刺激：在文本Ab12和中间光标处触发“Space”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在文本Ab12和中间光标处触发“Space”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。
- 父项：Q-S00-CAND-784-FUNC-PARENT

<a id="q-s00-cand-794-func-runtime"></a>

### Q-S00-CAND-794-FUNC-RUNTIME

- 问题：功能14 键盘组件/类型9 ASCII/BMP字符的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。 另需原生数值或ASCII输入框及可观察的键盘焦点、文本和光标位置。
- 刺激：在文本Ab12和中间光标处触发“ASCII/BMP字符”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：在文本Ab12和中间光标处触发“ASCII/BMP字符”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。
- 父项：Q-S00-CAND-784-FUNC-PARENT

<a id="q-s00-cand-795-func-parent"></a>

### Q-S00-CAND-795-FUNC-PARENT

- 问题：功能15 无线设置的运行观察？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能15 无线设置的运行观察。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-724-FUNC-PARENT
- 子项：Q-S00-CAND-796-FUNC-RUNTIME

<a id="q-s00-cand-796-func-runtime"></a>

### Q-S00-CAND-796-FUNC-RUNTIME

- 问题：功能15 无线设置的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：记录原生是否无动作、提示或打开无线界面；帮助暂不支持只作H假设，未观测前不写可用结论。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：记录原生是否无动作、提示或打开无线界面；帮助暂不支持只作H假设，未观测前不写可用结论。
- 父项：Q-S00-CAND-795-FUNC-PARENT

<a id="q-s00-cand-797-func-parent"></a>

### Q-S00-CAND-797-FUNC-PARENT

- 问题：功能16 背光调节的运行观察？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能16 背光调节的运行观察。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-724-FUNC-PARENT
- 子项：Q-S00-CAND-798-FUNC-RUNTIME

<a id="q-s00-cand-798-func-runtime"></a>

### Q-S00-CAND-798-FUNC-RUNTIME

- 问题：功能16 背光调节的实际设备动作和状态变化？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；未闭合；atomic
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先取得固定AMT固件及可记录的屏端/寄存器观察；对应配置及原生期望不足时仅阻塞本E问项
- 前置：先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。
- 刺激：先记录亮度，触碰后观察是否有调节入口及亮度变化，保留两篇帮助的冲突。
- 预期观察：记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。
- 下一探针：先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：先记录亮度，触碰后观察是否有调节入口及亮度变化，保留两篇帮助的冲突。
- 父项：Q-S00-CAND-797-FUNC-PARENT

<a id="q-s00-cand-799-func-parent"></a>

### Q-S00-CAND-799-FUNC-PARENT

- 问题：功能17 下载方式的运行观察？
- 来源/归属：CAND-20261110/U-079；功能按钮；C05
- 层/状态：E；父项开放，按子项汇总；parent
- 适用范围：关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。
- 依赖：先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在
- 前置：按各子问题分别建立前置。
- 刺激：仅汇总子问题，不作为单个测试执行。 对象：功能17 下载方式的运行观察。
- 预期观察：所有声明范围子项都有证据才可关闭父项。
- 现有证据：EV-S00-CAND-CSV、EV-S00-CAND-MANUAL-FUNC、EV-S00-CAND-FUNC、EV-S00-CAND-FUNC-DEVICE、EV-S00-CAND-FUNC-USB；原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。
- 下一探针：仅汇总子问题，不作为单个测试执行。 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。
- 父项：Q-S00-CAND-724-FUNC-PARENT
- 子项：Q-S00-CAND-164-FUNC-RUNTIME、Q-S00-CAND-165-FUNC-RUNTIME

## 证据身份

| 证据ID | 路径 | SHA256 | 可支持范围 |
|---|---|---|---|
| EV-S00-CAND-CSV | `design_plan/复刻待验线索台账.csv` | `27f423c4b35d387b243d0e4ca69485ff2a892b41e8b3cd81e19cfc81b532b87c` | 冻结308候选身份、原文及原始来源；不是问题答案或测试结果。 |
| EV-S00-CAND-PRIOR | `design_plan/复刻待验核销记录.md` | `190a579681645204712fd21c4e3e6a164782a2b25d3832fac998fa3d720be3f3` | 沿用3事实关闭、U181渲染开放、U079父项开放、U308非问题。 |
| EV-S00-CAND-STAGE0 | `design_plan/01_阶段0_基线范围与证据管理.md` | `2889dd5b24ee468775be47a3a648b5770f379ef85f924d6508066644e9d61b2f` | S00-04核销方法、范围与放行边界。 |
| EV-S00-CAND-ENTRY | `design_plan/00_复刻执行总入口.md` | `d5125103f7203f1fe34b595adbe16f8f48a97f23d6bddf9b6561c10bbc20db02` | 当前用户授权下的阶段方法；文档原始调查模式不替代本轮阶段0实现授权。 |
| EV-S00-CAND-A04 | `design_plan/A04_31控件实施索引.md` | `09a646eb8509b96d1db86a592aa1be83f240f7fef6c8b5b26ccc664f6fc0c852` | 31控件调用归属，不替代逐字段或运行通过。 |
| EV-S00-CAND-MATRIX | `design_plan/控件文档与实施覆盖矩阵.md` | `92d76becfd9060203c30f37f48e010baeb4cbf8540f94230349a8211e9eb859c` | D/U/P/R/C/T/E起始边界与5类历史局部范围。 |
| EV-S00-CAND-README | `README.md` | `a98e19faf7f0fe2f29f0c179d5f75e11a349ce6a7f4d8201655c7e3fbea105ec` | 0.7.0软件受支持范围，非完整控件或E证明。 |
| EV-S00-CAND-TEXT | `docs/静态文本手册实现与验收.md` | `9f3bcd51af850ca10daedbbbb8956fb3a9deb43894417581851df05bada6d385` | 历史0.3实现；开头明确0.4已扩展多语言及显现编译，旧拒绝表不可恢复为当前边界。仅一个指定外观/位置样例有用户现场确认。 |
| EV-S00-CAND-BIT | `docs/位按钮手册实现与验收.md` | `ee0b3e769e9e6f8a29b1d7013927365e4c735c2a856fc33fe77b069ceb5669ba` | 0.4设备编译及后续图库版：宏工具链、权限记录、通知、语言图帧及sm互通有历史局部证据；四操作和权限现场待确认。 |
| EV-S00-CAND-WORD | `docs/字按钮手册实现与验收.md` | `a9514e3f8fa76225ff3372c80c3951871131da9d7d05a329302316a9d775f243` | 0.5.0：49组合活动字段、源往返及OFF/ON帧已有历史对照；键盘、寄存器运算、通知和权限实屏尚待反馈。 |
| EV-S00-CAND-LAMP | `docs/指示灯手册实现与验收.md` | `31012b898d8637a42d9aa7c394985191dcb229f4471b9605061b48c91d1593f0` | 0.6.0：真实Qt禁用、2..32范围、图序、默认/删除及无图同文已有补证；现场状态、闪烁与权限仍未确认。 |
| EV-S00-CAND-FUNC | `docs/功能按钮手册实现与验收.md` | `fef9ca574ff2dea3ea902a09c08377e3859923b6e917c30e78d2f140deb804e7` | 0.7.0：18功能/类型、Qt禁用、隐藏ON及源/编译已有补证；USB只证明T，回执明确screenDisplayAndTouchConfirmed=false。 |
| EV-S00-CAND-FUNC-DEVICE | `docs/function-button-evidence/原厂设备补证.md` | `9589e24e3fcd754721787fd6aba564c10031ac619be7eef9e6e3052ca8ff8229` | 真实Qt getter、原厂UI/源/编译及字符和隐藏图补证；不含现场动作通过。 |
| EV-S00-CAND-FUNC-USB | `docs/function-button-evidence/usb-function-integration.json` | `dc1fa600c54d6ea1fbed368abca0fc23f965f0deaa1d07bac6609e662549b624` | 历史16控件传输/提交ACK，screenDisplayAndTouchConfirmed=false；不关闭E。 |
| EV-S00-CAND-BIT-DEVICE | `docs/位按钮设备编译实现.md` | `784e47a759041ea2bd6f317466d751bcd1fe9b9317f1b32f9d1f274500eab5f6` | 后续type28文本、地址及图库补证，覆盖旧文本报告的过时拒绝边界。 |
| EV-S00-CAND-MANUAL-ASCII | `design_plan/ASCII显示功能与交互复刻手册.md` | `ea1c06a5df57fba0beb8fb71983c9dbfd56e1fade528a1b99653649dc3cbe63e` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-GIF | `design_plan/GIF显示功能与交互复刻手册.md` | `3bc68459db1a4f149aad918f0dca0064b4c0fd5a5541422a53aaa8aa79bcd6d0` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-XY | `design_plan/X-Y曲线功能与交互复刻手册.md` | `52da03b2c4a13438f2db44ae80775b306c470eb43af184f3adc8dd7ecd885a6d` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-DROPLIST | `design_plan/下拉框功能与交互复刻手册.md` | `83d11892c74237aca00c525b0607de9df389b720faf100a16b750aef92d30b83` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-QR | `design_plan/二维码功能与交互复刻手册.md` | `48bd1cf79622070803fa2ab9f918d037189481fcfd3c866c507249cb4e92fb5f` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-BIT | `design_plan/位按钮功能与交互复刻手册.md` | `901c6c4efbbbe60f68524b0385ea8da03fe05990995b1005cc8d0a87bdc9adc4` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-SCALE | `design_plan/刻度功能与交互复刻手册.md` | `5b07283c67e61717a206562599cb6abdf928211ebf733d8e917dc2b88958186c` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-FUNC | `design_plan/功能按钮功能与交互复刻手册.md` | `bb645e6437d5f8ee6ae76435219a2aa746e1c63d2f8388c5f06f0d16c8dc11a6` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-ALARMBAR | `design_plan/动态报警条功能与交互复刻手册.md` | `adfb126cbec376deca190058642ac3a6fce27cff2e75faaac74598b35b660cd1` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-HISTORYALARM | `design_plan/历史报警显示功能与交互复刻手册.md` | `582682d7d027c2428243de541d9abab4bb9a985bb442382d1682c516045771ed` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-HISTORYDATA | `design_plan/历史数据显示器功能与交互复刻手册.md` | `da820bc2b92a60f719fd6260a8dc4abd63c925ea20729f69c5eb92c7ead71b2f` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-HISTORYTREND | `design_plan/历史趋势图功能与交互复刻手册.md` | `47b04c47e08ff0496d8fb30f5c2bd1e65db6004e247fe48b00a919132d617d58` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-PICTURE | `design_plan/图片显示功能与交互复刻手册.md` | `30b53be0d2a62531902a193f1df7e1d26b944f28def2cf3f21e0302df81abd23` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-GRAPH | `design_plan/图表功能与交互复刻手册.md` | `29ccdd06ffbbdd27a26affcde14e13b0a155c1ba7ce383e03e508d56e0f01855` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-MULTIFUNC | `design_plan/多功能按钮功能与交互复刻手册.md` | `86aab6d5f546899c49618b939a428b47a814fc38eaf664228dcee1f0afb34ba4` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-MULTISTATE | `design_plan/多态按钮功能与交互复刻手册.md` | `b705904a8f160cada6eabac2112e721ccfb9efab085ae616b2406e67e79d1038` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-WORD | `design_plan/字按钮功能与交互复刻手册.md` | `354abbd5140db06db6431311ec027b04a2b5794428577b67eb2e926b59838311` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-ALARM | `design_plan/报警显示功能与交互复刻手册.md` | `e47b146a993c4706e345b2fc9d1658fa9879e9d3e2d6a00301e55819acbc8cea` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-LAMP | `design_plan/指示灯功能与交互复刻手册.md` | `f16804edeb3662b6a4e07aeebf8cd973dfcf3a8acf0189eb18cfae39f0e8e9c8` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-NUMBER | `design_plan/数值显示功能与交互复刻手册.md` | `cd29986f7575072a2778ca26de5eb93594911b6a9389d48adc9ca8fa67999a51` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-DATAGROUP | `design_plan/数据群组功能与交互复刻手册.md` | `f37066fe13f382e29509c89c562db7a73eeaa880969d4caac33a1879ea10b932` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-TIME | `design_plan/时间显示功能与交互复刻手册.md` | `d567769800b818f3ebc725affefbbc6d7a028c3add123b0d240c55b33c98494f` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-FLOW | `design_plan/流动块功能与交互复刻手册.md` | `af077abf2c531239dfedf1091bf720857d640cded3ae9140549325d84daa5836` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-MESSAGE | `design_plan/消息显示功能与交互复刻手册.md` | `ae1ca61b0e06ce664ecdeb2bcac7fa721627ee56e5b0214a0ecd0db51db09241` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-SLIDE | `design_plan/滑动开关功能与交互复刻手册.md` | `aced242daeaeba44767a34054b3b2b60c30e1df54f70104baf8992fd3c8f12e2` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-WHEEL | `design_plan/滚轮功能与交互复刻手册.md` | `300269feaefbaa3d28b82588d6d9b567fdb660aafabcac5a7caa07c13f64d940` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-SCREEN | `design_plan/画面按钮功能与交互复刻手册.md` | `2a8e1f94fe5f01ed5bbff5fecf34c22a90c00c47060dbbd31ec59633a8b3ef5c` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-TRACK | `design_plan/移动轨迹功能与交互复刻手册.md` | `1bea4584d6d808518647382bc7c17b0db8de991d0ade30ecca1c1633130a5f0b` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-TREND | `design_plan/趋势图功能与交互复刻手册.md` | `0c0291588434e387df64398ca9956127c99342d623f16df352d89788b0a40f52` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-RECIPE | `design_plan/配方显示器功能与交互复刻手册.md` | `6d9361a6c48bec0573e3587b8567819a38f044d892f74c2e13359d21a6002f61` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |
| EV-S00-CAND-MANUAL-TEXT | `design_plan/静态文本功能与交互复刻手册.md` | `158ff80b8fe92e52b267bb1d8f3fe853132353418f9c8de37b7c808fbf3b3471` | 按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。 |

恢复点：从具体Q选择下一最小探针；复用当前身份一致的已有证据，缺前置只阻塞相关层。后续补证在同Q追加采用范围或重开原因，新增含义另给Q，不重排原U或重用测试数。
