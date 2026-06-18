param(
    [string]$Configuration = "Release",
    [string]$Preset = "windows-msvc-release",
    [string]$BuildPreset = "release"
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot "build/windows-msvc-release"
$packageRoot = Join-Path $repoRoot "dist/OfflineTranscriber"
$exeName = "offline_transcriber_desktop.exe"

Write-Host "Configuring release build..."
cmake --preset $Preset
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Building release binary..."
cmake --build --preset $BuildPreset
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$builtExe = Join-Path $buildDir "apps/desktop/$exeName"
if (-not (Test-Path $builtExe)) {
    throw "Expected desktop executable was not found at $builtExe"
}

Write-Host "Creating portable package layout at $packageRoot"
if (Test-Path $packageRoot) {
    Remove-Item -Recurse -Force $packageRoot
}

$folders = @(
    "data",
    "logs",
    "exports",
    "models",
    "tools",
    "licenses"
)
foreach ($folder in $folders) {
    New-Item -ItemType Directory -Force -Path (Join-Path $packageRoot $folder) | Out-Null
}

Copy-Item $builtExe (Join-Path $packageRoot "OfflineTranscriber.exe")
Copy-Item (Join-Path $repoRoot "models/index.json") (Join-Path $packageRoot "models/index.json")

$ffmpegPlaceholder = Join-Path $packageRoot "tools/ffmpeg.exe.placeholder.txt"
@(
    "FFmpeg is not bundled in this repository snapshot."
    "Place a licensed ffmpeg.exe in tools/ffmpeg.exe before shipping."
) | Set-Content -Path $ffmpegPlaceholder -Encoding UTF8

$licenseFiles = @(
    "LICENSE-app.txt",
    "LICENSE-qt.txt",
    "LICENSE-sqlite.txt",
    "LICENSE-ffmpeg.txt",
    "LICENSE-whisper.txt"
)
foreach ($licenseFile in $licenseFiles) {
    $source = Join-Path $repoRoot "licenses/$licenseFile"
    if (Test-Path $source) {
        Copy-Item $source (Join-Path $packageRoot "licenses/$licenseFile")
    } else {
        @(
            "Placeholder license file for $licenseFile."
            "Replace with the real third-party license text before release."
        ) | Set-Content -Path (Join-Path $packageRoot "licenses/$licenseFile") -Encoding UTF8
    }
}

$windeployqt = $null
$qtCandidates = @(
    "C:/Qt/6.8.2/msvc2022_64/bin/windeployqt.exe",
    "C:/Qt/6.8.2/msvc2019_64/bin/windeployqt.exe"
)
if (-not [string]::IsNullOrWhiteSpace($env:CMAKE_PREFIX_PATH)) {
    $qtCandidates = @((Join-Path $env:CMAKE_PREFIX_PATH "bin/windeployqt.exe")) + $qtCandidates
}
foreach ($candidate in $qtCandidates) {
    if (Test-Path $candidate) {
        $windeployqt = $candidate
        break
    }
}

if (Test-Path $windeployqt) {
    Write-Host "Running windeployqt..."
    & $windeployqt (Join-Path $packageRoot "OfflineTranscriber.exe")
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
} else {
    Write-Warning "windeployqt was not found. Qt runtime files must be copied manually."
}

Write-Host "Portable package created at $packageRoot"
