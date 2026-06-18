#include "app/JobManager.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <vector>

namespace {

app::TranscriptionJobRequest sampleRequest() {
    app::TranscriptionJobRequest request;
    request.inputFile = "meeting.mp3";
    request.outputDirectory = "exports";
    request.language = "pt";
    request.modelId = "small";
    return request;
}

} // namespace

TEST_CASE("JobManager runs shell lifecycle and emits progress", "[app][job-manager]") {
    app::JobManager manager;
    const auto jobId = manager.submitJob(sampleRequest());

    std::vector<app::TranscriptionJobProgress> progressEvents;
    shared::CancellationToken cancellation;

    const auto result = manager.runJobShell(
        jobId, cancellation,
        [&](const app::TranscriptionJobProgress &progress) { progressEvents.push_back(progress); });

    REQUIRE(result.ok);

    const auto job = manager.getJob(jobId);
    REQUIRE(job.has_value());
    REQUIRE(job->status() == app::JobStatus::Completed);
    REQUIRE(job->progress01() == 1.0);
    REQUIRE_FALSE(progressEvents.empty());
    REQUIRE(progressEvents.back().status == app::JobStatus::Completed);
}

TEST_CASE("JobManager shell follows the full pipeline status sequence", "[app][job-manager]") {
    app::JobManager manager;
    const auto jobId = manager.submitJob(sampleRequest());

    std::vector<app::JobStatus> observedStatuses;
    shared::CancellationToken cancellation;

    const auto result = manager.runJobShell(
        jobId, cancellation,
        [&](const app::TranscriptionJobProgress &progress) {
            if (observedStatuses.empty() || observedStatuses.back() != progress.status) {
                observedStatuses.push_back(progress.status);
            }
        });

    REQUIRE(result.ok);

    const std::vector<app::JobStatus> expected{
        app::JobStatus::ProbingMedia,
        app::JobStatus::ExtractingAudio,
        app::JobStatus::NormalizingAudio,
        app::JobStatus::LoadingModel,
        app::JobStatus::Transcribing,
        app::JobStatus::PostProcessing,
        app::JobStatus::SavingTranscript,
        app::JobStatus::Exporting,
        app::JobStatus::Completed,
    };
    REQUIRE(observedStatuses == expected);

    const auto job = manager.getJob(jobId);
    REQUIRE(job.has_value());
    REQUIRE(job->progressHistory().front().status == app::JobStatus::Queued);
}

TEST_CASE("JobManager honours cancellation at checkpoint", "[app][job-manager]") {
    app::JobManager manager;
    const auto jobId = manager.submitJob(sampleRequest());

    shared::CancellationToken cancellation;
    cancellation.cancel();

    const auto result = manager.runJobShell(jobId, cancellation);
    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.code == shared::ErrorCode::Cancelled);

    const auto job = manager.getJob(jobId);
    REQUIRE(job.has_value());
    REQUIRE(job->status() == app::JobStatus::Cancelled);
}

TEST_CASE("JobManager emits structured lifecycle log events", "[app][job-manager]") {
    app::JobManager manager;
    const auto jobId = manager.submitJob(sampleRequest());

    std::vector<std::string> events;
    shared::CancellationToken cancellation;

    const auto result = manager.runJobShell(
        jobId, cancellation, nullptr,
        [&](const shared::StructuredLogEntry &entry) { events.push_back(entry.event); });

    REQUIRE(result.ok);
    REQUIRE(std::find(events.begin(), events.end(), shared::log_events::jobCreated) != events.end());
    REQUIRE(std::find(events.begin(), events.end(), shared::log_events::transcriptionStarted) !=
            events.end());
    REQUIRE(std::find(events.begin(), events.end(), shared::log_events::exportCompleted) !=
            events.end());
    REQUIRE(std::find(events.begin(), events.end(), shared::log_events::jobCompleted) !=
            events.end());
}
