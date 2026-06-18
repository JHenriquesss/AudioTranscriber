#include "shared/Error.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Failed error preserves typed code and message", "[error]") {
    shared::AppError error;
    error.code = shared::ErrorCode::AudioNormalizationFailed;
    error.message = "The selected file could not be converted to audio.";
    error.technicalDetails = "FFmpeg exited with code 1.";

    REQUIRE(error.code == shared::ErrorCode::AudioNormalizationFailed);
    REQUIRE(error.message == "The selected file could not be converted to audio.");
    REQUIRE(error.technicalDetails == "FFmpeg exited with code 1.");
}

TEST_CASE("ErrorCode maps to stable string labels", "[error]") {
    REQUIRE(shared::to_string(shared::ErrorCode::Unknown) == "Unknown");
    REQUIRE(shared::to_string(shared::ErrorCode::FileNotFound) == "FileNotFound");
    REQUIRE(shared::to_string(shared::ErrorCode::Cancelled) == "Cancelled");
}
