# Phase 01: Project Foundation And Test Harness

## Scope
Create the C++20/CMake project skeleton, documentation map, shared primitives, and first test harness. This phase must make the repository buildable and testable without Qt UI, whisper.cpp, SQLite, or FFmpeg features being implemented yet.

## Entry condition
- `offline-transcription-architecture.md`, `myrules.txt`, and `CPP-CODE-QUALITY-GATE.md` exist in the original project root.
- The isolated phase folder contains this plan, AGENTS.md, QUALITY-GATES.md, and LANGUAGE-QUALITY-GATE.md.

## Exit condition
- A minimal CMake project configures, builds, and runs at least one real automated test.
- The repository layout matches the architecture map and forbids Clean Architecture folder drift.
- `PHASE-RESULT.md` documents command evidence and an evidence-based score.

## Must-exist checklist
- [ ] `CMakeLists.txt` and `CMakePresets.json` define a C++20 build with test support.
- [ ] `src/shared/Result.hpp`, `src/shared/Error.hpp`, `src/shared/CancellationToken.hpp`, and `src/shared/Time.hpp` exist with focused tests.
- [ ] `docs/architecture.md`, `docs/ai-map.md`, `docs/build-windows.md`, `docs/error-codes.md`, and `docs/transcription-pipeline.md` exist with only phase-relevant content.
- [ ] `tests/unit/` contains shared primitive tests that can fail for broken behavior.
- [ ] A local test command is documented and runnable from the phase root.

## Must-not-exist checklist
- [ ] No `src/domain`, `src/application`, `src/infrastructure`, `usecases`, `entities`, or generic Clean Architecture folders.
- [ ] No direct Qt UI, whisper.cpp, SQLite, or FFmpeg implementation beyond documented placeholders.
- [ ] No generic `Utils`, `Helper`, `Manager`, or `Core` dumping-ground modules.
- [ ] No tests that only check that files exist.
- [ ] No final response before `PHASE-RESULT.md` exists.

## Test plan
### Positive
- [ ] Configure, build, and run unit tests from a clean phase folder.
- [ ] Shared `Result` returns success and failure values without exceptions.
- [ ] Cancellation token changes from not-cancelled to cancelled deterministically.

### Negative
- [ ] A failed `Result` preserves typed error code and message.
- [ ] Architecture check fails if forbidden folder names are present.
- [ ] Quality score is capped if tests or build commands are skipped.

## Test tree integration
- Trunk touch point: Start the trunk: buildable app foundation -> shared primitives -> first automated test command.
- New branches added: Shared primitive tests, architecture folder guard, build smoke test.

## Quality gates for this phase
- Follow `QUALITY-GATES.md`.
- Follow `LANGUAGE-QUALITY-GATE.md`.
- Run or document these commands as applicable:
- `cmake --preset windows-msvc-debug`
- `cmake --build --preset debug`
- `ctest --test-dir build/windows-msvc-debug --output-on-failure`
- `clang-format --dry-run --Werror <changed C++ files>`
- Document coverage evidence or why coverage tooling is unavailable.
- Keep business/workflow decisions out of UI, database, external tool, and engine adapter code.
- Create `PHASE-RESULT.md` before sending the final message.

## Next phase seed
Phase 02 builds transcript data and deterministic exporters on the tested foundation.
