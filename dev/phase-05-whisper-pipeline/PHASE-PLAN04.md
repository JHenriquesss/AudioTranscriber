# Phase 04: Audio Preparation And FFmpeg Boundary

## Scope
Implement the platform process runner and audio module that validates media and normalizes audio to 16 kHz mono PCM WAV through FFmpeg. This phase must keep FFmpeg out of UI and app workflow code.

## Entry condition
- Phase 03 runtime and shared primitives exist.
- A small audio fixture strategy is defined or fixture blockers are documented.

## Exit condition
- Audio command construction is deterministic and tested.
- Integration evidence exists for FFmpeg when available, or blockers and replacement contract tests are documented.
- `PHASE-RESULT.md` records passed/failed/not-run commands.

## Must-exist checklist
- [ ] `src/platform/ProcessRunner.*` wraps process execution with timeout, exit code, stdout/stderr capture, and typed errors.
- [ ] `src/audio/AudioProbe.*`, `AudioExtractor.*`, and `AudioNormalizer.*` exist.
- [ ] Audio normalizer targets WAV, PCM signed 16-bit little-endian, mono, 16000 Hz.
- [ ] Tests verify FFmpeg argument construction without shell-string injection.
- [ ] Integration test uses a tiny media fixture when FFmpeg is available.

## Must-not-exist checklist
- [ ] No direct FFmpeg command strings in UI, app runtime, transcription, or tests outside audio/platform boundaries.
- [ ] No shell command construction from untrusted input.
- [ ] No tests requiring large media files.
- [ ] No hidden network calls or online codecs.
- [ ] No temporary file behavior without cleanup or documented keep-temp setting.

## Test plan
### Positive
- [ ] Valid WAV or media fixture normalizes to the required WAV shape.
- [ ] ProcessRunner captures exit code and output.
- [ ] AudioNormalizer creates job-specific temp output paths.

### Negative
- [ ] Missing input file returns `FileNotFound`.
- [ ] FFmpeg missing returns a clear typed error.
- [ ] FFmpeg non-zero exit maps to `AudioNormalizationFailed` with technical details.

## Test tree integration
- Trunk touch point: User selected media -> audio is probed -> normalized WAV is ready for transcription.
- New branches added: ProcessRunner contract, FFmpeg argument construction, missing file, ffmpeg failure, temp cleanup.

## Quality gates for this phase
- Follow `QUALITY-GATES.md`.
- Follow `LANGUAGE-QUALITY-GATE.md`.
- Run or document these commands as applicable:
- `cmake --preset windows-msvc-debug`
- `cmake --build --preset debug`
- `ctest --test-dir build/windows-msvc-debug --output-on-failure`
- `ffmpeg -version`
- `clang-format --dry-run --Werror <changed C++ files>`
- Document coverage evidence or why coverage tooling is unavailable.
- Keep business/workflow decisions out of UI, database, external tool, and engine adapter code.
- Create `PHASE-RESULT.md` before sending the final message.

## Next phase seed
Phase 05 consumes normalized WAV through the whisper engine boundary and pipeline.
