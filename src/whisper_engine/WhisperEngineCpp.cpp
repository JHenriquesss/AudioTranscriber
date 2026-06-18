#include "whisper_engine/IWhisperEngine.hpp"
#include "whisper_engine/WhisperWavReader.hpp"

#include "shared/CancellationToken.hpp"

#include <whisper.h>

#include <memory>
#include <string>
#include <vector>

namespace whisper_engine {

namespace {

bool whisperAbortRequested(void *userData) {
    if (userData == nullptr) {
        return false;
    }
    const auto *token = static_cast<const shared::CancellationToken *>(userData);
    return token->isCancelled();
}

class WhisperCppEngine final : public IWhisperEngine {
  public:
    WhisperCppEngine() = default;
    ~WhisperCppEngine() override { unloadModel(); }

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

        unloadModel();

        whisper_context_params contextParams = whisper_context_default_params();
        contextParams.use_gpu = false;

        whisper_context *context =
            whisper_init_from_file_with_params(modelPath.string().c_str(), contextParams);
        if (context == nullptr) {
            return shared::Result<void>::failure(shared::AppError{
                shared::ErrorCode::ModelLoadFailed,
                "Whisper model could not be loaded.",
                "whisper_init_from_file_with_params failed for " + modelPath.string(),
            });
        }

        context_ = context;
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

        if (context_ == nullptr) {
            return shared::Result<std::vector<WhisperTranscriptSegment>>::failure(shared::AppError{
                shared::ErrorCode::ModelLoadFailed,
                "Whisper model is not loaded.",
                "stage=transcribe",
            });
        }

        const auto samplesResult = readMonoPcm16Wav(normalizedWavPath);
        if (!samplesResult.ok) {
            return shared::Result<std::vector<WhisperTranscriptSegment>>::failure(samplesResult.error);
        }

        whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
        params.print_progress = false;
        params.print_realtime = false;
        params.print_timestamps = false;
        params.token_timestamps = options.enableWordTimestamps;
        params.n_threads = 1;
        params.abort_callback = whisperAbortRequested;
        params.abort_callback_user_data = &token;

        if (options.languageCode == "auto") {
            params.language = nullptr;
            params.detect_language = true;
        } else {
            params.language = options.languageCode.c_str();
            params.detect_language = false;
        }

        const int whisperResult =
            whisper_full(context_, params, samplesResult.value.data(),
                         static_cast<int>(samplesResult.value.size()));
        if (token.isCancelled()) {
            return shared::Result<std::vector<WhisperTranscriptSegment>>::failure(shared::AppError{
                shared::ErrorCode::Cancelled,
                "Transcription was cancelled.",
                "stage=transcribe",
            });
        }
        if (whisperResult != 0) {
            return shared::Result<std::vector<WhisperTranscriptSegment>>::failure(shared::AppError{
                shared::ErrorCode::TranscriptionFailed,
                "Transcription failed.",
                "whisper_full returned " + std::to_string(whisperResult),
            });
        }

        const int segmentCount = whisper_full_n_segments(context_);
        if (segmentCount <= 0) {
            return shared::Result<std::vector<WhisperTranscriptSegment>>::failure(shared::AppError{
                shared::ErrorCode::TranscriptionFailed,
                "Transcription produced no segments.",
                normalizedWavPath.string(),
            });
        }

        std::string detectedLanguage = options.languageCode;
        if (options.languageCode == "auto") {
            const int languageId = whisper_full_lang_id(context_);
            if (const char *language = whisper_lang_str(languageId); language != nullptr) {
                detectedLanguage = language;
            }
        }

        std::vector<WhisperTranscriptSegment> segments;
        segments.reserve(static_cast<std::size_t>(segmentCount));
        for (int index = 0; index < segmentCount; ++index) {
            const char *text = whisper_full_get_segment_text(context_, index);
            if (text == nullptr || text[0] == '\0') {
                continue;
            }

            WhisperTranscriptSegment segment;
            segment.startMs = whisper_full_get_segment_t0(context_, index) * 10;
            segment.endMs = whisper_full_get_segment_t1(context_, index) * 10;
            segment.text = text;
            segment.detectedLanguage = detectedLanguage;
            segments.push_back(std::move(segment));
        }

        if (segments.empty()) {
            return shared::Result<std::vector<WhisperTranscriptSegment>>::failure(shared::AppError{
                shared::ErrorCode::TranscriptionFailed,
                "Transcription produced no segments.",
                normalizedWavPath.string(),
            });
        }

        return shared::Result<std::vector<WhisperTranscriptSegment>>::success(std::move(segments));
    }

  private:
    void unloadModel() {
        if (context_ != nullptr) {
            whisper_free(context_);
            context_ = nullptr;
        }
        loadedModelPath_.clear();
    }

    whisper_context *context_ = nullptr;
    std::filesystem::path loadedModelPath_;
};

} // namespace

std::unique_ptr<IWhisperEngine> createWhisperCppEngine() {
    return std::make_unique<WhisperCppEngine>();
}

} // namespace whisper_engine
