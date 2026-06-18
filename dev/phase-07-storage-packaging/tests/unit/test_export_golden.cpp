#include "FixtureIO.hpp"
#include "TranscriptTestHelpers.hpp"

#include "export/JsonExporter.hpp"
#include "export/SrtExporter.hpp"
#include "export/TxtExporter.hpp"
#include "export/VttExporter.hpp"

#include <catch2/catch_test_macros.hpp>

namespace {

transcription::TranscriptDocument loadSampleTranscript() {
    const auto fixturePath = test_support::fixturesRoot() / "transcript-sample.json";
    const auto jsonText = test_support::readTextFile(fixturePath);
    const auto parsed = export_format::JsonExporter::parse(jsonText);
    REQUIRE(parsed.ok);
    return parsed.value;
}

} // namespace

TEST_CASE("sample transcript exports to golden TXT", "[export][golden]") {
    const auto transcript = loadSampleTranscript();
    const auto expected = test_support::readTextFile(test_support::fixturesRoot() / "expected.txt");

    const auto rendered = export_format::TxtExporter::render(transcript);

    REQUIRE(rendered.ok);
    REQUIRE(rendered.value == expected);
}

TEST_CASE("sample transcript exports to golden SRT", "[export][golden]") {
    const auto transcript = loadSampleTranscript();
    const auto expected = test_support::readTextFile(test_support::fixturesRoot() / "expected.srt");

    const auto rendered = export_format::SrtExporter::render(transcript);

    REQUIRE(rendered.ok);
    REQUIRE(rendered.value == expected);
}

TEST_CASE("sample transcript exports to golden VTT", "[export][golden]") {
    const auto transcript = loadSampleTranscript();
    const auto expected = test_support::readTextFile(test_support::fixturesRoot() / "expected.vtt");

    const auto rendered = export_format::VttExporter::render(transcript);

    REQUIRE(rendered.ok);
    REQUIRE(rendered.value == expected);
}

TEST_CASE("sample transcript exports to golden JSON with word metadata", "[export][golden]") {
    const auto transcript = loadSampleTranscript();
    const auto expected =
        test_support::readTextFile(test_support::fixturesRoot() / "expected.json");

    const auto rendered = export_format::JsonExporter::render(transcript);

    REQUIRE(rendered.ok);
    REQUIRE(rendered.value == expected);
    REQUIRE_FALSE(transcript.segments.empty());
    REQUIRE(transcript.segments.front().words.size() == 2);
}

TEST_CASE("JsonExporter roundtrips transcript document", "[export][json]") {
    const auto original = loadSampleTranscript();

    const auto rendered = export_format::JsonExporter::render(original);
    REQUIRE(rendered.ok);

    const auto parsed = export_format::JsonExporter::parse(rendered.value);
    REQUIRE(parsed.ok);
    REQUIRE(test_support::transcriptDocumentsEqual(original, parsed.value));
}

TEST_CASE("empty transcript exports predictable empty outputs", "[export][empty]") {
    const transcription::TranscriptDocument document;

    const auto txt = export_format::TxtExporter::render(document);
    const auto srt = export_format::SrtExporter::render(document);
    const auto vtt = export_format::VttExporter::render(document);
    const auto json = export_format::JsonExporter::render(document);

    REQUIRE(txt.ok);
    REQUIRE(txt.value.empty());
    REQUIRE(srt.ok);
    REQUIRE(srt.value.empty());
    REQUIRE(vtt.ok);
    REQUIRE(vtt.value == "WEBVTT\n");
    REQUIRE(json.ok);
    const auto parsed = export_format::JsonExporter::parse(json.value);
    REQUIRE(parsed.ok);
    REQUIRE(parsed.value.segments.empty());
}

TEST_CASE("invalid segment timing returns export failure from renderer", "[export][negative]") {
    transcription::TranscriptDocument document;
    document.segments.push_back(transcription::TranscriptSegment{
        1,
        1000,
        500,
        "Backwards timing.",
        {},
    });

    const auto rendered = export_format::SrtExporter::render(document);

    REQUIRE_FALSE(rendered.ok);
    REQUIRE(rendered.error.code == shared::ErrorCode::ExportFailed);
}
