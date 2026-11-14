# 阶段11保护用户改动的本地发布工具

标记：`//20261114-10 add`。实现为 `tools/publish_stage11.py`，本工具不创建候选提交、不推送、不 reset、不 stash 用户工作区。真实发布由 root 在候选内容提交并冻结后显式执行；本子任务只在专属临时 Git 仓执行过合成测试。

## 必需输入

- `--workspace-root`：允许的绝对工作区边界。
- `--main`：共享 Git 元数据的主仓根；必须处于 `main`，HEAD 等于 baseline，暂存区为空。
- `--worktree`：候选工作树根；必须 detached、完全干净、HEAD 等于 candidate。
- `--baseline` / `--candidate`：完整小写40位 SHA1；candidate 必须恰有一个父提交且等于 baseline。
- `--run-root`：位于工作区内、与两个 checkout 互不包含的专属操作根。每次产生新的 `publication-<UUID>`，不会覆盖旧回执。
- `--snapshot` / `--snapshot-sha256`：原始用户文件 JSON 和精确 SHA256；JSON 含 `head/files/mainIndexEmpty`，每个 file 含 `status/bytes/sha256`。
- `--snapshot-diff` / `--snapshot-diff-sha256`：对应 `git diff --binary <baseline> --` 的原始输出及精确 SHA256。实际分别生成显式 `core.quotepath=false/true` 两份完整 diff，只有其中一份完整匹配才接受；不删除路径头、不忽略正文。比较时只容忍 UTF-8 BOM/CRLF 文本封套差异，原始文件不改写。此前 snapshot 若含 `snapshotDiffQuotepath`，必须匹配其指定表示。
- `--allowlist` / `--allowlist-sha256`：`{"paths":["精确路径", ...]}` 或字符串数组；必须与整个 baseline→candidate 变更路径集合完全相等，无 glob。
- `--expected-user-count`：默认 **1891**；合成测试显式设3，不能据此降低真实运行门禁。
- `--mode`：默认 `preflight`。只有显式 `publish` 才写主仓应用文件、index 和 main ref。

先以冻结的输入运行 `preflight`，阅读 stdout 指向的 `receipt.json`；通过后以相同参数显式改为 `publish`。不能为了通过门禁重写原始用户快照或扩大白名单到无关路径。命令逐个处理文件，不将数百路径拼成 Windows 长 argv。

## 合并和保护证明

所有目标先在独占操作根生成完整预览及原始字节备份。只接受新增/修改常规文件，不处理删除、重命名、symlink、submodule。已有 reparse/junction/symlink 路径及边界逃逸均拒绝；新增目标已存在时拒绝。

用户文件与候选重叠时，仅允许已冻结的“未暂存跟踪文本修改”。按 UTF-8 严格解码，记录 BOM 和 LF/CRLF；混合换行、孤立 CR、NUL 或任务擅改 BOM 均拒绝。合并仅在私有 LF 副本中执行：

1. 以 base、用户内容、候选内容作 Git 三方合并。
2. 从结果反向移除任务变更，必须精确恢复原用户规范文本，并按原封套恢复**原用户完整字节**。
3. 从结果反向移除用户变更，必须恢复候选规范文本；防止候选偷偷包含用户 delta。
4. 比较 base→user 与 candidate→merged 的有序删除/新增内容块及其 SHA256；不能把行号偏移视为丢用户，也不能只凭最终文件同时含两段标识宣称保留。
5. 输出按用户原 BOM/EOL 封套写回，保留证明和原始备份。

候选自己的任务行号或 CRLF 变化不会被当成用户原始字节改变。未重叠的1891保护集合成员逐字不变；重叠成员只有经过上述证明后，最终 merged hash 才登记到新的保护快照。

## 受控写入和失败

全部预检成功后，再次核对 HEAD/index/1891文件及候选状态，持本工具专属锁，写每项变更意图到回执。目标仍等于预检 hash 才原子替换；index 以 `read-tree candidate` 更新，main ref 以 `update-ref ... candidate baseline` 比较并交换。最后验证暂存区为空、用户路径/状态仍相同及所有目标精确 hash。

发生异常时，仅尝试恢复本工具实际改过且当前 hash 仍等于本工具输出的路径；新增路径也只有属于本次创建且字节未被别人改变时才删除。原有用户文件不会被删除。index/ref 同样做身份核对。只有原 HEAD/index/全部原目标及用户保护均重新核实，才报告 `rolled_back`。检测到别人中途改写时保留其字节并报告 `state_unknown_manual_recovery_required`，不能自动重复发布。

每次独占回执记录输入 hash、前状态、全部目标、每个重叠证明、失败位置、回滚结果及后状态。无法建立安全工作区/run 根时不会尝试在不可信路径写“错误回执”；无法写任何存储或突然断电时也不能保证最后回执已落盘，不声称已回滚。

`status --porcelain=v1 -z --untracked-files=all` 和显式工作区 `diff --binary baseline --` 各有600秒有界预算；cached diff、两个提交间的差异以及所有 Git 写操作仍为120秒。调用不跳过、不自动重试。root 可在外部异步等待工具进程，以便期间告知进度。工具本身不额外刷新真实索引缓存，也不改变失败回滚路径。

成功另写 `after-protection-snapshot.json` 和 `.diff`，绑定新 HEAD、此前 snapshot hash 和原始重叠证明链，并以 `snapshotDiffQuotepath` 记录此次完整匹配选定的 `"true"` 或 `"false"`。后继 `.diff` 按同一显式设置生成。**不覆盖初始快照**。后续纯元数据提交应以此新快照/新 diff 及新 baseline 继续；因此用户未提交差异仍留在主仓，并且不会混入发布提交。

## 本轮实际测试

最终 `unittest-r5.log`：**16项通过（30.036秒）**。保留全部15项保护/中文路径测试，增加预算回归：完整 status 和真实工作区 diff 使用600秒，cached/提交间 diff 仍120秒，扫描不跳过也不重试。机器回执在 `test-receipts/r5/`，16份夹具全部清理。`readonly-review-r5.json` 绑定最终程序/测试 hash。R4仅status预算修复的16项历史通过另保留，不作为最终版本身份。

历史 `unittest-r3.log`：15项通过（27.738秒）。包含历史11项保护测试和新增4项中文路径真实 Git 测试：quoted/unquoted 原始表示均可完整匹配；后继快照保留选定表示；正文被改且重算输入 hash 仍拒绝；此前快照声明的表示不静默切换。机器回执位于 `test-receipts/r3/`，15份测试夹具均已清理。

历史 `unittest-r2.log`：11项通过（22.921秒）。包括2个重叠文件、BOM/CRLF封套、合并冲突不写、候选夹带用户修改拒绝、HEAD/index/用户字节漂移拒绝、ref更新后注入异常完整回滚、并发外部修改保留且状态未知、快照/白名单身份、用新保护快照发布后续纯元数据提交。所有测试仓位于 Z 盘独占根，已删除，机器回执复制到 `test-receipts/r2/`。

首轮 `unittest-r1.log` 的11个ERROR均出现在测试收尾删除 Windows Git 只读 loose objects；该轮不计通过。专属 fixture 清理116,743字节，见 `r1-cleanup.json`。首轮包装回执中 `fixtureRemovedAfterCapture=true` 是清理意图而非成功事实，以该清理错误日志和随后清理回执为准；第二轮已改为删除成功后再置 true。没有因此更换主仓清理路径或删除用户文件。

首次真实预检只读拒绝了不同的 Git 路径表示：原始冻结 diff 和当时默认/quotepath=true 的当前 diff **逐字相同**，强制 false 时中文 CSV 的 `diff --git`、`---`、`+++` 三个路径头改为 UTF-8，正文未变。1891份原字节检查已通过，主仓没有发生发布写入。原始快照、首次候选和拒绝回执均保留。R3用两份真实 diff 的完整匹配修复该表示兼容问题，既不放宽文件内容检查，也不把错误行模糊过滤。

随后真实发布在 `verify-after` 的完整 status 遭遇原120秒超时，回执确认恢复原 HEAD/index及1891保护。只读复查当前 main status 为0.266秒，HEAD/index前后完全未变。专属 `GIT_INDEX_FILE` 试验复现了与失败回执完全相同的 read-tree 后索引 hash：新33,605条 stat 的 mtime 全为0，而原32,940条仅19条为0，原缓存尺寸合计829,099,202字节。该操作清空缓存为实证，未为取证再次运行冷 status，因此不虚报完整冷扫耗时。私有索引 `read-tree -m baseline candidate` 因 `.gitattributes` 非 uptodate 被拒，未采用未证安全的优化。诊断保留在 `status-cache-diagnostic/`，两个可再生索引副本已清理9,741,412字节。

重跑合成测试需显式设置 `KINSEAL_PUBLICATION_TEST_ROOT` 为专属 Z 盘目录；可用 `KINSEAL_PUBLICATION_TEST_LABEL=r3` 保留新一轮独立回执。默认测试根不存在时测试跳过，避免误用 C 盘。测试中递归删除前核实绝对路径位于该专属根，只对其中的 Git 只读对象清除只读属性。
