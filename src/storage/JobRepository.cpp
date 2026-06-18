#include "storage/JobRepository.hpp"

#include <sqlite3.h>

namespace storage {

namespace {

shared::AppError databaseError(std::string message, std::string details = {}) {
    return shared::AppError{
        shared::ErrorCode::DatabaseError,
        std::move(message),
        std::move(details),
    };
}

std::optional<std::string> readOptionalText(sqlite3_stmt *statement, int columnIndex) {
    if (sqlite3_column_type(statement, columnIndex) == SQLITE_NULL) {
        return std::nullopt;
    }
    const unsigned char *text = sqlite3_column_text(statement, columnIndex);
    if (text == nullptr) {
        return std::nullopt;
    }
    return std::string(reinterpret_cast<const char *>(text));
}

JobRecord readJobRecord(sqlite3_stmt *statement) {
    JobRecord record;
    record.id = reinterpret_cast<const char *>(sqlite3_column_text(statement, 0));
    record.sourceFile = reinterpret_cast<const char *>(sqlite3_column_text(statement, 1));
    record.status = reinterpret_cast<const char *>(sqlite3_column_text(statement, 2));
    record.modelId = reinterpret_cast<const char *>(sqlite3_column_text(statement, 3));
    record.requestedLanguage = reinterpret_cast<const char *>(sqlite3_column_text(statement, 4));
    record.detectedLanguage = readOptionalText(statement, 5);
    record.createdAt = reinterpret_cast<const char *>(sqlite3_column_text(statement, 6));
    record.completedAt = readOptionalText(statement, 7);
    record.errorCode = readOptionalText(statement, 8);
    record.errorMessage = readOptionalText(statement, 9);
    return record;
}

} // namespace

JobRepository::JobRepository(Database &database) : database_(database) {}

shared::Result<void> JobRepository::createJob(const JobRecord &record) {
    sqlite3_stmt *statement = nullptr;
    const char *sql =
        "INSERT INTO jobs("
        "id, source_file, status, model_id, requested_language, detected_language, created_at, "
        "completed_at, error_code, error_message"
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(database_.handle(), sql, -1, &statement, nullptr) != SQLITE_OK) {
        return shared::Result<void>::failure(
            databaseError("Failed to prepare job insert.", sqlite3_errmsg(database_.handle())));
    }

    sqlite3_bind_text(statement, 1, record.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, record.sourceFile.string().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, record.status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 4, record.modelId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 5, record.requestedLanguage.c_str(), -1, SQLITE_TRANSIENT);
    if (record.detectedLanguage.has_value()) {
        sqlite3_bind_text(statement, 6, record.detectedLanguage->c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(statement, 6);
    }
    sqlite3_bind_text(statement, 7, record.createdAt.c_str(), -1, SQLITE_TRANSIENT);
    if (record.completedAt.has_value()) {
        sqlite3_bind_text(statement, 8, record.completedAt->c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(statement, 8);
    }
    if (record.errorCode.has_value()) {
        sqlite3_bind_text(statement, 9, record.errorCode->c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(statement, 9);
    }
    if (record.errorMessage.has_value()) {
        sqlite3_bind_text(statement, 10, record.errorMessage->c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(statement, 10);
    }

    const int stepResult = sqlite3_step(statement);
    sqlite3_finalize(statement);
    if (stepResult != SQLITE_DONE) {
        return shared::Result<void>::failure(
            databaseError("Failed to insert job.", sqlite3_errmsg(database_.handle())));
    }

    return shared::Result<void>::success();
}

shared::Result<void> JobRepository::updateJob(const JobRecord &record) {
    sqlite3_stmt *statement = nullptr;
    const char *sql =
        "UPDATE jobs SET source_file = ?, status = ?, model_id = ?, requested_language = ?, "
        "detected_language = ?, created_at = ?, completed_at = ?, error_code = ?, error_message = ? "
        "WHERE id = ?;";

    if (sqlite3_prepare_v2(database_.handle(), sql, -1, &statement, nullptr) != SQLITE_OK) {
        return shared::Result<void>::failure(
            databaseError("Failed to prepare job update.", sqlite3_errmsg(database_.handle())));
    }

    sqlite3_bind_text(statement, 1, record.sourceFile.string().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, record.status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, record.modelId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 4, record.requestedLanguage.c_str(), -1, SQLITE_TRANSIENT);
    if (record.detectedLanguage.has_value()) {
        sqlite3_bind_text(statement, 5, record.detectedLanguage->c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(statement, 5);
    }
    sqlite3_bind_text(statement, 6, record.createdAt.c_str(), -1, SQLITE_TRANSIENT);
    if (record.completedAt.has_value()) {
        sqlite3_bind_text(statement, 7, record.completedAt->c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(statement, 7);
    }
    if (record.errorCode.has_value()) {
        sqlite3_bind_text(statement, 8, record.errorCode->c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(statement, 8);
    }
    if (record.errorMessage.has_value()) {
        sqlite3_bind_text(statement, 9, record.errorMessage->c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(statement, 9);
    }
    sqlite3_bind_text(statement, 10, record.id.c_str(), -1, SQLITE_TRANSIENT);

    const int stepResult = sqlite3_step(statement);
    sqlite3_finalize(statement);
    if (stepResult != SQLITE_DONE) {
        return shared::Result<void>::failure(
            databaseError("Failed to update job.", sqlite3_errmsg(database_.handle())));
    }

    return shared::Result<void>::success();
}

shared::Result<JobRecord> JobRepository::getJob(const std::string &jobId) const {
    sqlite3_stmt *statement = nullptr;
    const char *sql =
        "SELECT id, source_file, status, model_id, requested_language, detected_language, "
        "created_at, completed_at, error_code, error_message FROM jobs WHERE id = ?;";

    if (sqlite3_prepare_v2(database_.handle(), sql, -1, &statement, nullptr) != SQLITE_OK) {
        return shared::Result<JobRecord>::failure(
            databaseError("Failed to prepare job lookup.", sqlite3_errmsg(database_.handle())));
    }

    sqlite3_bind_text(statement, 1, jobId.c_str(), -1, SQLITE_TRANSIENT);
    const int stepResult = sqlite3_step(statement);
    if (stepResult != SQLITE_ROW) {
        sqlite3_finalize(statement);
        return shared::Result<JobRecord>::failure(
            databaseError("Job not found in database.", jobId));
    }

    const JobRecord record = readJobRecord(statement);
    sqlite3_finalize(statement);
    return shared::Result<JobRecord>::success(record);
}

shared::Result<std::vector<JobRecord>> JobRepository::listJobs() const {
    sqlite3_stmt *statement = nullptr;
    const char *sql =
        "SELECT id, source_file, status, model_id, requested_language, detected_language, "
        "created_at, completed_at, error_code, error_message FROM jobs ORDER BY created_at;";

    if (sqlite3_prepare_v2(database_.handle(), sql, -1, &statement, nullptr) != SQLITE_OK) {
        return shared::Result<std::vector<JobRecord>>::failure(
            databaseError("Failed to prepare job list.", sqlite3_errmsg(database_.handle())));
    }

    std::vector<JobRecord> records;
    int stepResult = sqlite3_step(statement);
    while (stepResult == SQLITE_ROW) {
        records.push_back(readJobRecord(statement));
        stepResult = sqlite3_step(statement);
    }

    sqlite3_finalize(statement);
    if (stepResult != SQLITE_DONE) {
        return shared::Result<std::vector<JobRecord>>::failure(
            databaseError("Failed to list jobs.", sqlite3_errmsg(database_.handle())));
    }

    return shared::Result<std::vector<JobRecord>>::success(std::move(records));
}

} // namespace storage
