# Offline Transcriber

Offline-first Windows desktop transcription app built with C++20, Qt 6 Widgets, whisper.cpp, SQLite, and FFmpeg.

## Prerequisites

- Visual Studio 2022 with C++ desktop development workload
- CMake 3.24+
- Ninja
- Qt 6.8+ (MSVC 64-bit)
- Git
- PowerShell

## Bootstrap

```powershell
powershell -ExecutionPolicy Bypass -File tools/bootstrap-third-party.ps1
powershell -ExecutionPolicy Bypass -File tools/setup-runtime-artifacts.ps1
```

`setup-runtime-artifacts.ps1` copies FFmpeg from PATH into `tools/ffmpeg.exe` and downloads verified Whisper models into `models/`.

## Build and test

```powershell
cmake --preset windows-msvc-release
cmake --build --preset release
ctest --test-dir build/windows-msvc-release
```

## Release packaging

See [docs/RELEASE-ARTIFACTS.md](docs/RELEASE-ARTIFACTS.md) for external FFmpeg/model requirements and strict packaging commands.

## Architecture

See [offline-transcription-architecture.md](offline-transcription-architecture.md).

## Development workflow

Phase-isolated implementation environments live under `dev/`. Quality evidence is tracked in `dev/FINAL-CAVEMAN-QUALITY-REVIEW.md`.
