"""Verify Lite-side compile determinism for checklist S05-02.

Repeats ``--compile-project`` with independent KinSealStudioLite.exe processes,
diffs every artifact byte-by-byte across runs, groups the differing offset
segments per container file, and writes a Chinese Markdown report.

Exit codes: 0 = all runs identical, 3 = differences found, 2 = input/tool error.
"""
# -*- coding: utf-8 -*-
import argparse
import hashlib
import itertools
import json
import os
import subprocess
import sys
from datetime import datetime
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]                  # KinSealStudio repository root
DEFAULT_EXE = REPO_ROOT / "build" / "KinSealStudioLite.exe"      # offline Lite compiler entry
DEFAULT_OUT_ROOT = REPO_ROOT.parent / ".scratch" / ("kinseal-" + datetime.now().strftime("%Y%m%d")) / "determinism"
DEFAULT_QT_PLUGIN_PATH = r"G:\01_Professional\QT6\Installed\6.12.0\mingw_64\plugins"

# Historical acceptance reports (docs/阶段5_*编译与差分验收.md) observed unstable raw
# bytes in these containers; they are reported as "已知非确定容器待归因".
KNOWN_NONDETERMINISTIC = {"SysPara.dat", "AlarmPara.dat", "upLoad.knr"}
# Static segments / pixel frames / font payloads expected to be byte-stable.
KNOWN_DETERMINISTIC = {"SheetPara.dat", "Collect.dat", "AddrPara.dat", "PicLenPara.dat",
                       "Curve.dat", "SheetPixel.dat", "Modbus_RTU_com1.bin",
                       "ControlPara.dat", "ControlPixel.dat", "MarcoVar.dat", "MarcoRun.dat"}
MANIFEST_NAME = "compiler-manifest.json"  # derived from payloads, so it changes together with them


def sha256_bytes(data):
    """Input: raw bytes. Return: lowercase hex SHA-256 digest."""
    return hashlib.sha256(data).hexdigest()


def unique_root(base):
    """Input: requested output root. Return: first non-existing path (base, base-2, base-3, ...)."""
    candidate = base
    suffix = 2
    while candidate.exists():
        candidate = base.with_name(base.name + "-" + str(suffix))
        suffix += 1
    return candidate


def run_compile(exe, sample, out_dir, timeout, plugin_path):
    """Input: exe/sample paths, fresh output dir, per-run timeout, Qt plugin path.
    Return: record dict with command, env flags, stdout and retry info.
    A 30 s unresponsive run is treated as a hidden-modal-dialog hang: kill it and
    retry exactly once with QT_PLUGIN_PATH forced on."""
    command = [str(exe), "--compile-project", str(sample), str(out_dir)]
    record = {"command": command, "attempts": [], "ok": False}
    for attempt in range(1, 3):
        env = os.environ.copy()
        env["QT_QPA_PLATFORM"] = "offscreen"        # never touch the user's desktop
        if plugin_path:
            env["QT_PLUGIN_PATH"] = plugin_path     # required, otherwise Qt shows a hidden modal box
        attempt_record = {"attempt": attempt, "qtPluginPath": env.get("QT_PLUGIN_PATH", ""), "timedOut": False}
        try:
            proc = subprocess.run(command, env=env, capture_output=True, text=True,
                                  encoding="utf-8", errors="replace", timeout=timeout)
            attempt_record["returncode"] = proc.returncode
            attempt_record["stdout"] = proc.stdout.strip()
            attempt_record["stderr"] = proc.stderr.strip()
            record["attempts"].append(attempt_record)
            if proc.returncode == 0:
                record["ok"] = True
                break
            break                                   # deterministic compile failure is an input error, not a hang
        except subprocess.TimeoutExpired:
            attempt_record["timedOut"] = True       # process killed by subprocess.run; hidden dialog suspected
            record["attempts"].append(attempt_record)
            if attempt == 2:
                break
    return record


def collect_artifacts(run_dir):
    """Input: one run output directory. Return: dict of file name -> raw bytes."""
    artifacts = {}
    for path in sorted(run_dir.iterdir()):
        if path.is_file():
            artifacts[path.name] = path.read_bytes()
    return artifacts


def hexdump(data, lo, hi, mark_start=None, mark_end=None):
    """Input: bytes plus a render range and an optional highlighted span.
    Return: hex string; highlighted bytes are wrapped in brackets."""
    tokens = []
    for index in range(lo, min(hi, len(data))):
        token = "%02x" % data[index]
        if mark_start is not None and mark_start <= index < mark_end:
            token = "[" + token + "]"
        tokens.append(token)
    return " ".join(tokens)


def diff_segments(reference, other, merge_gap=8):
    """Input: two payloads. Return: merged difference segments (start, end) lists."""
    if len(reference) != len(other):
        return [{"start": 0, "end": max(len(reference), len(other)), "sizeMismatch": True}]
    indices = [i for i in range(len(reference)) if reference[i] != other[i]]
    if not indices:
        return []
    segments = []
    start = previous = indices[0]
    for index in indices[1:]:
        if index - previous > merge_gap:            # separate stable islands into their own segments
            segments.append({"start": start, "end": previous + 1, "sizeMismatch": False})
            start = index
        previous = index
    segments.append({"start": start, "end": previous + 1, "sizeMismatch": False})
    return segments


def classify(name):
    """Input: artifact file name. Return: report category label."""
    if name == MANIFEST_NAME:
        return "编译清单（随载荷联动）"
    if name in KNOWN_NONDETERMINISTIC:
        return "历史已知非确定容器"
    if name in KNOWN_DETERMINISTIC:
        return "已知确定性文件（静态段/字体帧等）"
    return "其他"


def compare_runs(run_artifacts):
    """Input: per-run artifact dicts. Return: per-file comparison records."""
    names = sorted(set(itertools.chain.from_iterable(run_artifacts)))
    records = []
    for name in names:
        digests = [sha256_bytes(arts[name]) if name in arts else None for arts in run_artifacts]
        record = {"name": name, "category": classify(name),
                  "sizes": [len(arts[name]) if name in arts else None for arts in run_artifacts],
                  "digests": digests, "deterministic": len(set(digests)) == 1 and digests[0] is not None,
                  "pairs": []}
        if record["deterministic"]:
            records.append(record)
            continue
        present = [(i, d) for i, d in enumerate(digests) if d is not None]
        counts = {}
        first_seen = {}
        for index, digest in present:
            counts[digest] = counts.get(digest, 0) + 1
            first_seen.setdefault(digest, index)
        # Reference = most common payload; ties resolve to the earliest run so diffs stay stable.
        reference_digest = None
        for digest, count in counts.items():
            if (reference_digest is None or count > counts[reference_digest]
                    or (count == counts[reference_digest] and first_seen[digest] < first_seen[reference_digest])):
                reference_digest = digest
        reference_run = first_seen[reference_digest]
        reference_bytes = run_artifacts[reference_run][name]
        for index, digest in enumerate(digests):
            if digest == reference_digest:
                continue
            if digest is None:
                record["pairs"].append({"referenceRun": reference_run + 1, "otherRun": index + 1,
                                        "missing": True, "segments": [], "diffBytes": 0})
                continue
            segments = diff_segments(reference_bytes, run_artifacts[index][name])
            diff_bytes = sum(seg["end"] - seg["start"] for seg in segments)
            record["pairs"].append({"referenceRun": reference_run + 1, "otherRun": index + 1,
                                    "missing": False, "segments": segments, "diffBytes": diff_bytes})
        records.append(record)
    return records


def render_report(args, run_records, run_artifacts, comparisons, sample_sha):
    """Input: tool options, per-run metadata, artifacts and comparison records.
    Return: full Markdown report text (Chinese)."""
    lines = []
    total_files = len(comparisons)
    stable_files = [c for c in comparisons if c["deterministic"]]
    unstable_files = [c for c in comparisons if not c["deterministic"]]
    verdict = "通过：全部产物逐字节一致" if not unstable_files else "不通过：存在非确定字节"
    lines.append("# Lite 侧编译确定性差分报告（清单 S05-02）")
    lines.append("")
    lines.append("- 生成时间：%s" % datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
    lines.append("- 样例输入：`%s`（SHA256 `%s`）" % (args.sample, sample_sha))
    lines.append("- 编译器：`%s`" % args.exe)
    lines.append("- 重复次数：%d 次独立 CLI 进程，输出目录：" % args.runs)
    for run_record, out_dir in zip(run_records, args.run_dirs):
        lines.append("  - run%d：`%s`" % (run_record["run"], out_dir))
    lines.append("- 每次运行超时阈值：%d 秒；超时按隐藏模态框挂死处理，杀进程后带 `QT_PLUGIN_PATH` 重试一次。" % args.timeout)
    lines.append("- 运行环境：`QT_QPA_PLATFORM=offscreen`，`QT_PLUGIN_PATH=%s`。" % (args.qt_plugin_path or "<继承外部环境>"))
    lines.append("- 进程替代说明：CLI 为单命令进程，同进程内二次编译不可行；本工具以「%d 次独立 CLI 进程」"
                 "替代同进程重复（等价于\"进程A跑2次+进程B跑1次\"的进程间隔离方案），特此如实说明。" % args.runs)
    lines.append("- 工具命令行：`%s`" % " ".join(sys.argv))
    lines.append("")
    lines.append("## 编译命令与运行结果")
    lines.append("")
    lines.append("| 次数 | 命令行 | 退出码 | 编译返回消息 | 重试 |")
    lines.append("| --- | --- | --- | --- | --- |")
    for run_record in run_records:
        attempt = run_record["attempts"][-1]
        retried = "是（首次超时）" if len(run_record["attempts"]) > 1 else "否"
        message = ""
        if attempt.get("stdout"):
            try:
                message = json.loads(attempt["stdout"]).get("message", "")
            except json.JSONDecodeError:
                message = attempt["stdout"][:60]
        lines.append("| run%d | `%s` | %s | %s | %s |"
                     % (run_record["run"], " ".join(run_record["command"]), attempt.get("returncode", "超时"),
                        message, retried))
    lines.append("")
    lines.append("## 各次产物 SHA256 清单")
    lines.append("")
    for run_record, arts in zip(run_records, run_artifacts):
        lines.append("### run%d" % run_record["run"])
        lines.append("")
        lines.append("| 文件 | 长度（字节） | SHA256 |")
        lines.append("| --- | --- | --- |")
        for name in sorted(arts):
            lines.append("| %s | %d | `%s` |" % (name, len(arts[name]), sha256_bytes(arts[name])))
        lines.append("")
    lines.append("## 差分结论：%s" % verdict)
    lines.append("")
    lines.append("- 对比文件总数：%d；逐字节一致：%d；存在差异：%d。" % (total_files, len(stable_files), len(unstable_files)))
    if unstable_files:
        for comparison in unstable_files:
            lines.append("- `%s`（%s）：非确定区待归因。" % (comparison["name"], comparison["category"]))
    lines.append("")
    lines.append("## 已知确定性文件汇总（静态段/字体帧等）")
    lines.append("")
    lines.append("| 文件 | 本次结果 |")
    lines.append("| --- | --- |")
    for name in sorted(KNOWN_DETERMINISTIC):
        comparison = next((c for c in comparisons if c["name"] == name), None)
        state = "一致" if comparison and comparison["deterministic"] else ("差异" if comparison else "本次产物缺失")
        lines.append("| %s | %s |" % (name, state))
    lines.append("")
    lines.append("## 历史已知非确定容器汇总（SysPara/AlarmPara/upLoad.knr 等）")
    lines.append("")
    lines.append("| 文件 | 本次结果 | 差异段数 | 差异字节总数 | 首段偏移（十进制） | 首段偏移（十六进制） | 状态 |")
    lines.append("| --- | --- | --- | --- | --- | --- | --- |")
    for name in sorted(KNOWN_NONDETERMINISTIC):
        comparison = next((c for c in comparisons if c["name"] == name), None)
        if comparison is None:
            lines.append("| %s | 本次产物缺失 | - | - | - | - | - |" % name)
            continue
        if comparison["deterministic"]:
            lines.append("| %s | 一致（本次样例下复现稳定） | 0 | 0 | - | - | 归因可收敛 |" % name)
            continue
        pair = comparison["pairs"][0]
        segment = pair["segments"][0]
        lines.append("| %s | 差异 | %d | %d | %d | 0x%X | 非确定区待归因 |"
                     % (name, len(pair["segments"]), pair["diffBytes"], segment["start"], segment["start"]))
    lines.append("")
    lines.append("## 非确定区明细表")
    lines.append("")
    if not unstable_files:
        lines.append("本次三次编译产物逐字节一致，无非确定区。")
        lines.append("")
    for comparison in unstable_files:
        lines.append("### `%s`（%s，非确定区待归因）" % (comparison["name"], comparison["category"]))
        lines.append("")
        for pair in comparison["pairs"]:
            if pair["missing"]:
                lines.append("- run%d 缺少该文件，run%d 存在（结构级差异）。" % (pair["otherRun"], pair["referenceRun"]))
                continue
            reference = run_artifacts[pair["referenceRun"] - 1][comparison["name"]]
            other = run_artifacts[pair["otherRun"] - 1][comparison["name"]]
            lines.append("- 对比值：run%d vs run%d；差异段 %d 个（展示前 %d 个），差异字节共 %d。"
                         % (pair["referenceRun"], pair["otherRun"], len(pair["segments"]),
                            min(len(pair["segments"]), args.max_segments), pair["diffBytes"]))
            lines.append("")
            lines.append("| 段 | 偏移（十进制） | 偏移（十六进制） | 长度 | 参考侧字节 | 对比侧字节 |")
            lines.append("| --- | --- | --- | --- | --- | --- |")
            for number, segment in enumerate(pair["segments"][:args.max_segments], 1):
                ref_hex = hexdump(reference, segment["start"], segment["end"])
                other_hex = hexdump(other, segment["start"], segment["end"])
                length = segment["end"] - segment["start"]
                if segment.get("sizeMismatch"):
                    ref_hex = "长度=%d" % len(reference)
                    other_hex = "长度=%d" % len(other)
                    length = abs(len(reference) - len(other))
                lines.append("| %d | %d | 0x%X | %d | `%s` | `%s` |" % (number, segment["start"], segment["start"], length, ref_hex, other_hex))
            if len(pair["segments"]) > args.max_segments:
                lines.append("")
                lines.append("（其余 %d 个差异段未展开，仅计数。）" % (len(pair["segments"]) - args.max_segments))
            lines.append("")
            context = max(0, args.context)          # neighbouring bytes shown before/after each segment
            shown = 0
            for segment in pair["segments"][:args.max_segments]:
                if shown >= 2 and not args.all_context:
                    break
                lo = max(0, segment["start"] - context)
                hi = max(segment["end"] + context, segment["start"] + 1)
                lines.append("差异段 0x%X（run%d vs run%d，±%d 字节上下文，`[ ]` 内为差异字节）：" % (segment["start"], pair["referenceRun"], pair["otherRun"], context))
                lines.append("")
                lines.append("```text")
                lines.append("run%d: %s" % (pair["referenceRun"], hexdump(reference, lo, hi, segment["start"], segment["end"])))
                lines.append("run%d: %s" % (pair["otherRun"], hexdump(other, lo, hi, segment["start"], segment["end"])))
                lines.append("```")
                lines.append("")
                shown += 1
    lines.append("## 归因提示")
    lines.append("")
    lines.append("历史验收（docs/阶段5_扇形默认角度背景编译与差分验收.md 等）记录了 27 次同源编译对比，")
    lines.append("其中 SysPara/AlarmPara/Usbload.bin/load.kns/upLoad.knr 等容器存在非确定原始字节或长度，原因未知。")
    lines.append("本工具不做忽略掩码、不删改容器，只按文件名分组呈现差异偏移段；差异容器统一标注\"非确定区待归因\"，")
    lines.append("供后续在 buildProjectPayload/publishDevicePackage 链路上定位随机源（时间戳、未初始化字段、随机序等）。")
    lines.append("")
    return "\n".join(lines)


def main():
    """Input: command line arguments. Return: process exit code (0/3/2)."""
    parser = argparse.ArgumentParser(
        description="Lite 侧编译确定性差分工具：重复运行 --compile-project 并逐字节对比产物（S05-02）。")
    parser.add_argument("sample", type=Path, help="有效 .ksp 工程样例路径")
    parser.add_argument("--runs", type=int, default=3, help="独立编译进程次数（默认 3）")
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE, help="KinSealStudioLite.exe 路径")
    parser.add_argument("--out-root", type=Path, default=DEFAULT_OUT_ROOT, help="编译输出根目录（默认在仓库外 .scratch 下）")
    parser.add_argument("--report", type=Path, default=None, help="中文 Markdown 报告输出路径（默认 <输出根目录上级>/determinism-report.md）")
    parser.add_argument("--timeout", type=int, default=30, help="单次编译超时秒数（默认 30，超时视为挂死并重试一次）")
    parser.add_argument("--max-segments", type=int, default=8, help="每文件最多展开的差异段数量（默认 8）")
    parser.add_argument("--context", type=int, default=16, help="差异段邻近上下文字节数（默认 ±16）")
    parser.add_argument("--qt-plugin-path", default=DEFAULT_QT_PLUGIN_PATH, help="QT_PLUGIN_PATH（留空字符串则继承外部环境）")
    parser.add_argument("--all-context", action="store_true", help="为所有展开的差异段输出上下文（默认只展示前 2 段）")
    args = parser.parse_args()

    sample = args.sample.resolve()
    exe = args.exe.resolve()
    if not sample.is_file():
        print("工具错误：样例不存在：%s" % sample, file=sys.stderr)
        return 2
    if not exe.is_file():
        print("工具错误：编译器不存在：%s" % exe, file=sys.stderr)
        return 2
    if args.runs < 2:
        print("工具错误：--runs 至少为 2 才能构成差分。", file=sys.stderr)
        return 2

    out_root = unique_root(args.out_root.resolve())
    out_root.mkdir(parents=True, exist_ok=True)
    run_dirs = [out_root / ("run%d" % index) for index in range(1, args.runs + 1)]
    args.run_dirs = run_dirs                      # render_report lists the actual per-run directories
    for run_dir in run_dirs:
        if run_dir.exists():
            print("工具错误：输出目录已存在（编译器拒绝覆盖）：%s" % run_dir, file=sys.stderr)
            return 2
    report_path = args.report.resolve() if args.report else out_root.parent / "determinism-report.md"

    run_records = []
    for index, run_dir in enumerate(run_dirs, 1):
        record = run_compile(exe, sample, run_dir, args.timeout, args.qt_plugin_path)
        record["run"] = index
        run_records.append(record)
        if not record["ok"]:
            last = record["attempts"][-1]
            if last.get("timedOut"):
                print("工具错误：run%d 两次尝试均超时（疑似隐藏模态框挂死）：%s" % (index, run_dir), file=sys.stderr)
            else:
                print("工具错误：run%d 编译失败（退出码 %s）：%s" % (index, last.get("returncode"), last.get("stdout", "")), file=sys.stderr)
            return 2

    run_artifacts = [collect_artifacts(run_dir) for run_dir in run_dirs]
    if any(len(arts) == 0 for arts in run_artifacts):
        print("工具错误：某次运行输出目录为空，无法差分。", file=sys.stderr)
        return 2
    comparisons = compare_runs(run_artifacts)
    sample_sha = sha256_bytes(sample.read_bytes())
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(render_report(args, run_records, run_artifacts, comparisons, sample_sha),
                           encoding="utf-8")

    unstable = [c["name"] for c in comparisons if not c["deterministic"]]
    print("报告：%s" % report_path)
    if unstable:
        print("结论：存在非确定字节（%d/%d 个文件）：%s" % (len(unstable), len(comparisons), ", ".join(unstable)))
        return 3
    print("结论：三次编译产物逐字节一致，确定性通过。")
    return 0


if __name__ == "__main__":
    sys.exit(main())
