# Offline Transcription Desktop Architecture

**Version:** 1.0  
**Target platform:** Windows PC first  
**Primary languages:** Portuguese and English  
**Execution model:** Fully offline desktop executable  
**Recommended stack:** C++20, Qt 6 Widgets, whisper.cpp, SQLite, FFmpeg, CMake

---

## 1. Product Goal

Build a desktop application that transcribes audio and video files in Portuguese and English, fully offline, as a direct executable for regular PC users.

The product must feel like a normal desktop application: the user installs it, opens it, selects a media file, chooses a language/model, starts transcription, edits the result if needed, and exports to common formats.

The application must not require Python, cloud APIs, accounts, browsers, terminals, Docker, or an internet connection during normal use.

---

## 2. Architecture Philosophy

This project should be built as a **feature-oriented modular desktop application**.

It should not use traditional Clean Architecture, domain/use-case/entity folders, or exaggerated abstractions that make the code harder to navigate. The architecture should be pragmatic, explicit, testable, and easy for both humans and AI coding assistants to understand.

### Core principles

1. The main product workflow drives the architecture.
2. Modules are organized by real product capability, not academic layers.
3. The UI never performs transcription directly.
4. `whisper.cpp` is isolated behind a narrow engine boundary.
5. Audio preparation is explicit and testable.
6. Exporters only depend on the transcript data model.
7. Long-running work is cancellable and observable.
8. Logs and error codes are first-class product features.
9. File names and module names must be obvious.
10. The repository must be friendly to AI agents and future maintainers.

### What this architecture avoids

Avoid this:

```text
src/
  domain/
  application/
  infrastructure/
  usecases/
  entities/
  repositories/
  services/
```

Prefer this:

```text
src/
  app/
  audio/
  transcription/
  whisper_engine/
  export/
  storage/
  platform/
  shared/
```

The second structure tells a developer or AI assistant where to work immediately.

---

## 3. Technology Decisions

### Recommended stack

```text
C++20
Qt 6 Widgets
whisper.cpp
SQLite
FFmpeg as bundled external executable
CMake Presets
Inno Setup or portable ZIP packaging
```

### Why C++20

C++ is the best fit for this product because the final application must be a direct desktop executable, run offline, integrate with native desktop UI, and call a local speech recognition engine efficiently.

### Why Qt 6 Widgets

Qt Widgets is stable, mature, and ideal for a practical desktop application. It is less visually flashy than QML but faster to ship, easier to debug, and very suitable for tools such as transcription apps.

Qt/QML can be considered later if the product needs a more animated or premium visual interface.

### Why whisper.cpp

`whisper.cpp` is a native C/C++ implementation of Whisper suitable for offline transcription. It avoids Python at runtime and integrates well into a C++ desktop application.

### Why SQLite

SQLite is ideal for local history, settings, model registry, job metadata, and transcript references. It requires no server and works well inside a desktop application.

### Why FFmpeg as an external tool

FFmpeg supports a wide range of media formats. Bundling it as an external executable is simpler and safer than linking directly against multimedia libraries in the first version.

The application should call FFmpeg through a controlled `ProcessRunner` wrapper and treat FFmpeg as an implementation detail of the audio module.

---

## 4. Distribution Model

The app should be distributed in two formats:

```text
1. Windows installer
   TranscriberSetup.exe

2. Portable ZIP
   OfflineTranscriber.zip
```

A realistic portable package should look like this:

```text
OfflineTranscriber/
├─ OfflineTranscriber.exe
├─ models/
│  ├─ ggml-small.bin
│  ├─ ggml-medium.bin
│  └─ ggml-large-v3-turbo.bin
├─ tools/
│  └─ ffmpeg.exe
├─ data/
│  └─ app.db
├─ logs/
├─ exports/
├─ Qt6Core.dll
├─ Qt6Gui.dll
├─ Qt6Widgets.dll
└─ platforms/
   └─ qwindows.dll
```

Do not try to embed large AI model files inside a single `.exe`. Model files should remain separate and visible under `models/`.

The user experience can still be simple: install, open, transcribe.

---

## 5. High-Level Architecture

```text
┌─────────────────────────────────────────┐
│ Desktop UI                              │
│ Qt Widgets                              │
└─────────────────────┬───────────────────┘
                      │
┌─────────────────────▼───────────────────┐
│ App Runtime                              │
│ app state, jobs, settings, events        │
└─────────────────────┬───────────────────┘
                      │
┌─────────────────────▼───────────────────┐
│ Transcription Pipeline                   │
│ media → audio → engine → transcript      │
└──────┬──────────────┬──────────────┬─────┘
       │              │              │
┌──────▼──────┐ ┌─────▼───────┐ ┌────▼─────┐
│ Audio       │ │ Whisper     │ │ Export   │
│ preparation │ │ engine      │ │ formats  │
└──────┬──────┘ └─────┬───────┘ └────┬─────┘
       │              │              │
┌──────▼──────────────▼──────────────▼─────┐
│ Storage, Files, Logs, Models             │
│ SQLite, model files, exports, temp files │
└──────────────────────────────────────────┘
```

This is a modular monolith. It is one application, not a distributed system.

---

## 6. Repository Layout

```text
offline-transcriber/
├─ CMakeLists.txt
├─ CMakePresets.json
├─ README.md
├─ AGENTS.md
├─ LICENSES.md
│
├─ docs/
│  ├─ architecture.md
│  ├─ ai-map.md
│  ├─ build-windows.md
│  ├─ packaging.md
│  ├─ transcription-pipeline.md
│  ├─ data-formats.md
│  ├─ error-codes.md
│  └─ decisions/
│     ├─ 0001-use-qt-widgets.md
│     ├─ 0002-use-whisper-cpp.md
│     ├─ 0003-use-sqlite.md
│     └─ 0004-use-portable-folder-distribution.md
│
├─ apps/
│  └─ desktop/
│     ├─ main.cpp
│     ├─ MainWindow.hpp
│     ├─ MainWindow.cpp
│     ├─ widgets/
│     │  ├─ TranscriptionPanel.hpp
│     │  ├─ TranscriptionPanel.cpp
│     │  ├─ JobListWidget.hpp
│     │  ├─ JobListWidget.cpp
│     │  ├─ TranscriptEditor.hpp
│     │  └─ TranscriptEditor.cpp
│     ├─ dialogs/
│     │  ├─ SettingsDialog.hpp
│     │  ├─ SettingsDialog.cpp
│     │  ├─ ExportDialog.hpp
│     │  ├─ ExportDialog.cpp
│     │  ├─ ModelManagerDialog.hpp
│     │  └─ ModelManagerDialog.cpp
│     └─ resources/
│
├─ src/
│  ├─ app/
│  │  ├─ AppContext.hpp
│  │  ├─ AppContext.cpp
│  │  ├─ AppPaths.hpp
│  │  ├─ AppPaths.cpp
│  │  ├─ AppSettings.hpp
│  │  ├─ AppSettings.cpp
│  │  ├─ JobManager.hpp
│  │  ├─ JobManager.cpp
│  │  ├─ TranscriptionJob.hpp
│  │  └─ TranscriptionJob.cpp
│  │
│  ├─ audio/
│  │  ├─ AudioProbe.hpp
│  │  ├─ AudioProbe.cpp
│  │  ├─ AudioExtractor.hpp
│  │  ├─ AudioExtractor.cpp
│  │  ├─ AudioNormalizer.hpp
│  │  └─ AudioNormalizer.cpp
│  │
│  ├─ transcription/
│  │  ├─ Transcript.hpp
│  │  ├─ Transcript.cpp
│  │  ├─ TranscriptionOptions.hpp
│  │  ├─ TranscriptionResult.hpp
│  │  ├─ TranscriptionPipeline.hpp
│  │  ├─ TranscriptionPipeline.cpp
│  │  ├─ TranscriptPostProcessor.hpp
│  │  └─ TranscriptPostProcessor.cpp
│  │
│  ├─ whisper_engine/
│  │  ├─ WhisperEngine.hpp
│  │  ├─ WhisperEngine.cpp
│  │  ├─ WhisperModel.hpp
│  │  ├─ WhisperModel.cpp
│  │  ├─ WhisperOptionsMapper.hpp
│  │  └─ WhisperOptionsMapper.cpp
│  │
│  ├─ export/
│  │  ├─ TxtExporter.hpp
│  │  ├─ TxtExporter.cpp
│  │  ├─ SrtExporter.hpp
│  │  ├─ SrtExporter.cpp
│  │  ├─ VttExporter.hpp
│  │  ├─ VttExporter.cpp
│  │  ├─ JsonExporter.hpp
│  │  ├─ JsonExporter.cpp
│  │  ├─ ExportService.hpp
│  │  └─ ExportService.cpp
│  │
│  ├─ storage/
│  │  ├─ Database.hpp
│  │  ├─ Database.cpp
│  │  ├─ JobRepository.hpp
│  │  ├─ JobRepository.cpp
│  │  ├─ TranscriptRepository.hpp
│  │  ├─ TranscriptRepository.cpp
│  │  ├─ ModelRepository.hpp
│  │  └─ ModelRepository.cpp
│  │
│  ├─ platform/
│  │  ├─ FileDialogs.hpp
│  │  ├─ FileDialogs.cpp
│  │  ├─ ProcessRunner.hpp
│  │  ├─ ProcessRunner.cpp
│  │  ├─ SystemInfo.hpp
│  │  ├─ SystemInfo.cpp
│  │  ├─ CrashHandler.hpp
│  │  └─ CrashHandler.cpp
│  │
│  └─ shared/
│     ├─ Result.hpp
│     ├─ Error.hpp
│     ├─ Time.hpp
│     ├─ Logger.hpp
│     ├─ CancellationToken.hpp
│     └─ Json.hpp
│
├─ third_party/
│  └─ whisper.cpp/
│
├─ models/
│  └─ index.json
│
├─ tools/
│  ├─ package-windows.ps1
│  ├─ verify-release.ps1
│  └─ update-model-index.py
│
├─ tests/
│  ├─ unit/
│  ├─ integration/
│  └─ fixtures/
│     ├─ sample-pt-5s.wav
│     ├─ sample-en-5s.wav
│     ├─ expected.srt
│     └─ transcript-sample.json
│
└─ packaging/
   ├─ windows/
   │  ├─ installer.iss
   │  └─ app.manifest
   └─ licenses/
```

---

## 7. Module Responsibilities

### 7.1 `apps/desktop`

Responsible for the Qt desktop user interface.

It may:

```text
- Render windows, dialogs, buttons, forms, progress bars, and transcript views.
- Collect user input.
- Display job status and errors.
- Send user actions to AppContext or JobManager.
```

It must not:

```text
- Call whisper.cpp directly.
- Execute FFmpeg directly.
- Write export files directly.
- Contain business workflow logic.
- Own long-running worker threads directly.
```

### 7.2 `src/app`

Responsible for application runtime behavior.

It owns:

```text
- App startup and shutdown
- App paths
- Settings
- Job lifecycle
- Progress events
- Cancellation
- Coordination between UI, pipeline, and storage
```

Key classes:

```text
AppContext
AppPaths
AppSettings
JobManager
TranscriptionJob
```

### 7.3 `src/audio`

Responsible for turning any supported media input into a clean WAV file suitable for transcription.

It owns:

```text
- Media probing
- Audio extraction
- Audio normalization
- FFmpeg command construction
- Temporary audio file generation
```

Expected output:

```text
16 kHz
mono
WAV
PCM
```

### 7.4 `src/transcription`

Responsible for the product-level transcription workflow and transcript data model.

It owns:

```text
- TranscriptDocument
- TranscriptSegment
- TranscriptWord
- TranscriptionOptions
- TranscriptionPipeline
- TranscriptPostProcessor
```

This module should be testable without launching the Qt UI.

### 7.5 `src/whisper_engine`

Responsible for all direct integration with `whisper.cpp`.

It owns:

```text
- Model loading
- Model unloading
- Mapping application options to whisper.cpp options
- Running transcription
- Translating whisper.cpp output into TranscriptDocument
```

No other module should include whisper.cpp headers.

### 7.6 `src/export`

Responsible for converting a `TranscriptDocument` into external formats.

Supported formats for V1:

```text
TXT
SRT
VTT
JSON
```

Each exporter should be deterministic and easy to unit test.

### 7.7 `src/storage`

Responsible for local persistence.

It owns:

```text
- SQLite initialization
- Schema migrations
- Job history
- Transcript metadata
- Model registry
- App settings if stored in database
```

### 7.8 `src/platform`

Responsible for operating-system boundaries.

It owns:

```text
- Native file dialogs if not handled directly by Qt widgets
- Process execution
- System information
- Crash handling
- Platform-specific paths if needed
```

### 7.9 `src/shared`

Small shared primitives only.

Allowed:

```text
Result
Error
Logger
Time helpers
CancellationToken
Small JSON helpers
```

Avoid dumping unrelated utilities into this folder.

---

## 8. Core Runtime Flow

```text
1. User selects an audio or video file.
2. UI builds a TranscriptionJobRequest.
3. UI submits request to JobManager.
4. JobManager creates a TranscriptionJob.
5. JobManager persists initial job state.
6. Worker thread starts job execution.
7. AudioProbe validates the input media.
8. AudioExtractor extracts audio if needed.
9. AudioNormalizer generates WAV 16 kHz mono PCM.
10. WhisperEngine loads the selected model.
11. WhisperEngine transcribes the normalized audio.
12. TranscriptPostProcessor cleans and normalizes the result.
13. Transcript JSON is saved.
14. ExportService writes selected output formats.
15. Job state is marked Completed.
16. UI displays the transcript and export paths.
```

---

## 9. Job State Machine

```text
Queued
  ↓
ProbingMedia
  ↓
ExtractingAudio
  ↓
NormalizingAudio
  ↓
LoadingModel
  ↓
Transcribing
  ↓
PostProcessing
  ↓
SavingTranscript
  ↓
Exporting
  ↓
Completed
```

Any state can transition to:

```text
Failed
Cancelled
```

Recommended enum:

```cpp
#pragma once

#include <string>

namespace app {

enum class JobStatus {
    Queued,
    ProbingMedia,
    ExtractingAudio,
    NormalizingAudio,
    LoadingModel,
    Transcribing,
    PostProcessing,
    SavingTranscript,
    Exporting,
    Completed,
    Failed,
    Cancelled
};

std::string to_string(JobStatus status);

} // namespace app
```

---

## 10. Main Data Types

### 10.1 Transcription job request

```cpp
#pragma once

#include <filesystem>
#include <string>

namespace app {

struct TranscriptionJobRequest {
    std::filesystem::path inputFile;
    std::filesystem::path outputDirectory;

    std::string language; // "auto", "pt", "en"
    std::string modelId;  // "small", "medium", "large-v3-turbo"

    bool enableWordTimestamps = true;
    bool exportTxt = true;
    bool exportSrt = true;
    bool exportVtt = true;
    bool exportJson = true;
};

} // namespace app
```

### 10.2 Progress event

```cpp
#pragma once

#include <string>
#include "JobStatus.hpp"

namespace app {

struct TranscriptionJobProgress {
    std::string jobId;
    JobStatus status;
    double progress01 = 0.0;
    std::string message;
};

} // namespace app
```

### 10.3 Transcript model

```cpp
#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace transcription {

struct TranscriptWord {
    std::string text;
    int64_t startMs = 0;
    int64_t endMs = 0;
    float probability = 0.0f;
};

struct TranscriptSegment {
    int index = 0;
    int64_t startMs = 0;
    int64_t endMs = 0;
    std::string text;
    std::vector<TranscriptWord> words;
};

struct TranscriptDocument {
    std::string id;
    std::filesystem::path sourceFile;
    std::string detectedLanguage;
    std::string requestedLanguage;
    std::string modelId;
    int64_t durationMs = 0;
    std::string createdAtIso;
    std::vector<TranscriptSegment> segments;
};

} // namespace transcription
```

---

## 11. Transcription Pipeline

The pipeline is the main product workflow.

```text
input media
    ↓
AudioProbe
    ↓
AudioExtractor
    ↓
AudioNormalizer
    ↓
WhisperEngine
    ↓
TranscriptPostProcessor
    ↓
Storage
    ↓
ExportService
```

Recommended interface:

```cpp
#pragma once

#include <filesystem>
#include "shared/Result.hpp"
#include "shared/CancellationToken.hpp"
#include "transcription/Transcript.hpp"
#include "transcription/TranscriptionOptions.hpp"

namespace transcription {

class TranscriptionPipeline {
public:
    Result<TranscriptDocument> run(
        const std::filesystem::path& inputFile,
        const TranscriptionOptions& options,
        CancellationToken& cancellation
    );
};

} // namespace transcription
```

The pipeline should report progress through callbacks or application events, but the pipeline itself should not know about widgets, dialogs, or windows.

---

## 12. Whisper Engine Boundary

`whisper.cpp` must be isolated inside `src/whisper_engine`.

Recommended public interface:

```cpp
#pragma once

#include <filesystem>
#include <memory>
#include "shared/Result.hpp"
#include "shared/CancellationToken.hpp"
#include "transcription/Transcript.hpp"
#include "transcription/TranscriptionOptions.hpp"

namespace whisper_engine {

struct WhisperEngineConfig {
    int threadCount = 4;
    bool useGpu = false;
};

class WhisperEngine {
public:
    explicit WhisperEngine(const WhisperEngineConfig& config);
    ~WhisperEngine();

    Result<void> loadModel(const std::filesystem::path& modelPath);

    Result<transcription::TranscriptDocument> transcribe(
        const std::filesystem::path& wavFile,
        const transcription::TranscriptionOptions& options,
        CancellationToken& cancellation
    );

    void unloadModel();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace whisper_engine
```

Use the PIMPL pattern here so whisper headers do not leak into the rest of the codebase.

This keeps builds faster, improves isolation, and prevents random modules from depending on the transcription engine internals.

---

## 13. Audio Subsystem

The audio module prepares media for transcription.

### Responsibilities

```text
- Validate input file existence.
- Detect container and audio stream information.
- Extract audio from video files.
- Convert audio to a stable transcription format.
- Create job-specific temporary files.
- Return typed errors when conversion fails.
```

### Normalization target

```text
Format: WAV
Codec: PCM signed 16-bit little-endian
Channels: 1
Sample rate: 16000 Hz
```

### FFmpeg command shape

```text
ffmpeg -y -i input.ext -vn -ac 1 -ar 16000 -c:a pcm_s16le output.wav
```

The actual command construction should live inside `AudioNormalizer` or a dedicated helper owned by the audio module.

Never construct FFmpeg command strings directly in the UI.

---

## 14. Export System

All exporters receive a `TranscriptDocument` and write a file.

```cpp
class SrtExporter {
public:
    Result<void> exportToFile(
        const transcription::TranscriptDocument& transcript,
        const std::filesystem::path& outputFile
    );
};
```

### Required formats

#### TXT

Plain text with paragraphs or one segment per line.

#### SRT

Subtitle format with numbered blocks and timestamps.

#### VTT

Web subtitle format.

#### JSON

Canonical internal transcript format. This should be the most complete format and should preserve segment and word-level metadata.

Example JSON:

```json
{
  "id": "job_2026_06_17_001",
  "source_file": "meeting.mp3",
  "requested_language": "auto",
  "detected_language": "pt",
  "model": "medium",
  "duration_ms": 183000,
  "created_at": "2026-06-17T14:22:31Z",
  "segments": [
    {
      "index": 1,
      "start_ms": 0,
      "end_ms": 4200,
      "text": "Hello, this is a test.",
      "words": [
        {
          "text": "Hello",
          "start_ms": 0,
          "end_ms": 500,
          "probability": 0.91
        }
      ]
    }
  ]
}
```

JSON should be treated as the internal long-term representation because it is excellent for debugging, tests, reprocessing, and AI-assisted workflows.

---

## 15. Storage Design

Use SQLite for local persistence.

### Initial schema

```sql
CREATE TABLE app_settings (
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL
);

CREATE TABLE models (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    path TEXT NOT NULL,
    size_bytes INTEGER,
    installed_at TEXT NOT NULL
);

CREATE TABLE jobs (
    id TEXT PRIMARY KEY,
    source_file TEXT NOT NULL,
    status TEXT NOT NULL,
    model_id TEXT NOT NULL,
    requested_language TEXT NOT NULL,
    detected_language TEXT,
    created_at TEXT NOT NULL,
    completed_at TEXT,
    error_code TEXT,
    error_message TEXT
);

CREATE TABLE transcripts (
    id TEXT PRIMARY KEY,
    job_id TEXT NOT NULL,
    json_path TEXT NOT NULL,
    txt_path TEXT,
    srt_path TEXT,
    vtt_path TEXT,
    created_at TEXT NOT NULL,
    FOREIGN KEY(job_id) REFERENCES jobs(id)
);
```

### Recommended database behavior

```text
- Create database on first launch.
- Run migrations at startup.
- Use transactions for job completion.
- Store transcript body in JSON files, not necessarily inside SQLite.
- Store metadata and paths in SQLite.
```

This keeps the database small and makes transcripts easy to inspect manually.

---

## 16. Settings

Settings should be stored in a human-readable JSON file for portability, or in SQLite if centralization is preferred.

Recommended V1 file:

```json
{
  "default_language": "auto",
  "default_model": "small",
  "output_directory": "exports",
  "keep_temp_files": false,
  "max_parallel_jobs": 1,
  "enable_word_timestamps": true,
  "theme": "system"
}
```

### Portable mode

```text
./data/settings.json
```

### Installed mode

```text
%APPDATA%/OfflineTranscriber/settings.json
```

The application should detect portable mode by checking for a local `data/` directory next to the executable.

---

## 17. Model Management

Models should live under:

```text
models/
```

Model registry file:

```json
{
  "models": [
    {
      "id": "small",
      "name": "Fast",
      "path": "ggml-small.bin",
      "recommended_for": ["basic-pc", "short-audio"],
      "languages": ["pt", "en", "auto"]
    },
    {
      "id": "medium",
      "name": "Balanced",
      "path": "ggml-medium.bin",
      "recommended_for": ["general-use"],
      "languages": ["pt", "en", "auto"]
    },
    {
      "id": "large-v3-turbo",
      "name": "Best Quality",
      "path": "ggml-large-v3-turbo.bin",
      "recommended_for": ["quality"],
      "languages": ["pt", "en", "auto"]
    }
  ]
}
```

### V1 recommendation

Bundle one default model, preferably `small`, to keep the installer manageable.

Allow advanced users to place additional model files inside `models/` and refresh the model list from the UI.

The product can support a future online model downloader, but the core product must not depend on internet access.

---

## 18. Threading Model

Transcription must never run on the UI thread.

Recommended model:

```text
UI thread
  - windows
  - progress display
  - transcript editor
  - user actions

Worker thread
  - audio probing
  - audio conversion
  - model loading
  - transcription
  - exports
```

Use Qt's threading tools at the app boundary:

```text
QThreadPool
QRunnable
signals/slots
QMetaObject::invokeMethod for UI-safe callbacks
```

Internal pipeline code should not depend directly on Qt widgets.

### Parallelism

V1 should default to:

```text
max_parallel_jobs = 1
```

Local transcription is CPU/GPU intensive. Running multiple jobs at once may degrade the user's system heavily. Batch processing can be implemented as a queue.

---

## 19. Cancellation

Every long-running operation should receive a `CancellationToken`.

```cpp
#pragma once

#include <atomic>

class CancellationToken {
public:
    void cancel() {
        cancelled_.store(true);
    }

    bool isCancelled() const {
        return cancelled_.load();
    }

private:
    std::atomic<bool> cancelled_ = false;
};
```

Cancellation checkpoints should exist between major pipeline steps:

```text
before probing
before extraction
before normalization
before model loading
inside transcription callback if supported
before exporting
```

If transcription cannot stop immediately, the UI should show:

```text
Cancelling after current segment...
```

---

## 20. Error Handling

Use typed errors. Do not spread raw exceptions across the application.

```cpp
#pragma once

#include <string>

namespace shared {

enum class ErrorCode {
    Unknown,
    FileNotFound,
    UnsupportedMediaFormat,
    AudioProbeFailed,
    AudioExtractionFailed,
    AudioNormalizationFailed,
    ModelNotFound,
    ModelLoadFailed,
    TranscriptionFailed,
    ExportFailed,
    DatabaseError,
    Cancelled
};

struct AppError {
    ErrorCode code = ErrorCode::Unknown;
    std::string message;
    std::string technicalDetails;
};

} // namespace shared
```

Recommended `Result<T>`:

```cpp
#pragma once

#include "Error.hpp"

namespace shared {

template <typename T>
struct Result {
    bool ok = false;
    T value{};
    AppError error{};

    static Result<T> success(T v) {
        Result<T> r;
        r.ok = true;
        r.value = std::move(v);
        return r;
    }

    static Result<T> failure(AppError e) {
        Result<T> r;
        r.ok = false;
        r.error = std::move(e);
        return r;
    }
};

} // namespace shared
```

For `Result<void>`, provide a specialization or use a small expected-like implementation.

### User-facing error rule

Each error should have:

```text
- Short message for the user
- Technical details for logs
- Error code for support/debugging
```

Example:

```text
User message:
The selected file could not be converted to audio.

Technical details:
FFmpeg exited with code 1 while converting input.mp4 to WAV.

Error code:
AudioNormalizationFailed
```

---

## 21. Logging

Logs are part of the product.

Recommended folder:

```text
logs/
├─ app-2026-06-17.log
├─ crash-2026-06-17.log
└─ transcription-job-abc123.log
```

Recommended events:

```text
app_started
app_shutdown
settings_loaded
model_index_loaded
model_loaded
job_created
audio_probe_started
audio_probe_completed
audio_extraction_started
audio_extraction_failed
audio_normalization_started
audio_normalization_completed
transcription_started
transcription_completed
export_started
export_completed
job_completed
job_failed
job_cancelled
```

Recommended structured log shape:

```json
{
  "timestamp": "2026-06-17T14:22:31Z",
  "level": "info",
  "event": "transcription_started",
  "job_id": "job_abc123",
  "model": "small",
  "language": "pt",
  "source_file": "meeting.mp3"
}
```

Do not log transcript content by default. Transcripts may contain private data.

---

## 22. Privacy and Security

The product must be private by design.

### Rules

```text
- No audio leaves the machine.
- No transcript leaves the machine.
- No telemetry in V1.
- No cloud transcription fallback.
- No hidden network calls.
- Local logs must not include transcript content by default.
- Crash reports, if added later, must be opt-in.
```

### Temporary files

Temporary files should be stored under a job-specific directory:

```text
data/temp/job_<id>/
```

On successful completion, delete temporary files unless `keep_temp_files` is enabled.

On app startup, old temporary directories can be cleaned if they are older than a configurable threshold.

---

## 23. User Interface Design

V1 should be simple and operational.

```text
┌─────────────────────────────────────────────────────┐
│ Offline Transcriber                                 │
├─────────────────────────────────────────────────────┤
│ File:    [ Select audio or video file... ]          │
│ Language:[ Auto | Portuguese | English ]            │
│ Model:   [ Fast | Balanced | Best Quality ]         │
│                                                     │
│ [ Start Transcription ] [ Cancel ]                  │
│                                                     │
│ Progress: ███████████░░░░░ 72%                      │
│ Status: Transcribing audio...                       │
├─────────────────────────────────────────────────────┤
│ Transcript                                           │
│                                                     │
│ Editable transcript text appears here.              │
│                                                     │
├─────────────────────────────────────────────────────┤
│ [Save TXT] [Save SRT] [Save VTT] [Save JSON]        │
└─────────────────────────────────────────────────────┘
```

### Recommended screens

```text
Transcribe
History
Models
Settings
Logs
```

### V1 interface components

```text
MainWindow
TranscriptionPanel
TranscriptEditor
ExportDialog
SettingsDialog
ModelManagerDialog
```

Keep the UI boring and reliable before making it beautiful.

---

## 24. Build System

Use CMake Presets.

Example `CMakePresets.json`:

```json
{
  "version": 6,
  "configurePresets": [
    {
      "name": "windows-msvc-release",
      "displayName": "Windows MSVC Release",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/windows-msvc-release",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release",
        "CMAKE_CXX_STANDARD": "20"
      }
    },
    {
      "name": "windows-msvc-debug",
      "displayName": "Windows MSVC Debug",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/windows-msvc-debug",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CMAKE_CXX_STANDARD": "20"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "release",
      "configurePreset": "windows-msvc-release"
    },
    {
      "name": "debug",
      "configurePreset": "windows-msvc-debug"
    }
  ]
}
```

Build commands:

```powershell
cmake --preset windows-msvc-release
cmake --build --preset release
```

---

## 25. Packaging Pipeline

Release process:

```text
1. Configure Release build.
2. Build executable.
3. Run tests.
4. Copy OfflineTranscriber.exe to dist folder.
5. Run Qt deployment tool.
6. Copy models folder.
7. Copy FFmpeg executable.
8. Copy licenses.
9. Create portable ZIP.
10. Create installer.
11. Test on a clean Windows machine.
```

Example PowerShell script:

```powershell
cmake --preset windows-msvc-release
cmake --build --preset release

New-Item -ItemType Directory -Force dist\OfflineTranscriber

Copy-Item build\windows-msvc-release\apps\desktop\OfflineTranscriber.exe dist\OfflineTranscriber\

windeployqt dist\OfflineTranscriber\OfflineTranscriber.exe

Copy-Item models dist\OfflineTranscriber\models -Recurse -Force
Copy-Item tools dist\OfflineTranscriber\tools -Recurse -Force
Copy-Item LICENSES.md dist\OfflineTranscriber\
```

---

## 26. Testing Strategy

Do not rely on manual UI testing.

### Unit tests

Prioritize:

```text
SrtExporter timestamp formatting
VttExporter timestamp formatting
JsonExporter roundtrip
TranscriptPostProcessor text cleanup
AudioNormalizer command construction
AppSettings load/save
Job state transitions
Error mapping
```

### Integration tests

Prioritize:

```text
TranscriptionPipeline with a short WAV fixture
Audio conversion with a tiny MP3 fixture
Database schema creation
JobRepository create/update/read
ExportService writing all formats
```

### Test fixtures

```text
tests/fixtures/
├─ sample-pt-5s.wav
├─ sample-en-5s.wav
├─ sample-short.mp3
├─ transcript-sample.json
├─ expected.srt
└─ expected.vtt
```

Keep fixtures small so tests remain fast.

---

## 27. AI-Friendly Repository Rules

This project must be easy for AI coding tools to modify safely.

### Required files

```text
AGENTS.md
docs/ai-map.md
docs/architecture.md
docs/transcription-pipeline.md
docs/data-formats.md
docs/error-codes.md
docs/build-windows.md
```

### `AGENTS.md` should contain

```md
# Agent Instructions

## Project goal
Build an offline desktop transcription app for Portuguese and English.

## Stack
- C++20
- Qt 6 Widgets
- whisper.cpp
- SQLite
- FFmpeg
- CMake

## Main rules
- Do not call whisper.cpp outside src/whisper_engine.
- Do not run transcription on the UI thread.
- Do not put business workflow logic inside widgets.
- Do not add generic Clean Architecture folders.
- Keep modules named by product capability.
- Prefer explicit names over abstract names.
- Add tests for exporters, settings, job state, and error handling.

## Where to edit
- UI: apps/desktop
- Job orchestration: src/app
- Audio conversion: src/audio
- Pipeline: src/transcription
- Whisper integration: src/whisper_engine
- Export formats: src/export
- Database: src/storage
- Platform utilities: src/platform
```

### `docs/ai-map.md` should contain

```md
# AI Map

## Product
Offline desktop application for transcribing audio and video files in Portuguese and English.

## Main workflow
User selects file → audio is normalized → model transcribes → transcript is saved → exports are generated.

## Important files
- apps/desktop/MainWindow.cpp: main window
- src/app/JobManager.cpp: job lifecycle
- src/transcription/TranscriptionPipeline.cpp: transcription workflow
- src/whisper_engine/WhisperEngine.cpp: whisper.cpp integration
- src/audio/AudioNormalizer.cpp: FFmpeg conversion
- src/export/SrtExporter.cpp: SRT export
- src/storage/Database.cpp: SQLite setup

## Forbidden patterns
- Do not create domain/application/infrastructure folder structures.
- Do not call FFmpeg from UI classes.
- Do not include whisper.cpp headers outside src/whisper_engine.
- Do not add generic Utils folders for unrelated code.
```

---

## 28. Naming Rules

Prefer explicit product language.

Good names:

```text
TranscriptionPipeline
AudioNormalizer
WhisperEngine
TranscriptDocument
SrtExporter
JobRepository
ModelManagerDialog
```

Avoid vague names:

```text
Core
Manager
Handler
Processor
Service
Helper
Utils
Base
Common
```

`Manager` is allowed only when a class truly owns ongoing state or lifecycle, such as `JobManager`.

---

## 29. File Size Rules

Recommended limits:

```text
Header files: up to 150 lines when possible
Implementation files: up to 300 lines when possible
Functions: up to 40 lines when possible
Classes: one operational responsibility
```

These are guidelines, not dogma. The goal is to keep files readable by humans and AI tools.

---

## 30. Comments Rule

Do not comment obvious code.

Bad:

```cpp
// Increment index
i++;
```

Good:

```cpp
// whisper.cpp expects normalized 16 kHz mono WAV input here.
// Keep this conversion stable because segment timestamps depend on it.
```

Comments should explain why a decision exists, not what the syntax already says.

---

## 31. Architecture Decision Records

Keep small ADR files under:

```text
docs/decisions/
```

Example:

```md
# 0002 - Use whisper.cpp for offline transcription

## Status
Accepted

## Context
The product must run fully offline on Windows PCs and must not require Python at runtime.

## Decision
Use whisper.cpp as the local transcription engine.

## Consequences
- Models are distributed as local files.
- The app must manage model paths.
- Transcription runs in worker threads.
- whisper.cpp must be isolated behind src/whisper_engine.
```

ADRs prevent future contributors or AI tools from repeatedly reopening settled decisions.

---

## 32. Licensing Checklist

Before public release, verify licenses for:

```text
Qt
whisper.cpp
FFmpeg build
SQLite wrapper if used
JSON library if used
Logging library if used
Installer tool
```

Important rule:

```text
Do not ship third-party binaries without storing their licenses under packaging/licenses/ and summarizing them in LICENSES.md.
```

For FFmpeg, verify whether the bundled build is LGPL-compatible or includes GPL components.

For Qt, decide early whether the product will use an open-source license-compatible distribution model or a commercial license.

---

## 33. Minimum Viable Product

MVP scope:

```text
- Windows executable
- Select WAV file
- Select language: Auto, Portuguese, English
- Select model from local models folder
- Transcribe using whisper.cpp
- Display transcript in editable text area
- Export TXT
- Write logs
- Basic error messages
```

Do not include in MVP:

```text
- Diarization
- Cloud sync
- User accounts
- Online model downloader
- Complex editor
- Batch processing
- Speaker labels
- Real-time microphone mode
```

The MVP should prove the core offline transcription workflow first.

---

## 34. Product Roadmap

### Phase 1 — Core transcription

```text
- Build Qt desktop shell
- Load local model
- Transcribe WAV
- Display transcript
- Export TXT
```

### Phase 2 — Real media support

```text
- Add MP3, M4A, MP4 support through FFmpeg
- Add SRT, VTT, JSON export
- Add progress updates
- Add cancellation
- Add structured logs
```

### Phase 3 — Usable desktop product

```text
- Add SQLite history
- Add settings screen
- Add model manager
- Add portable ZIP packaging
- Add Windows installer
- Add clean Windows machine testing
```

### Phase 4 — Quality improvements

```text
- Word-level timestamps
- Transcript editor improvements
- Search in history
- Batch queue
- Better error recovery
- Crash handling
```

### Phase 5 — Advanced features

```text
- Optional diarization
- Optional GPU acceleration profiles
- Custom vocabulary hints if supported
- Microphone recording mode
- Project/session files
```

---

## 35. Definition of Done

A feature is done only when:

```text
- It works from the desktop UI.
- It does not block the UI thread.
- It has typed errors.
- It logs relevant events.
- It has at least basic tests if it is not purely visual.
- It is documented in the relevant docs file.
- It does not violate module boundaries.
- It works in a packaged build, not only inside the IDE.
```

For transcription features, also verify:

```text
- Portuguese sample works.
- English sample works.
- Cancellation works.
- Missing model error is clear.
- Invalid media file error is clear.
- Exported files are valid.
```

---

## 36. Final Architecture Summary

The final architecture is:

```text
Feature-Oriented Modular Desktop Architecture
```

The final stack is:

```text
C++20
Qt 6 Widgets
whisper.cpp
SQLite
FFmpeg external executable
CMake Presets
Inno Setup or portable ZIP
```

The main module map is:

```text
apps/desktop       → Qt interface
src/app            → app runtime, settings, jobs
src/audio          → media probing and audio normalization
src/transcription  → transcript model and pipeline
src/whisper_engine → isolated whisper.cpp integration
src/export         → TXT, SRT, VTT, JSON exporters
src/storage        → SQLite repositories and migrations
src/platform       → OS boundaries and process runner
src/shared         → small shared primitives
```

The most important boundary is:

```text
Only src/whisper_engine may talk directly to whisper.cpp.
```

The most important product rule is:

```text
The application must remain fully offline during normal use.
```

The most important engineering rule is:

```text
Build the codebase around real product workflows, not abstract architecture patterns.
```
