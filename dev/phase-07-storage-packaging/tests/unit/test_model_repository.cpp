#include "storage/ModelRepository.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

#ifndef PROJECT_SOURCE_ROOT
#error "PROJECT_SOURCE_ROOT must be defined for model repository tests"
#endif

namespace {

std::filesystem::path projectRoot() {
    return std::filesystem::path(PROJECT_SOURCE_ROOT);
}

std::filesystem::path writeInvalidIndex(const std::filesystem::path &directory) {
    const auto indexPath = directory / "index.json";
    std::ofstream stream(indexPath);
    stream << R"({"models":[{"id":"broken"}]})";
    return indexPath;
}

} // namespace

TEST_CASE("ModelRepository loads models/index.json contract", "[storage][models]") {
    const auto loaded = storage::ModelRepository::loadIndexFile(projectRoot() / "models" / "index.json");
    REQUIRE(loaded.ok);
    REQUIRE(loaded.value.size() >= 3);

    const auto small = storage::ModelRepository::findById(loaded.value, "small");
    REQUIRE(small.ok);
    REQUIRE(small.value.name == "Fast");
    REQUIRE(small.value.relativePath.filename() == "ggml-small.bin");
}

TEST_CASE("Invalid model index returns ConfigurationError", "[storage][models]") {
    const auto tempRoot = std::filesystem::temp_directory_path() / "phase07-invalid-model-index";
    std::filesystem::create_directories(tempRoot);
    const auto indexPath = writeInvalidIndex(tempRoot);

    const auto loaded = storage::ModelRepository::loadIndexFile(indexPath);
    REQUIRE_FALSE(loaded.ok);
    REQUIRE(loaded.error.code == shared::ErrorCode::ConfigurationError);

    std::filesystem::remove_all(tempRoot);
}

TEST_CASE("Unknown model id returns ModelNotFound", "[storage][models]") {
    const auto loaded = storage::ModelRepository::loadIndexFile(projectRoot() / "models" / "index.json");
    REQUIRE(loaded.ok);

    const auto missing = storage::ModelRepository::findById(loaded.value, "does-not-exist");
    REQUIRE_FALSE(missing.ok);
    REQUIRE(missing.error.code == shared::ErrorCode::ModelNotFound);
}
