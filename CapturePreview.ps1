param(
    [string]$OutputPath = 'artifacts\KinSealStudioLite-preview.png'
)

# 20261107-6 add: Render only this project's window on Qt's offscreen platform.
$ErrorActionPreference = 'Stop'
$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$Application = Join-Path $ProjectRoot 'build\KinSealStudioLite.exe'
$QtBin = 'G:\01_Professional\QT6\Installed\6.12.0\mingw_64\bin'
$CompilerBin = 'G:\01_Professional\QT6\Installed\Tools\mingw1310_64\bin'
$PreviousPlatform = $env:QT_QPA_PLATFORM
$PreviousFontDirectory = $env:QT_QPA_FONTDIR
$PreviousPath = $env:Path
$ResolvedOutput = if ([IO.Path]::IsPathRooted($OutputPath)) {
    $OutputPath
} else {
    Join-Path $ProjectRoot $OutputPath
}

if (!(Test-Path -LiteralPath $Application -PathType Leaf)) {
    throw "Build the application first: $Application"
}
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $ResolvedOutput) | Out-Null

try {
    $env:QT_QPA_PLATFORM = 'offscreen'
    if (Test-Path -LiteralPath 'C:\Windows\Fonts' -PathType Container) {
        $env:QT_QPA_FONTDIR = 'C:\Windows\Fonts'
    }
    $env:Path = "$QtBin;$CompilerBin;$PreviousPath"
    $Capture = Start-Process -FilePath $Application `
        -ArgumentList @('--capture-preview', $ResolvedOutput) `
        -Wait -PassThru -WindowStyle Hidden # 20261108-7 change: Wait for offscreen render completion.
    if ($Capture.ExitCode -ne 0) {
        throw "The offscreen Qt capture failed with code $($Capture.ExitCode)."
    }
    if (!(Test-Path -LiteralPath $ResolvedOutput -PathType Leaf)) {
        throw 'The offscreen Qt capture did not create an image.'
    }
    Get-Item -LiteralPath $ResolvedOutput | Select-Object FullName,Length
}
finally {
    $env:QT_QPA_PLATFORM = $PreviousPlatform
    $env:QT_QPA_FONTDIR = $PreviousFontDirectory
    $env:Path = $PreviousPath
}
