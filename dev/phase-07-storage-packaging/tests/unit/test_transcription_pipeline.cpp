#include "FakeWhisperEngine.hpp"
#include "WhisperHeaderGuard.hpp"

#include "transcription/TranscriptionPipeline.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

#ifndef PROJECT_SOURCE_ROOT
#error "PROJECT_SOURCE_ROOT must be defined for whisper header guard tests"
#endif

namespace {

std::filesystem::path projectSourceRoot() {
    return std::filesystem::path(PROJECT_SOURCE_ROOT);
}

std::filesystem::path createPipelineWorkspace() {
    const auto workspace =
        std::filesystem::temp_directory_path() / "phase05-transcription-pipeline";
    std::filesystem::create_directories(workspace);
    return workspace;
}

void writeTinyWav(const std::filesystem::path &path) {
    std::ofstream stream(path, std::ios::binary);
    stream << "RIFF";
    stream.write("\0\0\0\0", 4);
    stream << "WAVE";
}

void writeModelFile(const std::filesystem::path &modelsDirectory, const std::string &modelId) {
    std::filesystem::create_directories(modelsDirectory);
    std::ofstream stream(modelsDirectory / ("ggml-" + modelId + ".bin"), std::ios::binary);
    stream << "fake-model";
}

transcription::TranscriptionOptions buildOptions(const std::filesystem::path &workspace,
                                                 const std::string &language) {
    transcription::TranscriptionOptions options;
    options.jobId = "job-001";
    options.normalizedAudioPath = workspace / "normalized.wav";
    options.sourceFile = workspace / "source.mp4";
    options.language = language;
    options.modelId = "small";
    options.modelsDirectory = workspace / "models";
    return options;
}

} // namespace

TEST_CASE("Fake transcription pipeline produces transcript document", "[transcription][pipeline]") {
    const auto workspace = createPipelineWorkspace();
    writeTinyWav(workspace / "normalized.wav");
    writeModelFile(workspace / "models", "small");

    auto fakeEngine = std::make_unique<test_support::FakeWhisperEngine>();
    transcription::TranscriptionPipeline pipeline(whisper_engine::WhisperEngine(std::move(fakeEngine)));

    shared::CancellationToken token;
    const auto result = pipeline.run(buildOptions(workspace, "auto"), token);

    REQUIRE(result.ok);
    REQUIRE(result.value.transcript.id == "job-001");
    REQUIRE(result.value.transcript.segments.size() == 1);
    REQUIRE(result.value.transcript.segments.front().text == "hello world");
    REQUIRE(result.value.transcript.detectedLanguage == "en");

    std::filesystem::remove_all(workspace);
}

TEST_CASE("Missing model in pipeline returns ModelNotFound", "[transcription][pipeline]") {
    const auto workspace = createPipelineWorkspace();
    writeTinyWav(workspace / "normalized.wav");

    auto fakeEngine = std::make_unique<test_support::FakeWhisperEngine>();
    transcription::TranscriptionPipeline pipeline(whisper_engine::WhisperEngine(std::move(fakeEngine)));

    shared::CancellationToken token;
    const auto result = pipeline.run(buildOptions(workspace, "auto"), token);

    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.code == shared::ErrorCode::ModelNotFound);

    std::filesystem::remove_all(workspace);
}

TEST_CASE("Engine load failure returns ModelLoadFailed", "[transcription][pipeline]") {
    const auto workspace = createPipelineWorkspace();
    writeTinyWav(workspace / "normalized.wav");
    writeModelFile(workspace / "models", "small");

    auto fakeEngine = std::make_unique<test_support::FakeWhisperEngine>();
    fakeEngine->failLoad = true;

    transcription::TranscriptionPipeline pipeline(
        whisper_engine::WhisperEngine(std::move(fakeEngine)));

    shared::CancellationToken token;
    const auto result = pipeline.run(buildOptions(workspace, "en"), token);

    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.code == shared::ErrorCode::ModelLoadFailed);

    std::filesystem::remove_all(workspace);
}

TEST_CASE("Cancellation before pipeline returns Cancelled", "[transcription][pipeline]") {
    const auto workspace = createPipelineWorkspace();
    writeTinyWav(workspace / "normalized.wav");
    writeModelFile(workspace / "models", "small");

    auto fakeEngine = std::make_unique<test_support::FakeWhisperEngine>();
    transcription::TranscriptionPipeline pipeline(whisper_engine::WhisperEngine(std::move(fakeEngine)));

    shared::CancellationToken token;
    token.cancel();

    const auto result = pipeline.run(buildOptions(workspace, "pt"), token);

    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.code == shared::ErrorCode::Cancelled);

    std::filesystem::remove_all(workspace);
}

TEST_CASE("Cancellation during transcription returns Cancelled", "[transcription][pipeline]") {
    const auto workspace = createPipelineWorkspace();
    writeTinyWav(workspace / "normalized.wav");
    writeModelFile(workspace / "models", "small");

    auto fakeEngine = std::make_unique<test_support::FakeWhisperEngine>();
    fakeEngine->cancelBeforeTranscribe = true;

    transcription::TranscriptionPipeline pipeline(whisper_engine::WhisperEngine(std::move(fakeEngine)));

    shared::CancellationToken token;
    const auto result = pipeline.run(buildOptions(workspace, "auto"), token);

    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.code == shared::ErrorCode::Cancelled);

    std::filesystem::remove_all(workspace);
}

TEST_CASE("Whisper header guard finds no leaks outside whisper_engine", "[architecture][whisper]") {
    const auto violations = test_support::findWhisperHeaderLeaks(projectSourceRoot());

    REQUIRE(violations.empty());
}
