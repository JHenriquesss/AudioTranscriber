# Error Codes

Phase 01 defines typed error codes in `src/shared/Error.hpp`. User-facing modules map these codes to short messages and log technical details separately.

## ErrorCode values

| Code | String label | Intended use (future phases) |
|------|--------------|------------------------------|
| `Unknown` | `Unknown` | Unclassified failure |
| `FileNotFound` | `FileNotFound` | Missing input, model, or output path |
| `UnsupportedMediaFormat` | `UnsupportedMediaFormat` | Container or codec not supported |
| `AudioProbeFailed` | `AudioProbeFailed` | Media probe could not read input |
| `AudioExtractionFailed` | `AudioExtractionFailed` | FFmpeg extraction failed |
| `AudioNormalizationFailed` | `AudioNormalizationFailed` | WAV normalization failed |
| `ModelNotFound` | `ModelNotFound` | Requested model file missing |
| `ModelLoadFailed` | `ModelLoadFailed` | whisper.cpp model load failed |
| `TranscriptionFailed` | `TranscriptionFailed` | Engine returned failure |
| `ExportFailed` | `ExportFailed` | Exporter could not write output |
| `DatabaseError` | `DatabaseError` | SQLite operation failed |
| `Cancelled` | `Cancelled` | Operation cancelled by user |

## AppError shape

```cpp
struct AppError {
    ErrorCode code;
    std::string message;           // short user-facing text
    std::string technicalDetails;  // log/support detail, not shown by default in UI
};
```

## Result usage

Business and adapter code return `shared::Result<T>` instead of throwing for expected failures:

```cpp
shared::Result<int> loadCount();
shared::Result<void> validateInput();
```

## User-facing message rule

Each error shown in the UI should include:

1. A short message (`message`)
2. An error code label via `to_string(code)`
3. Technical details only in logs (`technicalDetails`)

Example:

```text
User message:
The selected file could not be converted to audio.

Technical details:
FFmpeg exited with code 1 while converting input.mp4.

Error code:
AudioNormalizationFailed
```

## Phase 01 coverage

Phase 01 tests verify that failure results preserve code and message fields. Full error mapping for FFmpeg, whisper.cpp, and SQLite arrives in later phases.
