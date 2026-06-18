#include "export/SrtExporter.hpp"

#include "export/ExportFileWrite.hpp"
#include "export/SubtitleTimestamp.hpp"
#include "transcription/TranscriptValidation.hpp"

#include <sstream>

namespace export_format {

shared::Result<std::string>
SrtExporter::render(const transcription::TranscriptDocument &transcript) {
    const auto validation = transcription::validateTranscript(transcript);
    if (!validation.ok) {
        return shared::Result<std::string>::failure(validation.error);
    }

    std::ostringstream stream;
    for (std::size_t index = 0; index < transcript.segments.size(); ++index) {
        const auto &segment = transcript.segments[index];
        if (index > 0) {
            stream << '\n';
        }

        stream << segment.index << '\n'
               << formatSrtTimestamp(segment.startMs) << " --> "
               << formatSrtTimestamp(segment.endMs) << '\n'
               << segment.text << '\n';
    }

    return shared::Result<std::string>::success(stream.str());
}

shared::Result<void> SrtExporter::exportToFile(const transcription::TranscriptDocument &transcript,
                                               const std::filesystem::path &outputFile) {
    const auto rendered = render(transcript);
    if (!rendered.ok) {
        return shared::Result<void>::failure(rendered.error);
    }

    return writeTextFile(outputFile, rendered.value);
}

} // namespace export_format
