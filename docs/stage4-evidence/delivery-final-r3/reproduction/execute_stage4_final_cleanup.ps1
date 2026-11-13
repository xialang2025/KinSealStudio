$ErrorActionPreference = 'Stop'
function Assert-OwnedFileSystemEntry([string]$Path) {
    $taskEntryInfo = Get-Item -LiteralPath $Path
    if (($taskEntryInfo.Attributes -band [IO.FileAttributes]::ReparsePoint) -eq 0) { return }
    # Z uses non-redirecting cloud placeholders (observed 0x9000601a), not junctions.
    # Accept only this cloud tag family; name-surrogate links remain rejected.
    if ($taskEntryInfo.LinkType -or $taskEntryInfo.Target) { throw 'Refuse redirected cleanup entry.' }
    $taskQuery = (& fsutil reparsepoint query $Path 2>&1) -join "`n"
    if ($LASTEXITCODE -ne 0 -or $taskQuery -notmatch '0x([0-9a-fA-F]{8})') { throw 'Unidentified reparse entry.' }
    $taskTag = [Convert]::ToUInt32($matches[1],16)
    if (($taskTag -band 0xffff0fffL) -ne 0x9000001aL) { throw 'Reparse entry is outside the observed cloud-placeholder family.' }
}
$taskEvidence = Join-Path $PSScriptRoot 'worktree\docs\stage4-evidence\delivery-final-r3'
$taskPlan = Get-Content -LiteralPath (Join-Path $taskEvidence 'cleanup-plan.json') -Raw | ConvertFrom-Json
$taskWorkspace = (Resolve-Path -LiteralPath $PSScriptRoot).Path
$taskMainDist = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..\KinSealStudio\dist')).Path
$taskExpected = @((Join-Path $taskWorkspace 'worktree\build'), (Join-Path $taskMainDist 'KinSealStudioLite-Stage4-41items-0.10.1-final-r1'), (Join-Path $taskMainDist 'KinSealStudioLite-Stage4-41items-0.10.1-final-r2'))
if (@(Get-Process KinSealStudio*,cmake*,ninja*,cc1plus* -ErrorAction SilentlyContinue).Count -ne 0) { throw 'Wait until owned GUI and compiler processes exit.' }
foreach ($taskRoot in $taskPlan.allowedRoots) {
    $taskResolved = (Resolve-Path -LiteralPath $taskRoot).Path
    if ($taskResolved -notin $taskExpected) { throw 'Unexpected resolved cleanup root.' }
    Assert-OwnedFileSystemEntry $taskResolved
}
$taskFreeBeforeC = (Get-PSDrive C).Free
$taskFreeBeforeZ = (Get-PSDrive Z).Free
$taskCount = 0
$taskBytes = 0L
foreach ($taskEntry in $taskPlan.files) {
    $taskResolved = (Resolve-Path -LiteralPath $taskEntry.path).Path
    if (-not @($taskExpected | Where-Object { $taskResolved.StartsWith($_+'\',[StringComparison]::OrdinalIgnoreCase) }).Count) { throw 'File outside explicit cleanup roots.' }
    $taskInfo = Get-Item -LiteralPath $taskResolved
    Assert-OwnedFileSystemEntry $taskResolved
    if ($taskInfo.Length -ne $taskEntry.bytes -or (Get-FileHash -LiteralPath $taskResolved -Algorithm SHA256).Hash -ne $taskEntry.sha256) { throw 'Cleanup input identity changed.' }
    if ($taskEntry.retained) {
        $taskPeer = (Resolve-Path -LiteralPath $taskEntry.retained).Path
        if (-not ($taskPeer.StartsWith($taskWorkspace+'\',[StringComparison]::OrdinalIgnoreCase) -or $taskPeer.StartsWith($taskMainDist+'\KinSealStudioLite-Stage4-41items-0.10.1-final-r3\',[StringComparison]::OrdinalIgnoreCase))) { throw 'Retained peer outside owned evidence or final package.' }
        if ((Get-FileHash -LiteralPath $taskPeer -Algorithm SHA256).Hash -ne $taskEntry.sha256) { throw 'Retained peer identity changed.' }
    }
    Remove-Item -LiteralPath $taskResolved
    $taskCount++
    $taskBytes += $taskInfo.Length
}
$taskResult = [ordered]@{ deletedFiles=$taskCount; deletedLogicalBytes=$taskBytes; logicalBytesAreNotPhysicalSpace=$true; cDriveFreeBefore=$taskFreeBeforeC; cDriveFreeAfter=(Get-PSDrive C).Free; zDriveFreeBefore=$taskFreeBeforeZ; zDriveFreeAfter=(Get-PSDrive Z).Free; allRetainedPeersVerified=$true; mainUserBuildPreserved=$true; sourcesAndEvidencePreserved=$true }
$taskResult | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $taskEvidence 'cleanup.json') -Encoding utf8
$taskResult | ConvertTo-Json
