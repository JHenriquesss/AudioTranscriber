# Phase 02: Transcript Model And Exporters

## Scope
Implement the transcript data model and deterministic TXT, SRT, VTT, and JSON exporters. This phase proves output correctness through golden tests before any real transcription engine exists.

## Entry condition
- Phase 01 foundation exists and its build/test commands pass or blockers are documented.
- Shared result/error primitives are available.

## Exit condition
- Transcript model and exporters generate stable outputs for fixed fixtures.
- Golden tests cover happy paths and malformed transcript boundaries.
- `PHASE-RESULT.md` includes exact command evidence.

## Must-exist checklist
- [ ] `src/transcription/Transcript.hpp` defines `TranscriptDocument`, `TranscriptSegment`, and `TranscriptWord`.
- [ ] `src/export/TxtExporter.*`, `SrtExporter.*`, `VttExporter.*`, `JsonExporter.*`, and `ExportService.*` exist.
- [ ] `tests/fixtures/transcript-sample.json`, `expected.txt`, `expected.srt`, and `expected.vtt` exist.
- [ ] Unit/golden tests verify deterministic TXT, SRT, VTT, and JSON output.
- [ ] Timestamp formatting is tested at zero, sub-second, minute, and hour boundaries.

## Must-not-exist checklist
- [ ] No exporter depends on UI, storage, FFmpeg, or whisper.cpp.
- [ ] No hand-waved JSON contract with untested optional fields.
- [ ] No nondeterministic ordering, local timezone dependency, or current-time dependency in exporter tests.
- [ ] No business rules hidden in file dialogs or UI components.
- [ ] No generated golden files accepted without review notes in `PHASE-RESULT.md`.

## Test plan
### Positive
- [ ] A sample transcript exports to exact TXT/SRT/VTT/JSON golden files.
- [ ] Word metadata is preserved in JSON.
- [ ] ExportService writes only requested formats.

### Negative
- [ ] Invalid segment timing returns a typed export error.
- [ ] Empty transcript exports predictable empty output or typed error, as documented.
- [ ] Unwritable output path maps to `ExportFailed`.

## Test tree integration
- Trunk touch point: User transcript appears -> user saves/export files -> files match expected contracts.
- New branches added: TXT golden, SRT timestamp, VTT timestamp, JSON roundtrip, ExportService failure path.

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
Phase 03 adds app runtime, settings, job lifecycle, logging, and cancellation around these outputs.
