#pragma once

#include "shared/CancellationToken.hpp"
#include "shared/Result.hpp"
#include "transcription/TranscriptionOptions.hpp"
#include "transcription/TranscriptionResult.hpp"
#include "whisper_engine/WhisperEngine.hpp"

namespace transcription {

class TranscriptionPipeline {
  public:
    explicit TranscriptionPipeline(whisper_engine::WhisperEngine engine);

    [[nodiscard]] shared::Result<TranscriptionResult>
    run(const TranscriptionOptions &options, shared::CancellationToken &token);

  private:
    whisper_engine::WhisperEngine engine_;
};

} // namespace transcription
