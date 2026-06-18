#pragma once

#include "JobStatus.hpp"

#include "shared/Result.hpp"
#include "transcription/Transcript.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace app {

struct TranscriptionJobRequest {
    std::filesystem::path inputFile;
    std::filesystem::path outputDirectory;
    std::string language;
    std::string modelId;
    bool enableWordTimestamps = true;
    bool exportTxt = true;
    bool exportSrt = true;
    bool exportVtt = true;
    bool exportJson = true;
};

struct TranscriptionJobProgress {
    std::string jobId;
    JobStatus status = JobStatus::Queued;
    double progress01 = 0.0;
    std::string message;
};

class TranscriptionJob {
  public:
    TranscriptionJob(std::string jobId, TranscriptionJobRequest request);

    [[nodiscard]] const std::string &id() const;
    [[nodiscard]] const TranscriptionJobRequest &request() const;
    [[nodiscard]] JobStatus status() const;
    [[nodiscard]] double progress01() const;
    [[nodiscard]] const std::vector<TranscriptionJobProgress> &progressHistory() const;
    [[nodiscard]] const shared::AppError *error() const;
    [[nodiscard]] const transcription::TranscriptDocument *transcript() const;
    [[nodiscard]] std::string transcriptPlainText() const;

    [[nodiscard]] shared::Result<void> transitionTo(JobStatus newStatus);
    void setProgress(double progress01, std::string message);
    void setTranscript(transcription::TranscriptDocument transcript);
    void setError(shared::AppError error);

  private:
    void recordProgress(std::string message);

    std::string id_;
    TranscriptionJobRequest request_;
    JobStatus status_ = JobStatus::Queued;
    double progress01_ = 0.0;
    std::vector<TranscriptionJobProgress> progressHistory_;
    std::optional<shared::AppError> error_;
    std::optional<transcription::TranscriptDocument> transcript_;
};

} // namespace app
