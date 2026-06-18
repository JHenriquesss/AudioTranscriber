#pragma once

#include "transcription/Transcript.hpp"

#include <cmath>

namespace test_support {

inline bool transcriptDocumentsEqual(const transcription::TranscriptDocument &left,
                                     const transcription::TranscriptDocument &right) {
    if (left.id != right.id || left.sourceFile != right.sourceFile ||
        left.detectedLanguage != right.detectedLanguage ||
        left.requestedLanguage != right.requestedLanguage || left.modelId != right.modelId ||
        left.durationMs != right.durationMs || left.createdAtIso != right.createdAtIso ||
        left.segments.size() != right.segments.size()) {
        return false;
    }

    for (std::size_t index = 0; index < left.segments.size(); ++index) {
        const auto &leftSegment = left.segments[index];
        const auto &rightSegment = right.segments[index];
        if (leftSegment.index != rightSegment.index ||
            leftSegment.startMs != rightSegment.startMs ||
            leftSegment.endMs != rightSegment.endMs || leftSegment.text != rightSegment.text ||
            leftSegment.words.size() != rightSegment.words.size()) {
            return false;
        }

        for (std::size_t wordIndex = 0; wordIndex < leftSegment.words.size(); ++wordIndex) {
            const auto &leftWord = leftSegment.words[wordIndex];
            const auto &rightWord = rightSegment.words[wordIndex];
            if (leftWord.text != rightWord.text || leftWord.startMs != rightWord.startMs ||
                leftWord.endMs != rightWord.endMs ||
                std::fabs(leftWord.probability - rightWord.probability) > 0.0001F) {
                return false;
            }
        }
    }

    return true;
}

} // namespace test_support
