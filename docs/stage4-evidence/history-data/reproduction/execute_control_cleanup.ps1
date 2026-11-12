param(
    [Parameter(Mandatory=$true)][ValidatePattern('^[a-z0-9-]+$')][string]$Slug,
    [Parameter(Mandatory=$true)][ValidatePattern('^windows-[a-z0-9-]+$')][string]$Final
)
$ErrorActionPreference='Stop'
$taskWorkspace=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot 'worktree'))
$taskBase=(Resolve-Path -LiteralPath (Join-Path $taskWorkspace ('docs\stage4-evidence\'+$Slug))).Path
if (-not $taskBase.StartsWith($taskWorkspace+'\',[StringComparison]::OrdinalIgnoreCase)) { throw 'Cleanup scope outside task worktree' }
$taskPlan=Get-Content -LiteralPath (Join-Path $taskBase 'cleanup-plan.json') -Raw | ConvertFrom-Json
if ([IO.Path]::GetFullPath($taskPlan.scope).TrimEnd('\') -ne $taskBase.TrimEnd('\')) { throw 'Cleanup scope mismatch' }
$taskRemoved=@()
foreach ($taskEntry in $taskPlan.files) {
    $taskFile=(Resolve-Path -LiteralPath $taskEntry.path).Path
    $taskPeer=(Resolve-Path -LiteralPath $taskEntry.retained).Path
    if (-not $taskFile.StartsWith($taskBase+'\',[StringComparison]::OrdinalIgnoreCase) -or $taskFile -eq $taskPeer -or [IO.Path]::GetExtension($taskFile) -notin @('.png','.ksp')) { throw 'Cleanup target outside declared scope/type' }
    $taskInfo=Get-Item -LiteralPath $taskFile
    if (($taskInfo.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0 -or $taskInfo.Length -ne $taskEntry.bytes) { throw 'Cleanup identity mismatch' }
    if ((Get-FileHash -LiteralPath $taskFile -Algorithm SHA256).Hash.ToLowerInvariant() -ne $taskEntry.sha256 -or (Get-FileHash -LiteralPath $taskPeer -Algorithm SHA256).Hash.ToLowerInvariant() -ne $taskEntry.sha256) { throw 'Cleanup SHA mismatch' }
    Remove-Item -LiteralPath $taskFile
    $taskRemoved += $taskEntry
}
$taskRun=Get-Content -LiteralPath (Join-Path $taskBase ($Final+'\run.json')) -Raw | ConvertFrom-Json
$taskResult=[ordered]@{
    scope=$taskBase
    deletedFiles=$taskRemoved.Count
    deletedBytes=($taskRemoved | Measure-Object -Property bytes -Sum).Sum
    retainedPeerShaVerified=$true
    files=$taskRemoved
    buildTemporaryFiles=@(Get-ChildItem -LiteralPath (Join-Path $taskWorkspace 'build\temp') -File -Recurse).Count
    testTemporaryFiles=$taskRun.temporaryFilesRemaining
    cDriveFreeBytes=(Get-PSDrive C).Free
    zDriveFreeBytes=(Get-PSDrive Z).Free
    cTemporaryPolicy='No Frida host started; Python, Qt and GCC temporary directories redirected to owned Z task directories'
}
$taskResult | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $taskBase 'cleanup.json') -Encoding utf8
[pscustomobject]$taskResult | Select-Object deletedFiles,deletedBytes,buildTemporaryFiles,cDriveFreeBytes,zDriveFreeBytes | ConvertTo-Json
