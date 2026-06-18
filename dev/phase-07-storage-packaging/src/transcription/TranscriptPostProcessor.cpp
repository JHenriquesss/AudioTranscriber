#include "transcription/TranscriptPostProcessor.hpp"

#include <cctype>
#include <sstream>

namespace transcription {

namespace {

std::string normalizeWhitespace(std::string text) {
    std::string normalized;
    normalized.reserve(text.size());
    bool previousWasSpace = false;

    for (const char character : text) {
        if (std::isspace(static_cast<unsigned char>(character)) != 0) {
            if (!previousWasSpace && !normalized.empty()) {
                normalized.push_back(' ');
                previousWasSpace = true;
            }
            continue;
        }

        normalized.push_back(character);
        previousWasSpace = false;
    }

    if (!normalized.empty() && normalized.back() == ' ') {
        normalized.pop_back();
    }

    return normalized;
}

} // namespace

TranscriptDocument postProcessTranscript(const TranscriptDocument &document) {
    TranscriptDocument processed = document;

    for (auto &segment : processed.segments) {
        segment.text = normalizeWhitespace(segment.text);
        for (auto &word : segment.words) {
            word.text = normalizeWhitespace(word.text);
        }
    }

    return processed;
}

} // namespace transcription
