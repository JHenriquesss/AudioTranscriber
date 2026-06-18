#include "JobStatus.hpp"

namespace app {

std::string to_string(JobStatus status) {
    switch (status) {
    case JobStatus::Queued:
        return "Queued";
    case JobStatus::ProbingMedia:
        return "ProbingMedia";
    case JobStatus::ExtractingAudio:
        return "ExtractingAudio";
    case JobStatus::NormalizingAudio:
        return "NormalizingAudio";
    case JobStatus::LoadingModel:
        return "LoadingModel";
    case JobStatus::Transcribing:
        return "Transcribing";
    case JobStatus::PostProcessing:
        return "PostProcessing";
    case JobStatus::SavingTranscript:
        return "SavingTranscript";
    case JobStatus::Exporting:
        return "Exporting";
    case JobStatus::Completed:
        return "Completed";
    case JobStatus::Failed:
        return "Failed";
    case JobStatus::Cancelled:
        return "Cancelled";
    }
    return "Unknown";
}

bool isTerminalStatus(JobStatus status) {
    return status == JobStatus::Completed || status == JobStatus::Failed ||
           status == JobStatus::Cancelled;
}

std::optional<JobStatus> nextPipelineStatus(JobStatus status) {
    switch (status) {
    case JobStatus::Queued:
        return JobStatus::ProbingMedia;
    case JobStatus::ProbingMedia:
        return JobStatus::ExtractingAudio;
    case JobStatus::ExtractingAudio:
        return JobStatus::NormalizingAudio;
    case JobStatus::NormalizingAudio:
        return JobStatus::LoadingModel;
    case JobStatus::LoadingModel:
        return JobStatus::Transcribing;
    case JobStatus::Transcribing:
        return JobStatus::PostProcessing;
    case JobStatus::PostProcessing:
        return JobStatus::SavingTranscript;
    case JobStatus::SavingTranscript:
        return JobStatus::Exporting;
    case JobStatus::Exporting:
        return JobStatus::Completed;
    case JobStatus::Completed:
    case JobStatus::Failed:
    case JobStatus::Cancelled:
        return std::nullopt;
    }
    return std::nullopt;
}

bool canTransition(JobStatus from, JobStatus to) {
    if (from == to) {
        return true;
    }

    if (isTerminalStatus(from)) {
        return false;
    }

    if (to == JobStatus::Failed || to == JobStatus::Cancelled) {
        return true;
    }

    const auto expectedNext = nextPipelineStatus(from);
    return expectedNext.has_value() && *expectedNext == to;
}

} // namespace app
