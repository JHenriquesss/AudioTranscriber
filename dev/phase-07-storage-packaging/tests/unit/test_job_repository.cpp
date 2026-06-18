#include "StorageTestHelpers.hpp"
#include "storage/Database.hpp"
#include "storage/JobRepository.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <memory>
#include <optional>

namespace {

struct JobRepositoryFixture {
    std::unique_ptr<storage::Database> database;
    storage::JobRepository *repository = nullptr;

    explicit JobRepositoryFixture(const std::filesystem::path &databasePath) {
        auto opened = storage::Database::open(databasePath);
        REQUIRE(opened.ok);
        REQUIRE(opened.value->runMigrations().ok);
        database = std::move(opened.value);
        repositoryStorage_.emplace(*database);
        repository = &repositoryStorage_.value();
    }

  private:
    std::optional<storage::JobRepository> repositoryStorage_;
};

storage::JobRecord sampleJob(const std::string &jobId) {
    storage::JobRecord record;
    record.id = jobId;
    record.sourceFile = "meeting.mp3";
    record.status = "Queued";
    record.modelId = "small";
    record.requestedLanguage = "pt";
    record.createdAt = "2026-01-01T00:00:00Z";
    return record;
}

} // namespace

TEST_CASE("JobRepository roundtrips create update and read", "[storage][jobs]") {
    const auto root = test_support::uniqueTempRoot("phase07-job-repo");
    {
        JobRepositoryFixture fixture(root / "app.db");

        storage::JobRecord record = sampleJob("job_1");
        REQUIRE(fixture.repository->createJob(record).ok);

        record.status = "Completed";
        record.completedAt = "2026-01-01T00:05:00Z";
        REQUIRE(fixture.repository->updateJob(record).ok);

        const auto loaded = fixture.repository->getJob("job_1");
        REQUIRE(loaded.ok);
        REQUIRE(loaded.value.status == "Completed");
        REQUIRE(loaded.value.completedAt.has_value());

        const auto listed = fixture.repository->listJobs();
        REQUIRE(listed.ok);
        REQUIRE(listed.value.size() == 1);
    }

    test_support::cleanupTempRoot(root);
}
