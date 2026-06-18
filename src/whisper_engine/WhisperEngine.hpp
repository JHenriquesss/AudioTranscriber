#pragma once

#include "whisper_engine/IWhisperEngine.hpp"

#include <memory>

namespace whisper_engine {

class WhisperEngine final : public IWhisperEngine {
  public:
    WhisperEngine();
    explicit WhisperEngine(std::unique_ptr<IWhisperEngine> implementation);

    WhisperEngine(const WhisperEngine &) = delete;
    WhisperEngine &operator=(const WhisperEngine &) = delete;
    WhisperEngine(WhisperEngine &&) noexcept;
    WhisperEngine &operator=(WhisperEngine &&) noexcept;
    ~WhisperEngine() override;

    shared::Result<void> loadModel(const std::filesystem::path &modelPath,
                                   shared::CancellationToken &token) override;

    shared::Result<std::vector<WhisperTranscriptSegment>>
    transcribe(const std::filesystem::path &normalizedWavPath,
               const WhisperRuntimeOptions &options,
               shared::CancellationToken &token) override;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

std::unique_ptr<IWhisperEngine> createWhisperCppEngine();

} // namespace whisper_engine
