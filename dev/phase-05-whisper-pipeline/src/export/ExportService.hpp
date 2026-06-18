#pragma once

#include "shared/Result.hpp"
#include "transcription/Transcript.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace export_format {

enum class ExportFormat { Txt, Srt, Vtt, Json };

struct ExportRequest {
    std::filesystem::path outputDirectory;
    std::string baseName;
    std::vector<ExportFormat> formats;
};

class ExportService {
  public:
    shared::Result<void> exportTranscript(const transcription::TranscriptDocument &transcript,
                                          const ExportRequest &request);
};

} // namespace export_format
