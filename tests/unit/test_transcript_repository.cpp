#include "StorageTestHelpers.hpp"
#include "storage/Database.hpp"
#include "storage/JobRepository.hpp"
#include "storage/TranscriptRepository.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

TEST_CASE("TranscriptRepository stores metadata paths without transcript body", "[storage][transcripts]") {
    const auto root = test_support::uniqueTempRoot("phase07-transcript-repo");
    const auto databasePath = root / "app.db";

    {
        auto opened = storage::Database::open(databasePath);
        REQUIRE(opened.ok);
        REQUIRE(opened.value->runMigrations().ok);

        storage::JobRepository jobRepository(*opened.value);
        storage::TranscriptRepository transcriptRepository(*opened.value);

        storage::JobRecord job;
        job.id = "job_1";
        job.sourceFile = "clip.wav";
        job.status = "Completed";
        job.modelId = "small";
        job.requestedLanguage = "en";
        job.createdAt = "2026-01-01T00:00:00Z";
        REQUIRE(jobRepository.createJob(job).ok);

        const auto jsonPath = root / "data" / "transcripts" / "job_1.json";
        std::filesystem::create_directories(jsonPath.parent_path());
        std::ofstream jsonFile(jsonPath);
        jsonFile << "{\"id\":\"job_1\"}";

        storage::TranscriptRecord transcript;
        transcript.id = "transcript_job_1";
        transcript.jobId = "job_1";
        transcript.jsonPath = jsonPath;
        transcript.createdAt = "2026-01-01T00:01:00Z";
        REQUIRE(transcriptRepository.createTranscript(transcript).ok);

        const auto loaded = transcriptRepository.getByJobId("job_1");
        REQUIRE(loaded.ok);
        REQUIRE(loaded.value.jsonPath == jsonPath);
    }

    test_support::cleanupTempRoot(root);
}

TEST_CASE("TranscriptRepository updates export paths for completed job", "[storage][transcripts]") {
    const auto root = test_support::uniqueTempRoot("phase08-transcript-export-paths");
    const auto databasePath = root / "app.db";

    {
        auto opened = storage::Database::open(databasePath);
        REQUIRE(opened.ok);
        REQUIRE(opened.value->runMigrations().ok);

        storage::JobRepository jobRepository(*opened.value);
        storage::TranscriptRepository transcriptRepository(*opened.value);

        storage::JobRecord job;
        job.id = "job_2";
        job.sourceFile = "clip.wav";
        job.status = "Completed";
        job.modelId = "small";
        job.requestedLanguage = "en";
        job.createdAt = "2026-01-01T00:00:00Z";
        REQUIRE(jobRepository.createJob(job).ok);

        const auto jsonPath = root / "data" / "transcripts" / "job_2.json";
        std::filesystem::create_directories(jsonPath.parent_path());
        std::ofstream jsonFile(jsonPath);
        jsonFile << "{\"id\":\"job_2\"}";

        storage::TranscriptRecord transcript;
        transcript.id = "transcript_job_2";
        transcript.jobId = "job_2";
        transcript.jsonPath = jsonPath;
        transcript.createdAt = "2026-01-01T00:01:00Z";
        REQUIRE(transcriptRepository.createTranscript(transcript).ok);

        storage::TranscriptRecord exportPaths;
        exportPaths.jobId = "job_2";
        exportPaths.txtPath = root / "exports" / "clip.txt";
        exportPaths.srtPath = root / "exports" / "clip.srt";
        exportPaths.vttPath = root / "exports" / "clip.vtt";
        REQUIRE(transcriptRepository.updateExportPaths("job_2", exportPaths).ok);

        const auto loaded = transcriptRepository.getByJobId("job_2");
        REQUIRE(loaded.ok);
        REQUIRE(loaded.value.txtPath == exportPaths.txtPath);
        REQUIRE(loaded.value.srtPath == exportPaths.srtPath);
        REQUIRE(loaded.value.vttPath == exportPaths.vttPath);
    }

    test_support::cleanupTempRoot(root);
}
