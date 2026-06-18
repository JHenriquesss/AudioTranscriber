#pragma once

#include "shared/CancellationToken.hpp"
#include "shared/Result.hpp"
#include "whisper_engine/IWhisperEngine.hpp"

#include <filesystem>
#include <vector>

namespace test_support {

class FakeWhisperEngine final : public whisper_engine::IWhisperEngine {
  public:
    bool failLoad = false;
    bool failTranscribe = false;
    bool cancelBeforeTranscribe = false;
    int loadCallCount = 0;
    int transcribeCallCount = 0;
    std::vector<whisper_engine::WhisperTranscriptSegment> segments = {
        whisper_engine::WhisperTranscriptSegment{
            0,
            1'500,
            "hello  world",
            "en",
        },
    };

    shared::Result<void> loadModel(const std::filesystem::path &modelPath,
                                   shared::CancellationToken &token) override {
        ++loadCallCount;
        if (token.isCancelled()) {
            return shared::Result<void>::failure(shared::AppError{
                shared::ErrorCode::Cancelled,
                "Transcription was cancelled before the model could be loaded.",
                "stage=model_load",
            });
        }

        if (failLoad) {
            return shared::Result<void>::failure(shared::AppError{
                shared::ErrorCode::ModelLoadFailed,
                "Fake engine simulated model load failure.",
                "modelPath=" + modelPath.string(),
            });
        }

        loadedModelPath_ = modelPath;
        return shared::Result<void>::success();
    }

    shared::Result<std::vector<whisper_engine::WhisperTranscriptSegment>>
    transcribe(const std::filesystem::path &normalizedWavPath,
               const whisper_engine::WhisperRuntimeOptions &options,
               shared::CancellationToken &token) override {
        ++transcribeCallCount;
        (void)normalizedWavPath;
        (void)options;

        if (cancelBeforeTranscribe) {
            token.cancel();
        }

        if (token.isCancelled()) {
            return shared::Result<std::vector<whisper_engine::WhisperTranscriptSegment>>::failure(
                shared::AppError{
                    shared::ErrorCode::Cancelled,
                    "Transcription was cancelled.",
                    "stage=transcribe",
                });
        }

        if (failTranscribe) {
            return shared::Result<std::vector<whisper_engine::WhisperTranscriptSegment>>::failure(
                shared::AppError{
                    shared::ErrorCode::TranscriptionFailed,
                    "Fake engine simulated transcription failure.",
                    "stage=transcribe",
                });
        }

        return shared::Result<std::vector<whisper_engine::WhisperTranscriptSegment>>::success(
            segments);
    }

    [[nodiscard]] bool isModelLoaded() const { return !loadedModelPath_.empty(); }

  private:
    std::filesystem::path loadedModelPath_;
};

} // namespace test_support
