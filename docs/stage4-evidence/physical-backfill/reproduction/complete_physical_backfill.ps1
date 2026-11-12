param(
    [Parameter(Mandatory=$true)][string]$Control,
    [Parameter(Mandatory=$true)][string]$Family,
    [Parameter(Mandatory=$true)][string]$Run
)
$ErrorActionPreference='Stop'
$taskPython='C:\Users\lenovo\AppData\Local\Programs\Python\Python311\python.exe'
$env:TEMP=$env:TMP=$env:TMPDIR=Join-Path $PSScriptRoot 'temp'
$env:PYTHONDONTWRITEBYTECODE='1'
$env:PYTHONIOENCODING='utf-8'
Push-Location -LiteralPath $PSScriptRoot
try {
    & $taskPython archive_physical_run.py $Family $Run
    if ($LASTEXITCODE -ne 0) { throw 'Physical archive failed' }
    $taskPlan=Join-Path $PSScriptRoot ('native-physical\physical-cleanup-'+$Run+'.json')
    & (Join-Path $PSScriptRoot 'execute_native_cleanup.ps1') -Plan $taskPlan
    $taskEvidence=Join-Path $PSScriptRoot ('worktree\docs\stage4-evidence\'+$Family+'\physical\'+$Run+'\cleanup.json')
    Copy-Item -LiteralPath ($taskPlan+'.result.json') -Destination $taskEvidence
    & $taskPython finalize_physical_backfill.py $Control $Run
    if ($LASTEXITCODE -ne 0) { throw 'Physical finalization failed' }
    & $taskPython commit_physical_backfill.py $Control
    if ($LASTEXITCODE -ne 0) { throw 'Physical commit audit failed' }
} finally {
    Pop-Location
}
