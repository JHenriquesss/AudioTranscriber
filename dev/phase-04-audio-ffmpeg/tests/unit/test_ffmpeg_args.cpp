#include "audio/FfmpegArgs.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

TEST_CASE("FFmpeg normalize arguments target 16 kHz mono PCM WAV", "[audio][ffmpeg-args]") {
    const auto arguments =
        audio::buildNormalizeArguments(std::filesystem::path("input/video.mp4"),
                                       std::filesystem::path("output/normalized.wav"));

    REQUIRE(arguments ==
            std::vector<std::string>{
                "-y",
                "-i",
                "input/video.mp4",
                "-vn",
                "-ac",
                "1",
                "-ar",
                "16000",
                "-c:a",
                "pcm_s16le",
                "output/normalized.wav",
            });
}

TEST_CASE("FFmpeg arguments keep untrusted paths as separate argv entries",
          "[audio][ffmpeg-args]") {
    const auto arguments = audio::buildNormalizeArguments(
        std::filesystem::path(R"(C:/unsafe"; malicious.exe)"),
        std::filesystem::path("safe/output.wav"));

    REQUIRE(arguments[1] == "-i");
    REQUIRE(arguments[2] == R"(C:/unsafe"; malicious.exe)");
    REQUIRE(arguments.back() == "safe/output.wav");
}

TEST_CASE("FFmpeg probe and extract argument builders are deterministic",
          "[audio][ffmpeg-args]") {
    const auto probeArgs = audio::buildProbeArguments(std::filesystem::path("clip.wav"));
    REQUIRE(probeArgs ==
            std::vector<std::string>{"-hide_banner", "-i", "clip.wav", "-f", "null", "-"});

    const auto extractArgs =
        audio::buildExtractArguments(std::filesystem::path("clip.mp4"),
                                     std::filesystem::path("temp/extract.wav"));
    REQUIRE(extractArgs ==
            std::vector<std::string>{"-y", "-i", "clip.mp4", "-vn", "-acodec", "pcm_s16le",
                                     "temp/extract.wav"});
}
