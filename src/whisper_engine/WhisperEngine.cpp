#include "whisper_engine/WhisperEngine.hpp"

namespace whisper_engine {

struct WhisperEngine::Impl {
    explicit Impl(std::unique_ptr<IWhisperEngine> engine)
        : engine(std::move(engine)) {}

    std::unique_ptr<IWhisperEngine> engine;
};

WhisperEngine::WhisperEngine() : impl_(std::make_unique<Impl>(createWhisperCppEngine())) {}

WhisperEngine::WhisperEngine(std::unique_ptr<IWhisperEngine> implementation)
    : impl_(std::make_unique<Impl>(std::move(implementation))) {}

WhisperEngine::WhisperEngine(WhisperEngine &&) noexcept = default;
WhisperEngine &WhisperEngine::operator=(WhisperEngine &&) noexcept = default;
WhisperEngine::~WhisperEngine() = default;

shared::Result<void> WhisperEngine::loadModel(const std::filesystem::path &modelPath,
                                               shared::CancellationToken &token) {
    return impl_->engine->loadModel(modelPath, token);
}

shared::Result<std::vector<WhisperTranscriptSegment>>
WhisperEngine::transcribe(const std::filesystem::path &normalizedWavPath,
                          const WhisperRuntimeOptions &options,
                          shared::CancellationToken &token) {
    return impl_->engine->transcribe(normalizedWavPath, options, token);
}

} // namespace whisper_engine
