#include "whisper_engine/WhisperModel.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path createModelDirectory() {
    const auto directory =
        std::filesystem::temp_directory_path() / "phase05-whisper-model-tests";
    std::filesystem::create_directories(directory);
    return directory;
}

void writePlaceholderModel(const std::filesystem::path &path) {
    std::ofstream stream(path, std::ios::binary);
    stream << "fake-model";
}

} // namespace

TEST_CASE("Whisper model resolver finds ggml model file", "[whisper][model]") {
    const auto modelsDirectory = createModelDirectory();
    const auto modelPath = modelsDirectory / "ggml-small.bin";
    writePlaceholderModel(modelPath);

    whisper_engine::WhisperModel modelResolver(modelsDirectory);
    const auto resolved = modelResolver.resolveModelPath("small");

    REQUIRE(resolved.ok);
    REQUIRE(resolved.value == modelPath);

    std::filesystem::remove_all(modelsDirectory);
}

TEST_CASE("Missing whisper model returns ModelNotFound", "[whisper][model]") {
    const auto modelsDirectory = createModelDirectory();

    whisper_engine::WhisperModel modelResolver(modelsDirectory);
    const auto resolved = modelResolver.resolveModelPath("missing-model");

    REQUIRE_FALSE(resolved.ok);
    REQUIRE(resolved.error.code == shared::ErrorCode::ModelNotFound);
    REQUIRE(resolved.error.technicalDetails.find("missing-model") != std::string::npos);

    std::filesystem::remove_all(modelsDirectory);
}
