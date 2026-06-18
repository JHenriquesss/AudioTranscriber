#include "audio/AudioProbe.hpp"

#include "audio/FfmpegArgs.hpp"

#include <cctype>
#include <regex>

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
        shared::ErrorCode::AudioProbeFailed,
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

shared::AppError probeFailedError(const std::filesystem::path &inputPath,
                                  const platform::ProcessOutput &output) {
    return shared::AppError{
        shared::ErrorCode::AudioProbeFailed,
        "The selected media file could not be inspected.",
        "FFmpeg probe failed for " + inputPath.generic_string() + " with exit code " +
            std::to_string(output.exitCode) + ".\n" + output.stderrText,
    };
}

bool containsCaseInsensitive(const std::string &haystack, const std::string &needle) {
    if (needle.empty()) {
        return true;
    }
    auto lowerHaystack = haystack;
    auto lowerNeedle = needle;
    for (char &character : lowerHaystack) {
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    }
    for (char &character : lowerNeedle) {
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    }
    return lowerHaystack.find(lowerNeedle) != std::string::npos;
}

} // namespace

AudioProbe::AudioProbe(AudioToolPaths toolPaths, platform::ProcessRunner processRunner)
    : toolPaths_(std::move(toolPaths)), processRunner_(std::move(processRunner)) {}

shared::Result<MediaProbeResult> AudioProbe::probe(const std::filesystem::path &inputPath) const {
    if (!std::filesystem::exists(inputPath)) {
        return shared::Result<MediaProbeResult>::failure(inputMissingError(inputPath));
    }

    if (!executableIsAvailable(toolPaths_.ffmpegExecutable)) {
        return shared::Result<MediaProbeResult>::failure(ffmpegMissingError(toolPaths_.ffmpegExecutable));
    }

    platform::ProcessSpec spec{};
    spec.executable = toolPaths_.ffmpegExecutable;
    spec.arguments = buildProbeArguments(inputPath);
    spec.timeout = std::chrono::seconds{30};

    const auto processResult = processRunner_.run(spec);
    if (!processResult.ok) {
        if (processResult.error.code == shared::ErrorCode::FileNotFound) {
            return shared::Result<MediaProbeResult>::failure(
                ffmpegMissingError(toolPaths_.ffmpegExecutable));
        }

        platform::ProcessOutput output{};
        output.exitCode = 1;
        output.stderrText = processResult.error.technicalDetails;
        return shared::Result<MediaProbeResult>::failure(probeFailedError(inputPath, output));
    }

    auto parsed = parseProbeOutput(inputPath, processResult.value.stderrText);
    if (!parsed.audio.hasAudio) {
        return shared::Result<MediaProbeResult>::failure(shared::AppError{
            shared::ErrorCode::UnsupportedMediaFormat,
            "The selected file does not contain an audio stream.",
            "FFmpeg probe found no audio stream in: " + inputPath.generic_string(),
        });
    }

    return shared::Result<MediaProbeResult>::success(std::move(parsed));
}

MediaProbeResult parseProbeOutput(const std::filesystem::path &inputPath,
                                  const std::string &ffmpegStderr) {
    MediaProbeResult result{};
    result.inputPath = inputPath;

    if (containsCaseInsensitive(ffmpegStderr, "video:")) {
        result.audio.hasVideo = true;
    }
    if (containsCaseInsensitive(ffmpegStderr, "audio:")) {
        result.audio.hasAudio = true;
    }

    static const std::regex durationRegex(R"(Duration:\s*(\d+):(\d+):(\d+(?:\.\d+)?))");
    std::smatch durationMatch;
    if (std::regex_search(ffmpegStderr, durationMatch, durationRegex) &&
        durationMatch.size() == 4) {
        const double hours = std::stod(durationMatch[1].str());
        const double minutes = std::stod(durationMatch[2].str());
        const double seconds = std::stod(durationMatch[3].str());
        result.audio.durationSeconds = hours * 3600.0 + minutes * 60.0 + seconds;
    }

    static const std::regex audioStreamRegex(
        R"(Audio:\s*([^,\s(]+)[^,\n]*,\s*([0-9]+)\s*Hz)");
    std::smatch audioMatch;
    if (std::regex_search(ffmpegStderr, audioMatch, audioStreamRegex) &&
        audioMatch.size() == 3) {
        result.audio.codec = audioMatch[1].str();
        result.audio.sampleRate = std::stoi(audioMatch[2].str());
    }

    static const std::regex sampleFormatRegex(R"(Audio:[^\n]*,\s*([0-9]+)\s*Hz,\s*([^,\n]+))");
    std::smatch sampleFormatMatch;
    if (std::regex_search(ffmpegStderr, sampleFormatMatch, sampleFormatRegex) &&
        sampleFormatMatch.size() == 3) {
        result.formatName = sampleFormatMatch[2].str();
    }

    static const std::regex channelRegex(R"(Audio:[^\n]*,\s*([0-9]+)\s*channels?)");
    std::smatch channelMatch;
    if (std::regex_search(ffmpegStderr, channelMatch, channelRegex) &&
        channelMatch.size() == 2) {
        result.audio.channels = std::stoi(channelMatch[1].str());
    } else if (containsCaseInsensitive(ffmpegStderr, "mono")) {
        result.audio.channels = 1;
    } else if (containsCaseInsensitive(ffmpegStderr, "stereo")) {
        result.audio.channels = 2;
    }

    return result;
}

} // namespace audio
