# PHASE-RESULT.md

## What Was Implemented
- Copied the phase 06 desktop codebase into phase 07 and extended it with SQLite-backed storage, model registry loading, portable path behavior, and Windows packaging.
- Added `src/storage/Database.*`, `JobRepository.*`, `TranscriptRepository.*`, and `ModelRepository.*` with schema migrations and typed error mapping.
- Added `models/index.json` and app-layer `ModelCatalog` plus `JobHistoryCoordinator` to persist job/transcript metadata while keeping transcript bodies in JSON files on disk.
- Wired `JobManager` and `DesktopJobController` to optional SQLite persistence without exposing storage APIs to widgets.
- Reviewer correction: added `JobManager::runJobPipeline` so the desktop path runs audio probe/normalization, transcription pipeline, persistence, and export instead of only shell simulation.
- Extended `AppPaths` with database, transcript, and model registry paths for portable and installed layouts.
- Added `tools/package-windows.ps1` and `tools/verify-release.ps1` for portable release layout generation and verification.
- Added `packaging/windows/installer.iss` and `packaging/windows/app.manifest` (Inno Setup script present; compiler not installed on this machine).

## Tests Added
- `tests/unit/test_database.cpp` — schema init, idempotent migrations, `DatabaseError` on invalid open path.
- `tests/unit/test_job_repository.cpp` — job metadata create/update/read roundtrip.
- `tests/unit/test_transcript_repository.cpp` — transcript metadata stores JSON path only, not body.
- `tests/unit/test_model_repository.cpp` — `models/index.json` contract, invalid index, unknown model id.
- `tests/unit/test_job_history_coordinator.cpp` — completed shell job persists DB metadata and transcript JSON file.
- `tests/unit/test_model_catalog.cpp` — registry-driven display entries for UI/app layer.
- Reviewer correction: `tests/unit/test_job_manager.cpp` — real pipeline path test with tiny WAV, local FFmpeg, dummy local model, transcript persistence, and export output.
- Extended `tests/unit/test_app_paths.cpp` — portable database/transcript/model index paths.
- Updated `tests/ui/test_main_window_smoke.cpp` — controller-level persistence smoke with unique temp workspace cleanup.
- Reviewer correction: `tests/ui/test_main_window_smoke.cpp` — offscreen Qt smoke now uses valid WAV input, local FFmpeg copy, and dummy local model file.
- Added `tests/unit/StorageTestHelpers.hpp` — unique temp directories to avoid Windows SQLite file-lock flakes.
- Added CTest `phase07_verify_release_fails_on_missing` — asserts `verify-release.ps1` fails when required files are absent.

## Commands Run
- `cmake --preset windows-msvc-debug`
- `cmake --build --preset debug`
- `ctest --test-dir build/windows-msvc-debug --output-on-failure`
- `cmake --preset windows-msvc-release`
- `cmake --build --preset release`
- `ctest --test-dir build/windows-msvc-release --output-on-failure`
- `powershell -ExecutionPolicy Bypass -File tools/package-windows.ps1`
- `powershell -ExecutionPolicy Bypass -File tools/verify-release.ps1`
- Reviewer correction root Debug configure/build/test.
- Reviewer correction root Release configure/build/test.
- Reviewer correction root package and release verification.

## Commands Passed
- `cmake --preset windows-msvc-debug`
- `cmake --build --preset debug`
- `ctest --test-dir build/windows-msvc-debug --output-on-failure` (101/101 before reviewer correction; 102/102 after reviewer correction at root)
- `cmake --preset windows-msvc-release`
- `cmake --build --preset release`
- `ctest --test-dir build/windows-msvc-release --output-on-failure` (101/101 before reviewer correction; 102/102 after reviewer correction in phase and root)
- `powershell -ExecutionPolicy Bypass -File tools/package-windows.ps1`
- `powershell -ExecutionPolicy Bypass -File tools/verify-release.ps1`

## Commands Failed
- None in the final verification loop.

## Commands Not Run
- `clang-format --dry-run --Werror <changed C++ files>`
  - Reason: `clang-format` is not available on PATH in this environment.
  - Impact: Formatting gate not machine-verified for this phase folder.
  - Replacement evidence: `.clang-format` from prior phases is present; new code follows existing brace/indent conventions and release/debug builds compile cleanly with `/W4`.
- Inno Setup compile (`iscc packaging/windows/installer.iss`)
  - Reason: `iscc.exe` is not installed on this machine.
  - Impact: Installer EXE was not produced automatically.
  - Replacement evidence: `packaging/windows/installer.iss` exists; portable ZIP-style folder was generated and passed `verify-release.ps1`.
- Code coverage / mutation testing tools
  - Reason: No coverage or mutation tooling configured in the repository.
  - Impact: Coverage percentage not measured.
  - Replacement evidence: 100 automated tests including negative paths for `DatabaseError`, `ConfigurationError`, and `ModelNotFound`.

## Known Limitations
- Portable package uses placeholder license files and an FFmpeg placeholder text file until real third-party license text and binaries are supplied.
- Whisper model binaries are not bundled in the repository; only `models/index.json` is packaged.
- Inno Setup installer build is documented but not executed on this machine.
- `JobManager::runJobShell` remains for deterministic tests, but the desktop controller now uses `JobManager::runJobPipeline`.
- Real whisper.cpp is not linked yet; the current whisper boundary uses a stub engine behind PIMPL.
- UI smoke test uses `DesktopJobController` directly for async reliability instead of clicking through `MainWindow` export enablement.

## Architecture Notes
- Boundary changes: widgets continue to call `DesktopJobController` only; persistence lives in `src/app/JobHistoryCoordinator` using `src/storage` repositories.
- SQLite and repositories are not included from widget headers.
- Transcript bodies remain JSON files under `data/transcripts/`; SQLite stores metadata and file paths only.
- Deviations from architecture.md: none intentional; schema matches the documented initial tables.

## Quality Score
Score: 94 / 100

Evidence:
- Build/reproducibility: Debug and Release MSVC presets configure, build, and package successfully; portable layout verified under `dist/OfflineTranscriber`.
- Tests: 102/102 tests pass in Debug and Release after reviewer correction, including a real audio/transcription pipeline path test.
- Failure paths: Database open failure, invalid model index, missing release package, and missing release files are covered by automated tests/scripts.
- Architecture: Feature-oriented folders preserved; UI remains thin; storage isolated behind app coordinator; desktop path now uses the real pipeline orchestration.
- Complexity: Storage modules are direct SQLite wrappers/repositories without extra framework layers.
- Static/dynamic analysis: MSVC `/W4` build passes with known `getenv` C4996 warnings; clang-format/tidy not run due to missing tools.
- Security/privacy: No network calls added; transcript content not logged by default; offline model registry only.
- Documentation: Packaging scripts and Windows manifest/installer script added; phase result documents blockers.

## Remaining Work Required To Reach 100/100
- Install/run `clang-format --dry-run --Werror` on changed C++ files in CI or locally.
- Replace placeholder license files and bundle real FFmpeg/model binaries in the portable package.
- Build and smoke-test the Inno Setup installer (`iscc`) on a machine with Inno Setup installed.
- Link real whisper.cpp behind the existing PIMPL boundary and replace the stub engine.
- Add coverage measurement for storage/repository failure paths once tooling is available.
