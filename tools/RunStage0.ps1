param(
    [string]$OutputDirectory,
    [switch]$Download
)

#20261110-4 add: Reproduce one stage-0 run without opening the user's editor or changing the input desktop.
$ErrorActionPreference = 'Stop'
$StageProjectRoot = Split-Path -Parent $PSScriptRoot
$StagePython = Join-Path $env:USERPROFILE '.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
if (-not (Test-Path -LiteralPath $StagePython -PathType Leaf)) {
    throw 'Bundled Python was not found. Run stage0_baseline.py with an existing Python 3 interpreter.'
}
if ([string]::IsNullOrWhiteSpace($OutputDirectory)) {
    $StageRunName = 'run-' + [DateTimeOffset]::UtcNow.ToString('yyyyMMdd-HHmmss')
    $OutputDirectory = Join-Path $StageProjectRoot ('docs\stage0-evidence\' + $StageRunName)
}
$StageArguments = @('-X', 'utf8', (Join-Path $PSScriptRoot 'stage0_baseline.py'), '--output', $OutputDirectory)
if ($Download) {
    $StageArguments += '--download'
}
& $StagePython @StageArguments
if ($LASTEXITCODE -ne 0) {
    throw "Stage-0 baseline failed with exit code $LASTEXITCODE; keep the new run directory for diagnosis."
}
