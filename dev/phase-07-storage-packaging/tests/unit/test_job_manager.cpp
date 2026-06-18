#include "app/JobManager.hpp"

#include "AudioTestHelpers.hpp"
#include "app/AppPaths.hpp"
#include "app/JobHistoryCoordinator.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

namespace {

std::filesystem::path createJobManagerWorkspace() {
    const auto workspace =
        std::filesystem::temp_directory_path() / "phase06-job-manager-tests";
    std::filesystem::create_directories(workspace);
    return workspace;
}

std::filesystem::path writeInputFile(const std::filesystem::path &workspace) {
    const auto inputPath = workspace / "meeting.mp3";
    std::ofstream stream(inputPath, std::ios::binary);
    stream << "fake media";
    return inputPath;
}

std::filesystem::path writeWavInputFile(const std::filesystem::path &workspace) {
    const auto inputPath = workspace / "meeting.wav";
    audio_test::writePcmMonoWav(inputPath, 16000, std::vector<std::int16_t>(1600, 0));
    return inputPath;
}

void preparePipelineWorkspace(const std::filesystem::path &workspace) {
    std::filesystem::create_directories(workspace / "data");
    std::filesystem::create_directories(workspace / "models");
    std::filesystem::create_directories(workspace / "tools");

    std::ofstream model(workspace / "models" / "ggml-small.bin", std::ios::binary);
    model << "stub model";

    const auto ffmpeg = audio_test::resolveFfmpegExecutable();
    if (std::filesystem::exists(ffmpeg)) {
        std::filesystem::copy_file(ffmpeg, workspace / "tools" / "ffmpeg.exe",
                                   std::filesystem::copy_options::overwrite_existing);
    }
}

app::TranscriptionJobRequest sampleRequest(const std::filesystem::path &workspace) {
    app::TranscriptionJobRequest request;
    request.inputFile = writeInputFile(workspace);
    request.outputDirectory = workspace / "exports";
    std::filesystem::create_directories(request.outputDirectory);
    request.language = "pt";
    request.modelId = "small";
    return request;
}

} // namespace

TEST_CASE("JobManager runs shell lifecycle and emits progress", "[app][job-manager]") {
    const auto workspace = createJobManagerWorkspace();
    app::JobManager manager;
    const auto submission = manager.submitJob(sampleRequest(workspace));
    REQUIRE(submission.ok);
    const auto jobId = submission.value;

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
    REQUIRE(job->transcript() != nullptr);
    REQUIRE_FALSE(job->transcriptPlainText().empty());
}

TEST_CASE("JobManager shell follows the full pipeline status sequence", "[app][job-manager]") {
    const auto workspace = createJobManagerWorkspace();
    app::JobManager manager;
    const auto submission = manager.submitJob(sampleRequest(workspace));
    REQUIRE(submission.ok);
    const auto jobId = submission.value;

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
    const auto workspace = createJobManagerWorkspace();
    app::JobManager manager;
    const auto submission = manager.submitJob(sampleRequest(workspace));
    REQUIRE(submission.ok);
    const auto jobId = submission.value;

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
    const auto workspace = createJobManagerWorkspace();
    app::JobManager manager;
    const auto submission = manager.submitJob(sampleRequest(workspace));
    REQUIRE(submission.ok);
    const auto jobId = submission.value;

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

TEST_CASE("JobManager rejects missing input file on submit", "[app][job-manager]") {
    app::JobManager manager;
    app::TranscriptionJobRequest request;
    request.outputDirectory = std::filesystem::temp_directory_path() / "phase06-missing-input";
    std::filesystem::create_directories(request.outputDirectory);
    request.language = "en";
    request.modelId = "small";

    const auto submission = manager.submitJob(request);
    REQUIRE_FALSE(submission.ok);
    REQUIRE(submission.error.code == shared::ErrorCode::FileNotFound);
}

TEST_CASE("JobManager exports completed shell transcript", "[app][job-manager]") {
    const auto workspace = createJobManagerWorkspace();
    app::JobManager manager;
    const auto submission = manager.submitJob(sampleRequest(workspace));
    REQUIRE(submission.ok);
    const auto jobId = submission.value;

    shared::CancellationToken cancellation;
    const auto runResult = manager.runJobShell(jobId, cancellation);
    REQUIRE(runResult.ok);

    const auto exportResult = manager.exportJobResults(jobId);
    REQUIRE(exportResult.ok);
    REQUIRE(std::filesystem::exists(workspace / "exports" / "meeting.txt"));
}

TEST_CASE("JobManager records typed failure for failed jobs", "[app][job-manager]") {
    const auto workspace = createJobManagerWorkspace();
    app::JobManager manager;
    const auto submission = manager.submitJob(sampleRequest(workspace));
    REQUIRE(submission.ok);

    const auto failResult = manager.failJob(
        submission.value,
        shared::AppError{
            shared::ErrorCode::TranscriptionFailed,
            "Transcription failed.",
            "fake engine failure",
        });
    REQUIRE(failResult.ok);

    const auto job = manager.getJob(submission.value);
    REQUIRE(job.has_value());
    REQUIRE(job->status() == app::JobStatus::Failed);
    REQUIRE(job->error() != nullptr);
    REQUIRE(job->error()->code == shared::ErrorCode::TranscriptionFailed);
}

TEST_CASE("JobManager runs real audio and transcription pipeline path", "[app][job-manager]") {
    const auto workspace = std::filesystem::temp_directory_path() / "phase07-job-pipeline-tests";
    std::filesystem::remove_all(workspace);
    preparePipelineWorkspace(workspace);

    auto paths = app::AppPaths::resolve(workspace);
    auto historyResult = app::initializeJobHistory(paths);
    REQUIRE(historyResult.ok);

    app::JobManager manager(historyResult.value.get());
    auto request = sampleRequest(workspace);
    request.inputFile = writeWavInputFile(workspace);

    const auto submission = manager.submitJob(request);
    REQUIRE(submission.ok);

    std::vector<app::TranscriptionJobProgress> progressEvents;
    shared::CancellationToken cancellation;
    const auto result = manager.runJobPipeline(
        submission.value, paths, cancellation,
        [&](const app::TranscriptionJobProgress &progress) { progressEvents.push_back(progress); });

    REQUIRE(result.ok);
    const auto job = manager.getJob(submission.value);
    REQUIRE(job.has_value());
    REQUIRE(job->status() == app::JobStatus::Completed);
    REQUIRE(job->transcript() != nullptr);
    REQUIRE(job->transcriptPlainText() == "stub transcription");
    REQUIRE(std::filesystem::exists(workspace / "exports" / "meeting.txt"));
    REQUIRE(std::filesystem::exists(paths.transcriptsDirectory() / (submission.value + ".json")));
    REQUIRE_FALSE(progressEvents.empty());
    REQUIRE(progressEvents.back().status == app::JobStatus::Completed);

    std::error_code cleanupError;
    std::filesystem::remove_all(workspace, cleanupError);
}
