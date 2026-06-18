#include "app/TranscriptionJob.hpp"

#include <catch2/catch_test_macros.hpp>

namespace {

app::TranscriptionJobRequest sampleRequest() {
    app::TranscriptionJobRequest request;
    request.inputFile = "sample.wav";
    request.outputDirectory = "exports";
    request.language = "en";
    request.modelId = "small";
    return request;
}

} // namespace

TEST_CASE("TranscriptionJob records progress history", "[app][transcription-job]") {
    app::TranscriptionJob job("job_1", sampleRequest());

    const auto transitioned = job.transitionTo(app::JobStatus::ProbingMedia);
    REQUIRE(transitioned.ok);
    job.setProgress(0.2, "Probing media.");

    REQUIRE(job.status() == app::JobStatus::ProbingMedia);
    REQUIRE(job.progress01() == 0.2);
    REQUIRE(job.progressHistory().size() >= 2);
}

TEST_CASE("TranscriptionJob rejects invalid status transition", "[app][transcription-job]") {
    app::TranscriptionJob job("job_2", sampleRequest());

    const auto transitioned = job.transitionTo(app::JobStatus::Completed);
    REQUIRE_FALSE(transitioned.ok);
    REQUIRE(job.status() == app::JobStatus::Queued);
}

TEST_CASE("TranscriptionJob allows transition to Failed from active state", "[app][transcription-job]") {
    app::TranscriptionJob job("job_3", sampleRequest());

    const auto probing = job.transitionTo(app::JobStatus::ProbingMedia);
    REQUIRE(probing.ok);

    const auto failed = job.transitionTo(app::JobStatus::Failed);
    REQUIRE(failed.ok);
    REQUIRE(job.status() == app::JobStatus::Failed);
    REQUIRE(app::isTerminalStatus(job.status()));
}
