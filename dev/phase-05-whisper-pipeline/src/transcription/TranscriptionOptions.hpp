#pragma once

#include <filesystem>
#include <string>

namespace transcription {

struct TranscriptionOptions {
    std::string jobId;
    std::filesystem::path normalizedAudioPath;
    std::filesystem::path sourceFile;
    std::string language = "auto";
    std::string modelId = "small";
    std::filesystem::path modelsDirectory;
    bool enableWordTimestamps = true;
};

} // namespace transcription
