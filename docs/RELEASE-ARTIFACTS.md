# Release Runtime Artifacts

Runtime binaries are intentionally excluded from git. Provide them locally or run:

```powershell
powershell -ExecutionPolicy Bypass -File tools/setup-runtime-artifacts.ps1
```

## Required for strict release packaging

| Artifact | Default path | Environment override |
|----------|--------------|----------------------|
| FFmpeg | `tools/ffmpeg.exe` | `OFFLINE_TRANSCRIBER_FFMPEG_PATH` |
| Default model | `models/ggml-small.bin` | `OFFLINE_TRANSCRIBER_MODEL_PATH` |
| License bundle | `licenses/` | `OFFLINE_TRANSCRIBER_LICENSES_DIR` |

## Commands

```powershell
# One-shot local release CI (artifacts + build + tests + strict package + verify)
powershell -ExecutionPolicy Bypass -File tools/run-release-ci.ps1

# Layout-only development package (placeholders allowed)
powershell -ExecutionPolicy Bypass -File tools/package-windows.ps1 -AllowPlaceholders

# Strict release package (requires real artifacts)
$env:OFFLINE_TRANSCRIBER_FFMPEG_PATH = 'C:\path\to\ffmpeg.exe'
$env:OFFLINE_TRANSCRIBER_MODEL_PATH = 'C:\path\to\ggml-small.bin'
powershell -ExecutionPolicy Bypass -File tools/package-windows.ps1
powershell -ExecutionPolicy Bypass -File tools/verify-release.ps1
```

## Real whisper integration test

```powershell
$env:OFFLINE_TRANSCRIBER_TEST_MODEL = 'C:\path\to\ggml-tiny.bin'
ctest --test-dir build/windows-msvc-release -R whisper_integration
```
