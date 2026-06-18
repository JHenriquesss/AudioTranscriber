#include "whisper_engine/WhisperWavReader.hpp"

#include <array>
#include <cstring>
#include <fstream>

namespace whisper_engine {

namespace {

shared::AppError wavError(std::string message, std::string details = {}) {
    return shared::AppError{
        shared::ErrorCode::TranscriptionFailed,
        std::move(message),
        std::move(details),
    };
}

bool readExact(std::ifstream &stream, void *buffer, std::size_t size) {
    stream.read(static_cast<char *>(buffer), static_cast<std::streamsize>(size));
    return stream.gcount() == static_cast<std::streamsize>(size);
}

} // namespace

shared::Result<std::vector<float>>
readMonoPcm16Wav(const std::filesystem::path &wavPath, int expectedSampleRateHz) {
    if (!std::filesystem::is_regular_file(wavPath)) {
        return shared::Result<std::vector<float>>::failure(
            wavError("Normalized audio file was not found.", wavPath.string()));
    }

    std::ifstream stream(wavPath, std::ios::binary);
    if (!stream) {
        return shared::Result<std::vector<float>>::failure(
            wavError("Failed to open normalized audio file.", wavPath.string()));
    }

    char riffHeader[4]{};
    std::uint32_t riffSize = 0;
    char waveHeader[4]{};
    if (!readExact(stream, riffHeader, 4) || std::strncmp(riffHeader, "RIFF", 4) != 0 ||
        !readExact(stream, &riffSize, sizeof(riffSize)) ||
        !readExact(stream, waveHeader, 4) || std::strncmp(waveHeader, "WAVE", 4) != 0) {
        return shared::Result<std::vector<float>>::failure(
            wavError("Audio file is not a valid WAV container.", wavPath.string()));
    }

    std::uint16_t audioFormat = 0;
    std::uint16_t numChannels = 0;
    std::uint32_t sampleRate = 0;
    std::uint16_t bitsPerSample = 0;
    bool foundFmt = false;
    bool foundData = false;
    std::uint32_t dataSize = 0;
    std::streampos dataOffset = 0;

    while (stream && !(foundFmt && foundData)) {
        char chunkId[4]{};
        std::uint32_t chunkSize = 0;
        if (!readExact(stream, chunkId, 4) || !readExact(stream, &chunkSize, sizeof(chunkSize))) {
            break;
        }

        if (std::strncmp(chunkId, "fmt ", 4) == 0) {
            if (chunkSize < 16) {
                return shared::Result<std::vector<float>>::failure(
                    wavError("WAV fmt chunk is too small.", wavPath.string()));
            }
        if (!readExact(stream, &audioFormat, sizeof(audioFormat)) ||
            !readExact(stream, &numChannels, sizeof(numChannels)) ||
            !readExact(stream, &sampleRate, sizeof(sampleRate))) {
            return shared::Result<std::vector<float>>::failure(
                wavError("Failed to read WAV format chunk.", wavPath.string()));
        }
        if (chunkSize >= 16) {
            stream.seekg(6, std::ios::cur);
            if (!readExact(stream, &bitsPerSample, sizeof(bitsPerSample))) {
                return shared::Result<std::vector<float>>::failure(
                    wavError("Failed to read WAV bits per sample.", wavPath.string()));
            }
            if (chunkSize > 16) {
                stream.seekg(chunkSize - 16, std::ios::cur);
            }
        } else {
            stream.seekg(chunkSize - 6, std::ios::cur);
        }
        foundFmt = true;
        continue;
        }

        if (std::strncmp(chunkId, "data", 4) == 0) {
            dataSize = chunkSize;
            dataOffset = stream.tellg();
            foundData = true;
            stream.seekg(chunkSize, std::ios::cur);
            continue;
        }

        stream.seekg(chunkSize, std::ios::cur);
    }

    if (!foundFmt || !foundData) {
        return shared::Result<std::vector<float>>::failure(
            wavError("WAV file is missing required fmt or data chunks.", wavPath.string()));
    }

    if (audioFormat != 1 || numChannels != 1 || bitsPerSample != 16) {
        return shared::Result<std::vector<float>>::failure(wavError(
            "Expected 16-bit mono PCM WAV audio.",
            "format=" + std::to_string(audioFormat) + "; channels=" + std::to_string(numChannels)));
    }

    if (static_cast<int>(sampleRate) != expectedSampleRateHz) {
        return shared::Result<std::vector<float>>::failure(wavError(
            "Normalized audio sample rate does not match the transcription engine.",
            "expected_hz=" + std::to_string(expectedSampleRateHz) +
                "; actual_hz=" + std::to_string(sampleRate)));
    }

    stream.clear();
    stream.seekg(dataOffset);
    std::vector<std::int16_t> pcmSamples(dataSize / sizeof(std::int16_t));
    if (!readExact(stream, pcmSamples.data(), dataSize)) {
        return shared::Result<std::vector<float>>::failure(
            wavError("Failed to read WAV PCM samples.", wavPath.string()));
    }

    std::vector<float> samples;
    samples.reserve(pcmSamples.size());
    for (const std::int16_t sample : pcmSamples) {
        samples.push_back(static_cast<float>(sample) / 32768.0F);
    }

    return shared::Result<std::vector<float>>::success(std::move(samples));
}

} // namespace whisper_engine
