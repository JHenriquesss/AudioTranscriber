param(
    [string]$RepoRoot = ""
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
    $RepoRoot = Split-Path -Parent $PSScriptRoot
}

$thirdPartyRoot = Join-Path $RepoRoot "third_party"
New-Item -ItemType Directory -Force -Path $thirdPartyRoot | Out-Null

$sqliteDir = Join-Path $thirdPartyRoot "sqlite"
if (-not (Test-Path (Join-Path $sqliteDir "sqlite3.c"))) {
    $sqliteZip = Join-Path $thirdPartyRoot "sqlite-amalgamation-3460100.zip"
    if (-not (Test-Path $sqliteZip)) {
        Write-Host "Downloading SQLite amalgamation..."
        Invoke-WebRequest -Uri "https://www.sqlite.org/2024/sqlite-amalgamation-3460100.zip" -OutFile $sqliteZip
    }

    $sqliteTmp = Join-Path $thirdPartyRoot "sqlite-tmp"
    if (Test-Path $sqliteTmp) {
        Remove-Item -Recurse -Force $sqliteTmp
    }
    Expand-Archive -Path $sqliteZip -DestinationPath $sqliteTmp -Force
    $extracted = Get-ChildItem $sqliteTmp -Directory | Select-Object -First 1
    New-Item -ItemType Directory -Force -Path $sqliteDir | Out-Null
    Copy-Item (Join-Path $extracted.FullName "sqlite3.c") (Join-Path $sqliteDir "sqlite3.c")
    Copy-Item (Join-Path $extracted.FullName "sqlite3.h") (Join-Path $sqliteDir "sqlite3.h")
    Copy-Item (Join-Path $extracted.FullName "sqlite3ext.h") (Join-Path $sqliteDir "sqlite3ext.h")
    Remove-Item -Recurse -Force $sqliteTmp
}

$catchDir = Join-Path $thirdPartyRoot "catch2"
if (-not (Test-Path (Join-Path $catchDir "CMakeLists.txt"))) {
    Write-Host "Cloning Catch2 v3.5.4..."
    git clone --depth 1 --branch v3.5.4 https://github.com/catchorg/Catch2.git $catchDir
}

$whisperDir = Join-Path $thirdPartyRoot "whisper.cpp"
if (-not (Test-Path (Join-Path $whisperDir "CMakeLists.txt"))) {
    Write-Host "Cloning whisper.cpp v1.7.4..."
    git clone --depth 1 --branch v1.7.4 https://github.com/ggerganov/whisper.cpp.git $whisperDir
}

Write-Host "Third-party bootstrap complete at $thirdPartyRoot"
