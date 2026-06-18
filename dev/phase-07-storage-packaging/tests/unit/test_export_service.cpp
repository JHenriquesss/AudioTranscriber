#include "export/ExportService.hpp"
#include "export/SrtExporter.hpp"

#include "FixtureIO.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

namespace {

transcription::TranscriptDocument makeMinimalTranscript() {
    transcription::TranscriptDocument document;
    document.id = "job_min";
    document.segments.push_back(transcription::TranscriptSegment{
        1,
        0,
        1000,
        "One second.",
        {},
    });
    return document;
}

} // namespace

TEST_CASE("ExportService writes only requested formats", "[export][service]") {
    const auto transcript = makeMinimalTranscript();
    const auto outputDirectory =
        std::filesystem::temp_directory_path() / "phase02_export_service_only_srt";
    std::filesystem::create_directories(outputDirectory);

    export_format::ExportRequest request;
    request.outputDirectory = outputDirectory;
    request.baseName = "transcript";
    request.formats = {export_format::ExportFormat::Srt};

    export_format::ExportService service;
    const auto result = service.exportTranscript(transcript, request);

    REQUIRE(result.ok);
    REQUIRE(std::filesystem::exists(outputDirectory / "transcript.srt"));
    REQUIRE_FALSE(std::filesystem::exists(outputDirectory / "transcript.txt"));
    REQUIRE_FALSE(std::filesystem::exists(outputDirectory / "transcript.vtt"));
    REQUIRE_FALSE(std::filesystem::exists(outputDirectory / "transcript.json"));

    std::filesystem::remove_all(outputDirectory);
}

TEST_CASE("unwritable output path maps to ExportFailed", "[export][service][negative]") {
    const auto transcript = makeMinimalTranscript();
    export_format::SrtExporter exporter;

#if defined(_WIN32)
    const std::filesystem::path outputFile("C:\\invalid<>\\export.srt");
#else
    const std::filesystem::path outputFile("/dev/null/invalid/export.srt");
#endif

    const auto result = exporter.exportToFile(transcript, outputFile);

    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.code == shared::ErrorCode::ExportFailed);
}
