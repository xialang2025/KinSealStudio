# 阶段11本地帮助交付

本批新增应用内本地帮助。源码为 `src/local_help.cpp/.h`，用户文件为运行程序旁的 `help/`。默认使用 `QCoreApplication::applicationDirPath()/help`，不依赖开发目录、原厂安装或网络。用户页不放逆向偏移和协议内部字段。

## 接线与有限上下文

主窗口持有 `new LocalHelpController(this)`，构造完成后调用一次 `installContextHelp()`；`action_H` 调用 `showCommand("action_H")`。显式入口可调用 `showCommand(commandId, contextWidget)` 或 `showTopic(topic, contextWidget)`。

F1先接受 `ShortcutOverride`，再按 `KeyPress` 打开，避免主窗口动作提前吞掉嵌套窗口上下文。`QEvent::WhatsThis` 同样适用。过滤器只处理所属编辑器及其子窗口，不接管其他编辑器实例。帮助窗口自身不再次递归处理。弹出的帮助以当前上下文窗口为父窗口；内层对话框销毁后可重新创建，owner销毁会先回收帮助窗口，再销毁资源store。

可给窗口或控件设置 `localHelpTopic` 属性；否则使用 `help/command-map.json` 的有限objectName/标题映射。未知上下文回目录，未知topic/command显示“此入口尚无独立帮助章节”。缺文件、损坏/过大索引和页面、失效锚点都显示恢复同版本help的提示，可以关闭并继续编辑。错误文本不泄漏开发路径。

对象名：`LocalHelpDialog`、`LocalHelpBrowser`、`LocalHelpStatus`、`LocalHelpContents`。没有前置/激活窗口、全局键鼠、系统浏览器和网络进程调用。

现有部分属性窗口的“帮助”按钮仍保留原简短MessageBox；本模块提供F1/WhatsThis上下文，没有通过拦截按钮信号改变其原业务路径。当前映射为8主题、128条命令、39项有限上下文；109条是章节概要，19条仅说明入口/限制。动态最近工程条目、所有右键、双击、嵌套字段和错误分支没有全量盘点，不声明全软件D/U通过。

## 本地文件规则

- 只读帮助根直属普通HTML/HTM；锚点必须实际存在。
- 子资源仅同一根目录PNG/JPG/JPEG/BMP/CSS；拒绝其他类型、子目录、目录对象和文件别名逃逸。
- 根目录、索引、页面/资源的junction/symlink均拒绝；解析后canonical父目录必须等于帮助根。
- 拒绝网络/UNC、data/qrc/mailto等方案、query、路径遍历及目录外file URL。`QTextBrowser::loadResource`覆写后不调用基类fallback，图片和样式请求同样经过边界。
- 索引最大128KiB、页面最大1MiB、资源最大2MiB；topic最大128字符、command最大256字符、目标URL最大2048字符。所有读操作先查文件大小，再以硬上限读取。
- `helpCatalogReport(root)`仅只读检查索引、主题、命令能力说明和HTML带引号的href/src，输出`valid/failures/topicsChecked/commandCount/linkCount`。`globalEntryCoverageComplete`固定为false，报告有效不代表设备或全部入口通过。

## 验证与复核

实际独立Qt6.12/MinGW编译、`QT_QPA_PLATFORM=offscreen`，TEMP/TMP/TMPDIR全部在本阶段Z盘专用根。

- r1保留首个编译失败：启用`QT_NO_CAST_FROM_ASCII`时`QChar(char)`不允许，改用`QLatin1Char`；没有放宽编译选项。
- r2为初次完整离屏回归26通过；只读检查发现F1可能受QAction shortcut优先级影响，补了ShortcutOverride规则和实际竞争事件验证。
- 最终r3：29通过、0失败、0跳过。包含真实Windows junction根/子目录拒绝，有限F1/WhatsThis，竞争QAction，外链/目录/路径跳转/子资源，过大/缺失/篡改索引与页面，真实锚点，关闭重开、owner与嵌套对话框销毁，以及实际8章目录审计。
- `readonly-review-r3.json`为最终只读复核，9项检查通过，无阻断发现；编译日志无warning/error。Qt离屏插件的`propagateSizeHints`提示保留在测试日志，不伪称Windows真实显示/DPI已验。

当前可复用harness位于 `.stage11-run-20261114/help-owned/build-r3/LocalHelpTests.exe`。独立构建/运行argv、输入哈希和程序哈希在`build-r3.json`；`tests-r3.txt`为原始结果。可用同一两份C++源与QtTest重新构建，无设备或SDK依赖。测试真实手册通过`KINSEAL_HELP_SOURCE_ROOT`指向待验help，发布默认路径仍由程序相对定位。

`cleanup-r3.json`记录仅清理本批旧构建、junction探针和重复脚本558393字节；保留最终harness供主任务复用及全部代表日志。未操作历史拒绝清理路径。

## 未闭合项

帮助模块没有执行任何设备下载/上载，也没有升级驱动/SDK。设备编译、USB传输、实际HMI/PLC执行各层不能互相替代。手册保留阶段10语言/通知组合待答、第二次恢复未执行、PLC/保持/上载/长期验证与型号固件身份未闭合。后续取得现场新反馈时须同步这些声明。

安装器、升级和卸载未提供；所有Windows字体、缩放、普通用户、只读安装目录与多显示器条件仍需按发布矩阵分别取证，29项帮助回归不扩大这些范围。
