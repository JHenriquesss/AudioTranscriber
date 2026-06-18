#include "transcription/TranscriptValidation.hpp"

#include "shared/Error.hpp"

namespace transcription {

namespace {

shared::Result<void> validateWord(const TranscriptWord &word, int segmentIndex) {
    if (word.startMs < 0 || word.endMs < 0) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::ExportFailed,
            "Transcript word timing is negative.",
            "Segment " + std::to_string(segmentIndex) +
                " contains a word with negative "
                "start or end time.",
        });
    }

    if (word.endMs <= word.startMs) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::ExportFailed,
            "Transcript word timing is invalid.",
            "Segment " + std::to_string(segmentIndex) +
                " contains a word where end_ms is not after start_ms.",
        });
    }

    return shared::Result<void>::success();
}

shared::Result<void> validateSegment(const TranscriptSegment &segment) {
    if (segment.startMs < 0 || segment.endMs < 0) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::ExportFailed,
            "Transcript segment timing is negative.",
            "Segment " + std::to_string(segment.index) + " has negative start_ms or end_ms.",
        });
    }

    if (segment.endMs <= segment.startMs) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::ExportFailed,
            "Transcript segment timing is invalid.",
            "Segment " + std::to_string(segment.index) + " has end_ms that is not after start_ms.",
        });
    }

    for (const auto &word : segment.words) {
        const auto wordResult = validateWord(word, segment.index);
        if (!wordResult.ok) {
            return wordResult;
        }
    }

    return shared::Result<void>::success();
}

} // namespace

shared::Result<void> validateTranscript(const TranscriptDocument &document) {
    for (const auto &segment : document.segments) {
        const auto segmentResult = validateSegment(segment);
        if (!segmentResult.ok) {
            return segmentResult;
        }
    }

    return shared::Result<void>::success();
}

} // namespace transcription
