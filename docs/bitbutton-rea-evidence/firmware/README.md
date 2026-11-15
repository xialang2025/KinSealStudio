# 位按钮 ARM 固件静态证据

入口为[专项报告第9节](../../位按钮行为编码逆向分析.md)，逐项结论索引为[findings.json](findings.json)，独立分析说明为[findings.md](findings.md)。源固件SHA256固定为 `74d510f150dc8edadf8e967bdb3c00ca5790ca68b9da6f906125a5234a485c24`。

这些产物来自Windows上已有Ghidra 12.1.4与Capstone 5.0.7的直接静态分析，不是REA Evidence记录，也没有执行固件、读取屏内固件或连接设备。PC模拟器的78条REA记录位于上级目录。所有地址限各自固定SHA，不能跨二进制使用。

复现参数见[reproduction.md](reproduction.md)。该记录列出原执行路径；若从本归档重放，先将脚本放到自有新目录并相应调整输出路径，不覆盖现有证据。原固件、可再生内存映像和Ghidra项目不提交Git。`archive-manifest.json`校验原分析的31个保留文件；本README与派生JSON索引不在其清单中，由上级manifest统一校验。函数边界/反编译失败及首次路径失败保留在日志与导出，窄范围原字补足自动函数边界的缺口。

清理详情见[cleanup.json](cleanup.json)：自动审批两次拒绝，仅返回blocked by policy，实际删除0字节；自有Ghidra项目/temp合计35,908,325字节保留且不再重试。新临时目录放Z盘，未泛删C盘或用户文件。残留路径另入统一台账。
