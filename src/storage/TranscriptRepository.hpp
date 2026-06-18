#pragma once

#include "storage/Database.hpp"

#include "shared/Result.hpp"

#include <filesystem>
#include <optional>
#include <string>

namespace storage {

struct TranscriptRecord {
    std::string id;
    std::string jobId;
    std::filesystem::path jsonPath;
    std::optional<std::filesystem::path> txtPath;
    std::optional<std::filesystem::path> srtPath;
    std::optional<std::filesystem::path> vttPath;
    std::string createdAt;
};

class TranscriptRepository {
  public:
    explicit TranscriptRepository(Database &database);

    [[nodiscard]] shared::Result<void> createTranscript(const TranscriptRecord &record);
    [[nodiscard]] shared::Result<void> updateExportPaths(const std::string &jobId,
                                                         const TranscriptRecord &paths);
    [[nodiscard]] shared::Result<TranscriptRecord> getByJobId(const std::string &jobId) const;

  private:
    Database &database_;
};

} // namespace storage
