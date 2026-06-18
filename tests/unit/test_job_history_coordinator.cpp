#include "StorageTestHelpers.hpp"
#include "app/AppPaths.hpp"
#include "app/JobHistoryCoordinator.hpp"
#include "app/JobManager.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

namespace {

app::TranscriptionJobRequest sampleRequest(const std::filesystem::path &workspace) {
    const auto inputPath = workspace / "input.mp3";
    std::ofstream input(inputPath, std::ios::binary);
    input << "fake";

    app::TranscriptionJobRequest request;
    request.inputFile = inputPath;
    request.outputDirectory = workspace / "exports";
    std::filesystem::create_directories(request.outputDirectory);
    request.language = "pt";
    request.modelId = "small";
    return request;
}

} // namespace

TEST_CASE("JobHistoryCoordinator persists completed job metadata and transcript json", "[app][history]") {
    const auto root = test_support::uniqueTempRoot("phase07-job-history");
    std::filesystem::create_directories(root / "data");
    const auto paths = app::AppPaths::fromUserDataRoot(root, true);

    {
        auto historyResult = app::initializeJobHistory(paths);
        REQUIRE(historyResult.ok);

        app::JobManager manager(historyResult.value.get());
        const auto submission = manager.submitJob(sampleRequest(root));
        REQUIRE(submission.ok);

        shared::CancellationToken cancellation;
        REQUIRE(manager.runJobShell(submission.value, cancellation).ok);

        const auto jobRecord = historyResult.value->jobs().getJob(submission.value);
        REQUIRE(jobRecord.ok);
        REQUIRE(jobRecord.value.status == "Completed");

        const auto transcriptRecord = historyResult.value->transcripts().getByJobId(submission.value);
        REQUIRE(transcriptRecord.ok);
        REQUIRE(std::filesystem::exists(transcriptRecord.value.jsonPath));
    }

    test_support::cleanupTempRoot(root);
}
