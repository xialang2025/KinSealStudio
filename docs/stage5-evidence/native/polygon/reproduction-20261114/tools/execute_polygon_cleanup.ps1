param([switch]$Execute)
$ErrorActionPreference = 'Stop'
$taskRoot = (Resolve-Path -LiteralPath $PSScriptRoot).Path
$taskExpectedRoot = 'Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage5-20261114'
if ($taskRoot -ne $taskExpectedRoot) { throw 'Cleanup script is outside the fixed new-date task root.' }
$taskEvidence = Join-Path $taskRoot '..\kinseal-stage4-20261112\worktree\docs\stage5-evidence\native\polygon'
$taskEvidence = (Resolve-Path -LiteralPath $taskEvidence).Path
$taskPlanPath = Join-Path $taskEvidence 'cleanup-plan-polygon-20261114.json'
$taskPlan = Get-Content -LiteralPath $taskPlanPath -Raw -Encoding UTF8 | ConvertFrom-Json
if ($taskPlan.schema -ne 1 -or $taskPlan.family -ne 'polygon' -or $taskPlan.taskRoot -ne $taskRoot) { throw 'Unexpected cleanup manifest identity.' }
if ($taskPlan.cDriveCleanupPlanned -or $taskPlan.decodedFramesDeletionPlanned) { throw 'Unapproved cleanup category.' }
$taskPublicManifestPath = Join-Path $taskEvidence 'reproduction-20261114\public-qt-sources-manifest.json'
if ($taskPlan.publicSourcesManifest.path -ne $taskPublicManifestPath -or (Get-FileHash -LiteralPath $taskPublicManifestPath -Algorithm SHA256).Hash -ne $taskPlan.publicSourcesManifest.sha256) { throw 'Public-source/license manifest changed or is absent.' }
$taskPublicManifest = Get-Content -LiteralPath $taskPublicManifestPath -Raw -Encoding UTF8 | ConvertFrom-Json
$taskReproductionRoot = Join-Path $taskEvidence 'reproduction-20261114'
$taskLicenseNames = @()
foreach ($taskSourceEntry in $taskPublicManifest.sourcesAndLicenses) {
    $taskSourcePath = [IO.Path]::GetFullPath((Join-Path $taskReproductionRoot $taskSourceEntry.path))
    if (-not $taskSourcePath.StartsWith($taskReproductionRoot + '\', [StringComparison]::OrdinalIgnoreCase)) { throw 'Public-source manifest path escaped reproduction evidence.' }
    if ((Get-FileHash -LiteralPath $taskSourcePath -Algorithm SHA256).Hash -ne $taskSourceEntry.sha256) { throw 'Retained public source/license changed.' }
    if ($taskSourceEntry.kind -eq 'upstream_license') { $taskLicenseNames += [IO.Path]::GetFileName($taskSourcePath) }
}
foreach ($taskLicenseName in @('LICENSE.LGPL', 'LICENSE.GPL', 'LGPL_EXCEPTION.txt', 'FTL.TXT')) {
    if ($taskLicenseNames -notcontains $taskLicenseName) { throw 'Required Qt or FreeType license is absent.' }
}

function Assert-OwnedPath([string]$Path) {
    # Every removable file must resolve inside this exact task, without junctions or symlinks.
    $absolute = [IO.Path]::GetFullPath($Path)
    if (-not $absolute.StartsWith($taskRoot + '\', [StringComparison]::OrdinalIgnoreCase)) { throw "Path outside new task root: $Path" }
    $cursor = $absolute
    while ($cursor -ne $taskRoot) {
        if (Test-Path -LiteralPath $cursor) {
            $item = Get-Item -LiteralPath $cursor -Force
            if (($item.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) { throw "Redirected cleanup path: $cursor" }
        }
        $cursor = [IO.Path]::GetDirectoryName($cursor)
        if (-not $cursor) { throw 'Path ancestry escaped the task root.' }
    }
    if ((Get-Item -LiteralPath $taskRoot -Force).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Redirected task root.' }
    return $absolute
}

function Read-GzipText([string]$Path) {
    $file = [IO.File]::OpenRead($Path)
    try {
        $gzip = New-Object IO.Compression.GZipStream($file, [IO.Compression.CompressionMode]::Decompress)
        try {
            $reader = New-Object IO.StreamReader($gzip, [Text.Encoding]::UTF8)
            try { return $reader.ReadToEnd() } finally { $reader.Dispose() }
        } finally { $gzip.Dispose() }
    } finally { $file.Dispose() }
}

function Get-GzipIdentity([string]$Path) {
    $file = [IO.File]::OpenRead($Path)
    try {
        $gzip = New-Object IO.Compression.GZipStream($file, [IO.Compression.CompressionMode]::Decompress)
        try {
            $hasher = [Security.Cryptography.SHA256]::Create()
            try { return ([BitConverter]::ToString($hasher.ComputeHash($gzip))).Replace('-', '').ToLowerInvariant() }
            finally { $hasher.Dispose() }
        } finally { $gzip.Dispose() }
    } finally { $file.Dispose() }
}

$taskProofs = @{}
foreach ($taskProof in $taskPlan.proofs) {
    if ($taskProof.run -notmatch '^polygon-[A-Za-z0-9][A-Za-z0-9-]*$') { throw 'Invalid proof run name.' }
    $taskRuntime = Assert-OwnedPath $taskProof.runtime
    $taskRequiredRuntime = Join-Path $taskRoot ('native\runtime-' + $taskProof.run)
    if ($taskRuntime -ne $taskRequiredRuntime) { throw 'Runtime does not match the exact proof run.' }
    if ($taskProof.ownedProcessTerminated -ne $true -or $taskProof.usbOrPlc -ne $false) { throw 'Incomplete proof flags.' }
    $taskArchive = [IO.Path]::GetFullPath($taskProof.rawArchive)
    $taskRequiredArchive = Join-Path $taskEvidence ($taskProof.run + '\native-probe.json.gz')
    if ($taskArchive -ne $taskRequiredArchive) { throw 'Raw archive outside matching evidence run.' }
    if ((Get-FileHash -LiteralPath $taskArchive -Algorithm SHA256).Hash -ne $taskProof.rawEvidence.archiveSha256) { throw 'Raw probe archive changed.' }
    if ((Get-GzipIdentity $taskArchive) -ne $taskProof.rawEvidence.uncompressedSha256) { throw 'Raw probe gzip round-trip identity changed.' }
    $taskRawProof = Read-GzipText $taskArchive | ConvertFrom-Json
    if ($taskRawProof.run -ne $taskProof.run -or $taskRawProof.ownedProcessTerminated -ne $true -or $taskRawProof.usbOrPlc -ne $false) { throw 'Raw probe does not prove completed owned USB-free process.' }
    $taskProofs[$taskProof.run] = $taskProof
}

foreach ($taskDirectory in $taskPlan.roots) {
    $taskAbsolute = Assert-OwnedPath $taskDirectory
    $taskRun = [IO.Path]::GetFileName($taskAbsolute).Substring('runtime-'.Length)
    if (-not $taskProofs.ContainsKey($taskRun) -or $taskAbsolute -ne $taskProofs[$taskRun].runtime) { throw 'Unregistered runtime cleanup root.' }
}
$taskLive = Get-CimInstance Win32_Process | Where-Object {
    $taskExe = $_.ExecutablePath
    $taskExe -and @($taskPlan.roots | Where-Object { $taskExe.StartsWith($_ + '\', [StringComparison]::OrdinalIgnoreCase) }).Count
}
if (@($taskLive).Count -ne 0) { throw 'A proved polygon runtime still has a live executable.' }

# Validate every identity and retained peer before the first deletion.
$taskPrepared = @()
$taskDeletionPaths = @{}
foreach ($taskSourceEntry in $taskPlan.files) { $taskDeletionPaths[[IO.Path]::GetFullPath($taskSourceEntry.path)] = $true }
foreach ($taskEntry in $taskPlan.files) {
    $taskPath = Assert-OwnedPath $taskEntry.path
    if (-not $taskProofs.ContainsKey([string]$taskEntry.proofRun)) { throw 'Deletion entry has no matching process proof.' }
    $taskProof = $taskProofs[[string]$taskEntry.proofRun]
    if ($taskEntry.category -eq 'owned_polygon_runtime') {
        if (-not $taskPath.StartsWith($taskProof.runtime + '\', [StringComparison]::OrdinalIgnoreCase)) { throw 'Runtime file is outside its proof root.' }
    } elseif ($taskEntry.category -eq 'owned_polygon_log') {
        if ($taskPath -ne (Join-Path $taskRoot ($taskEntry.proofRun + '.log'))) { throw 'Log is not the exact proved run log.' }
    } else { throw 'Unregistered cleanup file category.' }
    if (-not (Test-Path -LiteralPath $taskPath -PathType Leaf)) {
        # A second run is idempotent; an already removed path never broadens the target set.
        continue
    }
    $taskInfo = Get-Item -LiteralPath $taskPath -Force
    if ($taskInfo.Length -ne $taskEntry.bytes -or (Get-FileHash -LiteralPath $taskPath -Algorithm SHA256).Hash -ne $taskEntry.sha256) { throw 'Cleanup source identity changed.' }
    if (-not $taskEntry.retained) { throw 'No retained identity for deletion entry.' }
    if ((Get-FileHash -LiteralPath $taskEntry.retained -Algorithm SHA256).Hash -ne $taskEntry.retainedArchiveSha256) { throw 'Retained file/archive changed.' }
    if ($taskEntry.retainedEncoding -eq 'gzip') {
        if ((Get-GzipIdentity $taskEntry.retained) -ne $taskEntry.sha256) { throw 'Retained gzip contents differ.' }
    } elseif ($taskEntry.retainedEncoding -eq 'identity') {
        if ((Get-FileHash -LiteralPath $taskEntry.retained -Algorithm SHA256).Hash -ne $taskEntry.sha256) { throw 'Retained peer differs.' }
    } else { throw 'Unknown retained encoding.' }
    if ($taskDeletionPaths.ContainsKey([IO.Path]::GetFullPath($taskEntry.retained))) { throw 'Retained peer is itself scheduled for deletion.' }
    $taskPrepared += $taskEntry
}
if (-not $Execute) {
    [ordered]@{ mode='validated_preview_only'; files=$taskPrepared.Count; logicalBytes=($taskPrepared | Measure-Object -Property bytes -Sum).Sum; skippedRuns=$taskPlan.skippedRuns; deletionPerformed=$false } | ConvertTo-Json -Depth 12
    return
}
$taskBeforeC = (Get-PSDrive C).Free
$taskBeforeZ = (Get-PSDrive Z).Free
$taskCount = 0
$taskBytes = 0L
foreach ($taskEntry in $taskPrepared) {
    $taskPath = Assert-OwnedPath $taskEntry.path
    $taskInfo = Get-Item -LiteralPath $taskPath -Force
    if ($taskInfo.Length -ne $taskEntry.bytes -or (Get-FileHash -LiteralPath $taskPath -Algorithm SHA256).Hash -ne $taskEntry.sha256) { throw 'Cleanup source changed after preflight.' }
    Remove-Item -LiteralPath $taskPath -Force
    $taskCount++
    $taskBytes += $taskInfo.Length
}
$taskReceiptPath = Join-Path $taskEvidence 'cleanup-polygon-20261114.json'
$taskPreviousReceipt = $null
if (Test-Path -LiteralPath $taskReceiptPath) { $taskPreviousReceipt = Get-Content -LiteralPath $taskReceiptPath -Raw -Encoding UTF8 | ConvertFrom-Json }
[ordered]@{family='polygon';taskRoot=$taskRoot;deletedFiles=$taskCount;deletedLogicalBytes=$taskBytes;logicalBytesAreNotPhysicalSpace=$true;cFreeBefore=$taskBeforeC;cFreeAfter=(Get-PSDrive C).Free;zFreeBefore=$taskBeforeZ;zFreeAfter=(Get-PSDrive Z).Free;rawArchivesAndUniqueEvidencePreserved=$true;publicSourcesAndLicensesPreserved=$true;oldSharedAndInstallationPreserved=$true;incrementalBuildPreserved=$true;userMainBuildPreserved=$true;cDriveFilesDeleted=0;skippedRuns=$taskPlan.skippedRuns;previousReceipt=$taskPreviousReceipt} | ConvertTo-Json -Depth 20 | Set-Content -LiteralPath $taskReceiptPath -Encoding UTF8
Get-Content -LiteralPath $taskReceiptPath -Raw -Encoding UTF8
