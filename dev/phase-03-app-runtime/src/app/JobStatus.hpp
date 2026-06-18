#pragma once

#include <optional>
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

[[nodiscard]] bool isTerminalStatus(JobStatus status);
[[nodiscard]] bool canTransition(JobStatus from, JobStatus to);
[[nodiscard]] std::optional<JobStatus> nextPipelineStatus(JobStatus status);

} // namespace app
