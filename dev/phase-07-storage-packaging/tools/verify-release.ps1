param(
    [string]$PackageRoot = ""
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($PackageRoot)) {
    $PackageRoot = Join-Path $repoRoot "dist/OfflineTranscriber"
}

$requiredFiles = @(
    "OfflineTranscriber.exe",
    "models/index.json",
    "data",
    "logs",
    "exports",
    "tools/ffmpeg.exe.placeholder.txt",
    "licenses/LICENSE-app.txt",
    "licenses/LICENSE-qt.txt",
    "licenses/LICENSE-sqlite.txt",
    "licenses/LICENSE-ffmpeg.txt",
    "licenses/LICENSE-whisper.txt"
)

$missing = @()
foreach ($relativePath in $requiredFiles) {
    $fullPath = Join-Path $PackageRoot $relativePath
    if (-not (Test-Path $fullPath)) {
        $missing += $relativePath
    }
}

$qtDll = Get-ChildItem -Path $PackageRoot -Filter "Qt6Core.dll" -ErrorAction SilentlyContinue
if (-not $qtDll) {
    $missing += "Qt6Core.dll"
}

if ($missing.Count -gt 0) {
    Write-Error ("Release verification failed. Missing required files:`n - " + ($missing -join "`n - "))
    exit 1
}

Write-Host "Release verification passed for $PackageRoot"
