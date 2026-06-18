#include "app/TranscriptionRequestValidation.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path writeTempInput() {
    const auto path = std::filesystem::temp_directory_path() / "phase06-request-validation-input.mp3";
    std::ofstream stream(path, std::ios::binary);
    stream << "fake";
    return path;
}

app::TranscriptionJobRequest validRequest() {
    const auto input = writeTempInput();
    app::TranscriptionJobRequest request;
    request.inputFile = input;
    request.outputDirectory = std::filesystem::temp_directory_path() / "phase06-request-validation";
    std::filesystem::create_directories(request.outputDirectory);
    request.language = "auto";
    request.modelId = "small";
    return request;
}

} // namespace

TEST_CASE("validateTranscriptionJobRequest accepts valid request", "[app][request-validation]") {
    const auto result = app::validateTranscriptionJobRequest(validRequest());
    REQUIRE(result.ok);
}

TEST_CASE("validateTranscriptionJobRequest rejects empty input file", "[app][request-validation]") {
    auto request = validRequest();
    request.inputFile.clear();

    const auto result = app::validateTranscriptionJobRequest(request);
    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.code == shared::ErrorCode::FileNotFound);
}

TEST_CASE("validateTranscriptionJobRequest rejects missing input file", "[app][request-validation]") {
    auto request = validRequest();
    request.inputFile = "missing-file.wav";

    const auto result = app::validateTranscriptionJobRequest(request);
    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.code == shared::ErrorCode::FileNotFound);
}

TEST_CASE("validateTranscriptionJobRequest rejects empty output directory",
          "[app][request-validation]") {
    auto request = validRequest();
    request.outputDirectory.clear();

    const auto result = app::validateTranscriptionJobRequest(request);
    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.code == shared::ErrorCode::ConfigurationError);
}

TEST_CASE("validateTranscriptionJobRequest rejects empty language", "[app][request-validation]") {
    auto request = validRequest();
    request.language.clear();

    const auto result = app::validateTranscriptionJobRequest(request);
    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.code == shared::ErrorCode::ConfigurationError);
}

TEST_CASE("validateTranscriptionJobRequest rejects empty model", "[app][request-validation]") {
    auto request = validRequest();
    request.modelId.clear();

    const auto result = app::validateTranscriptionJobRequest(request);
    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.code == shared::ErrorCode::ConfigurationError);
}
