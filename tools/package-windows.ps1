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

Copy-Item -LiteralPath $builtExe -Destination (Join-Path $packageRoot "OfflineTranscriber.exe") -Force
Copy-Item -LiteralPath (Join-Path $repoRoot "models/index.json") -Destination (Join-Path $packageRoot "models/index.json") -Force

if ([string]::IsNullOrWhiteSpace($FfmpegPath) -and -not [string]::IsNullOrWhiteSpace($env:OFFLINE_TRANSCRIBER_FFMPEG_PATH)) {
    $FfmpegPath = $env:OFFLINE_TRANSCRIBER_FFMPEG_PATH
}
if ([string]::IsNullOrWhiteSpace($ModelPath) -and -not [string]::IsNullOrWhiteSpace($env:OFFLINE_TRANSCRIBER_MODEL_PATH)) {
    $ModelPath = $env:OFFLINE_TRANSCRIBER_MODEL_PATH
}
if ([string]::IsNullOrWhiteSpace($LicensesDir) -and -not [string]::IsNullOrWhiteSpace($env:OFFLINE_TRANSCRIBER_LICENSES_DIR)) {
    $LicensesDir = $env:OFFLINE_TRANSCRIBER_LICENSES_DIR
}

function Resolve-ExistingPath {
    param([string]$Path)
    if ([string]::IsNullOrWhiteSpace($Path)) {
        return $null
    }
    if (Test-Path $Path) {
        return (Resolve-Path $Path).Path
    }
    return $null
}

function Resolve-FfmpegSource {
    param([string]$ExplicitPath, [string]$RepoRoot)

    $candidates = @(
        (Resolve-ExistingPath $ExplicitPath),
        (Resolve-ExistingPath $env:OFFLINE_TRANSCRIBER_FFMPEG_PATH),
        (Resolve-ExistingPath (Join-Path $RepoRoot "tools/ffmpeg.exe"))
    ) | Where-Object { $_ -ne $null }

    if (@($candidates).Count -gt 0) {
        return @($candidates)[0]
    }

    $whereOutput = & where.exe ffmpeg 2>$null
    if ($LASTEXITCODE -eq 0 -and $whereOutput) {
        $fromPath = ($whereOutput | Select-Object -First 1).Trim()
        $resolved = Resolve-ExistingPath $fromPath
        if ($null -ne $resolved) {
            return $resolved
        }
    }

    return $null
}

$sourceFfmpeg = Resolve-FfmpegSource -ExplicitPath $FfmpegPath -RepoRoot $repoRoot
if ($null -ne $sourceFfmpeg) {
    Copy-Item -LiteralPath $sourceFfmpeg -Destination (Join-Path $packageRoot "tools/ffmpeg.exe") -Force
} elseif ($AllowPlaceholders) {
    @(
        "FFmpeg is not bundled in this repository snapshot."
        "Place a licensed ffmpeg.exe in tools/ffmpeg.exe before shipping."
    ) | Set-Content -Path (Join-Path $packageRoot "tools/ffmpeg.exe.placeholder.txt") -Encoding UTF8
} else {
    throw "Missing FFmpeg executable. Run tools/setup-runtime-artifacts.ps1 or set OFFLINE_TRANSCRIBER_FFMPEG_PATH."
}

$defaultModel = if ([string]::IsNullOrWhiteSpace($ModelPath)) {
    Join-Path $repoRoot "models/ggml-small.bin"
} else {
    $ModelPath
}
if (Test-Path -LiteralPath $defaultModel) {
    Copy-Item -LiteralPath $defaultModel -Destination (Join-Path $packageRoot "models/ggml-small.bin") -Force
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
        Copy-Item -LiteralPath $source -Destination (Join-Path $packageRoot "licenses/$licenseFile") -Force
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
