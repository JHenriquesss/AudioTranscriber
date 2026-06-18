#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace whisper_engine {

struct WhisperTranscriptSegment {
    int64_t startMs = 0;
    int64_t endMs = 0;
    std::string text;
    std::string detectedLanguage;
};

struct WhisperRuntimeOptions {
    std::string languageCode;
    bool enableWordTimestamps = true;
};

} // namespace whisper_engine
