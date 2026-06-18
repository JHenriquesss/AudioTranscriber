#include "whisper_engine/WhisperEngine.hpp"

namespace whisper_engine {

namespace {

class StubWhisperEngine final : public IWhisperEngine {
  public:
    shared::Result<void> loadModel(const std::filesystem::path &modelPath,
                                   shared::CancellationToken &token) override {
        if (token.isCancelled()) {
            return shared::Result<void>::failure(shared::AppError{
                shared::ErrorCode::Cancelled,
                "Transcription was cancelled before the model could be loaded.",
                "stage=model_load",
            });
        }

        if (!std::filesystem::is_regular_file(modelPath)) {
            return shared::Result<void>::failure(shared::AppError{
                shared::ErrorCode::ModelLoadFailed,
                "Whisper model could not be loaded.",
                "modelPath=" + modelPath.string(),
            });
        }

        loadedModelPath_ = modelPath;
        return shared::Result<void>::success();
    }

    shared::Result<std::vector<WhisperTranscriptSegment>>
    transcribe(const std::filesystem::path &normalizedWavPath,
               const WhisperRuntimeOptions &options,
               shared::CancellationToken &token) override {
        if (token.isCancelled()) {
            return shared::Result<std::vector<WhisperTranscriptSegment>>::failure(shared::AppError{
                shared::ErrorCode::Cancelled,
                "Transcription was cancelled.",
                "stage=transcribe",
            });
        }

        if (loadedModelPath_.empty()) {
            return shared::Result<std::vector<WhisperTranscriptSegment>>::failure(shared::AppError{
                shared::ErrorCode::ModelLoadFailed,
                "Whisper model is not loaded.",
                "stage=transcribe",
            });
        }

        if (!std::filesystem::is_regular_file(normalizedWavPath)) {
            return shared::Result<std::vector<WhisperTranscriptSegment>>::failure(shared::AppError{
                shared::ErrorCode::FileNotFound,
                "Normalized audio file was not found.",
                "normalizedWavPath=" + normalizedWavPath.string(),
            });
        }

        WhisperTranscriptSegment segment;
        segment.startMs = 0;
        segment.endMs = 1'000;
        segment.text = "stub transcription";
        segment.detectedLanguage = options.languageCode == "auto" ? "en" : options.languageCode;
        return shared::Result<std::vector<WhisperTranscriptSegment>>::success({segment});
    }

  private:
    std::filesystem::path loadedModelPath_;
};

} // namespace

std::unique_ptr<IWhisperEngine> createStubWhisperEngine() {
    return std::make_unique<StubWhisperEngine>();
}

struct WhisperEngine::Impl {
    explicit Impl(std::unique_ptr<IWhisperEngine> engine)
        : engine(std::move(engine)) {}

    std::unique_ptr<IWhisperEngine> engine;
};

WhisperEngine::WhisperEngine() : impl_(std::make_unique<Impl>(createStubWhisperEngine())) {}

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
