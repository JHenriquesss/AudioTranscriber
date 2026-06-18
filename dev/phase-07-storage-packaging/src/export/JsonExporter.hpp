#pragma once

#include "shared/Result.hpp"
#include "transcription/Transcript.hpp"

#include <filesystem>
#include <string>

namespace export_format {

class JsonExporter {
  public:
    static shared::Result<std::string> render(const transcription::TranscriptDocument &transcript);
    static shared::Result<transcription::TranscriptDocument> parse(const std::string &json);

    shared::Result<void> exportToFile(const transcription::TranscriptDocument &transcript,
                                      const std::filesystem::path &outputFile);
};

} // namespace export_format
