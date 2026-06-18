#include "app/AppPaths.hpp"
#include "app/ModelCatalog.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

#ifndef PROJECT_SOURCE_ROOT
#error "PROJECT_SOURCE_ROOT must be defined for model catalog tests"
#endif

namespace {

std::filesystem::path projectRoot() {
    return std::filesystem::path(PROJECT_SOURCE_ROOT);
}

} // namespace

TEST_CASE("ModelCatalog loads display entries from registry", "[app][models]") {
    const auto paths = app::AppPaths::fromUserDataRoot(projectRoot(), true);

    auto catalog = app::ModelCatalog::load(paths);
    REQUIRE(catalog.ok);
    REQUIRE_FALSE(catalog.value->entries().empty());

    const auto &first = catalog.value->entries().front();
    REQUIRE_FALSE(first.id.empty());
    REQUIRE_FALSE(first.name.empty());
}
