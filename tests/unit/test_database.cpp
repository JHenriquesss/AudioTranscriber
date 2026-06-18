#include "StorageTestHelpers.hpp"
#include "storage/Database.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <memory>

namespace {

std::unique_ptr<storage::Database> openFreshDatabase(const std::filesystem::path &databasePath) {
    auto opened = storage::Database::open(databasePath);
    REQUIRE(opened.ok);
    const auto migrated = opened.value->runMigrations();
    REQUIRE(migrated.ok);
    return std::move(opened.value);
}

} // namespace

TEST_CASE("Database initializes schema on first launch", "[storage][database]") {
    const auto root = test_support::uniqueTempRoot("phase07-db-init");
    const auto databasePath = root / "data" / "app.db";

    {
        auto database = openFreshDatabase(databasePath);
        const auto migratedAgain = database->runMigrations();
        REQUIRE(migratedAgain.ok);
        REQUIRE(std::filesystem::exists(databasePath));
    }

    test_support::cleanupTempRoot(root);
}

TEST_CASE("Database migrations are idempotent", "[storage][database]") {
    const auto root = test_support::uniqueTempRoot("phase07-db-migrations");
    const auto databasePath = root / "app.db";

    {
        auto database = openFreshDatabase(databasePath);
        REQUIRE(database->runMigrations().ok);
        REQUIRE(database->runMigrations().ok);
    }

    test_support::cleanupTempRoot(root);
}

TEST_CASE("Database open failure maps to DatabaseError", "[storage][database]") {
    const auto root = test_support::uniqueTempRoot("phase07-db-failure");
    std::filesystem::create_directories(root / "blocked");

    const auto opened = storage::Database::open(root / "blocked");
    REQUIRE_FALSE(opened.ok);
    REQUIRE(opened.error.code == shared::ErrorCode::DatabaseError);

    test_support::cleanupTempRoot(root);
}
