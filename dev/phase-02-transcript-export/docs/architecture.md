# Architecture

Phase 01 establishes the feature-oriented modular layout for the offline transcription desktop application. This document describes only what exists in phase 01.

## Product goal

Build a fully offline Windows desktop application that transcribes Portuguese and English audio and video files.

## Phase 01 scope

Phase 01 provides:

- A C++20 CMake project that configures, builds, and runs unit tests
- Shared primitives in `src/shared/`
- Documentation map for future phases
- Architecture folder guard tests

Phase 01 does not implement Qt UI, whisper.cpp, SQLite, FFmpeg, exporters, or transcription workflow code.

## Allowed repository layout

```text
phase-01-foundation/
├─ CMakeLists.txt
├─ CMakePresets.json
├─ docs/
├─ src/
│  └─ shared/
├─ tests/
│  └─ unit/
└─ build/                 # generated, not committed
```

Future phases extend the root project with:

```text
apps/desktop/
src/app/
src/audio/
src/transcription/
src/whisper_engine/
src/export/
src/storage/
src/platform/
src/shared/
tests/
tools/
models/
packaging/
```

## Module rules

- Organize code by product capability, not Clean Architecture layers.
- UI must not call whisper.cpp, FFmpeg, SQLite, or exporters directly.
- Only `src/whisper_engine` may include whisper.cpp headers (future phase).
- FFmpeg command construction belongs only in `src/audio` or `src/platform` (future phase).
- Exporters depend only on transcript data and shared primitives (future phase).
- The application remains fully offline during normal use.

## Forbidden folder names

Do not create these under `src/`:

```text
domain/
application/
infrastructure/
usecases/
entities/
```

Generic dumping-ground modules such as `Utils`, `Helper`, `Manager`, or `Core` are also forbidden unless they own a clearly named product responsibility.

## Shared primitives (phase 01)

| File | Responsibility |
|------|----------------|
| `src/shared/Error.hpp` | Typed `ErrorCode` and `AppError` |
| `src/shared/Result.hpp` | Success/failure result without exceptions |
| `src/shared/CancellationToken.hpp` | Deterministic cancellation flag |
| `src/shared/Time.hpp` | Duration and UTC timestamp formatting helpers |

## Dependency direction

Phase 01 has one interface library target:

```text
offline_transcriber_shared (INTERFACE)
  └─ tests/unit/phase01_unit_tests
```

Later phases add feature modules that depend on the same way: product modules may use `src/shared`, but shared code must not depend on product modules.

## References

- Parent architecture: `offline-transcription-architecture.md` in the repository root
- Agent contract: `AGENTS.md`
- Build instructions: `docs/build-windows.md`
