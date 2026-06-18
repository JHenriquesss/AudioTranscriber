# Transcription Pipeline

This document describes the planned end-to-end transcription workflow. Phase 01 implements only shared primitives that later pipeline stages will use.

## Planned workflow (future phases)

```text
User selects media file
        ↓
JobManager creates TranscriptionJob
        ↓
AudioProbe validates input
        ↓
AudioExtractor extracts audio (FFmpeg, when needed)
        ↓
AudioNormalizer produces 16 kHz mono PCM WAV
        ↓
WhisperEngine transcribes normalized audio
        ↓
TranscriptPostProcessor cleans text
        ↓
Storage saves transcript metadata and JSON path
        ↓
ExportService writes TXT/SRT/VTT/JSON
        ↓
UI shows transcript and export paths
```

## Job states (planned)

```text
Queued → ProbingMedia → ExtractingAudio → NormalizingAudio → LoadingModel
  → Transcribing → PostProcessing → SavingTranscript → Exporting → Completed
```

Any state may transition to `Failed` or `Cancelled`.

## Cancellation checkpoints (planned)

Long-running steps receive a `shared::CancellationToken` and check it between major steps:

- before probing
- before extraction
- before normalization
- before model loading
- inside transcription callbacks when supported
- before exporting

Phase 01 provides `CancellationToken` with deterministic `cancel()` / `isCancelled()` behavior tested in unit tests.

## Audio normalization target (planned)

```text
Format: WAV
Codec: PCM signed 16-bit little-endian
Channels: 1
Sample rate: 16000 Hz
```

## Module boundaries (planned)

| Step | Owning module |
|------|----------------|
| UI actions | `apps/desktop` |
| Job orchestration | `src/app` |
| Media probe/extract/normalize | `src/audio` |
| Pipeline coordination | `src/transcription` |
| whisper.cpp calls | `src/whisper_engine` only |
| Export formats | `src/export` |
| Persistence | `src/storage` |
| Process execution | `src/platform` |
| Shared types | `src/shared` |

## Phase 01 deliverables used by the pipeline

| Primitive | Pipeline use |
|-----------|--------------|
| `Result<T>` | Typed outcomes from every stage |
| `AppError` | User and log-facing failure details |
| `CancellationToken` | Cooperative cancellation between stages |
| `Time` helpers | Duration labels and ISO timestamps in exports/logs |

## Out of scope in phase 01

- `TranscriptionPipeline` class implementation
- FFmpeg command construction
- whisper.cpp integration
- Transcript data model
- Export writers
- SQLite job history

Phase 02 adds transcript data structures and deterministic exporters on top of this foundation.
