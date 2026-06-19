#include "app/JobRequestPaths.hpp"

#include "app/AppPaths.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path makePortableRoot(const std::string &name) {
    const auto root = std::filesystem::temp_directory_path() / name;
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "data");
    return root;
}

} // namespace

TEST_CASE("resolveJobOutputDirectory resolves relative portable exports folder", "[app][paths]") {
    const auto root = makePortableRoot("phase08-job-request-paths");
    const auto paths = app::AppPaths::resolve(root);

    const auto resolved = app::resolveJobOutputDirectory("exports", paths);

    REQUIRE(resolved == std::filesystem::absolute(root / "exports"));

    std::filesystem::remove_all(root);
}

TEST_CASE("exportBaseNameForInputFile preserves unicode stem", "[app][paths][unicode]") {
#if defined(_WIN32)
    const auto input = std::filesystem::path(L"r_tr\u00e1s_da_ordena\u00e7\u00e3o_eficiente.m4a");
#else
    const auto input = std::filesystem::path("r_trás_da_ordenação_eficiente.m4a");
#endif

    const auto baseName = app::exportBaseNameForInputFile(input);

    REQUIRE(baseName == "r_trás_da_ordenação_eficiente");
}

TEST_CASE("normalizeJobRequest resolves output directory against application root", "[app][paths]") {
    const auto root = makePortableRoot("phase08-job-request-normalize");
    const auto paths = app::AppPaths::resolve(root);

    app::TranscriptionJobRequest request;
    request.outputDirectory = "exports";
    request.language = "pt";
    request.modelId = "small";

    const auto normalized = app::normalizeJobRequest(request, paths);

    REQUIRE(normalized.outputDirectory == std::filesystem::absolute(root / "exports"));

    std::filesystem::remove_all(root);
}

TEST_CASE("ensureDirectoryExists creates missing output directory", "[app][paths]") {
    const auto root = makePortableRoot("phase08-job-request-create-dir");
    const auto output = root / "nested" / "exports";

    const auto result = app::ensureDirectoryExists(output);

    REQUIRE(result.ok);
    REQUIRE(std::filesystem::is_directory(output));

    std::filesystem::remove_all(root);
}
