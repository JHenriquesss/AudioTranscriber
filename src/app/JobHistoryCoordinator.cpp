#include "app/JobHistoryCoordinator.hpp"

#include "app/JobStatus.hpp"
#include "export/JsonExporter.hpp"
#include "shared/Time.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>

namespace app {

namespace {

shared::AppError databaseError(std::string message, std::string details = {}) {
    return shared::AppError{
        shared::ErrorCode::DatabaseError,
        std::move(message),
        std::move(details),
    };
}

} // namespace

JobHistoryCoordinator::JobHistoryCoordinator(AppPaths paths, storage::Database database)
    : paths_(std::move(paths)), database_(std::move(database)), jobRepository_(database_),
      transcriptRepository_(database_) {}

shared::Result<void> JobHistoryCoordinator::persistJobCreated(const TranscriptionJob &job) {
    return jobRepository_.createJob(toRecord(job));
}

shared::Result<void> JobHistoryCoordinator::persistJobUpdated(const TranscriptionJob &job) {
    return jobRepository_.updateJob(toRecord(job));
}

shared::Result<void> JobHistoryCoordinator::persistJobCompleted(const TranscriptionJob &job) {
    const auto *transcript = job.transcript();
    if (transcript == nullptr) {
        return shared::Result<void>::failure(databaseError(
            "Cannot persist completed job without transcript metadata.", job.id()));
    }

    std::error_code errorCode;
    std::filesystem::create_directories(paths_.transcriptsDirectory(), errorCode);
    if (errorCode) {
        return shared::Result<void>::failure(databaseError("Failed to create transcripts directory.",
                                                           errorCode.message()));
    }

    const auto jsonPath = paths_.transcriptsDirectory() / (job.id() + ".json");
    const auto tempJsonPath = paths_.transcriptsDirectory() / (job.id() + ".json.tmp");
    export_format::JsonExporter jsonExporter;
    const auto exported = jsonExporter.exportToFile(*transcript, tempJsonPath);
    if (!exported.ok) {
        return exported;
    }

    std::filesystem::rename(tempJsonPath, jsonPath, errorCode);
    if (errorCode) {
        std::filesystem::remove(tempJsonPath);
        return shared::Result<void>::failure(databaseError("Failed to finalize transcript JSON.",
                                                           errorCode.message()));
    }

    storage::TranscriptRecord record;
    record.id = "transcript_" + job.id();
    record.jobId = job.id();
    record.jsonPath = jsonPath;
    record.createdAt = currentTimestampIso();

    const auto begin = database_.execute("BEGIN IMMEDIATE TRANSACTION;");
    if (!begin.ok) {
        std::filesystem::remove(jsonPath);
        return begin;
    }

    const auto inserted = transcriptRepository_.createTranscript(record);
    if (!inserted.ok) {
        (void)database_.execute("ROLLBACK;");
        std::filesystem::remove(jsonPath);
        return inserted;
    }

    const auto updated = jobRepository_.updateJob(toRecord(job));
    if (!updated.ok) {
        (void)database_.execute("ROLLBACK;");
        std::filesystem::remove(jsonPath);
        return updated;
    }

    const auto committed = database_.execute("COMMIT;");
    if (!committed.ok) {
        (void)database_.execute("ROLLBACK;");
        std::filesystem::remove(jsonPath);
        return committed;
    }

    return shared::Result<void>::success();
}

shared::Result<void>
JobHistoryCoordinator::persistExportPaths(const std::string &jobId,
                                          const storage::TranscriptRecord &exportPaths) {
    const auto begin = database_.execute("BEGIN IMMEDIATE TRANSACTION;");
    if (!begin.ok) {
        return begin;
    }

    const auto updated = transcriptRepository_.updateExportPaths(jobId, exportPaths);
    if (!updated.ok) {
        (void)database_.execute("ROLLBACK;");
        return updated;
    }

    const auto committed = database_.execute("COMMIT;");
    if (!committed.ok) {
        (void)database_.execute("ROLLBACK;");
        return committed;
    }

    return shared::Result<void>::success();
}

storage::JobRepository &JobHistoryCoordinator::jobs() {
    return jobRepository_;
}

storage::TranscriptRepository &JobHistoryCoordinator::transcripts() {
    return transcriptRepository_;
}

const AppPaths &JobHistoryCoordinator::paths() const {
    return paths_;
}

storage::JobRecord JobHistoryCoordinator::toRecord(const TranscriptionJob &job) const {
    storage::JobRecord record;
    record.id = job.id();
    record.sourceFile = job.request().inputFile;
    record.status = to_string(job.status());
    record.modelId = job.request().modelId;
    record.requestedLanguage = job.request().language;
    if (const auto *transcript = job.transcript()) {
        record.detectedLanguage = transcript->detectedLanguage;
        if (!transcript->createdAtIso.empty()) {
            record.createdAt = transcript->createdAtIso;
        } else {
            record.createdAt = currentTimestampIso();
        }
    } else {
        record.createdAt = currentTimestampIso();
    }
    if (isTerminalStatus(job.status())) {
        record.completedAt = currentTimestampIso();
    }
    if (const auto *error = job.error()) {
        record.errorCode = shared::to_string(error->code);
        record.errorMessage = error->message;
    }
    return record;
}

std::string JobHistoryCoordinator::currentTimestampIso() const {
    return shared::formatTimestampUtc(std::chrono::system_clock::now());
}

shared::Result<std::unique_ptr<JobHistoryCoordinator>> initializeJobHistory(const AppPaths &paths) {
    std::error_code errorCode;
    std::filesystem::create_directories(paths.dataDirectory(), errorCode);
    if (errorCode) {
        return shared::Result<std::unique_ptr<JobHistoryCoordinator>>::failure(
            databaseError("Failed to create data directory.", errorCode.message()));
    }

    auto databaseResult = storage::Database::open(paths.databaseFile());
    if (!databaseResult.ok) {
        return shared::Result<std::unique_ptr<JobHistoryCoordinator>>::failure(databaseResult.error);
    }

    const auto migrated = databaseResult.value->runMigrations();
    if (!migrated.ok) {
        return shared::Result<std::unique_ptr<JobHistoryCoordinator>>::failure(migrated.error);
    }

    return shared::Result<std::unique_ptr<JobHistoryCoordinator>>::success(
        std::unique_ptr<JobHistoryCoordinator>(
            new JobHistoryCoordinator(paths, std::move(*databaseResult.value))));
}

} // namespace app
