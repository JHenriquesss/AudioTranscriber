#pragma once

#include "shared/CancellationToken.hpp"
#include "shared/Result.hpp"
#include "whisper_engine/WhisperTypes.hpp"

#include <filesystem>
#include <vector>

namespace whisper_engine {

class IWhisperEngine {
  public:
    virtual ~IWhisperEngine() = default;

    virtual shared::Result<void> loadModel(const std::filesystem::path &modelPath,
                                           shared::CancellationToken &token) = 0;

    virtual shared::Result<std::vector<WhisperTranscriptSegment>>
    transcribe(const std::filesystem::path &normalizedWavPath,
               const WhisperRuntimeOptions &options, shared::CancellationToken &token) = 0;
};

} // namespace whisper_engine
