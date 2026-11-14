"""Archive only this agent's candidate evidence; verify all payloads before bounded cleanup."""
from pathlib import Path
from collections import Counter
import gzip
import hashlib
import json
import shutil
import tarfile


here = Path(__file__).resolve().parent
stage = here.parent
work = stage.parent / 'kinseal-stage4-20261112/worktree'
check = here / 'production_check'
destination = work / 'docs/stage5-evidence/native/sector/reproduction-20261114/independent-candidate'
destination.mkdir(parents=True, exist_ok=False)


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def write(relative, raw):
    target = destination / relative
    target.parent.mkdir(parents=True, exist_ok=True)
    with target.open('xb') as stream:
        stream.write(raw)


def copy(path, relative):
    write(relative, path.read_bytes())


result = json.loads((check / 'production-sharing-crosscheck-r2.json').read_text(encoding='utf-8'))
assert result['ellipseCases'] == 32 and result['sectorCandidateCases'] == 28 and result['allEqual']
assert len(result['cases']) == 60 and all(x['differentPixels'] == 0 for x in result['cases'])
assert result['buildLogBytes'] == 0 and result['unusedStrokeOutlineExtensionRemoved']
for name, expected in result['productionSha256'].items():
    assert sha((work / 'src' / name).read_bytes()) == expected, name
for name in (*result['productionSha256'], 'device_polygon_raster.cpp', 'device_polygon_raster.h', 'graphic_properties.h'):
    copy(work / 'src' / name, 'source/current/' + name)
for name in ('device_ellipse_raster.cpp', 'device_ellipse_raster.h'):
    copy(check / 'old-ellipse' / name, 'source/old-ellipse/' + name)
for name in ('ellipse_curve_candidate.cpp', 'ellipse_curve_candidate.h'):
    copy(stage / 'ellipse_algorithm' / name, 'source/independent/' + name)
copy(here / 'sector_curve_cli.cpp', 'source/independent/sector_curve_cli.cpp')
copy(check / 'paint_probe.cpp', 'source/paint_probe.cpp')
for name in ('crosscheck.py', 'recheck_final_api.py'):
    copy(check / name, 'scripts/' + name)
for name in ('build_sector_cli.py', 'archive_final_candidate.py'):
    copy(here / name, 'scripts/' + name)
copy(here / 'replay_archived_module.py', 'replay_archived_module.py')
for name in ('production-sharing-crosscheck-r1.json', 'production-sharing-crosscheck-r2.json', 'commands-r1.json',
             'commands-r2.json', 'old-build-r1.log', 'current-build-r1.log', 'current-final-build-r2.log'):
    copy(check / name, 'reports/' + name)
for name in ('sector-cli-build-r1.json', 'sector-cli-build-r2.json', 'sector-cli-build-r1.log', 'sector-cli-build-r2.log'):
    copy(here / name, 'reports/' + name)
for path in (here / 'sector_curve_cli.exe', check / 'paint-old.exe', check / 'paint-current.exe', check / 'paint-current-final.exe'):
    write('binaries/' + path.name + '.gz', gzip.compress(path.read_bytes(), compresslevel=9, mtime=0))

# Preserve the earlier one-pixel failure as an exact compact copy, without mutating its original evidence.
earlier = work / 'docs/stage5-evidence/native/sector-rounding-20261114'
copy(earlier / 'sector-rounding-reproduction-r1.tar.gz', 'prior-rounding/sector-rounding-reproduction-r1.tar.gz')
copy(earlier / 'sector-rounding-r1.json', 'prior-rounding/sector-rounding-r1.json')
copy(earlier / '默认扇形舍入排查与候选规则.md', 'prior-rounding/默认扇形舍入排查与候选规则.md')

# Keep all self-owned expanded products content-addressed and gzip compressed, with every source-name association.
records = []
blob_lookup = {}
cleanup = []
for directory in (check / 'temp', here / 'cli-smoke-local-r2'):
    resolved = directory.resolve(strict=True)
    assert resolved.is_relative_to(here.resolve())
    for path in sorted(directory.iterdir()):
        assert path.is_file() and not path.is_symlink(), path
        assert path.suffix in ('.rgb565', '.mask', '.pathstream', '.json'), path
        raw = path.read_bytes()
        digest = sha(raw)
        if digest not in blob_lookup:
            relative = 'blobs/' + digest + '.gz'
            write(relative, gzip.compress(raw, compresslevel=9, mtime=0))
            blob_lookup[digest] = dict(path=relative, sha256=digest, bytes=len(raw))
        records.append(dict(source=path.relative_to(here).as_posix(), sha256=digest, bytes=len(raw)))
        cleanup.append(dict(path=str(path.resolve()), sha256=digest, bytes=len(raw)))
for case in result['cases']:
    assert case['referenceSha256'] in blob_lookup and case['finalSha256'] in blob_lookup
write('artifact-index-r1.json', (json.dumps(dict(artifacts=records, blobs=list(blob_lookup.values())), indent=2) + '\n').encode())
write('cleanup-plan-r1.json', (json.dumps(dict(allowedDirectories=[str((check/'temp').resolve()), str((here/'cli-smoke-local-r2').resolve())],
                                           files=cleanup, totalBytes=sum(x['bytes'] for x in cleanup)), indent=2) + '\n').encode())
readme = '''# 扇形独立候选与共享曲线模块复现包

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
'''
write('README.md', readme.encode('utf-8'))
payloads = []
for path in sorted(destination.rglob('*')):
    if path.is_file():
        raw = path.read_bytes()
        payloads.append(dict(path=path.relative_to(destination).as_posix(), bytes=len(raw), sha256=sha(raw)))
manifest = dict(files=payloads, verifiedModuleCases=60, ellipseCases=32, sectorIndependentCases=28,
                productionSha256=result['productionSha256'], nativeAcceptanceStoredSeparately=True,
                nativeProcessLaunched=False, originalDllLoaded=False, usbAccessed=False)
write('manifest-r1.json', (json.dumps(manifest, indent=2) + '\n').encode())
archive = destination / 'independent-candidate-reproduction-r1.tar.gz'
with tarfile.open(archive, 'x:gz', compresslevel=9) as stream:
    for record in payloads:
        stream.add(destination / record['path'], arcname=record['path'], recursive=False)
    stream.add(destination / 'manifest-r1.json', arcname='manifest-r1.json', recursive=False)
verified = 0
with tarfile.open(archive, 'r:gz') as stream:
    archive_manifest = json.loads(stream.extractfile('manifest-r1.json').read())
    assert archive_manifest == manifest
    for record in manifest['files']:
        raw = stream.extractfile(record['path']).read()
        assert len(raw) == record['bytes'] and sha(raw) == record['sha256'], record['path']
        if record['path'].startswith('blobs/'):
            unpacked = gzip.decompress(raw)
            digest = record['path'].split('/')[-1].split('.')[0]
            assert sha(unpacked) == digest
            assert len(unpacked) == blob_lookup[digest]['bytes']
        verified += 1
roundtrip = dict(archive=archive.name, archiveBytes=archive.stat().st_size, archiveSha256=sha(archive.read_bytes()),
                 verifiedPayloadFiles=verified, verifiedBlobs=len(blob_lookup), sourceArtifacts=len(records),
                 verifiedModuleCases=60, productionSha256=result['productionSha256'])
write('roundtrip-verification-r1.json', (json.dumps(roundtrip, indent=2) + '\n').encode())

# Delete only files whose exact absolute identity and content match the round-tripped cleanup plan.
allowed = {(check / 'temp').resolve(), (here / 'cli-smoke-local-r2').resolve()}
deleted = Counter()
for record in cleanup:
    path = Path(record['path'])
    assert path.resolve(strict=True) == path and path.parent in allowed and not path.is_symlink()
    raw = path.read_bytes()
    assert len(raw) == record['bytes'] and sha(raw) == record['sha256']
    path.unlink()
    deleted[path.suffix] += 1
cleanup_result = dict(deletedFiles=len(cleanup), deletedBytes=sum(x['bytes'] for x in cleanup),
                      byExtension=dict(deleted), archiveRoundtripVerified=True,
                      nativeOwnerExecutableRetained=(here / 'sector_curve_cli.exe').is_file(),
                      remainingAllowedFiles=sum(len(list(path.iterdir())) for path in allowed))
write('cleanup-r1.json', (json.dumps(cleanup_result, indent=2) + '\n').encode())
print(json.dumps(dict(roundtrip=roundtrip, cleanup=cleanup_result)))
