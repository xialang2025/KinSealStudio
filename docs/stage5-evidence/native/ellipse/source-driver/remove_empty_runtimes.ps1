$ErrorActionPreference = 'Stop'
$taskRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if ($taskRoot -ne 'Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage5-20261114') { throw 'Wrong task root.' }
$taskEvidence = [IO.Path]::GetFullPath((Join-Path $taskRoot '..\kinseal-stage4-20261112\worktree\docs\stage5-evidence\native\ellipse'))
$taskPlan = Get-Content -LiteralPath (Join-Path $taskEvidence 'cleanup-plan-ellipse-20261114.json') -Raw | ConvertFrom-Json
$taskReceipt = Get-Content -LiteralPath (Join-Path $taskEvidence 'cleanup-ellipse-20261114.json') -Raw | ConvertFrom-Json
if ($taskPlan.family -ne 'ellipse' -or $taskReceipt.deletedFiles -ne $taskPlan.files.Count) { throw 'File cleanup was not completed.' }
$taskPrepared = @()
foreach ($taskDirectory in $taskPlan.roots) {
    $taskAbsolute = [IO.Path]::GetFullPath($taskDirectory)
    $taskParent = Join-Path $taskRoot 'native'
    if ([IO.Path]::GetDirectoryName($taskAbsolute) -ne $taskParent -or [IO.Path]::GetFileName($taskAbsolute) -notmatch '^runtime-ellipse-[A-Za-z0-9][A-Za-z0-9-]*$') { throw 'Unexpected runtime root.' }
    if (-not (Test-Path -LiteralPath $taskAbsolute)) { continue }
    $taskRootInfo = Get-Item -LiteralPath $taskAbsolute -Force
    if (($taskRootInfo.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) { throw 'Redirected runtime root.' }
    $taskItems = @(Get-ChildItem -LiteralPath $taskAbsolute -Recurse -Force)
    if (@($taskItems | Where-Object { -not $_.PSIsContainer }).Count -ne 0) { throw 'An unexpected file remains; no directory deletion started.' }
    if (@($taskItems | Where-Object { ($_.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0 }).Count -ne 0) { throw 'Redirected child directory.' }
    $taskPrepared += @($taskItems | Sort-Object @{Expression={$_.FullName.Length};Descending=$true} | Select-Object -ExpandProperty FullName)
    $taskPrepared += $taskAbsolute
}
$taskRemoved = @()
foreach ($taskDirectory in $taskPrepared) {
    if (@(Get-ChildItem -LiteralPath $taskDirectory -Force).Count -ne 0) { throw 'Directory gained content after preflight.' }
    Remove-Item -LiteralPath $taskDirectory -Force
    $taskRemoved += $taskDirectory
}
[ordered]@{ family='ellipse'; scope='only empty exact-whitelist runtime directories after file cleanup'; deletedDirectories=$taskRemoved.Count; paths=$taskRemoved; recursiveDeletionUsed=$false; deletedFiles=0; allRootsAbsent=(@($taskPlan.roots | Where-Object { Test-Path -LiteralPath $_ }).Count -eq 0) } | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $taskEvidence 'cleanup-empty-directories-ellipse-20261114.json') -Encoding UTF8
[ordered]@{ deletedDirectories=$taskRemoved.Count; deletedFiles=0; allRootsAbsent=(@($taskPlan.roots | Where-Object { Test-Path -LiteralPath $_ }).Count -eq 0) } | ConvertTo-Json
