# Third-Party Source Dependencies

This project vendors source dependencies locally so CMake can configure without network access.

## Bootstrap

Run once on a fresh checkout:

```powershell
powershell -ExecutionPolicy Bypass -File tools/bootstrap-third-party.ps1
```

## Contents

| Directory | Purpose | Version |
|-----------|---------|---------|
| `sqlite/` | SQLite amalgamation | 3.46.1 |
| `catch2/` | Catch2 test framework | v3.5.4 |
| `whisper.cpp/` | whisper.cpp inference engine | v1.7.4 |

## Runtime Artifacts (not vendored)

These are intentionally **not** committed:

- `tools/ffmpeg.exe`
- `models/ggml-*.bin`
- Generated `build/` and `dist/` output

Provide them locally or via environment variables documented in `docs/RELEASE-ARTIFACTS.md`.
