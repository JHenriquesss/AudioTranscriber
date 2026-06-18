#include "storage/Database.hpp"

#include <sqlite3.h>

#include <filesystem>

namespace storage {

namespace {

shared::AppError databaseError(std::string message, std::string details = {}) {
    return shared::AppError{
        shared::ErrorCode::DatabaseError,
        std::move(message),
        std::move(details),
    };
}

constexpr int kCurrentSchemaVersion = 1;

const char *kMigrationStatements[] = {
    "CREATE TABLE IF NOT EXISTS schema_version ("
    "  version INTEGER NOT NULL"
    ");",
    "CREATE TABLE IF NOT EXISTS app_settings ("
    "  key TEXT PRIMARY KEY,"
    "  value TEXT NOT NULL"
    ");",
    "CREATE TABLE IF NOT EXISTS models ("
    "  id TEXT PRIMARY KEY,"
    "  name TEXT NOT NULL,"
    "  path TEXT NOT NULL,"
    "  size_bytes INTEGER,"
    "  installed_at TEXT NOT NULL"
    ");",
    "CREATE TABLE IF NOT EXISTS jobs ("
    "  id TEXT PRIMARY KEY,"
    "  source_file TEXT NOT NULL,"
    "  status TEXT NOT NULL,"
    "  model_id TEXT NOT NULL,"
    "  requested_language TEXT NOT NULL,"
    "  detected_language TEXT,"
    "  created_at TEXT NOT NULL,"
    "  completed_at TEXT,"
    "  error_code TEXT,"
    "  error_message TEXT"
    ");",
    "CREATE TABLE IF NOT EXISTS transcripts ("
    "  id TEXT PRIMARY KEY,"
    "  job_id TEXT NOT NULL,"
    "  json_path TEXT NOT NULL,"
    "  txt_path TEXT,"
    "  srt_path TEXT,"
    "  vtt_path TEXT,"
    "  created_at TEXT NOT NULL,"
    "  FOREIGN KEY(job_id) REFERENCES jobs(id)"
    ");",
};

shared::Result<void> executeStatement(sqlite3 *connection, const char *sql) {
    char *errorMessage = nullptr;
    const int result = sqlite3_exec(connection, sql, nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        const std::string details = errorMessage != nullptr ? errorMessage : "Unknown SQLite error.";
        sqlite3_free(errorMessage);
        return shared::Result<void>::failure(databaseError("Database statement failed.", details));
    }
    return shared::Result<void>::success();
}

shared::Result<int> readSchemaVersion(sqlite3 *connection) {
    sqlite3_stmt *statement = nullptr;
    if (sqlite3_prepare_v2(connection, "SELECT version FROM schema_version LIMIT 1;", -1, &statement,
                           nullptr) != SQLITE_OK) {
        return shared::Result<int>::failure(
            databaseError("Failed to read schema version.", sqlite3_errmsg(connection)));
    }

    int version = 0;
    const int stepResult = sqlite3_step(statement);
    if (stepResult == SQLITE_ROW) {
        version = sqlite3_column_int(statement, 0);
    } else if (stepResult != SQLITE_DONE) {
        sqlite3_finalize(statement);
        return shared::Result<int>::failure(
            databaseError("Failed to read schema version.", sqlite3_errmsg(connection)));
    }

    sqlite3_finalize(statement);
    return shared::Result<int>::success(version);
}

} // namespace

void Database::Deleter::operator()(sqlite3 *connection) const {
    if (connection != nullptr) {
        sqlite3_close(connection);
    }
}

Database::Database(sqlite3 *connection) : connection_(connection) {}

Database::Database(Database &&other) noexcept = default;
Database &Database::operator=(Database &&other) noexcept = default;
Database::~Database() = default;

shared::Result<std::unique_ptr<Database>> Database::open(const std::filesystem::path &databasePath) {
    if (databasePath.empty()) {
        return shared::Result<std::unique_ptr<Database>>::failure(
            databaseError("Database path is empty.", databasePath.string()));
    }

    if (std::filesystem::exists(databasePath) && std::filesystem::is_directory(databasePath)) {
        return shared::Result<std::unique_ptr<Database>>::failure(
            databaseError("Database path points to a directory.", databasePath.string()));
    }

    const auto parent = databasePath.parent_path();
    if (!parent.empty() && !std::filesystem::exists(parent)) {
        std::error_code errorCode;
        std::filesystem::create_directories(parent, errorCode);
        if (errorCode) {
            return shared::Result<std::unique_ptr<Database>>::failure(
                databaseError("Failed to create database directory.", errorCode.message()));
        }
    }

    sqlite3 *connection = nullptr;
    const int openResult =
        sqlite3_open_v2(databasePath.string().c_str(), &connection,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);
    if (openResult != SQLITE_OK) {
        const std::string details = connection != nullptr ? sqlite3_errmsg(connection) : "Open failed.";
        if (connection != nullptr) {
            sqlite3_close(connection);
        }
        return shared::Result<std::unique_ptr<Database>>::failure(
            databaseError("Failed to open database.", details));
    }

    return shared::Result<std::unique_ptr<Database>>::success(
        std::unique_ptr<Database>(new Database(connection)));
}

shared::Result<void> Database::runMigrations() {
    if (connection_ == nullptr) {
        return shared::Result<void>::failure(databaseError("Database is not open."));
    }

    for (const char *statement : kMigrationStatements) {
        const auto executed = executeStatement(connection_.get(), statement);
        if (!executed.ok) {
            return executed;
        }
    }

    const auto versionResult = readSchemaVersion(connection_.get());
    if (!versionResult.ok) {
        return shared::Result<void>::failure(versionResult.error);
    }

    if (versionResult.value == 0) {
        const auto inserted = executeStatement(connection_.get(),
                                               "INSERT INTO schema_version(version) VALUES (1);");
        if (!inserted.ok) {
            return inserted;
        }
    } else if (versionResult.value != kCurrentSchemaVersion) {
        return shared::Result<void>::failure(databaseError(
            "Unsupported schema version.", std::to_string(versionResult.value)));
    }

    return shared::Result<void>::success();
}

shared::Result<void> Database::execute(const char *sql) {
    if (connection_ == nullptr) {
        return shared::Result<void>::failure(databaseError("Database is not open."));
    }
    return executeStatement(connection_.get(), sql);
}

sqlite3 *Database::handle() const {
    return connection_.get();
}

} // namespace storage
