$ErrorActionPreference = 'Stop'
$taskRoot = [IO.Path]::GetFullPath('Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage5-20261114')
$taskEvidence = [IO.Path]::GetFullPath('Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage4-20261112\worktree\docs\stage5-evidence\native\sector')
$taskPlan = Get-Content -LiteralPath (Join-Path $taskEvidence 'cleanup-plan-sector-20261114.json') -Raw -Encoding UTF8 | ConvertFrom-Json
if ($taskPlan.family -ne 'sector' -or $taskPlan.taskRoot -ne $taskRoot) { throw 'Unexpected cleanup identity.' }
$taskRemoved = 0
$taskRootsRemoved = 0
foreach ($taskPath in $taskPlan.roots) {
    $taskAbsolute = [IO.Path]::GetFullPath($taskPath)
    $taskName = [IO.Path]::GetFileName($taskAbsolute)
    if ([IO.Path]::GetDirectoryName($taskAbsolute) -ne (Join-Path $taskRoot 'native') -or $taskName -notmatch '^runtime-sector-[a-zA-Z0-9-]+$') { throw 'Runtime root escaped the exact task.' }
    if (-not (Test-Path -LiteralPath $taskAbsolute)) { continue }
    $taskDirectories = @(Get-ChildItem -LiteralPath $taskAbsolute -Directory -Recurse -Force) + @(Get-Item -LiteralPath $taskAbsolute -Force)
    foreach ($taskDirectory in ($taskDirectories | Sort-Object { $_.FullName.Length } -Descending)) {
        $taskResolved = [IO.Path]::GetFullPath($taskDirectory.FullName)
        if ($taskResolved -ne $taskAbsolute -and -not $taskResolved.StartsWith($taskAbsolute + '\', [StringComparison]::OrdinalIgnoreCase)) { throw 'Directory escaped runtime root.' }
        if (($taskDirectory.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) { throw 'Redirected runtime directory.' }
        if (@(Get-ChildItem -LiteralPath $taskResolved -Force).Count -ne 0) { throw 'Expected a completely empty runtime.' }
        Remove-Item -LiteralPath $taskResolved -Force
        $taskRemoved++
        if ($taskResolved -eq $taskAbsolute) { $taskRootsRemoved++ }
    }
}
[ordered]@{family='sector'; emptyDirectoriesRemoved=$taskRemoved; registeredRuntimeRootsRemoved=$taskRootsRemoved; onlyRegisteredEmptyDirectories=$true} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $taskEvidence 'cleanup-empty-directories-r1.json') -Encoding UTF8
Get-Content -LiteralPath (Join-Path $taskEvidence 'cleanup-empty-directories-r1.json') -Raw -Encoding UTF8
