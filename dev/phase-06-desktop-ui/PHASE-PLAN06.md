# Phase 06: Qt Desktop UI Integration

## Scope
Implement the first Qt Widgets desktop shell and connect user actions to the tested runtime and pipeline boundaries. The UI must stay thin: collect input, display progress, show transcript, and invoke app services without owning transcription logic.

## Entry condition
- Phase 05 pipeline and runtime contracts exist.
- Qt 6 toolchain availability is known or documented as a blocker.

## Exit condition
- Desktop app starts, displays main workflow controls, and can run a fake/deterministic transcription flow in tests or smoke mode.
- UI thread is not blocked by transcription work.
- `PHASE-RESULT.md` documents Qt build/test evidence.

## Must-exist checklist
- [ ] `apps/desktop/main.cpp`, `MainWindow.*`, `widgets/TranscriptionPanel.*`, and `widgets/TranscriptEditor.*` exist.
- [ ] UI offers file selection, language/model choices, start/cancel controls, progress/status display, transcript display, and export actions.
- [ ] UI submits a request to `JobManager` or app runtime boundary instead of running pipeline directly in widget code.
- [ ] A UI smoke test or documented manual smoke command exists.
- [ ] Threading handoff uses Qt-safe signal/slot or equivalent UI-safe callback behavior.

## Must-not-exist checklist
- [ ] No whisper.cpp, FFmpeg, SQLite, or exporter implementation details inside widgets.
- [ ] No long-running transcription work on UI thread.
- [ ] No complex editor, diarization, account, cloud, or batch features.
- [ ] No UI tests depending on a live large model by default.
- [ ] No transcript content written to logs from UI.

## Test plan
### Positive
- [ ] Main window launches and displays the core controls.
- [ ] Start action creates/submits a transcription request with selected language/model/output settings.
- [ ] Progress and transcript display update from fake job events.

### Negative
- [ ] Missing input selection prevents start with a clear error.
- [ ] Cancel action requests cancellation without blocking UI.
- [ ] Pipeline failure displays user-safe error plus error code.

## Test tree integration
- Trunk touch point: User opens app -> selects file/model/language -> starts job -> sees progress/transcript -> exports.
- New branches added: Main window smoke, request mapping, progress update, cancellation button, error display.

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
Phase 07 adds persistence, model registry, packaging, and release verification around the completed workflow.
