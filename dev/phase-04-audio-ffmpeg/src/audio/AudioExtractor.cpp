#include "audio/AudioExtractor.hpp"

#include "audio/FfmpegArgs.hpp"

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
        shared::ErrorCode::AudioExtractionFailed,
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

shared::AppError extractionFailedError(const std::filesystem::path &inputPath,
                                       const std::string &details) {
    return shared::AppError{
        shared::ErrorCode::AudioExtractionFailed,
        "The selected file could not be converted to audio.",
        "FFmpeg extraction failed for " + inputPath.generic_string() + ".\n" + details,
    };
}

} // namespace

AudioExtractor::AudioExtractor(AudioToolPaths toolPaths, AudioWorkspaceOptions workspaceOptions,
                               platform::ProcessRunner processRunner)
    : toolPaths_(std::move(toolPaths)), workspaceOptions_(std::move(workspaceOptions)),
      processRunner_(std::move(processRunner)) {}

std::filesystem::path AudioExtractor::buildOutputPath(const std::string &jobId) const {
    return workspaceOptions_.tempDirectory / ("audio_extract_" + jobId + ".wav");
}

shared::Result<std::filesystem::path>
AudioExtractor::extract(const std::filesystem::path &inputPath, const std::string &jobId) const {
    if (!std::filesystem::exists(inputPath)) {
        return shared::Result<std::filesystem::path>::failure(inputMissingError(inputPath));
    }

    if (!executableIsAvailable(toolPaths_.ffmpegExecutable)) {
        return shared::Result<std::filesystem::path>::failure(
            ffmpegMissingError(toolPaths_.ffmpegExecutable));
    }

    std::filesystem::create_directories(workspaceOptions_.tempDirectory);
    const auto outputPath = buildOutputPath(jobId);

    platform::ProcessSpec spec{};
    spec.executable = toolPaths_.ffmpegExecutable;
    spec.arguments = buildExtractArguments(inputPath, outputPath);
    spec.timeout = std::chrono::minutes{10};

    const auto processResult = processRunner_.run(spec);
    if (!processResult.ok) {
        if (!workspaceOptions_.keepTempFiles && std::filesystem::exists(outputPath)) {
            std::filesystem::remove(outputPath);
        }

        if (processResult.error.code == shared::ErrorCode::FileNotFound) {
            return shared::Result<std::filesystem::path>::failure(
                ffmpegMissingError(toolPaths_.ffmpegExecutable));
        }

        return shared::Result<std::filesystem::path>::failure(
            extractionFailedError(inputPath, processResult.error.technicalDetails));
    }

    if (!std::filesystem::exists(outputPath)) {
        return shared::Result<std::filesystem::path>::failure(extractionFailedError(
            inputPath, "FFmpeg reported success but no output file was created."));
    }

    return shared::Result<std::filesystem::path>::success(outputPath);
}

} // namespace audio
