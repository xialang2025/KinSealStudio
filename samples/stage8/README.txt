阶段8普通零地址变量宏源码样例。
打开ordinary-two-macros.ksp，选择工具中的宏指令编辑器，可编辑ClearLB1/SetLB1或插入函数库示例。
两个.c为UTF-8源码，供单独导入；宏包含内部LocalBit[1]直接访问，没有25项地址关联变量。
编译须另配现有AMT ARM SDK。命令：KinSealStudioLite.exe --compile-macros samples/stage8/ordinary-two-macros.ksp report.json
页面没有控件、没有宏绑定或调度；本样例不代表HMI下载或执行已验证。
