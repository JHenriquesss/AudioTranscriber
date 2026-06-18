#include "whisper_engine/WhisperModel.hpp"

namespace whisper_engine {

namespace {

shared::AppError modelNotFoundError(const std::string &modelId,
                                    const std::filesystem::path &modelsDirectory) {
    return shared::AppError{
        shared::ErrorCode::ModelNotFound,
        "Whisper model file was not found.",
        "modelId=" + modelId + "; modelsDirectory=" + modelsDirectory.string(),
    };
}

} // namespace

WhisperModel::WhisperModel(std::filesystem::path modelsDirectory)
    : modelsDirectory_(std::move(modelsDirectory)) {}

shared::Result<std::filesystem::path>
WhisperModel::resolveModelPath(const std::string &modelId) const {
    if (modelId.empty()) {
        return shared::Result<std::filesystem::path>::failure(
            modelNotFoundError(modelId, modelsDirectory_));
    }

    const std::filesystem::path candidates[] = {
        modelsDirectory_ / ("ggml-" + modelId + ".bin"),
        modelsDirectory_ / (modelId + ".bin"),
        modelsDirectory_ / ("ggml-" + modelId + ".gguf"),
        modelsDirectory_ / (modelId + ".gguf"),
    };

    for (const auto &candidate : candidates) {
        if (std::filesystem::is_regular_file(candidate)) {
            return shared::Result<std::filesystem::path>::success(candidate);
        }
    }

    return shared::Result<std::filesystem::path>::failure(
        modelNotFoundError(modelId, modelsDirectory_));
}

} // namespace whisper_engine
