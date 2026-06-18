param(
    [string]$RepoRoot = "",
    [string]$FfmpegPath = "",
    [ValidateSet("tiny", "small", "both")]
    [string]$Models = "both",
    [switch]$SkipModels
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
    $RepoRoot = Split-Path -Parent $PSScriptRoot
}

function Resolve-FfmpegPath {
    param([string]$ExplicitPath)

    if (-not [string]::IsNullOrWhiteSpace($ExplicitPath) -and (Test-Path $ExplicitPath)) {
        return (Resolve-Path $ExplicitPath).Path
    }

    if (-not [string]::IsNullOrWhiteSpace($env:OFFLINE_TRANSCRIBER_FFMPEG_PATH) -and (Test-Path $env:OFFLINE_TRANSCRIBER_FFMPEG_PATH)) {
        return (Resolve-Path $env:OFFLINE_TRANSCRIBER_FFMPEG_PATH).Path
    }

    if (-not [string]::IsNullOrWhiteSpace($env:FFMPEG_PATH) -and (Test-Path $env:FFMPEG_PATH)) {
        return (Resolve-Path $env:FFMPEG_PATH).Path
    }

    $whereOutput = & where.exe ffmpeg 2>$null
    if ($LASTEXITCODE -eq 0 -and $whereOutput) {
        $candidate = ($whereOutput | Select-Object -First 1).Trim()
        if (Test-Path $candidate) {
            return (Resolve-Path $candidate).Path
        }
    }

    throw "FFmpeg was not found. Install FFmpeg or pass -FfmpegPath."
}

function Install-Ffmpeg {
    param([string]$SourcePath, [string]$DestinationPath)

    New-Item -ItemType Directory -Force -Path (Split-Path $DestinationPath -Parent) | Out-Null
    Copy-Item $SourcePath $DestinationPath -Force
    Write-Host "Installed FFmpeg at $DestinationPath"
}

function Get-ModelSpec {
    param([string]$Name)

    switch ($Name) {
        "tiny" {
            return @{
                FileName = "ggml-tiny.bin"
                Url = "https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-tiny.bin"
                Sha1 = "bd577a113a864445d4c299885e0cb97d4ba92b5f"
            }
        }
        "small" {
            return @{
                FileName = "ggml-small.bin"
                Url = "https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-small.bin"
                Sha1 = "55356645c2b361a969dfd0ef2c5a50d530afd8d5"
            }
        }
        default { throw "Unknown model name: $Name" }
    }
}

function Install-Model {
    param(
        [string]$Name,
        [string]$ModelsDirectory
    )

    $spec = Get-ModelSpec -Name $Name
    $destination = Join-Path $ModelsDirectory $spec.FileName
    if (Test-Path $destination) {
        $existingHash = (Get-FileHash -Algorithm SHA1 $destination).Hash.ToLowerInvariant()
        if ($existingHash -eq $spec.Sha1) {
            Write-Host "Model already present: $destination"
            return $destination
        }
        Write-Host "Replacing model with mismatched hash at $destination"
    }

    New-Item -ItemType Directory -Force -Path $ModelsDirectory | Out-Null
    $tempFile = Join-Path $ModelsDirectory ($spec.FileName + ".download")
    if (Test-Path $tempFile) {
        Remove-Item -Force $tempFile
    }

    Write-Host "Downloading $($spec.FileName)..."
    Invoke-WebRequest -Uri $spec.Url -OutFile $tempFile

    $downloadedHash = (Get-FileHash -Algorithm SHA1 $tempFile).Hash.ToLowerInvariant()
    if ($downloadedHash -ne $spec.Sha1) {
        Remove-Item -Force $tempFile
        throw "Downloaded $($spec.FileName) failed SHA1 verification."
    }

    Move-Item -Force $tempFile $destination
    Write-Host "Installed model at $destination"
    return $destination
}

$resolvedFfmpeg = Resolve-FfmpegPath -ExplicitPath $FfmpegPath
$toolsDirectory = Join-Path $RepoRoot "tools"
$modelsDirectory = Join-Path $RepoRoot "models"
$ffmpegDestination = Join-Path $toolsDirectory "ffmpeg.exe"

Install-Ffmpeg -SourcePath $resolvedFfmpeg -DestinationPath $ffmpegDestination

$installedModels = @()
if (-not $SkipModels) {
    switch ($Models) {
        "tiny" { $installedModels += Install-Model -Name "tiny" -ModelsDirectory $modelsDirectory }
        "small" { $installedModels += Install-Model -Name "small" -ModelsDirectory $modelsDirectory }
        "both" {
            $installedModels += Install-Model -Name "tiny" -ModelsDirectory $modelsDirectory
            $installedModels += Install-Model -Name "small" -ModelsDirectory $modelsDirectory
        }
    }
}

$env:OFFLINE_TRANSCRIBER_FFMPEG_PATH = $ffmpegDestination
$smallModel = Join-Path $modelsDirectory "ggml-small.bin"
if (Test-Path $smallModel) {
    $env:OFFLINE_TRANSCRIBER_MODEL_PATH = $smallModel
}
$tinyModel = Join-Path $modelsDirectory "ggml-tiny.bin"
if (Test-Path $tinyModel) {
    $env:OFFLINE_TRANSCRIBER_TEST_MODEL = $tinyModel
}

Write-Host "Runtime artifacts ready."
Write-Host "FFmpeg: $ffmpegDestination"
foreach ($modelPath in $installedModels) {
    Write-Host "Model: $modelPath"
}
