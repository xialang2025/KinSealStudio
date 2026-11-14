param(
    [ValidatePattern('^[A-Za-z0-9_-]+$')]
    [string]$DeployDirectoryName = 'KinSealStudioLite-Stage11-release-r1',
    [string]$RuntimeDirectory,
    [string]$EvidenceDirectory,
    [switch]$BuildOnly,
    [string]$Python = 'C:\Users\lenovo\AppData\Local\Programs\Python\Python311\python.exe'
)

#20261114-10 add: Keep the source build reproducible and replace broad development-document deployment with an explicit user whitelist.
$ErrorActionPreference = 'Stop'
$Stage11Project = Split-Path -Parent $PSScriptRoot
$Stage11Build = Join-Path $Stage11Project 'build'
$Stage11Temp = Join-Path $Stage11Build 'temp'
$Stage11Qt = 'G:\01_Professional\QT6\Installed\6.12.0\mingw_64'
$Stage11Tools = 'G:\01_Professional\QT6\Installed\Tools'
$Stage11CMake = Join-Path $Stage11Tools 'CMake_64\bin\cmake.exe'
$Stage11Compiler = Join-Path $Stage11Tools 'mingw1310_64\bin'
$Stage11Ninja = Join-Path $Stage11Tools 'Ninja\ninja.exe'
$Stage11Records = [System.Collections.Generic.List[object]]::new()
if (-not $EvidenceDirectory) {
    $EvidenceDirectory = Join-Path $Stage11Project ('docs\stage11-evidence\build-' + [guid]::NewGuid().ToString('N'))
}
if (Test-Path -LiteralPath $EvidenceDirectory) {
    throw 'Build evidence directory must be new; previous diagnostics are preserved.'
}
New-Item -ItemType Directory -Path $EvidenceDirectory | Out-Null
New-Item -ItemType Directory -Force -Path $Stage11Temp | Out-Null
$env:TEMP = [System.IO.Path]::GetFullPath($Stage11Temp)
$env:TMP = $env:TEMP
$env:TMPDIR = $env:TEMP
$env:KINSEAL_MACRO_TEMP_ROOT = $env:TEMP
$env:KINSEAL_HELP_SOURCE_ROOT = Join-Path $Stage11Project 'help'
$env:PYTHONIOENCODING = 'utf-8'
$env:QT_QPA_PLATFORM = 'offscreen'
$env:PATH = "$Stage11Compiler;$Stage11Qt\bin;$($Stage11Tools)\Ninja;$env:PATH"

<#
Function Name   : ConvertTo-Stage11Argument
Description     : Quote one Windows argument without invoking a command shell.
Input Parameters: Value
Return Value    : One correctly escaped command-line token.
Date            : 2026.11.14
#>
function ConvertTo-Stage11Argument([string]$Value) { #20261114-10 add
    $Stage11Builder = [System.Text.StringBuilder]::new()
    [void]$Stage11Builder.Append('"')
    $Stage11Slashes = 0
    foreach ($Stage11Character in $Value.ToCharArray()) {
        if ($Stage11Character -eq '\') {
            $Stage11Slashes++
            continue
        }
        if ($Stage11Character -eq '"') {
            [void]$Stage11Builder.Append(('\' * (2 * $Stage11Slashes + 1)))
        } else {
            [void]$Stage11Builder.Append(('\' * $Stage11Slashes))
        }
        [void]$Stage11Builder.Append($Stage11Character)
        $Stage11Slashes = 0
    }
    [void]$Stage11Builder.Append(('\' * (2 * $Stage11Slashes)))
    [void]$Stage11Builder.Append('"')
    return $Stage11Builder.ToString()
}

<#
Function Name   : Invoke-Stage11Step
Description     : Execute a hidden owned process and preserve both streams independently of PowerShell native-error parsing.
Input Parameters: Name, File, Arguments
Return Value    : None; throws on failure after preserving the step record.
Date            : 2026.11.14
#>
function Invoke-Stage11Step([string]$Name, [string]$File, [string[]]$Arguments) { #20261114-10 add
    $Stage11Started = [DateTimeOffset]::UtcNow.ToString('o')
    $Stage11Info = [System.Diagnostics.ProcessStartInfo]::new()
    $Stage11Info.FileName = $File
    $Stage11Info.Arguments = (($Arguments | ForEach-Object { ConvertTo-Stage11Argument $_ }) -join ' ')
    $Stage11Info.WorkingDirectory = $Stage11Project
    $Stage11Info.UseShellExecute = $false
    $Stage11Info.CreateNoWindow = $true
    $Stage11Info.WindowStyle = [System.Diagnostics.ProcessWindowStyle]::Hidden
    $Stage11Info.RedirectStandardOutput = $true
    $Stage11Info.RedirectStandardError = $true
    $Stage11Info.StandardOutputEncoding = [System.Text.Encoding]::UTF8
    $Stage11Info.StandardErrorEncoding = [System.Text.Encoding]::UTF8
    $Stage11Process = [System.Diagnostics.Process]::new()
    $Stage11Process.StartInfo = $Stage11Info
    try {
        if (-not $Stage11Process.Start()) { throw "Unable to start $Name" }
        $Stage11Stdout = $Stage11Process.StandardOutput.ReadToEndAsync()
        $Stage11Stderr = $Stage11Process.StandardError.ReadToEndAsync()
        $Stage11Process.WaitForExit()
        $Stage11Exit = $Stage11Process.ExitCode
        [System.IO.File]::WriteAllText((Join-Path $EvidenceDirectory ($Name + '-stdout.txt')), $Stage11Stdout.Result, [System.Text.UTF8Encoding]::new($false))
        [System.IO.File]::WriteAllText((Join-Path $EvidenceDirectory ($Name + '-stderr.txt')), $Stage11Stderr.Result, [System.Text.UTF8Encoding]::new($false))
        $Stage11Records.Add([ordered]@{ name=$Name; executable=$File; arguments=$Arguments; startedAtUtc=$Stage11Started; exitCode=$Stage11Exit })
        $Stage11Records | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $EvidenceDirectory 'steps.json') -Encoding UTF8
        if ($Stage11Exit -ne 0) { throw "Stage11 step $Name failed with exit $Stage11Exit; see $EvidenceDirectory" }
    } finally {
        $Stage11Process.Dispose()
    }
}

Invoke-Stage11Step 'configure' $Stage11CMake @('-S',$Stage11Project,'-B',$Stage11Build,'-G','Ninja',
    "-DCMAKE_MAKE_PROGRAM=$Stage11Ninja", "-DCMAKE_CXX_COMPILER=$Stage11Compiler\g++.exe",
    "-DCMAKE_RC_COMPILER=$Stage11Compiler\windres.exe", "-DCMAKE_PREFIX_PATH=$Stage11Qt", '-DCMAKE_BUILD_TYPE=Release')
Invoke-Stage11Step 'build' $Stage11CMake @('--build',$Stage11Build,'--target','KinSealStudioLite','KinSealLocalHelpTests','--parallel','4')
Invoke-Stage11Step 'local-help-tests' (Join-Path $Stage11Build 'KinSealLocalHelpTests.exe') @('-o',((Join-Path $EvidenceDirectory 'local-help-qt.txt') + ',txt'))
Invoke-Stage11Step 'self-test' (Join-Path $Stage11Build 'KinSealStudioLite.exe') @('--self-test')
Invoke-Stage11Step 'embedded-payload' (Join-Path $Stage11Build 'KinSealStudioLite.exe') @('--verify-independent-payload')
if ($BuildOnly) {
    Write-Output "Stage11 build and required host checks completed: $Stage11Build"
    exit 0
}
if (-not $RuntimeDirectory) {
    $RuntimeDirectory = Join-Path $Stage11Project 'dist\KinSealStudioLite-Stage9-plc-r2'
    if (-not (Test-Path -LiteralPath $RuntimeDirectory)) {
        $RuntimeDirectory = Join-Path $Stage11Project '..\..\KinSealStudio\dist\KinSealStudioLite-Stage9-plc-r2'
    }
}
$Stage11Exe = Join-Path $Stage11Build 'KinSealStudioLite.exe'
$Stage11Sha = (Get-FileHash -LiteralPath $Stage11Exe -Algorithm SHA256).Hash.ToLowerInvariant()
$Stage11Package = Join-Path $Stage11Project ('dist\' + $DeployDirectoryName)
Invoke-Stage11Step 'package' $Python @('-B',(Join-Path $PSScriptRoot 'package_stage11.py'),'create',
    '--worktree',$Stage11Project,'--executable',$Stage11Exe,'--expected-exe-sha',$Stage11Sha,
    '--runtime',$RuntimeDirectory,'--runtime-manifest-sha','2b6e2a668fa84915378bb082f4fbc75e11a70f0d508fbd9d2fb92a202e2f10da',
    '--help-dir',(Join-Path $Stage11Project 'help'),'--licenses',(Join-Path $Stage11Project 'docs\stage11-evidence\dependencies-owned\licenses'),
    '--dependencies',(Join-Path $Stage11Project 'release\stage11-dependencies.json'),
    '--guide',(Join-Path $Stage11Project 'release\使用说明.md'),'--package',$Stage11Package,
    '--evidence',(Join-Path $EvidenceDirectory 'package'))
Invoke-Stage11Step 'package-verify' $Python @('-B',(Join-Path $PSScriptRoot 'package_stage11.py'),'verify','--package',$Stage11Package)
Write-Output "Stage11 portable package created: $Stage11Package"
exit 0
