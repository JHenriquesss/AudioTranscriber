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
    export_format::JsonExporter jsonExporter;
    const auto exported = jsonExporter.exportToFile(*transcript, jsonPath);
    if (!exported.ok) {
        return exported;
    }

    storage::TranscriptRecord record;
    record.id = "transcript_" + job.id();
    record.jobId = job.id();
    record.jsonPath = jsonPath;
    record.createdAt = currentTimestampIso();
    const auto inserted = transcriptRepository_.createTranscript(record);
    if (!inserted.ok) {
        return inserted;
    }

    return jobRepository_.updateJob(toRecord(job));
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
