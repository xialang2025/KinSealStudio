# 扇形独立候选与共享曲线模块复现包

本目录记录最终已冻结生产 API：`paintDeviceWhiteCurvePath(painter, geometry, properties, path, error)`，未保留未使用的可选描边参数。

- 最终 r2 实际执行 60 帧：椭圆 8 个几何 × 4 线填组合，共 32 帧，与共享化前的生产模块相等；扇形 7 个几何 × 4 线填组合，共 28 帧，与独立 Qt6 路径 CLI 相等。全部整帧 RGB565 不同像素数为 0。编译日志 0 字节。
- 这里的扇形参考为独立候选；原厂 27 份 B/S/M 及 coverage 正式验收由唯一原厂采样代理另行归档。本目录未启动原厂、未加载原厂 DLL、未使用 USB。
- `reports/production-sharing-crosscheck-r2.json` 给出最终四文件 SHA、60 个实际案例的几何与结果；r1 是移除可选 API 前的历史运行，不能视作最终源码的再次验收。
- `source/current` 保留四个最终生产文件与只读依赖；`source/old-ellipse` 为原先已验收椭圆模块；`source/independent` 为独立 CLI 及曲线算法。原始构建/运行命令完整保存在 `reports`，原始配方保存在 `scripts`。
- `blobs` 按原始内容 SHA256 去重并 gzip 压缩全部自有展开帧、mask、路径和元数据；`artifact-index-r1.json` 逐项关联原始文件名。没有丢弃全帧。
- `prior-rounding` 完整保留默认扇形旧舍入公式的 1 像素失败复现包；旧失败没有覆盖。

解开 `independent-candidate-reproduction-r1.tar.gz` 后，可运行：

```powershell
& 'C:/Users/lenovo/AppData/Local/Programs/Python/Python311/python.exe' -B ./replay_archived_module.py --output 'Z:/fresh-sector-replay'
```

如 Qt/MinGW 安装路径改变，用 `--qt` 与 `--mingw` 指定。输出目录必须不存在。脚本先验证 manifest，然后在新目录后台编译最终模块，再对 60 份保留全帧逐像素比较；不启动原厂，不访问 USB。原始 CLI 源码中的相对 include 保留原样，单独重编它时按 `reports/sector-cli-build-r2.json` 的相对目录布置依赖。

清理限定为本代理 `sector_followup/production_check/temp` 与 `sector_followup/cli-smoke-local-r2` 中已逐项归档并回解核 SHA 的文件。保留运行中可能使用的 `sector_curve_cli.exe`，不触碰其他代理目录与原厂安装。
