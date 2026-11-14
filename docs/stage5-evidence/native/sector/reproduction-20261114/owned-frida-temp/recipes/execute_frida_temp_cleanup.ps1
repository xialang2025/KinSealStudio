param([switch]$Execute)
$ErrorActionPreference = 'Stop'
$taskRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if ($taskRoot -ne 'Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage5-20261114') { throw 'Wrong task root.' }
$taskTemp = Join-Path $taskRoot 'temp'
$taskEvidence = [IO.Path]::GetFullPath((Join-Path $taskRoot '..\kinseal-stage4-20261112\worktree\docs\stage5-evidence\native\sector\reproduction-20261114\owned-frida-temp'))
$taskPlan = Get-Content -LiteralPath (Join-Path $taskEvidence 'cleanup-plan.json') -Raw | ConvertFrom-Json
if ($taskPlan.schema -ne 1 -or $taskPlan.taskRoot -ne $taskRoot -or $taskPlan.tempRoot -ne $taskTemp -or $taskPlan.cDriveFilesDeletionPlanned) { throw 'Unexpected plan identity or scope.' }
$taskLive = @(Get-CimInstance Win32_Process | Where-Object { $_.Name -like 'KinSealStudio*' -or ($_.ExecutablePath -and $_.ExecutablePath.StartsWith($taskTemp + '\', [StringComparison]::OrdinalIgnoreCase)) })
if ($taskLive.Count) { throw 'A native application or task-temp executable is still active.' }

function Assert-TaskPath([string]$Path) {
    $absolute = [IO.Path]::GetFullPath($Path)
    if (-not $absolute.StartsWith($taskTemp + '\', [StringComparison]::OrdinalIgnoreCase)) { throw 'Path outside exact task temp.' }
    $cursor = $absolute
    while ($cursor -ne $taskRoot) {
        if (Test-Path -LiteralPath $cursor) {
            if ((Get-Item -LiteralPath $cursor -Force).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Redirected cleanup path.' }
        }
        $cursor = [IO.Path]::GetDirectoryName($cursor)
        if (-not $cursor) { throw 'Path ancestry escaped task root.' }
    }
    if ((Get-Item -LiteralPath $taskRoot -Force).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Redirected task root.' }
    return $absolute
}

function Get-GzipIdentity([string]$Path) {
    $file = [IO.File]::OpenRead($Path)
    try {
        $gzip = New-Object IO.Compression.GZipStream($file, [IO.Compression.CompressionMode]::Decompress)
        try {
            $digest = [Security.Cryptography.SHA256]::Create()
            try { return ([BitConverter]::ToString($digest.ComputeHash($gzip))).Replace('-', '').ToLowerInvariant() }
            finally { $digest.Dispose() }
        } finally { $gzip.Dispose() }
    } finally { $file.Dispose() }
}

$taskRoots = @{}
foreach ($taskDirectory in $taskPlan.roots) {
    $taskAbsolute = Assert-TaskPath $taskDirectory
    if ([IO.Path]::GetDirectoryName($taskAbsolute) -ne $taskTemp -or [IO.Path]::GetFileName($taskAbsolute) -cnotmatch '^frida-[0-9a-fA-F]{32}$') { throw 'Not an exact Frida extraction root.' }
    $taskRoots[$taskAbsolute] = $true
}
$taskArchives = @{}
foreach ($taskArchive in $taskPlan.uniqueBytes) {
    $taskArchivePath = [IO.Path]::GetFullPath($taskArchive.path)
    if ([IO.Path]::GetDirectoryName($taskArchivePath) -ne (Join-Path $taskEvidence 'unique-bytes') -or [IO.Path]::GetFileName($taskArchivePath) -ne ($taskArchive.uncompressedSha256 + '.gz')) { throw 'Archive escaped exact retention root.' }
    if ((Get-FileHash -LiteralPath $taskArchivePath -Algorithm SHA256).Hash -ne $taskArchive.archiveSha256 -or (Get-GzipIdentity $taskArchivePath) -ne $taskArchive.uncompressedSha256) { throw 'Retained archive does not round-trip.' }
    $taskArchives[$taskArchivePath] = $taskArchive
}
$taskPrepared = @()
foreach ($taskEntry in $taskPlan.files) {
    $taskPath = Assert-TaskPath $taskEntry.path
    $taskEntryRoot = [IO.Path]::GetFullPath($taskEntry.root)
    if (-not $taskRoots.ContainsKey($taskEntryRoot) -or -not $taskPath.StartsWith($taskEntryRoot + '\', [StringComparison]::OrdinalIgnoreCase) -or [IO.Path]::GetExtension($taskPath).ToLowerInvariant() -notin @('.dll','.exe')) { throw 'File is outside its exact whitelist root or type.' }
    if (-not $taskArchives.ContainsKey([IO.Path]::GetFullPath($taskEntry.archive))) { throw 'No retained byte identity for file.' }
    $taskArchive = $taskArchives[[IO.Path]::GetFullPath($taskEntry.archive)]
    if ($taskArchive.uncompressedSha256 -ne $taskEntry.sha256 -or $taskArchive.uncompressedBytes -ne $taskEntry.bytes) { throw 'Archive identity differs from removal file.' }
    $taskInfo = Get-Item -LiteralPath $taskPath -Force
    if ($taskInfo.Length -ne $taskEntry.bytes -or (Get-FileHash -LiteralPath $taskPath -Algorithm SHA256).Hash -ne $taskEntry.sha256) { throw 'Source changed before full preflight completed.' }
    $taskPrepared += $taskEntry
}
if (-not $Execute) {
    [ordered]@{mode='full_preflight_no_deletion'; directories=$taskRoots.Count; files=$taskPrepared.Count; logicalBytes=($taskPrepared | Measure-Object bytes -Sum).Sum; verifiedUniqueArchives=$taskArchives.Count; deletionPerformed=$false} | ConvertTo-Json
    return
}
$taskLive = @(Get-CimInstance Win32_Process | Where-Object { $_.Name -like 'KinSealStudio*' -or ($_.ExecutablePath -and $_.ExecutablePath.StartsWith($taskTemp + '\', [StringComparison]::OrdinalIgnoreCase)) })
if ($taskLive.Count) { throw 'A native application or temp executable started after preflight.' }
$taskBeforeC = (Get-PSDrive C).Free
$taskBeforeZ = (Get-PSDrive Z).Free
$taskDeleted = 0
$taskBytes = 0L
$taskSkipped = @()
foreach ($taskEntry in $taskPrepared) {
    $taskPath = Assert-TaskPath $taskEntry.path
    try {
        $taskInfo = Get-Item -LiteralPath $taskPath -Force
        if ($taskInfo.Length -ne $taskEntry.bytes -or (Get-FileHash -LiteralPath $taskPath -Algorithm SHA256).Hash -ne $taskEntry.sha256) { throw 'Source identity changed after preflight.' }
        Remove-Item -LiteralPath $taskPath -Force
        $taskDeleted++
        $taskBytes += [long]$taskEntry.bytes
    } catch {
        $taskSkipped += [ordered]@{path=$taskPath;reason=$_.Exception.Message}
    }
}
$taskDeletedDirectories = 0
foreach ($taskDirectory in $taskPlan.roots) {
    if (-not (Test-Path -LiteralPath $taskDirectory)) { continue }
    $taskChildren = @(Get-ChildItem -LiteralPath $taskDirectory -Recurse -Directory -Force | Sort-Object @{Expression={$_.FullName.Length};Descending=$true} | Select-Object -ExpandProperty FullName)
    foreach ($taskChild in @($taskChildren) + @($taskDirectory)) {
        $taskChild = Assert-TaskPath $taskChild
        if (@(Get-ChildItem -LiteralPath $taskChild -Force).Count -eq 0) {
            try { Remove-Item -LiteralPath $taskChild -Force; $taskDeletedDirectories++ }
            catch { $taskSkipped += [ordered]@{path=$taskChild;reason=$_.Exception.Message} }
        }
    }
}
$taskReceipt = [ordered]@{scope='shared_owned_frida_extractions_only_not_sector_native_runtimes';separateFromSectorRuntime856Files=$true;deletedFiles=$taskDeleted;deletedLogicalBytes=$taskBytes;deletedEmptyDirectories=$taskDeletedDirectories;logicalBytesAreNotPhysicalRelease=$true;cFreeBefore=$taskBeforeC;cFreeAfter=(Get-PSDrive C).Free;zFreeBefore=$taskBeforeZ;zFreeAfter=(Get-PSDrive Z).Free;retainedUniqueArchives=$taskArchives.Count;retainedCompressedBytes=($taskPlan.uniqueBytes | Measure-Object archiveBytes -Sum).Sum;fridaVersion=$taskPlan.fridaVersion;installationPydIdentity=$taskPlan.installationPydIdentity;cDriveFilesDeleted=0;otherTempPreserved=$taskPlan.preservedOtherTempEntries;remainingWhitelistedRoots=@($taskPlan.roots | Where-Object {Test-Path -LiteralPath $_});skipped=$taskSkipped;deletionPerformed=$true}
$taskReceipt | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $taskEvidence 'cleanup-receipt.json') -Encoding UTF8
$taskReceipt | ConvertTo-Json -Depth 8
