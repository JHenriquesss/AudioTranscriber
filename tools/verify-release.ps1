param(
    [string]$PackageRoot = "",
    [switch]$AllowPlaceholders
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
    "tools/ffmpeg.exe",
    "models/ggml-small.bin",
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

if ($AllowPlaceholders) {
    $missing = $missing | Where-Object {
        $_ -ne "tools/ffmpeg.exe" -and $_ -ne "models/ggml-small.bin"
    }
    foreach ($placeholder in @("tools/ffmpeg.exe.placeholder.txt", "models/ggml-small.bin.placeholder.txt")) {
        $fullPath = Join-Path $PackageRoot $placeholder
        if (-not (Test-Path $fullPath)) {
            $missing += $placeholder
        }
    }
}

if (-not $AllowPlaceholders) {
    $placeholderFiles = Get-ChildItem -Path $PackageRoot -Recurse -Filter "*.placeholder.txt" -ErrorAction SilentlyContinue
    if ($placeholderFiles) {
        $missing += "no placeholder files in release package"
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
