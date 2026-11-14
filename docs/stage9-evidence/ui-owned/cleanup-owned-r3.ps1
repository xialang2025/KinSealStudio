#20261114-8 add: Remove only this subtask's newly generated reproducible compile intermediates.
$ErrorActionPreference = 'Stop'
$taskTemp = [IO.Path]::GetFullPath('Z:\Project\vscode_prj_temp\REA\.scratch\kinseal-stage9-20261114\temp')
$outputRoots = @('online-ui-owned-r1', 'online-ui-owned-r2-red', 'online-ui-owned-r3-green')
$ownedNames = @('plc_online_dialog.o', 'plc_serial_session.o', 'plc_protocol_capabilities.o',
    'plc_address.o', 'plc_native_catalog.o', 'simulation_modbus_transport.o',
    'stage9_online_ui_tests.o', 'moc_dialog.o', 'catalog.o',
    'stage9_online_ui_tests.moc', 'moc_plc_online_dialog.cpp', 'qrc_plc_catalog.cpp')
$manifest = @()
$freedBytes = [long]0

foreach ($rootName in $outputRoots) {
    $resolvedRoot = [IO.Path]::GetFullPath((Join-Path $taskTemp $rootName))
    if ([IO.Path]::GetDirectoryName($resolvedRoot) -ne $taskTemp) { throw 'Owned output escaped task temp.' }
    foreach ($name in $ownedNames) {
        $target = [IO.Path]::GetFullPath((Join-Path $resolvedRoot $name))
        if ([IO.Path]::GetDirectoryName($target) -ne $resolvedRoot) { throw 'Owned intermediate escaped output.' }
        if (!(Test-Path -LiteralPath $target -PathType Leaf)) { throw "Expected new intermediate missing: $target" }
        $item = Get-Item -LiteralPath $target
        $manifest += [ordered]@{ path = $target; bytes = $item.Length; sha256 = (Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash.ToLowerInvariant() }
    }
}

#20261114-8 add: Complete the full exact-file inventory before the first removal; no recursive deletion is used.
foreach ($item in $manifest) {
    Remove-Item -LiteralPath $item.path
    if (Test-Path -LiteralPath $item.path) { throw "Intermediate still exists: $($item.path)" }
    $freedBytes += $item.bytes
}
$record = [ordered]@{
    scope = 'Only newly generated online-ui-owned r1/r2-red/r3-green compile intermediates'
    recursiveDeletion = $false
    historicalArtifactsTouched = $false
    sharedBuildTouched = $false
    scriptsEvidenceScreenshotsAndTestExecutablesPreserved = $true
    removedCount = $manifest.Count
    freedBytes = $freedBytes
    files = $manifest
}
$record | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'cleanup-r3.json') -Encoding utf8
[ordered]@{ removedCount = $manifest.Count; freedBytes = $freedBytes } | ConvertTo-Json -Compress
