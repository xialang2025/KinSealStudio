# 阶段1私有运行副本只读清理计划

日期：2026-11-10。修改标记：`//20261110-5`。

状态：`not_executed_due_prior_policy_target_ambiguity`。此前已记录自动审批拒绝删除，但无法确认被拒完整目标是否覆盖本次重复文件。主代理明确决定本轮不尝试删除；没有生成删除命令，没有删除任何文件。

唯一范围：`Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native`。当前拥有PID路径实查活动数为0。只枚举runtime-*直接子文件中的4个名称：KinSealStudio5.1-private.exe、model.dat、modelInfo.dat、language.ini。
枚举144文件；其中140个是相同名称+SHA的重复候选，假设字节660440970。实际仍保留181文件、679499152B。

所有.sm、脚本、报告、截图、其他名称、历史阶段0/S02目录均不在清理候选。每种唯一内容优先保留runtime-r4中的一份运行证据；其他独有内容也保留。原安装文件、源文件和正式报告不受影响。

正式身份和行为证据保留在 `Z:\Project\vscode_prj_temp\REA\KinSealStudio\docs\stage1-evidence\native-analysis\manifest.json`、`Z:\Project\vscode_prj_temp\REA\KinSealStudio\docs\stage1-evidence\native-analysis\findings.md`、`Z:\Project\vscode_prj_temp\REA\KinSealStudio\docs\stage1-evidence\native-analysis\model-metadata.json` 及两份目标XML摘录。

完整机器可读记录见 [cleanup-plan.json](cleanup-plan.json)。下表中的“重复候选（未删除）”只是只读盘点。

| 分类 | 绝对路径 | 字节 | SHA256 |
|---|---|---:|---|
| 唯一内容保留证据 | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r4\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-reopen-r1\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-resave-r1\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r1\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r2\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r3\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r4\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r5\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r6\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-external-r1\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-monitor-r1\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-notice-r1\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-notice-r2\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-operations-r1\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-operations-r2\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-empty-a-reopen-r1\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-english-startup-reopen-r1\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-english-startup-reopen-r2\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-english-startup-resave-r1\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-english-startup-resave-r2\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-macro-bound-reopen-r1\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-macro-bound-resave-r1\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-mixed-bilingual-reopen-r1\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-mixed-list-inspect-r1\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-mixed-list-resave-r1\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-mixed-list-resave-r2\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-mixed-list-resave-r3\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-named-d-reopen-r1\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r1\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r2\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r3\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r5\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r6\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r7\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r8\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-static-level-r1\KinSealStudio5.1-private.exe` | 18682368 | `38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89` |
| 唯一内容保留证据 | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r4\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-reopen-r1\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-resave-r1\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r1\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r2\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r3\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r4\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r5\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r6\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-external-r1\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-monitor-r1\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-notice-r1\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-notice-r2\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-operations-r1\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-operations-r2\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-empty-a-reopen-r1\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-english-startup-reopen-r1\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-english-startup-reopen-r2\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-english-startup-resave-r1\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-english-startup-resave-r2\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-macro-bound-reopen-r1\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-macro-bound-resave-r1\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-mixed-bilingual-reopen-r1\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-mixed-list-inspect-r1\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-mixed-list-resave-r1\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-mixed-list-resave-r2\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-mixed-list-resave-r3\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-named-d-reopen-r1\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r1\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r2\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r3\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r5\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r6\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r7\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r8\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-static-level-r1\language.ini` | 224 | `4763f15b4deb45a81da0286907b6b72b98778fd5f462dfaa58aa8d075c8473a0` |
| 唯一内容保留证据 | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r4\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-reopen-r1\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-resave-r1\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r1\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r2\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r3\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r4\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r5\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r6\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-external-r1\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-monitor-r1\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-notice-r1\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-notice-r2\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-operations-r1\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-operations-r2\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-empty-a-reopen-r1\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-english-startup-reopen-r1\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-english-startup-reopen-r2\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-english-startup-resave-r1\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-english-startup-resave-r2\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-macro-bound-reopen-r1\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-macro-bound-resave-r1\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-mixed-bilingual-reopen-r1\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-mixed-list-inspect-r1\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-mixed-list-resave-r1\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-mixed-list-resave-r2\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-mixed-list-resave-r3\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-named-d-reopen-r1\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r1\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r2\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r3\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r5\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r6\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r7\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r8\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-static-level-r1\model.dat` | 25936 | `3be6a993ec1bf25bd5178a7c36bd5b4f17968bd5ae9236cd9a9372dfe7d3c384` |
| 唯一内容保留证据 | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r4\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-reopen-r1\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-resave-r1\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r1\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r2\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r3\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r4\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r5\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bilingual-saveas-r6\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-external-r1\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-monitor-r1\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-notice-r1\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-notice-r2\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-operations-r1\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-bit-operations-r2\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-empty-a-reopen-r1\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-english-startup-reopen-r1\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-english-startup-reopen-r2\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-english-startup-resave-r1\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-english-startup-resave-r2\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-macro-bound-reopen-r1\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-macro-bound-resave-r1\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-mixed-bilingual-reopen-r1\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-mixed-list-inspect-r1\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-mixed-list-resave-r1\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-mixed-list-resave-r2\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-mixed-list-resave-r3\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-named-d-reopen-r1\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r1\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r2\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r3\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r5\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r6\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r7\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-r8\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
| 重复候选（未删除） | `Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage1-20261110\native\runtime-static-level-r1\modelInfo.dat` | 161214 | `7b832c98b6ba762fed622fbd67aad92627d6498d6701d2dcd2fa9d07a59adc67` |
