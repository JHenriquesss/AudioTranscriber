# Phase 05: Whisper Boundary And Transcription Pipeline

## Scope
Implement the isolated whisper.cpp boundary, options mapping, and transcription pipeline orchestration from normalized audio to transcript document. Use a fake engine for deterministic tests and only integrate real whisper.cpp when dependency evidence is available.

## Entry condition
- Phase 04 audio normalization boundary exists.
- Transcript model/exporters and app cancellation primitives exist.

## Exit condition
- Only `src/whisper_engine` includes whisper.cpp headers.
- Pipeline tests cover success, engine failure, missing model, and cancellation.
- `PHASE-RESULT.md` records whether real whisper.cpp was built or why it was not.

## Must-exist checklist
- [ ] `src/whisper_engine/WhisperEngine.*`, `WhisperModel.*`, and `WhisperOptionsMapper.*` exist with PIMPL isolation.
- [ ] `src/transcription/TranscriptionOptions.hpp`, `TranscriptionResult.hpp`, `TranscriptionPipeline.*`, and `TranscriptPostProcessor.*` exist.
- [ ] Fake engine or test double enables deterministic pipeline tests without real model files.
- [ ] Architecture check confirms no whisper.cpp headers outside `src/whisper_engine`.
- [ ] Cancellation checkpoints exist before model load, during transcription if supported, and before export handoff.

## Must-not-exist checklist
- [ ] No whisper.cpp headers leak into `src/transcription`, `src/app`, `src/audio`, `src/export`, or UI.
- [ ] No real model download or network dependency.
- [ ] No transcription on UI thread.
- [ ] No tests requiring large model files by default.
- [ ] No engine errors collapsed into `Unknown` without context.

## Test plan
### Positive
- [ ] Fake pipeline produces a transcript document for a normalized WAV path.
- [ ] Options mapper handles auto, Portuguese, and English language options.
- [ ] Post-processor normalizes whitespace deterministically.

### Negative
- [ ] Missing model returns `ModelNotFound`.
- [ ] Engine load failure returns `ModelLoadFailed`.
- [ ] Cancellation before or during pipeline returns `Cancelled`.

## Test tree integration
- Trunk touch point: Normalized WAV -> model load -> transcript document -> ready for display/export.
- New branches added: Options mapping, fake engine pipeline, missing model, cancellation, whisper header leak check.

## Quality gates for this phase
- Follow `QUALITY-GATES.md`.
- Follow `LANGUAGE-QUALITY-GATE.md`.
- Run or document these commands as applicable:
- `cmake --preset windows-msvc-debug`
- `cmake --build --preset debug`
- `ctest --test-dir build/windows-msvc-debug --output-on-failure`
- `clang-format --dry-run --Werror <changed C++ files>`
- `rg "whisper" src --glob "*.hpp" --glob "*.cpp"`
- Document coverage evidence or why coverage tooling is unavailable.
- Keep business/workflow decisions out of UI, database, external tool, and engine adapter code.
- Create `PHASE-RESULT.md` before sending the final message.

## Next phase seed
Phase 06 wires the tested runtime and pipeline into the Qt desktop UI.
