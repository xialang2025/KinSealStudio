param([Parameter(Mandatory=$true)][string]$Plan)
$ErrorActionPreference='Stop'
$taskRoot=(Resolve-Path -LiteralPath $PSScriptRoot).Path
$taskPlanPath=(Resolve-Path -LiteralPath $Plan).Path
if (-not $taskPlanPath.StartsWith($taskRoot+'\',[StringComparison]::OrdinalIgnoreCase)) { throw 'Plan outside task' }
$taskPlan=Get-Content -LiteralPath $taskPlanPath -Raw | ConvertFrom-Json
$taskOwner=(Resolve-Path -LiteralPath $taskPlan.ownerScope).Path
$taskEvidence=(Resolve-Path -LiteralPath $taskPlan.evidenceScope).Path
foreach ($taskScope in @($taskOwner,$taskEvidence)) {
    if (-not $taskScope.StartsWith($taskRoot+'\',[StringComparison]::OrdinalIgnoreCase)) { throw 'Scope outside task' }
}
$taskLive=@(Get-CimInstance Win32_Process | Where-Object { $_.ExecutablePath -and $_.ExecutablePath.StartsWith($taskOwner+'\',[StringComparison]::OrdinalIgnoreCase) })
if ($taskLive.Count -ne 0) { throw 'Owned native process still running' }
$taskRemoved=@()
foreach ($taskEntry in $taskPlan.files) {
    $taskPath=(Resolve-Path -LiteralPath $taskEntry.path).Path
    if (-not ($taskPath.StartsWith($taskOwner+'\',[StringComparison]::OrdinalIgnoreCase) -or $taskPath.StartsWith($taskEvidence+'\',[StringComparison]::OrdinalIgnoreCase))) { throw 'File outside declared scopes' }
    $taskInfo=Get-Item -LiteralPath $taskPath
    if (($taskInfo.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0 -or $taskInfo.Length -ne $taskEntry.bytes -or (Get-FileHash -LiteralPath $taskPath -Algorithm SHA256).Hash.ToLowerInvariant() -ne $taskEntry.sha256) { throw 'Identity mismatch' }
    if ($taskEntry.retained -and (Get-FileHash -LiteralPath $taskEntry.retained -Algorithm SHA256).Hash.ToLowerInvariant() -ne $taskEntry.sha256) { throw 'Retained peer mismatch' }
    Remove-Item -LiteralPath $taskPath
    $taskRemoved += $taskEntry
}
$taskResult=[ordered]@{deletedFiles=$taskRemoved.Count;logicalBytes=($taskRemoved | Measure-Object -Property bytes -Sum).Sum;files=$taskRemoved;cFree=(Get-PSDrive C).Free;zFree=(Get-PSDrive Z).Free;physicalSpaceNote='Logical sizes can include hard links; not claimed as physical bytes released.'}
$taskResult | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath ($taskPlanPath+'.result.json') -Encoding utf8
[pscustomobject]$taskResult | Select-Object deletedFiles,logicalBytes,cFree,zFree | ConvertTo-Json
