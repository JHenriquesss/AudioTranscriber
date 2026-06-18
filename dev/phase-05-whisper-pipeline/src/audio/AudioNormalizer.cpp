#include "audio/AudioNormalizer.hpp"

#include "audio/FfmpegArgs.hpp"

#include <fstream>

namespace audio {

namespace {

bool executableIsAvailable(const std::filesystem::path &executable) {
    if (executable.parent_path().empty() || executable.parent_path() == ".") {
        return true;
    }
    return std::filesystem::exists(executable);
}

shared::AppError ffmpegMissingError(const std::filesystem::path &ffmpegPath) {
    return shared::AppError{
        shared::ErrorCode::AudioNormalizationFailed,
        "FFmpeg is not available.",
        "FFmpeg executable was not found: " + ffmpegPath.generic_string(),
    };
}

shared::AppError inputMissingError(const std::filesystem::path &inputPath) {
    return shared::AppError{
        shared::ErrorCode::FileNotFound,
        "The selected media file was not found.",
        "Input path does not exist: " + inputPath.generic_string(),
    };
}

shared::AppError normalizationFailedError(const std::filesystem::path &inputPath,
                                          const std::string &details) {
    return shared::AppError{
        shared::ErrorCode::AudioNormalizationFailed,
        "The selected file could not be converted to audio.",
        "FFmpeg normalization failed for " + inputPath.generic_string() + ".\n" + details,
    };
}

bool isPcmWavHeaderValid(const std::filesystem::path &wavPath) {
    std::ifstream stream(wavPath, std::ios::binary);
    if (!stream.is_open()) {
        return false;
    }

    char riff[4] = {};
    std::uint32_t chunkSize = 0;
    char wave[4] = {};
    char fmt[4] = {};
    std::uint32_t fmtChunkSize = 0;
    std::uint16_t audioFormat = 0;
    std::uint16_t numChannels = 0;
    std::uint32_t sampleRate = 0;
    std::uint16_t bitsPerSample = 0;

    stream.read(riff, 4);
    stream.read(reinterpret_cast<char *>(&chunkSize), sizeof(chunkSize));
    stream.read(wave, 4);
    stream.read(fmt, 4);
    stream.read(reinterpret_cast<char *>(&fmtChunkSize), sizeof(fmtChunkSize));
    stream.read(reinterpret_cast<char *>(&audioFormat), sizeof(audioFormat));
    stream.read(reinterpret_cast<char *>(&numChannels), sizeof(numChannels));
    stream.read(reinterpret_cast<char *>(&sampleRate), sizeof(sampleRate));
    stream.ignore(4);
    stream.ignore(2);
    stream.read(reinterpret_cast<char *>(&bitsPerSample), sizeof(bitsPerSample));

    if (!stream) {
        return false;
    }

    return std::string(riff, 4) == "RIFF" && std::string(wave, 4) == "WAVE" &&
           std::string(fmt, 4) == "fmt " && audioFormat == 1 && numChannels == kNormalizedChannels &&
           sampleRate == static_cast<std::uint32_t>(kNormalizedSampleRate) &&
           bitsPerSample == 16;
}

} // namespace

AudioNormalizer::AudioNormalizer(AudioToolPaths toolPaths, AudioWorkspaceOptions workspaceOptions,
                                 platform::ProcessRunner processRunner)
    : toolPaths_(std::move(toolPaths)), workspaceOptions_(std::move(workspaceOptions)),
      processRunner_(std::move(processRunner)) {}

std::filesystem::path AudioNormalizer::buildOutputPath(const std::string &jobId) const {
    return workspaceOptions_.tempDirectory / ("audio_normalize_" + jobId + ".wav");
}

shared::Result<NormalizedAudioResult>
AudioNormalizer::normalize(const std::filesystem::path &inputPath, const std::string &jobId) const {
    if (!std::filesystem::exists(inputPath)) {
        return shared::Result<NormalizedAudioResult>::failure(inputMissingError(inputPath));
    }

    if (!executableIsAvailable(toolPaths_.ffmpegExecutable)) {
        return shared::Result<NormalizedAudioResult>::failure(
            ffmpegMissingError(toolPaths_.ffmpegExecutable));
    }

    std::filesystem::create_directories(workspaceOptions_.tempDirectory);
    const auto outputPath = buildOutputPath(jobId);

    platform::ProcessSpec spec{};
    spec.executable = toolPaths_.ffmpegExecutable;
    spec.arguments = buildNormalizeArguments(inputPath, outputPath);
    spec.timeout = std::chrono::minutes{10};

    const auto processResult = processRunner_.run(spec);
    if (!processResult.ok) {
        if (!workspaceOptions_.keepTempFiles && std::filesystem::exists(outputPath)) {
            std::filesystem::remove(outputPath);
        }

        if (processResult.error.code == shared::ErrorCode::FileNotFound) {
            return shared::Result<NormalizedAudioResult>::failure(
                ffmpegMissingError(toolPaths_.ffmpegExecutable));
        }

        return shared::Result<NormalizedAudioResult>::failure(
            normalizationFailedError(inputPath, processResult.error.technicalDetails));
    }

    if (!std::filesystem::exists(outputPath) || !isPcmWavHeaderValid(outputPath)) {
        if (!workspaceOptions_.keepTempFiles && std::filesystem::exists(outputPath)) {
            std::filesystem::remove(outputPath);
        }
        return shared::Result<NormalizedAudioResult>::failure(normalizationFailedError(
            inputPath, "FFmpeg reported success but output WAV is missing or invalid."));
    }

    NormalizedAudioResult result{};
    result.outputPath = outputPath;
    result.sampleRate = kNormalizedSampleRate;
    result.channels = kNormalizedChannels;
    return shared::Result<NormalizedAudioResult>::success(std::move(result));
}

} // namespace audio
