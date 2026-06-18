#include "audio/AudioNormalizer.hpp"

#include "AudioTestHelpers.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

TEST_CASE("AudioNormalizer creates job-specific temp output paths", "[audio][normalizer]") {
    audio::AudioNormalizer normalizer{{std::filesystem::path("ffmpeg.exe")},
                                      {std::filesystem::path("C:/temp/audio")}};

    REQUIRE(normalizer.buildOutputPath("job_001").generic_string() ==
            std::filesystem::path("C:/temp/audio/audio_normalize_job_001.wav").generic_string());
    REQUIRE(normalizer.buildOutputPath("job_002").generic_string() !=
            normalizer.buildOutputPath("job_001").generic_string());
}

TEST_CASE("AudioNormalizer returns FileNotFound for missing input", "[audio][normalizer]") {
    audio::AudioNormalizer normalizer{{std::filesystem::path("ffmpeg.exe")},
                                      {std::filesystem::temp_directory_path() / "phase04-normalize"}};

    const auto result = normalizer.normalize(std::filesystem::path("missing/input.wav"), "job_1");

    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.code == shared::ErrorCode::FileNotFound);
}

TEST_CASE("AudioNormalizer returns typed failure when FFmpeg is missing", "[audio][normalizer]") {
    const auto tempRoot = std::filesystem::temp_directory_path() / "phase04-normalize-missing-ffmpeg";
    std::filesystem::create_directories(tempRoot);
    const auto inputPath = tempRoot / "input.wav";
    audio_test::writePcmMonoWav(inputPath, 44100, {0, 100, -100, 0});

    audio::AudioNormalizer normalizer{{std::filesystem::path("missing/ffmpeg.exe")},
                                      {tempRoot / "work"}};

    const auto result = normalizer.normalize(inputPath, "job_missing_ffmpeg");

    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.code == shared::ErrorCode::AudioNormalizationFailed);
    REQUIRE(result.error.message == "FFmpeg is not available.");

    std::filesystem::remove_all(tempRoot);
}

TEST_CASE("AudioNormalizer maps FFmpeg non-zero exit to AudioNormalizationFailed",
          "[audio][normalizer]") {
    const auto ffmpegExecutable = audio_test::resolveFfmpegExecutable();
    if (!audio_test::isFfmpegAvailable(ffmpegExecutable)) {
        SKIP("FFmpeg is not available on this machine.");
    }

    const auto tempRoot = std::filesystem::temp_directory_path() / "phase04-normalize-failure";
    std::filesystem::create_directories(tempRoot);
    const auto inputPath = tempRoot / "invalid-media.bin";
    std::ofstream(inputPath) << "this is not a media file";

    audio::AudioNormalizer normalizer{{ffmpegExecutable}, {tempRoot / "work"}};
    const auto result = normalizer.normalize(inputPath, "job_failure");

    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.code == shared::ErrorCode::AudioNormalizationFailed);
    REQUIRE(result.error.message ==
            "The selected file could not be converted to audio.");
    REQUIRE(result.error.technicalDetails.find("FFmpeg normalization failed") != std::string::npos);
    REQUIRE_FALSE(std::filesystem::exists(tempRoot / "work" / "audio_normalize_job_failure.wav"));

    std::filesystem::remove_all(tempRoot);
}
