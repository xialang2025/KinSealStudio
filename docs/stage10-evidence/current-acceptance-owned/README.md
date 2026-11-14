# 阶段10 · 当前实物记录与严格验收矩阵

本目录仅汇总已经存在的编译/下载回执和用户实际回答；生成器不操作USB、PLC、屏幕或电源，不修改已冻结的验收工具。

## 首轮 r1

正式冻结入口为 `current-cases-r1.json`、`current-summary-r1.json`、`validation-r1.json`；使用冻结工具真实执行 `validate --require-complete`，退出3，总结果partial。17条声明中，C产物及既有命令回执审计pass 3条、partial 11条、not_run 1条、blocked_external 2条；八个阶段任务总体都未关闭。

`round-r1/`保存23份小文本原始字节，共754,612 B，包含用户回答、任务书/预期来源、实际KSP、编译manifest和原始下载流水。这样后续追加功能按钮答复、更新任务书不会改写首轮证据。两个大EXE、Stage9原发布manifest及编译二进制块仍引用原来的、明确hash绑定的位置，没有复制大运行包。每次重放都检查其实际hash。

首次现场汇总的`current-cases.json/current-summary.json/validation.json`包含当时仍可追加的原始用户记录路径，只是非正式草稿。确认冻结替代完整且正式artifact无引用后已精确清除，逐文件大小/hash见`provisional-cleanup.json`；正式重放只使用带`-r1`的冻结入口。生成器`build_current_records.py`记录首轮生成过程，检测到正式r1后拒绝重跑；后续结果使用独立r2路径。

| 样例 | 实际传输流水 | 用户实际确认 | 严格T/E |
|---|---|---|---|
| bit-lamp-local | 12段、提交、reset/尾读、释放关闭完整 | 标识/4按钮/3独立指示框正常；LB0两次交替复原、LB1置1再复0、LB2按住1松手0 | partial |
| word-lamp-local | 12段、提交、reset/尾读、释放关闭完整 | LW20独立灯选择0/1/4，LW30独立灯选择0/3，无宽度提示 | partial |
| restore-macro-14 | 14段、提交、reset/尾读、释放关闭完整 | 恢复S06-MACRO-14-1114标识及“写常数并执行宏”按钮，无宽度提示 | partial |

局部实际观察符合预期，未被写成失败。严格T/E保持partial的原因是唯一硬件序列号和屏内固件尚未取得；USB实例路径仅作端点身份，不冒充唯一序列。冻结schema明确要求唯一hardwareId才能pass物理T/E，未绕过此规则。`*-facts.json`保留三个`transportComplete=true`与用户原话，故严格status与局部现场事实可分别评估。

3条C pass仅证明本次实际compile/verify成功回执及其EXE/KSP/contract/每段编译文件和wire字节一致；不把源码manifest、EXE、原始KSP及规范化模型hash混成一个身份，不外推未编译配置。C proof的observedAt是本次只读产物复核的真实时刻，不是虚构的早先编译/触摸时间。

首轮恢复只证明已知macro14测试工程重新下载并恢复可见标识/按钮。它不是原工程上载、原项目备份、宏执行、LB/LW/RTC/历史值恢复或断电保持。原始用户记录的recordedAt是答复收录时刻；每次触摸的精确刺激时刻未采集，明确为unknown，不据此计算延迟。

总体保留：完整五类控件/数字类型/边界、权限/通知/语言/重叠、多画面/系统数据配置、真实PLC、上载/断电/值保持、HMI时序/长期运行及发布全矩阵。S10-04的首轮blocked_external包含未完成的编译前置依赖，后续回合需区分内部未实现前置（not_run）与真正外部PLC端点阻塞，不能把未实现功能当作已完成。

## 重放

```powershell
& 'C:/Users/lenovo/AppData/Local/Programs/Python/Python311/python.exe' -B tools/stage10_acceptance.py validate docs/stage10-evidence/current-acceptance-owned/current-cases-r1.json another-r1-summary.json --root Z:/Project/vscode_prj_temp/REA --require-complete
```

预期退出3；输出必须使用新文件名，不得覆盖输入或原证据。`validation-r1.json`记录实际命令、输入/输出hash和冻结工具hash。

## 后续功能页里程碑，尚未冻结 r2

`physical-function-r1/run.json`已有12段完整传输。原始用户记录`function-visibility-touch`回答“显现、隐藏和出现后的触控全部符合”：LB41复0隐藏、置1显示，出现后的GATED CN触控使独立标识回到CN，再次复0后隐藏。这里没有隐藏态触控尝试、显现与触控权限分离、组合条件或全语言/状态结论。

通知与剩余独立语言问题仍待用户答复，功能页后的第二次macro14恢复尚未执行。当前不换屏、不推测答复、不创建physical-restore-r2或r2通过回执。`build_round_r2.py`只作为后续可执行源保留，须提供真实答复ID及实际最终恢复记录才可运行；首轮r1冻结输入和回执保持不变。
