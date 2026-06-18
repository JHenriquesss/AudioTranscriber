#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace audio {

struct AudioStreamInfo {
    bool hasAudio = false;
    bool hasVideo = false;
    std::string codec;
    int channels = 0;
    int sampleRate = 0;
    double durationSeconds = 0.0;
};

struct MediaProbeResult {
    std::filesystem::path inputPath;
    std::string formatName;
    AudioStreamInfo audio;
};

struct NormalizedAudioResult {
    std::filesystem::path outputPath;
    int sampleRate = 16000;
    int channels = 1;
};

struct AudioToolPaths {
    std::filesystem::path ffmpegExecutable;
};

struct AudioWorkspaceOptions {
    std::filesystem::path tempDirectory;
    bool keepTempFiles = false;
};

constexpr int kNormalizedSampleRate = 16000;
constexpr int kNormalizedChannels = 1;

} // namespace audio
