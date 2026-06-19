#include "JobManager.hpp"

#include "JobHistoryCoordinator.hpp"
#include "JobRequestPaths.hpp"
#include "TranscriptionRequestValidation.hpp"

#include "audio/AudioNormalizer.hpp"
#include "audio/AudioProbe.hpp"
#include "transcription/TranscriptionOptions.hpp"
#include "transcription/TranscriptionPipeline.hpp"
#include "whisper_engine/WhisperEngine.hpp"

#include <algorithm>
#include <filesystem>
#include <functional>
#include <map>
#include <string_view>

namespace app {

namespace {

std::function<whisper_engine::WhisperEngine()> g_testWhisperEngineSupplier;

constexpr double kProgressStep = 0.1;

shared::AppError cancelledError() {
    return shared::AppError{
        shared::ErrorCode::Cancelled,
        "The job was cancelled.",
        "Cancellation requested by user.",
    };
}

const char *statusLogEvent(JobStatus status) {
    switch (status) {
    case JobStatus::ProbingMedia:
        return shared::log_events::audioProbeStarted;
    case JobStatus::ExtractingAudio:
        return shared::log_events::audioExtractionStarted;
    case JobStatus::NormalizingAudio:
        return shared::log_events::audioNormalizationStarted;
    case JobStatus::Transcribing:
        return shared::log_events::transcriptionStarted;
    case JobStatus::Exporting:
        return shared::log_events::exportStarted;
    default:
        return nullptr;
    }
}

const char *statusCompletionLogEvent(JobStatus status) {
    switch (status) {
    case JobStatus::ProbingMedia:
        return shared::log_events::audioProbeCompleted;
    case JobStatus::NormalizingAudio:
        return shared::log_events::audioNormalizationCompleted;
    case JobStatus::Transcribing:
        return shared::log_events::transcriptionCompleted;
    case JobStatus::Exporting:
        return shared::log_events::exportCompleted;
    default:
        return nullptr;
    }
}

} // namespace

JobManager::JobManager() = default;

JobManager::JobManager(JobHistoryCoordinator *history) : history_(history) {
    if (history_ == nullptr) {
        return;
    }

    const auto existingJobs = history_->jobs().listJobs();
    if (!existingJobs.ok) {
        return;
    }

    constexpr std::string_view kJobIdPrefix = "job_";
    int maxJobNumber = 0;
    for (const storage::JobRecord &record : existingJobs.value) {
        if (record.id.size() <= kJobIdPrefix.size() ||
            record.id.compare(0, kJobIdPrefix.size(), kJobIdPrefix) != 0) {
            continue;
        }

        try {
            const int jobNumber = std::stoi(record.id.substr(kJobIdPrefix.size()));
            maxJobNumber = std::max(maxJobNumber, jobNumber);
        } catch (const std::exception &) {
            continue;
        }
    }

    nextJobNumber_ = maxJobNumber + 1;
}

void JobManager::setTestWhisperEngineSupplier(
    std::function<whisper_engine::WhisperEngine()> supplier) {
    g_testWhisperEngineSupplier = std::move(supplier);
}

void JobManager::clearTestWhisperEngineSupplier() {
    g_testWhisperEngineSupplier = {};
}

whisper_engine::WhisperEngine JobManager::createWhisperEngine() {
    if (g_testWhisperEngineSupplier) {
        return g_testWhisperEngineSupplier();
    }
    return whisper_engine::WhisperEngine{};
}

shared::Result<std::string> JobManager::submitJob(const TranscriptionJobRequest &request) {
    const auto validation = validateTranscriptionJobRequest(request);
    if (!validation.ok) {
        return shared::Result<std::string>::failure(validation.error);
    }

    std::lock_guard lock(mutex_);
    const std::string jobId = "job_" + std::to_string(nextJobNumber_++);
    jobs_.emplace(jobId, TranscriptionJob(jobId, request));

    if (history_ != nullptr) {
        const auto persisted = history_->persistJobCreated(jobs_.at(jobId));
        if (!persisted.ok) {
            jobs_.erase(jobId);
            return shared::Result<std::string>::failure(persisted.error);
        }
    }

    return shared::Result<std::string>::success(jobId);
}

shared::Result<void> JobManager::cancelJob(const std::string &jobId) {
    std::lock_guard lock(mutex_);
    auto *job = findJob(jobId);
    if (job == nullptr) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::Unknown,
            "Job not found.",
            jobId,
        });
    }

    if (isTerminalStatus(job->status())) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::Unknown,
            "Job is already finished.",
            to_string(job->status()),
        });
    }

    const auto transitioned = job->transitionTo(JobStatus::Cancelled);
    if (!transitioned.ok) {
        return transitioned;
    }

    return shared::Result<void>::success();
}

std::optional<TranscriptionJob> JobManager::getJob(const std::string &jobId) const {
    std::lock_guard lock(mutex_);
    const auto *job = findJob(jobId);
    if (job == nullptr) {
        return std::nullopt;
    }
    return *job;
}

shared::Result<void> JobManager::runJobShell(const std::string &jobId,
                                             shared::CancellationToken &cancellation,
                                             ProgressCallback onProgress, LogCallback onLog) {
    TranscriptionJob *job = nullptr;
    {
        std::lock_guard lock(mutex_);
        job = findJob(jobId);
        if (job == nullptr) {
            return shared::Result<void>::failure(shared::AppError{
                shared::ErrorCode::Unknown,
                "Job not found.",
                jobId,
            });
        }
    }

    emitLog(onLog, shared::LogLevel::Info, shared::log_events::jobCreated, *job);

    double progress = 0.0;
    while (true) {
        if (cancellation.isCancelled()) {
            const auto transitioned = job->transitionTo(JobStatus::Cancelled);
            if (!transitioned.ok) {
                return transitioned;
            }
            emitLog(onLog, shared::LogLevel::Warning, shared::log_events::jobCancelled, *job);
            publishProgress(*job, onProgress, "Job cancelled.");
            return shared::Result<void>::failure(cancelledError());
        }

        const auto nextStatus = nextPipelineStatus(job->status());
        if (!nextStatus.has_value()) {
            break;
        }

        const auto transitioned = job->transitionTo(*nextStatus);
        if (!transitioned.ok) {
            return transitioned;
        }

        if (const char *startedEvent = statusLogEvent(*nextStatus)) {
            emitLog(onLog, shared::LogLevel::Info, startedEvent, *job);
        }

        if (*nextStatus == JobStatus::PostProcessing) {
            attachFakeTranscript(*job);
        }

        if (history_ != nullptr) {
            if (*nextStatus == JobStatus::SavingTranscript) {
                const auto persisted = history_->persistJobCompleted(*job);
                if (!persisted.ok) {
                    return persisted;
                }
            } else {
                const auto persisted = history_->persistJobUpdated(*job);
                if (!persisted.ok) {
                    return persisted;
                }
            }
        }

        progress = std::min(1.0, progress + kProgressStep);
        job->setProgress(progress, "Running " + to_string(*nextStatus) + " (shell).");
        publishProgress(*job, onProgress, job->progressHistory().back().message);

        if (const char *completedEvent = statusCompletionLogEvent(*nextStatus)) {
            emitLog(onLog, shared::LogLevel::Info, completedEvent, *job);
        }

        if (*nextStatus == JobStatus::Completed) {
            job->setProgress(1.0, "Job completed.");
            publishProgress(*job, onProgress, "Job completed.");
            emitLog(onLog, shared::LogLevel::Info, shared::log_events::jobCompleted, *job);
            break;
        }
    }

    return shared::Result<void>::success();
}

shared::Result<void> JobManager::runJobPipeline(const std::string &jobId, const AppPaths &paths,
                                                shared::CancellationToken &cancellation,
                                                ProgressCallback onProgress, LogCallback onLog) {
    std::optional<TranscriptionJobRequest> request;
    {
        std::lock_guard lock(mutex_);
        const auto *job = findJob(jobId);
        if (job == nullptr) {
            return shared::Result<void>::failure(shared::AppError{
                shared::ErrorCode::Unknown,
                "Job not found.",
                jobId,
            });
        }
        request = job->request();
        emitLog(onLog, shared::LogLevel::Info, shared::log_events::jobCreated, *job);
    }

    const auto ffmpegPath = std::filesystem::exists(paths.applicationRoot / "tools" / "ffmpeg.exe")
                                ? paths.applicationRoot / "tools" / "ffmpeg.exe"
                                : std::filesystem::path("ffmpeg");
    audio::AudioToolPaths toolPaths{ffmpegPath};

    if (auto result = transitionJob(jobId, JobStatus::ProbingMedia, 0.10, "Probing media.",
                                    onProgress);
        !result.ok) {
        return result;
    }
    emitLog(onLog, shared::LogLevel::Info, shared::log_events::audioProbeStarted,
            *getJob(jobId));

    audio::AudioProbe probe(toolPaths);
    const auto probeResult = probe.probe(request->inputFile);
    if (!probeResult.ok) {
        return failJobFromRunner(jobId, probeResult.error, onLog, onProgress);
    }
    emitLog(onLog, shared::LogLevel::Info, shared::log_events::audioProbeCompleted,
            *getJob(jobId));

    if (auto result = transitionJob(jobId, JobStatus::ExtractingAudio, 0.20,
                                    "Preparing audio extraction.", onProgress);
        !result.ok) {
        return result;
    }
    emitLog(onLog, shared::LogLevel::Info, shared::log_events::audioExtractionStarted,
            *getJob(jobId));

    if (auto result = transitionJob(jobId, JobStatus::NormalizingAudio, 0.35,
                                    "Normalizing audio.", onProgress);
        !result.ok) {
        return result;
    }
    emitLog(onLog, shared::LogLevel::Info, shared::log_events::audioNormalizationStarted,
            *getJob(jobId));

    audio::AudioWorkspaceOptions workspaceOptions;
    workspaceOptions.tempDirectory = paths.tempDirectory();
    workspaceOptions.keepTempFiles = false;
    audio::AudioNormalizer normalizer(toolPaths, workspaceOptions);
    const auto normalized = normalizer.normalize(request->inputFile, jobId);
    if (!normalized.ok) {
        return failJobFromRunner(jobId, normalized.error, onLog, onProgress);
    }
    emitLog(onLog, shared::LogLevel::Info, shared::log_events::audioNormalizationCompleted,
            *getJob(jobId));

    if (auto result = transitionJob(jobId, JobStatus::LoadingModel, 0.50, "Loading model.",
                                    onProgress);
        !result.ok) {
        return result;
    }

    if (auto result = transitionJob(jobId, JobStatus::Transcribing, 0.65, "Transcribing audio.",
                                    onProgress);
        !result.ok) {
        return result;
    }
    emitLog(onLog, shared::LogLevel::Info, shared::log_events::transcriptionStarted,
            *getJob(jobId));

    transcription::TranscriptionOptions options;
    options.jobId = jobId;
    options.normalizedAudioPath = normalized.value.outputPath;
    options.sourceFile = request->inputFile;
    options.language = request->language;
    options.modelId = request->modelId;
    options.modelsDirectory = paths.modelsDirectory();
    options.enableWordTimestamps = request->enableWordTimestamps;

    transcription::TranscriptionPipeline pipeline(createWhisperEngine());
    const auto transcribed = pipeline.run(options, cancellation);
    if (!transcribed.ok) {
        return failJobFromRunner(jobId, transcribed.error, onLog, onProgress);
    }
    emitLog(onLog, shared::LogLevel::Info, shared::log_events::transcriptionCompleted,
            *getJob(jobId));

    {
        std::lock_guard lock(mutex_);
        auto *job = findJob(jobId);
        if (job == nullptr) {
            return shared::Result<void>::failure(shared::AppError{
                shared::ErrorCode::Unknown,
                "Job not found.",
                jobId,
            });
        }
        job->setTranscript(std::move(transcribed.value.transcript));
    }

    if (auto result = transitionJob(jobId, JobStatus::PostProcessing, 0.80,
                                    "Post-processing transcript.", onProgress);
        !result.ok) {
        return result;
    }
    if (auto result = transitionJob(jobId, JobStatus::SavingTranscript, 0.88,
                                    "Saving transcript metadata.", onProgress);
        !result.ok) {
        return result;
    }
    if (auto result = persistJobSnapshot(jobId, true); !result.ok) {
        return failJobFromRunner(jobId, result.error, onLog, onProgress);
    }

    if (auto result = transitionJob(jobId, JobStatus::Exporting, 0.95,
                                    "Exporting transcript.", onProgress);
        !result.ok) {
        return result;
    }
    emitLog(onLog, shared::LogLevel::Info, shared::log_events::exportStarted, *getJob(jobId));
    if (auto result = exportJobResults(jobId); !result.ok) {
        return failJobFromRunner(jobId, result.error, onLog, onProgress);
    }
    emitLog(onLog, shared::LogLevel::Info, shared::log_events::exportCompleted, *getJob(jobId));

    if (auto result = transitionJob(jobId, JobStatus::Completed, 1.0, "Job completed.",
                                    onProgress);
        !result.ok) {
        return result;
    }
    emitLog(onLog, shared::LogLevel::Info, shared::log_events::jobCompleted, *getJob(jobId));
    return shared::Result<void>::success();
}

shared::Result<void> JobManager::failJob(const std::string &jobId, shared::AppError error) {
    std::lock_guard lock(mutex_);
    auto *job = findJob(jobId);
    if (job == nullptr) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::Unknown,
            "Job not found.",
            jobId,
        });
    }

    job->setError(std::move(error));

    if (history_ != nullptr) {
        return history_->persistJobUpdated(*job);
    }

    return shared::Result<void>::success();
}

shared::Result<void> JobManager::exportJobResults(const std::string &jobId) {
    std::lock_guard lock(mutex_);
    const auto *job = findJob(jobId);
    if (job == nullptr) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::Unknown,
            "Job not found.",
            jobId,
        });
    }

    const auto *transcript = job->transcript();
    if (transcript == nullptr) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::ExportFailed,
            "No transcript is available to export.",
            jobId,
        });
    }

    export_format::ExportRequest request;
    request.outputDirectory = job->request().outputDirectory;
    request.baseName = exportBaseNameForInputFile(job->request().inputFile);
    if (job->request().exportTxt) {
        request.formats.push_back(export_format::ExportFormat::Txt);
    }
    if (job->request().exportSrt) {
        request.formats.push_back(export_format::ExportFormat::Srt);
    }
    if (job->request().exportVtt) {
        request.formats.push_back(export_format::ExportFormat::Vtt);
    }
    if (job->request().exportJson) {
        request.formats.push_back(export_format::ExportFormat::Json);
    }

    if (request.formats.empty()) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::ExportFailed,
            "No export formats were selected.",
            jobId,
        });
    }

    const auto outputReady = ensureDirectoryExists(request.outputDirectory);
    if (!outputReady.ok) {
        return outputReady;
    }

    const auto exportResult = exportService_.exportTranscript(*transcript, request);
    if (!exportResult.ok) {
        return exportResult;
    }

    if (history_ != nullptr) {
        storage::TranscriptRecord exportPaths;
        exportPaths.jobId = jobId;
        if (job->request().exportTxt) {
            exportPaths.txtPath = request.outputDirectory / (request.baseName + ".txt");
        }
        if (job->request().exportSrt) {
            exportPaths.srtPath = request.outputDirectory / (request.baseName + ".srt");
        }
        if (job->request().exportVtt) {
            exportPaths.vttPath = request.outputDirectory / (request.baseName + ".vtt");
        }
        const auto persisted = history_->persistExportPaths(jobId, exportPaths);
        if (!persisted.ok) {
            return persisted;
        }
    }

    return shared::Result<void>::success();
}

void JobManager::attachFakeTranscript(TranscriptionJob &job) const {
    transcription::TranscriptDocument transcript;
    transcript.id = job.id();
    transcript.sourceFile = job.request().inputFile;
    transcript.detectedLanguage = job.request().language;
    transcript.requestedLanguage = job.request().language;
    transcript.modelId = job.request().modelId;
    transcript.durationMs = 1200;
    transcript.createdAtIso = "2026-01-01T00:00:00Z";
    transcript.segments.push_back(transcription::TranscriptSegment{
        0,
        0,
        1200,
        "Deterministic shell transcript for desktop smoke testing.",
        {},
    });
    job.setTranscript(std::move(transcript));
}

shared::Result<void> JobManager::transitionJob(const std::string &jobId, JobStatus status,
                                               double progress01, std::string message,
                                               ProgressCallback &onProgress) {
    std::lock_guard lock(mutex_);
    auto *job = findJob(jobId);
    if (job == nullptr) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::Unknown,
            "Job not found.",
            jobId,
        });
    }

    const auto transitioned = job->transitionTo(status);
    if (!transitioned.ok) {
        return transitioned;
    }
    if (history_ != nullptr) {
        const auto persisted = history_->persistJobUpdated(*job);
        if (!persisted.ok) {
            return persisted;
        }
    }

    job->setProgress(progress01, std::move(message));
    publishProgress(*job, onProgress, job->progressHistory().back().message);
    return shared::Result<void>::success();
}

shared::Result<void> JobManager::persistJobSnapshot(const std::string &jobId, bool completed) {
    if (history_ == nullptr) {
        return shared::Result<void>::success();
    }

    std::lock_guard lock(mutex_);
    const auto *job = findJob(jobId);
    if (job == nullptr) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::Unknown,
            "Job not found.",
            jobId,
        });
    }
    return completed ? history_->persistJobCompleted(*job) : history_->persistJobUpdated(*job);
}

shared::Result<void> JobManager::failJobFromRunner(const std::string &jobId,
                                                   shared::AppError error,
                                                   LogCallback &onLog,
                                                   ProgressCallback &onProgress) {
    std::lock_guard lock(mutex_);
    auto *job = findJob(jobId);
    if (job == nullptr) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::Unknown,
            "Job not found.",
            jobId,
        });
    }

    job->setError(error);
    if (history_ != nullptr) {
        const auto persisted = history_->persistJobUpdated(*job);
        if (!persisted.ok) {
            return persisted;
        }
    }
    emitLog(onLog, shared::LogLevel::Error, shared::log_events::jobFailed, *job,
            "error_code", shared::to_string(error.code));
    publishProgress(*job, onProgress, error.message);
    return shared::Result<void>::failure(std::move(error));
}

TranscriptionJob *JobManager::findJob(const std::string &jobId) {
    const auto iterator = jobs_.find(jobId);
    if (iterator == jobs_.end()) {
        return nullptr;
    }
    return &iterator->second;
}

const TranscriptionJob *JobManager::findJob(const std::string &jobId) const {
    const auto iterator = jobs_.find(jobId);
    if (iterator == jobs_.end()) {
        return nullptr;
    }
    return &iterator->second;
}

void JobManager::emitLog(LogCallback &onLog, shared::LogLevel level, const char *event,
                         const TranscriptionJob &job, std::string extraFieldKey,
                         std::string extraFieldValue) const {
    if (!onLog) {
        return;
    }

    std::map<std::string, std::string> fields{
        {"job_id", job.id()},
        {"model", job.request().modelId},
        {"language", job.request().language},
        {"source_file", job.request().inputFile.filename().string()},
    };

    if (!extraFieldKey.empty()) {
        fields.emplace(std::move(extraFieldKey), std::move(extraFieldValue));
    }

    onLog(shared::makeLogEntry(level, event, std::move(fields)));
}

void JobManager::publishProgress(TranscriptionJob &job, ProgressCallback &onProgress,
                                 std::string message) const {
    if (!onProgress) {
        return;
    }

    onProgress(TranscriptionJobProgress{
        job.id(),
        job.status(),
        job.progress01(),
        std::move(message),
    });
}

} // namespace app
