$ErrorActionPreference='Stop'
$taskFailed = 'Z:\Project\vscode_prj_temp\REA\KinSealStudio\dist\KinSealStudioLite-Stage5-compile-slice7-r1'
$taskExpectedDist = 'Z:\Project\vscode_prj_temp\REA\KinSealStudio\dist'
$taskEvidence = 'Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage4-20261112\worktree\docs\stage5-evidence\delivery-slice7-r1'
$taskGoodEvidence = 'Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage4-20261112\worktree\docs\stage5-evidence\delivery-slice7-r2'
$taskGood = Get-Content -LiteralPath (Join-Path $taskGoodEvidence 'run.json') -Raw -Encoding UTF8 | ConvertFrom-Json
$taskManifest = Get-Content -LiteralPath (Join-Path $taskGoodEvidence 'package-manifest.json') -Raw -Encoding UTF8 | ConvertFrom-Json
if (-not $taskGood.passed -or $taskManifest.checksPassed -ne 102) { throw 'No verified replacement portable package.' }
$taskResolved = (Resolve-Path -LiteralPath $taskFailed).Path
if ($taskResolved -ne $taskFailed -or [IO.Path]::GetDirectoryName($taskResolved) -ne $taskExpectedDist) { throw 'Unexpected failed package target.' }
if ((Get-Item -LiteralPath $taskResolved).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Redirected package root.' }
$taskProcesses = @(Get-CimInstance Win32_Process | Where-Object { $_.ExecutablePath -and $_.ExecutablePath.StartsWith($taskResolved + '\', [StringComparison]::OrdinalIgnoreCase) })
if ($taskProcesses.Count -ne 0) { throw 'Failed package is still running.' }
if ((Get-FileHash -LiteralPath (Join-Path $taskResolved 'KinSealStudioLite.exe') -Algorithm SHA256).Hash -ne $taskManifest.executableSha256) { throw 'Failed package executable differs from retained verified replacement.' }
Copy-Item -LiteralPath (Join-Path $taskResolved 'samples\polygon-default.ksp') -Destination (Join-Path $taskEvidence 'failed-polygon-default-input.ksp')
$taskFiles = @(Get-ChildItem -LiteralPath $taskResolved -File -Recurse | ForEach-Object { [pscustomobject]@{path=$_.FullName.Substring($taskResolved.Length+1);bytes=$_.Length;sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash} })
$taskBytes = ($taskFiles | Measure-Object -Property bytes -Sum).Sum
[ordered]@{failedPackage=$taskResolved;verifiedReplacement=$taskManifest.package;replacementSourceCommit=$taskManifest.sourceCommit;reason='Polygon JSON sample wrongly contained dormant endArrow/endCap keys; strict parser rejected them. Production unchanged; r2 has102 actual passed checks.';files=$taskFiles;deletedFiles=$taskFiles.Count;deletedLogicalBytes=$taskBytes;failedInputPreserved=$true;userPackageProcesses=0} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $taskEvidence 'failed-package-cleanup.json') -Encoding UTF8
# The resolved target is exactly the failed self-created r1 directory in the fixed dist root.
Remove-Item -LiteralPath $taskResolved -Recurse -Force
if (Test-Path -LiteralPath $taskResolved) { throw 'Failed package directory remains.' }
Write-Output ('Deleted failed r1 package files: ' + $taskFiles.Count + ', bytes: ' + $taskBytes)
