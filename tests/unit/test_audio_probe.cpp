#include "audio/AudioProbe.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

TEST_CASE("AudioProbe parseProbeOutput extracts audio stream metadata", "[audio][probe]") {
    const std::string ffmpegStderr =
        "Input #0, wav, from 'sample.wav':\n"
        "  Duration: 00:00:01.50, bitrate: 256 kb/s\n"
        "  Stream #0:0: Audio: pcm_s16le, 44100 Hz, stereo, s16, 256 kb/s\n";

    const auto parsed =
        audio::parseProbeOutput(std::filesystem::path("sample.wav"), ffmpegStderr);

    REQUIRE(parsed.audio.hasAudio);
    REQUIRE_FALSE(parsed.audio.hasVideo);
    REQUIRE(parsed.audio.codec == "pcm_s16le");
    REQUIRE(parsed.audio.sampleRate == 44100);
    REQUIRE(parsed.audio.channels == 2);
    REQUIRE(parsed.audio.durationSeconds == 1.5);
}

TEST_CASE("AudioProbe returns FileNotFound for missing input", "[audio][probe]") {
    audio::AudioProbe probe{{std::filesystem::path("ffmpeg.exe")}};
    const auto result = probe.probe(std::filesystem::path("missing/input.wav"));

    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.code == shared::ErrorCode::FileNotFound);
}

TEST_CASE("AudioProbe returns typed failure when FFmpeg is missing", "[audio][probe]") {
    audio::AudioProbe probe{{std::filesystem::path("missing/ffmpeg.exe")}};
    const auto tempInput = std::filesystem::temp_directory_path() / "phase04-probe-input.wav";
    std::ofstream(tempInput) << "not-a-real-wav";

    const auto result = probe.probe(tempInput);

    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.code == shared::ErrorCode::AudioProbeFailed);
    REQUIRE(result.error.message == "FFmpeg is not available.");

    std::filesystem::remove(tempInput);
}
