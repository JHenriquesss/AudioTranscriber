#pragma once

#include "app/AppPaths.hpp"
#include "app/TranscriptionJob.hpp"
#include "storage/Database.hpp"
#include "storage/JobRepository.hpp"
#include "storage/TranscriptRepository.hpp"

#include "shared/Result.hpp"

namespace app {

class JobHistoryCoordinator {
  public:
    JobHistoryCoordinator(AppPaths paths, storage::Database database);

    [[nodiscard]] shared::Result<void> persistJobCreated(const TranscriptionJob &job);
    [[nodiscard]] shared::Result<void> persistJobUpdated(const TranscriptionJob &job);
    [[nodiscard]] shared::Result<void> persistJobCompleted(const TranscriptionJob &job);
    [[nodiscard]] shared::Result<void>
    persistExportPaths(const std::string &jobId, const storage::TranscriptRecord &exportPaths);

    [[nodiscard]] storage::JobRepository &jobs();
    [[nodiscard]] storage::TranscriptRepository &transcripts();
    [[nodiscard]] const AppPaths &paths() const;

  private:
    [[nodiscard]] storage::JobRecord toRecord(const TranscriptionJob &job) const;
    [[nodiscard]] std::string currentTimestampIso() const;

    AppPaths paths_;
    storage::Database database_;
    storage::JobRepository jobRepository_;
    storage::TranscriptRepository transcriptRepository_;
};

[[nodiscard]] shared::Result<std::unique_ptr<JobHistoryCoordinator>>
initializeJobHistory(const AppPaths &paths);

} // namespace app
