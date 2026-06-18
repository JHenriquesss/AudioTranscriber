#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace transcription {

struct TranscriptWord {
    std::string text;
    int64_t startMs = 0;
    int64_t endMs = 0;
    float probability = 0.0f;
};

struct TranscriptSegment {
    int index = 0;
    int64_t startMs = 0;
    int64_t endMs = 0;
    std::string text;
    std::vector<TranscriptWord> words;
};

struct TranscriptDocument {
    std::string id;
    std::filesystem::path sourceFile;
    std::string detectedLanguage;
    std::string requestedLanguage;
    std::string modelId;
    int64_t durationMs = 0;
    std::string createdAtIso;
    std::vector<TranscriptSegment> segments;
};

} // namespace transcription
