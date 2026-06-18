# AI Map

This map helps coding agents find the right place to work. Phase 01 contains only foundation code.

## Product

Offline desktop application for transcribing audio and video files in Portuguese and English without network access during normal use.

## Phase 01 workflow

Configure CMake → build shared primitive tests → run CTest from the phase root.

There is no desktop UI or transcription pipeline in this phase.

## Important files (phase 01)

| Path | Purpose |
|------|---------|
| `CMakeLists.txt` | Root build definition |
| `CMakePresets.json` | Windows MSVC Debug/Release presets |
| `src/shared/Result.hpp` | Typed success/failure results |
| `src/shared/Error.hpp` | Error codes and messages |
| `src/shared/CancellationToken.hpp` | Cancellation flag for long-running work |
| `src/shared/Time.hpp` | Time formatting helpers |
| `tests/unit/` | Behavior tests for shared primitives and architecture guard |
| `docs/build-windows.md` | Local build and test commands |

## Future files (not in phase 01)

| Path | Purpose |
|------|---------|
| `apps/desktop/MainWindow.cpp` | Qt main window |
| `src/app/JobManager.cpp` | Job lifecycle |
| `src/transcription/TranscriptionPipeline.cpp` | Transcription workflow |
| `src/whisper_engine/WhisperEngine.cpp` | whisper.cpp integration |
| `src/audio/AudioNormalizer.cpp` | FFmpeg conversion |
| `src/export/SrtExporter.cpp` | SRT export |
| `src/storage/Database.cpp` | SQLite setup |

## Forbidden patterns

- Do not create `domain/`, `application/`, `infrastructure/`, `usecases/`, or `entities/` folders.
- Do not call FFmpeg from UI classes.
- Do not include whisper.cpp headers outside `src/whisper_engine`.
- Do not add generic `Utils/` folders for unrelated code.
- Do not add tests that only assert file existence.

## Test tree (phase 01 trunk)

```text
Buildable foundation
  ├─ Shared primitive tests (Result, Error, CancellationToken, Time)
  ├─ Architecture folder guard
  └─ Build smoke via CTest
```

Phase 02 extends the trunk with transcript data types and deterministic exporters.
