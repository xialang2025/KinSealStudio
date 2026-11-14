# 阶段10 · 失败恢复与发布证据校验

本目录对应 S10-06 的受控故障记录判定和 S10-08 的发布限制汇总，不含新一轮 USB/PLC 操作、断电、固件升级或屏幕观察。标记：`//20261114-9 add`。

## 1. 本批实际结果

工具为 `tools/stage10_acceptance.py`；回归为 `tests/test_stage10_acceptance.py`。工具只读证据，只有调用者指定的汇总文件会原子替换。

- 新安全树内最终测试见 `unit-r13.txt`：37 项方法通过，包含多组子例。测试中的“physical_hmi”正向样本只是 schema 的合成声明，不是新实物证据。
- `unit-r1.txt` 保留真实回放时发现的3个错误：reset 的 `expectedReply.length=0` 不产生 ACK 读取；最终r13按正确ABI回放通过。
- `unit-red-r4.txt` 保留新验收工具的2个伪通过反例：同时删去 contract/receipt 的 commit 或 reset，仅保留成功布尔值。固定生命周期骨架、命令帧与命令号检查后在最终r13中通过。该问题不涉及生产 USB 实现。
- `unit-red-r6.txt` 保留非E层显式`expectedMatched=false`仍可能声明pass的反例；最终版本对全部pass统一要求`expectedMatched=true`。最终测试及源码绑定以`final-status.json`为准。
- 同行复核发现同时删去payload与ACK会留下伪完整骨架。`unit-red-r8.txt`保留无payload、缺段ACK、segment表不绑定、错误payloadCount及零长度identify应答5个反例；最终`_contract_shape`逐项核对真实10/12/14段完整计划、每段ACK、options、nextSequence及完成计数。
- `unit-red-r10.txt`保留已编译块字节被篡改但旧导入未重算文件的反例；最终逐文件重算长度/hash，并单独计算upLoad.knr外BE32后的wire hash。同行复核的JSON指数溢出`±1e999`由`unit-red-r12.txt`记录，最终在parse_float入口拒绝。
- `final-status.json` 包含4次实际 CLI 只读导入回执。静态10、重试后的动态12、宏14有完整的旧传输流水，但旧源码 manifest、唯一硬件号/固件均未记录，严格矩阵内为 `T=partial`、`E=not_run`、恢复=`not_run`。第一次动态12仍保留 `T=fail`，后一次成功不覆盖它。
- 原屏幕答复“显示字符，没有表格”“显示标识和按钮，没有宽度提示”均逐字保留。前者页面标识未闭合且用户已暂缓；后者缺少触控/宏执行/恢复观察。两者都没有升为 E 通过。

旧 `.scratch/kinseal-stage4-20261112/worktree` 于实施中消失，原因未确认；当时未完成的工具草稿和未归档结果不计为通过。本目录结果均在 `.worktrees/kinseal-stage10-20261114` 内重新实际执行。未写主工程，未尝试清理历史拒绝路径。

## 2. 命令与退出码

在仓库根目录，以实际 Python 路径运行，测试临时目录须显式指向 Z 盘自有目录：

```powershell
$env:KINSEAL_STAGE10_TEST_ROOT='Z:/Project/vscode_prj_temp/REA/.stage10-run-20261114/temp/recovery-owned'
$env:TEMP=$env:KINSEAL_STAGE10_TEST_ROOT
$env:TMP=$env:TEMP
& 'C:/Users/lenovo/AppData/Local/Programs/Python/Python311/python.exe' -B tests/test_stage10_acceptance.py -v

& 'C:/Users/lenovo/AppData/Local/Programs/Python/Python311/python.exe' -B tools/stage10_acceptance.py validate docs/stage10-evidence/recovery-owned/pending-template.json output-summary.json --root . --require-complete

& 'C:/Users/lenovo/AppData/Local/Programs/Python/Python311/python.exe' -B tools/stage10_acceptance.py import-stage6 old-macro-summary.json --root . --run physical-r2 --sample macro-14
```

`validate` 默认退出0仅表示输入及证据结构有效，查看输出 `status` 才能知道验收结果。发布脚本使用 `--require-complete`：声明的必需用例全为 pass 才退出0；有效但未全通过退出3；解析、hash、身份、文件或原子写入错误退出2。`import-stage6` 退出0仅表示历史记录可审计，不是当前版本通过。

`pending-template.json` 是七层分开的待填模板；预期闸口退出3。它只演示字段，不代表完整阶段10用例矩阵。实际发布必须将八项任务的每个声明范围列入 `requirements`。`fullReplicaCertified` 始终为 false；本工具不能为未声明、未观察的范围签发“完整复刻”。

## 3. 输入格式与身份字典

顶层 `format=KinSealStage10-acceptance-v1`，字段如下：

| 字段 | 含义及检查 |
|---|---|
| identities | 1..256项，按 identityId 引用，JSON规范化摘要绑定到每个 proof |
| artifacts | evidenceId → `{path,sha256}`；path 必须位于 `--root` 内，实际文件 SHA256 必须相同 |
| requirements | 1..4096项，必须非空；每项 `{caseId,layer,execution,identityId}`，caseId唯一 |
| cases | 0..4096项，caseId 必须已声明且唯一；不得更改 requirement 的层、执行环境或身份 |
| case.status | 仅 not_run / blocked_external / partial / pass / fail，必须有 reason |
| case.proof | pass/fail 必需；其他状态可无 proof，不会自动升级 |

七层是独立的 `D/U/P/R/C/T/E`，不接受合并值 `UPRC`。执行环境为 `host/mock/physical_hmi/pc_controlled/pc_plc`。即使层写 T，mock/PC 的 pass 也不会设置 `physicalTransportPassed`；E 仅接受 `physical_hmi`，且不继承 PC 通信结果。

每个 identity 必填：

| 字段 | 定义 |
|---|---|
| sourceSha256 | 源码快照/源码 manifest 文件的原始字节 SHA256，未知为 unknown；不等于 EXE/KSP |
| programSha256 | 被测 EXE 的原始字节 SHA256，未知为 unknown |
| projectSha256 | 被测原始 KSP 文件字节 SHA256，未知为 unknown |
| payloadSha256 | 本次完整 transfer contract 文件字节 SHA256，其他非传输用例为其明确绑定的输出/编译 manifest 字节 SHA256 |
| model / hardwareId | 目标型号、实物唯一身份；unknown 硬件不允许 pass 物理T/E |
| endpointId / protocol | 当次端点与协议；unknown 端点不允许 pass 物理T/E |
| firmware | `{state:known,value,basis}` 或 `{state:unknown,basis}`；不可把安装目录 bin hash 当屏内固件 |

所有 pass 必须以 proof 的 sourceArtifact/programArtifact/projectArtifact/payloadArtifact 指向实际保留文件，并逐项匹配四个 identity hash。固件 unknown 可以记录当前端点的实测结果，输出强制附“仅本端点、本次运行”限制，不外推其它固件/设备。hash 检查保证文件与声明一致，不能认证人工观察者或重建没有保留的现场行为。

`contract.identity.sourceSha256` 在生产代码中指原始 KSP 字节；`modelSha256` 指规范化 HmiProject JSON。二者都不是 C++ 源码。历史导入把前者放 projectSha256，后者放 normalizedProjectSha256，独立的源码 manifest 仍为 unknown。不能用当前文件的换行差异覆盖历史 hash。

## 4. proof、观察与前置层

proof 是 artifacts 内一个 JSON 文件，`format=KinSealStage10-proof-v1`，须包含：

```json
{
  "format": "KinSealStage10-proof-v1",
  "caseId": "实际用例ID",
  "layer": "C",
  "execution": "host",
  "status": "pass",
  "identitySha256": "identities对应项的规范化SHA256",
  "actor": "实际执行者",
  "initialState": "实际初态",
  "stimulusSteps": ["实际刺激步骤"],
  "expected": "按明确原生证据或产品约束给出的预期",
  "expectedMatched": true,
  "expectedSource": "预期来源的artifactId",
  "actual": "实际独立观察",
  "rawObservation": "原始日志/图片/人工记录的artifactId",
  "observedAt": "2026-11-14T12:00:00+00:00",
  "sourceArtifact": "源码manifest的artifactId",
  "programArtifact": "EXE的artifactId",
  "projectArtifact": "KSP的artifactId",
  "payloadArtifact": "完整contract或输出manifest的artifactId"
}
```

以上 hash 是占位说明，不能直接作为有效 proof。caseId/layer/execution/status 与 case 必须逐项一致；identitySha256 算法为 UTF8 JSON、键排序、无空白分隔、保留中文后做 SHA256。

T 另需 `transferReceipt` 与 `transferContract`，均引用已验证 artifacts；当前T适配器只处理KinSealUSB。contract 的 KSP raw hash/目标型号/完整字节 hash必须匹配 identity。固定31→53→34→35握手及末尾1B→1A→重启读取必须保留；10/12/14段的段名、顺序、每段command/payload/status、reply ABI、segment表、nextSequence、planned/completedSteps/payloadBlocks及段名列表须一致。按序核对每条 command、payload 长度/hash、reply 首字节/长度、commit/reset及释放关闭事件。只有成功flag不够；任一保留首错/取消/短写/错ACK/未关资源均不能 pass。历史导入还核对保留compiled目录的实际文件字节，不只比较几份JSON里的hash声明。

E 另需 `independentObservation=true`、`expectedMatched=true` 及 `recovery={status,action,actual}`。恢复状态为 pass / not_required / state_unconfirmed / not_run；pass E 仅接受前两项，not_required 必须说明动作与实情。必须同时存在同一 identity 的 C pass 与物理 T pass；不同 EXE/工程/payload 或 mock T 均不能借用。只看按钮自身颜色、USB O、枚举成功或重启写返回，都不是独立动作观察。

未执行或环境阻塞使用 not_run/blocked_external；没有完整观察用 partial，不得补造 proof。fail 必须有真实刺激/观察，也不能用外部设备缺失冒充实现失败。

## 5. 恢复与发布规则

| 流水状态 | 工具保留的恢复结论 |
|---|---|
| commit已接受、reset未写/未确认 | commit_accepted_reset_unconfirmed |
| commit尝试写、ACK未确认 | commit_state_unconfirmed |
| 其他已尝试写但未完整完成 | screen_state_unconfirmed |
| 完整传输及释放关闭，未独立看屏 | transport_complete_screen_unobserved |
| 没有成功下载声明 | no_successful_download_claim |

自动回滚、断点续传、断电保持、RTC/历史保持、上载可编辑源均不会从上述结论推导。原状态机的58项已有受控故障测试可复用，但不是物理断电测试。本批未改生产恢复逻辑，也未对原厂运行bug做无证据断言。

汇总保留每条结果与身份；未提供必需用例补 not_run，不能空矩阵通过。任一 fail →总状态 fail；全部 pass →pass；全部 not_run/全部 blocked_external 保留各自状态；其它组合为 partial。releaseRestrictions逐条列出未通过原因、受控T限制、未知固件/硬件限制，不让成功结果覆盖失败历史。

## 6. 文件与清理边界

单JSON≤8MiB、单artifact≤64MiB、artifact合计≤128MiB；拒绝重复JSON键、NaN、超限文件、越出root、无效hash。输出不得指向输入/证据同路径、大小写别名、符号链接或同inode硬链接。先完整验证再写同目录自有临时文件、flush/fsync后os.replace；失败保旧输出，并仅删除本次自有`.stage10-*.tmp`。

测试用独立Z盘 TemporaryDirectory，结束时清理其自有fixture；正式证据不清理，不扫描删除C盘、工作树或历史失败回执。TEMP剩余检查见本目录final-status。工具不提供真实断电/USB/PLC动作入口。
