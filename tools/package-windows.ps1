param(
    [string]$Configuration = "Release",
    [string]$Preset = "windows-msvc-release",
    [string]$BuildPreset = "release",
    [string]$BuildDir = "",
    [string]$FfmpegPath = "",
    [string]$ModelPath = "",
    [string]$LicensesDir = "",
    [switch]$AllowPlaceholders
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    $BuildDir = Join-Path $repoRoot "build/$Preset"
}
$packageRoot = Join-Path $repoRoot "dist/OfflineTranscriber"
$exeName = "offline_transcriber_desktop.exe"

Write-Host "Configuring release build..."
cmake --preset $Preset
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Building release binary..."
cmake --build --preset $BuildPreset
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$builtExe = Join-Path $BuildDir "apps/desktop/$exeName"
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

if ([string]::IsNullOrWhiteSpace($FfmpegPath) -and -not [string]::IsNullOrWhiteSpace($env:OFFLINE_TRANSCRIBER_FFMPEG_PATH)) {
    $FfmpegPath = $env:OFFLINE_TRANSCRIBER_FFMPEG_PATH
}
if ([string]::IsNullOrWhiteSpace($ModelPath) -and -not [string]::IsNullOrWhiteSpace($env:OFFLINE_TRANSCRIBER_MODEL_PATH)) {
    $ModelPath = $env:OFFLINE_TRANSCRIBER_MODEL_PATH
}
if ([string]::IsNullOrWhiteSpace($LicensesDir) -and -not [string]::IsNullOrWhiteSpace($env:OFFLINE_TRANSCRIBER_LICENSES_DIR)) {
    $LicensesDir = $env:OFFLINE_TRANSCRIBER_LICENSES_DIR
}

$sourceFfmpeg = if ([string]::IsNullOrWhiteSpace($FfmpegPath)) {
    Join-Path $repoRoot "tools/ffmpeg.exe"
} else {
    $FfmpegPath
}
if (Test-Path $sourceFfmpeg) {
    Copy-Item $sourceFfmpeg (Join-Path $packageRoot "tools/ffmpeg.exe")
} elseif ($AllowPlaceholders) {
    @(
        "FFmpeg is not bundled in this repository snapshot."
        "Place a licensed ffmpeg.exe in tools/ffmpeg.exe before shipping."
    ) | Set-Content -Path (Join-Path $packageRoot "tools/ffmpeg.exe.placeholder.txt") -Encoding UTF8
} else {
    throw "Missing tools/ffmpeg.exe. Use -AllowPlaceholders only for layout-only development packages."
}

$defaultModel = if ([string]::IsNullOrWhiteSpace($ModelPath)) {
    Join-Path $repoRoot "models/ggml-small.bin"
} else {
    $ModelPath
}
if (Test-Path $defaultModel) {
    Copy-Item $defaultModel (Join-Path $packageRoot "models/ggml-small.bin")
} elseif ($AllowPlaceholders) {
    @(
        "Default Whisper model is not bundled in this repository snapshot."
        "Place ggml-small.bin in models/ before shipping."
    ) | Set-Content -Path (Join-Path $packageRoot "models/ggml-small.bin.placeholder.txt") -Encoding UTF8
} else {
    throw "Missing models/ggml-small.bin. Use -AllowPlaceholders only for layout-only development packages."
}

$licenseFiles = @(
    "LICENSE-app.txt",
    "LICENSE-qt.txt",
    "LICENSE-sqlite.txt",
    "LICENSE-ffmpeg.txt",
    "LICENSE-whisper.txt"
)
foreach ($licenseFile in $licenseFiles) {
    $licenseRoot = if ([string]::IsNullOrWhiteSpace($LicensesDir)) {
        Join-Path $repoRoot "licenses"
    } else {
        $LicensesDir
    }
    $source = Join-Path $licenseRoot $licenseFile
    if (Test-Path $source) {
        Copy-Item $source (Join-Path $packageRoot "licenses/$licenseFile")
    } else {
        if ($AllowPlaceholders) {
            @(
                "Placeholder license file for $licenseFile."
                "Replace with the real third-party license text before release."
            ) | Set-Content -Path (Join-Path $packageRoot "licenses/$licenseFile") -Encoding UTF8
        } else {
            throw "Missing licenses/$licenseFile. Use -AllowPlaceholders only for layout-only development packages."
        }
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

if ($null -ne $windeployqt -and (Test-Path $windeployqt)) {
    Write-Host "Running windeployqt..."
    & $windeployqt (Join-Path $packageRoot "OfflineTranscriber.exe")
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
} else {
    Write-Warning "windeployqt was not found. Qt runtime files must be copied manually."
}

Write-Host "Portable package created at $packageRoot"
