#include "whisper_engine/WhisperOptionsMapper.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Whisper options mapper handles auto language", "[whisper][options]") {
    REQUIRE(whisper_engine::mapLanguageToWhisperCode("auto") == "auto");
    REQUIRE(whisper_engine::mapLanguageToWhisperCode("AUTO") == "auto");
}

TEST_CASE("Whisper options mapper handles Portuguese language", "[whisper][options]") {
    REQUIRE(whisper_engine::mapLanguageToWhisperCode("pt") == "pt");
    REQUIRE(whisper_engine::mapLanguageToWhisperCode("portuguese") == "pt");
}

TEST_CASE("Whisper options mapper handles English language", "[whisper][options]") {
    REQUIRE(whisper_engine::mapLanguageToWhisperCode("en") == "en");
    REQUIRE(whisper_engine::mapLanguageToWhisperCode("english") == "en");
}
