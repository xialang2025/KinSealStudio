# 独立LZO解码工具

`lzo_decode.exe`只将捕获的LZO流解压为指定长度的字节串，没有链接KinSealStudioLite renderer或设备编译器。它使用仓库中的MIT许可 `third_party/lzokay`；许可证副本为 `LZOKAY-LICENSE.txt`。

输入依次为LZO文件、原始字节输出路径、预期解压长度；长度限制768000B。仅在LZO成功并且实际长度完全匹配时返回0。

构建工具链为MinGW 13.1.0、C++17、`-O2 -static`。在项目根目录可用以下参数重建到新文件，保留归档exe：

```powershell
& 'G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin/g++.exe' `
  -std=c++17 -O2 -static -I third_party/lzokay `
  docs/stage3-evidence/native/tools/lzo_decode.cpp third_party/lzokay/lzokay.cpp `
  -o docs/stage3-evidence/native/tools/lzo_decode-rebuilt.exe
```

编译前将该工具链bin添加到当前子进程PATH，并把TEMP/TMP设到有空间的目录。归档exe、源码、上游源码及编译器SHA256见 `decoder-identity.json`。分析脚本默认相对引用此归档exe，也支持 `--decoder` 指定重新构建的文件。
