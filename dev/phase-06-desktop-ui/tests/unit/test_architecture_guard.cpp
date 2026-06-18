#include "ArchitectureGuard.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <string>

#ifndef PROJECT_SOURCE_ROOT
#error "PROJECT_SOURCE_ROOT must be defined for architecture guard tests"
#endif

namespace {

std::filesystem::path projectSourceRoot() {
    return std::filesystem::path(PROJECT_SOURCE_ROOT);
}

} // namespace

TEST_CASE("Architecture guard passes with feature-oriented folders only", "[architecture]") {
    const auto violations = test_support::findForbiddenArchitectureFolders(projectSourceRoot());

    REQUIRE(violations.empty());
}

TEST_CASE("Architecture guard detects forbidden Clean Architecture folders", "[architecture]") {
    const auto tempRoot = std::filesystem::temp_directory_path() / "phase01-arch-guard";
    std::filesystem::create_directories(tempRoot / "src" / "domain" / "nested");

    const auto violations = test_support::findForbiddenArchitectureFolders(tempRoot);

    REQUIRE(violations.size() == 1);
    REQUIRE(violations.front().filename() == "domain");

    std::filesystem::remove_all(tempRoot);
}
