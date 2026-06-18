#include "audio/AudioNormalizer.hpp"
#include "audio/AudioProbe.hpp"

#include "AudioTestHelpers.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

namespace {

struct TempCleanup {
    std::filesystem::path path;

    ~TempCleanup() {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }
};

bool readWavFormat(const std::filesystem::path &wavPath, std::uint16_t &channels,
                   std::uint32_t &sampleRate, std::uint16_t &bitsPerSample) {
    std::ifstream stream(wavPath, std::ios::binary);
    if (!stream) {
        return false;
    }

    char riff[4] = {};
    stream.read(riff, 4);
    stream.ignore(4);
    char wave[4] = {};
    stream.read(wave, 4);
    char fmt[4] = {};
    stream.read(fmt, 4);
    std::uint32_t fmtChunkSize = 0;
    std::uint16_t audioFormat = 0;
    stream.read(reinterpret_cast<char *>(&fmtChunkSize), sizeof(fmtChunkSize));
    stream.read(reinterpret_cast<char *>(&audioFormat), sizeof(audioFormat));
    stream.read(reinterpret_cast<char *>(&channels), sizeof(channels));
    stream.read(reinterpret_cast<char *>(&sampleRate), sizeof(sampleRate));
    stream.ignore(4);
    stream.ignore(2);
    stream.read(reinterpret_cast<char *>(&bitsPerSample), sizeof(bitsPerSample));

    return stream.good() && std::string(riff, 4) == "RIFF" && std::string(wave, 4) == "WAVE" &&
           audioFormat == 1;
}

} // namespace

TEST_CASE("Audio integration normalizes tiny fixture when FFmpeg is available",
          "[audio][integration]") {
    const auto ffmpegExecutable = audio_test::resolveFfmpegExecutable();
    if (!audio_test::isFfmpegAvailable(ffmpegExecutable)) {
        SKIP("FFmpeg is not available on this machine.");
    }

    const auto tempRoot = std::filesystem::temp_directory_path() / "phase04-audio-integration";
    TempCleanup cleanup{tempRoot};
    std::filesystem::create_directories(tempRoot);

    const auto inputPath = tempRoot / "tiny-input.wav";
    audio_test::writePcmMonoWav(inputPath, 44100, {0, 5000, -5000, 2500, -2500, 0});

    audio::AudioProbe probe{{ffmpegExecutable}};
    const auto probeResult = probe.probe(inputPath);
    REQUIRE(probeResult.ok);
    REQUIRE(probeResult.value.audio.hasAudio);

    audio::AudioNormalizer normalizer{{ffmpegExecutable}, {tempRoot / "work", true}};
    const auto normalizeResult = normalizer.normalize(inputPath, "integration_job");
    REQUIRE(normalizeResult.ok);

    const auto &outputPath = normalizeResult.value.outputPath;
    REQUIRE(std::filesystem::exists(outputPath));
    REQUIRE(outputPath.filename() == "audio_normalize_integration_job.wav");

    std::uint16_t channels = 0;
    std::uint32_t sampleRate = 0;
    std::uint16_t bitsPerSample = 0;
    REQUIRE(readWavFormat(outputPath, channels, sampleRate, bitsPerSample));
    REQUIRE(channels == 1);
    REQUIRE(sampleRate == 16000);
    REQUIRE(bitsPerSample == 16);
}
