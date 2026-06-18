#include "TranscriptionJob.hpp"

namespace app {

TranscriptionJob::TranscriptionJob(std::string jobId, TranscriptionJobRequest request)
    : id_(std::move(jobId)), request_(std::move(request)) {
    recordProgress("Job queued.");
}

const std::string &TranscriptionJob::id() const {
    return id_;
}

const TranscriptionJobRequest &TranscriptionJob::request() const {
    return request_;
}

JobStatus TranscriptionJob::status() const {
    return status_;
}

double TranscriptionJob::progress01() const {
    return progress01_;
}

const std::vector<TranscriptionJobProgress> &TranscriptionJob::progressHistory() const {
    return progressHistory_;
}

const shared::AppError *TranscriptionJob::error() const {
    if (!error_.has_value()) {
        return nullptr;
    }
    return &*error_;
}

const transcription::TranscriptDocument *TranscriptionJob::transcript() const {
    if (!transcript_.has_value()) {
        return nullptr;
    }
    return &*transcript_;
}

std::string TranscriptionJob::transcriptPlainText() const {
    if (!transcript_.has_value()) {
        return {};
    }

    std::string text;
    for (const auto &segment : transcript_->segments) {
        if (!text.empty()) {
            text.push_back('\n');
        }
        text += segment.text;
    }
    return text;
}

shared::Result<void> TranscriptionJob::transitionTo(JobStatus newStatus) {
    if (!canTransition(status_, newStatus)) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::Unknown,
            "Invalid job status transition.",
            to_string(status_) + " -> " + to_string(newStatus),
        });
    }

    status_ = newStatus;
    recordProgress("Status changed to " + to_string(newStatus) + ".");
    return shared::Result<void>::success();
}

void TranscriptionJob::setProgress(double progress01, std::string message) {
    progress01_ = progress01;
    recordProgress(std::move(message));
}

void TranscriptionJob::setTranscript(transcription::TranscriptDocument transcript) {
    transcript_ = std::move(transcript);
}

void TranscriptionJob::setError(shared::AppError error) {
    error_ = std::move(error);
    status_ = JobStatus::Failed;
    recordProgress("Job failed.");
}

void TranscriptionJob::recordProgress(std::string message) {
    progressHistory_.push_back(TranscriptionJobProgress{
        id_,
        status_,
        progress01_,
        std::move(message),
    });
}

} // namespace app
