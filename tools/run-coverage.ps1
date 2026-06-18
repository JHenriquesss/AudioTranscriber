param(
    [string]$BuildDir = "build/windows-msvc-release"
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
$openCppCoverage = Get-Command OpenCppCoverage -ErrorAction SilentlyContinue

if ($null -eq $openCppCoverage) {
    Write-Error "OpenCppCoverage is not installed. Install it to run the coverage gate."
    exit 1
}

$testExe = Join-Path $repoRoot "$BuildDir/tests/unit/phase07_unit_tests.exe"
if (-not (Test-Path $testExe)) {
    Write-Error "Expected test executable at $testExe"
    exit 1
}

$coverageDir = Join-Path $repoRoot "build/coverage"
New-Item -ItemType Directory -Force -Path $coverageDir | Out-Null

& $openCppCoverage.Source `
    --sources "$repoRoot\src" `
    --export_type cobertura:"$coverageDir\cobertura.xml" `
    -- $testExe

if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Write-Host "Coverage report written to $coverageDir\cobertura.xml"
