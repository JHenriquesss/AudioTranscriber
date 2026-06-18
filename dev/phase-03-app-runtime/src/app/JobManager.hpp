#pragma once

#include "TranscriptionJob.hpp"

#include "shared/CancellationToken.hpp"
#include "shared/Logger.hpp"
#include "shared/Result.hpp"

#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

namespace app {

class JobManager {
  public:
    using ProgressCallback = std::function<void(const TranscriptionJobProgress &)>;
    using LogCallback = std::function<void(const shared::StructuredLogEntry &)>;

    [[nodiscard]] std::string submitJob(const TranscriptionJobRequest &request);
    [[nodiscard]] shared::Result<void> cancelJob(const std::string &jobId);
    [[nodiscard]] std::optional<TranscriptionJob> getJob(const std::string &jobId) const;

    [[nodiscard]] shared::Result<void>
    runJobShell(const std::string &jobId, shared::CancellationToken &cancellation,
                ProgressCallback onProgress = nullptr, LogCallback onLog = nullptr);

  private:
    [[nodiscard]] TranscriptionJob *findJob(const std::string &jobId);
    [[nodiscard]] const TranscriptionJob *findJob(const std::string &jobId) const;

    void emitLog(LogCallback &onLog, shared::LogLevel level, const char *event,
                 const TranscriptionJob &job, std::string extraFieldKey = {},
                 std::string extraFieldValue = {}) const;

    void publishProgress(TranscriptionJob &job, ProgressCallback &onProgress,
                         std::string message) const;

    mutable std::mutex mutex_;
    std::unordered_map<std::string, TranscriptionJob> jobs_;
    int nextJobNumber_ = 1;
};

} // namespace app
