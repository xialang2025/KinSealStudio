"""Derive exact-date ellipse-only archive/cleanup recipes from the reviewed polygon recipes."""
from pathlib import Path

ROOT=Path(__file__).resolve().parent.parent
source=(ROOT/'archive_polygon.py').read_text(encoding='utf-8')
source=source.replace('polygon','ellipse')
start=source.index('def archive_public_sources():')
end=source.index('\ndef main():',start)
replacement='''def archive_public_sources():
    """Verify and reference the already retained polygon diagnostic sources and licenses."""
    original_root=WORK/'docs/stage5-evidence/native/polygon/reproduction-20261114'
    original_manifest=original_root/'public-qt-sources-manifest.json'
    data=read_json(original_manifest)
    entries=[]
    licenses=set()
    for entry in data['sourcesAndLicenses']:
        retained=original_root/entry['path']
        if not retained.resolve().is_relative_to(original_root.resolve()) or sha(retained)!=entry['sha256']:
            raise ValueError('Previously retained diagnostic source/license changed: '+str(retained))
        if entry['kind']=='upstream_license':
            licenses.add(retained.name)
        entries.append(dict(**entry,retainedPath=str(retained.resolve())))
    if not {'LICENSE.LGPL','LICENSE.GPL','LGPL_EXCEPTION.txt','FTL.TXT'}.issubset(licenses):
        raise ValueError('Previously retained Qt/FreeType license set is incomplete')
    write_json(REPRO/'public-qt-sources-manifest.json',dict(
        scope='Reference-only reuse of retained polygon Qt4 diagnostic sources/licenses; no production dependency',
        reusedManifest=dict(path=str(original_manifest.resolve()),sha256=sha(original_manifest),sourceRoot=str(original_root.resolve())),
        sourcesAndLicenses=entries,originalLicenseHeadersPreserved=True,productionDependency=False))

'''
source=source[:start]+replacement+source[end:]
start=source.index('    script_names = ')
end=source.index('    copied = []',start)
source=source[:start]+'''    script_names = ('native_probe.py', 'native_paint_probe.py', 'legacy_coverage_probe.cpp',
                    'archive_ellipse.py', 'execute_ellipse_cleanup.ps1')
'''+source[end:]
marker="    write_json(BASE / 'raw-archives-20261114.json'"
start=source.index(marker)
source=source[:start]+'''    for source in sorted((ROOT/'ellipse_corpus').iterdir()):
        if not source.is_file() or source.suffix not in ('.py','.json','.log'):
            continue
        target=REPRO/'ellipse-corpus'/source.name
        target.parent.mkdir(parents=True,exist_ok=True)
        if target.exists() and target.read_bytes()!=source.read_bytes():
            target=target.with_name(target.stem+'-'+sha(source)[:12]+target.suffix)
        if not target.exists():
            shutil.copy2(source,target)
        copied.append(dict(path=str(target.relative_to(BASE)),sha256=sha(target),bytes=target.stat().st_size))
'''+source[start:]
(ROOT/'archive_ellipse.py').write_text(source,encoding='utf-8')

script=(ROOT/'execute_polygon_cleanup.ps1').read_text(encoding='utf-8-sig').replace('polygon','ellipse')
start=script.index("$taskReproductionRoot = ")
end=script.index('function Assert-OwnedPath',start)
replacement='''$taskOriginalReproductionRoot = [IO.Path]::GetFullPath((Join-Path $taskEvidence '..\\polygon\\reproduction-20261114'))
$taskOriginalManifest = Join-Path $taskOriginalReproductionRoot 'public-qt-sources-manifest.json'
if ($taskPublicManifest.reusedManifest.path -ne $taskOriginalManifest -or $taskPublicManifest.reusedManifest.sourceRoot -ne $taskOriginalReproductionRoot -or (Get-FileHash -LiteralPath $taskOriginalManifest -Algorithm SHA256).Hash -ne $taskPublicManifest.reusedManifest.sha256) { throw 'Original polygon public-source manifest changed.' }
$taskLicenseNames = @()
foreach ($taskSourceEntry in $taskPublicManifest.sourcesAndLicenses) {
    $taskSourcePath = [IO.Path]::GetFullPath($taskSourceEntry.retainedPath)
    if (-not $taskSourcePath.StartsWith($taskOriginalReproductionRoot + '\\', [StringComparison]::OrdinalIgnoreCase)) { throw 'Retained diagnostic source/license escaped original polygon evidence.' }
    if ((Get-FileHash -LiteralPath $taskSourcePath -Algorithm SHA256).Hash -ne $taskSourceEntry.sha256) { throw 'Retained public source/license changed.' }
    if ($taskSourceEntry.kind -eq 'upstream_license') { $taskLicenseNames += [IO.Path]::GetFileName($taskSourcePath) }
}
foreach ($taskLicenseName in @('LICENSE.LGPL', 'LICENSE.GPL', 'LGPL_EXCEPTION.txt', 'FTL.TXT')) {
    if ($taskLicenseNames -notcontains $taskLicenseName) { throw 'Required Qt or FreeType license is absent.' }
}

'''
script=script[:start]+replacement+script[end:]
(ROOT/'execute_ellipse_cleanup.ps1').write_text(script,encoding='utf-8')
print('Prepared exact-date ellipse-only recipes; no archive or deletion executed.')
