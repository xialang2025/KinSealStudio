param([switch]$Execute)
$ErrorActionPreference='Stop'
$taskTemp=(Resolve-Path -LiteralPath 'C:\Users\lenovo\AppData\Local\Temp').Path
$taskPlanPath=Join-Path $PSScriptRoot 'c-temp-older-frida-plan.json'
$taskPrior=Get-Content -LiteralPath (Join-Path $PSScriptRoot 'frida-temp-cleanup-plan.json') -Raw | ConvertFrom-Json
$taskReference=@{}
foreach ($taskEntry in $taskPrior.files) {
    $taskRelative=$taskEntry.path -replace '^.*\\frida-[0-9a-f]{32}\\',''
    $taskReference[$taskRelative]=$taskEntry
}
$taskLive=@(Get-CimInstance Win32_Process | Where-Object { $_.Name -match '^(python.*|frida.*|KinSeal.*)\.exe$' })
if ($taskLive.Count -ne 0) { throw 'Native/instrumentation process still live; defer cleanup' }
if (-not $Execute) {
    $taskFiles=@()
    foreach ($taskFolder in Get-ChildItem -LiteralPath $taskTemp -Directory -Filter 'frida-*') {
        if ($taskFolder.Name -notmatch '^frida-[0-9a-f]{32}$' -or ($taskFolder.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) { continue }
        $taskCandidates=@(Get-ChildItem -LiteralPath $taskFolder.FullName -File -Recurse)
        foreach ($taskFile in $taskCandidates) {
            $taskRelative=$taskFile.FullName.Substring($taskFolder.FullName.Length+1)
            if (-not $taskReference.ContainsKey($taskRelative)) { throw 'Unknown file in temporary Frida package' }
            $taskKnown=$taskReference[$taskRelative]
            $taskHash=(Get-FileHash -LiteralPath $taskFile.FullName -Algorithm SHA256).Hash
            if ($taskFile.Length -ne $taskKnown.bytes -or $taskHash -ne $taskKnown.sha256) { throw 'Frida package differs from known regenerable components' }
            $taskFiles += [ordered]@{path=$taskFile.FullName;bytes=$taskFile.Length;sha256=$taskHash}
        }
    }
    [ordered]@{scope=$taskTemp;files=$taskFiles;authorization='User requested automatic cleanup including C drive temporary files';excluded='No empty directories, user documents, or non-Frida temporary files';freeBefore=(Get-PSDrive C).Free} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $taskPlanPath -Encoding utf8
    [pscustomobject]@{files=$taskFiles.Count;bytes=($taskFiles|Measure-Object -Property bytes -Sum).Sum} | ConvertTo-Json
    exit
}
$taskPlan=Get-Content -LiteralPath $taskPlanPath -Raw | ConvertFrom-Json
if ($taskPlan.scope -ne $taskTemp) { throw 'Unexpected cleanup scope' }
$taskRemoved=0
foreach ($taskEntry in $taskPlan.files) {
    $taskPath=(Resolve-Path -LiteralPath $taskEntry.path).Path
    if (-not $taskPath.StartsWith($taskTemp+'\',[StringComparison]::OrdinalIgnoreCase) -or $taskPath.Substring($taskTemp.Length+1) -notmatch '^frida-[0-9a-f]{32}\\') { throw 'Resolved target outside explicit temporary-package scope' }
    $taskInfo=Get-Item -LiteralPath $taskPath
    if (($taskInfo.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0 -or $taskInfo.Length -ne $taskEntry.bytes -or (Get-FileHash -LiteralPath $taskPath -Algorithm SHA256).Hash -ne $taskEntry.sha256) { throw 'Cleanup identity changed' }
    Remove-Item -LiteralPath $taskPath
    $taskRemoved++
}
$taskResult=[ordered]@{deletedFiles=$taskRemoved;logicalBytes=($taskPlan.files|Measure-Object -Property bytes -Sum).Sum;freeBefore=$taskPlan.freeBefore;freeAfter=(Get-PSDrive C).Free;emptyDirectoriesRetained=$true;matchedKnownFridaSha=$true}
$taskResult | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'c-temp-older-frida-result.json') -Encoding utf8
[pscustomobject]$taskResult | ConvertTo-Json
