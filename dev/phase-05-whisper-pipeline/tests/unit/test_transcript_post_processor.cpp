#include "transcription/TranscriptPostProcessor.hpp"

#include <catch2/catch_test_macros.hpp>

namespace {

transcription::TranscriptDocument sampleDocument() {
    transcription::TranscriptDocument document;
    transcription::TranscriptSegment segment;
    segment.index = 0;
    segment.text = "  hello   world  ";
    transcription::TranscriptWord word;
    word.text = "  hello  ";
    segment.words.push_back(word);
    document.segments.push_back(segment);
    return document;
}

} // namespace

TEST_CASE("Transcript post-processor normalizes whitespace deterministically",
            "[transcription][postprocess]") {
    const auto processed = transcription::postProcessTranscript(sampleDocument());

    REQUIRE(processed.segments.size() == 1);
    REQUIRE(processed.segments.front().text == "hello world");
    REQUIRE(processed.segments.front().words.front().text == "hello");
}

TEST_CASE("Transcript post-processor is idempotent", "[transcription][postprocess]") {
    const auto once = transcription::postProcessTranscript(sampleDocument());
    const auto twice = transcription::postProcessTranscript(once);

    REQUIRE(twice.segments.front().text == once.segments.front().text);
}
