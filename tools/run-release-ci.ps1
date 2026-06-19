param(
    [string]$Preset = "windows-msvc-release",
    [string]$BuildPreset = "release"
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot

Write-Host "Setting up runtime artifacts..."
powershell -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot "setup-runtime-artifacts.ps1") -RepoRoot $repoRoot
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Running release build and tests..."
& cmd /c "call `"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat`" -arch=x64 && cmake --preset $Preset && cmake --build --preset $BuildPreset && ctest --test-dir build/$Preset --output-on-failure"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$testExe = Join-Path $repoRoot "build/$Preset/tests/unit/phase07_unit_tests.exe"
if (-not (Test-Path $testExe)) {
    throw "Expected test executable at $testExe"
}

if ($env:OFFLINE_TRANSCRIBER_TEST_MODEL) {
    Write-Host "Running real whisper integration test..."
    & $testExe "WhisperEngine integration transcribes real model when configured"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

Write-Host "Building strict release package..."
powershell -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot "package-windows.ps1") -Preset $Preset -BuildPreset $BuildPreset
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Verifying strict release package..."
powershell -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot "verify-release.ps1")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Release CI passed."
