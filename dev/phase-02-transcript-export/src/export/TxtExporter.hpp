#pragma once

#include "shared/Result.hpp"
#include "transcription/Transcript.hpp"

#include <filesystem>
#include <string>

namespace export_format {

class TxtExporter {
  public:
    static shared::Result<std::string> render(const transcription::TranscriptDocument &transcript);

    shared::Result<void> exportToFile(const transcription::TranscriptDocument &transcript,
                                      const std::filesystem::path &outputFile);
};

} // namespace export_format
