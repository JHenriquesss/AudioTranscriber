#include "audio/FfmpegArgs.hpp"

namespace audio {

namespace {

std::string pathArgument(const std::filesystem::path &path) {
    return path.generic_string();
}

} // namespace

std::vector<std::string> buildProbeArguments(const std::filesystem::path &inputPath) {
    return {
        "-hide_banner",
        "-i",
        pathArgument(inputPath),
        "-f",
        "null",
        "-",
    };
}

std::vector<std::string> buildExtractArguments(const std::filesystem::path &inputPath,
                                               const std::filesystem::path &outputPath) {
    return {
        "-y",
        "-i",
        pathArgument(inputPath),
        "-vn",
        "-acodec",
        "pcm_s16le",
        pathArgument(outputPath),
    };
}

std::vector<std::string> buildNormalizeArguments(const std::filesystem::path &inputPath,
                                                 const std::filesystem::path &outputPath) {
    return {
        "-y",
        "-i",
        pathArgument(inputPath),
        "-vn",
        "-ac",
        "1",
        "-ar",
        "16000",
        "-c:a",
        "pcm_s16le",
        pathArgument(outputPath),
    };
}

} // namespace audio
