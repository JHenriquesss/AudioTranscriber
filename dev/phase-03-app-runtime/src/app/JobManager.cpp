#include "JobManager.hpp"

#include <algorithm>

namespace app {

namespace {

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

std::string JobManager::submitJob(const TranscriptionJobRequest &request) {
    std::lock_guard lock(mutex_);
    const std::string jobId = "job_" + std::to_string(nextJobNumber_++);
    jobs_.emplace(jobId, TranscriptionJob(jobId, request));
    return jobId;
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
