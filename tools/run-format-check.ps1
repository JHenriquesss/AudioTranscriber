param(
    [string]$BuildDir = "build/windows-msvc-release"
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
$clangFormat = Get-Command clang-format -ErrorAction SilentlyContinue

if ($null -eq $clangFormat) {
    Write-Error "clang-format is not installed. Install LLVM clang-format to run this gate."
    exit 1
}

$sourceFiles = Get-ChildItem -Path (Join-Path $repoRoot "src"), (Join-Path $repoRoot "apps"), (Join-Path $repoRoot "tests") -Recurse -Include *.cpp,*.hpp -File
$failed = @()
foreach ($file in $sourceFiles) {
    $formatted = & $clangFormat.Source --dry-run --Werror $file.FullName 2>&1
    if ($LASTEXITCODE -ne 0) {
        $failed += $file.FullName
    }
}

if ($failed.Count -gt 0) {
    Write-Error ("clang-format violations:`n - " + ($failed -join "`n - "))
    exit 1
}

Write-Host "clang-format check passed for $($sourceFiles.Count) files."
