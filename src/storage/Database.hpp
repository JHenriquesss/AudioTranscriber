#pragma once

#include "shared/Result.hpp"

#include <filesystem>
#include <memory>

struct sqlite3;

namespace storage {

class Database {
  public:
    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;
    Database(Database &&) noexcept;
    Database &operator=(Database &&) noexcept;
    ~Database();

    [[nodiscard]] static shared::Result<std::unique_ptr<Database>>
    open(const std::filesystem::path &databasePath);
    [[nodiscard]] shared::Result<void> runMigrations();
    [[nodiscard]] shared::Result<void> execute(const char *sql);

    [[nodiscard]] sqlite3 *handle() const;

  private:
    explicit Database(sqlite3 *connection);

    struct Deleter {
        void operator()(sqlite3 *connection) const;
    };

    std::unique_ptr<sqlite3, Deleter> connection_;
};

} // namespace storage
