#include "app/AppSettings.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path makeTempRoot(const std::string &name) {
    const auto root = std::filesystem::temp_directory_path() / name;
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "data");
    return root;
}

} // namespace

TEST_CASE("AppSettingsStore creates defaults when settings file is missing", "[app][settings]") {
    const auto root = makeTempRoot("phase03-settings-defaults");
    app::AppSettingsStore store(app::AppPaths::fromUserDataRoot(root, true));

    const auto loaded = store.loadOrCreateDefaults();

    REQUIRE(loaded.ok);
    REQUIRE(loaded.value.defaultLanguage == "auto");
    REQUIRE(loaded.value.defaultModel == "small");
    REQUIRE(loaded.value.maxParallelJobs == 1);
    REQUIRE(loaded.value.enableWordTimestamps);
    REQUIRE(loaded.value.theme == "system");
    REQUIRE(loaded.value.outputDirectory == std::filesystem::absolute(root / "exports"));
    REQUIRE(std::filesystem::exists(root / "data" / "settings.json"));

    std::filesystem::remove_all(root);
}

TEST_CASE("AppSettingsStore rejects invalid configuration", "[app][settings]") {
    const auto root = makeTempRoot("phase03-settings-invalid");
    const auto settingsPath = root / "data" / "settings.json";
    std::ofstream(settingsPath) << R"({
  "default_language": "fr",
  "default_model": "small",
  "output_directory": "exports",
  "keep_temp_files": false,
  "max_parallel_jobs": 1,
  "enable_word_timestamps": true,
  "theme": "system"
})";

    app::AppSettingsStore store(app::AppPaths::fromUserDataRoot(root, true));
    const auto loaded = store.load();

    REQUIRE_FALSE(loaded.ok);
    REQUIRE(loaded.error.code == shared::ErrorCode::ConfigurationError);

    std::filesystem::remove_all(root);
}

TEST_CASE("AppSettingsStore round-trips valid settings", "[app][settings]") {
    const auto root = makeTempRoot("phase03-settings-roundtrip");
    app::AppSettingsStore store(app::AppPaths::fromUserDataRoot(root, true));

    app::AppSettings settings;
    settings.defaultLanguage = "pt";
    settings.defaultModel = "medium";
    settings.outputDirectory = "custom-exports";
    settings.maxParallelJobs = 2;
    settings.theme = "dark";

    const auto saved = store.save(settings);
    REQUIRE(saved.ok);

    const auto loaded = store.load();
    REQUIRE(loaded.ok);
    REQUIRE(loaded.value.defaultLanguage == "pt");
    REQUIRE(loaded.value.defaultModel == "medium");
    REQUIRE(loaded.value.outputDirectory ==
            std::filesystem::absolute(root / "custom-exports"));
    REQUIRE(loaded.value.maxParallelJobs == 2);
    REQUIRE(loaded.value.theme == "dark");

    std::filesystem::remove_all(root);
}

TEST_CASE("AppSettingsStore load fails when settings file is missing", "[app][settings]") {
    const auto root = makeTempRoot("phase03-settings-missing");
    app::AppSettingsStore store(app::AppPaths::fromUserDataRoot(root, true));

    const auto loaded = store.load();

    REQUIRE_FALSE(loaded.ok);
    REQUIRE(loaded.error.code == shared::ErrorCode::ConfigurationError);

    std::filesystem::remove_all(root);
}

TEST_CASE("AppSettingsStore rejects max_parallel_jobs below one", "[app][settings]") {
    app::AppSettings settings = app::AppSettingsStore::defaults();
    settings.maxParallelJobs = 0;

    const auto validated = app::validateSettings(settings);

    REQUIRE_FALSE(validated.ok);
    REQUIRE(validated.error.code == shared::ErrorCode::ConfigurationError);
}
