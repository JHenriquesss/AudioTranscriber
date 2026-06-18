param(
    [string]$BuildDir = "build/windows-msvc-release",
    [string]$CompileCommands = ""
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
$clangTidy = Get-Command clang-tidy -ErrorAction SilentlyContinue

if ($null -eq $clangTidy) {
    Write-Error "clang-tidy is not installed. Install LLVM clang-tidy to run this gate."
    exit 1
}

if ([string]::IsNullOrWhiteSpace($CompileCommands)) {
    $CompileCommands = Join-Path $repoRoot "$BuildDir/compile_commands.json"
}

if (-not (Test-Path $CompileCommands)) {
    Write-Error "Missing compile_commands.json at $CompileCommands. Configure with CMAKE_EXPORT_COMPILE_COMMANDS=ON."
    exit 1
}

$sourceFiles = Get-ChildItem -Path (Join-Path $repoRoot "src") -Recurse -Include *.cpp -File
foreach ($file in $sourceFiles) {
    & $clangTidy.Source $file.FullName -p (Split-Path $CompileCommands -Parent)
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}

Write-Host "clang-tidy check passed."
