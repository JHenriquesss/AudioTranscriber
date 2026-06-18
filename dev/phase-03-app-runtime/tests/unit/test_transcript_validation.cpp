#include "transcription/TranscriptValidation.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("validateTranscript accepts empty transcript", "[transcript][validation]") {
    const transcription::TranscriptDocument document;

    const auto result = transcription::validateTranscript(document);

    REQUIRE(result.ok);
}

TEST_CASE("validateTranscript rejects segment with invalid timing", "[transcript][validation]") {
    transcription::TranscriptDocument document;
    document.segments.push_back(transcription::TranscriptSegment{
        1,
        2500,
        2500,
        "Invalid timing.",
        {},
    });

    const auto result = transcription::validateTranscript(document);

    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.code == shared::ErrorCode::ExportFailed);
}

TEST_CASE("validateTranscript rejects word with invalid timing", "[transcript][validation]") {
    transcription::TranscriptDocument document;
    document.segments.push_back(transcription::TranscriptSegment{
        1,
        0,
        2500,
        "Word timing issue.",
        {transcription::TranscriptWord{"bad", 900, 800, 0.5F}},
    });

    const auto result = transcription::validateTranscript(document);

    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.code == shared::ErrorCode::ExportFailed);
}
