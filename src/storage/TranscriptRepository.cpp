#include "storage/TranscriptRepository.hpp"

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

std::optional<std::filesystem::path> readOptionalPath(sqlite3_stmt *statement, int columnIndex) {
    if (sqlite3_column_type(statement, columnIndex) == SQLITE_NULL) {
        return std::nullopt;
    }
    const unsigned char *text = sqlite3_column_text(statement, columnIndex);
    if (text == nullptr) {
        return std::nullopt;
    }
    return std::filesystem::path(reinterpret_cast<const char *>(text));
}

TranscriptRecord readTranscriptRecord(sqlite3_stmt *statement) {
    TranscriptRecord record;
    record.id = reinterpret_cast<const char *>(sqlite3_column_text(statement, 0));
    record.jobId = reinterpret_cast<const char *>(sqlite3_column_text(statement, 1));
    record.jsonPath = reinterpret_cast<const char *>(sqlite3_column_text(statement, 2));
    record.txtPath = readOptionalPath(statement, 3);
    record.srtPath = readOptionalPath(statement, 4);
    record.vttPath = readOptionalPath(statement, 5);
    record.createdAt = reinterpret_cast<const char *>(sqlite3_column_text(statement, 6));
    return record;
}

} // namespace

TranscriptRepository::TranscriptRepository(Database &database) : database_(database) {}

shared::Result<void> TranscriptRepository::createTranscript(const TranscriptRecord &record) {
    sqlite3_stmt *statement = nullptr;
    const char *sql =
        "INSERT INTO transcripts("
        "id, job_id, json_path, txt_path, srt_path, vtt_path, created_at"
        ") VALUES (?, ?, ?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(database_.handle(), sql, -1, &statement, nullptr) != SQLITE_OK) {
        return shared::Result<void>::failure(databaseError("Failed to prepare transcript insert.",
                                                           sqlite3_errmsg(database_.handle())));
    }

    sqlite3_bind_text(statement, 1, record.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, record.jobId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, record.jsonPath.string().c_str(), -1, SQLITE_TRANSIENT);
    if (record.txtPath.has_value()) {
        sqlite3_bind_text(statement, 4, record.txtPath->string().c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(statement, 4);
    }
    if (record.srtPath.has_value()) {
        sqlite3_bind_text(statement, 5, record.srtPath->string().c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(statement, 5);
    }
    if (record.vttPath.has_value()) {
        sqlite3_bind_text(statement, 6, record.vttPath->string().c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(statement, 6);
    }
    sqlite3_bind_text(statement, 7, record.createdAt.c_str(), -1, SQLITE_TRANSIENT);

    const int stepResult = sqlite3_step(statement);
    sqlite3_finalize(statement);
    if (stepResult != SQLITE_DONE) {
        return shared::Result<void>::failure(
            databaseError("Failed to insert transcript.", sqlite3_errmsg(database_.handle())));
    }

    return shared::Result<void>::success();
}

shared::Result<void> TranscriptRepository::updateExportPaths(const std::string &jobId,
                                                             const TranscriptRecord &paths) {
    sqlite3_stmt *statement = nullptr;
    const char *sql =
        "UPDATE transcripts SET txt_path = ?, srt_path = ?, vtt_path = ? WHERE job_id = ?;";

    if (sqlite3_prepare_v2(database_.handle(), sql, -1, &statement, nullptr) != SQLITE_OK) {
        return shared::Result<void>::failure(
            databaseError("Failed to prepare transcript export update.",
                          sqlite3_errmsg(database_.handle())));
    }

    if (paths.txtPath.has_value()) {
        sqlite3_bind_text(statement, 1, paths.txtPath->string().c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(statement, 1);
    }
    if (paths.srtPath.has_value()) {
        sqlite3_bind_text(statement, 2, paths.srtPath->string().c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(statement, 2);
    }
    if (paths.vttPath.has_value()) {
        sqlite3_bind_text(statement, 3, paths.vttPath->string().c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(statement, 3);
    }
    sqlite3_bind_text(statement, 4, jobId.c_str(), -1, SQLITE_TRANSIENT);

    const int stepResult = sqlite3_step(statement);
    sqlite3_finalize(statement);
    if (stepResult != SQLITE_DONE) {
        return shared::Result<void>::failure(
            databaseError("Failed to update transcript export paths.",
                          sqlite3_errmsg(database_.handle())));
    }

    return shared::Result<void>::success();
}

shared::Result<TranscriptRecord> TranscriptRepository::getByJobId(const std::string &jobId) const {
    sqlite3_stmt *statement = nullptr;
    const char *sql =
        "SELECT id, job_id, json_path, txt_path, srt_path, vtt_path, created_at "
        "FROM transcripts WHERE job_id = ?;";

    if (sqlite3_prepare_v2(database_.handle(), sql, -1, &statement, nullptr) != SQLITE_OK) {
        return shared::Result<TranscriptRecord>::failure(
            databaseError("Failed to prepare transcript lookup.", sqlite3_errmsg(database_.handle())));
    }

    sqlite3_bind_text(statement, 1, jobId.c_str(), -1, SQLITE_TRANSIENT);
    const int stepResult = sqlite3_step(statement);
    if (stepResult != SQLITE_ROW) {
        sqlite3_finalize(statement);
        return shared::Result<TranscriptRecord>::failure(
            databaseError("Transcript metadata not found.", jobId));
    }

    const TranscriptRecord record = readTranscriptRecord(statement);
    sqlite3_finalize(statement);
    return shared::Result<TranscriptRecord>::success(record);
}

} // namespace storage
