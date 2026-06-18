# Phase 03: App Runtime, Settings, Jobs, Logs

## Scope
Implement app paths, settings, job status transitions, structured logging shape, and a testable job lifecycle shell. This phase creates the runtime backbone without running real audio conversion or whisper transcription.

## Entry condition
- Phase 02 transcript/export code exists or equivalent interfaces are present.
- Shared error/result/cancellation primitives are available.

## Exit condition
- Job lifecycle state transitions are explicit, tested, cancellable, and logged.
- Settings load/save behavior is tested with valid, missing, and invalid config files.
- `PHASE-RESULT.md` contains evidence and remaining risk.

## Must-exist checklist
- [ ] `src/app/AppPaths.*`, `AppSettings.*`, `JobStatus.*`, `TranscriptionJob.*`, and `JobManager.*` exist.
- [ ] Settings support default language/model/output path/max parallel jobs/word timestamps/theme as defined by architecture.
- [ ] Job status transitions cover queued through completed plus failed and cancelled.
- [ ] Structured log event names match architecture for startup, settings, job, audio, transcription, export, failure, and cancellation.
- [ ] Tests verify settings, job state transitions, cancellation, and log redaction rules.

## Must-not-exist checklist
- [ ] No UI widget owns job workflow logic.
- [ ] No transcription, FFmpeg, SQLite, or whisper.cpp direct implementation in `src/app`.
- [ ] No transcript content logged by default.
- [ ] No global mutable settings read deep inside modules.
- [ ] No background work that cannot be cancelled or observed.

## Test plan
### Positive
- [ ] Default settings are created when no settings file exists.
- [ ] A job follows the expected status sequence and emits progress events.
- [ ] Cancellation changes job outcome to cancelled at a checkpoint.

### Negative
- [ ] Invalid settings produce typed configuration errors.
- [ ] Invalid job status transition is rejected or impossible by API design.
- [ ] Logs redact transcript-like sensitive content.

## Test tree integration
- Trunk touch point: User action becomes a job -> job progresses -> UI can observe status/error/export paths later.
- New branches added: Settings tests, job state machine tests, cancellation tests, logging redaction tests.

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
Phase 04 connects the runtime to real media preparation through the FFmpeg boundary.
