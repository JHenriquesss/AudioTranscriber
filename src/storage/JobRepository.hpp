#pragma once

#include "storage/Database.hpp"

#include "shared/Result.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace storage {

struct JobRecord {
    std::string id;
    std::filesystem::path sourceFile;
    std::string status;
    std::string modelId;
    std::string requestedLanguage;
    std::optional<std::string> detectedLanguage;
    std::string createdAt;
    std::optional<std::string> completedAt;
    std::optional<std::string> errorCode;
    std::optional<std::string> errorMessage;
};

class JobRepository {
  public:
    explicit JobRepository(Database &database);

    [[nodiscard]] shared::Result<void> createJob(const JobRecord &record);
    [[nodiscard]] shared::Result<void> updateJob(const JobRecord &record);
    [[nodiscard]] shared::Result<JobRecord> getJob(const std::string &jobId) const;
    [[nodiscard]] shared::Result<std::vector<JobRecord>> listJobs() const;

  private:
    Database &database_;
};

} // namespace storage
