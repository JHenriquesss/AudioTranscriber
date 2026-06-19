#include "app/AppPaths.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

namespace {

std::filesystem::path makeTempRoot(const std::string &name) {
    const auto root = std::filesystem::temp_directory_path() / name;
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    return root;
}

} // namespace

TEST_CASE("AppPaths uses portable layout when data directory exists", "[app][paths]") {
    const auto root = makeTempRoot("phase03-portable-paths");
    std::filesystem::create_directories(root / "data");

    const auto paths = app::AppPaths::resolve(root);

    REQUIRE(paths.isPortableMode());
    REQUIRE(paths.settingsFile() == root / "data" / "settings.json");
    REQUIRE(paths.logsDirectory() == root / "data" / "logs");
    REQUIRE(paths.exportsDirectory() == root / "exports");

    std::filesystem::remove_all(root);
}

TEST_CASE("AppPaths uses injected user data root for installed layout", "[app][paths]") {
    const auto root = makeTempRoot("phase03-installed-paths");
    const auto paths = app::AppPaths::fromUserDataRoot(root, false);

    REQUIRE_FALSE(paths.isPortableMode());
    REQUIRE(paths.settingsFile() == root / "settings.json");
    REQUIRE(paths.tempDirectory() == root / "temp");
    REQUIRE(paths.databaseFile() == root / "app.db");

    std::filesystem::remove_all(root);
}

TEST_CASE("AppPaths uses portable database and transcript directories", "[app][paths]") {
    const auto root = makeTempRoot("phase07-portable-paths");
    std::filesystem::create_directories(root / "data");

    const auto paths = app::AppPaths::resolve(root);

    REQUIRE(paths.isPortableMode());
    REQUIRE(paths.databaseFile() == root / "data" / "app.db");
    REQUIRE(paths.transcriptsDirectory() == root / "data" / "transcripts");
    REQUIRE(paths.modelIndexFile() == root / "models" / "index.json");

    std::filesystem::remove_all(root);
}
