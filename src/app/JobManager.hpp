#pragma once

#include "AppPaths.hpp"
#include "TranscriptionJob.hpp"

#include "export/ExportService.hpp"
#include "shared/CancellationToken.hpp"
#include "shared/Logger.hpp"
#include "shared/Result.hpp"
#include "whisper_engine/WhisperEngine.hpp"

#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

namespace app {

class JobHistoryCoordinator;

class JobManager {
  public:
    using ProgressCallback = std::function<void(const TranscriptionJobProgress &)>;
    using LogCallback = std::function<void(const shared::StructuredLogEntry &)>;

    JobManager();
    explicit JobManager(JobHistoryCoordinator *history);

    [[nodiscard]] shared::Result<std::string> submitJob(const TranscriptionJobRequest &request);
    [[nodiscard]] shared::Result<void> cancelJob(const std::string &jobId);
    [[nodiscard]] std::optional<TranscriptionJob> getJob(const std::string &jobId) const;

    [[nodiscard]] shared::Result<void>
    runJobShell(const std::string &jobId, shared::CancellationToken &cancellation,
                ProgressCallback onProgress = nullptr, LogCallback onLog = nullptr);

    [[nodiscard]] shared::Result<void>
    runJobPipeline(const std::string &jobId, const AppPaths &paths,
                   shared::CancellationToken &cancellation,
                   ProgressCallback onProgress = nullptr, LogCallback onLog = nullptr);

    [[nodiscard]] shared::Result<void> failJob(const std::string &jobId, shared::AppError error);
    [[nodiscard]] shared::Result<void> exportJobResults(const std::string &jobId);

    static void setTestWhisperEngineSupplier(
        std::function<whisper_engine::WhisperEngine()> supplier);
    static void clearTestWhisperEngineSupplier();

  private:
    [[nodiscard]] static whisper_engine::WhisperEngine createWhisperEngine();
    [[nodiscard]] TranscriptionJob *findJob(const std::string &jobId);
    [[nodiscard]] const TranscriptionJob *findJob(const std::string &jobId) const;

    void attachFakeTranscript(TranscriptionJob &job) const;
    [[nodiscard]] shared::Result<void> transitionJob(const std::string &jobId, JobStatus status,
                                                     double progress01, std::string message,
                                                     ProgressCallback &onProgress);
    [[nodiscard]] shared::Result<void> persistJobSnapshot(const std::string &jobId,
                                                          bool completed = false);
    [[nodiscard]] shared::Result<void> failJobFromRunner(const std::string &jobId,
                                                         shared::AppError error,
                                                         LogCallback &onLog,
                                                         ProgressCallback &onProgress);

    void emitLog(LogCallback &onLog, shared::LogLevel level, const char *event,
                 const TranscriptionJob &job, std::string extraFieldKey = {},
                 std::string extraFieldValue = {}) const;

    void publishProgress(TranscriptionJob &job, ProgressCallback &onProgress,
                         std::string message) const;

    mutable std::mutex mutex_;
    std::unordered_map<std::string, TranscriptionJob> jobs_;
    int nextJobNumber_ = 1;
    export_format::ExportService exportService_;
    JobHistoryCoordinator *history_ = nullptr;
};

} // namespace app
