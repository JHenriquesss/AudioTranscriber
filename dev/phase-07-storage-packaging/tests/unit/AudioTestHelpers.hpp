#pragma once

#include "platform/ProcessRunner.hpp"

#include <cstdint>
#include <cstdlib>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace audio_test {

inline void writeUint32(std::ofstream &stream, std::uint32_t value) {
    stream.write(reinterpret_cast<const char *>(&value), sizeof(value));
}

inline void writeUint16(std::ofstream &stream, std::uint16_t value) {
    stream.write(reinterpret_cast<const char *>(&value), sizeof(value));
}

inline void writePcmMonoWav(const std::filesystem::path &outputPath, int sampleRate,
                            const std::vector<std::int16_t> &samples) {
    const std::uint32_t dataSize =
        static_cast<std::uint32_t>(samples.size() * sizeof(std::int16_t));
    const std::uint32_t riffSize = 36 + dataSize;
    const std::uint16_t audioFormat = 1;
    const std::uint16_t numChannels = 1;
    const std::uint32_t byteRate = static_cast<std::uint32_t>(sampleRate * numChannels * 2);
    const std::uint16_t blockAlign = numChannels * 2;
    const std::uint16_t bitsPerSample = 16;

    std::ofstream stream(outputPath, std::ios::binary | std::ios::trunc);
    stream.write("RIFF", 4);
    writeUint32(stream, riffSize);
    stream.write("WAVE", 4);
    stream.write("fmt ", 4);
    writeUint32(stream, 16);
    writeUint16(stream, audioFormat);
    writeUint16(stream, numChannels);
    writeUint32(stream, static_cast<std::uint32_t>(sampleRate));
    writeUint32(stream, byteRate);
    writeUint16(stream, blockAlign);
    writeUint16(stream, bitsPerSample);
    stream.write("data", 4);
    writeUint32(stream, dataSize);
    stream.write(reinterpret_cast<const char *>(samples.data()),
                 static_cast<std::streamsize>(dataSize));
}

inline std::filesystem::path resolveFfmpegExecutable() {
    if (const char *fromEnv = std::getenv("FFMPEG_PATH")) {
        const std::filesystem::path configured(fromEnv);
        if (std::filesystem::exists(configured)) {
            return configured;
        }
    }

    platform::ProcessRunner runner{};
    platform::ProcessSpec spec{};
    spec.executable = std::filesystem::path("where.exe");
    spec.arguments = {"ffmpeg"};
    spec.timeout = std::chrono::seconds{10};
    const auto whereResult = runner.run(spec);
    if (whereResult.ok) {
        std::istringstream stream(whereResult.value.stdoutText);
        std::string line;
        if (std::getline(stream, line) && !line.empty()) {
            while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' ')) {
                line.pop_back();
            }
            return std::filesystem::path(line);
        }
    }

    return std::filesystem::path("ffmpeg");
}

inline bool isFfmpegAvailable(const std::filesystem::path &ffmpegExecutable) {
    platform::ProcessRunner runner{};
    platform::ProcessSpec spec{};
    spec.executable = ffmpegExecutable;
    spec.arguments = {"-version"};
    spec.timeout = std::chrono::seconds{10};
    return runner.run(spec).ok;
}

} // namespace audio_test
