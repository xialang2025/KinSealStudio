$ErrorActionPreference='Stop'
$taskRoot=(Resolve-Path -LiteralPath $PSScriptRoot).Path
$taskWork=(Resolve-Path -LiteralPath (Join-Path $taskRoot '..\kinseal-stage4-20261112\worktree')).Path
$taskOut=Join-Path $taskWork 'docs\stage5-evidence\delivery-slice2-r2'
$taskPlan=Get-Content -LiteralPath (Join-Path $taskOut 'cleanup-plan.json') -Raw | ConvertFrom-Json
$taskTarget=(Resolve-Path -LiteralPath $taskPlan.root).Path
$taskExpected=[IO.Path]::GetFullPath((Join-Path $taskRoot '..\..\KinSealStudio\dist\KinSealStudioLite-Stage5-compile-slice2-r1'))
if($taskTarget -ne $taskExpected) { throw 'Unexpected failed package target.' }
$taskLive=Get-CimInstance Win32_Process | Where-Object { $_.ExecutablePath -and $_.ExecutablePath.StartsWith($taskTarget+'\',[StringComparison]::OrdinalIgnoreCase) }
if(@($taskLive).Count -ne 0) { throw 'Failed package still has a live process.' }
$taskCount=0
$taskBytes=0L
foreach($taskEntry in $taskPlan.files) {
    $taskPath=(Resolve-Path -LiteralPath $taskEntry.path).Path
    if(-not $taskPath.StartsWith($taskTarget+'\',[StringComparison]::OrdinalIgnoreCase)) { throw 'File outside exact failed package.' }
    $taskInfo=Get-Item -LiteralPath $taskPath
    if($taskInfo.LinkType -and $taskInfo.LinkType -ne 'HardLink') { throw 'Refuse redirect.' }
    if($taskInfo.Length -ne $taskEntry.bytes -or (Get-FileHash -LiteralPath $taskPath).Hash -ne $taskEntry.sha256 -or (Get-FileHash -LiteralPath $taskEntry.retained).Hash -ne $taskEntry.sha256) { throw 'Identity or retained peer changed.' }
    Remove-Item -LiteralPath $taskPath
    $taskCount++
    $taskBytes+=$taskInfo.Length
}
[ordered]@{deletedFiles=$taskCount;deletedLogicalBytes=$taskBytes;retainedPeersVerified=$true;failedDiagnosticsPreserved=$true;successfulPackagePreserved=$true} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $taskOut 'cleanup.json') -Encoding utf8
Get-Content -LiteralPath (Join-Path $taskOut 'cleanup.json')
