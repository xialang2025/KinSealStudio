param(
    [Parameter(Mandatory = $true)]
    [string]$OutputDirectory,
    [switch]$Download
)

#20261110-5 add: Keep hardware transfer opt-in while sharing the frozen portable model/source checks.
$ErrorActionPreference = 'Stop'
$stage1Python = 'C:/Users/lenovo/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe'
$stage1Recipe = Join-Path $PSScriptRoot 'stage1_model_verification.py'
$stage1Arguments = @('-B', '-X', 'utf8', $stage1Recipe, '--output', $OutputDirectory)
if ($Download) {
    $stage1Arguments += '--download'
}
& $stage1Python @stage1Arguments
exit $LASTEXITCODE
