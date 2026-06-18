#include "TranscriptionRequestValidation.hpp"

namespace app {

shared::Result<void> validateTranscriptionJobRequest(const TranscriptionJobRequest &request) {
    if (request.inputFile.empty()) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::FileNotFound,
            "Select an audio or video file before starting.",
            "inputFile is empty",
        });
    }

    if (!std::filesystem::exists(request.inputFile)) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::FileNotFound,
            "The selected file was not found.",
            request.inputFile.string(),
        });
    }

    if (request.outputDirectory.empty()) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::ConfigurationError,
            "Select an output folder before starting.",
            "outputDirectory is empty",
        });
    }

    if (request.language.empty()) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::ConfigurationError,
            "Select a transcription language.",
            "language is empty",
        });
    }

    if (request.modelId.empty()) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::ConfigurationError,
            "Select a whisper model.",
            "modelId is empty",
        });
    }

    return shared::Result<void>::success();
}

} // namespace app
