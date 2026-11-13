param([Parameter(Mandatory=$true)][ValidateSet('table','rect','audit','line')][string]$Family)
$ErrorActionPreference='Stop'
$taskRoot=(Resolve-Path -LiteralPath $PSScriptRoot).Path
$taskWork=(Resolve-Path -LiteralPath (Join-Path $taskRoot '..\kinseal-stage4-20261112\worktree')).Path
$taskEvidence=Join-Path $taskWork ('docs\stage5-evidence\native\'+$Family)
$taskPlan=Get-Content -LiteralPath (Join-Path $taskEvidence 'cleanup-plan.json') -Raw | ConvertFrom-Json
foreach($taskDirectory in $taskPlan.roots) {
    $taskAbsolute=(Resolve-Path -LiteralPath $taskDirectory).Path
    if (-not ($taskAbsolute.StartsWith($taskRoot+'\native\runtime-'+$Family+'-',[StringComparison]::OrdinalIgnoreCase) -or $taskAbsolute -eq (Join-Path $taskRoot 'temp'))) { throw 'Unregistered cleanup root.' }
}
$taskLive=Get-CimInstance Win32_Process | Where-Object { $_.ExecutablePath -and $_.ExecutablePath.StartsWith($taskRoot+'\native\runtime-'+$Family+'-',[StringComparison]::OrdinalIgnoreCase) }
if (@($taskLive).Count -ne 0) { throw 'This family still has a live owned native process.' }
$taskBeforeC=(Get-PSDrive C).Free
$taskBeforeZ=(Get-PSDrive Z).Free
$taskCount=0
$taskBytes=0L
foreach($taskEntry in $taskPlan.files) {
    $taskPath=(Resolve-Path -LiteralPath $taskEntry.path).Path
    if (-not @($taskPlan.roots | Where-Object { $taskPath.StartsWith($_+'\',[StringComparison]::OrdinalIgnoreCase) }).Count) { throw 'Resolved file outside registered roots.' }
    $taskInfo=Get-Item -LiteralPath $taskPath
    if(($taskInfo.LinkType -and $taskInfo.LinkType -ne 'HardLink') -or ($taskInfo.LinkType -eq 'HardLink' -and -not $taskEntry.retained)) { throw 'Refuse redirected or unretained linked cleanup file.' }
    if($taskInfo.Length -ne $taskEntry.bytes -or (Get-FileHash -LiteralPath $taskPath -Algorithm SHA256).Hash -ne $taskEntry.sha256) { throw 'Cleanup identity changed.' }
    if($taskEntry.retained -and (Get-FileHash -LiteralPath $taskEntry.retained -Algorithm SHA256).Hash -ne $taskEntry.sha256) { throw 'Retained peer differs.' }
    Remove-Item -LiteralPath $taskPath
    $taskCount++
    $taskBytes += $taskInfo.Length
}
[ordered]@{family=$Family;deletedFiles=$taskCount;deletedLogicalBytes=$taskBytes;logicalBytesAreNotPhysicalSpace=$true;cFreeBefore=$taskBeforeC;cFreeAfter=(Get-PSDrive C).Free;zFreeBefore=$taskBeforeZ;zFreeAfter=(Get-PSDrive Z).Free;rawArchivesAndUniqueEvidencePreserved=$true;activeBuildPreserved=$true;userMainBuildPreserved=$true} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $taskEvidence 'cleanup.json') -Encoding utf8
Get-Content -LiteralPath (Join-Path $taskEvidence 'cleanup.json')
