$ErrorActionPreference = 'Stop'
$taskFixedTemp = 'Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage5-20261114\temp'
$taskEvidence = 'Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage4-20261112\worktree\docs\stage5-evidence\native\polygon'
$taskFixedFiles = @(
    'Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage5-20261114\temp\area-polygon-fill.mask',
    'Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage5-20261114\temp\area-polygon-fill.mask.pathstream',
    'Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage5-20261114\temp\area-polygon-native.rgb565',
    'Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage5-20261114\temp\area-polygon-stroke.mask',
    'Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage5-20261114\temp\area-polygon-stroke.mask.pathstream'
)
$taskArchive = Join-Path $taskEvidence 'reproduction-20261114\decoded-default'
New-Item -ItemType Directory -Path $taskArchive -Force | Out-Null
$taskEntries = @()
foreach ($taskFile in $taskFixedFiles) {
    $taskResolved = (Resolve-Path -LiteralPath $taskFile).Path
    if ([IO.Path]::GetDirectoryName($taskResolved) -ne $taskFixedTemp) { throw 'Unexpected decoded file parent.' }
    $taskInfo = Get-Item -LiteralPath $taskResolved
    if ($taskInfo.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Redirected decoded file.' }
    $taskDigest = (Get-FileHash -LiteralPath $taskResolved -Algorithm SHA256).Hash
    $taskTarget = Join-Path $taskArchive ($taskInfo.Name + '.gz')
    $taskData = [IO.File]::ReadAllBytes($taskResolved)
    $taskStream = [IO.File]::Create($taskTarget)
    try {
        $taskGzip = New-Object IO.Compression.GZipStream($taskStream, [IO.Compression.CompressionLevel]::Optimal)
        try { $taskGzip.Write($taskData, 0, $taskData.Length) } finally { $taskGzip.Dispose() }
    } finally { $taskStream.Dispose() }
    $taskStream = [IO.File]::OpenRead($taskTarget)
    try {
        $taskGzip = New-Object IO.Compression.GZipStream($taskStream, [IO.Compression.CompressionMode]::Decompress)
        try {
            $taskHash = [Security.Cryptography.SHA256]::Create()
            try { $taskVerified = ([BitConverter]::ToString($taskHash.ComputeHash($taskGzip))).Replace('-', '') }
            finally { $taskHash.Dispose() }
        } finally { $taskGzip.Dispose() }
    } finally { $taskStream.Dispose() }
    if ($taskVerified -ne $taskDigest) { throw 'Decoded archive identity mismatch.' }
    $taskEntries += [pscustomobject][ordered]@{ path=$taskResolved; bytes=$taskInfo.Length; sha256=$taskDigest; archive=$taskTarget }
}
# Delete only five exact files after all five retained gzip copies have passed verification.
foreach ($taskEntry in $taskEntries) {
    if ((Get-FileHash -LiteralPath $taskEntry.path -Algorithm SHA256).Hash -ne $taskEntry.sha256) { throw 'Decoded source changed.' }
    Remove-Item -LiteralPath $taskEntry.path -Force
}
[ordered]@{ deletedFiles=$taskEntries.Count; deletedLogicalBytes=($taskEntries | Measure-Object -Property bytes -Sum).Sum; entries=$taskEntries; cDriveFilesDeleted=0; buildObjectsPreserved=$true; otherTaskTemporaryFilesPreserved=$true } | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $taskEvidence 'cleanup-decoded-default-20261114.json') -Encoding utf8
$taskEntries | Select-Object path,bytes
