#include "export/ExportService.hpp"

#include "export/JsonExporter.hpp"
#include "export/SrtExporter.hpp"
#include "export/TxtExporter.hpp"
#include "export/VttExporter.hpp"

namespace export_format {

namespace {

std::filesystem::path buildOutputPath(const ExportRequest &request, const std::string &extension) {
    return request.outputDirectory / (request.baseName + extension);
}

shared::Result<void> exportFormat(const transcription::TranscriptDocument &transcript,
                                  const ExportRequest &request, ExportFormat format) {
    switch (format) {
    case ExportFormat::Txt: {
        TxtExporter exporter;
        return exporter.exportToFile(transcript, buildOutputPath(request, ".txt"));
    }
    case ExportFormat::Srt: {
        SrtExporter exporter;
        return exporter.exportToFile(transcript, buildOutputPath(request, ".srt"));
    }
    case ExportFormat::Vtt: {
        VttExporter exporter;
        return exporter.exportToFile(transcript, buildOutputPath(request, ".vtt"));
    }
    case ExportFormat::Json: {
        JsonExporter exporter;
        return exporter.exportToFile(transcript, buildOutputPath(request, ".json"));
    }
    }

    return shared::Result<void>::failure(shared::AppError{
        shared::ErrorCode::ExportFailed,
        "Unsupported export format requested.",
        "ExportService received an unknown format value.",
    });
}

} // namespace

shared::Result<void>
ExportService::exportTranscript(const transcription::TranscriptDocument &transcript,
                                const ExportRequest &request) {
    for (const auto format : request.formats) {
        const auto result = exportFormat(transcript, request, format);
        if (!result.ok) {
            return result;
        }
    }

    return shared::Result<void>::success();
}

} // namespace export_format
