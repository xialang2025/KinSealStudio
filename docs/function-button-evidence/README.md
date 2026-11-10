# 功能按钮交付证据

版本0.7.0，2026年11月10日。程序SHA256：`259CDA65C3F16A80950BAD96E55D70A75F8C3134B66DE94A36BF2533AE752C2D`。

- [交付结果](delivery-result.json)：最终Windows各套结果、便携检查数、USB回执和未确认边界。
- [Windows集成43项](integration-windows.txt)、[功能窗口Windows40项](function-ui-windows.txt)、[offscreen40项](function-ui-offscreen.txt)、[原厂源45项](native-source-windows.txt)、[功能设备编码71项](native-function-windows.txt)。
- [便携包131项/32样例](portable-verification.json)：移除安装Qt与编译器PATH后验证；预期拒绝的旧原型样例按预期失败计为检查通过。
- [87条原厂记录审计](native-corpus-audit.json)、[原厂设备补证](原厂设备补证.md)、[源格式调查原文](source-findings.txt)。这些记录中的原始文件名对应工作区`.scratch/kinseal-function-20261110/`，未把省略的原始目录当作随包文件。
- [REA资源Evidence](rea-evidence.json)、[首次Ghidra超时](rea-ghidra-timeout.json)：资源身份、普通Ghidra文本与动态编译证据分别记录。
- [真实USB回执](usb-function-integration.json)：07:53:37–07:53:48，数据段及提交确认，重启已发送；现场显示/触控确认仍为false。

原厂单语60控件矩阵、合法Space8及双语单控件已真正编译；原厂双语60控件大矩阵导出框未出现的额外验证尚未闭环。源往返、独立设备编译、USB确认和现场触控属于不同证据。自动关机保持取消。
